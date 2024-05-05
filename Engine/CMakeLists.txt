file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(Engine SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(Engine
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}
    $<$<CONFIG:Debug>:APEX_ENABLE_CHECKS>
)

set_target_precompiled_header(Engine apex_pch.h src/apex_pch.cpp)

target_link_libraries(Engine
    PUBLIC
    Foundation
    Graphics
    lua_static
)

target_include_directories(Engine
    PUBLIC
    include
    ${APEX_CMAKE_EXTERNAL_LIBS_DIR}/tinygltf
    ${APEX_CMAKE_EXTERNAL_LIBS_DIR}/json
    ${APEX_CMAKE_EXTERNAL_LIBS_DIR}/stb

    PRIVATE
    src
)

if (MSVC)
    target_compile_options(Engine PRIVATE /GR-) # Disable RTTI
    target_compile_options(Engine PRIVATE /EHs-c- /EHc-) # Disable synchronous exceptions and C++ exceptions
endif()
