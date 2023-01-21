#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"
#include "Iris/Platform/Vulkan/Buffer.hpp"
#include "Iris/Platform/Vulkan/PipelineBuilder.hpp"
#include "Iris/Platform/Vulkan/Mesh.hpp"
#include "Iris/Platform/Vulkan/UploadContext.hpp"
#include "Iris/Platform/Vulkan/Texture.hpp"

namespace Iris::Vulkan {
    class Renderer : public Iris::Renderer {
    public:
        explicit Renderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
    private:
        void Init() override;
        void Draw() override;
        void Cleanup() override;

        void InitDevice();
        void InitSwapchain();
        void InitQueues();
        void InitDepthBuffer();
        void InitRenderPass();
        void InitFramebuffers();
        void InitCommandBuffers();
        void InitSyncStructures();
        void InitUniformBuffer();
        void InitPipelines();

        void ProcessEntityQueue();
    private:
        vkb::Instance m_Instance;
        vk::SurfaceKHR m_Surface;
        vk::PhysicalDevice m_PhysicalDevice;
        vkb::Device m_Device;
        vk::Device m_Device2;

        vk::Format m_SwapchainFormat{};
        vk::SwapchainKHR m_Swapchain;
        vk::Extent2D m_SwapchainExtent;
        std::vector<vk::Image> m_SwapchainImages;
        std::vector<vk::ImageView> m_SwapchainImageViews;
        std::vector<vk::Framebuffer> m_Framebuffers;

        uint32_t m_GraphicsQueueFamilyIndex = 0;
        uint32_t m_ComputeQueueFamilyIndex = 0;
        uint32_t m_PresentQueueFamilyIndex = 0;
        uint32_t m_TransferQueueFamilyIndex = 0;
        vk::Queue m_GraphicsQueue;
        vk::Queue m_ComputeQueue;
        vk::Queue m_PresentQueue;
        vk::Queue m_TransferQueue;

        vk::Format m_DepthFormat = vk::Format::eD16Unorm;
        vk::DeviceMemory m_DepthMemory;
        vk::Image m_DepthImage;
        vk::ImageView m_DepthImageView;

        vk::RenderPass m_MainRenderPass;

        vk::CommandPool m_CommandPool;
        vk::CommandBuffer m_CommandBuffer;

        vk::Fence m_RenderFence;
        vk::Semaphore m_RenderSemaphore;
        vk::Semaphore m_PresentSemaphore;

        std::unique_ptr<Buffer<glm::mat4>> m_ViewProjectionBuffer;

        std::unique_ptr<PipelineBuilder> m_PipelineBuilder;
        std::unique_ptr<PipelineBuilder::Pipeline> m_Pipeline;

        std::vector<uint32_t> m_EntityQueue;
        std::mutex m_QueueMutex;

        std::vector<Mesh> m_Meshes;
        std::vector<Texture> m_Textures;

        std::shared_ptr<UploadContext> m_UploadContext;
    };
}

