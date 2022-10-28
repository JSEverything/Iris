#include "Application.hpp"

namespace Iris {
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationDetails details)
            : m_Details(std::move(details)) {
        s_Instance = this;
        m_Window = std::make_shared<Window>(m_Details.Name, m_Details.DesiredSize);
    }

    Application::~Application() = default;

    void Application::Run() {
        while (m_Running) {
            glfwPollEvents();
            if (m_Window->ShouldClose()) Close();
            OnUpdate();
        }
    }
}