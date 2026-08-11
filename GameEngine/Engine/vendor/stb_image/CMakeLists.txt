add_library(stb_image STATIC src/stb_image.cpp)

target_include_directories(stb_image PUBLIC include)

# Vendored third-party source - see the Engine/vendor/glad note on why
# warnings are silenced only for this one target, never for project code.
if(NOT MSVC)
    target_compile_options(stb_image PRIVATE -w)
else()
    target_compile_options(stb_image PRIVATE /w)
endif()
