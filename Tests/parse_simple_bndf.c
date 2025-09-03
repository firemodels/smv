#include "options.h"
// TODO: sort out imports
#include "getdata.h"
#include "dmalloc.h"
#include <stdint.h>
#include <stdlib.h>

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  if (argc < 3) return 2;
  int error = 0;
  char *filename = argv[1];
  size_t n_expected_frames = atol(argv[2]);
  int boundary_version = 1;
  int *i1 = NULL;
  int *i2 = NULL;
  int *j1 = NULL;
  int *j2 = NULL;
  int *k1 = NULL;
  int *k2 = NULL;
  int *patchdir = NULL;
  FILE *file;
  int npatches = 0;
  int headersize = 0;
  int npatchsize = 0;
  int framesize = 0;
  getpatchsizes1(&file, filename, &npatches, &headersize, &error);

  NewMemory((void **)&i1, npatches * sizeof(*i1));
  NewMemory((void **)&i2, npatches * sizeof(*i2));
  NewMemory((void **)&j1, npatches * sizeof(*j1));
  NewMemory((void **)&j2, npatches * sizeof(*j2));
  NewMemory((void **)&k1, npatches * sizeof(*k1));
  NewMemory((void **)&k2, npatches * sizeof(*k2));
  NewMemory((void **)&patchdir, npatches * sizeof(*k2));

  getpatchsizes2(file, boundary_version, npatches, &npatchsize, i1, i2, j1, j2,
                 k1, k2, patchdir, &headersize, &framesize);
  fclose(file);

  if (error) return error;
  file = openboundary(filename, boundary_version, &error);
  if (error) return error;
  float patchtime = 0.0;
  int file_size = 0;
  float *patchdata;
  int npatchdata = 0;
  int nsize = 0;
  for (int i = 0; i < npatches; i++) {
    int size = (i2[i] - i1[i] + 1) * (j2[i] - j1[i] + 1) * (k2[i] - k1[i] + 1);
    printf("patch[%d] %d-%d %d-%d %d-%d (%d)\n", i, i1[i], i2[i], j1[i], j2[i],
           k1[i], k2[i], size);
    nsize += size;
  }

  // TODO: we need information about the patches first, including
  //  1. Number of patches.
  //  2. IJK values.
  NewMemory((void **)&patchdata, nsize * sizeof(float));

  NewMemory((void **)&patchdir, npatches * sizeof(int));

  int i = 0;
  while (1) {
    getpatchdata(file, npatches, i1, i2, j1, j2, k1, k2, &patchtime, patchdata,
                 &npatchdata, &file_size, &error);
    if (error) break;
    i++;
  }

  FREEMEMORY(patchdata);
  FREEMEMORY(i1);
  FREEMEMORY(i2);
  FREEMEMORY(j1);
  FREEMEMORY(j2);
  FREEMEMORY(k1);
  FREEMEMORY(k2);
  if (i != n_expected_frames) {
    return 1;
  }
  closefortranfile(file);
  return 0;
}
