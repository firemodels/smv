SOURCE_DIR = ../../../Source/
BUILD_DIR  = ../../../Build
INC_DIR    = -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/hashfile

FILTERC =
FILTERF =

VPATH = $(SOURCE_DIR)/hashfile $(SOURCE_DIR)/shared  
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

obj =    assert.o dmalloc.o file_util.o string_util.o sha1.o md5.o sha256.o main.o
objwin = $(obj:.o=.obj)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o 

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<



# ********  rules for each platform supported *************

# ------------- Windows 64 ----------------

intel_win_64 : CFLAGS    = -Od -D WIN32 -D pp_INTEL /nologo $(GITINFO)
intel_win_64 : CC        = icl
intel_win_64 : exe       = hashfile_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- gnu Linux 32 ----------------

gnu_linux_32 : CFLAGS    = -O2 -Wall -D pp_GCC -D pp_LINUX $(GITINFO)
gnu_linux_32 : CC        = gcc
gnu_linux_32 : exe       = hashfile_linux_32

gnu_linux_32: $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -lm

# ------------- gnu Linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O2 -m64 -Wall -D pp_GCC -D pp_LINUX $(GITINFO)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = hashfile_linux_64

gnu_linux_64: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm


# ------------- Linux 64 ----------------
#  64 bit linux

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX -D pp_INTEL $(GITINFO)
intel_linux_64 : CC        = icc
intel_linux_64 : exe       = hashfile_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)
	
# ------------- OSX 64 ----------------
#  64 bit OSX

intel_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX -D pp_INTEL $(GITINFO)
intel_osx_64 : CC        = icc
intel_osx_64 : exe       = hashfile_osx_64

intel_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) -m64 $(obj)
