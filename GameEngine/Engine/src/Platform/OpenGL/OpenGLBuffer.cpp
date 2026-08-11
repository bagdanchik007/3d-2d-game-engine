#include "Engine/Platform/OpenGL/OpenGLBuffer.h"

#include <glad/gl.h>

namespace Engine
{
    // --- VertexBuffer -------------------------------------------------

    OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, std::uint32_t size)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, size, vertices, GL_STATIC_DRAW);
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(std::uint32_t size)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLVertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::SetData(const void* data, std::uint32_t size)
    {
        glNamedBufferSubData(m_RendererID, 0, size, data);
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(const float* vertices, std::uint32_t size)
    {
        return std::make_shared<OpenGLVertexBuffer>(vertices, size);
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(std::uint32_t size)
    {
        return std::make_shared<OpenGLVertexBuffer>(size);
    }

    // --- IndexBuffer ----------------------------------------------------

    OpenGLIndexBuffer::OpenGLIndexBuffer(const std::uint32_t* indices, std::uint32_t count)
        : m_Count(count)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, static_cast<GLsizeiptr>(count) * sizeof(std::uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLIndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void OpenGLIndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::Create(const std::uint32_t* indices, std::uint32_t count)
    {
        return std::make_shared<OpenGLIndexBuffer>(indices, count);
    }

} // namespace Engine
