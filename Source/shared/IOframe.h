#ifndef STDIO_FRAME_H_DEFINED
#define STDIO_FRAME_H_DEFINED

// ----------------------- structures -----------------------

#define FORTRAN_FILE 0
#define C_FILE       1
typedef struct _framedata {
  char *file, *size_file;
  int nframes, file_type;
  int *frame_sizes0, *frame_sizes;
  FILE_SIZE *offsets0, *offsets, filesize;
  unsigned char *uc_vals, **uc_valptrs0, **uc_valptrs;
  float *times0, *times, **r_valptrs0, **r_valptrs;
  void (*GetFrameInfo)(char *file, char *size_file, int **sizes, int *nsizes, FILE_SIZE *filesizeptr);
} framedata;

// ----------------------- headers -----------------------

framedata *FRAMEInit(char *file, char *size_file, int file_type, void GetFrameInfo(char *file, char *size_file, int **sizes, int *nsizes, FILE_SIZE *filesize_ptr));
void FRAMESetup(framedata *fi);
void FRAMEFree(framedata *fi);
void FRAMESetupVals(framedata *fi);
void FRAMEReadFrame(framedata *fi, int iframe, int nframes);
unsigned char *FRAMEGetFramePtr(framedata *fi, int iframe);
void GetSliceFrameInfo(char *file, char *size_file, int **sizesptr, int *nsizesptr, FILE_SIZE *filesizeptr);
void GetFrameFloatValptrs(framedata *fi, int iframe, int nframes);
void GetFrameUCValptrs(framedata *fi, int iframe, int nframes);
void GetFrameTimes(framedata *fi, int iframe, int nframes);
#endif
