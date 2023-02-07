#include "ScopeTimer.hpp"

namespace Iris::Debug {

    ScopeTimer::ScopeTimer(std::string_view name) : m_Name(name),
                                                    m_StartPoint(std::chrono::high_resolution_clock::now()) {
    }

    ScopeTimer::~ScopeTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto difference = end - m_StartPoint;

        int64_t differenceNs = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count();
        float differenceMs = static_cast<float>(differenceNs) / 1'000'000.f;

        Log::Core::Info("Timer {} took {:.3f}ms", m_Name, differenceMs);
    }
}