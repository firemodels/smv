#include "smokeviewvars.h"

#define MAX_TITLE_LINE_LENGTH 256
#define MAX_TITLE_LINES 256


/* --------------------------  titledata ---------------------------------- */

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
} titledata;

EXTERNCPP int addTitleLine(titledata *titleinfo, const char *string);
EXTERNCPP int clearTitleLines(titledata *titleinfo);
EXTERNCPP int initialiseInfoHeader(titledata *titleinfo,
                         char *release_title, char *smv_githash,
                         char *fds_githash, char *chidfilebase);
EXTERNCPP int renderInfoHeader(portdata *VP_title, titledata *titleinfo);

SVEXTERN titledata titleinfo;