@echo off
set SMVEDITION=SMV6

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
set forbundle=%svn_root%\smv\Build\Bundle\for_bundle
set smvscripts=%svn_root%\smv\scripts
set svzipbuild=%svn_root%\smv\Build\smokezip\%BUILDDIR%
set dem2fdsbuild=%svn_root%\smv\Build\dem2fds\%BUILDDIR%
set svdiffbuild=%svn_root%\smv\Build\smokediff\%BUILDDIR%
set bgbuild=%svn_root%\smv\Build\background\intel_win_64
set hashfilebuild=%svn_root%\smv\Build\hashfile\%BUILDDIR%
set flushfilebuild=%svn_root%\smv\Build\flush\%BUILDDIR%
set timepbuild=%svn_root%\smv\Build\timep\%BUILDDIR%
set windbuild=%svn_root%\smv\Build\wind2fds\%BUILDDIR%
set sh2bat=%svn_root%\smv\Build\sh2bat\intel_win_64
set gettime=%svn_root%\smv\Build\get_time\%BUILDDIR%

set zipbase=%version%_win%platform%
set smvdir=%zipbase%

cd %svn_root%\smv\Build\Bundle\uploads
set uploads=%CD%

echo.
echo --- filling distribution directory ---
echo.
IF EXIST %smvdir% rmdir /S /Q %smvdir%
mkdir %smvdir%
mkdir %smvdir%\hash

CALL :COPY  %svn_root%\smv\Build\set_path\intel_win_64\set_path64.exe "%smvdir%\set_path.exe"

CALL :COPY  %smvbuild%\smokeview_win_%platform%.exe %smvdir%\smokeview.exe

CALL :COPY  %smvscripts%\jp2conv.bat %smvdir%\jp2conv.bat

echo copying .po files
copy %forbundle%\*.po %smvdir%\.>Nul

echo copying .png files
copy %forbundle%\*.png %smvdir%\.>Nul

CALL :COPY  %forbundle%\volrender.ssf %smvdir%\volrender.ssf

CALL :COPY  %bgbuild%\background.exe                    %smvdir%\background.exe
CALL :COPY  %dem2fdsbuild%\dem2fds_win_%platform%.exe   %smvdir%\dem2fds.exe
CALL :COPY  %flushfilebuild%\flush_win_%platform%.exe   %smvdir%\flush.exe
CALL :COPY  %hashfilebuild%\hashfile_win_%platform%.exe %smvdir%\hashfile.exe
CALL :COPY  %svdiffbuild%\smokediff_win_%platform%.exe  %smvdir%\smokediff.exe
CALL :COPY  %svzipbuild%\smokezip_win_%platform%.exe    %smvdir%\smokezip.exe
CALL :COPY  %timepbuild%\timep_win_%platform%.exe       %smvdir%\timep.exe
CALL :COPY  %windbuild%\wind2fds_win_%platform%.exe     %smvdir%\wind2fds.exe

set curdir=%CD%
cd %smvdir%

hashfile smokeview.exe  >  hash\smokeview_%revision%.sha1
hashfile smokezip.exe   >  hash\smokezip_%revision%.sha1
hashfile smokediff.exe  >  hash\smokediff_%revision%.sha1
hashfile dem2fds.exe    >  hash\dem2fds_%revision%.sha1
hashfile background.exe >  hash\background_%revision%.sha1
hashfile hashfile.exe   >  hash\hashfile_%revision%.sha1
hashfile wind2fds.exe   >  hash\wind2fds_%revision%.sha1
cd hash
cat *.sha1              >  %uploads%\%zipbase%.sha1
cd %curdir%

CALL :COPY  %forbundle%\smokeview.ini %smvdir%\smokeview.ini

echo copying textures
mkdir %smvdir%\textures
copy %forbundle%\textures\*.jpg %smvdir%\textures>Nul
copy %forbundle%\textures\*.png %smvdir%\textures>Nul

CALL :COPY  %forbundle%\objects.svo %smvdir%\.

CALL :COPY  %sh2bat%\sh2bat.exe %smvdir%\.

CALL :COPY  %gettime%\get_time_64.exe %smvdir%\get_time.exe

CALL :COPY  %svn_root%\webpages\smv_readme.html %smvdir%\release_notes.html

CALL :COPY  %forbundle%\wrapup_smv_install_%platform%.bat %smvdir%\wrapup_smv_install.bat

echo.
echo --- compressing distribution directory ---
echo.
cd %smvdir%
wzzip -a -r -P %zipbase%.zip * >Nul
rename %zipbase%.zip smoke_update.zip
copy smoke_update.zip ..

echo.
echo --- creating installer ---
echo.
cd ..
if exist smoke_update.exe erase smoke_update.exe
wzipse32 smoke_update.zip -runasadmin -d "c:\Program Files\firemodels\%SMVEDITION%" -c wrapup_smv_install.bat
if exist %zipbase%.exe erase %zipbase%.exe
rename smoke_update.exe %zipbase%.exe

hashfile %zipbase%.exe  >   %smvdir%\hash\%zipbase%.exe.sha1
cd %smvdir%\hash
cat %zipbase%.exe.sha1 >> %uploads%\%zipbase%.sha1

cd ..\..
if not exist %zipbase%.exe echo ***warning: %zipbase%.exe was not created

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


