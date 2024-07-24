#include "options.h"
#ifdef pp_OPEN_TEST
#include <stdio.h>
int open_files=0;

/* ------------------ fopen_counting ------------------------ */

FILE *fopen_counting(const char *path, const char *mode)
{
  FILE *v;

  v = fopen(path, mode);
  if(v != NULL){
    open_files++;
  }
  return v;
}

/* ------------------ fclose_counting ------------------------ */

int fclose_counting(FILE *fp)
{
  int v;
  v = fclose(fp);
  if(v != EOF){
    open_files--;
  }
  return v;
}
#endif