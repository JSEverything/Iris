#pragma once

enum class EventType : uint32_t {
    WindowResize = 0,
    WindowClose,

    KB_KEY_DOWN,
    KB_KEY_REPEAT,
    KB_KEY_UP,

    MOUSE_BUTTON,
    MOUSE_MOVE,
};
