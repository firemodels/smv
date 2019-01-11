OPTS="-g"
LIBDIR=$(shell pwd)
SRCDIR=$(LIBDIR)/../../../Source

all: libgd.a libglui.a libglut.a libjpeg.a libpng.a libz.a liblua.a lpeg.so
all_nolua: libgd.a libglui.a libglut.a libjpeg.a libpng.a libz.a

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
	cd $(SRCDIR)/png-1.6.21; \
		./makelib.sh $(OPTS); \
		cp libpng.a $(LIBDIR)/.

# ZLIB
libz.a:
	cd $(SRCDIR)/zlib128; \
		./makelib.sh $(OPTS); \
		cp libz.a $(LIBDIR)/.

# Lua # Lua interpreter
liblua.a:
	cd $(SRCDIR)/lua-5.3.1; \
		export TARGET=linux; \
		./makelib.sh $(OPTS); \
		cp src/liblua.a $(LIBDIR)/.

# LPEG # Lua parsing libarary to parse SSF files
# This depends on lua being built first
lpeg.so: liblua.a
	cd $(SRCDIR)/lpeg-1.0.0; \
		pwd; \
		export TARGET=linux; \
		./makelib.sh $(OPTS); \
		cp lpeg.so $(LIBDIR)/.

.PHONY: all,all_nolua
