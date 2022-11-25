#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include "Iris/Core/Window.hpp"

namespace Iris {
    class Renderer {
    public:
        ~Renderer();

        [[nodiscard]] bool IsRunning() const { return m_Running; };

        void Close() { m_Running = false; };

        static std::shared_ptr<Renderer>
        Create(RenderAPI api, const WindowOptions& opts);
    protected:
        explicit Renderer(RenderAPI api, const WindowOptions& opts);
    private:
        void Run();

        virtual void Init() = 0;
        virtual void Draw() = 0;
        virtual void Cleanup() = 0;
    protected:
        std::shared_ptr<Window> m_Window;
        std::thread m_Thread;
        std::mutex m_Mutex;
        std::atomic<bool> m_Running = true;
        uint64_t m_FrameNr = 0;
    };
}

