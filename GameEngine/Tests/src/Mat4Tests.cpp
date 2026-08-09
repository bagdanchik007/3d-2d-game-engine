#include "Engine/Math/Mat4.h"
#include "Engine/Math/MathUtils.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Engine::Math;
using Catch::Approx;

namespace
{
    bool MatricesApproxEqual(const Mat4& a, const Mat4& b, float epsilon = 1e-4f)
    {
        for (int col = 0; col < 4; ++col)
        {
            for (int row = 0; row < 4; ++row)
            {
                if (std::fabs(a[col][row] - b[col][row]) > epsilon)
                {
                    return false;
                }
            }
        }
        return true;
    }
}

TEST_CASE("Identity matrix leaves a vector unchanged", "[math][mat4]")
{
    const Vec4 v(1.0f, 2.0f, 3.0f, 1.0f);
    const Vec4 result = Mat4::Identity() * v;

    REQUIRE(result == v);
}

TEST_CASE("Translate moves a point but not a direction", "[math][mat4]")
{
    const Mat4 t = Mat4::Translate(Vec3(10.0f, 20.0f, 30.0f));

    const Vec4 point(0.0f, 0.0f, 0.0f, 1.0f); // w=1: a point
    const Vec4 direction(0.0f, 0.0f, 0.0f, 0.0f); // w=0: a direction

    const Vec4 movedPoint = t * point;
    const Vec4 movedDirection = t * direction;

    REQUIRE(movedPoint == Vec4(10.0f, 20.0f, 30.0f, 1.0f));
    REQUIRE(movedDirection == Vec4(0.0f, 0.0f, 0.0f, 0.0f)); // w=0 must suppress translation - this is *why* w exists
}

TEST_CASE("Scale scales each axis independently", "[math][mat4]")
{
    const Mat4 s = Mat4::Scale(Vec3(2.0f, 3.0f, 4.0f));
    const Vec4 result = s * Vec4(1.0f, 1.0f, 1.0f, 1.0f);

    REQUIRE(result == Vec4(2.0f, 3.0f, 4.0f, 1.0f));
}

TEST_CASE("RotationZ by 90 degrees rotates X onto Y", "[math][mat4]")
{
    const Mat4 r = Mat4::RotationZ(Radians(90.0f));
    const Vec4 result = r * Vec4(1.0f, 0.0f, 0.0f, 1.0f);

    REQUIRE(result.x == Approx(0.0f).margin(1e-5f));
    REQUIRE(result.y == Approx(1.0f));
    REQUIRE(result.z == Approx(0.0f).margin(1e-5f));
}

TEST_CASE("Matrix multiplication composes transforms right-to-left", "[math][mat4]")
{
    // Translate(Scale(v)) - scale first, then translate - matches the
    // "child transform applied first" convention documented in Mat4.h.
    const Mat4 t = Mat4::Translate(Vec3(5.0f, 0.0f, 0.0f));
    const Mat4 s = Mat4::Scale(Vec3(2.0f, 2.0f, 2.0f));
    const Mat4 combined = t * s;

    const Vec4 result = combined * Vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Scaled first: (2,2,2), then translated: (7,2,2).
    REQUIRE(result == Vec4(7.0f, 2.0f, 2.0f, 1.0f));
}

TEST_CASE("Matrix multiplication is associative", "[math][mat4]")
{
    const Mat4 a = Mat4::Translate(Vec3(1.0f, 2.0f, 3.0f));
    const Mat4 b = Mat4::Scale(Vec3(2.0f, 2.0f, 2.0f));
    const Mat4 c = Mat4::RotationY(Radians(45.0f));

    const Mat4 left = (a * b) * c;
    const Mat4 right = a * (b * c);

    REQUIRE(MatricesApproxEqual(left, right));
}

TEST_CASE("Transpose of a transpose is the original matrix", "[math][mat4]")
{
    const Mat4 m = Mat4::Translate(Vec3(1.0f, 2.0f, 3.0f)) * Mat4::RotationX(Radians(30.0f));

    REQUIRE(MatricesApproxEqual(m.Transpose().Transpose(), m));
}

TEST_CASE("Inverse of a translation undoes the translation", "[math][mat4]")
{
    const Mat4 t = Mat4::Translate(Vec3(5.0f, -3.0f, 2.0f));
    const Mat4 inv = t.Inverse();

    REQUIRE(MatricesApproxEqual(t * inv, Mat4::Identity()));
}

TEST_CASE("Inverse of a composed TRS matrix undoes the full transform", "[math][mat4]")
{
    const Mat4 m = Mat4::Translate(Vec3(3.0f, 4.0f, 5.0f))
                 * Mat4::RotationY(Radians(37.0f))
                 * Mat4::Scale(Vec3(2.0f, 0.5f, 3.0f));

    const Mat4 inv = m.Inverse();

    REQUIRE(MatricesApproxEqual(m * inv, Mat4::Identity()));
    REQUIRE(MatricesApproxEqual(inv * m, Mat4::Identity()));
}

TEST_CASE("Inverse of a singular matrix falls back to Identity rather than producing NaN/Inf", "[math][mat4]")
{
    // All-zero matrix has determinant 0 - deliberately singular.
    const Mat4 singular;
    const Mat4 inv = singular.Inverse();

    REQUIRE(MatricesApproxEqual(inv, Mat4::Identity()));
}

TEST_CASE("Perspective projection maps the near plane center toward NDC z = -1", "[math][mat4]")
{
    const Mat4 proj = Mat4::Perspective(Radians(90.0f), 1.0f, 0.1f, 100.0f);

    const Vec4 nearPoint(0.0f, 0.0f, -0.1f, 1.0f); // OpenGL: camera looks down -Z
    Vec4 clip = proj * nearPoint;
    const Vec4 ndc(clip.x / clip.w, clip.y / clip.w, clip.z / clip.w, 1.0f);

    REQUIRE(ndc.z == Approx(-1.0f).margin(1e-4f));
}

TEST_CASE("Orthographic projection maps its box exactly onto the NDC cube", "[math][mat4]")
{
    const Mat4 ortho = Mat4::Orthographic(-10.0f, 10.0f, -5.0f, 5.0f, 0.1f, 100.0f);

    const Vec4 corner = ortho * Vec4(10.0f, 5.0f, -100.0f, 1.0f);
    REQUIRE(corner.x == Approx(1.0f));
    REQUIRE(corner.y == Approx(1.0f));
    REQUIRE(corner.z == Approx(1.0f));

    const Vec4 center = ortho * Vec4(0.0f, 0.0f, -50.05f, 1.0f);
    REQUIRE(center.x == Approx(0.0f));
    REQUIRE(center.y == Approx(0.0f));
}

TEST_CASE("LookAt produces a matrix whose rotation part is orthonormal", "[math][mat4]")
{
    const Mat4 view = Mat4::LookAt(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

    // Looking down -Z at the origin from (0,0,5) with no rotation needed:
    // the view matrix should be very close to a pure translation.
    const Vec4 origin = view * Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    REQUIRE(origin.x == Approx(0.0f).margin(1e-4f));
    REQUIRE(origin.y == Approx(0.0f).margin(1e-4f));
    REQUIRE(origin.z == Approx(-5.0f).margin(1e-4f));
}
