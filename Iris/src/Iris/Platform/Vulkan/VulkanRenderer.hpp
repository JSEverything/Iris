#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"
#include "Iris/Platform/Vulkan/VulkanBuffer.hpp"

namespace Iris {
    class VulkanRenderer : public Renderer {
    public:
        explicit VulkanRenderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
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
        void InitVertexBuffer();
        void InitPipelines();
        void InitDescriptorSet();
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

        vk::DescriptorSetLayout m_DescriptorSetLayout;
        vk::PipelineLayout m_PipelineLayout;
        vk::Pipeline m_Pipeline;

        std::shared_ptr<VulkanBuffer> m_MVPCBuffer;

        vk::DescriptorPool m_DescriptorPool;
        vk::DescriptorSet m_DescriptorSet;

        std::shared_ptr<VulkanBuffer> m_VertexBuffer;
    };
}

