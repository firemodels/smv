#ifndef SMOKEFORTHEADERS_H_DEFINED
#define SMOKEFORTHEADERS_H_DEFINED

#ifdef WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

#define FORTgetslicefiledirection getslicefiledirection
#define FORTfpoly2tri             fpoly2tri
#define FORTget_in_triangle       get_in_triangle
#define FORTget_is_angle_ge_180   get_is_angle_ge_180
#define FORTtest_in_tetra         test_in_tetra
#define FORTgetverts              getverts2
#define FORTgettetravol           get_tetrabox_volume_fb
#define FORTgeomout               geomout
#define FORTgetembeddatasize      getembeddatasize
#define FORTgetembeddata          getembeddata
#define FORTopenboundary          openboundary
#define FORTfcreate_part5sizefile fcreate_part5sizefile
#define FORTgetzonesize           getzonesize
#define FORTgetzonedata           getzonedata
#define FORTgetxyzdata            getxyzdata
#define FORTgetpatchsizes1        getpatchsizes1
#define FORTgetpatchsizes2        getpatchsizes2
#define FORTgetpatchdata          getpatchdata
#define FORTgetdata1              getdata1
#define FORTgetsizes              getsizes
#define FORTgetsizesa             getsizesa
#define FORTgetslicesizes         getslicesizes
#define FORTwriteslicedata        writeslicedata
#define FORTwriteslicedata2       writeslicedata2
#define FORTgetslicedata          getslicedata
#define FORTgetplot3dq            getplot3dq
#define FORTgetsliceparms         getsliceparms
#define FORTcolor2rgb             color2rgb
#define FORTget_file_unit         get_file_unit
#define FORTclosefortranfile      closefortranfile
#define FORTgetboundaryheader1    getboundaryheader1
#define FORTgetboundaryheader2    getboundaryheader2

STDCALLF FORTgetslicefiledirection(int *is1, int *is2, int *js1, int *js2, int *ks1, int *ks2, int *idir, int *joff, int *koff, int *volslice);
STDCALLF FORTfpoly2tri(float *verts, int *nverts, int *poly, int *npoly, int *tris, int *ntris);
STDCALLF FORTget_in_triangle(float *vert, float *v1, float *v2, float *v3, int *flag);
STDCALLF FORTget_is_angle_ge_180(float *v1, float *v2, float *v3, int *flag);
STDCALLF FORTtest_in_tetra(float *xyz, int *in_tetra, int *tetra_state);
STDCALLF FORTgettetravol(float *box_bounds,float *v0,float *v1,float *v2,float *v3,float *tetra_vol,float *areas,float *centroid);
STDCALLF FORTgetverts(float *box_bounds, float *v0, float *v1, float *v2, float *v3, float *out_verts,
                      int *nverts, int *faces, int *face_id, int *which_poly, int *nfaces, int *npolys, int *box_state);
STDCALLF FORTgeomout(float *verts, int *nverts, int *faces, int *nfaces);
STDCALLF FORTgetembeddatasize(char *filename, int *ntimes, int *nvars, int *error, FILE_SIZE lenfile);
STDCALLF FORTgetembeddata(char *filename, int *ntimes, int *nvals, float *times, int *nstatics, int *ndynamics,
                         float *vals, int *redirect, int *error, FILE_SIZE lenfile);
STDCALLF FORTgetboundaryheader1(char *boundaryfilename, int *boundaryunitnumber,
                               int *npatch, int *error, FILE_SIZE lenfile);
STDCALLF FORTgetboundaryheader2(int *boundaryunitnumber, int *version, int *npatches,
                               int *pi1, int *pi2, int *pj1, int *pj2, int *pk1, int *pk2, int *patchdir);
STDCALLF FORTopenboundary(char *boundaryfilename, int *boundaryunitnumber,
                         int *version, int *error, FILE_SIZE len);
STDCALLF FORTclosefortranfile(int *lunit);
STDCALLF FORTget_file_unit(int *funit, int *first_unit);
STDCALLF FORTcolor2rgb(int *rgb, char *color, FILE_SIZE colorsize);

