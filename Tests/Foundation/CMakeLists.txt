enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(ApexFoundationTests
    ${SOURCES}
)

target_link_libraries(ApexFoundationTests
    PRIVATE
    ApexFoundation
    GTest::gtest
)

target_include_directories(ApexFoundationTests
    PRIVATE
    ${test_common_INCLUDE_DIR}
)

target_compile_definitions(ApexFoundationTests
    PRIVATE
    APEX_ENABLE_TESTS
)

include(GoogleTest)
gtest_discover_tests(ApexFoundationTests)
