#include "Editor.hpp"
#include "Iris/Core/Log.hpp"
#include "Iris/Util/Input.hpp"

namespace Iris {
    Editor::Editor(const ApplicationDetails& details) : Application(details) {
        Iris::Log::App::Info("Editor()");

        auto& camera = m_Scene->CreateObject();
        camera.AddComponent<Camera>();
        camera.GetTransform().Move({ 0.f, 0.f, 0.f });
        camera.GetTransform().Rotate({ 0.f, 0.f, 0.f });
        m_Scene->AddObject(camera);

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

        auto& cube = m_Scene->CreateObject();
        cube.AddComponent<Mesh>("../Assets/cube.obj");
        cube.AddComponent<Material>("../Assets/RustingMetal/");
        cube.GetTransform().SetTranslation({ 2.f, 2.f, 2.f });
        m_Scene->AddObject(cube);
    }

    void Editor::OnUpdate(float dt) {
        float mul = dt / 16.667f;

    }

    Editor::~Editor() {
        Iris::Log::App::Info("~Editor()");
    }
}