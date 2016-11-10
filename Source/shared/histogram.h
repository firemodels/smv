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

#define MERGE_BOUNDS 1
#define KEEP_BOUNDS 0

/* --------------------------  flowlabels ------------------------------------ */

#define NHIST_BUCKETS 100000
typedef struct {
  int *buckets, *buckets_2d;
  int nbuckets, ndim, defined;
  int nx, ny, ntotal;
  float valmin, valmax;
  float valxmin, valxmax;
  float valymin, valymax;
  float valmean, valstdev;
  int complete;
} histogramdata;

//************************** headers ****************************************

void CompleteHistogram(histogramdata *histogram);
void CopyBuckets2Histogram(int *buckets, int nbuckets, float valmin, float valmax, histogramdata *histogram);
void CopyU2Histogram(float *vals, char *mask, int nvals, histogramdata *histogram);
void CopyPolar2Histogram(float *speed, float *angle, int nvals, float rmin, float rmax, histogramdata *histogram);
void CopyUV2Histogram(float *uvals, float *vvals, int nvals, float rmin, float rmax, histogramdata *histogram);
void FreeHistogram(histogramdata *histogram);
void FreeHistogram2d(histogramdata *histogram);
void Get2DMinMax(float *uvals, float *vvals, int nvals, float *rmin, float *rmax, int flag);
void GetPolarMinMax(float *speed, int nvals, float *rmin, float *rmax, int flag);
float GetHistogramVal(histogramdata *histogram, float cdf);
void GetHistogramStats(histogramdata *histogram);
void Histogram2Sum(histogramdata *histogram, float valmin, float valmax, int n);
void InitHistogram(histogramdata *histogram, int nbuckets, float *valmin, float *valmax);
void InitHistogram2D(histogramdata *histogram, int nx, int ny);
void MergeHistogram(histogramdata *histogramto, histogramdata *histogramfrom, int reset_bounds);
void ResetHistogram(histogramdata *histogram, float *valmin, float *valmax);
void ResetHistogram2d(histogramdata *histogram);
void UpdateHistogram(float *vals, char *mask, int nvals, histogramdata *histogram);

#endif
