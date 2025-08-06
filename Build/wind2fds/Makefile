
# wind2fds makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = $(SOURCE_DIR)/../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/wind2fds
FILTERC =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/wind2fds $(SOURCE_DIR)/shared
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

csrc =    main.c dmalloc.c string_util.c file_util.c  md5.c sha1.c sha256.c stdio_buffer.c

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
main.obj: CFLAGS = -Od /nologo -D WIN32 -D pp_WIN_INTEL

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .cpp   .chk .winchk

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
	$(CLANGSUMMARY) wind2fds.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC_DIR += -I $(SOURCE_DIR)/pthreads
win_check : $(win_chk)
	$(CLANGWINSUMMARY) wind2fds.summary

# ------------- intel win 64 ----------------

intel_win_64 : CFLAGS    = -Od /nologo -I $(SOURCE_DIR)/pthreads -D WIN32 -D X64 $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = wind2fds_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D GLEW_STATIC -D MINGW
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = wind2fds_win_64.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX $(GITINFO) $(GNU_COMPINFO)
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = wind2fds_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 -D pp_GCC -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = wind2fds_linux_64

gnu_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -lm


# ------------- gnu linux 64 db ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64_db : CFLAGS    = -O0 -m64 -D pp_GCC -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = wind2fds_linux_64_db

gnu_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -lm

# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = wind2fds_osx_64

gnu_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -D pp_OSX -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : exe       = wind2fds_osx_64_db

gnu_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm *.o

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:
