#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"
#include "IOframe.h"
#include "file_util.h"

  /* ------------------ FRAMEInit ------------------------ */

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int **sizes, int *nsizes)){
  framedata *frame=NULL;

  NewMemory((void **)&frame, sizeof(framedata));
  if(file != NULL && strlen(file) > 0){
    NewMemory((void **)&frame->file, strlen(file) + 1);
  }
  else{
    FREEMEMORY(frame);
    return NULL;
  }
  if(size_file != NULL && strlen(size_file) > 0){
    NewMemory((void **)&frame->size_file, strlen(size_file) + 1);
  }
  else{
    FREEMEMORY(frame->file);
    FREEMEMORY(frame);
    return NULL;
  }
  if(file_type != C_FILE)file_type = FORTRAN_FILE;
  strcpy(frame->file, file);
  strcpy(frame->size_file, size_file);
  frame->file_type    = file_type;
  frame->nframes      = 0;
  frame->frame_sizes  = NULL;
  frame->offsets      = NULL;
  frame->uc_vals      = NULL;
  frame->uc_valptrs   = NULL;
  frame->r_valptrs    = NULL;
  frame->GetFrameInfo = GetFrameInfo;
  return frame;
}

/* ------------------ FRAMESetup ------------------------ */

void FRAMESetup(framedata *fi){
  int *frame_sizes, nframes;
  int i;

  fi->GetFrameInfo(fi->file, fi->size_file, &frame_sizes, &nframes);
  if(nframes <= 0)return;
  fi->frame_sizes = frame_sizes;
  fi->nframes = nframes;
  if(nframes > 0){
    NewMemory((void **)&fi->offsets,    nframes*sizeof(FILE_SIZE));
    NewMemory((void **)&fi->uc_valptrs, nframes*sizeof(unsigned char *));
    NewMemory((void **)&fi->r_valptrs,  nframes*sizeof(float *));
    NewMemory((void **)&fi->times,      nframes*sizeof(float));
    fi->offsets[0] = 0;
    for(i = 1;i < fi->nframes;i++){
      fi->offsets[i] = fi->offsets[i - 1] + fi->frame_sizes[i - 1];
    }
  }
}

/* ------------------ FRAMEFree ------------------------ */

void FRAMEFree(framedata *fi){
  fi->nframes = 0;
  FREEMEMORY(fi->frame_sizes);
  FREEMEMORY(fi->uc_vals);
  FREEMEMORY(fi->offsets);
  FREEMEMORY(fi->uc_valptrs);
  FREEMEMORY(fi->r_valptrs);
  FREEMEMORY(fi->times);
}

/* ------------------ FRAMEReadFrame ------------------------ */

void FRAMEReadFrame(framedata *fi, int iframe, int nframes, FILE *stream){
  FILE_SIZE offset, total_size;
  unsigned char *cvalptr;
  int i, first_frame, last_frame;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  
  total_size = 0;
  for(i=0;i<nframes;i++){
    total_size += fi->frame_sizes[iframe+i];
  }
  offset  = fi->offsets[iframe];
  cvalptr = fi->uc_vals + offset;
  fseek(stream, offset, SEEK_SET);
  fread(cvalptr, 1, total_size, stream);
}

/* ------------------ FRAMEGetFramePtr ------------------------ */

unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe){
  unsigned char *valptr=NULL;

  if(fi->uc_vals != NULL){
    if(iframe < 0)iframe = 0;
    if(iframe > fi->nframes - 1)iframe = fi->nframes - 1;
    return (unsigned char *)(fi->uc_valptrs[fi->offsets[iframe]]);
  }
  return NULL;
}

/* ------------------ GetFrameTimes ------------------------ */

void GetFrameTimes(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  for(i = 0;i < nframes;i++){
    int offset;

    offset = fi->offsets[i];
    if(fi->file_type == FORTRAN_FILE)offset += 4;
    memcpy(fi->times + i, fi->uc_vals + offset, sizeof(float));
  }
}

/* ------------------ GetFrameFloatValptrs ------------------------ */

void GetFrameFloatValptrs(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame > fi->nframes - 1)last_frame = fi->nframes - 1;
  nframes = last_frame + 1 - first_frame;
  for(i = 0;i < nframes;i++){
    fi->r_valptrs[i] = (float *)fi->uc_valptrs[i];
    if(fi->file_type == FORTRAN_FILE)fi->r_valptrs[i] += 4;
  }
}


/* ------------------ GetFrameUCValptrs ------------------------ */

void GetFrameUCValptrs(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame > fi->nframes - 1)last_frame = fi->nframes - 1;
  nframes = last_frame + 1 - first_frame;
  for(i = 0;i < nframes;i++){
    fi->uc_valptrs[i] = (unsigned char *)fi->uc_valptrs[i];
    if(fi->file_type == FORTRAN_FILE)fi->uc_valptrs[i] += 16;
  }
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

  headersize = 3*(4+30+4);  // 3 30 byte labels 

  fseek(stream, 4+headersize, SEEK_CUR);

  fread(ijk, 4, 6, stream);
  fclose(stream);

  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  headersize += 4+6*4+4;  // 6 4 byte integers

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  framesize  = 4 + 4 + 4;                  // time
  framesize += 4 + 4*(nxsp*nysp*nzsp) + 4; // data
  
  nsizes  = 1;                                                  // header
  nsizes += (int)(GetFileSizeSMV(file) - headersize)/framesize; // time frames
  NewMemory((void **)&sizes, nsizes*sizeof(int));
  int i;
  sizes[0] = headersize;
  for(i=1;i<nsizes;i++){
    sizes[i] = framesize;
  }
  *sizesptr  = sizes;
  *nsizesptr = nsizes;
}
