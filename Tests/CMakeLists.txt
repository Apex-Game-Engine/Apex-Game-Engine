enable_testing()

project(ApexTests LANGUAGES C CXX)

if(APEX_COMPILER_TOOLSET_CLANG_CL)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/Apex-Game-Engine/googletest.git
        GIT_TAG        39bf7133781658a08e20d8862315d5b4c54ccc43
    )
else()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        03597a01ee50ed33e9dfd640b249b4be3799d395
    )
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set_target_properties(gtest PROPERTIES FOLDER "GoogleTest")
set_target_properties(gtest_main PROPERTIES FOLDER "GoogleTest")
set_target_properties(gmock PROPERTIES FOLDER "GoogleTest")
set_target_properties(gmock_main PROPERTIES FOLDER "GoogleTest")

set(test_common_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common")


set(CMAKE_FOLDER "Tests")

add_subdirectory(Foundation)
add_subdirectory(Platform)
add_subdirectory(Mount)
add_subdirectory(Graphics)
#add_subdirectory(Startup)

unset(CMAKE_FOLDER)
