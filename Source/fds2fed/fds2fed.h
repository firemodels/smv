#ifndef FDS2FED_H_DEFINED
#define FDS2FED_H_DEFINED
#include "string_util.h"
#include "MALLOCC.h"
#include "stdio_buffer.h"
#include "file_util.h"

//*** structures VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

/* --------------------------  mesh ------------------------------------ */

typedef struct _meshdata{
  int ibar, jbar, kbar;
  float xbar0, xbar, ybar0, ybar, zbar0, zbar;
} meshdata;

/* --------------------------  slicedata ------------------------------------ */

typedef struct _slicedata{
  char *file, *filebase, *size_file;
  int blocknumber;
  int is1, is2, js1, js2, ks1, ks2;
  int slicetype;
  flowlabels label;
} slicedata;

//*** symbols VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

#define SLCF 1
#define SLCC 2
#define SLCD 3
#define SLCT 4

#ifndef ABS
#define ABS(a) ((a)>=0 ? (a) : (-(a)))
#endif

#ifndef MIN
#define MIN(a,b)  ((a)<(b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)  ((a)>(b) ? (a) : (b))
#endif

//*** headers VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

EXTERNCPP int ReadSMV(char *smvfile);

//*** global variables VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SVEXTERN int SVDECL(nsliceinfo, 0), SVDECL(nmeshes, 0);
SVEXTERN slicedata SVDECL(*sliceinfo, NULL);
SVEXTERN meshdata SVDECL(*meshinfo, NULL);
SVEXTERN char SVDECL(*GLOBsourcedir, NULL);

#endif
