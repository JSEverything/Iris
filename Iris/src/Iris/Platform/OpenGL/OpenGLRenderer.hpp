#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "Iris/Platform/OpenGL/GLMesh.hpp"
#include "Iris/Platform/OpenGL/GLShaderProgram.hpp"

namespace Iris {
    class OpenGLRenderer : public Renderer {
    public:
        OpenGLRenderer(const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
    private:
        void Init() override;
        void Draw() override;
        void Cleanup() override;

        void LoadShaders();
    private:
        std::shared_ptr<GLShaderProgram> m_ShaderProgram{};
        std::vector<GLMesh> m_Meshes;
        std::vector<size_t> m_EntityQueue;
        std::mutex m_QueueMutex;
    };
}
