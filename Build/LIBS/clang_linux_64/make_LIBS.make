OPTS="-g"
LIBDIR=$(shell pwd)
SRCDIR=$(LIBDIR)/../../../Source

ALL = libgd.a libglui.a libglut.a libjpeg.a libpng.a libz.a

all: $(ALL)

# GD
libgd.a:
	@echo $(LIBDIR)
	cd $(SRCDIR)/gd-2.0.15; \
		./makelib.sh $(OPTS); \
		cp libgd.a $(LIBDIR)/.

# GLUI
libglui.a:
	cd $(SRCDIR)/glui_v2_1_beta; \
		./makelib.sh $(OPTS); \
		cp libglui.a $(LIBDIR)/.

# GLUT
libglut.a:
	cd $(SRCDIR)/glut-3.7.6; \
	export TARGET=libglut.a; \
	./makelib.sh $(OPTS); \
	cp libglut.a $(LIBDIR)/.

# JPEG
libjpeg.a:
	cd $(SRCDIR)/jpeg-9b; \
		./makelib.sh $(OPTS); \
		cp libjpeg.a $(LIBDIR)/.

# PNG
libpng.a:
	cd $(SRCDIR)/png-1.6.48; \
		./makelib.sh $(OPTS); \
		cp libpng.a $(LIBDIR)/.

# ZLIB
libz.a:
	cd $(SRCDIR)/zlib131; \
		./makelib.sh $(OPTS); \
		cp libz.a $(LIBDIR)/.

.PHONY: all
