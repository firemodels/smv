#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"
#include "gd.h"

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int size1;

  size1 = sizeof(struct jpeg_compress_struct);

  printf("in gdtest/main: %i:\n",size1);
  gdJpegTest();
  JpegTest("from main");
}
