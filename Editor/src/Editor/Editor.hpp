#pragma once
#include "Iris/Core/EntryPoint.hpp"
#include "Iris/Core/Application.hpp"
#include "Iris/Entity/Components/Camera.hpp"

using namespace std::chrono_literals;

namespace Iris {
    class Editor : public Application {
    public:
        explicit Editor(const ApplicationDetails& details);

        void OnUpdate(float dt) override;

        ~Editor() override;

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Camera> m_Camera;
    };

    Application* CreateApplication(const std::vector<std::string_view>& args) {
        ApplicationDetails details{
                .Name = "Iris Editor",
                .CommandLineArgs = args
        };
        return new Editor(details);
    }
}

