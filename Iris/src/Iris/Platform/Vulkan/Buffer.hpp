#pragma once
#include <vulkan/vulkan.hpp>
#include "Iris/Platform/Vulkan/Util.hpp"

namespace Iris::Vulkan {
    template <typename T>
    class Buffer {
    public:
        Buffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::BufferUsageFlags flags,
               const std::vector<T>& data) : Buffer(device, physicalDevice, flags, data.data(), data.size()) {}

        Buffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::BufferUsageFlags flags,
               const T& data) : Buffer(device, physicalDevice, flags, &data) {}

        Buffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice, vk::BufferUsageFlags flags,
               const T* data, size_t count = 1) : m_Device(device), m_Size(count * sizeof(T)) {
            m_Buffer = m_Device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), m_Size, flags));

            m_MemoryRequirements = m_Device.getBufferMemoryRequirements(m_Buffer);
            uint32_t typeIndex = findMemoryType(physicalDevice.getMemoryProperties(),
                                                m_MemoryRequirements.memoryTypeBits,
                                                vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent);
            m_Memory = m_Device.allocateMemory(vk::MemoryAllocateInfo(m_MemoryRequirements.size, typeIndex));

            auto* pData = Map();
            memcpy(pData, data, m_Size);
            Unmap();
            m_Device.bindBufferMemory(m_Buffer, m_Memory, 0);
        }

        [[nodiscard]] vk::DescriptorBufferInfo GetDescriptorBufferInfo() const {
            return { m_Buffer, 0, m_Size };
        }

        T* Map() {
            return static_cast<T*>(m_Device.mapMemory(m_Memory, 0, m_MemoryRequirements.size));
        }

        void Unmap() {
            m_Device.unmapMemory(m_Memory);
        }

        ~Buffer() {
            m_Device.freeMemory(m_Memory);
            m_Device.destroyBuffer(m_Buffer);
        }

    public:
        vk::Buffer m_Buffer;
    private:
        const vk::Device& m_Device;
        size_t m_Size{};
        vk::DeviceMemory m_Memory;
        vk::MemoryRequirements m_MemoryRequirements;
    };
}
