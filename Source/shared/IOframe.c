#include "options.h"
#ifdef pp_FRAME
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "dmalloc.h"
#include "IOframe.h"
#include "file_util.h"

// The routines is this file read data files consisting of a header followed by one or
// more data frames.  Each data frame starts with a time value followed by a number
// data of values.  The number of values in a frame may vary.  The frame data structures
// are initialized using FRAMEInit.  This routine is passed a file name,  a file type
// parameter (C or Fortran) and a routine that determines the size of each data frame.
// FDS generated files use the Fortran file type. Each Fortran generated data record
// is prefixed and suffixed with four bytes. Files compressed with smokezip use the
// C file type. The FRAMEsetup routine is then called when a file is read in define the
// header and data frame sizes.  These operations are consolidated by the FRAMELoadFrame routine.

  /* ------------------ FRAMEInit ------------------------ */

framedata *FRAMEInit(char *file, int file_type, void GetFrameInfo(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes,
                     int **subframeptrs, int **subframessizes, int *nsubframes,
                     int *compression_type, FILE_SIZE *filesizeptr)){
  framedata *frame=NULL;

  NewMemory((void **)&frame, sizeof(framedata));
  if(file != NULL && strlen(file) > 0){
    NewMemory((void **)&frame->file, strlen(file) + 1);
  }
  else{
    FREEMEMORY(frame);
    return NULL;
  }
  // fortran files contain an extra 4 bytes at the beginning and end of each record
  if(file_type != C_FILE)file_type = FORTRAN_FILE;
  strcpy(frame->file, file);
  frame->file_type    = file_type;
  frame->compression_type  = FRAME_UNCOMPRESSED;
  frame->nframes      = 0;
  frame->frames_read  = 0;
  frame->bytes_read   = 0;
  frame->update       = 0;
  frame->headersize   = 0;
  frame->filesize     = 0;
  frame->framesizes   = NULL;
  frame->offsets      = NULL;
  frame->frames       = NULL;
  frame->bufferinfo   = NULL;
  frame->frameptrs    = NULL;
  frame->times        = NULL;
  frame->header       = NULL;
  frame->subframeoffsets = NULL;
  frame->subframesizes = NULL;
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
  int *subframeoffsets=NULL, *subframesizes=NULL, nsubframes=0;

  fi->GetFrameInfo(fi->bufferinfo, &headersize, &framesizes, &nframes,
                   &subframeoffsets, &subframesizes, &nsubframes,
                   &fi->compression_type, &filesize);
  if(nframes <= 0)return;
  if(fi->compression_type == FRAME_ZLIB)fi->file_type = C_FILE;
  fi->subframeoffsets = subframeoffsets;
  fi->subframesizes   = subframesizes;
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
  int returnval = 0;

  for(i=0; i<fi->nframes; i++){
    int j;
    float *rvals;

    for(j=0; j<fi->nsubframes; j++){
      int k;

      rvals = (float *)FRAMEGetSubFramePtr(fi, i, j);
      if(rvals == NULL || fi->framesizes[i] == 0)continue;
      returnval = 1;
      for(k = 0;k < fi->subframesizes[j];k++){
        float val;

        val = rvals[k];
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
  }
  fi->valmin = valmin;
  fi->valmax = valmax;
  return returnval;
}

/* ------------------ FRAMEReadFrame ------------------------ */

bufferdata *FRAMEReadFrame(framedata *fi, int iframe, int nframes, int *nreadptr){
  FILE_SIZE total_size;
  bufferdata *bufferinfo;
  unsigned char *buffer;
  int i, nread;
  FILE *stream;

  if(fi == NULL)return NULL;
  stream = fopen(fi->file, "rb");
  if(stream == NULL)return NULL;

  total_size = 0;
  for(i=0;i<nframes;i++){
    total_size += fi->framesizes[iframe+i];
  }

  NewMemory((void **)&bufferinfo, sizeof(bufferdata));
  NewMemory((void **)&buffer,     fi->headersize+total_size);
  bufferinfo->file    = fi->file;
  bufferinfo->buffer  = buffer;
  bufferinfo->nbuffer = total_size;
  fi->frames_read     = nframes;
  fi->update          = 1;

  fread(buffer, 1, fi->headersize, stream);
  fseek(stream, fi->offsets[iframe], SEEK_SET);
  nread = fread(buffer+fi->headersize, 1, total_size, stream);
  bufferinfo->nfile  = bufferinfo->nbuffer;
  *nreadptr = nread;
  return bufferinfo;
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
    FILE_SIZE offset;

    offset = fi->offsets[i];
    if(fi->file_type == FORTRAN_FILE)offset += 4;
    memcpy(fi->times + i - first_frame, fi->frames + offset, sizeof(float));
#ifdef pp_FRAME_DEBUG2
    float time;
    time = fi->times[i];
    fprintf(stderr, "time[%i]=%f\n", i,time);
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
  if(fi == NULL)return NULL;
  if(iframe < 0)iframe = 0;
  if(iframe > fi->nframes-1)iframe = fi->nframes-1;
  return fi->frameptrs[iframe];
}

/* ------------------ FRAMEGetSubFramePtr ------------------------ */

unsigned char *FRAMEGetSubFramePtr(framedata *fi, int iframe, int isubframe){
  unsigned char *ptr;

  ptr = FRAMEGetFramePtr(fi, iframe);
  if(ptr == NULL)return NULL;
  if(isubframe <0)isubframe = 0;
  if(isubframe >fi->nsubframes-1)isubframe = fi->nsubframes - 1;;
  ptr += fi->subframeoffsets[isubframe];
  return ptr;
}

/* ------------------ FRAMELoadData ------------------------ */

framedata *FRAMELoadData(framedata *frameinfo, char *file, char *size_file, int *options, int load_flag, int time_frame, int file_type,
                  void GetFrameInfo(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes,
                                    int **subframeptrs, int **subframesizesptr, int *nsubframes,
                                    int *compression_type, FILE_SIZE *filesizeptr)){
  int nframes_before, nframes_after;
  FILE_SIZE nread;
  float load_time;

  if(file_type != C_FILE)file_type = FORTRAN_FILE;
  if(frameinfo == NULL)frameinfo = FRAMEInit(file, file_type, GetFrameInfo);

  START_TIMER(load_time);
  if(frameinfo->bufferinfo == NULL || load_flag != RELOAD){
    if(frameinfo->bufferinfo!=NULL){
      FreeBufferInfo(frameinfo->bufferinfo);
      frameinfo->bufferinfo = NULL;
    }
    frameinfo->bufferinfo = InitBufferData(file, size_file, options);
  }
  nframes_before = frameinfo->nframes;
  frameinfo->bufferinfo = File2Buffer(file, size_file, options, frameinfo->bufferinfo, &nread);
  FRAMESetup(frameinfo);
  frameinfo->bytes_read = nread;
  if(nread > 0){
    FRAMESetTimes(    frameinfo, 0, frameinfo->nframes);
    FRAMESetFramePtrs(frameinfo, 0, frameinfo->nframes);
  }
  frameinfo->update = 1;
  nframes_after = frameinfo->nframes;
  if(time_frame == ALL_FRAMES)frameinfo->frames_read = nframes_after - nframes_before;
  STOP_TIMER(load_time);
  if(frameinfo != NULL)frameinfo->load_time = load_time;
  return frameinfo;
}

/* ------------------ FRAMEGetNFrames ------------------------ */
// only used for FRAME_PART
// when used for other types, needs to work when files are compressed
int FRAMEGetNFrames(char *file, int type){
  framedata *frameinfo = NULL;
  int nframes = 0;

  if(file == NULL || FileExistsOrig(file) == 0)return 0;
  switch(type){
  case FRAME_3DSMOKE:
    frameinfo = FRAMEInit(file, FORTRAN_FILE, GetSmoke3DFrameInfo);
    break;
  case FRAME_BOUNDARY:
    frameinfo = FRAMEInit(file, FORTRAN_FILE, GetBoundaryFrameInfo);
    break;
  case FRAME_PART:
    frameinfo = FRAMEInit(file, FORTRAN_FILE, GetPartFrameInfo);
    break;
  case FRAME_ISO:
    frameinfo = FRAMEInit(file, FORTRAN_FILE, GetIsoFrameInfo);
    break;
  case FRAME_SLICE:
    frameinfo = FRAMEInit(file, FORTRAN_FILE, GetSliceFrameInfo);
    break;
  default:
    assert(0);
    return 0;
  }
  if(frameinfo != NULL){
    NewMemory((void **)&frameinfo->bufferinfo, sizeof(bufferdata));
    frameinfo->bufferinfo->file = file;
    frameinfo->bufferinfo->buffer = NULL;
    frameinfo->bufferinfo->nbuffer = 0;
    frameinfo->bufferinfo->options = NULL;
    FRAMESetup(frameinfo);
    nframes = frameinfo->nframes;
    FRAMEFree(frameinfo);
  }
  return nframes;
}

//******* The following routines define header and frame sizes for each file type
//        (3d smoke, slice, isosurface, and particle - boundary file routine not implemente)
/* ------------------ GetSmoke3DFrameInfo ------------------------ */

void GetSmoke3DFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                         int **subframeoffsetsptr, int **subframesizesptr, int *nsubframesptr,
                         int *compression_type,
                         FILE_SIZE *filesizeptr){
  FILE *stream;
  char buffer[255];
  int headersize, nframes, *frames;
  FILE_SIZE filesize;

  *compression_type = FRAME_RLE;
  if(bufferinfo == NULL)return;

  stream = fopen(bufferinfo->file, "rb");
  if(stream == NULL)return;
  fclose(stream);

  stream = fopen(bufferinfo->size_file, "r");
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
  if(*compression_type == FRAME_RLE){
    headersize = 40;
  }
  else{
    headersize = 32;
  }
  while(!feof(stream)){
    int nchars, nchars_compressed;
    float time_local;
    int framesize;

    //    WRITE(LU_SMOKE3D) TIME
    //    WRITE(LU_SMOKE3D) NCHARS_IN, NCHARS_OUT
    //    IF(NCHARS_OUT > 0) WRITE(LU_SMOKE3D)(BUFFER_OUT(I), I = 1, NCHARS_OUT)
    if(fgets(buffer, 255, stream) == NULL)break;
    if(*compression_type == FRAME_RLE){
      if(bufferinfo->options==NULL||bufferinfo->options[1]==0){
        sscanf(buffer, "%f %i %i", &time_local, &nchars, &nchars_compressed);
      }
      else{
        float rval;
        int ival;
        sscanf(buffer, "%f %i %i %f %i", &time_local, &nchars, &ival, &rval, &nchars_compressed);
      }
      framesize = 4 + 4 + 4;
      framesize += 4 + 8 + 4;
      if(nchars_compressed > 0)framesize += 4 + nchars_compressed + 4;
    }
    else{
      int dummy;

      sscanf(buffer, "%f %i %i %i", &time_local, &nchars, &dummy, &nchars_compressed);
      framesize = 4;
      framesize += 8;
      if(nchars_compressed > 0)framesize += nchars_compressed;
    }
    frames[nframes++] = framesize;
  }
  fclose(stream);

  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}

/* ------------------ GetBoundaryFrameInfo ------------------------ */

void GetBoundaryFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                          int **subframeoffsetsptr, int **subframesizesptr, int *nsubframesptr,
                          int *compression_type, FILE_SIZE *filesizeptr){
  FILE *stream;

  int headersize, framesize, nframes, *frames, datasize;
  FILE_SIZE filesize;
  int npatch, i;
  int *subframeoffsets=NULL, *subframesizes=NULL, nsubframes=0;
  char *ext;

  ext = strrchr(bufferinfo->file, '.');
  if(ext != NULL && strcmp(ext, ".svz") == 0){
    *compression_type = FRAME_ZLIB;
  }

  stream = fopen(bufferinfo->file, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    *filesizeptr = 0;
  }

  // uncompressed format
  // header
  // WRITE(LUBF) QUANTITY   (30 chars)
  // WRITE(LUBF) SHORT_NAME (30 chars)
  // WRITE(LUBF) UNITS      (30 chars)
  // WRITE(LUBF) NPATCH
  // WRITE(LUBF) I1, I2, J1, J2, K1, K2, IOR, NB, NM  (NPATCH entries)
  // WRITE(LUBF) I1, I2, J1, J2, K1, K2, IOR, NB, NM

  // compressed format
  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version  (bndf version)
  // global min max (used to perform conversion)
  // local min max  (min max found for this file)
  // npatch
  // i1,i2,j1,j2,k1,k2,idir,dummy,dummy (npatch times)



  if(*compression_type == FRAME_ZLIB){
    headersize = 32;            // QUANTITY, SHORT_NAME, UNITS
  }
  else{
    headersize = 3 * (4 + 30 + 4);            // QUANTITY, SHORT_NAME, UNITS
  }
  FSEEK(stream, headersize, SEEK_SET);


  if(*compression_type == FRAME_ZLIB){
    headersize += 4;
    fread(&npatch, sizeof(int), 1, stream);
  }
  else{
    FSEEK(stream, 4, SEEK_CUR);fread(&npatch, sizeof(int), 1, stream);FSEEK(stream, 4, SEEK_CUR);
    headersize += (4 + sizeof(int) + 4);       // NPATCH
  }


  nsubframes = npatch;
  NewMemory((void **)&subframeoffsets, nsubframes*sizeof(int));
  NewMemory((void **)&subframesizes,     nsubframes*sizeof(int));
  subframeoffsets[0] = 0;
  datasize = 0;
  for(i = 0;i < npatch;i++){
    int parms[9], ncells;

    if(*compression_type == FRAME_ZLIB){
      fread(parms, sizeof(int), 9, stream);
    }
    else{
      FSEEK(stream, 4, SEEK_CUR);fread(parms, sizeof(int), 9, stream);FSEEK(stream, 4, SEEK_CUR);
    }
    ncells  = (parms[1] + 1 - parms[0]);
    ncells *= (parms[3] + 1 - parms[2]);
    ncells *= (parms[5] + 1 - parms[4]);
    datasize += (4 + ncells* sizeof(float) + 4);
    if(i <npatch-1)subframeoffsets[i+1] = subframeoffsets[i] + (4 + ncells*sizeof(float) + 4);
    subframesizes[i] = ncells;
  }
  if(*compression_type == FRAME_ZLIB){
    headersize += npatch*9*sizeof(int); // I1, I2, J1, J2, K1, K2, IOR, NB, NM
  }
  else{
    headersize += npatch * (4 + 9 * sizeof(int) + 4); // I1, I2, J1, J2, K1, K2, IOR, NB, NM
  }


  if(*compression_type == FRAME_ZLIB){
    // compressed format
    // time_local
    // compressed size of frame
    // compressed buffer
    NewMemory((void **)&frames, 1000000 * sizeof(int));
    nframes=0;
    for(;;){
      int nchars;
      float time;
      int count, error;

      count = fread(&time,   4, 1, stream);    if(count!=1)break;
      count = fread(&nchars, 4, 1, stream);    if(count!=1)break;
      error = FSEEK(stream, nchars, SEEK_CUR); if(error != 0)break;
      frames[nframes++]=nchars+8;
    }
    if(nframes > 0){
      ResizeMemory((void **)&frames, nframes * sizeof(int));
    }
    else{
      FREEMEMORY(frames);
    }
    filesize = GetFileSizeSMV(bufferinfo->file);
  }
  else{
    // UNCOMPRESSED format
    // frame
    // WRITE(LUBF) TIME
    // WRITE(LUBF) (((QQ(I, J, K), I = 11, I2), J = J1, J2), K = K1, K2) (NPATH entries)
    framesize = 4 + sizeof(float) + 4; // time
    framesize += datasize;

    filesize = GetFileSizeSMV(bufferinfo->file);
    nframes = (filesize - headersize) / framesize;

    NewMemory((void **)&frames, nframes * sizeof(int));
    for(i = 0;i < nframes;i++){
      frames[i] = framesize;
    }
  }
  fclose(stream);
  *headersizeptr      = headersize;
  *framesptr          = frames;
  *nframesptr         = nframes;
  *filesizeptr        = filesize;
  *subframeoffsetsptr = subframeoffsets;
  *subframesizesptr   = subframesizes;
  *nsubframesptr      = nsubframes;
}

  /* ------------------ GetSliceFrameInfo ------------------------ */

void GetSliceFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                       int **subframeoffsetsptr, int **subframesizesptr, int *nsubframesptr,
                       int *compression_type, FILE_SIZE *filesizeptr){
  FILE_m *stream;

  int headersize, framesize, nframes, *frames;
  int ijk[6];
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  FILE_SIZE filesize;
  int returncode;
  int *subframeoffsets = NULL, *subframesizes = NULL;
  int nsubframes;

  nsubframes = 1;
  NewMemory((void **)&subframeoffsets, nsubframes * sizeof(int));
  NewMemory((void **)&subframesizes,   nsubframes * sizeof(int));
  subframeoffsets[0] = 0;

  stream = fopen_b(bufferinfo->file, bufferinfo->buffer, bufferinfo->nbuffer, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
  }

  headersize = 3*(4+30+4);  // 3 30 byte labels
  fseek_m(stream, headersize, SEEK_CUR);

  fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(ijk, 4, 6, stream);fseek_m(stream, 4, SEEK_CUR);

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
  subframesizes[0] = nxsp * nysp * nzsp;

  filesize = bufferinfo->nbuffer;
  nframes = (FILE_SIZE)(filesize - headersize)/framesize; // time frames
  NewMemory((void **)&frames, nframes *sizeof(int));
  int i;
  for(i=0;i< nframes;i++){
    frames[i] = framesize;
  }
  *headersizeptr      = headersize;
  *framesptr          = frames;
  *nframesptr         = nframes;
  *filesizeptr        = filesize;
  *subframeoffsetsptr = subframeoffsets;
  *subframesizesptr   = subframesizes;
  *nsubframesptr      = nsubframes;
}

