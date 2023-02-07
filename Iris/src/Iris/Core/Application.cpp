#include "Application.hpp"
#include "Iris/Entity/Entity.hpp"
#include "Iris/Entity/Components/Mesh.hpp"

using namespace std::chrono_literals;

namespace Iris {
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationDetails details)
            : m_Details(std::move(details)) {
        s_Instance = this;
        m_Scene->SetThis(m_Scene);
        glfwInit();

        m_LastFrameFinished = std::chrono::high_resolution_clock::now();
    }

    Application::~Application() {
        glfwTerminate();
    }

    void Application::Run() {
        while (m_Renderer) {
            glfwPollEvents();
            if (m_Renderer->GetWindow() && glfwWindowShouldClose(m_Renderer->GetWindow()->GetGLFWWindow())) break;

            auto frameDuration = std::chrono::high_resolution_clock::now() - m_LastFrameFinished;
            uint64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(frameDuration).count();

            float dt = static_cast<float>(micros) / 1000.f;
            OnUpdate(dt);
            m_LastFrameFinished = std::chrono::high_resolution_clock::now();
        }
    }
}