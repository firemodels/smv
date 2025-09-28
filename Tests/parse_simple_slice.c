#include "options.h"
// TODO: sort out imports
#include "dmalloc.h"
#include "getdata.h"
#include "readslice.h"
#include <assert.h>
#include <float.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  initMALLOC();
  if(argc < 3) return 2;

  size_t expected = atol(argv[2]);
  // Parse using getdata.c functions
  {
    FILE *file;
    int i1, i2, j1, j2, k1, k2 = 0;
    int error = 0;

    openslice(argv[1], &file, &i1, &i2, &j1, &j2, &k1, &k2, &error);

    size_t nx = i2 + 1 - i1;
    size_t ny = j2 + 1 - j1;
    size_t nz = k2 + 1 - k1;

    size_t nqframe = nx * ny * nz;
    float *qframe;
    NewMemory((void **)&qframe, nqframe * sizeof(float));
    float time;
    if(error) return 1;
    size_t count = 0;
    while(1) {
      getsliceframe(file, i1, i2, j1, j2, k1, k2, &time, qframe, 0, &error);
      if(error != 0) break;
      count++;
    }
    fprintf(stderr, "getdata: count: %zu, expected: %zu\n", count, expected);
    if(count != expected) {
      return 1;
    }
    FREEMEMORY(qframe);
    closefortranfile(file);
  }
  // Parse using readslice.c functions
  {
    // time_min and time_max are only set when their relative value is invalid
    // (i.e. time_min > time_max). Therefore we initially set an invalid set of
    // values.
    float time_min = FLT_MAX;
    float time_max = FLT_MIN;
    int n_frames = GetNSliceFrames(argv[1], &time_min, &time_max);
    fprintf(stderr, "readslice: count: %d, expected: %zu\n", n_frames,
            expected);
    assert(n_frames == expected);
    fprintf(stderr, "time_min: %f s, time_max: %f s\n", time_min, time_min);
    if(n_frames > 0) {
      // Check that the values have been updatd.
      assert(time_min != FLT_MAX);
      assert(time_max != FLT_MIN);
    }
    if(n_frames > 1) {
      assert(time_min < time_max);
    }
  }
  {
    int nsliceiptr = 0;
    int nslicejptr = 0;
    int nslicekptr = 0;
    int ntimesptr = 0;
    int tload_step_arg = 1;

    int errorptr = 0;
    int settmin_s_arg = 0;
    int settmax_s_arg = 0;
    float tmin_s_arg = 0;

    float tmax_s_arg = 0;
    int headersizeptr = 0;
    int framesizeptr = 0;
    GetSliceSizes(argv[1], -1, &nsliceiptr, &nslicejptr, &nslicekptr,
                  &ntimesptr, tload_step_arg, &errorptr, settmin_s_arg,
                  settmax_s_arg, tmin_s_arg, tmax_s_arg, &headersizeptr,
                  &framesizeptr);
  }
  {
    int is1ptr = 0;
    int is2ptr = 0;
    int js1ptr = 0;
    int js2ptr = 0;
    int ks1ptr = 0;
    int ks2ptr = 0;
    int idirptr = 0;
    float qminptr = 0;
    float qmaxptr = 0;
    float qdataptr = 0;
    float timesptr = 0;
    int ntimes_old_arg = 0;
    int ntimesptr = 0;
    int tload_step_arg = 1;
    int settmin_s_arg = 0;
    int settmax_s_arg = 0;
    float tmin_s_arg = 0;
    float tmax_s_arg = 0;

    slicedata sd = {0};
    FILE_SIZE r =
        GetSliceData(&sd, argv[1], -1, &is1ptr, &is2ptr, &js1ptr, &js2ptr,
                     &ks1ptr, &ks2ptr, &idirptr, &qminptr, &qmaxptr, &qdataptr,
                     &timesptr, ntimes_old_arg, &ntimesptr, tload_step_arg,
                     settmin_s_arg, settmax_s_arg, tmin_s_arg, tmax_s_arg);
    fprintf(stderr, "slicedata: size: %llu ntimes: %d\n", r, ntimesptr);
    assert(r != 0);
  }
  return 0;
}
