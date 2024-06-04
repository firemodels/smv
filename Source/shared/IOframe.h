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
  unsigned char *header, *frames;
  float **rframeptrs, *times;
  void (*GetFrameInfo)(char *file, char *size_file, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesizeptr);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int *headersize, int **sizes, int *nsizes, FILE_SIZE *filesize_ptr));
void FRAMESetup(framedata *fi);
void FRAMEFree(framedata *fi);
void FRAMESetupVals(framedata *fi);
void FRAMEReadFrame(framedata *fi, int iframe, int nframes);
unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe);
void GetSliceFrameInfo(char *file, char *size_file, int *headersizeptr, int **sizesptr, int *nsizesptr, FILE_SIZE *filesizeptr);
void GetFrameFloatValptrs(framedata *fi, int iframe, int nframes);
void GetFrameTimes(framedata *fi, int iframe, int nframes);
int FRAMEGetMinMax(framedata *fi, float *valmin, float *valmax);
#endif
