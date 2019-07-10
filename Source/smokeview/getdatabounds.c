#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"

/* ------------------ GetGlobalSliceBounds ------------------------ */

void GetGlobalSliceBounds(void){
  int j;

  for(j = 0;j<nslicebounds;j++){
    boundsdata *boundi;

    boundi = slicebounds+j;
    boundi->global_valmin = 1.0;
    boundi->global_valmax = 0.0;
  }
  for(j = 0;j<nsliceinfo;j++){
    slicedata *slicej;
    int i, count;
    float valmin, valmax;

    slicej = sliceinfo+j;
    if(slicej->firstshort_slice!=1)continue;
    count = 0;
    for(i = 0;i<nsliceinfo;i++){
      slicedata *slicei;
      FILE *stream;

      slicei = sliceinfo+i;
      if(strcmp(slicei->label.shortlabel, slicej->label.shortlabel)!=0)continue;
      stream = fopen(slicei->bound_file, "r");
      if(stream==NULL)continue;
      while(!feof(stream)){
        char buffer[255];
        float t, vmin, vmax;

        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
        if(count==0){
          valmin = vmin;
          valmax = vmax;
        }
        else{
          valmin = MIN(valmin, vmin);
          valmax = MAX(valmax, vmax);
        }
        count++;
      }
      fclose(stream);
    }
    if(count>0){
      int bound_index;

      bound_index = GetSliceBoundsIndex(slicej);
      if(bound_index>=0){
        boundsdata *boundi;

        boundi = slicebounds+bound_index;
        boundi->global_valmin = valmin;
        boundi->global_valmax = valmax;

      }
    }
  }
}

/* ------------------ AdjustDataBounds ------------------------ */

void AdjustDataBounds(const float *pdata, int local_skip, int ndata,
                   int setpmin, float *pmin, int setpmax, float *pmax)
{
    int nsmall, nbig, *buckets=NULL, n, level, total, alpha05;
    float dp, pmin2, pmax2;

    if(setpmin==PERCENTILE_MIN||setpmax==PERCENTILE_MAX){
      dp = (*pmax - *pmin)/NBUCKETS;
      nsmall=0;
      nbig=NBUCKETS;
      if(NewMemory((void **)&buckets,NBUCKETS*sizeof(int))==0){
        fprintf(stderr,"*** Error: Unable to allocate memory in getdatabounds\n");
        return;
      }

      for(n=0;n<NBUCKETS;n++){
        buckets[n]=0;
      }
      for(n=local_skip;n<ndata;n++){
        level=0;
        if(dp!=0.0f)level = CLAMP((int)((pdata[n] - *pmin)/dp),0,NBUCKETS-1);
        buckets[level]++;
      }
      alpha05 = (int)(percentile_level*ndata);
      total = 0;
      for(n=0;n<NBUCKETS;n++){
        total += buckets[n];
        if(total>alpha05){
          nsmall=n;
          break;
        }
      }
      total = 0;
      for(n=NBUCKETS;n>0;n--){
        total += buckets[n-1];
        if(total>alpha05){
          nbig=n;
          break;
        }
      }
      pmin2 = *pmin + (nsmall-1)*dp;
      pmax2 = *pmin + (nbig+1)*dp;
      if(setpmin==PERCENTILE_MIN)*pmin = pmin2;
      if(setpmax==PERCENTILE_MAX)*pmax = pmax2;
      FreeMemory(buckets);
    }
    if(axislabels_smooth==1){
      SmoothLabel(pmin,pmax,nrgb);
    }
}


/* ------------------ AdjustPart5Chops ------------------------ */

void AdjustPart5Chops(void){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    propi->imin=0;
    propi->imax=255;
    if(propi->setchopmin==1){
      float dval;

      dval = propi->valmax-propi->valmin;
      if(dval<=0.0)dval=1.0;
      propi->imin=CLAMP(255*(propi->chopmin-propi->valmin)/dval,0,255);
    }
    if(propi->setchopmax==1){
      float dval;

      dval = propi->valmax-propi->valmin;
      if(dval<=0.0)dval=1.0;
      propi->imax=CLAMP(255*(propi->chopmax-propi->valmin)/dval,0,255);
    }
  }
}

/* -----  ------------- ReadPartBounds ------------------------ */

