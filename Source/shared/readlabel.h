#ifndef READLABEL_H_DEFINED
#define READLABEL_H_DEFINED

/**
 * @brief A collection of labels. At it's core this collection
 * contains a linked list, but also an array of pointers into that linked list.
 *
 */
typedef struct {
  labeldata label_first;
  labeldata label_last;
  labeldata *label_first_ptr;
  labeldata *label_last_ptr;
} labels_collection;

EXTERNCPP labeldata *LabelGet(labels_collection *labelscoll, char *name);
EXTERNCPP labeldata *LabelInsert(labels_collection *labelscoll,
                                 labeldata *labeltemp);

EXTERNCPP int LabelGetNUserLabels(labels_collection *labelscoll);
EXTERNCPP labeldata *LabelPrevious(labels_collection *labelscoll,
                                   labeldata *label);
EXTERNCPP labeldata *LabelNext(labels_collection *labelscoll, labeldata *label);
EXTERNCPP void LabelCopy(labeldata *label_to, labeldata *label_from);
EXTERNCPP void LabelDelete(labeldata *label);
#endif
