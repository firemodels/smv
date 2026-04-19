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

cd %CURDIR%\..\..\Manuals\SMV_Summary
set SUMMARYDIR=%CD%

set WEBFROMDIR=%SUMMARYDIR%\diffs\base
set WEBTODIR=%SUMMARYDIR%\images
set WEBDIFFDIR=%SUMMARYDIR%\diffs\images

set HTMLFILE=%SUMMARYDIR%\index.html

cd "%TODIR%"
echo FROMDIR: %FROMDIR%
echo   TODIR: %TODIR%
echo DIFFDIR: %DIFFDIR%

for %%f in (*.png) do (
    set "FROMFILE=%FROMDIR%\%%f"
    set "TOFILE=%TODIR%\%%f"
    set "DIFFFILE=%DIFFDIR%\%%f"
    set "OUTFILE=%DIFFDIR%\%%f.txt"
    if exist "!FROMFILE!" if exist "!TOFILE!" (
       echo comparing %%f
       magick compare -metric RMSE "!FROMFILE!" "!TOFILE!" "!DIFFFILE!" > "!OUTFILE!" 2>&1
    )
)

set WIDTH=300

echo creating %HTMLFILE%
(
echo ^<html^>
echo ^<head^>
echo ^<TITLE^>title^</TITLE^>
echo ^</HEAD^>
echo ^<BODY BGCOLOR="#FFFFFF" ^>
echo ^<h2^>h2 title^</h2^>

echo ^<table^>
cd "%TODIR%"
for %%f in (*.png) do (
    set "FROMFILE=%FROMDIR%\%%f"
    set "TOFILE=%TODIR%\%%f"
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
      echo ^<tr^>^<td colspan=3^>%%f - !FIRSTLINE!^</th^>^</tr^>
    )
)
echo ^</table^>
echo ^<p^>^<hr^>
echo ^</BODY^>
echo ^</HTML^>
) > %HTMLFILE%
start explorer %HTMLFILE%


