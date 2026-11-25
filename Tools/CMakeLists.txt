cmake_minimum_required(VERSION 3.19)

project(Tools)

set(APEX_TOOLS_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

set(Blender_ROOT_DIR "$ENV{BLENDER_PATH}")
find_package(Python 3.10 COMPONENTS Interpreter Development)

add_subdirectory(ThirdParty)
add_subdirectory(PythonTools)
add_subdirectory(MeshViewer)