/* ------------------ GetIsoFrameInfo ------------------------ */

void GetIsoFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                     int **subframeoffsetsptr, int **subframesizesptr, int *nsubframesptr,
                     int *compression_type, FILE_SIZE *filesizeptr){
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


  stream = fopen_b(bufferinfo->file, bufferinfo->buffer, bufferinfo->nbuffer, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    return;
  }

  headersize  = 4 + 4 + 4; // ONE
  headersize += 4 + 4 + 4; // VERSION
  fseek_m(stream, headersize, SEEK_CUR);
  fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(&niso_levels, 4, 1, stream);fseek_m(stream, 4, SEEK_CUR);
  headersize += 4+4+4; // NISO_LEVELS

  levelsize = 0;
  if(niso_levels > 0)levelsize = 4 + niso_levels*4 + 4;

  headersize += 4 + 4 + 4; // ZERO (12 bytes)
  headersize += 4 + 8 + 4; // ZERO, ZERO (16 bytes)
  headersize += levelsize; // levels
  fseek_m(stream, 28+levelsize, SEEK_CUR);

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

    fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(times, 4, 2, stream);fseek_m(stream, 4, SEEK_CUR);
    if(returncode != 2)break;
    fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(nvals, 4, 2, stream);fseek_m(stream, 4, SEEK_CUR);
    if(returncode != 2)break;

    int skip;

    skip = 0;
    if(nvals[0] > 0)skip += (4 + 3*nvals[0]*4 + 4);
    if(nvals[1] > 0)skip += (4 + 3 * nvals[1] * 4 + 4) + (4 + nvals[1] * 4 + 4);
    fseek_m(stream, skip, SEEK_CUR);
    frames[nframes++] = 4 + 8 + 4 + 4 + 8 + 4 + skip;
  }
  if(nframes > 0){
    ResizeMemory((void **)&frames, nframes * sizeof(int));
  }
  else{
    FREEMEMORY(frames);
  }


  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}

