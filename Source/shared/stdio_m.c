#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "stdio_m.h"
#include "file_util.h"
#include "dmalloc.h"

/* ------------------ GetFileBuffer ------------------------ */

int GetFileBuffer(char *file, FILE_SIZE offset, unsigned char *buffer, size_t nbuffer){
  FILE *stream = NULL;

  stream = FOPEN(file, "rb");
  if(stream==NULL)return 0;
  fseek(stream, offset, SEEK_SET);
  fread(buffer, (size_t)1, (size_t)nbuffer, stream);
  fclose(stream);
  return 1;
}

/* ------------------ fopen_m ------------------------ */

FILE_m *fopen_mo(char *file, FILE_SIZE offset, FILE_SIZE size, char *mode){
  FILE_m *stream_m = NULL;
  FILE *stream;
  unsigned char *buffer;
  char  *m_file;
  FILE_SIZE nbuffer;

  if(file==NULL||strlen(file)==0||mode==NULL||strlen(mode)<2)return NULL;
  if(strcmp(mode, "rb")!=0&&strcmp(mode, "rbm")!=0)return NULL;

  if(NewMemory((void **)&m_file, strlen(file)+1)==0){ // memory allocation failed so abort
    return NULL;
  }

  if(NewMemory((void **)&stream_m, sizeof(FILE_m))==0){
    FREEMEMORY(m_file);
    return NULL;
  }
  stream_m->buffer = NULL;
  stream_m->buffer_beg = NULL;
  strcpy(m_file, file);
  stream_m->file = m_file;
  stream_m->stream = NULL;

  if(strcmp(mode, "rb")==0){    // not mode rbm so use regular IO
    stream = FOPEN(file, "rb");
    if(stream==NULL)return NULL;
    stream_m->stream = stream;
    return stream_m;
  }

  if(size>0){
    nbuffer = size;
  }
  else{
    stream = FOPEN(file, "rb");
    if(stream==NULL)return NULL;
    fseek(stream, 0L, SEEK_END);
    nbuffer = FTELL(stream);
    offset = 0;
    fclose(stream);
  }

  if(nbuffer<=0){            // file is empty so abort
    FREEMEMORY(stream_m);
    FREEMEMORY(m_file);
    return NULL;
  }

  if(NewMemory((void **)&buffer, nbuffer)==0){ // allocation of memory buffer failed so revert to regular IO
    stream = FOPEN(file, "rb");
    if(stream==NULL)return NULL;
    stream_m->stream = stream;
    return stream_m;
  }

  if(GetFileBuffer(file, offset, buffer, nbuffer)==0)return NULL;

  stream_m->buffer     = buffer;
  stream_m->buffer_beg = buffer;
  stream_m->buffer_end = buffer+nbuffer;
  stream_m->file       = m_file;
  stream_m->stream     = NULL;
  return stream_m;
}

/* ------------------ fopen_m ------------------------ */

FILE_m *fopen_m(char *file, char *mode){
  return fopen_mo(file, 0, 0, mode);
}

/* ------------------ fopen_b ------------------------ */

FILE_m *fopen_b(char *file, unsigned char *buffer, size_t nbuffer, char *mode){
  FILE_m *stream_m = NULL;
  char *m_file;

  if(file == NULL || strlen(file) == 0 || mode == NULL || strlen(mode) < 2)return NULL;
  if(strcmp(mode, "rb") !=0)return NULL;

  if(NewMemory((void **)&m_file, strlen(file) + 1) == 0){ // memory allocation failed so abort
    return NULL;
  }
  strcpy(m_file, file);

  if(NewMemory((void **)&stream_m, sizeof(FILE_m)) == 0){
    FREEMEMORY(m_file);
    return NULL;
  }
  assert((buffer == NULL && nbuffer == 0) || (buffer != NULL && nbuffer >= 0));
  stream_m->buffer     = buffer;
  stream_m->buffer_beg = buffer;
  stream_m->buffer_end = buffer;
  if(buffer != NULL)stream_m->buffer_end = buffer + nbuffer;
  stream_m->file       = m_file;
  if(buffer == NULL){
    stream_m->stream = FOPEN(file, mode);
    if(stream_m->stream == NULL){
      FREEMEMORY(stream_m);
      FREEMEMORY(m_file);
    }
  }
  else{
    stream_m->stream = NULL;
  }
  return stream_m;
}

/* ------------------ fclose_b ------------------------ */

void fclose_b(FILE_m *stream_m){
  if(stream_m == NULL)return;
  if(stream_m->stream == NULL){
    FREEMEMORY(stream_m->file);
    FREEMEMORY(stream_m);
  }
  else{
    fclose(stream_m->stream);
  }
}

/* ------------------ fclose_m ------------------------ */

