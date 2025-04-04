#ifndef READSLICE_H_DEFINED
#define READSLICE_H_DEFINED
#include "options.h"
#include "dmalloc.h"
#include "string_util.h"

#include <string.h>

void GetSliceFileHeader(char *file, int *ip1, int *ip2, int *jp1, int *jp2, int *kp1, int *kp2, int *error);

#endif
