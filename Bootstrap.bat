@echo off
if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

for /f "tokens=1,* delims= " %%a in ("%*") do set ALL_BUT_FIRST=%%b

set arg_toolset=%1
if "%arg_toolset%" == "msvc" goto MSVC
if "%arg_toolset%" == "clang" goto CLANG
if "%arg_toolset%" == "llvm" goto CLANG
goto UNKGEN

:MSVC
set build_dir=build-msvc
set cmake_gen="Visual Studio 17 2022"
set cmake_toolset=v143
goto CMAKE
:CLANG
set build_dir=build-clang
set cmake_gen="Visual Studio 17 2022"
set cmake_toolset=ClangCL
goto CMAKE
:UNKGEN
echo Unrecognized toolset "%arg_toolset%". Supported toolsets are msvc and clang/llvm
goto end

:CMAKE
cmake -S . -B %build_dir% -G%cmake_gen% -T %cmake_toolset% %ALL_BUT_FIRST%

:end

pause