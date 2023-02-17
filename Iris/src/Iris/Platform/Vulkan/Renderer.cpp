#include "Renderer.hpp"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include "Iris/Renderer/Vertex.hpp"
#include "Iris/Platform/Vulkan/Util.hpp"
#include "Iris/Platform/Vulkan/Texture.hpp"
#include "Iris/Platform/Vulkan/PushConstants.hpp"
#include "Iris/Util/Input.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Iris/Math/Math.hpp"

namespace Iris::Vulkan {
    Renderer::Renderer(const std::shared_ptr<Window>& window) : Iris::Renderer(window) {
        m_Ctx = std::make_shared<Context>(window);
        m_UploadContext = std::make_shared<UploadContext>(m_Ctx);

        InitSwapchain();
        InitDepthBuffer();
        InitIDBuffer();
        InitRenderPass();
        InitFramebuffers();
        InitCommandBuffers();
        InitSyncStructures();
        InitUniformBuffer();
        InitPipelines();
        InitImGui();

        Input::Get().on<Key>([&](int button, KeyMods mods) {
            if (ImGui::GetIO().WantCaptureMouse) return;
            if (Input::IsKeyPressed(GLFW_KEY_LEFT_ALT)) return;
            if (button == GLFW_MOUSE_BUTTON_1) {
                glm::uvec2 pos = Input::GetMousePos();

                m_IDTexture->CopyToStagingBuffer();
                auto data = m_IDTexture->MapStagingBuffer();
                uint32_t id = data[pos.y * m_Size.x + pos.x];
                selectedEntity = id;
                m_IDTexture->UnmapStagingBuffer();
            } else if (button == GLFW_KEY_Q)
                gizmoMode = -1;
            else if (button == GLFW_KEY_W)
                gizmoMode = 0;
            else if (button == GLFW_KEY_E)
                gizmoMode = 1;
            else if (button == GLFW_KEY_R)
                gizmoMode = 2;
        });

        m_Textures.emplace_back(m_Ctx, m_UploadContext, "../Assets/Icons/LightPoint.png", 4);
        m_Textures.emplace_back(m_Ctx, m_UploadContext, "../Assets/Icons/LightDirectional.png", 4);
        m_Textures.emplace_back(m_Ctx, m_UploadContext, "../Assets/Icons/LightSpot.png", 4);
        m_BillboardPipeline->UpdateImage(1, 0, m_Textures[m_Textures.size() - 3].GetDescriptor(), 0);
        m_BillboardPipeline->UpdateImage(1, 0, m_Textures[m_Textures.size() - 2].GetDescriptor(), 1);
        m_BillboardPipeline->UpdateImage(1, 0, m_Textures[m_Textures.size() - 1].GetDescriptor(), 2);
    }

    void Renderer::InitSwapchain() {
        // get the supported VkFormats
        std::vector<vk::SurfaceFormatKHR> formats = m_Ctx->GetPhysDevice().getSurfaceFormatsKHR(m_Ctx->GetSurface());
        assert(!formats.empty());
        m_SwapchainFormat = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0]
                .format;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_Ctx->GetPhysDevice()
                .getSurfaceCapabilitiesKHR(m_Ctx->GetSurface());
        if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            // If the surface size is undefined, the size is set to the size of the images requested.
            m_SwapchainExtent.width = glm::clamp(m_Size.x, surfaceCapabilities.minImageExtent.width,
                                                 surfaceCapabilities.maxImageExtent.width);
            m_SwapchainExtent.height = glm::clamp(m_Size.y, surfaceCapabilities.minImageExtent.height,
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
                                                       m_Ctx->GetSurface(),
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

        std::vector<uint32_t> queueFamilyIndices = { m_Ctx->GetGraphicsQueueFamilyIndex(),
                                                     m_Ctx->GetPresentQueueFamilyIndex() };

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

        m_Swapchain = m_Ctx->GetDevice().createSwapchainKHR(swapChainCreateInfo);

        m_SwapchainImages = m_Ctx->GetDevice().getSwapchainImagesKHR(m_Swapchain);

        m_SwapchainImageViews.reserve(m_SwapchainImages.size());
        vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, m_SwapchainFormat, {},
                                                    { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        for (auto image: m_SwapchainImages) {
            imageViewCreateInfo.image = image;
            m_SwapchainImageViews.push_back(m_Ctx->GetDevice().createImageView(imageViewCreateInfo));
        }
    }

    void Renderer::InitDepthBuffer() {
        vk::FormatProperties formatProperties = m_Ctx->GetPhysDevice().getFormatProperties(m_DepthFormat);

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
                                            vk::Extent3D(m_Size.x, m_Size.y, 1),
                                            1,
                                            1,
                                            vk::SampleCountFlagBits::e1,
                                            tiling,
                                            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        m_DepthImage = m_Ctx->GetDevice().createImage(imageCreateInfo);

        vk::PhysicalDeviceMemoryProperties memoryProperties = m_Ctx->GetPhysDevice().getMemoryProperties();
        vk::MemoryRequirements memoryRequirements = m_Ctx->GetDevice().getImageMemoryRequirements(m_DepthImage);
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
        m_DepthMemory = m_Ctx->GetDevice().allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));

