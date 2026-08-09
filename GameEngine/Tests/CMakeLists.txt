add_executable(EngineTests
    src/CoreTests.cpp
    src/TimestepTests.cpp
    src/EventTests.cpp
    src/LayerStackTests.cpp
    src/ApplicationTests.cpp
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
