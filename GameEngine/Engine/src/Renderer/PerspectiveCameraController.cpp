#include "Engine/Renderer/PerspectiveCameraController.h"

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace Engine
{
    PerspectiveCameraController::PerspectiveCameraController(float aspectRatio) noexcept
        : m_Camera(Math::Radians(45.0f), aspectRatio, 0.1f, 100.0f)
    {
    }

    void PerspectiveCameraController::OnUpdate(Timestep timestep)
    {
        const float distance = m_MovementSpeed * timestep.GetSeconds();

        const Math::Vec3 forward = m_Camera.GetForward();
        // Right = forward x worldUp, not the other way around: Cross is
        // anti-commutative (M4's QuaternionTests/VecTests both verify
        // this), so getting the operand order backwards here would
        // silently mirror every strafe direction - exactly the kind of
        // bug that "looks right" until you actually press the key.
        const Math::Vec3 right = Math::Cross(forward, Math::Vec3(0.0f, 1.0f, 0.0f)).Normalized();

        Math::Vec3 position = m_Camera.GetPosition();
        if (Input::IsKeyPressed(KeyCode::W)) { position += forward * distance; }
        if (Input::IsKeyPressed(KeyCode::S)) { position -= forward * distance; }
        if (Input::IsKeyPressed(KeyCode::A)) { position -= right * distance; }
        if (Input::IsKeyPressed(KeyCode::D)) { position += right * distance; }
        m_Camera.SetPosition(position);
    }

    void PerspectiveCameraController::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
        dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) { return OnMouseMoved(e); });
    }

    bool PerspectiveCameraController::OnWindowResize(WindowResizeEvent& event)
    {
        if (event.GetHeight() == 0)
        {
            return false; // minimized - an aspect ratio of width/0 is undefined; wait for a real resize instead
        }

        const float aspectRatio = static_cast<float>(event.GetWidth()) / static_cast<float>(event.GetHeight());
        m_Camera.SetPerspective(aspectRatio);
        return false; // Not "handled": other layers (e.g. a viewport-aware editor UI) still need to see resizes too.
    }

    bool PerspectiveCameraController::OnMouseMoved(MouseMovedEvent& event)
    {
        if (!m_LookEnabled)
        {
            return false;
        }

        if (!m_HasLastMousePosition)
        {
            // First move since look was enabled: record the position but
            // apply no delta yet. Without this, re-enabling look after it
            // was off would compute a delta against a stale mouse
            // position from before, snapping the camera instantly instead
            // of starting smoothly from wherever the mouse currently is.
            m_LastMouseX = event.GetX();
            m_LastMouseY = event.GetY();
            m_HasLastMousePosition = true;
            return false;
        }

        const float deltaX = event.GetX() - m_LastMouseX;
        const float deltaY = event.GetY() - m_LastMouseY;
        m_LastMouseX = event.GetX();
        m_LastMouseY = event.GetY();

        // Yaw += deltaX (mouse right -> look right); pitch -= deltaY
        // (mouse DOWN increases screen-space Y, but should look DOWN,
        // i.e. decrease pitch) - the sign flip on pitch is not a typo.
        m_Camera.SetYawPitch(
            m_Camera.GetYaw() + deltaX * m_MouseSensitivity,
            m_Camera.GetPitch() - deltaY * m_MouseSensitivity);

        return false;
    }

} // namespace Engine
