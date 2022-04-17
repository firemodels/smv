#include "options.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <share.h>
#endif
#include "MALLOCC.h"
#include "datadefs.h"
#include "getdata.h"

#ifdef WIN32
#define FOPEN(file, mode) _fsopen(file, mode, _SH_DENYNO)
#else
#define FOPEN(file, mode) fopen(file, mode)
#endif

//  ------------------ ffseek ------------------------
void ffseek(FILE *file, int *sizes, int nsizes, int mode, int *error) {
  long size = 0;
  for (int i = 0; i < nsizes; i++) {
    size += 4 + sizes[i] + 4;
  }
  *error = fseek(file, size, mode);
}

//  ------------------ getgeomdatasize ------------------------

void getgeomdatasize(const char *filename, int *ntimes, int *nvars, int *error) {
  FILE *file;
  double time, dummy;
  int i, one, version;
  int nvert_s, nvert_d, nface_s, nface_d;
  file = FOPEN(filename, "rb");
  if (file == NULL) {
    PRINTF(" The boundary element file name, %s does not exist\n", filename);
    *error = 1;
    return;
  }

  error = 0;
  fread(&one, 1, 1, file);
  fread(&version, 1, 1, file);
  ntimes = 0;
  nvars = 0;
  size_t nread;
  while (1) {
    nread = fread(&time, sizeof(double), 1, file);
    if (nread == 0) {
      nread = fread(&nvert_s, sizeof(nvert_s), 1, file);
      nread = fread(&nface_s, sizeof(nface_s), 1, file);
      nread = fread(&nvert_d, sizeof(nvert_d), 1, file);
      nread = fread(&nface_d, sizeof(nface_d), 1, file);
    }
    if (nread != 0 && nvert_s > 0)
      nread = fread(&dummy, sizeof(double), nvert_s, file);
    if (nread != 0 && nvert_d > 0)
      nread = fread(&dummy, sizeof(double), nvert_d, file);
    if (nread != 0 && nface_s > 0)
      nread = fread(&dummy, sizeof(double), nface_s, file);
    if (nread != 0 && nface_d > 0)
      nread = fread(&dummy, sizeof(double), nface_d, file);
    if (nread == 0) {
      break;
    }
    nvars = nvars + nvert_s + nvert_d + nface_s + nface_d;
    ntimes = ntimes + 1;
  }
  fclose(file);
}

//  ------------------ getzonesize ------------------------
void getzonesize(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, int *error) {
  int version;
  int i;
  double dummy, dummy2;
  int exit_all;

  error = 0;

  FILE *file;
  file = FOPEN(zonefilename, "rb");

  if (file == NULL) {
    PRINTF(" The zone file name, %s does not exist\n", zonefilename);
    *error = 1;
    return;
  }

  nzonet = 0;
  fread(&version, sizeof(version), 1, file);
  if (error == 0)
    fread(&nrooms, sizeof(nrooms), 1, file);
  if (error == 0)
    fread(&nfires, sizeof(nfires), 1, file);
  if (error != 0) {
    error = 0;
    rewind(file);
    return;
  }
  while (1) {
    exit_all = 0;
    fread(&dummy, sizeof(dummy), 1, file);
    if (error != 0) {
      error = 0;
      break;
    }
    for (int i = 0; i < *nrooms; i++) {
      fread(&dummy, sizeof(dummy), 1, file);
      fread(&dummy, sizeof(dummy), 1, file);
      fread(&dummy, sizeof(dummy), 1, file);
      fread(&dummy, sizeof(dummy), 1, file);
      if (error == 0)
        continue;
      error = 0;
      exit_all = 1;
      break;
    }
    if (exit_all == 1)
      break;
    for (int i = 0; i < *nfires; i++) {
      fread(&dummy, sizeof(dummy), 1, file);
      fread(&dummy2, sizeof(dummy2), 1, file);
      if (error == 0)
        continue;
      error = 0;
      exit_all = 1;
      break;
    }
    if (exit_all == 1)
      break;
    nzonet = nzonet + 1;
  }
  fclose(file);
}

// !  ------------------ getpatchsizes1 ------------------------

void getpatchsizes1(FILE *file, const char *patchfilename, int *npatch,
                    int *headersize, int *error) {
  int sizes[3], nsizes;

  error = 0;
  file = FOPEN(patchfilename, "rb");
  if (file == NULL) {
    PRINTF(" The boundary file name, %s does not exist\n", patchfilename);
    *error = 1;
    return;
  }

  sizes[0] = 30;
  sizes[1] = 30;
  sizes[2] = 30;
  nsizes = 3;
  ffseek(file, sizes, nsizes, SEEK_SET,
         error); // skip over long, short and unit labels (each 30 characters in
                 // length);
  if (error == 0)
    fread(&npatch, sizeof(npatch), 1, file);
  *headersize = 3 * (4 + 30 + 4) + 4 + 4 + 4;

  return;
}

// !  ------------------ getpatchsizes2 ------------------------

void getpatchsizes2(FILE *file, int version, int npatch, int *npatchsize,
                    int *pi1, int *pi2, int *pj1, int *pj2, int *pk1, int *pk2,
                    int *patchdir, int *headersize, int *framesize) {
  int i1, i2, j1, j2, k1, k2;

  int error = 0;

  *npatchsize = 0;
  for (int n = 0; n < npatch; n++) {
    if (version == 0) {
      fread(&i1, sizeof(i1), 1, file);
      fread(&i2, sizeof(i2), 1, file);
      fread(&j1, sizeof(j1), 1, file);
      fread(&j2, sizeof(j2), 1, file);
      fread(&k1, sizeof(k1), 1, file);
      fread(&k2, sizeof(k2), 1, file);
    } else {
      fread(&i1, sizeof(i1), 1, file);
      fread(&i2, sizeof(i2), 1, file);
      fread(&j1, sizeof(j1), 1, file);
      fread(&j2, sizeof(j2), 1, file);
      fread(&k1, sizeof(k1), 1, file);
      fread(&k2, sizeof(k2), 1, file);
      fread(&patchdir[n], sizeof(patchdir[n]), 1, file);
    }
    pi1[n] = i1;
    pi2[n] = i2;
    pj1[n] = j1;
    pj2[n] = j2;
    pk1[n] = k1;
    pk2[n] = k2;
    npatchsize = npatchsize + (i2 + 1 - i1) * (j2 + 1 - j1) * (k2 + 1 - k1);
  }
  headersize = headersize + npatch * (4 + 6 * 4 + 4);
  if (version == 1) {
    headersize = headersize + npatch * 4;
  }
  *framesize = 8 + 4 + 8 * npatch + (*npatchsize) * 4;

  return;
}

// !  ------------------ getsliceparms ------------------------

void getsliceparms(const char *slicefilename, int *ip1, int *ip2, int *jp1,
                   int *jp2, int *kp1, int *kp2, int *ni, int *nj, int *nk,
                   int *slice3d, int *error) {
  FILE *file;
  int idir, joff, koff, volslice;
  char longlbl[31];
  char shortlbl[31];
  char unitlbl[31];
  int iip1, iip2;

  if (*ip1 == -1 || *ip2 == -1 || *jp1 == -1 || *jp2 == -1 || *kp1 == -1 ||
      *kp2 == -1) {
    *ip1 = 0;
    *ip2 = 0;
    *jp1 = 0;
    *jp2 = 0;
    *kp1 = 0;
    *kp2 = 0;
    *error = 0;

    file = FOPEN(slicefilename, "rb");
    if (file == NULL) {
      *error = 1;
      return;
    }
    memset(longlbl, '\0', sizeof(longlbl));
    fread(longlbl, 30, 1, file);
    memset(shortlbl, '\0', sizeof(shortlbl));
    fread(shortlbl, 30, 1, file);
    memset(unitlbl, '\0', sizeof(unitlbl));
    fread(unitlbl, 30, 1, file);

    fread(ip1, sizeof(ip1), 1, file);
    fread(ip2, sizeof(ip2), 1, file);
    fread(jp1, sizeof(jp1), 1, file);
    fread(jp2, sizeof(jp2), 1, file);
    fread(kp1, sizeof(kp1), 1, file);
    fread(kp2, sizeof(kp2), 1, file);
    fclose(file);
  }

  *ni = *ip2 + 1 - *ip1;
  *nj = *jp2 + 1 - *jp1;
  *nk = *kp2 + 1 - *kp1;
  if (ip1 == ip2 || jp1 == jp2 || kp1 == kp2) {
    *slice3d = 0;
  } else {
    *slice3d = 1;
  }
  getslicefiledirection(ip1, ip2, &iip1, &iip2, jp1, jp2, kp1, kp2, &idir,
                        &joff, &koff, &volslice);

  return;
}

// !  ------------------ getsliceheader ------------------------

void getsliceheader(const char *slicefilename, int *ip1, int *ip2, int *jp1,
                    int *jp2, int *kp1, int *kp2, int *error) {
  FILE *file;
  int nsizes, sizes[3];

  error = 0;
  file = FOPEN(slicefilename, "rb");
  if (file == NULL) {
    *error = 1;
    return;
  }
  sizes[0] = 30;
  sizes[1] = 30;
  sizes[2] = 30;
  nsizes = 3;

  ffseek(file, sizes, nsizes, SEEK_SET, error);

  fread(ip1, sizeof(ip1), 1, file);
  fread(ip2, sizeof(ip2), 1, file);
  fread(jp1, sizeof(jp1), 1, file);
  fread(jp2, sizeof(jp2), 1, file);
  fread(kp1, sizeof(kp1), 1, file);
  fread(kp2, sizeof(kp2), 1, file);

  fclose(file);
}

// !  ------------------ getslicesizes ------------------------

void getslicesizes(const char *slicefilename, int *nslicei, int *nslicej,
                   int *nslicek, int *nsteps, int sliceframestep, int *error,
                   int settmin_s, int settmax_s, double tmin_s, double tmax_s,
                   int *headersize, int *framesize) {
  FILE *file;

  int ip1, ip2, jp1, jp2, kp1, kp2;
  int iip1, iip2;
  int nxsp, nysp, nzsp;

  double timeval, time_max;
  bool load; // boolean
  int idir, joff, koff, volslice;
  int count;
  int sizes[3], nsizes;

  *error = 0;
  nsteps = 0;

  file = FOPEN(slicefilename, "rb");
  if (file == NULL) {
    *error = 1;
    return;
  }

  sizes[0] = 30;
  sizes[1] = 30;
  sizes[2] = 30;
  nsizes = 3;
  *headersize = 3 * (4 + 30 + 4);

  ffseek(file, sizes, nsizes, SEEK_SET, error);

  fread(&ip1, sizeof(ip1), 1, file);
  fread(&ip2, sizeof(ip2), 1, file);
  fread(&jp1, sizeof(jp1), 1, file);
  fread(&jp2, sizeof(jp2), 1, file);
  fread(&kp1, sizeof(kp1), 1, file);
  fread(&kp2, sizeof(kp2), 1, file);

  headersize = headersize + 4 + 6 * 4 + 4;
  if (error != 0)
    return;

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  getslicefiledirection(&ip1, &ip2, &iip1, &iip2, &jp1, &jp2, &kp1, &kp2, &idir, &joff,
                        &koff, &volslice);
  *nslicei = nxsp;
  *nslicej = nysp + joff;
  *nslicek = nzsp + koff;

  *framesize = 4 * (1 + nxsp * nysp * nzsp) + 16;

  count = -1;
  time_max = -1000000.0;
  sizes[0] = 4 * nxsp * nysp * nzsp;
  nsizes = 1;
  while (1) {
    fread(&timeval, sizeof(timeval), 1, file);
    if (error != 0)
      break;
    if ((settmin_s != 0 && timeval < tmin_s) || timeval <= time_max) {
      load = false;
    } else {
      load = true;
      time_max = timeval;
    }
    if (settmax_s != 0 && timeval > tmax_s)
      break;
    ffseek(file, sizes, nsizes, SEEK_CUR, error);
    count = count + 1;
    if ((count % sliceframestep) != 0)
      load = false;
    if (error != 0)
      break;
    if (load)
      nsteps = nsteps + 1;
  }
  error = 0;
  fclose(file);
  return;
}

// !  ------------------ openpart ------------------------

FILE *openpart(const char *partfilename, int *fileunit, int *error) {
  FILE *file;
  *error = 0;
  file = FOPEN(partfilename, "rb");
  if (file == NULL) {
    *error = 1;
  }
  return file;
}

// !  ------------------ openslice ------------------------

void openslice(const char *slicefilename, int *unitnum, int *is1, int *is2,
               int *js1, int *js2, int *ks1, int *ks2, int *error) {
  FILE *file;
  char longlbl[31];
  char shortlbl[31];
  char unitlbl[31];
  *error = 0;
  file = FOPEN(slicefilename, "rb");
  if (file == NULL) {
    *error = 1;
    return;
  }
  memset(longlbl, '\0', sizeof(longlbl));
  fread(longlbl, 30, 1, file);
  memset(shortlbl, '\0', sizeof(shortlbl));
  fread(shortlbl, 30, 1, file);
  memset(unitlbl, '\0', sizeof(unitlbl));
  fread(unitlbl, 30, 1, file);

  fread(is1, sizeof(is1), 1, file);
  fread(is2, sizeof(is2), 1, file);
  fread(js1, sizeof(js1), 1, file);
  fread(js2, sizeof(js2), 1, file);
  fread(ks1, sizeof(ks1), 1, file);
  fread(ks2, sizeof(ks2), 1, file);

  return;
}

// !  ------------------ closefortranfile ------------------------

void closefortranfile(FILE *unit) {
  fclose(unit);
  return;
}

// !  ------------------ getboundaryheader1 ------------------------

