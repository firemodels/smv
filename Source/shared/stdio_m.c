#include "options.h"
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "MALLOCC.h"

/* ------------------ fopen_m ------------------------ */

FILE_m *fopen_m(char *file, char *mode){
  FILE_m *stream_m = NULL;
  FILE *stream;
  unsigned char *buffer;
  char  *m_file;
  size_t nbuffer;

  if(file==NULL||strlen(file)==0||mode==NULL||strlen(mode)<2)return NULL;
  if(strcmp(mode, "rb")!=0&&strcmp(mode, "rbm")!=0)return NULL;
  stream = fopen(file, "rb");
  if(stream==NULL){                                   // open of file failed so abort
    return NULL;
  }

  if(NewMemory((void **)&m_file, strlen(file)+1)==0){ // memory allocation failed so abort
    fclose(stream);
    return NULL;
  }

  if(NewMemory((void **)&stream_m, sizeof(FILE_m))==0)return NULL;
  stream_m->buffer = NULL;
  stream_m->buffer_base = NULL;
  stream_m->nbuffer = 0;
  strcpy(m_file, file);
  stream_m->file = m_file;
  stream_m->stream = NULL;

  if(strcmp(mode, "rb")==0){    // not mode rbm so use regular IO
    stream_m->stream = stream;
    return stream_m;
  }

  fseek(stream, 0L, SEEK_END);
  nbuffer = ftell(stream);
  if(nbuffer<=0){            // file is empty so abort
    fclose(stream);
    FREEMEMORY(stream_m);
    FREEMEMORY(m_file);
    return NULL;
  }

  if(NewMemory((void **)&buffer, nbuffer)==0){ // allocation of memory buffer failed so revert to regular IO
    stream_m->stream = stream;
    return stream_m;
  }

  fseek(stream, 0, SEEK_SET);
  fread(buffer, (size_t)1, (size_t)nbuffer, stream);
  stream_m->buffer = buffer;
  stream_m->buffer_base = buffer;
  stream_m->file = m_file;
  stream_m->nbuffer = nbuffer;
  stream_m->stream = NULL;
  return stream_m;
}

/* ------------------ fclose_m ------------------------ */

void fclose_m(FILE_m *stream_m){
  if(stream_m==NULL)return;
  if(stream_m->stream==NULL){
    FREEMEMORY(stream_m->buffer_base);
    FREEMEMORY(stream_m->file);
    FREEMEMORY(stream_m);
  }
  else{
    fclose(stream_m->stream);
  }
}

/* ------------------ fread_m ------------------------ */

size_t fread_m(void *ptr, size_t size, size_t nmemb, FILE_m *stream_m){
  unsigned char *buffer_end;
  size_t return_val;

  if(stream_m->stream==NULL){
    buffer_end = stream_m->buffer + size*nmemb;
    if(buffer_end - stream_m->buffer_base > stream_m->nbuffer)return 0;
    memcpy(ptr, stream_m->buffer, size*nmemb);
    stream_m->buffer += size*nmemb;
    return_val = size*nmemb;
  }
  else{
    return_val =  fread(ptr, size, nmemb, stream_m->stream);
  }
  return return_val;
}

/* ------------------ freadptr_m ------------------------ */

size_t freadptr_m(void **ptr, size_t size, size_t nmemb, FILE_m *stream_m){
  unsigned char *buffer, *buffer_end;
  size_t return_val;

  if(stream_m->stream==NULL){
    buffer_end = stream_m->buffer+4+size*nmemb+4;
    if(buffer_end-stream_m->buffer_base>stream_m->nbuffer)return 0;
    buffer = stream_m->buffer;
    buffer += 4;
    *ptr= buffer;
    buffer += size*nmemb+4;
    stream_m->buffer = buffer;
    return_val = size*nmemb;
  }
  return return_val;
}

/* ------------------ fseek_m ------------------------ */

int fseek_m(FILE_m *stream_m, long int offset, int whence){
  int return_val = 0;

  if(stream_m->stream==NULL){
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
    if(stream_m->buffer-stream_m->buffer_base<0||stream_m->buffer-stream_m->buffer_base>stream_m->nbuffer)return_val = 1;
  }
  else{
    return_val = fseek(stream_m->stream, offset, whence);
  }
  return return_val;
}

/* ------------------ ftell_m ------------------------ */

long int ftell_m(FILE_m *stream_m){
  long int return_val;

  if(stream_m->stream==NULL){
    return_val = stream_m->buffer-stream_m->buffer_base;
    if(return_val<0||return_val>stream_m->nbuffer)return_val = -1L;
  }
  else{
    return_val = ftell(stream_m->stream);
  }
  return return_val;
}

/* ------------------ rewind_m ------------------------ */

void rewind_m(FILE_m *stream_m){
  if(stream_m->stream==NULL){
    stream_m->buffer = stream_m->buffer_base;
  }
  else{
    rewind(stream_m->stream);
  }
}

