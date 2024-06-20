#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"
#include "IOframe.h"
#include "file_util.h"

// the routines is this file read files that consist of a header followed by 1 or 
// more frames of data.  Each frame starts with a time value followed by a number 
// of values.  The number of values in a frame may be different for each frame.  
// The frame data structures are initialized with the FRAMEInit routine.  This 
// routine is passed a data file name, a size data file name (which may be NULL), 
// a file type parameter (C or Fortran, currently now only Fortran) and a routine 
// that determines the size of each data frame.
// 
// The FRAMEsetup routine is then called when a file is read in define the header
// and data frame sizes

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
  // fortran files contain an extra 4 bytes at the beginning and end of each record
  if(file_type != C_FILE)file_type = FORTRAN_FILE;
  strcpy(frame->file, file);
  frame->file_type    = file_type;
  frame->nframes      = 0;
  frame->headersize   = 0;
  frame->filesize     = 0;
#ifdef pp_THREAD
  frame->nthreads     = 4;
#endif
  frame->framesizes   = NULL;
  frame->offsets      = NULL;
  frame->frames       = NULL;
  frame->bufferinfo   = NULL;
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
  fi->frames       = fi->bufferinfo->buffer;
  fi->header       = fi->bufferinfo->buffer;
  if(nframes > 0){
    NewMemory((void **)&fi->offsets,    nframes*sizeof(FILE_SIZE));
    NewMemory((void **)&fi->frameptrs,  nframes*sizeof(float *));
    NewMemory((void **)&fi->times,      nframes*sizeof(float));

    fi->offsets[0] = headersize;
    for(i = 1;i < fi->nframes;i++){
      fi->offsets[i] = fi->offsets[i - 1] + fi->framesizes[i - 1];
    }
  }
}

/* ------------------ FRAMEFree ------------------------ */

void FRAMEFree(framedata *fi){
  if(fi == NULL)return;
  fi->nframes = 0;
  FREEMEMORY(fi->framesizes);
  FREEMEMORY(fi->offsets);
  FREEMEMORY(fi->frameptrs);
  FREEMEMORY(fi->times);
  FreeBufferInfo(fi->bufferinfo);
  fi->bufferinfo = NULL;
  FREEMEMORY(fi);
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

#ifdef pp_THREAD
/* ------------------ FRAMESetNThreads ------------------------ */

void FRAMESetNThreads(framedata *fi, int nthreads){
  fi->nthreads = nthreads;
}
#endif

/* ------------------ FRAMEReadFrame ------------------------ */

void FRAMEReadFrame(framedata *fi, int iframe, int nframes){
  FILE_SIZE total_size;
  int i, first_frame, last_frame;
  FILE *stream;

  stream = fopen(fi->file, "rb");
  if(stream == NULL)return;

  if(iframe < 0)iframe = 0;
  first_frame = iframe;
  last_frame  = first_frame + nframes - 1;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  
  total_size = 0;
  for(i=0;i<nframes;i++){
    total_size += fi->framesizes[iframe+i];
  }
#ifdef pp_THREAD
  INIT_PRINT_TIMER(fread_p_timer);
  fread_p(fi->file, fi->frames + fi->offsets[iframe], fi->headersize + fi->offsets[iframe], total_size, fi->nthreads);
  PRINT_TIMER(fread_p_timer, "fread_p");
#else
  FRAME_FSEEK(stream, fi->headersize+fi->offsets[iframe], SEEK_SET);
  fread(fi->frames + fi->offsets[iframe], 1, total_size, stream);
#endif
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
#ifdef pp_FRAME_DEBUG
    float time;
    time = fi->times[i];
    printf("time[%i]=%f\n", i,time);
#endif
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

//******* The following routines define header and frame sizes for each file type 
//        (3d smoke, slice, isosurface, and particle - boundary file routine not implemente)
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
  fclose(stream);

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
  int returncode;
  
  stream = fopen(file, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
  }

  headersize = 3*(4+30+4);  // 3 30 byte labels 

  FRAME_FSEEK(stream, headersize, SEEK_CUR);

  FRAME_READ(ijk, 6, stream);
  if(returncode!=6){
    fclose(stream);
    return;
  }
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

/* ------------------ GetIsoFrameInfo ------------------------ */

void GetIsoFrameInfo(char *file, char *size_file, int *headersizeptr, int **framesptr, int *nframesptr, FILE_SIZE *filesizeptr){
  FILE *stream;
  int headersize, levelsize, *frames;
  int niso_levels;
  int nframes;
  int returncode;
  FILE_SIZE filesize;

  // header
//    WRITE(LU_ISO) ONE
//    WRITE(LU_ISO) VERSION
//    WRITE(LU_ISO) NISO_LEVELS
//    IF(NISO_LEVELS > 0) WRITE(LU_ISO) (ISO_LEVELS(I), I = 1, NISO_LEVELS)
//    WRITE(LU_ISO) ZERO
//    WRITE(LU_ISO) ZERO, ZERO

  stream = fopen(file, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    return;
  }

  headersize  = 4 + 4 + 4; // ONE
  headersize += 4 + 4 + 4; // VERSION
  FRAME_FSEEK(stream, headersize, SEEK_CUR);
  FRAME_READ(&niso_levels, 1, stream);
  headersize += 4+4+4; // NISO_LEVELS

  levelsize = 0;
  if(niso_levels > 0)levelsize = 4 + niso_levels*4 + 4;

  headersize += 4 + 4 + 4; // ZERO (12 bytes)
  headersize += 4 + 8 + 4; // ZERO, ZERO (16 bytes)
  headersize += levelsize; // levels
  FRAME_FSEEK(stream, 28+levelsize, SEEK_CUR);

  nframes = 0;
  NewMemory((void **)&frames, 1000000*sizeof(int));
  while(!feof(stream)){
    //  WRITE(LU_ISO) STIME, ZERO
    //  WRITE(LU_ISO) N_VERT, N_FACE
    //  IF(N_VERT_D > 0) WRITE(LU_ISO) (Xvert(I), Yvert(I), Zvert(I), I = 1, N_VERT)
    //  IF(N_FACE_D > 0) WRITE(LU_ISO) (FACE1(I), FACE2(I), FACE3(I), I = 1, N_FACE)
    //  IF(N_FACE_D > 0) WRITE(LU_ISO) (ISO_LEVEL_INDICES(I), I = 1, N_FACE)

    int nvals[2];
    float times[2];

    FRAME_READ(times, 2, stream);if(returncode != 2)break;
    FRAME_READ(nvals, 2, stream);if(returncode != 2)break;
    int skip;

    skip = 0;
    if(nvals[0] > 0)skip += (4 + 3*nvals[0]*4 + 4);
    if(nvals[1] > 0)skip += (4 + 3 * nvals[1] * 4 + 4) + (4 + nvals[1] * 4 + 4);
    FRAME_FSEEK(stream, skip, SEEK_CUR);
    frames[nframes++] = 4 + 8 + 4 + 4 + 8 + 4 + skip;
  }
  if(nframes > 0)ResizeMemory((void **)&frames, nframes * sizeof(int));


  filesize = GetFileSizeSMV(file);

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
  fclose(stream);
}

/* ------------------ GetPartFrameInfo ------------------------ */

void GetPartFrameInfo(char *file, char *size_file, int *headersizeptr, int **framesptr, int *nframesptr, FILE_SIZE *filesizeptr){
  FILE *stream;
  FILE_SIZE filesize;
  int headersize, *frames, nframes, returncode, n_part, *nquants;

//  WRITE(LUPF) ONE_INTEGER ! Integer 1 to check Endian-ness
//  WRITE(LUPF) NINT(VERSION*100.) ! FDS version number
//  WRITE(LUPF) N_PART ! Number of PARTicle classes
//  DO N=1,N_PART
//    PC => PARTICLE_CLASS(N)
//    WRITE(LUPF) PC%N_QUANTITIES,ZERO_INTEGER ! ZERO_INTEGER is a place holder
//    DO NN=1,PC%N_QUANTITIES
//      WRITE(LUPF) CDATA(PC%QUANTITIES_INDEX(NN)) ! 30 character output quantity
//      WRITE(LUPF) UDATA(PC%QUANTITIES_INDEX(NN)) ! 30 character output units
//    ENDDO
//  ENDDO
  stream = fopen(file, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    return;
  }

  headersize  = 4 + 4 + 4; // ONE
  headersize += 4 + 4 + 4; // FDS VERSION
  
  FRAME_FSEEK(stream, headersize, SEEK_CUR);
  FRAME_READ(&n_part, 1, stream);
  NewMemory((void **)&nquants, (n_part+1)*sizeof(int));
  
  headersize += 4 + 4 + 4;
  int i;
  for(i=0; i<n_part; i++){
    int labelsize;

    FRAME_READ(nquants+i, 2, stream);
    labelsize = 2*nquants[0]*(4+30+4);
    headersize += 4 + 8 + 4 + labelsize;
    FRAME_FSEEK(stream, labelsize, SEEK_CUR);
  }

  nframes = 0;
  NewMemory((void **)&frames, 1000000 * sizeof(int));
  while(!feof(stream)){
    int framesize;
//  WRITE(LUPF) REAL(T,FB) ! Write out the time T as a 4 byte real
//  DO N=1,N_PART
//    WRITE(LUPF) NPLIM ! Number of particles in the PART class
//    WRITE(LUPF) (XP(I),I=1,NPLIM),(YP(I),I=1,NPLIM),(ZP(I),I=1,NPLIM)
//    WRITE(LUPF) (TA(I),I=1,NPLIM) ! Integer "tag" for each particle
//    IF (PC%N_QUANTITIES > 0) WRITE(LUPF) ((QP(I,NN),I=1,NPLIM),NN=1,PC%N_QUANTITIES)
//  ENDDO
    float time_arg;
    FRAME_READ(&time_arg, 1, stream);
    if(returncode != 1)break;
    framesize = 4 + 4 + 4;
    for(i=0; i<n_part; i++){
      int nplim, skip;

      FRAME_READ(&nplim, 1, stream);
      framesize += 4 + 4 + 4;             // nplim
      skip  = 4 + 3*nplim*4 + 4;          // xp, yp, zp
      skip += 4 + nplim*4 + 4;            // tag
      if(nquants[i]>0){
        skip += 4 + nplim*nquants[i]*4 + 4; // qp
      }
      framesize += skip;
      FRAME_FSEEK(stream, skip, SEEK_CUR);
    }
    frames[nframes++] = framesize;
  }
  if(nframes > 0)ResizeMemory((void **)&frames, nframes * sizeof(int));

  filesize = GetFileSizeSMV(file);

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
  fclose(stream);
}
