@echo off

:: build libraries if one is missing
::call ..\..\scripts\test_clang_libs.bat ..\..\LIBS\

erase *.obj *.exe
make -j 4 GLUT="%GLUT%" SHELL="%ComSpec%" SMV_TESTFLAG="%SMV_TESTFLAG% %OPT%" SMV_TESTSTRING="%SMV_TESTSTRING%" SMV_PROFILEFLAG="%SMV_PROFILEFLAG%" SMV_PROFILESTRING="%SMV_PROFILESTRING%" -f ..\Makefile clang_win_64_db


