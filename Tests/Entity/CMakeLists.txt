enable_testing()

file(GLOB SOURCES *.cpp *.h)

add_executable(ApexEntityTests
    ${SOURCES}
)

target_link_libraries(ApexEntityTests
    PUBLIC
    ApexFoundation
	ApexEntity
	
    PRIVATE
    GTest::gtest
    Tracy::TracyClient
)

target_include_directories(ApexEntityTests
    PRIVATE
    ${test_common_INCLUDE_DIR}
)

target_compile_definitions(ApexEntityTests
    PRIVATE
    APEX_ENABLE_TESTS
)

include(GoogleTest)
gtest_discover_tests(ApexEntityTests)
