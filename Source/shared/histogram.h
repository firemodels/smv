#ifndef HISTOGRAM_H_DEFINED
#define HISTOGRAM_H_DEFINED
#ifdef pp_INTEL
#pragma warning (disable:2259)
#pragma warning (disable:1572)
#endif

#define HIST_USE_BOUNDS 0
#define HIST_COMPUTE_BOUNDS 1
#define HIST_OK 0
#define HIST_OLD 1
#define HIST_ERR -1

#define FMOD360(a)  ((a)>=0.0 ? fmod((a),360.0) : 360.0+fmod((a),360.0) )

#define MERGE_BOUNDS 1
#define KEEP_BOUNDS 0

/* --------------------------  flowlabels ------------------------------------ */

#define NHIST_BUCKETS 10000
typedef struct {
  unsigned int *buckets_polar;
  float *buckets, bucket_maxval, bucket_maxr, bucket_maxtheta;
  int nbuckets, ndim, defined;
  int nr, ntheta;
  float ntotal;
  float val_min, val_max;
  float val_rmin, val_rmax;
  float val_thetamin, val_thetamax;
  float val_mean, val_stdev;
  float time;
  int time_defined;
  int complete;
} histogramdata;

//************************** headers ****************************************

void CompleteHistogram(histogramdata *histogram);
void CopyBuckets2Histogram(int *buckets, int nbuckets, float valmin, float valmax, histogramdata *histogram);
void CopyPolar2Histogram(float *speed, float *angle, int nvals, float rmin, float rmax, histogramdata *histogram);
void CopyVals2Histogram(float *vals, char *mask, float *weight, int nvals, histogramdata *histogram);
void CopyUV2Histogram(float *times, float *uvals, float *vvals, int nvals, float tmin, float tmax, float rmin, float rmax, histogramdata *histogram);
void FreeHistogram(histogramdata *histogram);
void FreeHistogramPolar(histogramdata *histogram);
int Get2DBounds(float *times, float *uvals, float *vvals, int nvals, float tmin, float tmax, float *rmin, float *rmax);
void GetHistogramStats(histogramdata *histogram);
float GetHistogramCDF(histogramdata *histogram, float val);
float GetHistogramVal(histogramdata *histogram, float cdf);
void GetPolarBounds(float *speed, int nvals, float *rmin, float *rmax);
void InitHistogram(histogramdata *histogram, int nbuckets, float *valmin, float *valmax);
void InitHistogramPolar(histogramdata *histogram, int nx, int ny, float *rmin, float *rmax);
void MergeHistogram(histogramdata *histogramto, histogramdata *histogramfrom, int reset_bounds);
void ResetHistogram(histogramdata *histogram, float *valmin, float *valmax);
void ResetHistogramPolar(histogramdata *histogram, float *rmin, float *rmax);
void UpdateHistogram(float *vals, char *mask, int nvals, histogramdata *histogram);

#endif
