#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "Iris/Platform/OpenGL/GLMesh.hpp"
#include "Iris/Platform/OpenGL/GLShaderProgram.hpp"

namespace Iris {
    class OpenGLRenderer : public Renderer {
    public:
        explicit OpenGLRenderer(const WindowOptions& opts);
    private:
        void Init() override;
        void Draw() override;
        void Cleanup() override;

        void LoadShaders();
        void LoadTriangle();
    private:
        std::shared_ptr<GLMesh> m_Triangle{};
        std::shared_ptr<GLShaderProgram> m_ShaderProgram{};
    };
}
