#include "VulkanRenderer.hpp"
#include "Iris/Platform/Vulkan/Util.hpp"

namespace Iris {
    VulkanRenderer::VulkanRenderer(const WindowOptions& opts) : Renderer(opts) {

    }

    void VulkanRenderer::Init() {
        InitDevice();
        InitQueues();
        InitSwapchain();
        InitDepthBuffer();
        InitRenderPass();
        InitFramebuffers();
    }

    void VulkanRenderer::InitDevice() {
        vkb::InstanceBuilder instanceBuilder;

        auto instance = instanceBuilder
                .set_app_name(m_Window.GetTitle().data())
                .set_engine_name("Iris")
                .set_debug_callback([](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                       void* pUserData) -> VkBool32 {
                                        auto type = vkb::to_string_message_type(messageType);
                                        auto msg = std::string_view(pCallbackData->pMessage);

                                        switch (messageSeverity) {
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                                                Log::Core::Error("{}: {}\n", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                                                Log::Core::Warn("{}: {}\n", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                                                Log::Core::Info("{}: {}\n", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                                                Log::Core::Trace("{}: {}\n", type, msg);
                                                break;
                                            default:
                                                break;
                                        }
                                        return VK_FALSE;
                                    }
                )
                .request_validation_layers()
                .build();
        if (!instance) {
            Log::Core::Critical("Failed to create Vulkan instance. Error: {}\n", instance.error().message());
            std::exit(1);
        }
        m_Instance = instance.value();

        VkSurfaceKHR surface;
        VkCheck(glfwCreateWindowSurface(m_Instance, m_Window.GetGLFWWindow(), nullptr, &surface),
                "glfwCreateWindowSurface");
        m_Surface = vk::SurfaceKHR(surface);

        vkb::PhysicalDeviceSelector selector{ m_Instance };
        auto phys = selector
                .set_surface(m_Surface)
                .set_minimum_version(1, 1)
                .require_dedicated_transfer_queue()
                .select();
        if (!phys) {
            Log::Core::Critical("Failed to select Vulkan Physical Device. Error: {}\n", phys.error().message());
            std::exit(1);
        }
        m_PhysicalDevice = vk::PhysicalDevice(phys.value());

        vkb::DeviceBuilder device_builder{ phys.value() };
        auto dev = device_builder.build();
        if (!dev) {
            Log::Core::Critical("Failed to select Vulkan Device. Error: {}\n", dev.error().message());
            std::exit(1);
        }
        m_Device = dev.value();
        m_Device2 = vk::Device(m_Device);
    }

    void VulkanRenderer::InitQueues() {
        auto gq = m_Device.get_queue(vkb::QueueType::graphics);
        auto gqi = m_Device.get_queue_index(vkb::QueueType::graphics);
        if (!gq.has_value() || !gqi.has_value()) {
            Log::Core::Error("failed to get graphics queue: {}", gq.error().message());
            exit(1);
        }
        m_GraphicsQueue = gq.value();
        m_GraphicsQueueFamilyIndex = gqi.value();

        auto pq = m_Device.get_queue(vkb::QueueType::present);
        auto pqi = m_Device.get_queue_index(vkb::QueueType::present);
        if (!pq.has_value() || !pqi.has_value()) {
            Log::Core::Error("failed to get present queue: {}", pq.error().message());
            exit(1);
        }
        m_PresentQueue = pq.value();
        m_PresentQueueFamilyIndex = pqi.value();

        auto cq = m_Device.get_queue(vkb::QueueType::compute);
        auto cqi = m_Device.get_queue_index(vkb::QueueType::compute);
        if (!cq.has_value() || !cqi.has_value()) {
            Log::Core::Error("failed to get compute queue: {}", cq.error().message());
            exit(1);
        }
        m_ComputeQueue = cq.value();
        m_ComputeQueueFamilyIndex = cqi.value();

        auto tq = m_Device.get_queue(vkb::QueueType::transfer);
        auto tqi = m_Device.get_queue_index(vkb::QueueType::transfer);
        if (!tq.has_value() || !tqi.has_value()) {
            Log::Core::Error("failed to get transfer queue: {}", tq.error().message());
            exit(1);
        }
        m_TransferQueue = tq.value();
        m_TransferQueueFamilyIndex = tqi.value();
    }

    void VulkanRenderer::InitSwapchain() {
        // get the supported VkFormats
        std::vector<vk::SurfaceFormatKHR> formats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);
        assert(!formats.empty());
        m_SwapchainFormat = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0]
                .format;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
        if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            // If the surface size is undefined, the size is set to the size of the images requested.
            m_SwapchainExtent.width = glm::clamp(m_Window.GetWidth(), surfaceCapabilities.minImageExtent.width,
                                                 surfaceCapabilities.maxImageExtent.width);
            m_SwapchainExtent.height = glm::clamp(m_Window.GetHeight(), surfaceCapabilities.minImageExtent.height,
                                                  surfaceCapabilities.maxImageExtent.height);
        } else {
            // If the surface size is defined, the swap chain size must match
            m_SwapchainExtent = surfaceCapabilities.currentExtent;
        }

        // The FIFO present mode is guaranteed by the spec to be supported
        vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

        vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms &
                                                        vk::SurfaceTransformFlagBitsKHR::eIdentity)
                                                       ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                                                       : surfaceCapabilities.currentTransform;

