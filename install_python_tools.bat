@echo off
call setenv
cmake --build ./build-msvc/Tools/PythonTools --target install --config ToolRelease