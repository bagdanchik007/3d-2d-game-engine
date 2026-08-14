#include "Engine/Scene/SceneSerializer.h"
#include "Engine/ECS/View.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

using namespace Engine;
using namespace Engine::Scene;
using Catch::Approx;

namespace
{
    /// Same RAII temp-file pattern as MeshLoadingTests.cpp's TempObjFile,
    /// generalized to any filename/extension since this suite writes
    /// scene YAML rather than OBJ text.
    class TempFile
    {
    public:
        explicit TempFile(std::string filename)
            : m_Path(std::filesystem::temp_directory_path() / std::move(filename))
        {
        }

        ~TempFile()
        {
            std::error_code ignored;
            std::filesystem::remove(m_Path, ignored);
        }

        TempFile(const TempFile&) = delete;
        TempFile& operator=(const TempFile&) = delete;

        [[nodiscard]] std::string PathString() const { return m_Path.string(); }

    private:
        std::filesystem::path m_Path;
    };
}

TEST_CASE("A serialized-then-deserialized entity keeps its name and transform", "[scene][serializer]")
{
    const TempFile file("engine_scene_test_basic.yaml");

    Engine::Scene::Scene saveScene;
    const ECS::Entity original = saveScene.CreateEntity("Player");
    auto& transform = saveScene.GetRegistry().GetComponent<TransformComponent>(original);
    transform.Position = Math::Vec3(1.0f, 2.0f, 3.0f);
    transform.Rotation = Math::Quaternion::FromAxisAngle(Math::Vec3(0.0f, 1.0f, 0.0f), Math::Radians(45.0f));
    transform.Scale = Math::Vec3(2.0f, 2.0f, 2.0f);

    SceneSerializer(saveScene).SerializeToFile(file.PathString());

    Engine::Scene::Scene loadScene;
    const bool loaded = SceneSerializer(loadScene).DeserializeFromFile(file.PathString());
    REQUIRE(loaded);

    ECS::Entity restored = ECS::NullEntity;
    ECS::View<TagComponent> view(loadScene.GetRegistry());
    view.Each([&](ECS::Entity e, TagComponent& tag)
    {
        if (tag.Name == "Player")
        {
            restored = e;
        }
    });

    REQUIRE_FALSE(restored.IsNull());
    const auto& restoredTransform = loadScene.GetRegistry().GetComponent<TransformComponent>(restored);
    REQUIRE(restoredTransform.Position.x == Approx(1.0f));
    REQUIRE(restoredTransform.Position.y == Approx(2.0f));
    REQUIRE(restoredTransform.Position.z == Approx(3.0f));
    REQUIRE(restoredTransform.Scale.x == Approx(2.0f));
    REQUIRE(restoredTransform.Rotation.w == Approx(transform.Rotation.w));
}

TEST_CASE("A serialized-then-deserialized entity keeps the SAME UUID", "[scene][serializer]")
{
    // This is the property the entire IDComponent design exists for (see
    // Components.h) - if the UUID changed across a round trip, nothing
    // referencing this entity by ID from outside the scene file itself
    // (a save slot's "last selected entity", a quest system) could
    // survive a save/load cycle either.
    const TempFile file("engine_scene_test_uuid.yaml");

    Engine::Scene::Scene saveScene;
    const ECS::Entity original = saveScene.CreateEntity("Item");
    const UUID originalID = saveScene.GetRegistry().GetComponent<IDComponent>(original).ID;

    SceneSerializer(saveScene).SerializeToFile(file.PathString());

    Engine::Scene::Scene loadScene;
    REQUIRE(SceneSerializer(loadScene).DeserializeFromFile(file.PathString()));

    const ECS::Entity restored = loadScene.FindEntityByID(originalID);
    REQUIRE_FALSE(restored.IsNull());
}

