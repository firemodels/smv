
# Smokeview makefile

#To use this Makefile cd to a sub-directory and type make_smv.sh or make_smv.bat

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS

# Set the C standard we are using. Currently this is just the appropriate flag
# for gcc and is not applied to other compilers.
#C_STANDARD = -std=gnu89
#CPP_STANDARD = -libstd=c++

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

ifeq ($(shell echo "check_quotes"),"check_quotes")
  INTEL_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\intel_compversion)
  GNU_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\gnu_compversion)
else
  INTEL_COMPVERSION := $(shell ../../../Utilities/Scripts/intel_compversion.sh)
  GNU_COMPVERSION := $(shell ../../../Utilities/Scripts/gnu_compversion.sh)
endif
INTEL_COMPINFO=-Dpp_COMPVER=\"$(INTEL_COMPVERSION)\"
GNU_COMPINFO=-Dpp_COMPVER=\"$(GNU_COMPVERSION)\"

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

obj = main.o menus.o IOscript.o IOshooter.o glui_objects.o glui_shooter.o\
      csphere.o colortimebar.o smv_endian.o camera.o IOgeometry.o IOwui.o IOobjects.o\
      IOtour.o glui_display.o glui_tour.o getdatacolors.o smokeview.o output.o renderimage.o\
      renderhtml.o isobox.o getdatabounds.o readsmv.o scontour2d.o glui_smoke.o glui_clip.o\
      glui_stereo.o glui_geometry.o glui_motion.o glui_bounds.o dmalloc.o assert.o\
      compress.o IOvolsmoke.o IOsmoke.o IOplot3d.o IOslice.o IOboundary.o IOpart.o IOzone.o\
      IOiso.o callbacks.o drawGeometry.o glui_colorbar.o skybox.o file_util.o string_util.o\
      startup.o glui_trainer.o shaders.o unit.o threader.o histogram.o translate.o update.o\
      viewports.o smv_geometry.o showscene.o glew.o infoheader.o  md5.o sha1.o sha256.o\
      stdio_m.o stdio_buffer.o \
      fdsmodules.o gsmv.o getdata.o

incs = contourdefs.h glui_bounds.h glui_motion.h glui_smoke.h glui_tour.h glui_wui.h infoheader.h \
       IOobjects.h IOscript.h IOvolsmoke.h lua_api.h options.h smokefortheaders.h smokeheaders.h smokeviewdefs.h \
       smokeviewvars.h structures.h threader.h viewports.h ASSERT.h csphere.h dirent_win.h histogram.h isodefs.h \
       MALLOCC.h pragmas.h stdio_buffer.h string_util.h compress.h datadefs.h file_util.h interp.h lint.h options_common.h \
       smv_endian.h stdio_m.h translate.h

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
	$(SMVLUACORE_DIR)/camera.lua $(SMVLUACORE_DIR)/window.lua

smvluacore: $(SMVLUACORE_FILES)
	cp $(SMVLUACORE_FILES) .
# cannot use $^ here

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .F90 .c .obj .o .f90 .cpp

.c.obj:
	$(CC) -c $(CFLAGS) $(INC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(C_STANDARD) $(INC) $<

.cpp.obj:
	$(CPP) -c $(CFLAGS) $(INC) $<
.cpp.o:
	$(CPP) -c $(CFLAGS) $(CPP_STANDARD) $(INC) $<

.F90.o:
	$(FC) -c $(FFLAGS) $<
.f90.obj:
	$(FC) -c $(FFLAGS) $<
.f90.o:
	$(FC) -c $(FFLAGS) $<

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

# note: use /fpscomp:general option to allow smokeview to read files being currently written to by FDS
#       (instead of the non-standard SHARED keyword in a FORTRAN OPEN statement)

# ------------- intel_win_64_db ----------------

intel_win_64_db : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
intel_win_64_db : FFLAGS    = /Od /iface:stdref /fpp -D WIN32 /Zi /nologo /fpscomp:general /debug:full /extend_source:132 /warn:unused /warn:nointerfaces /Qtrapuv /fp:strict /fp:except /traceback /check:all /stand:f18
intel_win_64_db : CFLAGS    = /Od /W4 /debug:full /Zi -D pp_INTEL -D pp_WIN_CLANG -D _DEBUG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(GLUT),freeglut)
intel_win_64_db : CFLAGS   += -D FREEGLUT_STATIC
endif
intel_win_64_db : LFLAGS    = /F32768000 /traceback /Zi /DEBUG
intel_win_64_db : CC        = $(I_CC)
intel_win_64_db : CPP       = $(I_CC)
intel_win_64_db : FC        = $(I_IFORT)
intel_win_64_db : exe       = smokeview_win_$(SMV_TESTSTRING)64_db