        m_Ctx->GetDevice().bindImageMemory(m_DepthImage, m_DepthMemory, 0);

        m_DepthImageView = m_Ctx->GetDevice().createImageView(vk::ImageViewCreateInfo(
                vk::ImageViewCreateFlags(), m_DepthImage, vk::ImageViewType::e2D, m_DepthFormat, {},
                { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }));
    }

    void Renderer::InitIDBuffer() {
        m_IDTexture = std::make_shared<Texture<uint32_t>>(m_Ctx, m_UploadContext, m_Size,
                                                          vk::Format::eR32Uint,
                                                          vk::ImageUsageFlagBits::eColorAttachment |
                                                          vk::ImageUsageFlagBits::eTransferSrc);
    }

    void Renderer::InitRenderPass() {
        std::array<vk::AttachmentDescription, 3> attachmentDescriptions;
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

        attachmentDescriptions[2] = vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
                                                              vk::Format::eR32Uint,
                                                              vk::SampleCountFlagBits::e1,
                                                              vk::AttachmentLoadOp::eClear,
                                                              vk::AttachmentStoreOp::eStore,
                                                              vk::AttachmentLoadOp::eDontCare,
                                                              vk::AttachmentStoreOp::eDontCare,
                                                              vk::ImageLayout::eUndefined,
                                                              vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        vk::AttachmentReference idReference(2, vk::ImageLayout::eColorAttachmentOptimal);

        std::array<vk::AttachmentReference, 2> colorAttachments = { colorReference, idReference };

        vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                       colorAttachments, {}, &depthReference);

        m_MainRenderPass = m_Ctx->GetDevice().createRenderPass(
                vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpass));
    }

    void Renderer::InitFramebuffers() {
        m_Framebuffers.reserve(m_SwapchainImageViews.size());

        std::array<vk::ImageView, 3> attachments;
        attachments[1] = m_DepthImageView;
        attachments[2] = m_IDTexture->GetDescriptor().imageView;

        auto framebuffer_info = vk::FramebufferCreateInfo(vk::FramebufferCreateFlags(), m_MainRenderPass, attachments,
                                                          m_SwapchainExtent.width, m_SwapchainExtent.height, 1);

        for (auto& m_SwapchainImageView: m_SwapchainImageViews) {
            attachments[0] = m_SwapchainImageView;
            m_Framebuffers.push_back(m_Ctx->GetDevice().createFramebuffer(framebuffer_info));
        }
    }

    void Renderer::InitCommandBuffers() {
        m_CommandPool = m_Ctx->GetDevice()
                .createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                             m_Ctx->GetGraphicsQueueFamilyIndex()));
        m_CommandBuffer = m_Ctx->GetDevice().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
    }

    void Renderer::InitSyncStructures() {
        m_RenderFence = m_Ctx->GetDevice().createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        m_PresentSemaphore = m_Ctx->GetDevice().createSemaphore(vk::SemaphoreCreateInfo());
        m_RenderSemaphore = m_Ctx->GetDevice().createSemaphore(vk::SemaphoreCreateInfo());
    }

    void Renderer::InitUniformBuffer() {
        m_CameraDataBuffer = std::make_unique<Buffer<CameraData>>(
                m_Ctx, vk::BufferUsageFlagBits::eUniformBuffer, CameraData{});
        m_LightDataBuffer = std::make_unique<Buffer<LightData>>(
                m_Ctx, vk::BufferUsageFlagBits::eUniformBuffer, LightData{});
        m_LightStorageBuffer = std::make_unique<Buffer<Light>>(
                m_Ctx, vk::BufferUsageFlagBits::eStorageBuffer, 500u);
    }

    void Renderer::InitPipelines() {
        m_PipelineBuilder = std::make_unique<PipelineBuilder>(m_Ctx->GetDevice());

        auto shaderStagesVF = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

        m_PipelineBuilder->SetVertexInputAttributes(
                        vk::VertexInputBindingDescription(0, sizeof(Vertex)),
                        parseVertexDescription(Vertex::GetDescription(), 0))
                .AddVertexShader("./Shaders/UberShader.vert.spv")
                .AddFragmentShader("./Shaders/UberShader.frag.spv")
                .AddPushConstant(shaderStagesVF, sizeof(PushConstants))
                .AddUniform(0, 0, shaderStagesVF)                            // camera data
                .AddUniform(0, 1, shaderStagesVF)                            // light meta
                .AddStorageBuffer(0, 2, shaderStagesVF)                            // light array
                .AddImage(1, 0, vk::ShaderStageFlagBits::eFragment, 1024);   // textures
        m_Pipeline = m_PipelineBuilder->Build(m_MainRenderPass);

        m_PipelineBuilder->Clear()
                .AddVertexShader("./Shaders/Billboard.vert.spv")
                .AddFragmentShader("./Shaders/Billboard.frag.spv")
                .AddPushConstant(shaderStagesVF, sizeof(PushConstants))
                .AddUniform(0, 0, shaderStagesVF)                            // camera data
                .AddImage(1, 0, vk::ShaderStageFlagBits::eFragment, 1024);   // textures
        m_BillboardPipeline = m_PipelineBuilder->Build(m_MainRenderPass);

        m_Pipeline->UpdateBuffer(0, 0, m_CameraDataBuffer->GetDescriptorBufferInfo());
        m_Pipeline->UpdateBuffer(0, 1, m_LightDataBuffer->GetDescriptorBufferInfo());
        m_Pipeline->UpdateBuffer(0, 2, m_LightStorageBuffer->GetDescriptorBufferInfo());
        m_BillboardPipeline->UpdateBuffer(0, 0, m_CameraDataBuffer->GetDescriptorBufferInfo());
    }

    void Renderer::Render(const Camera& camera) {
        while (vk::Result::eTimeout == m_Ctx->GetDevice().waitForFences(m_RenderFence, VK_TRUE, 100000000));
        VkCheck(m_Ctx->GetDevice().resetFences(1, &m_RenderFence), "Reset Draw Fence");

        auto* cameraData = m_CameraDataBuffer->Map();
        cameraData->position = glm::vec4(camera.GetPosition(), 1.f);
        cameraData->view = camera.GetViewMatrix();
        cameraData->projection = camera.GetProjectionMatrix();
        cameraData->viewProjection = camera.GetProjectionMatrix() * camera.GetViewMatrix();
        m_CameraDataBuffer->Unmap();

        uint32_t count = glm::min(static_cast<uint32_t>(m_Lights.size()), 50u);

        auto* lightData = m_LightDataBuffer->Map();
        lightData->lightCount = count;
        m_LightDataBuffer->Unmap();

        auto* lights = m_LightStorageBuffer->Map();

        for (uint32_t i = 0; i < count; ++i) {
            auto& entity = m_Scene->GetEntity(m_Lights[i]);
            auto& light = entity.GetComponent<Iris::Light>();

            lights[i].position = glm::vec4(entity.GetTransform().GetTranslation(), 1.f);
            lights[i].rotation = glm::vec4(glm::radians(entity.GetTransform().GetRotation()), 1.f);
            lights[i].color = glm::vec4(light.color, 1.f);
            lights[i].flags.x = static_cast<uint32_t>(light.type);
        }

        m_LightStorageBuffer->Unmap();

        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        vk::Extent2D extent{ m_Window->GetWidth(), m_Window->GetHeight() };
        vk::ResultValue<uint32_t> currentBuffer = m_Ctx->GetDevice()
                .acquireNextImageKHR(m_Swapchain, 100000000, m_PresentSemaphore, nullptr);
        m_CommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        std::array<vk::ClearValue, 3> clearValues;
        clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{ 0.2f, 0.2f, 0.2f, 1.f });
        clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
        clearValues[2].color = vk::ClearColorValue(std::array<uint32_t, 4>{ 0, 0, 0, 0 });

        vk::RenderPassBeginInfo renderPassBeginInfo(
                m_MainRenderPass, m_Framebuffers[currentBuffer.value],
                vk::Rect2D(vk::Offset2D(0, 0), extent), clearValues);

        m_CommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        m_CommandBuffer.setViewport( // Viewport is flipped
                0, vk::Viewport(0.0f, static_cast<float>(extent.height),
                                static_cast<float>(extent.width),
                                -static_cast<float>(extent.height), 0.0f, 1.0f));
        m_CommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

        m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline->pipeline);
        m_CommandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, m_Pipeline->pipelineLayout, 0, m_Pipeline->descriptorSets, nullptr);

        for (auto& mesh: m_Meshes) {
            auto entityID = mesh.GetParentID();
            m_CommandBuffer.pushConstants<PushConstants>(
                    m_Pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0u, PushConstants{
                            .modelMat = m_Scene->GetEntity(entityID).GetComponent<Iris::Mesh>().GetModelMatrix(),
                            .objectID = static_cast<uint32_t>(entityID),
                            .textureID = static_cast<uint32_t>(entityID)
                    });
            mesh.Draw(m_CommandBuffer);
        }

        m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_BillboardPipeline->pipeline);
        m_CommandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, m_BillboardPipeline->pipelineLayout, 0,
                m_BillboardPipeline->descriptorSets, nullptr);

        for (auto& light: m_Lights) {
            auto& entity = m_Scene->GetEntity(light);

            m_CommandBuffer.pushConstants<PushConstants>(
                    m_Pipeline->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0u, PushConstants{
                            .modelMat = glm::translate(glm::mat4(1.f), entity.GetTransform().GetTranslation()),
                            .objectID = static_cast<uint32_t>(entity.GetId()),
                            .textureID = static_cast<uint32_t>(entity.GetComponent<Iris::Light>().type)
                    });
            m_CommandBuffer.draw(6, 1, 0, 0);
        }

        ImGui::Begin("Selection");
        ImGui::Text("Selected entity: %zu", selectedEntity);
        //ImGui::Separator();

        // selectedEntity is index + 1
        if (selectedEntity != 0 && selectedEntity <= m_Scene->GetObjects().size()) {
            auto& entity = m_Scene->GetEntity(selectedEntity - 1);
            entity.RenderUI();

            if (gizmoMode != -1) {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
                ImGuizmo::SetRect(0.f, 0.f,
                                  static_cast<float>(m_Size.x), static_cast<float>(m_Size.y));

                glm::mat4 view = camera.GetViewMatrix();
                glm::mat4 proj = camera.GetProjectionMatrix();
                auto& tc = entity.GetTransform();
                glm::mat4 transform = tc.GetMatrix();

                ImGuizmo::OPERATION op = gizmoMode == 0
                                         ? ImGuizmo::OPERATION::TRANSLATE : gizmoMode == 1
                                                                            ? ImGuizmo::OPERATION::ROTATE
                                                                            : ImGuizmo::OPERATION::SCALE;

                ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                                     op, ImGuizmo::LOCAL, glm::value_ptr(transform));
                if (ImGuizmo::IsUsing()) {
                    glm::vec3 translate, rotate, scale;
                    Math::DecomposeTransform(transform, translate, rotate, scale);
                    tc.SetTranslation(translate);
                    tc.Rotate(glm::degrees(rotate) - tc.GetRotation());
                    tc.SetScale(scale);
                }
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_CommandBuffer);

        m_CommandBuffer.endRenderPass();
        m_CommandBuffer.end();

        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        vk::SubmitInfo submitInfo(m_PresentSemaphore, waitDestinationStageMask, m_CommandBuffer, m_RenderSemaphore);

        m_Ctx->GetGraphicsQueue().submit(submitInfo, m_RenderFence);

        VkCheck(m_Ctx->GetGraphicsQueue()
                        .presentKHR(vk::PresentInfoKHR(m_RenderSemaphore, m_Swapchain, currentBuffer.value)),
                "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR");

        Iris::Renderer::Present();
    }

    Renderer::~Renderer() {
        m_Ctx->GetDevice().waitIdle();

        m_Meshes.clear();
        m_Textures.clear();

        ImGui_ImplVulkan_Shutdown();
        m_Ctx->GetDevice().destroyDescriptorPool(m_ImGuiPool);

        m_Ctx->GetDevice().destroySemaphore(m_PresentSemaphore);
        m_Ctx->GetDevice().destroySemaphore(m_RenderSemaphore);
        m_Ctx->GetDevice().destroyFence(m_RenderFence);

        m_CameraDataBuffer.reset();
        m_LightDataBuffer.reset();

        for (auto const& framebuffer: m_Framebuffers) {
            m_Ctx->GetDevice().destroyFramebuffer(framebuffer);
        }

        m_Ctx->GetDevice().destroyRenderPass(m_MainRenderPass);

        m_Ctx->GetDevice().destroyImageView(m_DepthImageView);
        m_Ctx->GetDevice().freeMemory(m_DepthMemory);
        m_Ctx->GetDevice().destroyImage(m_DepthImage);
        m_IDTexture.reset();

        for (auto& imageView: m_SwapchainImageViews) {
            m_Ctx->GetDevice().destroyImageView(imageView);
        }
        m_Ctx->GetDevice().destroySwapchainKHR(m_Swapchain);

        m_UploadContext.reset();
        m_Ctx->GetDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffer);
        m_Ctx->GetDevice().destroyCommandPool(m_CommandPool);

        m_Pipeline.reset();
        m_BillboardPipeline.reset();
        m_PipelineBuilder.reset();

        m_Ctx.reset();
    }

    void Renderer::SetScene(const std::shared_ptr<Scene>& scene) {
        Iris::Renderer::SetScene(scene);

        m_Scene->on<ObjectAdd>([this](uint32_t entity) {
            for (auto& mesh: m_Scene->GetEntity(entity).GetComponents<Iris::Mesh>()) {
                m_Meshes.emplace_back(m_Ctx, entity, mesh.GetVertices(), mesh.GetIndices());
            }

            if (!m_Scene->GetEntity(entity).GetComponents<Material>().empty()) {
                auto& material = m_Scene->GetEntity(entity).GetComponent<Material>();
                m_Textures.emplace_back(m_Ctx, m_UploadContext, material.getTexture(), 4);
                m_Pipeline->UpdateImage(1, 0, m_Textures[m_Textures.size() - 1].GetDescriptor(), entity);
            }

            for (auto& light: m_Scene->GetEntity(entity).GetComponents<Iris::Light>()) {
                m_Lights.emplace_back(light.GetParent().GetId());
            }
        });
    }

    void Renderer::InitImGui() {
        std::vector<vk::DescriptorPoolSize> poolSizes = {
                { vk::DescriptorType::eSampler,              1000 },
                { vk::DescriptorType::eCombinedImageSampler, 1000 },
                { vk::DescriptorType::eSampledImage,         1000 },
                { vk::DescriptorType::eStorageImage,         1000 },
                { vk::DescriptorType::eUniformTexelBuffer,   1000 },
                { vk::DescriptorType::eStorageTexelBuffer,   1000 },
                { vk::DescriptorType::eUniformBuffer,        1000 },
                { vk::DescriptorType::eStorageBuffer,        1000 },
                { vk::DescriptorType::eUniformBufferDynamic, 1000 },
                { vk::DescriptorType::eStorageBufferDynamic, 1000 },
                { vk::DescriptorType::eInputAttachment,      1000 },
        };

        m_ImGuiPool = m_Ctx->GetDevice().createDescriptorPool(
                vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1000, poolSizes));

        ImGui::CreateContext();
        ImGui_ImplVulkan_InitInfo t{};
        t.Instance = &*m_Ctx->GetInstance();
        t.DescriptorPool = &*m_ImGuiPool;
        t.Device = &*m_Ctx->GetDevice();
        t.PhysicalDevice = &*m_Ctx->GetPhysDevice();
        t.Queue = &*m_Ctx->GetGraphicsQueue();
        t.MinImageCount = 2;
        t.ImageCount = glm::max(2LLu, m_SwapchainImages.size());
        t.QueueFamily = m_Ctx->GetGraphicsQueueFamilyIndex();
        t.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&t, &*m_MainRenderPass);

        {
            m_CommandBuffer.begin(vk::CommandBufferBeginInfo(
                    vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));

            ImGui_ImplVulkan_CreateFontsTexture(&*m_CommandBuffer);

            m_CommandBuffer.end();

            vk::SubmitInfo submitInfo({}, {}, m_CommandBuffer);

            m_Ctx->GetGraphicsQueue().submit(submitInfo);

            m_Ctx->GetDevice().waitIdle();
            m_CommandBuffer.reset();
        }

        ImGui_ImplVulkan_DestroyFontUploadObjects();
        ImGui_ImplGlfw_InitForVulkan(m_Window->GetGLFWWindow(), true);
    }
}