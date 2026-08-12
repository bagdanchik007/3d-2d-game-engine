#pragma once

#include <cstdint>
#include <functional>
#include <random>

namespace Engine
{
    /// A randomly generated 64-bit identifier.
    ///
    /// Used as-is for AssetHandle (see Assets/AssetManager.h) rather than
    /// wrapped in yet another named type: an asset handle IS a UUID
    /// conceptually, and a same-named alias adds no safety a distinct
    /// class wouldn't already lack here (nothing stops mixing up two
    /// UUID-based handles of different intended meaning either way,
    /// exactly like Entity's index/generation packing doesn't stop mixing
    /// up two Entity values from different Registries - see Entity.h).
    ///
    /// 64 bits of randomness, not a sequential counter: a counter would
    /// need a single global source of truth for "the next ID", which is
    /// exactly the kind of hidden global state this project otherwise
    /// avoids (see LayerStack/Application's ownership-based design).
    /// Random collision odds at 64 bits are astronomically low for any
    /// realistic number of assets a single project will ever load.
    class UUID
    {
    public:
        UUID() noexcept : m_Value(GenerateRandom()) {}
        explicit constexpr UUID(uint64_t value) noexcept : m_Value(value) {}

        [[nodiscard]] constexpr uint64_t Value() const noexcept { return m_Value; }

        [[nodiscard]] constexpr bool operator==(const UUID&) const noexcept = default;

    private:
        static uint64_t GenerateRandom() noexcept
        {
            // A single engine-lifetime-static generator, not one
            // constructed per UUID: std::mt19937_64 has substantial seed
            // state (2.5kB) that would be wasteful to reconstruct on every
            // UUID() call, and re-seeding per call from the same clock-based
            // source repeatedly risks correlated, non-random-looking output
            // for UUIDs created in rapid succession within the same
            // millisecond - a real risk for asset loading, which often
            // creates several handles back-to-back in one function call.
            static std::mt19937_64 generator(std::random_device{}());
            static std::uniform_int_distribution<uint64_t> distribution;
            return distribution(generator);
        }

        uint64_t m_Value;
    };

} // namespace Engine

template <>
struct std::hash<Engine::UUID>
{
    std::size_t operator()(const Engine::UUID& uuid) const noexcept
    {
        return std::hash<uint64_t>()(uuid.Value());
    }
};
