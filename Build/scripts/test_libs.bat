@echo off
set LIBDIR=%1
set LUA=%2


call :makelibs libgd.a 
call :makelibs libglui.a 
call :makelibs libglut.a 
call :makelibs libjpeg.a 
call :makelibs libpng.a 
call :makelibs libz.a 
if "%LUA%" == "lua" (
  makelibs liblua.a
  makelibs lpeg.so
)
goto eof

:makelibs
  set lib=%1
  if exist %LIBDIR%\%lib% exit /b
  CURDIR=%CD%`
  cd %LIBDIR%
  makelibs.sh %LUA%
  cd %CURDIR%
  exit /b

:eof
