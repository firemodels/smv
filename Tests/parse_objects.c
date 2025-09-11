#include "options.h"

#include "getdata.h"

#include "dmalloc.h"

#include "string_util.h"
#include <stdlib.h>

#include "readobject.h"

int show_help;
int hash_option;
int show_version;

int main(int argc, char **argv) {
  initMALLOC();
  if (argc > 1) {
    const char *file_path = argv[1];
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
    int result = ReadObjectDefs(objectscoll, file_path);
    // Two of the objects in this bad file are still parsable so we should
    // parse 2 object definitions.
    assert(result > 0);
    FreeObjectCollection(objectscoll);
    return 0;
  }
  else {
    return 1;
  }
}
