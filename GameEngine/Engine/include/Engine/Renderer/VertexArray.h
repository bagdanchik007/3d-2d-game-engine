#pragma once

#include "Engine/Renderer/Buffer.h"

#include <memory>
#include <vector>

namespace Engine
{
    /// Binds one or more VertexBuffers (each with its own BufferLayout) and
    /// one IndexBuffer together into whatever the active graphics API needs
    /// bound to issue a draw call - an OpenGL VAO under the hood today.
    class VertexArray
    {
    public:
        virtual ~VertexArray() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
        virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

        [[nodiscard]] virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
        [[nodiscard]] virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

        [[nodiscard]] static std::shared_ptr<VertexArray> Create();
    };

} // namespace Engine
