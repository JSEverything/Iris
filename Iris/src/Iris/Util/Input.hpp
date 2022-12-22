#pragma once
#include "EventEmitter.hpp"
#include "Iris/Core/Window.hpp"

namespace Iris {
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

    struct Key final : public EventHandler<int, KeyMods> {
    };
    struct KeyPress final : public EventHandler<int, KeyMods> {
    };
    struct KeyRelease final : public EventHandler<int, KeyMods> {
    };
    struct KeyRepeat final : public EventHandler<int, KeyMods> {
    };
    struct MouseMove final : public EventHandler<glm::vec2> {
    };
    struct MousePosition final : public EventHandler<glm::vec2> {
    };
    struct MouseScroll final : public EventHandler<glm::vec2> {
    };

    class Input : public EventEmitter<
            Key,
            KeyPress,
            KeyRelease,
            KeyRepeat,
            MouseMove,
            MousePosition,
            MouseScroll
    > {
    public:
        static Input& Get();

        static glm::vec2& GetMousePos() { return Get().m_MousePos; };

        static bool IsKeyPressed(int key) { return Get().m_State[key]; };
    private:
        Input();
    private:
        glm::vec2 m_MousePos{};
        bool m_State[GLFW_KEY_LAST + 1]{};
    };
}
