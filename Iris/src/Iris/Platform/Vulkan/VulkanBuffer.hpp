#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Iris/Platform/Vulkan/Util.hpp"

namespace Iris {
    class VulkanBuffer {
    public:
        VulkanBuffer(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, vk::BufferUsageFlags flags,
                     size_t size, void* data);

        [[nodiscard]] vk::DescriptorBufferInfo GetDescriptorBufferInfo() const;
    public:
        vk::raii::Buffer m_Buffer;
    private:
        size_t m_Size;
    };
}
