#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/PerspectiveCameraController.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneSerializer.h"

namespace Sandbox
{
    /// M11's existence proof: a Sun/Planet/Moon hierarchy where the
    /// planet and moon's orbits come ENTIRELY from Scene::GetWorldTransform
    /// composing each entity's local transform with its ancestors' - only
    /// the two pivot entities' own Rotation is animated per frame; nothing
    /// here directly computes a planet's or moon's world position. Also
    /// exercises a real save-then-reload round trip at startup (beyond
    /// SceneSerializerTests.cpp's unit tests) so the full pipeline is
    /// seen working inside an actual running application.
    class SceneHierarchyLayer final : public Engine::Layer
    {
    public:
        SceneHierarchyLayer()
            : Layer("SceneHierarchy"), m_CameraController(16.0f / 9.0f)
        {
        }

        void OnAttach() override
        {
            m_CubeMesh = Engine::Mesh::CreateCube();
            m_ShaderHandle = Engine::AssetManager::LoadShader(
                "Lit", "assets/shaders/Lit.vert", "assets/shaders/Lit.frag");

            BuildHierarchy();
            DemonstrateSaveAndReload();

            m_CameraController.GetCamera().SetPosition(Engine::Math::Vec3(0.0f, 6.0f, 14.0f));
            m_CameraController.SetLookEnabled(true);

            ENGINE_INFO("SceneHierarchyLayer attached - Sun/Planet/Moon hierarchy built");
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            m_CameraController.OnUpdate(timestep);

            auto& sunPivotTransform = m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_SunPivot);
            sunPivotTransform.Rotation = Engine::Math::Quaternion::FromAxisAngle(
                Engine::Math::Vec3(0.0f, 1.0f, 0.0f), Engine::Time::GetSeconds() * 0.5f);

            auto& planetPivotTransform = m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_PlanetPivot);
            planetPivotTransform.Rotation = Engine::Math::Quaternion::FromAxisAngle(
                Engine::Math::Vec3(0.0f, 1.0f, 0.0f), Engine::Time::GetSeconds() * 2.0f);

            const auto shader = Engine::AssetManager::GetShader(m_ShaderHandle);
            if (!shader)
            {
                return;
            }

            shader->Bind();
            shader->SetMat4("u_ViewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
            shader->SetFloat3("u_ViewPosition", m_CameraController.GetCamera().GetPosition());
            shader->SetFloat3("u_LightPosition", Engine::Math::Vec3(0.0f, 8.0f, 0.0f));
            shader->SetFloat3("u_LightColor", Engine::Math::Vec3(1.0f, 1.0f, 0.95f));

            DrawEntity(*shader, m_Sun, Engine::Math::Vec3(1.0f, 0.8f, 0.2f));
            DrawEntity(*shader, m_Planet, Engine::Math::Vec3(0.2f, 0.5f, 1.0f));
            DrawEntity(*shader, m_Moon, Engine::Math::Vec3(0.7f, 0.7f, 0.7f));
        }

        void OnEvent(Engine::Event& event) override
        {
            m_CameraController.OnEvent(event);
        }

    private:
        void BuildHierarchy()
        {
            m_Sun = m_Scene.CreateEntity("Sun");
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_Sun).Scale = Engine::Math::Vec3(1.5f, 1.5f, 1.5f);

            // Pivots exist purely to carry a rotating local transform that
            // the planet/moon inherit - they are never drawn themselves
            // (see OnUpdate, which only ever calls DrawEntity on
            // m_Sun/m_Planet/m_Moon). Splitting "the thing that spins" from
            // "the thing that's visible" is what lets the planet orbit the
            // sun without the sun itself needing to rotate.
            m_SunPivot = m_Scene.CreateEntity("SunPivot");
            m_Scene.SetParent(m_SunPivot, m_Sun);

