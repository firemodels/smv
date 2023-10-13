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
#include "threader.h"

#ifdef pp_PART
#define rgb_white 12
#define rgb_yellow 13
#define rgb_blue 14
#define rgb_red 15
#define rgb_green 16
#define rgb_magenta 17
#define rgb_cyan 18
#define rgb_black 19
#endif

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


#ifdef X64
#ifndef STRUCTSTAT
#define STRUCTSTAT struct __stat64
#endif
#ifndef STAT
#define STAT _stat64
#endif
#else
#ifndef STRUCTSTAT
#define STRUCTSTAT struct stat
#endif
#ifndef STAT
#define STAT stat
#endif
#endif

#ifdef pp_SMOKE3D_FORT
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
#else
#define FORTSMOKEREAD(var,size, count,STREAM,option) \
                           fread(var,size,count,STREAM)

#define FORTSMOKEREADBR(var,size, count,STREAM,option) \
                           returncode=fread(var,size,count,STREAM);\
                           if(returncode!=count||returncode==0){
                             break;\
                           }
#endif


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

/* --------------------------  patch ------------------------------------ */

typedef struct {
  char *file,*filebase;
  int unit_start;
  char summary[1024];
  int compressed;
  int filesize;
  int is_geom;
  int inuse,inuse_getbounds;
  int doit, done;
  int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2, *patchdir, *patchsize;
  int npatches;
  int setvalmin, setvalmax;
  float valmin, valmax;
  int version;
  histogramdata *histogram;
  flowlabels label;
  int dup;
} patch;

/* --------------------------  slice ------------------------------------ */

typedef struct _slicedata {
  char *file,*filebase,*boundfile;
  int isvolslice,voltype;
  int unit_start;
  int blocknumber;
  char summary[1024];
  char volsummary[1024];
  int compressed,vol_compressed;
  int inuse,involuse,inuse_getbounds;
  int filesize;
  int doit, done, count;
  int setvalmin, setvalmax;
  float valmin, valmax;
  int setchopvalmin, setchopvalmax;
  float chopvalmin, chopvalmax;
  int version;
  histogramdata *histogram;
  flowlabels label;
  int dup;
} slicedata;

/* --------------------------  bound ------------------------------------ */

typedef struct {
  int setvalmin, setvalmax;
  float valmin, valmax;
} bounddata;

/* --------------------------  plot3d ------------------------------------ */

typedef struct {
  char *file,*filebase;
  int unit_start;
  char summary[1024];
  int compressed;
  int inuse;
  float time;
  int blocknumber;
  meshdata *plot3d_mesh;
  int filesize;
  int doit, done, count;
  bounddata bounds[5];
  int version;
  flowlabels labels[5];
  int dup;
} plot3d;

/* --------------------------  vert ------------------------------------ */

typedef struct {
  float normal[3];
} vert;

/* --------------------------  smoke3d ------------------------------------ */

typedef struct {
  char *file,*filebase;
  int unit_start;
#ifdef pp_SMOKE3D_FORT
  int file_type;
#endif
  char summary[1024];
  int compressed;
  int inuse,is_soot;
  int nx, ny, nz, filesize;
  meshdata *smokemesh;
} smoke3d;

#ifdef pp_PART

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
#endif

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

void InitVolRender(void);
void PrintSummary(void);
void *CompressAll(void *arg);
void CompressAllMT(void);
void RandABsdir(float xyz[3], int dir);
float Rand1D(float xmin, float xmax);
void Rand2D(float xy[2], float xmin, float xmax, float ymin, float ymax);
void Rand3D(float xyz[3], float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out);
int ReadSMV(char *file);
slicedata *GetSlice(char *string);
void *CompressSlices(void *arg);
void *CompressVolSlices(void *arg);
int IsPlot3DDup(plot3d *plot3dj, int iplot3d);
int SliceDup(slicedata *slicej, int islice);
void *CompressPlot3Ds(void *arg);
void MakeSVD(char *destdir, char *smvfile);
#ifdef pp_PART
void CompressParts(void *arg);
void *ConvertParts2Iso(void *arg);
partpropdata *GetPartProp(char *string);
int GetPartPropIndex(char *string);
void ConvertPart(part *parti, int *thread_index);
int ConvertablePart(part *parti);
#endif
void *CompressPatches(void *arg);
patch *GetPatch(char *string);
int PatchDup(patch *patchj, int ipatch);
void ReadINI(char *file);
void ReadINI2(char *file2);
#ifdef pp_PART
void Get_Part_Bounds(void);
#endif
void Convert3DSmoke(smoke3d *smoke3di, int *thread_index);
void *Compress3DSmokes(void *arg);
void Normal(unsigned short *v1, unsigned short *v2, unsigned short *v3, float *normal, float *area);
void GetSliceParmsC(char *file, int *ni, int *nj, int *nk);

//***********************
//************* variables
//***********************

EXTERN int nvolrenderinfo;
EXTERN int GLOBdoit_smoke3d, GLOBdoit_boundary, GLOBdoit_slice, GLOBdoit_plot3d, GLOBdoit_volslice;
#ifdef pp_PART2
EXTERN int GLOBdoit_particle;
#endif

EXTERN FILE *SMZLOG_STREAM;

EXTERN int GLOBfirst_initsphere,GLOBfirst_slice,GLOBfirst_patch,GLOBfirst_plot3d,GLOBfirst_part2iso,GLOBfirst_part2iso_smvopen;
EXTERN int GLOBframeskip;
EXTERN int GLOBno_chop;

EXTERN patch *patchinfo;
EXTERN meshdata *meshinfo;
EXTERN smoke3d *smoke3dinfo;
EXTERN slicedata *sliceinfo;
EXTERN plot3d *plot3dinfo;
EXTERN part *partinfo;
EXTERN partclassdata *partclassinfo;
EXTERN partpropdata *part5propinfo;
EXTERN threaddata *threadinfo;
EXTERN spherepoints sphereinfo;

EXTERN int npatchinfo, nsliceinfo, nplot3dinfo, npartinfo;
EXTERN int npartclassinfo;
EXTERN int nsmoke3dinfo;
#ifdef pp_PART
EXTERN int maxpart5propinfo, npart5propinfo;
#endif


EXTERN int nmeshes;
EXTERN int GLOBoverwrite_slice;
EXTERN int GLOBoverwrite_volslice;
EXTERN int GLOBoverwrite_plot3d;
#ifdef pp_PART
EXTERN int GLOBoverwrite_part;
#endif
EXTERN int GLOBoverwrite_b,GLOBoverwrite_s;
EXTERN int GLOBcleanfiles;
EXTERN char *GLOBdestdir,*GLOBsourcedir;
EXTERN char GLOBpp[2],GLOBx[2];
EXTERN int GLOBsmoke3dzipstep, GLOBboundzipstep, GLOBslicezipstep;
EXTERN int GLOBfilesremoved;
EXTERN int GLOBsyst;
EXTERN char *GLOBendianfile;
EXTERN int GLOBmake_demo;
EXTERN int GLOBget_bounds, GLOBget_slice_bounds, GLOBget_plot3d_bounds, GLOBget_boundary_bounds;
#ifdef pp_PART
EXTERN int GLOBget_part_bounds;
EXTERN int GLOBpartfile2iso;
#endif
EXTERN char GLOBsmvisofile[1024];

#endif
