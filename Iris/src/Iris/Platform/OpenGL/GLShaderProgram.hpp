#pragma once
#include <glad/glad.h>

namespace Iris {
    class GLShaderProgram {
    public:
        GLShaderProgram();
        void AddShader(std::string_view path, GLenum type);
        void AddVertexShader(std::string_view path);
        void AddFragmentShader(std::string_view path);
        void Link() const;
        void Use() const;
    private:
        GLuint m_ShaderProgram = -1;
    };
}
