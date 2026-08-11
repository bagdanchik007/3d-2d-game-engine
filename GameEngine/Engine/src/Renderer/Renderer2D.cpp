#include "Engine/Renderer/Renderer2D.h"

#include "Engine/Core/Assert.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/VertexArray.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace Engine
{
    namespace
    {
        struct QuadVertex
        {
            Math::Vec3 Position;
            Math::Vec4 Color;
            Math::Vec2 TexCoord;
            float TexIndex;
        };

        constexpr uint32_t kMaxQuads = 1000;
        constexpr uint32_t kMaxVertices = kMaxQuads * 4;
        constexpr uint32_t kMaxIndices = kMaxQuads * 6;
        // 16, not a value queried from GL_MAX_TEXTURE_IMAGE_UNITS: a
        // conservative constant that is safely within every GL 4.5-capable
        // driver's minimum guaranteed slot count. Querying the real
        // hardware limit would let this scale up on capable hardware, at
        // the cost of the texture slot array's size no longer being a
        // compile-time constant - not worth the complexity for a portfolio
        // 2D renderer that is never going to bind 16 distinct textures in
        // one batch in practice.
        constexpr uint32_t kMaxTextureSlots = 16;

        const char* QuadVertexShaderSource = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;
            layout(location = 2) in vec2 a_TexCoord;
            layout(location = 3) in float a_TexIndex;

            uniform mat4 u_ViewProjection;

            out vec4 v_Color;
            out vec2 v_TexCoord;
            out float v_TexIndex;

            void main()
            {
                v_Color = a_Color;
                v_TexCoord = a_TexCoord;
                v_TexIndex = a_TexIndex;
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            }
        )";

        const char* QuadFragmentShaderSource = R"(
            #version 450 core
            in vec4 v_Color;
            in vec2 v_TexCoord;
            in float v_TexIndex;

            uniform sampler2D u_Textures[16];

            out vec4 o_Color;

            void main()
            {
                // Dynamic array indexing with a non-constant index (an
                // interpolated fragment-shader input, not a literal) is
                // valid in GLSL from #version 400 onward on any hardware
                // targeting OpenGL 4.5 core - this is not relying on an
                // extension.
                o_Color = texture(u_Textures[int(v_TexIndex)], v_TexCoord) * v_Color;
            }
        )";

        struct Renderer2DStorage
        {
            std::shared_ptr<VertexArray> QuadVertexArray;
            std::shared_ptr<VertexBuffer> QuadVertexBuffer;
            std::shared_ptr<Shader> TextureShader;
            std::shared_ptr<Texture2D> WhiteTexture;

            uint32_t QuadIndexCount = 0;
            std::vector<QuadVertex> QuadVertexBufferBase;
            QuadVertex* QuadVertexBufferPtr = nullptr;

            std::array<std::shared_ptr<Texture2D>, kMaxTextureSlots> TextureSlots;
            uint32_t TextureSlotIndex = 1; // slot 0 is reserved for WhiteTexture

            Renderer2D::Statistics Stats;
        };

        // Allocated in Init(), destroyed in Shutdown() - NOT a static
        // object with a constructor that runs at program startup, unlike
        // RenderCommand::s_RendererAPI (see RenderCommand.cpp). Every
        // member here is a GPU resource that must not be constructed
        // before a GL context exists, and this engine's window/context
        // creation happens well after static initialization, at
        // Application-construction time - so ownership is a pointer
        // that starts null and is only ever populated inside Init().
        std::unique_ptr<Renderer2DStorage> s_Data;
    }

    void Renderer2D::Init()
    {
        s_Data = std::make_unique<Renderer2DStorage>();

        s_Data->QuadVertexArray = VertexArray::Create();

        s_Data->QuadVertexBuffer = VertexBuffer::Create(kMaxVertices * sizeof(QuadVertex));
        s_Data->QuadVertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float, "a_TexIndex"},
        });
        s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

        s_Data->QuadVertexBufferBase.resize(kMaxVertices);

        // The index pattern for every quad is identical (0,1,2,2,3,0
        // relative to that quad's own 4 vertices) - precomputed once for
        // the maximum possible quad count, unlike the vertex buffer, which
        // is genuinely rewritten every frame.
        std::vector<uint32_t> quadIndices(kMaxIndices);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < kMaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;
            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;
            offset += 4;
        }
        const auto quadIndexBuffer = IndexBuffer::Create(quadIndices.data(), kMaxIndices);
        s_Data->QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

        uint32_t whiteTexturePixel = 0xffffffff;
        s_Data->WhiteTexture = Texture2D::Create(1, 1);
        s_Data->WhiteTexture->SetData(&whiteTexturePixel, sizeof(uint32_t));

        s_Data->TextureShader = Shader::Create(QuadVertexShaderSource, QuadFragmentShaderSource);
        std::array<int, kMaxTextureSlots> samplers{};
        for (uint32_t i = 0; i < kMaxTextureSlots; ++i)
        {
            samplers[i] = static_cast<int>(i);
        }
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetIntArray("u_Textures", samplers.data(), kMaxTextureSlots);

        s_Data->TextureSlots[0] = s_Data->WhiteTexture;
    }

    void Renderer2D::Shutdown()
    {
        s_Data.reset();
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        Flush();
    }

    void Renderer2D::StartBatch()
    {
        s_Data->QuadIndexCount = 0;
        s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase.data();
        s_Data->TextureSlotIndex = 1;
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::Flush()
    {
        if (s_Data->QuadIndexCount == 0)
        {
            return; // Nothing was drawn since the last flush - a real, not hypothetical, case: EndScene() with an empty scene.
        }

        const auto dataSize = static_cast<uint32_t>(
            (reinterpret_cast<uint8_t*>(s_Data->QuadVertexBufferPtr) - reinterpret_cast<uint8_t*>(s_Data->QuadVertexBufferBase.data())));
        s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase.data(), dataSize);

        for (uint32_t i = 0; i < s_Data->TextureSlotIndex; ++i)
        {
            s_Data->TextureSlots[i]->Bind(i);
        }

        RenderCommand::DrawIndexed(s_Data->QuadVertexArray, s_Data->QuadIndexCount);
        ++s_Data->Stats.DrawCalls;
    }

    void Renderer2D::DrawQuad(const Math::Vec3& position, const Math::Vec2& size, const Math::Vec4& color)
    {
        // Reuses the general DrawQuad(texture) path with slot 0 (the white
        // texture) rather than a separate solid-color code path - see the
        // "one vertex format for everything" note in Renderer2D.h.
        DrawQuad(position, size, s_Data->WhiteTexture, color);
    }

    void Renderer2D::DrawQuad(const Math::Vec3& position, const Math::Vec2& size,
                               const std::shared_ptr<Texture2D>& texture, const Math::Vec4& tintColor)
    {
        if (s_Data->QuadIndexCount >= kMaxIndices)
        {
            NextBatch();
        }

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; ++i)
        {
            if (*s_Data->TextureSlots[i] == *texture)
            {
                textureIndex = static_cast<float>(i);
                break;
            }
        }

        if (textureIndex == 0.0f && texture != s_Data->WhiteTexture)
        {
            if (s_Data->TextureSlotIndex >= kMaxTextureSlots)
            {
                NextBatch();
            }

            textureIndex = static_cast<float>(s_Data->TextureSlotIndex);
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            ++s_Data->TextureSlotIndex;
        }

        const Math::Mat4 transform = Math::Mat4::Translate(position) * Math::Mat4::Scale(Math::Vec3(size.x, size.y, 1.0f));

        constexpr std::array<Math::Vec4, 4> quadVertexPositions = {
            Math::Vec4(-0.5f, -0.5f, 0.0f, 1.0f),
            Math::Vec4(0.5f, -0.5f, 0.0f, 1.0f),
            Math::Vec4(0.5f, 0.5f, 0.0f, 1.0f),
            Math::Vec4(-0.5f, 0.5f, 0.0f, 1.0f),
        };
        constexpr std::array<Math::Vec2, 4> quadTexCoords = {
            Math::Vec2(0.0f, 0.0f), Math::Vec2(1.0f, 0.0f), Math::Vec2(1.0f, 1.0f), Math::Vec2(0.0f, 1.0f),
        };

        for (int i = 0; i < 4; ++i)
        {
            s_Data->QuadVertexBufferPtr->Position = (transform * quadVertexPositions[static_cast<std::size_t>(i)]).XYZ();
            s_Data->QuadVertexBufferPtr->Color = tintColor;
            s_Data->QuadVertexBufferPtr->TexCoord = quadTexCoords[static_cast<std::size_t>(i)];
            s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
            ++s_Data->QuadVertexBufferPtr;
        }

        s_Data->QuadIndexCount += 6;
        ++s_Data->Stats.QuadCount;
    }

    Renderer2D::Statistics Renderer2D::GetStats() noexcept
    {
        return s_Data->Stats;
    }

    void Renderer2D::ResetStats() noexcept
    {
        s_Data->Stats = {};
    }

} // namespace Engine
