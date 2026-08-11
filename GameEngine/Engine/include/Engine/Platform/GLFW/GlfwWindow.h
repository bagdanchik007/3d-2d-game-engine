#pragma once

#include "Engine/Core/Window.h"
#include "Engine/Renderer/GraphicsContext.h"

#include <memory>

struct GLFWwindow;

namespace Engine
{
    /// The engine's only Window implementation, wrapping a GLFWwindow*.
    /// See Window.h for why the interface exists even with one backend.
    class GlfwWindow final : public Window
    {
    public:
        explicit GlfwWindow(const WindowProps& props);
        ~GlfwWindow() override;

        GlfwWindow(const GlfwWindow&) = delete;
        GlfwWindow& operator=(const GlfwWindow&) = delete;

        void OnUpdate() override;

        [[nodiscard]] uint32_t GetWidth() const override { return m_Data.Width; }
        [[nodiscard]] uint32_t GetHeight() const override { return m_Data.Height; }

        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled) override;
        [[nodiscard]] bool IsVSync() const override { return m_Data.VSync; }

        [[nodiscard]] void* GetNativeWindow() const override { return m_Window; }

    private:
        void Init(const WindowProps& props);
        void Shutdown();

        GLFWwindow* m_Window = nullptr;
        std::unique_ptr<GraphicsContext> m_Context;

        /// Data block whose address is handed to GLFW via
        /// glfwSetWindowUserPointer, so the static GLFW callbacks below can
        /// recover engine state (and thus produce/deliver Engine::Event
        /// instances) from a bare GLFWwindow*, which is all GLFW's C API
        /// gives them.
        struct WindowData
        {
            std::string Title;
            uint32_t Width = 0;
            uint32_t Height = 0;
            bool VSync = true;
            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };

} // namespace Engine
