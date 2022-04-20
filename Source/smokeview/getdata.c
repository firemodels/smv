#include "options.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <share.h>
#endif
#include "MALLOCC.h"
#include "datadefs.h"
#include "getdata.h"
#include <ctype.h>
#include <errno.h>
#if __STDC_VERSION__ >= 201112L
#include <limits.h>
#endif

// As with the Fortran code preceding this, it is assumed that a float is 4
// bytes for serialization/deserialization purposes. With C11 we can check this
// using static assertions.
#if __STDC_VERSION__ >= 201112L
_Static_assert(CHAR_BIT == 8, "getdata.c assumes that CHAR_BIT == 8");
_Static_assert(sizeof(float) == 4, "getdata.c assumes that float is 4 bytes");
#endif

#ifdef WIN32
FILE *FOPEN(const char *file, const char *mode) {
  return _fsopen(file, mode, _SH_DENYNO);
}
#else
FILE *FOPEN(const char *file, const char *mode) { return fopen(file, mode); }
#endif

/// TrimFrontConst duplicated here due to dependency problems.
const char *TrimFrontConst_(const char *line) {
  for (const char *c = line; c <= line + strlen(line) - 1; c++) {
    if (!isspace((unsigned char)(*c))) return c;
  }
  return line;
}

/// TrimBack duplicated here due to dependency problems.
void TrimBack_(char *line) {
  if (line == NULL) return;
  size_t len = strlen(line);
  if (len == 0) return;
  for (char *c = line + len - 1; c >= line; c--) {
    if (isspace((unsigned char)(*c))) continue;
    *(c + 1) = '\0';
    return;
  }
  *line = '\0';
}

int fortread(void *ptr, size_t size, size_t count, FILE *file) {
  // TODO: check endianess, currently little-endian is assumed
  // Read record header
  uint32_t header = 0;
  size_t header_read = fread(&header, sizeof(header), 1, file);
  if (header_read == 0) {
    if (feof(file)) {
      return -1;
    } else {
      fprintf(stderr, "Value of errno: %d\n", errno);
      fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
    }
  }
  if (header_read != 1) return 1;
  if (header != (size * count)) {
    // TODO: need to decide if we should accept larger than expected records.
    fprintf(stderr, "Expected record of %zu bytes, found one of %u bytes\n",
            size * count, header);
    return 2;
  }

  size_t data_read = fread(ptr, size, count, file);
  if (data_read != count) return 3;

  // Read record trailer
  uint32_t trailer = 0;
  size_t trailer_read = fread(&trailer, sizeof(trailer), 1, file);
  if (trailer_read != 1) return 4;
  if (trailer != (size * count)) return 5;
  ASSERT(data_read == count);
  ASSERT(trailer == (size * count));
  return 0;
}

int fortwrite(void *ptr, size_t size, size_t count, FILE *file) {
  // TODO: check endianess
  // Read record header
  uint32_t header = size * count;
  size_t header_written = fwrite(&header, sizeof(header), 1, file);
  if (header_written == 0) {
    fprintf(stderr, "Value of errno: %d\n", errno);
    fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
  }
  if (header_written != 1) return 1;

  size_t data_written = fwrite(ptr, size, count, file);
  if (data_written != count) return 3;

  // Write record trailer
  uint32_t trailer = size * count;
  size_t trailer_written = fwrite(&trailer, sizeof(trailer), 1, file);
  if (trailer_written != 1) return 4;
  ASSERT(header == (size * count));
  ASSERT(data_written == count);
  ASSERT(trailer == (size * count));
  return 0;
}

int fortseek(FILE *file, size_t size, size_t count, int whence) {
  return fseek(file, sizeof(uint32_t) + size * count + sizeof(uint32_t),
               whence);
}

//  ------------------ getgeomdatasize ------------------------

void getgeomdatasize(const char *filename, int *ntimes, int *nvars,
                     int *error) {
  float time, dummy;
  int one, version;
  int nvert_s, nvert_d, nface_s, nface_d;
  FILE *file = FOPEN(filename, "rb");
  if (file == NULL) {
    printf(" The boundary element file name, %s does not exist\n", filename);
    *error = 1;
    return;
  }

  *error = 0;
  *error = fortread(&one, 1, 1, file);
  *error = fortread(&version, 1, 1, file);
  *ntimes = 0;
  *nvars = 0;
  size_t error_local;
  uint32_t counts[4] = {0};
  while (1) {
    error_local = fortread(&time, sizeof(time), 1, file);
    if (error_local == 0) {
      error_local = fortread(&counts, sizeof(*counts), 4, file);
      nvert_s = counts[0];
      nface_s = counts[1];
      nvert_d = counts[2];
      nface_d = counts[3];
    }
    if (error_local == 0 && nvert_s > 0)
      error_local = fortread(&dummy, sizeof(dummy), nvert_s, file);
    if (error_local == 0 && nvert_d > 0)
      error_local = fortread(&dummy, sizeof(dummy), nvert_d, file);
    if (error_local == 0 && nface_s > 0)
      error_local = fortread(&dummy, sizeof(dummy), nface_s, file);
    if (error_local == 0 && nface_d > 0)
      error_local = fortread(&dummy, sizeof(dummy), nface_d, file);
    if (error_local != 0) {
      break;
    }
    *nvars += nvert_s + nvert_d + nface_s + nface_d;
    ntimes++;
  }
  fclose(file);
}

