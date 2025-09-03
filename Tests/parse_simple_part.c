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
  FILE *file = openpart(filename, &error);

  int size = 0;
  int fdsversion = 0;
  int nclasses = 0;
  int *nquantities;
  int *npoints;
  getpartheader1(file, &nclasses, &fdsversion, &size);

  NewMemory((void **)&nquantities, nclasses * sizeof(int));
  NewMemory((void **)&npoints, nclasses * sizeof(int));

  const size_t BUFFER_SIZE = 1000000;

  float *pdata;
  int *tagdata;
  NewMemory((void **)&pdata, BUFFER_SIZE * sizeof(float));
  NewMemory((void **)&tagdata, BUFFER_SIZE * sizeof(int));

  getpartheader2(file, nclasses, nquantities, &size);
  int i = 0;
  while (1) {
    float time;
    getpartdataframe(file, nclasses, nquantities, npoints, &time, tagdata,
                     pdata, &size, &error);
    if (error != 0) break;
    i++;
  }

  closefortranfile(file);

  FREEMEMORY(nquantities);
  FREEMEMORY(npoints);
  FREEMEMORY(pdata);
  FREEMEMORY(tagdata);
  if (i != n_expected_frames) {
    return 1;
  }
  return 0;
}