int ReadPartBounds(partdata *parti){
  FILE *stream=NULL;
  int j, eof=0;
  float *valmin, *valmax;
  int part_boundfile_version = 1;

  parti->bounds_set = 0;
  if(parti->valmin==NULL)NewMemory((void **)&parti->valmin, npart5prop*sizeof(float));
  if(parti->valmax==NULL)NewMemory((void **)&parti->valmax, npart5prop*sizeof(float));
  valmin = parti->valmin;
  valmax = parti->valmax;
  for(j=0;j<npart5prop;j++){
    valmin[j] = 1000000000.0;
    valmax[j] = -1000000000.0;
  }

  stream = fopen(parti->bound_file, "r");
  if(stream==NULL)return 0;
  for(;;){
    float time;
    int nclasses, k, version=-1;
    char buffer[255];

    if(fgets(buffer, 255, stream)==NULL)break;
    sscanf(buffer, "%f %i %i", &time, &nclasses, &version);
    if(version!=part_boundfile_version){
      fclose(stream);
      return 0;
    }

    for(k = 0; k<nclasses; k++){
      int nbounds, npoints;

      if(fgets(buffer, 255, stream)==NULL){
        eof = 1;
        break;
      }
      sscanf(buffer, "%i %i", &nbounds, &npoints);
      for(j = 0; j<nbounds; j++){
        float vmin, vmax;
        int prop_index;

        if(fgets(buffer, 255, stream)==NULL){
          eof = 1;
          break;
        }
        sscanf(buffer, "%f %f", &vmin, &vmax);
        if(vmax>=vmin){
          parti->bounds_set = 1;

          prop_index = GetPartPropIndex(k,j+2);

          valmin[prop_index] = MIN(valmin[prop_index], vmin);
          valmax[prop_index] = MAX(valmax[prop_index], vmax);
        }
      }
      if(eof==1)break;
    }
    if(eof==1)break;
  }
  fclose(stream);
  return 1;
}

/* ------------------ ReadAllPartBounds ------------------------ */

int ReadAllPartBounds(void){
  int i, have_bound_file=1;

  // find min/max for each particle file

  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(ReadPartBounds(parti)==0)have_bound_file=0;
  }

  for(i = 0; i<npart5prop; i++){
    partpropdata *propi;

    propi = part5propinfo+i;
    if(strcmp(propi->label->shortlabel, "Uniform")==0)continue;
    propi->valmin =  1000000000.0;
    propi->valmax = -1000000000.0;
  }

  // find min/max over all particle files

  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int j;

    parti = partinfo+i;
    if(parti->bounds_set==0)continue;
    for(j = 0; j<npart5prop; j++){
      partpropdata *propj;

      propj = part5propinfo + j;
      if(strcmp(propj->label->shortlabel, "Uniform")==0)continue;
      propj->valmin = MIN(propj->valmin,parti->valmin[j]);
      propj->valmax = MAX(propj->valmax,parti->valmax[j]);
    }
  }

  // set min/max for each particle file

  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int j;

    parti = partinfo+i;
    for(j = 0; j<npart5prop; j++){
      partpropdata *propj;

      propj = part5propinfo+j;
      if(strcmp(propj->label->shortlabel, "Uniform")==0)continue;
      parti->valmin[j] = propj->valmin;
      parti->valmax[j] = propj->valmax;
    }
  }

  // set properties

  for(i = 0; i<npart5prop; i++){
    partpropdata *propi;

    propi = part5propinfo+i;
    if(strcmp(propi->label->shortlabel, "Uniform")==0)continue;

    propi->global_min = propi->valmin;
    propi->global_max = propi->valmax;
    propi->setvalmax = GLOBAL_MAX;
    propi->setvalmin = GLOBAL_MIN;
  }
  return have_bound_file;
}

/* ------------------ GetPartBounds ------------------------ */

