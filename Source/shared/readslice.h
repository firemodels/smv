#ifndef READSLICE_H_DEFINED
#define READSLICE_H_DEFINED
#include "options.h"
#include "dmalloc.h"
#include "string_util.h"
#include "shared_structures.h"

#include <string.h>


#define SLICE_HEADER_SIZE 4
#define SLICE_TRAILER_SIZE 4

#define IJK_SLICE(i,j,k)      ( ((i)-sd->is1)*sd->nslicej*sd->nslicek + ((j)-sd->js1)*sd->nslicek + ((k)-sd->ks1) )

#define SLICEVAL(i,j,k) \
    (sd->compression_type==UNCOMPRESSED ? \
    sd->qslice[ IJK_SLICE((i), (j),  (k))] : \
    valmin + (valmax-valmin)*(float)sd->slicecomplevel[ IJK_SLICE((i), (j),  (k))]/255.0 \
    )

#define SLICETEXTURE(val) ( CLAMP((val-valmin)/(valmax-valmin),0.001,0.999) )

#define SLICECOLOR(cell_index) \
    (sd->compression_type==UNCOMPRESSED ? \
    4*CLAMP((int)(255.0*(sd->qslice[cell_index]-valmin)/(valmax-valmin)),0,255) :\
    4*CLAMP(sd->slicecomplevel[cell_index],0,255) \
    )

#define FOPEN_SLICE(a,b)         FOPEN(a,b)
#ifdef _WIN64
#define FSEEK_SLICE(a,b,c)       _fseeki64(a,b,c)
#define FTELL_SLICE(a)           _ftelli64(a)
#else
#define FSEEK_SLICE(a,b,c)       fseeko(a,b,c)
#define FTELL_SLICE(a)           ftello(a)
#endif
#define FREAD_SLICE(a,b,c,d)     fread(a,b,c,d)
#define FCLOSE_SLICE(a)          fclose(a)

#define FORT_SLICEREAD(var,count,STREAM) \
                           FSEEK_SLICE(STREAM,SLICE_HEADER_SIZE,SEEK_CUR);\
                           returncode=FREAD_SLICE(var,4,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           FSEEK_SLICE(STREAM,SLICE_TRAILER_SIZE,SEEK_CUR)

#define FORTRLESLICEREAD(var,size) FSEEK(RLESLICEFILE,4,SEEK_CUR);\
                           returncode=fread(var,4,size,RLESLICEFILE);\
                           FSEEK(RLESLICEFILE,4,SEEK_CUR)

#define GET_VAL(U,VAL,n) \
         VAL=0.0;           \
         if(U!=NULL){       \
           if(U->compression_type!=UNCOMPRESSED){\
             VAL = U->qval256[U->iqsliceframe[(n)]];\
           }                                  \
           else{                              \
             VAL = U->qslice[(n)];               \
           }                                  \
         }

#define GET_VAL_N(U,n)  ( (U)->compression_type!=UNCOMPRESSED ? (U)->qval256[(U)->iqsliceframe[(n)]] : (U)->qslice[(n)] )

#define GET_VEC_DXYZ(U,DU,n)                       \
         if(U==NULL){                              \
           DU=0.0;                                 \
         }                                         \
         else{                                     \
           if(U->compression_type==UNCOMPRESSED){  \
             DU = U->qslice[(n)];                  \
           }                                       \
           else{                                   \
             DU = U->qval256[U->iqsliceframe[(n)]];\
           }                                       \
         }

#define SCENE_FRACTION 0.05
#define SCENE_FACTOR   SCENE_FRACTION*vecfactor*vector_scalelength/(xyzmaxdiff*vel_max)

#define ADJUST_VEC_DX(dx)                       \
         if(vec_uniform_length==1){                              \
           float vecnorm; \
           vecnorm = ABS(dx);\
           if(vecnorm==0.0)vecnorm=1.0;\
           dx *= scene_factor/vecnorm;\
         }                                         \
         else{                                     \
           dx *= scene_factor;\
         }                                         \

#define ADJUST_VEC_DXYZ(dx,dy,dz)                       \
         if(vec_uniform_length==1){                              \
           float vecnorm; \
           vecnorm = sqrt(dx*dx+dy*dy+dz*dz);\
           if(vecnorm==0.0)vecnorm=1.0;\
           dx *= scene_factor/vecnorm;\
           dy *= scene_factor/vecnorm;\
           dz *= scene_factor/vecnorm;\
         }                                         \
         else{                                     \
           dx *= scene_factor;\
           dy *= scene_factor;\
           dz *= scene_factor;\
         }                                         \

#define GET_VEC_DXYZ_TERRAIN(U,DU,n)                                                 \
         if(U==NULL){                                                              \
           DU=0.0;                                                                 \
         }                                                                         \
         else{                                                                     \
           if(U->compression_type==UNCOMPRESSED){                                  \
             DU = U->qslice[n];                               \
           }                                                                       \
           else{                                                                   \
             DU = U->qval256[(int)(U->iqsliceframe[n])];\
           }                                                                       \
         }                                                                         \
         DU *= SCALE2FDS(.05*vecfactor/vel_max)

void GetSliceFileHeader(char *file, int *ip1, int *ip2, int *jp1, int *jp2, int *kp1, int *kp2, int *error);
EXTERNCPP void GetSliceSizes(const char *slicefilenameptr, int time_frame, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int tload_step_arg,
  int *errorptr, int tload_beg_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr);
EXTERNCPP FILE_SIZE GetSliceData(slicedata *sd, const char *slicefilename, int time_frame, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr,
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int tload_step_arg, int tload_beg_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg);
EXTERNCPP int  GetNSliceFrames(char *file, float *stime_min, float *stime_max);
#endif
