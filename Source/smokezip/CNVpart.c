#include "options.h"

#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>
#include "svzip.h"
#include "dmalloc.h"
#include "isobox.h"
#include "compress.h"
#include "getdata.h"


void Part2Iso(part *parti,int *thread_index);


/* ------------------ GetPartPropIndex ------------------------ */

int GetPartPropIndex(char *string){
  int i;
  partpropdata *partpropi;

  for(i=0;i<npart5propinfo;i++){
    partpropi = part5propinfo + i;
    if(strcmp(partpropi->label.shortlabel,string)==0)return i;
  }
  return -1;
}

/* ------------------ GetPartProp ------------------------ */

partpropdata *GetPartProp(char *string){
  int i;
  partpropdata *partpropi;

  for(i=0;i<npart5propinfo;i++){
    partpropi = part5propinfo + i;
    if(strcmp(partpropi->label.shortlabel,string)==0)return partpropi;
  }
  return NULL;
}


/* ------------------ ConvertParts2Iso ------------------------ */

void *ConvertParts2Iso(void *arg){
  int i;
  int *thread_index;

  thread_index=(int *)arg;

  LOCK_PART2ISO;
  if(GLOBfirst_part2iso==1){
    GLOBfirst_part2iso=0;

    if(GLOBcleanfiles==1){
      FILE *stream;
      int j;

      stream=FOPEN(GLOBsmvisofile,"rb");
      if(stream!=NULL){
        fclose(stream);
        PRINTF("  Removing %s\n",GLOBsmvisofile);
        UNLINK(GLOBsmvisofile);
        LOCK_COMPRESS;
        GLOBfilesremoved++;
        UNLOCK_COMPRESS;
      }
      for(j=0;j<npartinfo;j++){
        part *parti;

        parti = partinfo + j;

        for(i=0;i<npart5propinfo;i++){
          partpropdata *propi;
          flowlabels *labels;
          char isofilename[1024];

          propi = part5propinfo + i;
          labels = &propi->label;
          strcpy(isofilename,parti->file);
          strcat(isofilename,"_");
          strcat(isofilename,labels->shortlabel);
          strcat(isofilename,".tiso");

          stream=FOPEN(isofilename,"rb");
          if(stream!=NULL){
            fclose(stream);
            PRINTF("  Removing %s\n",isofilename);
            UNLINK(isofilename);
            LOCK_COMPRESS;
            GLOBfilesremoved++;
            UNLOCK_COMPRESS;
          }
        }
      }
      UNLOCK_PART2ISO;
      return NULL;
    }
  }
  UNLOCK_PART2ISO;

  if(GLOBcleanfiles==1)return NULL;

  if(GLOBpartfile2iso==1){
    for(i=0;i<npartinfo;i++){
      part *parti;

      parti = partinfo + i;
      LOCK_PART2ISO;
      if(parti->inuse_part2iso==1){
        UNLOCK_PART2ISO;
        continue;
      }
      parti->inuse_part2iso=1;
      UNLOCK_PART2ISO;

      Part2Iso(parti,thread_index);
    }
  }
  return NULL;
}

#define IJKVAL(ix,iy,iz) ((ix) + (iy)*nx2 + (iz)*nx2*ny2)

  /* ------------------ Part2Iso ------------------------ */

