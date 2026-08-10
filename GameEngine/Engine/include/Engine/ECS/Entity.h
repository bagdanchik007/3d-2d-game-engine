#pragma once

#include <cstdint>
#include <limits>

namespace Engine::ECS
{
    using EntityIndex = std::uint32_t;
    using EntityGeneration = std::uint32_t;

    /// A lightweight, trivially-copyable handle to an entity: an index into
    /// Registry's internal arrays, plus a generation counter.
    ///
    /// The generation exists to catch stale handles: when an entity is
    /// destroyed, its index is recycled for a future Create() call, but the
    /// generation at that index is incremented first. An old Entity value
    /// still holding the previous generation will therefore compare unequal
    /// to the live entity now occupying that index, and Registry::IsValid()
    /// (and every sparse-set lookup - see ComponentPool.h) rejects it. Without
    /// this, a dangling Entity from before a Destroy() call could silently
    /// read or write a completely unrelated entity's components after index
    /// reuse - the packed generation is what turns that into a detectable
    /// error instead of silent data corruption.
    class Entity
    {
    public:
        constexpr Entity() noexcept = default;

        [[nodiscard]] constexpr EntityIndex Index() const noexcept { return m_Index; }
        [[nodiscard]] constexpr EntityGeneration Generation() const noexcept { return m_Generation; }
        [[nodiscard]] constexpr bool IsNull() const noexcept { return m_Index == kInvalidIndex; }

        [[nodiscard]] constexpr bool operator==(const Entity&) const noexcept = default;

    private:
        // Only Registry may construct a non-null Entity - it is the sole
        // authority on which (index, generation) pairs are currently alive.
        friend class Registry;
        constexpr Entity(EntityIndex index, EntityGeneration generation) noexcept
            : m_Index(index), m_Generation(generation)
        {
        }

        static constexpr EntityIndex kInvalidIndex = std::numeric_limits<EntityIndex>::max();

        EntityIndex m_Index = kInvalidIndex;
        EntityGeneration m_Generation = 0;
    };

    inline constexpr Entity NullEntity{};

} // namespace Engine::ECS
