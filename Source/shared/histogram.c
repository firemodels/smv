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

float GetHistogramCDF(histogramdata *histogram, float val) {
  int i;
  int cutoff, sum=0;

  if(histogram->valmax <= histogram->valmin)return 0.0;

  cutoff = (val - histogram->valmin)*(float)histogram->nbuckets / (histogram->valmax - histogram->valmin);
  for (i = 0; i < cutoff; i++) {
    sum += histogram->buckets[i];
  }
  return (float)sum / (float)histogram->ntotal;
}

  /* ------------------ GetHistogramVal ------------------------ */

float GetHistogramVal(histogramdata *histogram, float cdf){

// get value of histogram for value cdf

  int cutoff, count;
  int i;
  float returnval;

  if(cdf<=0.0){
    return histogram->valmin;
  }
  if(cdf>=1.0){
    return histogram->valmax;
  }
  cutoff = cdf*histogram->ntotal;
  count=0;
  for(i=0;i<histogram->nbuckets;i++){
    count+=histogram->buckets[i];
    if(count>cutoff){
      returnval = histogram->valmin + (float)(i+0.5)*(histogram->valmax-histogram->valmin)/(float)histogram->nbuckets;
      return returnval;
    }
  }
  return histogram->valmax;
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
  if (valmin != NULL) {
    histogram->valmin = *valmin;
  }
  else {
    histogram->valmin = (float)pow(10.0, 20.0);
  }
  if (valmax != NULL) {
    histogram->valmax = *valmax;
  }
  else {
    histogram->valmax = -(float)pow(10.0, 20.0);
  }
  histogram->complete = 0;
}

/* ------------------ InitHistogram ------------------------ */

void InitHistogram(histogramdata *histogram, int nbuckets, float *valmin, float *valmax){

// initialize histogram data structures

  histogram->buckets=NULL;
  histogram->buckets_2d = NULL;
  NewMemory((void **)&histogram->buckets, nbuckets*sizeof(unsigned int));
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

  dval = (histogram->valmax - histogram->valmin) / histogram->nbuckets;
  valmean = 0.0;
  ntotal = 0;
  for(i = 0; i < histogram->nbuckets; i++){
    float val;
    int nbucketi;

    nbucketi = histogram->buckets[i];
    if(nbucketi == 0)continue;
    val = histogram->valmin + ((float)(i)+0.5)*dval;
    valmean += nbucketi * val;
    ntotal += nbucketi;
  }
  valmean /= (float)ntotal;
  histogram->valmean = valmean;
  ASSERT(histogram->ntotal == ntotal);

  stdev = 0.0;
  for(i = 0; i < histogram->nbuckets; i++){
    float valdiff;
    int nbucketi;

    nbucketi = histogram->buckets[i];
    if(nbucketi == 0)continue;
    valdiff = histogram->valmin + ((float)(i)+0.5)*dval - valmean;
    stdev += nbucketi*valdiff*valdiff;
  }
  stdev = sqrt(stdev / (float)ntotal);
  histogram->valstdev = stdev;
}

  /* ------------------ CopyBuckets2Histogram ------------------------ */

void CopyBuckets2Histogram(int *buckets, int nbuckets, float valmin, float valmax, histogramdata *histogram){
  int i, ntotal;


  FreeHistogram(histogram);
  InitHistogram(histogram, nbuckets, NULL, NULL);

  ntotal = 0;
  for(i = 0; i < nbuckets; i++){
    histogram->buckets[i] = buckets[i];
    ntotal += buckets[i];
  }
  histogram->ntotal = ntotal;
  histogram->valmin = valmin;
  histogram->valmax = valmax;
  histogram->defined = 1;
}

  /* ------------------ CopyU2Histogram ------------------------ */

