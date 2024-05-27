#ifndef STDIO_FRAME_H_DEFINED
#define STDIO_FRAME_H_DEFINED

// ----------------------- structures -----------------------

typedef struct _framedata {
  char *file, *size_file;
  int nframes, *sizes;
  FILE_SIZE *offsets;
  unsigned char *vals;
  void (*GetFrameInfo)(char *file, char *size_file, int **sizes, int *nsizes);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMEInit(char *file, char *size_file, void GetFrameInfo(char *file, char *size_file, int **sizes, int *nsizes));
void FRAMESetup(framedata *fi);
void FRAMEFree(framedata *fi);
void FRAMESetupVals(framedata *fi);
void FRAMEReadFrame(framedata *fi, int iframe, FILE *stream);
void *FRAMEGetFramePtr(framedata *fi, int iframe);
void GetSliceFrameInfo(char *file, char *size_file, int **sizesptr, int *nsizesptr);
#endif
