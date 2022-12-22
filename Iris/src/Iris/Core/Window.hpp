#pragma once
#include "Iris/Renderer/RenderAPI.hpp"
#include "Iris/Util/EventEmitter.hpp"
#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace Iris {
    struct WindowOptions {
        std::string_view Title;
        glm::ivec2 Size;
    };
    struct KeyMods {
        bool SHIFT;
        bool CONTROL;
        bool ALT;
        bool SUPER;
        bool CAPS_LOCK;
        bool NUM_LOCK;

        explicit KeyMods(int mods) {
            SHIFT = mods & GLFW_MOD_SHIFT;
            CONTROL = mods & GLFW_MOD_CONTROL;
            ALT = mods & GLFW_MOD_ALT;
            SUPER = mods & GLFW_MOD_SUPER;
            CAPS_LOCK = mods & GLFW_MOD_CAPS_LOCK;
            NUM_LOCK = mods & GLFW_MOD_NUM_LOCK;
        };
    };

    struct WindowClose final : public EventHandler<> {
    };
    struct WindowResize final : public EventHandler<uint32_t, uint32_t> {
    };
    struct Key final : public EventHandler<int, KeyMods> {
    };
    struct KeyPress final : public EventHandler<int, KeyMods> {
    };
    struct KeyRelease final : public EventHandler<int, KeyMods> {
    };
    struct KeyRepeat final : public EventHandler<int, KeyMods> {
    };

    class Window : public EventEmitter<
            WindowClose,
            WindowResize,
            Key,
            KeyPress,
            KeyRelease,
            KeyRepeat
    > {
    public:
        explicit Window(RenderAPI api, const WindowOptions& opts);
        ~Window();

        bool ShouldClose();

        [[nodiscard]] GLFWwindow* GetGLFWWindow() const { return m_Window; }

        [[nodiscard]] uint32_t GetWidth() const { return m_Size.x; };

        [[nodiscard]] uint32_t GetHeight() const { return m_Size.y; };

        [[nodiscard]] std::string_view GetTitle() const { return m_Title; };

        [[nodiscard]] RenderAPI GetAPI() const { return m_API; };
    private:
        GLFWwindow* m_Window;
        std::string m_Title;
        glm::ivec2 m_Size;
        RenderAPI m_API;
    };
}
