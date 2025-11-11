
# convert makefile

SOURCE_DIR = ../../../Source
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR = -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/convert
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/convert ../../../../FDS/Source $(SOURCE_DIR)/shared $(SOURCE_DIR)/smokeview
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

obj = dmalloc.o main.o md5.o sha1.o sha256.o\
      file_util.o string_util.o stdio_buffer.o threader.o
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

# ------------- intel winx 64 db ----------------

intel_win_db : CFLAGS    = /Od /Wall /debug:full /W4 /ZI -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_win_db : CC        = $(I_CC)
intel_win_db : CPP       = $(I_CC)
intel_win_db : exe       = convert_win_db.exe

intel_win_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- intel win 64 ----------------

intel_win : CFLAGS    = /O2 /nologo -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads $(GITINFO) $(INTEL_COMPINFO)
intel_win : CC        = $(I_CC)
intel_win : CPP       = $(I_CC)
intel_win : exe       = convert_win.exe

intel_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- clang win 64 ----------------

clang_win : CFLAGS    = -m64 -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads
clang_win : LFLAGS    = -m64
clang_win : CC        = clang-cl
clang_win : CPP       = clang-cl
clang_win : exe       = convert_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(SMV_LIBS_CLANG)

# ------------- gnu linux 64 ----------------

gnu_linux : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_linux : LFLAGS    = -m64
gnu_linux : CC        = gcc
gnu_linux : CPP       = g++
gnu_linux : exe       = convert_linux

gnu_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu linux 64 db ----------------

gnu_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
gnu_linux_db : LFLAGS    = -m64
gnu_linux_db : CC        = gcc
gnu_linux_db : CPP       = g++
gnu_linux_db : exe       = convert_linux_db

gnu_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)


# ------------- clang linux 64 ----------------

clang_linux : CFLAGS    = -O0 -m64 -D PTW32_STATIC_LIB $(GITINFO) $(GNU_COMPINFO)
clang_linux : LFLAGS    = -m64 -lpthread
clang_linux : CC        = clang
clang_linux : CPP       = clang++
clang_linux : exe       = convert_linux

clang_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang linux 64 db ----------------

clang_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D PTW32_STATIC_LIB -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
clang_linux_db : LFLAGS    = -m64 -lpthread
clang_linux_db : CC        = clang
clang_linux_db : CPP       = clang++
clang_linux_db : exe       = convert_linux_db

clang_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 ----------------

gnu_osx : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_osx : LFLAGS    = -m64
gnu_osx : CC        = gcc
gnu_osx : CPP       = g++
gnu_osx : exe       = convert_osx

gnu_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_db : LFLAGS    = -m64
gnu_osx_db : CC        = gcc
gnu_osx_db : CPP       = g++
gnu_osx_db : exe       = convert_osx_db

gnu_osx_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
clang_osx : LFLAGS    = -m64
clang_osx : CC        = clang
clang_osx : CPP       = clang
clang_osx : exe       = convert_osx

clang_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 db ----------------

clang_osx_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
clang_osx_db : LFLAGS    = -m64
clang_osx_db : CC        = clang
clang_osx_db : CPP       = clang
clang_osx_db : exe       = convert_osx_db

clang_osx_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = convert_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) -static-intel $(obj)

# ------------- intel linux 64 db ----------------

intel_linux_db : CFLAGS    = -O0 -m64 -g -check=stack,uninit -fp-stack-check -fp-trap-all=divzero,invalid,overflow -ftrapuv -Wuninitialized -Wunused-function -Wunused-variable $(GITINFO) $(INTEL_COMPINFO)
intel_linux_db : CC        = $(I_ICC)
intel_linux_db : exe       = convert_linux_db

intel_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) -static-intel $(obj)

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:
