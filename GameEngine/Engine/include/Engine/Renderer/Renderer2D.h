#pragma once

#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Texture.h"

namespace Engine
{
    /// Batched 2D quad renderer.
    ///
    /// The core idea: every quad, colored or textured, is one draw call's
    /// worth of *data appended to a CPU-side buffer*, not one draw call.
    /// The actual glDrawElements only happens in Flush() - when the batch
    /// fills up, when texture slots run out, or at EndScene(). A scene
    /// with a thousand quads sharing a handful of textures becomes one or
    /// two draw calls instead of a thousand, which is the entire point:
    /// draw call count, not triangle count, is the usual bottleneck for
    /// 2D-style workloads.
    class Renderer2D
    {
    public:
        Renderer2D() = delete;

        static void Init();
        static void Shutdown();

        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();

        static void DrawQuad(const Math::Vec3& position, const Math::Vec2& size, const Math::Vec4& color);
        static void DrawQuad(const Math::Vec3& position, const Math::Vec2& size,
                              const std::shared_ptr<Texture2D>& texture,
                              const Math::Vec4& tintColor = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;
        };

        [[nodiscard]] static Statistics GetStats() noexcept;
        static void ResetStats() noexcept;

    private:
        static void StartBatch();
        static void NextBatch();
        static void Flush();
    };

} // namespace Engine
