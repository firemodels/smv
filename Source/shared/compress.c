#include "options_common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dmalloc.h"
#include "compress.h"

#define MARK 255

/* ------------------ CompressZLIB ------------------------ */

int CompressZLIB(unsigned char *dest, uLongf *destLen, unsigned char *source, int sourceLen){
  return compress(dest, destLen, source, sourceLen);
}

/* ------------------ UnCompressZLIB ------------------------ */

int UnCompressZLIB(unsigned char *dest, uLongf *destLen, unsigned char *source, int sourceLen){
  return uncompress(dest, destLen, source, sourceLen);
}

/* ------------------ CompressRLE ------------------------ */

unsigned int CompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out){
  unsigned char lastchar=MARK, cmark=MARK, thischar, *buffer_start;
  unsigned char *buffer_in_end;
  int nrepeats=1;

  buffer_start=buffer_out;
  buffer_in_end = buffer_in + nchars_in;

  while(buffer_in<buffer_in_end){
    thischar=*buffer_in;
    if(thischar==lastchar){
      nrepeats++;
    }
    else{
      nrepeats=1;
    }
    switch(nrepeats){
    case 1:
    case 2:
    case 3:
      *buffer_out=thischar;
      lastchar=thischar;
      break;
    default:
      if(nrepeats==4){
        buffer_out-=3;
        *buffer_out++=cmark;
        *buffer_out++=thischar;
      }
      if(nrepeats!=4)buffer_out--;
      *buffer_out=nrepeats;
      if(nrepeats==254){
        nrepeats=1;
        lastchar=MARK;
      }
      break;
    }
    buffer_in++;
    buffer_out++;

  }
  return buffer_out-buffer_start;
}

/* ------------------ UnCompressRLE ------------------------ */

unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out){
  int nrepeats,nn;
  unsigned char thischar, *buffer_in_end;

  nn=0;
  buffer_in_end  = buffer_in  + nchars_in;

  while(buffer_in<buffer_in_end){
    if(*buffer_in==MARK){
      if(buffer_in+2>=buffer_in_end)break;
      buffer_in++;
      thischar=*buffer_in++;
      nrepeats=*buffer_in++;
      nn+=nrepeats;
      memset(buffer_out,thischar,nrepeats);
      buffer_out+=nrepeats;
    }
    else{
      *buffer_out++=*buffer_in++;
      nn++;
    }


  }
  return nn;
}

/* ------------------ AllZeroRLE ------------------------ */

unsigned char AllZeroRLE(unsigned char *buffer_in, int nchars_in){
  unsigned char *buffer_in_end;

  buffer_in_end = buffer_in + nchars_in;
  while(buffer_in < buffer_in_end){
    if(*buffer_in == MARK){
      unsigned char thischar;

      if(buffer_in + 2 >= buffer_in_end)break;
      thischar = buffer_in[1];
      if(thischar > 0)return 0;
      buffer_in+=3;
    }
    else{
      if(*buffer_in > 0)return 0;
      buffer_in++;
    }
  }
  return 1;
}

/* ------------------ CompressVolSliceFrame ------------------------ */

