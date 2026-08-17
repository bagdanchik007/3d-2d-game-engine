#pragma once

#include <array>
#include <cstddef>

namespace Engine::Editor
{
    /// Displays a rolling frame-time graph plus a table of the current
    /// frame's named ENGINE_PROFILE_SCOPE results (see Core/Profiler.h).
    class ProfilerPanel
    {
    public:
        void OnImGuiRender();

    private:
        static constexpr std::size_t kHistorySize = 120; // ~2 seconds at 60 FPS - enough to see a recent trend, not a full session history

        std::array<float, kHistorySize> m_FrameTimeHistoryMs{};
        std::size_t m_HistoryWriteIndex = 0;
        bool m_HistoryFilled = false;
    };

} // namespace Engine::Editor
