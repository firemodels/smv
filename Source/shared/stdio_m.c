#include "options.h"
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"

/* ------------------ fopen_m ------------------------ */

FILE_m *fopen_m(char *file, char *mode){
  FILE_m *stream_m = NULL;
  FILE *stream;
  char *buffer, *m_file;
  int nbuffer;

  stream = fopen(file, "rb");
  if(stream==NULL)return NULL;
  fseek(stream, 0L, SEEK_END);
  nbuffer = ftell(stream);
  fclose(stream);
  if(nbuffer<=0)return NULL;

  if(NewMemory((void **)&stream_m, sizeof(FILE_m))==0)return NULL;
  if(NewMemory((void **)&m_file, strlen(file)+1)==0)return NULL;
  strcpy(m_file, file);

  stream = fopen(file, "rb");
  if(stream==NULL){
    FREEMEMORY(stream_m);
    return NULL;
  }
  if(NewMemory((void **)&buffer, nbuffer)==0){
    FREEMEMORY(stream_m);
    return NULL;
  }

  fread(buffer, 1, nbuffer, stream);
  stream_m->buffer = buffer;
  stream_m->buffer_base = buffer;
  stream_m->file = m_file;
  stream_m->nbuffer = nbuffer;
  return stream_m;
}

/* ------------------ fclose_m ------------------------ */

void fclose_m(FILE_m *stream_m){
  if(stream_m==NULL);
  FREEMEMORY(stream_m->buffer);
  FREEMEMORY(stream_m->file);
  FREEMEMORY(stream_m);
}

/* ------------------ fread_m ------------------------ */

size_t fread_m(void *ptr, size_t size, size_t nmemb, FILE_m *stream_m){
  unsigned char *buffer, *buffer_end;

  buffer_end = stream_m->buffer+4+size*nmemb+4;
  if(buffer_end-stream_m->buffer_base>stream_m->nbuffer)return 0;
  buffer = stream_m->buffer;
  buffer += 4;
  memcpy(ptr, buffer, size*nmemb);
  buffer += size*nmemb+4;
  stream_m->buffer = buffer;
  return size*nmemb;
}

/* ------------------ fseek_m ------------------------ */

int fseek_m(FILE_m *stream_m, long int offset, int whence){
  int return_val = 0;

  switch(whence){
  case SEEK_SET:
    stream_m->buffer = stream_m->buffer_base+offset;
    break;
  case SEEK_CUR:
    stream_m->buffer += offset;
    break;
  case SEEK_END:
    stream_m->buffer = stream_m->buffer_base+stream_m->nbuffer+offset;
    break;
  }
  if(stream_m->buffer<stream_m->buffer||stream_m->buffer-stream_m->buffer_base>stream_m->nbuffer)return_val = 1;
  return return_val;
}

/* ------------------ fseek_m ------------------------ */

long int ftell_m(FILE_m *stream_m){
  long int return_val;

  return_val = stream_m->buffer-stream_m->buffer_base;
  if(return_val<0||return_val>stream_m->nbuffer)return_val = -1L;
  return return_val;
}

