.SUFFIXES: .c .cpp .o .obj

SIZE=-m64
PLATFORM=
CFLAGOPT=
GLUT_INC_LOCATION=../GLUTINC

RM=/bin/rm
COMPILER=icc
COMPILER2=icc
CC=${COMPILER}
CPP=${COMPILER2}
LPATH=${GLUTPATH} 
INCS= 

.c.obj:
	$(CC) -c $(CFLAGS) $(INCS) $(FILTERC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(INCS) $(FILTERC) $<
.cpp.obj:
	$(CPP) -c $(CFLAGS) $(INCS) $(FILTERC) $<
.cpp.o:
	$(CPP) -c $(CFLAGS) $(INCS) $(FILTERC) $<


GLUI_OBJS = glui_add_controls.o glui.o glui_bitmap_img_data.o glui_bitmaps.o \
            glui_button.o glui_edittext.o glui_checkbox.o glui_node.o \
            glui_radio.o glui_statictext.o glui_panel.o glui_separator.o \
            glui_spinner.o glui_control.o glui_column.o glui_translation.o \
            glui_rotation.o glui_mouse_iaction.o glui_listbox.o glui_rollout.o\
            arcball.o algebra3.o quaternion.o
GLUI_OBJSWIN = $(GLUI_OBJS:.o=.obj)


default: CFLAGS=$(PLATFORM) ${CFLAGOPT} -D pp_NOCLOSE -fpermissive -I${GLUT_INC_LOCATION} -I ../GLUTINC/glut ${SIZE} 
default: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_linux_64: CFLAGS=$(PLATFORM) ${CFLAGOPT} -D pp_NOCLOSE -fpermissive -I${GLUT_INC_LOCATION} -I ../GLUTINC/glut ${SIZE} 
intel_linux_64: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_osx_64: CFLAGS=$(PLATFORM) ${CFLAGOPT} -D pp_NOCLOSE -fpermissive -I ${GLUT_INC_LOCATION} -I ../GLUTINC/glut ${SIZE} 
intel_osx_64: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_osx_64q: CFLAGS=-I /usr/X11/include -I /usr/X11/include/GL $(PLATFORM) ${CFLAGOPT} -D pp_NOCLOSE -D pp_QUARTZ -fpermissive ${SIZE} 
intel_osx_64q: $(GLUI_OBJS)
	ar rcs libglui.a $(GLUI_OBJS)

intel_win_64: CFLAGS=$(PLATFORM) ${CFLAGOPT} -D pp_NOCLOSE -fpermissive -I${GLUT_INC_LOCATION} -I ../GLUTINC/glut ${SIZE} 
intel_win_64 : $(GLUI_OBJSWIN)
	lib $(GLUI_OBJSWIN) -OUT:libglui.lib 
