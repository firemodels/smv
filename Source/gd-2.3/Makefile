#If the ar command fails on your system, consult the ar manpage
#for your system. 
AR=ar

#CFLAGS=-g  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG -DHAVE_LIBX11
CFLAGS=-DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBX11

#INCLUDEDIRS=-I. -I../png-1.6.48 -I../jpeg-9b -I../zlib131
INCLUDEDIRS=-I. -I../png-1.6.48 -I../zlib131

#
#
# Changes should not be required below here.
#
#

# Update these with each release!

MAJOR_VERSION=2
VERSION=2.0.4

COMPILER=gcc -fPIC
#COMPILER=icc
#SIZE=-m32
SIZE=-m64

CC=$(COMPILER) $(SIZE) $(INCLUDEDIRS) -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types

.SUFFIXES:      .c .o .obj

.c.obj:
	$(CC) -c $(CFLAGS) $(INC) $(FILTERC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(INC) $(FILTERC) $<

#	gd_jpeg.o \

LIBOBJS=gd.o \
	gd_avif.o \
	gd_bmp.o \
	gd_color.o \
	gd_color_map.o \
	gd_color_match.o \
	gd_crop.o \
	gd_filename.o \
	gd_filter.o \
	gd_gd.o \
	gd_gd2.o \
	gd_gif_in.o \
	gd_gif_out.o \
	gd_heif.o \
	gd_interpolation.o \
	gd_io.o \
	gd_io_dp.o \
	gd_io_file.o \
	gd_io_ss.o \
	gd_matrix.o \
	gd_nnquant.o \
	gd_png.o \
	gd_rotate.o \
	gd_security.o \
	gd_ss.o \
	gd_tga.o \
	gd_tiff.o \
	gd_topal.o \
	gd_transform.o \
	gd_version.o \
	gd_wbmp.o \
	gd_webp.o \
	gd_xbm.o \
	gdcache.o \
	gdfontg.o \
	gdfontl.o \
	gdfontmb.o \
	gdfonts.o \
	gdfontt.o \
	gdft.o \
	gdfx.o \
	gdhelpers.o \
	gdkanji.o \
	gdtables.o \
	gdxpm.o \
	wbmp.o 

LIBOBJSWIN = $(LIBOBJS:.o=.obj)

#Static library, if you really need one for some reason.
libgd.a: ${LIBOBJS}
	$(AR) rc libgd.a ${LIBOBJS}
	-ranlib libgd.a

libgd.lib: ${LIBOBJSWIN}
	lib ${LIBOBJSWIN} -OUT:libgd.lib 
