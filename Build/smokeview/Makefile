# Smokeview makefile

#To use this Makefile cd to a sub-directory and type make_smv.sh or make_smv.bat

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS

SMV_TESTFLAG =
SMV_TESTSTRING =
SMV_PROFILEFLAG =
SMV_PROFILESTRING =

# Intel compiler definitions
include ../../scripts/intel_compiler.mak

COMP_VERSION =
LUA_SCRIPTING =
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

csrc = callbacks.c camera.c color2rgb.c readimage.c readgeom.c readobject.c readcad.c colortimebar.c compress.c csphere.c dmalloc.c \
      drawGeometry.c file_util.c getdata.c getdatabounds.c getdatacolors.c glew.c  \
      histogram.c infoheader.c readlabel.c readtour.c readhvac.c readslice.c readsmoke.c colorbars.c colorbar_defs.c \
      IOboundary.c IOgeometry.c IOhvac.c IOiso.c IOobjects.c IOpart.c IOplot2d.c IOplot3d.c \
      IOscript.c IOshooter.c IOslice.c IOsmoke.c IOtour.c IOvolsmoke.c IOwui.c IOzone.c IOframe.c isobox.c \
      main.c md5.c menus.c output.c readsmv.c renderhtml.c renderimage.c scontour2d.c sha1.c \
      sha256.c shaders.c showscene.c skybox.c smokestream.c smokeview.c smv_geometry.c startup.c fopen.c \
      stdio_buffer.c stdio_m.c string_util.c threader.c translate.c unit.c update.c viewports.c colortable.c command_args.c

cppsrc = glui_bounds.cpp \
     glui_clip.cpp glui_colorbar.cpp glui_display.cpp glui_geometry.cpp glui_motion.cpp glui_objects.cpp \
      glui_shooter.cpp glui_smoke.cpp glui_stereo.cpp glui_tour.cpp glui_trainer.cpp

src = $(csrc) $(cppsrc)

cobj = $(csrc:.c=.o)
cppobj = $(cppsrc:.cpp=.o)
obj = $(cobj) $(cppobj)

# setup for analyzing smokeview source code using clang

cclang   = $(csrc:.c=.chk)
cppclang = $(cppsrc:.cpp=.chk)
clangchk = $(cclang) $(cppclang)

wincclang   = $(csrc:.c=.winchk)
wincppclang = $(cppsrc:.cpp=.winchk)
winclangchk = $(wincclang) $(wincppclang)

#CLANGANALYZE=clang --analyze -Xanalyzer -analyzer-checker=core,deadcode
CLANGANALYZE=clang --analyze
CLANGSUMMARY=../../scripts/clang_summary.sh
CLANGWINSUMMARY=../../scripts/clang_summary.bat

incs = compress.h scontour2d.h csphere.h datadefs.h dirent_win.h file_util.h glui_bounds.h \
       glui_motion.h glui_smoke.h glui_tour.h histogram.h infoheader.h interp.h isobox.h \
       IOobjects.h IOscript.h IOvolsmoke.h lint.h lua_api.h dmalloc.h options.h options_common.h \
       pragmas.h smokeheaders.h smokeviewdefs.h smokeviewvars.h \
       stdio_buffer.h stdio_m.h string_util.h structures.h translate.h threader.h viewports.h command_args.h

ifeq ($(ICON),icon)
obj += resource.o
endif

ifeq ($(LUA_SCRIPTING),true)
obj += c_api.o lua_api.o
incs += c_api.h
endif

objwin = $(obj:.o=.obj)

INC += -I $(SOURCE_DIR)/glui_v2_1_beta -I $(SOURCE_DIR)/gd-2.0.15 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview -I $(SOURCE_DIR)/glew -I $(SOURCE_DIR)/zlib128

# windows include directories

ifeq ($(GLUT),freeglut)
  WININC = -I $(SOURCE_DIR)/freeglut3.0.0/include
else
  WININC = -I $(SOURCE_DIR)/glut_gl
endif
WININC += -I $(SOURCE_DIR)/pthreads

ifeq ($(LUA_SCRIPTING),true)
INC += -I $(SOURCE_DIR)/lua-5.3.1/src
endif

