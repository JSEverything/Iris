#include "GLMesh.hpp"

namespace Iris {
    GLMesh::GLMesh(GLenum primitiveType) : m_PrimitiveType(primitiveType) {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
    }

    void GLMesh::Render() const {
        glBindVertexArray(m_VAO);
        glDrawArrays(m_PrimitiveType, 0, static_cast<int>(m_VerticesCount));
    }

    template <typename T>
    void GLMesh::SetAttribute(GLMesh::BufferType type, const std::vector<T>& data) {
        if (!m_BufferObjects.contains(type)) {
            uint32_t vbo;
            glGenBuffers(1, &vbo);
            m_BufferObjects[type] = vbo;
        }

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_BufferObjects[type]);

        glBufferData(GL_ARRAY_BUFFER, static_cast<int64_t>(data.size() * sizeof(T)), data.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(T), nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void GLMesh::SetVertices(const std::vector<glm::vec4>& data) {
        m_VerticesCount = data.size();
        SetAttribute(BufferType::Vertices, data);
    }
}
