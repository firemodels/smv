@echo off
set paper=SMV_Technical_Reference_Guide

Title Building %paper%

set TEXINPUTS=.;..\LaTeX_Style_Files;

git describe --abbrev=7 --long --dirty > gitinfo.txt
set /p gitrevision=<gitinfo.txt
echo \newcommand^{\gitrevision^}^{%gitrevision%^} > ..\Bibliography\gitrevision.tex

echo pass 1
pdflatex -interaction nonstopmode %paper% > %paper%.err
bibtex %paper% > %paper%.err
echo pass 2
pdflatex -interaction nonstopmode %paper% > %paper%.err
echo pass 3
pdflatex -interaction nonstopmode %paper% > %paper%.err

find "! LaTeX Error:" %paper%.err
find "Fatal error" %paper%.err
find "Error:" %paper%.err

if exist ..\Bibliography\gitrevision.tex erase ..\Bibliography\gitrevision.tex
echo %paper% build complete
pause


