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
  fprintf(stdout, "Create a module from a bash script\n");
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "  env2mod -f file1 file2 -s script.sh -m modulefile\n");
  fprintf(stdout, "    -f file1 file2 - construct a module from files file1 and file2\n");
  fprintf(stdout, "    where file1 is created before running the script.sh and file2\n");
  fprintf(stdout, "    was after running script.sh .  In each case using:\n");
  fprintf(stdout, "        env | sort > filei\n");
  fprintf(stdout, "    -m modulefile - file containing module commands\n");
  fprintf(stdout, "    -s script.sh - bash script used to create a module\n");
  UsageCommon(prog, HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(prog, HELP_ALL);
  }
}

/* ------------------ CreateModule ------------------------ */

void CreateModule(char *file1, char* file2, char *modulefile_ptr){
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i, fatal_error = 0, f_opt = 0, s_opt = 0;
  char *file1 = NULL, *file2 = NULL, *scriptfile = NULL, *modulefile_ptr=NULL;
  char modulefile[1024];

  modulefile_ptr = modulefile;
  strcpy(modulefile, "modulefile");

  if(argc == 1){
    Usage("env2mod", HELP_ALL);
    return 0;
  }

  initMALLOC();
  SetStdOut(stdout);

  ParseCommonOptions(argc, argv);
  if(show_help != 0){
    Usage("env2mod", show_help);
    return 1;
  }
  if(show_version == 1){
    PRINTVERSION("env2mod", argv[0]);
    return 1;
  }

  for(i = 1; i < argc; i++){
    int lenarg;
    char *arg;


    arg = argv[i];
    lenarg = strlen(arg);
    if(arg[0] == '-'&&lenarg > 1){
      if(strncmp(arg, "-f", 2) == 0){
        int error = 0;

        i++;
        file1 = argv[i];
        if(FILE_EXISTS(file1) == NO)error = 1;
        i++;
        file2 = argv[i];
        if(FILE_EXISTS(file2) == NO)error = 1;
        if(error == 1){
          fprintf(stderr, "***error: invalid or missing files specified after the -f option\n");
          Usage("env2mod", HELP_SUMMARY);
          return 1;
        }
        else{
          f_opt = 1;
        }
      }
    }
    if(strncmp(arg, "-s", 2) == 0){
      int error = 0;

      i++;
      scriptfile = argv[i];
      if(FILE_EXISTS(scriptfile) == NO)error = 1;
      if(error == 1){
        fprintf(stderr, "***error: invalid or missing script file specified\n");
        Usage("env2mod", HELP_SUMMARY);
        return 1;
      }
      else{
        s_opt = 1;
      }
    }
    if(strncmp(arg, "-m", 2) == 0){
      int error = 0;

      i++;
      modulefile_ptr = argv[i];
    }
  }

  if(s_opt == 0 && f_opt == 0){
    fprintf(stderr, "***error: The -f or -s option must be specified\n");
    Usage("env2mod", HELP_SUMMARY);
    return 1;
  }

#ifdef WIN32
  if(f_opt == 0){
    fprintf(stderr, "***error: -f option required for Windows version of env2mod\n");
    Usage("env2mod", HELP_SUMMARY);
    return 1;
  }
#else
  if(s_opt == 1){

  }
#endif
  CreateModule(file1, file2, modulefile_ptr);
  return 0;
}
