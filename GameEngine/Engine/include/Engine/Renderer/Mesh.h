#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Renderer/VertexArray.h"

#include <optional>
#include <string>
#include <vector>

namespace Engine
{
    struct MeshVertex
    {
        Math::Vec3 Position;
        Math::Vec3 Normal;
        Math::Vec2 TexCoord;
    };

    /// Pure CPU-side vertex/index data, with no GPU resource attached.
    /// Exists specifically so OBJ parsing (LoadDataFromFile below) is
    /// testable without a live OpenGL context - Mesh's constructor calls
    /// VertexArray::Create(), which calls real GL functions and would
    /// crash in Tests/'s headless environment (see Tests/src/TestSupport/
    /// NullWindow.h for the same class of problem solved differently for
    /// Application). Separating "parse the file" from "upload to the GPU"
    /// was not the original design - it was a direct consequence of
    /// trying to write a test for file loading and discovering the two
    /// concerns were fused together.
    struct MeshData
    {
        std::vector<MeshVertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    /// Owns a VertexArray built from CPU-side vertex/index data.
    ///
    /// Only procedural generators (CreateCube, CreatePlane) exist for now.
    /// Loading an actual model file (OBJ/glTF via assimp or tinyobjloader)
    /// is Milestone 9's ("Assets": model loading) job, not this class's -
    /// Mesh's public shape (take vertices+indices, own a VertexArray)
    /// doesn't change at all when that loader is added; it just becomes
    /// another source feeding the same constructor.
    class Mesh
    {
    public:
        Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);

        [[nodiscard]] const std::shared_ptr<VertexArray>& GetVertexArray() const noexcept { return m_VertexArray; }
        [[nodiscard]] uint32_t GetIndexCount() const noexcept { return m_IndexCount; }

        [[nodiscard]] static std::shared_ptr<Mesh> CreateCube(float size = 1.0f);
        [[nodiscard]] static std::shared_ptr<Mesh> CreatePlane(float width = 1.0f, float depth = 1.0f);

        /// Loads an OBJ file via tinyobjloader (see Mesh.cpp). Returns
        /// nullptr on failure rather than throwing or asserting: a missing
        /// or malformed model file is an expected, recoverable failure
        /// mode for anything path-based (unlike, say, a failed shader
        /// compile, which this codebase does assert on) - AssetManager
        /// (see Assets/AssetManager.h) checks this return value and logs
        /// rather than crashing the whole application over one bad asset.
        [[nodiscard]] static std::shared_ptr<Mesh> CreateFromFile(const std::string& path);

        /// The CPU-only half of CreateFromFile - parsing without any GPU
        /// upload. Public specifically so it's independently testable
        /// (see Tests/src/MeshLoadingTests.cpp) without requiring a live
        /// GL context; most callers want CreateFromFile instead.
        [[nodiscard]] static std::optional<MeshData> LoadDataFromFile(const std::string& path);

    private:
        std::shared_ptr<VertexArray> m_VertexArray;
        uint32_t m_IndexCount;
    };

} // namespace Engine
