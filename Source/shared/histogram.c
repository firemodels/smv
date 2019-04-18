#include "options.h"
#include "lint.h"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "histogram.h"
#include "pragmas.h"
#include "MALLOC.h"
#include "datadefs.h"

/* ------------------ GetHistogramCDF ------------------------ */

float GetHistogramCDF(histogramdata *histogram, float val){
  int i;
  int cutoff;
  float sum = 0;

  if(histogram->val_max <= histogram->val_min)return 1.0;

  cutoff = (val - histogram->val_min)*(float)histogram->nbuckets / (histogram->val_max - histogram->val_min);
  for(i = 0; i < cutoff; i++){
    sum += histogram->buckets[i];
  }
  return (float)sum / (float)histogram->ntotal;
}

  /* ------------------ GetHistogramVal ------------------------ */

float GetHistogramVal(histogramdata *histogram, float cdf){

// get value of histogram for value cdf

  float cutoff, count;
  int i;
  float returnval;

  if(cdf<=0.0){
    return histogram->val_min;
  }
  if(cdf>=1.0){
    return histogram->val_max;
  }
  cutoff = cdf*histogram->ntotal;
  count=0;
  for(i=0;i<histogram->nbuckets;i++){
    count+=histogram->buckets[i];
    if(count>cutoff){
      returnval = histogram->val_min + (float)(i+0.5)*(histogram->val_max-histogram->val_min)/(float)histogram->nbuckets;
      return returnval;
    }
  }
  return histogram->val_max;
}

/* ------------------ CompleteHistogram ------------------------ */

void CompleteHistogram(histogramdata *histogram){

// set variable indicating that histogram is complete

  histogram->complete=1;
}

/* ------------------ ResetHistogram ------------------------ */

void ResetHistogram(histogramdata *histogram, float *valmin, float *valmax){

  // initialize histogram data structures

  int i;

  for(i = 0; i<histogram->nbuckets; i++){
    histogram->buckets[i] = 0;
  }
  histogram->defined = 0;
  histogram->ntotal = 0;
  if(valmin != NULL){
    histogram->val_min = *valmin;
  }
  else{
    histogram->val_min = (float)pow(10.0, 20.0);
  }
  if(valmax != NULL){
    histogram->val_max = *valmax;
  }
  else{
    histogram->val_max = -(float)pow(10.0, 20.0);
  }
  histogram->complete = 0;
}

/* ------------------ InitHistogram ------------------------ */

void InitHistogram(histogramdata *histogram, int nbuckets, float *valmin, float *valmax){

// initialize histogram data structures

  histogram->time_defined = 0;
  histogram->time = -1.0;
  histogram->buckets=NULL;
  histogram->buckets_polar = NULL;
  NewMemory((void **)&histogram->buckets, nbuckets*sizeof(float));
  histogram->ndim = 1;
  histogram->nbuckets = nbuckets;
  ResetHistogram(histogram,valmin,valmax);
}

/* ------------------ FreeHistogram ------------------------ */

void FreeHistogram(histogramdata *histogram){
  if(histogram != NULL){
    FREEMEMORY(histogram->buckets);
  }
}

/* ------------------ get_hist_statistics ------------------------ */

void GetHistogramStats(histogramdata *histogram){
  int i, ntotal;
  float valmean, stdev, dval;

  dval = (histogram->val_max - histogram->val_min) / histogram->nbuckets;
  valmean = 0.0;
  ntotal = 0;
  for(i = 0; i < histogram->nbuckets; i++){
    float val;
    int nbucketi;

    nbucketi = histogram->buckets[i];
    if(nbucketi == 0)continue;
    val = histogram->val_min + ((float)(i)+0.5)*dval;
    valmean += nbucketi * val;
    ntotal += nbucketi;
  }
  valmean /= (float)ntotal;
  histogram->val_mean = valmean;

  stdev = 0.0;
  for(i = 0; i < histogram->nbuckets; i++){
    float valdiff;
    float nbucketi;

    nbucketi = histogram->buckets[i];
    valdiff = histogram->val_min + ((float)(i)+0.5)*dval - valmean;
    stdev += nbucketi*valdiff*valdiff;
  }
  stdev = sqrt(stdev / (float)ntotal);
  histogram->val_stdev = stdev;
}

  /* ------------------ CopyBuckets2Histogram ------------------------ */

