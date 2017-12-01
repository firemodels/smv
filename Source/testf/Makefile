
# smokezip makefile

LIB_DIR = /opt/fltk/lib
INC_DIR =  -I /opt/fltk/include
FILTERC =
FILTERF =
OPT =

VPATH = .
bin = .


obj =    hello.o
objwin = $(obj:.o=.obj)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .F90 .cxx .c .obj .o .f90 .cpp

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.cxx.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.F90.o:
	$(FC) -c $(FFLAGS) $(FILTERF) $<
.f90.obj:
	$(FC) -c $(FFLAGS) $(FILTERF) $<
.f90.o:
	$(FC) -c $(FFLAGS) $(FILTERF) $<



# ********  rules for each platform supported *************

# ------------- Windows 64 ----------------
#  64 bit windows

intel_win_64 : CFLAGS    = -O2 -m64 $(INC_DIR) 
intel_win_64 : LFLAGS    = -L $(LIB_DIR) -lfltk
intel_win_64 : CC        = icc
intel_win_64 : CPP       = icc
intel_win_64 : exe       = hello_win_64

intel_win_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- Linux 64 ----------------
#  64 bit linux

intel_linux_64 : CFLAGS    = -O2 -m64 $(INC_DIR) 
intel_linux_64 : LFLAGS    = -L $(LIB_DIR) -lfltk
intel_linux_64 : CC        = icc
intel_linux_64 : CPP       = icc
intel_linux_64 : exe       = hello_linux_64

intel_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- OSX 64 ----------------
#  64 bit OSX

intel_osx_64 : CFLAGS    = -O2 -m64 $(INC_DIR)
intel_osx_64 : LFLAGS    = -L $(LIB_DIR) -lfltk
intel_osx_64 : CC        = icc
intel_osx_64 : CPP       = icc
intel_osx_64 : exe       = hello_osx_64

intel_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)
