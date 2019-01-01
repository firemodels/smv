.SUFFIXES: .c .cpp .o .obj

COMPILER=icc
COMPILER2=icc
CC=${COMPILER}
CPP=${COMPILER2}

ifeq ($(GLUT),freeglut)
  INCS = -I ../freeglut3.0.0/include
  NOCLOSE=-D FREEGLUT_STATIC
else
  INCS = -I ../GLUTINC/glut
  NOCLOSE=-D pp_NOCLOSE
endif

.c.obj:
	$(CC)  $(CFLAGS) $(INCS) $(FILTERC) -c $<
.c.o:
	$(CC)  $(CFLAGS) $(INCS) $(FILTERC) -c $<
.cpp.obj:
	$(CPP) $(CFLAGS) $(INCS) $(FILTERC) -c $<
.cpp.o:
	$(CPP) $(CFLAGS) $(INCS) $(FILTERC) -c $<


GLUI_OBJS = glui_add_controls.o glui.o glui_bitmap_img_data.o glui_bitmaps.o \
            glui_button.o glui_edittext.o glui_checkbox.o glui_node.o \
            glui_radio.o glui_statictext.o glui_panel.o glui_separator.o \
            glui_spinner.o glui_control.o glui_column.o glui_translation.o \
            glui_rotation.o glui_mouse_iaction.o glui_listbox.o glui_rollout.o\
            arcball.o algebra3.o quaternion.o
GLUI_OBJSWIN = $(GLUI_OBJS:.o=.obj)

default: CFLAGS=-m64 $(PLATFORM) $(NOCLOSE) $(CFLAGOPT)
default: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_linux_64: CFLAGS=-m64 $(NOCLOSE) -fpermissive 
intel_linux_64: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_osx_64: CFLAGS=-m64 $(NOCLOSE) -fpermissive 
intel_osx_64: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

gnu_osx_64: CFLAGS=-m64 $(NOCLOSE) -fpermissive 
gnu_osx_64: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_win_64: CFLAGS= $(NOCLOSE) 
intel_win_64 : $(GLUI_OBJSWIN)
	lib $(GLUI_OBJSWIN) -OUT:libglui.lib 

gnu_win_64: CFLAGS=$(NOCLOSE) -fpermissive -Wno-write-strings -Wno-int-to-pointer-cast 
gnu_win_64: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)