void getboundaryheader1(const char *boundaryfilename, FILE *file, int *npatch,
                        int *error) {
  char patchlonglabel[31];
  char patchshortlabel[31];
  char patchunit[31];

  *error = 0;
  file = FOPEN(boundaryfilename, "rb");
  if (file == NULL) {
    PRINTF(" The boundary file name, %s does not exist\n", boundaryfilename);
    *error = 1;
    return;
  }

  if (*error == 0) {
    memset(patchlonglabel, '\0', sizeof(patchlonglabel));
    fread(patchlonglabel, 30, 1, file);
  }
  if (*error == 0) {
    memset(patchshortlabel, '\0', sizeof(patchshortlabel));
    fread(patchshortlabel, 30, 1, file);
  }
  if (*error == 0) {
    memset(patchunit, '\0', sizeof(patchunit));
    fread(patchunit, 30, 1, file);
  }
  if (*error == 0) {
    fread(npatch, sizeof(*npatch), 1, file);
  }
  if (*error != 0) {
    fclose(file);
  }

  return;
}

// !  ------------------ getboundaryheader2 ------------------------

void getboundaryheader2(FILE *file, int version, int npatch, int *pi1, int *pi2,
                        int *pj1, int *pj2, int *pk1, int *pk2, int *patchdir) {
  int i1, i2, j1, j2, k1, k2;

  for (int n = 0; n < npatch; n++) {
    if (version == 0) {
      fread(&i1, sizeof(i1), 1, file);
      fread(&i2, sizeof(i2), 1, file);
      fread(&j1, sizeof(j1), 1, file);
      fread(&j2, sizeof(j2), 1, file);
      fread(&k1, sizeof(k1), 1, file);
      fread(&k2, sizeof(k2), 1, file);
    } else {
      fread(&i1, sizeof(i1), 1, file);
      fread(&i2, sizeof(i2), 1, file);
      fread(&j1, sizeof(j1), 1, file);
      fread(&j2, sizeof(j2), 1, file);
      fread(&k1, sizeof(k1), 1, file);
      fread(&k2, sizeof(k2), 1, file);
      fread(&patchdir[n], sizeof(patchdir[n]), 1, file);
    }
    pi1[n] = i1;
    pi2[n] = i2;
    pj1[n] = j1;
    pj2[n] = j2;
    pk1[n] = k1;
    pk2[n] = k2;
  }

  return;
}

// !  ------------------ openboundary ------------------------

FILE *openboundary(const char *boundaryfilename, int version, int *error) {
  FILE *file;
  char patchlonglabel[31];
  char patchshortlabel[31];
  char patchunit[31];

  int npatch, n;
  int i1, i2, j1, j2, k1, k2, patchdir;
  file = FOPEN(boundaryfilename, "rb");
  if (file == NULL) {
    PRINTF(" The boundary file name, %s does not exist\n", boundaryfilename);
    *error = 1;
    return file;
  }

  memset(patchlonglabel, '\0', sizeof(patchlonglabel));
  fread(patchlonglabel, 30, 1, file);
  if (error == 0) {
    memset(patchshortlabel, '\0', sizeof(patchshortlabel));
    fread(patchshortlabel, 30, 1, file);
  }
  if (error == 0) {
    memset(patchunit, '\0', sizeof(patchunit));
    fread(patchunit, 30, 1, file);
  }
  if (error == 0) {
    fread(&npatch, sizeof(npatch), 1, file);
  }

  for (int n = 0; n < npatch; n++) {
    if (version == 0) {
      if (error == 0) {
        fread(&i1, sizeof(i1), 1, file);
        fread(&i2, sizeof(i2), 1, file);
        fread(&j1, sizeof(j1), 1, file);
        fread(&j2, sizeof(j2), 1, file);
        fread(&k1, sizeof(k1), 1, file);
        fread(&k2, sizeof(k2), 1, file);
      }
    } else {
      {
        if (error == 0)
          fread(&i1, sizeof(i1), 1, file);
        fread(&i2, sizeof(i2), 1, file);
        fread(&j1, sizeof(j1), 1, file);
        fread(&j2, sizeof(j2), 1, file);
        fread(&k1, sizeof(k1), 1, file);
        fread(&k2, sizeof(k2), 1, file);
        fread(&patchdir, sizeof(patchdir), 1, file);
      }
    }
  }

  if (error != 0)
    fclose(file);

  return file;
}

// !  ------------------ getpartheader1 ------------------------

void getpartheader1(FILE *file, int *nclasses, int *fdsversion, int *size) {
  int one;

  fread(&one, sizeof(one), 1, file);
  fread(fdsversion, sizeof(*fdsversion), 1, file);
  fread(nclasses, sizeof(*nclasses), 1, file);

  *size = 12;

  return;
}

// !  ------------------ getpartheader2 ------------------------

void getpartheader2(FILE *file, int nclasses, int *nquantities, int *size) {
  char clabel[31];
  int dummy;

  size = 0;

  for (int i = 0; i < nclasses; i++) {
    fread(&nquantities[i], sizeof(nquantities[i]), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    size = size + 4 + 2 * nquantities[i] * (4 + 30 + 4);
    for (int j = 0; j < nquantities[i]; j++) {
      memset(clabel, '\0', sizeof(clabel));
      fread(clabel, 30, 1, file);
      memset(clabel, '\0', sizeof(clabel));
      fread(clabel, 30, 1, file);
    }
  }

  return;
}

// !  ------------------ getpartdataframe ------------------------

void getpartdataframe(FILE *file, int nclasses, int *nquantities, int *npoints,
                      double *time, int *tagdata, double *pdata, int *size,
                      int *error) {
  int pstart, pend;
  int tagstart, tagend;
  int i, j, nparticles;

  *size = 0;
  pend = 0;
  tagend = 0;
  *error = 0;
  fread(time, sizeof(*time), 1, file);
  *size = 4;
  if (error != 0)
    return;
  for (int i = 0; i < nclasses; i++) {
    fread(&nparticles, sizeof(nparticles), 1, file);
    if (error != 0)
      return;
    npoints[i] = nparticles;

    pstart = pend + 1;
    pend = pstart + 3 * nparticles - 1;
    fread(&pdata[pstart], sizeof(pdata[j]), pend - pstart, file);
    if (error != 0)
      return;

    tagstart = tagend + 1;
    tagend = tagstart + nparticles - 1;
    fread(&tagdata[tagstart], sizeof(tagdata[j]), tagend - tagstart, file);
    if (error != 0)
      return;

    if (nquantities[i] > 0) {
      pstart = pend + 1;
      pend = pstart + nparticles * nquantities[i] - 1;
      fread(&pdata[pstart], sizeof(pdata[j]), pend - pstart, file);
      if (error != 0)
        return;
    }
    size = size + 4 + (4 * 3 * nparticles) + 4 * nparticles +
           4 * nparticles * nquantities[i];
  }
  error = 0;
  return;
}

// !  ------------------ getgeomdata ------------------------

void getgeomdata(const char *filename, int ntimes, int nvals, double *times,
                 int *nstatics, int *ndynamics, double *vals, int *file_size,
                 int *error) {
  FILE *file;
  size_t nread;
  int one, nvars;
  int nvert_s, ntri_s, nvert_d, ntri_d;
  int version;

  *file_size = 0;
  file = FOPEN(filename, "rb");
  if (file == NULL) {
    PRINTF(" The boundary element file name, %s does not exist\n", filename);
    *error = 1;
    return;
  }

  *error = 0;
  nread = fread(&one, 1, 1, file);
  nread = fread(&version, sizeof(version), 1, file);
  *file_size = 2 * (4 + 4 + 4);
  nvars = 0;
  for (int itime = 0; itime < ntimes; itime++) {

    nread = fread(&times[itime], sizeof(times[itime]), 1, file);
    file_size = file_size + (4 + 4 + 4);
    if (nread != 0) {
      nread = fread(&nvert_s, sizeof(nvert_s), 1, file);
      nread = fread(&ntri_s, sizeof(ntri_s), 1, file);
      nread = fread(&nvert_d, sizeof(nvert_d), 1, file);
      nread = fread(&ntri_d, sizeof(ntri_d), 1, file);
      file_size = file_size + (4 + 4 * 4 + 4);
      nstatics[itime] = nvert_s + ntri_s;
    }

    if (nread != 0) {
      if (nvert_s > 0) {
        nread = fread(&vals[nvars], sizeof(vals[nvars]), nvert_s, file);
        file_size = file_size + (4 + 4 * nvert_s + 4);
      }
      nvars = nvars + nvert_s;
    }

    if (nread != 0) {
      if (ntri_s > 0) {
        nread = fread(&vals[nvars], sizeof(vals[nvars]), ntri_s, file);
        file_size += (4 + 4 * ntri_s + 4);
      }
      nvars = nvars + ntri_s;
    }

    ndynamics[itime] = nvert_d + ntri_d;
    if (nread != 0) {
      if (nvert_d > 0) {
        nread = fread(&vals[nvars], sizeof(vals[nvars]), nvert_d, file);
        file_size += (4 + 4 * nvert_d + 4);
      }
      nvars = nvars + nvert_d;
    }

    if (nread != 0) {
      if (ntri_d > 0) {
        nread = fread(&vals[nvars], sizeof(vals[nvars]), ntri_d, file);
        file_size += (4 + 4 * ntri_d + 4);
      }
      nvars = nvars + ntri_d;
    }

    if (nread == 0) {
      fclose(file);
      return;
    }
  }
  fclose(file);
  return;
}

// !  ------------------ getzonedata ------------------------

void getzonedata(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, double *zonet, double *zoneqfire, double *zonepr,
                 double *zoneylay, double *zonetl, double *zonetu, int *error) {
  FILE *file;
  int ii, ii2, idummy, version;
  double dummy, qdot;

  file = FOPEN(zonefilename, "rb");
  if (file == NULL) {
    PRINTF(" The zone file name, %s does not exist\n", zonefilename);
    *error = 1;
    return;
  }
  fread(&version, sizeof(version), 1, file);
  fread(&idummy, sizeof(idummy), 1, file);
  fread(&idummy, sizeof(idummy), 1, file);
  ii = 0;
  ii2 = 0;
  for (int j = 0; j < *nzonet; j++) {
    fread(&zonet[j], sizeof(zonet[j]), 1, file);
    for (int i = 0; i < *nrooms; i++) {
      fread(&zonepr[ii], sizeof(zonepr[ii]), 1, file);
      fread(&zoneylay[ii], sizeof(zoneylay[ii]), 1, file);
      fread(&zonetl[ii], sizeof(zonetl[ii]), 1, file);
      fread(&zonetu[ii], sizeof(zonetu[ii]), 1, file);
      ii++;
      if (error != 0) {
        *error = 1;
        *nzonet = j;
        fclose(file);
        return;
      }
    }
    for (int i = 1; i <= *nfires; i++) {
      ii2 = ii2 + 1;
      fread(&dummy, sizeof(dummy), 1, file);
      fread(&qdot, sizeof(qdot), 1, file);
      zoneqfire[ii2] = qdot;
      if (error != 0) {
        *error = 1;
        *nzonet = j - 1;
        fclose(file);
        return;
      }
    }
  }

  fclose(file);
  return;
}

// !  ------------------ skipdata ------------------------

void skipdata(FILE *file, int skip) {
  int error, sizes[1], nsizes;
  sizes[0] = skip;
  nsizes = 1l;
  ffseek(file, sizes, nsizes, SEEK_CUR, &error);
  return;
}

// !  ------------------ getpatchdata ------------------------

void getpatchdata(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                  int *pj2, int *pk1, int *pk2, double *patchtime, double *pqq,
                  int *npqq, int *file_size, int *error) {
  int i1, i2, j1, j2, k1, k2, size, ibeg, iend, ii;
  file_size = 0;
  error = 0;
  fread(&patchtime, sizeof(patchtime), 1, file);
  file_size = file_size + 4;
  if (error != 0) {
    fclose(file);
    return;
  }
  ibeg = 1;
  npqq = 0;
  for (int i = 0; i < npatch; i++) {
    i1 = pi1[i];
    i2 = pi2[i];
    j1 = pj1[i];
    j2 = pj2[i];
    k1 = pk1[i];
    k2 = pk2[i];
    size = (i2 + 1 - i1) * (j2 + 1 - j1) * (k2 + 1 - k1);
    npqq = npqq + size;
    iend = ibeg + size - 1;
    fread(&pqq[ibeg], sizeof(pqq[0]), iend - ibeg, file);
    file_size = file_size + 4 * (iend + 1 - ibeg);
    if (error != 0) {
      fclose(file);
      break;
    }
    ibeg = iend + 1;
  }
  return;
}

// !  ------------------ getdata1 ------------------------

void getdata1(FILE *file, int *ipart, int *error) {
  double sarx, sary, swpar;
  int i, j, k;
  int ndum2;
  int nspr, nv;
  int ibar, jbar, kbar;
  double dummy;
  int nb1, idummy;
  *error = 0;

  fread(&sarx, sizeof(sarx), 1, file);
  fread(&sary, sizeof(sary), 1, file);
  fread(&swpar, sizeof(swpar), 1, file);
  fread(&ipart, sizeof(ipart), 1, file);
  fread(&ndum2, sizeof(ndum2), 1, file);

  if (error != 0)
    return;
  fread(&ibar, sizeof(ibar), 1, file);
  fread(&jbar, sizeof(jbar), 1, file);
  fread(&kbar, sizeof(kbar), 1, file);

  if (error != 0)
    return;

  // TODO: use seek here
  for (int i = 0; i < ibar; i++) {
    fread(&dummy, sizeof(dummy), 1, file);
  }
  for (int j = 0; j < jbar; j++) {
    fread(&dummy, sizeof(dummy), 1, file);
  }
  for (int k = 0; k < kbar; k++) {
    fread(&dummy, sizeof(dummy), 1, file);
  }
  if (error != 0)
    return;

  fread(&nb1, sizeof(nb1), 1, file);
  if (error != 0)
    return;

  for (int i = 1; i <= nb1; i++) {
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    if (error != 0)
      return;
  }

  fread(&nv, sizeof(nv), 1, file);
  if (error != 0)
    return;

  for (int i = 1; i <= nv; i++) {
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    if (error != 0)
      return;
  }

  fread(&nspr, sizeof(nspr), 1, file);
  if (error != 0)
    return;

  for (int i = 1; i <= nspr; i++) {
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    fread(&dummy, sizeof(dummy), 1, file);
    if (error != 0)
      return;
  }

  return;
}

// !  ------------------ getslicefiledirection ------------------------
void getslicefiledirection(int *is1, int *is2, int *iis1, int *iis2, int *js1,
                           int *js2, int *ks1, int *ks2, int *idir, int *joff,
                           int *koff, int *volslice) {
  int nxsp, nysp, nzsp;

  nxsp = *is2 + 1 - *is1;
  nysp = *js2 + 1 - *js1;
  nzsp = *ks2 + 1 - *ks1;
  joff = 0;
  koff = 0;
  *volslice = 0;
  *iis1 = *is1;
  *iis2 = *is2;
  if (*is1 != *is2 && *js1 != *js2 && *ks1 != *ks2) {
    *idir = 1;
    *is2 = *is1;
    *volslice = 1;
    return;
  }
  int imin = MIN(MIN(nxsp, nysp), nzsp);
  if (nxsp == imin) {
    *idir = 1;
    *is2 = *is1;
  } else if (nysp == imin) {
    *idir = 2;
    *js2 = *js1;
  } else {
    *idir = 3;
    *ks2 = *ks1;
  }
  if (*is1 == *is2 && *js1 == *js2) {
    *idir = 1;
    *joff = 1;
  } else if (*is1 == *is2 && *ks1 == *ks2) {
    *idir = 1;
    *koff = 1;
  } else if (*js1 == *js2 && *ks1 == *ks2) {
    *idir = 2;
    *koff = 1;
  }
  return;
}

// !  ------------------ writeslicedata ------------------------

void writeslicedata(const char *slicefilename, int is1, int is2, int js1,
                    int js2, int ks1, int ks2, double *qdata, double *times,
                    int ntimes, int redirect_flag) {
  int error;
  char longlbl[31];
  char shortlbl[31];
  char unitlbl[31];
  int ibeg, iend, nframe;
  int nxsp, nysp, nzsp;
  int i, ii;

  FILE *file;
  file = FOPEN(slicefilename, "rb");

  memset(longlbl, '\0', sizeof(longlbl));
  strncpy(longlbl, " ", 30);
  memset(shortlbl, '\0', sizeof(shortlbl));
  strncpy(shortlbl, " ", 30);
  memset(unitlbl, '\0', sizeof(unitlbl));
  strncpy(unitlbl, " ", 30);

  fwrite(longlbl, 30, 1, file);
  fwrite(shortlbl, 30, 1, file);
  fwrite(unitlbl, 30, 1, file);

  fwrite(&is1, sizeof(is1), 1, file);
  fwrite(&is2, sizeof(is2), 1, file);
  fwrite(&js1, sizeof(js1), 1, file);
  fwrite(&js2, sizeof(js2), 1, file);
  fwrite(&ks1, sizeof(ks1), 1, file);
  fwrite(&ks2, sizeof(ks2), 1, file);

  nxsp = is2 + 1 - is1;
  nysp = js2 + 1 - js1;
  nzsp = ks2 + 1 - ks1;
  nframe = nxsp * nysp * nzsp;
  if (redirect_flag == 0)
    PRINTF("output slice data to %s\n", slicefilename);
  for (int i = 0; i < ntimes; i++) {
    fwrite(&times[i], sizeof(times[i]), 1, file);
    ibeg = 1 + (i)*nframe;
    iend = (i + 1) * nframe;
    fwrite(&qdata[ibeg], sizeof(qdata[ibeg]), iend - ibeg, file);
  }

  fclose(file);

  return;
}

// !  ------------------ writeslicedata2 ------------------------

void writeslicedata2(const char *slicefilename, const char *longlabel,
                     const char *shortlabel, const char *unitlabel, int is1,
                     int is2, int js1, int js2, int ks1, int ks2, double *qdata,
                     double *times, int ntimes) {
  int error;
  char longlabel30[31];
  char shortlabel30[31];
  char unitlabel30[31];
  int ibeg, iend, nframe;
  int nxsp, nysp, nzsp;
  int i, ii;

  FILE *file = FOPEN(slicefilename, "rb");

  // Copy labels into smaller buffers, truncating if necessary. The separate
  // TrimFront and TrimBack steps are intentionally separate steps.
  strncpy(longlabel30, TrimFront(longlabel), 30);
  TrimBack(longlabel30);
  strncpy(shortlabel30, TrimFront(shortlabel), 30);
  TrimBack(shortlabel30);
  strncpy(unitlabel30, TrimFront(unitlabel), 30);
  TrimBack(unitlabel30);

  fwrite(longlabel30, 30, 1, file);
  fwrite(shortlabel30, 30, 1, file);
  fwrite(unitlabel30, 30, 1, file);

  fwrite(&is1, sizeof(is1), 1, file);
  fwrite(&is2, sizeof(is2), 1, file);
  fwrite(&js1, sizeof(js1), 1, file);
  fwrite(&js2, sizeof(js2), 1, file);
  fwrite(&ks1, sizeof(ks1), 1, file);
  fwrite(&ks2, sizeof(ks2), 1, file);

  nxsp = is2 + 1 - is1;
  nysp = js2 + 1 - js1;
  nzsp = ks2 + 1 - ks1;
  nframe = nxsp * nysp * nzsp;
  for (int i = 0; i < ntimes; i++) {
    fwrite(&times[i], sizeof(times[i]), 1, file);
    ibeg = 1 + (i - 1) * nframe;
    iend = i * nframe;
    fwrite(&qdata[ibeg], sizeof(qdata[ibeg]), iend - ibeg, file);
  }

  fclose(file);

  return;
}

// !  ------------------ getsliceframe ------------------------

void getsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, double *time, double *qframe, int testslice,
                   int *error) {
  int i, j, k;
  int nxsp, nysp, nzsp;
  double val, factor;
  int index;
  double ii, jj, kk;

  nxsp = is2 + 1 - is1;
  nysp = js2 + 1 - js1;
  nzsp = ks2 + 1 - ks1;

  fread(time, sizeof(*time), 1, file);
  if (error != 0)
    return;
  fread(qframe, sizeof(*qframe), nxsp * nysp * nysp, file);
  if (testslice == 1 || testslice == 2) {
    factor = 1.0;
    if (testslice == 2)
      factor = 1.1;
    for (int k = 0; k < nzsp; k++) {
      kk = 2.0 * ((nzsp - 1) / 2.0 - k) / (nzsp - 1.0);
      for (int j = 0; j < nysp; j++) {
        jj = 2.0 * ((nysp - 1) / 2.0 - j) / (nysp - 1.0);
        for (int i = 0; i < nxsp; i++) {
          ii = 2.0 * ((nxsp - 1) / 2.0 - i) / (nxsp - 1.0);
          val = factor * (*time - 20.0) * (ii * ii + jj * jj + kk * kk) / 20.0;
          index = 1 + i + j * nxsp + k * nxsp * nysp;
          qframe[index] = val;
        }
      }
    }
  }
  return;
}

