# For compiling with source and object files in different directories.
srcdir = .

# The name of your C compiler:
COMPILER=icc
SIZE=-m64
CC=$(COMPILER) $(SIZE)
INC=-I headers -I vrcommon

# You may need to adjust these cc options:
CFLAGS= -O2  -I$(srcdir)
WINOPTS= -D _WIN64 -D WIN32 -D _WIN32 -D OPENVR_BUILD_STATIC

.cpp.obj:
	$(CC) -c $(CFLAGS) $(WINOPTS) $(INC) $(FILTERC) $<
.c.obj:
	$(CC) -c $(CFLAGS) $(WINOPTS) $(INC) $(FILTERC) $<
.cpp.o:
	$(CC) -c $(CFLAGS) $(INC) $(FILTERC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(INC) $(FILTERC) $<

# Link-time cc options:
LDFLAGS=

# miscellaneous OS-dependent stuff
SHELL= /bin/sh
# linker
LN= $(CC)
# file deletion command
RM= rm -f
# library (.a) file creation command
AR= ar rc
# second step in .a creation (use "touch" if not needed)
AR2= ranlib
LIB=lib

# End of configurable options.

OBJECTS= openvr_api_public.o jsoncpp.o
OBJECTSWIN = $(OBJECTS:.o=.obj)

.SUFFIXES: .o .obj

# the library:

libopenvr.a:  $(OBJECTS)
	$(RM) libopenvr.a
	$(AR) libopenvr.a  $(OBJECTS)
	$(AR2) libopenvr.a

libopenvr.lib:  $(OBJECTSWIN)
	$(LIB)   $(OBJECTSWIN) -OUT:libopenvr.lib
