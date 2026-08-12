add_executable(EngineTests
    src/CoreTests.cpp
    src/TimestepTests.cpp
    src/EventTests.cpp
    src/LayerStackTests.cpp
    src/ApplicationTests.cpp
    src/VecTests.cpp
    src/Mat4Tests.cpp
    src/QuaternionTests.cpp
    src/TransformComponentTests.cpp
    src/UUIDTests.cpp
    src/MeshLoadingTests.cpp
    src/AssetManagerTests.cpp
    src/ECS/EntityTests.cpp
    src/ECS/ComponentPoolTests.cpp
    src/ECS/RegistryTests.cpp
    src/ECS/ViewTests.cpp
)

target_link_libraries(EngineTests
    PRIVATE
        Engine
        Catch2::Catch2WithMain
)

engine_set_warnings(EngineTests)

set_target_properties(EngineTests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

include(CTest)
include(Catch)
catch_discover_tests(EngineTests)
