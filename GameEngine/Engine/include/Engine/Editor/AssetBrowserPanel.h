#pragma once

#include <filesystem>

namespace Engine::Editor
{
    /// Browses a directory tree via std::filesystem, starting from a
    /// fixed root the caller cannot navigate above.
    ///
    /// A simple list view (folder/file rows with a text-based type
    /// indicator), not a thumbnail grid: real thumbnail previews would
    /// mean rendering or decoding every visible texture/model just to
    /// show a small preview image, a genuinely separate feature (texture
    /// caching, background decode, placeholder icons while loading) with
    /// no current requirement driving it - deliberately scoped down
    /// rather than left half-implemented.
    class AssetBrowserPanel
    {
    public:
        explicit AssetBrowserPanel(std::filesystem::path rootDirectory)
            : m_RootDirectory(std::move(rootDirectory)), m_CurrentDirectory(m_RootDirectory)
        {
        }

        void OnImGuiRender();

    private:
        std::filesystem::path m_RootDirectory;
        std::filesystem::path m_CurrentDirectory;
    };

} // namespace Engine::Editor
