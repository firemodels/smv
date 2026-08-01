@echo off
setlocal
set CURDIR=%CD%

cd ..\..\..
set FMROOT=%CD%
cd %CURDIR%

set wc=%CURDIR%\bin\wc
set grep=%CURDIR%\bin\grep
set gawk=%CURDIR%\bin\gawk
set head=%CURDIR%\bin\head

git remote -v | %grep% origin | %head% -1 | %gawk%  "{print $2}" | %gawk% -F ":" "{print $1}">%CURDIR%\githeader.out
set /p GITHEADER=<%CURDIR%\githeader.out

if "%GITHEADER%" == "git@github.com" (
   set GITHEADER=%GITHEADER%:
   git remote -v | %grep% origin | %head% -1 | %gawk% -F ":" "{print $2}" | %gawk% -F\\/ "{print $1}" > %CURDIR%\gituser.out
   set /p GITUSER=<%CURDIR%\gituser.out
) else (
   set GITHEADER=https://github.com/
   git remote -v | %grep% origin | %head% -1 | %gawk% -F "." "{print $2}" | %gawk% -F\\/ "{print $2}" > %CURDIR%\gituser.out
   set /p GITUSER=<%CURDIR%\gituser.out
)

if exist %CURDIR%\gituser.out erase %CURDIR%\gituser.out
if exist %CURDIR%\githeader.out erase %CURDIR%\githeader.out

::git clone https://github.com/libjpeg-turbo/libjpeg-turbo.git
::git clone https://github.com/pnggroup/libpng.git
::git clone https://github.com/madler/zlib.git
::git clone https://github.com/libgd/libgd.git

call :clone_repo libjpeg-turbo  libjpeg-turbo
call :clone_repo pnggroup       libpng
call :clone_repo madler         zlib
call :clone_repo libgd          libgd

goto eof

::-----------------------------------------------------------------------
:clone_repo
::-----------------------------------------------------------------------
  set GITOWNER=%1
  set repo=%2
  set repo_out=%repo%
  
  echo.
  echo ------------------------------------------------------
  echo ------------------------------------------------------
  echo cloning %repo% repo
  echo.

  set GDBUILD=%FMROOT%\gd-build
  if not exist %GDBUILD% mkdir %GDBUILD%
  if not exist %GDBUILD%\src mkdir %GDBUILD%\src

  set repo_dir=%GDBUILD%\src\%repo_out%
  if exist %repo_dir% echo deleting %repo_dir%
  if exist %repo_dir% rmdir /S /Q %repo_dir%
  
  cd %GDBUILD%\src
  git clone %GITHEADER%%GITOWNER%/%repo%.git %repo_out%
  exit /b 0


::-----------------------------------------------------------------------
:eof
::-----------------------------------------------------------------------

exit /b 0


