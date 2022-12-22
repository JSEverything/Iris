#pragma once
#include <map>
#include <glad/glad.h>
#include "Iris/Renderer/Vertex.hpp"

namespace Iris {
    class GLMesh {
    public:
        enum class BufferType : uint16_t {
            Vertices = 0,
            Indices,
            Colors
        };

        explicit GLMesh(GLenum primitiveType, size_t parent);
        void Render();

        template <typename T>
        void SetAttribute(GLMesh::BufferType type, const std::vector<T>& data);
        void SetVertices(const std::vector<Vertex>& data);
        void SetIndices(const std::vector<uint32_t>& data);
        [[nodiscard]] size_t GetParentId() const { return m_ParentId; };
    private:
        std::map<BufferType, uint32_t> m_BufferObjects;
        GLenum m_PrimitiveType;
        GLuint m_VAO{};
        uint32_t m_VerticesCount = 0;
        uint32_t m_IndicesCount = 0;
        size_t m_ParentId;
    };
}
