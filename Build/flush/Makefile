
# flush makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/shared
INC_DIR += -I $(SOURCE_DIR)/flush


WIN_LIBDIR = $(LIB_DIR)/intel_win_64

FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/flush:$(SOURCE_DIR)/shared
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

obj =    md5.o sha1.o sha256.o dmalloc.o file_util.o string_util.o stdio_buffer.o main.o
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

# ********  rules for each platform supported *************

# ------------- intel win 64 ----------------

intel_win_64 : CFLAGS    = -Od -I $(SOURCE_DIR)/pthreads /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = flush_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LIBS_WIN)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D GLEW_STATIC -D MINGW
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = flush_win_64.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LIBS_WIN)

# ------------- gnu linux 64 db ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GNU_COMPINFO) $(GITINFO)
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = flush_linux_64_db

gnu_linux_64_db: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/gnu_linux_64 $(LIBS_LINUX)  -lm

# ------------- gnu linux 64  ----------------

gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : CFLAGS    = -O2 -m64 -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = flush_linux_64

gnu_linux_64: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(LIBS_LINUX)  -lm

# ------------- intel linux 64 ----------------

intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = flush_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -L$(LIB_DIR_PLAT) $(LIBS_LINUX) $(LFLAGS)

# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 $(GITINFO)
gnu_osx_64 : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_osx_64 $(LIBS_OSX)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : CPP       = g++
gnu_osx_64 : exe       = flush_osx_64

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -Wall $(GITINFO)
gnu_osx_64_db : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_osx_64 $(LIBS_OSX)
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : CPP       = g++
gnu_osx_64_db : exe       = flush_osx_64_db

gnu_osx_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx_64 : CFLAGS    = -O0 -m64 $(GITINFO)
clang_osx_64 : LFLAGS    = -m64 -L $(LIB_DIR)/clang_osx_64 $(LIBS_OSX)
clang_osx_64 : CC        = clang
clang_osx_64 : CPP       = clang
clang_osx_64 : exe       = flush_osx_64

clang_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

clang_osx_64_db : CFLAGS    = -O0 -m64 -Wall $(GITINFO)
clang_osx_64_db : LFLAGS    = -m64 -L $(LIB_DIR)/clang_osx_64 $(LIBS_OSX)
clang_osx_64_db : CC        = clang
clang_osx_64_db : CPP       = clang
clang_osx_64_db : exe       = flush_osx_64_db

clang_osx_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)
