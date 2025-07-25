#include "options.h"
// TODO: sort out imports
#include "getdata.h"
#include "dmalloc.h"
#include <stdint.h>
#include <stdlib.h>

// TODO: This is an additional function to aid in testing. This should be
// replaced by a better file IO API.
int get_pl3d_spec(const char *filename, int npts[3]) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) return 1;
  int error = fortread(npts, sizeof(*npts), 3, file);
  fclose(file);
  return error;
}

int main(int argc, char **argv) {
  initMALLOC();
  if (argc < 2) return 2;
  int error = 0;
  char *filename = argv[1];

  int isotest = 0;
  int nxpts[3] = {0};
  error = get_pl3d_spec(filename, nxpts);
  if (error) return error;
  size_t nq = 5 * nxpts[0] * nxpts[1] * nxpts[2];
  float *qframe;
  NewMemory((void **)&qframe, nq * sizeof(float));

  getplot3dq(filename, nxpts[0], nxpts[1], nxpts[2], qframe, NULL, NULL, &error, isotest);
  if (error) {
    return 1;
  }
  return 0;
}
