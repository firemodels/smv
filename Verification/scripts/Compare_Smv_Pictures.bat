@echo off
setlocal enabledelayedexpansion

set CURDIR=%CD%

cd ..\..\..\fig\smv\Reference_Figures\Default
set FROMDIR=%CD%
set DIFFDIR=%CURDIR%\DIFFS

if exist "%DIFFDIR%" rmdir /s /q "%DIFFDIR%"
mkdir "%DIFFDIR%"

cd %CURDIR%\..\..\Manuals\SMV_User_Guide\SCRIPT_FIGURES
set TODIR=%CD%

cd %CURDIR%\..\..\Manuals
set MANDIR=%CD%

cd %CURDIR%\..\..\..\smv
set SMVREPODIR=%CD%
git describe --dirty --long > %CURDIR%\smvrepo.txt
set /p SMVREPO=<%CURDIR%\smvrepo.txt

cd %CURDIR%\..\..\Manuals\SMV_Summary
set SUMMARYDIR=%CD%

set WEBFROMDIR=%SUMMARYDIR%\diffs\base
set WEBTODIR=%SUMMARYDIR%\images
set WEBDIFFDIR=%SUMMARYDIR%\diffs\images

set HTMLFILE=%SUMMARYDIR%\index.html

for %%d in ( SMV_User_Guide SMV_Verification_Guide ) do (
cd %MANDIR%\%%d\SCRIPT_FIGURES
for %%f in (*.png) do (
    set "FROMFILE=%FROMDIR%\%%f"
    set "TOFILE=%%f"
    set "DIFFFILE=%DIFFDIR%\%%f"
    set "OUTFILE=%DIFFDIR%\%%f.txt"
    if exist "!FROMFILE!" if exist "!TOFILE!" (
       echo comparing %%f
       magick compare -metric RMSE "!FROMFILE!" "!TOFILE!" "!DIFFFILE!" > "!OUTFILE!" 2>&1
    )
)
)

set WIDTH=300

echo creating %HTMLFILE%
(
echo ^<html^>
echo ^<head^>
echo ^<TITLE^> %SMVREPO% %date% %time% ^</TITLE^>
echo ^</HEAD^>
echo ^<BODY BGCOLOR="#FFFFFF" ^>
echo ^<h2^>  %date% %time% ^<br^>%SMVREPO% ^</h2^>

for %%d in (SMV_User_Guide SMV_Verification_Guide) do (
echo ^<a name="%%d"^>
if %%d == SMV_User_Guide         echo [SMV_User_Guide]
if %%d == SMV_User_Guide         echo [^<a href="#SMV_Verification_Guide"^>SMV_Verification_Guide^</a^>]
if %%d == SMV_Verification_Guide echo [^<a href="#SMV_User_Guide"^>SMV_User_Guide^</a^>]
if %%d == SMV_Verification_Guide echo [SMV_Verification_Guide]

echo ^<table border=on^>
cd %MANDIR%\%%d\SCRIPT_FIGURES
for %%f in (*.png) do (
    set "FROMFILE=%FROMDIR%\%%f"
    set "TOFILE=%%f"
    set "DIFFFILE=%DIFFDIR%\%%f"
    set "OUTFILE=%DIFFDIR%\%%f.txt"
    if exist "!FROMFILE!" if exist "!TOFILE!" (
      set /p FIRSTLINE=<!OUTFILE!
      copy !FROMFILE! %WEBFROMDIR%\%%f >NUL 2>&1
      copy !TOFILE!   %WEBTODIR%\%%f   >NUL 2>&1
      copy !DIFFFILE! %WEBDIFFDIR%\%%f >NUL 2>&1
      echo ^<tr^>
      echo ^<td^>^<img src="diffs/base/%%f"   width=%WIDTH% ^>^</td^>
      echo ^<td^>^<img src="images/%%f"       width=%WIDTH% ^>^</td^>
      echo ^<td^>^<img src="diffs/images/%%f" width=%WIDTH% ^>^</td^>
      echo ^</tr^>
      echo ^<tr^>^<td colspan=3 align=center^>%%f - !FIRSTLINE!^</th^>^</tr^>
    )
)
echo ^</table^>
)
echo ^<p^>^<hr^>
echo ^</BODY^>
echo ^</HTML^>
) > %HTMLFILE%
start explorer %HTMLFILE%
