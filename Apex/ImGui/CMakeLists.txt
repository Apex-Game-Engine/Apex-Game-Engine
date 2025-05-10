set(HEADER_FILES 
	${APEX_THIRDPARTY_DIR}/imgui-docking/imconfig.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_internal.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imstb_rectpack.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imstb_textedit.h
	${APEX_THIRDPARTY_DIR}/imgui-docking/imstb_truetype.h
	ApexImGui.h
)

set(SOURCE_FILES
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_demo.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_draw.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_tables.cpp
	${APEX_THIRDPARTY_DIR}/imgui-docking/imgui_widgets.cpp
	ApexImGui.cpp
)

if(WIN32)
	list(APPEND HEADER_FILES ${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_win32.h)
	list(APPEND HEADER_FILES ${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_vulkan.h)
	list(APPEND SOURCE_FILES ${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_win32.cpp)
	list(APPEND SOURCE_FILES ${APEX_THIRDPARTY_DIR}/imgui-docking/backends/imgui_impl_vulkan.cpp)
endif()

add_library(ApexImGui STATIC
	${HEADER_FILES}
	${SOURCE_FILES}
)

target_link_libraries(ApexImGui
	PRIVATE
	ApexFoundation
    ApexPlatform
	ApexGraphics
)

target_include_directories(ApexImGui
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
	${APEX_THIRDPARTY_DIR}/imgui-docking/
)
