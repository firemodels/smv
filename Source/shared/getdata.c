#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <share.h>
// #include <windows.h>
#endif
#include "dmalloc.h"
#include "datadefs.h"
#include "getdata.h"
#include "string_util.h"
#include "file_util.h"
#include <ctype.h>
#include <errno.h>
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L
#include <limits.h>
#endif
#endif

// As with the Fortran code preceding this, it is assumed that a float is 4
// bytes for serialization/deserialization purposes. With C11 we can check this
// using static assertions.
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L
_Static_assert(CHAR_BIT == 8, "getdata.c assumes that CHAR_BIT == 8");
_Static_assert(sizeof(float) == 4, "getdata.c assumes that float is 4 bytes");
#endif
#endif

//  ------------------ fortread ------------------------

int fortread(void *ptr, size_t size, size_t count, FILE *file){
  // TODO: check endianess, currently little-endian is assumed
  // Read record header
  uint32_t header = 0;
  size_t header_read = fread(&header, sizeof(header), 1, file);
  if(header_read == 0){
    if(feof(file)){
      return -1;
    } else{
      fprintf(stderr, "Value of errno: %d\n", errno);
      fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
    }
  }
  if(header_read != 1) return 1;
  if(header != (size * count)){
    // TODO: need to decide if we should accept larger than expected records.
    fprintf(stderr, "Expected record of %zu bytes, found one of %u bytes\n",
            size * count, header);
    return 2;
  }

  size_t data_read = fread(ptr, size, count, file);
  if(data_read != count) return 3;

  // Read record trailer
  uint32_t trailer = 0;
  size_t trailer_read = fread(&trailer, sizeof(trailer), 1, file);
  if(trailer_read != 1) return 4;
  if(trailer != (size * count)) return 5;
  assert(data_read == count);
  assert(trailer == (size * count));
  return 0;
}

//  ------------------ fortwrite ------------------------

int fortwrite(void *ptr, size_t size, size_t count, FILE *file){
  // TODO: check endianess
  // Read record header
  uint32_t header = size * count;
  size_t header_written = fwrite(&header, sizeof(header), 1, file);
  if(header_written == 0){
    fprintf(stderr, "Value of errno: %d\n", errno);
    fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
  }
  if(header_written != 1) return 1;

  size_t data_written = fwrite(ptr, size, count, file);
  if(data_written != count) return 3;

  // Write record trailer
  uint32_t trailer = size * count;
  size_t trailer_written = fwrite(&trailer, sizeof(trailer), 1, file);
  if(trailer_written != 1) return 4;
  assert(header == (size * count));
  assert(data_written == count);
  assert(trailer == (size * count));
  return 0;
}

//  ------------------ fortseek ------------------------

int fortseek(FILE *file, size_t size, size_t count, int whence){
  return fseek(file, sizeof(uint32_t) + size * count + sizeof(uint32_t),
               whence);
}

//  ------------------ getzonesize ------------------------

void getzonesize(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, int *error){
  int version;
  float dummy;
  int exit_all;

  *error = 0;

  FILE *file;
  file = FOPEN(zonefilename, "rb");

  if(file == NULL){
    fprintf(stderr, " The zone file name, %s does not exist\n", zonefilename);
    *error = 1;
    return;
  }

  *nzonet = 0;
  *error = fortread(&version, sizeof(version), 1, file);
  if(*error == 0) fortread(&nrooms, sizeof(nrooms), 1, file);
  if(*error == 0) fortread(&nfires, sizeof(nfires), 1, file);
  if(*error != 0){
    *error = 0;
    rewind(file);
    return;
  }
  while(1){
    exit_all = 0;
    *error = fortread(&dummy, sizeof(dummy), 1, file);
    if(*error != 0){
      *error = 0;
      break;
    }
    uint32_t dummies[4];

    int i;
    for(i = 0; i < *nrooms; i++){
      *error = fortread(dummies, sizeof(*dummies), 4, file);
      if(*error == 0) continue;
      *error = 0;
      exit_all = 1;
      break;
    }
    if(exit_all == 1) break;
    // TODO: technically float is not fixed width here.
    float fdummies[2];
    for(i = 0; i < *nfires; i++){
      *error = fortread(fdummies, sizeof(*fdummies), 2, file);
      if(*error == 0) continue;
      *error = 0;
      exit_all = 1;
      break;
    }
    if(exit_all == 1) break;
    (*nzonet)++;
  }
  fclose(file);
}

