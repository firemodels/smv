#define IN_FOPEN
#include "options.h"
#ifdef pp_OPEN_TEST
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "dmalloc.h"
#include "fopen.h"

/* ------------------ InitOpenTest ------------------------ */

void InitOpenTest(void){
#ifdef pp_THREAD
  pthread_mutex_init(&fopen_mutex, NULL);
#endif
}

/* ------------------ AddOpenFile ------------------------ */

void AddOpenFile(const char *file, FILE *stream, char *source, int line){
  if(stream != NULL){
    opendata *oi;

    LOCK_FOPEN;
    NEWMEM(openinfo, (nopeninfo+1) * sizeof(opendata));
    oi = openinfo + nopeninfo;
    assert(strlen(file) < 1023);
    assert(strlen(source) < 1023);
    assert(line >= 0);
    strcpy(oi->file, file);
    strcpy(oi->source, source);
    oi->line   = line;
    oi->stream = stream;
    nopeninfo++;
    UNLOCK_FOPEN;
  }
}

/* ------------------ RemoveOpenFile ------------------------ */

void RemoveOpenFile(FILE *stream){
  int i,remove_index=-1;

  LOCK_FOPEN;
  for(i = 0; i < nopeninfo; i++){
    opendata *oi;

    oi = openinfo + i;
    if(oi->stream == stream){
      fprintf(stderr, "close file:%s\n", oi->file);
      oi->stream = NULL;
      remove_index = i;
    }
  }
  if(remove_index < 0){
    fprintf(stderr, "file is not in list\n");
    return;
  }
  int nmove;

  nmove = nopeninfo - remove_index - 1;
  if(nmove>0){
    memmove(openinfo + remove_index, openinfo + remove_index + 1, nmove * sizeof(opendata));
  }
  nopeninfo--;
  if(nopeninfo > 0){
    ResizeMemory((void **)&openinfo, nopeninfo * sizeof(opendata));
  }
  else{
    nopeninfo=0;
    FREEMEMORY(openinfo);
  }
  if(nopeninfo > 0){
    for(i = 0; i < nopeninfo; i++){
      opendata *oi;

      oi = openinfo + i;
      fprintf(stderr, "file: %s\n", oi->file);
      fprintf(stderr, "source: %s\n", oi->source);
      fprintf(stderr, "line: %i\n\n", oi->line);
    }
  }
  UNLOCK_FOPEN;
}

/* ------------------ fopen_counting ------------------------ */

FILE *fopen_counting(const char *file, const char *mode, char *source, int line)
{
  FILE *stream;

  stream = fopen(file, mode);
  if(stream != NULL){
    AddOpenFile(file, stream, source, line);
    open_files++;
  }
  return stream;
}

/* ------------------ fclose_counting ------------------------ */

int fclose_counting(FILE *stream)
{
  int status;

  status = fclose(stream);
  if(status==0){
    RemoveOpenFile(stream);
    open_files--;
    if(open_files < 0){
      fprintf(stderr, "file count negative\n");
    }
  }
  return status;
}
#endif
