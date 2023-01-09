#include "GLTexture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Iris {
    GLTexture::GLTexture() {
        glGenTextures(1, &m_Handle);
    }

    void GLTexture::loadFromFile(std::string_view path) {
        int width, height, channels;
        unsigned char* data = stbi_load("../Assets/Cube2.png", &width, &height, &channels,
                                        0); // this is null for some reason
        if (data == nullptr) return;
        bind(m_Handle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        unbind();

        //stbi_image_free(data);
    }

    void GLTexture::bind(GLuint unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_Handle);
    }

    void GLTexture::unbind() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}