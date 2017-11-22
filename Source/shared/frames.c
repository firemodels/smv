#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MALLOC.h"
#include "frames.h"

/* --------------------------  InitFrames ------------------------------------ */

void InitFrames(framesdata *frames){
  frames->frames = NULL;
  frames->times = NULL;
  frames->nframes = 0;
  frames->nframes_old = 0;
}

/* --------------------------  FreeFrame ------------------------------------ */

void FreeFrame(framedata *frame){
  FREEMEMORY(frame->cvals);
  FREEMEMORY(frame->vals);
}

 /* --------------------------  FreeFrames ------------------------------------ */

void FreeFrames(framesdata *frames){
  int i;

  for(i = 0;i<frames->nframes;i++){
    FreeFrame(frames->frames[i]);
  }
  FREEMEMORY(frames->frames);
  FREEMEMORY(frames->times);
}

/* --------------------------  NewFrame ------------------------------------ */

framedata *NewFrame(int nvals){
  framedata *newframe;

  if(nvals <= 0)return NULL;
  NewMemory((void **)&newframe, sizeof(framedata));
  if(nvals>0){
    NewMemory((void **)&newframe->cvals, nvals*sizeof(unsigned char));
    NewMemory((void **)&newframe->vals,  nvals*sizeof(float));
  }
  else{
    newframe->cvals = NULL;
    newframe->vals = NULL;
  }
  newframe->nvals = nvals;
  return newframe;
}

  /* --------------------------  NewFrames ------------------------------------ */

void NewFrames(framesdata *frames, int nframes, int nvals){
  int i;

  if(nframes<=0){
    FreeFrames(frames);
    return;
  }

  if(frames->times==NULL){
    NewMemory((void **)&frames->times, nframes*sizeof(float));
  }
  else{
    ResizeMemory((void **)&frames->times, nframes*sizeof(float));
  }

  if(frames->frames==NULL){
    NewMemory((void **)&frames->frames, nframes*sizeof(framedata *));
  }
  else{
    ResizeMemory((void **)&frames->frames, nframes*sizeof(framedata *));
  }

  for(i = frames->nframes;i<nframes;i++){
    frames->frames[i] = NewFrame(nvals);
  }
  frames->nframes_old = frames->nframes;
  frames->nframes = nframes;
}

