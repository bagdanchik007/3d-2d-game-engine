#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Renderer/VertexArray.h"

#include <vector>

namespace Engine
{
    struct MeshVertex
    {
        Math::Vec3 Position;
        Math::Vec3 Normal;
        Math::Vec2 TexCoord;
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

    private:
        std::shared_ptr<VertexArray> m_VertexArray;
        uint32_t m_IndexCount;
    };

} // namespace Engine
