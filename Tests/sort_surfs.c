#define INMAIN
#define pp_GPU
#define pp_RENDER360_DEBUG
#define pp_memstatus
#define pp_REFRESH
#undef pp_OSX_HIGHRES
#ifdef pp_OSX
#ifndef pp_QUARTZ
#define pp_REFRESH      // refresh glui dialogs when they change size
#ifndef pp_OSX_LOWRES
#define pp_OSX_HIGHRES
#endif
#endif
#endif

#include "options.h"

#include "dmalloc.h"

#include "readcad.h"
#include "smokeviewvars.h"
#include <assert.h>

void UpdateSortedSurfIdList(surf_collection *surfcoll);
void InitSurface(surfdata *surf, float *color);

int show_help;
int hash_option;
int show_version;
char append_string[1024];
#define N_SURFS 3
int main(int argc, char **argv) {
  initMALLOC();
  char *surf_names[N_SURFS] = {"abc", "hij", "def"};
  surf_collection surfcoll = {0};
  surfcoll.nsurfinfo = N_SURFS;
  NewMemory((void **)&surfcoll.surfinfo, N_SURFS * sizeof(surfdata));
  for(int i = 0; i < N_SURFS; i++) {
    surfdata *sd = surfcoll.surfinfo + i;
    InitSurface(sd, block_ambient_orig);
    sd->surfacelabel = surf_names[i];
  }
  UpdateSortedSurfIdList(&surfcoll);
  assert(strcmp(surfcoll.surfinfo[surfcoll.sorted_surfidlist[0]].surfacelabel,
                "abc") == 0);
  assert(strcmp(surfcoll.surfinfo[surfcoll.sorted_surfidlist[1]].surfacelabel,
                "def") == 0);
  assert(strcmp(surfcoll.surfinfo[surfcoll.sorted_surfidlist[2]].surfacelabel,
                "hij") == 0);
  return 0;
}
