#pragma once
#include "Iris/Renderer/Vertex.hpp"
#include "Iris/Platform/Vulkan/Buffer.hpp"

namespace Iris::Vulkan {
    class Mesh final {
    public:
        Mesh(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
             size_t parentID, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        Mesh(Mesh&& other) noexcept;

        [[nodiscard]] size_t GetParentID() const;
        void Draw(vk::CommandBuffer& cmdBuf) const;
    private:
        size_t m_ParentID;
        size_t m_VertexCount;
        size_t m_IndexCount;
        std::unique_ptr<Buffer<Vertex>> m_VertexBuffer;
        std::unique_ptr<Buffer<uint32_t>> m_IndexBuffer;
    };
}
