#enable_testing()

file(GLOB SOURCES *.cpp *.h)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(Engine_test
    ${SOURCES}
)

target_link_libraries(Engine_test
    PUBLIC
    Engine
)

set_target_properties(Engine_test PROPERTIES FOLDER "Tests")

#include(GoogleTest)
#gtest_discover_tests(Engine_test)
