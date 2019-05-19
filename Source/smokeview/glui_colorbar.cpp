#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

GLUI *glui_colorbar=NULL;

GLUI_Panel *PANEL_cb1=NULL;
GLUI_Panel *PANEL_cb2=NULL;
GLUI_Panel *PANEL_cb2L=NULL;
GLUI_Panel *PANEL_cb2R=NULL;
GLUI_Panel *PANEL_cb2R2=NULL;
GLUI_Panel *PANEL_cb3=NULL;
GLUI_Panel *PANEL_cb4=NULL;
GLUI_Panel *PANEL_cb4L=NULL;
GLUI_Panel *PANEL_cb4R=NULL;
GLUI_Panel *PANEL_point=NULL;
GLUI_Panel *PANEL_cb5=NULL;
GLUI_Panel *PANEL_cb5a=NULL;
GLUI_Panel *PANEL_cb5b=NULL;
GLUI_Panel *PANEL_cb6=NULL;
GLUI_Panel *PANEL_cb10=NULL;

GLUI_Listbox *LISTBOX_colorbar=NULL;

GLUI_Spinner *SPINNER_left_red=NULL;
GLUI_Spinner *SPINNER_left_green=NULL;
GLUI_Spinner *SPINNER_left_blue=NULL;
GLUI_Spinner *SPINNER_right_red=NULL;
GLUI_Spinner *SPINNER_right_green=NULL;
GLUI_Spinner *SPINNER_right_blue=NULL;
GLUI_Spinner *SPINNER_valmin=NULL;
GLUI_Spinner *SPINNER_valmax=NULL;
GLUI_Spinner *SPINNER_val=NULL;
GLUI_Spinner *SPINNER_colorindex=NULL;

GLUI_Button *BUTTON_next=NULL,*BUTTON_prev=NULL;
GLUI_Button *BUTTON_new=NULL;
GLUI_Button *BUTTON_delete=NULL;
GLUI_Button *BUTTON_addpoint=NULL;
GLUI_Button *BUTTON_deletepoint=NULL;
GLUI_Button *BUTTON_savesettings=NULL;
GLUI_Button *BUTTON_update=NULL;
GLUI_Button *BUTTON_colorbar_save=NULL;
GLUI_Button *BUTTON_colorbar_close=NULL;
GLUI_Button *BUTTON_autonodes = NULL;

GLUI_Checkbox *CHECKBOX_hidesv=NULL;

GLUI_EditText *EDITTEXT_colorbar_label=NULL;

GLUI_StaticText *STATICTEXT_left=NULL, *STATICTEXT_right=NULL;

int cb_usecolorbar_extreme;

#define COLORBAR_LIST 0
#define COLORBAR_CLOSE 1
#define COLORBAR_NEXT 3
#define COLORBAR_PREV 4
#define COLORBAR_NEW 5
#define COLORBAR_ADDPOINT 7
#define COLORBAR_DELETEPOINT 8
#define COLORBAR_SAVE 9
#define COLORBAR_LABEL 10
#define COLORBAR_UPDATE 11
#define COLORBAR_COLORINDEX 12
#define COLORBAR_DELETE 14
#define COLORBAR_EXTREME 16
#define COLORBAR_UNIFORM 17

/* ------------------ UpdateColorbarList ------------------------ */

extern "C" void UpdateColorbarList(void){
  LISTBOX_colorbar->set_int_val(selectedcolorbar_index);
}

/* ------------------ UpdateColorbarType ------------------------ */

extern "C" void UpdateColorbarType(void){
  LISTBOX_colorbar->set_int_val(colorbartype);
}

/* ------------------ HideGluiColorbar ------------------------ */

extern "C" void HideGluiColorbar(void){
  CloseRollouts(glui_colorbar);
  viscolorbarpath=0;
  showcolorbar_dialog=0;
  if(show_extreme_mindata_save==1){
    show_extreme_mindata=1;
    UpdateExtreme();
  }
  if(show_extreme_maxdata_save==1){
    show_extreme_maxdata=1;
    UpdateExtreme();
  }
  if(glui_colorbar!=NULL){
    CopyCamera(camera_external,camera_external_save);
    ReshapeCB(screenWidth,screenHeight);
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
    glui_colorbar->hide();
  }
  updatemenu=1;
}

