#include "Engine/Editor/ViewportPanel.h"

#include <imgui.h>

#include <cstdint>

namespace Engine::Editor
{
    void ViewportPanel::OnImGuiRender()
    {
        m_SizeChangedThisFrame = false;

        // No window padding: the framebuffer image should fill the panel
        // edge-to-edge, matching every real editor's viewport - the
        // default padding would otherwise leave a visible border of the
        // window's background color around the rendered scene.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        m_Focused = ImGui::IsWindowFocused();
        m_Hovered = ImGui::IsWindowHovered();

        const ImVec2 panelSize = ImGui::GetContentRegionAvail();
        const Math::Vec2 newSize(panelSize.x, panelSize.y);

        constexpr float kMinDimension = 1.0f; // never report/request a 0-sized framebuffer - see OpenGLFramebuffer::Resize's own guard against exactly this
        if (newSize.x >= kMinDimension && newSize.y >= kMinDimension &&
            (newSize.x != m_Size.x || newSize.y != m_Size.y))
        {
            m_Size = newSize;
            m_SizeChangedThisFrame = true;
        }

        if (m_Framebuffer != nullptr)
        {
            // ImGui's texture coordinate origin is top-left; this engine's
            // (OpenGL) framebuffer texture origin is bottom-left - the
            // same flip FramebufferBlit.h (M8) already documents for the
            // non-editor blit path. UV0=(0,1)/UV1=(1,0) is the correction.
            const auto textureID = static_cast<ImTextureID>( // NOLINT(performance-no-int-to-ptr)
                reinterpret_cast<void*>(static_cast<uintptr_t>(m_Framebuffer->GetColorAttachmentRendererID())));
            ImGui::Image(textureID, ImVec2(m_Size.x, m_Size.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        }

        ImGui::End();
    }

} // namespace Engine::Editor
