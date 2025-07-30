#define IN_MAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "string_util.h"
#include "file_util.h"
#include "dmalloc.h"

/* ------------------ Usage ------------------------ */

void Usage(int option){
 char githash[100];
 char gitdate[100];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\nhashfile [options] file\n");
  fprintf(stdout, "%s %s\n\n", githash, __DATE__);
  fprintf(stdout, "Compute the md5, sha1 or sha256 hash of a specified file\n\n");
  fprintf(stdout, "options:\n");
  UsageCommon(HELP_SUMMARY);
  if(option==HELP_ALL)UsageCommon(HELP_ALL);
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  char *casename = NULL;
  unsigned char *hash = NULL;

  if(argc == 1){
    Usage(HELP_ALL);
    return 0;
  }

  SetStdOut(stdout);
  initMALLOC();

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage(show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("hashfile");
    return 1;
  }
  casename = argv[argc-1];

  if(casename==NULL||strlen(casename)==0||casename[0]=='-'){
    fprintf(stderr, "\n***error: input file not specified\n");
    return 1;
  }
  if(FILE_EXISTS(casename)==NO){
    fprintf(stderr, "\n***error: input file %s does not exist\n",casename);
    return 1;
  }

  // output MD5 hash

  if(hash_option==HASH_MD5||hash_option==HASH_ALL){
    hash = GetHashMD5(casename);
    if(hash==NULL){
      printf("***error: MD5 computation of %s failed\n", casename);
    }
    else{
      printf("%s (MD5): %s\n", hash, casename);
    }
  }

  // output SHA1 hash

  if(hash_option==HASH_SHA1||hash_option==HASH_ALL){
    hash = GetHashSHA1(casename);
    if(hash==NULL){
      printf("***error: SHA1 computation of %s failed\n", casename);
    }
    else{
      printf("%s (SHA1): %s\n", hash, casename);
    }
  }

  // output SHA256 hash

  if(hash_option==HASH_SHA256||hash_option==HASH_ALL){
    hash = GetHashSHA256(casename);
    if(hash==NULL){
      printf("***error: SHA256 computation of %s failed\n", casename);
    }
    else{
      printf("%s (SHA256): %s\n", hash, casename);
    }
  }
  return 0;
}
