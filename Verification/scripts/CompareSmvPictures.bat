@echo off
setlocal enabledelayedexpansion

call :is_file_installed magick
if %nothave% == 0 goto skip1
  echo to find a windows installer for imagemagick
  echo do a web search for imagemagick download 
  echo script exiting
  exit /b
:skip1

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
set /p SMVREPOREVISION=<%CURDIR%\smvrepo.txt

cd %CURDIR%\..\..\..\fig
set FIGREPODIR=%CD%
git describe --dirty --long > %CURDIR%\figrepo.txt
set /p FIGREPOREVISION=<%CURDIR%\figrepo.txt

set GAWK=%SMVREPODIR%\Verification\scripts\bin\gawk.exe
set SORT=%SMVREPODIR%\Verification\scripts\bin\sort.exe
set GREP=%SMVREPODIR%\Verification\scripts\bin\grep.exe
set WC=%SMVREPODIR%\Verification\scripts\bin\wc.exe

set SMVBASE=smokeview_win.exe
set SMVDIR=%CURDIR%\..\..\Build\smokeview\intel_win
set SMVEXE=%SMVDIR%\%SMVBASE%
%SMVEXE% -v 2>nul | %GREP% Revision | %GREP% SMV | %GAWK% "{print $3}" > %CURDIR%\smvrevision.txt
set /p SMOKEVIEWREVISION=<%CURDIR%\smvrevision.txt

set BLURFROMDIR=%CURDIR%\blurfrom
if exist %BLURFROMDIR% rmdir /s /q %BLURFROMDIR%
mkdir %BLURFROMDIR%

set BLURTODIR=%CURDIR%\blurto
if exist %BLURTODIR% rmdir /s /q %BLURTODIR%
mkdir %BLURTODIR%

cd %CURDIR%\..\..\Manuals\SMV_Summary
set SUMMARYDIR=%CD%

set WEBFROMDIR=%SUMMARYDIR%\diffs\base
set WEBTODIR=%SUMMARYDIR%\images
set WEBDIFFDIR=%SUMMARYDIR%\diffs\images

set HTMLFILE=%SUMMARYDIR%\index.html

for %%d in ( SMV_User_Guide SMV_Verification_Guide ) do (
if %%d == SMV_User_Guide         set FILELIST=%CURDIR%\user_filelist.txt
if %%d == SMV_Verification_Guide set FILELIST=%CURDIR%\veri_filelist.txt
if exist !FILELIST! erase !FILELIST!
cd %MANDIR%\%%d\SCRIPT_FIGURES
for %%f in (*.png) do (
    set "FROMFILE=%FROMDIR%\%%f"
    set "TOFILE=%%f"
    set "BLURFROMFILE=%BLURFROMDIR%\%%f"
    set "BLURTOFILE=%BLURTODIR%\%%f"
    set "DIFFFILE=%DIFFDIR%\%%f"
    set "OUTFILE=%DIFFDIR%\%%f.txt"
    if exist "!FROMFILE!" if exist "!TOFILE!" (
       echo comparing %%f

       magick "!FROMFILE!" -blur 0x2 "!BLURFROMFILE!"
       magick "!TOFILE!"   -blur 0x2 "!BLURTOFILE!"

       magick compare -metric RMSE "!BLURFROMFILE!" "!BLURTOFILE!" "!DIFFFILE!" > "!OUTFILE!" 2>&1
       magick "!BLURFROMFILE!" "!BLURTOFILE!" -compose difference -composite -negate "!DIFFFILE!"       
       %GAWK% -v fname="%%f" -F"[()]" "{ if ($2 < 0.02) $2 = 0; print fname, $2 }" !OUTFILE! >> !FILELIST!
    )
)
)

cd %CURDIR%

%SORT% -k2,2gr user_filelist.txt -o user_filelist.txt 
%SORT% -k2,2gr veri_filelist.txt -o veri_filelist.txt

%GAWK% -f zero.awk     user_filelist.txt > user_zerolist.txt
%GAWK% -f zero.awk     veri_filelist.txt > veri_zerolist.txt
%GAWK% -f nonzero.awk  user_filelist.txt > user_nonzerolist.txt
%GAWK% -f nonzero.awk  veri_filelist.txt > veri_nonzerolist.txt
type user_filelist.txt > total_filelist.txt
type veri_filelist.txt >> total_filelist.txt

%GAWK% -f nonzero.awk  total_filelist.txt | wc -l > differences_totallist.txt
set /p ndiffs=<differences_totallist.txt

%GAWK% -f errors.awk   total_filelist.txt | wc -l > errors_totallist.txt
set /p nerrors=<errors_totallist.txt


set WIDTH=250