//  ------------------ getzonesize ------------------------
void getzonesize(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, int *error) {
  int version;
  float dummy;
  int exit_all;

  *error = 0;

  FILE *file;
  file = FOPEN(zonefilename, "rb");

  if (file == NULL) {
    printf(" The zone file name, %s does not exist\n", zonefilename);
    *error = 1;
    return;
  }

  *nzonet = 0;
  *error = fortread(&version, sizeof(version), 1, file);
  if (*error == 0) fortread(&nrooms, sizeof(nrooms), 1, file);
  if (*error == 0) fortread(&nfires, sizeof(nfires), 1, file);
  if (*error != 0) {
    *error = 0;
    rewind(file);
    return;
  }
  while (1) {
    exit_all = 0;
    *error = fortread(&dummy, sizeof(dummy), 1, file);
    if (*error != 0) {
      *error = 0;
      break;
    }
    uint32_t dummies[4];
    for (int i = 0; i < *nrooms; i++) {
      *error = fortread(dummies, sizeof(*dummies), 4, file);
      if (*error == 0) continue;
      *error = 0;
      exit_all = 1;
      break;
    }
    if (exit_all == 1) break;
    // TODO: technically float is not fixed width here.
    float fdummies[2];
    for (int i = 0; i < *nfires; i++) {
      *error = fortread(fdummies, sizeof(*fdummies), 2, file);
      if (*error == 0) continue;
      *error = 0;
      exit_all = 1;
      break;
    }
    if (exit_all == 1) break;
    (*nzonet)++;
  }
  fclose(file);
}

// !  ------------------ getpatchsizes1 ------------------------

void getpatchsizes1(FILE **file, const char *patchfilename, int *npatch,
                    int *headersize, int *error) {

  *error = 0;
  *file = FOPEN(patchfilename, "rb");
  if (file == NULL) {
    printf(" The boundary file name, %s does not exist\n", patchfilename);
    *error = 1;
    return;
  }

  // skip over long, short and unit labels (each 30 characters in length);
  *error = fortseek(*file, sizeof(char), 30, SEEK_SET);
  *error = fortseek(*file, sizeof(char), 30, SEEK_CUR);
  *error = fortseek(*file, sizeof(char), 30, SEEK_CUR);
  if (*error == 0) fortread(npatch, sizeof(*npatch), 1, *file);
  *headersize = 3 * (4 + 30 + 4) + 4 + 4 + 4;

  return;
}

// !  ------------------ getpatchsizes2 ------------------------

void getpatchsizes2(FILE *file, int version, int npatch, int *npatchsize,
                    int *pi1, int *pi2, int *pj1, int *pj2, int *pk1, int *pk2,
                    int *patchdir, int *headersize, int *framesize) {
  uint32_t ijkp[7] = {0};

  *npatchsize = 0;
  for (int n = 0; n < npatch; n++) {
    if (version == 0) {
      fortread(ijkp, sizeof(*ijkp), 6, file);
    } else {
      fortread(ijkp, sizeof(*ijkp), 7, file);
      patchdir[n] = ijkp[6];
    }
    pi1[n] = ijkp[0];
    pi2[n] = ijkp[1];
    pj1[n] = ijkp[2];
    pj2[n] = ijkp[3];
    pk1[n] = ijkp[4];
    pk2[n] = ijkp[5];

    int i1 = ijkp[0];
    int i2 = ijkp[1];
    int j1 = ijkp[2];
    int j2 = ijkp[3];
    int k1 = ijkp[4];
    int k2 = ijkp[5];
    *npatchsize += (i2 + 1 - i1) * (j2 + 1 - j1) * (k2 + 1 - k1);
  }
  *headersize += npatch * (4 + 6 * 4 + 4);
  if (version == 1) {
    *headersize += npatch * 4;
  }
  *framesize = 8 + 4 + 8 * npatch + (*npatchsize) * 4;

  return;
}

// !  ------------------ getsliceparms ------------------------

void getsliceparms(const char *slicefilename, int *ip1, int *ip2, int *jp1,
                   int *jp2, int *kp1, int *kp2, int *ni, int *nj, int *nk,
                   int *slice3d, int *error) {
  int idir, joff, koff, volslice;
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
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

    FILE *file = FOPEN(slicefilename, "rb");
    if (file == NULL) {
      *error = 1;
      return;
    }
    *error = fortread(longlbl, 30, 1, file);
    *error = fortread(shortlbl, 30, 1, file);
    *error = fortread(unitlbl, 30, 1, file);

    uint32_t ijkp[6] = {0};
    *error = fortread(ijkp, sizeof(*ijkp), 6, file);
    *ip1 = ijkp[0];
    *ip2 = ijkp[1];
    *jp1 = ijkp[2];
    *jp2 = ijkp[3];
    *kp1 = ijkp[4];
    *kp2 = ijkp[5];
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

  *error = 0;
  FILE *file = FOPEN(slicefilename, "rb");
  if (file == NULL) {
    *error = 1;
    return;
  }
  // skip over long, short and unit labels (each 30 characters in length);
  *error = fortseek(file, sizeof(char), 30, SEEK_SET);
  *error = fortseek(file, sizeof(char), 30, SEEK_CUR);
  *error = fortseek(file, sizeof(char), 30, SEEK_CUR);

  uint32_t ijkp[6] = {0};
  *error = fortread(ijkp, sizeof(*ijkp), 6, file);
  *ip1 = ijkp[0];
  *ip2 = ijkp[1];
  *jp1 = ijkp[2];
  *jp2 = ijkp[3];
  *kp1 = ijkp[4];
  *kp2 = ijkp[5];

  fclose(file);
}

// !  ------------------ getslicesizes ------------------------

void getslicesizes(const char *slicefilename, int *nslicei, int *nslicej,
                   int *nslicek, int *nsteps, int sliceframestep, int *error,
                   int settmin_s, int settmax_s, float tmin_s, float tmax_s,
                   int *headersize, int *framesize) {
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int iip1, iip2;
  int nxsp, nysp, nzsp;

  float timeval, time_max;
  bool load; // boolean
  int idir, joff, koff, volslice;
  int count;

  *error = 0;
  *nsteps = 0;

  FILE *file = FOPEN(slicefilename, "rb");
  if (file == NULL) {
    *error = 1;
    return;
  }

  *headersize = 3 * (4 + 30 + 4);

  // skip over long, short and unit labels (each 30 characters in length);
  *error = fortseek(file, sizeof(char), 30, SEEK_SET);
  *error = fortseek(file, sizeof(char), 30, SEEK_CUR);
  *error = fortseek(file, sizeof(char), 30, SEEK_CUR);

  uint32_t ijkp[6] = {0};
  *error = fortread(ijkp, sizeof(*ijkp), 6, file);
  ip1 = ijkp[0];
  ip2 = ijkp[1];
  jp1 = ijkp[2];
  jp2 = ijkp[3];
  kp1 = ijkp[4];
  kp2 = ijkp[5];

  *headersize += 4 + 6 * 4 + 4;
  if (*error != 0) return;

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  getslicefiledirection(&ip1, &ip2, &iip1, &iip2, &jp1, &jp2, &kp1, &kp2, &idir,
                        &joff, &koff, &volslice);
  *nslicei = nxsp;
  *nslicej = nysp + joff;
  *nslicek = nzsp + koff;

  *framesize = 4 * (1 + nxsp * nysp * nzsp) + 16;

  count = -1;
  time_max = -1000000.0;
  while (1) {
    *error = fortread(&timeval, sizeof(timeval), 1, file);
    if (*error != 0) break;
    if ((settmin_s != 0 && timeval < tmin_s) || timeval <= time_max) {
      load = false;
    } else {
      load = true;
      time_max = timeval;
    }
    if (settmax_s != 0 && timeval > tmax_s) break;
    *error = fortseek(file, sizeof(float), nxsp * nysp * nzsp, SEEK_CUR);
    count = count + 1;
    if ((count % sliceframestep) != 0) load = false;
    if (*error != 0) break;
    if (load) (*nsteps)++;
  }
  *error = 0;
  fclose(file);
  return;
}

// !  ------------------ openpart ------------------------

FILE *openpart(const char *partfilename, int *error) {
  *error = 0;
  FILE *file = FOPEN(partfilename, "rb");
  if (file == NULL) {
    *error = 1;
  }
  return file;
}

// !  ------------------ openslice ------------------------

