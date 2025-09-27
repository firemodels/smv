#include "options.h"
#include "dmalloc.h"
#include "string_util.h"
#include <math.h>

#include <string.h>
#include "smokeviewdefs.h"
#include "scontour2d.h"
#include "isobox.h"
#include "histogram.h"
#include "structures.h"
#include "datadefs.h"
#include "readslice.h"
#include "getdata.h"

/* ------------------ GetSliceFileHeader ------------------------ */

void GetSliceFileHeader(char *file, int *ip1, int *ip2, int *jp1, int *jp2, int *kp1, int *kp2, int *error){
  FILE *stream = NULL;
  int vals[6];

  stream = FOPEN(file, "rb");
  *error = 1;
  *ip1 = 0;
  *ip2 = 0;
  *jp1 = 0;
  *jp2 = 0;
  *kp1 = 0;
  *kp2 = 0;
  if(stream==NULL)return;
  fseek(stream, 3*(4+30+4), SEEK_CUR);
  fseek(stream, 4, SEEK_CUR); fread(vals, sizeof(int), 6, stream);
  *ip1 = vals[0];
  *ip2 = vals[1];
  *jp1 = vals[2];
  *jp2 = vals[3];
  *kp1 = vals[4];
  *kp2 = vals[5];
  *error = 0;
  fclose(stream);
}

/* ------------------ GetSliceSizes ------------------------ */

void GetSliceSizes(const char *slicefilenameptr, int time_frame, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int tload_step_arg,
  int *errorptr, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr){

  int ip1, ip2, jp1, jp2, kp1, kp2;
  int iip1, iip2;
  int nxsp, nysp, nzsp;

  float timeval, time_max;
  int idir, joff, koff, volslice;
  int count,countskip;
  FILEBUFFER *SLICEFILE=NULL;
  int ijk[6];
  int returncode=0;

  *errorptr = 0;
  *ntimesptr = 0;

  if(SLICEFILE==NULL){
    SLICEFILE = FOPEN_SLICE(slicefilenameptr, "rb");
  }
  if(SLICEFILE==NULL){
    *errorptr = 1;
    return;
  }

  *headersizeptr = 3*(4+30+4);
  FSEEK_SLICE(SLICEFILE, *headersizeptr, SEEK_CUR);

  FORT_SLICEREAD(ijk, 6, SLICEFILE);
  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  *headersizeptr += 4+6*4+4;

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  GetSliceFileDirection(ip1, &ip2, &iip1, &iip2, jp1, &jp2, kp1, &kp2, &idir, &joff, &koff, &volslice);
  *nsliceiptr = nxsp;
  *nslicejptr = nysp + joff;
  *nslicekptr = nzsp + koff;

  *framesizeptr = 4*(1+nxsp*nysp*nzsp)+16;

  count = -1;
  countskip = -1;
  time_max = -1000000.0;
  for(;;){
    int loadframe;

    loadframe = 0;
    FORT_SLICEREAD(&timeval, 1, SLICEFILE);
    if(returncode==0)break;
    if((settmin_s_arg!=0&&timeval<tmin_s_arg)||timeval<=time_max){
    }
    else{
      if(time_frame==ALL_FRAMES)loadframe = 1;
      time_max = timeval;
    }
    if(settmax_s_arg!=0&&timeval>tmax_s_arg){
      FCLOSE_SLICE(SLICEFILE);
      return;
    }
    FSEEK_SLICE(SLICEFILE, *framesizeptr-12, SEEK_CUR);
    if(count%tload_step_arg==0){
      countskip++;
      if(time_frame>=0&&time_frame==countskip){
        *ntimesptr = *ntimesptr+1;
        FCLOSE_SLICE(SLICEFILE);
        *errorptr = 0;
        return;
      }
    }
    else{
      loadframe = 0;
    }
    if(loadframe==1)*ntimesptr = *ntimesptr+1;
    count++;
  }
  *errorptr = 0;
  FCLOSE_SLICE(SLICEFILE);
}

/* ------------------ GetSliceData ------------------------ */

