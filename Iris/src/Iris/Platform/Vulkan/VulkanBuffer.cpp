#include "VulkanBuffer.hpp"

namespace Iris {
    VulkanBuffer::VulkanBuffer(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice,
                               vk::BufferUsageFlags flags, size_t size, void* data)
            : m_Size(size), m_Buffer(std::move(device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, flags)))) {
        vk::MemoryRequirements memoryRequirements = m_Buffer.getMemoryRequirements();

        uint32_t typeIndex = findMemoryType(physicalDevice.getMemoryProperties(),
                                            memoryRequirements.memoryTypeBits,
                                            vk::MemoryPropertyFlagBits::eHostVisible |
                                            vk::MemoryPropertyFlagBits::eHostCoherent);
        vk::raii::DeviceMemory memory = device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));

        auto* pData = static_cast<uint8_t*>(memory.mapMemory(0, memoryRequirements.size));
        memcpy(pData, data, size);
        memory.unmapMemory();

        m_Buffer.bindMemory(*memory, 0);
    }

    vk::DescriptorBufferInfo VulkanBuffer::GetDescriptorBufferInfo() const {
        return { *m_Buffer, 0, m_Size };
    }
}