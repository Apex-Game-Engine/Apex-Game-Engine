
add_library(ApexModel STATIC)

set(HEADERS
    include/MeshSerializer.h
    #include/ImageSerializer.h
    #include/SkeletonSerializer.h
)

set(SOURCES
    src/MeshSerializer.cpp
    #src/ImageSerializer.cpp
)

target_sources(ApexModel
    PRIVATE
    ${HEADERS}
    ${SOURCES}
)

target_compile_definitions(ApexModel
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}
    $<$<CONFIG:Debug>:APEX_ENABLE_CHECKS>
)

target_link_libraries(ApexModel
    PUBLIC
    ApexFoundation
    ApexGraphics

    PRIVATE
    meshoptimizer
)

target_include_directories(ApexModel
    PUBLIC
    include

    PRIVATE
    src
    #${APEX_THIRDPARTY_DIR}/fast_obj
)