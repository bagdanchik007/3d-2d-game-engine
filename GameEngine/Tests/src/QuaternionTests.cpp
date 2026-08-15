#include "Engine/Math/MathUtils.h"
#include "Engine/Math/Quaternion.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace Engine::Math;
using Catch::Approx;

namespace
{
    bool VectorsApproxEqual(const Vec3& a, const Vec3& b, float epsilon = 1e-4f)
    {
        return std::fabs(a.x - b.x) <= epsilon && std::fabs(a.y - b.y) <= epsilon && std::fabs(a.z - b.z) <= epsilon;
    }

    /// Reference implementation via full quaternion multiplication
    /// (q * v * q^-1), used to verify RotateVector's optimized closed form
    /// against the textbook definition rather than only against
    /// hand-picked 90-degree cases.
    Vec3 RotateVectorReference(const Quaternion& q, const Vec3& v)
    {
        const Quaternion vAsQuat(v.x, v.y, v.z, 0.0f);
        const Quaternion result = q * vAsQuat * q.Inverse();
        return Vec3(result.x, result.y, result.z);
    }
}

TEST_CASE("Default Quaternion is the identity rotation", "[math][quaternion]")
{
    const Quaternion identity;
    const Vec3 v(1.0f, 2.0f, 3.0f);

    REQUIRE(identity.w == Approx(1.0f));
    REQUIRE(identity.RotateVector(v) == v);
}

TEST_CASE("FromAxisAngle produces a unit quaternion", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), Radians(45.0f));
    REQUIRE(q.Length() == Approx(1.0f));
}

TEST_CASE("90 degree rotation around Z maps X onto Y", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(90.0f));
    const Vec3 result = q.RotateVector(Vec3(1.0f, 0.0f, 0.0f));

    REQUIRE(VectorsApproxEqual(result, Vec3(0.0f, 1.0f, 0.0f)));
}

TEST_CASE("RotateVector's optimized form matches the reference q*v*q^-1 formula", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(1.0f, 1.0f, 0.0f).Normalized(), Radians(73.0f));
    const Vec3 v(3.0f, -1.5f, 2.25f);

    const Vec3 fast = q.RotateVector(v);
    const Vec3 reference = RotateVectorReference(q, v);

    REQUIRE(VectorsApproxEqual(fast, reference));
}

TEST_CASE("Rotation preserves vector length", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.3f, 0.7f, 0.1f).Normalized(), Radians(133.0f));
    const Vec3 v(2.0f, -3.0f, 5.0f);

    const Vec3 rotated = q.RotateVector(v);

    REQUIRE(rotated.Length() == Approx(v.Length()));
}

TEST_CASE("Quaternion composition applies the right-hand rotation first", "[math][quaternion]")
{
    const Quaternion rotZ90 = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(90.0f));
    const Quaternion rotY90 = Quaternion::FromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), Radians(90.0f));

    const Quaternion combined = rotY90 * rotZ90;

    const Vec3 viaComposition = combined.RotateVector(Vec3(1.0f, 0.0f, 0.0f));
    const Vec3 viaSequential = rotY90.RotateVector(rotZ90.RotateVector(Vec3(1.0f, 0.0f, 0.0f)));

    REQUIRE(VectorsApproxEqual(viaComposition, viaSequential));
}

TEST_CASE("Conjugate of a unit quaternion equals its inverse", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(60.0f));

    const Quaternion conjugate = q.Conjugate();
    const Quaternion inverse = q.Inverse();

    REQUIRE(conjugate.x == Approx(inverse.x));
    REQUIRE(conjugate.y == Approx(inverse.y));
    REQUIRE(conjugate.z == Approx(inverse.z));
    REQUIRE(conjugate.w == Approx(inverse.w));
}

TEST_CASE("q * q.Inverse() is the identity rotation", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.4f, 0.4f, 0.4f).Normalized(), Radians(51.0f));
    const Quaternion result = q * q.Inverse();

    REQUIRE(result.w == Approx(1.0f));
    REQUIRE(result.x == Approx(0.0f).margin(1e-5f));
    REQUIRE(result.y == Approx(0.0f).margin(1e-5f));
    REQUIRE(result.z == Approx(0.0f).margin(1e-5f));
}

TEST_CASE("Slerp at t=0 and t=1 returns the endpoints", "[math][quaternion]")
{
    const Quaternion a = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(0.0f));
    const Quaternion b = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(90.0f));

    const Quaternion atStart = Slerp(a, b, 0.0f);
    const Quaternion atEnd = Slerp(a, b, 1.0f);

    REQUIRE(atStart.w == Approx(a.w));
    REQUIRE(atEnd.w == Approx(b.w));
}

