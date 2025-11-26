# stars makefile

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

STARS_WIN_LIBDIR = $(LIB_DIR)/intel_win
ifeq ($(GLUT),freeglut)
  GLUT_WIN_LIB = $(STARS_WIN_LIBDIR)\freeglut_staticd.lib $(STARS_WIN_LIBDIR)\glut32.lib
else
  GLUT_WIN_LIB = $(STARS_WIN_LIBDIR)\glut32.lib
endif
STARS_LIBS_WIN = $(GLUT_WIN_LIB)

WIN32_LIBS = user32.lib gdi32.lib comdlg32.lib shell32.lib

# ------------- intel_win ----------------

intel_win : DYNLIB_EXT = .dll
intel_win : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/stars
intel_win : CFLAGS    = -O1  -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
intel_win : C_STANDARD    = -Qstd=gnu99
ifeq ($(GLUT),freeglut)
intel_win : CFLAGS   += -D FREEGLUT_STATIC
endif
intel_win : LFLAGS    = /F32768000 -D FREEGLUT_STATIC
intel_win : CC        = icx
intel_win : CPP       = icx
intel_win : exe       = stars_win

intel_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(STARS_LIBS_WIN)

# ------------- clang_win ----------------

clang_win : DYNLIB_EXT = .dll
clang_win : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/stars
clang_win : CFLAGS    = -O1  -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
ifeq ($(GLUT),freeglut)
clang_win : CFLAGS   += -D FREEGLUT_STATIC
endif
clang_win : LFLAGS    = -D FREEGLUT_STATIC
clang_win : CC        = clang-cl
clang_win : CPP       = clang-cl
clang_win : exe       = stars_win

clang_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(STARS_LIBS_WIN)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# ------------- intel_linux ----------------

intel_linux : DYNLIB_EXT = .so
intel_linux : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux
intel_linux : CFLAGS    = -O0 -traceback -m64
intel_linux : C_STANDARD    = -std=gnu99
intel_linux : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux : LFLAGS    = -m64
intel_linux : CC        = icx
intel_linux : CPP       = icpx
intel_linux : exe       = stars_linux

intel_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) -lglut -lX11 -lXmu -lGLU -lGL -lm -lrt

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   OSX   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# ------------- gnu_osx ----------------

gnu_osx : CFLAGS    = -O0 -m64              -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings
gnu_osx : C_STANDARD    = -std=gnu99
gnu_osx : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx
ifeq ($(GLUT),freeglut)
gnu_osx : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
gnu_osx : CFLAGS   += -D FREEGLUT_STATIC
else
gnu_osx : LFLAGS   += -framework OpenGL -framework GLUT
endif
gnu_osx : CC        = gcc
gnu_osx : CPP       = g++
gnu_osx : exe       = stars_osx

gnu_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang_osx ----------------

clang_osx : CFLAGS    = -O0 -m64              -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings
clang_osx : C_STANDARD    = -std=gnu99
clang_osx : LFLAGS    = -m64 -L$(LIB_DIR)/clang_osx
ifeq ($(GLUT),freeglut)
clang_osx : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
clang_osx : CFLAGS   += -D FREEGLUT_STATIC
else
clang_osx : LFLAGS   += -framework OpenGL -framework GLUT
endif
clang_osx : CC        = clang
clang_osx : CPP       = clang++
clang_osx : exe       = stars_osx

clang_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm -f *.o *.mod *.dll *.so
