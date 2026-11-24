#ifndef PART2ISO_H_DEFINED
#define PART2ISO_H_DEFINED
#include "lint.h"
#include "string_util.h"
#include "file_util.h"
#ifdef INMAIN
#define EXTERN
#else
#define EXTERN extern
#endif
#ifndef MAX
 #define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef CLAMP
#define CLAMP(x,lo,hi) MIN(MAX((x),(lo)),(hi))
#endif
#ifndef GETINDEX
#define GETINDEX(xval,xmin,dx,nx) CLAMP(((xval)-(xmin))/(dx),0,(nx)-1)
#endif
#define PERCENT_SKIP 25
typedef struct {
  int ibar, jbar, kbar;
  float *xpltcell, *ypltcell, *zpltcell;
  float xbar0, xbar, ybar0, ybar, zbar0, zbar;
  float dx, dy, dz;
} meshdata;
typedef struct {
  int used;
  char isofilename[1024];
  float *partvals;
  flowlabels label;
} partpropdata;
typedef struct {
  char *name;
  int ntypes;
  flowlabels *labels;
} partclassdata;
typedef struct {
  char *file;
  FILE_SIZE filesize;
  meshdata *partmesh;
  int nclasses;
  partclassdata **classptr;
} part;
int ReadSMV(char *file);
void Part2Iso(part *parti);
partpropdata *GetPartProp(char *string);
int GetPartPropIndex(char *string);
EXTERN meshdata *meshinfo;
EXTERN part *partinfo;
EXTERN partclassdata *partclassinfo;
EXTERN partpropdata *part5propinfo;
EXTERN int npartinfo, npartclassinfo, maxpart5propinfo, npart5propinfo, nmeshes;
EXTERN int GLOBfirst_part2iso_smvopen;
EXTERN char *GLOBsourcedir;
EXTERN char GLOBpp[2], GLOBsmvisofile[1024];
#endif