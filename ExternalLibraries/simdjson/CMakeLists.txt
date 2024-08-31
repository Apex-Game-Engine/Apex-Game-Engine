cmake_minimum_required(VERSION 3.14)

project(
    simdjson
    # The version number is modified by tools/release.py
    VERSION 3.9.1
    DESCRIPTION "Parsing gigabytes of JSON per second"
    HOMEPAGE_URL "https://simdjson.org/"
    LANGUAGES CXX C
)

set(SIMDJSON_GITHUB_REPOSITORY "https://github.com/simdjson/simdjson")

string(
    COMPARE EQUAL
    "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
    is_top_project
)

# ---- Options, variables ----

# These version numbers are modified by tools/release.py
set(SIMDJSON_LIB_VERSION "22.0.0" CACHE STRING "simdjson library version")
set(SIMDJSON_LIB_SOVERSION "22" CACHE STRING "simdjson library soversion")

option(SIMDJSON_BUILD_STATIC_LIB "Build simdjson_static library along with simdjson" OFF)

option(SIMDJSON_ENABLE_THREADS "Link with thread support" ON)

include(cmake/simdjson-props.cmake)
include(cmake/implementation-flags.cmake)
include(cmake/exception-flags.cmake)

option(SIMDJSON_DISABLE_DEPRECATED_API "Disables deprecated APIs" OFF)
if(SIMDJSON_DISABLE_DEPRECATED_API)
  simdjson_add_props(
      target_compile_definitions PUBLIC
      SIMDJSON_DISABLE_DEPRECATED_API=1
  )
endif()

option(SIMDJSON_DEVELOPMENT_CHECKS "Enable development-time aids, such as \
checks for incorrect API usage. Enabled by default in DEBUG." OFF)
if(SIMDJSON_DEVELOPMENT_CHECKS)
  simdjson_add_props(
      target_compile_definitions PUBLIC
      SIMDJSON_DEVELOPMENT_CHECKS
  )
endif()

if(is_top_project)
  option(SIMDJSON_DEVELOPER_MODE "Enable targets for developing simdjson" OFF)
  option(BUILD_SHARED_LIBS "Build simdjson as a shared library" OFF)
endif()

include(cmake/handle-deprecations.cmake)
include(cmake/developer-options.cmake)

# ---- simdjson library ----

set(SIMDJSON_SOURCES src/simdjson.cpp)

add_library(simdjson ${SIMDJSON_SOURCES})
add_library(simdjson::simdjson ALIAS simdjson)
set(SIMDJSON_LIBRARIES simdjson)

if(SIMDJSON_BUILD_STATIC_LIB)
  add_library(simdjson_static STATIC ${SIMDJSON_SOURCES})
  add_library(simdjson::simdjson_static ALIAS simdjson_static)
  list(APPEND SIMDJSON_LIBRARIES simdjson_static)
endif()

set_target_properties(
    simdjson PROPERTIES
    VERSION "${SIMDJSON_LIB_VERSION}"
    SOVERSION "${SIMDJSON_LIB_SOVERSION}"
    # FIXME: symbols should be hidden by default
    WINDOWS_EXPORT_ALL_SYMBOLS YES
)

# FIXME: Use proper CMake integration for exports
if(MSVC AND BUILD_SHARED_LIBS)
  target_compile_definitions(
      simdjson
      PRIVATE SIMDJSON_BUILDING_WINDOWS_DYNAMIC_LIBRARY=1
      INTERFACE SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY=1
  )
endif()

simdjson_add_props(
    target_include_directories
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

simdjson_add_props(target_compile_features PUBLIC cxx_std_11)

# workaround for GNU GCC poor AVX load/store code generation
if(
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.86|x86(_64)?)$"
)
  simdjson_add_props(
      target_compile_options PRIVATE
      -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store
  )
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(loongarch64)$")
    option(SIMDJSON_PREFER_LSX "Prefer LoongArch SX" ON)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-mlasx COMPILER_SUPPORTS_LASX)
    check_cxx_compiler_flag(-mlsx COMPILER_SUPPORTS_LSX)
    if(COMPILER_SUPPORTS_LASX AND NOT SIMDJSON_PREFER_LSX)
        simdjson_add_props(
            target_compile_options PRIVATE
            -mlasx
        )
    elseif(COMPILER_SUPPORTS_LSX)
        simdjson_add_props(
            target_compile_options PRIVATE
            -mlsx
        )
    endif()
endif()

# GCC and Clang have horrendous Debug builds when using SIMD.
# A common fix is to use '-Og' instead.
# bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
if(
    (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
)
  message(STATUS "Adding -Og to compile flag")
  simdjson_add_props(
      target_compile_options PRIVATE
      $<$<CONFIG:DEBUG>:-Og>
  )
endif()

if(SIMDJSON_ENABLE_THREADS)
  find_package(Threads REQUIRED)
  simdjson_add_props(target_link_libraries PUBLIC Threads::Threads)
  simdjson_add_props(target_compile_definitions PUBLIC SIMDJSON_THREADS_ENABLED=1)
endif()

simdjson_apply_props(simdjson)
if(SIMDJSON_BUILD_STATIC_LIB)
  simdjson_apply_props(simdjson_static)
endif()
