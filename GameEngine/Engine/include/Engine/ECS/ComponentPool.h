#pragma once

#include "Engine/Core/Assert.h"
#include "Engine/ECS/IComponentPool.h"

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace Engine::ECS
{
    /// Sparse-set storage for one component type T.
    ///
    /// Three parallel arrays:
    ///   - m_Sparse[entity.Index()]  -> position in the dense arrays, or
    ///     kInvalidDenseIndex if absent. Sized to the largest index ever
    ///     seen, so it can have gaps - that's the "sparse" part, and why
    ///     this is O(1) space in the number of *live* components, not in
    ///     the range of entity indices that happen to exist elsewhere.
    ///   - m_DenseEntities / m_DenseComponents -> tightly packed, no gaps,
    ///     iterated together. This is what makes View iteration cache-
    ///     friendly: walking m_DenseComponents is a linear scan with zero
    ///     wasted work on absent entities, unlike iterating a fixed-size
    ///     array indexed directly by entity index would be.
    ///
    /// Removal is O(1): swap the removed slot with the last dense slot,
    /// then pop_back - the classic sparse-set trick. The cost is that
    /// iteration order is not insertion order and changes on removal;
    /// nothing in this engine currently depends on stable iteration order,
    /// and if something eventually does, that's a signal for a different
    /// data structure for that specific case, not for weakening this one's
    /// O(1) removal for everyone.
    template <typename T>
    class ComponentPool final : public IComponentPool
    {
    public:
        template <typename... Args>
        T& Emplace(Entity entity, Args&&... args)
        {
            ENGINE_CORE_ASSERT(!Contains(entity), "Entity already has this component");

            const EntityIndex index = entity.Index();
            if (index >= m_Sparse.size())
            {
                m_Sparse.resize(static_cast<std::size_t>(index) + 1, kInvalidDenseIndex);
            }

            m_Sparse[index] = static_cast<std::uint32_t>(m_DenseEntities.size());
            m_DenseEntities.push_back(entity);
            return m_DenseComponents.emplace_back(std::forward<Args>(args)...);
        }

        void Remove(Entity entity) noexcept override
        {
            if (!Contains(entity))
            {
                return;
            }

            const std::uint32_t denseIndex = m_Sparse[entity.Index()];
            const std::uint32_t lastDenseIndex = static_cast<std::uint32_t>(m_DenseEntities.size() - 1);
            const Entity lastEntity = m_DenseEntities[lastDenseIndex];

            m_DenseEntities[denseIndex] = lastEntity;
            m_DenseComponents[denseIndex] = std::move(m_DenseComponents[lastDenseIndex]);
            m_Sparse[lastEntity.Index()] = denseIndex;

            m_DenseEntities.pop_back();
            m_DenseComponents.pop_back();
            m_Sparse[entity.Index()] = kInvalidDenseIndex;
        }

        [[nodiscard]] bool Contains(Entity entity) const noexcept override
        {
            const EntityIndex index = entity.Index();
            if (index >= m_Sparse.size() || m_Sparse[index] == kInvalidDenseIndex)
            {
                return false;
            }

            // Comparing the FULL stored Entity (index + generation), not
            // just the index, is what rejects a stale handle: m_Sparse is
            // keyed by raw index alone, so after that index is recycled for
            // a different (newer-generation) entity, an old handle would
            // otherwise alias the new entity's component silently. This
            // check is the sparse-set-level enforcement of the safety
            // Entity.h documents at the handle level.
            return m_DenseEntities[m_Sparse[index]] == entity;
        }

        [[nodiscard]] T& Get(Entity entity) noexcept
        {
            ENGINE_CORE_ASSERT(Contains(entity), "Entity does not have this component");
            return m_DenseComponents[m_Sparse[entity.Index()]];
        }

        [[nodiscard]] const T& Get(Entity entity) const noexcept
        {
            ENGINE_CORE_ASSERT(Contains(entity), "Entity does not have this component");
            return m_DenseComponents[m_Sparse[entity.Index()]];
        }

        [[nodiscard]] std::size_t Size() const noexcept override { return m_DenseEntities.size(); }

        [[nodiscard]] std::span<const Entity> GetEntities() const noexcept override
        {
            return std::span<const Entity>(m_DenseEntities.data(), m_DenseEntities.size());
        }

        [[nodiscard]] std::span<T> GetComponents() noexcept
        {
            return std::span<T>(m_DenseComponents.data(), m_DenseComponents.size());
        }

    private:
        static constexpr std::uint32_t kInvalidDenseIndex = std::numeric_limits<std::uint32_t>::max();

        std::vector<std::uint32_t> m_Sparse;
        std::vector<Entity> m_DenseEntities;
        std::vector<T> m_DenseComponents;
    };

} // namespace Engine::ECS
