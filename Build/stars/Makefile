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

#--- gnu

SMV_GNU_LIBDIR = $(LIB_DIR)\gnu_win_64
ifeq ($(GLUT),freeglut)
  GLUT_GNU_LIB = $(SMV_GNU_LIBDIR)\freeglut_staticd.a
  D_FREEGLUT = -D FREEGLUT_STATIC
else
  GLUT_GNU_LIB = $(SMV_GNU_LIBDIR)\glut32.a
  D_FREEGLUT =
endif
SMV_LIBS_GNU = $(GLUT_GNU_LIB)

WIN32_LIBS = user32.lib gdi32.lib comdlg32.lib shell32.lib

# ------------- intel_win_64_db ----------------

intel_win_64_db : DYNLIB_EXT = .dll
intel_win_64_db : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/stars
intel_win_64_db : CFLAGS    = /Od /W4 /Wno-cast-function-type-mismatch /Wno-unused-function /Wno-unused-parameter /debug:full /Zi   -D _DEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB 
intel_win_64_db : C_STANDARD    = -Qstd=gnu99

ifeq ($(GLUT),freeglut)
intel_win_64_db : CFLAGS   += -D FREEGLUT_STATIC
endif
intel_win_64_db : LFLAGS    = /F32768000 /traceback /Zi /DEBUG
intel_win_64_db : CC        = icx
intel_win_64_db : CPP       = icx

intel_win_64_db : exe       = stars_win_64_db

intel_win_64_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN)

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

# ------------- gnu_win_64 ----------------

gnu_win_64 : INC       += -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/stars
gnu_win_64 : CFLAGS    = -O0 -m64 $(D_FREEGLUT) -D GLEW_STATIC -D WIN32 -D pp_GCC -D NDEBUG -D __MINGW32__ -fno-builtin -Wno-write-strings $(GITINFO) $(GNU_COMPINFO) $(SMV_PROFILEFLAG)
gnu_win_64 : LFLAGS    = -m64 $(SMV_LIBS_GNU) -lm -lopengl32 -lglu32 -lgdi32 -lwinmm -lcomdlg32 -lhid -lquadmath $(SMV_PROFILEFLAG)
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = g++
gnu_win_64 : exe       = stars_win_64

gnu_win_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu_win_64_db ----------------

#-Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wunused-parameter -Wunused-but-set-parameter

# to profile the windows build of stars
# 1.  cd smv/Build/stars/gnu_win_64
# 2.  type: make_stars
# 2.  run case using stars_win_test_db
# 3.  type: gprof stars_application_name > results.out

gnu_win_64_db : INC       += -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/stars
gnu_win_64_db : CFLAGS    = -O0 -m64 -g -Wunused-label -Wunused-value -Wunused-variable -fno-builtin -Wno-write-strings $(GITINFO) $(GNU_COMPINFO)
gnu_win_64_db : LFLAGS    = -m64 $(SMV_LIBS_GNU) -lm -lopengl32 -lglu32 -lgdi32 -lwinmm -lcomdlg32 -lhid -lquadmath
gnu_win_64_db : CC        = gcc
gnu_win_64_db : CPP       = g++
gnu_win_64_db : exe       = stars_win_64

gnu_win_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SMV_LIBS_LINUX= -lglui -lglut -lgd -ljpeg -lpng -lz
SYSTEM_LIBS_LINUX = -lpthread -lX11 -lXmu -lGLU -lGL -lm -lrt

# to profile stars on linux
# 1.  cd smv/Build/stars/intel_linux_64
# 1.  type: ./make_smv.sh -p
# 2.  run case as usual
# 3.  type: gprof stars_application_name > results.out

# ------------- intel_linux_64 ----------------

intel_linux_64 : DYNLIB_EXT = .so
intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux_64 : CFLAGS    = -O0 -traceback -m64 
intel_linux_64 : C_STANDARD    = -std=gnu99
intel_linux_64 : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux_64 : LFLAGS    = -m64
intel_linux_64 : CC        = icx
intel_linux_64 : CPP       = icpx
intel_linux_64 : exe       = stars_linux_64

