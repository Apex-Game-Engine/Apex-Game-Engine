file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})

add_library(Engine SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_compile_definitions(Engine
    PRIVATE
    APEX_INTERNAL
    ${APEX_INTERNAL_ENABLES}
    $<$<CONFIG:Debug>:APEX_ENABLE_CHECKS>
)

# target_precompile_headers(Engine PUBLIC include/apex_pch.h)
set_target_precompiled_header(Engine apex_pch.h src/apex_pch.cpp)

target_link_libraries(Engine
    PUBLIC
    Foundation
    Graphics
)

target_include_directories(Engine
    PUBLIC
    include

    PRIVATE
    src
)

add_custom_command(TARGET Engine POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:Engine>"
        "$<TARGET_FILE_DIR:Engine_test>/$<TARGET_FILE_NAME:Engine>"
    COMMENT "Copying to Engine_test"
)

