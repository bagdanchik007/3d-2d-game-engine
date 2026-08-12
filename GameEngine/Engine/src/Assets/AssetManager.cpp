#include "Engine/Assets/AssetManager.h"

#include "Engine/Core/Log.h"

#include <fstream>
#include <optional>
#include <sstream>

namespace Engine
{
    namespace
    {
        /// Local to this file, not a general-purpose Engine::FileSystem
        /// utility: AssetManager (specifically, LoadShader) is currently
        /// the only caller that needs "read a whole text file into a
        /// string". Promoting this to a shared Core utility is a one-line
        /// change the moment a second caller shows up - not worth doing
        /// speculatively for one use site today.
        std::optional<std::string> ReadTextFile(const std::string& path)
        {
            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file)
            {
                return std::nullopt;
            }

            std::ostringstream contents;
            contents << file.rdbuf();
            return contents.str();
        }
    }

    std::unordered_map<std::string, AssetHandle> AssetManager::s_TexturePathCache;
    std::unordered_map<std::string, AssetHandle> AssetManager::s_MeshPathCache;
    std::unordered_map<std::string, AssetHandle> AssetManager::s_ShaderNameCache;

    std::unordered_map<AssetHandle, std::shared_ptr<Texture2D>> AssetManager::s_Textures;
    std::unordered_map<AssetHandle, std::shared_ptr<Mesh>> AssetManager::s_Meshes;
    std::unordered_map<AssetHandle, std::shared_ptr<Shader>> AssetManager::s_Shaders;

    std::unordered_map<AssetHandle, AssetType> AssetManager::s_HandleTypes;

    AssetHandle AssetManager::LoadTexture2D(const std::string& path)
    {
        if (const auto it = s_TexturePathCache.find(path); it != s_TexturePathCache.end())
        {
            return it->second;
        }

        const auto texture = Texture2D::Create(path);
        // Texture2D::Create(path) asserts internally on a failed stbi_load
        // (see OpenGLTexture.cpp) rather than returning nullptr, so unlike
        // LoadMesh below, there is no failure path to handle here today -
        // texture loading and mesh loading made different error-handling
        // choices at different milestones (M6 vs M9), which is a real
        // inconsistency worth calling out rather than quietly leaving
        // undocumented.
        const AssetHandle handle;

        s_TexturePathCache[path] = handle;
        s_Textures[handle] = texture;
        s_HandleTypes[handle] = AssetType::Texture2D;
        return handle;
    }

    AssetHandle AssetManager::LoadMesh(const std::string& path)
    {
        if (const auto it = s_MeshPathCache.find(path); it != s_MeshPathCache.end())
        {
            return it->second;
        }

        const auto mesh = Mesh::CreateFromFile(path);
        if (!mesh)
        {
            // Mesh::CreateFromFile already logged the specific reason
            // (see Mesh.cpp) - returning a fresh, unregistered AssetHandle
            // here rather than a sentinel/null handle keeps the return
            // type a plain, always-valid-looking UUID (no separate
            // "invalid handle" state to check for everywhere else in the
            // codebase); the handle simply won't resolve to anything via
            // GetMesh, and IsLoaded(handle) reports false for it.
            return AssetHandle();
        }

        const AssetHandle handle;
        s_MeshPathCache[path] = handle;
        s_Meshes[handle] = mesh;
        s_HandleTypes[handle] = AssetType::Mesh;
        return handle;
    }

    AssetHandle AssetManager::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
    {
        if (const auto it = s_ShaderNameCache.find(name); it != s_ShaderNameCache.end())
        {
            return it->second;
        }

        const std::optional<std::string> vertexSource = ReadTextFile(vertexPath);
        const std::optional<std::string> fragmentSource = ReadTextFile(fragmentPath);
        if (!vertexSource.has_value() || !fragmentSource.has_value())
        {
            ENGINE_CORE_ERROR("Failed to load shader '{}': could not read '{}' or '{}'", name, vertexPath, fragmentPath);
            return AssetHandle();
        }

        const auto shader = Shader::Create(*vertexSource, *fragmentSource);

        const AssetHandle handle;
        s_ShaderNameCache[name] = handle;
        s_Shaders[handle] = shader;
        s_HandleTypes[handle] = AssetType::Shader;
        return handle;
    }

    std::shared_ptr<Texture2D> AssetManager::GetTexture2D(AssetHandle handle)
    {
        const auto it = s_Textures.find(handle);
        return it != s_Textures.end() ? it->second : nullptr;
    }

    std::shared_ptr<Mesh> AssetManager::GetMesh(AssetHandle handle)
    {
        const auto it = s_Meshes.find(handle);
        return it != s_Meshes.end() ? it->second : nullptr;
    }

    std::shared_ptr<Shader> AssetManager::GetShader(AssetHandle handle)
    {
        const auto it = s_Shaders.find(handle);
        return it != s_Shaders.end() ? it->second : nullptr;
    }

    AssetType AssetManager::GetAssetType(AssetHandle handle) noexcept
    {
        const auto it = s_HandleTypes.find(handle);
        return it != s_HandleTypes.end() ? it->second : AssetType::None;
    }

    bool AssetManager::IsLoaded(AssetHandle handle) noexcept
    {
        return s_HandleTypes.contains(handle);
    }

    void AssetManager::Clear()
    {
        s_TexturePathCache.clear();
        s_MeshPathCache.clear();
        s_ShaderNameCache.clear();
        s_Textures.clear();
        s_Meshes.clear();
        s_Shaders.clear();
        s_HandleTypes.clear();
    }

} // namespace Engine
