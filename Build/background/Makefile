
# background makefile

SOURCE_DIR = ../../../Source
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR = -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/background
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/background $(SOURCE_DIR)/shared
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

csrc = main.c string_util.c dmalloc.c file_util.c md5.c sha1.c sha256.c stdio_buffer.c threader.c
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

.SUFFIXES: .c .obj .o .cpp .chk .winchk

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
	$(CLANGSUMMARY) background.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC_DIR += -I $(SOURCE_DIR)/pthreads
win_check : $(win_chk)
	$(CLANGWINSUMMARY) background.summary

# ********  rules for each platform supported *************

# ------------- intel win ----------------

intel_win : CFLAGS    = -O1 /nologo -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads -I $(SOURCE_DIR)/shared $(GITINFO) $(INTEL_COMPINFO)
intel_win : CC        = $(I_CC)
intel_win : exe       = background_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- clang win ----------------

clang_win : CFLAGS    = -m64 -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads
clang_win : CC        = clang-cl
clang_win : exe       = background_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_CLANG)

# ------------- intel linux ----------------

intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = background_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -lpthread

# ------------- gnu linux ----------------

gnu_linux : CFLAGS    = -O2 -m64 -Wall $(GITINFO) $(GNU_COMPINFO) $(INC_DIR)
gnu_linux : CC        = gcc
gnu_linux : exe       = background_linux

gnu_linux: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm

# ------------- gnu linux db ----------------

gnu_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
gnu_linux_db : CC        = gcc
gnu_linux_db : exe       = background_linux

gnu_linux_db: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm

# ------------- clang linux ----------------

clang_linux : CFLAGS    = -O2 -m64 -Wall -D PTW32_STATIC_LIB $(GITINFO) $(GNU_COMPINFO) $(INC_DIR)
clang_linux : CC        = clang
clang_linux : exe       = background_linux

clang_linux: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm -lpthread

# ------------- gnu linux db ----------------

clang_linux_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D PTW32_STATIC_LIB -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
clang_linux_db : CC        = clang
clang_linux_db : exe       = background_linux

clang_linux_db: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm -lpthread

# ------------- gcc osx ----------------
gnu_osx : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_osx : CC        = gcc
gnu_osx : exe       = background_osx

gnu_osx : $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)  -lm

# ------------- gnu OSX debug ----------------

gnu_osx_db : CFLAGS    = -O0 -m64 -Wall -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_db : CC        = gcc
gnu_osx_db : exe       = background_osx_db

gnu_osx_db : $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)  -lm

# ------------- clang osx ----------------
clang_osx : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
clang_osx : CC        = clang
clang_osx : exe       = background_osx

clang_osx : $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)  -lm

# ------------- clang OSX debug ----------------

clang_osx_db : CFLAGS    = -O0 -m64 -Wall -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
clang_osx_db : CC        = clang
clang_osx_db : exe       = background_osx_db

clang_osx_db : $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)  -lm

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:
