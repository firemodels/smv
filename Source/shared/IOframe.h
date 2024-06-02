#ifndef STDIO_FRAME_H_DEFINED
#define STDIO_FRAME_H_DEFINED

// ----------------------- structures -----------------------

#define FORTRAN_FILE 0
#define C_FILE       1
typedef struct _framedata {
  char *file, *size_file;
  int nframes, *frame_sizes, file_type;
  FILE_SIZE *offsets;
  unsigned char *uc_vals, **uc_valptrs;
  float *times, **r_valptrs;
  void (*GetFrameInfo)(char *file, char *size_file, int **sizes, int *nsizes);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int **sizes, int *nsizes));
void FRAMESetup(framedata *fi);
void FRAMEFree(framedata *fi);
void FRAMESetupVals(framedata *fi);
void FRAMEReadFrame(framedata *fi, int iframe, int nframes, FILE *stream);
unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe);
void GetSliceFrameInfo(char *file, char *size_file, int **sizesptr, int *nsizesptr);
void GetFrameFloatValptrs(framedata *fi, int iframe, int nframes);
void GetFrameUCValptrs(framedata *fi, int iframe, int nframes);
#endif
