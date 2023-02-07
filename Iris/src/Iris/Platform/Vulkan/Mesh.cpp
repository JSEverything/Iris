#include "Mesh.hpp"

namespace Iris::Vulkan {
    Mesh::Mesh(const std::shared_ptr<Context>& ctx, size_t parentID,
               const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
            : m_ParentID(parentID), m_VertexCount(vertices.size()), m_IndexCount(indices.size()) {
        m_VertexBuffer = std::make_unique<Buffer<Vertex>>(ctx, vk::BufferUsageFlagBits::eVertexBuffer, vertices);
        m_IndexBuffer = std::make_unique<Buffer<uint32_t>>(ctx, vk::BufferUsageFlagBits::eIndexBuffer, indices);
    }

    Mesh::Mesh(Mesh&& other) noexcept: m_VertexCount(other.m_VertexCount), m_IndexCount(other.m_IndexCount),
                                       m_ParentID(other.m_ParentID), m_VertexBuffer(std::move(other.m_VertexBuffer)),
                                       m_IndexBuffer(std::move(other.m_IndexBuffer)) {}

    size_t Mesh::GetParentID() const {
        return m_ParentID;
    }

    void Mesh::Draw(vk::CommandBuffer& cmdBuf) const {
        cmdBuf.bindVertexBuffers(0, m_VertexBuffer->m_Buffer, { 0 });
        cmdBuf.bindIndexBuffer(m_IndexBuffer->m_Buffer, 0, vk::IndexType::eUint32);

        cmdBuf.drawIndexed(m_IndexCount, 1, 0, 0, 1);
    }
}