SMVLUACORE_DIR = $(SOURCE_DIR)/smvluacore
SMVLUACORE_FILES = $(SMVLUACORE_DIR)/smv.lua $(SMVLUACORE_DIR)/ssf.lua \
	$(SMVLUACORE_DIR)/ssfparser.lua $(SMVLUACORE_DIR)/ssfcommands.lua \
	$(SMVLUACORE_DIR)/clipping.lua $(SMVLUACORE_DIR)/bounds.lua \
	$(SMVLUACORE_DIR)/render.lua $(SMVLUACORE_DIR)/load.lua \
	$(SMVLUACORE_DIR)/view.lua $(SMVLUACORE_DIR)/tour.lua \
	$(SMVLUACORE_DIR)/iniparser.lua $(SMVLUACORE_DIR)/inioptions.lua \
	$(SMVLUACORE_DIR)/unload.lua $(SMVLUACORE_DIR)/constants.lua \
	$(SMVLUACORE_DIR)/gnuplot.lua $(SMVLUACORE_DIR)/plot.lua \
	$(SMVLUACORE_DIR)/pl3d.lua \
	$(SMVLUACORE_DIR)/camera.lua
SMVLUALIB_FILES = $(LIB_DIR_PLAT)/lfs$(DYNLIB_EXT)

smvluacore: $(SMVLUACORE_FILES) $(SMVLUALIB_FILES)
	cp $(SMVLUACORE_FILES) .
	cp $(SMVLUALIB_FILES) .
# cannot use $^ here

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

#--- Intel

SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win_64
ifeq ($(GLUT),freeglut)
  GLUT_WIN_LIB = $(SMV_WIN_LIBDIR)\freeglut_staticd.lib $(SMV_WIN_LIBDIR)\glut32.lib
else
  GLUT_WIN_LIB = $(SMV_WIN_LIBDIR)\glut32.lib
endif
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\glui.lib $(GLUT_WIN_LIB) $(SMV_WIN_LIBDIR)\gd.lib $(SMV_WIN_LIBDIR)\jpeg.lib $(SMV_WIN_LIBDIR)\png.lib $(SMV_WIN_LIBDIR)\zlib.lib $(SMV_WIN_LIBDIR)\pthreads.lib

#--- gnu

SMV_GNU_LIBDIR = $(LIB_DIR)\gnu_win_64
ifeq ($(GLUT),freeglut)
  GLUT_GNU_LIB = $(SMV_GNU_LIBDIR)\freeglut_staticd.a
  D_FREEGLUT = -D FREEGLUT_STATIC
else
  GLUT_GNU_LIB = $(SMV_GNU_LIBDIR)\glut32.a
  D_FREEGLUT =
endif
SMV_LIBS_GNU = $(SMV_GNU_LIBDIR)\glui.a $(GLUT_GNU_LIB) $(SMV_GNU_LIBDIR)\gd.a $(SMV_GNU_LIBDIR)\jpeg.a $(SMV_GNU_LIBDIR)\png.a  $(SMV_GNU_LIBDIR)\zlib.a $(SMV_GNU_LIBDIR)\pthreads.a

WIN32_LIBS = user32.lib gdi32.lib comdlg32.lib shell32.lib

# ------------- intel_win_64_db ----------------

intel_win_64_db : DYNLIB_EXT = .dll
intel_win_64_db : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
intel_win_64_db : CFLAGS    = /Od /W4 /Wno-cast-function-type-mismatch /Wno-unused-function /Wno-unused-parameter /debug:full /Zi  -D pp_INTEL -D _DEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(SANITIZE),1)
intel_win_64_db : CFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_win_64_db : C_STANDARD    = -Qstd=gnu99

ifeq ($(GLUT),freeglut)
intel_win_64_db : CFLAGS   += -D FREEGLUT_STATIC
endif
intel_win_64_db : LFLAGS    = /F32768000 /traceback /Zi /DEBUG
ifeq ($(SANITIZE),1)
intel_win_64_db : LFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_win_64_db : CC        = icx
intel_win_64_db : CPP       = icx

intel_win_64_db : exe       = smokeview_win_$(SMV_TESTSTRING)64_db

intel_win_64_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN)

# ------------- intel_win_64 ----------------

