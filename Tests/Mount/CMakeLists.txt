enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(ApexMountTests
    ${SOURCES}
)

target_link_libraries(ApexMountTests
    PUBLIC
    ApexFoundation
	ApexMount
	
    PRIVATE
    GTest::gtest
    Tracy::TracyClient
)

target_include_directories(ApexMountTests
    PRIVATE
    ${test_common_INCLUDE_DIR}
)

target_compile_definitions(ApexMountTests
    PRIVATE
    APEX_ENABLE_TESTS
)

include(GoogleTest)
gtest_discover_tests(ApexMountTests)
