file(GLOB_RECURSE HEADER_FILES include/*.h include/*.inl)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h src/*.inl)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(Foundation OBJECT
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(Foundation
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
)

target_include_directories(Foundation
    PUBLIC
    include

    PRIVATE
    src
)

if (MSVC)
    target_compile_options(Foundation PRIVATE /GR-) # Disable RTTI
    target_compile_options(Foundation PRIVATE /EHs-c- /EHc-) # Disable synchronous exceptions and C++ exceptions
endif()
