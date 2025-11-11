SOURCE_DIR = ..\..\..\Source\get_time
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
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

# ------------- intel win ----------------

intel_win : CFLAGS    = -Od /nologo
intel_win : CC        = $(I_CC)
intel_win : CPP       = $(I_CC)
intel_win : exe       = get_time_win.exe

intel_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin)

# ------------- clang win ----------------

clang_win : CFLAGS    = -m64
clang_win : CC        = clang-cl
clang_win : CPP       = clang-cl
clang_win : exe       = get_time_win.exe

clang_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin)
