#include "Engine/Editor/AssetBrowserPanel.h"

#include <imgui.h>

#include <algorithm>
#include <system_error>
#include <vector>

namespace Engine::Editor
{
    void AssetBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Asset Browser");

        if (m_CurrentDirectory != m_RootDirectory)
        {
            if (ImGui::Button("<- Back"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }
        else
        {
            // A disabled-looking placeholder rather than omitting the
            // button entirely: keeps the toolbar's layout stable instead
            // of every other button shifting position depending on
            // whether "Back" happens to be present this frame.
            ImGui::BeginDisabled();
            ImGui::Button("<- Back");
            ImGui::EndDisabled();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("%s", std::filesystem::relative(m_CurrentDirectory, m_RootDirectory).string().c_str());

        ImGui::Separator();

        std::error_code errorCode;
        if (!std::filesystem::exists(m_CurrentDirectory, errorCode) || errorCode)
        {
            ImGui::TextDisabled("Directory does not exist: %s", m_CurrentDirectory.string().c_str());
            ImGui::End();
            return;
        }

        // Collected and sorted (directories first, then alphabetical)
        // before display, rather than iterated and drawn directly:
        // std::filesystem::directory_iterator gives no ordering guarantee
        // at all, and a browser panel that re-shuffles its own rows every
        // frame depending on the OS's arbitrary enumeration order would
        // be actively unpleasant to use.
        struct Entry
        {
            std::filesystem::path Path;
            bool IsDirectory;
        };
        std::vector<Entry> entries;
        for (const auto& dirEntry : std::filesystem::directory_iterator(m_CurrentDirectory, errorCode))
        {
            entries.push_back({dirEntry.path(), dirEntry.is_directory()});
        }
        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b)
        {
            if (a.IsDirectory != b.IsDirectory)
            {
                return a.IsDirectory; // directories before files
            }
            return a.Path.filename().string() < b.Path.filename().string();
        });

        for (const Entry& entry : entries)
        {
            const std::string filename = entry.Path.filename().string();
            const std::string label = (entry.IsDirectory ? "[Dir]  " : "[File] ") + filename;

            if (ImGui::Selectable(label.c_str()))
            {
                if (entry.IsDirectory)
                {
                    m_CurrentDirectory = entry.Path;
                }
                // Selecting a file currently does nothing beyond
                // highlighting it - opening/importing a selected asset
                // (feeding its path into AssetManager::LoadTexture2D/
                // LoadMesh, M9) is a natural next step with no current
                // caller needing it yet, so it's left as a clearly-scoped
                // future addition rather than a half-wired guess at what
                // "open" should mean for every asset type.
            }
        }

        ImGui::End();
    }

} // namespace Engine::Editor
