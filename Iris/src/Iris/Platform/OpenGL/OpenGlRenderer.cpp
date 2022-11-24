#include "OpenGLRenderer.hpp"

namespace Iris {
    OpenGLRenderer::OpenGLRenderer(const std::shared_ptr<Window>& window) : Renderer(window) {}

    void OpenGLRenderer::Init() {
        glfwMakeContextCurrent(m_Window->GetGLFWWindow());
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        Log::Core::Info("OpenGL {}.{}", GLVersion.major, GLVersion.minor);

        Resize(m_Window->GetWidth(), m_Window->GetHeight());
        LoadTriangle();
        LoadShaders();
    }

    void OpenGLRenderer::LoadShaders() {
        m_ShaderProgram = std::make_shared<GLShaderProgram>();

        m_ShaderProgram->AddVertexShader("../Shaders/src/simple.vert");
        m_ShaderProgram->AddFragmentShader("../Shaders/src/simple.frag");
        m_ShaderProgram->Link();
    }

    void OpenGLRenderer::LoadTriangle() {
        m_Triangle = std::make_shared<GLMesh>(GL_TRIANGLES);
        std::vector<glm::vec3> vertices = {{-1.f, -1.f, 0.f}, {1.f, -1.f, 0.f}, {0.f, 1.f, 0.f}};
        m_Triangle->SetVertices(vertices);
    }

    void OpenGLRenderer::Draw() {
        ++m_FrameNr;
        const float pi3 = std::numbers::pi/3;
        glm::vec3 color = abs(glm::sin(glm::vec3(static_cast<float>(m_FrameNr) / 10.f) / glm::vec3(120.f) + glm::vec3(0.f, pi3, 2 * pi3)));

        glClearColor(color.r, color.g, color.b, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_ShaderProgram->Use();
        m_Triangle->Render();
    }

    void OpenGLRenderer::Resize(uint32_t width, uint32_t height) {
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }

    void OpenGLRenderer::Cleanup() {

    }
}
