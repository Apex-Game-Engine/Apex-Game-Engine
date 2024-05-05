cmake_minimum_required(VERSION 3.19)

project(lua VERSION 5.4.6 LANGUAGES C)

set(SOURCE_FILES
	"src/lapi.c"
	"src/lcode.c"
	"src/lctype.c"
	"src/ldebug.c"
	"src/ldo.c"
	"src/ldump.c"
	"src/lfunc.c"
	"src/lgc.c"
	"src/llex.c"
	"src/lmem.c"
	"src/lobject.c"
	"src/lopcodes.c"
	"src/lparser.c"
	"src/lstate.c"
	"src/lstring.c"
	"src/ltable.c"
	"src/ltm.c"
	"src/lundump.c"
	"src/lvm.c"
	"src/lzio.c"
	"src/lauxlib.c"
	"src/lbaselib.c"
	"src/lcorolib.c"
	"src/ldblib.c"
	"src/liolib.c"
	"src/lmathlib.c"
	"src/loadlib.c"
	"src/loslib.c"
	"src/lstrlib.c"
	"src/ltablib.c"
	"src/lutf8lib.c"
	"src/linit.c")

set(HEADER_FILES
    "include/lapi.h"
    "include/lcode.h"
    "include/lctype.h"
    "include/ldebug.h"
    "include/ldo.h"
    "include/lfunc.h"
    "include/lgc.h"
    "include/llex.h"
    "include/lmem.h"
    "include/lobject.h"
    "include/lopcodes.h"
    "include/lparser.h"
    "include/lstate.h"
    "include/lstring.h"
    "include/ltable.h"
    "include/ltm.h"
    "include/lundump.h"
    "include/lvm.h"
    "include/lzio.h"
    "include/lauxlib.h")


add_library(lua_include INTERFACE)
target_include_directories(lua_include INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(lua_static STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(lua_static PUBLIC lua_include)
