#pragma once

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/ECS/View.h"
#include "Engine/Editor/InspectorPanel.h"
#include "Engine/Editor/SceneHierarchyPanel.h"
#include "Engine/Editor/ViewportPanel.h"
#include "Engine/Renderer/Framebuffer.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/PerspectiveCameraController.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Scene/Scene.h"

namespace Sandbox
{
    namespace
    {
        constexpr const char* kEditorVertexSource = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            layout(location = 2) in vec2 a_TexCoord;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Model;

            out vec3 v_Normal;

            void main()
            {
                v_Normal = mat3(u_Model) * a_Normal;
                gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
            }
        )";

        constexpr const char* kEditorFragmentSource = R"(
            #version 450 core
            in vec3 v_Normal;
            uniform vec3 u_Color;
            out vec4 o_Color;

            void main()
            {
                // Flat, unlit shading with a cheap fake "headlamp" term
                // (normal dotted with a fixed up-ish direction) - plenty
                // for an editor's default object color, and deliberately
                // simpler than Mesh3DLayer/AssetManagerLayer's full
                // Blinn-Phong pass (M8/M9): the editor viewport's purpose
                // here is to prove the render-to-texture-then-display
                // pipeline works, not to demonstrate lighting again.
                float shade = 0.5 + 0.5 * max(dot(normalize(v_Normal), normalize(vec3(0.3, 1.0, 0.5))), 0.0);
                o_Color = vec4(u_Color * shade, 1.0);
            }
        )";
    }

    /// M12's existence proof: a real editor window layout (hierarchy,
    /// inspector, viewport) driving a real Scene, with the 3D view
    /// rendered into a Framebuffer and displayed via ViewportPanel exactly
    /// as a real editor would - not a simplified stand-in.
    class EditorLayer final : public Engine::Layer
    {
    public:
        EditorLayer()
            : Layer("EditorLayer"), m_CameraController(16.0f / 9.0f)
            , m_SceneHierarchyPanel(&m_Scene), m_InspectorPanel(&m_Scene)
        {
        }

        void OnAttach() override
        {
            m_Framebuffer = Engine::Framebuffer::Create({1280, 720});
            m_ViewportPanel.SetFramebuffer(m_Framebuffer);

            m_CubeMesh = Engine::Mesh::CreateCube();
            m_Shader = Engine::Shader::Create(kEditorVertexSource, kEditorFragmentSource);

            BuildSampleScene();

            m_CameraController.GetCamera().SetPosition(Engine::Math::Vec3(0.0f, 2.0f, 8.0f));

            ENGINE_INFO("EditorLayer attached - Hierarchy/Inspector/Viewport panels ready");
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            // Camera input is gated on the viewport actually being
            // focused/hovered - see ViewportPanel.h's class doc comment
            // for exactly the bug this avoids (WASD-driven camera
            // movement firing while the user is typing an entity name
            // into InspectorPanel's "Name" field, which contains a 'w').
            const bool viewportActive = m_ViewportPanel.IsFocused() || m_ViewportPanel.IsHovered();
            m_CameraController.SetLookEnabled(viewportActive && Engine::Input::IsMouseButtonPressed(Engine::MouseCode::ButtonRight));
            if (viewportActive)
            {
                m_CameraController.OnUpdate(timestep);
            }

            // Resize the framebuffer to match the viewport panel's
            // reported content size, but only on an actual change (see
            // ViewportPanel::SizeChanged's doc comment) - and only once
            // the size is known to be valid (a fresh panel reports 0x0
            // for a single frame before ImGui lays it out).
            if (m_ViewportPanel.SizeChanged())
            {
                const auto size = m_ViewportPanel.GetSize();
                m_Framebuffer->Resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
                m_CameraController.GetCamera().SetPerspective(size.x / size.y);
            }

            m_Framebuffer->Bind();
            Engine::RenderCommand::SetClearColor(Engine::Math::Vec4(0.12f, 0.12f, 0.15f, 1.0f));
            Engine::RenderCommand::Clear();

            RenderScene();

            m_Framebuffer->Unbind();
        }

        void OnImGuiRender() override
        {
            m_SceneHierarchyPanel.OnImGuiRender();
            m_InspectorPanel.OnImGuiRender(m_SceneHierarchyPanel.GetSelectedEntity());
            m_ViewportPanel.OnImGuiRender();
        }

        void OnEvent(Engine::Event& event) override
        {
            m_CameraController.OnEvent(event);
        }

    private:
        void BuildSampleScene()
        {
            const Engine::ECS::Entity root = m_Scene.CreateEntity("Root");

            const Engine::ECS::Entity cubeA = m_Scene.CreateEntity("Cube A");
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(cubeA).Position = Engine::Math::Vec3(-2.0f, 0.0f, 0.0f);
            m_Scene.SetParent(cubeA, root);

            const Engine::ECS::Entity cubeB = m_Scene.CreateEntity("Cube B");
            m_Scene.GetRegistry().GetComponent<Engine::Scene::TransformComponent>(cubeB).Position = Engine::Math::Vec3(2.0f, 0.0f, 0.0f);
            m_Scene.SetParent(cubeB, root);

            m_Scene.CreateEntity("Unparented Entity"); // demonstrates the hierarchy panel handling multiple roots, not just one tree
        }

        void RenderScene()
        {
            m_Shader->Bind();
            m_Shader->SetMat4("u_ViewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());

            Engine::ECS::View<Engine::Scene::TransformComponent> view(m_Scene.GetRegistry());
            view.Each([this](Engine::ECS::Entity entity, Engine::Scene::TransformComponent&)
            {
                // World transform (parent composed with local), not just
                // the entity's own local TransformComponent - the whole
                // reason BuildSampleScene parents Cube A/B under Root
                // rather than leaving all three as siblings is to make
                // this actually exercise Scene::GetWorldTransform (M11)
                // rather than rendering as if hierarchy didn't exist.
                const Engine::Math::Mat4 model = m_Scene.GetWorldTransform(entity);
                m_Shader->SetMat4("u_Model", model);

                const bool isSelected = entity == m_SceneHierarchyPanel.GetSelectedEntity();
                m_Shader->SetFloat3("u_Color", isSelected
                    ? Engine::Math::Vec3(1.0f, 0.7f, 0.2f)
                    : Engine::Math::Vec3(0.4f, 0.6f, 0.9f));

                Engine::RenderCommand::DrawIndexed(m_CubeMesh->GetVertexArray(), m_CubeMesh->GetIndexCount());
            });
        }

        Engine::PerspectiveCameraController m_CameraController;
        Engine::Scene::Scene m_Scene;
        Engine::Editor::SceneHierarchyPanel m_SceneHierarchyPanel;
        Engine::Editor::InspectorPanel m_InspectorPanel;
        Engine::Editor::ViewportPanel m_ViewportPanel;

        std::shared_ptr<Engine::Framebuffer> m_Framebuffer;
        std::shared_ptr<Engine::Mesh> m_CubeMesh;
        std::shared_ptr<Engine::Shader> m_Shader;
    };

} // namespace Sandbox
