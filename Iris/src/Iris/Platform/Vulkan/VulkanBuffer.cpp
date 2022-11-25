#include "VulkanBuffer.hpp"

namespace Iris {
    VulkanBuffer::VulkanBuffer(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
                               vk::BufferUsageFlags flags, size_t size, void* data)
            : m_Device(device), m_Size(size) {
        m_Buffer = m_Device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), m_Size,
                                                              flags));

        vk::MemoryRequirements memoryRequirements = m_Device.getBufferMemoryRequirements(m_Buffer);
        uint32_t typeIndex = findMemoryType(physicalDevice.getMemoryProperties(),
                                            memoryRequirements.memoryTypeBits,
                                            vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent);
        m_Memory = m_Device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));

        auto* pData = static_cast<uint8_t*>(m_Device.mapMemory(m_Memory, 0, memoryRequirements.size));
        memcpy(pData, data, size);
        m_Device.unmapMemory(m_Memory);

        m_Device.bindBufferMemory(m_Buffer, m_Memory, 0);
    }

    vk::DescriptorBufferInfo VulkanBuffer::GetDescriptorBufferInfo() const {
        return { m_Buffer, 0, m_Size };
    }

    VulkanBuffer::~VulkanBuffer() {
        m_Device.freeMemory(m_Memory);
        m_Device.destroyBuffer(m_Buffer);
    }
}