@echo off
set SMVEDITION=SMV6

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
set  smvdir=%svn_root%\smv\Build\Bundle\uploads\%zipbase%
set uploads=%svn_root%\smv\Build\Bundle\uploads
set smvscripts=%svn_root%\smv\scripts
set forbundle=%svn_root%\smv\Build\BUndle\for_bundle
set sh2bat=%svn_root%\smv\Build\sh2bat\intel_win_64
set gettime=%svn_root%\smv\Build\get_time\intel_win_64
set smvbuild=%svn_root%\smv\Build

cd %forbundle%

echo.
echo --- filling distribution directory ---
echo.
IF EXIST %smvdir% rmdir /S /Q %smvdir%
mkdir %smvdir%
mkdir %smvdir%\hash

CALL :COPY %smvbuild%\smokeview\intel_win_%platform%\smokeview_win_test_%platform%.exe %smvdir%\smokeview.exe

CALL :COPY  %smvscripts%\jp2conv.bat %smvdir%\jp2conv.bat

echo copying .png files
copy %forbundle%\*.png %smvdir%\.>Nul

echo copying .po files
copy %forbundle%\*.po %smvdir%\.>Nul

CALL :COPY %forbundle%\volrender.ssf %smvdir%\volrender.ssf

CALL :COPY %smvbuild%\background\intel_win_64\background.exe                      %smvdir%\background.exe
CALL :COPY %smvbuild%\dem2fds\intel_win_%platform%\dem2fds_win_%platform%.exe     %smvdir%\dem2fds.exe
CALL :COPY %smvbuild%\flush\intel_win_%platform%\flush_win_%platform%.exe         %smvdir%\flush.exe
CALL :COPY %smvbuild%\hashfile\intel_win_%platform%\hashfile_win_%platform%.exe   %smvdir%\hashfile.exe
CALL :COPY %smvbuild%\set_path\intel_win_64\set_path64.exe                        "%smvdir%\set_path.exe"
CALL :COPY %smvbuild%\smokediff\intel_win_%platform%\smokediff_win_%platform%.exe %smvdir%\smokediff.exe
CALL :COPY %smvbuild%\smokezip\intel_win_%platform%\smokezip_win_%platform%.exe   %smvdir%\smokezip.exe
CALL :COPY %smvbuild%\timep\intel_win_%platform%\timep_win_%platform%.exe         %smvdir%\timep.exe
CALL :COPY %smvbuild%\wind2fds\intel_win_%platform%\wind2fds_win_%platform%.exe   %smvdir%\wind2fds.exe

set curdir=%CD%
cd %smvdir%

hashfile hashfile.exe   >  hash\hashfile_%smv_revision%.sha1
hashfile background.exe >  hash\background_%smv_revision%.sha1
hashfile dem2fds.exe    >  hash\dem2fds_%smv_revision%.sha1
hashfile set_path.exe   >  hash\set_path_%smv_revision%.sha1
hashfile smokediff.exe  >  hash\smokediff_%smv_revision%.sha1
hashfile smokezip.exe   >  hash\smokezip_%smv_revision%.sha1
hashfile smokeview.exe  >  hash\smokeview_%smv_revision%.sha1
hashfile wind2fds.exe   >  hash\wind2fds_%smv_revision%.sha1
cd hash
cat *.sha1              >  %uploads%\%zipbase%.sha1

cd %curdir%

CALL :COPY %forbundle%\objects.svo                       %smvdir%\.
CALL :COPY %sh2bat%\sh2bat_win_64.exe                    %smvdir%\sh2bat.exe
CALL :COPY %gettime%\get_time_64.exe                     %smvdir%\get_time.exe
CALL :COPY %forbundle%\wrapup_smv_install_%platform%.bat %smvdir%\wrapup_smv_install.bat
CALL :COPY %forbundle%\smokeview.ini                     %smvdir%\smokeview.ini

echo copying textures
mkdir %smvdir%\textures
copy %forbundle%\textures\*.jpg                          %smvdir%\textures>Nul
copy %forbundle%\textures\*.png                          %smvdir%\textures>Nul

echo.
echo --- compressing distribution directory ---
echo.
cd %smvdir%
wzzip -a -r -p %zipbase%.zip *>Nul
rename %zipbase%.zip smoketest_update.zip
copy smoketest_update.zip ..

echo.
echo --- creating installer ---
echo.
cd ..
if exist smoketest_update.exe erase smoketest_update.exe
wzipse32 smoketest_update.zip -runasadmin -d "c:\Program Files\firemodels\%SMVEDITION%" -c wrapup_smv_install.bat
if exist %zipbase%.exe erase %zipbase%.exe
rename smoketest_update.exe %zipbase%.exe

hashfile %zipbase%.exe  >   %smvdir%\hash\%zipbase%.exe.sha1
cd %smvdir%\hash
cat %zipbase%.exe.sha1 >> %uploads%\%zipbase%.sha1

cd ..\..
if not exist %zipbase%.exe echo ***warning: %zipbase%.exe was not created

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

