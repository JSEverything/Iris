#pragma once
#include <spdlog/spdlog.h>

namespace Iris {
    class Log {
    public:
        static void Init();

        struct [[maybe_unused]] Core {
            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Trace(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_CoreLogger->log(spdlog::level::trace, std::forward<fmt::format_string<Args...>>(fmt),
                                         std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Info(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_CoreLogger->log(spdlog::level::info, std::forward<fmt::format_string<Args...>>(fmt),
                                         std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Warn(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_CoreLogger->log(spdlog::level::warn, std::forward<fmt::format_string<Args...>>(fmt),
                                         std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Error(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_CoreLogger->log(spdlog::level::err, std::forward<fmt::format_string<Args...>>(fmt),
                                         std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Critical(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_CoreLogger->log(spdlog::level::critical, std::forward<fmt::format_string<Args...>>(fmt),
                                         std::forward<Args>(args)...);
            }

        private:
            static std::shared_ptr<spdlog::logger> s_CoreLogger;
            static void Init(std::vector<spdlog::sink_ptr> sinks);
            friend Iris::Log;
        };

        struct [[maybe_unused]] App {
            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Trace(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_AppLogger->log(spdlog::level::trace, std::forward<fmt::format_string<Args...>>(fmt),
                                        std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Info(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_AppLogger->log(spdlog::level::info, std::forward<fmt::format_string<Args...>>(fmt),
                                        std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Warn(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_AppLogger->log(spdlog::level::warn, std::forward<fmt::format_string<Args...>>(fmt),
                                        std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Error(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_AppLogger->log(spdlog::level::err, std::forward<fmt::format_string<Args...>>(fmt),
                                        std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static constexpr inline void Critical(fmt::format_string<Args...>&& fmt, Args&& ... args) {
                return s_AppLogger->log(spdlog::level::critical, std::forward<fmt::format_string<Args...>>(fmt),
                                        std::forward<Args>(args)...);
            }

        private:
            static std::shared_ptr<spdlog::logger> s_AppLogger;
            static void Init(std::vector<spdlog::sink_ptr> sinks);
            friend Iris::Log;
        };
    };
}