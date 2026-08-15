#pragma once

#include "Engine/Math/Mat4.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Vec3.h"

#include <cmath>
#include <utility>

namespace Engine::Math
{
    /// Unit quaternion representing a rotation. Component order (x, y, z, w)
    /// - vector part first, scalar part last - matches Unity/Unreal
    /// convention rather than the (w, x, y, z) some math texts use; chosen
    /// so anyone with prior engine experience reads this without surprise.
    class Quaternion
    {
    public:
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 1.0f; // Default-constructed Quaternion IS the identity rotation,
                        // unlike Vec2/3/4 defaulting to zero - a zero quaternion
                        // isn't a valid rotation at all, so there is no
                        // "additive identity" convention worth preserving here;
                        // the only sensible default is the rotation that does
                        // nothing.

        constexpr Quaternion() noexcept = default;
        constexpr Quaternion(float x_, float y_, float z_, float w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

        [[nodiscard]] static Quaternion FromAxisAngle(const Vec3& axis, float angleRadians) noexcept
        {
            const Vec3 normalizedAxis = axis.Normalized();
            const float halfAngle = angleRadians * 0.5f;
            const float s = std::sin(halfAngle);
            return Quaternion(normalizedAxis.x * s, normalizedAxis.y * s, normalizedAxis.z * s, std::cos(halfAngle));
        }

        /// Inverse of FromAxisAngle. Returns {(1,0,0), 0} for a
        /// near-identity quaternion (angle close to 0) rather than
        /// dividing by the near-zero sin(halfAngle) term that would
        /// otherwise appear in the denominator of the axis calculation -
        /// same zero-length fallback philosophy as Vec3::Normalized: a
        /// well-defined, arbitrary answer beats propagating NaN through
        /// whatever reads this next (InspectorPanel's rotation widget, in
        /// this engine's only current caller).
        [[nodiscard]] std::pair<Vec3, float> ToAxisAngle() const noexcept
        {
            const float clampedW = Clamp(w, -1.0f, 1.0f);
            const float angle = 2.0f * std::acos(clampedW);

            const float s = std::sqrt(1.0f - clampedW * clampedW);
            if (s <= DefaultEpsilon)
            {
                return {Vec3(1.0f, 0.0f, 0.0f), 0.0f};
            }

            return {Vec3(x / s, y / s, z / s), angle};
        }

        [[nodiscard]] constexpr float LengthSquared() const noexcept { return x * x + y * y + z * z + w * w; }
        [[nodiscard]] float Length() const noexcept { return std::sqrt(LengthSquared()); }

        // See Vec2::Normalized for the zero-length fallback rationale. Here
        // the fallback is Identity() rather than a zero quaternion, for the
        // same reason the default constructor isn't zero: a zero quaternion
        // is never a meaningful rotation to hand back to a caller.
        [[nodiscard]] Quaternion Normalized() const noexcept
        {
            const float len = Length();
            if (len <= DefaultEpsilon)
            {
                return Quaternion();
            }
            return Quaternion(x / len, y / len, z / len, w / len);
        }

        [[nodiscard]] constexpr Quaternion Conjugate() const noexcept { return Quaternion(-x, -y, -z, w); }

        /// Inverse equals the conjugate only for unit quaternions. Dividing
        /// by LengthSquared() (rather than assuming unit length) keeps this
        /// correct even if a caller passes in a not-quite-normalized
        /// quaternion, at the cost of a few extra flops nobody will notice
        /// outside a profiler for something called this rarely.
        [[nodiscard]] Quaternion Inverse() const noexcept
        {
            const float lenSq = LengthSquared();
            if (lenSq <= DefaultEpsilon)
            {
                return Quaternion();
            }
            const Quaternion conjugate = Conjugate();
            return Quaternion(conjugate.x / lenSq, conjugate.y / lenSq, conjugate.z / lenSq, conjugate.w / lenSq);
        }

        [[nodiscard]] Vec3 RotateVector(const Vec3& v) const noexcept
        {
            // Optimized form of q * (v, 0) * q^-1 that avoids constructing
            // two intermediate quaternions and multiplying them out in
            // full (each a 16-multiply operation); this closed form does
            // the equivalent work in far fewer operations. Correctness is
            // verified in tests against the full q*v*q^-1 multiplication
            // for the same inputs, not just against hand-picked cases.
            const Vec3 qVec(x, y, z);
            const Vec3 t = 2.0f * Cross(qVec, v);
            return v + w * t + Cross(qVec, t);
        }

        [[nodiscard]] Mat4 ToMat4() const noexcept
        {
            const float xx = x * x, yy = y * y, zz = z * z;
            const float xy = x * y, xz = x * z, yz = y * z;
            const float wx = w * x, wy = w * y, wz = w * z;

            return Mat4({
                Vec4(1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f),
                Vec4(2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f),
                Vec4(2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f),
                Vec4(0.0f, 0.0f, 0.0f, 1.0f),
            });
        }
    };

    /// Composition: applying (a * b) to a vector applies b's rotation
    /// first, then a's - the same right-to-left convention Mat4
    /// multiplication uses, kept consistent across the math library so a
    /// caller who has internalized one doesn't get the other backwards.
    [[nodiscard]] constexpr Quaternion operator*(const Quaternion& a, const Quaternion& b) noexcept
    {
        return Quaternion(
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);
    }

    [[nodiscard]] constexpr float Dot(const Quaternion& a, const Quaternion& b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    /// Spherical linear interpolation. Falls back to linear interpolation
    /// (then renormalizes) when the two quaternions are nearly parallel,
    /// where sin(theta) in the slerp formula approaches zero and would
    /// otherwise divide by ~0 - a standard, necessary guard, not an edge
    /// case that "shouldn't happen": two nearly-identical rotations are a
    /// completely ordinary input (e.g. blending between adjacent animation
    /// keyframes).
    [[nodiscard]] inline Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t) noexcept
    {
        float cosHalfTheta = Dot(a, b);

        // If the dot product is negative, the two quaternions represent
        // rotations more than 180 degrees apart in 4D space; negating one
        // takes the shorter path, which is the rotation an artist or
        // gameplay programmer actually expects from a blend.
        Quaternion end = b;
        if (cosHalfTheta < 0.0f)
        {
            end = Quaternion(-b.x, -b.y, -b.z, -b.w);
            cosHalfTheta = -cosHalfTheta;
        }

        constexpr float kParallelThreshold = 0.9995f;
        if (cosHalfTheta > kParallelThreshold)
        {
            return Quaternion(
                Lerp(a.x, end.x, t),
                Lerp(a.y, end.y, t),
                Lerp(a.z, end.z, t),
                Lerp(a.w, end.w, t))
                .Normalized();
        }

        const float halfTheta = std::acos(cosHalfTheta);
        const float sinHalfTheta = std::sqrt(1.0f - cosHalfTheta * cosHalfTheta);

        const float ratioA = std::sin((1.0f - t) * halfTheta) / sinHalfTheta;
        const float ratioB = std::sin(t * halfTheta) / sinHalfTheta;

        return Quaternion(
            a.x * ratioA + end.x * ratioB,
            a.y * ratioA + end.y * ratioB,
            a.z * ratioA + end.z * ratioB,
            a.w * ratioA + end.w * ratioB);
    }

} // namespace Engine::Math
