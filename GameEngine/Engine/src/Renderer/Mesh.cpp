#include "Engine/Renderer/Mesh.h"

#include "Engine/Renderer/Buffer.h"

namespace Engine
{
    Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
        : m_IndexCount(static_cast<uint32_t>(indices.size()))
    {
        m_VertexArray = VertexArray::Create();

        const auto vertexBuffer = VertexBuffer::Create(
            reinterpret_cast<const float*>(vertices.data()), static_cast<uint32_t>(vertices.size() * sizeof(MeshVertex)));
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"},
        });
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        const auto indexBuffer = IndexBuffer::Create(indices.data(), m_IndexCount);
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    std::shared_ptr<Mesh> Mesh::CreateCube(float size)
    {
        const float h = size * 0.5f;

        // Deliberately 24 vertices, not 8: a cube has 8 unique CORNERS,
        // but each of the 6 faces needs its own normal at the corners it
        // touches, so corners cannot be shared between faces (a shared
        // vertex can only carry ONE normal). This is the standard
        // "flat-shaded cube" vertex count in every graphics API, not an
        // oversight - sharing all 8 corners would only work for a cube
        // that's meant to look faceted-wrong (averaged, smoothed normals
        // at what should be a sharp 90-degree edge).
        const std::vector<MeshVertex> vertices = {
            // Front (+Z)
            {{-h, -h, h}, {0, 0, 1}, {0, 0}}, {{h, -h, h}, {0, 0, 1}, {1, 0}}, {{h, h, h}, {0, 0, 1}, {1, 1}}, {{-h, h, h}, {0, 0, 1}, {0, 1}},
            // Back (-Z)
            {{h, -h, -h}, {0, 0, -1}, {0, 0}}, {{-h, -h, -h}, {0, 0, -1}, {1, 0}}, {{-h, h, -h}, {0, 0, -1}, {1, 1}}, {{h, h, -h}, {0, 0, -1}, {0, 1}},
            // Right (+X)
            {{h, -h, h}, {1, 0, 0}, {0, 0}}, {{h, -h, -h}, {1, 0, 0}, {1, 0}}, {{h, h, -h}, {1, 0, 0}, {1, 1}}, {{h, h, h}, {1, 0, 0}, {0, 1}},
            // Left (-X)
            {{-h, -h, -h}, {-1, 0, 0}, {0, 0}}, {{-h, -h, h}, {-1, 0, 0}, {1, 0}}, {{-h, h, h}, {-1, 0, 0}, {1, 1}}, {{-h, h, -h}, {-1, 0, 0}, {0, 1}},
            // Top (+Y)
            {{-h, h, h}, {0, 1, 0}, {0, 0}}, {{h, h, h}, {0, 1, 0}, {1, 0}}, {{h, h, -h}, {0, 1, 0}, {1, 1}}, {{-h, h, -h}, {0, 1, 0}, {0, 1}},
            // Bottom (-Y)
            {{-h, -h, -h}, {0, -1, 0}, {0, 0}}, {{h, -h, -h}, {0, -1, 0}, {1, 0}}, {{h, -h, h}, {0, -1, 0}, {1, 1}}, {{-h, -h, h}, {0, -1, 0}, {0, 1}},
        };

        std::vector<uint32_t> indices;
        indices.reserve(36);
        for (uint32_t face = 0; face < 6; ++face)
        {
            const uint32_t base = face * 4;
            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
            indices.push_back(base + 0);
        }

        return std::make_shared<Mesh>(vertices, indices);
    }

    std::shared_ptr<Mesh> Mesh::CreatePlane(float width, float depth)
    {
        const float hw = width * 0.5f;
        const float hd = depth * 0.5f;

        const std::vector<MeshVertex> vertices = {
            {{-hw, 0.0f, hd}, {0, 1, 0}, {0, 0}},
            {{hw, 0.0f, hd}, {0, 1, 0}, {1, 0}},
            {{hw, 0.0f, -hd}, {0, 1, 0}, {1, 1}},
            {{-hw, 0.0f, -hd}, {0, 1, 0}, {0, 1}},
        };
        const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

        return std::make_shared<Mesh>(vertices, indices);
    }

} // namespace Engine