intel_win_64 : DYNLIB_EXT = .dll
intel_win_64 : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
intel_win_64 : CFLAGS    = -O1 -D pp_INTEL -D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(SANITIZE),1)
intel_win_64 : CFLAGS   += $(SANITIZE_OPTIONS) -g
endif
intel_win_64 : C_STANDARD    = -Qstd=gnu99
ifeq ($(GLUT),freeglut)
intel_win_64 : CFLAGS   += -D FREEGLUT_STATIC
endif
ifeq ($(LUA_SCRIPTING),true)
intel_win_64 : CFLAGS    += -D pp_LUA
intel_win_64 : SMVLUACORE_FILES += $(SMV_WIN_LIBDIR)\lpeg.dll
intel_win_64 : SMVLUACORE_FILES += $(SMV_WIN_LIBDIR)\lua.dll
endif
intel_win_64 : LFLAGS    = /F32768000 -D FREEGLUT_STATIC
ifeq ($(SANITIZE),1)
intel_win_64 : LFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_win_64 : CC        = icx
intel_win_64 : CPP       = icx
intel_win_64 : exe       = smokeview_win_$(SMV_TESTSTRING)64

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN)

# ------------- gnu_win_64 ----------------

gnu_win_64 : INC       += -I $(SOURCE_DIR)/glui_gl -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
gnu_win_64 : CFLAGS    = -O0 -m64 $(D_FREEGLUT) -D GLEW_STATIC -D WIN32 -D pp_GCC -D NDEBUG -D __MINGW32__ -fno-builtin -Wno-write-strings $(GITINFO) $(GNU_COMPINFO) $(SMV_PROFILEFLAG)
gnu_win_64 : LFLAGS    = -m64 $(SMV_LIBS_GNU) -lm -lopengl32 -lglu32 -lgdi32 -lwinmm -lcomdlg32 -lhid -lquadmath $(SMV_PROFILEFLAG)
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = g++
gnu_win_64 : exe       = smokeview_win_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

gnu_win_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# ------------- gnu_win_64_db ----------------

#-Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wunused-parameter -Wunused-but-set-parameter

# to profile the windows build of smokeview
# 1.  cd smv/Build/smokeview/gnu_win_64
# 2.  type: make_smokeview
# 2.  run case using smokeview_win_test_db
# 3.  type: gprof smokeview_application_name > results.out

gnu_win_64_db : INC       += -I $(SOURCE_DIR)/glui_gl -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
gnu_win_64_db : CFLAGS    = -O0 -m64 -g -Wunused-label -Wunused-value -Wunused-variable $(D_FREEGLUT) $(SMV_PROFILEFLAG) -D GLEW_STATIC -D WIN32 -D pp_GCC -D __MINGW32__ -fno-builtin -Wno-write-strings $(GITINFO) $(GNU_COMPINFO)
gnu_win_64_db : LFLAGS    = -m64 $(SMV_LIBS_GNU) -lm -lopengl32 -lglu32 -lgdi32 -lwinmm -lcomdlg32 -lhid -lquadmath $(SMV_PROFILEFLAG)
gnu_win_64_db : CC        = gcc
gnu_win_64_db : CPP       = g++
gnu_win_64_db : exe       = smokeview_win_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

gnu_win_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SMV_LIBS_LINUX= -lglui -lglut -lgd -ljpeg -lpng -lz
SYSTEM_LIBS_LINUX = -lpthread -lX11 -lXmu -lGLU -lGL -lm -lrt

# to profile smokeview on linux
# 1.  cd smv/Build/smokeview/intel_linux_64
# 1.  type: ./make_smv.sh -p
# 2.  run case as usual
# 3.  type: gprof smokeview_application_name > results.out

# ------------- intel_linux_64 ----------------

intel_linux_64 : DYNLIB_EXT = .so
intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux_64 : CFLAGS    = -O0 -traceback -m64 -D pp_LINUX -D NDEBUG $(SMV_TESTFLAG) $(SMV_PROFILEFLAG) $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(SANITIZE),1)
intel_linux_64 : CFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_linux_64 : C_STANDARD    = -std=gnu99
intel_linux_64 : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_linux_64 : CFLAGS    += -D pp_LUA
intel_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
intel_linux_64 : LIBLUA    += $(LIB_DIR_PLAT)/liblua.a -ldl
endif
intel_linux_64 : LFLAGS    = -m64 $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
ifeq ($(SANITIZE),1)
intel_linux_64 : LFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_linux_64 : CC        = icx
intel_linux_64 : CPP       = icpx
intel_linux_64 : exe       = smokeview_linux_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

