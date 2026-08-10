#pragma once

#include "Engine/ECS/Entity.h"

#include <cstddef>
#include <span>

namespace Engine::ECS
{
    /// Non-template base every ComponentPool<T> implements, so Registry can
    /// hold pools of different component types in one container and
    /// perform type-agnostic operations on them (needed for Destroy(),
    /// which must remove an entity from every pool regardless of type, and
    /// for View, which needs to compare pool sizes without knowing what
    /// they store).
    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;

        [[nodiscard]] virtual std::size_t Size() const noexcept = 0;
        [[nodiscard]] virtual bool Contains(Entity entity) const noexcept = 0;
        virtual void Remove(Entity entity) noexcept = 0;

        /// Dense, packed entity list, parallel to the pool's dense component
        /// array. Exposed so View can drive iteration from whichever pool
        /// is smallest without needing to know that pool's component type.
        [[nodiscard]] virtual std::span<const Entity> GetEntities() const noexcept = 0;
    };

} // namespace Engine::ECS