intel_win_64_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN)

# ------------- intel_win_64 ----------------

intel_win_64 : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
ifeq ($(LUA_SCRIPTING),true)
intel_win_64 : INC += -I $(SOURCE_DIR)/lua-5.3.1/src
endif
intel_win_64 : FFLAGS    = -O2 /iface:stdref /fpp -D WIN32 /fpscomp:general /stand:f18
intel_win_64 : CFLAGS    = -O1 -D pp_INTEL -D pp_WIN_CLANG -D WIN32 -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
ifeq ($(GLUT),freeglut)
intel_win_64 : CFLAGS   += -D FREEGLUT_STATIC
endif
intel_win_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_win_64 : CFLAGS    += -D pp_LUA
intel_win_64 : SMV_LIBS_WIN += $(WIN_LIBDIR)\liblua.lib
intel_win_64 : SMV_LIBS_WIN += $(WIN_LIBDIR)\lpeg.lib
intel_win_64 : SMVLUACORE_FILES += $(WIN_LIBDIR)/lpeg.dll
endif
intel_win_64 : LFLAGS    = /F32768000 -D FREEGLUT_STATIC
intel_win_64 : CC        = $(I_CC)
intel_win_64 : CPP       = $(I_CC)
intel_win_64 : FC        = $(I_IFORT)
intel_win_64 : exe       = smokeview_win_$(SMV_TESTSTRING)64

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN) $(LIBLUA)

# ------------- gnu_win_64 ----------------

gnu_win_64 : INC       += -I $(SOURCE_DIR)/glui_gl -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
gnu_win_64 : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC -ffree-form -frecord-marker=4 $(SMV_PROFILEFLAG)
gnu_win_64 : CFLAGS    = -O0 -m64 $(D_FREEGLUT) -D GLEW_STATIC -D WIN32 -D pp_GCC -D __MINGW32__ -fno-builtin -Wno-write-strings $(GITINFO) $(GNU_COMPINFO) $(SMV_PROFILEFLAG)
gnu_win_64 : LFLAGS    = -m64 $(SMV_LIBS_GNU) -lgfortran -lm -lopengl32 -lglu32 -lgdi32 -lwinmm -lcomdlg32 -lhid -lquadmath $(SMV_PROFILEFLAG)
gnu_win_64 : CC        = gcc
gnu_win_64 : CPP       = g++
gnu_win_64 : FC        = gfortran
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
gnu_win_64_db : FFLAGS    = -O0 -m64 -x f95-cpp-input -D _DEBUG -D pp_GCC -ffree-form -frecord-marker=4 $(SMV_PROFILEFLAG) 
gnu_win_64_db : CFLAGS    = -O0 -m64 -g -Wunused-label -Wunused-value -Wunused-variable $(D_FREEGLUT) $(SMV_PROFILEFLAG) -D GLEW_STATIC -D WIN32 -D pp_GCC -D __MINGW32__ -fno-builtin -Wno-write-strings $(GITINFO) $(GNU_COMPINFO)
gnu_win_64_db : LFLAGS    = -m64 $(SMV_LIBS_GNU) -lgfortran -lm -lopengl32 -lglu32 -lgdi32 -lwinmm -lcomdlg32 -lhid -lquadmath $(SMV_PROFILEFLAG)
gnu_win_64_db : CC        = gcc
gnu_win_64_db : CPP       = g++
gnu_win_64_db : FC        = gfortran
gnu_win_64_db : exe       = smokeview_win_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

gnu_win_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) 

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SMV_LIBS_LINUX= -lglui -lglut -lgd -ljpeg -lpng -lz
SYSTEM_LIBS_LINUX = -lpthread -lX11 -lXmu -lGLU -lGL -lm -lrt
INTEL_LIBS_LINUX =$(IFORT_COMPILER_LIB)/libifcore.a $(IFORT_COMPILER_LIB)/libifport.a

# to profile smokeview on linux
# 1.  cd smv/Build/smokeview/intel_linux_64
# 1.  type: ./make_smv.sh -p
# 2.  run case as usual
# 3.  type: gprof smokeview_application_name > results.out

# ------------- intel_linux_64 ----------------

intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : FFLAGS    = -O0 -traceback -m64 -fpp -stand:f18 $(SMV_PROFILEFLAG)
intel_linux_64 : CFLAGS    = -O0 -traceback -m64 -D pp_LINUX $(SMV_TESTFLAG) $(SMV_PROFILEFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_linux_64 : CFLAGS    += -D pp_LUA
intel_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
intel_linux_64 : LIBLUA    += $(LIB_DIR_PLAT)/liblua.a -ldl
endif
intel_linux_64 : LFLAGS    = -m64 $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
intel_linux_64 : CC        = $(I_ICC)
intel_linux_64 : CPP       = $(I_ICPP)
intel_linux_64 : FC        = $(I_IFORT)
intel_linux_64 : exe       = smokeview_linux_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

intel_linux_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX) $(LIBLUA)

