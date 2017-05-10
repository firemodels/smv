@echo off

:: batch file to create a test smokeview bundle

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

set platform=%1

set version=%smv_revision%
set zipbase=%version%_win%platform%
set smvdir=%svn_root%\smv\uploads\%zipbase%
set smvscripts=%svn_root%\smv\scripts
set forbundle=%svn_root%\smv\for_bundle
set sh2bat=%svn_root%\smv\Build\sh2bat\intel_win_64
set smvbuild=%svn_root%\smv\Build

cd %forbundle%

echo.
echo --- filling distribution directory ---
echo.
IF EXIST %smvdir% rmdir /S /Q %smvdir%
mkdir %smvdir%
mkdir %smvdir%\MD5

CALL :COPY %smvbuild%\smokeview\intel_win_%platform%\smokeview_win_test_%platform%.exe %smvdir%\smokeview.exe

CALL :COPY  %smvscripts%\jp2conv.bat %smvdir%\jp2conv.bat

echo copying .po files
copy %forbundle%\*.po %smvdir%\.>Nul

CALL :COPY %forbundle%\volrender.ssf %smvdir%\volrender.ssf

CALL :COPY %smvbuild%\background\intel_win_64\background.exe %smvdir%\background.exe
CALL :COPY %smvbuild%\dem2fds\intel_win_%platform%\dem2fds_win_%platform%.exe %smvdir%\dem2fds.exe
CALL :COPY %smvbuild%\hashfile\intel_win_%platform%\hashfile_win_%platform%.exe %smvdir%\hashfile.exe
CALL :COPY %smvbuild%\set_path\intel_win_64\set_path64.exe "%smvdir%\set_path.exe"
CALL :COPY %smvbuild%\smokediff\intel_win_%platform%\smokediff_win_%platform%.exe %smvdir%\smokediff.exe
CALL :COPY %smvbuild%\smokezip\intel_win_%platform%\smokezip_win_%platform%.exe %smvdir%\smokezip.exe
CALL :COPY %smvbuild%\wind2fds\intel_win_%platform%\wind2fds_win_%platform%.exe %smvdir%\wind2fds.exe

set curdir=%CD%
cd %smvdir%

certutil -hashfile background.exe MD5 >  MD5\background_%revision%.md5
certutil -hashfile dem2fds.exe    MD5 >  MD5\dem2fds_%revision%.md5
certutil -hashfile set_path.exe   MD5 >  MD5\set_path.md5
certutil -hashfile smokediff.exe  MD5 >  MD5\smokediff_%revision%.md5
certutil -hashfile smokezip.exe   MD5 >  MD5\smokezip_%revision%.md5
certutil -hashfile wind2fds.exe   MD5 >  MD5\wind2fds_%revision%.md5
cd %curdir%

CALL :COPY %forbundle%\objects.svo %smvdir%\.

CALL :COPY %sh2bat%\sh2bat.exe %smvdir%\sh2bat.exe

CALL :COPY %forbundle%\wrapup_smv_install_%platform%.bat %smvdir%\wrapup_smv_install.bat

CALL :COPY %forbundle%\smokeview.ini %smvdir%\smokeview.ini

echo copying textures
mkdir %smvdir%\textures
copy %forbundle%\textures\*.jpg %smvdir%\textures>Nul
copy %forbundle%\textures\*.png %smvdir%\textures>Nul

echo.
echo --- compressing distribution directory ---
echo.
cd %smvdir%
wzzip -a -r -p %zipbase%.zip *>Nul

echo.
echo --- creating installer ---
echo.
wzipse32 %zipbase%.zip -runasadmin -d "c:\Program Files\firemodels\%smv_edition%" -c wrapup_smv_install.bat

certutil -hashfile %zipbase%.exe MD5 >   MD5\%zipbase%.exe.md5

CALL :COPY %zipbase%.exe ..\.

echo.
echo --- Smokeview win%platform% test installer built ---
echo.

cd %CURDIR%
GOTO :EOF

:COPY
set label=%~n1%~x1
set infile=%1
set infiletime=%~t1
set outfile=%2
IF EXIST %infile% (
   echo copying %label% %infiletime%
   copy %infile% %outfile% >Nul
) ELSE (
   echo.
   echo *** warning: %infile% does not exist
   echo.
   pause
)
exit /b

