#pragma once

#include "Engine/Math/Math.h"

#include <string>
#include <utility>

namespace Engine::Scene
{
    /// Local (not yet hierarchical - that's M7's "Transform hierarchy")
    /// position/rotation/scale, with the composed matrix computed on
    /// demand rather than cached. Caching would mean tracking a dirty flag
    /// and invalidating it correctly from every mutation site; with no
    /// profiling evidence yet that recomputing this per-access is a
    /// bottleneck, that complexity isn't justified today.
    struct TransformComponent
    {
        Math::Vec3 Position{0.0f, 0.0f, 0.0f};
        Math::Quaternion Rotation{};
        Math::Vec3 Scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] Math::Mat4 GetLocalMatrix() const noexcept
        {
            return Math::Mat4::Translate(Position) * Rotation.ToMat4() * Math::Mat4::Scale(Scale);
        }
    };

    /// Human-readable identifier, mainly for the future editor's hierarchy
    /// panel (M8) and debug logging - never used for entity lookup, which
    /// stays Entity-handle-based throughout the engine.
    struct TagComponent
    {
        std::string Name;

        TagComponent() = default;
        explicit TagComponent(std::string name) noexcept : Name(std::move(name)) {}
    };

} // namespace Engine::Scene