STDCALLF FORTfcreate_part5sizefile(char *part5file, char *part5sizefile, int *angle_flag, int *redirect_flag, int *error,
                                  FILE_SIZE lenpart5file, FILE_SIZE lenpart5sizefile);

STDCALLF FORTgetsliceparms(char *file,
                          int *is1,int *is2,int *js1,int *js2,int *ks1, int *ks2,int *ni, int *nj, int *nk, int *slice3d, int *error,FILE_SIZE lenfile);
STDCALLF FORTgetzonesize(char *zonefilename, int *nzone_times, int *nrooms, int *nfires, int *error, FILE_SIZE len);
STDCALLF FORTgetzonedata(char *zonefilename, int *nzone_times, int *nrooms, int *nfires,
                        float *zone_times, float *zoneqfire, float *zonepr, float *zoneylay,float *zonetl,float *zonetu,
                        int *error, FILE_SIZE len);
STDCALLF FORTgetxyzdata(int *iblank,int *nx,int *ny,int *nz,int *error);
STDCALLF FORTgetpatchsizes1(int *file_unit,char *patchfilename,char *patchlonglabel,char *patchshortlabel,char *patchunit,
                           int *npatch, int *headersize, int *error,
                           FILE_SIZE len1, FILE_SIZE len2, FILE_SIZE len3, FILE_SIZE len4);
STDCALLF FORTgetpatchsizes2(int *file_unit,int *version, int *npatch,int *npatchsize,
                           int *pi1,int *pi2,int *pj1,int *pj2,int *pk1,int *pk2, int *patchdir,
                           int *headersize, int *framesize);
STDCALLF FORTgetpatchdata(int *lunit, int *npatch,int *pi1,int *pi2,int *pj1,int *pj2,int *pk1,int *pk2,
                         float *patch_times,float *pqq, int *npqq, int *error);
STDCALLF FORTgetdata1(int *file_unit, int *ipart, int *error);

STDCALLF FORTgetsizesa(char *partfilename, int *npartpoint,int *npartframes,FILE_SIZE lenfile);
STDCALLF FORTgetsizes(int *file_unit,char *partfilename,
                     int *nb, int *nv, int *nspr,int *mxframepoints, int *showstaticsmoke, int *error, FILE_SIZE filelen);
STDCALLF FORTgetslicesizes(char *slicefilename, int *nslicei, int *nslicej, int *nslicek,
                          int *nsteps,int *sliceframestep, int *error,
                          int *settime_p, int *settmax_p, float *tmin_p, float *tmax_p,
                          int *headersize, int *framesize,FILE_SIZE slicefilelen);
STDCALLF FORTwriteslicedata(int *file_unit,char *slicefilename,
                            int *is1,int *is2,int *js1,int *js2,int *ks1,int *ks2,
                            float *qdata,float *times,int *ntimes, int *redirect,FILE_SIZE slicefilelen);
STDCALLF FORTwriteslicedata2(int *file_unit, char *slicefilename, char *longlabel, char *shortlabel, char *unitlabel,
                          int *is1, int *is2, int *js1, int *js2, int *ks1, int *ks2,
                          float *qdata, float *times, int *ntimes, FILE_SIZE slicefilelen, FILE_SIZE longfilelen, FILE_SIZE shortfilelen, FILE_SIZE unitfilelen);
STDCALLF FORTgetslicedata(int *file_unit,char *slicefilename, char *shortlabels,
                          int *is1,int *is2,int *js1,int *js2, int *ks1,int *ks2,
                          int *idir, float *qslicemin,float *qslicemax,
                          float *qslicedata,float *times, int *nsteps, int *sliceframestep,
                          int *settime_p, int *settmax_p, float *tmin_p, float *tmax_p, int *redirect,
                          FILE_SIZE slicefilelen, FILE_SIZE shortlabelslen);
STDCALLF FORTgetplot3dq(char *qfilename, int *nx, int *ny, int *nz, float *qq, int *error, int *isotest, FILE_SIZE filelen);
#endif
