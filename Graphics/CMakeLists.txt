file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

find_package(Vulkan REQUIRED)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(Graphics OBJECT
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(Graphics
    PRIVATE
    APEX_INTERNAL
    ${APEX_INTERNAL_ENABLES}

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
