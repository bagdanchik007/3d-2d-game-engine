#include "Engine/Scene/SceneSerializer.h"

#include "Engine/Core/Log.h"
#include "Engine/ECS/View.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace Engine::Scene
{
    namespace
    {
        YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Vec3& v)
        {
            out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
            return out;
        }

        YAML::Emitter& operator<<(YAML::Emitter& out, const Math::Quaternion& q)
        {
            out << YAML::Flow << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
            return out;
        }

        Math::Vec3 ParseVec3(const YAML::Node& node)
        {
            return Math::Vec3(node[0].as<float>(), node[1].as<float>(), node[2].as<float>());
        }

        Math::Quaternion ParseQuaternion(const YAML::Node& node)
        {
            return Math::Quaternion(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>());
        }
    }

    void SceneSerializer::SerializeToFile(const std::string& path) const
    {
        const ECS::Registry& registry = m_Scene.GetRegistry();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        // IDComponent is present on every Scene-created entity
        // unconditionally (see Scene::CreateEntityWithID) - so a
        // View<IDComponent> visits exactly the set of entities this
        // serializer needs to save, with no separate "get all entities"
        // Registry API required (see the note in Scene.h making the same
        // point about FindEntityByID/GetWorldTransform's design).
        ECS::View<IDComponent> view(const_cast<ECS::Registry&>(registry));
        view.Each([&](ECS::Entity entity, IDComponent& idComponent)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Entity" << YAML::Value << idComponent.ID.Value();

            if (registry.HasComponent<TagComponent>(entity))
            {
                out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Name" << YAML::Value << registry.GetComponent<TagComponent>(entity).Name;
                out << YAML::EndMap;
            }

            if (registry.HasComponent<TransformComponent>(entity))
            {
                const auto& transform = registry.GetComponent<TransformComponent>(entity);
                out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Position" << YAML::Value << transform.Position;
                out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
                out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
                out << YAML::EndMap;
            }

            // Only Parent is written, never Children - see IDComponent's
            // doc comment / Scene.h for why Children is reconstructed from
            // Parent references on load rather than treated as
            // independently serialized truth.
            const ECS::Entity parent = m_Scene.GetParent(entity);
            if (!parent.IsNull() && registry.HasComponent<IDComponent>(parent))
            {
                out << YAML::Key << "RelationshipComponent" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Parent" << YAML::Value << registry.GetComponent<IDComponent>(parent).ID.Value();
                out << YAML::EndMap;
            }

            out << YAML::EndMap;
        });

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream file(path);
        file << out.c_str();
    }

    bool SceneSerializer::DeserializeFromFile(const std::string& path) const
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path);
        }
        catch (const YAML::Exception& e)
        {
            ENGINE_CORE_ERROR("Failed to load scene '{}': {}", path, e.what());
            return false;
        }

        if (!data["Scene"] || !data["Entities"])
        {
            ENGINE_CORE_ERROR("Failed to load scene '{}': missing top-level 'Scene' or 'Entities' key", path);
            return false;
        }

        try
        {
            // Pass 1: create every entity with its ORIGINAL UUID (see
            // Scene::CreateEntityWithID) and every non-relationship
            // component. Parent references are deliberately NOT resolved
            // in this pass: a child can appear in the file before its
            // parent, and YAML's sequence order is whatever this file
            // happens to have been written or hand-edited in - nothing
            // about the format guarantees parents precede children.
            for (const auto& entityNode : data["Entities"])
            {
                const auto uuidValue = entityNode["Entity"].as<uint64_t>();
                const UUID uuid(uuidValue);

                std::string name = "Entity";
                if (entityNode["TagComponent"])
                {
                    name = entityNode["TagComponent"]["Name"].as<std::string>();
                }

                const ECS::Entity entity = m_Scene.CreateEntityWithID(uuid, name);

                if (entityNode["TransformComponent"])
                {
                    const auto transformNode = entityNode["TransformComponent"];
                    auto& transform = m_Scene.GetRegistry().GetComponent<TransformComponent>(entity);
                    transform.Position = ParseVec3(transformNode["Position"]);
                    transform.Rotation = ParseQuaternion(transformNode["Rotation"]);
                    transform.Scale = ParseVec3(transformNode["Scale"]);
                }
            }

            // Pass 2: now that every entity exists, resolve each child's
            // Parent UUID (re-reading the file, not any in-memory
            // structure carried over from pass 1) via FindEntityByID and
            // wire up the hierarchy through Scene::SetParent - which is
            // also where cycle-safety is enforced (see Scene.h), so a
            // corrupted or hand-edited file that describes a parenting
            // cycle is rejected there rather than infinite-looping here.
            for (const auto& entityNode : data["Entities"])
            {
                if (!entityNode["RelationshipComponent"] || !entityNode["RelationshipComponent"]["Parent"])
                {
                    continue;
                }

                const auto childUUID = UUID(entityNode["Entity"].as<uint64_t>());
                const auto parentUUID = UUID(entityNode["RelationshipComponent"]["Parent"].as<uint64_t>());

                const ECS::Entity child = m_Scene.FindEntityByID(childUUID);
                const ECS::Entity parent = m_Scene.FindEntityByID(parentUUID);

                if (child.IsNull() || parent.IsNull())
                {
                    ENGINE_CORE_WARN("Scene '{}': could not resolve a parent/child UUID reference - skipped", path);
                    continue;
                }

                m_Scene.SetParent(child, parent);
            }
        }
        catch (const YAML::Exception& e)
        {
            ENGINE_CORE_ERROR("Failed to parse scene '{}': malformed entity data ({})", path, e.what());
            return false;
        }

        return true;
    }

} // namespace Engine::Scene
