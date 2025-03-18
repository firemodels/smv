// _GNU_SOURCE needs to be switched on to access qsort_r. qsort_r is included in
// POSIX 2024 as standard but is also available via GNU extensions.
#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>


#include "translate.h"
#include "file_util.h"
#include "datadefs.h"
#include "readimage.h"
#include "readhvac.h"
#include "readslice.h"
#include "readobject.h"
#include "readsmoke.h"
#include "readlabel.h"
#include "readtour.h"
#include "readgeom.h"
#include "colorbars.h"
#include "readcad.h"
#include "readsmoke.h"
#include "readcad.h"
#include "readtour.h"

#define BREAK break
#define BREAK2 \
      if((stream==stream1&&stream2==NULL)||stream==stream2)break;\
      stream=stream2;\
      continue

#define COLOR_INVISIBLE -2

#define BLOCK_OUTLINE 2

#define DEVICE_DEVICE 0

#define ZVENT_1ROOM 1
#define ZVENT_2ROOM 2

#define PARTBUFFER(len)    scase->part_buffer;    scase->part_buffer    += (len)
#define SMOKE3DBUFFER(len) scase->smoke3d_buffer; scase->smoke3d_buffer += (len)
#define SLICEBUFFER(len)   scase->slice_buffer;   scase->slice_buffer   += (len)

#define SCAN    0
#define NO_SCAN 1
