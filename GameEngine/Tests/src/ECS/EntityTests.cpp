#include "Engine/ECS/Entity.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine::ECS;

TEST_CASE("Default-constructed Entity is null", "[ecs][entity]")
{
    const Entity e;
    REQUIRE(e.IsNull());
}

TEST_CASE("NullEntity is null", "[ecs][entity]")
{
    REQUIRE(NullEntity.IsNull());
}

TEST_CASE("Entity equality compares both index and generation", "[ecs][entity]")
{
    // Registry is the only public way to construct a non-null Entity, but
    // we can still verify the value semantics contract via two nulls and
    // Registry-produced entities in RegistryTests.cpp; here we confirm the
    // default/null case, which needs no Registry.
    const Entity a;
    const Entity b;
    REQUIRE(a == b);
}
