#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "string_util.h"
#include "file_util.h"
#include "dmalloc.h"
#ifdef WIN32
#include <windows.h>
#endif

#define LEN_BUFFER 1024

#ifdef _DEBUG
int debug_print = 1;
#else
int debug_print=0;
#endif
float flush_size=8.0;
int write_buffer=0;

/* ------------------ Usage ------------------------ */

void Usage(int option){
 char githash[LEN_BUFFER];
 char gitdate[LEN_BUFFER];

  GetGitInfo(githash,gitdate);    // get githash


  fprintf(stdout, "\nflush [options]\n");
  fprintf(stdout, "%s %s\n\n", githash, __DATE__);
  fprintf(stdout, "flush the cache\n\n");
  PRINTF("options:\n");
  PRINTF("%s\n", " -g size - allocate a memory buffer of 'size' GB");
  PRINTF("%s\n", " -w      - initialize buffer");
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ FlushCache ------------------------ */
#define BUFFERSIZE 250000000
void FlushCache(float flush_size_arg){
  int i, nbuffers;

  nbuffers = (int)(flush_size_arg+0.5);
  for(i = 0;i<nbuffers;i++){
    int *buffptr;
    int j;

    if(debug_print==1)printf("Allocating buffer %i",i+1);
    NewMemory((void **)&buffptr, sizeof(int)*BUFFERSIZE);
    if(buffptr==NULL){
      if(debug_print==1)printf(" - failed\n");
      continue;
    }
    if(write_buffer==1){
      for(j = 0;j<BUFFERSIZE;j++){
        buffptr[j] = 1;
      }
    }
    if(debug_print==1)printf(" - complete\n");
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;

  initMALLOC();
  SetStdOut(stdout);

  common_opts opts = ParseCommonOptions(argc, argv);
  if(opts.show_help!=0){
    Usage(opts.show_help);
    return 1;
  }
  if(opts.show_version==1){
    PRINTVERSION("flushcache", &opts);
    return 1;
  }

  for(i = 1; i<argc; i++){
    int lenarg;
    char *arg;


    arg = argv[i];
    lenarg = strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      if(strncmp(arg, "-g", 2)==0){
        i++;
        sscanf(argv[i], "%f", &flush_size);
      }
      if(strncmp(arg, "-w", 2)==0){
        write_buffer=1;
      }
    }
  }


  FlushCache(flush_size);
  return 0;
}
