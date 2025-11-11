
# test makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS


FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/test
bin = .

# Definition of the object variables

srcs =    main.c
obj = $(srcs:.c=.o)
objwin = $(obj:.o=.obj)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<

# ------------- intel win 64 ----------------

intel_win : CFLAGS    = -Od /nologo
intel_win : CC        = $(I_CC)
intel_win : exe       = test_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- clang win 64 ----------------

clang_win : CFLAGS    = -m64
clang_win : LFLAGS    = -m64
clang_win : CC        = clang-cl
clang_win : exe       = test_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(objwin)

# ------------- gnu linux 64----------------

gnu_linux : CFLAGS    = -O2 -m64 -Wall
gnu_linux : LFLAGS    =
gnu_linux : CC        = gcc
gnu_linux : exe       = test_linux

gnu_linux: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)

gnu_linux_check : CFLAGS    = -O2 -m64 -Wall
gnu_linux_check: $(srcs)
	clang-tidy $^ --  $(CFLAGS) $(INC)

gnu_linux_format: $(srcs)
	clang-format -i $^

# ------------- intel linux 64 ----------------

intel_linux : CFLAGS    = -O2 -m64
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = test_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx 64 ----------------

gnu_osx : CFLAGS    = -O0 -m64
gnu_osx : LFLAGS    = -m64
gnu_osx : CC        = gcc
gnu_osx : CPP       = g++
gnu_osx : exe       = test_osx

gnu_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx : CFLAGS    = -O0 -m64
clang_osx : LFLAGS    = -m64
clang_osx : CC        = clang
clang_osx : CPP       = clang++
clang_osx : exe       = test_osx

clang_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)
