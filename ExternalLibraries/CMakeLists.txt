# Add external libraries
add_subdirectory(fmt)
add_subdirectory(lua)
#add_subdirectory(glfw)
# Set Visual Studio folder
set_target_properties(fmt PROPERTIES FOLDER "External")
#set_target_properties(glfw PROPERTIES FOLDER "External")
set_target_properties(lua_static PROPERTIES FOLDER "External")
