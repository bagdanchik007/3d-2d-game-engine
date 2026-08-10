#pragma once

#include "Engine/Core/Assert.h"
#include "Engine/ECS/ComponentPool.h"
#include "Engine/ECS/Entity.h"
#include "Engine/ECS/IComponentPool.h"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine::ECS
{
    /// Owns entity lifecycle (creation, destruction, stale-handle
    /// detection) and a ComponentPool<T> per component type ever used.
    ///
    /// Entity creation/destruction (below) are declared here but defined in
    /// Registry.cpp: unlike the AddComponent/GetComponent family, they
    /// don't depend on a template parameter, so there is no reason to force
    /// every translation unit that touches a Registry to recompile them.
    /// Component-related methods stay inline in this header because they
    /// are templates - GetOrCreatePool<T>, in particular, has to be visible
    /// wherever it's instantiated for a new T.
    class Registry
    {
    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;

        [[nodiscard]] Entity Create();
        void Destroy(Entity entity);
        [[nodiscard]] bool IsValid(Entity entity) const noexcept;

        template <typename T, typename... Args>
        T& AddComponent(Entity entity, Args&&... args)
        {
            ENGINE_CORE_ASSERT(IsValid(entity), "Cannot add a component to an invalid entity");
            return GetOrCreatePool<T>().Emplace(entity, std::forward<Args>(args)...);
        }

        template <typename T>
        void RemoveComponent(Entity entity)
        {
            if (ComponentPool<T>* pool = GetPool<T>())
            {
                pool->Remove(entity);
            }
        }

        template <typename T>
        [[nodiscard]] bool HasComponent(Entity entity) const
        {
            const ComponentPool<T>* pool = GetPool<T>();
            return pool != nullptr && pool->Contains(entity);
        }

        template <typename T>
        [[nodiscard]] T& GetComponent(Entity entity)
        {
            ComponentPool<T>* pool = GetPool<T>();
            ENGINE_CORE_ASSERT(pool != nullptr && pool->Contains(entity), "Entity does not have this component");
            return pool->Get(entity);
        }

        template <typename T>
        [[nodiscard]] const T& GetComponent(Entity entity) const
        {
            const ComponentPool<T>* pool = GetPool<T>();
            ENGINE_CORE_ASSERT(pool != nullptr && pool->Contains(entity), "Entity does not have this component");
            return pool->Get(entity);
        }

        /// Exposed (rather than kept private) specifically for View, which
        /// needs to compare pool sizes and iterate the smallest one without
        /// knowing every component type up front. Everything View needs is
        /// already reachable through the public template methods above -
        /// this does not hand out any capability those don't already
        /// provide, just avoids repeating the type_index lookup through
        /// public Has/Get calls when View already knows exactly which pool
        /// it wants.
        template <typename T>
        [[nodiscard]] ComponentPool<T>* GetPool()
        {
            const auto it = m_Pools.find(std::type_index(typeid(T)));
            if (it == m_Pools.end())
            {
                return nullptr;
            }
            // static_cast, not dynamic_cast: this map's key-to-value
            // mapping is an invariant this class alone maintains (only
            // GetOrCreatePool<T> inserts, always as ComponentPool<T> under
            // key typeid(T)), so the dynamic type is already known here -
            // dynamic_cast's runtime check would be paying for safety
            // against a class of bug (a wrong type under this key) that
            // cannot occur given how the map is populated.
            return static_cast<ComponentPool<T>*>(it->second.get());
        }

        template <typename T>
        [[nodiscard]] const ComponentPool<T>* GetPool() const
        {
            const auto it = m_Pools.find(std::type_index(typeid(T)));
            if (it == m_Pools.end())
            {
                return nullptr;
            }
            return static_cast<const ComponentPool<T>*>(it->second.get());
        }

    private:
        template <typename T>
        ComponentPool<T>& GetOrCreatePool()
        {
            const std::type_index key(typeid(T));
            auto it = m_Pools.find(key);
            if (it == m_Pools.end())
            {
                it = m_Pools.emplace(key, std::make_unique<ComponentPool<T>>()).first;
            }
            return *static_cast<ComponentPool<T>*>(it->second.get());
        }

        std::vector<EntityGeneration> m_Generations;
        std::vector<EntityIndex> m_FreeIndices;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_Pools;
    };

} // namespace Engine::ECS
