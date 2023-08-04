file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
file(GLOB_RECURSE SHADER_FILES
        shaders/*.vert
        shaders/*.tesc
        shaders/*.tese
        shaders/*.geom
        shaders/*.frag
        shaders/*.comp
)

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
    ${SHADER_FILES}
)

target_vulkan_shaders(Graphics
    SOURCES
    ${SHADER_FILES}
)

target_compile_definitions(Graphics
    PRIVATE
    APEX_INTERNAL
    ${APEX_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_VK_ENABLE_VALIDATION>"
)

target_link_libraries(Graphics
    PUBLIC
    Foundation
    Vulkan::Vulkan
)

target_include_directories(Graphics
    PUBLIC
    include
    ${Vulkan_INCLUDE_DIRS}

    PRIVATE
    src
)
