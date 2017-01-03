@echo off

:: batch file used to update Windows, Linux and OSX GIT repos

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

:: location of batch files used to set up Intel compilation environment

call %envfile%

set smv_revision_xxx=unknown
set fds_revision_xxx=unknown
if exist %userprofile%\smv_revision.txt (
  set /p smv_revision_xxx=<%userprofile%\smv_revision.txt
)
if exist %userprofile%\fds_revision.txt (
  set /p fds_revision_xxx=<%userprofile%\fds_revision.txt
)
echo smv_revision=%smv_revision_xxx%
echo fds_revision=%fds_revision_xxx%

pause
