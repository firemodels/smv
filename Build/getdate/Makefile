
# getdate makefile

SOURCE_DIR = ../../../Source/
FILTERC =
FILTERF =

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

ifeq ($(shell echo "check_quotes"),"check_quotes")
  INTEL_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\intel_compversion)
  GNU_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\gnu_compversion)
else
  INTEL_COMPVERSION := $(shell ../../../Utilities/Scripts/intel_compversion.sh)
  GNU_COMPVERSION := $(shell ../../../Utilities/Scripts/gnu_compversion.sh)
endif
INTEL_COMPINFO=-Dpp_COMPVER=\"$(INTEL_COMPVERSION)\"
GNU_COMPINFO=-Dpp_COMPVER=\"$(GNU_COMPVERSION)\"

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
intel_win_64 : CC        = icl
intel_win_64 : exe       = getdate_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D WIN32 -D _WIN32 -D GLEW_STATIC -D MINGW
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = getdate_win_64.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = icc
intel_linux_64 : exe       = getdate_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) 

# ------------- intel osx 64 ----------------

intel_osx_64 : CFLAGS    = -O2 -m64 $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64 : CC        = icc
intel_osx_64 : exe       = getdate_osx_64

intel_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) -m64 $(obj)
