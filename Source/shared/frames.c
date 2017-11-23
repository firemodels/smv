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
  FREEMEMORY(frame->subsizes);
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

framedata *NewFrame(int nvals, int nsubsizes){
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
  if(nsubsizes>0){
    NewMemory((void **)&newframe->subsizes, nsubsizes*sizeof(int));
  }
  else{
    newframe->subsizes = NULL;
  }
  newframe->nvals = nvals;
  newframe->nsubsizes = nsubsizes;
  return newframe;
}

  /* --------------------------  NewFrames ------------------------------------ */

void NewFrames(framesdata *frames, int nframes, int nvals, int nsubsizes){
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
    frames->frames[i] = NewFrame(nvals, nsubsizes);
  }
  frames->nframes_old = frames->nframes;
  frames->nframes = nframes;
}

/* --------------------------  SetSliceFrameSizeOffsets ------------------------------------ */

void SetSliceFrameSizeOffsets(char *file, framesdata *frames){
  FILE *stream;
  int i,returncode, nframes=0, xb[6], dx, dy, dz;
  int total_framesize_bytes, data_framesize_floats, label_size_bytes = 3*(4+30+4), header_size_bytes;
  float time;

  if(file==NULL)return;
  stream = fopen(file, "rb");
  if(stream==NULL)return;
  FSEEK(stream, label_size_bytes, SEEK_CUR);
  FORTREAD(xb, 6, stream);
  dx = xb[1]+1-xb[0];
  dy = xb[3]+1-xb[2];
  dz = xb[5]+1-xb[4];
  header_size_bytes = label_size_bytes+(4+6*4+4);
  data_framesize_floats = dx*dy*dz;
  if(data_framesize_floats<=0)return;

  // slice file header:
  // label 1 - 30 characters
  // label 2 - 30 characters
  // label 3 - 30 characters
  // imin imax jmin jmax kmin kmax - 6 integers

  // one slice file frame (time + data )
  // time - 1 float
  // data - framesize floats (framesize = (imax+1-imin)*(jmax+1-jmin)*(kmax+1-kmin)
  total_framesize_bytes = (4+4+4)+(4+4*data_framesize_floats+4); // time + data
  for(;;){
    if(FSEEK(stream, total_framesize_bytes, SEEK_CUR)!=0)break;
    nframes++;
  }
  fclose(stream);

  NewFrames(frames, nframes, data_framesize_floats, 1);
  frames->frames[0]->offset = header_size_bytes;
  frames->frames[0]->subsizes[0] = data_framesize_floats;
  for(i = 1;i<nframes;i++){
    framedata *framei, *frameim1;

    framei = frames->frames[i];
    frameim1 = frames->frames[i-1];
    framei->subsizes[0] = data_framesize_floats;
    framei->offset = frameim1->offset+total_framesize_bytes;
  }
}


/* --------------------------  SetBoundaryFrameSizeOffsets ------------------------------------ */

void SetBoundaryFrameSizeOffsets(char *file, framesdata *frames){
  FILE *stream;
  int i, returncode, nframes = 0, xb[7];
  int dataframe_size_floats, label_size_bytes = 3*(4+30+4), dataframe_offset_bytes;
  int npatches, *subsizes;
  float time;

  if(file==NULL)return;
  stream = fopen(file, "rb");
  if(stream==NULL)return;

  FSEEK(stream, label_size_bytes, SEEK_CUR);
  FORTREAD(&npatches, 1, stream);
  NewMemory((void **)&subsizes, npatches*sizeof(int));
  dataframe_size_floats = 0;
  for(i = 0;i<npatches;i++){
    int dx, dy, dz;

    FORTREAD(xb, 7, stream);
    dx = xb[1]+1-xb[0];
    dy = xb[3]+1-xb[2];
    dz = xb[5]+1-xb[4];
    dataframe_size_floats += dx*dy*dz;
    subsizes[i] = dx*dy*dz;
  }
  dataframe_offset_bytes = label_size_bytes+(4+4+4)+npatches*(4+7*4+4);
  NewFrames(frames, nframes, dataframe_size_floats, npatches);

  if(dataframe_size_floats<=0)return;

  // slice file header:
  // label 1 - 30 characters
  // label 2 - 30 characters
  // label 3 - 30 characters
  // imin imax jmin jmax kmin kmax - 6 integers

  // one slice file frame (time + data )
  // time - 1 float
  // data - framesize floats (framesize = (imax+1-imin)*(jmax+1-jmin)*(kmax+1-kmin)
  FSEEK(stream, dataframe_offset_bytes, SEEK_SET);
  for(;;){
    if(FSEEK(stream, (4+4+4)+4*dataframe_size_floats, SEEK_CUR)!=0)break;
    nframes++;
  }
  fclose(stream);

  NewFrames(frames, nframes, dataframe_size_floats, npatches);
  frames->frames[0]->offset = dataframe_offset_bytes;
  for(i = 1;i<nframes;i++){
    framedata *framei, *frameim1;
    int j;

    framei = frames->frames[i];
    frameim1 = frames->frames[i-1];
    for(j = 0;j<framei->nsubsizes;j++){
      framei->subsizes[j] = subsizes[j];
    }
    framei->offset = frameim1->offset+4*dataframe_size_floats;
  }
}

/* --------------------------  UpdateFrames ------------------------------------ */

void UpdateFrames(char *file, framesdata *frames){
  int i,nframes_old;
  FILE *stream;

  nframes_old = frames->nframes;
  SetSliceFrameSizeOffsets(file, frames);
  if(file==NULL)return;
  stream = fopen(file, "rb");
  if(stream==NULL)return;
  for(i = nframes_old;i<frames->nframes;i++){
    framedata *framei;
    int j, returncode;
    float *vals;

    framei = frames->frames[i];
    FSEEK(stream, framei->offset, SEEK_SET);
    vals = framei->vals;
    for(j = 0;j<framei->nsubsizes;j++){
      FORTREAD(framei->vals, framei->subsizes[j], stream);
      vals += framei->subsizes[j];
    }
  }
  fclose(stream);
}
