#pragma once

#include "Engine/Math/Vec2.h"
#include "Engine/Renderer/Framebuffer.h"

#include <memory>

namespace Engine::Editor
{
    /// Displays a Framebuffer's color attachment inside an ImGui window
    /// and reports focus/hover/size back to the caller.
    ///
    /// IsFocused/IsHovered exist so a camera controller can gate input
    /// consumption on them (see EditorLayer, Sandbox) - without this, a
    /// WASD flycam would move the camera even while the user is typing an
    /// entity name into InspectorPanel's text field, which happens to
    /// contain the letter 'w'.
    class ViewportPanel
    {
    public:
        void SetFramebuffer(std::shared_ptr<Framebuffer> framebuffer) noexcept { m_Framebuffer = std::move(framebuffer); }

        void OnImGuiRender();

        [[nodiscard]] bool IsFocused() const noexcept { return m_Focused; }
        [[nodiscard]] bool IsHovered() const noexcept { return m_Hovered; }
        [[nodiscard]] Math::Vec2 GetSize() const noexcept { return m_Size; }

        /// True exactly once, the frame after ImGui reports a content
        /// region size different from the last one recorded - the signal
        /// EditorLayer uses to call Framebuffer::Resize at most once per
        /// actual size change, not every frame the viewport happens to be
        /// open (which would mean recreating GPU attachments 60 times a
        /// second for a perfectly static window).
        [[nodiscard]] bool SizeChanged() const noexcept { return m_SizeChangedThisFrame; }

    private:
        std::shared_ptr<Framebuffer> m_Framebuffer;
        Math::Vec2 m_Size{0.0f, 0.0f};
        bool m_Focused = false;
        bool m_Hovered = false;
        bool m_SizeChangedThisFrame = false;
    };

} // namespace Engine::Editor
