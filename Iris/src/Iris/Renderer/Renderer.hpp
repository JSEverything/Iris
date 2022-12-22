#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include "Iris/Core/Window.hpp"
#include "Iris/Scene/Scene.hpp"

namespace Iris {
    class Renderer {
    public:
        ~Renderer();

        [[nodiscard]] bool IsRunning() const { return m_Running; };

        void Close() { m_Running = false; };

        [[nodiscard]] std::shared_ptr<Window> GetWindow() const { return m_Window; };

        static std::shared_ptr<Renderer>
        Create(RenderAPI api, const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
    protected:
        explicit Renderer(RenderAPI api, const WindowOptions& opts, const std::shared_ptr<Scene>& scene);
    private:
        void Run();

        virtual void Init() = 0;
        virtual void Draw() = 0;
        virtual void Cleanup() = 0;
    public:
        size_t m_CameraEntityId = -1;
    protected:
        std::shared_ptr<Window> m_Window;
        std::thread m_Thread;
        std::mutex m_Mutex;
        std::atomic<bool> m_Running = true;
        uint64_t m_FrameNr = 0;
        std::shared_ptr<Scene> m_Scene;
    };
}

