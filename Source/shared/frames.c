#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MALLOC.h"
#include "frames.h"

#define FORTREAD(var,count,STREAM) FSEEK(STREAM,4,SEEK_CUR);\
                           returncode=fread(var,4,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           FSEEK(STREAM,4,SEEK_CUR)

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

/* --------------------------  SetSliceFrameSizeOffsets ------------------------------------ */

void SetSliceFrameSizeOffsets(char *file, framesdata *frames){
  FILE *stream;
  int i,returncode, nframes=0, skip=3*(4+30+4), data_framesize, total_framesize, xb[6], dx, dy, dz;
  float time;

  if(file==NULL)return;
  stream = fopen(file, "rb");
  if(stream==NULL)return;
  FSEEK(stream, skip, SEEK_CUR);
  FORTREAD(xb, 6, stream);
  dx = xb[1]+1-xb[0];
  dy = xb[3]+1-xb[2];
  dz = xb[5]+1-xb[4];
  data_framesize = dx*dy*dz;
  if(data_framesize<=0)return;
  // slice file header:
  // label 1 - 30 characters
  // label 2 - 30 characters
  // label 3 - 30 characters
  // imin imax jmin jmax kmin kmax - 6 integers

  // one slice file frame (time + data )
  // time - 1 float
  // data - framesize floats (framesize = (imax+1-imin)*(jmax+1-jmin)*(kmax+1-kmin)
  total_framesize = (4+4+4)+(4+data_framesize+4); // time + data
  for(;;){
    if(FSEEK(stream, total_framesize, SEEK_CUR)!=0)break;
    nframes++;
  }
  fclose(stream);

  NewFrames(frames, nframes, data_framesize);
  frames->frames[0]->offset = 3*(4+30+4)+4+6*4+4;
  frames->frames[0]->size = data_framesize;
  for(i = 1;i<nframes;i++){
    framedata *framei, *frameim1;

    framei = frames->frames[i];
    frameim1 = frames->frames[i-1];
    framei->size = data_framesize;
    framei->offset = frameim1->offset+total_framesize;
  }
}