void GetPartBounds(void){
  int i;

  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(parti->valmin==NULL){
      NewMemory((void **)&parti->valmin, npart5prop*sizeof(float));
    }
    if(parti->valmax==NULL){
      NewMemory((void **)&parti->valmax, npart5prop*sizeof(float));
    }
  }
  if(partfast==YES){
    if(update_part_bounds==1){
      int have_bound_file = 0;

      have_bound_file = ReadAllPartBounds();
      update_part_bounds = 0;
      if(have_bound_file==0){
        printf("***warning: Unable to read one or more particle bound files. Reverting to normal particle loading\n");
        partfast = NO;
        updatemenu = 1;
        UpdateGluiPartfast();
        GetPartBounds();
      }
    }
  }
  else{
    for(i=0;i<npart5prop;i++){
      partpropdata *propi;
      histogramdata *histi;
      int j;

      propi = part5propinfo+i;
      if(strcmp(propi->label->shortlabel, "Uniform")==0)continue;

      histi = &propi->histogram;

      propi->global_min = histi->val_min;
      propi->global_max = histi->val_max;

      propi->percentile_min = GetHistogramVal(histi, percentile_level);
      propi->percentile_max = GetHistogramVal(histi, 1.0-percentile_level);

      switch(propi->setvalmin){
      case PERCENTILE_MIN:
        propi->valmin = propi->percentile_min;
        break;
      case GLOBAL_MIN:
        propi->valmin = propi->global_min;
        break;
      case SET_MIN:
        propi->valmin = propi->user_min;
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      switch(propi->setvalmax){
      case PERCENTILE_MAX:
        propi->valmax = propi->percentile_max;
        break;
      case GLOBAL_MAX:
        propi->valmax = propi->global_max;
        break;
      case SET_MAX:
        propi->valmax = propi->user_max;
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      for(j = 0; j<npartinfo; j++){
        partdata *partj;

        partj = partinfo+j;
        partj->valmin[i] = propi->valmin;
        partj->valmax[i] = propi->valmax;
      }
    }
  }
  AdjustPart5Chops();
#ifdef _DEBUG
  PrintPartProp();
#endif
}

/* ------------------ AdjustPlot3DBounds ------------------------ */

void AdjustPlot3DBounds(int plot3dvar, int setpmin, float *pmin, int setpmax, float *pmax)
{
    int nsmall, nbig, *buckets=NULL, n, level, total, alpha05;
    float dp, pmin2, pmax2;
    plot3ddata *p;
    meshdata *meshi;
    int i;
    char *iblank;
    float *q;
    int ndata=0;
    int ntotal;

    if(setpmin==PERCENTILE_MIN||setpmax==PERCENTILE_MAX){
      dp = (*pmax - *pmin)/NBUCKETS;
      nsmall=0;
      nbig=NBUCKETS;
      if(NewMemory((void **)&buckets,NBUCKETS*sizeof(int))==0){
        fprintf(stderr,"*** Error: Unable to allocate memory in getdatabounds\n");
        return;
      }

      for(n=0;n<NBUCKETS;n++){
        buckets[n]=0;
      }
      for(i=0;i<nplot3dinfo;i++){
        p = plot3dinfo+i;
        if(p->loaded==0||p->display==0)continue;
        meshi=meshinfo+p->blocknumber;
        ntotal=(meshi->ibar+1)*(meshi->jbar+1)*(meshi->kbar+1);
        ndata += ntotal;
        iblank=meshi->c_iblank_node;
        q=meshi->qdata+plot3dvar*ntotal;
        for(n=0;n<ntotal;n++){
          if(iblank==NULL||*iblank++==GAS){
            level=0;
            if(dp!=0.0f)level = CLAMP((int)((q[n] - *pmin)/dp),0,NBUCKETS-1);
            buckets[level]++;
          }
        }
      }
      alpha05 = (int)(percentile_level*ndata);
      total = 0;
      for(n=0;n<NBUCKETS;n++){
        total += buckets[n];
        if(total>alpha05){
          nsmall=n;
          break;
        }
      }
      total = 0;
      for(n=NBUCKETS;n>0;n--){
        total += buckets[n-1];
        if(total>alpha05){
          nbig=n;
          break;
        }
      }
      pmin2 = *pmin + (nsmall-1)*dp;
      pmax2 = *pmin + (nbig+1)*dp;
      if(setpmin==PERCENTILE_MIN)*pmin = pmin2;
      if(setpmax==PERCENTILE_MAX)*pmax = pmax2;
      FreeMemory(buckets);
    }
    if(axislabels_smooth==1&&setpmin!=SET_MIN&&setpmax!=SET_MAX){
      SmoothLabel(pmin,pmax,nrgb);
    }
}

/* ------------------ GetZoneGlobalBounds ------------------------ */

void GetZoneGlobalBounds(const float *pdata, int ndata, float *pglobalmin, float *pglobalmax)
{
    int n;
    float pmin2, pmax2, val;

    pmin2 = pdata[0];
    pmax2 = pmin2;
    for(n=0;n<ndata;n++){
      val=*pdata++;
      pmin2=MIN(val,pmin2);
      pmax2=MAX(val,pmax2);
    }
    *pglobalmin = pmin2;
    *pglobalmax = pmax2;
}

/* ------------------ SmoothLabel ------------------------ */

void SmoothLabel(float *a, float *b, int n){
  double delta, factor, logdelta;
  int ndigits;
  double half;

  half=0.5;

  delta = ((double)*b-(double)*a)/(double)(n-2);
  if(delta==0.0)return;
  logdelta = log10((double)delta);
  ndigits=logdelta-1;
  if(logdelta<=1)ndigits--;
  factor = 5*pow(10,ndigits);
  delta = (int)(delta/factor + half)*factor;

  *a = factor*(int)(*a/factor+half);
  *b = *a + (n-2)*delta;
}
