@echo off
for %%I in ("%~f0") do set "scriptdir=%%~dpI"
if %1 == intel call %scriptdir%\setup_intel_compilers.bat
if %1 == clang call %scriptdir%\setup_clang_compilers.bat
