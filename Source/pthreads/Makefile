COMPILER=icx
SIZE=-m64
CC=$(COMPILER) $(SIZE)

INC=-I.
CFLAGS=
CFLAGS=$(CFLAGS)
FILTERC=-D PTW32_BUILD -D PTW32_STATIC_LIB
OPT=

.SUFFIXES:      .c .o .obj

.c.obj:
	$(CC) -c $(CFLAGS) $(INC) $(OPT) $(FILTERC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(INC) $(FILTERC) $<

RANLIB=ranlib
RM=/bin/rm -f
LIB=lib
OBJS = pthread.o
OBJSWIN = $(OBJS:.o=.obj)

libpthreads.a: $(OBJS)
	ar rc $@ $(OBJS)
	$(RANLIB) $@

libpthreads.lib: $(OBJSWIN)
	$(LIB) $(OBJSWIN) -OUT:$@
