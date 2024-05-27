#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"
#include "IOframe.h"
#include "file_util.h"

  /* ------------------ FRAMEInit ------------------------ */

framedata *FRAMEInit(char *file, char *size_file, void GetFrameInfo(char *file, char *size_file, int **sizes, int *nsizes)){
  framedata *frame;

  NewMemory((void **)&frame, sizeof(framedata));
  if(file != NULL && strlen(file) > 0){
    NewMemory((void **)&frame->file, strlen(file) + 1);
  }
  else{
    frame->file = NULL;
  }
  if(size_file != NULL && strlen(size_file) > 0){
    NewMemory((void **)&frame->size_file, strlen(size_file) + 1);
  }
  else{
    frame->size_file = NULL;
  }
  strcpy(frame->file, file);
  strcpy(frame->size_file, size_file);
  frame->nframes      = 0;
  frame->sizes        = NULL;
  frame->offsets      = NULL;
  frame->vals         = NULL;
  frame->GetFrameInfo = GetFrameInfo;
  return frame;
}

/* ------------------ FRAMESetup ------------------------ */

void FRAMESetup(framedata *fi){
  int *sizes, nsizes;
  int i;

  fi->GetFrameInfo(fi->file, fi->size_file, &sizes, &nsizes);
  if(nsizes <= 0)return;
  fi->sizes = sizes;
  fi->nframes = nsizes;
  if(nsizes > 0){
    NewMemory((void **)&fi->offsets, nsizes * sizeof(FILE_SIZE));
    fi->offsets[0] = 0;
    for(i = 1;i < fi->nframes;i++){
      fi->offsets[i] = fi->offsets[i - 1] + fi->sizes[i - 1];
    }
  }
}
/* ------------------ FRAMEFree ------------------------ */

void FRAMEFree(framedata *fi){
  fi->nframes = 0;
  FREEMEMORY(fi->sizes);
  FREEMEMORY(fi->vals);
  FREEMEMORY(fi->offsets);
}

/* ------------------ FRAMEReadFrame ------------------------ */

void FRAMEReadFrame(framedata *fi, int iframe, FILE *stream){
  FILE_SIZE offset;
  unsigned char *cvalptr;
  int size;

  if(iframe < 0)iframe = 0;
  if(iframe > fi->nframes - 1)iframe = fi->nframes - 1;
  offset  = fi->offsets[iframe];
  size    = fi->sizes[iframe];
  cvalptr = fi->vals + offset;
  fseek(stream, offset, SEEK_SET);
  fread(cvalptr, 1, size, stream);
}

/* ------------------ FRAMEGetFramePtr ------------------------ */

void *FRAMEGetFramePtr(framedata *fi, int iframe){
  unsigned char *valptr=NULL;

  if(fi->vals != NULL){
    if(iframe < 0)iframe = 0;
    if(iframe > fi->nframes - 1)iframe = fi->nframes - 1;
    valptr = (unsigned char *)(fi->vals + fi->offsets[iframe]);
  }
  return valptr;
}

/* ------------------ GetSliceFrameInfo ------------------------ */

void GetSliceFrameInfo(char *file, char *size_file, int **sizesptr, int *nsizesptr){
  FILE *stream;

  int headersize, framesize, nsizes, *sizes;
  int ijk[6];
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  
  stream = fopen(file, "rb");
  if(stream == NULL){
    *nsizesptr = 0;
    *sizesptr  = NULL;
  }

  headersize = 3*(4+30+4);

  fseek(stream, 4+headersize, SEEK_CUR);

  fread(ijk, 4, 6, stream);
  fclose(stream);

  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  headersize += 4+6*4+4;

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  framesize = 4*(1+nxsp*nysp*nzsp)+16;
  
  nsizes = 1;                                                     // header
  nsizes += (int)(GetFileSizeSMV(file) - headersize) / framesize; // time frames
  NewMemory((void **)&sizes, nsizes*sizeof(int));
  int i;
  sizes[0] = headersize;
  for(i=1;i<nsizes;i++){
    sizes[i] = framesize;
  }
  *sizesptr  = sizes;
  *nsizesptr = nsizes;
}
