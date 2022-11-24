#include "Application.hpp"

namespace Iris {
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationDetails details)
            : m_Details(std::move(details)) {
        s_Instance = this;
        m_Window = std::make_shared<Window>(WindowOptions{ m_Details.Name, m_Details.DesiredSize }, RenderAPI::OpenGL);
        m_Renderer = Renderer::Create(RenderAPI::OpenGL, m_Window);
        m_Renderer->Init();
    }

    Application::~Application() = default;

    void Application::Run() {
        while (m_Running) {
            glfwPollEvents();
            m_Renderer->Draw();
            if (m_Window->ShouldClose()) Close();
            OnUpdate();
            glfwSwapBuffers(m_Window->GetGLFWWindow());
        }

        m_Renderer->Cleanup();
    }
}