TEST_CASE("Parent/child relationships survive a save/load round trip", "[scene][serializer]")
{
    const TempFile file("engine_scene_test_hierarchy.yaml");

    Engine::Scene::Scene saveScene;
    const ECS::Entity parent = saveScene.CreateEntity("Parent");
    const ECS::Entity child = saveScene.CreateEntity("Child");
    saveScene.SetParent(child, parent);

    const UUID parentID = saveScene.GetRegistry().GetComponent<IDComponent>(parent).ID;
    const UUID childID = saveScene.GetRegistry().GetComponent<IDComponent>(child).ID;

    SceneSerializer(saveScene).SerializeToFile(file.PathString());

    Engine::Scene::Scene loadScene;
    REQUIRE(SceneSerializer(loadScene).DeserializeFromFile(file.PathString()));

    const ECS::Entity restoredParent = loadScene.FindEntityByID(parentID);
    const ECS::Entity restoredChild = loadScene.FindEntityByID(childID);
    REQUIRE_FALSE(restoredParent.IsNull());
    REQUIRE_FALSE(restoredChild.IsNull());

    REQUIRE(loadScene.GetParent(restoredChild) == restoredParent);
    REQUIRE(loadScene.GetChildren(restoredParent).size() == 1);
    REQUIRE(loadScene.GetChildren(restoredParent)[0] == restoredChild);
}

TEST_CASE("A child appearing before its parent in the file still resolves correctly", "[scene][serializer]")
{
    // Hand-written YAML with the child entity listed FIRST - this is
    // exactly the ordering DeserializeFromFile's two-pass design (see
    // SceneSerializer.cpp) exists to handle correctly.
    const TempFile file("engine_scene_test_forward_reference.yaml");

    constexpr uint64_t kChildID = 111;
    constexpr uint64_t kParentID = 222;

    std::ofstream out(file.PathString());
    out << "Scene: Untitled\n";
    out << "Entities:\n";
    out << "  - Entity: " << kChildID << "\n";
    out << "    TagComponent:\n";
    out << "      Name: Child\n";
    out << "    TransformComponent:\n";
    out << "      Position: [0, 0, 0]\n";
    out << "      Rotation: [0, 0, 0, 1]\n";
    out << "      Scale: [1, 1, 1]\n";
    out << "    RelationshipComponent:\n";
    out << "      Parent: " << kParentID << "\n";
    out << "  - Entity: " << kParentID << "\n";
    out << "    TagComponent:\n";
    out << "      Name: Parent\n";
    out << "    TransformComponent:\n";
    out << "      Position: [0, 0, 0]\n";
    out << "      Rotation: [0, 0, 0, 1]\n";
    out << "      Scale: [1, 1, 1]\n";
    out.close();

    Engine::Scene::Scene scene;
    REQUIRE(SceneSerializer(scene).DeserializeFromFile(file.PathString()));

    const ECS::Entity child = scene.FindEntityByID(UUID(kChildID));
    const ECS::Entity parent = scene.FindEntityByID(UUID(kParentID));
    REQUIRE_FALSE(child.IsNull());
    REQUIRE_FALSE(parent.IsNull());
    REQUIRE(scene.GetParent(child) == parent);
}

TEST_CASE("DeserializeFromFile returns false for a nonexistent file", "[scene][serializer]")
{
    Engine::Scene::Scene scene;
    REQUIRE_FALSE(SceneSerializer(scene).DeserializeFromFile("/this/path/does/not/exist.yaml"));
}

TEST_CASE("DeserializeFromFile returns false for a file missing the expected top-level structure", "[scene][serializer]")
{
    const TempFile file("engine_scene_test_malformed.yaml");
    std::ofstream out(file.PathString());
    out << "NotAScene: true\n";
    out.close();

    Engine::Scene::Scene scene;
    REQUIRE_FALSE(SceneSerializer(scene).DeserializeFromFile(file.PathString()));
}

TEST_CASE("A round trip of an entity with no parent produces no RelationshipComponent YAML key, and loads with a null parent", "[scene][serializer]")
{
    const TempFile file("engine_scene_test_no_parent.yaml");

    Engine::Scene::Scene saveScene;
    saveScene.CreateEntity("Orphan");
    SceneSerializer(saveScene).SerializeToFile(file.PathString());

    Engine::Scene::Scene loadScene;
    REQUIRE(SceneSerializer(loadScene).DeserializeFromFile(file.PathString()));

    ECS::Entity restored = ECS::NullEntity;
    ECS::View<TagComponent> view(loadScene.GetRegistry());
    view.Each([&](ECS::Entity e, TagComponent& tag)
    {
        if (tag.Name == "Orphan")
        {
            restored = e;
        }
    });

    REQUIRE_FALSE(restored.IsNull());
    REQUIRE(loadScene.GetParent(restored).IsNull());
}
