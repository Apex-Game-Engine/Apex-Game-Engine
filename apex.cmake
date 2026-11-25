function(add_apex_tool NAME)
	cmake_parse_arguments(TOOL "USE_GUI;USE_CONSOLE;LINK_DYNAMIC" "TITLE;VERSION" "SOURCES;LINKS;DEPENDS" ${ARGN})

	add_executable(${NAME} ${TOOL_SOURCES} ${APEX_APEX_DIR}/ApexMain.cpp)

	target_link_libraries(${NAME}
		PRIVATE
		ApexFoundation
		ApexPlatform
		ApexGraphics
		ApexScheduler
		${TOOL_LINKS}
	)

	add_dependencies(${NAME}
		${TOOL_DEPENDS}
	)

	target_compile_definitions(${NAME} PRIVATE APEX_APPLICATION_TITLE="${TOOL_TITLE}" APEX_APPLICATION_VERSION="${TOOL_VERSION}")
	target_compile_definitions(${NAME} PRIVATE APEX_TOOL=1)
	
	if(TOOL_USE_GUI)
		target_compile_definitions(${NAME} PRIVATE APEX_USE_GUI=1)
	else()
		target_compile_definitions(${NAME} PRIVATE APEX_USE_GUI=0)
	endif()
	
	if(TOOL_USE_CONSOLE)
		target_compile_definitions(${NAME} PRIVATE APEX_USE_CONSOLE=1)
	else()
		target_compile_definitions(${NAME} PRIVATE APEX_USE_CONSOLE=0)
	endif()

	if(TOOL_LINK_DYNAMIC)
		target_compile_definitions(${NAME} PRIVATE APEX_IMPORTS=1)
	else()
		target_compile_definitions(${NAME} PRIVATE APEX_IMPORTS=0)
	endif()

	message(STATUS "Added Tool: ${NAME}")
endfunction()