void fclose_m(FILE_m *stream_m){
  if(stream_m==NULL)return;
  if(stream_m->stream==NULL){
    FREEMEMORY(stream_m->buffer_beg);
    FREEMEMORY(stream_m->file);
    FREEMEMORY(stream_m);
  }
  else{
    fclose(stream_m->stream);
  }
}

/* ------------------ fread_m ------------------------ */

size_t fread_m(void *ptr, size_t size, size_t nmemb, FILE_m *stream_m){
  size_t return_val=0;

  if(stream_m->stream==NULL){
    unsigned char *buffer_end;

    buffer_end = stream_m->buffer + size*nmemb;
    if(buffer_end-stream_m->buffer_end>0){
      stream_m->buffer = buffer_end;
      return 0;
    }
    memcpy(ptr, stream_m->buffer, size*nmemb);
    stream_m->buffer = buffer_end;
    return_val = nmemb;
  }
  else{
    return_val =  fread(ptr, size, nmemb, stream_m->stream);
  }
  return return_val;
}

/* ------------------ fread_mv ------------------------ */

size_t fread_mv(void **ptr, size_t size, size_t nmemb, FILE_m *stream_m){
  if(stream_m->stream!=NULL)return 0;
  *ptr= stream_m->buffer;
  stream_m->buffer += size*nmemb;
  return nmemb;
}

/* ------------------ feof_m ------------------------ */

int feof_m(FILE_m *stream_m){
  if(stream_m->stream!=NULL){
    int return_code;

    return_code = feof(stream_m->stream);
    if(return_code!=PASS_m)return_code = FAIL_m;
    return return_code;
  }
  if(stream_m->buffer-stream_m->buffer_end>=0)return FAIL_m;
  return PASS_m;
}

  /* ------------------ fseek_m ------------------------ */

int fseek_m(FILE_m *stream_m, long int offset, int whence){
  int return_val = PASS_m;

  if(stream_m->stream==NULL){
    switch(whence){
    case SEEK_SET:
      stream_m->buffer = stream_m->buffer_beg+offset;
      break;
    case SEEK_CUR:
      stream_m->buffer += offset;
      break;
    case SEEK_END:
      stream_m->buffer = stream_m->buffer_end + offset;
      break;
    default:
      assert(FFALSE);
      break;
    }
    if(stream_m->buffer-stream_m->buffer_beg<0||stream_m->buffer-stream_m->buffer_end>=0)return_val = FAIL_m;
  }
  else{
    return_val = fseek(stream_m->stream, offset, whence);
  }
  return return_val;
}

/* ------------------ fseek_m_long ------------------------ */

int fseek_m_long(FILE_m *stream_m, long long offset, int whence){
  int return_val = PASS_m;

  if(stream_m->stream == NULL){
    switch(whence){
    case SEEK_SET:
      stream_m->buffer = stream_m->buffer_beg + offset;
      break;
    case SEEK_CUR:
      stream_m->buffer += offset;
      break;
    case SEEK_END:
      stream_m->buffer = stream_m->buffer_end + offset;
      break;
    default:
      assert(FFALSE);
      break;
    }
    if(stream_m->buffer - stream_m->buffer_beg < 0 || stream_m->buffer - stream_m->buffer_end >= 0)return_val = FAIL_m;
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
    return_val = stream_m->buffer-stream_m->buffer_beg;
    if(return_val<0||return_val>stream_m->buffer_end-stream_m->buffer_beg)return_val = -1L;
  }
  else{
    return_val = FTELL(stream_m->stream);
  }
  return return_val;
}

/* ------------------ rewind_m ------------------------ */

void rewind_m(FILE_m *stream_m){
  if(stream_m->stream==NULL){
    stream_m->buffer = stream_m->buffer_beg;
  }
  else{
    rewind(stream_m->stream);
  }
}

/* ------------------ fgets_m ------------------------ */

char *fgets_m(char *str, int num, FILE_m *stream_m){
  char *str_base=str;
  int i;

  if(stream_m->stream!=NULL)return fgets(str, num, stream_m->stream);  // use regular io routine
  if(stream_m->buffer-stream_m->buffer_end>=0)return NULL;             // past eof

  for(i = 0; i<num; i++){
    char c;
    int nl = 10;

    if(i==num-1||stream_m->buffer-stream_m->buffer_end>=0){
      *str = 0;
      if(i==num-1)break;        // filled up buffer, advance past next nl
      return str_base;          // past eof, exit
    }

    c = *(stream_m->buffer)++;
    *str++ = c;                 // copy character
    if(c==nl||c==0){            // terminate string and advance past next nl
      *str = 0;
      break;
    }
  }
  for(;;){                      // advance past next nl or to eof
    char c;
    int nl = 10;

    c = *(stream_m->buffer)++;
    if(c==nl||stream_m->buffer-stream_m->buffer_end>=0)break;
  }
  return str_base;
}
