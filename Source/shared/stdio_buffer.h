#ifndef STDIO_BUFFER_H_DEFINED
#define STDIO_BUFFER_H_DEFINED

/* --------------------------  _filedata ------------------------------------ */

#ifdef pp_SLICE_BUFFER
#define FILEBUFFER               filedata
#else
#define FILEBUFFER               FILE
#endif
#define FILE_ASCII  0
#define FILE_BINARY 1
typedef struct _filedata {
  int mode;
  char *buffer, **lines;
  int iline, nlines;
  FILE_SIZE pos, filesize;
} filedata;

/* --------------------------  bufferstreamdata ------------------------------------ */

typedef struct bufferstreamdata{
  FILE *stream, *stream1, *stream2;
  filedata *fileinfo;
} bufferstreamdata;

EXTERNCPP bufferstreamdata *GetSMVBuffer(char *file, char *file2);
EXTERNCPP bufferstreamdata *CopySMVBuffer(bufferstreamdata *stream_in);
EXTERNCPP int AppendFileBuffer(filedata *file1, filedata *file2);
EXTERNCPP void OutputFileBuffer(filedata *fileinfo);
EXTERNCPP void fclose_buffer(filedata *fileinfo);
EXTERNCPP FILE_SIZE ftell_buffer(filedata *stream);
EXTERNCPP int fseek_buffer(filedata *stream, FILE_SIZE offset, int origin);
FILE_SIZE fread_buffer(void *ptr, FILE_SIZE size, FILE_SIZE count, filedata *stream);
FILE_SIZE freadptr_buffer(void **ptr, FILE_SIZE size, FILE_SIZE count, filedata *stream);
EXTERNCPP int feof_buffer(filedata *fileinfo);
EXTERNCPP char *fgets_buffer(filedata *fileinfo, char *buffer, int size);
EXTERNCPP void rewind_buffer(filedata *fileinfo);
EXTERNCPP filedata *fopen_buffer(char *filename, char *mode);


#endif
