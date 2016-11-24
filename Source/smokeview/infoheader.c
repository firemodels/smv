#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H

#include "MALLOC.h"
#include "infoheader.h"
#include "smokeviewvars.h"

// holds all of the information that will be printed to screen in the titlebox
titledata titleinfo;

 /* ------------------------ addTitleLine ------------------------- */
// handles heap allocation and bookkeeping for adding a line to the title box
int addTitleLine(titledata *titleinfo, const char *string) {
  char *line;
  if (titleinfo->nlines >= MAX_TITLE_LINES) {
    fprintf(stderr, "MAX_TITLE_LINES exceeded\n");
    return 2;
  }
  int string_length = strlen(string);
  if (string_length >= MAX_TITLE_LINE_LENGTH) {
    fprintf(stderr, "MAX_TITLE_LINE_LENGTH exceeded\n");
  }
  NewMemory((void **)&line, (string_length+1)*sizeof(char));
  if (line==NULL) {
    fprintf(stderr, "addTitleLine: memory allocation failed\n");
    return 1;
  }
  strncpy(line,string,string_length);
  line[string_length] = '\0';
  titleinfo->lines[titleinfo->nlines] = line;
  titleinfo->nlines++;
  return 0;
}

 /* ------------------------ clearTitleLines ------------------------- */
// walk through all the title lines and clear them
int clearTitleLines(titledata *titleinfo) {
  int i;
  for (i = 0; i < titleinfo->nlines; i++) {
    FREEMEMORY(titleinfo->lines[i]);
    titleinfo->lines[i]=NULL;
  }
  titleinfo->nlines=0;
  return 0;
}

 /* ------------------------ initialiseInfoHeader --------------------------- */
int initialiseInfoHeader(titledata *titleinfo,
                         char *release_title, char *smv_githash,
                         char *fds_githash, char *chidfilebase) {
  fprintf(stderr, "initialising info header\n");
  char line[MAX_TITLE_LINE_LENGTH];

  strncpy(titleinfo->titleline, release_title, MAX_TITLE_LINE_LENGTH);
  fprintf(stderr, "initialised title: %s\n", titleinfo->titleline);

  snprintf(line,MAX_TITLE_LINE_LENGTH,"Smokeview (64 bit) build: %s",smv_githash);
  strncpy(titleinfo->smvbuildline, line, MAX_TITLE_LINE_LENGTH);

  if(fds_githash!=NULL){
    snprintf(line,MAX_TITLE_LINE_LENGTH,"FDS build: %s",fds_githash);
    strncpy(titleinfo->fdsbuildline, line, MAX_TITLE_LINE_LENGTH);
  } else {
    titleinfo->fdsbuildline[0] = '\0';
  }

  snprintf(line,MAX_TITLE_LINE_LENGTH,"CHID: %s",chidfilebase);
  strncpy(titleinfo->chidline, line, MAX_TITLE_LINE_LENGTH);

  titleinfo->nlines = 0;

  return 0;
}

int renderInfoHeader(portdata *VP_title, titledata *titleinfo) {
  float left, textdown;
  left=0;
  textdown=VP_title->down;
  int textbox_bottom = textdown+titleinfo->bottom_margin;
  int textbox_top = textdown+VP_title->height-titleinfo->top_margin;
  int pen_pos = textbox_top - titleinfo->text_height;

  // first display hardcoded lines
  if(visTitle==1){
    OutputText(left, pen_pos, titleinfo->titleline);
    pen_pos -= titleinfo->text_height;
    pen_pos -= titleinfo->line_space;
  }
  if(gversion==1){
    OutputText(left, pen_pos, titleinfo->smvbuildline);
    pen_pos -= titleinfo->text_height;
    pen_pos -= titleinfo->line_space;
  }
  if(gversion==1&&(strlen(titleinfo->fdsbuildline)>0)){
    OutputText(left, pen_pos, titleinfo->fdsbuildline);
    pen_pos -= titleinfo->text_height;
    pen_pos -= titleinfo->line_space;
  }
  if(visCHID==1){
    OutputText(left, pen_pos, titleinfo->chidline);
    pen_pos -= titleinfo->text_height;
    pen_pos -= titleinfo->line_space;
  }

  int i;
  for (i = 0; i < titleinfo->nlines; i++) {
    OutputText(left, pen_pos, titleinfo->lines[i]);
    pen_pos -= titleinfo->text_height;
    pen_pos -= titleinfo->line_space;
  }
  return 0;
}