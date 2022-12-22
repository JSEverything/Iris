#pragma once
#include "Iris/Core/EntryPoint.hpp"
#include "Iris/Core/Application.hpp"

using namespace std::chrono_literals;

namespace Iris {
    class Editor : public Application {
    public:
        explicit Editor(const ApplicationDetails& details);

        void OnUpdate(float dt) override;

        ~Editor() override;

    private:
    };

    Application* CreateApplication(const std::vector<std::string_view>& args) {
        ApplicationDetails details{
                .Name = "Iris Editor",
                .CommandLineArgs = args
        };
        return new Editor(details);
    }
}

