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

find_package(Vulkan 1.4 REQUIRED COMPONENTS glslc)
find_program(GLSLC NAMES glslc HINTS Vulkan::glslc)

if(DEFINED GLSLC)
    message(CHECK_PASS "found at ${GLSLC}")
else()
    message(CHECK_PASS "not found")
endif()

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})
group_shader_files(${SHADER_FILES})

add_library(ApexGraphics STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${APEX_THIRDPARTY_DIR}/SPIRV-Reflect/spirv_reflect.c
)

target_compile_definitions(ApexGraphics
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_VK_ENABLE_VALIDATION>"
)

target_link_libraries(ApexGraphics
	PRIVATE
	ApexFoundation
    ApexPlatform

    PUBLIC
    Vulkan::Vulkan
)

target_include_directories(ApexGraphics
    PUBLIC
    include
    ${Vulkan_INCLUDE_DIRS}
    ${APEX_THIRDPARTY_DIR}/vma
    ${APEX_THIRDPARTY_DIR}/SPIRV-Reflect
    ${APEX_THIRDPARTY_DIR}/tracy/public/tracy

    PRIVATE
    src
)

# Shaders
add_shader_library(ApexShaders
    SOURCES
    ${SHADER_FILES}
    INCLUDES
    ${SHADER_INCLUDES}
)

target_include_directories(ApexShaders
	INTERFACE
	shaders
)
