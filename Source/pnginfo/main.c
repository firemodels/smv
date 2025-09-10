#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "string_util.h"
#include "dmalloc.h"

/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[256];
  char gitdate[256];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("pnginfo [options] image_file\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);
  PRINTF("add/get FDS and Smokeview repo revisions to/from an image file\n\n");
  PRINTF("options:\n");
  PRINTF("-g - get FDS/Smokeview revisions from image file\n");
  PRINTF("-h - display this message\n");
  PRINTF("-p - put FDS/Smokeview revisions into image file\n");

  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *filebase=NULL;
  int put_revisions = 1;

  initMALLOC();
  SetStdOut(stdout);

  if(argc == 1){
    Usage(HELP_ALL);
    return 1;
  }
  for(i=1;i<argc;i++){
    size_t lenarg;
    char *arg;

    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0] == '-' && lenarg>1){
      switch(arg[1]){
      case 'g':
        put_revisions = 0;
        break;
      case 'h':
        Usage(HELP_ALL);
        return 1;
      case 'p':
        put_revisions = 1;
        break;
      default:
        Usage(HELP_ALL);
        return 1;
      }
    }
    else{
      if(filebase==NULL){
        filebase=argv[i];
      }
    }
  }
  if(filebase!=NULL)printf("filebase=%s\n",filebase);

  return 0;
}
