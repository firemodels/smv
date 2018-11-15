#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "string_util.h"
#include "file_util.h"
#include "MALLOC.h"
#ifdef WIN32
#include <windows.h>
#endif

#define LEN_BUFFER 1024

/* ------------------ Usage ------------------------ */

void Usage(char *prog, int option){
 char githash[LEN_BUFFER];
 char gitdate[LEN_BUFFER];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stdout, "flush the cache\n");
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ FlushCache ------------------------ */
#define BUFFERSIZE 250000000
void FlushCache(void){
#ifdef WIN32
  MEMORYSTATUSEX statex;
#endif
  int *buffer[8], i;

#ifdef WIN32
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatusEx(&statex);
#endif
  for(i = 0;i<8;i++){
    int *buffptr;
    int j;

    NewMemory((void **)&buffptr, sizeof(int)*BUFFERSIZE);
    buffer[i] = buffptr;
    for(j = 0;j<BUFFERSIZE;j++){
      buffptr[j] = 1;
    }
  }
  for(i = 0;i<8;i++){
    FREEMEMORY(buffer[i]);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  initMALLOC();
  SetStdOut(stdout);

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("flushcache",show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("flushcache", argv[0]);
    return 1;
  }

  FlushCache();
  return 0;
}
