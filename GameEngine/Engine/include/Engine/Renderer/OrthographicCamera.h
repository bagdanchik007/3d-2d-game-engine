#pragma once

#include "Engine/Math/Math.h"

namespace Engine
{
    /// A 2D camera: position + rotation (around Z only - 2D scenes never
    /// need pitch/yaw) and an orthographic projection.
    ///
    /// Deliberately data-only: no input handling, no movement speed, no
    /// "controller" concept. That is M8's job once a general Camera
    /// abstraction exists alongside perspective projection - this class
    /// exists now purely because Renderer2D (below) needs *some*
    /// view-projection matrix to be meaningful at all, not because camera
    /// movement is in scope for M7.
    class OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top) noexcept
            : m_ProjectionMatrix(Math::Mat4::Orthographic(left, right, bottom, top, -1.0f, 1.0f))
        {
            RecalculateViewMatrix();
        }

        void SetProjection(float left, float right, float bottom, float top) noexcept
        {
            m_ProjectionMatrix = Math::Mat4::Orthographic(left, right, bottom, top, -1.0f, 1.0f);
            m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        }

        [[nodiscard]] const Math::Vec3& GetPosition() const noexcept { return m_Position; }
        void SetPosition(const Math::Vec3& position) noexcept
        {
            m_Position = position;
            RecalculateViewMatrix();
        }

        [[nodiscard]] float GetRotation() const noexcept { return m_RotationRadians; }
        void SetRotation(float rotationRadians) noexcept
        {
            m_RotationRadians = rotationRadians;
            RecalculateViewMatrix();
        }

        [[nodiscard]] const Math::Mat4& GetProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
        [[nodiscard]] const Math::Mat4& GetViewMatrix() const noexcept { return m_ViewMatrix; }
        [[nodiscard]] const Math::Mat4& GetViewProjectionMatrix() const noexcept { return m_ViewProjectionMatrix; }

    private:
        void RecalculateViewMatrix() noexcept
        {
            const Math::Mat4 transform = Math::Mat4::Translate(m_Position) * Math::Mat4::RotationZ(m_RotationRadians);

            // The view matrix is the INVERSE of the camera's world
            // transform: moving the camera right is equivalent to moving
            // the entire world left. Mat4::Inverse() is general-purpose
            // (M4) and correct here, but for a pure translate+Z-rotation
            // matrix specifically, inverse = transpose-of-rotation-part
            // plus negated-translation - a cheaper closed form this class
            // doesn't bother with because camera transforms recompute at
            // most once per SetPosition/SetRotation call, never per-object,
            // per-frame in a hot loop.
            m_ViewMatrix = transform.Inverse();
            m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        }

        Math::Mat4 m_ProjectionMatrix;
        Math::Mat4 m_ViewMatrix;
        Math::Mat4 m_ViewProjectionMatrix;

        Math::Vec3 m_Position{0.0f, 0.0f, 0.0f};
        float m_RotationRadians = 0.0f;
    };

} // namespace Engine