void CopyBuckets2Histogram(int *buckets, int nbuckets, float valmin, float valmax, histogramdata *histogram){
  int i;
  float ntotal;


  FreeHistogram(histogram);
  InitHistogram(histogram, nbuckets, NULL, NULL);

  ntotal = 0.0;
  for(i = 0; i < nbuckets; i++){
    histogram->buckets[i] = buckets[i];
    ntotal += buckets[i];
  }
  histogram->ntotal = ntotal;
  histogram->val_min = valmin;
  histogram->val_max = valmax;
  histogram->defined = 1;
}

  /* ------------------ CopyU2Histogram ------------------------ */

void CopyVals2Histogram(float *vals, char *mask, float *weight, int nvals, histogramdata *histogram){

// copy vals into histogram

  int i;
  float valmin, valmax;
  float dbucket;
  int first=1;
  float nnvals=0.0;
#ifdef pp_PARTDEBUG  
  int have_inf = 0;
#endif

// initialize

  valmin=(float)pow(10.0,20.0);
  valmax=-valmin;
  histogram->defined=1;
  for(i=0;i<histogram->nbuckets;i++){
    histogram->buckets[i]=0.0;
  }

// compute min/max, skip over mask'd data

  for(i=0;i<nvals;i++){
    if(mask!=NULL&&mask[i]==0)continue;
    if(weight != NULL){
      nnvals += weight[i];
    }
    else{
      nnvals++;
    }
#ifdef pp_PARTDEBUG
    if(isinf(vals[i])){
      vals[i] = 0.0;
      have_inf = 1;
      printf("%i ", i);
    }
#endif
    if(first==1){
      valmin=vals[i];
      valmax=vals[i];
      first=0;
      continue;
    }
    valmin=MIN(vals[i],valmin);
    valmax=MAX(vals[i],valmax);
  }
#ifdef pp_PARTDEBUG
  if(have_inf==1)printf("\n\n");
#endif

// record unmasked data in histogram

  if(nnvals>0){
    dbucket=(valmax-valmin)/histogram->nbuckets;
    if(dbucket==0.0){
      if(weight != NULL){
        histogram->buckets[0] = nnvals;
      }
      else{
        histogram->buckets[0] = nnvals;
      }
      for(i = 1; i < histogram->nbuckets; i++){
        histogram->buckets[i] = 0.0;
      }
    }
    else{
      for(i=0;i<nvals;i++){
        int ival;

        if(mask!=NULL&&mask[i]==0)continue;
        ival = (vals[i]-valmin)/dbucket;
        ival = MAX(0,ival);
        ival = MIN(histogram->nbuckets-1,ival);
        if(weight != NULL){
          histogram->buckets[ival]+=weight[i];
        }
        else{
          histogram->buckets[ival]++;
        }
      }
    }
  }
  histogram->ntotal=nnvals;
  histogram->val_min=valmin;
  histogram->val_max=valmax;
}

  /* ------------------ UpdateHistogram ------------------------ */

void UpdateHistogram(float *vals, char *mask, int nvals, histogramdata *histogram_to){

// merge nvals of the floating point array, vals, into the histogram histogram

  histogramdata histogram_from;

  if(nvals<=0)return;
  InitHistogram(&histogram_from,NHIST_BUCKETS, NULL, NULL);

  CopyVals2Histogram(vals,mask,NULL,nvals,&histogram_from);
  MergeHistogram(histogram_to,&histogram_from,MERGE_BOUNDS);
  FreeHistogram(&histogram_from);
}

/* ------------------ MergeHistogram ------------------------ */

