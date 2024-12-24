#ifndef READHVAC_H_DEFINED
#define READHVAC_H_DEFINED

#include "options_common.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file_util.h"
#include "stdio_buffer.h"

#include "shared_structures.h"

#define DUCT_COMPONENT_TEXT 0
#define DUCT_COMPONENT_SYMBOLS 1
#define DUCT_COMPONENT_HIDE 2

#define NODE_FILTERS_LABELS 0
#define NODE_FILTERS_SYMBOLS 1
#define NODE_FILTERS_HIDE 2

#define DUCT_XYZ 0
#define DUCT_YXZ 1
#define DUCT_XZY 2
#define DUCT_ZXY 3
#define DUCT_YZX 4
#define DUCT_ZYX 5

#define HVAC_FILTER_NO 0
#define HVAC_FILTER_YES 1
#define HVAC_NONE 0
#define HVAC_FAN 1
#define HVAC_AIRCOIL 2
#define HVAC_DAMPER 3

#define HVAC_STATE_INACTIVE 0
#define HVAC_STATE_ACTIVE 1

hvacductdata *GetHVACDuctID(hvacdatacollection *hvaccoll, char *duct_name);
int GetHVACDuctValIndex(hvacdatacollection *hvaccoll, char *shortlabel);

hvacnodedata *GetHVACNodeID(hvacdatacollection *hvaccoll, char *node_name);
int GetHVACNodeValIndex(hvacdatacollection *hvaccoll, char *shortlabel);

void InitHvacData(hvacvaldata *hi);

/**
 * @brief Are any of the hvac items visible (i.e., have display set to true)?
 *
 * @param hvaccoll The HVAC collection.
 * @return 1 if ANY HVAC item is visible. 0 if NO HVAC item is visible.
 */
int IsHVACVisible(hvacdatacollection *hvaccoll);

/**
 * @brief Parse the definition of HVAC nodes etc. from an *.smv file.
 *
 * @param hvaccoll The HVAC collection.
 * @param stream The stream that is currently being parsed. The position of this
 * stream should be just after the "HVAC" keyword.
 * @param hvac_node_color Default HVAC node color
 * @param hvac_duct_color Default HVAC duct color
 * @return 0 on success, 1 to break parsing loop, 2 to continue to continue
 * parsing loop.
 */
int ParseHVACEntry(hvacdatacollection *hvaccoll, bufferstreamdata *stream,
                   int hvac_node_color[3], int hvac_duct_color[3]);

/**
 * @brief Parse the definition of HVAC values from an *.smv file.
 *
 * @param hvaccoll The HVAC collection.
 * @param stream The stream that is currently being parsed. The position of this
 * stream should be just after the "HVACVALS" keyword.
 * @return 0 on success, 1 to break parsing loop, 2 to continue to continue
 * parsing loop.
 */
int ParseHVACValsEntry(hvacdatacollection *hvaccoll, bufferstreamdata *stream);

/**
 * @brief Parse HVAC value data from a *.hvac file.
 *
 * @param[inout] hvaccoll The HVAC collection.
 * @param[in] flag The flag to control whether we load or unload.
 * @param[out] file_size A location to output the size of the file.
 */
int ReadHVACData0(hvacdatacollection *hvaccoll, int flag, FILE_SIZE *file_size);
#endif
