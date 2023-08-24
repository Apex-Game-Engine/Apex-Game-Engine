enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(Foundation_test
    ${SOURCES}
)

target_link_libraries(Foundation_test
    PRIVATE
    Foundation
    GTest::gtest
)

target_include_directories(Foundation_test
    PRIVATE
    ${test_common_INCLUDE_DIR}
)

target_compile_definitions(Foundation_test
    PRIVATE
    APEX_ENABLE_TESTS
)

set_target_properties(Foundation_test PROPERTIES FOLDER "Tests")

include(GoogleTest)
gtest_discover_tests(Foundation_test)