void openslice(const char *slicefilename, FILE **file, int *is1, int *is2,
               int *js1, int *js2, int *ks1, int *ks2, int *error) {
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  *error = 0;
  *file = FOPEN(slicefilename, "rb");
  if (*file == NULL) {
    *error = 1;
    return;
  }
  *error = fortread(longlbl, 30, 1, *file);
  *error = fortread(shortlbl, 30, 1, *file);
  *error = fortread(unitlbl, 30, 1, *file);

  uint32_t ijk[6] = {0};
  *error = fortread(ijk, sizeof(*ijk), 6, *file);
  *is1 = ijk[0];
  *is2 = ijk[1];
  *js1 = ijk[2];
  *js2 = ijk[3];
  *ks1 = ijk[4];
  *ks2 = ijk[5];

  return;
}

// !  ------------------ closefortranfile ------------------------

void closefortranfile(FILE *unit) {
  fclose(unit);
  return;
}

// !  ------------------ getboundaryheader1 ------------------------

void getboundaryheader1(const char *boundaryfilename, FILE **file, int *npatch,
                        int *error) {
  char patchlonglabel[31] = {0};
  char patchshortlabel[31] = {0};
  char patchunit[31] = {0};

  *error = 0;
  *file = FOPEN(boundaryfilename, "rb");
  if (*file == NULL) {
    printf(" The boundary file name, %s does not exist\n", boundaryfilename);
    *error = 1;
    return;
  }

  if (*error == 0) {
    *error = fortread(patchlonglabel, 30, 1, *file);
  }
  if (*error == 0) {
    *error = fortread(patchshortlabel, 30, 1, *file);
  }
  if (*error == 0) {
    *error = fortread(patchunit, 30, 1, *file);
  }
  if (*error == 0) {
    *error = fortread(npatch, sizeof(*npatch), 1, *file);
  }
  if (*error != 0) {
    fclose(*file);
  }

  return;
}

// !  ------------------ getboundaryheader2 ------------------------

void getboundaryheader2(FILE *file, int version, int npatch, int *pi1, int *pi2,
                        int *pj1, int *pj2, int *pk1, int *pk2, int *patchdir) {
  uint32_t ijk[9];
  for (int n = 0; n < npatch; n++) {
    if (version == 0) {
      fortread(ijk, sizeof(*ijk), 6, file);
    } else {
      fortread(ijk, sizeof(*ijk), 9, file);
      patchdir[n] = ijk[6];
    }
    pi1[n] = ijk[0];
    pi2[n] = ijk[1];
    pj1[n] = ijk[2];
    pj2[n] = ijk[3];
    pk1[n] = ijk[4];
    pk2[n] = ijk[5];
  }

  return;
}

// !  ------------------ openboundary ------------------------

FILE *openboundary(const char *boundaryfilename, int version, int *error) {
  char patchlonglabel[31] = {0};
  char patchshortlabel[31] = {0};
  char patchunit[31] = {0};
  *error = 0;
  int npatch;
  FILE *file = FOPEN(boundaryfilename, "rb");
  if (file == NULL) {
    printf(" The boundary file name, %s does not exist\n", boundaryfilename);
    *error = 1;
    return file;
  }

  *error = fortread(patchlonglabel, 30, 1, file);
  if (*error != 0) goto end;

  *error = fortread(patchshortlabel, 30, 1, file);
  if (*error != 0) goto end;

  *error = fortread(patchunit, 30, 1, file);
  if (*error != 0) goto end;

  *error = fortread(&npatch, sizeof(npatch), 1, file);
  if (*error != 0) goto end;

  uint32_t ijk[9] = {0};
  for (int n = 0; n < npatch; n++) {
    if (version == 0) {
      if (*error == 0) {
        *error = fortread(ijk, sizeof(*ijk), 6, file);
        if (*error != 0) goto end;
      }
    } else {
      if (*error == 0) {
        *error = fortread(ijk, sizeof(*ijk), 9, file);
        if (*error) goto end;
      }
    }
  }

end:
  if (*error != 0) {
    fclose(file);
    file = NULL;
  }

  return file;
}

// !  ------------------ getpartheader1 ------------------------

void getpartheader1(FILE *file, int *nclasses, int *fdsversion, int *size) {
  int one;

  fortread(&one, sizeof(one), 1, file);
  fortread(fdsversion, sizeof(*fdsversion), 1, file);
  fortread(nclasses, sizeof(*nclasses), 1, file);

  *size = 12;

  return;
}

// !  ------------------ getpartheader2 ------------------------

void getpartheader2(FILE *file, int nclasses, int *nquantities, int *size) {
  char clabel[31] = {0};

  *size = 0;

  for (int i = 0; i < nclasses; i++) {
    int t[2] = {0};
    fortread(t, sizeof(*t), 2, file);
    nquantities[i] = t[0];
    *size += 4 + 2 * nquantities[i] * (4 + 30 + 4);
    for (int j = 0; j < nquantities[i]; j++) {
      fortread(clabel, 30, 1, file);
      fortread(clabel, 30, 1, file);
    }
  }

  return;
}

