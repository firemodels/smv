FILTERC =
FILTERF =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

VPATH = ../../../Source/hashfort
bin = .

# Definition of the object variables

obj = hashfort.o main.o
objwin =  hashfort.obj main.obj

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .cpp

# ------------- intel win 64 ----------------

intel_win_64 : FFLAGS    = -O2 /nologo /fpscomp:general /stand:f18
intel_win_64 : FC        = $(I_IFORT)
intel_win_64 : exe       = hashfort_win_64.exe

intel_win_64 : $(objwin)
	$(FC) -o $(bin)/$(exe) $(objwin)

# ------------- intel linux 64 ----------------

intel_linux_64 : FFLAGS    = -O2 -m64 -stand:f18
intel_linux_64 : FC        = $(I_IFORT)
intel_linux_64 : exe       = hashfort_linux_64

intel_linux_64 : $(obj)
	$(FC) -o $(bin)/$(exe) $(obj)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm *.o
