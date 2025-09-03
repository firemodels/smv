#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svzip.h"

/* ------------------ GetFileBounds ------------------------ */

int GetFileBounds(char *file, float *valmin, float *valmax){
  FILE *stream;
  char buffer[255];
  float t, vmin, vmax;

  stream = FOPEN(file, "r");
  if(stream == NULL || fgets(buffer, 255, stream) == NULL){
    *valmin = 1.0;
    *valmax = 0.0;
    if(stream != NULL)fclose(stream);
    return 0;
  }
  sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
  *valmin = vmin;
  *valmax = vmax;
  for(;;){
    if(fgets(buffer, 255, stream) == NULL)break;
    sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
    *valmin = MIN(*valmin, vmin);
    *valmax = MAX(*valmax, vmax);
  }
  fclose(stream);
  return 1;
}
