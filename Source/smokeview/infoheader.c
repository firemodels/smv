#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H

#include "dmalloc.h"
#include "infoheader.h"
#include "smokeviewvars.h"

// holds all of the information that will be printed to screen in the titlebox
titledata titleinfo;

 /* ------------------------ addTitleLine ------------------------- */
// handles heap allocation and bookkeeping for adding a line to the title box
int addTitleLine(titledata *titleinfo_ptr, const char *string){
  char *line;
  if(titleinfo_ptr->nlines >= MAX_TITLE_LINES){
    PRINTF("MAX_TITLE_LINES exceeded\n");
    return 2;
  }
  int string_length = strlen(string);
  if(string_length >= MAX_TITLE_LINE_LENGTH){
    PRINTF("MAX_TITLE_LINE_LENGTH exceeded\n");
  }
  NewMemory((void **)&line, (string_length+1)*sizeof(char));
  if(line==NULL){
    PRINTF("addTitleLine: memory allocation failed\n");
    return 1;
  }
  strncpy(line,string,string_length);
  line[string_length] = '\0';
  titleinfo_ptr->lines[titleinfo_ptr->nlines] = line;
  titleinfo_ptr->nlines++;
  return 0;
}

 /* ------------------------ clearTitleLines ------------------------- */
// walk through all the title lines and clear them
int clearTitleLines(titledata *titleinfo_ptr){
  int i;
  for(i = 0; i < titleinfo_ptr->nlines; i++){
    FREEMEMORY(titleinfo_ptr->lines[i]);
    titleinfo_ptr->lines[i]=NULL;
  }
  titleinfo_ptr->nlines=0;
  return 0;
}

 /* ------------------------ initialiseInfoHeader --------------------------- */

int initialiseInfoHeader(titledata *titleinfo_ptr,
                         char *release_title_string, char *smv_githash_string,
                         char *fds_version_string, char *chidfilebase_string, char *fds_title_arg){
  char line[MAX_TITLE_LINE_LENGTH];

  strncpy(titleinfo_ptr->titleline, release_title_string, MAX_TITLE_LINE_LENGTH);

  snprintf(line,MAX_TITLE_LINE_LENGTH,"Smokeview version: %s",smv_githash_string);
  strncpy(titleinfo_ptr->smvbuildline, line, MAX_TITLE_LINE_LENGTH);

  if(fds_version_string!=NULL){
    snprintf(line,MAX_TITLE_LINE_LENGTH,"FDS version: %s",fds_version_string);
    strncpy(titleinfo_ptr->fdsbuildline, line, MAX_TITLE_LINE_LENGTH);
  }
  else{
    titleinfo_ptr->fdsbuildline[0] = '\0';
  }

  snprintf(line,MAX_TITLE_LINE_LENGTH,"CHID: %s",chidfilebase_string);
  strncpy(titleinfo_ptr->chidline, line, MAX_TITLE_LINE_LENGTH);

  strcpy(titleinfo_ptr->fdstitleline, "");
  if(fds_title_arg!=NULL)strcpy(titleinfo_ptr->fdstitleline, fds_title_arg);

  titleinfo_ptr->nlines = 0;

  return 0;
}

 /* ------------------------ renderInfoHeader --------------------------- */

int renderInfoHeader(titledata *titleinfo_ptr){
  float left, textdown;

  left=0;
  textdown=VP_title.down;
//  int textbox_bottom = textdown+titleinfo_ptr->bottom_margin;
  int textbox_top = textdown+VP_title.height-titleinfo_ptr->top_margin;
  int pen_pos = textbox_top - titleinfo_ptr->text_height;

  if(vis_title_smv_version==1){
    OutputText(left, pen_pos, titleinfo_ptr->titleline);
    pen_pos -= titleinfo_ptr->text_height;
    pen_pos -= titleinfo_ptr->line_space;
  }
  if(vis_title_gversion==1){
    OutputText(left, pen_pos, titleinfo_ptr->smvbuildline);
    pen_pos -= titleinfo_ptr->text_height;
    pen_pos -= titleinfo_ptr->line_space;
  }
  if(vis_title_gversion==1&&(strlen(titleinfo_ptr->fdsbuildline)>0)){
    OutputText(left, pen_pos, titleinfo_ptr->fdsbuildline);
    pen_pos -= titleinfo_ptr->text_height;
    pen_pos -= titleinfo_ptr->line_space;
  }
  if(vis_title_fds==1){
    OutputText(left, pen_pos, titleinfo_ptr->fdstitleline);
    pen_pos -= titleinfo_ptr->text_height;
    pen_pos -= titleinfo_ptr->line_space;
  }
  if(vis_title_CHID==1){
    OutputText(left, pen_pos, titleinfo_ptr->chidline);
    pen_pos -= titleinfo_ptr->text_height;
    pen_pos -= titleinfo_ptr->line_space;
  }

  int i;
  for(i = 0; i < titleinfo_ptr->nlines; i++){
    OutputText(left, pen_pos, titleinfo_ptr->lines[i]);
    pen_pos -= titleinfo_ptr->text_height;
    pen_pos -= titleinfo_ptr->line_space;
  }
  return 0;
}
