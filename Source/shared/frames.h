#ifndef FRAMES_H_DEFINED
#define FRAMES_H_DEFINED

// vvvvvvvvvvvvvvvvvvvvvvvv header files vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#include "options.h"

// vvvvvvvvvvvvvvvvvvvvvvvv structures vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

/* --------------------------  framedata ------------------------------------ */

typedef struct _framedata {
  int nvals;
  float time, *vals;
  size_t offset, size;
  unsigned char *cvals;
} framedata;

/* --------------------------  framesdata ------------------------------------ */

typedef struct _framesdata {
  int nframes_old, nframes;
  float *times;
  framedata **frames;
} framesdata;

// vvvvvvvvvvvvvvvvvvvvvvvv preprocessing directives vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

// vvvvvvvvvvvvvvvvvvvvvvvv headers vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

EXTERNCPP void       FreeFrame(framedata *frame);
EXTERNCPP void       FreeFrames(framesdata *frames);
EXTERNCPP void       InitFrames(framesdata *frames);
EXTERNCPP framedata *NewFrame(int nvals);
EXTERNCPP void       NewFrames(framesdata *frames, int nframes, int nvals);

// vvvvvvvvvvvvvvvvvvvvvvvv variables vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv


#endif
