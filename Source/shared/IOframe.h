#ifndef STDIO_FRAME_H_DEFINED
#define STDIO_FRAME_H_DEFINED

// ----------------------- structures -----------------------

#define FORTRAN_FILE 0
#define C_FILE       1
typedef struct _framedata {
  char *file, *size_file;
  int nframes,     file_type;
  int headersize, *framesizes;
  FILE_SIZE *offsets, filesize;
  unsigned char *header, *frames, **frameptrs;
  float *times;
  void (*GetFrameInfo)(char *file, char *size_file, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesizeptr);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesize_ptr));
void FRAMEFree(framedata *fi);
unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe);
int FRAMEGetMinMax(framedata *fi, float *valmin, float *valmax);
void FRAMEReadFrame(framedata *fi, int iframe, int nframes);
void FRAMESetFramePtrs(framedata *fi, int iframe, int nframes);
void FRAMESetTimes(framedata *fi, int iframe, int nframes);
void FRAMESetup(framedata *fi);
void FRAMESetupVals(framedata *fi);

void GetSliceFrameInfo(char *file, char *size_file, int *headersizeptr, int **sizesptr, int *nsizesptr, FILE_SIZE *filesizeptr);
#endif