// !  ------------------ endianout ------------------------

void endianout(const char *endianfilename) {
  int one;
  FILE *file = FOPEN(endianfilename, "rb");
  one = 1;
  fwrite(&one, sizeof(one), 1, file);
  fclose(file);
  return;
}

// !  ------------------ outsliceheader ------------------------

void outsliceheader(const char *slicefilename, FILE *file, int ip1, int ip2,
                    int jp1, int jp2, int kp1, int kp2, int *error) {
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  file = FOPEN(slicefilename, "rb");

  strncpy(longlbl, "long                          ", 30);
  strncpy(shortlbl, "short                         ", 30);
  strncpy(unitlbl, "unit                          ", 30);

  fwrite(longlbl, 30, 1, file);
  fwrite(shortlbl, 30, 1, file);
  fwrite(unitlbl, 30, 1, file);

  fwrite(&ip1, sizeof(ip1), 1, file);
  fwrite(&ip2, sizeof(ip2), 1, file);
  fwrite(&jp1, sizeof(jp1), 1, file);
  fwrite(&jp2, sizeof(jp2), 1, file);
  fwrite(&kp1, sizeof(kp1), 1, file);
  fwrite(&kp2, sizeof(kp2), 1, file);
  return;
}

// !  ------------------ outsliceframe ------------------------

void outsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, double time, double *qframe, int *error) {
  int i, j, k;
  int nxsp, nysp, nzsp;

  nxsp = is2 + 1 - is1;
  nysp = js2 + 1 - js1;
  nzsp = ks2 + 1 - ks1;

  fwrite(&time, sizeof(time), 1, file);
  if (error != 0)
    return;
  fwrite(qframe, sizeof(*qframe), nxsp * nysp * nysp, file);
  return;
}

// !  ------------------ outboundaryheader ------------------------

void outboundaryheader(const char *boundaryfilename, FILE *file, int npatches,
                       int *pi1, int *pi2, int *pj1, int *pj2, int *pk1,
                       int *pk2, int *patchdir, int *error) {
  char blank[31];

  *error = 0;
  file = FOPEN(boundaryfilename, "rb");

  strncpy(blank, "                              ", 30);
  fwrite(blank, 30, 1, file);
  fwrite(blank, 30, 1, file);
  fwrite(blank, 30, 1, file);
  fwrite(&npatches, sizeof(npatches), 1, file);

  for (int n = 0; n < npatches; n++) {
    fwrite(&pi1[n], sizeof(pi1[n]), 1, file);
    fwrite(&pj1[n], sizeof(pj1[n]), 1, file);
    fwrite(&pj2[n], sizeof(pj2[n]), 1, file);
    fwrite(&pk1[n], sizeof(pk1[n]), 1, file);
    fwrite(&pk2[n], sizeof(pk2[n]), 1, file);
    fwrite(&patchdir[n], sizeof(patchdir[n]), 1, file);
  }

  return;
}

// !  ------------------ outpatchframe ------------------------

void outpatchframe(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                   int *pj2, int *pk1, int *pk2, double patchtime, double *pqq,
                   int *error) {
  int i, i1, i2, j1, j2, k1, k2, size, ibeg, iend, ii;

  *error = 0;
  fwrite(&patchtime, sizeof(patchtime), 1, file);
  ibeg = 1;
  for (int i = 0; i < npatch; i++) {
    i1 = pi1[i];
    i2 = pi2[i];
    j1 = pj1[i];
    j2 = pj2[i];
    k1 = pk1[i];
    k2 = pk2[i];
    size = (i2 + 1 - i1) * (j2 + 1 - j1) * (k2 + 1 - k1);
    iend = ibeg + size - 1;
    fwrite(&pqq[ibeg], sizeof(pqq[ibeg]), iend - ibeg, file);
    ibeg = iend + 1;
  }
  return;
}

// !  ------------------ getplot3dq ------------------------

void getplot3dq(const char *qfilename, int nx, int ny, int nz, double *qq,
                int *error, int *isotest) {
  FILE *file;

  double dum1, dum2, dum3, dum4;
  int error2;
  double dummy, qval;

  int nxpts, nypts, nzpts;
  int i, j, k, n;

  if (isotest == 0) {
    error = 0;
    file = FOPEN(qfilename, "rb");
    if (file == NULL) {
      PRINTF(" The file name, %s does not exist\n", qfilename);
      fread(&dummy, sizeof(dummy), 1, file);
      exit(1);
    }
    fread(&nxpts, sizeof(nxpts), 1, file);
    fread(&nypts, sizeof(nypts), 1, file);
    fread(&nzpts, sizeof(nzpts), 1, file);
    if (nx == nxpts && ny == nypts && nz == nzpts) {
      fread(&dum1, sizeof(dum1), 1, file);
      fread(&dum2, sizeof(dum2), 1, file);
      fread(&dum3, sizeof(dum3), 1, file);
      fread(&dum4, sizeof(dum4), 1, file);
      fread(qq, sizeof(*qq), nxpts * nypts * nzpts * 5, file);

    } else {
      *error = 1;
      PRINTF(" *** Fatal error in getplot3dq ***\n");
      PRINTF(" Grid size found in plot3d file was: %d,%d,%d\n", nxpts, nypts,
             nzpts);
      PRINTF(" Was expecting: %d,%d,%d\n", nx, ny, nz);
      exit(1);
    }
    fclose(file);
  } else {
    for (int i = 1; i <= nx; i++) {
      for (int j = 1; j <= ny; j++) {
        for (int k = 1; k <= nz; k++) {
          qval = pow(i - nx / 2, 2) + pow(j - ny / 2, 2) + pow(k - nz / 2, 2);
          qval = sqrt(qval);
          if (isotest == 1) {
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 1] = 0.0;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 2] = 0.0;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 3] = qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 4] = qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 5] = qval;
          }
          if (isotest == 2) {
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 1] = qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 2] = 1.1 * qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 3] = 1.1 * qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 4] = 1.1 * qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 5] = 1.1 * qval;
          }
        }
      }
    }
    *error = 0;
  }
  fclose(file);
  return;
}

// !  ------------------ plot3dout ------------------------

