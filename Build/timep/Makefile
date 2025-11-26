
# timep makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/shared
INC_DIR += -I $(SOURCE_DIR)/timep


WIN_LIBDIR = $(LIB_DIR)/intel_win

FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/timep:$(SOURCE_DIR)/shared
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

obj =    md5.o sha1.o sha256.o dmalloc.o file_util.o string_util.o stdio_buffer.o main.o threader.o
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
intel_win : exe       = timep_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- intel win ----------------

clang_win : CFLAGS    = -Od -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads /nologo $(GITINFO)
clang_win : CC        = clang-cl
clang_win : exe       = timep_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_CLANG)

# ------------- gnu linux db ----------------

gnu_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GNU_COMPINFO) $(GITINFO)
gnu_linux_db : LFLAGS    =
gnu_linux_db : CC        = gcc
gnu_linux_db : exe       = timep_linux_db

gnu_linux_db: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/gnu_linux $(LIBS_LINUX)  -lm

# ------------- gnu linux----------------

gnu_linux : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux
gnu_linux : CFLAGS    = -O2 -m64 -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_linux : LFLAGS    =
gnu_linux : CC        = gcc
gnu_linux : exe       = timep_linux

gnu_linux: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(LIBS_LINUX)  -lm

# ------------- clang linux db ----------------

clang_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D PTW32_STATIC_LIB -D _DEBUG $(GNU_COMPINFO) $(GITINFO)
clang_linux_db : LFLAGS    = -lpthread
clang_linux_db : CC        = clang
clang_linux_db : exe       = timep_linux_db

clang_linux_db: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/gnu_linux $(LIBS_LINUX)  -lm

# ------------- gnu linux----------------

clang_linux : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux
clang_linux : CFLAGS    = -O2 -m64 -Wall -D PTW32_STATIC_LIB $(GITINFO) $(GNU_COMPINFO)
clang_linux : LFLAGS    = -lpthread
clang_linux : CC        = clang
clang_linux : exe       = timep_linux

clang_linux: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(LIBS_LINUX)  -lm

# ------------- intel linux ----------------

intel_linux : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux
intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = timep_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -L$(LIB_DIR_PLAT) $(LIBS_LINUX) $(LFLAGS)

# ------------- gnu osx ----------------

gnu_osx : CFLAGS    = -O0 -m64 $(GITINFO)
gnu_osx : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_osx $(LIBS_OSX)
gnu_osx : CC        = gcc
gnu_osx : CPP       = g++
gnu_osx : exe       = timep_osx

gnu_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx db ----------------

gnu_osx_db : CFLAGS    = -O0 -m64 -Wall $(GITINFO)
gnu_osx_db : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_osx $(LIBS_OSX)
gnu_osx_db : CC        = gcc
gnu_osx_db : CPP       = g++
gnu_osx_db : exe       = timep_osx_db

gnu_osx_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx ----------------

clang_osx : CFLAGS    = -O0 -m64 $(GITINFO)
clang_osx : LFLAGS    = -m64 -L $(LIB_DIR)/clang_osx $(LIBS_OSX)
clang_osx : CC        = clang
clang_osx : CPP       = clang++
clang_osx : exe       = timep_osx

clang_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx db ----------------

clang_osx_db : CFLAGS    = -O0 -m64 -Wall $(GITINFO)
clang_osx_db : LFLAGS    = -m64 -L $(LIB_DIR)/clang_osx $(LIBS_OSX)
clang_osx_db : CC        = clang
clang_osx_db : CPP       = clang++
clang_osx_db : exe       = timep_osx_db

clang_osx_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)
