#include "options_common.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmalloc.h"
#include "scontour2d.h"
#include "histogram.h"
#include "isobox.h"
#include "readsmoke.h"
#include "string_util.h"

/* ------------------ FreeSmoke3d ------------------------ */

void FreeSmoke3D(smv_case *scase, smoke3ddata *smoke3di){
  meshdata *meshi;

  meshi = scase->meshescoll.meshinfo + smoke3di->blocknumber;
  FREEMEMORY(meshi->is_firenode);
  meshi->is_firenodeptr = NULL;
  smoke3di->lastiframe = -999;
  float ext;
  char *label;

  label = smoke3di->label.shortlabel;
  ext = smoke3di->extinct;
  if(ext > 0.0) {
    meshi->smoke3d_soot = NULL;
  }
  if(strcmp(label, "hrrpuv") == 0){
    meshi->smoke3d_hrrpuv = NULL;
  }
  if(strcmp(label, "temp") == 0){
    meshi->smoke3d_temp = NULL;
  }
  if(strcmp(label, "rho_CO2") == 0 || strcmp(label, "Y_CO2") == 0){
    meshi->smoke3d_co2 = NULL;
  }

  FREEMEMORY(smoke3di->smokeframe_in);
  FREEMEMORY(smoke3di->smokeframe_out);
  FREEMEMORY(smoke3di->timeslist);
  FREEMEMORY(smoke3di->histtimes);
  FREEMEMORY(smoke3di->times);
  FREEMEMORY(smoke3di->times_map);
  FREEMEMORY(smoke3di->use_smokeframe);
  FREEMEMORY(smoke3di->nchars_compressed_smoke_full);
  FREEMEMORY(smoke3di->nchars_compressed_smoke);
  FREEMEMORY(smoke3di->frame_all_zeros);
  FREEMEMORY(smoke3di->smoke_boxmin);
  FREEMEMORY(smoke3di->smoke_boxmax);
  FREEMEMORY(smoke3di->maxvals);
  FREEMEMORY(smoke3di->smoke_comp_all);
  FREEMEMORY(smoke3di->smokeframe_comp_list);
  FREEMEMORY(smoke3di->smokeview_tmp);
  FREEMEMORY(smoke3di->smokeframe_loaded);
}
