#pragma once
#include "Iris/Renderer/Renderer.hpp"
#include "Iris/Platform/OpenGL/GLMesh.hpp"
#include "Iris/Platform/OpenGL/GLShaderProgram.hpp"

namespace Iris {
    class OpenGLRenderer : public Renderer {
    public:
        explicit OpenGLRenderer(const std::shared_ptr<Window>& window);
        void Init() override;
        void Draw() override;
        void Resize(uint32_t width, uint32_t height);
        void Cleanup() override;
    private:
        void LoadShaders();
        void LoadTriangle();
    private:
        uint64_t m_FrameNr = 0;
        std::shared_ptr<GLMesh> m_Triangle{};
        std::shared_ptr<GLShaderProgram> m_ShaderProgram{};
    };
}
