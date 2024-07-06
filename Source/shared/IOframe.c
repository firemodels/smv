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

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes, int **subframeptrs, int *nsubframes, FILE_SIZE *filesizeptr)){
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
  frame->frames_read  = 0;
  frame->bytes_read   = 0;
  frame->update       = 0;
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
  frame->subframeoffsets = NULL;
  frame->nsubframes = 0;
  frame->GetFrameInfo = GetFrameInfo;
  return frame;
}

/* ------------------ FRAMESetup ------------------------ */

void FRAMESetup(framedata *fi){
  int *framesizes, nframes;
  int i;
  FILE_SIZE filesize;
  int headersize;
  int *subframeoffsets, nsubframes;

  fi->GetFrameInfo(fi->bufferinfo, &headersize, &framesizes, &nframes, &subframeoffsets, &nsubframes, &filesize);
  if(nframes <= 0)return;
  fi->subframeoffsets = subframeoffsets;
  fi->nsubframes      = nsubframes;
  fi->framesizes      = framesizes;
  fi->nframes         = nframes;
  fi->filesize        = filesize;
  fi->headersize      = headersize;
  fi->bytes_read      = 0;
  fi->load_time       = 0.0;
  fi->total_time      = 0.0;
  fi->frames_read     = 0;
  fi->frames          = fi->bufferinfo->buffer;
  fi->header          = fi->bufferinfo->buffer;
  if(nframes > 0){
    NEWMEM(fi->offsets,   nframes*sizeof(FILE_SIZE));
    NEWMEM(fi->frameptrs, nframes*sizeof(float *));
    NEWMEM(fi->times,     nframes*sizeof(float));

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

int FRAMEGetMinMax(framedata *fi){
  int i;
  float valmin = 1.0, valmax = 0.0;
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
      if(valmin > valmax){
        valmin = val;
        valmax = val;
      }
      else{
        if(val < valmin)valmin = val;
        if(val > valmax)valmax = val;
      }
    }
  }
  fi->valmin = valmin;
  fi->valmax = valmax;
  return returnval;
}

#ifdef pp_THREAD
/* ------------------ FRAMESetNThreads ------------------------ */

void FRAMESetNThreads(framedata *fi, int nthreads){
  fi->nthreads = nthreads;
}
#endif

/* ------------------ FRAMEReadFrame ------------------------ */

bufferdata *FRAMEReadFrame(framedata *fi, int option, int iframe, int nframes, int *nreadptr){
  FILE_SIZE total_size, offset;
  bufferdata *bufferinfo, *bufferinfoptr;
  unsigned char *buffer;
  int i, nread, nframe_threads = 4;

  total_size = 0;
  for(i=0;i<nframes;i++){
    total_size += fi->framesizes[iframe+i];
  }
  offset = fi->offsets[iframe];
  NewMemory((void **)&bufferinfo, sizeof(bufferdata));
  NewMemory((void **)&buffer, fi->headersize+total_size);
  bufferinfo->file    = fi->file;
  bufferinfo->buffer  = buffer;
  bufferinfo->nbuffer = total_size;
  fi->frames_read     = nframes;
  fi->update          = 1;
  bufferinfoptr       = File2Buffer(fi->file, bufferinfo, option, fi->headersize, offset, total_size, nframe_threads, &nread);
  *nreadptr = nread;
  return bufferinfoptr;
}

/* ------------------ FRAMESetTimes ------------------------ */

