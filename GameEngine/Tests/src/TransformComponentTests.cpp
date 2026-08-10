#include "Engine/Scene/Components.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Engine;
using Catch::Approx;

TEST_CASE("Default TransformComponent produces the identity matrix", "[scene][transform]")
{
    const Scene::TransformComponent transform;
    const Math::Mat4 m = transform.GetLocalMatrix();

    const Math::Vec4 point(1.0f, 2.0f, 3.0f, 1.0f);
    const Math::Vec4 result = m * point;

    REQUIRE(result.x == Approx(1.0f));
    REQUIRE(result.y == Approx(2.0f));
    REQUIRE(result.z == Approx(3.0f));
}

TEST_CASE("TransformComponent combines translation, rotation, and scale in TRS order", "[scene][transform]")
{
    Scene::TransformComponent transform;
    transform.Position = Math::Vec3(10.0f, 0.0f, 0.0f);
    transform.Scale = Math::Vec3(2.0f, 2.0f, 2.0f);

    const Math::Vec4 result = transform.GetLocalMatrix() * Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // Scaled first (1,0,0)->(2,0,0), then translated: (12,0,0).
    REQUIRE(result.x == Approx(12.0f));
    REQUIRE(result.y == Approx(0.0f));
    REQUIRE(result.z == Approx(0.0f));
}

TEST_CASE("TagComponent stores its name", "[scene][transform]")
{
    const Scene::TagComponent tag("Player");
    REQUIRE(tag.Name == "Player");
}