void plot3dout(const char *outfile, int nx, int ny, int nz, double *qout,
               int *error3) {
  int u_out;
  int i, j, k, n;
  double dummy;

  error3 = 0;

  dummy = 0.0;
  FILE *file = FOPEN(outfile, "rb");
  if (error3 != 0)
    return;

  fwrite(&nx, sizeof(nx), 1, file);
  fwrite(&ny, sizeof(ny), 1, file);
  fwrite(&nz, sizeof(nz), 1, file);

  fwrite(&dummy, sizeof(dummy), 1, file);
  fwrite(&dummy, sizeof(dummy), 1, file);
  fwrite(&dummy, sizeof(dummy), 1, file);
  fwrite(&dummy, sizeof(dummy), 1, file);

  fwrite(qout, sizeof(*qout), nx * ny * nz * 5, file);
  fclose(file);

  return;
}

// Translate character string of a color name to RGB value
void color2rgb(int rgb[3], const char *color) {

  if (strcmp(color, "ALICE BLUE") == 0) {
    rgb[0] = 240;
    rgb[1] = 248;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "ANTIQUE WHITE") == 0) {
    rgb[0] = 250;
    rgb[1] = 235;
    rgb[2] = 215;
    return;
  }
  if (strcmp(color, "ANTIQUE WHITE 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 239;
    rgb[2] = 219;
    return;
  }
  if (strcmp(color, "ANTIQUE WHITE 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 223;
    rgb[2] = 204;
    return;
  }
  if (strcmp(color, "ANTIQUE WHITE 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 192;
    rgb[2] = 176;
    return;
  }
  if (strcmp(color, "ANTIQUE WHITE 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 131;
    rgb[2] = 120;
    return;
  }
  if (strcmp(color, "AQUAMARINE") == 0) {
    rgb[0] = 127;
    rgb[1] = 255;
    rgb[2] = 212;
    return;
  }
  if (strcmp(color, "AQUAMARINE 1") == 0) {
    rgb[0] = 118;
    rgb[1] = 238;
    rgb[2] = 198;
    return;
  }
  if (strcmp(color, "AQUAMARINE 2") == 0) {
    rgb[0] = 102;
    rgb[1] = 205;
    rgb[2] = 170;
    return;
  }
  if (strcmp(color, "AQUAMARINE 3") == 0) {
    rgb[0] = 69;
    rgb[1] = 139;
    rgb[2] = 116;
    return;
  }
  if (strcmp(color, "AZURE") == 0) {
    rgb[0] = 240;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "AZURE 1") == 0) {
    rgb[0] = 224;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "AZURE 2") == 0) {
    rgb[0] = 193;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "AZURE 3") == 0) {
    rgb[0] = 131;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "BANANA") == 0) {
    rgb[0] = 227;
    rgb[1] = 207;
    rgb[2] = 87;
    return;
  }
  if (strcmp(color, "BEIGE") == 0) {
    rgb[0] = 245;
    rgb[1] = 245;
    rgb[2] = 220;
    return;
  }
  if (strcmp(color, "BISQUE") == 0) {
    rgb[0] = 255;
    rgb[1] = 228;
    rgb[2] = 196;
    return;
  }
  if (strcmp(color, "BISQUE 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 213;
    rgb[2] = 183;
    return;
  }
  if (strcmp(color, "BISQUE 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 183;
    rgb[2] = 158;
    return;
  }
  if (strcmp(color, "BISQUE 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 125;
    rgb[2] = 107;
    return;
  }
  if (strcmp(color, "BLACK") == 0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "BLANCHED ALMOND") == 0) {
    rgb[0] = 255;
    rgb[1] = 235;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "BLUE") == 0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "BLUE 2") == 0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "BLUE 3") == 0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "BLUE 4") == 0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "BLUE VIOLET") == 0) {
    rgb[0] = 138;
    rgb[1] = 43;
    rgb[2] = 226;
    return;
  }
  if (strcmp(color, "BRICK") == 0) {
    rgb[0] = 156;
    rgb[1] = 102;
    rgb[2] = 31;
    return;
  }
  if (strcmp(color, "BROWN") == 0) {
    rgb[0] = 165;
    rgb[1] = 42;
    rgb[2] = 42;
    return;
  }
  if (strcmp(color, "BROWN 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 64;
    rgb[2] = 64;
    return;
  }
  if (strcmp(color, "BROWN 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 59;
    rgb[2] = 59;
    return;
  }
  if (strcmp(color, "BROWN 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 51;
    rgb[2] = 51;
    return;
  }
  if (strcmp(color, "BROWN 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 35;
    rgb[2] = 35;
    return;
  }
  if (strcmp(color, "BURLY WOOD") == 0) {
    rgb[0] = 222;
    rgb[1] = 184;
    rgb[2] = 135;
    return;
  }
  if (strcmp(color, "BURLY WOOD 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 211;
    rgb[2] = 155;
    return;
  }
  if (strcmp(color, "BURLY WOOD 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 197;
    rgb[2] = 145;
    return;
  }
  if (strcmp(color, "BURLY WOOD 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 170;
    rgb[2] = 125;
    return;
  }
  if (strcmp(color, "BURLY WOOD 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 115;
    rgb[2] = 85;
    return;
  }
  if (strcmp(color, "BURNT ORANGE") == 0) {
    rgb[0] = 204;
    rgb[1] = 85;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "BURNT SIENNA") == 0) {
    rgb[0] = 138;
    rgb[1] = 54;
    rgb[2] = 15;
    return;
  }
  if (strcmp(color, "BURNT UMBER") == 0) {
    rgb[0] = 138;
    rgb[1] = 51;
    rgb[2] = 36;
    return;
  }
  if (strcmp(color, "CADET BLUE") == 0) {
    rgb[0] = 95;
    rgb[1] = 158;
    rgb[2] = 160;
    return;
  }
  if (strcmp(color, "CADET BLUE 1") == 0) {
    rgb[0] = 152;
    rgb[1] = 245;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "CADET BLUE 2") == 0) {
    rgb[0] = 142;
    rgb[1] = 229;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "CADET BLUE 3") == 0) {
    rgb[0] = 122;
    rgb[1] = 197;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "CADET BLUE 4") == 0) {
    rgb[0] = 83;
    rgb[1] = 134;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "CADMIUM ORANGE") == 0) {
    rgb[0] = 255;
    rgb[1] = 97;
    rgb[2] = 3;
    return;
  }
  if (strcmp(color, "CADMIUM YELLOW") == 0) {
    rgb[0] = 255;
    rgb[1] = 153;
    rgb[2] = 18;
    return;
  }
  if (strcmp(color, "CARROT") == 0) {
    rgb[0] = 237;
    rgb[1] = 145;
    rgb[2] = 33;
    return;
  }
  if (strcmp(color, "CHARTREUSE") == 0) {
    rgb[0] = 127;
    rgb[1] = 255;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "CHARTREUSE 1") == 0) {
    rgb[0] = 118;
    rgb[1] = 238;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "CHARTREUSE 2") == 0) {
    rgb[0] = 102;
    rgb[1] = 205;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "CHARTREUSE 3") == 0) {
    rgb[0] = 69;
    rgb[1] = 139;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "CHOCOLATE") == 0) {
    rgb[0] = 210;
    rgb[1] = 105;
    rgb[2] = 30;
    return;
  }
  if (strcmp(color, "CHOCOLATE 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 127;
    rgb[2] = 36;
    return;
  }
  if (strcmp(color, "CHOCOLATE 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 118;
    rgb[2] = 33;
    return;
  }
  if (strcmp(color, "CHOCOLATE 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 102;
    rgb[2] = 29;
    return;
  }
  if (strcmp(color, "CHOCOLATE 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 69;
    rgb[2] = 19;
    return;
  }
  if (strcmp(color, "COBALT") == 0) {
    rgb[0] = 61;
    rgb[1] = 89;
    rgb[2] = 171;
    return;
  }
  if (strcmp(color, "COBALT GREEN") == 0) {
    rgb[0] = 61;
    rgb[1] = 145;
    rgb[2] = 64;
    return;
  }
  if (strcmp(color, "COLD GREY") == 0) {
    rgb[0] = 128;
    rgb[1] = 138;
    rgb[2] = 135;
    return;
  }
  if (strcmp(color, "CORAL") == 0) {
    rgb[0] = 255;
    rgb[1] = 127;
    rgb[2] = 80;
    return;
  }
  if (strcmp(color, "CORAL 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 114;
    rgb[2] = 86;
    return;
  }
  if (strcmp(color, "CORAL 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 106;
    rgb[2] = 80;
    return;
  }
  if (strcmp(color, "CORAL 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 91;
    rgb[2] = 69;
    return;
  }
  if (strcmp(color, "CORAL 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 62;
    rgb[2] = 47;
    return;
  }
  if (strcmp(color, "CORNFLOWER BLUE") == 0) {
    rgb[0] = 100;
    rgb[1] = 149;
    rgb[2] = 237;
    return;
  }
  if (strcmp(color, "CORNSILK") == 0) {
    rgb[0] = 255;
    rgb[1] = 248;
    rgb[2] = 220;
    return;
  }
  if (strcmp(color, "CORNSILK 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 232;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "CORNSILK 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 200;
    rgb[2] = 177;
    return;
  }
  if (strcmp(color, "CORNSILK 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 136;
    rgb[2] = 120;
    return;
  }
  if (strcmp(color, "CRIMSON") == 0) {
    rgb[0] = 220;
    rgb[1] = 20;
    rgb[2] = 60;
    return;
  }
  if (strcmp(color, "CYAN") == 0) {
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "CYAN 2") == 0) {
    rgb[0] = 0;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "CYAN 3") == 0) {
    rgb[0] = 0;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "CYAN 4") == 0) {
    rgb[0] = 0;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "DARK GOLDENROD") == 0) {
    rgb[0] = 184;
    rgb[1] = 134;
    rgb[2] = 11;
    return;
  }
  if (strcmp(color, "DARK GOLDENROD 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 185;
    rgb[2] = 15;
    return;
  }
  if (strcmp(color, "DARK GOLDENROD 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 173;
    rgb[2] = 14;
    return;
  }
  if (strcmp(color, "DARK GOLDENROD 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 149;
    rgb[2] = 12;
    return;
  }
  if (strcmp(color, "DARK GOLDENROD 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 101;
    rgb[2] = 8;
    return;
  }
  if (strcmp(color, "DARK GRAY") == 0) {
    rgb[0] = 169;
    rgb[1] = 169;
    rgb[2] = 169;
    return;
  }
  if (strcmp(color, "DARK GREEN") == 0) {
    rgb[0] = 0;
    rgb[1] = 100;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "DARK KHAKI") == 0) {
    rgb[0] = 189;
    rgb[1] = 183;
    rgb[2] = 107;
    return;
  }
  if (strcmp(color, "DARK OLIVE GREEN") == 0) {
    rgb[0] = 85;
    rgb[1] = 107;
    rgb[2] = 47;
    return;
  }
  if (strcmp(color, "DARK OLIVE GREEN 1") == 0) {
    rgb[0] = 202;
    rgb[1] = 255;
    rgb[2] = 112;
    return;
  }
  if (strcmp(color, "DARK OLIVE GREEN 2") == 0) {
    rgb[0] = 188;
    rgb[1] = 238;
    rgb[2] = 104;
    return;
  }
  if (strcmp(color, "DARK OLIVE GREEN 3") == 0) {
    rgb[0] = 162;
    rgb[1] = 205;
    rgb[2] = 90;
    return;
  }
  if (strcmp(color, "DARK OLIVE GREEN 4") == 0) {
    rgb[0] = 110;
    rgb[1] = 139;
    rgb[2] = 61;
    return;
  }
  if (strcmp(color, "DARK ORANGE") == 0) {
    rgb[0] = 255;
    rgb[1] = 140;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "DARK ORANGE 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 127;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "DARK ORANGE 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 118;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "DARK ORANGE 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 102;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "DARK ORANGE 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 69;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "DARK ORCHID") == 0) {
    rgb[0] = 153;
    rgb[1] = 50;
    rgb[2] = 204;
    return;
  }
  if (strcmp(color, "DARK ORCHID 1") == 0) {
    rgb[0] = 191;
    rgb[1] = 62;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "DARK ORCHID 2") == 0) {
    rgb[0] = 178;
    rgb[1] = 58;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "DARK ORCHID 3") == 0) {
    rgb[0] = 154;
    rgb[1] = 50;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "DARK ORCHID 4") == 0) {
    rgb[0] = 104;
    rgb[1] = 34;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "DARK SALMON") == 0) {
    rgb[0] = 233;
    rgb[1] = 150;
    rgb[2] = 122;
    return;
  }
  if (strcmp(color, "DARK SEA GREEN") == 0) {
    rgb[0] = 143;
    rgb[1] = 188;
    rgb[2] = 143;
    return;
  }
  if (strcmp(color, "DARK SEA GREEN 1") == 0) {
    rgb[0] = 193;
    rgb[1] = 255;
    rgb[2] = 193;
    return;
  }
  if (strcmp(color, "DARK SEA GREEN 2") == 0) {
    rgb[0] = 180;
    rgb[1] = 238;
    rgb[2] = 180;
    return;
  }
  if (strcmp(color, "DARK SEA GREEN 3") == 0) {
    rgb[0] = 155;
    rgb[1] = 205;
    rgb[2] = 155;
    return;
  }
  if (strcmp(color, "DARK SEA GREEN 4") == 0) {
    rgb[0] = 105;
    rgb[1] = 139;
    rgb[2] = 105;
    return;
  }
  if (strcmp(color, "DARK SLATE BLUE") == 0) {
    rgb[0] = 72;
    rgb[1] = 61;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "DARK SLATE GRAY") == 0) {
    rgb[0] = 47;
    rgb[1] = 79;
    rgb[2] = 79;
    return;
  }
  if (strcmp(color, "DARK SLATE GRAY 1") == 0) {
    rgb[0] = 151;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "DARK SLATE GRAY 2") == 0) {
    rgb[0] = 141;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "DARK SLATE GRAY 3") == 0) {
    rgb[0] = 121;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "DARK SLATE GRAY 4") == 0) {
    rgb[0] = 82;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "DARK TURQUOISE") == 0) {
    rgb[0] = 0;
    rgb[1] = 206;
    rgb[2] = 209;
    return;
  }
  if (strcmp(color, "DARK VIOLET") == 0) {
    rgb[0] = 148;
    rgb[1] = 0;
    rgb[2] = 211;
    return;
  }
  if (strcmp(color, "DEEP PINK") == 0) {
    rgb[0] = 255;
    rgb[1] = 20;
    rgb[2] = 147;
    return;
  }
  if (strcmp(color, "DEEP PINK 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 18;
    rgb[2] = 137;
    return;
  }
  if (strcmp(color, "DEEP PINK 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 16;
    rgb[2] = 118;
    return;
  }
  if (strcmp(color, "DEEP PINK 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 10;
    rgb[2] = 80;
    return;
  }
  if (strcmp(color, "DEEP SKYBLUE") == 0) {
    rgb[0] = 0;
    rgb[1] = 191;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "DEEP SKYBLUE 1") == 0) {
    rgb[0] = 0;
    rgb[1] = 178;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "DEEP SKYBLUE 2") == 0) {
    rgb[0] = 0;
    rgb[1] = 154;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "DEEP SKYBLUE 3") == 0) {
    rgb[0] = 0;
    rgb[1] = 104;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "DIM GRAY") == 0) {
    rgb[0] = 105;
    rgb[1] = 105;
    rgb[2] = 105;
    return;
  }
  if (strcmp(color, "DODGERBLUE") == 0) {
    rgb[0] = 30;
    rgb[1] = 144;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "DODGERBLUE 1") == 0) {
    rgb[0] = 28;
    rgb[1] = 134;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "DODGERBLUE 2") == 0) {
    rgb[0] = 24;
    rgb[1] = 116;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "DODGERBLUE 3") == 0) {
    rgb[0] = 16;
    rgb[1] = 78;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "EGGSHELL") == 0) {
    rgb[0] = 252;
    rgb[1] = 230;
    rgb[2] = 201;
    return;
  }
  if (strcmp(color, "EMERALD GREEN") == 0) {
    rgb[0] = 0;
    rgb[1] = 201;
    rgb[2] = 87;
    return;
  }
  if (strcmp(color, "FIREBRICK") == 0) {
    rgb[0] = 178;
    rgb[1] = 34;
    rgb[2] = 34;
    return;
  }
  if (strcmp(color, "FIREBRICK 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 48;
    rgb[2] = 48;
    return;
  }
  if (strcmp(color, "FIREBRICK 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 44;
    rgb[2] = 44;
    return;
  }
  if (strcmp(color, "FIREBRICK 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 38;
    rgb[2] = 38;
    return;
  }
  if (strcmp(color, "FIREBRICK 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 26;
    rgb[2] = 26;
    return;
  }
  if (strcmp(color, "FLESH") == 0) {
    rgb[0] = 255;
    rgb[1] = 125;
    rgb[2] = 64;
    return;
  }
  if (strcmp(color, "FLORAL WHITE") == 0) {
    rgb[0] = 255;
    rgb[1] = 250;
    rgb[2] = 240;
    return;
  }
  if (strcmp(color, "FOREST GREEN") == 0) {
    rgb[0] = 34;
    rgb[1] = 139;
    rgb[2] = 34;
    return;
  }
  if (strcmp(color, "GAINSBORO") == 0) {
    rgb[0] = 220;
    rgb[1] = 220;
    rgb[2] = 220;
    return;
  }
  if (strcmp(color, "GHOST WHITE") == 0) {
    rgb[0] = 248;
    rgb[1] = 248;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "GOLD") == 0) {
    rgb[0] = 255;
    rgb[1] = 215;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GOLD 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 201;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GOLD 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 173;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GOLD 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 117;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GOLDENROD") == 0) {
    rgb[0] = 218;
    rgb[1] = 165;
    rgb[2] = 32;
    return;
  }
  if (strcmp(color, "GOLDENROD 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 193;
    rgb[2] = 37;
    return;
  }
  if (strcmp(color, "GOLDENROD 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 180;
    rgb[2] = 34;
    return;
  }
  if (strcmp(color, "GOLDENROD 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 155;
    rgb[2] = 29;
    return;
  }
  if (strcmp(color, "GOLDENROD 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 105;
    rgb[2] = 20;
    return;
  }
  if (strcmp(color, "GRAY") == 0) {
    rgb[0] = 128;
    rgb[1] = 128;
    rgb[2] = 128;
    return;
  }
  if (strcmp(color, "GRAY 1") == 0) {
    rgb[0] = 3;
    rgb[1] = 3;
    rgb[2] = 3;
    return;
  }
  if (strcmp(color, "GRAY 10") == 0) {
    rgb[0] = 26;
    rgb[1] = 26;
    rgb[2] = 26;
    return;
  }
  if (strcmp(color, "GRAY 11") == 0) {
    rgb[0] = 28;
    rgb[1] = 28;
    rgb[2] = 28;
    return;
  }
  if (strcmp(color, "GRAY 12") == 0) {
    rgb[0] = 31;
    rgb[1] = 31;
    rgb[2] = 31;
    return;
  }
  if (strcmp(color, "GRAY 13") == 0) {
    rgb[0] = 33;
    rgb[1] = 33;
    rgb[2] = 33;
    return;
  }
  if (strcmp(color, "GRAY 14") == 0) {
    rgb[0] = 36;
    rgb[1] = 36;
    rgb[2] = 36;
    return;
  }
  if (strcmp(color, "GRAY 15") == 0) {
    rgb[0] = 38;
    rgb[1] = 38;
    rgb[2] = 38;
    return;
  }
  if (strcmp(color, "GRAY 16") == 0) {
    rgb[0] = 41;
    rgb[1] = 41;
    rgb[2] = 41;
    return;
  }
  if (strcmp(color, "GRAY 17") == 0) {
    rgb[0] = 43;
    rgb[1] = 43;
    rgb[2] = 43;
    return;
  }
  if (strcmp(color, "GRAY 18") == 0) {
    rgb[0] = 46;
    rgb[1] = 46;
    rgb[2] = 46;
    return;
  }
  if (strcmp(color, "GRAY 19") == 0) {
    rgb[0] = 48;
    rgb[1] = 48;
    rgb[2] = 48;
    return;
  }
  if (strcmp(color, "GRAY 2") == 0) {
    rgb[0] = 5;
    rgb[1] = 5;
    rgb[2] = 5;
    return;
  }
  if (strcmp(color, "GRAY 20") == 0) {
    rgb[0] = 51;
    rgb[1] = 51;
    rgb[2] = 51;
    return;
  }
  if (strcmp(color, "GRAY 21") == 0) {
    rgb[0] = 54;
    rgb[1] = 54;
    rgb[2] = 54;
    return;
  }
  if (strcmp(color, "GRAY 22") == 0) {
    rgb[0] = 56;
    rgb[1] = 56;
    rgb[2] = 56;
    return;
  }
  if (strcmp(color, "GRAY 23") == 0) {
    rgb[0] = 59;
    rgb[1] = 59;
    rgb[2] = 59;
    return;
  }
  if (strcmp(color, "GRAY 24") == 0) {
    rgb[0] = 61;
    rgb[1] = 61;
    rgb[2] = 61;
    return;
  }
  if (strcmp(color, "GRAY 25") == 0) {
    rgb[0] = 64;
    rgb[1] = 64;
    rgb[2] = 64;
    return;
  }
  if (strcmp(color, "GRAY 26") == 0) {
    rgb[0] = 66;
    rgb[1] = 66;
    rgb[2] = 66;
    return;
  }
  if (strcmp(color, "GRAY 27") == 0) {
    rgb[0] = 69;
    rgb[1] = 69;
    rgb[2] = 69;
    return;
  }
  if (strcmp(color, "GRAY 28") == 0) {
    rgb[0] = 71;
    rgb[1] = 71;
    rgb[2] = 71;
    return;
  }
  if (strcmp(color, "GRAY 29") == 0) {
    rgb[0] = 74;
    rgb[1] = 74;
    rgb[2] = 74;
    return;
  }
  if (strcmp(color, "GRAY 3") == 0) {
    rgb[0] = 8;
    rgb[1] = 8;
    rgb[2] = 8;
    return;
  }
  if (strcmp(color, "GRAY 30") == 0) {
    rgb[0] = 77;
    rgb[1] = 77;
    rgb[2] = 77;
    return;
  }
  if (strcmp(color, "GRAY 31") == 0) {
    rgb[0] = 79;
    rgb[1] = 79;
    rgb[2] = 79;
    return;
  }
  if (strcmp(color, "GRAY 32") == 0) {
    rgb[0] = 82;
    rgb[1] = 82;
    rgb[2] = 82;
    return;
  }
  if (strcmp(color, "GRAY 33") == 0) {
    rgb[0] = 84;
    rgb[1] = 84;
    rgb[2] = 84;
    return;
  }
  if (strcmp(color, "GRAY 34") == 0) {
    rgb[0] = 87;
    rgb[1] = 87;
    rgb[2] = 87;
    return;
  }
  if (strcmp(color, "GRAY 35") == 0) {
    rgb[0] = 89;
    rgb[1] = 89;
    rgb[2] = 89;
    return;
  }
  if (strcmp(color, "GRAY 36") == 0) {
    rgb[0] = 92;
    rgb[1] = 92;
    rgb[2] = 92;
    return;
  }
  if (strcmp(color, "GRAY 37") == 0) {
    rgb[0] = 94;
    rgb[1] = 94;
    rgb[2] = 94;
    return;
  }
  if (strcmp(color, "GRAY 38") == 0) {
    rgb[0] = 97;
    rgb[1] = 97;
    rgb[2] = 97;
    return;
  }
  if (strcmp(color, "GRAY 39") == 0) {
    rgb[0] = 99;
    rgb[1] = 99;
    rgb[2] = 99;
    return;
  }
  if (strcmp(color, "GRAY 4") == 0) {
    rgb[0] = 10;
    rgb[1] = 10;
    rgb[2] = 10;
    return;
  }
  if (strcmp(color, "GRAY 40") == 0) {
    rgb[0] = 102;
    rgb[1] = 102;
    rgb[2] = 102;
    return;
  }
  if (strcmp(color, "GRAY 42") == 0) {
    rgb[0] = 107;
    rgb[1] = 107;
    rgb[2] = 107;
    return;
  }
  if (strcmp(color, "GRAY 43") == 0) {
    rgb[0] = 110;
    rgb[1] = 110;
    rgb[2] = 110;
    return;
  }
  if (strcmp(color, "GRAY 44") == 0) {
    rgb[0] = 112;
    rgb[1] = 112;
    rgb[2] = 112;
    return;
  }
  if (strcmp(color, "GRAY 45") == 0) {
    rgb[0] = 115;
    rgb[1] = 115;
    rgb[2] = 115;
    return;
  }
  if (strcmp(color, "GRAY 46") == 0) {
    rgb[0] = 117;
    rgb[1] = 117;
    rgb[2] = 117;
    return;
  }
  if (strcmp(color, "GRAY 47") == 0) {
    rgb[0] = 120;
    rgb[1] = 120;
    rgb[2] = 120;
    return;
  }
  if (strcmp(color, "GRAY 48") == 0) {
    rgb[0] = 122;
    rgb[1] = 122;
    rgb[2] = 122;
    return;
  }
  if (strcmp(color, "GRAY 49") == 0) {
    rgb[0] = 125;
    rgb[1] = 125;
    rgb[2] = 125;
    return;
  }
  if (strcmp(color, "GRAY 5") == 0) {
    rgb[0] = 13;
    rgb[1] = 13;
    rgb[2] = 13;
    return;
  }
  if (strcmp(color, "GRAY 50") == 0) {
    rgb[0] = 127;
    rgb[1] = 127;
    rgb[2] = 127;
    return;
  }
  if (strcmp(color, "GRAY 51") == 0) {
    rgb[0] = 130;
    rgb[1] = 130;
    rgb[2] = 130;
    return;
  }
  if (strcmp(color, "GRAY 52") == 0) {
    rgb[0] = 133;
    rgb[1] = 133;
    rgb[2] = 133;
    return;
  }
  if (strcmp(color, "GRAY 53") == 0) {
    rgb[0] = 135;
    rgb[1] = 135;
    rgb[2] = 135;
    return;
  }
  if (strcmp(color, "GRAY 54") == 0) {
    rgb[0] = 138;
    rgb[1] = 138;
    rgb[2] = 138;
    return;
  }
  if (strcmp(color, "GRAY 55") == 0) {
    rgb[0] = 140;
    rgb[1] = 140;
    rgb[2] = 140;
    return;
  }
  if (strcmp(color, "GRAY 56") == 0) {
    rgb[0] = 143;
    rgb[1] = 143;
    rgb[2] = 143;
    return;
  }
  if (strcmp(color, "GRAY 57") == 0) {
    rgb[0] = 145;
    rgb[1] = 145;
    rgb[2] = 145;
    return;
  }
  if (strcmp(color, "GRAY 58") == 0) {
    rgb[0] = 148;
    rgb[1] = 148;
    rgb[2] = 148;
    return;
  }
  if (strcmp(color, "GRAY 59") == 0) {
    rgb[0] = 150;
    rgb[1] = 150;
    rgb[2] = 150;
    return;
  }
  if (strcmp(color, "GRAY 6") == 0) {
    rgb[0] = 15;
    rgb[1] = 15;
    rgb[2] = 15;
    return;
  }
  if (strcmp(color, "GRAY 60") == 0) {
    rgb[0] = 153;
    rgb[1] = 153;
    rgb[2] = 153;
    return;
  }
  if (strcmp(color, "GRAY 61") == 0) {
    rgb[0] = 156;
    rgb[1] = 156;
    rgb[2] = 156;
    return;
  }
  if (strcmp(color, "GRAY 62") == 0) {
    rgb[0] = 158;
    rgb[1] = 158;
    rgb[2] = 158;
    return;
  }
  if (strcmp(color, "GRAY 63") == 0) {
    rgb[0] = 161;
    rgb[1] = 161;
    rgb[2] = 161;
    return;
  }
  if (strcmp(color, "GRAY 64") == 0) {
    rgb[0] = 163;
    rgb[1] = 163;
    rgb[2] = 163;
    return;
  }
  if (strcmp(color, "GRAY 65") == 0) {
    rgb[0] = 166;
    rgb[1] = 166;
    rgb[2] = 166;
    return;
  }
  if (strcmp(color, "GRAY 66") == 0) {
    rgb[0] = 168;
    rgb[1] = 168;
    rgb[2] = 168;
    return;
  }
  if (strcmp(color, "GRAY 67") == 0) {
    rgb[0] = 171;
    rgb[1] = 171;
    rgb[2] = 171;
    return;
  }
  if (strcmp(color, "GRAY 68") == 0) {
    rgb[0] = 173;
    rgb[1] = 173;
    rgb[2] = 173;
    return;
  }
  if (strcmp(color, "GRAY 69") == 0) {
    rgb[0] = 176;
    rgb[1] = 176;
    rgb[2] = 176;
    return;
  }
  if (strcmp(color, "GRAY 7") == 0) {
    rgb[0] = 18;
    rgb[1] = 18;
    rgb[2] = 18;
    return;
  }
  if (strcmp(color, "GRAY 70") == 0) {
    rgb[0] = 179;
    rgb[1] = 179;
    rgb[2] = 179;
    return;
  }
  if (strcmp(color, "GRAY 71") == 0) {
    rgb[0] = 181;
    rgb[1] = 181;
    rgb[2] = 181;
    return;
  }
  if (strcmp(color, "GRAY 72") == 0) {
    rgb[0] = 184;
    rgb[1] = 184;
    rgb[2] = 184;
    return;
  }
  if (strcmp(color, "GRAY 73") == 0) {
    rgb[0] = 186;
    rgb[1] = 186;
    rgb[2] = 186;
    return;
  }
  if (strcmp(color, "GRAY 74") == 0) {
    rgb[0] = 189;
    rgb[1] = 189;
    rgb[2] = 189;
    return;
  }
  if (strcmp(color, "GRAY 75") == 0) {
    rgb[0] = 191;
    rgb[1] = 191;
    rgb[2] = 191;
    return;
  }
  if (strcmp(color, "GRAY 76") == 0) {
    rgb[0] = 194;
    rgb[1] = 194;
    rgb[2] = 194;
    return;
  }
  if (strcmp(color, "GRAY 77") == 0) {
    rgb[0] = 196;
    rgb[1] = 196;
    rgb[2] = 196;
    return;
  }
  if (strcmp(color, "GRAY 78") == 0) {
    rgb[0] = 199;
    rgb[1] = 199;
    rgb[2] = 199;
    return;
  }
  if (strcmp(color, "GRAY 79") == 0) {
    rgb[0] = 201;
    rgb[1] = 201;
    rgb[2] = 201;
    return;
  }
  if (strcmp(color, "GRAY 8") == 0) {
    rgb[0] = 20;
    rgb[1] = 20;
    rgb[2] = 20;
    return;
  }
  if (strcmp(color, "GRAY 80") == 0) {
    rgb[0] = 204;
    rgb[1] = 204;
    rgb[2] = 204;
    return;
  }
  if (strcmp(color, "GRAY 81") == 0) {
    rgb[0] = 207;
    rgb[1] = 207;
    rgb[2] = 207;
    return;
  }
  if (strcmp(color, "GRAY 82") == 0) {
    rgb[0] = 209;
    rgb[1] = 209;
    rgb[2] = 209;
    return;
  }
  if (strcmp(color, "GRAY 83") == 0) {
    rgb[0] = 212;
    rgb[1] = 212;
    rgb[2] = 212;
    return;
  }
  if (strcmp(color, "GRAY 84") == 0) {
    rgb[0] = 214;
    rgb[1] = 214;
    rgb[2] = 214;
    return;
  }
  if (strcmp(color, "GRAY 85") == 0) {
    rgb[0] = 217;
    rgb[1] = 217;
    rgb[2] = 217;
    return;
  }
  if (strcmp(color, "GRAY 86") == 0) {
    rgb[0] = 219;
    rgb[1] = 219;
    rgb[2] = 219;
    return;
  }
  if (strcmp(color, "GRAY 87") == 0) {
    rgb[0] = 222;
    rgb[1] = 222;
    rgb[2] = 222;
    return;
  }
  if (strcmp(color, "GRAY 88") == 0) {
    rgb[0] = 224;
    rgb[1] = 224;
    rgb[2] = 224;
    return;
  }
  if (strcmp(color, "GRAY 89") == 0) {
    rgb[0] = 227;
    rgb[1] = 227;
    rgb[2] = 227;
    return;
  }
  if (strcmp(color, "GRAY 9") == 0) {
    rgb[0] = 23;
    rgb[1] = 23;
    rgb[2] = 23;
    return;
  }
  if (strcmp(color, "GRAY 90") == 0) {
    rgb[0] = 229;
    rgb[1] = 229;
    rgb[2] = 229;
    return;
  }
  if (strcmp(color, "GRAY 91") == 0) {
    rgb[0] = 232;
    rgb[1] = 232;
    rgb[2] = 232;
    return;
  }
  if (strcmp(color, "GRAY 92") == 0) {
    rgb[0] = 235;
    rgb[1] = 235;
    rgb[2] = 235;
    return;
  }
  if (strcmp(color, "GRAY 93") == 0) {
    rgb[0] = 237;
    rgb[1] = 237;
    rgb[2] = 237;
    return;
  }
  if (strcmp(color, "GRAY 94") == 0) {
    rgb[0] = 240;
    rgb[1] = 240;
    rgb[2] = 240;
    return;
  }
  if (strcmp(color, "GRAY 95") == 0) {
    rgb[0] = 242;
    rgb[1] = 242;
    rgb[2] = 242;
    return;
  }
  if (strcmp(color, "GRAY 97") == 0) {
    rgb[0] = 247;
    rgb[1] = 247;
    rgb[2] = 247;
    return;
  }
  if (strcmp(color, "GRAY 98") == 0) {
    rgb[0] = 250;
    rgb[1] = 250;
    rgb[2] = 250;
    return;
  }
  if (strcmp(color, "GRAY 99") == 0) {
    rgb[0] = 252;
    rgb[1] = 252;
    rgb[2] = 252;
    return;
  }
  if (strcmp(color, "GREEN") == 0) {
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GREEN 2") == 0) {
    rgb[0] = 0;
    rgb[1] = 238;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GREEN 3") == 0) {
    rgb[0] = 0;
    rgb[1] = 205;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GREEN 4") == 0) {
    rgb[0] = 0;
    rgb[1] = 139;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "GREEN YELLOW") == 0) {
    rgb[0] = 173;
    rgb[1] = 255;
    rgb[2] = 47;
    return;
  }
  if (strcmp(color, "HONEYDEW") == 0) {
    rgb[0] = 240;
    rgb[1] = 255;
    rgb[2] = 240;
    return;
  }
  if (strcmp(color, "HONEYDEW 1") == 0) {
    rgb[0] = 224;
    rgb[1] = 238;
    rgb[2] = 224;
    return;
  }
  if (strcmp(color, "HONEYDEW 2") == 0) {
    rgb[0] = 193;
    rgb[1] = 205;
    rgb[2] = 193;
    return;
  }
  if (strcmp(color, "HONEYDEW 3") == 0) {
    rgb[0] = 131;
    rgb[1] = 139;
    rgb[2] = 131;
    return;
  }
  if (strcmp(color, "HOT PINK") == 0) {
    rgb[0] = 255;
    rgb[1] = 105;
    rgb[2] = 180;
    return;
  }
  if (strcmp(color, "HOT PINK 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 110;
    rgb[2] = 180;
    return;
  }
  if (strcmp(color, "HOT PINK 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 106;
    rgb[2] = 167;
    return;
  }
  if (strcmp(color, "HOT PINK 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 96;
    rgb[2] = 144;
    return;
  }
  if (strcmp(color, "HOT PINK 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 58;
    rgb[2] = 98;
    return;
  }
  if (strcmp(color, "INDIAN RED") == 0) {
    rgb[0] = 205;
    rgb[1] = 92;
    rgb[2] = 92;
    return;
  }
  if (strcmp(color, "INDIAN RED 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 106;
    rgb[2] = 106;
    return;
  }
  if (strcmp(color, "INDIAN RED 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 99;
    rgb[2] = 99;
    return;
  }
  if (strcmp(color, "INDIAN RED 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 85;
    rgb[2] = 85;
    return;
  }
  if (strcmp(color, "INDIAN RED 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 58;
    rgb[2] = 58;
    return;
  }
  if (strcmp(color, "INDIGO") == 0) {
    rgb[0] = 75;
    rgb[1] = 0;
    rgb[2] = 130;
    return;
  }
  if (strcmp(color, "IVORY") == 0) {
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 240;
    return;
  }
  if (strcmp(color, "IVORY 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 238;
    rgb[2] = 224;
    return;
  }
  if (strcmp(color, "IVORY 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 205;
    rgb[2] = 193;
    return;
  }
  if (strcmp(color, "IVORY 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 139;
    rgb[2] = 131;
    return;
  }
  if (strcmp(color, "IVORY BLACK") == 0) {
    rgb[0] = 41;
    rgb[1] = 36;
    rgb[2] = 33;
    return;
  }
  if (strcmp(color, "KELLY GREEN") == 0) {
    rgb[0] = 0;
    rgb[1] = 128;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "KHAKI") == 0) {
    rgb[0] = 240;
    rgb[1] = 230;
    rgb[2] = 140;
    return;
  }
  if (strcmp(color, "KHAKI 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 246;
    rgb[2] = 143;
    return;
  }
  if (strcmp(color, "KHAKI 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 230;
    rgb[2] = 133;
    return;
  }
  if (strcmp(color, "KHAKI 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 198;
    rgb[2] = 115;
    return;
  }
  if (strcmp(color, "KHAKI 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 134;
    rgb[2] = 78;
    return;
  }
  if (strcmp(color, "LAVENDER") == 0) {
    rgb[0] = 230;
    rgb[1] = 230;
    rgb[2] = 250;
    return;
  }
  if (strcmp(color, "LAVENDER BLUSH") == 0) {
    rgb[0] = 255;
    rgb[1] = 240;
    rgb[2] = 245;
    return;
  }
  if (strcmp(color, "LAVENDER BLUSH 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 224;
    rgb[2] = 229;
    return;
  }
  if (strcmp(color, "LAVENDER BLUSH 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 193;
    rgb[2] = 197;
    return;
  }
  if (strcmp(color, "LAVENDER BLUSH 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 131;
    rgb[2] = 134;
    return;
  }
  if (strcmp(color, "LAWN GREEN") == 0) {
    rgb[0] = 124;
    rgb[1] = 252;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "LEMON CHIFFON") == 0) {
    rgb[0] = 255;
    rgb[1] = 250;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "LEMON CHIFFON 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 233;
    rgb[2] = 191;
    return;
  }
  if (strcmp(color, "LEMON CHIFFON 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 201;
    rgb[2] = 165;
    return;
  }
  if (strcmp(color, "LEMON CHIFFON 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 137;
    rgb[2] = 112;
    return;
  }
  if (strcmp(color, "LIGHT BLUE") == 0) {
    rgb[0] = 173;
    rgb[1] = 216;
    rgb[2] = 230;
    return;
  }
  if (strcmp(color, "LIGHT BLUE 1") == 0) {
    rgb[0] = 191;
    rgb[1] = 239;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "LIGHT BLUE 2") == 0) {
    rgb[0] = 178;
    rgb[1] = 223;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "LIGHT BLUE 3") == 0) {
    rgb[0] = 154;
    rgb[1] = 192;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "LIGHT BLUE 4") == 0) {
    rgb[0] = 104;
    rgb[1] = 131;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "LIGHT CORAL") == 0) {
    rgb[0] = 240;
    rgb[1] = 128;
    rgb[2] = 128;
    return;
  }
  if (strcmp(color, "LIGHT CYAN") == 0) {
    rgb[0] = 224;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "LIGHT CYAN 1") == 0) {
    rgb[0] = 209;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "LIGHT CYAN 2") == 0) {
    rgb[0] = 180;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "LIGHT CYAN 3") == 0) {
    rgb[0] = 122;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "LIGHT GOLDENROD") == 0) {
    rgb[0] = 255;
    rgb[1] = 236;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "LIGHT GOLDENROD 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 220;
    rgb[2] = 130;
    return;
  }
  if (strcmp(color, "LIGHT GOLDENROD 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 190;
    rgb[2] = 112;
    return;
  }
  if (strcmp(color, "LIGHT GOLDENROD 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 129;
    rgb[2] = 76;
    return;
  }
  if (strcmp(color, "LIGHT GOLDENROD YELLOW") == 0) {
    rgb[0] = 250;
    rgb[1] = 250;
    rgb[2] = 210;
    return;
  }
  if (strcmp(color, "LIGHT GREY") == 0) {
    rgb[0] = 211;
    rgb[1] = 211;
    rgb[2] = 211;
    return;
  }
  if (strcmp(color, "LIGHT PINK") == 0) {
    rgb[0] = 255;
    rgb[1] = 182;
    rgb[2] = 193;
    return;
  }
  if (strcmp(color, "LIGHT PINK 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 174;
    rgb[2] = 185;
    return;
  }
  if (strcmp(color, "LIGHT PINK 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 162;
    rgb[2] = 173;
    return;
  }
  if (strcmp(color, "LIGHT PINK 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 140;
    rgb[2] = 149;
    return;
  }
  if (strcmp(color, "LIGHT PINK 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 95;
    rgb[2] = 101;
    return;
  }
  if (strcmp(color, "LIGHT SALMON") == 0) {
    rgb[0] = 255;
    rgb[1] = 160;
    rgb[2] = 122;
    return;
  }
  if (strcmp(color, "LIGHT SALMON 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 149;
    rgb[2] = 114;
    return;
  }
  if (strcmp(color, "LIGHT SALMON 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 129;
    rgb[2] = 98;
    return;
  }
  if (strcmp(color, "LIGHT SALMON 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 87;
    rgb[2] = 66;
    return;
  }
  if (strcmp(color, "LIGHT SEA GREEN") == 0) {
    rgb[0] = 32;
    rgb[1] = 178;
    rgb[2] = 170;
    return;
  }
  if (strcmp(color, "LIGHT SKY BLUE") == 0) {
    rgb[0] = 135;
    rgb[1] = 206;
    rgb[2] = 250;
    return;
  }
  if (strcmp(color, "LIGHT SKY BLUE 1") == 0) {
    rgb[0] = 176;
    rgb[1] = 226;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "LIGHT SKY BLUE 2") == 0) {
    rgb[0] = 164;
    rgb[1] = 211;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "LIGHT SKY BLUE 3") == 0) {
    rgb[0] = 141;
    rgb[1] = 182;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "LIGHT SKY BLUE 4") == 0) {
    rgb[0] = 96;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "LIGHT SLATE BLUE") == 0) {
    rgb[0] = 132;
    rgb[1] = 112;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "LIGHT SLATE GRAY") == 0) {
    rgb[0] = 119;
    rgb[1] = 136;
    rgb[2] = 153;
    return;
  }
  if (strcmp(color, "LIGHT STEEL BLUE") == 0) {
    rgb[0] = 176;
    rgb[1] = 196;
    rgb[2] = 222;
    return;
  }
  if (strcmp(color, "LIGHT STEEL BLUE 1") == 0) {
    rgb[0] = 202;
    rgb[1] = 225;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "LIGHT STEEL BLUE 2") == 0) {
    rgb[0] = 188;
    rgb[1] = 210;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "LIGHT STEEL BLUE 3") == 0) {
    rgb[0] = 162;
    rgb[1] = 181;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "LIGHT STEEL BLUE 4") == 0) {
    rgb[0] = 110;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "LIGHT YELLOW 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 224;
    return;
  }
  if (strcmp(color, "LIGHT YELLOW 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 238;
    rgb[2] = 209;
    return;
  }
  if (strcmp(color, "LIGHT YELLOW 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 205;
    rgb[2] = 180;
    return;
  }
  if (strcmp(color, "LIGHT YELLOW 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 139;
    rgb[2] = 122;
    return;
  }
  if (strcmp(color, "LIME GREEN") == 0) {
    rgb[0] = 50;
    rgb[1] = 205;
    rgb[2] = 50;
    return;
  }
  if (strcmp(color, "LINEN") == 0) {
    rgb[0] = 250;
    rgb[1] = 240;
    rgb[2] = 230;
    return;
  }
  if (strcmp(color, "MAGENTA") == 0) {
    rgb[0] = 255;
    rgb[1] = 0;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "MAGENTA 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 0;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "MAGENTA 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 0;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "MAGENTA 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 0;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "MANGANESE BLUE") == 0) {
    rgb[0] = 3;
    rgb[1] = 168;
    rgb[2] = 158;
    return;
  }
  if (strcmp(color, "MAROON") == 0) {
    rgb[0] = 128;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "MAROON 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 52;
    rgb[2] = 179;
    return;
  }
  if (strcmp(color, "MAROON 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 48;
    rgb[2] = 167;
    return;
  }
  if (strcmp(color, "MAROON 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 41;
    rgb[2] = 144;
    return;
  }
  if (strcmp(color, "MAROON 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 28;
    rgb[2] = 98;
    return;
  }
  if (strcmp(color, "MEDIUM ORCHID") == 0) {
    rgb[0] = 186;
    rgb[1] = 85;
    rgb[2] = 211;
    return;
  }
  if (strcmp(color, "MEDIUM ORCHID 1") == 0) {
    rgb[0] = 224;
    rgb[1] = 102;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "MEDIUM ORCHID 2") == 0) {
    rgb[0] = 209;
    rgb[1] = 95;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "MEDIUM ORCHID 3") == 0) {
    rgb[0] = 180;
    rgb[1] = 82;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "MEDIUM ORCHID 4") == 0) {
    rgb[0] = 122;
    rgb[1] = 55;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "MEDIUM PURPLE") == 0) {
    rgb[0] = 147;
    rgb[1] = 112;
    rgb[2] = 219;
    return;
  }
  if (strcmp(color, "MEDIUM PURPLE 1") == 0) {
    rgb[0] = 171;
    rgb[1] = 130;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "MEDIUM PURPLE 2") == 0) {
    rgb[0] = 159;
    rgb[1] = 121;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "MEDIUM PURPLE 3") == 0) {
    rgb[0] = 137;
    rgb[1] = 104;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "MEDIUM PURPLE 4") == 0) {
    rgb[0] = 93;
    rgb[1] = 71;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "MEDIUM SEA GREEN") == 0) {
    rgb[0] = 60;
    rgb[1] = 179;
    rgb[2] = 113;
    return;
  }
  if (strcmp(color, "MEDIUM SLATE BLUE") == 0) {
    rgb[0] = 123;
    rgb[1] = 104;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "MEDIUM SPRING GREEN") == 0) {
    rgb[0] = 0;
    rgb[1] = 250;
    rgb[2] = 154;
    return;
  }
  if (strcmp(color, "MEDIUM TURQUOISE") == 0) {
    rgb[0] = 72;
    rgb[1] = 209;
    rgb[2] = 204;
    return;
  }
  if (strcmp(color, "MEDIUM VIOLET RED") == 0) {
    rgb[0] = 199;
    rgb[1] = 21;
    rgb[2] = 133;
    return;
  }
  if (strcmp(color, "MELON") == 0) {
    rgb[0] = 227;
    rgb[1] = 168;
    rgb[2] = 105;
    return;
  }
  if (strcmp(color, "MIDNIGHT BLUE") == 0) {
    rgb[0] = 25;
    rgb[1] = 25;
    rgb[2] = 112;
    return;
  }
  if (strcmp(color, "MINT") == 0) {
    rgb[0] = 189;
    rgb[1] = 252;
    rgb[2] = 201;
    return;
  }
  if (strcmp(color, "MINT CREAM") == 0) {
    rgb[0] = 245;
    rgb[1] = 255;
    rgb[2] = 250;
    return;
  }
  if (strcmp(color, "MISTY ROSE") == 0) {
    rgb[0] = 255;
    rgb[1] = 228;
    rgb[2] = 225;
    return;
  }
  if (strcmp(color, "MISTY ROSE 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 213;
    rgb[2] = 210;
    return;
  }
  if (strcmp(color, "MISTY ROSE 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 183;
    rgb[2] = 181;
    return;
  }
  if (strcmp(color, "MISTY ROSE 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 125;
    rgb[2] = 123;
    return;
  }
  if (strcmp(color, "MOCCASIN") == 0) {
    rgb[0] = 255;
    rgb[1] = 228;
    rgb[2] = 181;
    return;
  }
  if (strcmp(color, "NAVAJO WHITE") == 0) {
    rgb[0] = 255;
    rgb[1] = 222;
    rgb[2] = 173;
    return;
  }
  if (strcmp(color, "NAVAJO WHITE 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 207;
    rgb[2] = 161;
    return;
  }
  if (strcmp(color, "NAVAJO WHITE 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 179;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "NAVAJO WHITE 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 121;
    rgb[2] = 94;
    return;
  }
  if (strcmp(color, "NAVY") == 0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 128;
    return;
  }
  if (strcmp(color, "OLD LACE") == 0) {
    rgb[0] = 253;
    rgb[1] = 245;
    rgb[2] = 230;
    return;
  }
  if (strcmp(color, "OLIVE") == 0) {
    rgb[0] = 128;
    rgb[1] = 128;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "OLIVE DRAB") == 0) {
    rgb[0] = 192;
    rgb[1] = 255;
    rgb[2] = 62;
    return;
  }
  if (strcmp(color, "OLIVE DRAB 1") == 0) {
    rgb[0] = 179;
    rgb[1] = 238;
    rgb[2] = 58;
    return;
  }
  if (strcmp(color, "OLIVE DRAB 2") == 0) {
    rgb[0] = 154;
    rgb[1] = 205;
    rgb[2] = 50;
    return;
  }
  if (strcmp(color, "OLIVE DRAB 3") == 0) {
    rgb[0] = 105;
    rgb[1] = 139;
    rgb[2] = 34;
    return;
  }
  if (strcmp(color, "ORANGE") == 0) {
    rgb[0] = 255;
    rgb[1] = 128;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 165;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 154;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 133;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 90;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE RED") == 0) {
    rgb[0] = 255;
    rgb[1] = 69;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE RED 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 64;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE RED 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 55;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORANGE RED 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 37;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ORCHID") == 0) {
    rgb[0] = 218;
    rgb[1] = 112;
    rgb[2] = 214;
    return;
  }
  if (strcmp(color, "ORCHID 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 131;
    rgb[2] = 250;
    return;
  }
  if (strcmp(color, "ORCHID 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 122;
    rgb[2] = 233;
    return;
  }
  if (strcmp(color, "ORCHID 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 105;
    rgb[2] = 201;
    return;
  }
  if (strcmp(color, "ORCHID 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 71;
    rgb[2] = 137;
    return;
  }
  if (strcmp(color, "PALE GOLDENROD") == 0) {
    rgb[0] = 238;
    rgb[1] = 232;
    rgb[2] = 170;
    return;
  }
  if (strcmp(color, "PALE GREEN") == 0) {
    rgb[0] = 152;
    rgb[1] = 251;
    rgb[2] = 152;
    return;
  }
  if (strcmp(color, "PALE GREEN 1") == 0) {
    rgb[0] = 154;
    rgb[1] = 255;
    rgb[2] = 154;
    return;
  }
  if (strcmp(color, "PALE GREEN 2") == 0) {
    rgb[0] = 144;
    rgb[1] = 238;
    rgb[2] = 144;
    return;
  }
  if (strcmp(color, "PALE GREEN 3") == 0) {
    rgb[0] = 124;
    rgb[1] = 205;
    rgb[2] = 124;
    return;
  }
  if (strcmp(color, "PALE GREEN 4") == 0) {
    rgb[0] = 84;
    rgb[1] = 139;
    rgb[2] = 84;
    return;
  }
  if (strcmp(color, "PALE TURQUOISE") == 0) {
    rgb[0] = 187;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "PALE TURQUOISE 1") == 0) {
    rgb[0] = 174;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "PALE TURQUOISE 2") == 0) {
    rgb[0] = 150;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "PALE TURQUOISE 3") == 0) {
    rgb[0] = 102;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "PALE VIOLET RED") == 0) {
    rgb[0] = 219;
    rgb[1] = 112;
    rgb[2] = 147;
    return;
  }
  if (strcmp(color, "PALE VIOLET RED 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 130;
    rgb[2] = 171;
    return;
  }
  if (strcmp(color, "PALE VIOLET RED 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 121;
    rgb[2] = 159;
    return;
  }
  if (strcmp(color, "PALE VIOLET RED 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 104;
    rgb[2] = 137;
    return;
  }
  if (strcmp(color, "PALE VIOLET RED 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 71;
    rgb[2] = 93;
    return;
  }
  if (strcmp(color, "PAPAYA WHIP") == 0) {
    rgb[0] = 255;
    rgb[1] = 239;
    rgb[2] = 213;
    return;
  }
  if (strcmp(color, "PEACH PUFF") == 0) {
    rgb[0] = 255;
    rgb[1] = 218;
    rgb[2] = 185;
    return;
  }
  if (strcmp(color, "PEACH PUFF 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 203;
    rgb[2] = 173;
    return;
  }
  if (strcmp(color, "PEACH PUFF 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 175;
    rgb[2] = 149;
    return;
  }
  if (strcmp(color, "PEACH PUFF 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 119;
    rgb[2] = 101;
    return;
  }
  if (strcmp(color, "PEACOCK") == 0) {
    rgb[0] = 51;
    rgb[1] = 161;
    rgb[2] = 201;
    return;
  }
  if (strcmp(color, "PINK") == 0) {
    rgb[0] = 255;
    rgb[1] = 192;
    rgb[2] = 203;
    return;
  }
  if (strcmp(color, "PINK 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 181;
    rgb[2] = 197;
    return;
  }
  if (strcmp(color, "PINK 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 169;
    rgb[2] = 184;
    return;
  }
  if (strcmp(color, "PINK 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 145;
    rgb[2] = 158;
    return;
  }
  if (strcmp(color, "PINK 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 99;
    rgb[2] = 108;
    return;
  }
  if (strcmp(color, "PLUM") == 0) {
    rgb[0] = 221;
    rgb[1] = 160;
    rgb[2] = 221;
    return;
  }
  if (strcmp(color, "PLUM 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 187;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "PLUM 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 174;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "PLUM 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 150;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "PLUM 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 102;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "POWDER BLUE") == 0) {
    rgb[0] = 176;
    rgb[1] = 224;
    rgb[2] = 230;
    return;
  }
  if (strcmp(color, "PURPLE") == 0) {
    rgb[0] = 128;
    rgb[1] = 0;
    rgb[2] = 128;
    return;
  }
  if (strcmp(color, "PURPLE 1") == 0) {
    rgb[0] = 155;
    rgb[1] = 48;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "PURPLE 2") == 0) {
    rgb[0] = 145;
    rgb[1] = 44;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "PURPLE 3") == 0) {
    rgb[0] = 125;
    rgb[1] = 38;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "PURPLE 4") == 0) {
    rgb[0] = 85;
    rgb[1] = 26;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "RASPBERRY") == 0) {
    rgb[0] = 135;
    rgb[1] = 38;
    rgb[2] = 87;
    return;
  }
  if (strcmp(color, "RAW SIENNA") == 0) {
    rgb[0] = 199;
    rgb[1] = 97;
    rgb[2] = 20;
    return;
  }
  if (strcmp(color, "RED") == 0) {
    rgb[0] = 255;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "RED 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "RED 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "RED 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "ROSY BROWN") == 0) {
    rgb[0] = 188;
    rgb[1] = 143;
    rgb[2] = 143;
    return;
  }
  if (strcmp(color, "ROSY BROWN 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 193;
    rgb[2] = 193;
    return;
  }
  if (strcmp(color, "ROSY BROWN 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 180;
    rgb[2] = 180;
    return;
  }
  if (strcmp(color, "ROSY BROWN 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 155;
    rgb[2] = 155;
    return;
  }
  if (strcmp(color, "ROSY BROWN 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 105;
    rgb[2] = 105;
    return;
  }
  if (strcmp(color, "ROYAL BLUE") == 0) {
    rgb[0] = 65;
    rgb[1] = 105;
    rgb[2] = 225;
    return;
  }
  if (strcmp(color, "ROYAL BLUE 1") == 0) {
    rgb[0] = 72;
    rgb[1] = 118;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "ROYAL BLUE 2") == 0) {
    rgb[0] = 67;
    rgb[1] = 110;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "ROYAL BLUE 3") == 0) {
    rgb[0] = 58;
    rgb[1] = 95;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "ROYAL BLUE 4") == 0) {
    rgb[0] = 39;
    rgb[1] = 64;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "SALMON") == 0) {
    rgb[0] = 250;
    rgb[1] = 128;
    rgb[2] = 114;
    return;
  }
  if (strcmp(color, "SALMON 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 140;
    rgb[2] = 105;
    return;
  }
  if (strcmp(color, "SALMON 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 130;
    rgb[2] = 98;
    return;
  }
  if (strcmp(color, "SALMON 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 112;
    rgb[2] = 84;
    return;
  }
  if (strcmp(color, "SALMON 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 76;
    rgb[2] = 57;
    return;
  }
  if (strcmp(color, "SANDY BROWN") == 0) {
    rgb[0] = 244;
    rgb[1] = 164;
    rgb[2] = 96;
    return;
  }
  if (strcmp(color, "SAP GREEN") == 0) {
    rgb[0] = 48;
    rgb[1] = 128;
    rgb[2] = 20;
    return;
  }
  if (strcmp(color, "SEA GREEN") == 0) {
    rgb[0] = 84;
    rgb[1] = 255;
    rgb[2] = 159;
    return;
  }
  if (strcmp(color, "SEA GREEN 1") == 0) {
    rgb[0] = 78;
    rgb[1] = 238;
    rgb[2] = 148;
    return;
  }
  if (strcmp(color, "SEA GREEN 2") == 0) {
    rgb[0] = 67;
    rgb[1] = 205;
    rgb[2] = 128;
    return;
  }
  if (strcmp(color, "SEA GREEN 3") == 0) {
    rgb[0] = 46;
    rgb[1] = 139;
    rgb[2] = 87;
    return;
  }
  if (strcmp(color, "SEASHELL") == 0) {
    rgb[0] = 255;
    rgb[1] = 245;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "SEASHELL 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 229;
    rgb[2] = 222;
    return;
  }
  if (strcmp(color, "SEASHELL 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 197;
    rgb[2] = 191;
    return;
  }
  if (strcmp(color, "SEASHELL 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 134;
    rgb[2] = 130;
    return;
  }
  if (strcmp(color, "SEPIA") == 0) {
    rgb[0] = 94;
    rgb[1] = 38;
    rgb[2] = 18;
    return;
  }
  if (strcmp(color, "SIENNA") == 0) {
    rgb[0] = 160;
    rgb[1] = 82;
    rgb[2] = 45;
    return;
  }
  if (strcmp(color, "SIENNA 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 130;
    rgb[2] = 71;
    return;
  }
  if (strcmp(color, "SIENNA 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 121;
    rgb[2] = 66;
    return;
  }
  if (strcmp(color, "SIENNA 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 104;
    rgb[2] = 57;
    return;
  }
  if (strcmp(color, "SIENNA 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 71;
    rgb[2] = 38;
    return;
  }
  if (strcmp(color, "SILVER") == 0) {
    rgb[0] = 192;
    rgb[1] = 192;
    rgb[2] = 192;
    return;
  }
  if (strcmp(color, "SKY BLUE") == 0) {
    rgb[0] = 135;
    rgb[1] = 206;
    rgb[2] = 235;
    return;
  }
  if (strcmp(color, "SKY BLUE 1") == 0) {
    rgb[0] = 135;
    rgb[1] = 206;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "SKY BLUE 2") == 0) {
    rgb[0] = 126;
    rgb[1] = 192;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "SKY BLUE 3") == 0) {
    rgb[0] = 108;
    rgb[1] = 166;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "SKY BLUE 4") == 0) {
    rgb[0] = 74;
    rgb[1] = 112;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "SLATE BLUE") == 0) {
    rgb[0] = 106;
    rgb[1] = 90;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "SLATE BLUE 1") == 0) {
    rgb[0] = 131;
    rgb[1] = 111;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "SLATE BLUE 2") == 0) {
    rgb[0] = 122;
    rgb[1] = 103;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "SLATE BLUE 3") == 0) {
    rgb[0] = 105;
    rgb[1] = 89;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "SLATE BLUE 4") == 0) {
    rgb[0] = 71;
    rgb[1] = 60;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "SLATE GRAY") == 0) {
    rgb[0] = 112;
    rgb[1] = 128;
    rgb[2] = 144;
    return;
  }
  if (strcmp(color, "SLATE GRAY 1") == 0) {
    rgb[0] = 198;
    rgb[1] = 226;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "SLATE GRAY 2") == 0) {
    rgb[0] = 185;
    rgb[1] = 211;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "SLATE GRAY 3") == 0) {
    rgb[0] = 159;
    rgb[1] = 182;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "SLATE GRAY 4") == 0) {
    rgb[0] = 108;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "SNOW") == 0) {
    rgb[0] = 255;
    rgb[1] = 250;
    rgb[2] = 250;
    return;
  }
  if (strcmp(color, "SNOW 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 233;
    rgb[2] = 233;
    return;
  }
  if (strcmp(color, "SNOW 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 201;
    rgb[2] = 201;
    return;
  }
  if (strcmp(color, "SNOW 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 137;
    rgb[2] = 137;
    return;
  }
  if (strcmp(color, "SPRING GREEN") == 0) {
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 127;
    return;
  }
  if (strcmp(color, "SPRING GREEN 1") == 0) {
    rgb[0] = 0;
    rgb[1] = 238;
    rgb[2] = 118;
    return;
  }
  if (strcmp(color, "SPRING GREEN 2") == 0) {
    rgb[0] = 0;
    rgb[1] = 205;
    rgb[2] = 102;
    return;
  }
  if (strcmp(color, "SPRING GREEN 3") == 0) {
    rgb[0] = 0;
    rgb[1] = 139;
    rgb[2] = 69;
    return;
  }
  if (strcmp(color, "STEEL BLUE") == 0) {
    rgb[0] = 70;
    rgb[1] = 130;
    rgb[2] = 180;
    return;
  }
  if (strcmp(color, "STEEL BLUE 1") == 0) {
    rgb[0] = 99;
    rgb[1] = 184;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "STEEL BLUE 2") == 0) {
    rgb[0] = 92;
    rgb[1] = 172;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "STEEL BLUE 3") == 0) {
    rgb[0] = 79;
    rgb[1] = 148;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "STEEL BLUE 4") == 0) {
    rgb[0] = 54;
    rgb[1] = 100;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "TAN") == 0) {
    rgb[0] = 210;
    rgb[1] = 180;
    rgb[2] = 140;
    return;
  }
  if (strcmp(color, "TAN 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 165;
    rgb[2] = 79;
    return;
  }
  if (strcmp(color, "TAN 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 154;
    rgb[2] = 73;
    return;
  }
  if (strcmp(color, "TAN 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 133;
    rgb[2] = 63;
    return;
  }
  if (strcmp(color, "TAN 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 90;
    rgb[2] = 43;
    return;
  }
  if (strcmp(color, "TEAL") == 0) {
    rgb[0] = 0;
    rgb[1] = 128;
    rgb[2] = 128;
    return;
  }
  if (strcmp(color, "THISTLE") == 0) {
    rgb[0] = 216;
    rgb[1] = 191;
    rgb[2] = 216;
    return;
  }
  if (strcmp(color, "THISTLE 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 225;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "THISTLE 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 210;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "THISTLE 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 181;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "THISTLE 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "TOMATO") == 0) {
    rgb[0] = 255;
    rgb[1] = 99;
    rgb[2] = 71;
    return;
  }
  if (strcmp(color, "TOMATO 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 92;
    rgb[2] = 66;
    return;
  }
  if (strcmp(color, "TOMATO 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 79;
    rgb[2] = 57;
    return;
  }
  if (strcmp(color, "TOMATO 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 54;
    rgb[2] = 38;
    return;
  }
  if (strcmp(color, "TURQUOISE") == 0) {
    rgb[0] = 64;
    rgb[1] = 224;
    rgb[2] = 208;
    return;
  }
  if (strcmp(color, "TURQUOISE 1") == 0) {
    rgb[0] = 0;
    rgb[1] = 245;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "TURQUOISE 2") == 0) {
    rgb[0] = 0;
    rgb[1] = 229;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "TURQUOISE 3") == 0) {
    rgb[0] = 0;
    rgb[1] = 197;
    rgb[2] = 205;
    return;
  }
  if (strcmp(color, "TURQUOISE 4") == 0) {
    rgb[0] = 0;
    rgb[1] = 134;
    rgb[2] = 139;
    return;
  }
  if (strcmp(color, "TURQUOISE BLUE") == 0) {
    rgb[0] = 0;
    rgb[1] = 199;
    rgb[2] = 140;
    return;
  }
  if (strcmp(color, "VIOLET") == 0) {
    rgb[0] = 238;
    rgb[1] = 130;
    rgb[2] = 238;
    return;
  }
  if (strcmp(color, "VIOLET RED") == 0) {
    rgb[0] = 208;
    rgb[1] = 32;
    rgb[2] = 144;
    return;
  }
  if (strcmp(color, "VIOLET RED 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 62;
    rgb[2] = 150;
    return;
  }
  if (strcmp(color, "VIOLET RED 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 58;
    rgb[2] = 140;
    return;
  }
  if (strcmp(color, "VIOLET RED 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 50;
    rgb[2] = 120;
    return;
  }
  if (strcmp(color, "VIOLET RED 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 34;
    rgb[2] = 82;
    return;
  }
  if (strcmp(color, "WARM GREY") == 0) {
    rgb[0] = 128;
    rgb[1] = 128;
    rgb[2] = 105;
    return;
  }
  if (strcmp(color, "WHEAT") == 0) {
    rgb[0] = 245;
    rgb[1] = 222;
    rgb[2] = 179;
    return;
  }
  if (strcmp(color, "WHEAT 1") == 0) {
    rgb[0] = 255;
    rgb[1] = 231;
    rgb[2] = 186;
    return;
  }
  if (strcmp(color, "WHEAT 2") == 0) {
    rgb[0] = 238;
    rgb[1] = 216;
    rgb[2] = 174;
    return;
  }
  if (strcmp(color, "WHEAT 3") == 0) {
    rgb[0] = 205;
    rgb[1] = 186;
    rgb[2] = 150;
    return;
  }
  if (strcmp(color, "WHEAT 4") == 0) {
    rgb[0] = 139;
    rgb[1] = 126;
    rgb[2] = 102;
    return;
  }
  if (strcmp(color, "WHITE") == 0) {
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if (strcmp(color, "WHITE SMOKE") == 0) {
    rgb[0] = 245;
    rgb[1] = 245;
    rgb[2] = 245;
    return;
  }
  if (strcmp(color, "YELLOW") == 0) {
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "YELLOW 1") == 0) {
    rgb[0] = 238;
    rgb[1] = 238;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "YELLOW 2") == 0) {
    rgb[0] = 205;
    rgb[1] = 205;
    rgb[2] = 0;
    return;
  }
  if (strcmp(color, "YELLOW 3") == 0) {
    rgb[0] = 139;
    rgb[1] = 139;
    rgb[2] = 0;
  } else {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
  }
  return;
}
