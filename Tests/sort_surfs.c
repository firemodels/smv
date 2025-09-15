#define INMAIN
#include "options.h"
#include "dmalloc.h"
#include "shared_structures.h"
#include <assert.h>
#include <string.h>

void UpdateSortedSurfIdList(surf_collection *surfcoll);
void InitSurface(surfdata *surf, float *color);

int show_help;
int hash_option;
int show_version;

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