/* ------------------ GetGeomDataFrameInfo ------------------------ */

void GetGeomDataFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
  int **subframeoffsetsptr, int **subframesizesptr, int *nsubframesptr,
  int *compression_type, FILE_SIZE *filesizeptr){
  FILE_m *stream;
  int headersize, *frames;
  int nframes;
  FILE_SIZE filesize;
  char *ext=NULL;

  // uncompressed header
  // 1
  // version
  // compressed header
  // 1
  // completion (0/1)
  // fileversion (compressed format)
  // min max (used to perform conversion)

  ext = strrchr(bufferinfo->file, '.');
  if(ext != NULL && strcmp(ext, ".svz") == 0){
    *compression_type = FRAME_ZLIB;
  }

  stream = fopen_b(bufferinfo->file, bufferinfo->buffer, bufferinfo->nbuffer, "rb");
  if(stream == NULL){
    *nframesptr = 0;
    *framesptr = NULL;
    return;
  }

  if(*compression_type != FRAME_ZLIB){
    headersize  = 4+4+4;  // 1
    headersize += 4+4+4;  // version
  }
  else{
    headersize  = 4;  // 1
    headersize += 4;  // complesion
    headersize += 4;  // compressed format
    headersize += 8;  // min max
  }

  // uncompressed data frame
  // for each time step:
  // time
  // nvert_static, ntri_static, nvert_dynamic, ntri_dynamic
  // if(nvert_static>0) vals_1, ...vals_nvert_static
  // if(ntri_static>0)  vals_1, ...vals_ntri_static
  // if(nvert_dynamic>0)vals_1, ...vals_nvert_dynamic
  // if(ntri_dynamic>0) vals_1, ...vals_ntri_dynamic
  // compressed data frame
  // for each time step
  // time
  // ncompressed
  // compressed_1,...,compressed_ncompressed


  fseek_m(stream, headersize, SEEK_CUR);
  NewMemory((void **)&frames, 1000000 * sizeof(int));
  nframes = 0;
  while(!feof_m(stream)){
    int nvals[4];
    int skip;

    if(*compression_type != FRAME_ZLIB){
      fseek_m(stream, 12, SEEK_CUR); // time 12 bytes
      fseek_m(stream, 4, SEEK_CUR); fread_m(nvals, 4, 4, stream); fseek_m(stream, 4, SEEK_CUR); // nvals 24 bytes
      skip = 0;
      if(nvals[0] > 0)skip += 4 + 4*nvals[0] + 4;
      if(nvals[1] > 0)skip += 4 + 4*nvals[1] + 4;
      if(nvals[2] > 0)skip += 4 + 4*nvals[2] + 4;
      if(nvals[3] > 0)skip += 4 + 4*nvals[3] + 4;
      fseek_m(stream, skip, SEEK_CUR);
      frames[nframes++] = 36 + skip;
    }
    else{
      fseek_m(stream, 4, SEEK_CUR);        // time
      fread_m(nvals,  4, 1, stream);       // ncompressed
      fseek_m(stream, nvals[0], SEEK_CUR); // ncompressed chars
    }
  }
  if(nframes > 0){
    ResizeMemory((void **)&frames, nframes * sizeof(int));
  }
  else{
    FREEMEMORY(frames);
  }
  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
}

