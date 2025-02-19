#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svdiff.h"
#include "dmalloc.h"
#include "datadefs.h"
#include "file_util.h"
#include "getdata.h"

/* ------------------ SetupSlice ------------------------ */

void SetupSlice(FILE *stream_out){
  casedata *case1, *case2;
  int i;

  case1 = caseinfo;
  case2 = caseinfo + 1;

  for(i=0;i<case1->nsliceinfo;i++){
    slice *slicei;

    slicei = case1->sliceinfo + i;
    slicei->slice2 = GetSlice(slicei,case2);
    if(slicei->slice2!=NULL&&stream_out!=NULL){
      char outfile[1024];

      fprintf(stream_out,"%s\n",slicei->keyword);
      MakeOutFile(outfile,NULL,slicei->file,".sf");
      fprintf(stream_out," %s\n",outfile);
      fprintf(stream_out," %s\n",slicei->label.longlabel);
      fprintf(stream_out," %s\n",slicei->label.shortlabel);
      fprintf(stream_out," %s\n",slicei->label.unit);
    }
  }
}

/* ------------------ GetSlice ------------------------ */

slice *GetSlice(slice *slicein, casedata *case2){
  int i;
  float dx, dy, dz;

  dx = slicein->slicemesh->dx/2.0;
  dy = slicein->slicemesh->dy/2.0;
  dz = slicein->slicemesh->dz/2.0;
  if(strlen(type_label)>0&&strcmp(type_label,slicein->label.shortlabel)!=0){
    return NULL;
  }
  for(i=0;i<case2->nsliceinfo;i++){
    slice *sliceout;

    sliceout = case2->sliceinfo + i;
    if(slicein->slicetype!=sliceout->slicetype)continue;
    if(strcmp(slicein->label.longlabel,sliceout->label.longlabel)!=0)continue;
    if(ABS(slicein->xmin-sliceout->xmin)>dx)continue;
    if(ABS(slicein->xmax-sliceout->xmax)>dx)continue;
    if(ABS(slicein->ymin-sliceout->ymin)>dy)continue;
    if(ABS(slicein->ymax-sliceout->ymax)>dy)continue;
    if(ABS(slicein->zmin-sliceout->zmin)>dz)continue;
    if(ABS(slicein->zmax-sliceout->zmax)>dz)continue;
    if(SimilarGrid(slicein->slicemesh,sliceout->slicemesh,slicein->factor)==0)continue;
    return sliceout;
  }
  return NULL;
}

/* ------------------ DiffSlices ------------------------ */

