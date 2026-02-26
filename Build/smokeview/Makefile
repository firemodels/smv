# Smokeview makefile

#To use this Makefile cd to a sub-directory and type make_smv.sh or make_smv.bat

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
FREEGLUT_DIR=../../../../libs/freeglut

SMV_TESTFLAG =
SMV_TESTSTRING =
SMV_PROFILEFLAG =
SMV_PROFILESTRING =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

COMP_VERSION =
ifeq ($(shell echo "check_quotes"),"check_quotes")
  GIT_HASH   := $(shell ..\..\..\Utilities\Scripts\githash)
  GIT_DATE   := $(shell ..\..\..\Utilities\Scripts\gitlog)
  GIT_BRANCH := $(shell ..\..\..\Utilities\Scripts\gitbranch)
else
  GLIBVERS   := $(shell echo $(GLIBVERS))
  GIT_HASH   := $(shell git describe --long --dirty)
  GIT_DATE   := $(shell git log -1 --format=%cd)
  GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
endif
GITINFO=-Dpp_GITHASH=\"$(GIT_HASH)-$(GIT_BRANCH)\" -Dpp_GITDATE=\""$(GIT_DATE)\""

#oneapi debug options
SANITIZE_OPTIONS=-fsanitize=address -fsanitize=undefined -D pp_SANITIZE

.rc.obj:
	windres $< -o $@
%.obj : %.rc
	windres $< -o $@

#----------------------------------------------------------------------------
# Should not need to edit lines below except to add or 'debug' target entries

# The variable VPATH defines the source code directory relative to the current directory

VPATH = $(SOURCE_DIR)/smokeview:$(SOURCE_DIR)/shared:$(SOURCE_DIR)/glew
bin = .

# Definition of the object variables

csrc = callbacks.c camera.c color2rgb.c readimage.c readgeom.c readobject.c readcad.c colortimebar.c  \
       compress.c csphere.c dmalloc.c  drawGeometry.c file_util.c getdata.c getdatabounds.c \
       getdatacolors.c glew.c histogram.c infoheader.c readlabel.c readtour.c readhvac.c readslice.c  \
       readsmoke.c colorbars.c colorbar_defs.c IOboundary.c IOgeometry.c IOhvac.c IOiso.c IOobjects.c \
       IOpart.c IOplot2d.c IOplot3d.c IOscript.c IOshooter.c IOslice.c IOsmoke.c IOtour.c IOvolsmoke.c \
       IOwui.c IOzone.c isobox.c main.c md5.c menus.c output.c readsmvfile.c readsmv.c  \
       renderimage.c scontour2d.c sha1.c sha256.c shaders.c showscene.c skybox.c \
       smokeview.c smv_geometry.c startup.c stdio_buffer.c stdio_m.c string_util.c threader.c  \
       translate.c unit.c update.c viewports.c colortable.c command_args.c paths.c

cppsrc = glui_bounds.cpp  glui_clip.cpp glui_colorbar.cpp glui_display.cpp glui_geometry.cpp glui_motion.cpp \
         glui_objects.cpp glui_shooter.cpp glui_smoke.cpp glui_stereo.cpp glui_tour.cpp glui_trainer.cpp

src = $(csrc) $(cppsrc)

cobj = $(csrc:.c=.o)
cppobj = $(cppsrc:.cpp=.o)
obj = $(cobj) $(cppobj)

# setup for analyzing smokeview source code using clang

linux_osx_c_chk    = $(csrc:.c=.chk)
linux_osx_cpp_chk  = $(cppsrc:.cpp=.chk)
linux_osx_chk      = $(linux_osx_c_chk) $(linux_osx_cpp_chk)

win_c_chk   = $(csrc:.c=.winchk)
win_cpp_chk = $(cppsrc:.cpp=.winchk)
win_chk     = $(win_c_chk) $(win_cpp_chk)

