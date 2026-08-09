#pragma once

#include "Engine/Events/Event.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace Engine
{
    struct WindowProps
    {
        std::string Title = "Engine Application";
        uint32_t Width = 1280;
        uint32_t Height = 720;
    };

    /// Platform-agnostic window interface.
    ///
    /// Exactly one concrete implementation exists today (GlfwWindow), so
    /// one might ask why this interface exists at all rather than just
    /// using GlfwWindow directly. The answer: Application and every future
    /// system that touches a window (the M8 renderer's swapchain setup, a
    /// possible headless test backend) should depend on this interface, not
    /// on GLFW specifically. That is a deliberate, currently-unexercised
    /// seam — cheap to keep now, expensive to retrofit later once dozens of
    /// call sites assume a concrete GLFW type.
    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        [[nodiscard]] virtual uint32_t GetWidth() const = 0;
        [[nodiscard]] virtual uint32_t GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        [[nodiscard]] virtual bool IsVSync() const = 0;

        [[nodiscard]] virtual void* GetNativeWindow() const = 0;

        [[nodiscard]] static std::unique_ptr<Window> Create(const WindowProps& props = WindowProps());
    };

} // namespace Engine