intel_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- intel_linux_64_db ----------------

intel_linux_64_db : CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux_64_db : CFLAGS    = -O0 -g -m64 $(SMV_TESTFLAG) -D _DEBUG -D pp_LINUX $(SMV_PROFILEFLAG) \
	                        -fstack-protector -fno-omit-frame-pointer -traceback -debug variable-locations \
                                -Wall -Wextra -Wuninitialized -Wunused-function -Wunused-variable -Wno-sign-compare \
                                -Wno-unused-parameter
intel_linux_64_db : C_STANDARD    = -std=gnu99
intel_linux_64_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux_64_db : LFLAGS    = -m64 -static-intel $(INTEL_LIBS_LINUX)
intel_linux_64_db : CC        = icx
intel_linux_64_db : CPP       = icpx
intel_linux_64_db : exe       = stars_linux_64

intel_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/intel_linux_64 $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux_64_db ----------------

gnu_linux_64_db : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64_db : CFLAGS    = -O0  -m64 -ggdb -Wall -Wunreachable-code -Wswitch-default -Wshadow -Winit-self -Wundef -Wpointer-arith -Wuninitialized -Wno-address -Wno-format-overflow -Wno-format-truncation -Wno-undef -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_linux_64_db : C_STANDARD    = -std=gnu99
gnu_linux_64_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
gnu_linux_64_db : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : CPP       = g++
gnu_linux_64_db : exe       = stars_linux_64

gnu_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux_64 ----------------

gnu_linux_64 : DYNLIB_EXT = .so
gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings           -D pp_LINUX -D pp_GCC -D NDEBUG $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_linux_64 : C_STANDARD    = -std=gnu99
gnu_linux_64 : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
gnu_linux_64 : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : CPP       = g++
gnu_linux_64 : exe       = stars_linux_64

gnu_linux_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   check source   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# ------------- gnu_linux_64_check ----------------

gnu_linux_64_check : CFLAGS    = -O2 -m64 -Wall
gnu_linux_64_check : $(src)
	clang-tidy $^ --  $(CFLAGS) $(INC)

# ------------- linux_osx_check ----------------

linux_osx_check : CFLAGS    = $(SMV_TESTFLAG)
linux_osx_check : $(linux_osx_chk)
	$(CLANGSUMMARY) stars.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC += $(WININC)
win_check : $(win_chk)
	$(CLANGWINSUMMARY) stars.summary

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   OSX   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

ifeq ($(GLUT),freeglut)
  SMV_LIBS_OSX =  -lglui  -lglut -lgd -ljpeg -lpng -lz
else
  SMV_LIBS_OSX =  -lglui        -lgd -ljpeg -lpng -lz
endif

# ------------- gnu_osx_64_db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -ggdb -m64 -D _DEBUG -D pp_OSX -D pp_GCC $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx_64_db : C_STANDARD    = -std=gnu99
gnu_osx_64_db : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_osx_64_db : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
gnu_osx_64_db : CFLAGS   += -D FREEGLUT_STATIC
else
gnu_osx_64_db : LFLAGS   += -framework OpenGL -framework GLUT
endif
gnu_osx_64_db : CC        = ${COMPILER}
gnu_osx_64_db : CPP       = ${COMPILER2}
gnu_osx_64_db : exe       = stars_osx_64

gnu_osx_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu_osx_64 ----------------

gnu_osx_64 : CFLAGS    = -O0 -m64              -D pp_OSX -D pp_GCC -D NDEBUG $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx_64 : C_STANDARD    = -std=gnu99
gnu_osx_64 : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_osx_64 : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
gnu_osx_64 : CFLAGS   += -D FREEGLUT_STATIC
else
gnu_osx_64 : LFLAGS   += -framework OpenGL -framework GLUT
endif
gnu_osx_64 : CC        = ${COMPILER}
gnu_osx_64 : CPP       = ${COMPILER2}
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
