add_executable(Sandbox
    src/main.cpp
)

target_link_libraries(Sandbox PRIVATE Engine)

engine_set_warnings(Sandbox)

set_target_properties(Sandbox PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
