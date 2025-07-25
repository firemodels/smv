#include "options.h"

// TODO: sort out imports
#include "dmalloc.h"
#include "getdata.h"
#include <stdlib.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dmalloc.h"
#include "datadefs.h"
#include "histogram.h"
#include "isobox.h"
#include "string_util.h"

#include "file_util.h"
#include "stdio_buffer.h"

#include "readhvac.h"

int show_help;
int hash_option;
int show_version;
char append_string[1024];

void init_vals(hvacdatacollection *hvaccoll, const char *file) {
  NewMemory((void **)&(hvaccoll->hvacductvalsinfo), sizeof(hvacvalsdata));
  hvaccoll->hvacductvalsinfo->times = NULL;
  hvaccoll->hvacductvalsinfo->loaded = 0;
  hvaccoll->hvacductvalsinfo->node_vars = NULL;
  hvaccoll->hvacductvalsinfo->duct_vars = NULL;
  hvaccoll->hvacductvalsinfo->file = strdup(file);
  hvaccoll->hvacductvalsinfo->n_node_vars = 0;

  hvaccoll->hvacductvalsinfo->n_duct_vars = 4;

  if (hvaccoll->hvacductvalsinfo->n_duct_vars > 0) {
    NewMemory((void **)&hvaccoll->hvacductvalsinfo->duct_vars,
              hvaccoll->hvacductvalsinfo->n_duct_vars * sizeof(hvacvaldata));
    {
      hvacvaldata *hi = hvaccoll->hvacductvalsinfo->duct_vars;
      InitHvacData(hi);
      hi->label.longlabel = "DUCT VELOCITY";
      hi->label.shortlabel = "vel_d";
      hi->label.unit = "m/s";
    }
    {
      hvacvaldata *hi = hvaccoll->hvacductvalsinfo->duct_vars + 1;
      InitHvacData(hi);
      hi->label.longlabel = "DUCT LOSS";
      hi->label.shortlabel = "loss_d";
      hi->label.unit = "";
    }
    {
      hvacvaldata *hi = hvaccoll->hvacductvalsinfo->duct_vars + 2;
      InitHvacData(hi);
      hi->label.longlabel = "OXYGEN DUCT VOLUME FRACTION";
      hi->label.shortlabel = "X_d_O2";
      hi->label.unit = "mol/mol";
    }
    {
      hvacvaldata *hi = hvaccoll->hvacductvalsinfo->duct_vars + 3;
      InitHvacData(hi);
      hi->label.longlabel = "NITROGEN DUCT VOLUME FRACTION";
      hi->label.shortlabel = "X_d_N2";
      hi->label.unit = "mol/mol";
    }
  }

  FREEMEMORY(hvaccoll->hvacnodevalsinfo);
  NewMemory((void **)&hvaccoll->hvacnodevalsinfo, sizeof(hvacvalsdata));
  memcpy(hvaccoll->hvacnodevalsinfo, hvaccoll->hvacductvalsinfo,
         sizeof(hvacvalsdata));
}

int main(int argc, char **argv) {
  initMALLOC();
  fprintf(stderr, "file: %s\n", argv[1]);
  hvacdatacollection hvaccoll = {0};
  init_vals(&hvaccoll, argv[1]);

  FILE_SIZE file_size;
  int ret = ReadHVACData0(&hvaccoll, 0, &file_size);
  if (ret) return ret;
  for (int i = 0; i < hvaccoll.hvacductvalsinfo->n_duct_vars; i++) {
    hvacvaldata *hi = hvaccoll.hvacductvalsinfo->duct_vars + i;
    fprintf(stderr, "%s\n", hi->label.longlabel);
    fprintf(stderr, "  shortlabel: %s\n", hi->label.shortlabel);
    fprintf(stderr, "  units: %s\n", hi->label.unit);
    fprintf(stderr, "  nvals: %d\n", hi->nvals);
    for (int j = 0; j < hi->nvals + 100; j++) {
      fprintf(stderr, "    %d: %g\n", j, hi->vals[j]);
    }
  }
  return 0;
}
