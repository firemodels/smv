SOURCE_DIR = ..\..\..\Source\get_time
INC_DIR =  -I $(SOURCE_DIR)
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)
bin = .

# Definition of the object variables

obj =    get_time.o
objwin = $(obj:.o=.obj)

# main has to be compiled without optimization (on a Windows PC) otherwise errors occcur
main.obj: CFLAGS = -Od /nologo

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .cpp

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<

# ********  rules for each platform supported *************

# ------------- intel win 64 ----------------

intel_win_64 : CFLAGS    = -Od /nologo
intel_win_64 : CC        = $(I_CC)
intel_win_64 : CPP       = $(I_CC)
intel_win_64 : exe       = get_time_win_64.exe

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D GLEW_STATIC -D MINGW
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = g++
gnu_win_64 : exe       = get_time_win_64.exe

gnu_win_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- clang win 64 ----------------

clang_win_64 : CFLAGS    = -m64 -D GLEW_STATIC -D MINGW
clang_win_64 : CC        = clang-cl
clang_win_64 : CPP       = clang-cl
clang_win_64 : exe       = get_time_win_64.exe

clang_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin)