void CompressVolSliceFrame(float *data_in, int n_data_in, float timeval_in, float *vmin_in, float *vmax_in,
                unsigned char **compressed_data_out, uLongf *ncompressed_data_out
                ){
  float valmin, valmax;
  int i;
  uLongf n_data_compressed,n_data_compressedm32;
  unsigned char *c_data, *c_data_compressed;
  int one=1;
  int version=0;
  int nbytes=1;

  // determine bounds
  CheckMemory;
  if(vmin_in==NULL){
    valmin=data_in[0];
    for(i=1;i<n_data_in;i++){
      if(data_in[i]<valmin)valmin=data_in[i];
    }
    CheckMemory;
  }
  else{
    valmin=*vmin_in;
  }
  CheckMemory;

  if(vmax_in==NULL){
    valmax=data_in[0];
    for(i=1;i<n_data_in;i++){
      if(data_in[i]>valmax)valmax=data_in[i];
    }
    CheckMemory;
  }
  else{
    valmax=*vmax_in;
    CheckMemory;
  }
  CheckMemory;

  // allocate buffers

  n_data_compressed = 1.1*(n_data_in+32) + 600;
  n_data_compressedm32=n_data_compressed-32;
  NewMemory((void **)&c_data,n_data_in);
  NewMemory((void **)&c_data_compressed,n_data_compressed*sizeof(unsigned char));

  // scale data

  if(valmax>valmin){
    for(i=0;i<n_data_in;i++){
      c_data[i]=255*(data_in[i]-valmin)/(valmax-valmin);
    }
    CheckMemory;
  }
  else{
    memset(c_data,0,n_data_in);
    CheckMemory;
  }

  //  compress data

  compress(c_data_compressed+32,&n_data_compressedm32, c_data, n_data_in);

  n_data_compressed=n_data_compressedm32+32;
  CheckMemory;
// 1,completion,version
// 1,version,n_data_compressed,nbytes,n_data_in,time,valmin,valmax,data ....
  memcpy(c_data_compressed,&one,4);
  memcpy(c_data_compressed+4,&version,4);
  memcpy(c_data_compressed+8,&n_data_compressed,4);
  memcpy(c_data_compressed+12,&nbytes,4);
  memcpy(c_data_compressed+16,&n_data_in,4);
  memcpy(c_data_compressed+20,&timeval_in,4);
  memcpy(c_data_compressed+24,&valmin,4);
  memcpy(c_data_compressed+28,&valmax,4);
  CheckMemory;

  // resize and deallocate buffers

  ResizeMemory((void **)&c_data_compressed, n_data_compressed);
  FREEMEMORY(c_data);
  *compressed_data_out=c_data_compressed;
  *ncompressed_data_out=n_data_compressed;
}

/* ------------------ UnCompressVolSliceFrame ------------------------ */

int UnCompressVolSliceFrame(unsigned char *compressed_data_in,
                           float *data_out, int n_data_in, float *timeval_out,
                           unsigned char *fullbuffer
                ){
  float valmin, valmax;
  int i,ndatafile;
  uLongf countin,countout;

  valmin=*(float *)(compressed_data_in+24);
  valmax=*(float *)(compressed_data_in+28);
  *timeval_out=*(float *)(compressed_data_in+20);
  countin = *(int *)(compressed_data_in+8)-32;
  ndatafile = *(int *)(compressed_data_in+16);

  uncompress(fullbuffer,&countout,compressed_data_in+32,countin);

  if(countout==ndatafile&&n_data_in>=countout){
    for(i=0;(uLongf)i<countout;i++){
      data_out[i]=valmin+fullbuffer[i]*(valmax-valmin)/255.0;
    }
  }
  return countout;
}

#ifndef IJKMAP
#define IJKMAP(i,j,k,nx,nxy) ((i)+(j)*nx+(k)*nxy)
#endif

/* ------------------ Compress3D ------------------------ */

unsigned char AverageCube(unsigned *buffer, int *ijk_start, int *ijk_end, int *nijk, unsigned char *minmax){
  unsigned char average=0;
  unsigned int faverage=0;
  int i, j, k;
  int nx = nijk[0], nxy = nijk[0] * nijk[1];
  int sum;

  nx = nijk[0];
  nxy = nijk[0]*nijk[1];
  sum  = (ijk_end[0] + 1 - ijk_start[0]);
  sum *= (ijk_end[1] + 1 - ijk_start[1]);
  sum *= (ijk_end[2] + 1 - ijk_start[2]);


  for(k = ijk_start[2]; k <= ijk_end[2]; k++){
    for(j = ijk_start[1]; j <= ijk_end[1]; j++){
      for(i = ijk_start[0]; i <= ijk_end[0]; i++){
        faverage += buffer[IJKMAP(i, j, k, nx, nxy)];
      }
    }
  }
  if(sum!=0)average = (unsigned char)(faverage/sum);
  return average;
}

