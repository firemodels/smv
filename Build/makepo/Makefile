SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR = -I $(SOURCE_DIR)/makepo -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/makepo:$(SOURCE_DIR)/shared
bin = .

ifeq ($(shell echo "check_quotes"),"check_quotes")
  GIT_HASH := $(shell ..\..\..\Utilities\Scripts\githash)
  GIT_DATE := $(shell ..\..\..\Utilities\Scripts\gitlog)
else
  GIT_HASH := $(shell git describe --long --dirty)
  GIT_DATE := $(shell git log -1 --format=%cd)
endif
GITINFO=-Dpp_GITHASH=\"$(GIT_HASH)\" -Dpp_GITDATE=\""$(GIT_DATE)\""

# Definition of the object variables

obj = main.o file_util.o string_util.o dmalloc.o md5.o sha1.o sha256.o stdio_buffer.o threader.o
objwin = $(obj:.o=.obj)

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\pthreads.lib

SMV_CLANG_LIBDIR = $(LIB_DIR)/clang_win
SMV_LIBS_CLANG = $(SMV_CLANG_LIBDIR)\pthreads.lib


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

intel_win : CFLAGS    = -O2 /nologo -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads -D PTW32_STATIC_LIB  $(GITINFO) $(INTEL_COMPINFO)
intel_win : LFLAGS    =
intel_win : CC        = $(I_CC)
intel_win : exe       = makepo_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LFLAGS) $(SMV_LIBS_WIN)

# ------------- clang win 64 ----------------

clang_win : CFLAGS    = -m64 -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads
clang_win : LFLAGS    =
clang_win : CC        = clang-cl
clang_win : exe       = clang-cl_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LFLAGS) $(SMV_LIBS_CLANG)

# ------------- intel linux 64 ----------------

intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux : LFLAGS    = -lpthread
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = makepo_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gnu_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_linux : LFLAGS    = -lm
gnu_linux : CC        = gcc
gnu_linux : exe       = makepo_linux

gnu_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang linux 64 ----------------

clang_linux : CFLAGS    = -O2 -m64 $(GITINFO) -D PTW32_STATIC_LIB $(GNU_COMPINFO)
clang_linux : LFLAGS    = -lm -lpthread
clang_linux : CC        = clang
clang_linux : exe       = makepo_linux

clang_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gcc osx 64 ----------------

gnu_osx : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_osx : CC        = clang
gnu_osx : exe       = makepo_osx

gnu_osx : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- clang osx 64 ----------------

clang_osx : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
clang_osx : CC        = gcc
clang_osx : exe       = makepo_osx

clang_osx : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)


#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm *.o
