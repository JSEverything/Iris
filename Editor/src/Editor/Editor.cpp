#include "Editor.hpp"
#include "Iris/Core/Log.hpp"
#include "Iris/Util/Input.hpp"

namespace Iris {
    Editor::Editor(const ApplicationDetails& details) : Application(details) {
        Iris::Log::App::Info("Editor()");

        m_Window = std::make_shared<Window>(RenderAPI::Vulkan, WindowOptions{ "Iris Editor", { 1600, 900 } });
        m_Renderer = Renderer::Create(RenderAPI::Vulkan, m_Window);
        m_Renderer->SetScene(m_Scene);

        m_Camera = std::make_shared<Camera>(-1, nullptr, 90.f, 1600.f / 900.f);

        auto& floor = m_Scene->CreateObject();
        floor.AddComponent<Mesh>("../Assets/plane.obj");
        floor.GetTransform().SetTranslation({ 0.f, -0.5f, 0.f });
        m_Scene->AddObject(floor);

        auto& axis = m_Scene->CreateObject();
        axis.AddComponent<Mesh>("../Assets/Axis/axis.obj");
        axis.AddComponent<Material>("../Assets/Axis/albedo.png");
        axis.GetTransform().SetScale({ 0.3f, 0.3f, 0.3f });
        axis.GetTransform().SetTranslation({ -1.7f, 0.f, -7.3f });
        m_Scene->AddObject(axis);

        auto& light = m_Scene->CreateObject();
        light.AddComponent<Light>(glm::vec3(1.f, 0.f, 0.f), LightType::POINT);
        light.GetTransform().SetTranslation({ 0.f, 0.f, 5.f });
        m_Scene->AddObject(light);

        auto& light2 = m_Scene->CreateObject();
        light2.AddComponent<Light>(glm::vec3(0.f, 1.f, 0.f), LightType::POINT);
        light2.GetTransform().SetTranslation({ -4.f, 0.f, 5.f });
        m_Scene->AddObject(light2);

        auto& light3 = m_Scene->CreateObject();
        light3.AddComponent<Light>(glm::vec3(0.f, 0.f, 1.f), LightType::POINT);
        light3.GetTransform().SetTranslation({ 1.5f, 1.f, 4.f });
        m_Scene->AddObject(light3);

        auto& light4 = m_Scene->CreateObject();
        light4.AddComponent<Light>(glm::vec3(0.1f, 0.1f, 0.1f), LightType::DIRECTIONAL);
        light4.GetTransform().SetTranslation({ 3.f, 1.f, 7.f });
        m_Scene->AddObject(light4);

        auto& light5 = m_Scene->CreateObject();
        light5.AddComponent<Light>(glm::vec3(0.6f, 0.6f, 0.6f), LightType::SPOT);
        light5.GetTransform().SetTranslation({ -3.f, 5.f, -7.f });
        m_Scene->AddObject(light5);

        auto& light6 = m_Scene->CreateObject();
        light6.AddComponent<Light>(glm::vec3(0.6f, 0.6f, 0.6f), LightType::SPOT);
        light6.GetTransform().SetTranslation({ 3.f, 5.f, 7.f });
        m_Scene->AddObject(light6);

        auto& monkey = m_Scene->CreateObject();
        monkey.AddComponent<Mesh>("../Assets/monkey-low.obj");
        monkey.GetTransform().SetTranslation({ 1.f, 1.f, 3.f });
        m_Scene->AddObject(monkey);
    }

    void Editor::OnUpdate(float dt) {
        m_Scene->Update(dt);
        m_Camera->Update(dt);

        //float mul = dt / 16.667f;
        m_Renderer->Render(*m_Camera);
    }

    Editor::~Editor() {
        Iris::Log::App::Info("~Editor()");
        m_Renderer.reset();
        m_Camera.reset();
        m_Window.reset();
    }
}