#include "Iris/Core/EntryPoint.hpp"
#include "Iris/Core/Application.hpp"

namespace Iris {
    class Editor : public Application {
    public:
        explicit Editor(const ApplicationDetails& details)
                : Application(details) {
            Iris::Log::App::Info("Editor()");
        }

        void OnUpdate() override {

        }

        ~Editor() override {
            Iris::Log::App::Info("~Editor()");
        }
    };

    Application* CreateApplication(const std::vector<std::string_view>& args) {
        ApplicationDetails details{
                .Name = "Iris Editor",
                .CommandLineArgs = args
        };
        return new Editor(details);
    }
}

