#include "Engine/Assets/AssetManager.h"

#include <catch2/catch_test_macros.hpp>

using namespace Engine;

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

TEST_CASE("An AssetHandle that was never loaded reports AssetType::None", "[assets][assetmanager]")
{
    AssetManager::Clear();

    const AssetHandle neverLoaded;
    REQUIRE(AssetManager::GetAssetType(neverLoaded) == AssetType::None);
    REQUIRE_FALSE(AssetManager::IsLoaded(neverLoaded));
}

TEST_CASE("GetTexture2D/GetMesh/GetShader return nullptr for an unknown handle", "[assets][assetmanager]")
{
    AssetManager::Clear();

    const AssetHandle unknown;
    REQUIRE(AssetManager::GetTexture2D(unknown) == nullptr);
    REQUIRE(AssetManager::GetMesh(unknown) == nullptr);
    REQUIRE(AssetManager::GetShader(unknown) == nullptr);
}

TEST_CASE("Clear leaves AssetManager in the same observable state as never having loaded anything", "[assets][assetmanager]")
{
    // Without calling any Load* function (see the file-level note on why),
    // this at least verifies Clear() is safe to call repeatedly and on an
    // already-empty cache - a real edge case for any Shutdown-path cleanup
    // that might call it more than once.
    AssetManager::Clear();
    REQUIRE_NOTHROW(AssetManager::Clear());

    const AssetHandle handle;
    REQUIRE_FALSE(AssetManager::IsLoaded(handle));
}
