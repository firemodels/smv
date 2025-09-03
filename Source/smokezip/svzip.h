#ifndef SVZIP_H_DEFINED
#define SVZIP_H_DEFINED
#include "lint.h"
//***********************
//************* #definess
//***********************
#ifdef INMAIN
#define EXTERN
#else
#define EXTERN extern
#endif
#include "csphere.h"
#include "string_util.h"
#include "file_util.h"

#include "histogram.h"
#include "threaderzip.h"

#define rgb_white 12
#define rgb_yellow 13
#define rgb_blue 14
#define rgb_red 15
#define rgb_green 16
#define rgb_magenta 17
#define rgb_cyan 18
#define rgb_black 19

#ifndef MAX
#define MAX(a,b)  ((a)>(b) ? (a) : (b))
#define MIN(a,b)  ((a)<(b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(x,lo,hi)  MIN(MAX((x),(lo)),(hi))
#endif

#ifndef GETINDEX
#define GETINDEX(xval,xmin,dx,nx) CLAMP(((xval)-(xmin))/(dx),0,(nx)-1)
#endif

#ifndef C_FILE
#define C_FILE 0
#endif
#ifndef FORTRAN_FILE
#define FORTRAN_FILE 1
#endif

#define PERCENT_SKIP 25

#define FORTSMOKEREAD(var,size, count,STREAM,option) \
                           if(option==1){FSEEK(STREAM,4,SEEK_CUR);}\
                           fread(var,size,count,STREAM);\
                           if(option==1){FSEEK(STREAM,4,SEEK_CUR);}

#define FORTSMOKEREADBR(var,size, count,STREAM,option) \
                           if(option==1){FSEEK(STREAM,4,SEEK_CUR);}\
                           returncode=fread(var,size,count,STREAM);\
                           if(returncode!=count||returncode==0)break;\
                           if(option==1){FSEEK(STREAM,4,SEEK_CUR);}


//***********************
//************* structures
//***********************


/* --------------------------  volrenderdata ------------------------------------ */

typedef struct {
  struct _meshdata *rendermesh;
  struct _slicedata *smoke, *fire;
} volrenderdata;

/* --------------------------  mesh ------------------------------------ */

typedef struct _meshdata {
  int ibar, jbar, kbar;
  float *xplt, *yplt, *zplt;
  float *xpltcell, *ypltcell, *zpltcell;
  float xbar0, xbar, ybar0, ybar, zbar0, zbar;
  float dx, dy, dz;
  float dxx, dyy, dzz;
  volrenderdata volrenderinfo;
} meshdata;

/* --------------------------  patchdata ------------------------------------ */

typedef struct {
  char *file,*filebase,*boundfile;
  int unit_start;
  char summary[1024];
  int compressed;
  int filesize;
  int is_geom;
  int inuse;
  int doit, done;
  int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2, *patchdir, *patchsize;
  int npatches;
  int setvalmin, setvalmax;
  float valmin, valmax;
  int version;
  flowlabels label;
} patchdata;

/* --------------------------  slice ------------------------------------ */

typedef struct _slicedata {
  char *file,*filebase,*boundfile;
  int isvolslice,voltype;
  int unit_start;
  int blocknumber;
  char summary[1024];
  char volsummary[1024];
  int compressed,vol_compressed;
  int inuse,involuse;
  int filesize;
  int doit, done;
  int setvalmin, setvalmax;
  float valmin, valmax;
  int setchopvalmin, setchopvalmax;
  float chopvalmin, chopvalmax;
  int version;
  flowlabels label;
} slicedata;

/* --------------------------  bound ------------------------------------ */

typedef struct {
  char label[64];
  int setvalmin, setvalmax;
  float valmin, valmax;
} bounddata;

/* --------------------------  vert ------------------------------------ */

typedef struct {
  float normal[3];
} vert;

/* --------------------------  smoke3d ------------------------------------ */

typedef struct {
  char *file,*filebase;
  int unit_start;
  int file_type;
  char summary[1024];
  int compressed;
  int inuse,is_soot;
  int nx, ny, nz, filesize;
  meshdata *smokemesh;
} smoke3d;

/* --------------------------  partpropdata ------------------------------------ */

typedef struct {
  int used;
  char isofilename[1024];
  float *partvals;
  flowlabels label;
  float valmin, valmax;
  histogramdata *histogram;
  int setvalmin, setvalmax;
} partpropdata;

