#enable_testing()

file(GLOB SOURCES *.cpp *.h)

#if (WIN32)
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
#endif()

add_executable(ApexSchedulerTests
    ${SOURCES}
)

target_link_libraries(ApexSchedulerTests
    PUBLIC
    ApexFoundation
	ApexPlatform
    ApexScheduler

    PRIVATE
    Tracy::TracyClient
)

target_include_directories(ApexSchedulerTests
    PRIVATE
)

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
