@echo off
rmdir /s /q solutions
mkdir solutions
for /f "delims=" %%k in ('dir ".\build-msvc\*.sln" /S /B') do (
	mklink ".\solutions\%%~nxk" "%%~k"
)