// !  ------------------ getpatchsizes1 ------------------------

void getpatchsizes1(FILE **file, const char *patchfilename, int *npatch,
                    int *headersize, int *error){

  *error = 0;
  assert(file!=NULL);
  if(file == NULL){
    fprintf(stderr, "***Error: null pointer in getpatchsizes1 routine\n");
    *error = 1;
    return;
  }
  *file = FOPEN(patchfilename, "rb");
  if(*file == NULL){
    fprintf(stderr, " The boundary file name, %s does not exist\n", patchfilename);
    *error = 1;
    return;
  }

  // skip over long, short and unit labels (each 30 characters in length);
  *error = fortseek(*file, sizeof(char), 30, SEEK_SET);
  *error = fortseek(*file, sizeof(char), 30, SEEK_CUR);
  *error = fortseek(*file, sizeof(char), 30, SEEK_CUR);
  if(*error == 0) fortread(npatch, sizeof(*npatch), 1, *file);
  *headersize = 3 * (4 + 30 + 4) + 4 + 4 + 4;

  return;
}

// !  ------------------ getpatchsizes2 ------------------------

void getpatchsizes2(FILE *file, int version, int npatch, int *npatchsize,
                    int *pi1, int *pi2, int *pj1, int *pj2, int *pk1, int *pk2,
                    int *patchdir, int *headersize, int *framesize){
  uint32_t ijkp[9] = {0};

  *npatchsize = 0;

  int n;
  for(n = 0; n < npatch; n++){
    if(version == 0){
      fortread(ijkp, sizeof(*ijkp), 6, file);
    } else{
      fortread(ijkp, sizeof(*ijkp), 9, file);
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
  if(version == 1){
    *headersize += npatch * 4;
  }
  *framesize = 8 + 4 + 8 * npatch + (*npatchsize) * 4;

  return;
}

/* ------------------ GetSliceFileDirection ------------------------ */

void GetSliceFileDirection(int is1, int *is2ptr, int *iis1ptr, int *iis2ptr, int js1, int *js2ptr, int ks1, int *ks2ptr, int *idirptr, int *joffptr, int *koffptr, int *volsliceptr){
  int nxsp, nysp, nzsp;
  int imin;

  nxsp = *is2ptr + 1 - is1;
  nysp = *js2ptr + 1 - js1;
  nzsp = *ks2ptr + 1 - ks1;
  *joffptr = 0;
  *koffptr = 0;
  *volsliceptr = 0;
  *iis1ptr = is1;
  *iis2ptr = *is2ptr;
  if(is1 != *is2ptr && js1 != *js2ptr && ks1 != *ks2ptr){
    *idirptr = 1;
    *is2ptr = is1;
    *volsliceptr = 1;
    return;
  }
  imin = MIN(nxsp, nysp);
  imin = MIN(imin, nzsp);
  if(nxsp == imin){
    *idirptr = 1;
    *is2ptr = is1;
  }
  else if(nysp == imin){
    *idirptr = 2;
    *js2ptr = js1;
  }
  else{
    *idirptr = 3;
    *ks2ptr = ks1;
  }
  if(is1 == *is2ptr && js1 == *js2ptr){
    *idirptr = 1;
    *joffptr = 1;
  }
  else if(is1 == *is2ptr && ks1 == *ks2ptr){
    *idirptr = 1;
    *koffptr = 1;
  }
  else if(js1 == *js2ptr && ks1 == *ks2ptr){
    *idirptr = 2;
    *koffptr = 1;
  }
}

/* ------------------ GetSliceParms ------------------------ */

void GetSliceParms(const char *slicefilename, int *ip1, int *ip2, int *jp1,
  int *jp2, int *kp1, int *kp2, int *ni, int *nj, int *nk,
  int *slice3d, int *error){
  int idir, joff, koff, volslice;
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  int iip1, iip2;

  if(*ip1 == -1 || *ip2 == -1 || *jp1 == -1 || *jp2 == -1 || *kp1 == -1 ||
    *kp2 == -1){
    *ip1 = 0;
    *ip2 = 0;
    *jp1 = 0;
    *jp2 = 0;
    *kp1 = 0;
    *kp2 = 0;
    *error = 0;

    FILE *file = FOPEN(slicefilename, "rb");
    if(file == NULL){
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
  if(ip1 == ip2 || jp1 == jp2 || kp1 == kp2){
    *slice3d = 0;
  }
  else{
    *slice3d = 1;
  }
  GetSliceFileDirection(*ip1, ip2, &iip1, &iip2, *jp1, jp2, *kp1, kp2, &idir, &joff, &koff, &volslice);
  return;
}

// !  ------------------ openpart ------------------------

FILE *openpart(const char *partfilename, int *error){
  *error = 0;
  FILE *file = FOPEN(partfilename, "rb");
  if(file == NULL){
    *error = 1;
  }
  return file;
}

// !  ------------------ openslice ------------------------

void openslice(const char *slicefilename, FILE **file, int *is1, int *is2,
               int *js1, int *js2, int *ks1, int *ks2, int *error){
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  *error = 0;
  *file = FOPEN(slicefilename, "rb");
  if(*file == NULL){
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

void closefortranfile(FILE *unit){
  fclose(unit);
  return;
}

// !  ------------------ getboundaryheader1 ------------------------

void getboundaryheader1(const char *boundaryfilename, FILE **file, int *npatch,
                        int *error){
  char patchlonglabel[31] = {0};
  char patchshortlabel[31] = {0};
  char patchunit[31] = {0};

  *error = 0;
  *file = FOPEN(boundaryfilename, "rb");
  if(*file == NULL){
    fprintf(stderr, " The boundary file name, %s does not exist\n", boundaryfilename);
    *error = 1;
    return;
  }

  if(*error == 0){
    *error = fortread(patchlonglabel, 30, 1, *file);
  }
  if(*error == 0){
    *error = fortread(patchshortlabel, 30, 1, *file);
  }
  if(*error == 0){
    *error = fortread(patchunit, 30, 1, *file);
  }
  if(*error == 0){
    *error = fortread(npatch, sizeof(*npatch), 1, *file);
  }
  if(*error != 0){
    fclose(*file);
  }

  return;
}

// !  ------------------ getboundaryheader2 ------------------------

void getboundaryheader2(FILE *file, int version, int npatch, int *pi1, int *pi2,
                        int *pj1, int *pj2, int *pk1, int *pk2, int *patchdir){
  uint32_t ijk[9];

  int n;
  for(n = 0; n < npatch; n++){
    if(version == 0){
      fortread(ijk, sizeof(*ijk), 6, file);
    } else{
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

FILE *openboundary(const char *boundaryfilename, int version, int *error){
  char patchlonglabel[31] = {0};
  char patchshortlabel[31] = {0};
  char patchunit[31] = {0};
  *error = 0;
  int npatch;
  FILE *file = FOPEN(boundaryfilename, "rb");
  if(file == NULL){
    fprintf(stderr, " The boundary file name, %s does not exist\n", boundaryfilename);
    *error = 1;
    return file;
  }

  *error = fortread(patchlonglabel, 30, 1, file);
  if(*error != 0) goto end;

  *error = fortread(patchshortlabel, 30, 1, file);
  if(*error != 0) goto end;

  *error = fortread(patchunit, 30, 1, file);
  if(*error != 0) goto end;

  *error = fortread(&npatch, sizeof(npatch), 1, file);
  if(*error != 0) goto end;

  uint32_t ijk[9] = {0};

  int n;
  for(n = 0; n < npatch; n++){
    if(version == 0){
      if(*error == 0){
        *error = fortread(ijk, sizeof(*ijk), 6, file);
        if(*error != 0) goto end;
      }
    } else{
      if(*error == 0){
        *error = fortread(ijk, sizeof(*ijk), 9, file);
        if(*error) goto end;
      }
    }
  }

end:
  if(*error != 0){
    fclose(file);
    file = NULL;
  }

  return file;
}

// !  ------------------ getpartheader1 ------------------------

void getpartheader1(FILE *file, int *nclasses, int *fdsversion, int *size){
  int one;

  fortread(&one, sizeof(one), 1, file);
  fortread(fdsversion, sizeof(*fdsversion), 1, file);
  fortread(nclasses, sizeof(*nclasses), 1, file);

  *size = 12;

  return;
}

// !  ------------------ getpartheader2 ------------------------

void getpartheader2(FILE *file, int nclasses, int *nquantities, int *size){
  char clabel[31] = {0};

  *size = 0;

  int i;
  for(i = 0; i < nclasses; i++){
    int t[2] = {0};
    fortread(t, sizeof(*t), 2, file);
    nquantities[i] = t[0];
    *size += 4 + 2 * nquantities[i] * (4 + 30 + 4);

    int j;
    for(j = 0; j < nquantities[i]; j++){
      fortread(clabel, 30, 1, file);
      fortread(clabel, 30, 1, file);
    }
  }

  return;
}

// !  ------------------ getpartdataframe ------------------------

void getpartdataframe(FILE *file, int nclasses, int *nquantities, int *npoints,
                      float *time, int *tagdata, float *pdata, int *size,
                      int *error){
  int nparticles;

  *size = 0;
  int pstart = 0;
  int tagstart = 0;
  *error = 0;
  *error = fortread(time, sizeof(*time), 1, file);
  *size = 4;
  if(*error != 0) return;

  int i;
  for(i = 0; i < nclasses; i++){
    *error = fortread(&nparticles, sizeof(nparticles), 1, file);
    if(*error != 0) return;
    npoints[i] = nparticles;

    // get the particle data
    int nvalues = 3 * nparticles;
    *error = fortread(&pdata[pstart], sizeof(*pdata), nvalues, file);
    if(*error != 0) return;
    pstart += nvalues;

    // get tag data
    int ntagvalues = tagstart + nparticles;
    *error = fortread(&tagdata[tagstart], sizeof(*tagdata), ntagvalues, file);
    if(*error != 0) return;
    tagstart += ntagvalues;

    if(nquantities[i] > 0){
      int nvalues2 = nparticles * nquantities[i];
      *error = fortread(&pdata[pstart], sizeof(*pdata), nvalues2, file);
      if(*error != 0) return;
      pstart += nvalues2;
    }
    *size += 4 + (4 * 3 * nparticles) + 4 * nparticles +
             4 * nparticles * nquantities[i];
  }
  *error = 0;
  return;
}

// !  ------------------ getzonedata ------------------------

void getzonedata(const char *zonefilename, int *nzonet, int *nrooms,
                 int *nfires, float *zonet, float *zoneqfire, float *zonepr,
                 float *zoneylay, float *zonetl, float *zonetu, int *error){
  int ii, ii2, idummy, version;
  float qdot;

  FILE *file = FOPEN(zonefilename, "rb");
  if(file == NULL){
    fprintf(stderr, " The zone file name, %s does not exist\n", zonefilename);
    *error = 1;
    return;
  }
  *error = fortread(&version, sizeof(version), 1, file);
  *error = fortread(&idummy, sizeof(idummy), 1, file);
  *error = fortread(&idummy, sizeof(idummy), 1, file);
  ii = 0;
  ii2 = 0;

  int j;
  for(j = 0; j < *nzonet; j++){
    *error = fortread(&zonet[j], sizeof(zonet[j]), 1, file);

    int i;
    for(i = 0; i < *nrooms; i++){
      float zonevals[4];
      *error = fortread(zonevals, sizeof(*zonevals), 4, file);
      zonepr[ii] = zonevals[0];
      zoneylay[ii] = zonevals[1];
      zonetl[ii] = zonevals[2];
      zonetu[ii] = zonevals[3];
      ii++;
      if(*error != 0){
        *error = 1;
        *nzonet = j;
        fclose(file);
        return;
      }
    }
    for(i = 1; i <= *nfires; i++){
      ii2 = ii2 + 1;
      float qdot_arr[2];
      *error = fortread(&qdot_arr, sizeof(*qdot_arr), 2, file);
      qdot = qdot_arr[1];
      zoneqfire[ii2] = qdot;
      if(*error != 0){
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

// !  ------------------ getpatchdata ------------------------
// TODO: distinguish between more error conditions
void getpatchdata(FILE *file, int npatch, int *pi1, int *pi2, int *pj1,
                  int *pj2, int *pk1, int *pk2, float *patchtime, float *pqq,
                  int *npqq, int *file_sizeptr, int *error){
  int i1, i2, j1, j2, k1, k2, size, ibeg;
  int file_size;

  file_size = 0;
  *error = 0;
  *error = fortread(patchtime, sizeof(*patchtime), 1, file);
  file_size = file_size + 4;
  if(*error != 0) return;
  ibeg = 0;
  *npqq = 0;

  int i;
  for(i = 0; i < npatch; i++){
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
    if(*error != 0) break;
    ibeg += size;
  }
  *file_sizeptr = file_size;
  return;
}

// !  ------------------ getdata1 ------------------------

void getdata1(FILE *file, int *ipart, int *error){
  int nspr, nv;
  int nb1;
  *error = 0;

  uint32_t s_arr[5];
  *error = fortread(s_arr, sizeof(*s_arr), 5, file);

  if(*error != 0) return;
  uint32_t ijk[3] = {0};
  *error = fortread(ijk, sizeof(*ijk), 3, file);
  int ibar = ijk[0];
  int jbar = ijk[1];
  int kbar = ijk[2];

  if(*error != 0) return;

  // Skip over irrelevant data.
  *error = fseek(file, 4+sizeof(float)*ibar*jbar*kbar+4, SEEK_CUR);
  if(*error != 0) return;

  *error = fortread(&nb1, sizeof(nb1), 1, file);
  if(*error != 0) return;

  int idummy[7];

  int i;
  for(i = 1; i <= nb1; i++){
    *error = fortread(idummy, sizeof(*idummy), 7, file);
    if(*error != 0) return;
  }

  *error = fortread(&nv, sizeof(nv), 1, file);
  if(*error != 0) return;
  for(i = 1; i <= nv; i++){
    *error = fortread(idummy, sizeof(*idummy), 7, file);
    if(*error != 0) return;
  }

  *error = fortread(&nspr, sizeof(nspr), 1, file);
  if(*error != 0) return;

  float dummy[3];
  for(i = 1; i <= nspr; i++){
    *error = fortread(dummy, sizeof(*dummy), 3, file);
    if(*error != 0) return;
  }

  return;
}

// !  ------------------ writeslicedata ------------------------

void writeslicedata(const char *slicefilename, int is1, int is2, int js1,
                    int js2, int ks1, int ks2, float *qdata, float *times,
                    int ntimes, int redirect_flag){
  char longlbl[31] = {0};
  char shortlbl[31] = {0};
  char unitlbl[31] = {0};
  int nframe;
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
  if(redirect_flag == 0) fprintf(stderr, "output slice data to %s\n", slicefilename);
  int i;
  for(i = 0; i < ntimes; i++){
    fortwrite(times + i, sizeof(float), 1, file);
    fortwrite(qdata + i*nframe, sizeof(float), nframe, file);
  }

  fclose(file);

  return;
}

// !  ------------------ getsliceframe ------------------------

void getsliceframe(FILE *file, int is1, int is2, int js1, int js2, int ks1,
                   int ks2, float *time, float *qframe, int testslice,
                   int *error){
  size_t nxsp = is2 + 1 - is1;
  size_t nysp = js2 + 1 - js1;
  size_t nzsp = ks2 + 1 - ks1;

  *error = fortread(time, sizeof(*time), 1, file);
  if(*error != 0) return;
  *error = fortread(qframe, sizeof(*qframe), nxsp * nysp * nzsp, file);
  if(testslice == 1 || testslice == 2){
    float factor = 1.0;
    if(testslice == 2) factor = 1.1;

    size_t k;
    for(k = 0; k < nzsp; k++){
      float kk = 2.0 * ((nzsp - 1) / 2.0 - k) / (nzsp - 1.0);

      size_t j;
      for(j = 0; j < nysp; j++){
        float jj = 2.0 * ((nysp - 1) / 2.0 - j) / (nysp - 1.0);

	size_t i;
        for(i = 0; i < nxsp; i++){
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

// !  ------------------ outsliceheader ------------------------

void outsliceheader(const char *slicefilename, FILE **file, int ip1, int ip2,
                    int jp1, int jp2, int kp1, int kp2, int *error){
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
                   int ks2, float time, float *qframe, int *error){
  int nxsp = is2 + 1 - is1;
  int nysp = js2 + 1 - js1;
  int nzsp = ks2 + 1 - ks1;

  *error = fortwrite(&time, sizeof(time), 1, file);
  if(*error != 0) return;
  *error = fortwrite(qframe, sizeof(*qframe), nxsp * nysp * nzsp, file);
  return;
}

// !  ------------------ outboundaryheader ------------------------

void outboundaryheader(const char *boundaryfilename, FILE **file, int npatches,
                       int *pi1, int *pi2, int *pj1, int *pj2, int *pk1,
                       int *pk2, int *patchdir, int *error){
  char blank[31];

  *error = 0;
  *file = FOPEN(boundaryfilename, "wb");
  if(*file == NULL) {
    fprintf(stderr, " Could not open %s\n", boundaryfilename);
    *error = 1;
    return;
  }
  strncpy(blank, "                              ", 31);
  *error = fortwrite(blank, 30, 1, *file);
  *error = fortwrite(blank, 30, 1, *file);
  *error = fortwrite(blank, 30, 1, *file);
  *error = fortwrite(&npatches, sizeof(npatches), 1, *file);

  uint32_t ijk[7] = {0};

  int n;
  for(n = 0; n < npatches; n++){
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
                   int *error){
  int i1, i2, j1, j2, k1, k2, size, ibeg, iend;
  *error = fortwrite(&patchtime, sizeof(patchtime), 1, file);
  ibeg = 1;

  int i;
  for(i = 0; i < npatch; i++){
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

// !  ------------------ getplot3dq ------------------------ TODO: we don't need
// to pass in the nx, ny, and nz values. This previously allowed us to allocate
// prior to this function.
void getplot3dq(const char *qfilename, int nx, int ny, int nz, float *qq, float *qmin, float *qmax,
                int *error, int isotest){
  float qval;

  if(qmin != NULL && qmax != NULL){
    int i;

    for(i = 0;i < 6;i++){
      qmin[i] = 0.0;
      qmax[i] = 1.0;
    }
  }
  uint32_t npts[3] = {0};
  if(isotest == 0){
    *error = 0;
    FILE *file = FOPEN(qfilename, "rb");
    if(file == NULL){
      fprintf(stderr, " The file name, %s does not exist\n", qfilename);
      exit(1);
    }
    *error = fortread(npts, sizeof(*npts), 3, file);
    if(*error) goto end;
    uint32_t nxpts = npts[0];
    uint32_t nypts = npts[1];
    uint32_t nzpts = npts[2];
    if(nx == nxpts && ny == nypts && nz == nzpts){
      float zeros[4];
      *error = fortread(zeros, sizeof(*zeros), 4, file);
      if(*error) goto end;
      *error = fortread(qq, sizeof(*qq), nxpts * nypts * nzpts * 5, file);
      if(*error) goto end;
    } else{
      *error = 1;
      fprintf(stderr, " *** Fatal error in getplot3dq ***\n");
      fprintf(stderr, " Grid size found in plot3d file was: %d,%d,%d\n", (int)nxpts, (int)nypts, (int)nzpts);
      fprintf(stderr, " Was expecting: %d,%d,%d\n", nx, ny, nz);
      exit(1);
    }
  end:
    fclose(file);
  }
  else{
    int i;
    for(i = 0; i < nx; i++){

      int j;
      for(j = 0; j < ny; j++){

        int k;
        for(k = 0; k < nz; k++){
          qval = pow(i - nx / 2, 2) + pow(j - ny / 2, 2) + pow(k - nz / 2, 2);
          qval = sqrt(qval);
          if(isotest == 1){
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 1] = 0.0;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 2] = 0.0;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 3] = qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 4] = qval;
            qq[i * nx * ny * nz + j * ny * nz + k * nz + 5] = qval;
          }
          if(isotest == 2){
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
  if(qmin != NULL && qmax != NULL){
    int i, j;

    for(i = 0;i < 5;i++){
      float *qqq;

      qqq = qq + i*nx*ny*nz;
      qmin[i] = qqq[0];
      qmax[i] = qqq[0];
      for(j = 1;j < nx * ny * nz;j++){
        if(qqq[j] < qmin[i])qmin[i] = qqq[j];
        if(qqq[j] > qmax[i])qmax[i] = qqq[j];
      }
    }
    float *u, *v, *w;
    // assume u, v, w components of velocity are 2nd, 3rd and 4th variables in the plot3d file
    u = qq + nx*ny*nz;
    v = qq + 2*nx*ny*nz;
    w = qq + 3*nx*ny*nz;
    qmin[5] = sqrt(u[0] * u[0] + v[0] * v[0] + w[0] * w[0]);
    qmax[5] = qmin[5];
    for(j = 1;j < nx*ny*nz;j++){
      float speed;

      speed = sqrt(u[j]*u[j] + v[j]*v[j] + w[j]*w[j]);
      if(speed < qmin[5])qmin[5] = speed;
      if(speed > qmax[5])qmax[5] = speed;
    }
  }
}

// !  ------------------ plot3dout ------------------------

void plot3dout(const char *outfile, int nx, int ny, int nz, float *qout,
               int *error){
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
