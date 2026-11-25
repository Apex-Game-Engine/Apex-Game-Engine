@echo off
rmdir /s /q solutions
mkdir solutions
for /f "delims=" %%k in ('dir ".\build-clang\*.sln" /S /B') do (
	mklink ".\solutions\%%~nxk" "%%~k"
)