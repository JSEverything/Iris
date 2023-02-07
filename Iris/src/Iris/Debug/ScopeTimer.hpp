#pragma once

namespace Iris::Debug {
    class ScopeTimer {
    public:
        explicit ScopeTimer(std::string_view name);
        virtual ~ScopeTimer();
    private:
        std::string m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartPoint;
    };
}