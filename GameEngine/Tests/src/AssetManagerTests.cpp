#include "Engine/Assets/AssetManager.h"
#include "Engine/Core/JobSystem.h"
#include "Engine/Core/Log.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

using namespace Engine;

namespace
{
    /// This file's async tests can reach a log call (ENGINE_CORE_ERROR,
    /// if a background decode job runs after its source file was already
    /// removed - see the TempBmpFile destruction-order note below) from a
    /// worker thread. Relying on some OTHER test file having already
    /// called Log::Init() first (true only by accident, depending on
    /// Catch2's run order) segfaulted the moment this file's tests were
    /// run in isolation via a tag filter - the same guarded-static pattern
    /// CoreTests.cpp and ApplicationTests.cpp already use, applied here
    /// for the same reason.
    void EnsureLogInitialized()
    {
        static const bool initialized = []
        {
            Log::Init();
            return true;
        }();
        (void)initialized;
    }

    /// Writes a minimal, valid, hand-constructed 1x1 24-bit BMP file -
    /// enough for a real stbi_load() call to succeed without needing to
    /// vendor an image-writing library just to produce test fixtures.
    /// RAII-cleaned up the same way MeshLoadingTests.cpp's TempObjFile and
    /// SceneSerializerTests.cpp's TempFile already are.
    class TempBmpFile
    {
    public:
        TempBmpFile() : m_Path(std::filesystem::temp_directory_path() / "engine_asset_test.bmp")
        {
            std::ofstream file(m_Path, std::ios::binary);

            const auto writeU32 = [&file](uint32_t value) { file.write(reinterpret_cast<const char*>(&value), 4); };
            const auto writeU16 = [&file](uint16_t value) { file.write(reinterpret_cast<const char*>(&value), 2); };

            // BITMAPFILEHEADER (14 bytes)
            file.write("BM", 2);
            writeU32(58); // file size: 14 + 40 + 4 bytes of pixel data
            writeU32(0);  // reserved
            writeU32(54); // pixel data offset (14 + 40)

            // BITMAPINFOHEADER (40 bytes)
            writeU32(40); // header size
            writeU32(1);  // width
            writeU32(1);  // height
            writeU16(1);  // planes
            writeU16(24); // bits per pixel
            writeU32(0);  // no compression
            writeU32(4);  // image data size (1 row, padded to 4 bytes)
            writeU32(0);
            writeU32(0);
            writeU32(0);
            writeU32(0);

            // Pixel data: one BGR pixel + 1 padding byte (BMP rows are
            // padded to a multiple of 4 bytes).
            const unsigned char pixel[4] = {255, 0, 0, 0}; // blue=255, green=0, red=0
            file.write(reinterpret_cast<const char*>(pixel), 4);
        }

        ~TempBmpFile()
        {
            std::error_code ignored;
            std::filesystem::remove(m_Path, ignored);
        }

        TempBmpFile(const TempBmpFile&) = delete;
        TempBmpFile& operator=(const TempBmpFile&) = delete;

        [[nodiscard]] std::string PathString() const { return m_Path.string(); }

    private:
        std::filesystem::path m_Path;
    };
}

// NOTE on scope: AssetManager::LoadTexture2D/LoadMesh/LoadShader all
// create a real GPU resource under the hood (Texture2D::Create,
// Mesh::CreateFromFile, Shader::Create), which requires a live OpenGL
// context this headless test binary does not have - the exact class of
// problem Tests/src/TestSupport/NullWindow.h solves for Application, not
// yet solved for AssetManager's three Load* paths. Rather than fake a GPU
// context, these tests cover only the behavior that is genuinely
// independent of any actual asset having been loaded: querying an unknown
// handle, and Clear()'s effect on that query. The Load* functions
// themselves are verified manually under Xvfb, consistent with how
// Renderer2D/Mesh3DLayer's GPU-dependent code was verified in M7/M8.
//
// LoadTexture2DAsync and ProcessPendingGPUUploads are the M14 exception to
// that rule where possible: the BACKGROUND DECODE step (stb_image, pure
// CPU) never touches GL, only ProcessPendingGPUUploads's actual
// Texture2D::Create call does - so the tests below exercise everything up
// to, but not including, that GPU upload.