void CopyU2Histogram(float *vals, char *mask, float *weight, int nvals, histogramdata *histogram){

// copy vals into histogram

  int i;
  float valmin, valmax;
  float dbucket;
  int first=1;
  int nnvals=0;

// initialize

  valmin=(float)pow(10.0,20.0);
  valmax=-valmin;
  histogram->defined=1;
  for(i=0;i<histogram->nbuckets;i++){
    histogram->buckets[i]=0;
  }

// compute min/max, skip over mask'd data

  for(i=0;i<nvals;i++){
    if(mask!=NULL&&mask[i]==0)continue;
    nnvals++;
    if(first==1){
      valmin=vals[i];
      valmax=vals[i];
      first=0;
      continue;
    }
    valmin=MIN(vals[i],valmin);
    valmax=MAX(vals[i],valmax);
  }

// record unmasked data in histogram

  if(nnvals>0){
    dbucket=(valmax-valmin)/histogram->nbuckets;
    if(dbucket==0.0){
      histogram->buckets[0]=nnvals;
    }
    else{
      for(i=0;i<nvals;i++){
        int ival;

        if(mask!=NULL&&mask[i]==0)continue;
        ival = (vals[i]-valmin)/dbucket;
        ival = MAX(0,ival);
        ival = MIN(histogram->nbuckets-1,ival);
        histogram->buckets[ival]++;
      }
    }
  }
  histogram->ntotal=nnvals;
  histogram->valmin=valmin;
  histogram->valmax=valmax;
}

/* ------------------ Histogram2Sum ------------------------ */

void Histogram2Sum(histogramdata *histogram, float valmin, float valmax, int n) {
}

  /* ------------------ UpdateHistogram ------------------------ */

void UpdateHistogram(float *vals, char *mask, int nvals, histogramdata *histogram_to){

// merge nvals of the floating point array, vals, into the histogram histogram

  histogramdata histogram_from;

  if(nvals<=0)return;
  InitHistogram(&histogram_from,NHIST_BUCKETS, NULL, NULL);

  CopyU2Histogram(vals,mask,NULL,nvals,&histogram_from);
  MergeHistogram(histogram_to,&histogram_from,MERGE_BOUNDS);
  FreeHistogram(&histogram_from);
}

/* ------------------ MergeHistogram ------------------------ */

void MergeHistogram(histogramdata *histogram_to, histogramdata *histogram_from, int reset_bounds){

  // merge histogram histogram_from into histogram_to

  int i;
  float dbucket_to, dbucket_from, dbucket_new;
  unsigned int *bucket_to_copy;
  float valmin_new, valmax_new;

  histogram_to->defined=1;
  valmin_new = histogram_to->valmin;
  valmax_new = histogram_to->valmax;
  if (reset_bounds == MERGE_BOUNDS) {
    valmin_new = MIN(valmin_new, histogram_from->valmin);
    valmax_new = MAX(valmax_new, histogram_from->valmax);
  }

  NewMemory((void **)&bucket_to_copy,histogram_to->nbuckets*sizeof(unsigned int));

  for(i = 0; i<histogram_to->nbuckets; i++){
    bucket_to_copy[i]=histogram_to->buckets[i];
    histogram_to->buckets[i]=0;
  }
  dbucket_to   = (histogram_to->valmax  -histogram_to->valmin  )/  histogram_to->nbuckets;
  dbucket_from = (histogram_from->valmax-histogram_from->valmin)/histogram_from->nbuckets;
  dbucket_new  = (valmax_new-valmin_new)/histogram_to->nbuckets;

  if(dbucket_new==0.0){
    histogram_to->buckets[0]=histogram_to->ntotal+histogram_from->ntotal;
    histogram_to->ntotal=histogram_to->buckets[0];
  }
  else{
    for(i=0;i<histogram_to->nbuckets;i++){
      if(bucket_to_copy[i]!=0) {
        float val;
        int ival;

        val = histogram_to->valmin + ((float)i + 0.5)*dbucket_to;
        ival = (val - valmin_new) / dbucket_new;
        ival = CLAMP(ival, 0, histogram_to->nbuckets - 1);
        histogram_to->buckets[ival] += bucket_to_copy[i];
      }
    }
    for(i=0;i<histogram_from->nbuckets;i++){
      if(histogram_from->buckets[i]!=0){
        float val;
        int ival;

        val = histogram_from->valmin + ((float)i+0.5)*dbucket_from;
        ival = (val - valmin_new) / dbucket_new;
        ival = CLAMP(ival,0,histogram_to->nbuckets-1);
        histogram_to->buckets[ival]+=histogram_from->buckets[i];
      }
    }
  }
  histogram_to->valmin=valmin_new;
  histogram_to->valmax=valmax_new;
  histogram_to->ntotal+=histogram_from->ntotal;
  FREEMEMORY(bucket_to_copy);
}

/* ------------------ ResetHistogram2d ------------------------ */

