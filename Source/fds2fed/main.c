#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fds2fed.h"

/* ------------------ usage ------------------------ */

void Usage(int option){
  char githash[100];
  char gitdate[100];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("fds2fed [options] smv_case1\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);

  PRINTF("fds2fed computes fed slices\n\n");
  PRINTF("options:\n");

  UsageCommon(HELP_SUMMARY);

  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *smvbase=NULL, *arg;
  char *sourcedir=NULL;
  char smvfile[1024], smvfullfile[1024];
  char fedsmvfile[1024], fedsmvfullfile[1024];

  SetStdOut(stdout);
  initMALLOC();

  common_opts opts = ParseCommonOptions(argc, argv);
  if(opts.show_help!=0){
    Usage(opts.show_help);
    return 0;
  }
  if(opts.show_version==1){
    PRINTVERSION("fds2fed", &opts);
    return 0;
  }
    if(argc==1){
    PRINTVERSION("fds2fed ", &opts);
    return 0;
  }
  for(i=1;i<argc;i++){
    arg=argv[i];
    if(arg[0]=='-'&&strlen(arg)>1){
      switch(arg[1]){
      case 't':
        break;
      default:
        Usage(HELP_ALL);
        return 1;
      }
    }
    else{
      if(smvbase==NULL){
        smvbase=argv[i];
      }
    }
  }
  if(smvbase == NULL)return 0;
  char *ext;

  ext = strrchr(smvbase, '.');
  if(ext != NULL)ext[0] = 0;

  strcpy(smvfile,smvbase);
  strcat(smvfile,".smv");
  FullFile(smvfullfile,sourcedir,smvfile);

  strcpy(fedsmvfile, smvbase);
  strcat(fedsmvfile, ".fedsmv");
  FullFile(fedsmvfullfile, sourcedir, fedsmvfile);

  ReadSMV(smvfullfile);
  MakeFED();
  MakeFEDSmv(fedsmvfullfile);
  MakeFEDSlices();
  return 0;
}
