enable_testing()

file(GLOB SOURCES *.cpp)

add_executable(Foundation_test
    ${SOURCES}
)

target_link_libraries(Foundation_test
    PRIVATE
    Foundation
    GTest::gtest_main
)

set_target_properties(Foundation_test PROPERTIES FOLDER "Tests")

include(GoogleTest)
gtest_discover_tests(Foundation_test)
