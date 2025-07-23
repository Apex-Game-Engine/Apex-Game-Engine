file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
file(GLOB_RECURSE NATVIS_FILES *.natvis)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(ApexMount STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${NATVIS_FILES}
)

target_link_libraries(ApexMount
    PRIVATE
    ApexFoundation
	ApexPlatform
)

target_include_directories(ApexMount
    PUBLIC
    include

    PRIVATE
    src
)
