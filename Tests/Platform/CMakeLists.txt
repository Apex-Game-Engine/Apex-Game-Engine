#enable_testing()

file(GLOB SOURCES *.cpp *.h)

#if (WIN32)
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
#endif()

add_executable(PlatformTest
    ${SOURCES}
)

target_link_libraries(PlatformTest
    PUBLIC
    ApexFoundation
    ApexPlatform
    PRIVATE
    Tracy::TracyClient
)

target_include_directories(PlatformTest
    PRIVATE
)

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
