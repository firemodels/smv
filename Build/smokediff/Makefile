
# smokediff makefile

SOURCE_DIR = ../../../Source
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR = -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokediff
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

SMV_TESTFLAG=

VPATH = $(SOURCE_DIR)/smokediff $(SOURCE_DIR)/shared
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

csrc = color2rgb.c dmalloc.c file_util.c getdata.c histogram.c IOdboundary.c IOdplot.c \
      IOdslice.c main.c md5.c readsmv.c sha1.c sha256.c stdio_buffer.c string_util.c utilities.c threader.c

obj = $(csrc:.c=.o)
objwin = $(obj:.o=.obj)

# setup for analyzing source code using clang

linux_osx_c_chk    = $(csrc:.c=.chk)
linux_osx_chk      = $(linux_osx_c_chk)

win_c_chk   = $(csrc:.c=.winchk)
win_chk     = $(win_c_chk)

#CLANGANALYZE=clang --analyze -Xanalyzer -analyzer-checker=core,deadcode
CLANGANALYZE=clang --analyze
CLANGSUMMARY=../../scripts/clang_summary.sh
CLANGWINSUMMARY=..\..\scripts\clang_summary.bat

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\pthreads.lib

SMV_CLANG_LIBDIR = $(LIB_DIR)/clang_win
SMV_LIBS_CLANG = $(SMV_CLANG_LIBDIR)\pthreads.lib


#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: c .obj .o .cpp  .chk .winchk

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<

# ------------- linux and osx clang checks ----------------

.c.chk:
	$(CLANGANALYZE) $(CFLAGS) $(INC_DIR) $< 2> $@

# ------------- windows clang checks ----------------

.c.winchk:
	$(CLANGANALYZE) $(CFLAGS) $(INC_DIR) $< 2> $@

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   check source   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# ------------- linux_osx_check ----------------

linux_osx_check : CFLAGS    = $(SMV_TESTFLAG)
linux_osx_check : $(linux_osx_chk)
	$(CLANGSUMMARY) smokediff.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC_DIR += -I $(SOURCE_DIR)/pthreads
win_check : $(win_chk)
	$(CLANGWINSUMMARY) smokediff.summary

# ------------- intel winx 64 db ----------------

intel_win_db : INC_DIR   += -I $(SOURCE_DIR)/pthreads
intel_win_db : FFLAGS    = /Od /iface:stdref /fpp /nologo /debug:full /extend_source:132 /warn:unused /warn:nointerfaces /Qtrapuv /fp:strict /fp:except /traceback /check:all /stand:f18 /fpscomp:general
intel_win_db : CFLAGS    = /Od /Wall /debug:full /W4 /ZI -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_win_db : CC        = $(I_CC)
intel_win_db : exe       = smokediff_win_db.exe

intel_win_db : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- intel win 64 ----------------

intel_win : INC_DIR   += -I $(SOURCE_DIR)/pthreads
intel_win : FFLAGS    = /O2 -fpp /nologo /iface:stdref /stand:f18 /fpscomp:general
intel_win : CFLAGS    = /O2 /nologo -D PTW32_STATIC_LIB $(GITINFO) $(SMV_TESTFLAG) $(INTEL_COMPINFO)
intel_win : CC        = $(I_CC)
intel_win : exe       = smokediff_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- clang win 64 ----------------

clang_win : INC_DIR   += -I $(SOURCE_DIR)/pthreads
clang_win : CFLAGS    = -m64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG)
clang_win : LFLAGS    = -m64
clang_win : CC        = clang-cl
clang_win : exe       = smokediff_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(SMV_LIBS_CLANG)

# ------------- gnu linux 64 ----------------

gnu_linux : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux : LFLAGS    = -m64 -lm
gnu_linux : CC        = gcc
gnu_linux : exe       = smokediff_linux

gnu_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu linux 64 db ----------------

gnu_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux_db : LFLAGS    = -m64 -lm
gnu_linux_db : CC        = gcc
gnu_linux_db : exe       = smokediff_linux_db

gnu_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang linux 64 ----------------

clang_linux : CFLAGS    = -O0 -m64 -D PTW32_STATIC_LIB $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_linux : LFLAGS    = -m64 -lm -lpthread
clang_linux : CC        = clang
clang_linux : exe       = smokediff_linux

clang_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu linux 64 db ----------------

clang_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D PTW32_STATIC_LIB -D _DEBUG $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_linux_db : LFLAGS    = -m64 -lm -lpthread
clang_linux_db : CC        = clang
clang_linux_db : exe       = smokediff_linux_db

clang_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)



# ------------- gnu osx 64 ----------------

gnu_osx : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG) -Wno-unknown-pragmas
gnu_osx : LFLAGS    = -m64
gnu_osx : CC        = gcc
gnu_osx : exe       = smokediff_osx

gnu_osx : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
gnu_osx_db : LFLAGS    = -m64
gnu_osx_db : CC        = gcc
gnu_osx_db : exe       = smokediff_osx_db

gnu_osx_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG) -Wno-unknown-pragmas
clang_osx : LFLAGS    = -m64
clang_osx : CC        = clang
clang_osx : exe       = smokediff_osx

clang_osx : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

clang_osx_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
clang_osx_db : LFLAGS    = -m64
clang_osx_db : CC        = clang
clang_osx_db : exe       = smokediff_osx_db

clang_osx_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = smokediff_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) -static-intel $(obj)

# ------------- intel linux 64 db ----------------

intel_linux_db : CFLAGS    = -O0 -m64 -g -check=stack,uninit -fp-stack-check -fp-trap-all=divzero,invalid,overflow -ftrapuv -Wuninitialized -Wunused-function -Wunused-variable $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_db : CC        = $(I_ICC)
intel_linux_db : exe       = smokediff_linux_db

intel_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) -static-intel $(obj)

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

#-------------- force compilation of string_util.c -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
