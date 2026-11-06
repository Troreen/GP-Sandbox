@echo off
setlocal
pushd "%~dp0"
if not exist "Bin\Debug\Sandbox.exe" (
 echo Build Debug first: Build.bat
 popd
 exit /b 1
)
start "GP Sandbox" /D "%~dp0Bin\Debug" "%~dp0Bin\Debug\Sandbox.exe"
popd
