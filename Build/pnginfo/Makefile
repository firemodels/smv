
# pnginfo makefile

SOURCE_DIR = ../../../Source
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS

INC_DIR = -I $(SOURCE_DIR)/pthreads  -I $(SOURCE_DIR)/gd-2.0.15 -I $(SOURCE_DIR)/zlib128 \
          -I $(SOURCE_DIR)/png-1.6.48 -I $(SOURCE_DIR)/shared  -I $(SOURCE_DIR)/pnginfo

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win_64
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\gd.lib $(SMV_WIN_LIBDIR)\jpeg.lib $(SMV_WIN_LIBDIR)\png.lib $(SMV_WIN_LIBDIR)\zlib.lib $(SMV_WIN_LIBDIR)\pthreads.lib

PNG_LIBS = -lgd -ljpeg -lpng -lz
SYSTEM_LIBS = -lpthread

FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/pnginfo $(SOURCE_DIR)/shared
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

csrc = dmalloc.c main.c md5.c sha1.c sha256.c readimage.c \
      file_util.c string_util.c stdio_buffer.c

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

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .chk .winchk

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
	$(CLANGSUMMARY) pnginfo.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC_DIR += -I $(SOURCE_DIR)/pthreads
win_check : $(win_chk)
	$(CLANGWINSUMMARY) pnginfo.summary


# ------------- intel winx 64 db ----------------

intel_win_64_db : CFLAGS    = /Od /Wall /debug:full /W4 /ZI -I $(SOURCE_DIR)/pthreads -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_win_64_db : CC        = $(I_CC)
intel_win_64_db : exe       = pnginfo_win_64_db.exe

intel_win_64_db : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- intel win 64 ----------------

intel_win_64 : CFLAGS    = /O2 /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = pnginfo_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D __MINGW32__
gnu_win_64 : LFLAGS    = -m64
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = pnginfo_win_64.exe

gnu_win_64 : $(obj)
	$(FC) -o $(bin)/$(exe) $(LFLAGS) $(obj)

# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -lm
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = pnginfo_linux_64

gnu_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu linux 64 ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64_db : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -lm
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = pnginfo_linux_64_db

gnu_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang linux 64 ----------------

clang_linux_64 : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
clang_linux_64 : LFLAGS    = -m64 -L $(LIB_DIR)/clang_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -lm
clang_linux_64 : CC        = clang
clang_linux_64 : exe       = pnginfo_linux_64

clang_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu linux 64 ----------------

clang_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(GITINFO) $(GNU_COMPINFO)
clang_linux_64_db : LFLAGS    = -m64 -L $(LIB_DIR)/clang_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -lm
clang_linux_64_db : CC        = clang
clang_linux_64_db : exe       = pnginfo_linux_64_db

clang_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)


# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64 : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 $(PNG_LIBS) $(SYSTEM_LIBS)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = pnginfo_osx_64

gnu_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_64_db : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 $(PNG_LIBS) $(SYSTEM_LIBS)
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : exe       = pnginfo_osx_64_db

gnu_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 db ----------------

clang_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
clang_osx_64_db : LFLAGS    = -L $(LIB_DIR)/clang_osx_64 $(PNG_LIBS) $(SYSTEM_LIBS)
clang_osx_64_db : CC        = clang
clang_osx_64_db : exe       = pnginfo_osx_64_db

clang_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang osx 64 ----------------

clang_osx_64 : CFLAGS    = -O0 -m64 $(GITINFO) $(GNU_COMPINFO)
clang_osx_64 : LFLAGS    = -L $(LIB_DIR)/clang_osx_64 $(PNG_LIBS) $(SYSTEM_LIBS)
clang_osx_64 : CC        = clang
clang_osx_64 : exe       = pnginfo_osx_64

clang_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

# ------------- intel linux 64 ----------------
intel_linux_64 : CFLAGS    = -O2 -m64 $(INC_DIR) $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64 : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS)  -static-intel
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = pnginfo_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

intel_linux_64_db : CFLAGS    = -O2 -m64 $(INC_DIR) -Wall -Wextra -Wuninitialized -Wunused-function \
                                -Wunused-variable -Wno-sign-compare -Wno-unused-parameter -Wno-string-concatenation \
                                $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64_db : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -static-intel
intel_linux_64_db : CC        = $(I_ICC)
intel_linux_64_db : exe       = pnginfo_linux_64_db

intel_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm -f *.o *.mod *.dll *.so

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
