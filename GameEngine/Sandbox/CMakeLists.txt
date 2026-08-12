add_executable(Sandbox
    src/main.cpp
)

target_link_libraries(Sandbox PRIVATE Engine)

engine_set_warnings(Sandbox)

set_target_properties(Sandbox PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_executable(Sandbox
    src/main.cpp
)

target_link_libraries(Sandbox PRIVATE Engine)

engine_set_warnings(Sandbox)

set_target_properties(Sandbox PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Sandbox>/assets
    COMMENT "Copying Sandbox assets next to the built executable"
)
