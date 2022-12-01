#include "VulkanRenderer.hpp"
#include "Iris/Renderer/Vertex.hpp"
#include "Iris/Platform/Vulkan/Util.hpp"
#include <fstream>

namespace Iris {
    VulkanRenderer::VulkanRenderer(const WindowOptions& opts)
            : Renderer(RenderAPI::Vulkan, opts) {
    }

    void VulkanRenderer::Init() {
        InitDevice();
        InitQueues();
        InitSwapchain();
        InitDepthBuffer();
        InitRenderPass();
        InitFramebuffers();
        InitCommandBuffers();
        InitSyncStructures();
        InitUniformBuffer();
        InitVertexBuffer();
        InitPipelines();
        InitDescriptorSet();
    }

    void VulkanRenderer::InitDevice() {
        vk::ApplicationInfo appInfo(m_Window->GetTitle().data(), 1, "Iris", 1, VK_API_VERSION_1_1);

        m_EnabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        m_InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef IRIS_PLATFORM_WINDOWS
        //m_InstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
        //m_InstanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

        m_Instance = std::make_shared<vk::raii::Instance>(m_Context, vk::InstanceCreateInfo(vk::InstanceCreateFlags(),
                &appInfo, m_EnabledLayers, m_InstanceExtensions
                ));

        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt(
                vk::DebugUtilsMessengerCreateFlagsEXT(),
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32 {
                    std::string type = vk::to_string( static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType));
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
                });

        if (!m_Instance) {
            Log::Core::Critical("Failed to create Vulkan instance.");
            std::exit(1);
        }

        m_DebugMessenger = std::make_shared<vk::raii::DebugUtilsMessengerEXT>(*m_Instance, debugUtilsMessengerCreateInfoExt);

        VkSurfaceKHR surface;
        VkCheck(glfwCreateWindowSurface(**m_Instance, m_Window->GetGLFWWindow(), nullptr, &surface), "Failed to create window surface");
        m_Surface = std::make_shared<vk::raii::SurfaceKHR>(*m_Instance, surface);

        auto devices = vk::raii::PhysicalDevices(*m_Instance);
        Log::Core::Info("Devices: ");
        for (auto& device : devices) {
            Log::Core::Info("\t{0}", device.getProperties().deviceName);

            if (checkDevice(device) && !m_PhysicalDevice) {
                m_PhysicalDevice = std::make_shared<vk::raii::PhysicalDevice>(device);
            }
        }

