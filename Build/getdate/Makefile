
# getdate makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = $(SOURCE_DIR)/getdate
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

obj =    main.o
objwin = $(obj:.o=.obj)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $<



# ********  rules for each platform supported *************

# ------------- intel win 64 ----------------

intel_win : CFLAGS    = -Od /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win : CC        = $(I_CC)
intel_win : exe       = getdate_win.exe

intel_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- clang win 64 ----------------

clang_win : CFLAGS    = -m64 -Wno-deprecated-declarations
clang_win : CC        = clang-cl
clang_win : exe       = getdate_win.exe

clang_win : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- intel linux 64 ----------------

intel_linux : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux : CC        = $(I_ICC)
intel_linux : exe       = getdate_linux

intel_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gnu_linux : CFLAGS    = -O2 -m64 $(GITINFO)
gnu_linux : CC        = gcc
gnu_linux : exe       = getdate_linux

gnu_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- clang linux 64 ----------------

clang_linux : CFLAGS    = -O2 -m64 $(GITINFO)
clang_linux : CC        = clang
clang_linux : exe       = getdate_linux

clang_linux : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)
