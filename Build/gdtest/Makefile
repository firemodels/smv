
# test makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS


FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/gdtest
bin = .

LIB_DIR = $(BUILD_DIR)/LIBS
INTEL_WIN_LIBDIR = $(LIB_DIR)/intel_win
CLANG_WIN_LIBDIR = $(LIB_DIR)/clang_win
INTEL_LINUX_LIBDIR = $(LIB_DIR)/intel_linux
CLANG_LINUX_LIBDIR = $(LIB_DIR)/clang_linux
GNU_LINUX_LIBDIR = $(LIB_DIR)/gnu_linux
CLANG_OSX_LIBDIR = $(LIB_DIR)/clang_osx

INC_DIR = -I $(SOURCE_DIR)/gd-2.3.3 -I $(SOURCE_DIR)/jpeg-9b
INTEL_WIN_LIBS = $(INTEL_WIN_LIBDIR)\gd.lib $(INTEL_WIN_LIBDIR)\jpeg.lib
CLANG_WIN_LIBS = $(CLANG_WIN_LIBDIR)\gd.lib $(CLANG_WIN_LIBDIR)\jpeg.lib
LINUXOSX_LIBS= -lgd -ljpeg

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

# ------------- intel win ----------------

intel_win : CFLAGS    = -Od /nologo -D BGDWIN32
intel_win : CC        = $(I_CC)
intel_win : exe       = gdtest_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(INTEL_WIN_LIBS)

# ------------- clang win ----------------

clang_win : CFLAGS    = -m64 -D BGDWIN32  
clang_win : LFLAGS    = -m64
clang_win : CC        = clang-cl
clang_win : exe       = gdtest_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(CLANG_WIN_LIBS)

# ------------- intel linux ----------------

intel_linux : CFLAGS    = -O2 -m64
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = gdtest_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -L$(INTEL_LINUX_LIBDIR) $(LINUXOSX_LIBS)

# ------------- gnu linux ----------------

gnu_linux : CFLAGS    = -O2 -m64
gnu_linux : CC        = gcc
gnu_linux : exe       = gdtest_linux

gnu_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -lm -L$(GNU_LINUX_LIBDIR) $(LINUXOSX_LIBS)

# ------------- clang linux ----------------

clang_linux : CFLAGS    = -O2 -m64
clang_linux : CC        = clang
clang_linux : exe       = gdtest_linux

clang_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -L$(CLANG_LINUX_LIBDIR) $(LINUXOSX_LIBS)

# ------------- clang osx ----------------

clang_osx : CFLAGS    = -O0 -m64
clang_osx : LFLAGS    = -m64  -L$(CLANG_OSX_LIBDIR) $(LINUXOSX_LIBS)
clang_osx : CC        = clang
clang_osx : CPP       = clang++
clang_osx : exe       = gdtest_osx

clang_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)
