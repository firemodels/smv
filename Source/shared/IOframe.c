#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"
#include "IOframe.h"
#include "file_util.h"

  /* ------------------ FRAMEInit ------------------------ */

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesizeptr)){
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
    strcpy(frame->size_file, size_file);
  }
  else{
    frame->size_file=NULL;
  }
  if(file_type != C_FILE)file_type = FORTRAN_FILE;
  strcpy(frame->file, file);
  frame->file_type    = file_type;
  frame->nframes      = 0;
  frame->headersize   = 0;
  frame->filesize     = 0;
  frame->framesizes   = NULL;
  frame->offsets      = NULL;
  frame->frames       = NULL;
  frame->frameptrs    = NULL;
  frame->times        = NULL;
  frame->header       = NULL;
  frame->GetFrameInfo = GetFrameInfo;
  return frame;
}

/* ------------------ FRAMESetup ------------------------ */

void FRAMESetup(framedata *fi){
  int *framesizes, nframes;
  int i;
  FILE_SIZE filesize;
  int headersize;

  fi->GetFrameInfo(fi->file, fi->size_file, &headersize, &framesizes, &nframes, &filesize);
  if(nframes <= 0)return;
  fi->framesizes   = framesizes;
  fi->nframes      = nframes;
  fi->filesize     = filesize;
  fi->headersize   = headersize;
  if(nframes > 0){
    NewMemory((void **)&fi->header,     headersize*sizeof(unsigned char));
    NewMemory((void **)&fi->offsets,    nframes*sizeof(FILE_SIZE));
    NewMemory((void **)&fi->frameptrs,  nframes*sizeof(float *));
    NewMemory((void **)&fi->times,      nframes*sizeof(float));
    NewMemory((void **)&fi->frames,     fi->filesize);

    fi->offsets[0] = 0;
    for(i = 1;i < fi->nframes;i++){
      fi->offsets[i] = fi->offsets[i - 1] + fi->framesizes[i - 1];
    }
  }
}

/* ------------------ FRAMEFree ------------------------ */

void FRAMEFree(framedata *fi){
  fi->nframes = 0;
  FREEMEMORY(fi->framesizes);
  FREEMEMORY(fi->frames);
  FREEMEMORY(fi->offsets);
  FREEMEMORY(fi->frameptrs);
  FREEMEMORY(fi->times);
  FREEMEMORY(fi->header);
}

/* ------------------ FRAMEGetMinMax ------------------------ */

int FRAMEGetMinMax(framedata *fi, float *valmin, float *valmax){
  int i;
  float vmin = 1.0, vmax = 0.0;
  int returnval = 0, nvals;

  for(i = 0;i < fi->nframes;i++){
    int j;
    float *rvals;

    rvals = (float *)fi->frameptrs[i];
    if(rvals == NULL||fi->framesizes[i]==0)continue;
    returnval = 1;
    nvals = (fi->framesizes[i] - 20) / 4;
    for(j = 0;j < nvals;j++){
      float val;
      
      val = rvals[j];
      if(vmin > vmax){
        vmin = val;
        vmax = val;
      }
      else{
        if(val < vmin)vmin = val;
        if(val > vmax)vmax = val;
      }
    }
  }
  *valmin = vmin;
  *valmax = vmax;
  return returnval;
}

/* ------------------ FRAMEReadHeader ------------------------ */

void FRAMEReadHeader(framedata *fi){
  FILE *stream;

  if(fi->headersize == 0)return;
  stream = fopen(fi->file, "rb");
  if(stream == NULL)return;
  fread(fi->header, 1, fi->headersize, stream);
  fclose(stream);
}

/* ------------------ FRAMEReadFrame ------------------------ */

