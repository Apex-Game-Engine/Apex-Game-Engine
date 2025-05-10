#enable_testing()

file(GLOB SOURCES *.cpp *.h)

#if (WIN32)
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
#endif()

add_executable(ApexGraphicsTests
    ${SOURCES}
)

target_link_libraries(ApexGraphicsTests
    PUBLIC
    ApexFoundation
    ApexGraphics
    ApexPlatform
    ApexModel
	ApexShaders
    ApexImGui

    PRIVATE
    Tracy::TracyClient
)

#add_dependencies(ApexGraphicsTests ApexShaders)

target_include_directories(ApexGraphicsTests
    PRIVATE
)

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
