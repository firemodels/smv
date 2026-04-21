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
set GAWK=%SMVREPODIR%\Verification\scripts\bin\gawk.exe
set SORT=%SMVREPODIR%\Verification\scripts\bin\sort.exe

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

set WIDTH=250

echo creating %HTMLFILE%
(
echo ^<html^>
echo ^<head^>
echo ^<TITLE^> %SMVREPO% %date% %time% ^</TITLE^>
echo ^</HEAD^>
echo ^<BODY BGCOLOR="#FFFFFF" ^>
echo ^<h2^>  %date% %time% ^<br^>%SMVREPO% ^</h2^>

for %%d in (SMV_User_Guide SMV_Verification_Guide) do (
if %%d == SMV_User_Guide echo ^<a name="changeduser"^>
if %%d == SMV_Verification_Guide echo ^<a name="changedveri"^>

if %%d == SMV_User_Guide         echo ^<h2^>Changed User Images Cases^</h2^>

if %%d == SMV_Verification_Guide echo ^<h2^>Changed Verification Images Cases^</h2^>

if %%d == SMV_User_Guide         set FILELOOP=%CURDIR%\user_nonzerolist.txt 
if %%d == SMV_Verification_Guide set FILELOOP=%CURDIR%\veri_nonzerolist.txt 

echo [^<a href="#changeduser"  ^>Changed User Images          ^</a^>]
echo [^<a href="#Unchangeduser"^>Unchanged User Images        ^</a^>]
echo [^<a href="#changedveri"  ^>Changed Verification Images  ^</a^>]
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

if %%d == SMV_User_Guide         echo ^<h2^>Unchanged User Images Cases^</h2^>
if %%d == SMV_Verification_Guide echo ^<h2^>Unchanged Verification Images Cases^</h2^>

echo [^<a href="#changeduser"  ^>Changed User Images          ^</a^>]
echo [^<a href="#Unchangeduser"^>Unchanged User Images        ^</a^>]
echo [^<a href="#changedveri"  ^>Changed Verification Images  ^</a^>]
echo [^<a href="#Unchangedveri"^>Unchanged Verification Images^</a^>]

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
      echo ^<td^>^<img src="diffs/base/%%A"   width=%WIDTH% ^>^<br^>%%A ^</td^>
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