TEST_CASE("An AssetHandle that was never loaded reports AssetType::None", "[assets][assetmanager]")
{
    EnsureLogInitialized();
    AssetManager::Clear();

    const AssetHandle neverLoaded;
    REQUIRE(AssetManager::GetAssetType(neverLoaded) == AssetType::None);
    REQUIRE_FALSE(AssetManager::IsLoaded(neverLoaded));
}

TEST_CASE("GetTexture2D/GetMesh/GetShader return nullptr for an unknown handle", "[assets][assetmanager]")
{
    EnsureLogInitialized();
    AssetManager::Clear();

    const AssetHandle unknown;
    REQUIRE(AssetManager::GetTexture2D(unknown) == nullptr);
    REQUIRE(AssetManager::GetMesh(unknown) == nullptr);
    REQUIRE(AssetManager::GetShader(unknown) == nullptr);
}

TEST_CASE("Clear leaves AssetManager in the same observable state as never having loaded anything", "[assets][assetmanager]")
{
    EnsureLogInitialized();
    // Without calling any Load* function (see the file-level note on why),
    // this at least verifies Clear() is safe to call repeatedly and on an
    // already-empty cache - a real edge case for any Shutdown-path cleanup
    // that might call it more than once.
    AssetManager::Clear();
    REQUIRE_NOTHROW(AssetManager::Clear());

    const AssetHandle handle;
    REQUIRE_FALSE(AssetManager::IsLoaded(handle));
}

TEST_CASE("LoadTexture2DAsync called twice with the same path returns the same handle immediately", "[assets][assetmanager]")
{
    EnsureLogInitialized();
    AssetManager::Clear();
    JobSystem jobSystem(2);

    const TempBmpFile bmpFile;
    const AssetHandle first = AssetManager::LoadTexture2DAsync(bmpFile.PathString(), jobSystem);
    const AssetHandle second = AssetManager::LoadTexture2DAsync(bmpFile.PathString(), jobSystem);

    // This is checked BEFORE any background decode job necessarily has a
    // chance to run - and must hold regardless, since the deduplication
    // happens synchronously in LoadTexture2DAsync itself via the path
    // cache (see AssetManager.cpp), before any job is even submitted for
    // a not-yet-seen path.
    REQUIRE(first == second);
}

TEST_CASE("A handle from LoadTexture2DAsync is not yet loaded before ProcessPendingGPUUploads runs", "[assets][assetmanager]")
{
    EnsureLogInitialized();
    AssetManager::Clear();
    JobSystem jobSystem(2);

    const TempBmpFile bmpFile;
    const AssetHandle handle = AssetManager::LoadTexture2DAsync(bmpFile.PathString(), jobSystem);

    // True immediately (before the background job could plausibly have
    // even started) AND expected to remain true for as long as this test
    // never calls ProcessPendingGPUUploads - GetTexture2D/IsLoaded only
    // become true once that GPU-upload step actually runs, which is
    // exactly the "worker decodes, main thread uploads" split this
    // milestone exists to demonstrate.
    REQUIRE_FALSE(AssetManager::IsLoaded(handle));
    REQUIRE(AssetManager::GetTexture2D(handle) == nullptr);
}

TEST_CASE("Clear safely frees an in-flight async decode without crashing or leaking", "[assets][assetmanager]")
{
    EnsureLogInitialized();
    AssetManager::Clear();
    JobSystem jobSystem(2);

    const TempBmpFile bmpFile;
    const AssetHandle handle = AssetManager::LoadTexture2DAsync(bmpFile.PathString(), jobSystem);

    // Give the background decode a real chance to complete and push its
    // result into AssetManager's pending-uploads queue before Clear() runs -
    // this is what actually exercises the fix in Clear() (freeing
    // s_PendingUploads' stbi-allocated buffers) rather than trivially
    // passing because the job never got far enough to matter. A short
    // sleep is timing-dependent in principle, but decoding a 1x1 BMP is a
    // sub-millisecond operation - 50ms is generous headroom, not a tight
    // race.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    REQUIRE_NOTHROW(AssetManager::Clear());
    REQUIRE_FALSE(AssetManager::IsLoaded(handle));
}
