#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"
#include "IOframe.h"
#include "file_util.h"

  /* ------------------ FRAMEInit ------------------------ */

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int **sizes, int *nsizes, FILE_SIZE *filesizeptr)){
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
  frame->times        = NULL;
  frame->GetFrameInfo = GetFrameInfo;
  return frame;
}

/* ------------------ FRAMESetup ------------------------ */

void FRAMESetup(framedata *fi){
  int *frame_sizes, nframes;
  int i;
  FILE_SIZE filesize;

  fi->GetFrameInfo(fi->file, fi->size_file, &frame_sizes, &nframes, &filesize);
  if(nframes <= 0)return;
  fi->frame_sizes = frame_sizes;
  fi->nframes     = nframes;
  fi->filesize    = filesize;
  if(nframes > 0){
    NewMemory((void **)&fi->offsets,    nframes*sizeof(FILE_SIZE));
    NewMemory((void **)&fi->uc_valptrs, nframes*sizeof(unsigned char *));
    NewMemory((void **)&fi->r_valptrs,  nframes*sizeof(float *));
    NewMemory((void **)&fi->times,      nframes*sizeof(float));
    NewMemory((void **)&fi->uc_vals,    fi->filesize);
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

void FRAMEReadFrame(framedata *fi, int iframe, int nframes){
  FILE_SIZE offset, total_size;
  unsigned char *cvalptr;
  int i, first_frame, last_frame;
  FILE *stream;

  stream = fopen(fi->file, "rb");

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
  fclose(stream);
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

  if(iframe < 1)iframe = 1;
  first_frame = iframe;
  last_frame = first_frame + nframes - 2;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  for(i = iframe;i < nframes;i++){
    int offset;

    offset = fi->offsets[i];
    if(fi->file_type == FORTRAN_FILE)offset += 4;
    memcpy(fi->times + i - 1, fi->uc_vals + offset, sizeof(float));
  }
}

/* ------------------ GetFrameFloatValptrs ------------------------ */

void GetFrameFloatValptrs(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 1)iframe = 1;
  first_frame = iframe;
  last_frame = first_frame + nframes - 2;
  if(last_frame > fi->nframes - 1)last_frame = fi->nframes - 1;
  nframes = last_frame + 1 - first_frame;
  if(fi->file_type == FORTRAN_FILE){
    for(i = iframe;i < nframes;i++){
      fi->r_valptrs[i - 1] = (float *)(fi->uc_vals+fi->offsets[i] + 4);
    }
  }
  else{
    for(i = iframe;i < nframes;i++){
      fi->r_valptrs[i - 1] = (float *)(fi->uc_vals + fi->offsets[i]);
    }
  }
}


/* ------------------ GetFrameUCValptrs ------------------------ */

void GetFrameUCValptrs(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 1)iframe = 1;
  first_frame = iframe;
  last_frame = first_frame + nframes - 2;
  if(last_frame > fi->nframes - 1)last_frame = fi->nframes - 1;
  nframes = last_frame + 1 - first_frame;
  if(fi->file_type == FORTRAN_FILE){
    for(i = iframe;i < nframes;i++){
      fi->uc_valptrs[i - 1] = (unsigned char *)(fi->uc_valptrs[i] + 4);
    }
  }
  else{
    for(i = iframe;i < nframes;i++){
      fi->uc_valptrs[i - 1] = (unsigned char *)fi->uc_valptrs[i];
    }
  }
}

/* ------------------ GetSliceFrameInfo ------------------------ */

void GetSliceFrameInfo(char *file, char *size_file, int **framesptr, int *nframesptr, FILE_SIZE *filesizeptr){
  FILE *stream;

  int headersize, framesize, nframes, *frames;
  int ijk[6];
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  FILE_SIZE filesize;
  
  stream = fopen(file, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
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
  
  filesize = GetFileSizeSMV(file);
  nframes = 1;                                                  // header
  nframes += (int)(filesize - headersize)/framesize; // time frames
  NewMemory((void **)&frames, nframes *sizeof(int));
  int i;
  frames[0] = headersize;
  for(i=1;i< nframes;i++){
    frames[i] = framesize;
  }
  *framesptr  = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}
