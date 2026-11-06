@echo off
pushd "%~dp0"
Premake\premake5.exe vs2022
set result=%errorlevel%
popd
exit /b %result%
