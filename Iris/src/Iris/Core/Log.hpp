#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Iris {
    class Log {
    public:
        static void Init();

        struct [[maybe_unused]] Core {
            template <typename ... Args>
            [[maybe_unused]] static inline void Trace(Args&& ... args) {
                return s_CoreLogger->template trace(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Info(Args&& ... args) {
                return s_CoreLogger->template info(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Warn(Args&& ... args) {
                return s_CoreLogger->template warn(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Error(Args&& ... args) {
                return s_CoreLogger->template error(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Critical(Args&& ... args) {
                return s_CoreLogger->template critical(std::forward<Args>(args)...);
            }

        private:
            static std::shared_ptr<spdlog::logger> s_CoreLogger;
            static void Init(std::vector<spdlog::sink_ptr> sinks);
            friend Iris::Log;
        };

        struct [[maybe_unused]] App {
            template <typename ... Args>
            [[maybe_unused]] static inline void Trace(Args&& ... args) {
                return s_AppLogger->template trace(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Info(Args&& ... args) {
                return s_AppLogger->template info(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Warn(Args&& ... args) {
                return s_AppLogger->template warn(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Error(Args&& ... args) {
                return s_AppLogger->template error(std::forward<Args>(args)...);
            }

            template <typename ... Args>
            [[maybe_unused]] static inline void Critical(Args&& ... args) {
                return s_AppLogger->template critical(std::forward<Args>(args)...);
            }

        private:
            static std::shared_ptr<spdlog::logger> s_AppLogger;
            static void Init(std::vector<spdlog::sink_ptr> sinks);
            friend Iris::Log;
        };
    };
}