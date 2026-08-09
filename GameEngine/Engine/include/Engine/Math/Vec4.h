#pragma once

#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Vec3.h"

#include <cmath>

namespace Engine::Math
{
    struct Vec4
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

        constexpr Vec4() noexcept = default;
        constexpr explicit Vec4(float scalar) noexcept : x(scalar), y(scalar), z(scalar), w(scalar) {}
        constexpr Vec4(float x_, float y_, float z_, float w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}
        constexpr Vec4(const Vec3& xyz, float w_) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w_) {}

        constexpr Vec4& operator+=(const Vec4& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
        constexpr Vec4& operator-=(const Vec4& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
        constexpr Vec4& operator*=(float scalar) noexcept { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        constexpr Vec4& operator/=(float scalar) noexcept { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

        [[nodiscard]] constexpr Vec4 operator-() const noexcept { return Vec4(-x, -y, -z, -w); }

        [[nodiscard]] constexpr bool operator==(const Vec4& rhs) const noexcept
        {
            return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
        }
        [[nodiscard]] constexpr bool operator!=(const Vec4& rhs) const noexcept { return !(*this == rhs); }

        [[nodiscard]] float Length() const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }
        [[nodiscard]] constexpr float LengthSquared() const noexcept { return x * x + y * y + z * z + w * w; }

        [[nodiscard]] constexpr Vec3 XYZ() const noexcept { return Vec3(x, y, z); }

        /// Index-based access, needed by Mat4 (see Mat4.h) to treat a Vec4
        /// as a matrix column/row during multiplication and inversion.
        /// Deliberately not added to Vec2/Vec3, which have no such caller.
        [[nodiscard]] constexpr float operator[](int index) const noexcept
        {
            switch (index)
            {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default: return w;
            }
        }

        [[nodiscard]] constexpr float& operator[](int index) noexcept
        {
            switch (index)
            {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default: return w;
            }
        }
    };

    [[nodiscard]] constexpr Vec4 operator+(Vec4 lhs, const Vec4& rhs) noexcept { lhs += rhs; return lhs; }
    [[nodiscard]] constexpr Vec4 operator-(Vec4 lhs, const Vec4& rhs) noexcept { lhs -= rhs; return lhs; }
    [[nodiscard]] constexpr Vec4 operator*(Vec4 lhs, float scalar) noexcept { lhs *= scalar; return lhs; }
    [[nodiscard]] constexpr Vec4 operator*(float scalar, Vec4 rhs) noexcept { rhs *= scalar; return rhs; }
    [[nodiscard]] constexpr Vec4 operator/(Vec4 lhs, float scalar) noexcept { lhs /= scalar; return lhs; }

    [[nodiscard]] constexpr float Dot(const Vec4& a, const Vec4& b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

} // namespace Engine::Math
