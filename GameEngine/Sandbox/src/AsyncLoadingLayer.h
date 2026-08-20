#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Core/JobSystem.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Log.h"
#include "Engine/Renderer/OrthographicCamera.h"
#include "Engine/Renderer/Renderer2D.h"

namespace Sandbox
{
    /// M14's existence proof: AsyncTest.bmp is decoded on a JobSystem
    /// worker thread (pure CPU work, no GL calls) while this layer keeps
    /// rendering every frame without stalling on the load. Once
    /// AssetManager::ProcessPendingGPUUploads (called once per frame from
    /// Sandbox::main, on the main/GL thread) picks up the finished decode
    /// and creates the real GPU texture, this layer's quad switches from
    /// a placeholder gray square to the actual loaded image - a visible,
    /// not just logged, demonstration that the async path works.
    class AsyncLoadingLayer final : public Engine::Layer
    {
    public:
        AsyncLoadingLayer()
            : Layer("AsyncLoading"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_JobSystem(2)
        {
        }

        void OnAttach() override
        {
            m_TextureHandle = Engine::AssetManager::LoadTexture2DAsync("assets/textures/AsyncTest.bmp", m_JobSystem);
            ENGINE_INFO("AsyncLoadingLayer attached - texture decode submitted to a {}-thread JobSystem", m_JobSystem.GetThreadCount());
        }

        void OnUpdate(Engine::Timestep) override
        {
            ++m_FrameCount;

            // Called here, once per frame, because AsyncLoadingLayer is
            // currently the only user of AssetManager's async texture
            // path in this Sandbox - a real application with several
            // layers loading assets asynchronously would want exactly
            // ONE well-known call site for this (an AssetManagerLayer, or
            // an explicit call from Sandbox::main alongside
            // RenderCommand::Init()), not leave it to whichever layer
            // happens to use the async API first. Scoped down
            // deliberately rather than over-engineered for a second
            // caller that doesn't exist yet.
            Engine::AssetManager::ProcessPendingGPUUploads();

            const bool wasLoaded = m_HasLoggedLoadCompletion;
            const bool isLoadedNow = Engine::AssetManager::IsLoaded(m_TextureHandle);
            if (isLoadedNow && !wasLoaded)
            {
                // Logged exactly once, at the frame the async load first
                // becomes visible - not every frame it happens to already
                // be loaded, which would just be noise identical to every
                // other synchronous asset load's single "attached" log line.
                ENGINE_INFO("AsyncLoadingLayer: texture finished loading asynchronously after {} frame(s)", m_FrameCount);
                m_HasLoggedLoadCompletion = true;
            }

            Engine::Renderer2D::BeginScene(m_Camera);

            const auto texture = Engine::AssetManager::GetTexture2D(m_TextureHandle);
            if (texture)
            {
                Engine::Renderer2D::DrawQuad(Engine::Math::Vec3(0.0f, 0.0f, 0.0f), Engine::Math::Vec2(1.0f, 1.0f), texture);
            }
            else
            {
                // Placeholder while the background decode/upload is still
                // in flight - a dark gray square that visibly gets
                // replaced the moment the real texture becomes available,
                // rather than the quad simply not existing (and the scene
                // looking broken) for however many frames loading takes.
                Engine::Renderer2D::DrawQuad(Engine::Math::Vec3(0.0f, 0.0f, 0.0f), Engine::Math::Vec2(1.0f, 1.0f), Engine::Math::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
            }

            Engine::Renderer2D::EndScene();
        }

    private:
        Engine::OrthographicCamera m_Camera;
        Engine::JobSystem m_JobSystem;
        Engine::AssetHandle m_TextureHandle;
        int m_FrameCount = 0;
        bool m_HasLoggedLoadCompletion = false;
    };

} // namespace Sandbox
