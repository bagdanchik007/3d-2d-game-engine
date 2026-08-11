#include "Engine/Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/gl.h>

namespace Engine
{
    void OpenGLRendererAPI::Init()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::SetClearColor(const Math::Vec4& color)
    {
        glClearColor(color.x, color.y, color.z, color.w);
    }

    void OpenGLRendererAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, std::uint32_t indexCount)
    {
        const std::uint32_t count = (indexCount != 0) ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr);
    }

    std::unique_ptr<RendererAPI> RendererAPI::Create()
    {
        return std::make_unique<OpenGLRendererAPI>();
    }

} // namespace Engine
