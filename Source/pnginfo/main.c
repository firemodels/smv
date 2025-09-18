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
  PRINTF("-h    - display this message\n");
  PRINTF("-html - convert line feeds to <br> html tags\n");

  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *file=NULL;
  int use_html = 0;

  initMALLOC();
  SetStdOut(stdout);

  common_opts opts = ParseCommonOptions(argc, argv);
  if(opts.show_help!=0){
    Usage(opts.show_help);
    return 0;
  }
  if(opts.show_version==1){
    PRINTVERSION("pnginfo", &opts);
    return 0;
  }
  for(i=1;i<argc;i++){
    size_t lenarg;
    char *arg;

    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0] == '-' && lenarg>1){
      if(strcmp(arg, "-h") == 0 || strcmp(arg, "-help") == 0){
        Usage(HELP_ALL);
        return 1;
      }
      else if(strcmp(arg, "-html") == 0){
        use_html = 1;
      }
      else{
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

  stream = FOPEN(file, "rb");
  if(stream == NULL)return 1;
  fclose(stream);

  unsigned char *image_buffer, *revision_data;
  int width, height, is_transparent, nrevision_data, nimage_buffer;
  int skip=4, channel=2;

  image_buffer =  ReadPNG(file, &width, &height, &is_transparent);
  nimage_buffer = width*height;

  revision_data = DecodeData(image_buffer, nimage_buffer, &nrevision_data, skip, channel);
  if(revision_data == NULL){
    if(use_html == 1){
      printf("FDS revision unavailable<br>SMV revision unavailable\n");
    }
    else{
      printf("\nFDS revision unavailable\nSMV revision unavailable\n");
    }
  }
  else{
    if(use_html == 1){
      int i,ibeg=0;

      if(strlen((char *)revision_data)>=4){
        if((char *)strncmp(revision_data,"<br>",4)==0)ibeg=4;
      }
      for(i = ibeg; i < strlen((char *)revision_data); i++){
        if(revision_data[i] == '\n'){
          printf("<br>");
        }
        else{
          printf("%c", revision_data[i]);
        }
      }
    }
    else{
      printf("%s\n", revision_data);
    }
  }
  return 0;
}
