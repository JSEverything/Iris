#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include "Iris/Core/Window.hpp"

namespace Iris {
    class Renderer {
    public:
        explicit Renderer(const WindowOptions& opts);
        virtual ~Renderer();

        virtual void Init() = 0;
        virtual void Cleanup() = 0;

        Window& GetWindow() { return m_Window; }

        static std::shared_ptr<Renderer> Create(RenderAPI api, const WindowOptions& opts);
    protected:
        Window m_Window;
    };
}

