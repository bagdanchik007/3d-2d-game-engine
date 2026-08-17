#include "Engine/Editor/ConsolePanel.h"

#include "Engine/Core/ConsoleSink.h"
#include "Engine/Core/Log.h"

#include <imgui.h>

namespace Engine::Editor
{
    namespace
    {
        ImVec4 ColorForLevel(spdlog::level::level_enum level)
        {
            switch (level)
            {
                case spdlog::level::trace:    return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
                case spdlog::level::debug:    return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
                case spdlog::level::info:     return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
                case spdlog::level::warn:     return ImVec4(0.9f, 0.75f, 0.2f, 1.0f);
                case spdlog::level::err:      return ImVec4(0.95f, 0.3f, 0.3f, 1.0f);
                case spdlog::level::critical: return ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
                case spdlog::level::off:
                case spdlog::level::n_levels:
                    break;
            }
            return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    void ConsolePanel::OnImGuiRender()
    {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear"))
        {
            if (const auto sink = Log::GetConsoleSink())
            {
                sink->Clear();
            }
        }
        ImGui::SameLine();
        ImGui::Checkbox("Trace", &m_ShowTrace);
        ImGui::SameLine();
        ImGui::Checkbox("Info", &m_ShowInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warn", &m_ShowWarn);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &m_ShowError);
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

        ImGui::Separator();

        ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (const auto sink = Log::GetConsoleSink())
        {
            // CopyEntries(), not a live reference - see ConsoleSink.h for
            // why: a log call (and therefore a sink_it_ mutation) can
            // happen on any thread at any time, including while this exact
            // loop is rendering. Copying once per frame under the sink's
            // own lock is the only way to iterate without holding that
            // lock for the entire duration of ImGui text rendering.
            const std::vector<LogEntry> entries = sink->CopyEntries();
            for (const LogEntry& entry : entries)
            {
                const bool visible =
                    (entry.Level == spdlog::level::trace && m_ShowTrace) ||
                    (entry.Level == spdlog::level::info && m_ShowInfo) ||
                    (entry.Level == spdlog::level::warn && m_ShowWarn) ||
                    (entry.Level >= spdlog::level::err && m_ShowError) ||
                    (entry.Level == spdlog::level::debug && m_ShowTrace);
                if (!visible)
                {
                    continue;
                }

                ImGui::PushStyleColor(ImGuiCol_Text, ColorForLevel(entry.Level));
                ImGui::TextUnformatted(entry.Message.c_str());
                ImGui::PopStyleColor();
            }
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

} // namespace Engine::Editor
