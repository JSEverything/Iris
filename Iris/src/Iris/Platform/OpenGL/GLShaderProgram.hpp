#pragma once
#include <glad/glad.h>

namespace Iris {
    class GLShaderProgram {
    public:
        GLShaderProgram();
        void AddShader(std::string_view path, GLenum type);
        void AddVertexShader(std::string_view path);
        void AddFragmentShader(std::string_view path);
        void SetUniform(std::string_view name, const glm::vec3& data);
        void SetUniform(std::string_view name, const glm::mat4x4& data);
        void SetUniform(std::string_view name, const float& data);
        void SetUniform(std::string_view name, const uint32_t& data);
        void Link() const;
        void Use() const;
    private:
        GLuint m_ShaderProgram = -1;
    };
}
