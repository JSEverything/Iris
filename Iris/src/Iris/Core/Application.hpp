#pragma once
#include "Iris/Core/Window.hpp"

int AppMain(const std::vector<std::string_view>& args);

namespace Iris {
    struct ApplicationDetails {
        std::string Name;
        glm::ivec2 DesiredSize{ 1600, 900 };
        std::vector<std::string_view> CommandLineArgs;
    };

    class Application {
    public:
        explicit Application(ApplicationDetails details);
        virtual ~Application();
        virtual void OnUpdate() = 0;

        [[nodiscard]] const ApplicationDetails& GetDetails() const { return m_Details; }

        void Close() { m_Running = false; }

        static Application& Get() { return *s_Instance; }

    private:
        void Run();
    protected:
        ApplicationDetails m_Details;
        bool m_Running = true;
        std::shared_ptr<Window> m_Window;
    private:
        static Application* s_Instance;
        friend int::AppMain(const std::vector<std::string_view>& args);
    };

    Application* CreateApplication(const std::vector<std::string_view>& args);
}