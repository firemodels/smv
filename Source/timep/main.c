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

#ifndef START_TIMER
#define START_TIMER(a) a = (float)clock()/(float)CLOCKS_PER_SEC;
#endif
#ifndef STOP_TIMER
#define STOP_TIMER(a) a = (float)clock()/(float)CLOCKS_PER_SEC-a;
#endif

#define LEN_BUFFER 1024
 
int debug_print=0;
int nbuffers=16;

/* ------------------ Usage ------------------------ */

void Usage(char *prog, int option){
 char githash[LEN_BUFFER];
 char gitdate[LEN_BUFFER];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stdout, "get timep\n");
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i, nargs;
  char command_line[1000];
  float cpu_time=0.0;

  initMALLOC();
  SetStdOut(stdout);

  nargs=ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("timep",show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("timep", argv[0]);
    return 1;
  }
  if(nargs<argc){
    strcpy(command_line,"");
    for(i = nargs; i<argc; i++){
      strcat(command_line, argv[i]);
      strcat(command_line, " ");
    }
    START_TIMER(cpu_time);
    system(command_line);
    STOP_TIMER(cpu_time);
  }
  printf("%f\n", cpu_time);

  return 0;
}
