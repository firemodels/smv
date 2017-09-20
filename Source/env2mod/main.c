#define INMAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"
#include "env2mod.h"
#include "datadefs.h"
#include "string_util.h"
#include "MALLOC.h"

/* ------------------ Usage ------------------------ */

void Usage(char *prog, int option){
 char githash[LEN_BUFFER];
 char gitdate[LEN_BUFFER];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stdout, "Create a module from and environmente script\n");
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "  env2mod script.sh\n");
  UsageCommon(prog, HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(prog, HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *casename = NULL;
  char file_default[LEN_BUFFER];
  int fatal_error = 0;

  if(argc == 1){
    Usage("env2mod",HELP_ALL);
    return 0;
  }

  initMALLOC();
  SetStdOut(stdout);

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("env2mod",show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("env2mod", argv[0]);
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
      }
      else{
        Usage("env2mod",HELP_ALL);
        return 1;
      }
    }
    else{
      if(casename == NULL)casename = argv[i];
    }
  }

  if(fatal_error == 1) return 1;

  if(casename == NULL)casename = file_default;
  return 0;
}
