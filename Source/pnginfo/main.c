#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "gd.h"
#include "string_util.h"
#include "dmalloc.h"
#include "readimage.h"

/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[256];
  char gitdate[256];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("pnginfo [options] image_file.png\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);
  PRINTF("get FDS and Smokeview repo revisions from an image file\n\n");
  PRINTF("options:\n");
#ifdef pp_ENCODE_FILE
  PRINTF("-encode file - encode data found in file\n");
  PRINTF("-out file    - output modified png file to file [default: image_file_mod.png]\n");
#endif
  PRINTF("-h           - display this message\n");
  PRINTF("-html        - convert line feeds to <br> html tags\n");

  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *png_infile=NULL, *encode_file=NULL, *png_outfile=NULL;
  FILE *stream_png_infile, *stream_png_outfile;
  int use_html = 0, encode = 0;

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
#ifdef pp_ENCODE_FILE
      else if(strcmp(arg, "-encode") == 0){
        int lenfile;
        char *file_arg;

        i++;
        file_arg = (char *)argv[i];
        lenfile = strlen(file_arg)+1;
        NewMemory((void **)&encode_file, lenfile);
        strcpy(encode_file, file_arg);
        encode = 1;
      }
      else if(strcmp(arg, "-out") == 0){
        int lenfile;
        char *file_arg;

        i++;
        file_arg = (char *)argv[i];
        lenfile = strlen(file_arg)+1;
        NewMemory((void **)&png_outfile, lenfile);
        strcpy(encode_file, file_arg);
      }
#endif
      else{
        Usage(HELP_ALL);
        return 1;
      }
    }
    else{
      if(png_infile==NULL){
        png_infile=argv[i];
      }
    }
  }
  if(png_infile == NULL){
    printf("***error: png file missing\n");
    return 1;
  }
  char *pngext;

  pngext = strrchr(png_infile, '.');
  if(pngext == NULL || strcmp(pngext, ".png") != 0){
    printf("***error: file %s not a png file\n", png_infile);
    return 1;
  }

  stream_png_infile = FOPEN(png_infile, "rb");
  if(stream_png_infile == NULL){
    printf("***error: file %s could not be opened\n", png_infile);
    return 1;
  }
  fclose(stream_png_infile);

  unsigned char *image_buffer, *revision_data;
  int width, height, is_transparent, nrevision_data;
  int skip = 4, channel = 2;

  image_buffer = ReadPNG(png_infile, &width, &height, &is_transparent);
  if(width <= 0 || height <= 0){
    printf("***error: image file %s has zero width or height\n", png_infile);
    return 1;
  }

  if(encode == 0){
    revision_data = DecodePNGData(image_buffer, width*height, &nrevision_data, skip, channel);
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
        int i, ibeg = 0;

        if(strlen((char *)revision_data) >= 4){
          if(strncmp((char *)revision_data, "<br>", 4) == 0)ibeg = 4;
        }
        for(i = ibeg; i < strlen((char *)revision_data); i++){
          if((char)revision_data[i] == '\n'){
            printf("<br>");
          }
          else{
            printf("%c", (char)revision_data[i]);
          }
        }
      }
      else{
        printf("%s\n", (char *)revision_data);
      }
    }
  }
  else{
    FILE *stream_encode_file = NULL;
    unsigned char *buffer_encode_file = NULL;
    int nencode_file;

    if(png_outfile == NULL){
      NewMemory((void **)&png_outfile, strlen(png_infile)+5);
      char *ext;

      strcpy(png_outfile, png_infile);
      ext = strrchr(png_outfile, '.');
      if(ext != NULL)*ext=0;
      strcat(png_outfile, "_mod.png");
    }
    stream_png_outfile = FOPEN(png_outfile, "wb");
    if(stream_png_outfile == NULL){
      printf("***error: unable to open png file %s for writing", png_outfile);
      return 1;
    }
    if(encode_file != NULL){
      FILE *stream;

      stream = fopen(encode_file, "r");
      if(stream == NULL){
        printf("***error: file %s could not be opened for input\n", encode_file);
        return 1;
      }
      fclose(stream);
    }

    stream_encode_file = fopen(encode_file, "rb");
    if(stream_encode_file == NULL){
      printf("***error: file %s could not be opened\n", encode_file);
    }
    nencode_file = GetFileSizeSMV(encode_file);
    if(nencode_file <= 0){
      printf("***error: file %s is empty\n",encode_file);
      return 1;
    }
    NewMemory((void **)&buffer_encode_file, nencode_file+1);
    fread(buffer_encode_file, 1, nencode_file, stream_encode_file);
    fclose(stream_encode_file);
    int nencode_file_max = (width*height - 64)/32;
    if(nencode_file>nencode_file_max){
      printf("***warning: maximum number characters that can be encoded in %s is %i\n", png_infile, nencode_file_max);
      printf("            file %s truncated to %i characters\n", encode_file, nencode_file_max);
      nencode_file = nencode_file_max;
    }
    EncodePNGData(image_buffer, width * height, buffer_encode_file, nencode_file, skip, channel);

    gdImagePtr RENDERimage;

    RENDERimage = gdImageCreateTrueColor(width, height);

    int count = 0;
    for(i = 0; i < height; i++){
      int j;

      for(j = 0; j < width; j++){
        unsigned char r, g, b;
        unsigned int rgb_local;

        r = image_buffer[count++];
        g = image_buffer[count++];
        b = image_buffer[count++];
        count++;
        rgb_local = (r << 16) | (g << 8) | b;
        gdImageSetPixel(RENDERimage, j, height-1-i, rgb_local);
      }
    }
    gdImagePng(RENDERimage, stream_png_outfile);
  }
  return 0;
}
