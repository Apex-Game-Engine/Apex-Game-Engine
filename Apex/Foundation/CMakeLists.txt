file(GLOB_RECURSE HEADER_FILES include/*.h include/*.inl)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h src/*.inl)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(ApexFoundation STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(ApexFoundation
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
)

target_include_directories(ApexFoundation
    PUBLIC
    include

    PRIVATE
    src
    ${APEX_THIRDPARTY_DIR}/tracy/public/tracy
)

target_link_libraries(ApexFoundation
	PUBLIC
	fmt::fmt-header-only
)

if (MSVC)
    target_compile_options(ApexFoundation PRIVATE /GR-) # Disable RTTI
    target_compile_options(ApexFoundation PRIVATE /EHs-c- /EHc-) # Disable synchronous exceptions and C++ exceptions
endif()
