
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

intel_win_64 : CFLAGS    = -Od /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = $(I_CC)
intel_win_64 : exe       = getdate_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- clang win 64 ----------------

clang_win_64 : CFLAGS    = -m64 -Wno-deprecated-declarations
clang_win_64 : CC        = clang-cl
clang_win_64 : exe       = getdate_win_64.exe

clang_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : exe       = getdate_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)
