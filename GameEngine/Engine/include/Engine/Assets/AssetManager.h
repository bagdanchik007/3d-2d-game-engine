#pragma once

#include "Engine/Core/UUID.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"

#include <string>
#include <unordered_map>

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
        static std::unordered_map<std::string, AssetHandle> s_TexturePathCache;
        static std::unordered_map<std::string, AssetHandle> s_MeshPathCache;
        static std::unordered_map<std::string, AssetHandle> s_ShaderNameCache;

        static std::unordered_map<AssetHandle, std::shared_ptr<Texture2D>> s_Textures;
        static std::unordered_map<AssetHandle, std::shared_ptr<Mesh>> s_Meshes;
        static std::unordered_map<AssetHandle, std::shared_ptr<Shader>> s_Shaders;

        static std::unordered_map<AssetHandle, AssetType> s_HandleTypes;
    };

} // namespace Engine
