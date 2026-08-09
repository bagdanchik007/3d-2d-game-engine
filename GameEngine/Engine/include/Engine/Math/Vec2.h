#pragma once

#include "Engine/Math/MathUtils.h"

#include <cmath>

namespace Engine::Math
{
    /// Plain, trivially-copyable 2-component vector. Deliberately not a
    /// template<N> instantiation - see the M4 architecture note on why a
    /// generic Vec<N,T> is not justified here.
    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2() noexcept = default;
        constexpr explicit Vec2(float scalar) noexcept : x(scalar), y(scalar) {}
        constexpr Vec2(float x_, float y_) noexcept : x(x_), y(y_) {}

        constexpr Vec2& operator+=(const Vec2& rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
        constexpr Vec2& operator-=(const Vec2& rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
        constexpr Vec2& operator*=(float scalar) noexcept { x *= scalar; y *= scalar; return *this; }
        constexpr Vec2& operator/=(float scalar) noexcept { x /= scalar; y /= scalar; return *this; }

        [[nodiscard]] constexpr Vec2 operator-() const noexcept { return Vec2(-x, -y); }

        [[nodiscard]] constexpr bool operator==(const Vec2& rhs) const noexcept { return x == rhs.x && y == rhs.y; }
        [[nodiscard]] constexpr bool operator!=(const Vec2& rhs) const noexcept { return !(*this == rhs); }

        [[nodiscard]] float Length() const noexcept { return std::sqrt(x * x + y * y); }
        [[nodiscard]] constexpr float LengthSquared() const noexcept { return x * x + y * y; }

        /// Returns a zero vector if this vector's length is (near) zero,
        /// rather than dividing by zero and propagating NaN silently
        /// through everything downstream. A caller relying on a non-zero
        /// result from a zero-length input has a bug regardless of what
        /// this function returns; returning a well-defined, inert value
        /// makes that bug visible as "nothing moved" instead of NaN
        /// poisoning unrelated systems three frames later.
        [[nodiscard]] Vec2 Normalized() const noexcept
        {
            const float len = Length();
            if (len <= DefaultEpsilon)
            {
                return Vec2(0.0f, 0.0f);
            }
            return Vec2(x / len, y / len);
        }
    };

    [[nodiscard]] constexpr Vec2 operator+(Vec2 lhs, const Vec2& rhs) noexcept { lhs += rhs; return lhs; }
    [[nodiscard]] constexpr Vec2 operator-(Vec2 lhs, const Vec2& rhs) noexcept { lhs -= rhs; return lhs; }
    [[nodiscard]] constexpr Vec2 operator*(Vec2 lhs, float scalar) noexcept { lhs *= scalar; return lhs; }
    [[nodiscard]] constexpr Vec2 operator*(float scalar, Vec2 rhs) noexcept { rhs *= scalar; return rhs; }
    [[nodiscard]] constexpr Vec2 operator/(Vec2 lhs, float scalar) noexcept { lhs /= scalar; return lhs; }

    [[nodiscard]] constexpr float Dot(const Vec2& a, const Vec2& b) noexcept { return a.x * b.x + a.y * b.y; }
    [[nodiscard]] inline float Distance(const Vec2& a, const Vec2& b) noexcept { return (b - a).Length(); }

} // namespace Engine::Math
