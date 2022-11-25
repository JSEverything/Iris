#pragma once
#include <vulkan/vulkan.hpp>
#include "Iris/Platform/Vulkan/Util.hpp"

namespace Iris {
    class VulkanBuffer {
    public:
        VulkanBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::BufferUsageFlags flags,
                     size_t size, void* data);

        [[nodiscard]] vk::DescriptorBufferInfo GetDescriptorBufferInfo() const;

        ~VulkanBuffer();

    public:
        vk::Buffer m_Buffer;
    private:
        const vk::Device& m_Device;
        size_t m_Size;
        vk::DeviceMemory m_Memory;

    };
}
