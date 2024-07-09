#ifndef STDIO_FRAME_H_DEFINED
#define STDIO_FRAME_H_DEFINED
#include "file_util.h"

// ----------------------- structures -----------------------

#define FRAME_FSEEK(a,b,c)       fseek_m(a,b,c)

#define FRAME_READ(var,count,STREAM) \
    FRAME_FSEEK(STREAM,4,SEEK_CUR);returncode=fread_m(var,4,(count),STREAM);FRAME_FSEEK(STREAM,4,SEEK_CUR)

#define FORTRAN_FILE 0
#define C_FILE       1

#ifndef RELOAD
#define RELOAD      3
#endif
#ifndef ALL_FRAMES
#define ALL_FRAMES -1
#endif


typedef struct _framedata {
  char *file;
  unsigned char *header, *frames, **frameptrs;
  int nframes, frames_read, update, file_type;
#ifdef pp_THREAD
  int nthreads;
#endif
  int headersize, *framesizes;
  int *subframeoffsets, *subframesizes, nsubframes;
  FILE_SIZE *offsets, filesize, bytes_read;
  bufferdata *bufferinfo;
  float *times, load_time, total_time;
  float valmin, valmax;
  void (*GetFrameInfo)(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes, int **subframeoffsets, int **subframesizes, int *nsubframes, FILE_SIZE *filesizeptr);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMELoadFrameData(framedata *frameinfo, char *file, int load_flag, int time_frame, int file_type,
                              void GetFrameInfo(bufferdata *bufferinfo, int *headersize, int **sizes, int *nsizes, int **subframeptrs, int **subframesizesptr, int *nsubframes, FILE_SIZE *filesizeptr));
void FRAMEFree(framedata *fi);
#ifdef pp_THREAD
void FRAMESetNThreads(framedata *fi, int nthreads);
#endif
unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe);
unsigned char *FRAMEGetSubFramePtr(framedata *fi, int iframe, int isubframe);
int FRAMEGetMinMax(framedata *fi);

void GetBoundaryFrameInfo(bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, int **subframeoffsetptrs, int **subframesizesptr, int *nsubframeoffsets, FILE_SIZE *filesizeptr);
void GetIsoFrameInfo(     bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, int **subframeoffsetptrs, int **subframesizesptr, int *nsubframeoffsets, FILE_SIZE *filesizeptr);
void GetPartFrameInfo(    bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, int **subframeoffsetptrs, int **subframesizesptr, int *nsubframeoffsets, FILE_SIZE *filesizeptr);
void GetSliceFrameInfo(   bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, int **subframeoffsetptrs, int **subframesizesptr, int *nsubframeoffsets, FILE_SIZE *filesizeptr);
void GetSmoke3DFrameInfo( bufferdata *bufferinfo, int *headersizeptr, int **framesptr, int *nframesptr, int **subframeoffsetptrs, int **subframesizesptr, int *nsubframeoffsets, FILE_SIZE *filesizeptr);
#endif
