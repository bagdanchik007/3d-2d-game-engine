#include "Engine/Math/Vec2.h"
#include "Engine/Math/Vec3.h"
#include "Engine/Math/Vec4.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Engine::Math;
using Catch::Approx;

TEST_CASE("Vec2 arithmetic operators", "[math][vec2]")
{
    const Vec2 a(1.0f, 2.0f);
    const Vec2 b(3.0f, 4.0f);

    REQUIRE((a + b) == Vec2(4.0f, 6.0f));
    REQUIRE((a - b) == Vec2(-2.0f, -2.0f));
    REQUIRE((a * 2.0f) == Vec2(2.0f, 4.0f));
    REQUIRE((2.0f * a) == Vec2(2.0f, 4.0f));
    REQUIRE((b / 2.0f) == Vec2(1.5f, 2.0f));
    REQUIRE(-a == Vec2(-1.0f, -2.0f));
}

TEST_CASE("Vec2 length and normalization", "[math][vec2]")
{
    const Vec2 v(3.0f, 4.0f);

    REQUIRE(v.Length() == Approx(5.0f));
    REQUIRE(v.LengthSquared() == Approx(25.0f));

    const Vec2 n = v.Normalized();
    REQUIRE(n.Length() == Approx(1.0f));
    REQUIRE(n.x == Approx(0.6f));
    REQUIRE(n.y == Approx(0.8f));
}

TEST_CASE("Vec2::Normalized on a zero vector returns zero, not NaN", "[math][vec2]")
{
    const Vec2 zero(0.0f, 0.0f);
    const Vec2 n = zero.Normalized();

    REQUIRE(n == Vec2(0.0f, 0.0f));
    REQUIRE_FALSE(std::isnan(n.x));
    REQUIRE_FALSE(std::isnan(n.y));
}

TEST_CASE("Vec2 dot product", "[math][vec2]")
{
    REQUIRE(Dot(Vec2(1.0f, 0.0f), Vec2(0.0f, 1.0f)) == Approx(0.0f));
    REQUIRE(Dot(Vec2(2.0f, 3.0f), Vec2(4.0f, 5.0f)) == Approx(23.0f));
}

TEST_CASE("Vec3 arithmetic operators", "[math][vec3]")
{
    const Vec3 a(1.0f, 2.0f, 3.0f);
    const Vec3 b(4.0f, 5.0f, 6.0f);

    REQUIRE((a + b) == Vec3(5.0f, 7.0f, 9.0f));
    REQUIRE((b - a) == Vec3(3.0f, 3.0f, 3.0f));
    REQUIRE((a * 2.0f) == Vec3(2.0f, 4.0f, 6.0f));
}

TEST_CASE("Vec3 cross product is perpendicular to both inputs", "[math][vec3]")
{
    const Vec3 x(1.0f, 0.0f, 0.0f);
    const Vec3 y(0.0f, 1.0f, 0.0f);
    const Vec3 z = Cross(x, y);

    REQUIRE(z == Vec3(0.0f, 0.0f, 1.0f));
    REQUIRE(Dot(z, x) == Approx(0.0f));
    REQUIRE(Dot(z, y) == Approx(0.0f));
}

TEST_CASE("Vec3 cross product is anti-commutative", "[math][vec3]")
{
    const Vec3 a(1.0f, 2.0f, 3.0f);
    const Vec3 b(4.0f, 5.0f, 6.0f);

    const Vec3 ab = Cross(a, b);
    const Vec3 ba = Cross(b, a);

    REQUIRE(ab.x == Approx(-ba.x));
    REQUIRE(ab.y == Approx(-ba.y));
    REQUIRE(ab.z == Approx(-ba.z));
}

TEST_CASE("Vec3::Normalized on a zero vector returns zero, not NaN", "[math][vec3]")
{
    const Vec3 n = Vec3(0.0f, 0.0f, 0.0f).Normalized();

    REQUIRE(n == Vec3(0.0f, 0.0f, 0.0f));
    REQUIRE_FALSE(std::isnan(n.x));
}

TEST_CASE("Vec3::XY extracts the first two components", "[math][vec3]")
{
    const Vec3 v(1.0f, 2.0f, 3.0f);
    REQUIRE(v.XY() == Vec2(1.0f, 2.0f));
}

TEST_CASE("Vec4 arithmetic and Dot", "[math][vec4]")
{
    const Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    const Vec4 b(5.0f, 6.0f, 7.0f, 8.0f);

    REQUIRE((a + b) == Vec4(6.0f, 8.0f, 10.0f, 12.0f));
    REQUIRE(Dot(a, b) == Approx(1 * 5 + 2 * 6 + 3 * 7 + 4 * 8));
}

TEST_CASE("Vec4 index operator matches named components", "[math][vec4]")
{
    Vec4 v(1.0f, 2.0f, 3.0f, 4.0f);

    REQUIRE(v[0] == Approx(1.0f));
    REQUIRE(v[1] == Approx(2.0f));
    REQUIRE(v[2] == Approx(3.0f));
    REQUIRE(v[3] == Approx(4.0f));

    v[2] = 99.0f;
    REQUIRE(v.z == Approx(99.0f));
}

TEST_CASE("Vec4 constructed from Vec3 and w", "[math][vec4]")
{
    const Vec3 xyz(1.0f, 2.0f, 3.0f);
    const Vec4 v(xyz, 1.0f);

    REQUIRE(v == Vec4(1.0f, 2.0f, 3.0f, 1.0f));
    REQUIRE(v.XYZ() == xyz);
}