void FRAMESetTimes(framedata *fi, int iframe, int nframes){
  int i, first_frame, last_frame;

  fi->frames = fi->bufferinfo->buffer;
  fi->header = fi->bufferinfo->buffer;
  if(iframe < 0)iframe = 0;
  if(iframe > fi->nframes - 1)iframe = fi->nframes - 1;
  first_frame = iframe;

  last_frame = first_frame + nframes - 1;
  if(last_frame>fi->nframes - 1)last_frame = fi->nframes-1;
  nframes = last_frame + 1 - first_frame;
  for(i = first_frame;i <= last_frame;i++){
    int offset;

    offset = fi->offsets[i];
    if(fi->file_type == FORTRAN_FILE)offset += 4;
    memcpy(fi->times + i - first_frame, fi->frames + offset, sizeof(float));
#ifdef pp_FRAME_DEBUG2
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

/* ------------------ FRAMEGetPtr ------------------------ */

unsigned char *FRAMEGetSubFramePtr(framedata *fi, int iframe, int isubframe){
  unsigned char *ptr;

  ptr = FRAMEGetFramePtr(fi, iframe);
  if(isubframe <0)isubframe = 0;
  if(isubframe >fi->nsubframes-1)isubframe = fi->nsubframes - 1;;
  ptr += fi->subframeoffsets[isubframe];
  return ptr;
}

//******* The following routines define header and frame sizes for each file type 
//        (3d smoke, slice, isosurface, and particle - boundary file routine not implemente)
/* ------------------ GetSmoke3DFrameInfo ------------------------ */

void GetSmoke3DFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                         int **subframeoffsetsptr, int *nsubframesptr, FILE_SIZE *filesizeptr){
  FILE *stream;
  char buffer[255];
  int headersize, nframes, *frames;
  FILE_SIZE filesize;
  char sizefile[1024];

  strcpy(sizefile, bufferinfo->file);
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
  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}
  
/* ------------------ GetBoundaryFrameInfo ------------------------ */

void GetBoundaryFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, 
  int **subframeoffsetsptr, int *nsubframesptr, FILE_SIZE *filesizeptr){
  FILE *stream;

  int headersize, framesize, nframes, *frames, datasize;
  FILE_SIZE filesize;
  int npatch, i;
  int *subframeoffsets=NULL, nsubframes=0;

  stream = fopen(bufferinfo->file, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    *filesizeptr = 0;
  }

  // header
  // WRITE(LUBF) QUANTITY   (30 chars)
  // WRITE(LUBF) SHORT_NAME (30 chars)
  // WRITE(LUBF) UNITS      (30 chars)
  // WRITE(LUBF) NPATCH
  // WRITE(LUBF) I1, I2, J1, J2, K1, K2, IOR, NB, NM  (NPATCH entries)
  // WRITE(LUBF) I1, I2, J1, J2, K1, K2, IOR, NB, NM

  headersize = 3 * (4 + 30 + 4);            // QUANTITY, SHORT_NAME, UNITS
  FSEEK(stream, headersize, SEEK_SET);

  FSEEK(stream, 4, SEEK_CUR);fread(&npatch, sizeof(int), 1, stream);FSEEK(stream, 4, SEEK_CUR);
  headersize += (4 + sizeof(int) + 4);       // NPATCH

  nsubframes = npatch;
  NewMemory(( void ** )&subframeoffsets, nsubframes*sizeof(int));
  subframeoffsets[0] = 0;
  datasize = 0;
  for(i = 0;i < npatch;i++){
    int parms[9], ncells;

    FSEEK(stream, 4, SEEK_CUR);fread(parms, sizeof(int), 9, stream);FSEEK(stream, 4, SEEK_CUR);
    ncells  = (parms[1] + 1 - parms[0]);
    ncells *= (parms[3] + 1 - parms[2]);
    ncells *= (parms[5] + 1 - parms[4]);
    datasize += (4 + ncells* sizeof(float) + 4);
    if(i <npatch-1)subframeoffsets[i+1] = subframeoffsets[i] + (4 + ncells * sizeof(float) + 4);
  }
  headersize += npatch*(4 + 9 * sizeof(int) + 4); // I1, I2, J1, J2, K1, K2, IOR, NB, NM

  // frame
  // WRITE(LUBF) TIME
  // WRITE(LUBF) (((QQ(I, J, K), I = 11, I2), J = J1, J2), K = K1, K2) (NPATH entries)
  framesize = 4 + sizeof(float) + 4; // time
  framesize += datasize;
  fclose(stream);

  filesize = GetFileSizeSMV(bufferinfo->file);
  nframes = (filesize - headersize) / framesize;

  NewMemory((void **)&frames, nframes * sizeof(int));
  for(i = 0;i < nframes;i++){
    frames[i] = framesize;
  }
  *headersizeptr      = headersize;
  *framesptr          = frames;
  *nframesptr         = nframes;
  *filesizeptr        = filesize;
  *subframeoffsetsptr = subframeoffsets;
  *nsubframesptr      = nsubframes;
}

  /* ------------------ GetSliceFrameInfo ------------------------ */

void GetSliceFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                       int **subframeoffsetsptr, int *nsubframesptr, FILE_SIZE *filesizeptr){
  FILE_m *stream;

  int headersize, framesize, nframes, *frames;
  int ijk[6];
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  FILE_SIZE filesize;
  int returncode;
  
  stream = fopen_b(bufferinfo->file, bufferinfo->buffer, bufferinfo->nbuffer, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
  }

  headersize = 3*(4+30+4);  // 3 30 byte labels 
  FRAME_FSEEK(stream, headersize, SEEK_CUR);

  FRAME_READ(ijk, 6, stream);
  if(returncode!=6){
//    fclose(stream);
    return;
  }
  //fclose(stream);

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
  
  filesize = bufferinfo->nbuffer;
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

void GetIsoFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                     int **subframeoffsetsptr, int *nsubframesptr, FILE_SIZE *filesizeptr){
  FILE_m *stream;
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


  stream = fopen_b(bufferinfo->file, NULL, 0, "rb");
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
  while(!feof_m(stream)){
    //  WRITE(LU_ISO) STIME, ZERO
    //  WRITE(LU_ISO) N_VERT, N_FACE
    //  IF(N_VERT_D > 0) WRITE(LU_ISO) (Xvert(I), Yvert(I), Zvert(I), I = 1, N_VERT)
    //  IF(N_FACE_D > 0) WRITE(LU_ISO) (FACE1(I), FACE2(I), FACE3(I), I = 1, N_FACE)
    //  IF(N_FACE_D > 0) WRITE(LU_ISO) (ISO_LEVEL_INDICES(I), I = 1, N_FACE)

    int nvals[2];
    float times[2];

    FRAME_READ(times, 2, stream);
    if(returncode != 2)break;
    FRAME_READ(nvals, 2, stream);
    if(returncode != 2)break;

    int skip;

    skip = 0;
    if(nvals[0] > 0)skip += (4 + 3*nvals[0]*4 + 4);
    if(nvals[1] > 0)skip += (4 + 3 * nvals[1] * 4 + 4) + (4 + nvals[1] * 4 + 4);
    FRAME_FSEEK(stream, skip, SEEK_CUR);
    frames[nframes++] = 4 + 8 + 4 + 4 + 8 + 4 + skip;
  }
  if(nframes > 0)ResizeMemory((void **)&frames, nframes * sizeof(int));


  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
  //fclose(stream);
}

/* ------------------ GetPartFrameInfo ------------------------ */

void GetPartFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                      int **subframeoffsetsptr, int *nsubframesptr, FILE_SIZE *filesizeptr){
  FILE_m *stream;
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
  stream = fopen_b(bufferinfo->file, bufferinfo->buffer, bufferinfo->nbuffer, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    return;
  }

  headersize  = 4 + 4 + 4; // ONE
  headersize += 4 + 4 + 4; // FDS VERSION
  FRAME_FSEEK(stream, headersize, SEEK_CUR);

  FRAME_READ(&n_part, 1, stream);
  headersize += 4 + 4 + 4; // n_part

  NewMemory((void **)&nquants, 2*n_part*sizeof(int));
  
  int i;
  for(i=0; i<n_part; i++){
    int labelsize;

    FRAME_READ(nquants+2*i, 2, stream);
    headersize += 4 + 2*4 + 4;
    
    labelsize = 2*nquants[2*i]*(4+30+4);
    headersize += labelsize;
    FRAME_FSEEK(stream, labelsize, SEEK_CUR);
  }

  nframes = 0;
  NewMemory((void **)&frames, 1000000 * sizeof(int));
  while(!feof_m(stream)){
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
    framesize = 4 + 4 + 4;

//    printf("time_arg=%f\n", time_arg);
    if(returncode != 1)break;
    for(i=0; i<n_part; i++){
      int nplim;
      long int skip;

      FRAME_READ(&nplim, 1, stream);
      framesize += 4 + 4 + 4;             // nplim

//      printf("nplim %i: %i\n",i, nplim);
      skip  = 4 + 3*nplim*sizeof(float) + 4;          // xp, yp, zp
      skip += 4 +   nplim*sizeof(int)   + 4;          // tag
      if(nquants[2*i] > 0){
        skip += 4 + nplim*nquants[2*i]*sizeof(float) + 4; // qp
      }
      framesize += skip;
      FRAME_FSEEK(stream, skip, SEEK_CUR);
    }
    frames[nframes++] = framesize;
  }
  if(nframes > 0)ResizeMemory((void **)&frames, nframes * sizeof(int));

  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}
