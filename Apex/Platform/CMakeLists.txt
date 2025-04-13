file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(ApexPlatform STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_link_libraries(ApexPlatform
    PRIVATE
    ApexFoundation
    glfw
)

target_include_directories(ApexPlatform
    PUBLIC
    include

    PRIVATE
    src
)