// !  ------------------ getpartdataframe ------------------------

void getpartdataframe(FILE *file, int nclasses, int *nquantities, int *npoints,
                      float *time, int *tagdata, float *pdata, int *size,
                      int *error) {
  int pstart, pend;
  int tagstart, tagend;
  int nparticles;

  *size = 0;
  pend = 0;
  tagend = 0;
  *error = 0;
  *error = fortread(time, sizeof(*time), 1, file);
  *size = 4;
  if (*error != 0) return;
  for (int i = 0; i < nclasses; i++) {
    *error = fortread(&nparticles, sizeof(nparticles), 1, file);
    if (*error != 0) return;
    npoints[i] = nparticles;

    pstart = pend + 1;
    pend = pstart + 3 * nparticles - 1;
    *error = fortread(&pdata[pstart], sizeof(pdata[0]), pend - pstart, file);
    if (*error != 0) return;

    tagstart = tagend + 1;
    tagend = tagstart + nparticles - 1;
    *error = fortread(&tagdata[tagstart], sizeof(tagdata[0]), tagend - tagstart,
                      file);
    if (*error != 0) return;

    if (nquantities[i] > 0) {
      pstart = pend + 1;
      pend = pstart + nparticles * nquantities[i] - 1;
      *error = fortread(&pdata[pstart], sizeof(pdata[0]), pend - pstart, file);
      if (*error != 0) return;
    }
    *size += 4 + (4 * 3 * nparticles) + 4 * nparticles +
             4 * nparticles * nquantities[i];
  }
  *error = 0;
  return;
}

// !  ------------------ getgeomdata ------------------------

void getgeomdata(const char *filename, int ntimes, int nvals, float *times,
                 int *nstatics, int *ndynamics, float *vals, int *file_size,
                 int *error) {
  int one, nvars;
  int nvert_s, ntri_s, nvert_d, ntri_d;
  int version;

  *file_size = 0;
  FILE *file = FOPEN(filename, "rb");
  if (file == NULL) {
    printf(" The boundary element file name, %s does not exist\n", filename);
    *error = 1;
    return;
  }

  *error = 0;
  *error = fortread(&one, 1, 1, file);
  if (*error != 0) goto end;
  *error = fortread(&version, sizeof(version), 1, file);
  if (*error != 0) goto end;
  *file_size = 2 * (4 + 4 + 4);
  nvars = 0;
  for (int itime = 0; itime < ntimes; itime++) {

    *error = fortread(&times[itime], sizeof(times[itime]), 1, file);
    if (*error != 0) goto end;
    *file_size += (4 + 4 + 4);

    int counts[4] = {0};
    *error = fortread(&counts, sizeof(*counts), 4, file);
    nvert_s = counts[0];
    ntri_s = counts[1];
    nvert_d = counts[2];
    ntri_d = counts[3];
    *file_size += (4 + 4 * 4 + 4);
    nstatics[itime] = nvert_s + ntri_s;

    if (nvert_s > 0) {
      *error = fortread(&vals[nvars], sizeof(vals[nvars]), nvert_s, file);
      if (*error != 0) goto end;
      *file_size += (4 + 4 * nvert_s + 4);
    }
    nvars = nvars + nvert_s;

    if (ntri_s > 0) {
      *error = fortread(&vals[nvars], sizeof(vals[nvars]), ntri_s, file);
      if (*error != 0) goto end;
      *file_size += (4 + 4 * ntri_s + 4);
    }
    nvars = nvars + ntri_s;

    ndynamics[itime] = nvert_d + ntri_d;
    if (nvert_d > 0) {
      *error = fortread(&vals[nvars], sizeof(vals[nvars]), nvert_d, file);
      if (*error != 0) goto end;
      file_size += (4 + 4 * nvert_d + 4);
    }
    nvars = nvars + nvert_d;

    if (ntri_d > 0) {
      *error = fortread(&vals[nvars], sizeof(vals[nvars]), ntri_d, file);
      if (*error != 0) goto end;
      *file_size += (4 + 4 * ntri_d + 4);
    }
    nvars = nvars + ntri_d;
  }

end:
  fclose(file);
  return;
}

// !  ------------------ getzonedata ------------------------

