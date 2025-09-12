#include "options.h"

#include "dmalloc.h"

#include "readcad.h"

#include <assert.h>

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "error: insufficient arguments");
  }
  initMALLOC();
  char *input_path = argv[1];
  cadgeom_collection *coll = CreateCADGeomCollection(20);
  assert(NCADGeom(coll) == 0);
  int res = ReadCADGeomToCollection(coll, input_path, 100.0);
  fprintf(stderr, "#CAD defs: %i\n", NCADGeom(coll));
  assert(res == 0);
  assert(NCADGeom(coll) == 1);
  FreeCADGeomCollection(coll);
  return 0;
}
