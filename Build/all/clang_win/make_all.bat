@echo off

set ALLDIR=%CD%
cd ..\..
set BUILDDIR=%CD%
set BUILD=clang_win

set allprogs=fds2fed pnginfo smokediff smokeview smokezip wind2fds

for %%x in ( %allprogs% ) do ( call :make_prog %%x )
goto eof

:make_prog
  set prog=%1
  set TARGET=%prog%
  cd %BUILDDIR%\%TARGET%\%BUILD%
  echo.
  echo -------------------------------------------------------
  echo building %prog% in %BUILDDIR%\%TARGET%\%BUILD%
  echo -------------------------------------------------------
  call make_%TARGET% bot
  echo.
  dir %prog%_win.exe
  exit /b

:eof
echo.
echo ***build complete

cd %ALLDIR%
