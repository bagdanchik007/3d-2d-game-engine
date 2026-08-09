#pragma once

#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Vec3.h"
#include "Engine/Math/Vec4.h"

#include <array>
#include <cmath>

namespace Engine::Math
{
    /// Column-major 4x4 matrix, storing four Vec4 columns. Column-major so
    /// GetData() can be handed directly to glUniformMatrix4fv (M6+) without
    /// a transpose - the whole reason this convention was chosen over the
    /// (arguably more readable) row-major layout.
    ///
    /// Convention: column-vector, so a point is transformed as `M * v`, and
    /// composing transforms is `parent * child` (right-to-left application,
    /// matching OpenGL/GLM, not DirectX's row-vector convention).
    class Mat4
    {
    public:
        /// Zero matrix, not identity - matches the "default-constructed
        /// numeric type is its additive identity" convention Vec2/3/4
        /// already follow. Use Mat4::Identity() explicitly when you want
        /// the multiplicative identity; leaving that implicit in a default
        /// constructor is exactly the kind of silent surprise that causes
        /// a transform bug three systems away from where it was written.
        constexpr Mat4() noexcept = default;

        constexpr explicit Mat4(const std::array<Vec4, 4>& columns) noexcept
            : m_Columns(columns)
        {
        }

        [[nodiscard]] static constexpr Mat4 Identity() noexcept
        {
            return Mat4({Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0), Vec4(0, 0, 1, 0), Vec4(0, 0, 0, 1)});
        }

        [[nodiscard]] static constexpr Mat4 Translate(const Vec3& t) noexcept
        {
            Mat4 result = Identity();
            result.m_Columns[3] = Vec4(t, 1.0f);
            return result;
        }

        [[nodiscard]] static constexpr Mat4 Scale(const Vec3& s) noexcept
        {
            return Mat4({Vec4(s.x, 0, 0, 0), Vec4(0, s.y, 0, 0), Vec4(0, 0, s.z, 0), Vec4(0, 0, 0, 1)});
        }

        [[nodiscard]] static Mat4 RotationX(float radians) noexcept
        {
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            return Mat4({Vec4(1, 0, 0, 0), Vec4(0, c, s, 0), Vec4(0, -s, c, 0), Vec4(0, 0, 0, 1)});
        }

        [[nodiscard]] static Mat4 RotationY(float radians) noexcept
        {
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            return Mat4({Vec4(c, 0, -s, 0), Vec4(0, 1, 0, 0), Vec4(s, 0, c, 0), Vec4(0, 0, 0, 1)});
        }

        [[nodiscard]] static Mat4 RotationZ(float radians) noexcept
        {
            const float c = std::cos(radians);
            const float s = std::sin(radians);
            return Mat4({Vec4(c, s, 0, 0), Vec4(-s, c, 0, 0), Vec4(0, 0, 1, 0), Vec4(0, 0, 0, 1)});
        }