/* ------------------ ShowGluiColorbar ------------------------ */

extern "C" void ShowGluiColorbar(void){
// show colorbar dialog box and redefine initial view point
  showcolorbar_dialog=1;
  viscolorbarpath=1;
  show_extreme_mindata_save = show_extreme_mindata;
  if(show_extreme_mindata==1){
    show_extreme_mindata=0;
    UpdateExtreme();
  }
  show_extreme_maxdata_save = show_extreme_maxdata;
  if(show_extreme_maxdata==1){
    show_extreme_maxdata=0;
    UpdateExtreme();
  }
  if(glui_colorbar!=NULL){
    ReshapeCB(screenWidth,screenHeight);
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
    glui_colorbar->show();
  }
}

/* ------------------ ColorbarCB ------------------------ */

extern "C" void ColorbarCB(int var){
  colorbardata *cbi;
  unsigned char *rgb_nodes;
  int i;

  switch(var){
  case COLORBAR_UNIFORM:
    if(colorbartype >= ndefaultcolorbars&&colorbartype < ncolorbars){
      cbi = colorbarinfo + colorbartype;
      UpdateColorbarNodes(cbi);
    }
    break;
  case COLORBAR_COLORINDEX:
    if(colorbartype >= ndefaultcolorbars&&colorbartype < ncolorbars){
      cbi = colorbarinfo + colorbartype;
      UpdateCurrentColorbar(cbi);

      cbi->index_node[colorbarpoint] = cb_colorindex;

      ColorbarGlobal2Local();
      UpdateColorbarSplits(current_colorbar);
      RemapColorbar(cbi);
      UpdateRGBColors(COLORBAR_INDEX_NONE);
    }
    break;
  case COLORBAR_UPDATE:
    ColorbarCB(COLORBAR_LABEL);
    break;
  case COLORBAR_LABEL:
    if(colorbartype >= ndefaultcolorbars&&colorbartype < ncolorbars){
      char *clabel;

      cbi = colorbarinfo + colorbartype;
      clabel = EDITTEXT_colorbar_label->get_text();
      strcpy(cbi->label, clabel);
      LISTBOX_colorbar->delete_item(colorbartype);
      LISTBOX_colorbar->add_item(colorbartype, colorbar_label);
      LISTBOX_colorbar->set_int_val(0);
      LISTBOX_colorbar->set_int_val(colorbartype);
      updatemenu = 1;
    }
    break;
  case COLORBAR_SAVE:
    updatemenu = 1;
    WriteIni(LOCAL_INI, NULL);
    break;
  case COLORBAR_ADDPOINT:
    if(colorbartype < ndefaultcolorbars || colorbartype >= ncolorbars)return;
    cbi = colorbarinfo + colorbartype;
    if(colorbarpoint <= 0 || colorbarpoint > cbi->nnodes - 1)return;

    cbi->nnodes++;
    if(colorbarpoint < 1)colorbarpoint = 1;
    cbi->nodehilight = colorbarpoint;
    for(i = cbi->nnodes - 1;i >= colorbarpoint + 1;i--){
      unsigned char *rgb1, *rgb2_local;

      rgb2_local = cbi->rgb_node + 3 * i;
      rgb1 = rgb2_local - 3;
      rgb2_local[0] = rgb1[0];
      rgb2_local[1] = rgb1[1];
      rgb2_local[2] = rgb1[2];
      cbi->index_node[i] = cbi->index_node[i - 1];
    }
    {
      unsigned char *rnew, *rbef, *raft;
      unsigned char *inew, *ibef, *iaft;

      rnew = cbi->rgb_node + 3 * colorbarpoint;
      rbef = rnew - 3;
      raft = rnew + 3;
      rnew[0] = (int)(((float)rbef[0] + (float)raft[0]) / 2.0);
      rnew[1] = (int)(((float)rbef[1] + (float)raft[1]) / 2.0);
      rnew[2] = (int)(((float)rbef[2] + (float)raft[2]) / 2.0);

      inew = cbi->index_node + colorbarpoint;
      ibef = inew - 1;
      iaft = inew + 1;
      *inew = (*ibef + *iaft) / 2;
    }

    ColorbarGlobal2Local();
    UpdateColorbarSplits(current_colorbar);
    RemapColorbar(cbi);
    UpdateRGBColors(COLORBAR_INDEX_NONE);

    if(colorbarpoint == cbi->nnodes)colorbarpoint = cbi->nnodes - 1;
    break;
  case COLORBAR_DELETEPOINT:
    if(colorbartype < ndefaultcolorbars || colorbartype >= ncolorbars)return;
    cbi = colorbarinfo + colorbartype;
    if(colorbarpoint<0 || colorbarpoint>cbi->nnodes - 1)return;

    if(cbi->nnodes < 2)return;
    for(i = colorbarpoint + 1;i < cbi->nnodes;i++){
      unsigned char *rgb1, *rgb2_local;

      cbi->index_node[i - 1] = cbi->index_node[i];
      rgb2_local = cbi->rgb_node + 3 * i;
      rgb1 = rgb2_local - 3;
      rgb1[0] = rgb2_local[0];
      rgb1[1] = rgb2_local[1];
      rgb1[2] = rgb2_local[2];
    }
    cbi->nnodes--;
    //UpdateColorbarSplits(current_colorbar);
    UpdateColorbarSplits(cbi);
    RemapColorbar(cbi);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    if(colorbarpoint == cbi->nnodes)colorbarpoint = cbi->nnodes - 1;
    break;
  case COLORBAR_RGB:
    show_colorbar_hint = 0;
    if(colorbartype < 0 || colorbartype >= ncolorbars)return;
    cbi = colorbarinfo + colorbartype;
    if(colorbarpoint<0 || colorbarpoint>cbi->nnodes - 1)return;
    rgb_nodes = cbi->rgb_node + 3 * colorbarpoint;

    for(i = 0;i < 3;i++){
      rgb_nodes[i] = cb_rgb[i];
    }
    RemapColorbar(cbi);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    break;
  case COLORBAR_LIST:
    selectedcolorbar_index2 = LISTBOX_colorbar->get_int_val();
    if(show_firecolormap==0){
      colorbartype=selectedcolorbar_index2;
    }
    else{
      fire_colorbar_index= selectedcolorbar_index2;
    }
    UpdateColorbarList2();
    ColorbarMenu(selectedcolorbar_index2);
    ColorbarGlobal2Local();
    break;
  case COLORBAR_CLOSE:
    HideGluiColorbar();
    break;
  case COLORBAR_NEXT:
  case COLORBAR_PREV:
  case COLORBAR_SET:
    if(colorbartype < 0 || colorbartype >= ncolorbars)return;
    show_colorbar_hint = 0;
    cbi = colorbarinfo + colorbartype;
    if(var == COLORBAR_NEXT){
      colorbarpoint++;
      if(colorbarpoint > cbi->nnodes - 1)colorbarpoint = 0;
    }
    else if(var == COLORBAR_PREV){
      colorbarpoint--;
      if(colorbarpoint < 0)colorbarpoint = cbi->nnodes - 1;
    }
    cbi->nodehilight = colorbarpoint;
    ColorbarGlobal2Local();
    break;
  case COLORBAR_NEW:
    if(colorbartype < 0 || colorbartype >= ncolorbars)return;
    AddColorbar(colorbartype);
    colorbartype = ncolorbars - 1;
    UpdateCurrentColorbar(colorbarinfo + colorbartype);
    UpdateColorbarSplits(current_colorbar);
    cbi = colorbarinfo + colorbartype;  //AddColorbar resizes (and possibly moves) colorbarinfo
    LISTBOX_colorbar->add_item(colorbartype, cbi->label);
    LISTBOX_colorbar->set_int_val(colorbartype);
    AddColorbarList2(colorbartype, cbi->label);
    ColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_DELETE:
    if(colorbartype >= ndefaultcolorbars&&colorbartype < ncolorbars){
      colorbardata *cb_from, *cb_to;

      for(i = colorbartype;i < ncolorbars - 1;i++){
        cb_to = colorbarinfo + i;
        cb_from = cb_to + 1;
        memcpy(cb_to, cb_from, sizeof(colorbardata));
        cb_to->label_ptr = cb_to->label;
      }
      for(i = colorbartype;i < ncolorbars;i++){
        LISTBOX_colorbar->delete_item(i);
      }
      ncolorbars--;
      for(i = colorbartype;i < ncolorbars;i++){
        cbi = colorbarinfo + i;
        LISTBOX_colorbar->add_item(i, cbi->label_ptr);
      }
      if(colorbartype == ncolorbars)colorbartype--;
      LISTBOX_colorbar->set_int_val(0);
      ColorbarCB(COLORBAR_LIST);
    }
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ GluiColorbarSetup ------------------------ */

extern "C" void GluiColorbarSetup(int main_window){
  colorbardata *cbi;
  int i;

  cb_valmin=0.0;
  cb_valmax=100.0;
  cb_val=50.0;
  cb_colorindex=128;

  if(colorbar_label!=NULL){
    FREEMEMORY(colorbar_label);
  }
  NewMemory((void **)&colorbar_label,sizeof(GLUI_String));
  strcpy(colorbar_label,_("New colorbar"));

  update_glui_colorbar=0;
  if(glui_colorbar!=NULL){
    glui_colorbar->close();
    glui_colorbar=NULL;
  }
  glui_colorbar = GLUI_Master.create_glui(_("Customize Colorbar"),0,0,0);
  if(showcolorbar_dialog==0)glui_colorbar->hide();

  PANEL_cb2R2 = glui_colorbar->add_panel("",GLUI_PANEL_NONE);
  BUTTON_new=glui_colorbar->add_button_to_panel(PANEL_cb2R2,_("New colorbar"),COLORBAR_NEW,ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb2R2,false);
  BUTTON_delete=glui_colorbar->add_button_to_panel(PANEL_cb2R2,_("Delete colorbar"),COLORBAR_DELETE,ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb2R2,false);
  colorbar_hidescene=1;
  CHECKBOX_hidesv = glui_colorbar->add_checkbox_to_panel(PANEL_cb2R2,_("Hide scene"),&colorbar_hidescene);

  PANEL_cb1 = glui_colorbar->add_panel(_("Colorbar"));
  if(ncolorbars>0){
    selectedcolorbar_index=-1;
    LISTBOX_colorbar=glui_colorbar->add_listbox_to_panel(PANEL_cb1,"",&selectedcolorbar_index,COLORBAR_LIST,ColorbarCB);

    for(i=0;i<ncolorbars;i++){
      cbi = colorbarinfo + i;
      cbi->label_ptr=cbi->label;
      LISTBOX_colorbar->add_item(i,cbi->label_ptr);
    }
    LISTBOX_colorbar->set_int_val(colorbartype);
  }
  EDITTEXT_colorbar_label=glui_colorbar->add_edittext_to_panel(PANEL_cb1,_("Label"),GLUI_EDITTEXT_TEXT,colorbar_label,COLORBAR_LABEL,ColorbarCB);
  BUTTON_update=glui_colorbar->add_button_to_panel(PANEL_cb1,_("Update label"),COLORBAR_UPDATE,ColorbarCB);
  BUTTON_autonodes=glui_colorbar->add_button_to_panel(PANEL_cb1,_("Distribute nodes uniformly"),COLORBAR_UNIFORM,ColorbarCB);

  PANEL_point = glui_colorbar->add_panel(_("Node"));

  PANEL_cb5 = glui_colorbar->add_panel_to_panel(PANEL_point,"",GLUI_PANEL_NONE);

  BUTTON_prev=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Previous"),COLORBAR_PREV,ColorbarCB);
  BUTTON_deletepoint=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Delete"),COLORBAR_DELETEPOINT,ColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb5,false);

  BUTTON_next=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Next"),COLORBAR_NEXT,ColorbarCB);
  BUTTON_addpoint=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Insert"),COLORBAR_ADDPOINT,ColorbarCB);

  PANEL_cb4 = glui_colorbar->add_panel_to_panel(PANEL_point,"",GLUI_PANEL_NONE);
  SPINNER_colorindex=  glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("node index"),  GLUI_SPINNER_INT,&cb_colorindex,  COLORBAR_COLORINDEX,ColorbarCB);
  SPINNER_colorindex->set_int_limits(0,255);
  SPINNER_right_red=  glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("red"),  GLUI_SPINNER_INT,cb_rgb,COLORBAR_RGB,ColorbarCB);
  SPINNER_right_green=glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("green"),GLUI_SPINNER_INT,cb_rgb+1,COLORBAR_RGB,ColorbarCB);
  SPINNER_right_blue= glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("blue"), GLUI_SPINNER_INT,cb_rgb+2,COLORBAR_RGB,ColorbarCB);

  SPINNER_right_red->set_int_limits(0,255);
  SPINNER_right_green->set_int_limits(0,255);
  SPINNER_right_blue->set_int_limits(0,255);

  ColorbarGlobal2Local();

  PANEL_cb10 = glui_colorbar->add_panel("",GLUI_PANEL_NONE);
  BUTTON_colorbar_save=glui_colorbar->add_button_to_panel(PANEL_cb10,_("Save settings"),COLORBAR_SAVE,ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb10,false);
  BUTTON_colorbar_close=glui_colorbar->add_button_to_panel(PANEL_cb10,_("Close"),COLORBAR_CLOSE,ColorbarCB);

  glui_colorbar->set_main_gfx_window( main_window );
}

