enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(Engine_test
    ${SOURCES}
)

target_link_libraries(Engine_test
    PRIVATE
    Engine
    GTest::gtest
)

target_compile_definitions(Engine_test
    PRIVATE
    APEX_ENABLE_TESTS
)

add_custom_command(TARGET Engine_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:Engine>/Engine.dll $<TARGET_FILE_DIR:Engine_test>/Engine.dll
)

set_target_properties(Engine_test PROPERTIES FOLDER "Tests")

include(GoogleTest)
gtest_discover_tests(Engine_test)
