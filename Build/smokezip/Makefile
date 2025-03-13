
# smokezip makefile

SOURCE_DIR = ../../../Source/
LIB_DIR = ../../LIBS
INC_DIR =  -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokezip
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

obj =    CNV3dsmoke.o CNVboundary.o CNVpart.o CNVslice.o  \
         compress.o dmalloc.o file_util.o getdata.o \
         isobox.o main.o md5.o readfiles.o sha1.o sha256.o stdio_buffer.o \
         string_util.o threaderzip.o threader.o utilities.o

objwin = $(obj:.o=.obj)

# main has to be compiled without optimization (on a Windows PC) otherwise errors occcur
# don't think the following is needed anymore - keeping it around for awhile to make it easier to put
# back in case it is needed
#main.obj: CFLAGS = -Od /nologo -D WIN32 -D pp_WIN_INTEL $(OPT)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .cpp

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<

# ------------- intel win 64 ----------------

intel_win_64 : INC_DIR   += -I $(SOURCE_DIR)/pthreads
intel_win_64 : CFLAGS    = -Od /nologo -D WIN32 -D PTW32_STATIC_LIB -D X64  $(OPT) $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = smokezip_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LIB_DIR)/intel_win_64/pthreads.lib $(LIB_DIR)/intel_win_64/zlib.lib

# ------------- gnu win 64 ----------------

gnu_win_64 : INC_DIR   += -I $(SOURCE_DIR)/pthreads
gnu_win_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D GLEW_STATIC -D MINGW $(SMV_TESTFLAG)
gnu_win_64 : LFLAGS    = -m64
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = smokezip_win_64.exe

gnu_win_64 : $(obj)
	$(FC) -o $(bin)/$(exe) $(obj) $(LIB_DIR)/gnu_win_64/pthreads.a $(LIB_DIR)/gnu_win_64/zlib.a

# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 -Wall -D pp_GCC -D pp_LINUX $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux_64 : LFLAGS    = -m64 -lm
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = smokezip_linux_64

gnu_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/gnu_linux_64 -lz -lpthread

# ------------- gnu linux 64 ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_linux_64_db : LFLAGS    = -m64 -lm
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = smokezip_linux_64_db

gnu_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR)/gnu_linux_64 -lz -lpthread


# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 -D pp_GCC -D pp_OSX $(GITINFO) $(GNU_COMPINFO) $(SMV_TESTFLAG)
gnu_osx_64 : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 -lz -lpthread
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = smokezip_osx_64

gnu_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D _DEBUG -D pp_OSX -D pp_GCC -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_TESTFLAG)
gnu_osx_64_db : LFLAGS    = -L $(LIB_DIR)/gnu_osx_64 -lz -lpthread
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : exe       = smokezip_osx_64_db

gnu_osx_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

intel_linux_64_db : CFLAGS    = -O2 -m64 ${INC_DIR} -Wall -Wextra -Wuninitialized -Wunused-function \
                                -Wunused-variable -Wno-sign-compare -Wno-unused-parameter -Wno-string-concatenation -D pp_LINUX \
                                $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
intel_linux_64_db : LFLAGS    = -L $(LIB_DIR)/intel_linux_64 -lz -lpthread -static-intel
intel_linux_64_db : CC        = $(I_ICC)
intel_linux_64_db : exe       = smokezip_linux_64_db

intel_linux_64_db : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------
intel_linux_64 : CFLAGS    = -O2 -m64 ${INC_DIR} -D pp_LINUX $(GITINFO) $(INTEL_COMPINFO) $(SMV_TESTFLAG)
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
