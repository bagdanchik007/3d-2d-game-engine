#pragma once

#include <algorithm>
#include <cmath>

namespace Engine::Math
{
    inline constexpr float Pi = 3.14159265358979323846f;
    inline constexpr float TwoPi = 2.0f * Pi;

    // Default tolerance for floating-point comparisons throughout the math
    // library. Exact equality (operator==) is provided on the vector/
    // matrix types too, for the rare cases that genuinely want it (e.g.
    // "did this value change at all"), but most geometric comparisons
    // should go through NearlyEqual to survive accumulated rounding error.
    inline constexpr float DefaultEpsilon = 1e-5f;

    [[nodiscard]] constexpr float Radians(float degrees) noexcept
    {
        return degrees * (Pi / 180.0f);
    }

    [[nodiscard]] constexpr float Degrees(float radians) noexcept
    {
        return radians * (180.0f / Pi);
    }

    [[nodiscard]] inline bool NearlyEqual(float a, float b, float epsilon = DefaultEpsilon) noexcept
    {
        return std::fabs(a - b) <= epsilon;
    }

    template <typename T>
    [[nodiscard]] constexpr T Clamp(T value, T low, T high) noexcept
    {
        return std::min(std::max(value, low), high);
    }

    template <typename T>
    [[nodiscard]] constexpr T Lerp(T a, T b, float t) noexcept
    {
        return static_cast<T>(a + (b - a) * t);
    }

} // namespace Engine::Math
