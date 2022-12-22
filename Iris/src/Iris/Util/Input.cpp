#include "Input.hpp"

namespace Iris {
    Input::Input() {
        on<MousePosition>([this](glm::vec2 pos) {
            m_MousePos = pos;
        });
        on<KeyPress>([this](int key, KeyMods mods) {
            m_State[key] = true;
        });
        on<KeyRelease>([this](int key, KeyMods mods) {
            m_State[key] = false;
        });
    };

    Input& Input::Get() {
        static Input instance;
        return instance;
    }
}
