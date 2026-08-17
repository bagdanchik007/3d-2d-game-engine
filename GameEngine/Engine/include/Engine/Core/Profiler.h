#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace Engine
{
    struct ProfileResult
    {
        std::string Name;
        float DurationMs;
    };

    /// Collects named scope timings for the CURRENT frame only - not a
    /// historical trace, not a sampling profiler. Deliberately minimal:
    /// this answers "which named sections of this specific frame took how
    /// long", which is enough for ProfilerPanel's (M13) table view and for
    /// spotting an obviously expensive section during development, without
    /// building out a full timeline/flame-graph capture system nothing in
    /// this engine currently has a demonstrated need for.
    ///
    /// A Core utility, not a Renderer or UI class: frame timing is a
    /// generic concept Application already owns the raw ingredients for
    /// (see Application::Run() calling SetLastFrameTimeMs each frame) -
    /// no GPU or ImGui dependency belongs here.
    class Profiler
    {
    public:
        Profiler() = delete;

        /// Clears the previous frame's results - called once per frame by
        /// Application::Run(), before any layer's OnUpdate runs, so every
        /// ENGINE_PROFILE_SCOPE executed during this frame's update
        /// contributes to a clean, single-frame result set.
        static void BeginFrame() noexcept { s_Results.clear(); }

        static void Submit(std::string name, float durationMs)
        {
            s_Results.push_back(ProfileResult{std::move(name), durationMs});
        }

        [[nodiscard]] static const std::vector<ProfileResult>& GetResults() noexcept { return s_Results; }

        static void SetLastFrameTimeMs(float ms) noexcept { s_LastFrameTimeMs = ms; }
        [[nodiscard]] static float GetLastFrameTimeMs() noexcept { return s_LastFrameTimeMs; }

    private:
        static std::vector<ProfileResult> s_Results;
        static float s_LastFrameTimeMs;
    };

    /// RAII scope timer: measures its own lifetime and submits the result
    /// to Profiler on destruction. Used via the ENGINE_PROFILE_SCOPE macro
    /// below rather than constructed directly, purely so call sites read
    /// as a single declarative line rather than needing a named variable
    /// whose only purpose is to exist until the end of the scope.
    class ScopedTimer
    {
    public:
        explicit ScopedTimer(std::string name) noexcept
            : m_Name(std::move(name)), m_Start(std::chrono::steady_clock::now())
        {
        }

        ~ScopedTimer()
        {
            const auto end = std::chrono::steady_clock::now();
            const float durationMs = std::chrono::duration<float, std::milli>(end - m_Start).count();
            Profiler::Submit(m_Name, durationMs);
        }

        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

    private:
        std::string m_Name;
        std::chrono::steady_clock::time_point m_Start;
    };

} // namespace Engine

// ENGINE_PROFILE_SCOPE(name) declares a ScopedTimer that lives until the
// end of the enclosing scope; ## with __LINE__ gives each one a unique
// variable name so multiple scopes can coexist in the same function
// without a manual naming scheme at each call site.
#define ENGINE_PROFILE_SCOPE_LINE2(name, line) ::Engine::ScopedTimer timer##line(name)
#define ENGINE_PROFILE_SCOPE_LINE(name, line) ENGINE_PROFILE_SCOPE_LINE2(name, line)
#define ENGINE_PROFILE_SCOPE(name) ENGINE_PROFILE_SCOPE_LINE(name, __LINE__)
