#pragma once

namespace Engine
{
    /// Strong value type for a per-frame delta time, in seconds.
    ///
    /// A bare `float deltaTime` parameter is ambiguous at every call site:
    /// seconds? milliseconds? Wrapping it removes that ambiguity and gives
    /// GetMilliseconds() a single, unambiguous home instead of scattering
    /// "* 1000.0f" across the codebase.
    class Timestep
    {
    public:
        explicit Timestep(float seconds = 0.0f) noexcept
            : m_Seconds(seconds)
        {
        }

        [[nodiscard]] float GetSeconds() const noexcept { return m_Seconds; }
        [[nodiscard]] float GetMilliseconds() const noexcept { return m_Seconds * 1000.0f; }

        // Implicit conversion to float (seconds) is intentional: most call
        // sites just want "dt" for a physics/animation update and forcing
        // .GetSeconds() everywhere would be ceremony without benefit.
        operator float() const noexcept { return m_Seconds; } // NOLINT(hicpp-explicit-conversions)

    private:
        float m_Seconds;
    };

} // namespace Engine
