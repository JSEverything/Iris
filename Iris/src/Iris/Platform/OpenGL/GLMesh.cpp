#include "GLMesh.hpp"

namespace Iris {
    GLMesh::GLMesh(GLenum primitiveType, size_t parent) : m_PrimitiveType(primitiveType), m_ParentId(parent) {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
    }

    void GLMesh::Render() {
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferObjects[BufferType::Indices]);
        glDrawElements(m_PrimitiveType, static_cast<GLsizei>(m_IndicesCount), GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(T), nullptr);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(Vertex, color));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(Vertex, normal));
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(T), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void GLMesh::SetVertices(const std::vector<Vertex>& data) {
        m_VerticesCount = data.size();
        SetAttribute(BufferType::Vertices, data);
    }

    void GLMesh::SetIndices(const std::vector<GLuint>& data) {
        if (!m_BufferObjects.contains(BufferType::Indices)) {
            uint32_t vbo;
            glGenBuffers(1, &vbo);
            m_BufferObjects[BufferType::Indices] = vbo;
        }

        m_IndicesCount = data.size();

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_BufferObjects[BufferType::Indices]);
        glBufferData(GL_ARRAY_BUFFER, m_IndicesCount * sizeof(GLuint), data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

}
