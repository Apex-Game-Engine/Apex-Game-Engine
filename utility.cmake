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
