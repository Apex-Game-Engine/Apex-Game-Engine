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
        cmake_path(GET _source FILENAME _source_filename)
        source_group("Shader Sources\\${_group_path}" FILES "${_source}")
        # source_group("Shader Outputs\\${_group_path}" FILES "${CMAKE_CURRENT_BINARY_DIR}/spv/${_source_filename}.spv")
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
function(add_shader_library Target)
    set(multiValueArgs SOURCES INCLUDES)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "${multiValueArgs}")
	set(SHADER_SOURCES)
    set(SPIRV_OUTPUTS)
    foreach(source ${arg_SOURCES})
		cmake_path(ABSOLUTE_PATH source NORMALIZE)
        cmake_path(GET source FILENAME source_filename)
        set(spirv_output "${CMAKE_CURRENT_BINARY_DIR}/spv/${source_filename}.spv")
        message(STATUS "Adding ${source_filename} as shader source")
        list(APPEND SHADER_SOURCES ${source})
		list(APPEND SPIRV_OUTPUTS ${spirv_output})
    endforeach()
	set(TargetSpv "${Target}_SPV")
    add_custom_target(${TargetSpv} ALL DEPENDS ${SPIRV_OUTPUTS} ${arg_SOURCES} ${arg_INCLUDES})
	add_library(${Target} INTERFACE)
	foreach(source spirv_output IN ZIP_LISTS SHADER_SOURCES SPIRV_OUTPUTS)
		add_custom_command(
			OUTPUT ${spirv_output}
			DEPENDS ${source}
			COMMAND
				${GLSLC}
				--target-env=vulkan1.3
				-I "$<TARGET_PROPERTY:${Target},INTERFACE_INCLUDE_DIRECTORIES>"
				-o ${spirv_output}
				${source}
			COMMENT "Compiling shader file : ${source}"
			VERBATIM
		)
	endforeach()
	target_sources(${Target} PRIVATE ${arg_SOURCES} ${arg_INCLUDES})
	add_dependencies(${Target} ${TargetSpv})
endfunction()

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang AND "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    set(APEX_COMPILER_TOOLSET_CLANG_CL TRUE)
endif()
