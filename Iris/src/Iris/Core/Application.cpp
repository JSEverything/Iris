#include "Application.hpp"

namespace Iris {
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationDetails details)
            : m_Details(std::move(details)) {
        s_Instance = this;
        glfwInit();
        glfwSetErrorCallback([](int error_code, const char* description){
            Log::Core::Error("GLFW error code {}: {}", error_code, description);
        });
        m_Renderers.push_back(Renderer::Create(RenderAPI::OpenGL,
                                               WindowOptions{ m_Details.Name + " [OpenGL]",
                                                              m_Details.DesiredSize }));
        m_Renderers.emplace_back(Renderer::Create(RenderAPI::Vulkan,
                                                  WindowOptions{ m_Details.Name + " [Vulkan]",
                                                                 m_Details.DesiredSize }));
    }

    Application::~Application() {
        m_Renderers.clear();
        glfwTerminate();
    }

    void Application::Run() {
        while (!m_Renderers.empty()) {
            glfwPollEvents();
            OnUpdate();

            for (size_t i = 0; i < m_Renderers.size(); ++i) {
                if (!m_Renderers[i]->IsRunning()) {
                    if (m_Renderers.size() > 1) std::swap(m_Renderers[i], m_Renderers[m_Renderers.size() - 1]);
                    m_Renderers.pop_back();
                }
            }
        }
    }
}