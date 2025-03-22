#include "options.h"
#include "dmalloc.h"
#include "string_util.h"
#include <math.h>

#include <string.h>
#include "smokeviewdefs.h"
#include "scontour2d.h"
#include "isobox.h"
#include "histogram.h"
#include "structures.h"
#include "datadefs.h"

/* ------------------ GetSliceFileHeader ------------------------ */

void GetSliceFileHeader(char *file, int *ip1, int *ip2, int *jp1, int *jp2, int *kp1, int *kp2, int *error){
  FILE *stream = NULL;
  int vals[6];

  stream = fopen(file, "rb");
  *error = 1;
  *ip1 = 0;
  *ip2 = 0;
  *jp1 = 0;
  *jp2 = 0;
  *kp1 = 0;
  *kp2 = 0;
  if(stream==NULL)return;
  fseek(stream, 3*(4+30+4), SEEK_CUR);
  fseek(stream, 4, SEEK_CUR); fread(vals, sizeof(int), 6, stream);
  *ip1 = vals[0];
  *ip2 = vals[1];
  *jp1 = vals[2];
  *jp2 = vals[3];
  *kp1 = vals[4];
  *kp2 = vals[5];
  *error = 0;
  fclose(stream);
}
