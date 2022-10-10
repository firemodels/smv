#ifdef pp_SMOKESTREAM
EXTERNCPP char *MemMap(char *file, size_t *size);
EXTERNCPP void MemUnMap(char *data, size_t size);

typedef struct _streamdata {
  char *file;
  char **frameptrs, *filebuffer;
  int nframes;
  size_t *framesizes, *frame_offsets, filesize;
} streamdata;

EXTERNCPP streamdata *StreamOpen(streamdata *streamin, char *file, size_t offset, int *framesizes, int nframes, int constant_frame_size);
EXTERNCPP void StreamClose(streamdata **stream);
EXTERNCPP int GetFrameIndex(float time, float *times, int ntimes);
FILE_SIZE StreamRead(streamdata *stream, int frame_index);

#endif
