#pragma once

#include "Engine/Scene/Scene.h"

#include <string>

namespace Engine::Scene
{
    /// Saves/loads a Scene as YAML: one document, one sequence of entities,
    /// each entity listing its components. See Scene.h and IDComponent's
    /// doc comment for why parent/child links are serialized as UUIDs
    /// rather than raw ECS::Entity handles.
    class SceneSerializer
    {
    public:
        explicit SceneSerializer(Scene& scene) noexcept : m_Scene(scene) {}

        void SerializeToFile(const std::string& path) const;

        /// Returns false (and logs why) on a missing file, malformed YAML,
        /// or a document missing the expected top-level structure - the
        /// same "expected, recoverable failure -> bool/log, not throw or
        /// assert" pattern Mesh::LoadDataFromFile and AssetManager's Load*
        /// functions already use for path-based loading (M9). yaml-cpp
        /// itself throws exceptions internally on parse errors; those are
        /// caught here and converted at this boundary; nothing above this
        /// call needs to know yaml-cpp uses exceptions at all.
        [[nodiscard]] bool DeserializeFromFile(const std::string& path) const;

    private:
        Scene& m_Scene;
    };

} // namespace Engine::Scene
