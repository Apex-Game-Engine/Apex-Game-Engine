#enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(Scripting_test
    ${SOURCES}
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
