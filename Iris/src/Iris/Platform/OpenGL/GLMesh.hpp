#pragma once
#include <map>
#include <glad/glad.h>

namespace Iris {
    class GLMesh {
    public:
        enum class BufferType : uint16_t {
            Vertices = 0,
            Indices,
            Colors
        };

        explicit GLMesh(GLenum primitiveType);
        void Render() const;

        template <typename T>
        void SetAttribute(GLMesh::BufferType type, const std::vector<T>& data);
        void SetVertices(const std::vector<glm::vec3>& data);
    private:
        std::map<BufferType, uint32_t> m_BufferObjects;
        GLenum m_PrimitiveType;
        GLuint m_VAO{};
        uint32_t m_VerticesCount = 0;
    };
}
