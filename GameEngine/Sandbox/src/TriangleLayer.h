#pragma once

#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/VertexArray.h"

namespace Sandbox
{
    namespace
    {
        constexpr const char* kVertexSource = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;

            out vec4 v_Color;

            void main()
            {
                v_Color = a_Color;
                gl_Position = vec4(a_Position, 1.0);
            }
        )";

        constexpr const char* kFragmentSource = R"(
            #version 450 core
            in vec4 v_Color;
            out vec4 o_Color;

            void main()
            {
                o_Color = v_Color;
            }
        )";
    }

    /// M6's existence proof: a real compiled shader, GPU-uploaded
    /// vertex/index buffers, and an actual draw call through the RHI
    /// abstraction layer - no camera, no scene submission queue, since
    /// those are M7/M8 scope. Vertex positions are hardcoded directly in
    /// NDC space for exactly that reason.
    class TriangleLayer final : public Engine::Layer
    {
    public:
        TriangleLayer() : Layer("Triangle") {}

        void OnAttach() override
        {
            m_VertexArray = Engine::VertexArray::Create();

            // Interleaved position (3 floats) + color (4 floats) per
            // vertex - one buffer, one BufferLayout describing both
            // attributes, rather than a separate VBO per attribute. Fewer
            // buffer binds per draw call at the cost of the two attributes
            // no longer being independently updatable; with nothing here
            // needing to update position and color at different rates,
            // that cost is purely hypothetical right now.
            constexpr float vertices[] = {
                //  x,     y,    z,    r,    g,    b,    a
                -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            };

            const auto vertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));
            vertexBuffer->SetLayout({
                {Engine::ShaderDataType::Float3, "a_Position"},
                {Engine::ShaderDataType::Float4, "a_Color"},
            });
            m_VertexArray->AddVertexBuffer(vertexBuffer);

            constexpr std::uint32_t indices[] = {0, 1, 2};
            const auto indexBuffer = Engine::IndexBuffer::Create(indices, 3);
            m_VertexArray->SetIndexBuffer(indexBuffer);

            m_Shader = Engine::Shader::Create(kVertexSource, kFragmentSource);

            ENGINE_INFO("TriangleLayer attached - shader compiled, {} vertices uploaded", 3);
        }

        void OnUpdate(Engine::Timestep) override
        {
            Engine::RenderCommand::SetClearColor(Engine::Math::Vec4(0.1f, 0.1f, 0.15f, 1.0f));
            Engine::RenderCommand::Clear();

            m_Shader->Bind();
            Engine::RenderCommand::DrawIndexed(m_VertexArray);
        }

    private:
        std::shared_ptr<Engine::VertexArray> m_VertexArray;
        std::shared_ptr<Engine::Shader> m_Shader;
    };

} // namespace Sandbox
