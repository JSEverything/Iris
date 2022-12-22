#include "Application.hpp"
#include "Iris/Entity/Entity.hpp"
#include "Iris/Entity/Components/Mesh.hpp"

namespace Iris {
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationDetails details)
            : m_Details(std::move(details)) {
        s_Instance = this;
        m_Scene->SetThis(m_Scene);
        glfwInit();
        m_Renderers.push_back(Renderer::Create(RenderAPI::OpenGL,
                                               WindowOptions{ m_Details.Name + " [OpenGL]",
                                                              m_Details.DesiredSize }, m_Scene));
        //m_Renderers.emplace_back(Renderer::Create(RenderAPI::Vulkan,
        //                                          WindowOptions{ m_Details.Name + " [Vulkan]",
        //                                                         m_Details.DesiredSize }, m_Scene));

        auto& camera = m_Scene->CreateObject();
        camera.AddComponent<Camera>();
        camera.GetTransform().Move({0.f, 0.f, 0.f});
        camera.GetTransform().Rotate({0.f, 0.f, 0.f});
        m_Scene->AddObject(camera);

        for (uint32_t i = 0; i < 6; ++i) {
            auto& monkey = m_Scene->CreateObject();
            monkey.AddComponent<Mesh>("../Assets/monkey-hd.obj");
            monkey.GetTransform().Move({(float)i * 3.f - 7.5f, glm::sin((float)i / 5.f * 3.14f * 2.5f) * 3.f, -5.f});
            m_Scene->AddObject(monkey);
        }
    }

    Application::~Application() {
        m_Renderers.clear();
        glfwTerminate();
    }

    void Application::Run() {
        while (!m_Renderers.empty()) {
            glfwPollEvents();
            m_Scene->Update(16.667f);
            OnUpdate();
            for (uint32_t i = 1; i < 7; ++i) {
                m_Scene->GetEntity(i).GetTransform().Rotate({(float)i * 0.01f, (float)i * 0.02f, 0.f});
            }

            for (size_t i = 0; i < m_Renderers.size(); ++i) {
                if (!m_Renderers[i]->IsRunning()) {
                    if (m_Renderers.size() > 1) std::swap(m_Renderers[i], m_Renderers[m_Renderers.size() - 1]);
                    m_Renderers.pop_back();
                }
            }
        }
    }
}