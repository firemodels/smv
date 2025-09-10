
# pnginfo makefile

SOURCE_DIR = ../../../Source
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS

INC_DIR = -I $(SOURCE_DIR)/pthreads  -I $(SOURCE_DIR)/gd-2.0.15 -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/png-1.6.48 -I $(SOURCE_DIR)/shared  -I $(SOURCE_DIR)/pnginfo

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win_64
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\gd.lib $(SMV_WIN_LIBDIR)\jpeg.lib $(SMV_WIN_LIBDIR)\png.lib $(SMV_WIN_LIBDIR)\zlib.lib $(SMV_WIN_LIBDIR)\pthreads.lib

PNG_LIBS = -lgd -ljpeg -lpng -lz
SYSTEM_LIBS = -lpthread

FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/pnginfo ../../../../FDS/Source $(SOURCE_DIR)/shared
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

obj = dmalloc.o main.o md5.o sha1.o sha256.o readimage.o \
      file_util.o string_util.o stdio_buffer.o
objwin = $(obj:.o=.obj)

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

intel_win_64_db : CFLAGS    = /Od /Wall /debug:full /W4 /ZI -I $(SOURCE_DIR)/pthreads -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_win_64_db : CC        = $(I_CC)
intel_win_64_db : CPP       = $(I_CC)
intel_win_64_db : exe       = pnginfo_win_64_db.exe

intel_win_64_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin)

# ------------- intel win 64 ----------------

intel_win_64 : CFLAGS    = /O2 /nologo -D X64 -D WIN32 $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : CPP       = $(I_CC)
intel_win_64 : exe       = pnginfo_win_64.exe

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D GLEW_STATIC -D MINGW
gnu_win_64 : LFLAGS    = -m64
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = g++
gnu_win_64 : exe       = pnginfo_win_64.exe

gnu_win_64 : $(obj)
	$(FC) -o $(bin)/$(exe) $(LFLAGS) $(obj)

# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 -D pp_GCC -D pp_LINUX $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -lm
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = pnginfo_linux_64

gnu_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) 

# ------------- gnu linux 64 ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64_db : LFLAGS    = -m64 -L $(LIB_DIR)/gnu_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -lm
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = pnginfo_linux_64_db

gnu_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)


# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 -D pp_GCC -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64 : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 $(PNG_LIBS) $(SYSTEM_LIBS)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = pnginfo_osx_64

gnu_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -D pp_OSX -D pp_GCC -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_64_db : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 $(PNG_LIBS) $(SYSTEM_LIBS)
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : exe       = smokezip_osx_64_db

gnu_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

# ------------- intel linux 64 ----------------
intel_linux_64 : CFLAGS    = -O2 -m64 $(INC_DIR) -D pp_LINUX $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64 : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS)  -static-intel
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = pnginfo_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

intel_linux_64_db : CFLAGS    = -O2 -m64 $(INC_DIR) -Wall -Wextra -Wuninitialized -Wunused-function \
                                -Wunused-variable -Wno-sign-compare -Wno-unused-parameter -Wno-string-concatenation -D pp_LINUX \
                                $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64_db : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 $(PNG_LIBS) $(SYSTEM_LIBS) -static-intel
intel_linux_64_db : CC        = $(I_ICC)
intel_linux_64_db : exe       = pnginfo_linux_64_db

intel_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
