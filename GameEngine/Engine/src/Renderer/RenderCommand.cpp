#include "Engine/Renderer/RenderCommand.h"

namespace Engine
{
    // Constructed at static-initialization time (before main(), before any
    // GLFW window or OpenGL context exists) - exactly like
    // RenderCommand::s_RendererAPI being an object at all, not something
    // wrong with doing it here. This is only safe because RendererAPI::Create()
    // and OpenGLRendererAPI's constructor do not touch any actual GL call;
    // those only happen in Init(), called explicitly from Application after
    // the window (and its GL context) exist. If that invariant is ever
    // violated, this static initializer is exactly where it would blow up
    // silently before main() even starts.
    std::unique_ptr<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

    void RenderCommand::Init()
    {
        s_RendererAPI->Init();
    }

    void RenderCommand::SetClearColor(const Math::Vec4& color)
    {
        s_RendererAPI->SetClearColor(color);
    }

    void RenderCommand::Clear()
    {
        s_RendererAPI->Clear();
    }

    void RenderCommand::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, std::uint32_t indexCount)
    {
        s_RendererAPI->DrawIndexed(vertexArray, indexCount);
    }

} // namespace Engine
