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

        const uint16_t renderer = 1;

        if constexpr (renderer == 0) {
            m_Renderer = Renderer::Create(RenderAPI::OpenGL,
                                          WindowOptions{ m_Details.Name + " [OpenGL]",
                                                         m_Details.DesiredSize }, m_Scene);
        } else if constexpr (renderer == 1) {
            m_Renderer = Renderer::Create(RenderAPI::Vulkan,
                                          WindowOptions{ m_Details.Name + " [Vulkan]",
                                                         m_Details.DesiredSize }, m_Scene);
        }

        m_LastFrameFinished = std::chrono::high_resolution_clock::now();
    }

    Application::~Application() {
        m_Renderer.reset();
        glfwTerminate();
    }

    void Application::Run() {
        while (m_Renderer) {
            glfwPollEvents();
            auto frameDuration = std::chrono::high_resolution_clock::now() - m_LastFrameFinished;
            uint64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(frameDuration).count();

            float dt = static_cast<float>(micros) / 1000.f;
            m_Scene->Update(dt);
            OnUpdate(dt);

            if (!m_Renderer->IsRunning())
                m_Renderer.reset();

            m_LastFrameFinished = std::chrono::high_resolution_clock::now();
            std::this_thread::sleep_for(1ms);
        }
    }
}