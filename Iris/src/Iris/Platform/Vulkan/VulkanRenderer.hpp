#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"
#include "Iris/Platform/Vulkan/VulkanBuffer.hpp"

namespace Iris {
    struct QueueFamilies
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily;

        std::shared_ptr<vk::raii::Queue> graphicsQueue;
        std::shared_ptr<vk::raii::Queue> presentQueue;
        std::shared_ptr<vk::raii::Queue> computeQueue;
        std::shared_ptr<vk::raii::Queue> transferQueue;

        [[nodiscard]] bool isComplete() const
        {
            return graphicsFamily.has_value()
                && presentFamily.has_value()
                && computeFamily.has_value()
                && transferFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class VulkanRenderer : public Renderer {
    public:
        explicit VulkanRenderer(const WindowOptions& opts);
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

        QueueFamilies findQueueFamilies(vk::raii::PhysicalDevice& device);
        bool checkDeviceExtensionSupport(vk::raii::PhysicalDevice& device);
        bool checkDevice(vk::raii::PhysicalDevice& device);
        SwapChainSupportDetails querySwapChainSupport(vk::raii::PhysicalDevice& device);
    private:
        std::vector<const char*> m_DeviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        std::vector<const char*> m_InstanceExtensions = {
                VK_KHR_SURFACE_EXTENSION_NAME
        };
        std::vector<const char*> m_EnabledLayers;

        vk::raii::Context m_Context;
        std::shared_ptr<vk::raii::DebugUtilsMessengerEXT> m_DebugMessenger;
        std::shared_ptr<vk::raii::Instance> m_Instance;
        std::shared_ptr<vk::raii::SurfaceKHR> m_Surface;
        std::shared_ptr<vk::raii::PhysicalDevice> m_PhysicalDevice;
        std::shared_ptr<vk::raii::Device> m_Device;

        std::shared_ptr<vk::raii::SwapchainKHR> m_Swapchain;

        std::shared_ptr<vk::raii::Image> m_DepthImage;
        std::shared_ptr<vk::raii::ImageView> m_DepthImageView;

        std::shared_ptr<vk::raii::RenderPass> m_MainRenderPass;

        std::shared_ptr<vk::raii::CommandPool> m_CommandPool;
        std::shared_ptr<vk::raii::CommandBuffer> m_CommandBuffer;

        std::shared_ptr<vk::raii::Fence> m_RenderFence;
        std::shared_ptr<vk::raii::Semaphore> m_RenderSemaphore;
        std::shared_ptr<vk::raii::Semaphore> m_PresentSemaphore;

        std::shared_ptr<vk::raii::DescriptorSetLayout> m_DescriptorSetLayout;
        std::shared_ptr<vk::raii::PipelineLayout> m_PipelineLayout;
        std::shared_ptr<vk::raii::Pipeline> m_Pipeline;

        std::shared_ptr<vk::raii::DescriptorPool> m_DescriptorPool;
        std::shared_ptr<vk::raii::DescriptorSet> m_DescriptorSet;

        QueueFamilies m_Queues;
        vk::Extent2D m_SurfaceExtent;
        vk::Format m_SwapchainImageFormat = vk::Format::eB8G8R8A8Unorm;
        vk::Format m_DepthFormat = vk::Format::eD16Unorm;

        std::vector<VkImage> m_SwapchainImages;
        std::vector<vk::raii::ImageView> m_SwapchainImageViews;

        std::vector<vk::raii::Framebuffer> m_Framebuffers;

        std::shared_ptr<VulkanBuffer> m_MVPCBuffer;
        std::shared_ptr<VulkanBuffer> m_VertexBuffer;
    };
}

