#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fds2fed.h"

/* ------------------ usage ------------------------ */

void Usage(char *prog, int option){
  char smv_version[100];
  char githash[100];
  char gitdate[100];

  GetProgVersion(smv_version);  // get Smokeview version (ie 5.x.z)
  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("  %s [options] smv_case1 \n", prog);
  PRINTF("    version: %s (githash %s) - %s\n\n", smv_version, githash, __DATE__);

  PRINTF("  fds2fed computes fed slices\n");

  UsageCommon(HELP_SUMMARY);

  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
  PRINTF("\n  smv_case1,smv_case2 - Two smokeview cases to compare.\n");
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *smvbase=NULL, *arg;
  char *sourcedir=NULL;
  char smvfile[1024], smvfullfile[1024];


  SetStdOut(stdout);
  initMALLOC();

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("fds2fed",show_help);
    return 0;
  }
  if(show_version==1){
    PRINTVERSION("fds2fed", argv[0]);
    return 0;
  }
    if(argc==1){
    PRINTVERSION("fds2fed ",argv[0]);
    return 0;
  }
  for(i=1;i<argc;i++){
    arg=argv[i];
    if(arg[0]=='-'&&strlen(arg)>1){
      switch(arg[1]){
      case 't':
        break;
      default:
        Usage("fds2fed",HELP_ALL);
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

  strcpy(smvfile,smvbase);
  strcat(smvfile,".smv");
  FullFile(smvfullfile,sourcedir,smvfile);
  ReadSMV(smvfullfile);
  MakeFED();

  return 0;
}