TEST_CASE("Slerp at t=0.5 is halfway between two rotations", "[math][quaternion]")
{
    const Quaternion a = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(0.0f));
    const Quaternion b = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(90.0f));

    const Quaternion mid = Slerp(a, b, 0.5f);
    const Vec3 rotated = mid.RotateVector(Vec3(1.0f, 0.0f, 0.0f));

    // Halfway from 0 to 90 degrees around Z is 45 degrees.
    REQUIRE(rotated.x == Approx(std::cos(Radians(45.0f))).margin(1e-4f));
    REQUIRE(rotated.y == Approx(std::sin(Radians(45.0f))).margin(1e-4f));
}

TEST_CASE("Slerp between nearly identical quaternions does not divide by zero", "[math][quaternion]")
{
    const Quaternion a = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(30.0f));
    const Quaternion b = Quaternion::FromAxisAngle(Vec3(0.0f, 0.0f, 1.0f), Radians(30.0001f));

    const Quaternion mid = Slerp(a, b, 0.5f);

    REQUIRE_FALSE(std::isnan(mid.w));
    REQUIRE(mid.Length() == Approx(1.0f));
}

TEST_CASE("ToMat4 produces a rotation matrix consistent with RotateVector", "[math][quaternion]")
{
    const Quaternion q = Quaternion::FromAxisAngle(Vec3(0.2f, 0.9f, 0.1f).Normalized(), Radians(84.0f));
    const Vec3 v(1.5f, -2.0f, 0.5f);

    const Vec3 viaQuaternion = q.RotateVector(v);
    const Vec4 viaMatrix4 = q.ToMat4() * Vec4(v, 0.0f);

    REQUIRE(VectorsApproxEqual(viaQuaternion, viaMatrix4.XYZ()));
}

TEST_CASE("ToAxisAngle round-trips through FromAxisAngle for a general rotation", "[math][quaternion]")
{
    const Vec3 originalAxis = Vec3(1.0f, 2.0f, 3.0f).Normalized();
    const float originalAngle = Radians(73.0f);
    const Quaternion q = Quaternion::FromAxisAngle(originalAxis, originalAngle);

    const auto [axis, angle] = q.ToAxisAngle();

    // Not comparing axis/angle component-by-component against the
    // originals directly: ToAxisAngle's acos-based angle is always in
    // [0, 2*pi], and FromAxisAngle(axis, angle) is equivalent to
    // FromAxisAngle(-axis, -angle) - so the recovered (axis, angle) pair
    // is not guaranteed to be bit-for-bit identical to the input even
    // when it represents the exact same rotation. What must hold, and
    // what's actually checked here, is that reconstructing a quaternion
    // from the recovered pair rotates a test vector identically to the
    // original.
    const Quaternion reconstructed = Quaternion::FromAxisAngle(axis, angle);
    const Vec3 testVector(0.4f, -1.2f, 2.1f);
    const Vec3 original = q.RotateVector(testVector);
    const Vec3 roundTripped = reconstructed.RotateVector(testVector);

    REQUIRE(VectorsApproxEqual(original, roundTripped));
}

TEST_CASE("ToAxisAngle on the identity quaternion returns a zero angle, not NaN", "[math][quaternion]")
{
    const Quaternion identity;
    const auto [axis, angle] = identity.ToAxisAngle();

    REQUIRE_FALSE(std::isnan(axis.x));
    REQUIRE_FALSE(std::isnan(axis.y));
    REQUIRE_FALSE(std::isnan(axis.z));
    REQUIRE(angle == Approx(0.0f).margin(1e-5f));
}

TEST_CASE("ToAxisAngle for a 180 degree rotation recovers the correct axis", "[math][quaternion]")
{
    const Vec3 originalAxis(0.0f, 1.0f, 0.0f);
    const Quaternion q = Quaternion::FromAxisAngle(originalAxis, Radians(180.0f));

    const auto [axis, angle] = q.ToAxisAngle();

    REQUIRE(angle == Approx(Radians(180.0f)).margin(1e-4f));
    // 180 degrees is exactly the case where FromAxisAngle's w component is
    // 0 (cos(90 degrees)) - the sign-ambiguity case the general round-trip
    // test above describes. The axis magnitude/direction (not sign) must
    // still match.
    REQUIRE(std::fabs(axis.y) == Approx(1.0f));
}