void Part2Iso(part *parti, int *thread_index){
  float *pdata;
  int *tagdata;
  int fdsversion;

  int blocknumber;
  FILE *unit;
  int error1;
  int nclasses;
  int *nquantities, *npoints, *partindex;
  float time_local;
  int error, size;
  int j;
  int npartcount, i;
  meshdata *partmesh;
  int nx, ny, nz;
  char *isofile, tisofile[1024];
  char isolonglabel[32], isoshortlabel[32], isounits[32];
  int nlevels;
  float levels[1];
  int reduce_triangles=1;
  float *xpltcell, *ypltcell, *zpltcell;
  int data2flag=1;
  float *partcount;
  FILE *SMVISOFILE=NULL;
  int nx2, ny2, nz2;
  float xmin, ymin, zmin;
  partpropdata *part5propinfo_copy=NULL;
  int percent_done;
  float file_size;
  int percent_next = PERCENT_SKIP;

  parti->compressed2=0;
#ifdef pp_THREAD
  if(GLOBcleanfiles==0){
    int fileindex;

    fileindex = parti + 1 - partinfo;
    sprintf(threadinfo[*thread_index].label,"prt2iso %i",fileindex);
  }
#else
  PRINTF("Converting %s to\n",parti->file);
#endif

  NewMemory((void **)&pdata,1000000*sizeof(float));
  NewMemory((void **)&tagdata,1000000*sizeof(int));
  NewMemory((void **)&partindex,1000000*sizeof(int));

  LOCK_COMPRESS;
  unit = openpart(parti->file,&error1);
  UNLOCK_COMPRESS;

  getpartheader1(unit,&nclasses,&fdsversion,&size);
  NewMemory((void **)&nquantities,nclasses*sizeof(int));
  NewMemory((void **)&npoints,nclasses*sizeof(int));

  getpartheader2(unit,nclasses,nquantities,&size);

  partmesh = parti->partmesh;

  blocknumber = partmesh-meshinfo + 1;

  nx = partmesh->ibar;
  ny = partmesh->jbar;
  nz = partmesh->kbar;

  nx2 = nx+2;
  ny2 = ny+2;
  nz2 = nz+2;

  npartcount = nx2*ny2*nz2;

  xmin = partmesh->xbar0-partmesh->dx;
  ymin = partmesh->ybar0-partmesh->dy;
  zmin = partmesh->zbar0-partmesh->dz;

  xpltcell = partmesh->xpltcell;
  ypltcell = partmesh->ypltcell;
  zpltcell = partmesh->zpltcell;

  NewMemory((void **)&isofile,strlen(parti->file)+5);
  strcpy(isofile,parti->file);
  strcat(isofile,".iso");

  NewMemory((void **)&tisofile,strlen(parti->file)+6);
  strcpy(tisofile,parti->file);
  strcat(tisofile,".tiso");

  strcpy(isolonglabel,"particle boundary");
  strcpy(isoshortlabel,"pbound");
  strcpy(isounits,"");

  nlevels=1;
  levels[0]=0.5;

  if(npart5propinfo>0)NewMemory((void **)&part5propinfo_copy,npart5propinfo*sizeof(partpropdata));
  if(part5propinfo_copy == NULL)return;

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    propi->used=0;
  }
  for(j=0;j<nclasses;j++){
    int k;

    for(k=0;k<nquantities[j];k++){
      partclassdata *classj;
      partpropdata *propi;

      classj=parti->classptr[j];
      propi=part5propinfo_copy+GetPartPropIndex(classj->labels[k].shortlabel);
      propi->used=1;
    }
  }

  NewMemory((void **)&partcount,npartcount*sizeof(float));

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    NewMemory((void **)&propi->partvals,npartcount*sizeof(float));
  }

  CCIsoHeader(isofile,isolonglabel,isoshortlabel,isounits,levels,&nlevels,&error);

  LOCK_PART2ISO;
  if(GLOBfirst_part2iso_smvopen==1){
    GLOBfirst_part2iso_smvopen=0;
    SMVISOFILE=FOPEN(GLOBsmvisofile,"w");
  }
  else{
    SMVISOFILE=FOPEN(GLOBsmvisofile,"a");
  }

  fprintf(SMVISOFILE,"ISOF %i\n",blocknumber);
  fprintf(SMVISOFILE," %s\n",isofile);
  fprintf(SMVISOFILE," %s\n",isolonglabel);

  fprintf(SMVISOFILE," %s\n",isoshortlabel);
  fprintf(SMVISOFILE," %s\n",isounits);
  fprintf(SMVISOFILE,"\n");

#ifndef pp_THREAD
  PRINTF("  %s\n",isofile);
