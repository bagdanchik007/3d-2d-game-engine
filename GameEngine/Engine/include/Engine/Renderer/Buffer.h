#pragma once

#include "Engine/Core/Assert.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
    enum class ShaderDataType
    {
        None = 0,
        Float, Float2, Float3, Float4,
        Mat3, Mat4,
        Int, Int2, Int3, Int4,
        Bool,
    };

    [[nodiscard]] constexpr std::uint32_t ShaderDataTypeSize(ShaderDataType type) noexcept
    {
        switch (type)
        {
            case ShaderDataType::Float:  return 4;
            case ShaderDataType::Float2: return 4 * 2;
            case ShaderDataType::Float3: return 4 * 3;
            case ShaderDataType::Float4: return 4 * 4;
            case ShaderDataType::Mat3:   return 4 * 3 * 3;
            case ShaderDataType::Mat4:   return 4 * 4 * 4;
            case ShaderDataType::Int:    return 4;
            case ShaderDataType::Int2:   return 4 * 2;
            case ShaderDataType::Int3:   return 4 * 3;
            case ShaderDataType::Int4:   return 4 * 4;
            case ShaderDataType::Bool:   return 1;
            case ShaderDataType::None:   return 0;
        }
        return 0;
    }

    /// One named attribute within a vertex (e.g. "a_Position": Float3).
    /// Offset and normalization are filled in by BufferLayout, not by the
    /// caller, because Offset depends on every preceding element in the
    /// same layout - computing it eagerly per-element would just move the
    /// bug-prone bookkeeping onto every call site instead of doing it once.
    struct BufferElement
    {
        std::string Name;
        ShaderDataType Type;
        std::uint32_t Size;
        std::size_t Offset;
        bool Normalized;

        BufferElement(ShaderDataType type, std::string name, bool normalized = false)
            : Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
        {
        }

        /// Number of scalar components glVertexAttribPointer needs (e.g. 3
        /// for Float3) - matrices report the column count and are uploaded
        /// as multiple sequential attribute slots by OpenGLVertexArray,
        /// since OpenGL has no single vertex attribute wider than 4 floats.
        [[nodiscard]] constexpr std::uint32_t GetComponentCount() const noexcept
        {
            switch (Type)
            {
                case ShaderDataType::Float:  return 1;
                case ShaderDataType::Float2: return 2;
                case ShaderDataType::Float3: return 3;
                case ShaderDataType::Float4: return 4;
                case ShaderDataType::Mat3:   return 3;
                case ShaderDataType::Mat4:   return 4;
                case ShaderDataType::Int:    return 1;
                case ShaderDataType::Int2:   return 2;
                case ShaderDataType::Int3:   return 3;
                case ShaderDataType::Int4:   return 4;
                case ShaderDataType::Bool:   return 1;
                case ShaderDataType::None:   return 0;
            }
            return 0;
        }
    };

    /// Describes the layout of one vertex: which attributes it has, in
    /// what order, and (computed here) at what byte offset and stride.
    /// Built once per vertex format and attached to a VertexBuffer, then
    /// read by OpenGLVertexArray to issue the matching glVertexAttribPointer
    /// calls - this is what lets the renderer stay ignorant of any
    /// particular vertex format while still binding it correctly.
    class BufferLayout
    {
    public:
        BufferLayout() = default;

        BufferLayout(std::initializer_list<BufferElement> elements)
            : m_Elements(elements)
        {
            CalculateOffsetsAndStride();
        }

        [[nodiscard]] std::uint32_t GetStride() const noexcept { return m_Stride; }
        [[nodiscard]] const std::vector<BufferElement>& GetElements() const noexcept { return m_Elements; }

        [[nodiscard]] auto begin() const noexcept { return m_Elements.begin(); }
        [[nodiscard]] auto end() const noexcept { return m_Elements.end(); }

    private:
        void CalculateOffsetsAndStride() noexcept
        {
            std::size_t offset = 0;
            m_Stride = 0;
            for (auto& element : m_Elements)
            {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }

        std::vector<BufferElement> m_Elements;
        std::uint32_t m_Stride = 0;
    };

    /// VertexBuffer/IndexBuffer are stored via shared_ptr, unlike Window or
    /// Layer's unique ownership: the same buffer is a legitimate candidate
    /// for reuse across multiple VertexArrays (e.g. several instances of
    /// the same mesh), and neither the renderer nor any one VertexArray is
    /// positioned to be sole owner of GPU data that outlives any single
    /// draw call's setup.
    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void SetLayout(const BufferLayout& layout) = 0;
        [[nodiscard]] virtual const BufferLayout& GetLayout() const = 0;

        [[nodiscard]] static std::shared_ptr<VertexBuffer> Create(const float* vertices, std::uint32_t size);
    };

    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        [[nodiscard]] virtual std::uint32_t GetCount() const = 0;

        [[nodiscard]] static std::shared_ptr<IndexBuffer> Create(const std::uint32_t* indices, std::uint32_t count);
    };

} // namespace Engine
