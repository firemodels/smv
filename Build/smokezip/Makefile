# smokezip makefile

SOURCE_DIR = ../../../Source/
LIB_DIR = ../../LIBS
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

linux_osx_check : CFLAGS    = $(SMV_TESTFLAG)
linux_osx_check : $(linux_osx_chk)
	$(CLANGSUMMARY) smokezip.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC_DIR += -I $(SOURCE_DIR)/pthreads
win_check : $(win_chk)
	$(CLANGWINSUMMARY) smokezip.summary

# ------------- intel win 64 ----------------

intel_win_64 : INC_DIR   += -I $(SOURCE_DIR)/pthreads
intel_win_64 : CFLAGS    = -Od /nologo -D PTW32_STATIC_LIB  $(OPT) $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = smokezip_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LIB_DIR)/intel_win_64/pthreads.lib $(LIB_DIR)/intel_win_64/zlib.lib

# ------------- gnu win 64 ----------------

gnu_win_64 : INC_DIR   += -I $(SOURCE_DIR)/pthreads
gnu_win_64 : CFLAGS    = -O0 -m64 -D GLEW_STATIC -D MINGW $(SMV_TESTFLAG)
gnu_win_64 : LFLAGS    = -m64
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = smokezip_win_64.exe

gnu_win_64 : $(obj)
	$(FC) -o $(bin)/$(exe) $(obj) $(LIB_DIR)/gnu_win_64/pthreads.a $(LIB_DIR)/gnu_win_64/zlib.a

# ------------- clang win 64 ----------------

clang_win_64 : INC_DIR   += -I $(SOURCE_DIR)/pthreads
clang_win_64 : CFLAGS    = -m64  -D PTW32_STATIC_LIB -D BGDWIN32 -D GLEW_STATIC $(SMV_TESTFLAG)
clang_win_64 : LFLAGS    = -m64
clang_win_64 : CC        = clang-cl
clang_win_64 : exe       = smokezip_win_64.exe

clang_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LIB_DIR)\clang_win_64\pthreads.lib $(LIB_DIR)\clang_win_64\zlib.lib $(LIB_DIR)\clang_win_64\pthreads.lib


# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 -Wall $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux_64 : LFLAGS    = -m64 -lm
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = smokezip_linux_64

gnu_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/gnu_linux_64 -lz -lpthread

# ------------- gnu linux 64 ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux_64_db : LFLAGS    = -m64 -lm
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = smokezip_linux_64_db

gnu_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/gnu_linux_64 -lz -lpthread

# ------------- clang linux 64 ----------------

clang_linux_64 : CFLAGS    = -O0 -m64 -Wall $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_linux_64 : LFLAGS    = -m64 -lm
clang_linux_64 : CC        = clang
clang_linux_64 : exe       = smokezip_linux_64

clang_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/clang_linux_64 -lz -lpthread

# ------------- gnu linux 64 ----------------

clang_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_linux_64_db : LFLAGS    = -m64 -lm
clang_linux_64_db : CC        = clang
clang_linux_64_db : exe       = smokezip_linux_64_db

clang_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/clang_linux_64 -lz -lpthread


# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_osx_64 : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 -lz -lpthread
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = smokezip_osx_64

gnu_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
gnu_osx_64_db : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 -lz -lpthread
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : exe       = smokezip_osx_64_db

gnu_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx_64 : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
clang_osx_64 : LFLAGS    = -L $(LIB_DIR)/clang_osx_64 -lz -lpthread
clang_osx_64 : CC        = clang
clang_osx_64 : exe       = smokezip_osx_64

clang_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 db ----------------

clang_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
clang_osx_64_db : LFLAGS    = -L $(LIB_DIR)/clang_osx_64 -lz -lpthread
clang_osx_64_db : CC        = clang
clang_osx_64_db : exe       = smokezip_osx_64_db

clang_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------
# ------------- intel linux 64 ----------------

intel_linux_64_db : CFLAGS    = -O2 -m64 ${INC_DIR} -Wall -Wextra -Wuninitialized -Wunused-function \
                                -Wunused-variable -Wno-sign-compare -Wno-unused-parameter -Wno-string-concatenation \
                                $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64_db : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 -lz -lpthread -static-intel
intel_linux_64_db : CC        = $(I_ICC)
intel_linux_64_db : exe       = smokezip_linux_64_db

intel_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------
intel_linux_64 : CFLAGS    = -O2 -m64 ${INC_DIR} $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64 : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 -lz -lpthread -static-intel
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = smokezip_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