#endif

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi,*propi_ro;
    flowlabels *labels;

    propi_ro = part5propinfo + i;
    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    labels = &propi_ro->label;
    strcpy(propi->isofilename,parti->file);
    strcat(propi->isofilename,"_");
    strcat(propi->isofilename,labels->shortlabel);
    strcat(propi->isofilename,".tiso");
    CCTIsoHeader(propi->isofilename, labels->longlabel, labels->shortlabel, labels->unit, levels, &nlevels, &error);
#ifndef pp_THREAD
    PRINTF("  %s\n",propi->isofilename);
#endif

    fprintf(SMVISOFILE,"TISOF %i\n",blocknumber);
    fprintf(SMVISOFILE," %s\n",propi->isofilename);
    fprintf(SMVISOFILE," %s\n",isolonglabel);
    fprintf(SMVISOFILE," %s\n",isoshortlabel);
    fprintf(SMVISOFILE," %s\n",isounits);
    fprintf(SMVISOFILE," %s\n",labels->longlabel);
    fprintf(SMVISOFILE," %s\n",labels->shortlabel);
    fprintf(SMVISOFILE," %s\n",labels->unit);
    fprintf(SMVISOFILE," \n");
  }
  fclose(SMVISOFILE);
  UNLOCK_PART2ISO;

#ifndef pp_THREAD
  PRINTF(" ");
#endif
  error=0;
  file_size=0.0;
  for(;;){
    float *x, *y, *z, *vals;
    int k;

    getpartdataframe(unit,nclasses,nquantities,npoints,&time_local,tagdata,pdata,&size,&error);

    file_size+=size;

    percent_done=100.0*(float)file_size/(float)parti->filesize;
#ifdef pp_THREAD
    threadinfo[*thread_index].stat=percent_done;
    if(percent_done>percent_next){
      LOCK_PRINT;
      PrintThreadStats();
      UNLOCK_PRINT;
      percent_next += PERCENT_SKIP;
    }
#else
    if(percent_done>percent_next){
      PRINTF(" %i%s",percent_next,GLOBpp);
      FFLUSH();
      percent_next += PERCENT_SKIP;
    }
#endif
    if(error!=0)break;

    for(j=0;j<npartcount;j++){
      partcount[j]=0.0;
    }
    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;

      for(j=0;j<npartcount;j++){
        propi->partvals[j]=0.0;
      }
    }

    vals=pdata;
    for(j=0;j<nclasses;j++){
      partclassdata *classj;

      if(npoints[j]==0)continue;
      classj=parti->classptr[j];
      x = vals;
      y = x + npoints[j];
      z = y + npoints[j];
      vals = z + npoints[j];

// construct 3D particle density array

      for(i=0;i<npoints[j];i++){
        int ix, iy, iz;
        int ijkval;

        ix = GETINDEX(x[i],xmin,partmesh->dx,nx2);
        iy = GETINDEX(y[i],ymin,partmesh->dy,ny2);
        iz = GETINDEX(z[i],zmin,partmesh->dz,nz2);
        ijkval = IJKVAL(ix,iy,iz);
        partindex[i]=ijkval;
        partcount[ijkval]++;
      }
      for(k=0;k<nquantities[j];k++){
        partpropdata *propi;

        propi=part5propinfo_copy+GetPartPropIndex(classj->labels[k].shortlabel);

        for(i=0;i<npoints[j];i++){
          int ijkval;

          ijkval = partindex[i];
          propi->partvals[ijkval]+=vals[i];
        }
        for(i=0;i<npartcount;i++){
          if(partcount[i]>0){
            propi->partvals[i]/=partcount[i];
          }
        }
        vals += npoints[j];
      }
    }
    CCIsoSurface2File(isofile, &time_local, partcount, NULL, levels, &nlevels,
        xpltcell, &nx2, ypltcell, &ny2, zpltcell, &nz2,
        &reduce_triangles, &error);

    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;

      CCIsoSurfaceT2File(propi->isofilename, &time_local, partcount, &data2flag, propi->partvals, NULL, levels, &nlevels,
            xpltcell, &nx2, ypltcell, &ny2, zpltcell, &nz2,
            &reduce_triangles, &error);
    }
  }

