#include "options.h"

#include "getdata.h"

#include "MALLOCC.h"

#include "string_util.h"
#include <stdlib.h>

#include "readobject.h"

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  {
    // Create an objects collection and then immediately free it.
    object_collection *objectscoll = CreateObjectCollection();
    // There should be no objects
    assert(objectscoll->nobject_defs == 0);
    FreeObjectCollection(objectscoll);
  }
  {
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
    // There should be no objects to begin with
    assert(objectscoll->nobject_defs == 0);
    ReadDefaultObjectCollection(objectscoll, NULL, 0, 0);
    // for (int i = 0; i < objectscoll->nobject_defs; i++) {
    //   sv_object *objecti = objectscoll->object_defs[i];
    //   printf("label[%d]: %s\n", i, objecti->label);
    // }
    FreeObjectCollection(objectscoll);
  }
  {
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
    ReadDefaultObjectCollection(objectscoll, NULL, 0, 0);
    // for (int i = 0; i < objectscoll->nobject_defs; i++) {
    //   sv_object *objecti = objectscoll->object_defs[i];
    //   printf("label[%d]: %s\n", i, objecti->label);
    // }
    FreeObjectCollection(objectscoll);
  }
  if (argc > 1) {
    const char *bad_file_path = argv[1];
    // Create an object collection, read in object definitions, then free it.
    object_collection *objectscoll = CreateObjectCollection();
    int result = ReadObjectDefs(objectscoll, bad_file_path, 0);
    // Two of the objects in this bad file are still parsable so we should
    // parse 2 object definitions.
    assert(result == 2);
    FreeObjectCollection(objectscoll);
  }
  return 0;
}
