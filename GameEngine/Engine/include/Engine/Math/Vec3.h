#pragma once

#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Vec2.h"

#include <cmath>

namespace Engine::Math
{
    struct Vec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        constexpr Vec3() noexcept = default;
        constexpr explicit Vec3(float scalar) noexcept : x(scalar), y(scalar), z(scalar) {}
        constexpr Vec3(float x_, float y_, float z_) noexcept : x(x_), y(y_), z(z_) {}
        constexpr Vec3(const Vec2& xy, float z_) noexcept : x(xy.x), y(xy.y), z(z_) {}

        constexpr Vec3& operator+=(const Vec3& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        constexpr Vec3& operator-=(const Vec3& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        constexpr Vec3& operator*=(float scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; return *this; }
        constexpr Vec3& operator/=(float scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; return *this; }

        [[nodiscard]] constexpr Vec3 operator-() const noexcept { return Vec3(-x, -y, -z); }

        [[nodiscard]] constexpr bool operator==(const Vec3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        [[nodiscard]] constexpr bool operator!=(const Vec3& rhs) const noexcept { return !(*this == rhs); }

        [[nodiscard]] float Length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
        [[nodiscard]] constexpr float LengthSquared() const noexcept { return x * x + y * y + z * z; }

        // See Vec2::Normalized for why a zero-length input maps to a
        // well-defined zero vector rather than propagating NaN.
        [[nodiscard]] Vec3 Normalized() const noexcept
        {
            const float len = Length();
            if (len <= DefaultEpsilon)
            {
                return Vec3(0.0f, 0.0f, 0.0f);
            }
            return Vec3(x / len, y / len, z / len);
        }

        [[nodiscard]] constexpr Vec2 XY() const noexcept { return Vec2(x, y); }
    };

    [[nodiscard]] constexpr Vec3 operator+(Vec3 lhs, const Vec3& rhs) noexcept { lhs += rhs; return lhs; }
    [[nodiscard]] constexpr Vec3 operator-(Vec3 lhs, const Vec3& rhs) noexcept { lhs -= rhs; return lhs; }
    [[nodiscard]] constexpr Vec3 operator*(Vec3 lhs, float scalar) noexcept { lhs *= scalar; return lhs; }
    [[nodiscard]] constexpr Vec3 operator*(float scalar, Vec3 rhs) noexcept { rhs *= scalar; return rhs; }
    [[nodiscard]] constexpr Vec3 operator/(Vec3 lhs, float scalar) noexcept { lhs /= scalar; return lhs; }

    [[nodiscard]] constexpr float Dot(const Vec3& a, const Vec3& b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    [[nodiscard]] constexpr Vec3 Cross(const Vec3& a, const Vec3& b) noexcept
    {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
    }

    [[nodiscard]] inline float Distance(const Vec3& a, const Vec3& b) noexcept { return (b - a).Length(); }

} // namespace Engine::Math