#CLANGANALYZE=clang --analyze -Xanalyzer -analyzer-checker=core,deadcode
CLANGANALYZE=clang --analyze
CLANGSUMMARY=../../scripts/clang_summary.sh
CLANGWINSUMMARY=..\..\scripts\clang_summary.bat

incs = compress.h scontour2d.h csphere.h datadefs.h dirent_win.h file_util.h glui_bounds.h \
       glui_motion.h glui_smoke.h glui_tour.h histogram.h infoheader.h interp.h isobox.h \
       IOobjects.h IOscript.h IOvolsmoke.h lint.h dmalloc.h options.h options_common.h \
       pragmas.h smokeheaders.h smokeviewdefs.h smokeviewvars.h \
       stdio_buffer.h stdio_m.h string_util.h structures.h translate.h threader.h viewports.h command_args.h

ifeq ($(ICON),icon)
obj += resource.o
endif

objwin = $(obj:.o=.obj)

#*** common include directories

INC  = -I $(SOURCE_DIR)/glui_v2_1_beta -I $(SOURCE_DIR)/gd-2.3.3 -I $(SOURCE_DIR)/shared \
       -I $(SOURCE_DIR)/smokeview      -I $(SOURCE_DIR)/glew     -I $(SOURCE_DIR)/zlib131

ifeq ($(GLUT),freeglut)
  INC += -I $(FREEGLUT_DIR)/include
endif
ifeq ($(shell echo "check_quotes"),"check_quotes")
  INC += -I $(SOURCE_DIR)/pthreads
endif

#*** common C flags

CFLAG_COM=$(SMV_TESTFLAG) $(SMV_PROFILEFLAG) $(GITINFO)
ifeq ($(SANITIZE),1)
CFLAG_COM   += $(SANITIZE_OPTIONS)
endif
ifeq ($(GLUT),freeglut)
CFLAG_COM   += -D pp_FREEGLUT -DFREEGLUT_STATIC
endif
CFLAG_COM   += -D GLEW_STATIC -D PTW32_STATIC_LIB -D BGDWIN32

#*** common linker flags

LFLAG_COM    = -m64 $(SMV_PROFILEFLAG)
ifeq ($(SANITIZE),1)
LFLAG_COM   += $(SANITIZE_OPTIONS)
endif

LFLAG_WIN   =  /link /STACK:32768000 /NODEFAULTLIB:msvcrt.lib
ifeq ($(GLUT),freeglut)
  LFLAG_WIN  +=  /NODEFAULTLIB:freeglut.lib /NODEFAULTLIB:freeglut_staticd.lib
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

# linux and osx clang checks
.c.chk:
	$(CLANGANALYZE) $(CFLAGS) $(INC) $< 2> $@
.cpp.chk:
	$(CLANGANALYZE) $(CFLAGS) $(INC) $< 2> $@

# windows clang checks
.c.winchk:
	$(CLANGANALYZE) $(CFLAGS) $(INC) $< 2> $@
.cpp.winchk:
	$(CLANGANALYZE) $(CFLAGS) $(INC) $< 2> $@

# ********  rules for each platform supported *************

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Windows VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

ifeq ($(GLUT),freeglut)
  GLUTLIB_WIN = freeglut_static.lib
else
  GLUTLIB_WIN = glut32.lib
endif
SMV_LIBS_WIN = glui.lib gd.lib jpeg.lib png.lib zlib.lib pthreads.lib $(GLUTLIB_WIN)
SYSTEM_LIBS_WIN = user32.lib gdi32.lib comdlg32.lib shell32.lib

# ------------- intel_win_db ----------------

intel_win_db : DYNLIB_EXT = .dll
intel_win_db : CFLAGS    = /Od -D pp_INTEL -D _DEBUG -D _CONSOLE \
                           /Zi /W4 /Wno-cast-function-type-mismatch /Wno-unused-function /Wno-unused-parameter /debug:full \
                           $(INTEL_COMPINFO) $(CFLAG_COM)
