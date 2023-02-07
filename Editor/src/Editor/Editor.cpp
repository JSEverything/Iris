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
        floor.GetTransform().SetRotation({ 90.f, 00.f, 00.f });
        floor.GetTransform().SetTranslation({ 0.f, -20.f, 0.f });
        floor.GetTransform().SetScale({ 50.f, 50.f, 1.f });
        m_Scene->AddObject(floor);

        auto& axis = m_Scene->CreateObject();
        axis.AddComponent<Mesh>("../Assets/Axis/axis.obj");
        axis.AddComponent<Material>("../Assets/Axis/albedo.png");
        axis.GetTransform().SetScale({ 0.3f, 0.3f, 0.3f });
        m_Scene->AddObject(axis);

        /*auto& cube = m_Scene->CreateObject();
        cube.AddComponent<Mesh>("../Assets/cube.obj");
        cube.AddComponent<Material>("../Assets/RustingMetal/");
        cube.GetTransform().SetTranslation({ 2.f, 2.f, 2.f });
        m_Scene->AddObject(cube);*/
    }

    void Editor::OnUpdate(float dt) {
        m_Scene->Update(dt);
        m_Camera->Update(dt);

        float mul = dt / 16.667f;
        m_Renderer->Render(*m_Camera);
    }

    Editor::~Editor() {
        Iris::Log::App::Info("~Editor()");
        m_Renderer.reset();
        m_Camera.reset();
        m_Window.reset();
    }
}