echo creating %HTMLFILE%
(
echo ^<html^>
echo ^<head^>
echo ^<TITLE^> %SMVREPOREVISION% %date% %time% ^</TITLE^>
echo ^</HEAD^>
echo ^<BODY BGCOLOR="#FFFFFF" ^>

echo ^<h2^>  Image Comparison Summary %date% %time% ^</h2^>
echo ^<br^>^<table^>
echo ^<tr^>^<th align=left^>Smokeview version:^</th^>^<td^>%SMOKEVIEWREVISION%^</td^>^</tr^>
echo ^<tr^>^<th align=left^>SMV revision:^</th^>^<td^>%SMVREPOREVISION%^</td^>^</tr^>
echo ^<tr^>^<th align=left^>FIG revision:^</th^>^<td^>%FIGREPOREVISION%^</td^>^</tr^>
echo ^<tr^>^<th align=left^>Metric/Tolerance:^</th^>^<td^>0.2^</td^>^</tr^>
echo ^<tr^>^<th align=left^>Difference/Errors:^</th^>^<td^>%ndiffs%/%nerrors%^</td^>^</tr^>
echo ^</table^>

for %%d in (SMV_User_Guide SMV_Verification_Guide) do (
if %%d == SMV_User_Guide echo ^<a name="changeduser"^>
if %%d == SMV_Verification_Guide echo ^<a name="changedveri"^>

if %%d == SMV_User_Guide         echo ^<h2^>Changed User Images^</h2^>
if %%d == SMV_Verification_Guide echo ^<h2^>Changed Verification Images^</h2^>

if %%d == SMV_User_Guide         set FILELOOP=%CURDIR%\user_nonzerolist.txt 
if %%d == SMV_Verification_Guide set FILELOOP=%CURDIR%\veri_nonzerolist.txt 

if %%d == SMV_User_Guide         echo [Changed User Images]
if %%d == SMV_Verification_Guide echo [^<a href="#changeduser"^>Changed User Images^</a^>]
                                 echo [^<a href="#Unchangeduser"^>Unchanged User Images        ^</a^>]
if %%d == SMV_User_Guide         echo [^<a href="#changedveri"  ^>Changed Verification Images  ^</a^>]
if %%d == SMV_Verification_Guide echo [Changed Verification Images]
echo [^<a href="#Unchangedveri"^>Unchanged Verification Images^</a^>]

echo ^<table border=on^>
echo ^<tr^>^<th^>Reference^</th^>^<th^>Current^</th^>^<th^>Difference^</th^>^</tr^>
cd %MANDIR%\%%d\SCRIPT_FIGURES
for /f "tokens=1,2" %%A in (!FILELOOP!) do (
    set "FROMFILE=%FROMDIR%\%%A"
    set "TOFILE=%%A"
    set "DIFFFILE=%DIFFDIR%\%%A"
    set "OUTFILE=%DIFFDIR%\%%A.txt"
    if exist "!FROMFILE!" if exist "!TOFILE!" (
      copy !FROMFILE! %WEBFROMDIR%\%%A >NUL 2>&1
      copy !TOFILE!   %WEBTODIR%\%%A   >NUL 2>&1
      copy !DIFFFILE! %WEBDIFFDIR%\%%A >NUL 2>&1
      echo ^<tr^>
      echo ^<td^>^<img src="diffs/base/%%A"   width=%WIDTH% ^>^</td^>
      echo ^<td^>^<img src="images/%%A"       width=%WIDTH% ^>^</td^>
      echo ^<td^>^<img src="diffs/images/%%A" width=%WIDTH% ^>^</td^>
      echo ^</tr^>
      echo ^<tr^>^<td colspan=3 align=center^>%%A - %%B^</th^>^</tr^>
    )
)
echo ^</table^>

if %%d == SMV_User_Guide echo ^<a name="Unchangeduser"^>
if %%d == SMV_Verification_Guide echo ^<a name="UnchangedVeri"^>

if %%d == SMV_User_Guide         echo ^<h2^>Unchanged User Images^</h2^>
if %%d == SMV_Verification_Guide echo ^<h2^>Unchanged Verification Images^</h2^>

echo [^<a href="#changeduser"  ^>Changed User Images          ^</a^>]
if %%d == SMV_User_Guide         echo [Unchanged User Images]
if %%d == SMV_Verification_Guide echo [^<a href="#Unchangeduser"^>Unchanged User Images        ^</a^>]
                                 echo [^<a href="#changedveri"  ^>Changed Verification Images  ^</a^>]
if %%d == SMV_User_Guide         echo [^<a href="#Unchangedveri"^>Unchanged Verification Images^</a^>]
if %%d == SMV_Verification_Guide echo [Unchanged Verification Images]

if %%d == SMV_User_Guide         set FILELOOP=%CURDIR%\user_zerolist.txt 
if %%d == SMV_Verification_Guide set FILELOOP=%CURDIR%\veri_zerolist.txt 

echo ^<table border=on^>
echo ^<tr^>
set count=0
cd %MANDIR%\%%d\SCRIPT_FIGURES
for /f "tokens=1,2" %%A in (!FILELOOP!) do (
    set "FROMFILE=%FROMDIR%\%%A"
    set "TOFILE=%%A"
    set "DIFFFILE=%DIFFDIR%\%%A"
    set "OUTFILE=%DIFFDIR%\%%A.txt"
    if exist "!FROMFILE!" if exist "!TOFILE!" (
      copy !FROMFILE! %WEBFROMDIR%\%%A >NUL 2>&1
      copy !TOFILE!   %WEBTODIR%\%%A   >NUL 2>&1
      copy !DIFFFILE! %WEBDIFFDIR%\%%A >NUL 2>&1
      set /a count+=1
      echo ^<td align=center^>^<img src="diffs/base/%%A"   width=%WIDTH% ^>^<br^>%%A ^</td^>
      if !count! geq 4 echo ^</tr^>^<tr^>
      if !count! geq 4 set count=0
    )
)
echo ^</tr^>
echo ^</table^>

)
echo ^<p^>^<hr^>
echo ^</BODY^>
echo ^</HTML^>
) > %HTMLFILE%
start explorer %HTMLFILE%

goto eof

:: -------------------------------------------------------------
:is_file_installed
:: -------------------------------------------------------------

  set program=%1
  %program% --help 1> %temp%\file_exist.txt 2>&1
  type %temp%\file_exist.txt | find /i /c "not recognized" > %temp%\file_exist_count.txt
  set /p nothave=<%temp%\file_exist_count.txt
  if %nothave% == 1 (
    echo ***error: %program% not installed or not in path
    exit /b 1
  )
  exit /b 0

:eof
