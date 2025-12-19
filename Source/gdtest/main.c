#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int size1;

  size1 = sizeof(struct jpeg_compress_struct);

  printf("in test: %i:\n",size1);
}
