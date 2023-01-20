#pragma once
#include <glad/glad.h>
#include "Iris/Platform/OpenGL/GLTexture.hpp"
#include "Iris/Platform/OpenGL/GLShaderProgram.hpp"

namespace Iris {
    class GLSkybox {
    public:
        GLSkybox();
        void loadFromFolder(const std::string& path);
        void render();
    private:
        std::array<GLTexture, 6> m_Textures;
        std::array<std::string, 6> m_Suffixes = {"px", "py", "pz", "nx", "ny", "nz"};
    };
}