void getzonedata(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, float *zonet, float *zoneqfire, float *zonepr,
                 float *zoneylay, float *zonetl, float *zonetu, int *error) {
  int ii, ii2, idummy, version;
  float qdot;

  FILE *file = FOPEN(zonefilename, "rb");
  if (file == NULL) {
    printf(" The zone file name, %s does not exist\n", zonefilename);
    *error = 1;
    return;
  }
  *error = fortread(&version, sizeof(version), 1, file);
  *error = fortread(&idummy, sizeof(idummy), 1, file);
  *error = fortread(&idummy, sizeof(idummy), 1, file);
  ii = 0;
  ii2 = 0;
  for (int j = 0; j < *nzonet; j++) {
    *error = fortread(&zonet[j], sizeof(zonet[j]), 1, file);
    for (int i = 0; i < *nrooms; i++) {
      float zonevals[4];
      *error = fortread(zonevals, sizeof(*zonevals), 4, file);
      zonepr[ii] = zonevals[0];
      zoneylay[ii] = zonevals[1];
      zonetl[ii] = zonevals[2];
      zonetu[ii] = zonevals[3];
      ii++;
      if (*error != 0) {
        *error = 1;
        *nzonet = j;
        fclose(file);
        return;
      }
    }
    for (int i = 1; i <= *nfires; i++) {
      ii2 = ii2 + 1;
      float qdot_arr[2];
      *error = fortread(&qdot_arr, sizeof(*qdot_arr), 2, file);
      qdot = qdot_arr[1];
      zoneqfire[ii2] = qdot;
      if (*error != 0) {
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

int skipdata(FILE *file, int skip) { return fortseek(file, 1, skip, SEEK_CUR); }

// !  ------------------ getpatchdata ------------------------
// TODO: distinguish between more error conditions
void getpatchdata(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                  int *pj2, int *pk1, int *pk2, float *patchtime, float *pqq,
                  int *npqq, int *file_size, int *error) {
  int i1, i2, j1, j2, k1, k2, size, ibeg;
  file_size = 0;
  *error = 0;
  *error = fortread(patchtime, sizeof(*patchtime), 1, file);
  file_size = file_size + 4;
  if (*error != 0) {
    fclose(file);
    return;
  }
  ibeg = 0;
  *npqq = 0;
  for (int i = 0; i < npatch; i++) {
    i1 = pi1[i];
    i2 = pi2[i];
    j1 = pj1[i];
    j2 = pj2[i];
    k1 = pk1[i];
    k2 = pk2[i];
    size = (i2 + 1 - i1) * (j2 + 1 - j1) * (k2 + 1 - k1);
    *npqq += size;
    *error = fortread(&pqq[ibeg], sizeof(*pqq), size, file);
    // TODO: hardcodes float size.
    file_size += 4 * size;
    if (*error != 0) {
      // TODO: this function shouldn't close this file.
      fclose(file);
      break;
    }
    ibeg += size;
  }
  return;
}

// !  ------------------ getdata1 ------------------------

void getdata1(FILE *file, int *ipart, int *error) {
  int nspr, nv;
  int nb1;
  *error = 0;

  uint32_t s_arr[5];
  *error = fortread(s_arr, sizeof(*s_arr), 5, file);

  if (*error != 0) return;
  uint32_t ijk[3] = {0};
  *error = fortread(ijk, sizeof(*ijk), 3, file);
  int ibar = ijk[0];
  int jbar = ijk[1];
  int kbar = ijk[2];

  if (*error != 0) return;

  // Skip over irrelevant data.
  *error = fortseek(file, sizeof(float), ibar * jbar * kbar, SEEK_CUR);
  if (*error != 0) return;

  *error = fortread(&nb1, sizeof(nb1), 1, file);
  if (*error != 0) return;

  int idummy[7];
  for (int i = 1; i <= nb1; i++) {
    *error = fortread(idummy, sizeof(*idummy), 7, file);
    if (*error != 0) return;
  }

  *error = fortread(&nv, sizeof(nv), 1, file);
  if (*error != 0) return;

  for (int i = 1; i <= nv; i++) {
    *error = fortread(idummy, sizeof(*idummy), 7, file);
    if (*error != 0) return;
  }

  *error = fortread(&nspr, sizeof(nspr), 1, file);
  if (*error != 0) return;

  float dummy[3];
  for (int i = 1; i <= nspr; i++) {
    *error = fortread(dummy, sizeof(*dummy), 3, file);
    if (*error != 0) return;
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
  *joff = 0;
  *koff = 0;
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
                    int js2, int ks1, int ks2, float *qdata, float *times,
                    int ntimes, int redirect_flag) {
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  int ibeg, iend, nframe;
  int nxsp, nysp, nzsp;

  FILE *file = FOPEN(slicefilename, "wb");

  strncpy(longlbl, " ", 30);
  strncpy(shortlbl, " ", 30);
  strncpy(unitlbl, " ", 30);

  fortwrite(longlbl, 30, 1, file);
  fortwrite(shortlbl, 30, 1, file);
  fortwrite(unitlbl, 30, 1, file);

  uint32_t ijk[6] = {0};
  ijk[0] = is1;
  ijk[1] = is2;
  ijk[2] = js1;
  ijk[3] = js2;
  ijk[4] = ks1;
  ijk[5] = ks2;
  fortwrite(ijk, sizeof(*ijk), 6, file);

  nxsp = is2 + 1 - is1;
  nysp = js2 + 1 - js1;
  nzsp = ks2 + 1 - ks1;
  nframe = nxsp * nysp * nzsp;
  if (redirect_flag == 0) printf("output slice data to %s\n", slicefilename);
  for (int i = 0; i < ntimes; i++) {
    fortwrite(&times[i], sizeof(times[i]), 1, file);
    ibeg = 1 + (i)*nframe;
    iend = (i + 1) * nframe;
    fortwrite(&qdata[ibeg], sizeof(*qdata), iend - ibeg, file);
  }

  fclose(file);

  return;
}

// !  ------------------ writeslicedata2 ------------------------

void writeslicedata2(const char *slicefilename, const char *longlabel,
                     const char *shortlabel, const char *unitlabel, int is1,
                     int is2, int js1, int js2, int ks1, int ks2, float *qdata,
                     float *times, int ntimes) {
  char longlabel30[31] = {0};
  char shortlabel30[31] = {0};
  char unitlabel30[31] = {0};
  int ibeg, iend, nframe;

  FILE *file = FOPEN(slicefilename, "wb");

  // Copy labels into smaller buffers, truncating if necessary. The separate
  // TrimFront and TrimBack steps are intentionally separate steps.
  strncpy(longlabel30, TrimFrontConst_(longlabel), 30);
  TrimBack_(longlabel30);
  strncpy(shortlabel30, TrimFrontConst_(shortlabel), 30);
  TrimBack_(shortlabel30);
  strncpy(unitlabel30, TrimFrontConst_(unitlabel), 30);
  TrimBack_(unitlabel30);

  fortwrite(longlabel30, 30, 1, file);
  fortwrite(shortlabel30, 30, 1, file);
  fortwrite(unitlabel30, 30, 1, file);

  uint32_t ijk[6] = {0};
  ijk[0] = is1;
  ijk[1] = is2;
  ijk[2] = js1;
  ijk[3] = js2;
  ijk[4] = ks1;
  ijk[5] = ks2;
  fortwrite(ijk, sizeof(*ijk), 6, file);

  int nxsp = is2 + 1 - is1;
  int nysp = js2 + 1 - js1;
  int nzsp = ks2 + 1 - ks1;
  nframe = nxsp * nysp * nzsp;
  for (int i = 0; i < ntimes; i++) {
    fortwrite(&times[i], sizeof(times[i]), 1, file);
    ibeg = 1 + (i - 1) * nframe;
    iend = i * nframe;
    fortwrite(&qdata[ibeg], sizeof(qdata[ibeg]), iend - ibeg, file);
  }

  fclose(file);

  return;
}

// !  ------------------ getsliceframe ------------------------

void getsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, float *time, float *qframe, int testslice,
                   int *error) {
  size_t nxsp = is2 + 1 - is1;
  size_t nysp = js2 + 1 - js1;
  size_t nzsp = ks2 + 1 - ks1;

  *error = fortread(time, sizeof(*time), 1, file);
  if (*error != 0) return;
  *error = fortread(qframe, sizeof(*qframe), nxsp * nysp * nzsp, file);
  if (testslice == 1 || testslice == 2) {
    float factor = 1.0;
    if (testslice == 2) factor = 1.1;
    for (size_t k = 0; k < nzsp; k++) {
      float kk = 2.0 * ((nzsp - 1) / 2.0 - k) / (nzsp - 1.0);
      for (size_t j = 0; j < nysp; j++) {
        float jj = 2.0 * ((nysp - 1) / 2.0 - j) / (nysp - 1.0);
        for (size_t i = 0; i < nxsp; i++) {
          float ii = 2.0 * ((nxsp - 1) / 2.0 - i) / (nxsp - 1.0);
          float val =
              factor * (*time - 20.0) * (ii * ii + jj * jj + kk * kk) / 20.0;
          size_t index = 1 + i + j * nxsp + k * nxsp * nysp;
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
  fortwrite(&one, sizeof(one), 1, file);
  fclose(file);
  return;
}

// !  ------------------ outsliceheader ------------------------

void outsliceheader(const char *slicefilename, FILE **file, int ip1, int ip2,
                    int jp1, int jp2, int kp1, int kp2, int *error) {
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  *file = FOPEN(slicefilename, "wb");

  strncpy(longlbl, "long                          ", 31);
  strncpy(shortlbl, "short                         ", 31);
  strncpy(unitlbl, "unit                          ", 31);

  *error = fortwrite(longlbl, 30, 1, *file);
  *error = fortwrite(shortlbl, 30, 1, *file);
  *error = fortwrite(unitlbl, 30, 1, *file);

  uint32_t ijk[6] = {0};
  ijk[0] = ip1;
  ijk[1] = ip2;
  ijk[2] = jp1;
  ijk[3] = jp2;
  ijk[4] = kp1;
  ijk[5] = kp2;
  *error = fortwrite(ijk, sizeof(*ijk), 6, *file);
  return;
}

// !  ------------------ outsliceframe ------------------------

void outsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, float time, float *qframe, int *error) {
  int nxsp = is2 + 1 - is1;
  int nysp = js2 + 1 - js1;
  int nzsp = ks2 + 1 - ks1;

  *error = fortwrite(&time, sizeof(time), 1, file);
  if (*error != 0) return;
  *error = fortwrite(qframe, sizeof(*qframe), nxsp * nysp * nzsp, file);
  return;
}

// !  ------------------ outboundaryheader ------------------------

void outboundaryheader(const char *boundaryfilename, FILE **file, int npatches,
                       int *pi1, int *pi2, int *pj1, int *pj2, int *pk1,
                       int *pk2, int *patchdir, int *error) {
  char blank[31];

  *error = 0;
  *file = FOPEN(boundaryfilename, "wb");

  strncpy(blank, "                              ", 31);
  *error = fortwrite(blank, 30, 1, *file);
  *error = fortwrite(blank, 30, 1, *file);
  *error = fortwrite(blank, 30, 1, *file);
  *error = fortwrite(&npatches, sizeof(npatches), 1, *file);

  uint32_t ijk[7] = {0};
  for (int n = 0; n < npatches; n++) {
    ijk[0] = pi1[n];
    ijk[1] = pi2[n];
    ijk[2] = pj1[n];
    ijk[3] = pj2[n];
    ijk[4] = pk1[n];
    ijk[5] = pk2[n];
    ijk[5] = patchdir[n];
    *error = fortwrite(ijk, sizeof(*ijk), 7, *file);
  }

  return;
}

// !  ------------------ outpatchframe ------------------------

void outpatchframe(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                   int *pj2, int *pk1, int *pk2, float patchtime, float *pqq,
                   int *error) {
  int i1, i2, j1, j2, k1, k2, size, ibeg, iend;
  *error = fortwrite(&patchtime, sizeof(patchtime), 1, file);
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
    *error = fortwrite(&pqq[ibeg], sizeof(pqq[ibeg]), iend - ibeg, file);
    ibeg = iend + 1;
  }
  return;
}

// !  ------------------ getplot3dq ------------------------

void getplot3dq(const char *qfilename, int nx, int ny, int nz, float *qq,
                int *error, int isotest) {
  float dummies[4];
  float dummy, qval;

  uint32_t npts[3] = {0};
  if (isotest == 0) {
    *error = 0;
    FILE *file = FOPEN(qfilename, "rb");
    if (file == NULL) {
      printf(" The file name, %s does not exist\n", qfilename);
      *error = fortread(&dummy, sizeof(dummy), 1, file);
      exit(1);
    }
    *error = fortread(npts, sizeof(*npts), 3, file);
    uint32_t nxpts = npts[0];
    uint32_t nypts = npts[1];
    uint32_t nzpts = npts[2];
    if (nx == nxpts && ny == nypts && nz == nzpts) {
      // TODO: determine what these values actually are.
      *error = fortread(dummies, sizeof(*dummies), 4, file);
      *error = fortread(qq, sizeof(*qq), nxpts * nypts * nzpts * 5, file);

    } else {
      *error = 1;
      printf(" *** Fatal error in getplot3dq ***\n");
      printf(" Grid size found in plot3d file was: %d,%d,%d\n", nxpts, nypts,
             nzpts);
      printf(" Was expecting: %d,%d,%d\n", nx, ny, nz);
      exit(1);
    }
    fclose(file);
  } else {
    for (int i = 0; i < nx; i++) {
      for (int j = 0; j < ny; j++) {
        for (int k = 0; k < nz; k++) {
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
  return;
}

// !  ------------------ plot3dout ------------------------

void plot3dout(const char *outfile, int nx, int ny, int nz, float *qout,
               int *error) {
  FILE *file = FOPEN(outfile, "wb");

  uint32_t nxyz[3];
  nxyz[0] = nx;
  nxyz[1] = ny;
  nxyz[2] = nz;
  *error = fortwrite(nxyz, sizeof(*nxyz), 3, file);

  float dummies[4] = {0.0};
  *error = fortwrite(dummies, sizeof(*dummies), 4, file);

  *error = fortwrite(qout, sizeof(*qout), nx * ny * nz * 5, file);
  fclose(file);

  return;
}
