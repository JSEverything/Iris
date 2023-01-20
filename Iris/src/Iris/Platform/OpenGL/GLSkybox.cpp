#include "GLSkybox.hpp"
#include <stb_image.h>

namespace Iris {
    GLSkybox::GLSkybox() = default;

    void GLSkybox::loadFromFolder(const std::string& path) {
        for (uint32_t i = 0; i < m_Textures.size(); ++i) {
            m_Textures[i].loadFromFile(path + "_" + m_Suffixes[i] + ".png");
        }
    }

    void GLSkybox::render() {

    }
}