# ------------- intel_linux_64_db ----------------

intel_linux_64_db : FFLAGS    = -O0 -m64 -static-intel -g -fpe0 -fltconsistency -WB -fpp -stand:f18 $(SMV_PROFILEFLAG)
intel_linux_64_db : CFLAGS    = -O0 -g -m64 $(SMV_TESTFLAG) -D _DEBUG -D pp_LINUX $(SMV_PROFILEFLAG) \
                                -Wall -Wextra -Wuninitialized -Wunused-function -Wunused-variable -Wno-sign-compare \
                                -Wno-unused-parameter $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64_db : LFLAGS    = -m64 -static-intel $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
intel_linux_64_db : CC        = $(I_ICC)
intel_linux_64_db : CPP       = $(I_ICPP)
intel_linux_64_db : FC        = $(I_IFORT)
intel_linux_64_db : exe       = smokeview_linux_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

intel_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/intel_linux_64 $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux_64_db ----------------

gnu_linux_64_db : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64_db : FFLAGS    = -O0 -m64 -ggdb -Wall -x f95-cpp-input -D pp_GCC -ffree-form -frecord-marker=4 -fcheck=all -fbacktrace $(SMV_PROFILEFLAG)
gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wunreachable-code -Wswitch-default -Wshadow -Winit-self -Wundef -Wpointer-arith -Wuninitialized -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
ifeq ($(LUA_SCRIPTING),true)
gnu_linux_64_db : CFLAGS    += -D pp_LUA
gnu_linux_64_db : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
endif
gnu_linux_64_db : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : CPP       = g++
gnu_linux_64_db : FC        = gfortran
gnu_linux_64_db : exe       = smokeview_linux_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

gnu_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) -lgfortran $(SYSTEM_LIBS_LINUX) \
		$(if $(LUA_SCRIPTING),$(LIB_DIR_PLAT)/liblua.a -ldl)

# ------------- gnu_linux_64 ----------------

gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : FFLAGS    = -O0 -m64 -ggdb -Wall -x f95-cpp-input -D pp_GCC -ffree-form -frecord-marker=4 -fcheck=all -fbacktrace $(SMV_PROFILEFLAG)
gnu_linux_64 : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings           -D pp_LINUX -D pp_GCC $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
ifeq ($(LUA_SCRIPTING),true)
gnu_linux_64 : CFLAGS    += -D pp_LUA
gnu_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
endif
gnu_linux_64 : LFLAGS    = -m64 $(SMV_PROFILEFLAG)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : CPP       = g++
gnu_linux_64 : FC        = gfortran
gnu_linux_64 : exe       = smokeview_linux_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

gnu_linux_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) -lgfortran $(SYSTEM_LIBS_LINUX) \
		$(if $(LUA_SCRIPTING),$(LIB_DIR_PLAT)/liblua.a -ldl)

# ------------- mingw_win_64 ----------------

mingw_win_64 : LIB_DIR_PLAT = $(LIB_DIR)/mingw_win_64
mingw_win_64 : LIBS_PLAT = $(LIB_DIR_PLAT)/libglui.a \
	$(LIB_DIR_PLAT)/libgd.a $(LIB_DIR_PLAT)/libjpeg.a \
	$(LIB_DIR_PLAT)/libpng.a $(LIB_DIR_PLAT)/libz.a \
	$(LIB_DIR_PLAT)/libglutwin.a
mingw_win_64 : INC += -I $(SOURCE_DIR)/GLINC-mingw -I $(SOURCE_DIR)/pthreads -I $(SOURCE_DIR)/GLUTINC/glut
mingw_win_64 : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC \
						       -ffree-form -frecord-marker=4
mingw_win_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D GLEW_STATIC -D MINGW -Wno-write-strings
ifeq ($(LUA_SCRIPTING),true)
mingw_win_64 : CFLAGS    += -D pp_LUA
mingw_win_64 : LIBS_PLAT += $(LIB_DIR_PLAT)/lua53.dll
mingw_win_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.dll
endif
mingw_win_64 : LFLAGS    = -m64 -static-libgcc -static-libstdc++
mingw_win_64 : CC        = gcc
mingw_win_64 : CPP       = g++
mingw_win_64 : FC        = gfortran
mingw_win_64 : exe       = smokeview_mingw_$(SMV_TESTSTRING)64

