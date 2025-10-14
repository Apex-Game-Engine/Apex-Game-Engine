file(GLOB_RECURSE HEADER_FILES include/*.h)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
file(GLOB_RECURSE SHADER_FILES
        shaders/*.vert
        shaders/*.tesc
        shaders/*.tese
        shaders/*.geom
        shaders/*.frag
        shaders/*.comp
        shaders/*.glsl
)
file(GLOB_RECURSE SHADER_INCLUDES shaders/*.glh)

set(IMGUI_HEADERS
	${APEX_THIRDPARTY_DIR}/imgui-docking/imconfig.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_internal.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imstb_rectpack.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imstb_textedit.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imstb_truetype.h
)

set(IMGUI_SOURCES
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_demo.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_draw.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_tables.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_widgets.cpp
)

if(WIN32)
	list(APPEND IMGUI_HEADERS
		${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_win32.h
		${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_vulkan.h)
	list(APPEND IMGUI_SOURCES
		${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_win32.cpp
		${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_vulkan.cpp)
endif()

message("${IMGUI_HEADERS}")

message(CHECK_START "Finding glslc")

find_package(Vulkan 1.4 REQUIRED COMPONENTS glslc)
find_program(GLSLC NAMES glslc HINTS Vulkan::glslc)

if(DEFINED GLSLC)
    message(CHECK_PASS "found at ${GLSLC}")
else()
    message(CHECK_PASS "not found")
endif()

group_header_files(${HEADER_FILES})
group_source_files(${SOURCE_FILES})
group_shader_files(${SHADER_FILES})

foreach(_source IN ITEMS ${IMGUI_HEADERS})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${APEX_THIRDPARTY_DIR}/imgui-docking" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("Header Files\\${_group_path}" FILES "${_source}")
endforeach()

foreach(_source IN ITEMS ${IMGUI_SOURCES})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${APEX_THIRDPARTY_DIR}/imgui-docking" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("Source Files\\" FILES "${_source}")
endforeach()

add_library(ApexGraphics STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${IMGUI_HEADERS}
    ${IMGUI_SOURCES}
    ${APEX_THIRDPARTY_DIR}/SPIRV-Reflect/spirv_reflect.c
)

target_compile_definitions(ApexGraphics
    PRIVATE
    APEX_INTERNAL
    APEX_EXPORTS
    ${APEX_ENABLES}

    PUBLIC
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_ENABLE_MEMORY_TRACKING>"
    "$<$<CONFIG:Debug,DebugGame,Development>:APEX_VK_ENABLE_VALIDATION>"
)

target_link_libraries(ApexGraphics
	PRIVATE
	ApexFoundation
    ApexPlatform

    PUBLIC
    Vulkan::Vulkan
)

target_include_directories(ApexGraphics
    PUBLIC
    include
    ${Vulkan_INCLUDE_DIRS}
    ${APEX_THIRDPARTY_DIR}/vma
	${APEX_THIRDPARTY_DIR}/imgui-docking/
    ${APEX_THIRDPARTY_DIR}/SPIRV-Reflect
    ${APEX_THIRDPARTY_DIR}/tracy/public/tracy

    PRIVATE
    src
)

# Shaders
add_shader_library(ApexShaders
    SOURCES
    ${SHADER_FILES}
    INCLUDES
    ${SHADER_INCLUDES}
)

target_include_directories(ApexShaders
	INTERFACE
	shaders
)
