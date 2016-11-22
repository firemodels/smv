#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H

#include "infoheader.h"
#include "smokeviewvars.h"

// holds all of the information that will be printed to screen in the titlebox
infoboxdata titledata;

 /* ------------------------ addTitleLine ------------------------- */
// handles heap allocation and bookkeeping for adding a line to the title box
int addTitleLine(infoboxdata *titledata, const char *string) {
  if (titledata->nlines >= MAX_TITLE_LINES) {
    fprintf(stderr, "MAX_TITLE_LINES exceeded\n");
    return 2;
  }
  int string_length = strlen(string);
  if (string_length >= MAX_TITLE_LINE_LENGTH) {
    fprintf(stderr, "MAX_TITLE_LINE_LENGTH exceeded\n");
  }
  char *line = (char *) malloc ((string_length+1)*(sizeof (char)));
  if (line==NULL) {
    fprintf(stderr, "addTitleLine: memory allocation failed\n");
    return 1;
  }
  strncpy(line,string,string_length);
  line[string_length] = '\0';
  titledata->lines[titledata->nlines] = line;
  titledata->nlines++;
  return 0;
}

 /* ------------------------ clearTitleLines ------------------------- */
// walk through all the title lines and clear them
int clearTitleLines(infoboxdata *titledata) {
  int i;
  for (i = 0; i < titledata->nlines; i++) {
    free(titledata->lines[i]);
    titledata->lines[i]=NULL;
  }
  titledata->nlines=0;
  return 0;
}

 /* ------------------------ initialiseInfoHeader --------------------------- */
int initialiseInfoHeader(infoboxdata *titledata,
                         char *release_title, char *smv_githash,
                         char *fds_githash, char *chidfilebase) {
  fprintf(stderr, "initialising info header\n");
  char line[MAX_TITLE_LINE_LENGTH];

  strncpy(titledata->titleline, release_title, MAX_TITLE_LINE_LENGTH);
  fprintf(stderr, "initialised title: %s\n", titledata->titleline);

  snprintf(line,MAX_TITLE_LINE_LENGTH,"Smokeview (64 bit) build: %s",smv_githash);
  strncpy(titledata->smvbuildline, line, MAX_TITLE_LINE_LENGTH);

  if(fds_githash!=NULL){
    snprintf(line,MAX_TITLE_LINE_LENGTH,"FDS build: %s",fds_githash);
    strncpy(titledata->fdsbuildline, line, MAX_TITLE_LINE_LENGTH);
  } else {
    titledata->fdsbuildline[0] = '\0';
  }

  snprintf(line,MAX_TITLE_LINE_LENGTH,"CHID: %s",chidfilebase);
  strncpy(titledata->chidline, line, MAX_TITLE_LINE_LENGTH);

  titledata->nlines = 0;

  return 0;
}

int renderInfoHeader(portdata *VP_title, infoboxdata *titledata) {
  float left, textdown;
  left=0;
  textdown=VP_title->down;
  int textbox_bottom = textdown+titledata->bottom_margin;
  int textbox_top = textdown+VP_title->height-titledata->top_margin;
  int pen_pos = textbox_top - titledata->text_height;

  // first display hardcoded lines
  if(visTitle==1){
    OutputText(left, pen_pos, titledata->titleline);
    pen_pos -= titledata->text_height;
    pen_pos -= titledata->line_space;
  }
  if(gversion==1){
    OutputText(left, pen_pos, titledata->smvbuildline);
    pen_pos -= titledata->text_height;
    pen_pos -= titledata->line_space;
  }
  if(gversion==1&&(strlen(titledata->fdsbuildline)>0)){
    OutputText(left, pen_pos, titledata->fdsbuildline);
    pen_pos -= titledata->text_height;
    pen_pos -= titledata->line_space;
  }
  if(visCHID==1){
    OutputText(left, pen_pos, titledata->chidline);
    pen_pos -= titledata->text_height;
    pen_pos -= titledata->line_space;
  }

  int i;
  for (i = 0; i < titledata->nlines; i++) {
    OutputText(left, pen_pos, titledata->lines[i]);
    pen_pos -= titledata->text_height;
    pen_pos -= titledata->line_space;
  }
  return 0;
}