OPTS="-g -6"
LIBDIR=$(shell pwd)
SRCDIR=$(LIBDIR)/../../../Source

all: libgd.a libglui.a libglutwin.a libjpeg.a libpng.a libz.a lua53.dll lpeg.dll

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
libglutwin.a:
	cd $(SRCDIR)/glut-3.7.6; \
	export TARGET=libglutwin.a; \
	./makelib.sh $(OPTS); \
	cp libglutwin.a $(LIBDIR)/.

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
lua53.dll liblua.a:
	cd $(SRCDIR)/lua-5.3.1; \
		export TARGET=mingw; \
		./makelib.sh $(OPTS); \
		cp src/lua53.dll $(LIBDIR)/.; \
		cp src/liblua.a $(LIBDIR)/.

# LPEG # Lua parsing libarary to parse SSF files
# This depends on lua being built first
lpeg.dll: liblua.a
	cd $(SRCDIR)/lpeg-1.0.0; \
		pwd; \
		export TARGET=mingw; \
		./makelib.sh $(OPTS); \
		cp lpeg.dll $(LIBDIR)/.

.PHONY: all
