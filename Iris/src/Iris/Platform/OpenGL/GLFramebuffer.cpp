#include "GLFramebuffer.hpp"

namespace Iris {
    GLuint GLFramebuffer::DefaultFramebuffer = 0;

    void GLFramebuffer::bindDefault() {
        glBindFramebuffer(GL_FRAMEBUFFER, GLFramebuffer::DefaultFramebuffer);
    }

    GLFramebuffer::GLFramebuffer() {
        m_fbo = 0;
        m_rbo = 0;
        m_depthTex = 0;
        depthTexture = nullptr;
    }

    void GLFramebuffer::initialize(int w, int h) {
        m_width = w;
        m_height = h;

        if (m_fbo)
            release();

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFramebuffer::release() {
        if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;

        if (m_rbo) glDeleteRenderbuffers(1, &m_rbo);
        m_rbo = 0;

        textures.clear();
    }

    bool GLFramebuffer::isValid() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        GLenum stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (stat == GL_FRAMEBUFFER_COMPLETE) return true;
        return false;
    }

    void GLFramebuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        std::vector<GLenum> buffs(textures.size());
        for (std::pair<int, GLTexture*> tx: textures)
            buffs[tx.first] = GL_COLOR_ATTACHMENT0 + tx.first;

        glDrawBuffers(textures.size(), buffs.data());

        GLenum stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (stat != GL_FRAMEBUFFER_COMPLETE)
            Log::Core::Error("Framebuffer is not complete.");
    }

    void GLFramebuffer::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, GLFramebuffer::DefaultFramebuffer);
    }

    void GLFramebuffer::setTextureAsColorAttachment(int unit, GLTexture* tex) {
        if (m_fbo) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            if (tex->getSize().x != m_width || tex->getSize().y != m_height) {
                tex->create({ m_width, m_height });

            }
            textures[unit] = tex;
            textures[unit]->bind(0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + unit, GL_TEXTURE_2D,
                                   textures[unit]->getHandle(),
                                   0);
            GLTexture::unbind(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void GLFramebuffer::setTextureAsDepthAttachment(GLTexture* tex) {
        if (m_fbo) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

            if (tex->getSize().x != m_width || tex->getSize().y != m_height)
                tex->createDepth({ m_width, m_height });

            depthTexture = tex;
            depthTexture->bind(0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->getHandle(), 0);

            Iris::GLTexture::unbind();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
}
