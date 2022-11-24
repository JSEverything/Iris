#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include "Iris/Core/Window.hpp"

namespace Iris {
    class Window;

    class Renderer {
    public:
        explicit Renderer(std::shared_ptr<Window> window);
        virtual ~Renderer();

        virtual void Init() = 0;
        virtual void Draw() = 0;
        virtual void Cleanup() = 0;

        static std::shared_ptr<Renderer> Create(RenderAPI api, const std::shared_ptr<Window>& window);
    protected:
        std::shared_ptr<Window> m_Window;
    };
}

