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

/* ------------------ Usage ------------------------ */

void Usage(char *prog){
 char githash[100];
 char gitdate[100];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stdout, "Create a hash of a specified file\n");
  fprintf(stdout, "  data obtained from http://viewer.nationalmap.gov \n\n");
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "  hashfile file\n");
  fprintf(stdout, "  -md5 - create an md5 hash [default]\n");
  fprintf(stdout, "  -help     - display this message\n");
  fprintf(stdout, "  -version  - show version information\n");
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *casename = NULL;
  unsigned char *hash = NULL;

  if(argc == 1){
    Usage("hashfile");
    return 0;
  }

  SetStdOut(stdout);
  initMALLOC();

  for(i = 1; i<argc; i++){
    int lenarg;
    char *arg;

    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      if(strncmp(arg, "-help", 5) == 0 || strncmp(arg, "-h", 2) == 0){
        Usage("hashfile");
        return 1;
      }
      else if(strncmp(arg, "-version", 8) == 0|| strncmp(arg, "-v", 2) == 0){
        PRINTversion("hashfile",argv[0]);
        return 1;
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

  hash=GetHash(casename);
  if(hash==NULL){
    printf(" : %s\n", casename);
  }
  else{
    printf("%s : %s\n", hash,casename);
  }
  return 0;
}
