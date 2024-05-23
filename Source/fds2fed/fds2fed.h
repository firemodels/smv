#ifndef FDS2FED_H_DEFINED
#define FDS2FED_H_DEFINED
#include "string_util.h"
#include "MALLOCC.h"
#include "stdio_buffer.h"
#include "file_util.h"

//*** structures VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

/* --------------------------  slicedata ------------------------------------ */

typedef struct _slicedata{
  char *file, *filebase;
  int blocknumber, slicetype, quant, in_fed;
  int is1, is2, js1, js2, ks1, ks2;
  flowlabels label;
} slicedata;


/* --------------------------  feddata ------------------------------------ */

typedef struct _feddata{
  slicedata *o2, *co2, *co;
} feddata;

//*** symbols VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

#define SLCF 1
#define SLCC 2
#define SLCD 3
#define SLCT 4

#define OTHER -1
#define O2     0
#define CO     1
#define CO2    2

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
EXTERNCPP void MakeFED(void);
EXTERNCPP void PrintFED(void);

//*** global variables VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SVEXTERN int SVDECL(nfedinfo, 0), SVDECL(nsliceinfo, 0);
SVEXTERN feddata SVDECL(*fedinfo, NULL);
SVEXTERN slicedata SVDECL(*sliceinfo, NULL);
#endif
