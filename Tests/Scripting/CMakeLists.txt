#enable_testing()

file(GLOB SOURCES *.cpp *.h)
file(GLOB SCRIPTS scripts/*.lua)

group_lua_files(${SCRIPTS})

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(Scripting_test
    ${SOURCES}
    ${SCRIPTS}
)

target_link_libraries(Scripting_test
    PRIVATE
    Engine
)

add_custom_command(TARGET Scripting_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Engine>/Engine.dll $<TARGET_FILE_DIR:Scripting_test>/Engine.dll
)

#include(GoogleTest)
#gtest_discover_tests(Scripting_test)
