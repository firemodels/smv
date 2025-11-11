
# sh2bat makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR = -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/sh2bat
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/sh2bat:$(SOURCE_DIR)/shared
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

obj = sh2bat.o string_util.o dmalloc.o file_util.o md5.o sha1.o sha256.o stdio_buffer.o threader.o
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

# ********  rules for each platform supported *************

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\pthreads.lib

SMV_CLANG_LIBDIR = $(LIB_DIR)/clang_win
SMV_LIBS_CLANG = $(SMV_CLANG_LIBDIR)\pthreads.lib

# ------------- intel win 64 ----------------

intel_win : CFLAGS    = -D PTW32_STATIC_LIB -I $(SOURCE_DIR)/pthreads -O2 /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win : CC        = $(I_CC)
intel_win : CPP       = $(I_CC)
intel_win : exe       = sh2bat_win.exe 

intel_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN)

# ------------- clang win 64 ----------------

clang_win : CFLAGS    = $(GITINFO) -I $(SOURCE_DIR)/pthreads -D PTW32_STATIC_LIB
clang_win : CC        = clang-cl
clang_win : CPP       = clang-cl
clang_win : exe       = sh2bat_win.exe

clang_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_CLANG)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	erase *.obj

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:
