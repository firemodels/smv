@echo off
setlocal
set CURDIR=%CD%
set compile=%1
set glut=%2
if not "x%compile%" == "xclang"      set compile=intel
if not "x%glut%"    == "xfreeglut"  set    glut=glut
echo *** build smokeview using %compile% and %glut%
echo *** cleaning repo
cd ..\..\..
git clean -dxf > Nul
cd Build\smokeview\%compile%_win
start "building %compile% using %glut%" /wait cmd /c  "make_smokeview -%glut%  > %CURDIR%\smokeview_%compile%_%glut%.out"
smokeview_win -v
echo *** build complete