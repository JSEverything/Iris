#include "GLShaderProgram.hpp"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

namespace Iris {
    bool checkShaderStatus(GLuint shader)
    {
        GLint stat;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);
        if(stat == GL_FALSE)
        {
            GLchar infoLog[10240];
            glGetShaderInfoLog(shader, 10240, nullptr, infoLog);
            Log::Core::Error("Shader error: {}", infoLog);
            return false;
        }
        return true;
    }

    bool checkProgramStatus(GLuint program)
    {
        GLint stat;
        glGetProgramiv(program, GL_LINK_STATUS, &stat);
        if(stat == GL_FALSE)
        {
            GLchar infoLog[10240];
            glGetProgramInfoLog(program, 10240, nullptr, infoLog);
            Log::Core::Error("Program error: {}", infoLog);
            return false;
        }
        return true;
    }

    GLShaderProgram::GLShaderProgram() {
        m_ShaderProgram = glCreateProgram();
    }

    void GLShaderProgram::AddShader(std::string_view path, GLenum type) {
        std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            Log::Core::Error("Shader {} not found", path);
        }

        auto fileSize = static_cast<size_t>(file.tellg());
        std::string code;
        code.resize(fileSize);

        file.seekg(0);
        file.read(code.data(), static_cast<std::streamsize>(fileSize));
        file.close();

        auto shader = glCreateShader(type);
        GLchar* sources[] = { const_cast<GLchar*>(code.c_str()) };
        glShaderSource(shader, 1, sources, nullptr);

        glCompileShader(shader);
        //glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, code.c_str(), static_cast<GLint>(code.size()));
        //        glSpecializeShader(shader, "main", 0, nullptr, nullptr);

        if(checkShaderStatus(shader)) {
            glAttachShader(m_ShaderProgram, shader);
            glDeleteShader(shader);
        }
    }

    void GLShaderProgram::AddVertexShader(std::string_view path) {
        AddShader(path, GL_VERTEX_SHADER);
    }

    void GLShaderProgram::AddFragmentShader(std::string_view path) {
        AddShader(path, GL_FRAGMENT_SHADER);
    }

    void GLShaderProgram::Link() const {
        glLinkProgram(m_ShaderProgram);
        checkProgramStatus(m_ShaderProgram);
    }

    void GLShaderProgram::Use() const {
        glUseProgram(m_ShaderProgram);
    }

    void GLShaderProgram::SetUniform(std::string_view name, const glm::vec3& data) {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.data());
        if (location == -1) return;
        glUniform3f(location, data.x, data.y, data.z);
    }

    void GLShaderProgram::SetUniform(std::string_view name, const glm::mat4x4& data) {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.data());
        if (location == -1) return;
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
    }

    void GLShaderProgram::SetUniform(std::string_view name, const float& data) {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.data());
        if (location == -1) return;
        glUniform1f(location, data);
    }
}
