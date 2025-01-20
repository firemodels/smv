@echo off
set file=%1
set option=%2
for %%f in (%file%) do (
    set base=%%~nf
)
set "CLANGOPTS=--analyze"
if not "x%option%" == "x-t%" goto endif1
  set "CLANGOPTS=--analyze -Xanalyzer -analyzer-checker=core,deadcode"
:endif1
set COPTS=-D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB
set INC=-I ../glut_gl -I ../pthreads -I ../smokeview -I ../glui_v2_1_beta -I ../shared -I ../glew -I . -I ../gd-2.0.15 -I ../zlib128 %file%
clang %CLANGOPTS% %INC% %COPTS% %file% 2> %base%.chk
