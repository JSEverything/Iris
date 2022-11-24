#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"

namespace Iris {
    class VulkanRenderer : public Renderer {
    public:
        explicit VulkanRenderer(const WindowOptions& opts);
        ~VulkanRenderer() override;

        void Init() override;
        void Cleanup() override;
    private:
        void InitDevice();
        void InitSwapchain();
        void InitQueues();
        void InitDepthBuffer();
        void InitRenderPass();
        void InitFramebuffers();
    private:
        vkb::Instance m_Instance;
        vk::SurfaceKHR m_Surface;
        vk::PhysicalDevice m_PhysicalDevice;
        vkb::Device m_Device;
        vk::Device m_Device2;

        vk::Format m_SwapchainFormat;
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
    };
}
