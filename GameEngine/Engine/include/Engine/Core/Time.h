#pragma once

namespace Engine
{
    /// Measures wall-clock time elapsed since Init() was called.
    ///
    /// Deliberately separate from Timestep: Timestep is a per-frame delta
    /// value passed around by value; Time is a single global clock that
    /// Application queries once per frame to *compute* that delta. Merging
    /// them would conflate "a duration" with "a clock that produces
    /// durations" — different responsibilities.
    class Time
    {
    public:
        Time() = delete;

        /// Must be called once before GetSeconds() is used. Not done via
        /// static initialization for the same reason as Log::Init(): avoids
        /// depending on unspecified static-initialization order.
        static void Init();

        [[nodiscard]] static float GetSeconds();

    private:
        static bool s_Initialized;
    };

} // namespace Engine
