macro(group_header_files)
    foreach(_source IN ITEMS ${ARGV})
        get_filename_component(_source_path "${_source}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/include" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        source_group("Header Files\\${_group_path}" FILES "${_source}")
    endforeach()
endmacro(group_header_files)

macro(group_source_files)
    foreach(_source IN ITEMS ${ARGV})
        get_filename_component(_source_path "${_source}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/src" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        source_group("Source Files\\${_group_path}" FILES "${_source}")
    endforeach()
endmacro(group_source_files)

macro(group_shader_files)
    foreach(_source IN ITEMS ${ARGV})
        get_filename_component(_source_path "${_source}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        source_group("Shader Files\\${_group_path}" FILES "${_source}")
        source_group("Shader Outputs\\${_group_path}" FILES "${_source}.spv")
    endforeach()
endmacro(group_shader_files)

# The following two functions have been taken from https://stackoverflow.com/a/29672231
# set PCH for VS project
function(set_target_precompiled_header Target PrecompiledHeader PrecompiledSource)
    if(MSVC)
        set_target_properties(${Target} PROPERTIES COMPILE_FLAGS "/Yu${PrecompiledHeader}")
        set_source_files_properties(${PrecompiledSource} PROPERTIES COMPILE_FLAGS "/Yc${PrecompiledHeader}")
    endif()
endfunction(set_target_precompiled_header)

# ignore PCH for a specified list of files
function(ignore_precompiled_header SourcesVar)
    if(MSVC)
        set_source_files_properties(${${SourceVar}} PROPERTIES COMPILE_FLAGS "/Y-")
    endif()
endfunction(ignore_precompiled_header)


# shader compiler taken from https://stackoverflow.com/a/60472877
function(target_vulkan_shaders Target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES")
    foreach(source ${arg_SOURCES})
        message(STATUS "Adding ${source} as shader source")
        add_custom_command(
            OUTPUT ${source}.spv
            DEPENDS ${source}
            COMMAND
                ${GLSLC}
                -o ${source}.spv
                ${source}
            COMMENT "Compiling shader file : ${source}"
        )
        target_sources(${Target} PRIVATE ${source}.spv)
    endforeach()
endfunction()
