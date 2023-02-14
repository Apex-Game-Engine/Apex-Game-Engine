enable_testing()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set_target_properties(gtest PROPERTIES FOLDER "GoogleTest")
set_target_properties(gtest_main PROPERTIES FOLDER "GoogleTest")
set_target_properties(gmock PROPERTIES FOLDER "GoogleTest")
set_target_properties(gmock_main PROPERTIES FOLDER "GoogleTest")

add_subdirectory(Foundation)