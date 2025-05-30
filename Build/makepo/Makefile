SOURCE_DIR = ../../../Source/
INC_DIR = -I $(SOURCE_DIR)/makepo -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/makepo:$(SOURCE_DIR)/shared
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

obj = main.o file_util.o string_util.o dmalloc.o md5.o sha1.o sha256.o stdio_buffer.o
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

# ------------- intel win 64 ----------------

intel_win_64 : CFLAGS    = -O2 /nologo -I $(SOURCE_DIR)/pthreads -D X64 -D WIN32 -D PTW32_STATIC_LIB  $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : LFLAGS    =
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = makepo_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LFLAGS)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D WIN32 -D _WIN32 -D GLEW_STATIC -D MINGW
gnu_win_64 : LFLAGS    =
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = makepo_win_64.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : LFLAGS    = -lpthread
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = makepo_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gcc_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX $(GITINFO) $(GNU_COMPINFO)
gcc_linux_64 : LFLAGS    = -lm
gcc_linux_64 : CC        = gcc
gcc_linux_64 : exe       = makepo_linux_64

gcc_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gcc osx 64 ----------------

gcc_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gcc_osx_64 : CC        = gcc
gcc_osx_64 : exe       = makepo_osx_64

gcc_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)


#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm *.o
