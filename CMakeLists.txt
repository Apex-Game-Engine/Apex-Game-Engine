cmake_minimum_required(VERSION 3.19)

include(FetchContent)

set(CMAKE_CXX_EXCEPTIONS Off)
set(CMAKE_CXX_RTTI Off)
set(CMAKE_CXX_STANDARD 20)

# FetchContent_Declare(
#     glfw
#     GIT_REPOSITORY https://github.com/glfw/glfw
#     GIT_TAG 8f470597d625ae28758c16b4293dd42d63e8a83a
# )
# 
# FetchContent_MakeAvailable(glfw)


project(ApexGameEngine
        VERSION 0.2.0
        LANGUAGES C CXX
    )


# Similar to Unreal Engine
# Debug       :: Engine: (Opt off  (Od), Sym on, Checks on),   Game: (Opt off, Sym on,   Checks on , Profiling on)
# DebugGame   :: Engine: (Opt med  (Ot), Sym off, Checks off), Game: (Opt off, Sym on,   Checks on , Profiling on)
# Development :: Engine: (Opt med  (Ot), Sym off, Checks off), Game: (Opt med, Sym off,  Checks off, Profiling on)
# Shipping    :: Engine: (Opt full (O2), Sym off, Checks off), Game: (Opt full, Sym off, Checks off, Profiling off)
# Test        :: Engine: (Opt full (O2), Sym off, Checks off), Game: (Opt full, Sym off, Checks off, Profiling on)

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES
        Debug
        DebugGame
        Development
        Release
        #Shipping
        #Test
    )
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(utility.cmake)

set(CMAKE_CXX_FLAGS_DEBUGGAME   ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_DEVELOPMENT ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})

set(CMAKE_EXE_LINKER_FLAGS_DEBUGGAME   ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})

set(CMAKE_SHARED_LINKER_FLAGS_DEBUGGAME   ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})

#set(APEX_ENABLES
#    # APEX_ENABLE_ASSERTS
#    "$<$<CONFIG:Debug>:APEX_ENABLE_ASSERTS=2>"
#    "$<$<CONFIG:DebugGame,Development>:APEX_ENABLE_ASSERTS=1>"
#    "$<$<CONFIG:Release>:APEX_ENABLE_ASSERTS=0>"
#)

add_compile_definitions(
    $<$<CONFIG:Debug>:APEX_CONFIG_DEBUG>
    $<$<CONFIG:DebugGame>:APEX_CONFIG_DEBUGGAME>
    $<$<CONFIG:Development>:APEX_CONFIG_DEVELOPMENT>
    $<$<CONFIG:Release>:APEX_CONFIG_RELEASE>
    
    $<$<CONFIG:Debug>:APEX_ENABLE_ASSERTS=2>
    $<$<CONFIG:DebugGame,Development>:APEX_ENABLE_ASSERTS=1>
    $<$<CONFIG:Release>:APEX_ENABLE_ASSERTS=0>
)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS On)
    add_compile_definitions(APEX_PLATFORM_WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
else()
endif()

set(APEX_CMAKE_EXTERNAL_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ExternalLibraries)

add_subdirectory(ExternalLibraries)

add_subdirectory(Foundation)
add_subdirectory(Graphics)
add_subdirectory(Engine)
# add_subdirectory(Runtime)
# add_subdirectory(Editor)

# Unit Tests
add_subdirectory(Tests)

# Examples
# add_subdirectory(examples)
