@echo off

:: Windows batch file to build a smokeview bundle

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
set BUILDDIR=intel_win_%platform%

set version=%smv_version%
set smvbuild=%svn_root%\smv\Build\smokeview\%BUILDDIR%
set forbundle=%svn_root%\smv\for_bundle
set smvscripts=%svn_root%\smv\scripts
set svzipbuild=%svn_root%\smv\Build\smokezip\%BUILDDIR%
set dem2fdsbuild=%svn_root%\smv\Build\dem2fds\%BUILDDIR%
set svdiffbuild=%svn_root%\smv\Build\smokediff\%BUILDDIR%
set bgbuild=%svn_root%\smv\Build\background\intel_win_64
set windbuild=%svn_root%\smv\Build\wind2fds\%BUILDDIR%
set sh2bat=%svn_root%\smv\Build\sh2bat\intel_win_64
set md5hash=%svn_root%\smv\Utilities\Scripts\md5hash.bat

set zipbase=smv_%version%_win%platform%
set smvdir=%zipbase%

cd %svn_root%\smv\uploads
set upload=%CD%

echo.
echo --- filling distribution directory ---
echo.
IF EXIST %smvdir% rmdir /S /Q %smvdir%
mkdir %smvdir%
mkdir %smvdir%\MD5

CALL :COPY  %svn_root%\smv\Build\set_path\intel_win_64\set_path64.exe "%smvdir%\set_path.exe"

CALL :COPY  %smvbuild%\smokeview_win_%platform%.exe %smvdir%\smokeview.exe

CALL :COPY  %smvscripts%\jp2conv.bat %smvdir%\jp2conv.bat

echo copying .po files
copy %forbundle%\*.po %smvdir%\.>Nul

CALL :COPY  %forbundle%\volrender.ssf %smvdir%\volrender.ssf

CALL :COPY  %svzipbuild%\smokezip_win_%platform%.exe    %smvdir%\smokezip.exe
CALL :COPY  %svdiffbuild%\smokediff_win_%platform%.exe  %smvdir%\smokediff.exe
CALL :COPY  %dem2fdsbuild%\dem2fds_win_%platform%.exe   %smvdir%\dem2fds.exe
CALL :COPY  %bgbuild%\background.exe                    %smvdir%\background.exe
CALL :COPY  %windbuild%\wind2fds_win_%platform%.exe     %smvdir%\wind2fds.exe

set curdir=%CD%
cd %smvdir%

call %md5hash% smokezip.exe   >  MD5\smokezip_%revision%.md5
call %md5hash% smokediff.exe  >  MD5\smokediff_%revision%.md5
call %md5hash% dem2fds.exe    >  MD5\dem2fds_%revision%.md5
call %md5hash% background.exe >  MD5\background_%revision%.md5
call %md5hash% wind2fds.exe   >  MD5\wind2fds_%revision%.md5
cd MD5
cat *.md5                     >  smv_%revision%win_bundle.md5s
cd %curdir%

CALL :COPY  %forbundle%\smokeview.ini %smvdir%\smokeview.ini

echo copying textures
mkdir %smvdir%\textures
copy %forbundle%\textures\*.jpg %smvdir%\textures>Nul
copy %forbundle%\textures\*.png %smvdir%\textures>Nul

CALL :COPY  %forbundle%\objects.svo %smvdir%\.

if "%platform%"=="64" CALL :COPY  %forbundle%\glew32_x64.dll %smvdir%\.

if "%platform%"=="64" CALL :COPY  %forbundle%\pthreadVC2_x64.dll %smvdir%\.

CALL :COPY  %sh2bat%\sh2bat.exe %smvdir%\.

CALL :COPY  %svn_root%\webpages\smv_readme.html %smvdir%\release_notes.html

CALL :COPY  %forbundle%\wrapup_smv_install_%platform%.bat %smvdir%\wrapup_smv_install.bat

echo.
echo --- compressing distribution directory ---
echo.
cd %smvdir%
wzzip -a -r -P %zipbase%.zip * >Nul

echo.
echo --- creating installer ---
echo.
wzipse32 %zipbase%.zip -runasadmin -d "C:\Program Files\firemodels\%smv_edition%" -c wrapup_smv_install.bat

call %md5hash% %zipbase%.exe>   MD5\%zipbase%.exe.md5
copy MD5\%zipbase%.exe.md5 ..\%zipbase%.exe.md5
cd MD5
cat %zipbase%.exe.md5 >> smv_%revision%win_bundle.md5s
cd ..

copy  %zipbase%.exe ..\.>Nul

CALL :COPY  %zipbase%.exe "%upload%"

echo.
echo --- Smokeview win%platform% installer built
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