void MergeHistogram(histogramdata *histogram_to, histogramdata *histogram_from, int reset_bounds){

  // merge histogram histogram_from into histogram_to

  int i;
  float dbucket_to, dbucket_from, dbucket_new;
  float *bucket_to_copy;
  float valmin_new, valmax_new;

  histogram_to->defined=1;
  valmin_new = histogram_to->val_min;
  valmax_new = histogram_to->val_max;
  if(reset_bounds == MERGE_BOUNDS){
    valmin_new = MIN(valmin_new, histogram_from->val_min);
    valmax_new = MAX(valmax_new, histogram_from->val_max);
  }

  NewMemory((void **)&bucket_to_copy,histogram_to->nbuckets*sizeof(float));

  for(i = 0; i<histogram_to->nbuckets; i++){
    bucket_to_copy[i]=histogram_to->buckets[i];
    histogram_to->buckets[i]=0;
  }
  dbucket_to   = (histogram_to->val_max  -histogram_to->val_min  )/  histogram_to->nbuckets;
  dbucket_from = (histogram_from->val_max-histogram_from->val_min)/histogram_from->nbuckets;
  dbucket_new  = (valmax_new-valmin_new)/histogram_to->nbuckets;

  if(dbucket_new==0.0){
    histogram_to->buckets[0]=histogram_to->ntotal+histogram_from->ntotal;
    histogram_to->ntotal=histogram_to->buckets[0];
  }
  else{
    for(i=0;i<histogram_to->nbuckets;i++){
      if(bucket_to_copy[i]!=0){
        float val;
        int ival;

        val = histogram_to->val_min + ((float)i + 0.5)*dbucket_to;
        ival = (val - valmin_new) / dbucket_new;
        ival = CLAMP(ival, 0, histogram_to->nbuckets - 1);
        histogram_to->buckets[ival] += bucket_to_copy[i];
      }
    }
    for(i=0;i<histogram_from->nbuckets;i++){
      if(histogram_from->buckets[i]!=0){
        float val;
        int ival;

        val = histogram_from->val_min + ((float)i+0.5)*dbucket_from;
        ival = (val - valmin_new) / dbucket_new;
        ival = CLAMP(ival,0,histogram_to->nbuckets-1);
        histogram_to->buckets[ival]+=histogram_from->buckets[i];
      }
    }
  }
  histogram_to->val_min=valmin_new;
  histogram_to->val_max=valmax_new;
  histogram_to->ntotal+=histogram_from->ntotal;
  FREEMEMORY(bucket_to_copy);
}

/* ------------------ ResetHistogramPolar ------------------------ */

void ResetHistogramPolar(histogramdata *histogram, float *rmin, float *rmax){

  // initialize histogram data structures

  int i;

  for(i = 0; i<histogram->nbuckets; i++){
    histogram->buckets_polar[i] = 0;
  }
  histogram->defined = 0;
  histogram->ntotal = 0;
  if(rmin == NULL){
    histogram->val_rmin = (float)pow(10.0, 20.0);
  }
  else{
    histogram->val_rmin = *rmin;
  }
  if(rmax == NULL){
    histogram->val_rmax = -(float)pow(10.0, 20.0);
  }
  else{
    histogram->val_rmax = *rmax;
  }
  histogram->val_thetamin = 0.0;
  histogram->val_thetamax = 360.0;
  histogram->complete = 0;
}

/* ------------------ InitHistogramPolar ------------------------ */

void InitHistogramPolar(histogramdata *histogram, int nr, int ntheta, float *rmin, float *rmax){

// initialize histogram data structures

  int nbuckets;

  histogram->buckets = NULL;
  histogram->buckets_polar = NULL;
  histogram->nr = nr;
  histogram->ntheta = ntheta;
  nbuckets = nr*ntheta;
  histogram->ndim = 2;
  histogram->nbuckets = nbuckets;

  NewMemory((void **)&histogram->buckets_polar, histogram->nbuckets*sizeof(unsigned int));
  ResetHistogramPolar(histogram,rmin,rmax);
}

/* ------------------ Get2DBounds ------------------------ */

int Get2DBounds(float *times, float *uvals, float *vvals, int nvals, float tmin, float tmax, float *rmin, float *rmax){
  int i, first=1, count=0;

  if(nvals <= 0||rmin==NULL||rmax==NULL)return count;
  for(i = 0; i < nvals; i++){
    float r;

    if(times==NULL||(times[i]>=tmin&&times[i]<=tmax)){
      if(first==1){
        first = 0;
        *rmin = sqrt(uvals[0]*uvals[0]+vvals[0]*vvals[0]);
        *rmax = *rmin;
        count = 1;
      }
      else{
        r = sqrt(uvals[i]*uvals[i]+vvals[i]*vvals[i]);
        *rmin = MIN(*rmin, r);
        *rmax = MAX(*rmax, r);
        count++;
      }
    }
  }
  return count;
}

/* ------------------ GetPolarBounds ------------------------ */

void GetPolarBounds(float *speed, int nvals, float *rmin, float *rmax){
  int i;

  if(nvals <= 0 || rmin == NULL || rmax == NULL)return;
  *rmin = ABS(speed[0]);
  *rmax = *rmin;
  for(i = 1; i < nvals; i++){
    *rmin = MIN(*rmin, ABS(speed[i]));
    *rmax = MAX(*rmax, ABS(speed[i]));
  }
}

