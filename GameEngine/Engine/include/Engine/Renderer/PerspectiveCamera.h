#pragma once

#include "Engine/Math/Math.h"

#include <cmath>

namespace Engine
{
    /// A free-look 3D camera: position plus yaw/pitch (no roll - a flycam
    /// never needs it, and adding it would just be an unused axis every
    /// caller has to reason about).
    ///
    /// Orientation is stored as yaw/pitch angles, not a Quaternion, even
    /// though Quaternion exists (M4) and this class computes one
    /// internally to build the view matrix. Euler yaw/pitch is what a
    /// mouse-look controller naturally produces (horizontal delta -> yaw,
    /// vertical delta -> pitch) and is trivially clampable (pitch clamped
    /// to +-89 degrees below, preventing gimbal-flip at the poles) in a way
    /// a raw quaternion is not - clamping "how far this camera has looked
    /// up" has no simple quaternion-space equivalent. The Quaternion is
    /// reconstructed from (yaw, pitch) each time the view matrix is
    /// rebuilt, which is at most once per frame, not a hot per-object path.
    class PerspectiveCamera
    {
    public:
        PerspectiveCamera(float fovYRadians, float aspectRatio, float nearClip, float farClip) noexcept
            : m_ProjectionMatrix(Math::Mat4::Perspective(fovYRadians, aspectRatio, nearClip, farClip))
            , m_FovY(fovYRadians), m_NearClip(nearClip), m_FarClip(farClip)
        {
            RecalculateViewMatrix();
        }

        void SetPerspective(float aspectRatio) noexcept
        {
            m_ProjectionMatrix = Math::Mat4::Perspective(m_FovY, aspectRatio, m_NearClip, m_FarClip);
        }

        [[nodiscard]] const Math::Vec3& GetPosition() const noexcept { return m_Position; }
        void SetPosition(const Math::Vec3& position) noexcept { m_Position = position; RecalculateViewMatrix(); }

        [[nodiscard]] float GetYaw() const noexcept { return m_YawRadians; }
        [[nodiscard]] float GetPitch() const noexcept { return m_PitchRadians; }

        void SetYawPitch(float yawRadians, float pitchRadians) noexcept
        {
            m_YawRadians = yawRadians;

            // +-89 degrees, not +-90: at exactly 90 degrees, forward and
            // world-up become parallel and LookAt's Cross(forward, up)
            // degenerates to a zero vector, producing a NaN camera basis
            // (see Mat4::LookAt) - staying one degree short avoids landing
            // on that singularity at all rather than special-casing it.
            constexpr float kMaxPitch = Math::Radians(89.0f);
            m_PitchRadians = Math::Clamp(pitchRadians, -kMaxPitch, kMaxPitch);

            RecalculateViewMatrix();
        }

        [[nodiscard]] Math::Vec3 GetForward() const noexcept
        {
            // Standard yaw/pitch-to-direction conversion: yaw rotates
            // around world Y (turning left/right), pitch tilts up/down
            // afterward. This is the one piece of camera behavior that
            // isn't "just call an existing Math function", so it's worth
            // spelling out here rather than hiding it behind a Quaternion
            // that would need the exact same derivation internally.
            return Math::Vec3(
                std::cos(m_PitchRadians) * std::sin(m_YawRadians),
                std::sin(m_PitchRadians),
                -std::cos(m_PitchRadians) * std::cos(m_YawRadians))
                .Normalized();
        }

        [[nodiscard]] const Math::Mat4& GetProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
        [[nodiscard]] const Math::Mat4& GetViewMatrix() const noexcept { return m_ViewMatrix; }
        [[nodiscard]] const Math::Mat4& GetViewProjectionMatrix() const noexcept { return m_ViewProjectionMatrix; }

    private:
        void RecalculateViewMatrix() noexcept
        {
            const Math::Vec3 forward = GetForward();
            m_ViewMatrix = Math::Mat4::LookAt(m_Position, m_Position + forward, Math::Vec3(0.0f, 1.0f, 0.0f));
            m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        }

        Math::Mat4 m_ProjectionMatrix;
        Math::Mat4 m_ViewMatrix;
        Math::Mat4 m_ViewProjectionMatrix;

        Math::Vec3 m_Position{0.0f, 0.0f, 3.0f};
        float m_YawRadians = Math::Radians(-90.0f); // facing -Z, matching OpenGL's default camera-look convention (see Mat4::Perspective)
        float m_PitchRadians = 0.0f;

        float m_FovY;
        float m_NearClip;
        float m_FarClip;
    };

} // namespace Engine
