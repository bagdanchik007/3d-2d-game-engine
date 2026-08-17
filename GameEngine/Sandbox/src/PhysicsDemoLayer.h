#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Profiler.h"
#include "Engine/Physics/PhysicsWorld.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/PerspectiveCameraController.h"
#include "Engine/Renderer/RenderCommand.h"

namespace Sandbox
{
    /// M10's existence proof: several dynamic boxes with different
    /// restitution values dropped onto a static floor, simulated by
    /// PhysicsWorld and rendered via the same cube mesh + lit shader
    /// pipeline Mesh3DLayer (M8) established - physics doesn't need its
    /// own renderer, it just needs to hand back positions.
    class PhysicsDemoLayer final : public Engine::Layer
    {
    public:
        PhysicsDemoLayer()
            : Layer("PhysicsDemo"), m_CameraController(16.0f / 9.0f)
        {
        }

        void OnAttach() override
        {
            m_CubeMesh = Engine::Mesh::CreateCube();
            m_ShaderHandle = Engine::AssetManager::LoadShader(
                "Lit", "assets/shaders/Lit.vert", "assets/shaders/Lit.frag");

            m_World.SetGravity(Engine::Math::Vec3(0.0f, -9.81f, 0.0f));

            Engine::Physics::RigidBodyDef floorDef;
            floorDef.Position = Engine::Math::Vec3(0.0f, -0.5f, 0.0f);
            floorDef.HalfExtents = Engine::Math::Vec3(5.0f, 0.5f, 5.0f);
            floorDef.IsStatic = true;
            floorDef.Restitution = 0.3f;
            floorDef.Friction = 0.6f;
            m_FloorBody = m_World.CreateBody(floorDef);

            // A handful of boxes with visibly different restitution, so
            // the difference between "thuds and stays" and "bounces
            // several times" is actually observable frame to frame,
            // not just asserted in a unit test.
            constexpr int kBoxCount = 5;
            for (int i = 0; i < kBoxCount; ++i)
            {
                Engine::Physics::RigidBodyDef boxDef;
                boxDef.Position = Engine::Math::Vec3(static_cast<float>(i - kBoxCount / 2) * 1.2f, 4.0f + static_cast<float>(i) * 0.5f, 0.0f);
                boxDef.HalfExtents = Engine::Math::Vec3(0.4f, 0.4f, 0.4f);
                boxDef.Mass = 1.0f;
                boxDef.Restitution = static_cast<float>(i) / static_cast<float>(kBoxCount - 1); // 0.0 .. 1.0 across the row
                boxDef.Friction = 0.5f;
                m_Boxes.push_back(m_World.CreateBody(boxDef));
            }

            m_CameraController.GetCamera().SetPosition(Engine::Math::Vec3(0.0f, 3.0f, 10.0f));
            m_CameraController.SetLookEnabled(true);

            ENGINE_INFO("PhysicsDemoLayer attached - {} dynamic bodies over a static floor", kBoxCount);
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            m_CameraController.OnUpdate(timestep);
            {
                ENGINE_PROFILE_SCOPE("PhysicsWorld::Update");
                m_World.Update(timestep.GetSeconds());
            }

            const auto shader = Engine::AssetManager::GetShader(m_ShaderHandle);
            if (!shader)
            {
                return;
            }

            shader->Bind();
            shader->SetMat4("u_ViewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
            shader->SetFloat3("u_ViewPosition", m_CameraController.GetCamera().GetPosition());
            shader->SetFloat3("u_LightPosition", Engine::Math::Vec3(3.0f, 6.0f, 4.0f));
            shader->SetFloat3("u_LightColor", Engine::Math::Vec3(1.0f, 1.0f, 1.0f));

            // No rotation to account for in the normal matrix: this
            // engine's RigidBody is linear-dynamics-only (see
            // RigidBody.h), so every body's orientation is always
            // identity, and the model matrix is a pure translation - the
            // identity rotation part means the normal matrix IS the
            // identity too, no Inverse().Transpose() needed here (compare
            // Mesh3DLayer/AssetManagerLayer, which both rotate their mesh
            // and therefore do need it).
            shader->SetFloat3("u_NormalMatrixCol0", Engine::Math::Vec3(1.0f, 0.0f, 0.0f));
            shader->SetFloat3("u_NormalMatrixCol1", Engine::Math::Vec3(0.0f, 1.0f, 0.0f));
            shader->SetFloat3("u_NormalMatrixCol2", Engine::Math::Vec3(0.0f, 0.0f, 1.0f));

            DrawBody(*shader, m_FloorBody, Engine::Math::Vec3(0.4f, 0.4f, 0.4f)); // neutral gray - not part of the restitution color-coding below

            for (const Engine::Physics::RigidBody* box : m_Boxes)
            {
                const float restitution = box->GetRestitution();
                // Color encodes restitution (blue = bouncy, red = dead on
                // impact) so the visual difference between boxes lines up
                // with the property actually driving it - purely a debug
                // convenience, not a rendering feature this engine claims
                // more broadly.
                DrawBody(*shader, box, Engine::Math::Vec3(1.0f - restitution, 0.3f, restitution));
            }
        }

        void OnEvent(Engine::Event& event) override
        {
            m_CameraController.OnEvent(event);
        }

    private:
        void DrawBody(Engine::Shader& shader, const Engine::Physics::RigidBody* body, const Engine::Math::Vec3& color) const
        {
            const Engine::Math::Vec3 halfExtents = body->GetHalfExtents();
            const Engine::Math::Mat4 model =
                Engine::Math::Mat4::Translate(body->GetPosition()) * Engine::Math::Mat4::Scale(halfExtents * 2.0f);

            shader.SetFloat3("u_ObjectColor", color);
            shader.SetMat4("u_Model", model);

            Engine::RenderCommand::DrawIndexed(m_CubeMesh->GetVertexArray(), m_CubeMesh->GetIndexCount());
        }

        Engine::PerspectiveCameraController m_CameraController;
        Engine::Physics::PhysicsWorld m_World;
        std::shared_ptr<Engine::Mesh> m_CubeMesh;
        Engine::AssetHandle m_ShaderHandle;
        Engine::Physics::RigidBody* m_FloorBody = nullptr;
        std::vector<Engine::Physics::RigidBody*> m_Boxes;
    };

} // namespace Sandbox