        Log::Core::Info("Using: {0}", m_PhysicalDevice->getProperties().deviceName);
    }

    void VulkanRenderer::InitQueues() {
        m_Queues = findQueueFamilies(*m_PhysicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_Queues.graphicsFamily.value(), m_Queues.presentFamily.value(),
                                                   m_Queues.computeFamily.value(), m_Queues.transferFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamily, 1, &queuePriority);
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::DeviceCreateInfo createInfo(vk::DeviceCreateFlags(), queueCreateInfos, m_DeviceExtensions, m_EnabledLayers);

        m_Device = std::make_shared<vk::raii::Device>(*m_PhysicalDevice, createInfo);

        m_Queues.graphicsQueue = std::make_shared<vk::raii::Queue>(*m_Device, m_Queues.graphicsFamily.value(), 0);
        m_Queues.presentQueue = std::make_shared<vk::raii::Queue>(*m_Device, m_Queues.presentFamily.value(), 0);
        m_Queues.computeQueue = std::make_shared<vk::raii::Queue>(*m_Device, m_Queues.computeFamily.value(), 0);
        m_Queues.transferQueue = std::make_shared<vk::raii::Queue>(*m_Device, m_Queues.transferFamily.value(), 0);
    }

    void VulkanRenderer::InitSwapchain() {

        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*m_PhysicalDevice);

        if (swapChainSupport.formats.empty()){
            Log::Core::Critical("Empty surface formats");
            std::exit(1);
        }

        m_SwapchainImageFormat = (swapChainSupport.formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : swapChainSupport.formats[0].format;

        auto& capabilities = swapChainSupport.capabilities;

        if (swapChainSupport.capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
        {
            m_SurfaceExtent.width  = std::clamp(m_Window->GetWidth(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            m_SurfaceExtent.height = std::clamp(m_Window->GetHeight(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        } else {
            m_SurfaceExtent = capabilities.currentExtent;
        }

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainCreateFlagsKHR(),
                                              **m_Surface,
                                              imageCount,
                                              m_SwapchainImageFormat,
                                              vk::ColorSpaceKHR::eSrgbNonlinear,
                                              m_SurfaceExtent,
                                              1,
                                              vk::ImageUsageFlagBits::eColorAttachment,
                                              vk::SharingMode::eExclusive,
                                              {},
                                              capabilities.currentTransform,
                                              vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                              vk::PresentModeKHR::eFifo,
                                              true
        );

        if (m_Queues.graphicsFamily != m_Queues.presentFamily) {
            createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
            std::vector vIndices = { m_Queues.graphicsFamily.value(), m_Queues.presentFamily.value() };
            createInfo.setQueueFamilyIndices(vIndices);
        }

        m_Swapchain = std::make_shared<vk::raii::SwapchainKHR>(*m_Device, createInfo);
        m_SwapchainImages = m_Swapchain->getImages();

        m_SwapchainImageViews.reserve(m_SwapchainImages.size());

        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), {},
                                                    vk::ImageViewType::e2D,
                                                    m_SwapchainImageFormat,
                                                    {},
                                                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        for (auto image : m_SwapchainImages)
        {
            imageViewCreateInfo.image = static_cast<vk::Image>(image);
            m_SwapchainImageViews.emplace_back(*m_Device, imageViewCreateInfo);
        }
    }

    void VulkanRenderer::InitDepthBuffer() {
        vk::FormatProperties formatProperties = m_PhysicalDevice->getFormatProperties(m_DepthFormat);

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
        m_DepthImage = std::make_shared<vk::raii::Image>(*m_Device, imageCreateInfo);

        vk::PhysicalDeviceMemoryProperties memoryProperties = m_PhysicalDevice->getMemoryProperties();
        vk::MemoryRequirements memoryRequirements = m_DepthImage->getMemoryRequirements();
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
        vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, typeIndex);
        vk::raii::DeviceMemory depthMemory(*m_Device, memoryAllocateInfo);

        m_DepthImage->bindMemory(*depthMemory, 0);

        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(),
                                                        **m_DepthImage,
                                                        vk::ImageViewType::e2D,
                                                        m_DepthFormat,
                                                        {},
                                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

        m_DepthImageView = std::make_shared<vk::raii::ImageView>(*m_Device, imageViewCreateInfo);
    }

    void VulkanRenderer::InitRenderPass() {
        std::array<vk::AttachmentDescription, 2> attachmentDescriptions;
        attachmentDescriptions[0] = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
                                                              m_SwapchainImageFormat,
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

        m_MainRenderPass = std::make_shared<vk::raii::RenderPass>(*m_Device, vk::RenderPassCreateInfo(
                                                                          vk::RenderPassCreateFlags(),
                                                                          attachmentDescriptions,
                                                                          subpass));
    }

    void VulkanRenderer::InitFramebuffers() {
        m_Framebuffers.reserve(m_SwapchainImageViews.size());

        std::array<vk::ImageView, 2> attachments{};
        attachments[1] = **m_DepthImageView;

        auto framebuffer_info = vk::FramebufferCreateInfo(vk::FramebufferCreateFlags(), **m_MainRenderPass, attachments,
                                                          m_SurfaceExtent.width, m_SurfaceExtent.height, 1);

        for (auto& m_SwapchainImageView: m_SwapchainImageViews) {
            attachments[0] = *m_SwapchainImageView;
            m_Framebuffers.emplace_back(*m_Device, framebuffer_info);
        }
    }

    void VulkanRenderer::InitCommandBuffers() {
        vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlags(), m_Queues.graphicsFamily.value());

        m_CommandPool = std::make_shared<vk::raii::CommandPool>(*m_Device, commandPoolCreateInfo);

        vk::CommandBufferAllocateInfo commandBufferAllocateInfo(**m_CommandPool, vk::CommandBufferLevel::ePrimary, 1);

        m_CommandBuffer = std::make_shared<vk::raii::CommandBuffer>(
                std::move(vk::raii::CommandBuffers(*m_Device, commandBufferAllocateInfo).front()));
    }

    void VulkanRenderer::InitSyncStructures() {
        m_RenderFence = std::make_shared<vk::raii::Fence>(*m_Device, vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_PresentSemaphore = std::make_shared<vk::raii::Semaphore>(*m_Device, vk::SemaphoreCreateInfo());
        m_RenderSemaphore = std::make_shared<vk::raii::Semaphore>(*m_Device, vk::SemaphoreCreateInfo());
    }

    void VulkanRenderer::InitUniformBuffer() {
        glm::mat4x4 model = glm::mat4x4(1.0f);
        glm::mat4x4 view = glm::lookAt(glm::vec3(-5.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                       glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4x4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
        // clang-format off
        glm::mat4x4 clip = glm::mat4x4(1.0f, 0.0f, 0.0f, 0.0f,
                                       0.0f, -1.0f, 0.0f, 0.0f,
                                       0.0f, 0.0f, 0.5f, 0.0f,
                                       0.0f, 0.0f, 0.5f, 1.0f);  // vulkan clip space has inverted y and half z !
        // clang-format on
        glm::mat4x4 mvpc = clip * projection * view * model;

        m_MVPCBuffer = std::make_shared<VulkanBuffer>(*m_Device, *m_PhysicalDevice,
                                                      vk::BufferUsageFlagBits::eUniformBuffer, sizeof(mvpc), &mvpc);
    }

    void VulkanRenderer::InitVertexBuffer() {
        std::vector<glm::vec4> vertices = { { 1.f,  1.f,  0.f, 1.f },
                                            { -1.f, 1.f,  0.f, 1.f },
                                            { 0.f,  -1.f, 0.f, 1.f } };
        m_VertexBuffer = std::make_shared<VulkanBuffer>(*m_Device, *m_PhysicalDevice,
                                                        vk::BufferUsageFlagBits::eVertexBuffer,
                                                        vertices.size() * sizeof(glm::vec4), vertices.data());
    }

    void VulkanRenderer::InitPipelines() {
        vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding{};
        descriptorSetLayoutBinding.binding = 0;
        descriptorSetLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = 1;
        descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

        m_DescriptorSetLayout = std::make_shared<vk::raii::DescriptorSetLayout>(*m_Device, descriptorSetLayoutCreateInfo);

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &**m_DescriptorSetLayout;

        m_PipelineLayout = std::make_shared<vk::raii::PipelineLayout>(*m_Device, pipelineLayoutCreateInfo);

        auto loadShaderModule = [&](std::string_view filePath) {
            std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                Log::Core::Critical("Shader not found: ", filePath);
                std::exit(1);
            }

            size_t fileSize = (size_t)file.tellg();
            std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

            file.seekg(0);
            file.read((char*)buffer.data(), (std::streamsize)fileSize);
            file.close();

            return m_Device->createShaderModule(vk::ShaderModuleCreateInfo({}, buffer.size() * sizeof(uint32_t), buffer.data()));
        };

        vk::raii::ShaderModule triVert = loadShaderModule("./Shaders/simple.vert.spv");
        vk::raii::ShaderModule triFrag = loadShaderModule("./Shaders/simple.frag.spv");

        std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
                vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
                                                  vk::ShaderStageFlagBits::eVertex, *triVert, "main"),
                vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
                                                  vk::ShaderStageFlagBits::eFragment, *triFrag, "main")
        };

        vk::VertexInputBindingDescription vertexInputBindingDescription(0, sizeof(glm::vec4));
        std::array<vk::VertexInputAttributeDescription, 1> vertexInputAttributeDescriptions = {
                vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32A32Sfloat, 0),
                //vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, 16),
                //vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32A32Sfloat, 32)
        };
        vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
                vk::PipelineVertexInputStateCreateFlags(),  // flags
                vertexInputBindingDescription,              // vertexBindingDescriptions
                vertexInputAttributeDescriptions            // vertexAttributeDescriptions
        );

        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
                vk::PipelineInputAssemblyStateCreateFlags(),
                vk::PrimitiveTopology::eTriangleList);

        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1,
                                                                            nullptr, 1, nullptr);

        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
                vk::PipelineRasterizationStateCreateFlags(),  // flags
                false,                                        // depthClampEnable
                false,                                        // rasterizerDiscardEnable
                vk::PolygonMode::eFill,                       // polygonMode
                vk::CullModeFlagBits::eBack,                  // cullMode
                vk::FrontFace::eClockwise,                    // frontFace
                false,                                        // depthBiasEnable
                0.0f,                                         // depthBiasConstantFactor
                0.0f,                                         // depthBiasClamp
                0.0f,                                         // depthBiasSlopeFactor
                1.0f                                          // lineWidth
        );

        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
                vk::PipelineMultisampleStateCreateFlags(),  // flags
                vk::SampleCountFlagBits::e1                 // rasterizationSamples
                // other values can be default
        );

        vk::StencilOpState stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep,
                                          vk::CompareOp::eAlways);
        vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
                vk::PipelineDepthStencilStateCreateFlags(),  // flags
                true,                                        // depthTestEnable
                true,                                        // depthWriteEnable
                vk::CompareOp::eLessOrEqual,                 // depthCompareOp
                false,                                       // depthBoundTestEnable
                false,                                       // stencilTestEnable
                stencilOpState,                              // front
                stencilOpState                               // back
        );

        vk::ColorComponentFlags colorComponentFlags(
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(false,                   // blendEnable
                                                                                vk::BlendFactor::eZero,  // srcColorBlendFactor
                                                                                vk::BlendFactor::eZero,  // dstColorBlendFactor
                                                                                vk::BlendOp::eAdd,       // colorBlendOp
                                                                                vk::BlendFactor::eZero,  // srcAlphaBlendFactor
                                                                                vk::BlendFactor::eZero,  // dstAlphaBlendFactor
                                                                                vk::BlendOp::eAdd,       // alphaBlendOp
                                                                                colorComponentFlags      // colorWriteMask
        );
        vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
                vk::PipelineColorBlendStateCreateFlags(),  // flags
                false,                                     // logicOpEnable
                vk::LogicOp::eNoOp,                        // logicOp
                pipelineColorBlendAttachmentState,         // attachments
                { { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
        );

        std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(),
                                                                          dynamicStates);

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo(vk::PipelineCreateFlags{});
        pipelineCreateInfo.setStages(pipelineShaderStageCreateInfos);
        pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
        pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
        pipelineCreateInfo.layout = **m_PipelineLayout;
        pipelineCreateInfo.renderPass = **m_MainRenderPass;

        m_Pipeline = std::make_shared<vk::raii::Pipeline>(*m_Device, nullptr, pipelineCreateInfo);
    }

    void VulkanRenderer::InitDescriptorSet() {
        vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, 1);
        m_DescriptorPool = std::make_shared<vk::raii::DescriptorPool>(*m_Device, vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSize));

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(**m_DescriptorPool, **m_DescriptorSetLayout);
        m_DescriptorSet = std::make_shared<vk::raii::DescriptorSet>(std::move(m_Device->allocateDescriptorSets(descriptorSetAllocateInfo).front()));

        vk::DescriptorBufferInfo descriptorBufferInfo = m_MVPCBuffer->GetDescriptorBufferInfo();
        vk::WriteDescriptorSet writeDescriptorSet(**m_DescriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer, {},
                                                  descriptorBufferInfo);
        m_Device->updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    void VulkanRenderer::Cleanup() {
        m_Device->waitIdle();
    }

    void VulkanRenderer::Draw() {
        while (vk::Result::eTimeout == m_Device->waitForFences(**m_RenderFence, VK_TRUE, 100000000));
        m_Device->resetFences(**m_RenderFence);

        const float pi3 = std::numbers::pi / 3;
        glm::vec3 color = abs(glm::sin(
                glm::vec3(static_cast<float>(m_FrameNr) / 10.f) / glm::vec3(120.f) + glm::vec3(0.f, pi3, 2 * pi3)));

        std::pair<vk::Result, uint32_t> pair = m_Swapchain->acquireNextImage(100000000, **m_PresentSemaphore);
        if (pair.first != vk::Result::eSuccess) {
            Log::Core::Critical("Failed to acquire next swapchain image");
            std::exit(1);
        }
        uint32_t currentBuffer = pair.second;

        m_CommandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{ color.r, color.g, color.b, 1.f });
        clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

        vk::RenderPassBeginInfo renderPassBeginInfo(
                **m_MainRenderPass, *m_Framebuffers[currentBuffer],
                vk::Rect2D(vk::Offset2D(0, 0), m_SurfaceExtent), clearValues);

        m_CommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        m_CommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, **m_Pipeline);
        m_CommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **m_PipelineLayout, 0, **m_DescriptorSet,
                                           nullptr);

        m_CommandBuffer->bindVertexBuffers(0, *m_VertexBuffer->m_Buffer, { 0 });
        m_CommandBuffer->setViewport(
                0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_SurfaceExtent.width),
                                static_cast<float>(m_SurfaceExtent.height), 0.0f, 1.0f));
        m_CommandBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_SurfaceExtent));

        m_CommandBuffer->draw(3, 1, 0, 0);
        m_CommandBuffer->endRenderPass();
        m_CommandBuffer->end();

        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        vk::SubmitInfo submitInfo(**m_PresentSemaphore, waitDestinationStageMask, **m_CommandBuffer, **m_RenderSemaphore);

        m_Queues.graphicsQueue->submit(submitInfo, **m_RenderFence);

        VkCheck(m_Queues.presentQueue->presentKHR(vk::PresentInfoKHR(**m_RenderSemaphore, **m_Swapchain, currentBuffer)),
                "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR");
    }

    QueueFamilies VulkanRenderer::findQueueFamilies(vk::raii::PhysicalDevice& device) {
        QueueFamilies indices;
        std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
            }

            if (device.getSurfaceSupportKHR(i, **m_Surface)) {
                indices.presentFamily = i;
            }

            if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                indices.computeFamily = i;
            }

            if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
                indices.transferFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }

    bool VulkanRenderer::checkDeviceExtensionSupport(vk::raii::PhysicalDevice& device) {
        std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails VulkanRenderer::querySwapChainSupport(vk::raii::PhysicalDevice& device) {
        SwapChainSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(**m_Surface);
        details.formats = device.getSurfaceFormatsKHR(**m_Surface);
        details.presentModes = device.getSurfacePresentModesKHR(**m_Surface);

        return details;
    }

    bool VulkanRenderer::checkDevice(vk::raii::PhysicalDevice& device) {
        QueueFamilies indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }
}