#pragma once

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Renderer2D.h"

#include <vector>

namespace Sandbox
{
    /// M7's existence proof: a batched scene of colored quads plus one
    /// procedurally generated checkerboard texture, all drawn through
    /// Renderer2D, with a simple camera pan driven by Input polling (not
    /// a full M8 camera controller - just enough to prove the view-
    /// projection matrix is actually wired up correctly).
    class Renderer2DLayer final : public Engine::Layer
    {
    public:
        Renderer2DLayer()
            : Layer("Renderer2D"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
        {
        }

        void OnAttach() override
        {
            constexpr uint32_t kTextureSize = 8;
            std::vector<uint32_t> pixels(kTextureSize * kTextureSize);
            for (uint32_t y = 0; y < kTextureSize; ++y)
            {
                for (uint32_t x = 0; x < kTextureSize; ++x)
                {
                    const bool isLight = ((x + y) % 2) == 0;
                    pixels[y * kTextureSize + x] = isLight ? 0xffcccccc : 0xff333333; // grayscale, so RGBA vs byte-order doesn't matter here
                }
            }

            m_CheckerboardTexture = Engine::Texture2D::Create(kTextureSize, kTextureSize);
            m_CheckerboardTexture->SetData(pixels.data(), static_cast<uint32_t>(pixels.size() * sizeof(uint32_t)));

            ENGINE_INFO("Renderer2DLayer attached - {}x{} checkerboard texture generated", kTextureSize, kTextureSize);
        }

        void OnUpdate(Engine::Timestep timestep) override
        {
            constexpr float kCameraSpeed = 1.0f;
            const float moveDistance = kCameraSpeed * timestep.GetSeconds();

            Engine::Math::Vec3 cameraPosition = m_Camera.GetPosition();
            if (Engine::Input::IsKeyPressed(Engine::KeyCode::Left)) { cameraPosition.x -= moveDistance; }
            if (Engine::Input::IsKeyPressed(Engine::KeyCode::Right)) { cameraPosition.x += moveDistance; }
            if (Engine::Input::IsKeyPressed(Engine::KeyCode::Up)) { cameraPosition.y += moveDistance; }
            if (Engine::Input::IsKeyPressed(Engine::KeyCode::Down)) { cameraPosition.y -= moveDistance; }
            m_Camera.SetPosition(cameraPosition);

            Engine::Renderer2D::ResetStats();
            Engine::Renderer2D::BeginScene(m_Camera);

            // A small grid of solid-color quads...
            for (int y = -2; y <= 2; ++y)
            {
                for (int x = -2; x <= 2; ++x)
                {
                    const Engine::Math::Vec4 color(
                        (static_cast<float>(x) + 2.0f) / 4.0f, (static_cast<float>(y) + 2.0f) / 4.0f, 0.5f, 1.0f);
                    Engine::Renderer2D::DrawQuad(
                        Engine::Math::Vec3(static_cast<float>(x) * 0.3f, static_cast<float>(y) * 0.3f, 0.0f),
                        Engine::Math::Vec2(0.28f, 0.28f), color);
                }
            }

            // ...plus one textured quad, proving the same batch handles
            // both paths (and, with the grid above also in flight, that
            // texture-slot 0 staying reserved for the white texture
            // doesn't collide with a real bound texture).
            Engine::Renderer2D::DrawQuad(
                Engine::Math::Vec3(0.0f, 0.0f, 0.1f), Engine::Math::Vec2(1.0f, 1.0f), m_CheckerboardTexture);

            Engine::Renderer2D::EndScene();

            constexpr int kLogEveryNFrames = 60;
            ++m_FrameCount;
            if (m_FrameCount % kLogEveryNFrames == 1)
            {
                const auto stats = Engine::Renderer2D::GetStats();
                ENGINE_INFO("Renderer2D stats: {} draw call(s), {} quad(s)", stats.DrawCalls, stats.QuadCount);
            }
        }

    private:
        Engine::OrthographicCamera m_Camera;
        std::shared_ptr<Engine::Texture2D> m_CheckerboardTexture;
        int m_FrameCount = 0;
    };

} // namespace Sandbox