mingw_win_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(($(LUA_SCRIPTING),true),-I $(SOURCE_DIR)/lua-5.3.1/src) \
		$(LIBS_PLAT) -lm -lopengl32 -lglu32 \
		-Wl,-Bstatic  -lgfortran -lquadmath  \
		-Wl,-Bdynamic -lgdi32 -lwinmm -lcomdlg32

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
INTEL_LIBS_OSX = $(IFORT_COMPILER_LIB)/libifcoremt.a $(IFORT_COMPILER_LIB)/libifport.a

# ------------- intel_osx_64 ----------------

intel_osx_64 : FFLAGS    = -O0 -m64 -traceback -static-intel -fpp -D pp_OSX  -mmacosx-version-min=10.9 -stand:f18
intel_osx_64 : CFLAGS    = -O0 -m64 -I $(SOURCE_DIR)/glui_gl -traceback -static-intel -D pp_OSX $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO) -mmacosx-version-min=10.9
intel_osx_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_osx_64 : CFLAGS    += -D pp_LUA
intel_osx_64 : SMVLUACORE_FILES += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/lpeg.so
intel_osx_64 : LIBLUA    += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/liblua.a -ldl
endif
# remove following from LFLAGS line
# -L $(GLIBDIR) -lgfortran
intel_osx_64 : LFLAGS    = -m64 -L$(LIB_DIR)/intel_osx_$(NOQUARTZ)64 $(SMV_LIBS_OSX) -traceback -static-intel  -mmacosx-version-min=10.9
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
intel_osx_64 : FC        = ifort
intel_osx_64 : exe       = smokeview_osx_$(SMV_TESTSTRING)$(NOQUARTZ)64

intel_osx_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	icpc -o $(bin)/$(exe) $(obj) $(LFLAGS) $(INTEL_LIBS_OSX) $(LIBLUA)

# ------------- intel_osx_64_db ----------------

intel_osx_64_db : FFLAGS    = -O0 -g -m64 -traceback -static-intel -fpp     -D pp_OSX  -mmacosx-version-min=10.9 -stand:f18
intel_osx_64_db : CFLAGS    = -O0 -g -m64 -I $(SOURCE_DIR)/glui_gl -traceback -static-intel -D _DEBUG -D pp_OSX $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO) -mmacosx-version-min=10.9
intel_osx_64_db : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_osx_64_db : CFLAGS    += -D pp_LUA
intel_osx_64_db : SMVLUACORE_FILES += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/lpeg.so
intel_osx_64_db : LIBLUA    += $(LIB_DIR)/intel_osx_$(NOQUARTZ)64/liblua.a -ldl
endif
# remove following from LFLAGS line
intel_osx_64_db : LFLAGS    = -m64 -L$(LIB_DIR)/intel_osx_$(NOQUARTZ)64 $(SMV_LIBS_OSX) -traceback -static-intel -mmacosx-version-min=10.9
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
intel_osx_64_db : FC        = ifort
intel_osx_64_db : exe       = smokeview_osx_$(SMV_TESTSTRING)$(NOQUARTZ)64_db

intel_osx_64_db : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	icpc -o $(bin)/$(exe) $(obj) $(LFLAGS) $(INTEL_LIBS_OSX) $(LIBLUA)

# ------------- gnu_osx_64_db ----------------

gnu_osx_64_db : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC -ffree-form $(SMV_PROFILEFLAG)
gnu_osx_64_db : CFLAGS    = -O0 -ggdb -m64 -D _DEBUG -D pp_OSX -D pp_GCC $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx_64_db : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) $(GLIBDIROPT) -lgfortran $(SMV_PROFILEFLAG)
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
gnu_osx_64_db : FC        = gfortran$(GLIBVERS)
gnu_osx_64_db : exe       = smokeview_osx_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

gnu_osx_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) 

# ------------- gnu_osx_64 ----------------

gnu_osx_64 : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC -ffree-form $(SMV_PROFILEFLAG)
gnu_osx_64 : CFLAGS    = -O0 -m64              -D pp_OSX -D pp_GCC $(SMV_TESTFLAG) -Wall -Wno-comment -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO) $(SMV_PROFILEFLAG)
gnu_osx_64 : LFLAGS    = -m64 -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) $(GLIBDIROPT) -lgfortran $(SMV_PROFILEFLAG)
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
gnu_osx_64 : FC        = gfortran$(GLIBVERS)
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

# *** Object Dependencies ***

gsmv.o : fdsmodules.o
getdata.o : gsmv.o fdsmodules.o

gsmv.obj : fdsmodules.obj
getdata.obj : gsmv.obj fdsmodules.obj

# *** do a full rebuild if any header file has changed
#
$(obj) : $(incs)

$(objwin) : $(incs)

