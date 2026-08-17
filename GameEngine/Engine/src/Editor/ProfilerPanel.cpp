#include "Engine/Editor/ProfilerPanel.h"

#include "Engine/Core/Profiler.h"

#include <imgui.h>

#include <algorithm>

namespace Engine::Editor
{
    void ProfilerPanel::OnImGuiRender()
    {
        ImGui::Begin("Profiler");

        const float frameTimeMs = Profiler::GetLastFrameTimeMs();

        // Ring buffer write, not push_back+pop_front: fixed-size
        // std::array with a wrapping index is allocation-free, which
        // matters here specifically because this runs every single frame
        // for the lifetime of the editor session - the one place in this
        // engine's UI code where "called once per frame, forever" makes
        // an allocation-per-call pattern worth avoiding on principle, not
        // just as a hypothetical optimization.
        m_FrameTimeHistoryMs[m_HistoryWriteIndex] = frameTimeMs;
        m_HistoryWriteIndex = (m_HistoryWriteIndex + 1) % kHistorySize;
        if (m_HistoryWriteIndex == 0)
        {
            m_HistoryFilled = true;
        }

        const float fps = (frameTimeMs > 0.0f) ? (1000.0f / frameTimeMs) : 0.0f;
        ImGui::Text("Frame time: %.3f ms (%.1f FPS)", static_cast<double>(frameTimeMs), static_cast<double>(fps));

        const std::size_t sampleCount = m_HistoryFilled ? kHistorySize : m_HistoryWriteIndex;
        if (sampleCount > 0)
        {
            // PlotLines reads sequentially from `values_offset` for
            // `values_count` entries, wrapping via its own internal
            // modulo - passing m_HistoryWriteIndex as values_offset means
            // the graph always reads oldest-to-newest regardless of where
            // in the ring buffer the next write will land, without this
            // panel needing to physically reorder the array itself.
            const float maxScale = *std::max_element(m_FrameTimeHistoryMs.begin(), m_FrameTimeHistoryMs.begin() + static_cast<std::ptrdiff_t>(sampleCount));
            ImGui::PlotLines(
                "Frame Time (ms)",
                m_FrameTimeHistoryMs.data(),
                static_cast<int>(sampleCount),
                static_cast<int>(m_HistoryFilled ? m_HistoryWriteIndex : 0),
                nullptr,
                0.0f,
                std::max(maxScale, 1.0f),
                ImVec2(0.0f, 80.0f));
        }

        ImGui::Separator();
        ImGui::Text("Scoped timings (this frame):");

        if (ImGui::BeginTable("ProfilerResults", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Duration (ms)");
            ImGui::TableHeadersRow();

            for (const ProfileResult& result : Profiler::GetResults())
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(result.Name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.4f", static_cast<double>(result.DurationMs));
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }

} // namespace Engine::Editor
