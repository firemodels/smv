@echo off

IF "%SETUP_IFORT_COMPILER_64%"=="1" GOTO envexist

  set SETUP_IFORT_COMPILER_64=1

  IF DEFINED ONEAPI_ROOT goto oneapi

:: setup for old Intel compilers

  IF DEFINED ICPP_COMPILER14 set ICPP_COMPILER=%ICPP_COMPILER14%
  IF DEFINED ICPP_COMPILER15 set ICPP_COMPILER=%ICPP_COMPILER15%
  IF DEFINED ICPP_COMPILER16 set ICPP_COMPILER=%ICPP_COMPILER16%
  IF DEFINED ICPP_COMPILER17 set ICPP_COMPILER=%ICPP_COMPILER17%
  IF DEFINED ICPP_COMPILER18 set ICPP_COMPILER=%ICPP_COMPILER18%
  IF DEFINED ICPP_COMPILER19 set ICPP_COMPILER=%ICPP_COMPILER19%
  IF DEFINED ICPP_COMPILER20 set ICPP_COMPILER=%ICPP_COMPILER20%
  IF DEFINED ICPP_COMPILER21 set ICPP_COMPILER=%ICPP_COMPILER21%

  IF NOT DEFINED ICPP_COMPILER (
    echo "*** Error: Intel ICPP_COMPILER environment variable not defined."
  )
  IF DEFINED ICPP_COMPILER (
    if exist "%ICPP_COMPILER%\bin\compilervars.bat" (
      echo Setting up C/C++ compiler environment
      call "%ICPP_COMPILER%\bin\compilervars" intel64
    )
    if not exist "%ICPP_COMPILER%\bin\compilervars.bat" (
      echo.
      echo ***warning compiler setup script,
      echo    "%ICPP_COMPILER%\bin\compilervars.bat",
      echo    does not exist
      echo.
    )
  )

goto envexist

:oneapi
:: setup for Intel OneAPI compmilers
IF DEFINED IFORT_COMPILER19 set ICPP_COMPILER=%IFORT_COMPILER19%
doskey icl=icx.exe $*
doskey icpp=icx.exe $*
doskey ifort=ifort.exe $*

if exist "%ICPP_COMPILER%\..\env\vars.bat" (
  echo Setting up C/C++ compiler environment
  call "%ICPP_COMPILER%\..\env\vars.bat" intel64
)
if not exist "%ICPP_COMPILER%\..\env\vars.bat" (
  echo.
  echo ***warning compiler setup script,
  echo    "%ICPP_COMPILER%\..\env\vars.bat",
  echo    does not exist
  echo.
)


:envexist
