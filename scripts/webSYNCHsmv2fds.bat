@echo off

:: setup environment variables (defining where repository resides etc) 

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

set CURDIR=%CD%
call %envfile%

%svn_drive%

set fdsdir=%svn_root%\fds\Manuals\Bibliography
set smvdir=%svn_root%\smv\Manuals\Bibliography

call :COPY %smvdir%\FDS_general.bib  %fdsdir%
call :COPY %smvdir%\FDS_mathcomp.bib %fdsdir%
call :COPY %smvdir%\FDS_refs.bib     %fdsdir%
call :COPY %smvdir%\authors.tex      %fdsdir%

set fdsdir=%svn_root%\fds\Source
set smvdir=%svn_root%\smv\Source\smokeview

call :COPY %smvdir%\gsmv.f90 %fdsdir%

cd %CURDIR%
GOTO EOF

:COPY
set label=%~n1%~x1
set infile=%1
set outfile=%2
IF EXIST %infile% (
   echo copying %label% from smv to fds repo
   copy %infile% %outfile% >Nul
) ELSE (
   echo.
   echo *** warning: %infile% does not exist
   echo.
   pause
)
exit /b


:EOF
pause
