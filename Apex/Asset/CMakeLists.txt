file(GLOB_RECURSE HEADER_FILES include/*h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(ApexAsset STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(ApexAsset
    PRIVATE
    "$<$<CONFIG:Debug>:APEX_ENABLE_CHECKS>"
)

target_link_libraries(ApexAsset
    PRIVATE
    ApexInterface
    ApexFoundation
    ApexGraphics
    meshoptimizer
    pugixml
)

target_include_directories(ApexAsset
    PUBLIC
    include

    PRIVATE
    src
    #${APEX_THIRDPARTY_DIR}/fast_obj
)