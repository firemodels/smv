#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  fprintf(stdout, "This program creates a module file from a bash script by comparing\n");
  fprintf(stdout, "bash environments before and after an environment setting script is run.\n");
  fprintf(stdout, "This is done by running commands such as:\n");
  fprintf(stdout, "    env | sort > file1\n");
  fprintf(stdout, "    source environment_setting_script.sh\n");
  fprintf(stdout, "    env | sort > file2\n");
  fprintf(stdout, "in a bash shell and then running this program specifying file1 and file2\n");
  fprintf(stdout, "using the -f option");
#ifndef WIN32
  fprintf(stdout, " or running this program specifying the \n");
  fprintf(stdout, "environment_setting_script.sh using the -s option");
#endif
  fprintf(stdout, "\n\n");
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "  env2mod -f before_env after_env");
#ifndef WIN32
  fprintf(stdout, " -s script.sh");
#endif
  fprintf(stdout, " -m module_file");
  fprintf(stdout, "\n");
  fprintf(stdout, "    -f before_env after _env - two files containing sorted environment\n");
  fprintf(stdout, "       variables before and after a bash script has been run\n");
  fprintf(stdout, "    -m modulefile - file containing module commands or if '-', module\n");
  fprintf(stdout, "       module commands are output to the screen\n");
#ifndef WIN32
  fprintf(stdout, "    -s script.sh - bash script used to create a module\n");
#endif
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i, f_opt = 0, s_opt = 0;
  char *file1 = NULL, *file2 = NULL, *scriptfile = NULL, *modulefile_ptr=NULL;
  char file1val[1024], file2val[1024];
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
      i++;
      scriptfile = argv[i];
      if(FILE_EXISTS(scriptfile) == NO){
        fprintf(stderr, "***error: invalid or missing script file specified\n");
        Usage("env2mod", HELP_SUMMARY);
        return 1;
      }
      else{
        s_opt = 1;
      }
    }
    if(strncmp(arg, "-m", 2) == 0){
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
    char command[1024];

    strcpy(command,"env | sort > file1");
    system(command);

    strcpy(command,"source ");
    strcat(command,scriptfile);
    strcat(command," intel64 ; env | sort > file2");
    system(command);

    strcpy(file1val,"file1");
    strcpy(file2val,"file2");
    file1 = file1val;
    file2 = file2val;
  }
#endif
  CreateModule(file1, file2, modulefile_ptr);
  return 0;
}
