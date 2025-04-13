#enable_testing()

file(GLOB SOURCES *.cpp *.h)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(StartupTest
    ${SOURCES}
)

target_link_libraries(StartupTest
    PUBLIC
    Engine
)

add_custom_command(TARGET StartupTest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Engine>/Engine.dll $<TARGET_FILE_DIR:StartupTest>/Engine.dll
)

#include(GoogleTest)
#gtest_discover_tests(StartupTest)
