add_library(tiny_obj_loader STATIC src/tiny_obj_loader.cpp)

target_include_directories(tiny_obj_loader PUBLIC include)

# Vendored third-party source - see the Engine/vendor/glad note on why
# warnings are silenced only for this one target, never for project code.
if(NOT MSVC)
    target_compile_options(tiny_obj_loader PRIVATE -w)
else()
    target_compile_options(tiny_obj_loader PRIVATE /w)
endif()