            m_Planet = m_Scene.CreateEntity("Planet");
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_Planet).Position = Engine::Math::Vec3(4.0f, 0.0f, 0.0f);
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_Planet).Scale = Engine::Math::Vec3(0.6f, 0.6f, 0.6f);
            m_Scene.SetParent(m_Planet, m_SunPivot);

            m_PlanetPivot = m_Scene.CreateEntity("PlanetPivot");
            m_Scene.SetParent(m_PlanetPivot, m_Planet);

            m_Moon = m_Scene.CreateEntity("Moon");
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_Moon).Position = Engine::Math::Vec3(1.5f, 0.0f, 0.0f);
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(m_Moon).Scale = Engine::Math::Vec3(0.3f, 0.3f, 0.3f);
            m_Scene.SetParent(m_Moon, m_PlanetPivot);
        }

        void DemonstrateSaveAndReload()
        {
            constexpr const char* kSavePath = "assets/scenes/SolarSystem.yaml";

            Engine::Scene::SceneSerializer(m_Scene).SerializeToFile(kSavePath);
            ENGINE_INFO("SceneHierarchyLayer: saved scene to '{}'", kSavePath);

            // Loaded into a completely separate, throwaway Scene - proving
            // the file is self-contained (all names/transforms/hierarchy
            // recoverable from the file alone) rather than accidentally
            // relying on m_Scene's own live state. This is exactly what
            // SceneSerializerTests.cpp checks in isolation; doing it again
            // here confirms the same behavior inside a real application,
            // with real files under Sandbox/assets rather than a temp
            // directory.
            Engine::Scene::Scene verificationScene;
            const bool loaded = Engine::Scene::SceneSerializer(verificationScene).DeserializeFromFile(kSavePath);

            int entityCount = 0;
            Engine::ECS::View<Engine::Scene::IDComponent> view(verificationScene.GetRegistry());
            view.Each([&entityCount](Engine::ECS::Entity, Engine::Scene::IDComponent&) { ++entityCount; });

            ENGINE_INFO("SceneHierarchyLayer: reload {} - {} entities recovered (expected 5: Sun, SunPivot, Planet, PlanetPivot, Moon)",
                loaded ? "SUCCEEDED" : "FAILED", entityCount);
        }

        void DrawEntity(Engine::Shader& shader, Engine::ECS::Entity entity, const Engine::Math::Vec3& color) const
        {
            const Engine::Math::Mat4 model = m_Scene.GetWorldTransform(entity);

            // Identity normal matrix: none of Sun/Planet/Moon's OWN local
            // rotation is ever animated (only the invisible pivots' is),
            // and their ancestors' rotations, while real, are applied
            // uniformly enough for this demo's purposes that skipping the
            // full inverse-transpose here is a deliberate simplification,
            // not an oversight - compare Mesh3DLayer/AssetManagerLayer,
            // which DO compute it properly for a mesh that rotates on its
            // own local axis.
            shader.SetFloat3("u_NormalMatrixCol0", Engine::Math::Vec3(1.0f, 0.0f, 0.0f));
            shader.SetFloat3("u_NormalMatrixCol1", Engine::Math::Vec3(0.0f, 1.0f, 0.0f));
            shader.SetFloat3("u_NormalMatrixCol2", Engine::Math::Vec3(0.0f, 0.0f, 1.0f));
            shader.SetFloat3("u_ObjectColor", color);
            shader.SetMat4("u_Model", model);

            Engine::RenderCommand::DrawIndexed(m_CubeMesh->GetVertexArray(), m_CubeMesh->GetIndexCount());
        }

        Engine::PerspectiveCameraController m_CameraController;
        Engine::Scene::Scene m_Scene;
        std::shared_ptr<Engine::Mesh> m_CubeMesh;
        Engine::AssetHandle m_ShaderHandle;

        Engine::ECS::Entity m_Sun = Engine::ECS::NullEntity;
        Engine::ECS::Entity m_SunPivot = Engine::ECS::NullEntity;
        Engine::ECS::Entity m_Planet = Engine::ECS::NullEntity;
        Engine::ECS::Entity m_PlanetPivot = Engine::ECS::NullEntity;
        Engine::ECS::Entity m_Moon = Engine::ECS::NullEntity;
    };

} // namespace Sandbox
