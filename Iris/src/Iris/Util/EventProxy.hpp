#pragma once
#include "EventEmitter.hpp"
#include "Iris/Core/Window.hpp"

namespace Iris {
    class EventProxy : public EventEmitter<
            Key,
            KeyPress,
            KeyRelease,
            KeyRepeat
            >{
    public:
        static EventProxy& Get();
    protected:
        EventProxy();
    };
}