#ifdef pp_THREAD
  {
    int nconv=1;
    int lenfile;
    char **summaries;

    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==1)nconv++;
    }

    NewMemory((void **)&summaries,nconv*sizeof(char *));

    lenfile=strlen(isofile);
    NewMemory((void **)&summaries[0],lenfile+1);
    strcpy(summaries[0],isofile);

    parti->summaries=summaries;
    parti->nsummaries=nconv;

    nconv=1;
    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;
      lenfile=strlen(propi->isofilename);
      NewMemory((void **)&summaries[nconv],lenfile+1);
      strcpy(summaries[nconv],propi->isofilename);
      nconv++;
    }
    parti->compressed2=1;
    threadinfo[*thread_index].stat=-1;
  }
#else
  PRINTF(" 100%s completed\n",GLOBpp);
#endif

  FREEMEMORY(nquantities);
  FREEMEMORY(npoints);
  FREEMEMORY(partcount);
  FREEMEMORY(isofile);
  LOCK_COMPRESS;
  closefortranfile(unit);
  UNLOCK_COMPRESS;

  FREEMEMORY(pdata);
  FREEMEMORY(tagdata);
  FREEMEMORY(partindex);

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    FREEMEMORY(propi->partvals);
    FREEMEMORY(propi->partvals);
  }
  if(npart5propinfo>0){
    FREEMEMORY(part5propinfo_copy);
  }
}

/* ------------------ Part2Object ------------------------ */

void Part2Object(part *parti, int *thread_index){
  float *pdata;
  int *tagdata;
  int fdsversion;

  int blocknumber;
  FILE *unit;
  int error1;
  int nclasses;
  int *nquantities, *npoints, *partindex;
  float time_local;
  int error, size;
  int j;
  int npartcount, i;
  meshdata *partmesh;
  int nx, ny, nz;
  char *isofile, tisofile[1024];
  char isolonglabel[32], isoshortlabel[32], isounits[32];
  int nlevels;
  float levels[1];
  int reduce_triangles=1;
  float *xpltcell, *ypltcell, *zpltcell;
  int data2flag=1;
  float *partcount;
  FILE *SMVISOFILE=NULL;
  int nx2, ny2, nz2;
  float xmin, ymin, zmin;
  partpropdata *part5propinfo_copy=NULL;
  int percent_done;
  float file_size;
  int percent_next = PERCENT_SKIP;

  parti->compressed2=0;
#ifdef pp_THREAD
  if(GLOBcleanfiles==0){
    int fileindex;

    fileindex = parti + 1 - partinfo;
    sprintf(threadinfo[*thread_index].label,"prt2iso %i",fileindex);
  }
#else
  PRINTF("Converting %s to\n",parti->file);
#endif

  NewMemory((void **)&pdata,1000000*sizeof(float));
  NewMemory((void **)&tagdata,1000000*sizeof(int));
  NewMemory((void **)&partindex,1000000*sizeof(int));

  LOCK_COMPRESS;
  unit = openpart(parti->file,&error1);
  UNLOCK_COMPRESS;

  getpartheader1(unit,&nclasses,&fdsversion,&size);
  NewMemory((void **)&nquantities,nclasses*sizeof(int));
  NewMemory((void **)&npoints,nclasses*sizeof(int));

  getpartheader2(unit,nclasses,nquantities,&size);

  partmesh = parti->partmesh;

  blocknumber = partmesh-meshinfo + 1;

  nx = partmesh->ibar;
  ny = partmesh->jbar;
  nz = partmesh->kbar;

  nx2 = nx+2;
  ny2 = ny+2;
  nz2 = nz+2;

  npartcount = nx2*ny2*nz2;

  xmin = partmesh->xbar0-partmesh->dx;
  ymin = partmesh->ybar0-partmesh->dy;
  zmin = partmesh->zbar0-partmesh->dz;

  xpltcell = partmesh->xpltcell;
  ypltcell = partmesh->ypltcell;
  zpltcell = partmesh->zpltcell;

  NewMemory((void **)&isofile,strlen(parti->file)+5);
  strcpy(isofile,parti->file);
  strcat(isofile,".iso");

  NewMemory((void **)&tisofile,strlen(parti->file)+6);
  strcpy(tisofile,parti->file);
  strcat(tisofile,".tiso");

  strcpy(isolonglabel,"particle boundary");
  strcpy(isoshortlabel,"pbound");
  strcpy(isounits,"");

  nlevels=1;
  levels[0]=0.5;

  if(npart5propinfo>0)NewMemory((void **)&part5propinfo_copy,npart5propinfo*sizeof(partpropdata));
  if(part5propinfo_copy==NULL)return;

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    propi->used=0;
  }
  for(j=0;j<nclasses;j++){
    int k;

    for(k=0;k<nquantities[j];k++){
      partclassdata *classj;
      partpropdata *propi;

      classj=parti->classptr[j];
      propi=part5propinfo_copy+GetPartPropIndex(classj->labels[k].shortlabel);
      propi->used=1;
    }
  }

  NewMemory((void **)&partcount,npartcount*sizeof(float));

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    NewMemory((void **)&propi->partvals,npartcount*sizeof(float));
  }

  CCIsoHeader(isofile,isolonglabel,isoshortlabel,isounits,levels,&nlevels,&error);

  LOCK_PART2ISO;
  if(GLOBfirst_part2iso_smvopen==1){
    GLOBfirst_part2iso_smvopen=0;
    SMVISOFILE=FOPEN(GLOBsmvisofile,"w");
  }
  else{
    SMVISOFILE=FOPEN(GLOBsmvisofile,"a");
  }

  fprintf(SMVISOFILE,"ISOF %i\n",blocknumber);
  fprintf(SMVISOFILE," %s\n",isofile);
  fprintf(SMVISOFILE," %s\n",isolonglabel);

  fprintf(SMVISOFILE," %s\n",isoshortlabel);
  fprintf(SMVISOFILE," %s\n",isounits);
  fprintf(SMVISOFILE,"\n");

