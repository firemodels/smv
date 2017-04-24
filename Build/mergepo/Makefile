SOURCE_DIR = ../../../Source/
INC_DIR = -I $(SOURCE_DIR)/mergepo -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared 
FILTERC =
FILTERF =

VPATH = $(SOURCE_DIR)/mergepo:$(SOURCE_DIR)/shared
bin = .

LINUX64FORTLIBDIR = $(IFORT_COMPILER)/lib/intel64

# Definition of the object variables

obj = main.o dmalloc.o assert.o file_util.o translate.o string_util.o
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

# ------------- Win 64 ----------------
#  64 bit windows

intel_win_64 : CFLAGS    = -O2 /nologo -D X64 -D WIN32 -D PTW32_STATIC_LIB
intel_win_64 : LFLAGS    = 
intel_win_64 : CC        = icl
intel_win_64 : CPP       = icl
intel_win_64 : exe       = mergepo_win_64.exe

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) -l $(LFLAGS) $(objwin)

# ------------- Linux 64 ----------------
#  64 bit linux

intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX64
intel_linux_64 : CC        = icc
intel_linux_64 : CPP       = icc
intel_linux_64 : exe       = mergepo_linux_64

intel_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj)

# ------------- Linux 64 debug ----------------
#  64 bit linux debug

intel_linux_64_db : CFLAGS    =  -O0 -g -m64 -static-intel $(SMV_TESTFLAG) -D _DEBUG -D pp_LINUX -D pp_INTEL $(SMV_PROFILEFLAG) -traceback -Wall -Wextra -check=stack,uninit -fp-stack-check -fp-trap-all=divzero,invalid,overflow -ftrapuv -Wuninitialized -Wunused-function -Wunused-variable
intel_linux_64_db : LFLAGS    = -m64 -static-intel -traceback
intel_linux_64_db : CC        = icc
intel_linux_64_db : CPP       = icc
intel_linux_64_db : exe       = mergepo_linux_64_db

intel_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(obj)