intel_win_db : C_STANDARD    = -Qstd=gnu99
intel_win_db : LFLAGS   =   /traceback /Zi /DEBUG \
                           $(LFLAG_COM) $(LFLAG_WIN) /LIBPATH:$(LIB_DIR)/intel_win
intel_win_db : CC        = icx
intel_win_db : CPP       = icx
intel_win_db : exe       = smokeview_win$(SMV_TESTSTRING)_db

intel_win_db : $(objwin)
	$(CPP) -o $(bin)/$(exe)  -m64 $(objwin) $(SMV_LIBS_WIN) $(SYSTEM_LIBS_WIN) $(LFLAGS)

# ------------- intel_win ----------------

intel_win : CFLAGS    = -O1 -D pp_INTEL -D NDEBUG -D _CONSOLE -D NONDLL $(INTEL_COMPINFO) $(CFLAG_COM)
intel_win : C_STANDARD    = -Qstd=gnu99
intel_win : LFLAGS    = $(LFLAG_COM) $(LFLAG_WIN) /LIBPATH:$(LIB_DIR)/intel_win
intel_win : CC        = icx
intel_win : CPP       = icx
intel_win : exe       = smokeview_win$(SMV_TESTSTRING)

intel_win : $(objwin)
	$(CPP) -o $(bin)/$(exe)  -m64 $(objwin) $(SMV_LIBS_WIN) $(SYSTEM_LIBS_WIN) $(LFLAGS)

# ------------- clang_win_db ----------------

clang_win_db : CFLAGS    = -D NDEBUG -D _CONSOLE -D NONDLL $(GNU_COMPINFO) $(CFLAG_COM)
clang_win_db : LFLAGS    = $(LFLAG_COM) $(LFLAG_WIN) /LIBPATH:$(LIB_DIR)/clang_win
clang_win_db : CC        = clang-cl
clang_win_db : CPP       = clang-cl
clang_win_db : exe       = smokeview_win$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

clang_win_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN) $(SYSTEM_LIBS_WIN) $(LFLAGS)

# ------------- clang_win ----------------

clang_win : CFLAGS    = -D NDEBUG -D _CONSOLE -D NONDLL $(GNU_COMPINFO) $(CFLAG_COM)
clang_win : LFLAGS    = $(LFLAG_COM) $(LFLAG_WIN) /LIBPATH:$(LIB_DIR)/clang_win
clang_win : CC        = clang-cl
clang_win : CPP       = clang-cl
clang_win : exe       = smokeview_win$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

clang_win : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(objwin) $(SMV_LIBS_WIN) $(SYSTEM_LIBS_WIN) $(LFLAGS)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SMV_LIBS_LINUX= -lglui -lglut -lgd -ljpeg -lpng -lz
SYSTEM_LIBS_LINUX = -lpthread -lX11 -lXmu -lXi -lGLU -lGL -lm -lrt

# to profile smokeview on linux
# 1.  cd smv/Build/smokeview/intel_linux
# 1.  type: ./make_smv.sh -p
# 2.  run case as usual
# 3.  type: gprof smokeview_application_name > results.out

# ------------- intel_linux ----------------

intel_linux : DYNLIB_EXT = .so
intel_linux : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux
intel_linux : CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux : CFLAGS    = -O0 -traceback -m64 -D NDEBUG $(SMV_TESTFLAG) $(SMV_PROFILEFLAG) $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(SANITIZE),1)
intel_linux : CFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_linux : C_STANDARD    = -std=gnu99
intel_linux : LFLAGS    = -m64 $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
ifeq ($(SANITIZE),1)
intel_linux : LFLAGS   += $(SANITIZE_OPTIONS)
endif
ifeq ($(GLUT),freeglut)
intel_linux : LFLAGS   += -L$(FREEGLUT_DIR)/lib
intel_linux : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
endif
intel_linux : CC        = icx
intel_linux : CPP       = icpx
intel_linux : exe       = smokeview_linux$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