intel_linux_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX) $(LIBLUA)

# ------------- intel_linux_64_db ----------------

intel_linux_64_db : CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux_64_db : CFLAGS    = -O0 -g -m64 $(SMV_TESTFLAG) -D _DEBUG -D pp_LINUX $(SMV_PROFILEFLAG) \
	                        -fstack-protector -fno-omit-frame-pointer -traceback -debug variable-locations \
                                -Wall -Wextra -Wuninitialized -Wunused-function -Wunused-variable -Wno-sign-compare \
                                -Wno-unused-parameter $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(SANITIZE),1)
intel_linux_64_db : CFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_linux_64_db : C_STANDARD    = -std=gnu99
intel_linux_64_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
intel_linux_64_db : LFLAGS    = -m64 -static-intel $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
ifeq ($(SANITIZE),1)
intel_linux_64_db : LFLAGS   += $(SANITIZE_OPTIONS)
endif
intel_linux_64_db : CC        = icx
intel_linux_64_db : CPP       = icpx
intel_linux_64_db : exe       = smokeview_linux_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

intel_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/intel_linux_64 $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux_64_db ----------------

gnu_linux_64_db : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64_db : CFLAGS    = -O0  -m64 -ggdb -Wall -Wunreachable-code -Wswitch-default -Wshadow -Winit-self -Wundef -Wpointer-arith -Wuninitialized -Wno-address -Wno-format-overflow -Wno-format-truncation -Wno-undef -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_linux_64_db : C_STANDARD    = -std=gnu99
gnu_linux_64_db : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
ifeq ($(LUA_SCRIPTING),true)
gnu_linux_64_db : CFLAGS    += -D pp_LUA
gnu_linux_64_db : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
endif
gnu_linux_64_db : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : CPP       = g++
gnu_linux_64_db : exe       = smokeview_linux_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

gnu_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX) \
		$(if $(LUA_SCRIPTING),$(LIB_DIR_PLAT)/liblua.a -ldl)

# ------------- gnu_linux_64 ----------------

gnu_linux_64 : DYNLIB_EXT = .so
gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings           -D pp_LINUX -D pp_GCC -D NDEBUG $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_linux_64 : C_STANDARD    = -std=gnu99
gnu_linux_64 : INC += -I $(SOURCE_DIR)/freeglut3.0.0/include
ifeq ($(LUA_SCRIPTING),true)
gnu_linux_64 : CFLAGS    += -D pp_LUA
gnu_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
gnu_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lua.so
endif
gnu_linux_64 : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : CPP       = g++
gnu_linux_64 : exe       = smokeview_linux_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

gnu_linux_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) $(SYSTEM_LIBS_LINUX) \
		$(if $(LUA_SCRIPTING),$(LIB_DIR_PLAT)/liblua.a -ldl)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   check source   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# ------------- gnu_linux_64_check ----------------

gnu_linux_64_check : CFLAGS    = -O2 -m64 -Wall
gnu_linux_64_check : $(src)
	clang-tidy $^ --  $(CFLAGS) $(INC)

# ------------- linux_osx_check ----------------

linux_osx_check : CFLAGS    =
linux_osx_check : $(clangchk)
	$(CLANGSUMMARY) smokeview.summary

# ------------- win_check ----------------
# (not working yet)

win_check : CFLAGS    = -D NDEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB 
win_check : INC += $(WININC)
win_check : $(winclangchk)
	$(CLANGWINSUMMARY) smokeview.summary

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   OSX   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

ifeq ($(GLUT),freeglut)
  SMV_LIBS_OSX =  -lglui  -lglut -lgd -ljpeg -lpng -lz
else
  ifeq ($(QUARTZ),framework)
    SMV_LIBS_OSX = -lglui        -lgd -ljpeg -lpng -lz
  else
    SMV_LIBS_OSX = -lglui -lglut -lgd -ljpeg -lpng -lz
  endif
endif

# ------------- intel_osx_64 ----------------

