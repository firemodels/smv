@echo off

IF not EXIST placeholder.txt goto dircheck
echo ***error: This script is running in the wrong directory.
pause
exit
:dircheck

echo.
echo Wrapping up 64 bit Smokeview update
echo.

set "SMVBINDIR=%CD%"
set "FDSBINDIR=%SMVBINDIR%\..\FDS6\bin"

copy "%SMVBINDIR%\fds_test.txt"      "%FDSBINDIR%\fds_test.bat"     >Nul
copy "%SMVBINDIR%\fdsinit_test.txt"  "%FDSBINDIR%\fdsinit_test.bat" >Nul

echo.
echo Associating the .smv file extension with smokeview.exe

ftype smvDoc="%SMVBINDIR%\smokeview.exe" "%%1" >Nul
assoc .smv=smvDoc>Nul

:: adding path

echo.
echo Adding %SMVBINDIR% to the system path
call "%SMVBINDIR%\set_path.exe" -s -m -a "%SMVBINDIR%" >Nul

erase "%SMVBINDIR%\set_path.exe"
echo Press any key to complete update
pause>NUL
