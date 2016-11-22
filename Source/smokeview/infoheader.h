#include "smokeviewvars.h"

#define MAX_TITLE_LINE_LENGTH 256
#define MAX_TITLE_LINES 256


/* --------------------------  infoboxdata ---------------------------------- */

typedef struct {
  GLint left_margin, top_margin, bottom_margin, line_space, text_height;
  int nlines;
  char *lines[MAX_TITLE_LINES];
  // special allowance for hardcoded entries, it is necessary for these
  // to be separate as they need to be controlled differently
  char titleline[MAX_TITLE_LINE_LENGTH];
  char smvbuildline[MAX_TITLE_LINE_LENGTH];
  char fdsbuildline[MAX_TITLE_LINE_LENGTH];
  char chidline[MAX_TITLE_LINE_LENGTH];
} infoboxdata;

EXTERNCPP int addTitleLine(infoboxdata *titledata, const char *string);
EXTERNCPP int clearTitleLines(infoboxdata *titledata);
EXTERNCPP int initialiseInfoHeader(infoboxdata *titledata,
                         char *release_title, char *smv_githash,
                         char *fds_githash, char *chidfilebase);
EXTERNCPP int renderInfoHeader(portdata *VP_title, infoboxdata *titledata);

SVEXTERN infoboxdata titledata;