intel_osx_64 : CFLAGS    = -O0 -m64 -I $(SOURCE_DIR)/glui_gl -traceback -static-intel -D pp_OSX -D NDEBUG $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO) -diag-disable=10441
intel_osx_64 : C_STANDARD    = -std=gnu99
intel_osx_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_osx_64 : CFLAGS    += -D pp_LUA
intel_osx_64 : SMVLUACORE_FILES += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/lpeg.so
intel_osx_64 : LIBLUA    += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/liblua.a -ldl
endif
intel_osx_64 : LFLAGS    = -m64 -L$(LIB_DIR)/intel_osx_$(NOQUARTZ)64 $(SMV_LIBS_OSX) -traceback -static-intel  -diag-disable=10441
ifeq ($(GLUT),freeglut)
intel_osx_64 : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
else
ifeq ($(QUARTZ),framework)
intel_osx_64 : LFLAGS   += -framework OpenGL -framework GLUT
else
intel_osx_64 : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
endif
endif
intel_osx_64 : CC        = icc
intel_osx_64 : CPP       = icpc
intel_osx_64 : exe       = smokeview_osx_$(SMV_TESTSTRING)$(NOQUARTZ)64

intel_osx_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	icpc -o $(bin)/$(exe) $(obj) $(LFLAGS) $(INTEL_LIBS_OSX) $(LIBLUA)

# ------------- intel_osx_64_db ----------------

intel_osx_64_db : CFLAGS    = -O0 -g -m64 -I $(SOURCE_DIR)/glui_gl -traceback -static-intel -D _DEBUG -D pp_OSX $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO) -diag-disable=10441
intel_osx_64_db : C_STANDARD    = -std=gnu99
intel_osx_64_db : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_osx_64_db : CFLAGS    += -D pp_LUA
intel_osx_64_db : SMVLUACORE_FILES += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/lpeg.so
intel_osx_64_db : LIBLUA    += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/liblua.a -ldl
endif
# remove following from LFLAGS line
intel_osx_64_db : LFLAGS    = -m64 -L$(LIB_DIR)/intel_osx_$(NOQUARTZ)64 $(SMV_LIBS_OSX) -traceback -static-intel -diag-disable=10441
ifeq ($(GLUT),freeglut)
intel_osx_64_db : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
else
ifeq ($(QUARTZ),framework)
intel_osx_64_db : LFLAGS   += -framework OpenGL -framework GLUT
else
intel_osx_64_db : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
endif
endif
intel_osx_64_db : CC        = icc
intel_osx_64_db : CPP       = icpc
intel_osx_64_db : exe       = smokeview_osx_$(SMV_TESTSTRING)$(NOQUARTZ)64_db

intel_osx_64_db : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	icpc -o $(bin)/$(exe) $(obj) $(LFLAGS) $(INTEL_LIBS_OSX) $(LIBLUA)

# ------------- gnu_osx_64_db ----------------

gnu_osx_64_db : CFLAGS    = -O0 -ggdb -m64 -D _DEBUG -D pp_OSX -D pp_GCC $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx_64_db : C_STANDARD    = -std=gnu99
gnu_osx_64_db : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) $(GLIBDIROPT) $(SMV_PROFILEFLAG)
ifeq ($(GLUT),freeglut)
gnu_osx_64_db : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
gnu_osx_64_db : CFLAGS   += -D FREEGLUT_STATIC
else
ifeq ($(QUARTZ),framework)
gnu_osx_64_db : LFLAGS   += -framework OpenGL -framework GLUT
else
gnu_osx_64_db : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
endif
endif
gnu_osx_64_db : CC        = gcc$(GLIBVERS)
gnu_osx_64_db : CPP       = g++$(GLIBVERS)
gnu_osx_64_db : exe       = smokeview_osx_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

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
ifeq ($(QUARTZ),framework)
gnu_osx_64 : LFLAGS   += -framework OpenGL -framework GLUT
else
gnu_osx_64 : LFLAGS   += -L /opt/X11/lib -lX11 -lXmu -lGLU -lGL
endif
endif
gnu_osx_64 : CC        = gcc$(GLIBVERS)
gnu_osx_64 : CPP       = g++$(GLIBVERS)
gnu_osx_64 : exe       = smokeview_osx_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS)

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	rm -f *.o *.mod *.lua *.dll *.so

#-------------- force compilation of string_util.c -----------

string_util.o:	.FORCE
string_util.obj:	.FORCE

.FORCE:

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)
