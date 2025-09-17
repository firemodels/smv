#include "options.h"

#include "dmalloc.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "colorbars.h"

int main(int argc, char **argv) {
  initMALLOC();
  if (argc < 2) return 2;
  int error = 0;
  const char *filename = argv[1];
  colorbardata *cb;
  NEWMEMORY(cb, sizeof(colorbardata));

  int result = ReadCSVColorbar(cb, filename, "divergent", CB_DIVERGENT);
  assert(!result);
  assert(strcmp(cb->colorbar_type, "divergent") == 0);
  assert(strcmp(cb->menu_label, "light blue->white->light red") == 0);
  assert(cb->nnodes == 256);
  return error;
}
