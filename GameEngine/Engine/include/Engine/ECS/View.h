#pragma once

#include "Engine/ECS/Registry.h"

#include <array>

namespace Engine::ECS
{
    /// Iterates entities that have every component in Components...,
    /// driven by whichever of those components' pools currently has the
    /// fewest entities.
    ///
    /// This is a runtime decision (pool sizes change as the game runs), not
    /// a compile-time one, which is why the loop below is written once and
    /// works no matter which type ends up smallest, rather than generating
    /// a separate code path per possible "smallest" component.
    template <typename... Components>
    class View
    {
    public:
        explicit View(Registry& registry) noexcept
            : m_Registry(registry)
        {
        }

        template <typename Func>
        void Each(Func func)
        {
            IComponentPool* pools[] = {static_cast<IComponentPool*>(m_Registry.template GetPool<Components>())...};

            for (IComponentPool* pool : pools)
            {
                if (pool == nullptr)
                {
                    return; // At least one required component has never been added to any entity - view is empty.
                }
            }

            std::size_t smallestSlot = 0;
            for (std::size_t i = 1; i < sizeof...(Components); ++i)
            {
                if (pools[i]->Size() < pools[smallestSlot]->Size())
                {
                    smallestSlot = i;
                }
            }

            for (Entity entity : pools[smallestSlot]->GetEntities())
            {
                // Re-checks the driving pool too (it is trivially true for
                // that one) - a redundant O(1) sparse lookup, cheaper than
                // special-casing it out for the sake of a marginal saving
                // on a check that is not the bottleneck here.
                if ((m_Registry.template HasComponent<Components>(entity) && ...))
                {
                    func(entity, m_Registry.template GetComponent<Components>(entity)...);
                }
            }
        }

    private:
        Registry& m_Registry;
    };

} // namespace Engine::ECS
