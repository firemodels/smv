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

/* --------------------------  hvacconnectdata
 * ------------------------------------ */

typedef struct hvacconnectdata {
  int index, display;
} hvacconnectdata;

/* --------------------------  hvacnodedata ------------------------------------
 */

typedef struct _hvacnodedata {
  char *node_name, *vent_name, *duct_name, *network_name;
  char c_filter[10];
  int node_id, filter, use_node, connect_id;
  hvacconnectdata *connect;
  struct _hvacductdata *duct;
  float xyz[3], xyz_orig[3];
} hvacnodedata;

/* --------------------------  hvacductdata ------------------------------------
 */

typedef struct _hvacductdata {
  char *duct_name, *network_name, c_component[4];
  int duct_id, component, nduct_cells;
  int node_id_from, node_id_to, use_duct, connect_id;
  hvacconnectdata *connect;
  int nact_times, *act_states, metro_path;
  float *act_times;
  float xyz_symbol[3], xyz_symbol_metro[3];
  float xyz_label[3], xyz_label_metro[3];
  float normal[3], normal_metro[3];
  hvacnodedata *node_from, *node_to;
  float xyz_met[12], *xyz_reg;
  int nxyz_met, nxyz_reg;
  float *xyz_met_cell, *xyz_reg_cell;
  int nxyz_met_cell, nxyz_reg_cell;
  int *cell_met, *cell_reg;
} hvacductdata;

/* --------------------------  hvacdata ------------------------------------ */

typedef struct _hvacdata {
  char *network_name;
  int display;
  int show_node_labels, show_duct_labels;
  int show_filters, show_component;
  float cell_node_size, node_size, component_size, duct_width, filter_size;
  int duct_color[3], node_color[3];
} hvacdata;

/* --------------------------  hvacvaldata ------------------------------------
 */

typedef struct _hvacvaldata {
  float *vals, valmin, valmax;
  int setvalmin, setvalmax;
  int vis, nvals;
  char colorlabels[12][11];
  float colorvalues[12];
  float levels256[256];
  flowlabels label;
} hvacvaldata;

/* --------------------------  _hvacvalsdata
 * ------------------------------------ */

typedef struct _hvacvalsdata {
  char *file;
  int loaded;
  int n_node_vars, n_duct_vars, ntimes;
  float *times;
  hvacvaldata *node_vars, *duct_vars;
} hvacvalsdata;

typedef struct {
  hvacnodedata *hvacnodeinfo;
  int nhvacnodeinfo;
  hvacductdata *hvacductinfo;
  int nhvacductinfo;
  hvacconnectdata *hvacconnectinfo;
  int nhvacconnectinfo;
  hvacdata *hvacinfo;
  int nhvacinfo;

  int nhvacfilters;

  int nhvaccomponents;

  hvacvalsdata *hvacductvalsinfo;
  int hvacductvar_index;
  hvacvalsdata *hvacnodevalsinfo;
  int hvacnodevar_index;

  int hvac_maxcells;
  int hvac_n_ducts;

} hvacdatacollection;

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
