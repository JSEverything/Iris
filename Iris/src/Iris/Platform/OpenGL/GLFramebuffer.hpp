#pragma once
#include <map>
#include <glad/glad.h>
#include "Iris/Platform/OpenGL/GLTexture.hpp"

namespace Iris {
    class GLFramebuffer {
    public:
        static GLuint DefaultFramebuffer;
        static void bindDefault();

        int m_width{};
        int m_height{};
        int width() { return m_width; }
        int height() { return m_height; }

        std::map<int, GLTexture*> textures;
        GLTexture *depthTexture;

        GLFramebuffer();
        void initialize(int w, int h);

        void release();
        bool isValid();

        void bind();
        void unbind();

        void setTextureAsColorAttachment(int unit, GLTexture* tex);
        void setTextureAsDepthAttachment(GLTexture *tex);
    private:
        GLuint m_fbo, m_rbo;
        GLuint m_depthTex;
    };
}
