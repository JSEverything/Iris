#include "Application.hpp"

namespace Iris {
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationDetails details)
            : m_Details(std::move(details)) {
        s_Instance = this;
        m_Renderer = Renderer::Create(RenderAPI::Vulkan, { m_Details.Name, m_Details.DesiredSize });
        m_Renderer->Init();
    }

    Application::~Application() = default;

    void Application::Run() {
        while (m_Running) {
            glfwPollEvents();
            if (m_Renderer->GetWindow().ShouldClose()) Close();
            OnUpdate();
        }

        m_Renderer->Cleanup();
    }
}