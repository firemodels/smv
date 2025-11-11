# smokezip makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/zlib131 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokezip
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

OPT =

SMV_TESTFLAG=

VPATH = $(SOURCE_DIR)/smokezip $(SOURCE_DIR)/shared
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

csrc =    CNV3dsmoke.c CNVboundary.c CNVpart.c CNVslice.c  \
         compress.c dmalloc.c file_util.c getdata.c \
         isobox.c main.c md5.c readfiles.c sha1.c sha256.c stdio_buffer.c \
         string_util.c threaderzip.c threader.c utilities.c

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

# main has to be compiled without optimization (on a Windows PC) otherwise errors occcur
# don't think the following is needed anymore - keeping it around for awhile to make it easier to put
# back in case it is needed
#main.obj: CFLAGS = -Od /nologo -D pp_WIN_INTEL $(OPT)

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\pthreads.lib

SMV_CLANG_LIBDIR = $(LIB_DIR)/clang_win
SMV_LIBS_CLANG = $(SMV_CLANG_LIBDIR)\pthreads.lib $(SMV_CLANG_LIBDIR)\pthreads.lib $(SMV_CLANG_LIBDIR)\zlib.lib


#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .cpp  .chk .winchk

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

linux_osx_check : CFLAGS    =  $(SMV_TESTFLAG)
linux_osx_check : $(linux_osx_chk)
	$(CLANGSUMMARY) smokezip.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC_DIR += -I $(SOURCE_DIR)/pthreads
win_check : $(win_chk)
	$(CLANGWINSUMMARY) smokezip.summary

# ------------- intel win 64 ----------------

intel_win : INC_DIR   += -I $(SOURCE_DIR)/pthreads
intel_win : CFLAGS    = -Od /nologo -D PTW32_STATIC_LIB  $(OPT) $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_win : CC        = $(I_CC)
intel_win : exe       = smokezip_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LIB_DIR)/intel_win/pthreads.lib $(LIB_DIR)/intel_win/zlib.lib

# ------------- clang win 64 ----------------

clang_win : INC_DIR   += -I $(SOURCE_DIR)/pthreads
clang_win : CFLAGS    = -m64  -D PTW32_STATIC_LIB -D BGDWIN32 -D GLEW_STATIC $(SMV_TESTFLAG)
clang_win : LFLAGS    = -m64
clang_win : CC        = clang-cl
clang_win : exe       = smokezip_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_CLANG)


# ------------- gnu linux 64 ----------------

gnu_linux : CFLAGS    = -O0 -m64 -Wall $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux : LFLAGS    = -m64 -lm
gnu_linux : CC        = gcc
gnu_linux : exe       = smokezip_linux

gnu_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/gnu_linux -lz -lpthread

# ------------- gnu linux 64 ----------------

gnu_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux_db : LFLAGS    = -m64 -lm
gnu_linux_db : CC        = gcc
gnu_linux_db : exe       = smokezip_linux_db

gnu_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/gnu_linux -lz -lpthread

# ------------- clang linux 64 ----------------

clang_linux : CFLAGS    = -O0 -m64 -Wall $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_linux : LFLAGS    = -m64 -lm
clang_linux : CC        = clang
clang_linux : exe       = smokezip_linux

clang_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/clang_linux -lz -lpthread

# ------------- gnu linux 64 ----------------

clang_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_linux_db : LFLAGS    = -m64 -lm
clang_linux_db : CC        = clang
clang_linux_db : exe       = smokezip_linux_db

clang_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/clang_linux -lz -lpthread


# ------------- gnu osx 64 ----------------

gnu_osx : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_osx : LFLAGS    = -L $(LIB_DIR)/gnu_osx -lz -lpthread
gnu_osx : CC        = gcc
gnu_osx : exe       = smokezip_osx

gnu_osx : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
gnu_osx_db : LFLAGS    = -L $(LIB_DIR)/gnu_osx -lz -lpthread
gnu_osx_db : CC        = gcc
gnu_osx_db : exe       = smokezip_osx_db

gnu_osx_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_osx : LFLAGS    = -L $(LIB_DIR)/clang_osx -lz -lpthread
clang_osx : CC        = clang
clang_osx : exe       = smokezip_osx

clang_osx : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 db ----------------

clang_osx_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
clang_osx_db : LFLAGS    = -L $(LIB_DIR)/clang_osx -lz -lpthread
clang_osx_db : CC        = clang
clang_osx_db : exe       = smokezip_osx_db

clang_osx_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------
# ------------- intel linux 64 ----------------

intel_linux_db : CFLAGS    = -O2 -m64 ${INC_DIR} -Wall -Wextra -Wuninitialized -Wunused-function \
                                -Wunused-variable -Wno-sign-compare -Wno-unused-parameter -Wno-string-concatenation \
                                $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_db : LFLAGS    = -L $(LIB_DIR)/intel_linux -lz -lpthread -static-intel
intel_linux_db : CC        = $(I_ICC)
intel_linux_db : exe       = smokezip_linux_db

intel_linux_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------
intel_linux : CFLAGS    = -O2 -m64 ${INC_DIR} $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux : LFLAGS    = -L $(LIB_DIR)/intel_linux -lz -lpthread -static-intel
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = smokezip_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
