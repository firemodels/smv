# stars makefile

#To use this Makefile cd to a sub-directory and type make_smv.sh or make_smv.bat

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

#----------------------------------------------------------------------------
# Should not need to edit lines below except to add or 'debug' target entries

# The variable VPATH defines the source code directory relative to the current directory

VPATH = $(SOURCE_DIR)/stars
bin = .

# Definition of the object variables

csrc = stars.c

src = $(csrc)

cobj = $(csrc:.c=.o)
obj = $(cobj)


incs = 

objwin = $(obj:.o=.obj)

INC += -I $(SOURCE_DIR)/shared

# windows include directories

ifeq ($(GLUT),freeglut)
  WININC = -I $(SOURCE_DIR)/freeglut3.0.0/include
else
  WININC = -I $(SOURCE_DIR)/glut_gl
endif
WININC += -I $(SOURCE_DIR)/pthreads

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o .cpp .chk .winchk

.c.obj:
	$(CC) -c $(C_STANDARD) $(CFLAGS) $(INC) $<
.c.o:
	$(CC) -c $(C_STANDARD) $(CFLAGS) $(INC) $<

.cpp.obj:
	$(CPP) -c $(CPP_STANDARD) $(CFLAGS) $(INC) $<
.cpp.o:
	$(CPP) -c $(CPP_STANDARD) $(CFLAGS) $(INC) $<

# ********  rules for each platform supported *************

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Windows VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

#--- Intel

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win_64
ifeq ($(GLUT),freeglut)
  GLUT_WIN_LIB = $(SMV_WIN_LIBDIR)\freeglut_staticd.lib $(SMV_WIN_LIBDIR)\glut32.lib
else
  GLUT_WIN_LIB = $(SMV_WIN_LIBDIR)\glut32.lib
endif
SMV_LIBS_WIN = $(GLUT_WIN_LIB)

WIN32_LIBS = user32.lib gdi32.lib comdlg32.lib shell32.lib

# ------------- intel_win_64 ----------------

intel_win_64 : DYNLIB_EXT = .dll
intel_win_64 : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/stars
intel_win_64 : CFLAGS    = -O1  -D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB
intel_win_64 : C_STANDARD    = -Qstd=gnu99
ifeq ($(GLUT),freeglut)
intel_win_64 : CFLAGS   += -D FREEGLUT_STATIC
endif
intel_win_64 : LFLAGS    = /F32768000 -D FREEGLUT_STATIC
intel_win_64 : CC        = icx
intel_win_64 : CPP       = icx
intel_win_64 : exe       = stars_win_64

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SMV_LIBS_LINUX=-lglut
SYSTEM_LIBS_LINUX = -lpthread -lX11 -lXmu -lGLU -lGL -lm -lrt

# ------------- intel_linux_64 ----------------

intel_linux_64 : DYNLIB_EXT = .so
intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CFLAGS    = -O0 -traceback -m64 
intel_linux_64 : C_STANDARD    = -std=gnu99
intel_linux_64 : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux_64 : LFLAGS    = -m64
intel_linux_64 : CC        = icx
intel_linux_64 : CPP       = icpx
intel_linux_64 : exe       = stars_linux_64

intel_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   OSX   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

ifeq ($(GLUT),freeglut)
  SMV_LIBS_OSX =  -lglut
else
  SMV_LIBS_OSX =
endif

# ------------- gnu_osx_64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64              -D pp_OSX -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings
gnu_osx_64 : C_STANDARD    = -std=gnu99
gnu_osx_64 : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) $(GLIBDIROPT)
ifeq ($(GLUT),freeglut)
gnu_osx_64 : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
gnu_osx_64 : CFLAGS   += -D FREEGLUT_STATIC
else
gnu_osx_64 : LFLAGS   += -framework OpenGL -framework GLUT
endif
gnu_osx_64 : CC        = gcc
gnu_osx_64 : CPP       = g++
gnu_osx_64 : exe       = stars_osx_64

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm -f *.o *.mod *.dll *.so

#-------------- force compilation of string_util.c -----------

string_util.o:	.FORCE
string_util.obj:	.FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
