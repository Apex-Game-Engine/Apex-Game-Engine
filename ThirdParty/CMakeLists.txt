set(CMAKE_FOLDER "External")

add_subdirectory(fmt)
add_subdirectory(meshoptimizer)
add_subdirectory(tracy)
add_subdirectory(glfw)
add_subdirectory(pugixml)
add_subdirectory(lua)

unset(CMAKE_FOLDER)
