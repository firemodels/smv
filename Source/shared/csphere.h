#ifndef CSPHERE_H_DEFINED
#define CSPHERE_H_DEFINED

typedef struct {
  int n;
  float rad;
  float dphi;
  unsigned int npoints;
  float *dtheta;
  int *nlong;
  unsigned int *vallist;
  float *normals;
  short *snormals;
  float pi;
  float maxerr_deg;
} spherepoints;

void InitSpherePoints(spherepoints *sphereinfo, int n);
void FreeSpherePoints(spherepoints *sphereinfo);
unsigned int GetNormalIndex(spherepoints *sphereinfo, float *normal);
void GetNormalVector(spherepoints *sphereinfo, unsigned int index, float *normal);
float *GetNormalVectorPtr(spherepoints *sphereinfo, unsigned int index);

#endif
