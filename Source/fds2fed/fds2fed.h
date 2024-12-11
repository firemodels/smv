#ifndef FDS2FED_H_DEFINED
#define FDS2FED_H_DEFINED
#include "string_util.h"
#include "dmalloc.h"
#include "stdio_buffer.h"
#include "file_util.h"

//*** structures VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV


/* --------------------------  meshdata ------------------------------------ */

typedef struct _meshdata{
  int ibar, jbar, kbar;
  float *xplt, *yplt, *zplt;
} meshdata;

/* --------------------------  slicedata ------------------------------------ */

typedef struct _slicedata{
  char *file, *filebase, keyword_label[256];
  int blocknumber, slicetype, quant, in_fed;
  int is1, is2, js1, js2, ks1, ks2;
  int vol;
  float *vals, *times;
  int headersize, diskframesize, memframesize, nframes;
  flowlabels label;
} slicedata;

/* --------------------------  feddata ------------------------------------ */

typedef struct _feddata{
  char sf_file[1024], iso_file[1024], bndfile[1024], *keyword_label;
  slicedata *o2, *co2, *co, *fed;
  float *times, *vals;
  int nframes, memframesize;
} feddata;

//*** symbols VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

#ifndef CLAMP
#define CLAMP(x,lo,hi)  MIN(MAX((x),(lo)),(hi))
#endif

#define FEDCO(CO)  ( (2.764/100000.0)*pow(1000000.0*CLAMP(CO,0.0,0.1),1.036)/60.0 )
#define FEDO2(O2)  ( exp( -(8.13-0.54*(20.9-100.0*CLAMP(O2,0.0,0.2))) )/60.0 )
#define HVCO2(CO2) ( exp(0.1930*CLAMP(CO2,0.0,0.1)*100.0+2.0004)/7.1 )

#define FORTREAD(var,count,STREAM) FSEEK(STREAM,4,SEEK_CUR);fread(var,4,count,STREAM);FSEEK(STREAM,4,SEEK_CUR)

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
EXTERNCPP void MakeFEDSmv(char *file);
EXTERNCPP void MakeFEDSlices(void);

//*** global variables VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

SVEXTERN int SVDECL(nfedinfo, 0), SVDECL(nsliceinfo, 0), SVDECL(nmeshinfo, 0);
SVEXTERN int SVDECL(isocount, 0), SVDECL(nfedisos, 0);
SVEXTERN feddata SVDECL(*fedinfo, NULL);
SVEXTERN slicedata SVDECL(*sliceinfo, NULL);
SVEXTERN meshdata SVDECL(*meshinfo, NULL);
#endif
