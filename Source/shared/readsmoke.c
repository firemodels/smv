#include "options.h"
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
#include "smokeviewdefs.h"
#include "string_util.h"
#include "structures.h"

/* ------------------ FreeSmoke3d ------------------------ */

void FreeSmoke3D(smoke3ddata *smoke3di) {
  smoke3di->lastiframe = -999;
#ifdef pp_SMOKEFRAME
  FRAMEFree(smoke3di->frameinfo);
  smoke3di->frameinfo = NULL;
#endif
  FREEMEMORY(smoke3di->smokeframe_in);
  FREEMEMORY(smoke3di->smokeframe_out);
  FREEMEMORY(smoke3di->timeslist);
  FREEMEMORY(smoke3di->times);
  FREEMEMORY(smoke3di->times_map);
  FREEMEMORY(smoke3di->use_smokeframe);
  FREEMEMORY(smoke3di->nchars_compressed_smoke_full);
  FREEMEMORY(smoke3di->nchars_compressed_smoke);
  FREEMEMORY(smoke3di->frame_all_zeros);
  FREEMEMORY(smoke3di->smoke_boxmin);
  FREEMEMORY(smoke3di->smoke_boxmax);
  FREEMEMORY(smoke3di->maxvals);
#ifndef pp_SMOKEFRAME
  FREEMEMORY(smoke3di->smoke_comp_all);
#endif
  FREEMEMORY(smoke3di->smokeframe_comp_list);
  FREEMEMORY(smoke3di->smokeview_tmp);
  FREEMEMORY(smoke3di->smokeframe_loaded);
}