/* ------------------ Reorder3D ------------------------ */

void Reorder3D(int **fds2smv, int **smv2fds, int *nijk){
  int i, j, k;
  int skipi, skipj, skipk, nsize, nx, nxy;
  int *state, *fds2smvptr = NULL, *smv2fdsptr = NULL;
  int count = 0;
  int nskipi, nskipj, nskipk;

  if(nijk[0] * nijk[1] * nijk[2] <= 0)return;

  skipi = nijk[0];
  skipj = nijk[1];
  skipk = nijk[2];

  nskipi = 1;
  for(;;){
    if(skipi <= 1)break;
    nskipi++;
    skipi /= 2;
  }
  nskipj = 1;
  for(;;){
    if(skipj <= 1)break;
    nskipj++;
    skipj /= 2;
  }
  nskipk = 1;
  for(;;){
    if(skipk <= 1)break;
    nskipk++;
    skipk /= 2;
  }

  nsize = nskipi * nskipj * nskipk;
  nx = nskipi;
  nxy = nskipi * nskipj;

  NewMemory(( void ** )&state, nsize);

  NewMemory(( void ** )&fds2smvptr, nsize);
  *fds2smv = fds2smvptr;

  NewMemory(( void ** )&smv2fdsptr, nsize);
  *smv2fds = smv2fdsptr;

  for(i = 0; i < nsize; i++){
    state[i] = 0;
    fds2smvptr[i] = -1;
    smv2fdsptr[i] = -1;
  }

  skipi = nijk[0];
  skipj = nijk[1];
  skipk = nijk[2];
  for(;;){
    for(k = 0; k < nijk[2]; k += skipk){
      for(j = 0; j < nijk[1]; j += skipj){
        for(i = 0; i < nijk[0]; i += skipi){
          int ijk;

          ijk = IJKMAP(i, j, k, nx, nxy);
          if(state[ijk] == 0){
            fds2smvptr[count] = ijk;
            smv2fdsptr[ijk] = count;
            count++;
          }
          else{
            state[ijk] = 1;
          }
        }
      }
    }
    if(skipi == 1 && skipj == 1 && skipk == 1)break;
    skipi = skipi/2;
    if(skipi < 1)skipi = 1;
    skipj = skipj/2;
    if(skipj < 1)skipj = 1;
    skipk = skipk/2;
    if(skipk < 1)skipk = 1;
  }
  for(i = 0; i < nsize; i++){
    assert(fds2smvptr[i] >= 0);
    assert(smv2fdsptr[i] >= 0);
  }

  FREEMEMORY(state);
}


/* --------------------------  octtreedata ------------------------------- */

typedef struct _octreedata {
  unsigned char val, valmin, valmax, mask, bottom;
  struct _octreedata *subtrees[8];
  int ijkmin[3], ijkmax[3], nsubtrees;
} octtreedata;

/* ------------------ InitOctTree ------------------------ */

