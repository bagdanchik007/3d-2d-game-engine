#include "Engine/Platform/OpenGL/OpenGLVertexArray.h"

#include "Engine/Core/Assert.h"

#include <glad/gl.h>

namespace Engine
{
    namespace
    {
        GLenum ToOpenGLBaseType(ShaderDataType type)
        {
            switch (type)
            {
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4:
                    return GL_FLOAT;
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:
                    return GL_INT;
                case ShaderDataType::Bool:
                    return GL_BOOL;
                case ShaderDataType::None:
                    break;
            }
            ENGINE_CORE_ASSERT(false, "Unknown ShaderDataType");
            return 0;
        }
    }

    OpenGLVertexArray::OpenGLVertexArray()
    {
        glCreateVertexArrays(1, &m_RendererID);
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void OpenGLVertexArray::Bind() const
    {
        glBindVertexArray(m_RendererID);
    }

    void OpenGLVertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
    {
        ENGINE_CORE_ASSERT(!vertexBuffer->GetLayout().GetElements().empty(), "VertexBuffer has no layout set");

        glBindVertexArray(m_RendererID);
        vertexBuffer->Bind();

        const auto& layout = vertexBuffer->GetLayout();
        for (const auto& element : layout)
        {
            const std::uint32_t componentCount = element.GetComponentCount();

            // A matrix attribute (Mat3/Mat4) needs one attribute slot per
            // column - OpenGL has no single vertex attribute wider than 4
            // floats, so a Mat4 becomes 4 consecutive Float4 slots, each
            // offset by one column's worth of bytes. Every other type takes
            // exactly one slot.
            const std::uint32_t columnCount =
                (element.Type == ShaderDataType::Mat3 || element.Type == ShaderDataType::Mat4) ? componentCount : 1;
            const std::size_t columnByteSize = static_cast<std::size_t>(componentCount) * sizeof(float);

            for (std::uint32_t column = 0; column < columnCount; ++column)
            {
                glEnableVertexAttribArray(m_NextAttributeSlot);
                glVertexAttribPointer(
                    m_NextAttributeSlot,
                    static_cast<GLint>(componentCount),
                    ToOpenGLBaseType(element.Type),
                    element.Normalized ? GL_TRUE : GL_FALSE,
                    static_cast<GLsizei>(layout.GetStride()),
                    reinterpret_cast<const void*>(element.Offset + column * columnByteSize)); // NOLINT(performance-no-int-to-ptr)
                ++m_NextAttributeSlot;
            }
        }

        m_VertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
    {
        // No DSA glVertexArrayElementBuffer here: that would need the index
        // buffer's raw GL name, which IndexBuffer deliberately does not
        // expose (see OpenGLBuffer.h). Binding the VAO and then calling the
        // buffer's own Bind() achieves the same result - GL_ELEMENT_ARRAY_BUFFER
        // binding is itself part of the currently-bound VAO's state - while
        // keeping the buffer's internal ID private to its own class.
        glBindVertexArray(m_RendererID);
        indexBuffer->Bind();
        m_IndexBuffer = indexBuffer;
    }

    std::shared_ptr<VertexArray> VertexArray::Create()
    {
        return std::make_shared<OpenGLVertexArray>();
    }

} // namespace Engine
