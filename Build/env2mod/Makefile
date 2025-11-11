
# env2mod makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/env2mod

FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/env2mod $(SOURCE_DIR)/shared
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

obj = file_util.o md5.o sha1.o sha256.o dmalloc.o string_util.o stdio_buffer.o env2mod.o main.o threader.o
objwin = $(obj:.o=.obj)

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\pthreads.lib

SMV_CLANG_LIBDIR = $(LIB_DIR)/clang_win
SMV_LIBS_CLANG = $(SMV_CLANG_LIBDIR)\pthreads.lib


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

intel_win : CFLAGS    = -Od -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win : CC        = $(I_CC)
intel_win : exe       = env2mod_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- clang win ----------------

clang_win : CFLAGS    = -m64 -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads
clang_win : CC        = clang-cl
clang_win : exe       = env2mod_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_CLANG)

# ------------- gnu linux ----------------

gnu_linux : CFLAGS    = -O0 -m64 -Wno-write-strings $(GNU_COMPINFO)
gnu_linux : LFLAGS    = -m64
gnu_linux : CC        = gcc
gnu_linux : CPP       = g++
gnu_linux : exe       = env2mod_linux

gnu_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)


# ------------- gnu linux ----------------

gnu_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
gnu_linux_db : LFLAGS    = -m64
gnu_linux_db : CC        = gcc
gnu_linux_db : CPP       = g++
gnu_linux_db : exe       = env2mod_linux_db

gnu_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang linux ----------------

clang_linux : CFLAGS    = -O0 -m64 -Wno-write-strings -D PTW32_STATIC_LIB $(GNU_COMPINFO)
clang_linux : LFLAGS    = -m64 -lpthread
clang_linux : CC        = clang
clang_linux : CPP       = clang++
clang_linux : exe       = env2mod_linux

clang_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)


# ------------- clang linux ----------------

clang_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D PTW32_STATIC_LIB -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
clang_linux_db : LFLAGS    = -m64 -lpthread
clang_linux_db : CC        = clang
clang_linux_db : CPP       = clang++
clang_linux_db : exe       = env2mod_linux_db

clang_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux ----------------

intel_linux : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux
intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = env2mod_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx ----------------

gnu_osx : CFLAGS    = -O0 -m64 -Wno-write-strings $(GNU_COMPINFO)
gnu_osx : LFLAGS    = -m64
gnu_osx : CC        = gcc
gnu_osx : CPP       = g++
gnu_osx : exe       = env2mod_osx

gnu_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx ----------------

clang_osx : CFLAGS    = -O0 -m64 -Wno-write-strings $(GNU_COMPINFO)
clang_osx : LFLAGS    = -m64
clang_osx : CC        = clang
clang_osx : CPP       = clang
clang_osx : exe       = env2mod_osx

clang_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

.PHONY : clean
clean:
	rm -f *.o
