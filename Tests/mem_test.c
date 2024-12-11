#include "dmalloc.h"

#include <assert.h>

int main(int argc, char **argv) {
  initMALLOC();
  int *cb;
  NEWMEMORY(cb, 256 * sizeof(int));
  RESIZEMEMORY(cb, 256 * sizeof(int));
  RESIZEMEMORY(cb, 512 * sizeof(int));
  cb[400] = 25;
  assert(cb[400] == 25);
  CheckMemory;
  FREEMEMORY(cb);
  return 0;
}
