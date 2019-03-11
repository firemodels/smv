
# background makefile

SOURCE_DIR = ../../../Source
INC_DIR = -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/background
FILTERC =
FILTERF =

VPATH = $(SOURCE_DIR)/background $(SOURCE_DIR)/shared
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

obj = main.o string_util.o dmalloc.o assert.o file_util.o md5.o sha1.o sha256.o
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

# ********  rules for each platform supported *************

# ------------- intel Win 64 ----------------

intel_win_64 : CFLAGS    = -O1 /nologo -D WIN32 -D pp_INTEL -I $(SOURCE_DIR)/shared $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = icl
intel_win_64 : exe       = background.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) 

# ------------- gnu Win 64 ----------------

gnu_win_64 : CFLAGS    = -O0 -m64 -D WIN32 -D _WIN32 -D GLEW_STATIC -D MINGW
gnu_win_64 : CC        = gcc
gnu_win_64 : exe       = background.exe

gnu_win_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) 

# ------------- intel linux 64 ----------------

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_INTEL -D pp_LINUX $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = icc
intel_linux_64 : exe       = background

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) 

# ------------- gnu Linux 64 ----------------

gnu_linux_64 : CFLAGS    = -O2 -m64 -Wall -D pp_GCC -D pp_LINUX $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = background

gnu_linux_64: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm

# ------------- gnu Linux 64 db ----------------

gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : exe       = background

gnu_linux_64_db: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -lm

# ------------- OSX 64 ----------------
#  64 bit osx

intel_osx_64 : CFLAGS    = -O2 -m64 -D pp_INTEL -D pp_OSX $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64 : CC        = icc
intel_osx_64 : exe       = background

intel_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) -m64 $(obj) 

# ------------- gcc OSX 64 ----------------
gnu_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = background

gnu_osx_64 : $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)  -lm

# ------------- gnu OSX debug ----------------

gnu_osx_64_db : CFLAGS    = -O0 -m64 -Wall -D pp_OSX -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : exe       = background_db

gnu_osx_64_db : $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj)  -lm


#-------------- compile sring_util.c so revision strings are updated -----------

string_util.o:  .FORCE
string_util.obj:        .FORCE

.FORCE:

