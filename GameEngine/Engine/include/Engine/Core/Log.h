#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace Engine
{
    class ConsoleSink; // see Core/ConsoleSink.h - forward-declared here so ordinary logging call sites don't pull it in
}

// NOTE on a design trade-off, not a mistake to silently paper over:
// An earlier version of this header forward-declared spdlog::logger to keep
// spdlog out of Engine's public API entirely. That does not work: the
// ENGINE_*_LOG macros below call ->trace()/->info() at the *call site* in
// Sandbox/Tests, which requires the complete type there, not just a pointer
// to an incomplete one. Fully hiding spdlog would mean replacing these
// macros with non-template Log:: methods taking pre-formatted strings,
// which throws away spdlog's compile-time format-string checking for a
// stable, well-regarded dependency. That trade isn't worth it here, so
// Engine's public logging API does depend on spdlog's logger type.

namespace Engine
{
    /// Facade over the engine's logging backend.
    ///
    /// Two independent loggers are exposed rather than one shared logger:
    ///   - "Core" logger: engine-internal diagnostics (tagged ENGINE).
    ///   - "Client" logger: messages from code using the engine (tagged APP).
    /// Separating them lets consumers filter engine noise from game/app
    /// noise in the console without any string parsing.
    class Log
    {
    public:
        Log() = delete; // Static utility class: never instantiated.

        /// Must be called once before any ENGINE_*_LOG macro is used.
        /// Not done via static initialization because logger construction
        /// order relative to other static objects would otherwise be
        /// unspecified (the classic "static initialization order fiasco").
        static void Init();

        [[nodiscard]] static std::shared_ptr<spdlog::logger>& GetCoreLogger();
        [[nodiscard]] static std::shared_ptr<spdlog::logger>& GetClientLogger();

        /// The sink ConsolePanel (M13) reads from - see ConsoleSink.h for
        /// why it exists as a real spdlog sink rather than a second,
        /// parallel capture mechanism. Returns the same instance attached
        /// to both loggers in Init(), so ConsolePanel sees Core and
        /// Client messages together, exactly as they'd appear in the
        /// terminal's interleaved output.
        [[nodiscard]] static std::shared_ptr<ConsoleSink> GetConsoleSink();

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
        static std::shared_ptr<ConsoleSink> s_ConsoleSink;
    };

} // namespace Engine

// ---------------------------------------------------------------------------
// Convenience macros
// ---------------------------------------------------------------------------
// Macros (not functions) so spdlog's compile-time format-string checking and
// source-location capture still work at the call site.

#define ENGINE_CORE_TRACE(...)    ::Engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ENGINE_CORE_INFO(...)     ::Engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ENGINE_CORE_WARN(...)     ::Engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...)    ::Engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ENGINE_CORE_CRITICAL(...) ::Engine::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define ENGINE_TRACE(...)    ::Engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)     ::Engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...)     ::Engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)    ::Engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ENGINE_CRITICAL(...) ::Engine::Log::GetClientLogger()->critical(__VA_ARGS__)
