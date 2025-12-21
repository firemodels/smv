.SUFFIXES: .c .cpp .o .obj

COMPILER=icx
COMPILER2=icx
CC=${COMPILER}
CPP=${COMPILER2}
LIB=lib

ifeq ($(GLUT),freeglut)
  INCS = -I ../freeglut3.0.0/include
  NOCLOSE=-D FREEGLUT_STATIC
else
  INCS = -I ../glui_gl
  NOCLOSE=-D pp_NOCLOSE
endif

CFLAGS=-D_CRT_SECURE_NO_WARNINGS -Wno-deprecated-declarations -Wdeprecated-declarations

.c.obj:
	$(CC)  $(CFLAGS) $(INCS) $(FILTERC) -c $<
.c.o:
	$(CC)  $(CFLAGS) $(FILTERC) -c $<
.cpp.obj:
	$(CPP) $(CPP_STANDARD) $(CFLAGS) $(INCS) $(FILTERC) -c $<
.cpp.o:
	$(CPP) $(CPP_STANDARD) $(CFLAGS) $(FILTERC) -c $<


GLUI_OBJS = glui_add_controls.o glui.o glui_bitmap_img_data.o glui_bitmaps.o \
            glui_button.o glui_edittext.o glui_checkbox.o glui_node.o \
            glui_radio.o glui_statictext.o glui_panel.o glui_separator.o \
            glui_spinner.o glui_control.o glui_column.o glui_translation.o \
            glui_rotation.o glui_mouse_iaction.o glui_listbox.o glui_rollout.o\
            glut_hel20.o glut_hel24.o glut_hel36.o\
            arcball.o algebra3.o quaternion.o
GLUI_OBJSWIN = $(GLUI_OBJS:.o=.obj)

default: CFLAGS=-m64 $(INCS) $(PLATFORM) $(NOCLOSE) $(CFLAGOPT)
default: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_linux: CPP_STANDARD = -std=c++11 -stdlib=libc++
intel_linux: CFLAGS=-m64 $(INCS) $(PLATFORM) $(NOCLOSE) $(CFLAGOPT) -fpermissive
intel_linux: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_osx: CFLAGS=-m64 -stdlib=libc++ $(NOCLOSE) -fpermissive
intel_osx: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

gnu_osx: CFLAGS=-m64 $(NOCLOSE) -fpermissive
gnu_osx: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_win: CFLAGS+= $(NOCLOSE)
intel_win : $(GLUI_OBJSWIN)
	$(LIB) $(GLUI_OBJSWIN) -OUT:libglui.lib

clang_win: CFLAGS= $(NOCLOSE)
clang_win : $(GLUI_OBJSWIN)
	$(LIB) $(GLUI_OBJSWIN) -OUT:libglui.lib

gnu_win: CFLAGS=$(NOCLOSE) $(INCS) -fpermissive -Wno-write-strings -Wno-int-to-pointer-cast
gnu_win: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)
