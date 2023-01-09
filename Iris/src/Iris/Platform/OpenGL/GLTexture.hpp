#pragma once
#include <glad/glad.h>

namespace Iris {
    class GLTexture {
    public:
        GLTexture();
        void loadFromFile(std::string_view path);
        void bind(GLuint unit) const;
        static void unbind();
    private:
        GLuint m_Handle;
    };
}