/* --------------------------  partclass ------------------------------------ */

typedef struct {
  char *name;
  int ntypes;
  flowlabels *labels;
} partclassdata;

/* --------------------------  part5data ------------------------------------ */

typedef struct {
  int npoints,n_rtypes, n_itypes;
  int *tags,*sort_tags;
  float *rvals;
  unsigned char *irvals;
  unsigned char **cvals;
} part5data;

/* --------------------------  part ------------------------------------ */

typedef struct {
  char *file,*filebase;
  char summary[1024], summary2[1024];
  int unit_start;
  char **summaries;
  int nsummaries;
  int compressed,compressed2;
  int inuse,inuse_part2iso;
  int filesize;
  int setvalmin, setvalmax;
  float valmin, valmax;
  flowlabels *label;
  meshdata *partmesh;

  int nclasses;
  partclassdata **classptr;
  part5data *data5;
} part;

#define PDFMAX 100000
typedef struct {
  int ncount;
  int buckets[PDFMAX];
  float pdfmin,pdfmax;
} pdfdata;

#define BOUND(x,xmin,xmax) (((x)<(xmin))?(xmin):((x)>(xmax))?(xmax):(x))
#define GET_INTERVAL(xyz,xyz0,dxyz) (((xyz)-(xyz0))/(dxyz))


//***********************
//************* headers
//***********************

void InitSliceBounds(void);
bounddata *GetSliceBoundInfo(char *label);
void InitBoundaryBounds(void);
bounddata *GetPatchBoundInfo(char *label);
int GetFileBounds(char *file, float *valmin, float *valmax);
void PrintSummary(void);
void *CompressAll(void *arg);
void CompressAllMT(void);
unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out);
int ReadSMV(char *file);
void *CompressSlices(void *arg);
void *CompressVolSlices(void *arg);
void MakeSVD(char *destdir, char *smvfile);
void *ConvertParts2Iso(void *arg);
partpropdata *GetPartProp(char *string);
int GetPartPropIndex(char *string);
void *CompressPatches(void *arg);
void ReadINI(char *file);
void ReadINI2(char *file2);
void Convert3DSmoke(smoke3d *smoke3di, int *thread_index);
void *Compress3DSmokes(void *arg);
void GetSliceParmsC(char *file, int *ni, int *nj, int *nk);

//***********************
//************* variables
//***********************

EXTERN int nvolrenderinfo;
EXTERN int GLOBdoit_smoke3d, GLOBdoit_boundary, GLOBdoit_slice, GLOBdoit_volslice;

EXTERN FILE *SMZLOG_STREAM;

EXTERN int GLOBfirst_initsphere,GLOBfirst_slice,GLOBfirst_patch,GLOBfirst_part2iso,GLOBfirst_part2iso_smvopen;
EXTERN int GLOBframeskip;
EXTERN int GLOBno_chop;

EXTERN patchdata *patchinfo;
EXTERN bounddata *patchbounds;
EXTERN bounddata *slicebounds;
EXTERN meshdata *meshinfo;
EXTERN smoke3d *smoke3dinfo;
EXTERN slicedata *sliceinfo;
EXTERN part *partinfo;
EXTERN partclassdata *partclassinfo;
EXTERN partpropdata *part5propinfo;
EXTERN threaddata *threadinfo;
EXTERN spherepoints sphereinfo;
EXTERN char *smvzip_filename;

EXTERN int npatchinfo, nsliceinfo, npartinfo;
EXTERN int npatchbounds, nslicebounds;
EXTERN int npartclassinfo;
EXTERN int nsmoke3dinfo;
EXTERN int maxpart5propinfo, npart5propinfo;

EXTERN int nmeshes;
EXTERN int GLOBoverwrite_slice;
EXTERN int GLOBoverwrite_volslice;
EXTERN int GLOBoverwrite_b,GLOBoverwrite_s;
EXTERN int GLOBcleanfiles;
EXTERN char *GLOBdestdir,*GLOBsourcedir;
EXTERN char GLOBpp[2],GLOBx[2];
EXTERN int GLOBsmoke3dzipstep, GLOBboundzipstep, GLOBslicezipstep;
EXTERN int GLOBfilesremoved;
EXTERN int GLOBsyst;
EXTERN char *GLOBendianfile;
EXTERN int GLOBmake_demo;
EXTERN int GLOBpartfile2iso;
EXTERN char GLOBsmvisofile[1024];

#endif
