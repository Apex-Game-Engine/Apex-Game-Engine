add_subdirectory(fmt)
#add_subdirectory(glfw)
add_subdirectory(simdjson)
add_subdirectory(fastgltf)

set_target_properties(fmt PROPERTIES FOLDER "External")
#set_target_properties(glfw PROPERTIES FOLDER "External")
set_target_properties(simdjson PROPERTIES FOLDER "External")
set_target_properties(fastgltf PROPERTIES FOLDER "External")
