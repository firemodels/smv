#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "string_util.h"
#include "MALLOCC.h"

/* ------------------ ParseBytes ------------------------ */

void ParseBytes(char *buffer, FILE *stream, unsigned char *bytes, int *nbytes){
}

/* ------------------ ParseByteTypes ------------------------ */

void ParseByteTypes(FILE *stream, int *byte_types){

}

void HighRes(unsigned char *bytes, unsigned char *bytes2, int nbytes, int nrows, int ncols);

/* ------------------ main ------------------------ */

int main(int argc, char **argv){

  initMALLOC();
  SetStdOut(stdout);

#define LENBUFFER 255
  for(;;){
    unsigned char bytes[1000];
    unsigned char bytes2[1000];
    int byte_types[5];
    char buffer[LENBUFFER], buffcopy[LENBUFFER], *buff;

    if(fgets(buffer, LENBUFFER, stdin)==NULL)break;
    if(strstr(buffer, "static const")==NULL){
      printf("%s\n", buffer);
      continue;
    }
    if(strstr(buffer, "GLubyte")!=NULL){
      int nbytes, nrows, ncols;

      strcpy(buffcopy, buffer);
      ParseBytes(buffer, stdin, bytes, &nbytes);
      ParseByteTypes(stdin, byte_types);
      nrows = byte_types[0];
      ncols = byte_types[1];
      HighRes(bytes, bytes2, nbytes, nrows, ncols);
    }
    else{
      printf("%s\n", buffer);
    }
  }
  fprintf(stderr, "complete\n");


  return 0;
}
