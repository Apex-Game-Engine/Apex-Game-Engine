cmake_minimum_required(VERSION 3.12)

add_library(lua5.3 STATIC)

target_sources(lua5.3 PRIVATE
	src/lapi.c		src/lcode.c		src/lctype.c	src/ldebug.c	src/ldo.c		src/ldump.c
	src/lfunc.c		src/lgc.c		src/llex.c		src/lmem.c		src/lobject.c	src/lopcodes.c
	src/lparser.c	src/lstate.c	src/lstring.c	src/ltable.c	src/ltm.c		src/lundump.c
	src/lvm.c		src/lzio.c		src/lauxlib.c	src/lbaselib.c	src/lbitlib.c	src/lcorolib.c
	src/ldblib.c	src/liolib.c	src/lmathlib.c	src/loslib.c	src/lstrlib.c	src/ltablib.c
	src/lutf8lib.c	src/loadlib.c	src/linit.c)

target_include_directories(lua5.3 PUBLIC src)
#target_compile_definitions(lua5.3 PRIVATE LUA_BUILD_AS_DLL)
set_target_properties(lua5.3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug/bin)
