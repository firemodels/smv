@echo off
if defined VSCMD_VER exit /b
set "VSINSTALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
if defined VSCMD_USER set "VSINSTALL=%VSCMD_USER%"
if exist "%VSINSTALL%\vcvars64.bat" goto skip_errorout
  echo ***error: clang-cl setup file
  echo           "%VSINSTALL%\vcvars64.bat"
  exit /b
:skip_errorout
echo *** setting up clang compiler environment
call "%VSINSTALL%\vcvars64.bat" > Nul
if not defined VSCMD_VER echo ***error: clang-cl setup failed. 
