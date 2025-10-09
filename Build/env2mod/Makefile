
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

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win_64
SMV_WIN_LIBDIR_CLANG = $(LIB_DIR)/clang_win_64

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

intel_win_64 : CFLAGS    = -Od -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = env2mod_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_WIN_LIBDIR)\pthreads.lib

# ------------- clang win 64 ----------------

clang_win_64 : CFLAGS    = -m64 -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads
clang_win_64 : CC        = clang-cl
clang_win_64 : exe       = env2mod_win_64.exe

clang_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_WIN_LIBDIR_CLANG)\pthreads.lib

# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 -Wno-write-strings $(GNU_COMPINFO)
gnu_linux_64 : LFLAGS    = -m64
gnu_linux_64 : CC        = gcc
gnu_linux_64 : CPP       = g++
gnu_linux_64 : exe       = env2mod_linux_64

gnu_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)


# ------------- gnu linux 64 ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64_db : LFLAGS    = -m64
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : CPP       = g++
gnu_linux_64_db : exe       = env2mod_linux_64_db

gnu_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = env2mod_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 -Wno-write-strings $(GNU_COMPINFO)
gnu_osx_64 : LFLAGS    = -m64
gnu_osx_64 : CC        = gcc
gnu_osx_64 : CPP       = g++
gnu_osx_64 : exe       = env2mod_osx_64

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx_64 : CFLAGS    = -O0 -m64 -Wno-write-strings $(GNU_COMPINFO)
clang_osx_64 : LFLAGS    = -m64
clang_osx_64 : CC        = clang
clang_osx_64 : CPP       = clang
clang_osx_64 : exe       = env2mod_osx_64

clang_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

.PHONY : clean
clean:
	rm -f *.o
