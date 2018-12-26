
# wind2fds makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = $(SOURCE_DIR)/../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/wind2fds 
FILTERC =

VPATH = $(SOURCE_DIR)/wind2fds $(SOURCE_DIR)/shared 
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

obj =    main.o assert.o dmalloc.o string_util.o file_util.o  md5.o sha1.o sha256.o
objwin = $(obj:.o=.obj)

# main has to be compiled without optimization (on a Windows PC) otherwise errors occcur
main.obj: CFLAGS = -Od /nologo -D WIN32 -D pp_WIN_INTEL

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

intel_win_64 : CFLAGS    = -Od /nologo -D WIN32 -D X64 -D pp_INTEL $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = icl
intel_win_64 : CPP       = icl
intel_win_64 : exe       = wind2fds_win_64.exe

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin)

# ------------- gnu win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D GLEW_STATIC -D MINGW
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = g++
gnu_win_64 : exe       = wind2fds_win_64.exe

gnu_win_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX -D pp_INTEL $(GITINFO) $(GNU_COMPINFO)
intel_linux_64 : CC        = icc
intel_linux_64 : CPP       = icc
intel_linux_64 : exe       = wind2fds_linux_64

intel_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- gnu linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O0 -m64 -D pp_GCC -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : CPP       = g++
gnu_linux_64 : exe       = wind2fds_linux_64

gnu_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- intel osx 64 ----------------

intel_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX -D pp_INTEL $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64 : CC        = icc
intel_osx_64 : CPP       = icc
intel_osx_64 : exe       = wind2fds_osx_64

intel_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) -m64 $(obj)

# ------------- gnu osx 64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64 -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : CPP       = g++
gnu_osx_64 : exe       = wind2fds_osx_64

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- gnu osx 64 db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -D pp_OSX -Wall $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : CPP       = g++
gnu_osx_64_db : exe       = wind2fds_osx_64_db

gnu_osx_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm *.o

#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

