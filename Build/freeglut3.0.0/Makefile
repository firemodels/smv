
PLATFORM=
SIZE=
COMPILER=icc
CC=$(COMPILER) $(SIZE)

FILTERC=
# FILTERC= -D WIN32 -D _LIB

.c.obj:
	$(CC) -c $(SIZE) $(CFLAGS) $(INC) $(FILTERC) $<
.c.o:
	$(CC) -c $(SIZE) $(CFLAGS) $(INC) $(FILTERC) $<

SOURCE_DIR=../../../Source/freeglut3.0.0

ifeq ($(PLATFORM),win)
  VPATH=$(SOURCE_DIR):$(SOURCE_DIR)/mswin:$(SOURCE_DIR)/util
else
  VPATH=$(SOURCE_DIR):$(SOURCE_DIR)/x11:$(SOURCE_DIR)/util
endif

RANLIB=ranlib
# gles_stubs.o
OBJS_ALL=fg_callbacks.o fg_cursor.o fg_display.o fg_ext.o fg_font.o fg_font_data.o fg_gamemode.o \
fg_geometry.o fg_gl2.o fg_init.o fg_input_devices.o fg_joystick.o fg_main.o fg_menu.o \
fg_misc.o fg_overlay.o fg_spaceball.o fg_state.o fg_stroke_mono_roman.o fg_stroke_roman.o\
fg_structure.o fg_teapot.o fg_videoresize.o fg_window.o xparsegeometry_repl.o

OBJS_X11=fg_cursor_x11.o fg_display_x11_glx.o fg_ext_x11.o fg_gamemode_x11.o fg_glutfont_definitions_x11.o\
fg_init_x11.o fg_input_devices_x11.o fg_joystick_x11.o fg_main_x11.o fg_menu_x11.o fg_spaceball_x11.o\
fg_state_x11.o fg_state_x11_glx.o fg_structure_x11.o fg_window_x11.o fg_window_x11_glx.o fg_xinput_x11.o

OBJS_MSWIN=fg_cursor_mswin.o fg_display_mswin.o fg_ext_mswin.o fg_gamemode_mswin.o fg_init_mswin.o\
fg_input_devices_mswin.o fg_joystick_mswin.o fg_main_mswin.o fg_menu_mswin.o fg_spaceball_mswin.o\
fg_state_mswin.o fg_structure_mswin.o fg_window_mswin.o

# use for Linux and OSX
OBJS =  $(OBJS_ALL) $(OBJS_X11)

# use for Windows
OBJSWINS =  $(OBJS_ALL) $(OBJS_MSWIN)
OBJSWINS_WIN = $(OBJSWINS:.o=.obj)

.SUFFIXES:      .c .o .obj .pic.o

# libglut.a
intel_linux_64 : INC=-I $(SOURCE_DIR)/. -I $(SOURCE_DIR)/x11 -I $(SOURCE_DIR)/include 
intel_linux_64 : FILTERC=-D FREEGLUT_STATIC -D NEED_XPARSEGEOMETRY_IMPL -D GLUT_DISABLE_ATEXIT_HACK
intel_linux_64 : CC=icc
intel_linux_64 : $(OBJS)
	ar rcs $@ $(OBJS)

# libglutwin.a
gnu_win_64 : INC=-I $(SOURCE_DIR)/. -I $(SOURCE_DIR)/mswin -I $(SOURCE_DIR)/include 
gnu_win_64 : FILTERC=-D __MINGW32__ -D FREEGLUT_STATIC -D NEED_XPARSEGEOMETRY_IMPL -D GLUT_DISABLE_ATEXIT_HACK
gnu_win_64 : CC=gcc
gnu_win_64 : $(OBJSWINS)
	ar rcs libglutwin.a $(OBJSWINS) 

# libglutwin.lib
intel_win_64 : INC=-I $(SOURCE_DIR)/. -I $(SOURCE_DIR)/mswin -I $(SOURCE_DIR)/include 
intel_win_64 : FILTERC= -D FREEGLUT_STATIC -D NEED_XPARSEGEOMETRY_IMPL -D GLUT_DISABLE_ATEXIT_HACK
intel_win_64 : CC=icl
intel_win_64 : $(OBJSWINS_WIN)
	lib  $(OBJSWINS_WIN) -OUT:libglutwin.lib