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

#define MD5    0
#define SHA256 1
#define SHA1   2
#define ALL    3

/* ------------------ Usage ------------------------ */

void Usage(char *prog){
 char githash[100];
 char gitdate[100];

  GetGitInfo(githash,gitdate);    // get githash

  fprintf(stdout, "\n%s (%s) %s\n", prog, githash, __DATE__);
  fprintf(stdout, "Compute the md5, sha1 or sha256 hash of a specified file\n");
  fprintf(stdout, "Usage:\n");
  fprintf(stdout, "  hashfile [option] file\n");
  fprintf(stdout, "  -all     - compute all hashes of a file\n");
  fprintf(stdout, "  -help    - display this message\n");
  fprintf(stdout, "  -md5     - compute the md5 hash of a file[default]\n");
  fprintf(stdout, "  -sha1    - compute the sha1 hash of a file\n");
  fprintf(stdout, "  -sha256  - compute the sha256 hash of a file\n");
  fprintf(stdout, "  -version - show version information\n");
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
  char *casename = NULL;
  unsigned char *hash = NULL;
  int hashtype = MD5;

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
      int badarg;

      badarg = 1;
      if(strncmp(arg, "-help", 5) == 0 || strncmp(arg, "-h", 2) == 0){
        Usage("hashfile");
        return 1;
      }
      else if(strcmp(arg, "-version") == 0|| strcmp(arg, "-v") == 0){
        PRINTversion("hashfile",argv[0]);
        return 1;
      }
      else if(strcmp(arg, "-sha256")==0){
        badarg = 0;
        hashtype = SHA256;
      }
      else if(strcmp(arg, "-sha1")==0){
        badarg = 0;
        hashtype = SHA1;
      }
      else if(strcmp(arg, "-md5")==0){
        badarg = 0;
        hashtype = MD5;
      }
      else if(strncmp(arg, "-all", 4)==0||strncmp(arg, "-a", 2)==0){
        badarg = 0;
        hashtype = ALL;
      }
      if(badarg == 1){
        fprintf(stderr, "\n***error: unknown parameter: %s\n\n", arg);
        Usage("hashfile");
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

  // output MD5 hash

  if(hashtype==MD5||hashtype==ALL){
    hash = GetHashMD5(casename);
    if(hash==NULL){
      printf("***error: MD5 computation of %s failed\n", casename);
    }
    else{
      printf("%s (MD5): %s\n", hash, casename);
    }
  }

  // output SHA1 hash

  if(hashtype==SHA1||hashtype==ALL){
    hash = GetHashSHA1(casename);
    if(hash==NULL){
      printf("***error: SHA1 computation of %s failed\n", casename);
    }
    else{
      printf("%s (SHA1): %s\n", hash, casename);
    }
  }

  // output SHA256 hash

  if(hashtype==SHA256||hashtype==ALL){
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
