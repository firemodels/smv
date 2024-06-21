#ifndef STDIO_FRAME_H_DEFINED
#define STDIO_FRAME_H_DEFINED
#include "file_util.h"

// ----------------------- structures -----------------------

#define FRAME_FSEEK(a,b,c)       fseek_m(a,b,c)

#define FRAME_READ(var,count,STREAM) \
    FRAME_FSEEK(STREAM,4,SEEK_CUR);returncode=fread_m(var,4,(count),STREAM);FRAME_FSEEK(STREAM,4,SEEK_CUR)

#define FORTRAN_FILE 0
#define C_FILE       1
typedef struct _framedata {
  char *file, *size_file;
  int nframes, file_type;
#ifdef pp_THREAD
  int nthreads;
#endif
  int headersize, *framesizes;
  FILE_SIZE *offsets, filesize;
  unsigned char *header, *frames, **frameptrs;
  bufferdata *bufferinfo;
  float *times;
  float valmin, valmax;
  void (*GetFrameInfo)(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesizeptr);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesize_ptr));
void FRAMEFree(framedata *fi);
#ifdef pp_THREAD
void FRAMESetNThreads(framedata *fi, int nthreads);
#endif
unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe);
int FRAMEGetMinMax(framedata *fi);
void FRAMEReadFrame(framedata *fi, int iframe, int nframes);
void FRAMESetFramePtrs(framedata * fi, int iframe, int nframes);
void FRAMESetTimes(framedata *fi, int iframe, int nframes);
void FRAMESetup(framedata *fi);
void FRAMESetupVals(framedata *fi);

void GetIsoFrameInfo(    bufferdata *bufferinfo, int *headersizeptr, int **sizesptr, int *nsizesptr, FILE_SIZE *filesizeptr);
void GetSliceFrameInfo(  bufferdata *bufferinfo, int *headersizeptr, int **sizesptr, int *nsizesptr, FILE_SIZE *filesizeptr);
void GetSmoke3DFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **sizesptr, int *nsizesptr, FILE_SIZE *filesizeptr);
void GetPartFrameInfo(   bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, FILE_SIZE *filesizeptr);
#endif
