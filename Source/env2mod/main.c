#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "env2mod.h"
#include "datadefs.h"
#include "string_util.h"
#include "dmalloc.h"

/* ------------------ Usage ------------------------ */

void Usage(int option){
 char githash[LEN_BUFFER];
 char gitdate[LEN_BUFFER];

  GetGitInfo(githash,gitdate,NULL);    // get githash

  fprintf(stdout, "\nenv2mod [options] file1 file2\n");
  fprintf(stdout, "%s %s\n", githash, __DATE__);
  fprintf(stdout, "This program converts a bash script into a module file by comparing\n");
  fprintf(stdout, "the bash environment before and after the bash script is run. This\n");
  fprintf(stdout, "is done by running commands such as:\n");
  fprintf(stdout, "    env | sort > file1\n");
  fprintf(stdout, "    source bash_script.sh\n");
  fprintf(stdout, "    env | sort > file2\n");
  fprintf(stdout, "and then running env2mod specifying file1 and file2 with the -f option\n");
#ifndef _WIN32
  fprintf(stdout, "or by specifying the script file with the -s option\n");
#endif
  fprintf(stdout, "\n");
  fprintf(stdout, "options:\n");
#ifndef _WIN32
  fprintf(stdout, " -s script.sh");
#endif
  fprintf(stdout, " -m module_file");
  fprintf(stdout, "\n");
  fprintf(stdout, "    -f file1 file2 - two files containing sorted environment variables before\n");
  fprintf(stdout, "       and after a bash script has been run (not used if -s is specified)\n");
  fprintf(stdout, "    -m modulefile - file containing module commands or if '-' is specified,\n");
  fprintf(stdout, "       module commands are output to the screen\n");
#ifndef _WIN32
  fprintf(stdout, "    -s script.sh - bash script used to create a module file (not used if\n");
  fprintf(stdout, "       -f is specified).  If script.sh has arguemnts enclose in quotes as in:\n");
  fprintf(stdout, "        -s \"script.sh arg1 arg2\"\n");
#endif
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ SplitCommandline ------------------------ */

void SplitCommandline(char *commandline, char **command, char **args){
  int len;
  char *blank;

  *args = NULL;
  if(commandline[0] == '"')commandline[0] = ' ';
  len = strlen(commandline);
  if(commandline[len - 1] == '"')commandline[len - 1] = ' ';
  *command = TrimFrontBack(commandline);
  blank = strchr(*command,' ');
  if(blank==NULL)return;
  blank[0] = 0;
  *args = TrimFrontBack(blank+1);
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i, f_opt = 0, s_opt = 0;
  char *file1 = NULL, *file2 = NULL, *modulefile_ptr=NULL;
  char scriptfile[1024], *scriptfile_ptr;
  char *script_command, *script_args;
#ifndef _WIN32
  char file1val[1024], file2val[1024];
#endif
  char modulefile[1024];
  int error = 0;
  int create_script=0;

  strcpy(scriptfile,"scriptfile.sh");
  scriptfile_ptr = scriptfile;
  modulefile_ptr = modulefile;
  strcpy(modulefile, "modulefile");

  if(argc == 1){
    Usage(HELP_ALL);
    return 0;
  }

  initMALLOC();
  SetStdOut(stdout);

  common_opts opts = ParseCommonOptions(argc, argv);
  if(opts.show_help != 0){
    Usage(opts.show_help);
    return 1;
  }
  if(opts.show_version == 1){
    PRINTVERSION("env2mod", &opts);
    return 1;
  }

  for(i = 1; i < argc; i++){
    int lenarg;
    char *arg;


    arg = argv[i];
    lenarg = strlen(arg);
    if(arg[0] == '-'&&lenarg > 1){
      if(strncmp(arg, "-f", 2) == 0){
        i++;
        file1 = argv[i];
        if(FILE_EXISTS(file1) == NO){
          fprintf(stderr, "***error: file %s specified after the -f option not found\n",file1);
          error = 1;
        }
        i++;
        file2 = argv[i];
        if(FILE_EXISTS(file2) == NO)error = 1;
        if(error == 1){
          fprintf(stderr, "***error: file %s specified after the -f option not found\n",file2);
          error = 1;
        }
        else{
          f_opt = 1;
        }
      }
    }
    if(strncmp(arg, "-s", 2) == 0){
      i++;
      SplitCommandline(argv[i], &script_command, &script_args);
      if(FILE_EXISTS(script_command) == NO){
        fprintf(stderr, "***error: script %s not found\n",script_command);
        error = 1;
      }
      else{
        s_opt = 1;
      }
    }
    if(strncmp(arg, "-S", 2) == 0){
      create_script=1;
    }
    if(strncmp(arg, "-m", 2) == 0){
      i++;
      modulefile_ptr = argv[i];
    }
  }

  if(s_opt == 0 && f_opt == 0){
    fprintf(stderr, "***error: The -f or -s option must be specified\n");
    error = 1;
  }

#ifdef _WIN32
  if(f_opt == 0){
    fprintf(stderr, "***error: -f option required for Windows version of env2mod\n");
    error = 1;
  }
#else
  if(error==0&&s_opt == 1){
    char command[1024];

    strcpy(command,"env | sort > file1");
    system(command);

    strcpy(command,"source ");
    strcat(command,script_command);
    if(script_args!=NULL){
      strcat(command," ");
      strcat(command,script_args);
    }
    strcat(command," ; env | sort > file2");
    system(command);

    strcpy(file1val,"file1");
    strcpy(file2val,"file2");
    file1 = file1val;
    file2 = file2val;
  }
#endif
  if(error!=0)return 1;
  CreateModule(file1, file2, modulefile_ptr);
  if(create_script==1){
    CreateScript(file1, file2, scriptfile_ptr);
  }
#ifndef _WIN32
  if(s_opt==1){
    if(FileExistsOrig(file1))UNLINK(file1);
    if(FileExistsOrig(file2))UNLINK(file2);
  }
#endif
  return 0;
}
