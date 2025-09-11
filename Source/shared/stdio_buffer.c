#include "options_common.h"
#include <stdio.h>
#include <string.h>
#include "stdio_buffer.h"
#include "dmalloc.h"
#include "string_util.h"

/* ------------------ OutputFileBuffer ------------------------ */

void OutputFileBuffer(filedata *fileinfo){
  int i;

  if(fileinfo==NULL)return;
  for(i = 0;i<fileinfo->nlines;i++){
    char *buffer;

    buffer = fileinfo->lines[i];
    if(buffer==NULL||strlen(buffer)==0)continue;
    fprintf(stderr, "%s\n", buffer);
  }
}

/* ------------------ AppendFileBuffer ------------------------ */

bufferstreamdata *AppendFileBuffer(bufferstreamdata *stream, char *file){
  char *buffer, **lines;
  int i;
  bufferstreamdata *stream2;
  filedata *file1, *file2;

  if(file==NULL)return stream;
  stream2 = GetSMVBuffer(file);
  if(stream2==NULL)return stream;

  file1 = stream->fileinfo;
  file2 = stream2->fileinfo;

  if(NewMemory((void **)&buffer, file1->filesize + file2->filesize)==0){
    return stream;
  }
  memcpy(buffer,                 file1->buffer, file1->filesize);
  memcpy(buffer+file1->filesize, file2->buffer, file2->filesize);

  if(NewMemory((void **)&lines, (file1->nlines+file2->nlines)*sizeof(char *))==0){
    FREEMEMORY(buffer);
    return  stream;
  }

  for(i = 0;i<file1->nlines;i++){
    lines[i] = buffer + (file1->lines[i] - file1->buffer);
  }
  for(i = 0;i<file2->nlines;i++){
    lines[i+file1->nlines] = buffer + file1->filesize + (file2->lines[i] - file2->buffer);
  }

  FREEMEMORY(file1->buffer);
  FREEMEMORY(file1->lines);
  file1->buffer   = buffer;
  file1->lines    = lines;
  file1->filesize = file1->filesize + file2->filesize;
  file1->nlines   = file1->nlines   + file2->nlines;
  return stream;
}

/* ------------------ CopySMVBuffer ------------------------ */

bufferstreamdata *CopySMVBuffer(bufferstreamdata *stream_in){
  bufferstreamdata *stream_out;
  filedata *fileinfo;

  if(stream_in==NULL)return NULL;

  NewMemory((void **)&stream_out, sizeof(bufferstreamdata));
  memcpy(stream_out, stream_in, sizeof(bufferstreamdata));

  NewMemory((void **)&fileinfo, sizeof(filedata));
  stream_out->fileinfo = fileinfo;

  memcpy(fileinfo, stream_in->fileinfo, sizeof(filedata));
  return stream_out;
}

/* ------------------ GetSMVBuffer ------------------------ */

bufferstreamdata *GetSMVBuffer(char *file){
  bufferstreamdata *stream;

  NewMemory((void **)&stream, sizeof(bufferstreamdata));
  stream->fileinfo = NULL;

  if(file!=NULL)stream->fileinfo = fopen_buffer(file, "r", 1, 0);
  if(stream->fileinfo==NULL){
    FREEMEMORY(stream);
  }
  return stream;
}

/* ------------------ FOPEN_RB ------------------------ */

bufferstreamdata *FOPEN_RB(char *file, int nthreads, int use_threads){
  bufferstreamdata *stream;
  filedata *fileinfo;

  if(file==NULL)return NULL;
  fileinfo = fopen_buffer(file, "rb", nthreads, use_threads);
  if(fileinfo==NULL)return NULL;

  NewMemory((void **)&stream, sizeof(bufferstreamdata));
  stream->fileinfo = fileinfo;
  return stream;
}

/* ------------------ feof_buffer ------------------------ */

int feof_buffer(filedata *fileinfo){
  if(fileinfo->mode==FILE_ASCII&&fileinfo->iline>=fileinfo->nlines)return 1;
  if(fileinfo->pos>=fileinfo->filesize)return 1;
  return 0;
}

/* ------------------ fgets_buffer ------------------------ */

char *fgets_buffer(filedata *fileinfo,char *buffer,int size){
  char *file_buffer, *from, *to;
  int iline, i;

  if(fileinfo==NULL)return NULL;
  iline = fileinfo->iline;
  if(iline>=fileinfo->nlines)return NULL;
  file_buffer = fileinfo->lines[iline];
  from = file_buffer;
  to = buffer;
  for(i = 0;i<size;i++){
    *to++ = *from++;
    if(from[-1]==0)break;
  }
  to[-1] = 0;
  fileinfo->iline++;
  return buffer;
}

/* ------------------ rewind_buffer ------------------------ */

void rewind_buffer(filedata *fileinfo){
  if(fileinfo==NULL)return;
  fileinfo->iline = 0;
  fileinfo->pos   = 0;
}

/* ------------------ fclose_buffer ------------------------ */

void fclose_buffer(filedata *fileinfo){
  char *buffer;

  if(fileinfo==NULL)return;
  buffer = fileinfo->buffer;
  FREEMEMORY(buffer);
  FREEMEMORY(fileinfo);
}

/* ------------------ fseek_buffer ------------------------ */

