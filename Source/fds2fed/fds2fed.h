#ifndef FDS2FED_H_DEFINED
#define FDS2FED_H_DEFINED
#include "string_util.h"
#include "MALLOCC.h"
#include "stdio_buffer.h"
#include "file_util.h"

/* --------------------------  mesh ------------------------------------ */

typedef struct _meshdata{
  int ibar, jbar, kbar;
  float xbar0, xbar, ybar0, ybar, zbar0, zbar;
} meshdata;

/* --------------------------  slicedata ------------------------------------ */

typedef struct _slicedata{
  char *file, *filebase, *size_file;
  int blocknumber;
  flowlabels label;
} slicedata;

SVEXTERN int SVDECL(nsliceinfo, 0), SVDECL(nmeshes, 0);
SVEXTERN slicedata SVDECL(*sliceinfo, NULL);
SVEXTERN meshdata SVDECL(*meshinfo, NULL);
SVEXTERN char SVDECL(*GLOBsourcedir, NULL);

#endif
