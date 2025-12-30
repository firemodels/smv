@echo off

set debug=
set from=
set GLUT=glut
set ICON=
set inc=
set release=
set SMV_PROFILEFLAG=
set SMV_PROFILESTRING=
set setglut=
set stopscript=0
set SANITIZE=0

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

 if /I "%1" EQU "-bot" (
   set valid=1
   set from=bot
 )
 if /I "%1" EQU "bot" (
   set valid=1
   set from=bot
 )
 if /I "%1" EQU "-debug" (
   set valid=1
   set debug=_db
 )
 if /I "%1" EQU "-freeglut" (
   set valid=1
   set GLUT=freeglut
   set setglut=1
 )
 if /I "%1" EQU "-glut" (
   set valid=1
   set GLUT=glut
   set setglut=1
 )
 if /I "%1" EQU "-help" (
   set valid=1
   call :usage
   set stopscript=1
   exit /b 1
 )
 if /I "%1" EQU "-icon" (
   set valid=1
   set ICON=icon
 )
 if /I "%1" EQU "-inc" (
   set valid=1
   set inc=inc
 )
 if /I "%1" EQU "-sanitize" (
   set valid=1
   set SANITIZE=1
 )
 if /I "%1" EQU "-profile" (
   set valid=1
   set SMV_PROFILEFLAG=-pg
   set SMV_PROFILESTRING=p
 )
 shift
 if %valid% == 0 (
   echo.
   echo ***Error: the input argument %arg% is invalid
   set stopscript=1
   exit /b 1
 )
if not (%1)==() goto getopts
exit /b 0

::-----------------------------------------------------------------------
:usage
::-----------------------------------------------------------------------

:usage
echo Build smokeview
echo.
echo Options:
echo -bot      - run by a bot, do not pause at end of script
echo -debug    - build a debug version of smokeview
echo -freeglut - build smokeview using the freeglut library
echo -glut     - build smokeview using the glut library
echo -help     - display this message
echo -icon     - ceate an icon
echo -inc      - incremental build
echo -sanitize - build using sanitize options for debugging
echo             (-fsanitize=address -fsanitize=undefined -fsanitize=memory)
exit /b 0



:eof
exit /b 0
