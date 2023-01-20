#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "Iris/Platform/OpenGL/GLMesh.hpp"
#include "Iris/Platform/OpenGL/GLShaderProgram.hpp"
#include "Iris/Platform/OpenGL/GLTexture.hpp"
#include "Iris/Platform/OpenGL/GLSkybox.hpp"
#include "Iris/Platform/OpenGL/GLFramebuffer.hpp"

namespace Iris {
    class OpenGLRenderer : public Renderer {
    public:
        OpenGLRenderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
    private:
        void Init() override;
        void Draw() override;
        void Cleanup() override;

        void LoadShaders();
        void TransferObjects();
        void Render();
    private:
        std::shared_ptr<GLShaderProgram> m_ShaderProgram{};
        std::shared_ptr<GLShaderProgram> m_ShaderProgram2{};
        std::shared_ptr<GLSkybox> m_Skybox{};
        std::shared_ptr<GLFramebuffer> m_FrameBuffer{};
        std::shared_ptr<GLMesh> m_FramebufferMesh{};
        std::shared_ptr<GLTexture> m_FramebufferTexture{};
        std::shared_ptr<GLShaderProgram> m_FramebufferProgram{};

        std::vector<GLMesh> m_Meshes;
        std::vector<GLTexture> m_Textures;
        std::vector<size_t> m_EntityQueue;
        std::mutex m_QueueMutex;
    };
}
