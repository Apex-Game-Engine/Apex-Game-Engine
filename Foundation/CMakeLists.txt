file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(Foundation OBJECT
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(Foundation
    PRIVATE
    APEX_INTERNAL
    ${APEX_INTERNAL_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
)

target_include_directories(Foundation
    PUBLIC
    include

    PRIVATE
    src
)
