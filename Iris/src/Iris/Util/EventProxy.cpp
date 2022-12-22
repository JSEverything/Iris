#include "EventProxy.hpp"

namespace Iris {
    EventProxy::EventProxy() = default;

    EventProxy& EventProxy::Get() {
        static EventProxy instance;
        return instance;
    }
}