int fseek_buffer(filedata *stream, FILE_SIZE offset, int origin){
  if(origin==SEEK_SET){
    if(offset>stream->filesize)return 1;
    stream->pos = offset;
    return 0;
  }
  else if(origin==SEEK_CUR){
    FILE_SIZE new_pos;

    new_pos = stream->pos+offset;
    if(new_pos>stream->filesize)return 1;
    stream->pos = new_pos;
    return 0;
  }
  else{
    return 1;
  }
}

/* ------------------ ftell_buffer ------------------------ */

FILE_SIZE ftell_buffer(filedata *stream){
  return stream->pos;
}

/* ------------------ fread_buffer ------------------------ */

FILE_SIZE fread_buffer(void *ptr, FILE_SIZE size, FILE_SIZE count, filedata *stream){
  FILE_SIZE next_pos, copy_count;

  next_pos = stream->pos+count*size;
  if(next_pos>stream->filesize)next_pos = stream->filesize;
  copy_count = next_pos - stream->pos;
  memcpy(ptr, stream->buffer+stream->pos, copy_count);
  stream->pos += copy_count;
  return copy_count/size;
}

/* ------------------ freadptr_buffer ------------------------ */

FILE_SIZE freadptr_buffer(void **ptr, FILE_SIZE size, FILE_SIZE count, filedata *stream){
  FILE_SIZE last_pos, copy_count;

  last_pos = stream->pos+count*size;
  if(last_pos>stream->filesize)last_pos = stream->filesize;
  copy_count = last_pos - stream->pos;
  *ptr = stream->buffer+stream->pos;
  stream->pos += copy_count;
  return copy_count/size;
}

/* ------------------ ReadBufferi ------------------------ */

void ReadBufferi(readbufferdata *readbufferi){
  FILE *stream;

  stream = FOPEN(readbufferi->filename, "rb");
  if(stream==NULL){
    readbufferi->returnval = 0;
    return;
  }
  fseek(stream, readbufferi->start, SEEK_SET);
  fread(readbufferi->buffer+readbufferi->start, sizeof(char), readbufferi->size, stream);
  fclose(stream);
  readbufferi->returnval = 1;
}

/* ------------------ ReadBuffer ------------------------ */

int ReadBuffer(char *filename, int filesize, char *buffer){
  int returnval;
  readbufferdata *readbufferinfo=NULL;

  returnval = 1;

  NewMemory((void **)&readbufferinfo, sizeof(readbufferdata));

  readbufferinfo->buffer = buffer;
  readbufferinfo->filename = filename;
  readbufferinfo->start = 0;
  readbufferinfo->size = filesize;

  ReadBufferi(readbufferinfo);

  if(readbufferinfo->returnval == 0)returnval = 0;

  FREEMEMORY(readbufferinfo);
  return returnval;
}

/* ------------------ fopen_buffer ------------------------ */

filedata *fopen_buffer(char *filename, char *mode, int nthreads, int use_multithread){
  FILE_SIZE i,filesize;
  filedata *fileinfo;
  char *buffer, **lines;
  int nlines;

  // only support r and rb modes (ascii and binary)

  if(mode==NULL)return NULL;
  if( strcmp(mode, "r")!=0 && strcmp(mode, "rb")!=0 )return NULL;

  if(FILE_EXISTS(filename)==NO)return NULL;
  filesize = GetFileSizeSMV(filename);
  if(filesize==0)return NULL;

  if(NewMemory((void **)&buffer, filesize+1)==0)return NULL;

  int ReadBuffer(char *filename, int filesize, char *buffer);

  if(ReadBuffer(filename, filesize, buffer)==0){
    FREEMEMORY(buffer);
    return NULL;
  }

  filesize++;           // add an extra character to file and set it to the end of string character
  buffer[filesize-1]=0;

  NewMemory((void **)&fileinfo, sizeof(filedata));
  fileinfo->buffer   = buffer;
  fileinfo->filesize = filesize;
  fileinfo->lines    = NULL;
  fileinfo->iline    = 0;
  fileinfo->nlines   = 0;
  fileinfo->pos      = 0;
  CheckMemory;

  if(strcmp(mode, "r")==0)fileinfo->mode=FILE_ASCII;
  if(strcmp(mode, "rb")==0)fileinfo->mode = FILE_BINARY;

  if(fileinfo->mode==FILE_ASCII){

    // count number of lines

    nlines = 0;
    for(i = 0; i<filesize; i++){
      int ch;

      ch = buffer[i];
      if(ch=='\r'){      // end of line is \r\n or \n
        buffer[i] = ' ';   //  if a \r is found set it to a blank character
        continue;
      }
      if(ch=='\n'||ch==EOF||ch==0){
        buffer[i] = 0;
        nlines++;
      }
    }
    CheckMemory;
    NewMemory((void **)&lines, (nlines+1)*sizeof(char *));
    fileinfo->lines = lines;

    nlines = 0;
    lines[0] = buffer;
    for(i = 0; i<filesize; i++){
      int ch;

      ch = buffer[i];
      if(ch!=0)continue;
      if(i+1<filesize){
        nlines++;
        lines[nlines] = buffer+i+1;
      }
    }
    fileinfo->nlines = nlines;
  }
  CheckMemory;
  return fileinfo;
}
