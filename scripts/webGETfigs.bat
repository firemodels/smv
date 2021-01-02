@echo off
setlocal EnableDelayedExpansion
set whichguides=%1

::  batch to copy smokview/smokebot or fdsfirebot figures to local repo

::  setup environment variables (defining where repository resides etc) 

set envfile="%userprofile%"\fds_smv_env.bat
IF EXIST %envfile% GOTO endif_envexist
echo ***Fatal error.  The environment setup file %envfile% does not exist. 
echo Create a file named %envfile% and use smv/scripts/fds_smv_env_template.bat
echo as an example.
echo.
echo Aborting now...
pause>NUL
goto:eof

:endif_envexist

call %envfile%
echo.

%svn_drive%

if "%whichguides%" == "websummary" (
  Title Download web summary images

  cd %svn_root%\smv\Manuals\SMV_Summary\images
  pscp -P 22 %linux_logon%:%smokebothome%/.smokebot/Manuals/SMV_Summary/images/*  .

  cd %svn_root%\smv\Manuals\SMV_Summary\images2
  pscp -P 22 %linux_logon%:%smokebothome%/.smokebot/Manuals/SMV_Summary/images2/* .

  cd %svn_root%\smv\Manuals\SMV_Summary\movies
  pscp -P 22 %linux_logon%:%smokebothome%/.smokebot/MovieManuals/SMV_Summary/movies/*  .

  cd %svn_root%\smv\Manuals\SMV_Summary\manuals
  pscp -P 22 %linux_logon%:%smokebothome%/.smokebot/pubs/*.pdf .
  goto eof
)
if "%whichguides%" == "smvug" (
  Title Download smokeview user guide images

  cd %svn_root%\smv\Manuals\SMV_User_Guide\SCRIPT_FIGURES
  pscp -P 22 %linux_logon%:%smokebothome%/.smokebot/Manuals/SMV_User_Guide/SCRIPT_FIGURES/* .
  goto eof
)
if "%whichguides%" == "smvugfork" (
  Title Download smokeview user guide images

  cd %svn_root%\smv\Manuals\SMV_User_Guide\SCRIPT_FIGURES
  pscp -P 22 %linux_logon%:%userhome%/.smokebot/Manuals/SMV_User_Guide/SCRIPT_FIGURES/* .
  goto eof
)
if "%whichguides%" == "smvvg" (
  Title Download smokeview verification guide images

  cd %svn_root%\smv\Manuals\SMV_Verification_Guide\SCRIPT_FIGURES
  pscp -P 22 %linux_logon%:%smokebothome%/.smokebot/Manuals/SMV_Verification_Guide/SCRIPT_FIGURES/* .
  goto eof
)
if "%whichguides%" == "smvvgfork" (
  Title Download smokeview verification guide images

  cd %svn_root%\smv\Manuals\SMV_Verification_Guide\SCRIPT_FIGURES
  pscp -P 22 %linux_logon%:%userhome%/.smokebot/Manuals/SMV_Verification_Guide/SCRIPT_FIGURES/* .
  goto eof
)
if "%whichguides%" == "fdsug" (
  Title Download FDS user guide images

  cd %svn_root%\fds\Manuals\FDS_User_Guide\SCRIPT_FIGURES
  pscp -P 22 %linux_logon%:%firebothome%/.firebot/Manuals/FDS_User_Guide/SCRIPT_FIGURES/* .
  goto eof
)
if "%whichguides%" == "fdsvalg" (
  cd %svn_root%\fds\Manuals\FDS_Validation_Guide\SCRIPT_FIGURES
  for /D %%d in (*) do (
      echo.
      echo copying files from %%d
      cd %%d

      Title Download FDS validation guide %%d images

      pscp -P 22 %linux_logon%:%firebothome%/.firebot/Manuals/FDS_Validation_Guide/SCRIPT_FIGURES/%%d/* .
      cd ..
  )
  goto eof
)
if "%whichguides%" == "fdsverg" (
  Title Download FDS verification guide images

  cd %svn_root%\fds\Manuals\FDS_Verification_Guide\SCRIPT_FIGURES
  pscp -P 22 %linux_logon%:%firebothome%/.firebot/Manuals/FDS_Verification_Guide/SCRIPT_FIGURES/* .

  Title Download FDS verification guide scatterplot images

  cd %svn_root%\fds\Manuals\fds/FDS_Verification_Guide\SCRIPT_FIGURES\Scatterplots
  pscp -P 22 %linux_logon%:%firebothome%/.firebot/Manuals/FDS_Verification_Guide/SCRIPT_FIGURES/Scatterplots/* .
  goto eof
)

:eof
echo.
echo copy complete
pause
