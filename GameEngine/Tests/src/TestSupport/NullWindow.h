#pragma once

#include "Engine/Core/Window.h"

namespace Engine::Test
{
    /// A Window that does nothing: no real OS window, no GLFW, no display
    /// server required. Exists purely so ApplicationTests.cpp can construct
    /// an Application and exercise its event/layer logic in a headless CI
    /// environment. Lives under Tests/, not Engine/, because it is test
    /// scaffolding, not an engine feature - shipping it as part of the
    /// public engine API would misrepresent it as a supported headless
    /// backend, which it deliberately is not (it doesn't even track resize
    /// calls).
    class NullWindow final : public Window
    {
    public:
        explicit NullWindow(WindowProps props) noexcept
            : m_Width(props.Width), m_Height(props.Height)
        {
        }

        void OnUpdate() override {}

        [[nodiscard]] uint32_t GetWidth() const override { return m_Width; }
        [[nodiscard]] uint32_t GetHeight() const override { return m_Height; }

        void SetEventCallback(const EventCallbackFn& callback) override { m_EventCallback = callback; }
        void SetVSync(bool enabled) override { m_VSync = enabled; }
        [[nodiscard]] bool IsVSync() const override { return m_VSync; }

        [[nodiscard]] void* GetNativeWindow() const override { return nullptr; }

        [[nodiscard]] static std::unique_ptr<Window> Create(const WindowProps& props)
        {
            return std::make_unique<NullWindow>(props);
        }

    private:
        uint32_t m_Width;
        uint32_t m_Height;
        bool m_VSync = true;
        EventCallbackFn m_EventCallback;
    };

} // namespace Engine::Test
