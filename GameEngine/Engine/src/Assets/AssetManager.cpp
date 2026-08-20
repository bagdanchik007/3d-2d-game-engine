#include "Engine/Assets/AssetManager.h"

#include "Engine/Core/Log.h"

#include <stb_image/stb_image.h>

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

    std::mutex AssetManager::s_PendingUploadsMutex;
    std::vector<AssetManager::PendingTextureUpload> AssetManager::s_PendingUploads;

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

    AssetHandle AssetManager::LoadTexture2DAsync(const std::string& path, JobSystem& jobSystem)
    {
        // Same de-duplication as the synchronous LoadTexture2D: registering
        // the handle in s_TexturePathCache immediately (before the
        // background decode even starts) means a second call with the
        // same path - synchronous or async - hits this cache and returns
        // the SAME handle instead of submitting a duplicate decode job.
        // The handle isn't usable yet (IsLoaded is still false, since
        // s_HandleTypes isn't touched until upload completes), but it's
        // already reserved.
        if (const auto it = s_TexturePathCache.find(path); it != s_TexturePathCache.end())
        {
            return it->second;
        }

        const AssetHandle handle;
        s_TexturePathCache[path] = handle;

        // The job submitted here touches ONLY local variables and the
        // mutex-protected s_PendingUploads queue - never s_Textures,
        // s_HandleTypes, or any other AssetManager map. That boundary is
        // the entire thread-safety argument for this class (see the
        // THREADING CONTRACT note in AssetManager.h): a worker thread
        // executing this lambda has no way to touch state that the main
        // thread's synchronous methods also read/write without locking.
        jobSystem.Submit([path, handle]()
        {
            int width = 0;
            int height = 0;
            int sourceChannels = 0;

            // _thread, not the plain global variant: stb_image's global
            // flip flag has no synchronization of its own, so calling the
            // non-thread-local setter here would be a genuine data race
            // against the main thread's OWN synchronous Texture2D::Create
            // path (OpenGLTexture.cpp), which sets the same flag whenever
            // it loads a texture - both would very likely be setting it to
            // the same value (1), but a data race is undefined behavior
            // regardless of whether the racing writes agree.
            stbi_set_flip_vertically_on_load_thread(1);

            // Forced to 4 (RGBA), not the source image's native channel
            // count: Texture2D::Create(width, height) - used below in
            // ProcessPendingGPUUploads - hardcodes GL_RGBA/4 bytes per
            // pixel (see OpenGLTexture2D's empty-texture constructor,
            // M7), unlike Texture2D::Create(path)'s own file-loading
            // constructor, which inspects the real channel count and
            // picks GL_RGB or GL_RGBA accordingly. A 3-channel RGB source
            // (any JPEG without an alpha channel - the common case, not
            // an edge case) would otherwise produce byteCount = w*h*3
            // while SetData's internal assert expects w*h*4, failing on
            // exactly the most common kind of photo.
            unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &sourceChannels, 4);

            if (pixels == nullptr)
            {
                ENGINE_CORE_ERROR("Failed to async-load texture '{}': {}", path, stbi_failure_reason());
                return;
            }

            constexpr int kForcedChannels = 4;
            const std::lock_guard<std::mutex> lock(s_PendingUploadsMutex);
            s_PendingUploads.push_back(PendingTextureUpload{handle, width, height, kForcedChannels, pixels});
        });

        return handle;
    }

    void AssetManager::ProcessPendingGPUUploads()
    {
        // Swapped out under the lock, then processed lock-free: holding
        // s_PendingUploadsMutex for the entire duration of several
        // Texture2D::Create + SetData calls (real GPU work, potentially
        // slow) would block every worker thread trying to push a newly
        // completed decode during that whole window, for no benefit - the
        // lock only needs to protect the brief moment of taking ownership
        // of the queue's contents.
        std::vector<PendingTextureUpload> uploads;
        {
            const std::lock_guard<std::mutex> lock(s_PendingUploadsMutex);
            uploads.swap(s_PendingUploads);
        }

        for (const PendingTextureUpload& upload : uploads)
        {
            const auto texture = Texture2D::Create(static_cast<uint32_t>(upload.Width), static_cast<uint32_t>(upload.Height));

            const uint32_t byteCount = static_cast<uint32_t>(upload.Width) * static_cast<uint32_t>(upload.Height) * static_cast<uint32_t>(upload.Channels);
            texture->SetData(upload.Pixels, byteCount);
            stbi_image_free(upload.Pixels);

            s_Textures[upload.Handle] = texture;
            s_HandleTypes[upload.Handle] = AssetType::Texture2D;
        }
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

        // Any decode job that finished (or finishes shortly after this
        // call) but was never processed by ProcessPendingGPUUploads holds
        // a raw stbi-allocated pixel buffer that only this function or
        // ProcessPendingGPUUploads knows how to free - without this,
        // Clear() would either leak that memory, or (worse) leave it for
        // a LATER ProcessPendingGPUUploads() call to resurrect into
        // s_Textures/s_HandleTypes, silently undoing what Clear() was
        // just asked to erase.
        const std::lock_guard<std::mutex> lock(s_PendingUploadsMutex);
        for (const PendingTextureUpload& upload : s_PendingUploads)
        {
            stbi_image_free(upload.Pixels);
        }
        s_PendingUploads.clear();
    }

} // namespace Engine
