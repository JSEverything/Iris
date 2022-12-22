#include "Editor.hpp"

namespace Iris {
    Editor::Editor(const ApplicationDetails& details) : Application(details) {
        Iris::Log::App::Info("Editor()");

        auto& camera = m_Scene->CreateObject();
        camera.AddComponent<Camera>();
        camera.GetTransform().Move({ 0.f, 0.f, 0.f });
        camera.GetTransform().Rotate({ 0.f, 0.f, 0.f });
        m_Scene->AddObject(camera);

        for (uint32_t i = 0; i < 6; ++i) {
            auto& monkey = m_Scene->CreateObject();
            monkey.AddComponent<Mesh>("../Assets/monkey-hd.obj");
            monkey.GetTransform()
                    .Move({ (float)i * 4.f - 7.5f, glm::sin((float)i / 5.f * 3.14f * 2.5f) * 6.f, -5.f });
            monkey.GetTransform().SetScale({ i, i, i });
            m_Scene->AddObject(monkey);
        }

        auto& floor = m_Scene->CreateObject();
        floor.AddComponent<Mesh>("../Assets/plane.obj");
        floor.GetTransform().SetRotation({ 90.f, 00.f, 00.f });
        floor.GetTransform().SetTranslation({ 0.f, -20.f, 0.f });
        floor.GetTransform().SetScale({ 50.f, 50.f, 1.f });
        m_Scene->AddObject(floor);

        auto& axes = m_Scene->CreateObject();
        axes.AddComponent<Mesh>("../Assets/axes.obj");
        m_Scene->AddObject(axes);
    }

    void Editor::OnUpdate(float dt) {
        float mul = dt / 16.667f;
        for (uint32_t i = 1; i < 7; ++i) {
            m_Scene->GetEntity(i).GetTransform().Rotate({ (float)i * 0.1f * mul, (float)i * 0.2f * mul, 0.f });
        }
    }

    Editor::~Editor() {
        Iris::Log::App::Info("~Editor()");
    }
}