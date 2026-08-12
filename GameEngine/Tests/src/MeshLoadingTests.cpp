#include "Engine/Renderer/Mesh.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

using namespace Engine;
using Catch::Approx;

namespace
{
    /// Writes `contents` to a temp file and removes it when the guard goes
    /// out of scope - RAII cleanup so a failing REQUIRE partway through a
    /// test doesn't leave stray .obj files behind in the temp directory.
    class TempObjFile
    {
    public:
        explicit TempObjFile(const std::string& contents)
            : m_Path(std::filesystem::temp_directory_path() / "engine_mesh_test.obj")
        {
            std::ofstream file(m_Path);
            file << contents;
        }

        ~TempObjFile()
        {
            std::error_code ignored;
            std::filesystem::remove(m_Path, ignored);
        }

        TempObjFile(const TempObjFile&) = delete;
        TempObjFile& operator=(const TempObjFile&) = delete;

        [[nodiscard]] std::string PathString() const { return m_Path.string(); }

    private:
        std::filesystem::path m_Path;
    };

    // A single triangle with explicit normals and texture coordinates -
    // the "everything present" case.
    constexpr const char* kTriangleWithNormalsObj = R"(
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
vn 0.0 0.0 1.0
vn 0.0 0.0 1.0
vn 0.0 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0
vt 0.0 1.0
f 1/1/1 2/2/2 3/3/3
)";

    // Same triangle, but with no vn lines and faces that omit the normal
    // index entirely - exercises the flat-face-normal fallback.
    constexpr const char* kTriangleWithoutNormalsObj = R"(
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
f 1 2 3
)";
}

TEST_CASE("LoadDataFromFile parses a triangle with explicit normals and texcoords", "[assets][mesh]")
{
    const TempObjFile objFile(kTriangleWithNormalsObj);

    const auto data = Mesh::LoadDataFromFile(objFile.PathString());
    REQUIRE(data.has_value());

    REQUIRE(data->Vertices.size() == 3);
    REQUIRE(data->Indices.size() == 3);

    REQUIRE(data->Vertices[0].Position == Math::Vec3(0.0f, 0.0f, 0.0f));
    REQUIRE(data->Vertices[1].Position == Math::Vec3(1.0f, 0.0f, 0.0f));
    REQUIRE(data->Vertices[2].Position == Math::Vec3(0.0f, 1.0f, 0.0f));

    for (const auto& vertex : data->Vertices)
    {
        REQUIRE(vertex.Normal == Math::Vec3(0.0f, 0.0f, 1.0f));
    }
}

TEST_CASE("LoadDataFromFile computes a flat face normal when the OBJ has none", "[assets][mesh]")
{
    const TempObjFile objFile(kTriangleWithoutNormalsObj);

    const auto data = Mesh::LoadDataFromFile(objFile.PathString());
    REQUIRE(data.has_value());
    REQUIRE(data->Vertices.size() == 3);

    // Triangle lies in the XY plane with counter-clockwise winding as
    // seen from +Z, so the computed normal should point along +Z - this
    // is exactly the case Vec3::Normalized's zero-vector fallback and the
    // Cross-product anti-commutativity tests in Mat4Tests/VecTests exist
    // to make trustworthy building blocks for.
    for (const auto& vertex : data->Vertices)
    {
        REQUIRE(vertex.Normal.x == Approx(0.0f).margin(1e-5f));
        REQUIRE(vertex.Normal.y == Approx(0.0f).margin(1e-5f));
        REQUIRE(vertex.Normal.z == Approx(1.0f));
    }
}

TEST_CASE("LoadDataFromFile returns nullopt for a nonexistent file", "[assets][mesh]")
{
    const auto data = Mesh::LoadDataFromFile("/this/path/definitely/does/not/exist.obj");
    REQUIRE_FALSE(data.has_value());
}

TEST_CASE("LoadDataFromFile returns nullopt for a file with no geometry", "[assets][mesh]")
{
    const TempObjFile objFile("# just a comment, no v/f lines\n");

    const auto data = Mesh::LoadDataFromFile(objFile.PathString());
    REQUIRE_FALSE(data.has_value());
}
