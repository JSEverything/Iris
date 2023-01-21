#include "Renderer.hpp"
#include "Iris/Renderer/Vertex.hpp"
#include "Iris/Platform/Vulkan/Util.hpp"
#include "Iris/Platform/Vulkan/Texture.hpp"

namespace Iris::Vulkan {
    Renderer::Renderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene)
            : Iris::Renderer(RenderAPI::Vulkan, opts, scene) {
        scene->on<ObjectAdd>([this](uint32_t entityId) {
            if (!m_Scene->GetEntity(entityId).GetComponents<Camera>().empty()) {
                m_CameraEntityId = entityId;
            }
            if (!m_Scene->GetEntity(entityId).GetComponents<Iris::Mesh>().empty()
                || !m_Scene->GetEntity(entityId).GetComponents<Iris::Material>().empty()) {
                std::scoped_lock l(m_QueueMutex);
                m_EntityQueue.emplace_back(entityId);
            }
        });
    }

    void Renderer::Init() {
        InitDevice();
        InitQueues();
        InitSwapchain();
        InitDepthBuffer();
        InitRenderPass();
        InitFramebuffers();
        InitCommandBuffers();
        InitSyncStructures();
        InitUniformBuffer();
        InitPipelines();
    }

    void Renderer::InitDevice() {
        vkb::InstanceBuilder instanceBuilder;

        auto instance = instanceBuilder
                .set_app_name(m_Window->GetTitle().data())
                .set_engine_name("Iris")
                .set_debug_callback([](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                       void* pUserData) -> VkBool32 {
                                        auto type = vkb::to_string_message_type(messageType);
                                        auto msg = std::string_view(pCallbackData->pMessage);

                                        switch (messageSeverity) {
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                                                Log::Core::Error("{}: {}", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                                                Log::Core::Warn("{}: {}", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                                                Log::Core::Info("{}: {}", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                                                Log::Core::Trace("{}: {}", type, msg);
                                                break;
                                            default:
                                                break;
                                        }
                                        return VK_FALSE;
                                    }
                )
                .request_validation_layers()
                .require_api_version(1, 3)
                .build();
        if (!instance) {
            Log::Core::Critical("Failed to create Vulkan instance. Error: {}\n", instance.error().message());
            std::exit(1);
        }
        m_Instance = instance.value();

        VkSurfaceKHR surface;
        VkCheck(glfwCreateWindowSurface(m_Instance, m_Window->GetGLFWWindow(), nullptr, &surface),
                "glfwCreateWindowSurface");
        m_Surface = vk::SurfaceKHR(surface);

        VkPhysicalDeviceVulkan12Features features12{};
        features12.runtimeDescriptorArray = true;
        features12.descriptorBindingPartiallyBound = true;

        features12.shaderUniformBufferArrayNonUniformIndexing = true;
        features12.shaderStorageBufferArrayNonUniformIndexing = true;
        features12.shaderSampledImageArrayNonUniformIndexing = true;
        features12.shaderStorageImageArrayNonUniformIndexing = true;

        //features12.descriptorBindingUniformBufferUpdateAfterBind = true;
        features12.descriptorBindingStorageBufferUpdateAfterBind = true;
        features12.descriptorBindingSampledImageUpdateAfterBind = true;
        features12.descriptorBindingStorageImageUpdateAfterBind = true;

        vkb::PhysicalDeviceSelector selector{ m_Instance };
        auto phys = selector
                .set_surface(m_Surface)
                .set_minimum_version(1, 3)
                .require_dedicated_transfer_queue()
                .set_required_features_12(features12)
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

    void Renderer::InitQueues() {
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

    void Renderer::InitSwapchain() {
        // get the supported VkFormats
        std::vector<vk::SurfaceFormatKHR> formats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);
        assert(!formats.empty());
        m_SwapchainFormat = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0]
                .format;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
        if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            // If the surface size is undefined, the size is set to the size of the images requested.
            m_SwapchainExtent.width = glm::clamp(m_Window->GetWidth(), surfaceCapabilities.minImageExtent.width,
                                                 surfaceCapabilities.maxImageExtent.width);
            m_SwapchainExtent.height = glm::clamp(m_Window->GetHeight(), surfaceCapabilities.minImageExtent.height,
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

    void Renderer::InitDepthBuffer() {
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
                                            vk::Extent3D(m_Window->GetWidth(), m_Window->GetHeight(), 1),
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

    void Renderer::InitRenderPass() {
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

    void Renderer::InitFramebuffers() {
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

    void Renderer::InitCommandBuffers() {
        m_CommandPool = m_Device2
                .createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                             m_GraphicsQueueFamilyIndex));
        m_CommandBuffer = m_Device2.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();

        m_UploadContext = std::make_shared<UploadContext>(m_Device2, m_TransferQueue, m_TransferQueueFamilyIndex);
    }

    void Renderer::InitSyncStructures() {
        m_RenderFence = m_Device2.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_PresentSemaphore = m_Device2.createSemaphore(vk::SemaphoreCreateInfo());
        m_RenderSemaphore = m_Device2.createSemaphore(vk::SemaphoreCreateInfo());
    }

    void Renderer::InitUniformBuffer() {
        m_ViewProjectionBuffer = std::make_unique<Buffer<glm::mat4>>(
                m_Device2, m_PhysicalDevice, vk::BufferUsageFlagBits::eUniformBuffer, glm::mat4(1.f));
    }

    void Renderer::InitPipelines() {
        m_PipelineBuilder = std::make_unique<PipelineBuilder>(m_Device2);

        m_PipelineBuilder->SetVertexInputAttributes(
                        vk::VertexInputBindingDescription(0, sizeof(Vertex)),
                        parseVertexDescription(Vertex::GetDescription(), 0))
                .AddVertexShader("./Shaders/PBR.vert.spv")
                .AddFragmentShader("./Shaders/PBR.frag.spv")
                .AddPushConstant(vk::ShaderStageFlagBits::eVertex, sizeof(glm::mat4))   // model matrix
                .AddPushConstant(vk::ShaderStageFlagBits::eFragment, sizeof(uint32_t))  // texture id
                .AddUniform(0, 0, vk::ShaderStageFlagBits::eVertex)         // view-projection
                .AddImage(1, 0, vk::ShaderStageFlagBits::eFragment, 1024)   // albedo
                .AddImage(1, 1, vk::ShaderStageFlagBits::eFragment, 1024)   // normal
                .AddImage(1, 2, vk::ShaderStageFlagBits::eFragment, 1024)   // metalic
                .AddImage(1, 3, vk::ShaderStageFlagBits::eFragment, 1024)   // height
                .AddImage(1, 4, vk::ShaderStageFlagBits::eFragment, 1024)   // roughness
                .AddImage(1, 5, vk::ShaderStageFlagBits::eFragment, 1024);  // ao

        m_Pipeline = m_PipelineBuilder->Build(m_MainRenderPass);

        m_Pipeline->UpdateBuffer(0, 0, m_ViewProjectionBuffer->GetDescriptorBufferInfo());
    }

    void Renderer::Cleanup() {
        m_Device2.waitIdle();

        m_Meshes.clear();
        m_Textures.clear();

        m_Device2.destroySemaphore(m_PresentSemaphore);
        m_Device2.destroySemaphore(m_RenderSemaphore);
        m_Device2.destroyFence(m_RenderFence);

        m_ViewProjectionBuffer.reset();

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

        m_UploadContext.reset();
        m_Device2.freeCommandBuffers(m_CommandPool, m_CommandBuffer);
        m_Device2.destroyCommandPool(m_CommandPool);

        m_Pipeline.reset();
        m_PipelineBuilder.reset();

        vkb::destroy_device(m_Device);
        vkb::destroy_surface(m_Instance, m_Surface);
        vkb::destroy_instance(m_Instance);
    }

    void Renderer::Draw() {
        if (!m_EntityQueue.empty()) {
            std::scoped_lock l(m_QueueMutex);
            ProcessEntityQueue();
        }

        vk::Extent2D extent{ m_Window->GetWidth(), m_Window->GetHeight() };
        while (vk::Result::eTimeout == m_Device2.waitForFences(m_RenderFence, VK_TRUE, 100000000));
        VkCheck(m_Device2.resetFences(1, &m_RenderFence), "Reset Draw Fence");

        if (m_CameraEntityId != -1) {
            auto camera = m_Scene->GetEntity(m_CameraEntityId).GetComponent<Camera>();
            auto* viewProjection = m_ViewProjectionBuffer->Map();
            *viewProjection = camera.GetProjectionMatrix() * camera.GetViewMatrix();
            m_ViewProjectionBuffer->Unmap();
        }

        vk::ResultValue<uint32_t> currentBuffer = m_Device2
                .acquireNextImageKHR(m_Swapchain, 100000000, m_PresentSemaphore, nullptr);
        m_CommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{ 0.2f, 0.2f, 0.2f, 1.f });
        clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

        vk::RenderPassBeginInfo renderPassBeginInfo(
                m_MainRenderPass, m_Framebuffers[currentBuffer.value],
                vk::Rect2D(vk::Offset2D(0, 0), extent), clearValues);

        m_CommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        m_CommandBuffer.setViewport( // Viewport is flipped
                0, vk::Viewport(0.0f, static_cast<float>(extent.height), static_cast<float>(extent.width),
                                -static_cast<float>(extent.height), 0.0f, 1.0f));
        m_CommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

        m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline->pipeline);
        m_CommandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, m_Pipeline->pipelineLayout, 0, m_Pipeline->descriptorSets, nullptr);

        for (auto& mesh: m_Meshes) {
            m_CommandBuffer.pushConstants<glm::mat4>(m_Pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0u,
                                                     m_Scene->GetEntity(mesh.GetParentID()).GetComponent<Iris::Mesh>()
                                                             .GetModelMatrix());
            m_CommandBuffer.pushConstants<uint32_t>(m_Pipeline->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 64u,
                                                    mesh.GetParentID());
            mesh.Draw(m_CommandBuffer);
        }

        m_CommandBuffer.endRenderPass();
        m_CommandBuffer.end();

        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        vk::SubmitInfo submitInfo(m_PresentSemaphore, waitDestinationStageMask, m_CommandBuffer, m_RenderSemaphore);

        m_GraphicsQueue.submit(submitInfo, m_RenderFence);

        VkCheck(m_GraphicsQueue.presentKHR(vk::PresentInfoKHR(m_RenderSemaphore, m_Swapchain, currentBuffer.value)),
                "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR");
    }

    void Renderer::ProcessEntityQueue() {
        for (uint32_t& entity: m_EntityQueue) {
            for (auto& mesh: m_Scene->GetEntity(entity).GetComponents<Iris::Mesh>()) {
                m_Meshes.emplace_back(m_Device2, m_PhysicalDevice,
                                      entity, mesh.GetVertices(), mesh.GetIndices());
            }
            if (!m_Scene->GetEntity(entity).GetComponents<Material>().empty()) {
                auto& material = m_Scene->GetEntity(entity).GetComponent<Material>();
                if (material.getTexture().ends_with("/")) {
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext,
                                            material.getTexture() + "albedo.png");
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext,
                                            material.getTexture() + "normal.png");
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext,
                                            material.getTexture() + "metallic.png", 1);
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext,
                                            material.getTexture() + "height.png", 1);
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext,
                                            material.getTexture() + "roughness.png", 1);
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext,
                                            material.getTexture() + "ao.png", 1);
                    for (uint32_t i = 0; i < 6; ++i) {
                        m_Pipeline->UpdateImage(1, i, m_Textures[m_Textures.size() - 6 + i].GetDescriptor(), entity);
                    }
                } else {
                    m_Textures.emplace_back(m_Device2, m_PhysicalDevice, m_UploadContext, material.getTexture());
                    m_Pipeline->UpdateImage(1, 0, m_Textures[m_Textures.size() - 1].GetDescriptor(), entity);
                }
            }
        }
        m_EntityQueue.clear();
    }
}