SOURCE_DIR = ../../../Source/
INC_DIR = -I $(SOURCE_DIR)/mergepo -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared 
FILTERC =
FILTERF =

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

ifeq ($(shell echo "check_quotes"),"check_quotes")
  INTEL_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\intel_compversion)
  GNU_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\gnu_compversion)
else
  INTEL_COMPVERSION := $(shell ../../../Utilities/Scripts/intel_compversion.sh)
  GNU_COMPVERSION := $(shell ../../../Utilities/Scripts/gnu_compversion.sh)
endif
INTEL_COMPINFO=-Dpp_COMPVER=\"$(INTEL_COMPVERSION)\"
GNU_COMPINFO=-Dpp_COMPVER=\"$(GNU_COMPVERSION)\"


LINUX64FORTLIBDIR = $(IFORT_COMPILER)/lib/intel64

# Definition of the object variables

obj = main.o dmalloc.o assert.o file_util.o translate.o string_util.o md5.o sha1.o sha256.o
objwin = $(obj:.o=.obj)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .F90 .c .obj .o .f90 .cpp

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.F90.o:
	$(FC) -c $(FFLAGS) $(FILTERF) $<
.F90.obj:
	$(FC) -c $(FFLAGS) $(FILTERF) $<
.f90.obj:
	$(FC) -c $(FFLAGS) $(FILTERF) $<
.f90.o:
	$(FC) -c $(FFLAGS) $(FILTERF) $<



# ********  rules for each platform supported *************

# ------------- win 64 ----------------

intel_win_64 : CFLAGS    = -O2 /nologo -D X64 -D WIN32 -D pp_INTEL -D PTW32_STATIC_LIB $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : LFLAGS    = 
intel_win_64 : CC        = icl
intel_win_64 : CPP       = icl
intel_win_64 : exe       = mergepo_win_64.exe

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin)

# ------------- gnu 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D WIN32 -D _WIN32 -D GLEW_STATIC -D MINGW
gnu_win_64 : LFLAGS    = 
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = gcc
gnu_win_64 : exe       = mergepo_win_64.exe

gnu_win_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(obj)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX -D pp_INTEL $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = icc
intel_linux_64 : CPP       = icc
intel_linux_64 : exe       = mergepo_linux_64

intel_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gcc_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX $(GITINFO) $(GNU_COMPINFO)
gcc_linux_64 : CC        = gcc
gcc_linux_64 : CPP       = g++
gcc_linux_64 : exe       = mergepo_linux_64

gcc_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- intel osx 64 ----------------

intel_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX -D pp_INTEL $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64 : CC        = icc
intel_osx_64 : CPP       = icc
intel_osx_64 : exe       = mergepo_osx_64

intel_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx 64 ----------------

gcc_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gcc_osx_64 : CC        = gcc
gcc_osx_64 : CPP       = g++
gcc_osx_64 : exe       = mergepo_osx_64

gcc_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)
