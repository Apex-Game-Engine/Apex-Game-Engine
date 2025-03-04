file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
file(GLOB_RECURSE SHADER_FILES
        shaders/*.vert
        shaders/*.tesc
        shaders/*.tese
        shaders/*.geom
        shaders/*.frag
        shaders/*.comp
        shaders/*.glsl
)
file(GLOB_RECURSE SHADER_INCLUDES shaders/*.glh)

message(CHECK_START "Finding glslc")

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(GLSLC NAMES glslc HINTS Vulkan::glslc)

if(DEFINED GLSLC)
    message(CHECK_PASS "found at ${GLSLC}")
else()
    message(CHECK_PASS "not found")
endif()

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})
group_shader_files(${SHADER_FILES})

add_library(Graphics STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${APEX_CMAKE_EXTERNAL_LIBS_DIR}/SPIRV-Reflect/spirv_reflect.c
)

target_compile_definitions(Graphics
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_VK_ENABLE_VALIDATION>"
)

target_link_libraries(Graphics
    PUBLIC
    Vulkan::Vulkan
)

target_include_directories(Graphics
    PUBLIC
    include
    ${Vulkan_INCLUDE_DIRS}
    ${APEX_CMAKE_EXTERNAL_LIBS_DIR}/vma
    ${APEX_CMAKE_EXTERNAL_LIBS_DIR}/SPIRV-Reflect
    $<TARGET_PROPERTY:Engine,INTERFACE_INCLUDE_DIRECTORIES>

    PRIVATE
    src
    $<TARGET_PROPERTY:Foundation,INTERFACE_INCLUDE_DIRECTORIES>
)

if (MSVC)
    target_compile_options(Graphics PRIVATE /GR-) # Disable RTTI
    target_compile_options(Graphics PRIVATE /EHs-c- /EHc-) # Disable synchronous exceptions and C++ exceptions
endif()


# Shaders
add_custom_target(Shaders ALL)

target_sources(Shaders
    PRIVATE
    ${SHADER_FILES}
    ${SHADER_INCLUDES}
)

target_vulkan_shaders(ShaderBinaries
    SOURCES
    ${SHADER_FILES}
    INCLUDES
    ${SHADER_INCLUDES}
)


