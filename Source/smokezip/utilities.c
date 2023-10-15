#include "options.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zlib.h"
#include "svzip.h"
#include "MALLOCC.h"
#include "datadefs.h"

int iseed=0;

/* ------------------ GetFileBounds ------------------------ */

int GetFileBounds(char *file, float *valmin, float *valmax){
  FILE *stream;
  char buffer[255];
  float t, vmin, vmax;

  stream = fopen(file, "r");
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

/* ------------------ Normal ------------------------ */

void Normal(unsigned short *v1, unsigned short *v2, unsigned short *v3, float *normal, float *area){
  float u[3], v[3];

  float norm2;

  u[0]=v2[0]-v1[0];
  u[1]=v2[1]-v1[1];
  u[2]=v2[2]-v1[2];

  v[0]=v3[0]-v1[0];
  v[1]=v3[1]-v1[1];
  v[2]=v3[2]-v1[2];

  normal[0] = u[1]*v[2] - u[2]*v[1];
  normal[1] = u[2]*v[0] - u[0]*v[2];
  normal[2] = u[0]*v[1] - u[1]*v[0];

  norm2 = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
  *area = norm2/2.0;

  normal[0]/=norm2;
  normal[1]/=norm2;
  normal[2]/=norm2;
}

/* ------------------ RandABsdir ------------------------ */

void RandABsdir(float xyz[3], int dir){
  float x=1.0, y=1.0, z=1.0;
  float sum;

  sum=x*x+y*y+z*z;
  while(sum>1.0||sum==0.0){
    x = Rand1D(0.0,1.0);
    y = Rand1D(0.0,1.0);
    z = Rand1D(0.0,1.0);
    sum=x*x+y*y+z*z;
  }
  xyz[0]=x/sqrt(sum);
  xyz[1]=y/sqrt(sum);
  xyz[2]=z/sqrt(sum);
  if(abs(dir)>=1&&abs(dir)<=3){
    if(dir>0){
      xyz[dir]=ABS(xyz[dir]);
    }
    else{
      xyz[-dir]=-ABS(xyz[-dir]);
    }
  }
}

/* ------------------ Rand1D ------------------------ */

float Rand1D(float xmin, float xmax){
  float val;

  if(iseed==0){
    iseed=1;
    srand(iseed);
  }

  val = xmin + (xmax-xmin)*(float)rand()/(float)RAND_MAX;
  return val;
}

/* ------------------ Rand2D ------------------------ */

void Rand2D(float xy[2], float xmin, float xmax, float ymin, float ymax){
  xy[0]=Rand1D(xmin,xmax);
  xy[1]=Rand1D(ymin,ymax);
}

/* ------------------ rand_3d ------------------------ */

void Rand3D(float xyz[3], float xmin, float xmax, float ymin, float ymax, float zmin, float zmax){
  xyz[0]=Rand1D(xmin,xmax);
  xyz[1]=Rand1D(ymin,ymax);
  xyz[2]=Rand1D(zmin,zmax);
}
