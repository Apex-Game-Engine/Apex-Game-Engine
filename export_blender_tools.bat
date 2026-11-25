@echo off
mkdir blender_extensions
call install_python_tools
copy "Tools\bin\PyApex.*.pyd" "Tools\BlenderTools\ApexBlenderTools\"
"%BLENDER_PATH%\blender.exe" --command extension build --source-dir "Tools\BlenderTools\ApexBlenderTools" --output-dir blender_extensions
pause
