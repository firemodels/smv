#ifndef GETDATA_H_DEFINED
#define GETDATA_H_DEFINED
#include <stdio.h>

// TODO: remove this as a public-facing API when possible
int fortread(void *ptr, size_t size, size_t count, FILE *file);

void getzonesize(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, int *error);
void getpatchsizes1(FILE **file, const char *patchfilename, int *npatch,
                    int *headersize, int *error);
void getpatchsizes2(FILE *file, int version, int npatch, int *npatchsize,
                    int *pi1, int *pi2, int *pj1, int *pj2, int *pk1, int *pk2,
                    int *patchdir, int *headersize, int *framesize);
void getsliceparms(const char *slicefilename, int *ip1, int *ip2, int *jp1,
                   int *jp2, int *kp1, int *kp2, int *ni, int *nj, int *nk,
                   int *slice3d, int *error);
void getslicesizes(const char *slicefilename, int *nslicei, int *nslicej,
                   int *nslicek, int *nsteps, int sliceframestep, int *error,
                   int settmin_s, int settmax_s, float tmin_s, float tmax_s,
                   int *headersize, int *framesize);
FILE *openpart(const char *partfilename, int *error);
void openslice(const char *slicefilename, FILE **file, int *is1, int *is2,
               int *js1, int *js2, int *ks1, int *ks2, int *error);
void closefortranfile(FILE *unit);
void getboundaryheader1(const char *boundaryfilename, FILE **file, int *npatch,
                        int *error);
void getboundaryheader2(FILE *file, int version, int npatch, int *pi1, int *pi2,
                        int *pj1, int *pj2, int *pk1, int *pk2, int *patchdir);
FILE *openboundary(const char *boundaryfilename, int version, int *error);
void getpartheader1(FILE *file, int *nclasses, int *fdsversion, int *size);
void getpartheader2(FILE *file, int nclasses, int *nquantities, int *size);
void getpartdataframe(FILE *file, int nclasses, int *nquantities, int *npoints,
                      float *time, int *tagdata, float *pdata, int *size,
                      int *error);
void getzonedata(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, float *zonet, float *zoneqfire, float *zonepr,
                 float *zoneylay, float *zonetl, float *zonetu, int *error);
int skipdata(FILE *file, int skip);
void getpatchdata(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                  int *pj2, int *pk1, int *pk2, float *patchtime, float *pqq,
                  int *npqq, int *file_size, int *error);
void getdata1(FILE *file, int *ipart, int *error) ;
void getslicefiledirection(int *is1, int *is2, int *iis1, int *iis2, int *js1,
                           int *js2, int *ks1, int *ks2, int *idir, int *joff,
                           int *koff, int *volslice);
void writeslicedata(const char *slicefilename, int is1, int is2, int js1,
                    int js2, int ks1, int ks2, float *qdata, float *times,
                    int ntimes, int redirect_flag);
void writeslicedata2(const char *slicefilename, const char *longlabel,
                     const char *shortlabel, const char *unitlabel, int is1,
                     int is2, int js1, int js2, int ks1, int ks2, float *qdata,
                     float *times, int ntimes);
void getsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, float *time, float *qframe, int testslice,
                   int *error);
void outsliceheader(const char *slicefilename, FILE **file, int ip1, int ip2,
                    int jp1, int jp2, int kp1, int kp2, int *error);
void outsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, float time, float *qframe, int *error);
void outboundaryheader(const char *boundaryfilename, FILE **file, int npatches,
                       int *pi1, int *pi2, int *pj1, int *pj2, int *pk1,
                       int *pk2, int *patchdir, int *error);
void outpatchframe(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                   int *pj2, int *pk1, int *pk2, float patchtime, float *pqq,
                   int *error);
void getplot3dq(const char *qfilename, int nx, int ny, int nz, float *qq,
                int *error, int isotest);
void plot3dout(const char *outfile, int nx, int ny, int nz, float *qout,
               int *error3);
// Translate character string of a color name to RGB value
void color2rgb(int rgb[3], const char *color);
#endif
