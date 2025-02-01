#enable_testing()

file(GLOB SOURCES *.cpp *.h)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(Graphics_test
    ${SOURCES}
)

target_link_libraries(Graphics_test
    PUBLIC
    Foundation
    Graphics
)

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