intel_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- intel_linux_db ----------------

intel_linux_db : CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux_db : CFLAGS    = -O0 -g -m64 $(SMV_TESTFLAG) -D _DEBUG $(SMV_PROFILEFLAG) \
	                        -fstack-protector -fno-omit-frame-pointer -traceback -debug variable-locations \
                                -Wall -Wextra -Wuninitialized -Wunused-function -Wunused-variable -Wno-sign-compare \
                                -Wno-unused-parameter $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(SANITIZE),1)
intel_linux_db : CFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_linux_db : C_STANDARD    = -std=gnu99
intel_linux_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux_db : LFLAGS    = -m64 -static-intel $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
ifeq ($(SANITIZE),1)
intel_linux_db : LFLAGS   += $(SANITIZE_OPTIONS)
endif
ifeq ($(GLUT),freeglut)
intel_linux_db : LFLAGS   += -L$(FREEGLUT_DIR)/lib
intel_linux_db : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
endif
intel_linux_db : CC        = icx
intel_linux_db : CPP       = icpx
intel_linux_db : exe       = smokeview_linux$(SMV_TESTSTRING)_db$(SMV_PROFILESTRING)

intel_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/intel_linux $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux_db ----------------

gnu_linux_db : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux
gnu_linux_db : CFLAGS    = -O0  -m64 -ggdb -Wall -Wunreachable-code -Wswitch-default -Wshadow -Winit-self -Wundef -Wpointer-arith -Wuninitialized -Wno-address -Wno-format-overflow -Wno-format-truncation -Wno-undef -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_linux_db : C_STANDARD    = -std=gnu99
gnu_linux_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
gnu_linux_db : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_linux_db : LFLAGS   += -L$(FREEGLUT_DIR)/lib
gnu_linux_db : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
endif
gnu_linux_db : CC        = gcc
gnu_linux_db : CPP       = g++
gnu_linux_db : exe       = smokeview_linux$(SMV_TESTSTRING)_db$(SMV_PROFILESTRING)

gnu_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux ----------------

gnu_linux : DYNLIB_EXT = .so
gnu_linux : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux
gnu_linux : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings           -D NDEBUG $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_linux : C_STANDARD    = -std=gnu99
gnu_linux : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
gnu_linux : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_linux : LFLAGS   += -L$(FREEGLUT_DIR)/lib
gnu_linux : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
endif
gnu_linux : CC        = gcc
gnu_linux : CPP       = g++
gnu_linux : exe       = smokeview_linux$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

gnu_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- clang_linux_db ----------------

clang_linux_db : LIB_DIR_PLAT = $(LIB_DIR)/clang_linux
clang_linux_db : CFLAGS    = -O0  -m64 -ggdb -Wall -Wunreachable-code -Wswitch-default -Wshadow -Winit-self -Wundef -Wpointer-arith -Wuninitialized -Wno-address -Wno-undef -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
clang_linux_db : C_STANDARD    = -std=gnu99
clang_linux_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
clang_linux_db : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
clang_linux_db : LFLAGS   += -L$(FREEGLUT_DIR)/lib
clang_linux_db : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
endif
clang_linux_db : CC        = clang
clang_linux_db : CPP       = clang++
clang_linux_db : exe       = smokeview_linux$(SMV_TESTSTRING)_db$(SMV_PROFILESTRING)

clang_linux_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- clang_linux ----------------

clang_linux : LIB_DIR_PLAT = $(LIB_DIR)/clang_linux
clang_linux : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings          $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
clang_linux : C_STANDARD    = -std=gnu99
clang_linux : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
clang_linux : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
clang_linux : LFLAGS   += -L$(FREEGLUT_DIR)/lib
clang_linux : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
endif
clang_linux : CC        = clang
clang_linux : CPP       = clang++
clang_linux : exe       = smokeview_linux$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

