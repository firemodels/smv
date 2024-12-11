#include "options.h"

#include "dmalloc.h"

#include "readcad.h"

#include <assert.h>

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  cadgeom_collection *coll = CreateCADGeomCollection(20);
  assert(NCADGeom(coll) == 0);
  FreeCADGeomCollection(coll);
  return 0;
}
