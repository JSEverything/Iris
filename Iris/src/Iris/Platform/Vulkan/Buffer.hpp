#pragma once
#include <vulkan/vulkan.hpp>
#include "Iris/Platform/Vulkan/Context.hpp"
#include "Iris/Platform/Vulkan/Util.hpp"

namespace Iris::Vulkan {
    template <typename T>
    class Buffer {
    public:
        Buffer(std::shared_ptr<Context> ctx, vk::BufferUsageFlags flags,
               const std::vector<T>& data) : Buffer(ctx, flags, data.data(), data.size()) {}

        Buffer(std::shared_ptr<Context> ctx, vk::BufferUsageFlags flags,
               const T& data) : Buffer(ctx, flags, &data) {}

        explicit Buffer(std::shared_ptr<Context> ctx, vk::BufferUsageFlags flags,
                        size_t count = 1) : m_Ctx(std::move(ctx)), m_Size(count * sizeof(T)) {
            m_Buffer = m_Ctx->GetDevice().createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), m_Size, flags));

            m_MemoryRequirements = m_Ctx->GetDevice().getBufferMemoryRequirements(m_Buffer);
            uint32_t typeIndex = findMemoryType(m_Ctx->GetPhysDevice().getMemoryProperties(),
                                                m_MemoryRequirements.memoryTypeBits,
                                                vk::MemoryPropertyFlagBits::eHostVisible |
                                                vk::MemoryPropertyFlagBits::eHostCoherent);
            m_Memory = m_Ctx->GetDevice().allocateMemory(vk::MemoryAllocateInfo(m_MemoryRequirements.size, typeIndex));

            m_Ctx->GetDevice().bindBufferMemory(m_Buffer, m_Memory, 0);
        }

        Buffer(std::shared_ptr<Context> ctx, vk::BufferUsageFlags flags,
               const T* data, size_t count = 1) : Buffer(ctx, flags, count) {
            auto* pData = Map();
            memcpy(pData, data, m_Size);
            Unmap();
        }

        [[nodiscard]] vk::DescriptorBufferInfo GetDescriptorBufferInfo() const {
            return { m_Buffer, 0, m_Size };
        }

        T* Map() {
            return static_cast<T*>(m_Ctx->GetDevice().mapMemory(m_Memory, 0, m_MemoryRequirements.size));
        }

        void Unmap() {
            m_Ctx->GetDevice().unmapMemory(m_Memory);
        }

        ~Buffer() {
            m_Ctx->GetDevice().freeMemory(m_Memory);
            m_Ctx->GetDevice().destroyBuffer(m_Buffer);
        }

    public:
        vk::Buffer m_Buffer;
    private:
        std::shared_ptr<Context> m_Ctx{};
        size_t m_Size{};
        vk::DeviceMemory m_Memory;
        vk::MemoryRequirements m_MemoryRequirements;
    };
}