#ifndef pp_THREAD
  PRINTF("  %s\n",isofile);
#endif

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi,*propi_ro;
    flowlabels *labels;

    propi_ro = part5propinfo + i;
    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    labels = &propi_ro->label;
    strcpy(propi->isofilename,parti->file);
    strcat(propi->isofilename,"_");
    strcat(propi->isofilename,labels->shortlabel);
    strcat(propi->isofilename,".tiso");
    CCTIsoHeader(propi->isofilename, labels->longlabel, labels->shortlabel, labels->unit, levels, &nlevels, &error);
#ifndef pp_THREAD
    PRINTF("  %s\n",propi->isofilename);
#endif

    fprintf(SMVISOFILE,"TISOF %i\n",blocknumber);
    fprintf(SMVISOFILE," %s\n",propi->isofilename);
    fprintf(SMVISOFILE," %s\n",isolonglabel);
    fprintf(SMVISOFILE," %s\n",isoshortlabel);
    fprintf(SMVISOFILE," %s\n",isounits);
    fprintf(SMVISOFILE," %s\n",labels->longlabel);
    fprintf(SMVISOFILE," %s\n",labels->shortlabel);
    fprintf(SMVISOFILE," %s\n",labels->unit);
    fprintf(SMVISOFILE," \n");
  }
  fclose(SMVISOFILE);
  UNLOCK_PART2ISO;

#ifndef pp_THREAD
  PRINTF(" ");
