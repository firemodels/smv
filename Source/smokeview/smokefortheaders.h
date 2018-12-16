#ifndef SMOKEFORTHEADERS_H_DEFINED
#define SMOKEFORTHEADERS_H_DEFINED

#ifdef WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

#define HEADER_SIZE 4
#define TRAILER_SIZE 4

#define FORTREAD(var,count,STREAM) \
                           FSEEK(STREAM,HEADER_SIZE,SEEK_CUR);\
                           returncode=fread(var,4,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           FSEEK(STREAM,TRAILER_SIZE,SEEK_CUR)

#define FORTgetsliceheader        _F(getsliceheader)
#define FORTgetslicefiledirection _F(getslicefiledirection)
#define FORTgetgeomdatasize      _F(getgeomdatasize)
#define FORTgetgeomdata          _F(getgeomdata)
#define FORTgetzonesize           _F(getzonesize)
#define FORTgetzonedata           _F(getzonedata)
#define FORTgetxyzdata            _F(getxyzdata)
#define FORTgetpatchsizes1        _F(getpatchsizes1)
#define FORTgetpatchsizes2        _F(getpatchsizes2)
#define FORTgetpatchdata          _F(getpatchdata)
#define FORTskipdata              _F(skipdata)
#define FORTgetdata1              _F(getdata1)
#define FORTgetslicesizes         _F(getslicesizes)
#define FORTwriteslicedata        _F(writeslicedata)
#define FORTwriteslicedata2       _F(writeslicedata2)
#define FORTgetslicedata          _F(getslicedata)
#define FORTgetplot3dq            _F(getplot3dq)
#define FORTgetsliceparms         _F(getsliceparms)
#define FORTcolor2rgb             _F(color2rgb)
#define FORTclosefortranfile      _F(closefortranfile)
#define FORTgetboundaryheader1    _F(getboundaryheader1)
#define FORTgetboundaryheader2    _F(getboundaryheader2)

STDCALLF FORTgetslicefiledirection(int *is1, int *is2, int *iis1, int *iis2, int *js1, int *js2, int *ks1, int *ks2, int *idir, int *joff, int *koff, int *volslice);
STDCALLF FORTgetgeomdatasize(char *filename, int *ntimes, int *nvars, int *error, FILE_SIZE lenfile);
STDCALLF FORTgetgeomdata(char *filename, int *ntimes, int *nvals, float *times, int *nstatics, int *ndynamics, float *vals, int *file_size, int *error, FILE_SIZE lenfile);
STDCALLF FORTgetboundaryheader1(char *boundaryfilename, int *unit1, int *npatch, int *error, FILE_SIZE lenfile);
STDCALLF FORTgetboundaryheader2(int *unit1, int *version, int *npatches,
                               int *pi1, int *pi2, int *pj1, int *pj2, int *pk1, int *pk2, int *patchdir);
STDCALLF FORTclosefortranfile(int *lunit);
STDCALLF FORTcolor2rgb(int *rgb, char *color, FILE_SIZE colorsize);

STDCALLF FORTgetsliceparms(char *file,
                          int *is1,int *is2,int *js1,int *js2,int *ks1, int *ks2,int *ni, int *nj, int *nk, int *slice3d, int *error,FILE_SIZE lenfile);
STDCALLF FORTgetzonesize(char *zonefilename, int *nzone_times, int *nrooms, int *nfires, int *error, FILE_SIZE len);
STDCALLF FORTgetzonedata(char *zonefilename, int *nzone_times, int *nrooms, int *nfires,
                        float *zone_times, float *zoneqfire, float *zonepr, float *zoneylay,float *zonetl,float *zonetu,
                        int *error, FILE_SIZE len);
STDCALLF FORTgetxyzdata(int *iblank,int *nx,int *ny,int *nz,int *error);
STDCALLF FORTgetpatchsizes1(int *file_unit,char *patchfilename,
                           int *npatch, int *headersize, int *error,FILE_SIZE len1);
STDCALLF FORTgetpatchsizes2(int *file_unit,int *version, int *npatch,int *npatchsize,
                           int *pi1,int *pi2,int *pj1,int *pj2,int *pk1,int *pk2, int *patchdir,
                           int *headersize, int *framesize);
STDCALLF FORTgetpatchdata(int *lunit, int *npatch,int *pi1,int *pi2,int *pj1,int *pj2,int *pk1,int *pk2,
                         float *patch_times,float *pqq, int *npqq, int *file_size, int *error);
STDCALLF FORTskipdata(int *lunit, int *size);
STDCALLF FORTgetdata1(int *file_unit, int *ipart, int *error);

STDCALLF FORTgetsliceheader(char *slicefilename, int *is1, int *is2, int *js1, int *js2, int *ks1, int *ks2, int *error, FILE_SIZE slicefilelen);
STDCALLF FORTgetslicesizes(char *slicefilename, int *nslicei, int *nslicej, int *nslicek,
                          int *nsteps,int *sliceframestep, int *error,
                          int *settime_p, int *settmax_p, float *tmin_p, float *tmax_p,
                          int *headersize, int *framesize,FILE_SIZE slicefilelen);
STDCALLF FORTwriteslicedata(char *slicefilename,
                            int *is1,int *is2,int *js1,int *js2,int *ks1,int *ks2,
                            float *qdata,float *times,int *ntimes, int *redirect,FILE_SIZE slicefilelen);
STDCALLF FORTwriteslicedata2(char *slicefilename, char *longlabel, char *shortlabel, char *unitlabel,
                          int *is1, int *is2, int *js1, int *js2, int *ks1, int *ks2,
                          float *qdata, float *times, int *ntimes, FILE_SIZE slicefilelen, FILE_SIZE longfilelen, FILE_SIZE shortfilelen, FILE_SIZE unitfilelen);
STDCALLF FORTgetslicedata(char *slicefilename,
                          int *is1,int *is2,int *js1,int *js2, int *ks1,int *ks2,
                          int *idir, float *qslicemin,float *qslicemax,
                          float *qslicedata,float *times, int *ntimes_old, int *ntimes, int *sliceframestep,
                          int *settime_p, int *settmax_p, float *tmin_p, float *tmax_p, int *file_size,
                          FILE_SIZE slicefilelen);
STDCALLF FORTgetplot3dq(char *qfilename, int *nx, int *ny, int *nz, float *qq, int *error, int *isotest, FILE_SIZE filelen);
#endif
