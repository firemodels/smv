@echo off

set stopscript=0
set release=
set from=
set inc=
set GLUT=glut
set ICON=
set smv_mpi=false
set setglut=

:: parse command line arguments

call :getopts %*

if %stopscript% == 1 (
  exit /b
)

goto eof

:getopts
 if (%1)==() exit /b
 set valid=0
 set arg=%1
 if /I "%1" EQU "-release" (
   set valid=1
   set release=-r
 )
 if /I "%1" EQU "-bot" (
   set valid=1
   set from=bot
 )
 if /I "%1" EQU "-mpi" (
   set valid=1
   set smv_mpi=true
 )
 if /I "%1" EQU "-test" (
   set valid=1
 )
 if /I "%1" EQU "-inc" (
   set valid=1
   set inc=inc
 )
 if /I "%1" EQU "-glut" (
   set valid=1
   set GLUT=glut
   set setglut=1
 )
 if /I "%1" EQU "-freeglut" (
   set valid=1
   set GLUT=freeglut
   set setglut=1
 )
 if /I "%1" EQU "-icon" (
   set valid=1
   set ICON=icon
 )
 shift
 if %valid% == 0 (
   echo.
   echo ***Error: the input argument %arg% is invalid
   set stopscript=1
   exit /b
 )
if not (%1)==() goto getopts
exit /b

:eof
