@echo off
setlocal
set "paper=SMV_User_Guide"
set "TEXINPUTS=.;..\LaTeX_Style_Files;"

Title Building %paper%

git describe --abbrev=7 --long --dirty > gitinfo.txt
if errorlevel 1 exit /b %errorlevel%
set /p gitrevision=<gitinfo.txt
echo \newcommand^{\gitrevision^}^{%gitrevision%^} > ..\Bibliography\gitrevision.tex
if errorlevel 1 exit /b %errorlevel%

echo pass 1
pdflatex -interaction nonstopmode %paper% > %paper%.err 2>&1
if errorlevel 1 goto failed

echo bibliography
biber %paper% >> %paper%.err 2>&1
if errorlevel 1 goto failed

echo pass 2
pdflatex -interaction nonstopmode %paper% >> %paper%.err 2>&1
if errorlevel 1 goto failed

echo pass 3
pdflatex -interaction nonstopmode %paper% >> %paper%.err 2>&1
if errorlevel 1 goto failed

if exist ..\Bibliography\gitrevision.tex erase ..\Bibliography\gitrevision.tex
echo %paper% build complete
exit /b 0

:failed
set "build_status=%errorlevel%"
findstr /b /c:"!" /c:"ERROR" %paper%.err
if exist ..\Bibliography\gitrevision.tex erase ..\Bibliography\gitrevision.tex
echo %paper% build failed. See %CD%\%paper%.err.
exit /b %build_status%
