#include "options.h"
// TODO: sort out imports
#include "../Source/smokeview/getdata.h"
#include "MALLOCC.h"
#include <stdint.h>
#include <stdlib.h>

// TODO: This is an additional function to aid in testing. This should be
// replaced by a better file IO API.
int get_bndf_spec(const char *filename, int version, int *npatch, int **i1,
                  int **i2, int **j1, int **j2, int **k1, int **k2) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) return 1;
  char patchlonglabel[31] = {0};
  char patchshortlabel[31] = {0};
  char patchunit[31] = {0};
  int error = 0;

  error = fortread(patchlonglabel, 30, 1, file);
  if (error != 0) goto end;

  error = fortread(patchshortlabel, 30, 1, file);
  if (error != 0) goto end;

  error = fortread(patchunit, 30, 1, file);
  if (error != 0) goto end;

  error = fortread(npatch, sizeof(*npatch), 1, file);
  if (error != 0) goto end;

  NewMemory((void **)i1, (*npatch) * sizeof(**i1));
  NewMemory((void **)i2, (*npatch) * sizeof(**i2));
  NewMemory((void **)j1, (*npatch) * sizeof(**j1));
  NewMemory((void **)j2, (*npatch) * sizeof(**j2));
  NewMemory((void **)k1, (*npatch) * sizeof(**k1));
  NewMemory((void **)k2, (*npatch) * sizeof(**k2));

  uint32_t ijk[9] = {0};
  for (int n = 0; n < *npatch; n++) {
    if (version == 0) {
      if (error == 0) {
        error = fortread(ijk, sizeof(*ijk), 6, file);
        if (error != 0) goto end;
      }
    } else {
      if (error == 0) {
        error = fortread(ijk, sizeof(*ijk), 9, file);
        if (error) goto end;
      }
    }
    (*i1)[n] = ijk[0];
    (*i2)[n] = ijk[1];
    (*j1)[n] = ijk[2];
    (*j2)[n] = ijk[3];
    (*k1)[n] = ijk[4];
    (*k2)[n] = ijk[5];
  }

end:
  if (file != NULL) {
    fclose(file);
  }

  return error;
}

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
  int npatches = 0;
  error = get_bndf_spec(filename, boundary_version, &npatches, &i1, &i2, &j1,
                        &j2, &k1, &k2);
  if (error) return error;
  FILE *file = openboundary(filename, boundary_version, &error);
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
  // TODO: `file` should be closed here but `getpatchdata` does it for us.
  // closefortranfile(file);
  return 0;
}
