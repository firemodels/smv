@echo off

set ALLDIR=%CD%
cd ..\..
set BUILDDIR=%CD%
set BUILD=clang_win_64

set allprogs=background convert env2mod fds2fed flush get_time getdate hashfile makepo mergepo pnginfo set_path smokediff smokeview smokezip timep wind2fds

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
  dir %prog%_win_64.exe
  exit /b

:eof
echo.
echo ***build complete

cd %ALLDIR%
