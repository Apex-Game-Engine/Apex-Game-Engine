enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(FoundationTest
    ${SOURCES}
)

target_link_libraries(FoundationTest
    PRIVATE
    ApexFoundation
    GTest::gtest
)

target_include_directories(FoundationTest
    PRIVATE
    ${test_common_INCLUDE_DIR}
)

target_compile_definitions(FoundationTest
    PRIVATE
    APEX_ENABLE_TESTS
)

include(GoogleTest)
gtest_discover_tests(FoundationTest)