octtreedata *InitOctTree(unsigned char *vals, int *nijk, unsigned char level, int *ijkmin, int *ijkmax){
  octtreedata *oi;
  int i,j,k;
  unsigned char valmin = 255, valmax = 0, val;
  unsigned int valsum = 0;
  int nx, ny, nxy, nz, nvals;

  nx = nijk[0];
  ny = nijk[1];
  nz = nijk[2];
  nxy = nx*ny;
  nvals = nx * ny * nz;

  valmin = vals[IJKMAP(ijkmin[0], ijkmin[1], ijkmin[2], nx, nxy)];
  valmax = valmin;

  NewMemory((void **)&oi, sizeof(octtreedata));

  memcpy(oi->ijkmin, ijkmin, 3 * sizeof(int));
  memcpy(oi->ijkmax, ijkmax, 3 * sizeof(int));

  for(k = ijkmin[2];k <= ijkmax[2];k++){
    for(j = ijkmin[1];1 <= ijkmax[1];j++){
      for(i = ijkmin[0];i <= ijkmax[0];i++){
        val = vals[IJKMAP(i,j,k, nx, nxy)];
        if(val < valmin)valmin = val;
        if(val > valmax)valmax = val;
        valsum += val;
      }
    }
  }
  if(valmax - valmin <= level){
    oi->bottom = 1;
  }
  else{
    oi->bottom = 0;
  }
  oi->mask = 0;
  oi->valmin = valmin;
  oi->valmax = valmax;
  oi->nsubtrees = 0;
  oi->val = valsum / nvals;
  for(i = 0;i < 8;i++){
    oi->subtrees[i] = NULL;
  }
  return oi;
}

/* ------------------ MakeSubOctTrees ------------------------ */

void MakeSubOctTrees(octtreedata *oi, unsigned char level, unsigned char *vals, int *nijk){
  int i, j, k;
  int ijkmin[3], ijkmax[3];
  unsigned char submask[8] = {1, 2, 4, 8,16,32,64,128};

//  nx = nijk[0];
//  nxy = nijk[0] * nijk[1];

  if(oi->bottom==1)return;
  if(ijkmin[0] == ijkmax[0] && ijkmin[1] == ijkmax[1] && ijkmin[2] == ijkmax[2])return;

  int index = 0;
  for(k = 0;k < 2;k++){
    if(k == 0){
      ijkmin[2] = oi->ijkmin[2];
      ijkmax[2] = (oi->ijkmin[2] + oi->ijkmax[2]) / 2;
    }
    else{
      ijkmin[2] = (oi->ijkmin[2] + oi->ijkmax[2]) /2 +1;
      ijkmax[2] = oi->ijkmax[2];
    }
    for(j = 0;j < 2;j++){
      if(j == 0){
        ijkmin[1] = oi->ijkmin[1];
        ijkmax[1] = (oi->ijkmin[1] + oi->ijkmax[1]) / 2;
      }
      else{
        ijkmin[1] = (oi->ijkmin[1] + oi->ijkmax[1]) / 2 +1;
        ijkmax[1] = oi->ijkmax[1];
      }
      for(i = 0;i < 2;i++){
        if(i == 0){
          ijkmin[0] = oi->ijkmin[0];
          ijkmax[0] = (oi->ijkmin[0] + oi->ijkmax[0]) / 2;
        }
        else{
          ijkmin[0] = (oi->ijkmin[0] + oi->ijkmax[0]) / 2 +1;
          ijkmax[0] = oi->ijkmax[0];
        }
        if(ijkmin[0] == ijkmax[0] && ijkmin[1] == ijkmax[1] && ijkmin[2] == ijkmax[2]){
          memcpy(oi->ijkmin, ijkmin, 3 * sizeof(int));
          memcpy(oi->ijkmax, ijkmax, 3 * sizeof(int));
          oi->subtrees[index] = InitOctTree(vals, nijk, level, ijkmin, ijkmax);
          oi->mask &= submask[index];
        }
        else{
          oi->subtrees[index] = NULL;
        }
        index++;
      }
    }
  }
}

/* ------------------ OctTreeCompress ------------------------ */

octtreedata *OctTreeCompress(unsigned char *vals, int *nijk, unsigned char level){
  int ijkmin[3], ijkmax[3];

  ijkmin[0] = 0;
  ijkmin[1] = 0;
  ijkmin[2] = 0;
  ijkmax[0] = nijk[0] - 1;
  ijkmax[1] = nijk[1] - 1;
  ijkmax[2] = nijk[2] - 1;

  return InitOctTree(vals, nijk, level, ijkmin, ijkmax);
}
