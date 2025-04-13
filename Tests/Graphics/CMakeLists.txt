#enable_testing()

file(GLOB SOURCES *.cpp *.h)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(GraphicsTest
    ${SOURCES}
)

target_link_libraries(GraphicsTest
    PUBLIC
    ApexFoundation
    ApexGraphics
    ApexModel
    PRIVATE
    Tracy::TracyClient
)

add_dependencies(GraphicsTest ShaderBinaries)

target_include_directories(GraphicsTest
    PRIVATE
)

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
