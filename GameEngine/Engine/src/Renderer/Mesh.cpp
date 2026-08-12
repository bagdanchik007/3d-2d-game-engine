#include "Engine/Renderer/Mesh.h"

#include "Engine/Core/Log.h"
#include "Engine/Renderer/Buffer.h"

#include <tiny_obj_loader/tiny_obj_loader.h>

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

    std::shared_ptr<Mesh> Mesh::CreateFromFile(const std::string& path)
    {
        const std::optional<MeshData> data = LoadDataFromFile(path);
        if (!data.has_value())
        {
            return nullptr; // LoadDataFromFile already logged the specific reason
        }

        return std::make_shared<Mesh>(data->Vertices, data->Indices);
    }

    std::optional<MeshData> Mesh::LoadDataFromFile(const std::string& path)
    {
        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.triangulate = true; // this engine's Mesh always draws GL_TRIANGLES (see RendererAPI::DrawIndexed) - never quads/n-gons

        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(path, readerConfig))
        {
            ENGINE_CORE_ERROR("Failed to load mesh '{}': {}", path, reader.Error());
            return std::nullopt;
        }

        if (!reader.Warning().empty())
        {
            ENGINE_CORE_WARN("Mesh '{}' loaded with warnings: {}", path, reader.Warning());
        }

        const auto& attrib = reader.GetAttrib();
        const auto& shapes = reader.GetShapes();

        MeshData data;

        // Deliberately NOT deduplicating shared vertices across triangles
        // (every face gets its own fresh set of MeshVertex entries, exactly
        // like CreateCube's 24-vertex approach): correct and simple to
        // verify, at the cost of a larger vertex buffer than a fully
        // vertex-welded mesh would need. Vertex welding is a real
        // optimization for large imported models, but nothing has
        // profiled this engine's mesh loading as a bottleneck yet -
        // exactly the "don't invent complexity the requirements haven't
        // asked for" principle applied to model loading specifically.
        for (const auto& shape : shapes)
        {
            std::size_t indexOffset = 0;
            for (std::size_t face = 0; face < shape.mesh.num_face_vertices.size(); ++face)
            {
                const auto faceVertexCount = static_cast<std::size_t>(shape.mesh.num_face_vertices[face]);
                ENGINE_CORE_ASSERT(faceVertexCount == 3, "Mesh face is not a triangle despite requesting triangulation");

                Math::Vec3 facePositions[3];
                for (std::size_t v = 0; v < faceVertexCount; ++v)
                {
                    const tinyobj::index_t objIndex = shape.mesh.indices[indexOffset + v];

                    MeshVertex vertex{};
                    vertex.Position = Math::Vec3(
                        attrib.vertices[3 * static_cast<std::size_t>(objIndex.vertex_index) + 0],
                        attrib.vertices[3 * static_cast<std::size_t>(objIndex.vertex_index) + 1],
                        attrib.vertices[3 * static_cast<std::size_t>(objIndex.vertex_index) + 2]);
                    facePositions[v] = vertex.Position;

                    if (objIndex.normal_index >= 0)
                    {
                        vertex.Normal = Math::Vec3(
                            attrib.normals[3 * static_cast<std::size_t>(objIndex.normal_index) + 0],
                            attrib.normals[3 * static_cast<std::size_t>(objIndex.normal_index) + 1],
                            attrib.normals[3 * static_cast<std::size_t>(objIndex.normal_index) + 2]);
                    }
                    // else: left as Vec3's default (0,0,0) for now: the true
                    // flat-face normal isn't computable until all 3 of this
                    // face's positions are known, which happens below.

                    if (objIndex.texcoord_index >= 0)
                    {
                        vertex.TexCoord = Math::Vec2(
                            attrib.texcoords[2 * static_cast<std::size_t>(objIndex.texcoord_index) + 0],
                            attrib.texcoords[2 * static_cast<std::size_t>(objIndex.texcoord_index) + 1]);
                    }

                    data.Vertices.push_back(vertex);
                }

                // Fallback for OBJ files with no normals at all (a real,
                // fairly common case for simple test/placeholder models):
                // compute one flat face normal from the triangle's own
                // winding and apply it to all 3 vertices just added. This
                // mirrors CreateCube's per-face-normal approach rather
                // than leaving the mesh with degenerate (0,0,0) normals,
                // which would make every lighting calculation on it
                // produce NaN (see Vec3::Normalized's zero-vector
                // fallback for why that specific failure mode is avoided
                // throughout this math/renderer layer).
                const bool faceHasNoNormals = shape.mesh.indices[indexOffset].normal_index < 0;
                if (faceHasNoNormals)
                {
                    const Math::Vec3 faceNormal =
                        Math::Cross(facePositions[1] - facePositions[0], facePositions[2] - facePositions[0]).Normalized();
                    const std::size_t vertexBase = data.Vertices.size() - faceVertexCount;
                    for (std::size_t v = 0; v < faceVertexCount; ++v)
                    {
                        data.Vertices[vertexBase + v].Normal = faceNormal;
                    }
                }

                const auto baseIndex = static_cast<uint32_t>(data.Vertices.size() - faceVertexCount);
                data.Indices.push_back(baseIndex + 0);
                data.Indices.push_back(baseIndex + 1);
                data.Indices.push_back(baseIndex + 2);

                indexOffset += faceVertexCount;
            }
        }

        if (data.Vertices.empty())
        {
            ENGINE_CORE_ERROR("Mesh '{}' loaded successfully but contains no geometry", path);
            return std::nullopt;
        }

        return data;
    }

} // namespace Engine