        /// Right-handed perspective projection with NDC depth in [-1, 1],
        /// matching OpenGL's clip-space convention used by the M6 renderer.
        [[nodiscard]] static Mat4 Perspective(float fovYRadians, float aspect, float nearPlane, float farPlane) noexcept
        {
            const float tanHalfFovY = std::tan(fovYRadians * 0.5f);

            Mat4 result; // zero matrix
            result.m_Columns[0][0] = 1.0f / (aspect * tanHalfFovY);
            result.m_Columns[1][1] = 1.0f / tanHalfFovY;
            result.m_Columns[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
            result.m_Columns[2][3] = -1.0f;
            result.m_Columns[3][2] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
            return result;
        }

        [[nodiscard]] static constexpr Mat4 Orthographic(
            float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept
        {
            Mat4 result = Identity();
            result.m_Columns[0][0] = 2.0f / (right - left);
            result.m_Columns[1][1] = 2.0f / (top - bottom);
            result.m_Columns[2][2] = -2.0f / (farPlane - nearPlane);
            result.m_Columns[3][0] = -(right + left) / (right - left);
            result.m_Columns[3][1] = -(top + bottom) / (top - bottom);
            result.m_Columns[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
            return result;
        }

        [[nodiscard]] static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) noexcept
        {
            const Vec3 forward = (center - eye).Normalized();
            const Vec3 right = Cross(forward, up).Normalized();
            const Vec3 newUp = Cross(right, forward);

            // Rows here (not columns) because LookAt's rotation part is the
            // *inverse* of a basis-construction matrix, and for an
            // orthonormal basis the inverse is the transpose - writing the
            // basis vectors as rows constructs that transpose directly
            // instead of building the basis matrix and calling Transpose().
            Mat4 result = Identity();
            result.m_Columns[0][0] = right.x;   result.m_Columns[1][0] = right.y;   result.m_Columns[2][0] = right.z;
            result.m_Columns[0][1] = newUp.x;   result.m_Columns[1][1] = newUp.y;   result.m_Columns[2][1] = newUp.z;
            result.m_Columns[0][2] = -forward.x; result.m_Columns[1][2] = -forward.y; result.m_Columns[2][2] = -forward.z;
            result.m_Columns[3][0] = -Dot(right, eye);
            result.m_Columns[3][1] = -Dot(newUp, eye);
            result.m_Columns[3][2] = Dot(forward, eye);
            return result;
        }

        [[nodiscard]] constexpr Vec4& operator[](int col) noexcept { return m_Columns[static_cast<std::size_t>(col)]; }
        [[nodiscard]] constexpr const Vec4& operator[](int col) const noexcept { return m_Columns[static_cast<std::size_t>(col)]; }

        constexpr Mat4& operator*=(const Mat4& rhs) noexcept;

        [[nodiscard]] constexpr Mat4 Transpose() const noexcept
        {
            Mat4 result;
            for (int col = 0; col < 4; ++col)
            {
                for (int row = 0; row < 4; ++row)
                {
                    result.m_Columns[static_cast<std::size_t>(row)][col] = m_Columns[static_cast<std::size_t>(col)][row];
                }
            }
            return result;
        }

        /// General 4x4 inverse via cofactor expansion. Chosen for clarity
        /// and because it's the version whose correctness is easiest to
        /// verify against known cases (see Mat4Tests.cpp) - a faster
        /// block-decomposition inverse would be the right call if this
        /// were ever profiled as a bottleneck (it currently is not: matrix
        /// inversion is not a per-frame, per-object operation anywhere in
        /// this engine yet, only a setup-time one for cameras/transforms).
        [[nodiscard]] Mat4 Inverse() const noexcept
        {
            const auto& m = m_Columns;

            // 2x2 sub-determinants of the bottom two rows, reused across
            // multiple cofactors below - computing them once instead of
            // per-cofactor is the one performance concession made here,
            // since it costs nothing in clarity.
            const float s0 = m[0][0] * m[1][1] - m[1][0] * m[0][1];
            const float s1 = m[0][0] * m[1][2] - m[1][0] * m[0][2];
            const float s2 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
            const float s3 = m[0][1] * m[1][2] - m[1][1] * m[0][2];
            const float s4 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
            const float s5 = m[0][2] * m[1][3] - m[1][2] * m[0][3];

            const float c5 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
            const float c4 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
            const float c3 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
            const float c2 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
            const float c1 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
            const float c0 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

            const float determinant = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

            if (std::fabs(determinant) <= DefaultEpsilon)
            {
                // Singular matrix: no inverse exists. Returning Identity()
                // is a deliberate, documented fallback (same philosophy as
                // Vec::Normalized() on a zero vector) rather than dividing
                // by ~0 and returning a matrix full of near-infinities that
                // silently corrupts every transform downstream.
                return Identity();
            }

            const float invDet = 1.0f / determinant;

            Mat4 result;
            result.m_Columns[0][0] = (m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) * invDet;
            result.m_Columns[0][1] = (-m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3) * invDet;
            result.m_Columns[0][2] = (m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) * invDet;
            result.m_Columns[0][3] = (-m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3) * invDet;

            result.m_Columns[1][0] = (-m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1) * invDet;
            result.m_Columns[1][1] = (m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) * invDet;
            result.m_Columns[1][2] = (-m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1) * invDet;
            result.m_Columns[1][3] = (m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) * invDet;

            result.m_Columns[2][0] = (m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) * invDet;
            result.m_Columns[2][1] = (-m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0) * invDet;
            result.m_Columns[2][2] = (m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) * invDet;
            result.m_Columns[2][3] = (-m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0) * invDet;

            result.m_Columns[3][0] = (-m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0) * invDet;
            result.m_Columns[3][1] = (m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) * invDet;
            result.m_Columns[3][2] = (-m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0) * invDet;
            result.m_Columns[3][3] = (m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) * invDet;

            return result;
        }

        [[nodiscard]] const float* GetData() const noexcept { return &m_Columns[0].x; }

    private:
        std::array<Vec4, 4> m_Columns{}; // zero-initialized: see the note on the default constructor above.
    };

    [[nodiscard]] constexpr Vec4 operator*(const Mat4& m, const Vec4& v) noexcept
    {
        return m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
    }

    [[nodiscard]] constexpr Mat4 operator*(const Mat4& a, const Mat4& b) noexcept
    {
        Mat4 result;
        for (int col = 0; col < 4; ++col)
        {
            result[col] = a * b[col];
        }
        return result;
    }

    constexpr Mat4& Mat4::operator*=(const Mat4& rhs) noexcept
    {
        *this = *this * rhs;
        return *this;
    }

} // namespace Engine::Math