FILE_SIZE GetSliceData(slicedata *sd, const char *slicefilename, int time_frame, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr,
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int tload_step_arg, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg
){

  int i, j, k;
  int nsteps;
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  int istart, irowstart;
  float timeval;
  int loadframe;
  int ii, kk;
  int joff, koff, volslice;
  int count;
  int iis1, iis2;
  int ijk[6];
  FILE_SIZE file_size;
  FILEBUFFER *stream=NULL;
  int returncode=0;
  float *qq;
  int nx, ny, nxy;
  int count_timeframe;

  joff = 0;
  koff = 0;
  file_size = 0;

  if(stream==NULL){
    stream = FOPEN_SLICE(slicefilename,"rb");
  }
  if(stream==NULL){
    printf(" the slice file %s does not exist\n", slicefilename);
    return 0;
  }

  nsteps = 0;
  FSEEK_SLICE(stream, 3*(4+30+4), SEEK_CUR);

  FORT_SLICEREAD(ijk, 6, stream);
  file_size += (FILE_SIZE)(4 + 6*4 + 4);
  if(returncode==0){
    FCLOSE_SLICE(stream);
    return file_size;
  }

 // 1D not X dir

  if(ijk[0] == ijk[1] && sd->idir != 1){
    ijk[0]--;
    ijk[1]--;
  }

 // 1D not Y dir

  if(ijk[2] == ijk[3] && sd->idir != 2){
    ijk[2]--;
    ijk[3]--;
  }

 // 1D not Z dir

  if(ijk[4] == ijk[5] && sd->idir != 3){
    ijk[4]--;
    ijk[5]--;
  }
  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  *is1ptr = ip1;
  *is2ptr = ip2;
  *js1ptr = jp1;
  *js2ptr = jp2;
  *ks1ptr = kp1;
  *ks2ptr = kp2;

  nxsp = *is2ptr+1-*is1ptr;
  nysp = *js2ptr+1-*js1ptr;
  nzsp = *ks2ptr+1-*ks1ptr;

  nx = nxsp;
  ny = nysp;
  nxy = nx*ny;

  GetSliceFileDirection(*is1ptr, is2ptr, &iis1, &iis2, *js1ptr, js2ptr, *ks1ptr, ks2ptr, idirptr, &joff, &koff, &volslice);
  sd->iis1 = *is1ptr;
  NewMemory((void **)&qq, nxsp*(nysp+joff)*(nzsp+koff)*sizeof(float));

  count = -1;

  if(time_frame>0){
    int size;

    size = 4+4+4;                     // time
    size += (4+4*nxsp*nysp*nzsp+4);   // slice data
    size *= time_frame;               // number of steps to skip over
    FSEEK_SLICE(stream, size, SEEK_CUR);
  }
  else{
    if(*ntimesptr!=ntimes_old_arg&&ntimes_old_arg>0){
      int size;

      size = 0;
      for(i = 0; i<ntimes_old_arg; i++){
        size += 4+4+4;
        size += 4+4*nxsp*nysp*nzsp+4;
      }
      FSEEK_SLICE(stream, size, SEEK_CUR);
      nsteps = ntimes_old_arg;
    }
  }
  count_timeframe = 0;
  for(;;){
    int skipmin;

    if(time_frame>=0&&count_timeframe==1){
      break;
    }
    FORT_SLICEREAD(&timeval, 1, stream);
    file_size += (FILE_SIZE)(4 + 4 + 4);
    if(returncode==0)break;
    if((settmin_s_arg!=0&&timeval<tmin_s_arg)){
      loadframe = 0;
    }
    else{
      loadframe = 1;
    }
    if(settmax_s_arg!=0&&timeval>tmax_s_arg)break;
    //    read(lu11, iostat = error)(((qq(i, j, k), i = 1, nxsp), j = 1, nysp), k = 1, nzsp)
    FORT_SLICEREAD(qq, nxsp*nysp*nzsp, stream);
    file_size += (FILE_SIZE)(4 + 4*nxsp*nysp*nzsp + 4);
    if(returncode==0||nsteps>=*ntimesptr)break;
    count++;
    if(count%tload_step_arg!=0)loadframe = 0;
    if(loadframe==0)continue;
    if(koff==1){
//  qq(1:nxsp, 1:nysp, 2) = qq(1:nxsp, 1:nysp, 1)
      for(j = 0;j<nysp;j++){
        float *qqto, *qqfrom;

        qqfrom = qq + IJKNODE(0,j,0);
        qqto = qq + IJKNODE(0,j,1);
        for(i = 0;i<nxsp;i++){
//        qq[IJKNODE(i, j, 1)] = qq[IJKNODE(i, j, 0)];
          skipmin = 0;
          if(sd->slice_filetype == SLICE_CELL_CENTER){
            if(i == 0 || j == 0)skipmin = 1;
          }
          if(skipmin==0){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
          *qqto++ = *qqfrom++;
        }
      }
    }
    else if(joff==1){
//  qq(1:nxsp, 2, 1:nzsp) = qq(1:nxsp, 1, 1:nzsp)
      for(k = 0;k<nzsp;k++){
        float *qqto, *qqfrom;

        qqfrom = qq + IJKNODE(0,0,k);
        qqto = qq + IJKNODE(0,1,k);
        for(i = 0;i<nxsp;i++){
//        qq[IJKNODE(i, 1, k)] = qq[IJKNODE(i, 0, k)];
          skipmin = 0;
          if(sd->slice_filetype == SLICE_CELL_CENTER){
            if(i == 0 || k == 0)skipmin = 1;
          }
          if(skipmin == 0){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
          *qqto++ = *qqfrom++;
        }
      }
    }
    timesptr[nsteps] = timeval;
    nsteps = nsteps+1;

    if(*idirptr==3){
      float *qqto, *qqfrom;

      istart = (nsteps-1)*nxsp*nysp;
      for(i = 0; i<nxsp; i++){
        irowstart = i*nysp;
        ii = istart+irowstart;
        qqto = qdataptr+ii;
        qqfrom = qq + IJKNODE(i, 0, 0);
//      qdata(ii+1:ii+nysp) = qq(i, 1:nysp, 1)
        for(j = 0;j<nysp;j++){
//        qdataptr[ii+j] = qq[IJKNODE(i, j, 0)];
          skipmin = 0;
          if(sd->slice_filetype == SLICE_CELL_CENTER){
            if(i == 0 || j == 0)skipmin = 1;
          }
          if(skipmin==0){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
          *qqto++ = *qqfrom;
          qqfrom += nx;
        }
      }
    }
    else if(*idirptr==2){
      float *qqto, *qqfrom;

      istart = (nsteps-1)*nxsp*(nzsp+koff);
      for(i = 0; i<nxsp; i++){
        irowstart = i*(nzsp+koff);
        kk = istart+irowstart;
//      qdata(kk+1:kk+nzsp+koff) = qq(i, 1, 1:nzsp+koff)
        qqto = qdataptr+kk;
        qqfrom = qq + IJKNODE(i, 0, 0);
        for(k = 0;k<nzsp+koff;k++){
//        qdataptr[kk+k] = qq[IJKNODE(i, 0, k)];
          skipmin = 0;
          if(sd->slice_filetype == SLICE_CELL_CENTER){
            if(i == 0 || k == 0)skipmin = 1;
          }
          if(skipmin==0){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
          *qqto++ = *qqfrom;
          qqfrom += nxy;
        }
      }
    }
    else{
      float *qqto, *qqfrom;

      istart = (nsteps-1)*(nysp+joff)*(nzsp+koff)*nxsp;
      for(i = 0; i<nxsp; i++){
        for(j = 0; j<nysp+joff; j++){
          irowstart = i*nysp*(nzsp+koff)+j*(nzsp+koff);
          kk = istart+irowstart;
//        qdata(kk+1:kk+nzsp+koff) = qq(i, j, 1:nzsp+koff)
          qqto = qdataptr+kk;
          qqfrom = qq + IJKNODE(i, j, 0);
          for(k = 0;k<nzsp+koff;k++){
//          qdataptr[kk+k] = qq[IJKNODE(i, j, k)];
            skipmin = 0;
            if(sd->slice_filetype == SLICE_CELL_CENTER){
              if(j == 0 || k == 0 || (nxsp > 1 && i == 0))skipmin = 1;
            }
            if(skipmin==0){
              *qminptr = MIN(*qminptr, *qqfrom);
              *qmaxptr = MAX(*qmaxptr, *qqfrom);
            }
            *qqto++ = *qqfrom;
            qqfrom += nxy;
          }
        }
      }
    }
  }
  *ks2ptr += koff;
  *js2ptr += joff;
  *ntimesptr = nsteps;

  FCLOSE_SLICE(stream);
  FREEMEMORY(qq);
  return file_size;
}

/* ------------------ GetNSliceFrames ------------------------ */

/// @brief Count the number of frames in a slice file based on the file size.
/// If stime_min is greater than stime_max (i.e., an invalid value) this will
/// read the time values of the first and last frames and update the values of
/// stime_min and stime_max accordingly.
/// @param file The path to the slice file.
/// @param[inout] stime_min The minimum time value.
/// @param[inout] stime_max The maximum time value.
/// @return The number of frames in the file or < 0 on error.
int GetNSliceFrames(char *file, float *stime_min, float *stime_max){
  int is1, is2, js1, js2, ks1, ks2, error;

  int header_size = 3*(4+30+4)+4+6*4+4;
  FILE_SIZE file_size = GetFileSizeSMV(file);

  GetSliceFileHeader(file, &is1, &is2, &js1, &js2, &ks1, &ks2, &error);
  int frame_size = 12;
  frame_size += 8+(is2+1-is1)*(js2+1-js1)*(ks2+1-ks1)*4;
  int nframes = (file_size-header_size)/frame_size;

  if(*stime_min > *stime_max&&nframes>0){
    fprintf(stderr, "reading times\n" );
    FILE *stream = FOPEN(file, "rb");
    if(stream!=NULL){
      fseek(stream, header_size, SEEK_SET);
      fseek(stream, 4, SEEK_CUR); fread(stime_min, sizeof(float), 1, stream); fseek(stream, 4, SEEK_CUR);
      fprintf(stderr,"time_min: %f s\n",*stime_min);
      *stime_max = *stime_min;
      if(nframes>1){
        fseek(stream, header_size + (nframes-1)*frame_size, SEEK_SET);
        fseek(stream, 4, SEEK_CUR); fread(stime_max, sizeof(float), 1, stream); fseek(stream, 4, SEEK_CUR);
      }
      fprintf(stderr,"time_max: %f s\n",*stime_max);
    }
    if(stream!=NULL)fclose(stream);
  }
  return nframes;
}
