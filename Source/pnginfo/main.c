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
#include "readimage.h"

/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[256];
  char gitdate[256];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("pnginfo [options] image_file\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);
  PRINTF("get FDS and Smokeview repo revisions from an image file\n\n");
  PRINTF("options:\n");
  PRINTF("-h - display this message\n");

  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *file=NULL;
  int get_revisions = 1;

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
      case 'h':
        Usage(HELP_ALL);
        return 1;
      default:
        Usage(HELP_ALL);
        return 1;
      }
    }
    else{
      if(file==NULL){
        file=argv[i];
      }
    }
  }
  if(file==NULL)return 1;
  FILE *stream;

  stream = fopen(file, "rb");
  if(stream == NULL)return 1;
  fclose(stream);

  unsigned char *image_buffer, *revision_data;
  int width, height, is_transparent, nrevision_data, nimage_buffer;
  int skip=4, channel=2;
  
  image_buffer =  ReadPNG(file, &width, &height, &is_transparent);
  nimage_buffer = width*height;

  revision_data = DecodeData(image_buffer, nimage_buffer, &nrevision_data, skip, channel);
  if(revision_data == NULL){
    printf("unknown\n");
  }
  else{
    printf("%s\n", revision_data);
  }
  return 0;
}
