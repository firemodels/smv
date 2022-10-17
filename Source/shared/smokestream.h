#ifndef SMOKESTREAM_H_DEFINED
#define SMOKESTREAM_H_DEFINED
#ifdef pp_STREAM
EXTERNCPP char *MemMap(char *file, size_t *size);
EXTERNCPP void MemUnMap(char *data, size_t size);

#define STREAM_UNLOADED 0
#define STREAM_LOADING  1
#define STREAM_LOADED   2

#define STREAM_NO       0
#define STREAM_YES      1

typedef struct _streamdata {
  char *file, *label;
  char **frameptrs, *filebuffer;
  int nframes, load_status;
  size_t *framesizes, *frame_offsets, filesize;
} streamdata;

EXTERNCPP streamdata *StreamOpen(streamdata *streamin, char *file, size_t offset, int *framesizes, int nframes, char *label, int constant_frame_size);
EXTERNCPP void StreamClose(streamdata **stream);
EXTERNCPP int GetFrameIndex(float time, float *times, int ntimes);
EXTERNCPP FILE_SIZE StreamRead(streamdata *stream, int frame_index);
EXTERNCPP void StreamReadList(streamdata **streams, int nstreams);
EXTERNCPP void StreamCheck(streamdata *framestream);
#endif
#endif