/* ------------------ GetPartFrameInfo ------------------------ */

void GetPartFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr,
                      int **subframeoffsetsptr, int **subframesizesptr, int *nsubframesptr,
                      int *compression_type, FILE_SIZE *filesizeptr){
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
  fseek_m(stream, headersize, SEEK_CUR);

  fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(&n_part, 4, 1, stream);fseek_m(stream, 4, SEEK_CUR);
  headersize += 4 + 4 + 4; // n_part

  NewMemory((void **)&nquants, 2*n_part*sizeof(int));

  int i;
  for(i=0; i<n_part; i++){
    int labelsize;

    fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(nquants + 2 * i, 4, 2, stream);fseek_m(stream, 4, SEEK_CUR);
    headersize += 4 + 2*4 + 4;

    labelsize = 2*nquants[2*i]*(4+30+4);
    headersize += labelsize;
    fseek_m(stream, labelsize, SEEK_CUR);
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
    fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(&time_arg, 4, 1, stream);fseek_m(stream, 4, SEEK_CUR);
    framesize = 4 + 4 + 4;

    if(returncode != 1)break;
    for(i=0; i<n_part; i++){
      int nplim;
      long int skip;

      fseek_m(stream, 4, SEEK_CUR);returncode = fread_m(&nplim, 4, 1, stream);fseek_m(stream, 4, SEEK_CUR);
      framesize += 4 + 4 + 4;             // nplim

//      fprintf(stderr, "nplim %i: %i\n",i, nplim);
      skip  = 4 + 3*nplim*sizeof(float) + 4;          // xp, yp, zp
      skip += 4 +   nplim*sizeof(int)   + 4;          // tag
      if(nquants[2*i] > 0){
        skip += 4 + nplim*nquants[2*i]*sizeof(float) + 4; // qp
      }
      framesize += skip;
      fseek_m(stream, skip, SEEK_CUR);
    }
    frames[nframes++] = framesize;
  }
  if(nframes > 0){
    ResizeMemory((void **)&frames, nframes * sizeof(int));
  }
  else{
    FREEMEMORY(frames);
  }

  filesize = bufferinfo->nbuffer;

  *headersizeptr = headersize;
  *framesptr = frames;
  *nframesptr = nframes;
  *filesizeptr = filesize;
  if(stream->stream != NULL)fclose(stream->stream);
}
#endif
