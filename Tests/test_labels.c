#include "options.h"

#include "getdata.h"

#include "dmalloc.h"

#include "string_util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "readlabel.h"

int show_help;
int hash_option;
int show_version;
char append_string[1024];

int main(int argc, char **argv) {
  initMALLOC();
  SetStdOut(stdout);
  {
    // Create an objects collection and then immediately free it.
    labels_collection *coll = CreateLabelsCollection();
    // Check that collection is empty (but valid)
    assert(coll->label_first.next == &coll->label_last);
    labeldata label = {0};
    strcat(label.name, "Some Label");
    label.tstart_stop[0] = 20.0;
    label.tstart_stop[1] = 30.0;
    float xyz[3] = {1.0, 2.0, 3.0};
    memcpy(label.xyz, xyz, 3 * sizeof(float));
    int rgb[3] = {230, 230, 230};
    memcpy(label.rgb, rgb, 3 * sizeof(int));
    float frgb[3];
    frgb[0] = (float)rgb[0] / 255.0;
    frgb[1] = (float)rgb[1] / 255.0;
    frgb[2] = (float)rgb[2] / 255.0;
    memcpy(label.frgb, frgb, 3 * sizeof(int));
    label.useforegroundcolor = 0;
    label.show_always = 0;
    LabelInsert(coll, &label);
    // Check that collection is not empty
    assert(coll->label_first.next != &coll->label_last);
    // Do something with all of the labels
    LabelPrint(coll);
    FreeLabelsCollection(coll);
    // Check that collection is empty
    assert(coll->label_first.next == &coll->label_last);
  }
  // {
  //   // Create an object collection, read in object definitions, then free it.
  //   object_collection *objectscoll = CreateObjectCollection();
  //   // There should be no objects to begin with
  //   assert(objectscoll->nobject_defs == 0);
  //   ReadDefaultObjectCollection(objectscoll, NULL, 0, 0);
  //   // for (int i = 0; i < objectscoll->nobject_defs; i++) {
  //   //   sv_object *objecti = objectscoll->object_defs[i];
  //   //   printf("label[%d]: %s\n", i, objecti->label);
  //   // }
  //   FreeObjectCollection(objectscoll);
  // }
  // {
  //   // Create an object collection, read in object definitions, then free it.
  //   object_collection *objectscoll = CreateObjectCollection();
  //   ReadDefaultObjectCollection(objectscoll, NULL, 0, 0);
  //   // for (int i = 0; i < objectscoll->nobject_defs; i++) {
  //   //   sv_object *objecti = objectscoll->object_defs[i];
  //   //   printf("label[%d]: %s\n", i, objecti->label);
  //   // }
  //   FreeObjectCollection(objectscoll);
  // }
  // if (argc > 1) {
  //   const char *bad_file_path = argv[1];
  //   // Create an object collection, read in object definitions, then free it.
  //   object_collection *objectscoll = CreateObjectCollection();
  //   int result = ReadObjectDefs(objectscoll, bad_file_path);
  //   // Two of the objects in this bad file are still parsable so we should
  //   // parse 2 object definitions.
  //   assert(result == 2);
  //   FreeObjectCollection(objectscoll);
  // }
  return 0;
}