void FRAMEReadFrame(framedata *fi, int iframe, int nframes){
  FILE_SIZE total_size;
  int i, first_frame, last_frame;
  FILE *stream;
  float time0;

  stream = fopen(fi->file, "rb");
  if(stream == NULL)return;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  
  total_size = 0;
  for(i=0;i<nframes;i++){
    total_size += fi->framesizes[iframe+i];
  }
  fseek(stream, fi->headersize+fi->offsets[iframe], SEEK_SET);
  fread(fi->frames + fi->offsets[iframe], 1, total_size, stream);
  time0 = (float)*(fi->frames + 4);
  fclose(stream);
}

/* ------------------ FRAMESetTimes ------------------------ */

void FRAMESetTimes(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  for(i = iframe;i < nframes;i++){
    int offset;

    offset = fi->offsets[i];
    if(fi->file_type == FORTRAN_FILE)offset += 4;
    memcpy(fi->times + i, fi->frames + offset, sizeof(float));
  }
}

/* ------------------ FRAMESetFramePtrs ------------------------ */

void FRAMESetFramePtrs(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame = first_frame + nframes - 1;
  if(last_frame > fi->nframes - 1)last_frame = fi->nframes - 1;
  nframes = last_frame + 1 - first_frame;
  if(fi->file_type == FORTRAN_FILE){
    for(i = iframe;i < nframes;i++){
      fi->frameptrs[i] = (fi->frames+fi->offsets[i] + 16);
    }
  }
  else{
    for(i = iframe;i < nframes;i++){
      fi->frameptrs[i] = (fi->frames+fi->offsets[i]);
    }
  }
}

/* ------------------ FRAMEGetPtr ------------------------ */

unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe){
  if(iframe < 0)iframe = 0;
  if(iframe > fi->nframes-1)iframe = fi->nframes-1;
  return fi->frameptrs[iframe];
}


/* ------------------ GetSmoke3DFrameInfo ------------------------ */

void GetSmoke3DFrameInfo(char *file, char *size_file, int *headersizeptr, int **framesptr, int *nframesptr, FILE_SIZE *filesizeptr){
  FILE *stream;
  char buffer[255];
  int headersize, nframes, *frames;
  FILE_SIZE filesize;
  char sizefile[1024];

  strcpy(sizefile, file);
  strcat(sizefile, ".sz");

  stream = fopen(sizefile, "r");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    return;
  }

  fgets(buffer, 255, stream);
  nframes=0;
  while(!feof(stream)){
    if(fgets(buffer, 255, stream) == NULL)break;
    nframes++;
  }

  NewMemory((void **)&frames, nframes *sizeof(int));

  rewind(stream);
  fgets(buffer, 255, stream);
  nframes=0;
  while(!feof(stream)){
    int nchars, nchars_compressed;
    float time_local;
    int framesize;

    //    WRITE(LU_SMOKE3D) TIME
    //    WRITE(LU_SMOKE3D) NCHARS_IN, NCHARS_OUT
    //    IF(NCHARS_OUT > 0) WRITE(LU_SMOKE3D)(BUFFER_OUT(I), I = 1, NCHARS_OUT)
    if(fgets(buffer, 255, stream) == NULL)break;
    sscanf(buffer, "%f %i %i", &time_local, &nchars, &nchars_compressed);
    framesize = 4 + 4 + 4;
    framesize += 4 + 8 + 4;
    if(nchars_compressed > 0)framesize += 4 + nchars_compressed + 4;
    frames[nframes++] = framesize;
  }

  headersize = 40;
  filesize = GetFileSizeSMV(file);

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}
  
  /* ------------------ GetSliceFrameInfo ------------------------ */

void GetSliceFrameInfo(char *file, char *size_file, int *headersizeptr, int **framesptr, int *nframesptr, FILE_SIZE *filesizeptr){
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
  nframes = (int)(filesize - headersize)/framesize; // time frames
  NewMemory((void **)&frames, nframes *sizeof(int));
  int i;
  for(i=0;i< nframes;i++){
    frames[i] = framesize;
  }
  *headersizeptr = headersize;
  *framesptr  = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}
