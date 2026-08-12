#pragma once

#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Time.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Renderer/Framebuffer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/PerspectiveCameraController.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "FramebufferBlit.h"

namespace Sandbox
{
    namespace
    {
        constexpr const char* kLitVertexSource = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            layout(location = 2) in vec2 a_TexCoord;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Model;
            uniform vec3 u_NormalMatrixCol0;
            uniform vec3 u_NormalMatrixCol1;
            uniform vec3 u_NormalMatrixCol2;

            out vec3 v_WorldPosition;
            out vec3 v_WorldNormal;

            void main()
            {
                vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
                v_WorldPosition = worldPosition.xyz;

                // Normals need their OWN transform (the inverse-transpose
                // of the model matrix's upper 3x3), not the model matrix
                // itself: a non-uniform Scale distorts a plain-transformed
                // normal so it's no longer perpendicular to the surface it
                // came from. Passed in as three vec3 columns rather than a
                // single mat3 uniform - see Mesh3DLayer::OnUpdate for why -
                // and reassembled here with GLSL's column constructor.
                mat3 normalMatrix = mat3(u_NormalMatrixCol0, u_NormalMatrixCol1, u_NormalMatrixCol2);
                v_WorldNormal = normalize(normalMatrix * a_Normal);

                gl_Position = u_ViewProjection * worldPosition;
            }
        )";

        constexpr const char* kLitFragmentSource = R"(
            #version 450 core
            in vec3 v_WorldPosition;
            in vec3 v_WorldNormal;

            uniform vec3 u_ViewPosition;
            uniform vec3 u_LightPosition;
            uniform vec3 u_LightColor;
            uniform vec3 u_ObjectColor;

            out vec4 o_Color;

            void main()
            {
                vec3 normal = normalize(v_WorldNormal);

                // Ambient: a flat minimum so the unlit side of the cube
                // isn't pure black - physically dubious (real ambient
                // comes from bounced light) but standard for a first
                // lighting pass, and explicitly the "basic" in this
                // milestone's "basic Blinn-Phong" scope.
                vec3 ambient = 0.1 * u_LightColor;

                vec3 lightDir = normalize(u_LightPosition - v_WorldPosition);
                float diffuseStrength = max(dot(normal, lightDir), 0.0);
                vec3 diffuse = diffuseStrength * u_LightColor;

                // Blinn-Phong specular, not Phong: uses the halfway vector
                // between the view and light directions rather than the
                // reflected light vector. Cheaper (no reflect() call) and
                // avoids Phong's specular highlight vanishing at grazing
                // angles where the reflection vector points away from the
                // viewer entirely - the specific reason Blinn-Phong is the
                // one actually named in this milestone's scope.
                vec3 viewDir = normalize(u_ViewPosition - v_WorldPosition);
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
                vec3 specular = 0.5 * specularStrength * u_LightColor;

                vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
                o_Color = vec4(result, 1.0);
            }
        )";
    }

    class Mesh3DLayer final : public Engine::Layer
    {
    public:
        Mesh3DLayer()
            : Layer("Mesh3D"), m_CameraController(16.0f / 9.0f)
        {
        }

        void OnAttach() override
        {
            m_CubeMesh = Engine::Mesh::CreateCube();
            m_LitShader = Engine::Shader::Create(kLitVertexSource, kLitFragmentSource);

            m_Framebuffer = Engine::Framebuffer::Create({1280, 720});
            m_Blit = std::make_unique<FramebufferBlit>();

            m_CameraController.GetCamera().SetPosition(Engine::Math::Vec3(0.0f, 1.0f, 4.0f));
            m_CameraController.SetLookEnabled(true);

            ENGINE_INFO("Mesh3DLayer attached - cube mesh, Blinn-Phong shader, and framebuffer ready");
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            m_CameraController.OnUpdate(timestep);

            m_Framebuffer->Bind();
            Engine::RenderCommand::SetClearColor(Engine::Math::Vec4(0.05f, 0.05f, 0.08f, 1.0f));
            Engine::RenderCommand::Clear();

            const float angle = Engine::Time::GetSeconds();
            const Engine::Math::Mat4 model =
                Engine::Math::Mat4::RotationY(angle) * Engine::Math::Mat4::RotationX(angle * 0.3f);

            // Inverse-transpose of the model matrix's rotation-only upper
            // 3x3, needed for correct normal transformation (see the
            // vertex shader comment above). A pure rotation is already
            // orthonormal - its inverse IS its transpose - so for this
            // specific cube (rotation only, no scale) the "correct"
            // normal matrix is mathematically just the rotation itself.
            // Computing the general inverse-transpose anyway, rather than
            // hand-optimizing for "no scale today", is what keeps this
            // code correct the moment a non-uniform Scale is added to
            // this transform - which is a real near-term possibility, not
            // a hypothetical.
            const Engine::Math::Mat4 normalMatrix4 = model.Inverse().Transpose();

            m_LitShader->Bind();
            m_LitShader->SetMat4("u_ViewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
            m_LitShader->SetMat4("u_Model", model);
            // No Mat3 upload method exists on Shader (SetMat4 is the only
            // matrix uniform type M6/M7 ever needed) - rather than add one
            // for this single call site, the 3x3 rotation part is passed
            // through as vec3 columns via three SetFloat3 calls, matching
            // GLSL's mat3(vec3, vec3, vec3) column-constructor exactly.
            m_LitShader->SetFloat3("u_NormalMatrixCol0", normalMatrix4[0].XYZ());
            m_LitShader->SetFloat3("u_NormalMatrixCol1", normalMatrix4[1].XYZ());
            m_LitShader->SetFloat3("u_NormalMatrixCol2", normalMatrix4[2].XYZ());
            m_LitShader->SetFloat3("u_ViewPosition", m_CameraController.GetCamera().GetPosition());
            m_LitShader->SetFloat3("u_LightPosition", Engine::Math::Vec3(2.0f, 3.0f, 2.0f));
            m_LitShader->SetFloat3("u_LightColor", Engine::Math::Vec3(1.0f, 1.0f, 1.0f));
            m_LitShader->SetFloat3("u_ObjectColor", Engine::Math::Vec3(0.8f, 0.3f, 0.3f));

            Engine::RenderCommand::DrawIndexed(m_CubeMesh->GetVertexArray(), m_CubeMesh->GetIndexCount());

            m_Framebuffer->Unbind();

            // Back on the default framebuffer (the actual window): clear
            // it too, or the blit below would composite over whatever
            // Renderer2D/TriangleLayer left in the color buffer from
            // earlier in this same frame's layer stack.
            Engine::RenderCommand::SetClearColor(Engine::Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
            Engine::RenderCommand::Clear();
            m_Blit->Draw(*m_Framebuffer);
        }

        void OnEvent(Engine::Event& event) override
        {
            m_CameraController.OnEvent(event);
        }

    private:
        Engine::PerspectiveCameraController m_CameraController;
        std::shared_ptr<Engine::Mesh> m_CubeMesh;
        std::shared_ptr<Engine::Shader> m_LitShader;
        std::shared_ptr<Engine::Framebuffer> m_Framebuffer;
        std::unique_ptr<FramebufferBlit> m_Blit;
    };

} // namespace Sandbox
