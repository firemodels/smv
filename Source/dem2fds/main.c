#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "string_util.h"
#include "file_util.h"
#include "datadefs.h"
#include "MALLOCC.h"
#include "gd.h"
#include "dem_util.h"

/* ------------------ Usage ------------------------ */

void Usage(char *prog, int option){
 char githash[LEN_BUFFER];
 char gitdate[LEN_BUFFER];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stdout, "Create an FDS input file using elevation and image\n");
  fprintf(stdout, "  data obtained from http://viewer.nationalmap.gov \n\n");
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "  dem2fds [options] casename.in\n");
  fprintf(stdout, "  -dir dir      - directory containing map and elevation files\n");
  fprintf(stdout, "  -elevdir dir  - directory containing elevation files (if different than -dir directory)\n");
  fprintf(stdout, "  -fds          - specify fds input file [default: casename.fds]\n");
  fprintf(stdout, "  -geom         - represent terrain using using &GEOM keywords (experimental)\n");
  fprintf(stdout, "  -obst         - represent terrain using &OBST keywords \n");
  fprintf(stdout, "  -width w      - terrain image pixel width [default: 2048]\n");
  fprintf(stdout, "  -height h     - terrain image pixel height\n");
  fprintf(stdout, "  -jpeg         - generate a jpeg terrain image file otherwise generate a png image file\n");
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
  fprintf(stdout, "  -matl matl_id - specify a MATL ID for use with the -geom option \n");
  fprintf(stdout, "  -overlap      - assume that there is a 300 pixel overlap between maps.\n");
  fprintf(stdout, "  -show         - highlight image and fds scenario boundaries\n");
  fprintf(stdout, "  -surf1 surf_id - specify surf ID for use with OBSTs or geometry (interior to domain)\n");
  fprintf(stdout, "  -surf2 surf_id - specify surf ID for use with OBSTs or geometry (adjacent to domain)\n");
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  int gen_fds = FDS_OBST;
  char *casename = NULL, *last=NULL;
  char file_default[LEN_BUFFER];
  char casename_fds[LEN_BUFFER], image_file[LEN_BUFFER];
  elevdata fds_elevs;
  int fatal_error = 0;

  if(argc == 1){
    Usage("dem2fds",HELP_ALL);
    return 0;
  }

  strcpy(casename_fds, "");
  strcpy(file_default, "terrain");
  strcpy(image_dir,    ".");
  strcpy(elev_dir,     "");
  strcpy(surf_id1,     "surf1");
  strcpy(surf_id2,     "surf2");
  strcpy(matl_id,      "matl1");
  strcpy(image_type,   ".png");

  initMALLOC();
  SetStdOut(stdout);

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("dem2fds",show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("dem2fds", argv[0]);
    return 1;
  }

  for(i = 1; i<argc; i++){
    int lenarg;
    char *arg;


    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      if(strncmp(arg, "-dir", 4) == 0){
        i++;
        if(FILE_EXISTS(argv[i]) == NO)fatal_error = 1;
      }
      else if(strncmp(arg, "-width", 6)==0){
        int image_width = 0;

        i++;
        sscanf(argv[i], "%i", &image_width);
        if(image_width>0){
          terrain_image_width = image_width;
          terrain_image_height = 0;
        }
      }
      else if(strncmp(arg, "-height", 7)==0){
        int image_height = 0;

        i++;
        sscanf(argv[i], "%i", &image_height);
        if(image_height>0){
          terrain_image_height = image_height;
          terrain_image_width = 0;
        }
      }
      else if(strncmp(arg, "-fds", 4) == 0){
        i++;
        strcpy(casename_fds, argv[i]);
      }
      else if(strncmp(arg, "-jpeg", 5)==0){
        strcpy(image_type, ".jpg");
      }
      else if(strncmp(arg, "-elevdir", 8) == 0) {
        i++;
        if(FILE_EXISTS(argv[i]) == NO)fatal_error = 1;
      }
    }
    else{
      if(casename == NULL)casename = argv[i];
    }
  }

  if(casename==NULL){
    fprintf(stderr, "\n***error: input file not specified\n");
    return 1;
  }
  if(FILE_EXISTS(casename)==NO){
    fprintf(stderr, "\n***error: input file %s does not exist\n",casename);
    return 1;
  }
  if(fatal_error == 1) {
    fprintf(stderr, "\ncase: %s\n", casename);
  }

  fatal_error=0;
  for(i = 1; i<argc; i++){
    int lenarg;
    char *arg;


    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      if(strncmp(arg, "-dir", 4) == 0){
        i++;
        if(FILE_EXISTS(argv[i]) == YES){
          strcpy(image_dir, argv[i]);
          if(strlen(elev_dir) == 0) {
            strcpy(elev_dir, image_dir);
          }
        }
        else {
          fprintf(stderr, "***error: directory %s does not exist or cannot be accessed\n",argv[i]);
          fatal_error = 1;
        }
      }
      else if(strncmp(arg, "-elevdir", 8) == 0) {
        i++;
        if(FILE_EXISTS(argv[i]) == YES) {
          strcpy(elev_dir, argv[i]);
        }
        else {
          fprintf(stderr, "***error: directory %s does not exist or cannot be accessed\n", argv[i]);
          fatal_error = 1;
        }
      }
      else if(strncmp(arg, "-geom", 5) == 0 ){
        gen_fds = FDS_GEOM;
      }
      else if(strncmp(arg, "-matl", 5) == 0){
        i++;
        strcpy(matl_id, argv[i]);
      }
      else if(strncmp(arg, "-obst", 5) == 0 ){
        gen_fds = FDS_OBST;
      }
      else if(strncmp(arg, "-overlap", 8) == 0){
        overlap_size = 300;
      }
      else if(strncmp(arg, "-show", 5) == 0){
        show_maps = 1;
      }
      else if(strncmp(arg, "-surf", 5) == 0 || strncmp(arg, "-surf1", 6) == 0){
        i++;
        strcpy(surf_id1, argv[i]);
      }
      else if (strncmp(arg, "-surf2", 6) == 0) {
        i++;
        strcpy(surf_id2, argv[i]);
      }
      else if(strncmp(arg, "-fds", 4) == 0){
        i++;
        strcpy(casename_fds, argv[i]);
      }
      else if(strncmp(arg, "-width", 6)==0){
        i++;
      }
      else if(strncmp(arg, "-height", 7)==0){
        i++;
      }
      else if(strncmp(arg, "-jpeg", 5)==0){
      }
      else{
        Usage("dem2fds",HELP_ALL);
        return 1;
      }
    }
    else{
      if(casename == NULL)casename = argv[i];
    }
  }

  if(fatal_error == 1) return 1;

  if(strlen(elev_dir) == 0) {
    strcpy(elev_dir, image_dir);
  }
  if(casename == NULL)casename = file_default;
  if(strlen(casename_fds) == 0){
    strcpy(casename_fds, casename);
    last = strrchr(casename_fds, '.');
    if(last!=NULL)last[0]=0;
    strcat(casename_fds, ".fds");
  }
  strcpy(image_file, casename_fds);
  last = strrchr(image_file, '.');
  if(last != NULL)last[0] = 0;
  strcat(image_file,image_type);

  if(GetElevations(casename, image_file, image_type, &fds_elevs)==1) {
     GenerateFDSInputFile(casename, casename_fds, &fds_elevs, gen_fds);
  }
  return 0;
}
