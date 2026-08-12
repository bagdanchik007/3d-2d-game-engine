#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"
#include "Engine/Renderer/PerspectiveCameraController.h"
#include "Engine/Renderer/RenderCommand.h"

namespace Sandbox
{
    /// M9's existence proof: a shader and a model loaded from real files
    /// on disk through AssetManager, not the inline GLSL strings and
    /// procedural Mesh::CreateCube() every earlier layer uses. Also
    /// demonstrates the cache half of "Asset manager, Asset handles,
    /// Asset cache" by loading the same shader path twice and confirming
    /// the second call is free.
    class AssetManagerLayer final : public Engine::Layer
    {
    public:
        AssetManagerLayer()
            : Layer("AssetManager"), m_CameraController(16.0f / 9.0f)
        {
        }

        void OnAttach() override
        {
            const Engine::AssetHandle shaderHandle = Engine::AssetManager::LoadShader(
                "Lit", "assets/shaders/Lit.vert", "assets/shaders/Lit.frag");

            // Loading the identical name again must hit the cache, not
            // recompile the shader - this is the entire point of
            // AssetManager existing rather than every caller just calling
            // Shader::Create directly. Verified here at runtime, not just
            // asserted in a unit test, because this specific behavior
            // depends on the real file-loading path (see
            // Tests/src/AssetManagerTests.cpp for why that path isn't
            // covered by the headless test suite).
            const Engine::AssetHandle secondLoadHandle = Engine::AssetManager::LoadShader(
                "Lit", "assets/shaders/Lit.vert", "assets/shaders/Lit.frag");
            ENGINE_INFO("AssetManagerLayer: shader cache {} (handle unchanged: {})",
                (shaderHandle == secondLoadHandle) ? "HIT as expected" : "MISS - BUG", shaderHandle == secondLoadHandle);

            m_ShaderHandle = shaderHandle;
            m_MeshHandle = Engine::AssetManager::LoadMesh("assets/models/Pyramid.obj");

            if (!Engine::AssetManager::IsLoaded(m_MeshHandle))
            {
                ENGINE_ERROR("AssetManagerLayer: pyramid model failed to load - see the Mesh loading error above");
            }

            m_CameraController.GetCamera().SetPosition(Engine::Math::Vec3(0.0f, 1.5f, 4.0f));
            m_CameraController.SetLookEnabled(true);

            ENGINE_INFO("AssetManagerLayer attached - shader and mesh loaded from real files on disk");
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            m_CameraController.OnUpdate(timestep);

            const auto shader = Engine::AssetManager::GetShader(m_ShaderHandle);
            const auto mesh = Engine::AssetManager::GetMesh(m_MeshHandle);
            if (!shader || !mesh)
            {
                return; // Already logged the specific failure in OnAttach; nothing to draw.
            }

            const Engine::Math::Mat4 model = Engine::Math::Mat4::RotationY(Engine::Time::GetSeconds() * 0.5f);
            const Engine::Math::Mat4 normalMatrix4 = model.Inverse().Transpose();

            shader->Bind();
            shader->SetMat4("u_ViewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
            shader->SetMat4("u_Model", model);
            shader->SetFloat3("u_NormalMatrixCol0", normalMatrix4[0].XYZ());
            shader->SetFloat3("u_NormalMatrixCol1", normalMatrix4[1].XYZ());
            shader->SetFloat3("u_NormalMatrixCol2", normalMatrix4[2].XYZ());
            shader->SetFloat3("u_ViewPosition", m_CameraController.GetCamera().GetPosition());
            shader->SetFloat3("u_LightPosition", Engine::Math::Vec3(2.0f, 3.0f, 2.0f));
            shader->SetFloat3("u_LightColor", Engine::Math::Vec3(1.0f, 1.0f, 1.0f));
            shader->SetFloat3("u_ObjectColor", Engine::Math::Vec3(0.3f, 0.6f, 0.8f));

            Engine::RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());
        }

        void OnEvent(Engine::Event& event) override
        {
            m_CameraController.OnEvent(event);
        }

    private:
        Engine::PerspectiveCameraController m_CameraController;
        Engine::AssetHandle m_ShaderHandle;
        Engine::AssetHandle m_MeshHandle;
    };

} // namespace Sandbox