clang_linux : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   check source   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# ------------- gnu_linux_check ----------------

gnu_linux_check : CFLAGS    = -O2 -m64 -Wall
gnu_linux_check : $(src)
	clang-tidy $^ --  $(CFLAGS) $(INC)

# ------------- linux_osx_check ----------------

linux_osx_check : CFLAGS    = $(SMV_TESTFLAG)
linux_osx_check : $(linux_osx_chk)
	$(CLANGSUMMARY) smokeview.summary

# ------------- win_check ----------------

win_check : CFLAGS    = -D NDEBUG -D _CONSOLE -D GLEW_STATIC -D PTW32_STATIC_LIB
win_check : INC += $(WININC)
win_check : $(win_chk)
	$(CLANGWINSUMMARY) smokeview.summary

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   OSX   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SMV_LIBS_OSX =  -lglui -lgd -ljpeg -lpng -lz

# ------------- gnu_osx_db ----------------

gnu_osx_db : CFLAGS    = -O0 -ggdb -m64 -D _DEBUG $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx_db : C_STANDARD    = -std=gnu99
gnu_osx_db : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_osx_db : LFLAGS   += -framework CoreVideo -framework Cocoa -framework OpenGL
gnu_osx_db : LFLAGS   += -L$(FREEGLUT_DIR)/lib -lglut
gnu_osx_db : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
else
gnu_osx_db : LFLAGS   += -framework OpenGL -framework GLUT
endif
gnu_osx_db : CC        = ${COMPILER}
gnu_osx_db : CPP       = ${COMPILER2}
gnu_osx_db : exe       = smokeview_osx$(SMV_TESTSTRING)_db$(SMV_PROFILESTRING)

gnu_osx_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu_osx ----------------

gnu_osx : CFLAGS    = -O0 -m64              -D NDEBUG $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx : C_STANDARD    = -std=gnu99
gnu_osx : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_osx : LFLAGS   += -framework CoreVideo -framework Cocoa -framework OpenGL
gnu_osx : LFLAGS   += -L$(FREEGLUT_DIR)/lib -lglut
gnu_osx : CFLAGS   += -D pp_FREEGLUT -I$(FREEGLUT_DIR)/include
else
gnu_osx : LFLAGS   += -framework OpenGL -framework GLUT
endif
gnu_osx : CC        = ${COMPILER}
gnu_osx : CPP       = ${COMPILER2}
gnu_osx : exe       = smokeview_osx$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

gnu_osx : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang_osx_db ----------------

clang_osx_db : CFLAGS    = -O0 -ggdb -m64 -D _DEBUG $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
clang_osx_db : C_STANDARD    = -std=gnu99
clang_osx_db : LFLAGS    = -m64 -L$(LIB_DIR)/clang_osx $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
clang_osx_db : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
clang_osx_db : CFLAGS   += -D FREEGLUT_STATIC
else
clang_osx_db : LFLAGS   += -framework OpenGL -framework GLUT
endif
clang_osx_db : CC        = ${COMPILER}
clang_osx_db : CPP       = ${COMPILER2}
clang_osx_db : exe       = smokeview_osx$(SMV_TESTSTRING)_db$(SMV_PROFILESTRING)

clang_osx_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- clang_osx ----------------

clang_osx : CFLAGS    = -O0 -m64 -D NDEBUG $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
clang_osx : C_STANDARD    = -std=gnu99
clang_osx : LFLAGS    = -m64 -L$(LIB_DIR)/clang_osx $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
clang_osx : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
clang_osx : CFLAGS   += -D FREEGLUT_STATIC
else
clang_osx : LFLAGS   += -framework OpenGL -framework GLUT
endif
clang_osx : CC        = ${COMPILER}
clang_osx : CPP       = ${COMPILER2}
clang_osx : exe       = smokeview_osx$(SMV_TESTSTRING)$(SMV_PROFILESTRING)

clang_osx : $(obj)
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
