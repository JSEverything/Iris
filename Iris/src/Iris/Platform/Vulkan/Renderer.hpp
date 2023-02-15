#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"
#include "Iris/Platform/Vulkan/Context.hpp"
#include "Iris/Platform/Vulkan/Buffer.hpp"
#include "Iris/Platform/Vulkan/PipelineBuilder.hpp"
#include "Iris/Platform/Vulkan/Mesh.hpp"
#include "Iris/Platform/Vulkan/UploadContext.hpp"
#include "Iris/Platform/Vulkan/Texture.hpp"

namespace Iris::Vulkan {
    class Renderer final : public Iris::Renderer {
    public:
        //explicit Renderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
        explicit Renderer(const std::shared_ptr<Window>& window);

        void Render(const Camera& camera) override;
        void SetScene(const std::shared_ptr<Scene>& scene) override;

        ~Renderer() override;
    private:

        void InitSwapchain();
        void InitDepthBuffer();
        void InitIDBuffer();
        void InitRenderPass();
        void InitFramebuffers();
        void InitCommandBuffers();
        void InitSyncStructures();
        void InitUniformBuffer();
        void InitPipelines();
        void InitImGui();
    private:
        std::shared_ptr<Context> m_Ctx{ nullptr };

        vk::Format m_SwapchainFormat{};
        vk::SwapchainKHR m_Swapchain;
        vk::Extent2D m_SwapchainExtent;
        std::vector<vk::Image> m_SwapchainImages;
        std::vector<vk::ImageView> m_SwapchainImageViews;
        std::vector<vk::Framebuffer> m_Framebuffers;

        vk::Format m_DepthFormat = vk::Format::eD16Unorm;
        vk::DeviceMemory m_DepthMemory;
        vk::Image m_DepthImage;
        vk::ImageView m_DepthImageView;
        std::shared_ptr<Texture<uint32_t>> m_IDTexture;

        vk::RenderPass m_MainRenderPass;

        vk::CommandPool m_CommandPool;
        vk::CommandBuffer m_CommandBuffer;

        vk::Fence m_RenderFence;
        vk::Semaphore m_RenderSemaphore;
        vk::Semaphore m_PresentSemaphore;

        std::unique_ptr<Buffer<glm::mat4>> m_ViewProjectionBuffer;

        std::unique_ptr<PipelineBuilder> m_PipelineBuilder;
        std::unique_ptr<PipelineBuilder::Pipeline> m_Pipeline;

        std::vector<Mesh> m_Meshes;
        std::vector<Texture<float>> m_Textures;

        std::shared_ptr<UploadContext> m_UploadContext;

        vk::DescriptorPool m_ImGuiPool;
    };
}

