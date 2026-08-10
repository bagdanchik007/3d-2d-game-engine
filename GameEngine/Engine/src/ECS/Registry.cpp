#include "Engine/ECS/Registry.h"

namespace Engine::ECS
{
    Entity Registry::Create()
    {
        if (!m_FreeIndices.empty())
        {
            const EntityIndex index = m_FreeIndices.back();
            m_FreeIndices.pop_back();
            return Entity(index, m_Generations[index]);
        }

        const auto index = static_cast<EntityIndex>(m_Generations.size());
        m_Generations.push_back(0);
        return Entity(index, 0);
    }

    void Registry::Destroy(Entity entity)
    {
        if (!IsValid(entity))
        {
            return;
        }

        // Every pool gets a chance to drop this entity's component,
        // regardless of type - this is exactly the case IComponentPool's
        // type erasure exists for. Cost is one virtual call per
        // *component type that has ever existed in this Registry*, not
        // per component the entity actually has; acceptable because
        // Destroy() is a one-off lifecycle event, never a per-frame,
        // per-entity hot path operation.
        for (auto& [type, pool] : m_Pools)
        {
            pool->Remove(entity);
        }

        const EntityIndex index = entity.Index();
        ++m_Generations[index];
        m_FreeIndices.push_back(index);
    }

    bool Registry::IsValid(Entity entity) const noexcept
    {
        const EntityIndex index = entity.Index();
        return !entity.IsNull() && index < m_Generations.size() && m_Generations[index] == entity.Generation();
    }

} // namespace Engine::ECS
