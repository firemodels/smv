@echo off
:: setup compiler environment
call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat

Title Building getdate for 64 bit Windows

erase *.obj *.exe
make SHELL="%ComSpec%" -f ..\Makefile intel_win_64
if "x%EXIT_SCRIPT%" == "x" goto skip1
exit
:skip1
if x%from% == xbot goto skip_pause
pause
:skip_pause



