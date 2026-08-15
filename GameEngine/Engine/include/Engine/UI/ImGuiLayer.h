#pragma once

#include "Engine/Core/Layer.h"

namespace Engine::UI
{
    /// Owns the ImGui context and the GLFW+OpenGL3 backend integration.
    ///
    /// The frame-boundary calls (ImGui::NewFrame/Render) live in
    /// OnImGuiFrameBegin/OnImGuiFrameEnd, not OnAttach/OnUpdate - see the
    /// Layer.h doc comments on those hooks, and Application::SetImGuiLayer,
    /// for why the frame needs two distinct hook points rather than one.
    class ImGuiLayer final : public Layer
    {
    public:
        ImGuiLayer() : Layer("ImGuiLayer") {}

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;

        void OnImGuiFrameBegin() override;
        void OnImGuiFrameEnd() override;

        /// When true (the default), a mouse/keyboard event that ImGui
        /// itself wants (typing into a text field, dragging a slider) is
        /// marked Handled here and never reaches game layers underneath -
        /// without this, moving the mouse over an ImGui panel would also
        /// move an editor camera sitting in a layer below it, which is
        /// the single most common and most annoying integration bug in
        /// any engine that bolts ImGui onto an existing input pipeline.
        void SetBlockEvents(bool block) noexcept { m_BlockEvents = block; }

    private:
        bool m_BlockEvents = true;
    };

} // namespace Engine::UI