/* ------------------ CopyUV2Histogram ------------------------ */

void CopyUV2Histogram(float *times, float *uvals, float *vvals, int nvals, float tmin, float tmax, float rmin, float rmax, histogramdata *histogram){
  int i;
  float maxr, maxtheta;
  float sumr, sumtheta, sum;
  int ir, itheta, ixy;

  if(nvals<=0)return;

  histogram->val_min = sqrt(uvals[0]*uvals[0]+vvals[0]*vvals[0]);
  histogram->val_max = histogram->val_min;
  for(i = 0; i < nvals; i++){
    float r, theta;
    float u, v;

    if(times!=NULL&&(times[i]<tmin||times[i]>tmax))continue;
    u = uvals[i];
    v = vvals[i];
    r = sqrt(u*u + v*v);
    histogram->val_min = MIN(histogram->val_min, r);
    histogram->val_max = MAX(histogram->val_max, r);
    theta = fmod(RAD2DEG*atan2(v, u)+180.0/(float)histogram->ntheta,360.0);

    ir = 0;
    if(rmax>rmin)ir = CLAMP(histogram->nr*(r - rmin) / (rmax - rmin),0,histogram->nr-1);

    itheta = CLAMP(histogram->ntheta*(theta / 360.0),0,histogram->ntheta-1);

    ixy = ir + itheta*histogram->nr;
    histogram->buckets_polar[ixy]++;
  }

  maxr = 0.0;
  sum = 0.0;
  for(itheta = 0;itheta<histogram->ntheta;itheta++){
    sumr = 0.0;
    for(ir = 0;ir<histogram->nr;ir++){
      ixy = ir+itheta*histogram->nr;
      sumr += histogram->buckets_polar[ixy];
    }
    sum += sumr;
    maxr = MAX(sumr, maxr);
  }
  histogram->bucket_maxr = maxr;
  histogram->ntotal = sum;

  maxtheta = 0.0;
  for(ir = 0;ir<histogram->nr;ir++){
    sumtheta = 0.0;
    for(itheta = 0;itheta<histogram->ntheta;itheta++){
      ixy = ir+itheta*histogram->nr;
      sumtheta += histogram->buckets_polar[ixy];
    }
    maxtheta = MAX(sumtheta, maxtheta);
  }
  histogram->bucket_maxtheta = maxtheta;
}

/* ------------------ CopyPolar2Histogram ------------------------ */

void CopyPolar2Histogram(float *rad, float *angle, int nvals, float rmin, float rmax, histogramdata *histogram){
  int i;
  float maxr, sum, sumr, maxtheta, sumtheta;
  int itheta, ir, ixy;

  if(nvals <= 0)return;

  for(i = 0; i < nvals; i++){
    float r,theta;
    int ir, itheta, ixy;

    r = rad[i];
    theta = FMOD360(angle[i]);

    if(r<0.0){
      theta = FMOD360(theta + 180.0);
      r = -r;
    }

    ir = 0;
    if(rmax>rmin)ir = CLAMP(histogram->nr*(r - rmin) / (rmax - rmin), 0, histogram->nr - 1);

    itheta = CLAMP(histogram->ntheta*(theta / 360.0), 0, histogram->ntheta - 1);

    ixy = ir+itheta*histogram->nr;
    histogram->buckets_polar[ixy]++;
  }

  maxr = 0.0;
  sum = 0.0;
  for(itheta = 0;itheta < histogram->ntheta;itheta++){
    sumr = 0.0;
    for(ir = 0;ir < histogram->nr;ir++){
      ixy = ir + itheta*histogram->nr;
      sumr += histogram->buckets_polar[ixy];
    }
    sum += sumr;
    maxr = MAX(sumr, maxr);
  }
  histogram->bucket_maxr = maxr;
  histogram->ntotal = sum;

  maxtheta = 0.0;
  for(ir = 0;ir < histogram->nr;ir++){
    sumtheta = 0.0;
    for(itheta = 0;itheta < histogram->ntheta;itheta++){
      ixy = ir + itheta*histogram->nr;
      sumtheta += histogram->buckets_polar[ixy];
    }
    maxtheta = MAX(sumtheta, maxtheta);
  }
  histogram->bucket_maxtheta = maxtheta;

}
