#enable_testing()

file(GLOB SOURCES *.cpp *.h)

#if (WIN32)
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
#endif()

add_executable(ApexPlatformTests
    ${SOURCES}
)

target_link_libraries(ApexPlatformTests
    PUBLIC
    ApexFoundation
    ApexPlatform

    PRIVATE
    Tracy::TracyClient
)

target_include_directories(ApexPlatformTests
    PRIVATE
)

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
