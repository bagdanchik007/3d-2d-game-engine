#include "Engine/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace Engine
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        // Idempotent by necessity, not by accident: every test file that
        // uses logging guards its own Log::Init() call with a
        // translation-unit-local static bool (see CoreTests.cpp,
        // ApplicationTests.cpp). Those guards don't share state with each
        // other, so within one test binary Init() legitimately gets called
        // more than once. Without this early-out, spdlog::register_logger
        // throws "logger already exists" on the second call.
        if (s_CoreLogger != nullptr)
        {
            return;
        }

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        // Pattern: [time] [logger-tag] message
        sinks[0]->set_pattern("%^[%T] %n: %v%$");

        s_CoreLogger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
        spdlog::register_logger(s_CoreLogger);
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);

        s_ClientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
        spdlog::register_logger(s_ClientLogger);
        s_ClientLogger->set_level(spdlog::level::trace);
        s_ClientLogger->flush_on(spdlog::level::trace);
    }

    std::shared_ptr<spdlog::logger>& Log::GetCoreLogger()
    {
        return s_CoreLogger;
    }

    std::shared_ptr<spdlog::logger>& Log::GetClientLogger()
    {
        return s_ClientLogger;
    }

} // namespace Engine
