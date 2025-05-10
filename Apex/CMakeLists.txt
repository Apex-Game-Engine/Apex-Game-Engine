cmake_minimum_required(VERSION 3.19)

project(ApexGameEngine
        VERSION 0.2.0
        LANGUAGES C CXX
    )

set(CMAKE_FOLDER "Apex")

add_subdirectory(Foundation)
add_subdirectory(Platform)
add_subdirectory(Graphics)
add_subdirectory(ImGui)
#add_subdirectory(Rtti)
#add_subdirectory(Engine)
add_subdirectory(Model)
# add_subdirectory(Runtime)
# add_subdirectory(Editor)

unset(CMAKE_FOLDER)
