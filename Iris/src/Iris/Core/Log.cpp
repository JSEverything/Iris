#include "Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Iris {
    std::shared_ptr<spdlog::logger> Log::App::s_AppLogger;
    std::shared_ptr<spdlog::logger> Log::Core::s_CoreLogger;

    void Log::Init() {
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Iris.log", true));
        logSinks[0]->set_pattern("%^[%T] [%l] %n: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] %n: %v");

        Log::Core::Init(logSinks);
        Log::App::Init(logSinks);
    }

    void Log::Core::Init(std::vector<spdlog::sink_ptr> sinks) {
        s_CoreLogger = std::make_shared<spdlog::logger>("Core", begin(sinks), end(sinks));
        spdlog::register_logger(s_CoreLogger);
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);
    }

    void Log::App::Init(std::vector<spdlog::sink_ptr> sinks) {
        s_AppLogger = std::make_shared<spdlog::logger>("App", begin(sinks), end(sinks));
        spdlog::register_logger(s_AppLogger);
        s_AppLogger->set_level(spdlog::level::trace);
        s_AppLogger->flush_on(spdlog::level::trace);
    }
}