        vk::CompositeAlphaFlagBitsKHR compositeAlpha =
                (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
                ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
                : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
                  ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
                  : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
                    ? vk::CompositeAlphaFlagBitsKHR::eInherit
                    : vk::CompositeAlphaFlagBitsKHR::eOpaque;

        vk::SwapchainCreateInfoKHR swapChainCreateInfo(vk::SwapchainCreateFlagsKHR(),
                                                       m_Surface,
                                                       surfaceCapabilities.minImageCount,
                                                       m_SwapchainFormat,
                                                       vk::ColorSpaceKHR::eSrgbNonlinear,
                                                       m_SwapchainExtent,
                                                       1,
                                                       vk::ImageUsageFlagBits::eColorAttachment,
                                                       vk::SharingMode::eExclusive,
                                                       {},
                                                       preTransform,
                                                       compositeAlpha,
                                                       swapchainPresentMode,
                                                       true,
                                                       nullptr);

        std::vector<uint32_t> queueFamilyIndices = { m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex };

        if (std::any_of(queueFamilyIndices.begin(), queueFamilyIndices.end(), [&](auto& index) {
            return index != queueFamilyIndices[0];
        })) {
            // If the queues are from different queue families, we either have to explicitly transfer
            // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
            // VK_SHARING_MODE_CONCURRENT
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapChainCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }

        m_Swapchain = m_Device2.createSwapchainKHR(swapChainCreateInfo);

        m_SwapchainImages = m_Device2.getSwapchainImagesKHR(m_Swapchain);

        m_SwapchainImageViews.reserve(m_SwapchainImages.size());
        vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, m_SwapchainFormat, {},
                                                    { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        for (auto image: m_SwapchainImages) {
            imageViewCreateInfo.image = image;
            m_SwapchainImageViews.push_back(m_Device2.createImageView(imageViewCreateInfo));
        }
    }

    void VulkanRenderer::InitDepthBuffer() {
        vk::FormatProperties formatProperties = m_PhysicalDevice.getFormatProperties(m_DepthFormat);

        vk::ImageTiling tiling;
        if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            tiling = vk::ImageTiling::eLinear;
        } else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            tiling = vk::ImageTiling::eOptimal;
        } else {
            Log::Core::Critical("DepthStencilAttachment is not supported for D16Unorm depth format.");
            std::exit(1);
        }
        vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(),
                                            vk::ImageType::e2D,
                                            m_DepthFormat,
                                            vk::Extent3D(m_Window.GetWidth(), m_Window.GetHeight(), 1),
                                            1,
                                            1,
                                            vk::SampleCountFlagBits::e1,
                                            tiling,
                                            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        m_DepthImage = m_Device2.createImage(imageCreateInfo);

