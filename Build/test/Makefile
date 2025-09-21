
# test makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build


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

intel_win_64 : CFLAGS    = -Od /nologo
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = test_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = test_win_64.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64----------------

gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : CFLAGS    = -O2 -m64 -Wall
gnu_linux_64 : LFLAGS    =
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = test_linux_64

gnu_linux_64: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)

gnu_linux_64_check : CFLAGS    = -O2 -m64 -Wall
gnu_linux_64_check: $(srcs)
	clang-tidy $^ --  $(CFLAGS) $(INC)

gnu_linux_64_format: $(srcs)
	clang-format -i $^

# ------------- intel linux 64 ----------------

intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CFLAGS    = -O2 -m64
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = test_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64
gnu_osx_64 : LFLAGS    = -m64
gnu_osx_64 : CC        = gcc
gnu_osx_64 : CPP       = g++
gnu_osx_64 : exe       = test_osx_64

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx_64 : CFLAGS    = -O0 -m64
clang_osx_64 : LFLAGS    = -m64
clang_osx_64 : CC        = clang
clang_osx_64 : CPP       = clang++
clang_osx_64 : exe       = test_osx_64

clang_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)
