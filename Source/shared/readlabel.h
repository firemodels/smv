#ifndef READLABEL_H_DEFINED
#define READLABEL_H_DEFINED

#include "options_common.h"
#include "shared_structures.h"

#ifndef TYPE_SMV
#define TYPE_SMV 0
#endif
#ifndef TYPE_INI
#define TYPE_INI 1
#endif

EXTERNCPP labeldata *LabelGet(labels_collection *labelscoll, char *name);
EXTERNCPP labeldata *LabelInsert(labels_collection *labelscoll,
                                 labeldata *labeltemp);

EXTERNCPP int LabelGetNUserLabels(labels_collection *labelscoll);
EXTERNCPP labeldata *LabelPrevious(labels_collection *labelscoll,
                                   labeldata *label);
EXTERNCPP labeldata *LabelNext(labels_collection *labelscoll, labeldata *label);
EXTERNCPP void LabelCopy(labeldata *label_to, labeldata *label_from);
EXTERNCPP void LabelDelete(labeldata *label);
labels_collection *CreateLabelsCollection();
EXTERNCPP void InitLabelsCollection(labels_collection *labelscoll);
EXTERNCPP void FreeLabelsCollection(labels_collection *labelscoll);
EXTERNCPP void LabelPrint(labels_collection *labelscoll);
static inline int FileExistsCaseDir(smv_case *scase, char *filename) {
  return FileExists(filename, scase->filelist_coll.filelist_casename,
                    scase->filelist_coll.nfilelist_casename,
                    scase->filelist_coll.filelist_casedir,
                    scase->filelist_coll.nfilelist_casedir);
}

#endif
