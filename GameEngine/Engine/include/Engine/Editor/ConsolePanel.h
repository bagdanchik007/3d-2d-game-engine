#pragma once

namespace Engine::Editor
{
    /// Displays the log entries captured by Engine::ConsoleSink (see
    /// Core/ConsoleSink.h) - this panel has no logging logic of its own,
    /// it only reads and renders what the sink already collected.
    class ConsolePanel
    {
    public:
        void OnImGuiRender();

    private:
        bool m_ShowTrace = false; // trace-level messages are usually too noisy to show by default
        bool m_ShowInfo = true;
        bool m_ShowWarn = true;
        bool m_ShowError = true;
        bool m_AutoScroll = true;
    };

} // namespace Engine::Editor
