cmake_minimum_required(VERSION 3.19)

project(Apex VERSION 0.2.0)

include(FetchContent)

set(CMAKE_CXX_EXCEPTIONS Off)
set(CMAKE_CXX_RTTI Off)
set(CMAKE_CXX_STANDARD 20)


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

        Debug_Profile
        Release_Profile
        #Shipping
        #Test
    )
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(utility.cmake)

set(CMAKE_CXX_FLAGS_DEBUGGAME                 ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_DEVELOPMENT               ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_DEBUG_PROFILE             ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE_PROFILE           ${CMAKE_CXX_FLAGS_RELEASE})

set(CMAKE_EXE_LINKER_FLAGS_DEBUGGAME          ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT        ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
set(CMAKE_EXE_LINKER_FLAGS_DEBUG_PROFILE      ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_PROFILE    ${CMAKE_EXE_LINKER_FLAGS_RELEASE})

set(CMAKE_SHARED_LINKER_FLAGS_DEBUGGAME       ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT     ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG_PROFILE   ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE_PROFILE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})

set(CMAKE_MODULE_LINKER_FLAGS_DEBUGGAME       ${CMAKE_MODULE_LINKER_FLAGS_DEBUG})
set(CMAKE_MODULE_LINKER_FLAGS_DEVELOPMENT     ${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO})
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG_PROFILE   ${CMAKE_MODULE_LINKER_FLAGS_DEBUG})
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE_PROFILE ${CMAKE_MODULE_LINKER_FLAGS_RELEASE})

add_compile_definitions(
    $<$<CONFIG:Debug,Debug_Profile>:APEX_CONFIG_DEBUG>
    $<$<CONFIG:DebugGame>:APEX_CONFIG_DEBUGGAME>
    $<$<CONFIG:Development>:APEX_CONFIG_DEVELOPMENT>
    $<$<CONFIG:Release,Release_Profile>:APEX_CONFIG_RELEASE>

    $<$<CONFIG:Debug_Profile,Release_Profile>:APEX_PROFILE>
    $<$<CONFIG:Debug_Profile,Release_Profile>:TRACY_ENABLE>

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

if(MSVC)
    add_compile_options(/GR-) # Disable RTTI
    add_compile_options(/EHs-c- /EHc-) # Disable synchronous exceptions and C++ exceptions
endif()

set(APEX_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
set(APEX_APEX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Apex)
set(APEX_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Tools)

add_subdirectory(ThirdParty)
add_subdirectory(Apex)
add_subdirectory(Tests)
add_subdirectory(Tools)

# Examples
# add_subdirectory(Examples)