void ResetHistogram2d(histogramdata *histogram){

  // initialize histogram data structures

  int i;

  for(i = 0; i<histogram->nbuckets; i++){
    histogram->buckets_2d[i] = 0;
  }
  histogram->defined = 0;
  histogram->ntotal = 0;
  histogram->valxmin = (float)pow(10.0, 20.0);
  histogram->valxmax = -histogram->valxmin;
  histogram->valymin = (float)pow(10.0, 20.0);
  histogram->valymax = -histogram->valymin;
  histogram->complete = 0;
}

/* ------------------ InitHistogram2D ------------------------ */

void InitHistogram2D(histogramdata *histogram, int nx, int ny){

// initialize histogram data structures

  int nbuckets;

  histogram->buckets = NULL;
  histogram->buckets_2d = NULL;
  histogram->nx = nx;
  histogram->ny = ny;
  nbuckets = nx*ny;
  histogram->ndim = 2;
  histogram->nbuckets = nbuckets;

  NewMemory((void **)&histogram->buckets_2d, histogram->nbuckets*sizeof(unsigned int));
  ResetHistogram2d(histogram);
}

/* ------------------ FreeHistogram2d ------------------------ */

void FreeHistogram2d(histogramdata *histogram){
  FREEMEMORY(histogram->buckets_2d);
}

/* ------------------ Get2DMinMax ------------------------ */

void Get2DMinMax(float *uvals, float *vvals, int nvals, float *rmin, float *rmax, int flag){
  int i;
  float rrmin, rrmax;

  if(nvals <= 0)return;
  if(flag==HIST_USE_BOUNDS){
    rrmin = *rmin;
    rrmax = *rmax;
  }
  else{
    rrmin = sqrt(uvals[0]*uvals[0] + vvals[0]*vvals[0]);
    rrmax = rrmin;
  }
  for(i = 0; i < nvals; i++){
    float u, v, r;

    u = uvals[i];
    v = vvals[i];
    r = sqrt(u*u + v*v);
    rrmin = MIN(rrmin,r);
    rrmax = MAX(rrmax,r);
  }
  *rmin = rrmin;
  *rmax = rrmax;
}

/* ------------------ Get2DMinMax ------------------------ */

void GetPolarMinMax(float *speed, int nvals, float *rmin, float *rmax, int flag){
  int i;
  float rrmin, rrmax;

  if(nvals <= 0)return;
  if(flag == HIST_USE_BOUNDS){
    rrmin = *rmin;
    rrmax = *rmax;
  }
  else{
    rrmin = ABS(speed[0]);
    rrmax = rrmin;
  }
  for(i = 0; i < nvals; i++){
    rrmin = MIN(rrmin, ABS(speed[i]));
    rrmax = MAX(rrmax, ABS(speed[i]));
  }
  *rmin = rrmin;
  *rmax = rrmax;
}

/* ------------------ CopyUV2Histogram ------------------------ */

void CopyUV2Histogram(float *uvals, float *vvals, int nvals, float rmin, float rmax, histogramdata *histogram){
  int i;

  if(nvals<=0)return;

  for(i = 0; i < nvals; i++){
    float r, theta;
    float u, v;
    int ir, itheta, ixy;

    u = uvals[i];
    v = vvals[i];
    r = sqrt(u*u + v*v);

    ir = 0;
    if(rmax>rmin)ir = CLAMP(histogram->ny*(r - rmin) / (rmax - rmin),0,histogram->ny-1);

    theta = RAD2DEG*atan2(v, u);
    if(theta < 0.0)theta += 360.0;
    theta = fmod(theta,360.0);
    itheta = CLAMP(histogram->nx*(theta / 360.0),0,histogram->nx-1);

    ixy = itheta + ir*histogram->nx;
    histogram->buckets_2d[ixy]++;
  }
}


/* ------------------ CopyUV2Histogram ------------------------ */

void CopyPolar2Histogram(float *speed, float *angle, int nvals, float rmin, float rmax, histogramdata *histogram){
  int i;

  if(nvals <= 0)return;

  for(i = 0; i < nvals; i++){
    float vel,theta;
    int ir, itheta, ixy;

    ir = 0;
    vel = speed[i];
    theta = angle[i];
    if(vel<0.0){
      theta += 180.0;
      vel = -vel;
    }
    theta = fmod(theta, 360.0);
    if(rmax>rmin)ir = CLAMP(histogram->ny*(vel - rmin) / (rmax - rmin), 0, histogram->ny - 1);

    itheta = CLAMP(histogram->nx*(theta / 360.0), 0, histogram->nx - 1);

    ixy = itheta + ir*histogram->nx;
    histogram->buckets_2d[ixy]++;
  }
}
