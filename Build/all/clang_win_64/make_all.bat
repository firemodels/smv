@echo off

set ALLDIR=%CD%
set BUILDDIR=%CD%\..\..
set BUILD=clang_win_64

set TARGET=background
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=convert
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=env2mod
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=fds2fed
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=flush
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=get_time
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=getdata
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=hashfile
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

@REM set TARGET=hashfort
@REM echo %BUILDDIR%\%TARGET%\%BUILD%
@REM cd %BUILDDIR%\%TARGET%\%BUILD%
@REM dir
@REM call make_%TARGET%.bat bot

set TARGET=makepo
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=mergepo
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=pnginfo
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=set_path
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=smokediff
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=smokeview
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=smokezip
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=timep
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

set TARGET=wind2fds
echo %BUILDDIR%\%TARGET%\%BUILD%
cd %BUILDDIR%\%TARGET%\%BUILD%
dir
call make_%TARGET%.bat bot

cd %ALLDIR%
