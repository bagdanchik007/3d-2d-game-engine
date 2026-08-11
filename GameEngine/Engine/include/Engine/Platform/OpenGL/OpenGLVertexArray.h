#pragma once

#include "Engine/Renderer/VertexArray.h"

#include <cstdint>

namespace Engine
{
    class OpenGLVertexArray final : public VertexArray
    {
    public:
        OpenGLVertexArray();
        ~OpenGLVertexArray() override;

        OpenGLVertexArray(const OpenGLVertexArray&) = delete;
        OpenGLVertexArray& operator=(const OpenGLVertexArray&) = delete;

        void Bind() const override;
        void Unbind() const override;

        void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

        [[nodiscard]] const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
        [[nodiscard]] const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

    private:
        std::uint32_t m_RendererID = 0;
        std::uint32_t m_NextAttributeSlot = 0; // see AddVertexBuffer: tracks attribute slots across multiple vertex buffers
        std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
    };

} // namespace Engine