#endif
  error=0;
  file_size=0.0;
  for(;;){
    float *x, *y, *z, *vals;
    int k;

    getpartdataframe(unit,nclasses,nquantities,npoints,&time_local,tagdata,pdata,&size,&error);

    file_size+=size;

    percent_done=100.0*(float)file_size/(float)parti->filesize;
#ifdef pp_THREAD
    threadinfo[*thread_index].stat=percent_done;
    if(percent_done>percent_next){
      LOCK_PRINT;
      PrintThreadStats();
      UNLOCK_PRINT;
      percent_next+= PERCENT_SKIP;
    }
#else
    if(percent_done>percent_next){
      PRINTF(" %i%s",percent_next,GLOBpp);
      FFLUSH();
      percent_next+= PERCENT_SKIP;
    }
#endif
    if(error!=0)break;

    for(j=0;j<npartcount;j++){
      partcount[j]=0.0;
    }
    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;

      for(j=0;j<npartcount;j++){
        propi->partvals[j]=0.0;
      }
    }

    vals=pdata;
    for(j=0;j<nclasses;j++){
      partclassdata *classj;

      if(npoints[j]==0)continue;
      classj=parti->classptr[j];
      x = vals;
      y = x + npoints[j];
      z = y + npoints[j];
      vals = z + npoints[j];

      // construct 3D particle density array

      for(i=0;i<npoints[j];i++){
        int ix, iy, iz;
        int ijkval;

        ix = GETINDEX(x[i],xmin,partmesh->dx,nx2);
        iy = GETINDEX(y[i],ymin,partmesh->dy,ny2);
        iz = GETINDEX(z[i],zmin,partmesh->dz,nz2);
        ijkval = IJKVAL(ix,iy,iz);
        partindex[i]=ijkval;
        partcount[ijkval]++;
      }
      for(k=0;k<nquantities[j];k++){
        partpropdata *propi;

        propi=part5propinfo_copy+GetPartPropIndex(classj->labels[k].shortlabel);

        for(i=0;i<npoints[j];i++){
          int ijkval;

          ijkval = partindex[i];
          propi->partvals[ijkval]+=vals[i];
        }
        for(i=0;i<npartcount;i++){
          if(partcount[i]>0){
            propi->partvals[i]/=partcount[i];
          }
        }
        vals += npoints[j];
      }
    }
    CCIsoSurface2File(isofile, &time_local, partcount, NULL, levels, &nlevels,
      xpltcell, &nx2, ypltcell, &ny2, zpltcell, &nz2,
      &reduce_triangles, &error);

    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;

      CCIsoSurfaceT2File(propi->isofilename, &time_local, partcount, &data2flag, propi->partvals, NULL, levels, &nlevels,
        xpltcell, &nx2, ypltcell, &ny2, zpltcell, &nz2,
        &reduce_triangles, &error);
    }
  }

#ifdef pp_THREAD
  {
    int nconv=1;
    int lenfile;
    char **summaries;

    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==1)nconv++;
    }

    NewMemory((void **)&summaries,nconv*sizeof(char *));

    lenfile=strlen(isofile);
    NewMemory((void **)&summaries[0],lenfile+1);
    strcpy(summaries[0],isofile);

    parti->summaries=summaries;
    parti->nsummaries=nconv;

    nconv=1;
    for(i=0;i<npart5propinfo;i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;
      lenfile=strlen(propi->isofilename);
      NewMemory((void **)&summaries[nconv],lenfile+1);
      strcpy(summaries[nconv],propi->isofilename);
      nconv++;
    }
    parti->compressed2=1;
    threadinfo[*thread_index].stat=-1;
  }
#else
  PRINTF(" 100%s completed\n",GLOBpp);
#endif

  FREEMEMORY(nquantities);
  FREEMEMORY(npoints);
  FREEMEMORY(partcount);
  FREEMEMORY(isofile);
  LOCK_COMPRESS;
  closefortranfile(unit);
  UNLOCK_COMPRESS;

  FREEMEMORY(pdata);
  FREEMEMORY(tagdata);
  FREEMEMORY(partindex);

  for(i=0;i<npart5propinfo;i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    FREEMEMORY(propi->partvals);
    FREEMEMORY(propi->partvals);
  }
  if(npart5propinfo>0){
    FREEMEMORY(part5propinfo_copy);
  }
}
