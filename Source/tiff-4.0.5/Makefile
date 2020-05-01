srcdir = .

COMPILER=icc
SIZE=-m64
CC=$(COMPILER) $(SIZE)

# You may need to adjust these cc options:
CFLAGS= -O2  -I$(srcdir) -I../jpeg-9b -I../zlib128

.c.obj:
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

OBJECTS=tif_aux.o tif_dirinfo.o tif_fax3.o tif_jpeg.o tif_open.o tif_strip.o  \
tif_close.o tif_dirread.o tif_fax3sm.o tif_luv.o tif_packbits.o  tif_swab.o tif_warning.o   \
tif_codec.o tif_dirwrite.o tif_flush.o tif_thunder.o  \
tif_color.o tif_dumpmode.o tif_getimage.o tif_lzw.o tif_predict.o tif_tile.o  \
tif_compress.o  tif_error.o tif_jbig.o tif_next.o tif_print.o   \
tif_extension.o  tif_jpeg_12.o tif_ojpeg.o tif_read.o tif_version.o  tif_write.o

ifeq ($(SHELL echo "check_quotes"),"check_quotes")
#
else
  OBJECTS+= tif_zip.o tif_lzma.o tif_pixarlog.o tif_thunder.o tif_unix.o
endif

OBJECTSWIN = $(OBJECTS:.o=.obj)

.SUFFIXES: .o .obj

# the library:

libtiff.a:  $(OBJECTS)
	$(AR) libtiff.a  $(OBJECTS)
	$(AR2) libtiff.a

libtiff.lib:  $(OBJECTSWIN)
	lib   $(OBJECTSWIN) -OUT:libtiff.lib
