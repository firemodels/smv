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

void FlushCache(void){
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  if(argc == 1){
    Usage("flushcache",HELP_ALL);
    return 0;
  }

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
