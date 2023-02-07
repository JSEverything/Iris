#pragma once
#include <VkBootstrap.h>
#include <vulkan/vulkan.hpp>
#include "Iris/Core/Window.hpp"

namespace Iris::Vulkan {
    class Context final {
    public:
        explicit Context(const std::shared_ptr<Window>& window);
        virtual ~Context();

        [[nodiscard]] vk::Device GetDevice() const { return m_Device; };

        [[nodiscard]] vk::PhysicalDevice GetPhysDevice() const { return m_PhysicalDevice; };

        [[nodiscard]] vk::SurfaceKHR GetSurface() const { return m_Surface; };

        [[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const;
        [[nodiscard]] uint32_t GetComputeQueueFamilyIndex() const;
        [[nodiscard]] uint32_t GetPresentQueueFamilyIndex() const;
        [[nodiscard]] uint32_t GetTransferQueueFamilyIndex() const;
        [[nodiscard]] const vk::Queue& GetGraphicsQueue() const;
        [[nodiscard]] const vk::Queue& GetComputeQueue() const;
        [[nodiscard]] const vk::Queue& GetPresentQueue() const;
        [[nodiscard]] const vk::Queue& GetTransferQueue() const;
    private:
        void CreateInstance();
        void CreateSurface(const std::shared_ptr<Window>& window);
        void SelectDevice();
        void CreateQueues();

    private:
        vkb::Instance m_VKBInstance;
        vkb::Device m_VKBDevice;

        vk::Instance m_Instance;
        vk::SurfaceKHR m_Surface;
        vk::PhysicalDevice m_PhysicalDevice;
        vk::Device m_Device;

        uint32_t m_GraphicsQueueFamilyIndex = 0;
        uint32_t m_ComputeQueueFamilyIndex = 0;
        uint32_t m_PresentQueueFamilyIndex = 0;
        uint32_t m_TransferQueueFamilyIndex = 0;
        vk::Queue m_GraphicsQueue;
        vk::Queue m_ComputeQueue;
        vk::Queue m_PresentQueue;
        vk::Queue m_TransferQueue;
    };
}
