#pragma once
#include "Iris/Renderer/Renderer.hpp"

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
        virtual void OnUpdate(float dt) = 0;

        [[nodiscard]] const ApplicationDetails& GetDetails() const { return m_Details; }

        static Application& Get() { return *s_Instance; }

    private:
        void Run();
    protected:
        ApplicationDetails m_Details;
        std::unique_ptr<Renderer> m_Renderer;
        std::shared_ptr<Scene> m_Scene = std::make_shared<Scene>();
        std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<double>> m_LastFrameFinished;
    private:
        static Application* s_Instance;
        friend int::AppMain(const std::vector<std::string_view>& args);
    };

    Application* CreateApplication(const std::vector<std::string_view>& args);
}