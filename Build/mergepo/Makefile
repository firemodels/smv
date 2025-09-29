SOURCE_DIR = ../../../Source/
INC_DIR = -I $(SOURCE_DIR)/mergepo -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/mergepo:$(SOURCE_DIR)/shared
bin = .

ifeq ($(shell echo "check_quotes"),"check_quotes")
  GIT_HASH := $(shell ..\..\..\Utilities\Scripts\githash)
  GIT_DATE := $(shell ..\..\..\Utilities\Scripts\gitlog)
else
  GIT_HASH := $(shell git describe --long --dirty)
  GIT_DATE := $(shell git log -1 --format=%cd)
endif
GITINFO=-Dpp_GITHASH=\"$(GIT_HASH)\" -Dpp_GITDATE=\""$(GIT_DATE)\""

LINUX64FORTLIBDIR = $(IFORT_COMPILER)/lib/intel64

# Definition of the object variables

obj = main.o dmalloc.o file_util.o translate.o string_util.o md5.o sha1.o sha256.o stdio_buffer.o
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

# ------------- win 64 ----------------

intel_win_64 : CFLAGS    = -O2 /nologo -D PTW32_STATIC_LIB $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : LFLAGS    =
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = mergepo_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(objwin)

# ------------- gnu 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D GLEW_STATIC -D MINGW
gnu_win_64 : LFLAGS    =
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = mergepo_win_64.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(obj)

# ------------- clang 64 ----------------

clang_win_64 : CFLAGS    = -m64 -D GLEW_STATIC -D MINGW
clang_win_64 : LFLAGS    =
clang_win_64 : CC        = clang-cl
clang_win_64 : exe       = mergepo_win_64.exe

clang_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(LFLAGS) $(objwin)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = mergepo_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gcc_linux_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
gcc_linux_64 : LFLAGS    = -lm
gcc_linux_64 : CC        = gcc
gcc_linux_64 : exe       = mergepo_linux_64

gcc_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu osx 64 ----------------

gcc_osx_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
gcc_osx_64 : CC        = gcc
gcc_osx_64 : exe       = mergepo_osx_64

gcc_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- clang osx 64 ----------------

clang_osx_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(GNU_COMPINFO)
clang_osx_64 : CC        = clang
clang_osx_64 : exe       = mergepo_osx_64

clang_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)
