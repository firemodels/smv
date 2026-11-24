#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "part2iso.h"
#include "dmalloc.h"
#include "isobox.h"
#include "getdata.h"

/* ------------------ GetPartPropIndex ------------------------ */

int GetPartPropIndex(char *string){
  int i;
  partpropdata *partpropi;

  for(i=0; i<npart5propinfo; i++){
    partpropi = part5propinfo + i;
    if(strcmp(partpropi->label.shortlabel,string)==0)return i;
  }
  return -1;
}

/* ------------------ GetPartProp ------------------------ */

partpropdata *GetPartProp(char *string){
  int i;
  partpropdata *partpropi;

  for(i=0; i<npart5propinfo; i++){
    partpropi = part5propinfo + i;
    if(strcmp(partpropi->label.shortlabel,string)==0)return partpropi;
  }
  return NULL;
}

#define IJKVAL(ix,iy,iz) ((ix) + (iy)*nx2 + (iz)*nx2*ny2)

/* ------------------ Part2Iso ------------------------ */

void Part2Iso(part *parti){
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
  char *isofile;
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

  PRINTF("Converting %s to\n",parti->file);

  NewMemory((void **)&pdata,1000000*sizeof(float));
  NewMemory((void **)&tagdata,1000000*sizeof(int));
  NewMemory((void **)&partindex,1000000*sizeof(int));

  unit = openpart(parti->file,&error1);

  if(unit==NULL||error1!=0)return;
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

  strcpy(isolonglabel,"particle boundary");
  strcpy(isoshortlabel,"pbound");
  strcpy(isounits,"");

  nlevels=1;
  levels[0]=0.5;

  // Keep the property array valid even for particle classes without quantities.
  NewMemory((void **)&part5propinfo_copy,MAX(1,npart5propinfo)*sizeof(partpropdata));
  if(part5propinfo_copy==NULL)return;

  for(i=0; i<npart5propinfo; i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    propi->used=0;
  }
  for(j=0; j<nclasses; j++){
    int k;

    for(k=0; k<nquantities[j]; k++){
      partclassdata *classj;
      partpropdata *propi;
      int propindex;

      classj=parti->classptr[j];
      propindex=GetPartPropIndex(classj->labels[k].shortlabel);
      if(propindex<0||propindex>=npart5propinfo)continue;
      propi=part5propinfo_copy+propindex;
      propi->used=1;
    }
  }

  NewMemory((void **)&partcount,npartcount*sizeof(float));

  for(i=0; i<npart5propinfo; i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    NewMemory((void **)&propi->partvals,npartcount*sizeof(float));
  }

  CCIsoHeader(isofile,isolonglabel,isoshortlabel,isounits,levels,&nlevels,&error);

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

  PRINTF("  %s\n",isofile);

  for(i=0; i<npart5propinfo; i++){
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
    PRINTF("  %s\n",propi->isofilename);

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

  PRINTF(" ");
  error=0;
  file_size=0.0;
  for(;;){
    float *x, *y, *z, *vals;
    int k;

    getpartdataframe(unit,nclasses,nquantities,npoints,&time_local,tagdata,pdata,&size,&error);

    file_size+=size;

    percent_done=100.0*(float)file_size/(float)parti->filesize;
    if(percent_done>percent_next){
      PRINTF(" %i%s",percent_next,GLOBpp);
      FFLUSH();
      percent_next += PERCENT_SKIP;
    }
    if(error!=0)break;

    for(j=0; j<npartcount; j++){
      partcount[j]=0.0;
    }
    for(i=0; i<npart5propinfo; i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;

      for(j=0; j<npartcount; j++){
        propi->partvals[j]=0.0;
      }
    }

    vals=pdata;
    for(j=0; j<nclasses; j++){
      partclassdata *classj;

      if(npoints[j]==0)continue;
      classj=parti->classptr[j];
      x = vals;
      y = x + npoints[j];
      z = y + npoints[j];
      vals = z + npoints[j];

// construct 3D particle density array

      for(i=0; i<npoints[j]; i++){
        int ix, iy, iz;
        int ijkval;

        ix = GETINDEX(x[i],xmin,partmesh->dx,nx2);
        iy = GETINDEX(y[i],ymin,partmesh->dy,ny2);
        iz = GETINDEX(z[i],zmin,partmesh->dz,nz2);
        ijkval = IJKVAL(ix,iy,iz);
        partindex[i]=ijkval;
        partcount[ijkval]++;
      }
      for(k=0; k<nquantities[j]; k++){
        partpropdata *propi;
        int propindex;

        propindex=GetPartPropIndex(classj->labels[k].shortlabel);
        if(propindex<0||propindex>=npart5propinfo){
          vals+=npoints[j];
          continue;
        }
        propi=part5propinfo_copy+propindex;

        for(i=0; i<npoints[j]; i++){
          int ijkval;

          ijkval = partindex[i];
          propi->partvals[ijkval]+=vals[i];
        }
        for(i=0; i<npartcount; i++){
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

    for(i=0; i<npart5propinfo; i++){
      partpropdata *propi;

      propi = part5propinfo_copy + i;
      if(propi->used==0)continue;

      CCIsoSurfaceT2File(propi->isofilename, &time_local, partcount, &data2flag, propi->partvals, NULL, levels, &nlevels,
            xpltcell, &nx2, ypltcell, &ny2, zpltcell, &nz2,
            &reduce_triangles, &error);
    }
  }

  PRINTF(" 100%s completed\n",GLOBpp);

  FREEMEMORY(nquantities);
  FREEMEMORY(npoints);
  FREEMEMORY(partcount);
  FREEMEMORY(isofile);

  closefortranfile(unit);

  FREEMEMORY(pdata);
  FREEMEMORY(tagdata);
  FREEMEMORY(partindex);

  for(i=0; i<npart5propinfo; i++){
    partpropdata *propi;

    propi = part5propinfo_copy + i;
    if(propi->used==0)continue;

    FREEMEMORY(propi->partvals);
  }
  FREEMEMORY(part5propinfo_copy);
}

