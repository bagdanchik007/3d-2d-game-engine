#pragma once

#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Framebuffer.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/VertexArray.h"

namespace Sandbox
{
    namespace
    {
        constexpr const char* kBlitVertexSource = R"(
            #version 450 core
            layout(location = 0) in vec2 a_Position;
            layout(location = 1) in vec2 a_TexCoord;

            out vec2 v_TexCoord;

            void main()
            {
                v_TexCoord = a_TexCoord;
                gl_Position = vec4(a_Position, 0.0, 1.0);
            }
        )";

        constexpr const char* kBlitFragmentSource = R"(
            #version 450 core
            in vec2 v_TexCoord;
            uniform sampler2D u_Texture;
            out vec4 o_Color;

            void main()
            {
                o_Color = texture(u_Texture, v_TexCoord);
            }
        )";
    }

    /// Displays a Framebuffer's color attachment as a fullscreen quad.
    /// Exactly one quad, never batched, no scene/camera concept - a
    /// genuinely different operation from Renderer2D's sprite batching,
    /// which is why this doesn't route through Renderer2D at all (see the
    /// rationale in Engine/Renderer/Framebuffer.h).
    class FramebufferBlit
    {
    public:
        FramebufferBlit()
        {
            m_Shader = Engine::Shader::Create(kBlitVertexSource, kBlitFragmentSource);

            constexpr float vertices[] = {
                // x,     y,     u,    v
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f, -1.0f, 1.0f, 0.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, 0.0f, 1.0f,
            };
            const auto vertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));
            vertexBuffer->SetLayout({
                {Engine::ShaderDataType::Float2, "a_Position"},
                {Engine::ShaderDataType::Float2, "a_TexCoord"},
            });

            constexpr uint32_t indices[] = {0, 1, 2, 2, 3, 0};
            const auto indexBuffer = Engine::IndexBuffer::Create(indices, 6);

            m_VertexArray = Engine::VertexArray::Create();
            m_VertexArray->AddVertexBuffer(vertexBuffer);
            m_VertexArray->SetIndexBuffer(indexBuffer);
        }

        void Draw(const Engine::Framebuffer& source) const
        {
            source.BindColorAttachment(0);
            m_Shader->Bind();
            m_Shader->SetInt("u_Texture", 0);
            Engine::RenderCommand::DrawIndexed(m_VertexArray);
        }

    private:
        std::shared_ptr<Engine::Shader> m_Shader;
        std::shared_ptr<Engine::VertexArray> m_VertexArray;
    };

} // namespace Sandbox
