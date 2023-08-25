#enable_testing()

file(GLOB SOURCES *.cpp *.h)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(Startup_test
    ${SOURCES}
)

target_link_libraries(Startup_test
    PUBLIC
    Engine
)

add_custom_command(TARGET Startup_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Engine>/Engine.dll $<TARGET_FILE_DIR:Startup_test>/Engine.dll
)

set_target_properties(Startup_test PROPERTIES FOLDER "Tests")

#include(GoogleTest)
#gtest_discover_tests(Startup_test)