void DiffSlices(FILE *stream_out){
  int j;

  for(j=0;j<caseinfo->nsliceinfo;j++){
    float valmin, valmax;
    char *file1, *file2;
    char fullfile1[1024], fullfile2[1024], outfile[1024], outfile_bnd[1024], outfile2[1024];
    slice *slicei, *slice1;//, *slice2;
    FILE *stream;
    FILE *unit1, *unit2, *unit3;
    int is1a, is2a, js1a, js2a, ks1a, ks2a;
    int is1b, is2b, js1b, js2b, ks1b, ks2b;
    int error1=0,error2a=0,error2b=0;
    float time1, *qframe1;
    int nqframe1;
    float time2a, *qframe2a;
    float time2b, *qframe2b;
    int nqframe2;
    float *qframeout;
    int i;
    float f1, f2, dt;
    int slicetest1,slicetest2;
    float fraction_complete;
    FILE_SIZE size_sofar;
    int percent_complete;
    float valmin_percentile, valmax_percentile;
    int nx1, ny1, nz1;
    int nx2, ny2, nz2;

    slicei = caseinfo->sliceinfo+j;
    slice1 = slicei;
    if(slicei->slice2==NULL)continue;
    file1 = slicei->file;
    file2 = slicei->slice2->file;
    FullFile(fullfile1,sourcedir1,file1);
    FullFile(fullfile2,sourcedir2,file2);

    stream=fopen(fullfile1,"r");
    if(stream==NULL)continue;
    fclose(stream);

    stream=fopen(fullfile2,"r");
    if(stream==NULL)continue;
    fclose(stream);

    MakeOutFile(outfile,destdir,file1,".sf");
    if(strlen(outfile)==0)continue;
    strcpy(outfile_bnd, outfile);
    strcat(outfile_bnd, ".bnd");

    stream=fopen(outfile,"w");
    if(stream==NULL)continue;
    fclose(stream);

    MakeOutFile(outfile2,NULL,slice1->file,".sf");

    slicetest1=0;
    slicetest2=0;
    if(test_mode==1){
      slicetest1=1;
      slicetest2=2;
    }
    openslice(fullfile1,&unit1,&is1a,&is2a,&js1a,&js2a,&ks1a,&ks2a,&error1);
    openslice(fullfile2,&unit2,&is1b,&is2b,&js1b,&js2b,&ks1b,&ks2b,&error2a);
    if(error1!=0||error2a!=0){
      closefortranfile(unit1);
      closefortranfile(unit2);
      if(error1!=0||error2a!=0){
        if(error1==0)fprintf(stderr,"*** problem opening %s\n",fullfile1);
        if(error2a==0)fprintf(stderr,"*** problem opening %s\n",fullfile2);
      }
      continue;
    }

    nx1 = is2a + 1 - is1a;
    ny1 = js2a + 1 - js1a;
    nz1 = ks2a + 1 - ks1a;
    nqframe1 = nx1*ny1*nz1;
    NewMemory((void **)&qframe1,nqframe1*sizeof(float));
    NewMemory((void **)&qframeout,nqframe1*sizeof(float));

    nx2 = is2b + 1 - is1b;
    ny2 = js2b + 1 - js1b;
    nz2 = ks2b + 1 - ks1b;
    nqframe2 = nx2*ny2*nz2;
    NewMemory((void **)&qframe2a,nqframe2*sizeof(float));
    NewMemory((void **)&qframe2b,nqframe2*sizeof(float));

    outsliceheader(outfile,&unit3,is1a,is2a,js1a,js2a,ks1a,ks2a,&error1);
    if(error1!=0){
      closefortranfile(unit1);
      closefortranfile(unit2);
      fprintf(stderr,"*** problem writing out header for %s\n",fullfile1);
      continue;
    }
    PRINTF("Subtracting %s from %s\n",fullfile2,fullfile1);
    error1=1;
    error2a=1;
    error2b=1;
    ResetHistogram(slice1->histogram,NULL,NULL);
    getsliceframe(unit1,is1a,is2a,js1a,js2a,ks1a,ks2a,&time1,qframe1,slicetest1,&error1);
    if(error1==0 )getsliceframe(unit2,is1b,is2b,js1b,js2b,ks1b,ks2b,&time2a,qframe2a,slicetest2,&error2a);
    if(error2a==0)getsliceframe(unit2,is1b,is2b,js1b,js2b,ks1b,ks2b,&time2b,qframe2b,slicetest2,&error2b);
    if(error1!=0||error2a!=0||error2b!=0){
      closefortranfile(unit1);
      closefortranfile(unit2);
      closefortranfile(unit3);
      FREEMEMORY(qframe1);
      FREEMEMORY(qframe2a);
      FREEMEMORY(qframe2b);
      FREEMEMORY(qframeout);
      continue;
    }
    UpdateHistogram(qframe1, NULL,nqframe1, slice1->histogram);
    PRINTF("  Progress: ");
    FFLUSH();

    percent_complete=0;
    size_sofar=0;
    valmin = 1000000000.0;
    valmax = -valmin;
    for(;;){

      size_sofar+=nqframe1*sizeof(float);
      fraction_complete=(float)size_sofar/(float)slice1->filesize;
      if((int)(fraction_complete*100)>percent_complete+10){
        if(percent_complete<100)percent_complete+=10;
        PRINTF("%i%s ",percent_complete,pp);
        FFLUSH();
      }
      while(time1>time2b){
        for(i=0;i<nqframe2;i++){
          qframe2a[i]=qframe2b[i];
        }
        time2a=time2b;
        getsliceframe(unit2,is1b,is2b,js1b,js2b,ks1b,ks2b,&time2b,qframe2b,slicetest2,&error2a);
        if(error2a!=0)break;
      }
      if(error2a!=0)break;
      dt = time2b - time2a;
      f1 = 1.0;
      f2 = 0.0;
      if(dt!=0.0){
        f1 = (time2b - time1)/dt;
        f2 = (time1-time2a)/dt;
      }
      // ijk1 = k*nx1*ny1 + j*nx1 + i
      // k = ijk1/(nx1*ny1)
      // j = (ijk1 - k*nx1*ny1)/nx1
      // i = ijk1 - k*nx1*ny1 - j*nx1

      // ijk2 = k*nx2*ny2 + j*nx2 + i

      for(i=0;i<nqframe1;i++){
        int i1, jj1, k1;
        int i2, j2, k2;
        int ijk2;

        k1 = i/(nx1*ny1);
        jj1 = (i-k1*nx1*ny1)/nx1;
        i1 = i-k1*nx1*ny1-jj1*nx1;

        k2=slice1->factor[2]*k1;
        j2=slice1->factor[1]*jj1;
        i2=slice1->factor[0]*i1;
        ijk2=k2*nx2*ny2+j2*nx2+i2;

        qframeout[i]=qframe1[i] - (f1*qframe2a[ijk2]+f2*qframe2b[ijk2]);
        if(qframe1[i]<valmin)valmin=qframe1[i];
        if(qframe1[i]>valmax)valmax=qframe1[i];
      }
      outsliceframe(unit3,is1a,is2a,js1a,js2a,ks1a,ks2a,time1,qframeout,&error1);
      if(error1!=0)break;
      getsliceframe(unit1,is1a,is2a,js1a,js2a,ks1a,ks2a,&time1,qframe1,slicetest1,&error1);
      if(error1!=0)break;
      UpdateHistogram(qframe1, NULL,nqframe1, slice1->histogram);
    }
    PRINTF("\n");
    FFLUSH();

    valmin_percentile = GetHistogramVal(slice1->histogram, 0.01);
    valmax_percentile = GetHistogramVal(slice1->histogram, 0.99);
    fprintf(stream_out,"MINMAXSLCF\n");
    fprintf(stream_out,"  %s\n",outfile2);
    fprintf(stream_out,"  %f %f %f %f\n",valmin,valmax,valmin_percentile,valmax_percentile);

    {
      FILE *stream_bnd=NULL;

      stream_bnd = fopen(outfile_bnd, "w");
      if(stream_bnd!=NULL){
        fprintf(stream_bnd, "%f %f %f", 0.0, valmin, valmax);
        fclose(stream_bnd);
      }
    }

    closefortranfile(unit1);
    closefortranfile(unit2);
    closefortranfile(unit3);
    FREEMEMORY(qframe1);
    FREEMEMORY(qframe2a);
    FREEMEMORY(qframe2b);
    FREEMEMORY(qframeout);
  }
}