        vk::PhysicalDeviceMemoryProperties memoryProperties = m_PhysicalDevice.getMemoryProperties();
        vk::MemoryRequirements memoryRequirements = m_Device2.getImageMemoryRequirements(m_DepthImage);
        uint32_t typeBits = memoryRequirements.memoryTypeBits;
        auto typeIndex = uint32_t(~0);
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeBits & 1) &&
                ((memoryProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) ==
                 vk::MemoryPropertyFlagBits::eDeviceLocal)) {
                typeIndex = i;
                break;
            }
            typeBits >>= 1;
        }
        assert(typeIndex != uint32_t(~0));
        m_DepthMemory = m_Device2.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));

        m_Device2.bindImageMemory(m_DepthImage, m_DepthMemory, 0);

        m_DepthImageView = m_Device2.createImageView(vk::ImageViewCreateInfo(
                vk::ImageViewCreateFlags(), m_DepthImage, vk::ImageViewType::e2D, m_DepthFormat, {},
                { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }));
    }

    void VulkanRenderer::InitRenderPass() {
        std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
        attachmentDescriptions[0] = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
                                                              m_SwapchainFormat,
                                                              vk::SampleCountFlagBits::e1,
                                                              vk::AttachmentLoadOp::eClear,
                                                              vk::AttachmentStoreOp::eStore,
                                                              vk::AttachmentLoadOp::eDontCare,
                                                              vk::AttachmentStoreOp::eDontCare,
                                                              vk::ImageLayout::eUndefined,
                                                              vk::ImageLayout::ePresentSrcKHR);
        attachmentDescriptions[1] = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
                                                              m_DepthFormat,
                                                              vk::SampleCountFlagBits::e1,
                                                              vk::AttachmentLoadOp::eClear,
                                                              vk::AttachmentStoreOp::eDontCare,
                                                              vk::AttachmentLoadOp::eDontCare,
                                                              vk::AttachmentStoreOp::eDontCare,
                                                              vk::ImageLayout::eUndefined,
                                                              vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                       colorReference, {}, &depthReference);

        m_MainRenderPass = m_Device2.createRenderPass(
                vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpass));


    }

    void VulkanRenderer::InitFramebuffers() {
        m_Framebuffers.reserve(m_SwapchainImageViews.size());

        std::array<vk::ImageView, 2> attachments;
        attachments[1] = m_DepthImageView;

        auto framebuffer_info = vk::FramebufferCreateInfo(vk::FramebufferCreateFlags(), m_MainRenderPass, attachments,
                                                          m_SwapchainExtent.width, m_SwapchainExtent.height, 1);

        for (auto& m_SwapchainImageView: m_SwapchainImageViews) {
            attachments[0] = m_SwapchainImageView;
            m_Framebuffers.push_back(m_Device2.createFramebuffer(framebuffer_info));
        }
    }

    void VulkanRenderer::Cleanup() {
        for (auto const& framebuffer: m_Framebuffers) {
            m_Device2.destroyFramebuffer(framebuffer);
        }

        m_Device2.destroyRenderPass(m_MainRenderPass);

        m_Device2.destroyImageView(m_DepthImageView);
        m_Device2.freeMemory(m_DepthMemory);
        m_Device2.destroyImage(m_DepthImage);

        for (auto& imageView: m_SwapchainImageViews) {
            m_Device2.destroyImageView(imageView);
        }
        m_Device2.destroySwapchainKHR(m_Swapchain);

        vkb::destroy_device(m_Device);
        vkb::destroy_surface(m_Instance, m_Surface);
        vkb::destroy_instance(m_Instance);
    }

    VulkanRenderer::~VulkanRenderer() = default;
}