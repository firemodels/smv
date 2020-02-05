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
  char fdstitleline[MAX_TITLE_LINE_LENGTH];
  char smvbuildline[MAX_TITLE_LINE_LENGTH];
  char fdsbuildline[MAX_TITLE_LINE_LENGTH];
  char chidline[MAX_TITLE_LINE_LENGTH];
} titledata;

EXTERNCPP int addTitleLine(titledata *titleinfo_ptr, const char *string);
EXTERNCPP int clearTitleLines(titledata *titleinfo_ptr);
EXTERNCPP int initialiseInfoHeader(titledata *titleinfo_ptr,
                         char *release_title_string, char *smv_githash_string,
                         char *fds_githash_string, char *chidfilebase_string, char *fds_title_arg);
EXTERNCPP int renderInfoHeader(titledata *titleinfo);

SVEXTERN titledata titleinfo;
