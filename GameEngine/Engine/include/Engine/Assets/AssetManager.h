#pragma once

#include "Engine/Core/JobSystem.h"
#include "Engine/Core/UUID.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
    /// An asset handle IS a UUID - see UUID.h for why no separate wrapper
    /// type exists.
    using AssetHandle = UUID;

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Mesh,
        Shader,
    };

    /// Handle-based loading and caching for Texture2D, Mesh, and Shader.
    ///
    /// Static facade, same shape as Log/Input/RenderCommand: there is
    /// exactly one asset cache for the process's lifetime, so passing an
    /// AssetManager& through every function that might need to load
    /// something would buy nothing over a facade.
    ///
    /// No common Asset base class backs this - see the M9 architecture
    /// note in the milestone writeup for why retrofitting one onto
    /// Texture2D/Mesh/Shader (each already an independent interface since
    /// M6-M8) was rejected. Instead: one path-keyed cache and one
    /// handle-keyed store PER asset type, plus a single handle->type map
    /// used only to answer "does this handle exist at all" and "what kind
    /// of thing is it" without knowing which specific map to check first -
    /// the same type-tagging trick Registry uses in ECS (M5), scaled down
    /// to three known types instead of an open-ended set of components.
    ///
    /// THREADING CONTRACT (added M14): every method except
    /// LoadTexture2DAsync's background decode step, and
    /// ProcessPendingGPUUploads, must be called from the main thread only -
    /// the caches below (s_TexturePathCache, s_Textures, s_HandleTypes,
    /// etc.) have no locking of their own, exactly as they didn't before
    /// M14. LoadTexture2DAsync's worker-thread job touches NOTHING but the
    /// separate, mutex-protected s_PendingUploads queue - it never reaches
    /// into the unprotected maps. This is a deliberate choice over making
    /// every AssetManager method thread-safe: this engine's GL context is
    /// single-threaded by construction (see OpenGLContext, M6), so actual
    /// texture/mesh/shader creation can only ever happen on the main
    /// thread regardless of how thread-safe the bookkeeping around it is -
    /// locking the synchronous Load*/Get* paths would add real overhead
    /// to callers that never touch a second thread, to protect against a
    /// class of misuse (calling them off the main thread) that would be
    /// wrong for GL-context reasons even if the maps themselves were safe.
    class AssetManager
    {
    public:
        AssetManager() = delete;

        /// Loading the same path twice returns the SAME handle and does
        /// NOT reload from disk - this is the "cache" half of "Asset
        /// manager, Asset handles, Asset cache". Without it, every system
        /// that references "player.png" by path would silently duplicate
        /// GPU texture memory for every reference.
        [[nodiscard]] static AssetHandle LoadTexture2D(const std::string& path);
        [[nodiscard]] static AssetHandle LoadMesh(const std::string& path);
        [[nodiscard]] static AssetHandle LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

        /// Returns a handle IMMEDIATELY, before the file has even been
        /// read. Image decoding (stb_image, pure CPU work with no GL
        /// calls) runs on `jobSystem`; the actual GPU texture is only
        /// created later, on the main thread, by
        /// ProcessPendingGPUUploads(). Until that has happened,
        /// IsLoaded(handle) is false and GetTexture2D(handle) returns
        /// nullptr - exactly the same "not ready yet" contract every
        /// caller of the synchronous Load* functions already has to
        /// handle for a failed load, so no new failure mode is introduced
        /// at the call site, only a new REASON a handle might not resolve
        /// yet (in progress, not failed).
        [[nodiscard]] static AssetHandle LoadTexture2DAsync(const std::string& path, JobSystem& jobSystem);

        /// Must be called once per frame from the main thread (the one
        /// with the current GL context) to actually create GPU textures
        /// for any background decode jobs that finished since the last
        /// call. Callable from Application/Sandbox alongside
        /// RenderCommand::Init(), which established the same "explicit
        /// main-thread call, never automatic inside Application" pattern
        /// back in M6.
        static void ProcessPendingGPUUploads();

        [[nodiscard]] static std::shared_ptr<Texture2D> GetTexture2D(AssetHandle handle);
        [[nodiscard]] static std::shared_ptr<Mesh> GetMesh(AssetHandle handle);
        [[nodiscard]] static std::shared_ptr<Shader> GetShader(AssetHandle handle);

        [[nodiscard]] static AssetType GetAssetType(AssetHandle handle) noexcept;
        [[nodiscard]] static bool IsLoaded(AssetHandle handle) noexcept;

        /// Drops every cached asset. Exists primarily so Tests/ can reset
        /// AssetManager's otherwise-process-lifetime static state between
        /// test cases (see AssetManagerTests.cpp) - production code has no
        /// current need to call this mid-run, since assets are meant to
        /// live for the process's lifetime once loaded.
        static void Clear();

    private:
        struct PendingTextureUpload
        {
            AssetHandle Handle;
            int Width;
            int Height;
            int Channels;
            unsigned char* Pixels; // stbi_image_free'd by ProcessPendingGPUUploads after upload
        };

        static std::unordered_map<std::string, AssetHandle> s_TexturePathCache;
        static std::unordered_map<std::string, AssetHandle> s_MeshPathCache;
        static std::unordered_map<std::string, AssetHandle> s_ShaderNameCache;

        static std::unordered_map<AssetHandle, std::shared_ptr<Texture2D>> s_Textures;
        static std::unordered_map<AssetHandle, std::shared_ptr<Mesh>> s_Meshes;
        static std::unordered_map<AssetHandle, std::shared_ptr<Shader>> s_Shaders;

        static std::unordered_map<AssetHandle, AssetType> s_HandleTypes;

        /// The ONLY AssetManager state a worker thread ever touches -
        /// guarded by its own mutex specifically so the rest of
        /// AssetManager's state (above) can stay lock-free for the
        /// main-thread-only synchronous paths, per this class's threading
        /// contract documented above.
        static std::mutex s_PendingUploadsMutex;
        static std::vector<PendingTextureUpload> s_PendingUploads;
    };

} // namespace Engine
