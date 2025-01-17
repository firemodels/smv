@echo off
set file=%1
for %%f in (%file%) do (
    set base=%%~nf
)
set OPTS=-D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB
set INC=-I ../glut_gl -I ../pthreads -I ../smokeview -I ../glui_v2_1_beta -I ../shared -I ../glew -I . -I ../gd-2.0.15 -I ../zlib128 %file%
clang --analyze %INC% %OPTS% %file% 2> %base%.chk
