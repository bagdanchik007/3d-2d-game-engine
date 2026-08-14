#include "Engine/Scene/Scene.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Engine;
using namespace Engine::Scene;
using Catch::Approx;

TEST_CASE("CreateEntity attaches ID, Tag, Transform, and Relationship components by default", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity entity = scene.CreateEntity("Player");

    REQUIRE(scene.GetRegistry().HasComponent<IDComponent>(entity));
    REQUIRE(scene.GetRegistry().HasComponent<TagComponent>(entity));
    REQUIRE(scene.GetRegistry().HasComponent<TransformComponent>(entity));
    REQUIRE(scene.GetRegistry().HasComponent<RelationshipComponent>(entity));
    REQUIRE(scene.GetRegistry().GetComponent<TagComponent>(entity).Name == "Player");
}

TEST_CASE("Two entities created in the same Scene get different UUIDs", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity a = scene.CreateEntity();
    const ECS::Entity b = scene.CreateEntity();

    const auto& idA = scene.GetRegistry().GetComponent<IDComponent>(a);
    const auto& idB = scene.GetRegistry().GetComponent<IDComponent>(b);
    REQUIRE_FALSE(idA.ID == idB.ID);
}

TEST_CASE("SetParent establishes both the child's Parent and the parent's Children entry", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity parent = scene.CreateEntity("Parent");
    const ECS::Entity child = scene.CreateEntity("Child");

    scene.SetParent(child, parent);

    REQUIRE(scene.GetParent(child) == parent);
    REQUIRE(scene.GetChildren(parent).size() == 1);
    REQUIRE(scene.GetChildren(parent)[0] == child);
}

TEST_CASE("Re-parenting removes the child from its previous parent's Children list", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity parentA = scene.CreateEntity("A");
    const ECS::Entity parentB = scene.CreateEntity("B");
    const ECS::Entity child = scene.CreateEntity("Child");

    scene.SetParent(child, parentA);
    scene.SetParent(child, parentB);

    REQUIRE(scene.GetChildren(parentA).empty());
    REQUIRE(scene.GetChildren(parentB).size() == 1);
    REQUIRE(scene.GetParent(child) == parentB);
}

TEST_CASE("SetParent with NullEntity un-parents a child", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity parent = scene.CreateEntity();
    const ECS::Entity child = scene.CreateEntity();

    scene.SetParent(child, parent);
    scene.SetParent(child, ECS::NullEntity);

    REQUIRE(scene.GetParent(child).IsNull());
    REQUIRE(scene.GetChildren(parent).empty());
}

TEST_CASE("SetParent rejects an entity becoming its own parent", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity entity = scene.CreateEntity();

    scene.SetParent(entity, entity);

    REQUIRE(scene.GetParent(entity).IsNull()); // rejected - unchanged
}

TEST_CASE("SetParent rejects creating a cycle (grandchild becoming its own grandparent)", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity grandparent = scene.CreateEntity("Grandparent");
    const ECS::Entity parent = scene.CreateEntity("Parent");
    const ECS::Entity child = scene.CreateEntity("Child");

    scene.SetParent(parent, grandparent);
    scene.SetParent(child, parent);

    // Attempting to make grandparent a child of its own grandchild.
    scene.SetParent(grandparent, child);

    // Must be rejected - the original hierarchy stays intact.
    REQUIRE(scene.GetParent(grandparent).IsNull());
    REQUIRE(scene.GetParent(parent) == grandparent);
    REQUIRE(scene.GetParent(child) == parent);
}

TEST_CASE("DestroyEntity recursively destroys all descendants", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity root = scene.CreateEntity("Root");
    const ECS::Entity child = scene.CreateEntity("Child");
    const ECS::Entity grandchild = scene.CreateEntity("Grandchild");

    scene.SetParent(child, root);
    scene.SetParent(grandchild, child);

    scene.DestroyEntity(root);

    REQUIRE_FALSE(scene.GetRegistry().IsValid(root));
    REQUIRE_FALSE(scene.GetRegistry().IsValid(child));
    REQUIRE_FALSE(scene.GetRegistry().IsValid(grandchild));
}

TEST_CASE("DestroyEntity with multiple children destroys all of them without skipping any", "[scene]")
{
    // Specifically exercises the "copy the children list before
    // recursing" fix documented in Scene::DestroyEntity - a naive
    // implementation iterating the live Children vector while it shrinks
    // during recursion would skip every other child once there are 3+.
    Engine::Scene::Scene scene;
    const ECS::Entity root = scene.CreateEntity("Root");
    const ECS::Entity childA = scene.CreateEntity("A");
    const ECS::Entity childB = scene.CreateEntity("B");
    const ECS::Entity childC = scene.CreateEntity("C");

    scene.SetParent(childA, root);
    scene.SetParent(childB, root);
    scene.SetParent(childC, root);

    scene.DestroyEntity(root);

    REQUIRE_FALSE(scene.GetRegistry().IsValid(childA));
    REQUIRE_FALSE(scene.GetRegistry().IsValid(childB));
    REQUIRE_FALSE(scene.GetRegistry().IsValid(childC));
}

TEST_CASE("DestroyEntity removes the entity from its parent's Children list without destroying siblings", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity parent = scene.CreateEntity("Parent");
    const ECS::Entity childA = scene.CreateEntity("A");
    const ECS::Entity childB = scene.CreateEntity("B");
    scene.SetParent(childA, parent);
    scene.SetParent(childB, parent);

    scene.DestroyEntity(childA);

    REQUIRE(scene.GetRegistry().IsValid(childB));
    REQUIRE(scene.GetChildren(parent).size() == 1);
    REQUIRE(scene.GetChildren(parent)[0] == childB);
}

TEST_CASE("GetWorldTransform for a root entity equals its own local transform", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity entity = scene.CreateEntity();
    scene.GetRegistry().GetComponent<TransformComponent>(entity).Position = Math::Vec3(5.0f, 0.0f, 0.0f);

    const Math::Mat4 world = scene.GetWorldTransform(entity);
    const Math::Vec4 origin = world * Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f);

    REQUIRE(origin.x == Approx(5.0f));
}

TEST_CASE("GetWorldTransform composes a child's transform with its parent's", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity parent = scene.CreateEntity("Parent");
    scene.GetRegistry().GetComponent<TransformComponent>(parent).Position = Math::Vec3(10.0f, 0.0f, 0.0f);

    const ECS::Entity child = scene.CreateEntity("Child");
    scene.GetRegistry().GetComponent<TransformComponent>(child).Position = Math::Vec3(1.0f, 0.0f, 0.0f);
    scene.SetParent(child, parent);

    const Math::Mat4 childWorld = scene.GetWorldTransform(child);
    const Math::Vec4 origin = childWorld * Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Child is at local (1,0,0) under a parent translated to (10,0,0) -
    // world position should be (11,0,0), not (1,0,0): this is exactly the
    // case that would fail if GetWorldTransform ignored the parent chain
    // entirely and just returned the local matrix.
    REQUIRE(origin.x == Approx(11.0f));
}

TEST_CASE("FindEntityByID finds a created entity by its UUID", "[scene]")
{
    Engine::Scene::Scene scene;
    const ECS::Entity entity = scene.CreateEntity();
    const UUID id = scene.GetRegistry().GetComponent<IDComponent>(entity).ID;

    REQUIRE(scene.FindEntityByID(id) == entity);
}

TEST_CASE("FindEntityByID returns NullEntity for an unknown UUID", "[scene]")
{
    Engine::Scene::Scene scene;
    scene.CreateEntity();

    REQUIRE(scene.FindEntityByID(UUID()).IsNull());
}