/* ------------------ ColorbarGlobal2Local ------------------------ */

extern "C" void ColorbarGlobal2Local(void){
  colorbardata *cbi;
  unsigned char *rgb_local;
  int icolorbar;

  if(colorbartype<0||colorbartype>=ncolorbars)return;

  cbi = colorbarinfo + colorbartype;
  colorbarpoint=cbi->nodehilight;

  SPINNER_colorindex->set_int_val(cbi->index_node[colorbarpoint]);

  BUTTON_next->enable();
  BUTTON_prev->enable();

  strcpy(colorbar_label,cbi->label);
  EDITTEXT_colorbar_label->set_text(colorbar_label);
  icolorbar=LISTBOX_colorbar->get_int_val();
  if(icolorbar>=ndefaultcolorbars){
    BUTTON_delete->enable();
    EDITTEXT_colorbar_label->enable();
    BUTTON_update->enable();
    BUTTON_autonodes->enable();
    SPINNER_right_red->enable();
    SPINNER_right_green->enable();
    SPINNER_right_blue->enable();
    BUTTON_addpoint->enable();
    BUTTON_deletepoint->enable();
    SPINNER_colorindex->enable();
  }
  else{
    BUTTON_delete->disable();
    EDITTEXT_colorbar_label->disable();
    BUTTON_update->disable();
    BUTTON_autonodes->disable();
    SPINNER_right_red->disable();
    SPINNER_right_green->disable();
    SPINNER_right_blue->disable();
    BUTTON_addpoint->disable();
    BUTTON_deletepoint->disable();
    SPINNER_colorindex->disable();
  }
  rgb_local = cbi->rgb_node+3*colorbarpoint;
  SPINNER_right_red->set_int_val(  (int)(rgb_local[0]));
  SPINNER_right_green->set_int_val((int)(rgb_local[1]));
  SPINNER_right_blue->set_int_val( (int)(rgb_local[2]));

  UpdateExtremeVals();

  ExtremeCB(COLORBAR_EXTREME);
}
