#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

GLUI *glui_colorbar=NULL;

GLUI_Panel *PANEL_cb1=NULL;
GLUI_Panel *PANEL_cb2R2=NULL;
GLUI_Panel *PANEL_cb4=NULL;
GLUI_Panel *PANEL_point=NULL;
GLUI_Panel *PANEL_cb5=NULL;
GLUI_Panel *PANEL_cb10=NULL;
GLUI_Panel *PANEL_cb11r=NULL;
GLUI_Panel *PANEL_cb12 = NULL;
#ifdef pp_COLOR_TOGGLE
GLUI_Panel *PANEL_toggle_cb = NULL;
#endif

GLUI_Listbox *LISTBOX_colorbar=NULL;
#ifdef pp_COLOR_TOGGLE
GLUI_Listbox *LISTBOX_colorbar1 = NULL;
GLUI_Listbox *LISTBOX_colorbar2 = NULL;
extern GLUI_Listbox *LISTBOX_colorbar1a;
extern GLUI_Listbox *LISTBOX_colorbar2a;
#endif
extern GLUI_Listbox *LISTBOX_colorbar2a;

GLUI_Spinner *SPINNER_rgb[3];
#ifdef pp_COLOR_CIE
GLUI_Spinner *SPINNER_Lab2[3];
GLUI_Spinner *SPINNER_rgb2[3];
#endif
GLUI_Spinner *SPINNER_valmin=NULL;
GLUI_Spinner *SPINNER_valmax=NULL;
GLUI_Spinner *SPINNER_val=NULL;
GLUI_Spinner *SPINNER_colorindex=NULL;

GLUI_Button *BUTTON_node_next=NULL,*BUTTON_node_prev=NULL;
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
#ifdef pp_COLOR_TOGGLE
GLUI_Button *BUTTON_colorbar1 = NULL;
GLUI_Button *BUTTON_colorbar2 = NULL;
extern GLUI_Button *BUTTON_colorbar1a;
extern GLUI_Button *BUTTON_colorbar2a;
#endif

GLUI_RadioGroup *RADIO_colorbar_coord_type;

GLUI_Checkbox *CHECKBOX_hidesv=NULL;
#ifdef pp_COLOR_CIE
GLUI_Checkbox *CHECKBOX_cb_interp = NULL;
#endif

GLUI_EditText *EDITTEXT_colorbar_label=NULL;

GLUI_StaticText *STATICTEXT_left=NULL, *STATICTEXT_right=NULL;

int cb_usecolorbar_extreme;

#define COLORBAR_LIST                 0
#define COLORBAR_CLOSE                1
#define COLORBAR_NODE_NEXT            3
#define COLORBAR_NODE_PREV            4
#define COLORBAR_NEW                  5
#define COLORBAR_ADDPOINT             7
#define COLORBAR_DELETEPOINT          8
#define COLORBAR_SAVE                 9
#define COLORBAR_LABEL               10
#define COLORBAR_UPDATE              11
#define COLORBAR_COLORINDEX          12
#define COLORBAR_DELETE              14
#define COLORBAR_EXTREME             16
#define COLORBAR_UNIFORM             17
#define COLORBAR_PREV                21
#define COLORBAR_NEXT                22
#ifdef pp_COLOR_CIE
#define COLORBAR_ADJUST              23
#define COLORBAR_REVERT              24
#define COLORBAR_CIE_OUTPUT          25
#define COLORBAR_LAB2                26
#define COLORBAR_RGB2                27
#endif

/* ------------------ UpdateColorbarList ------------------------ */

extern "C" void UpdateColorbarList(void){
  LISTBOX_colorbar->set_int_val(selectedcolorbar_index);
}

/* ------------------ UpdateColorbarType ------------------------ */

extern "C" void UpdateColorbarType(void){
  if(LISTBOX_colorbar!=NULL)LISTBOX_colorbar->set_int_val(colorbartype);
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
#ifndef pp_CLOSEOFF
    glui_colorbar->hide();
#endif
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
#ifdef pp_COLOR_CIE
  unsigned char rgb_local[3];
#endif

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

      rgb2_local = cbi->rgb_node_orig + 3 * i;
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

      rnew = cbi->rgb_node_orig + 3 * colorbarpoint;
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
#ifdef pp_COLOR_CIE
  case COLORBAR_RGB2:
    FRgb2CIE(cb_frgb2, cb_lab2);
    SPINNER_Lab2[0]->set_float_val(cb_lab2[0]);
    SPINNER_Lab2[1]->set_float_val(cb_lab2[1]);
    SPINNER_Lab2[2]->set_float_val(cb_lab2[2]);
    break;
  case COLORBAR_LAB2:
    CIE2Rgb(rgb_local, cb_frgb2, cb_lab2);
    SPINNER_rgb2[0]->set_float_val(cb_frgb2[0]);
    SPINNER_rgb2[1]->set_float_val(cb_frgb2[1]);
    SPINNER_rgb2[2]->set_float_val(cb_frgb2[2]);
    break;
#ifdef pp_COLOR_TOGGLE
  case COLORBAR_CB1:
    interp_cielab = 0;
    LISTBOX_colorbar->set_int_val(index_colorbar1);
    ColorbarCB(COLORBAR_ADJUST);
    ColorbarCB(COLORBAR_LIST);
    CHECKBOX_cb_interp->set_int_val(interp_cielab);
    break;
  case COLORBAR_CB2:
    interp_cielab = 1;
    LISTBOX_colorbar->set_int_val(index_colorbar2);
    ColorbarCB(COLORBAR_ADJUST);
    ColorbarCB(COLORBAR_LIST);
    CHECKBOX_cb_interp->set_int_val(interp_cielab);
    break;
#endif
#endif
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
    int list_index;

    list_index = LISTBOX_colorbar->get_int_val();
    if(list_index<0)break;
    selectedcolorbar_index2 = list_index;
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
#ifdef pp_COLOR_TOGGLE
  case COLORBAR_LISTA:
    if(BUTTON_colorbar1 != NULL)BUTTON_colorbar1->set_name(colorbarinfo[index_colorbar1].label);
    if(BUTTON_colorbar2 != NULL)BUTTON_colorbar2->set_name(colorbarinfo[index_colorbar2].label);
    if(BUTTON_colorbar1a!=NULL)BUTTON_colorbar1a->set_name(colorbarinfo[index_colorbar1].label);
    if(BUTTON_colorbar2a != NULL)BUTTON_colorbar2a->set_name(colorbarinfo[index_colorbar2].label);
    if(LISTBOX_colorbar1!=NULL)LISTBOX_colorbar1->set_int_val(index_colorbar1);
    if(LISTBOX_colorbar1a!=NULL)LISTBOX_colorbar1a->set_int_val(index_colorbar1);
    break;
  case COLORBAR_LISTB:
    if(BUTTON_colorbar1 != NULL)BUTTON_colorbar1->set_name(colorbarinfo[index_colorbar1].label);
    if(BUTTON_colorbar2 != NULL)BUTTON_colorbar2->set_name(colorbarinfo[index_colorbar2].label);
    if(BUTTON_colorbar1a!=NULL)BUTTON_colorbar1a->set_name(colorbarinfo[index_colorbar1].label);
    if(BUTTON_colorbar2a != NULL)BUTTON_colorbar2a->set_name(colorbarinfo[index_colorbar2].label);
    if(LISTBOX_colorbar2!=NULL)LISTBOX_colorbar2->set_int_val(index_colorbar2);
    if(LISTBOX_colorbar2a!=NULL)LISTBOX_colorbar2a->set_int_val(index_colorbar2);
    break;
#endif
  case COLORBAR_CLOSE:
    HideGluiColorbar();
    break;
  case COLORBAR_PREV:
  case COLORBAR_NEXT:
    if(var==COLORBAR_PREV)colorbartype--;
    if(var==COLORBAR_NEXT)colorbartype++;
    if(colorbartype<0)colorbartype=ncolorbars-1;
    if(colorbartype>ncolorbars-1)colorbartype=0;
    LISTBOX_colorbar->set_int_val(colorbartype);
    ColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_NODE_NEXT:
  case COLORBAR_NODE_PREV:
  case COLORBAR_SET:
    if(colorbartype < 0 || colorbartype >= ncolorbars)return;
    show_colorbar_hint = 0;
    cbi = colorbarinfo + colorbartype;
    if(var == COLORBAR_NODE_NEXT){
      colorbarpoint++;
      if(colorbarpoint > cbi->nnodes - 1)colorbarpoint = 0;
    }
    else if(var == COLORBAR_NODE_PREV){
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
#ifdef pp_COLOR_TOGGLE
    LISTBOX_colorbar2->add_item(colorbartype, cbi->label);
#endif
    ColorbarCB(COLORBAR_LIST);
    break;
#ifdef pp_COLOR_CIE
  case COLORBAR_CIE_OUTPUT:
    CIEdE2Csv(dEcsv_filename);
    break;
  case COLORBAR_ADJUST:
    AdjustColorBar(colorbarinfo + colorbartype);
    ColorbarCB(COLORBAR_RGB);
    break;
  case COLORBAR_REVERT:
    RevertColorBar(colorbarinfo + colorbartype);
    ColorbarCB(COLORBAR_RGB);
    break;
#endif
  case COLORBAR_DELETE:
    if(colorbartype >= ndefaultcolorbars&&colorbartype < ncolorbars){
      colorbardata *cb_from, *cb_to;

      for(i = colorbartype;i < ncolorbars - 1;i++){
        cb_to = colorbarinfo + i;
        cb_from = cb_to + 1;
        memcpy(cb_to, cb_from, sizeof(colorbardata));
      }
      for(i = colorbartype;i < ncolorbars;i++){
        LISTBOX_colorbar->delete_item(i);
      }
      ncolorbars--;
      for(i = colorbartype;i < ncolorbars;i++){
        cbi = colorbarinfo + i;
        LISTBOX_colorbar->add_item(i, cbi->label);
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

/* ------------------ AddColorbarList2 ------------------------ */

void AddColorbarList2(GLUI_Listbox *LIST_cbar, char *label_arg){
  char cbar_type[256];
  int i;

  strcpy(cbar_type, "***");
  strcat(cbar_type, label_arg);
  strcat(cbar_type, "***");
  LIST_cbar->add_item(-1, cbar_type);
  for(i = 0; i < ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbarinfo + i;
    if(strcmp(cbi->type, label_arg) != 0)continue;
    LIST_cbar->add_item(i, cbi->label);
  }
}

/* ------------------ GluiColorbarSetup ------------------------ */

extern "C" void GluiColorbarSetup(int main_window){
  cb_valmin=0.0;
  cb_valmax=100.0;
  cb_val=50.0;
  cb_colorindex=128;

  if(colorbar_label!=NULL){
    FREEMEMORY(colorbar_label);
  }
  NewMemory((void **)&colorbar_label,sizeof(GLUI_String));
  strcpy(colorbar_label,_("New colorbar"));

  if(glui_colorbar!=NULL){
    glui_colorbar->close();
    glui_colorbar=NULL;
  }
  glui_colorbar = GLUI_Master.create_glui(_("Customize Colorbar"),0,0,0);
  if(showcolorbar_dialog==0)glui_colorbar->hide();

  PANEL_cb2R2 = glui_colorbar->add_panel("",GLUI_PANEL_NONE);
  BUTTON_new=glui_colorbar->add_button_to_panel(PANEL_cb2R2,_("New colorbar"),COLORBAR_NEW,ColorbarCB);
  BUTTON_delete=glui_colorbar->add_button_to_panel(PANEL_cb2R2,_("Delete colorbar"),COLORBAR_DELETE,ColorbarCB);
#ifdef pp_COLOR_CIE
  glui_colorbar->add_button_to_panel(PANEL_cb2R2, _("Adjust colorbar"), COLORBAR_ADJUST, ColorbarCB);
  glui_colorbar->add_button_to_panel(PANEL_cb2R2, _("Output CIElab distances"), COLORBAR_CIE_OUTPUT, ColorbarCB);
#endif
  colorbar_hidescene=1;
  CHECKBOX_hidesv = glui_colorbar->add_checkbox_to_panel(PANEL_cb2R2,_("Hide scene"),&colorbar_hidescene);
#ifdef pp_COLOR_CIE
  RADIO_colorbar_coord_type = glui_colorbar->add_radiogroup_to_panel(PANEL_cb2R2, &colorbar_coord_type);
  glui_colorbar->add_radiobutton_to_group(RADIO_colorbar_coord_type, "rgb");
  glui_colorbar->add_radiobutton_to_group(RADIO_colorbar_coord_type, "cielab");
  CHECKBOX_cb_interp = glui_colorbar->add_checkbox_to_panel(PANEL_cb2R2, "interpolate using cielab", &interp_cielab, COLORBAR_ADJUST, ColorbarCB);
  glui_colorbar->add_button_to_panel(PANEL_cb2R2,_("Revert CIE"), COLORBAR_REVERT, ColorbarCB);
#endif
  PANEL_cb1 = glui_colorbar->add_panel(_("Colorbar"));
  if(ncolorbars>0){
    selectedcolorbar_index=-1;
    LISTBOX_colorbar=glui_colorbar->add_listbox_to_panel(PANEL_cb1,"",&selectedcolorbar_index,COLORBAR_LIST,ColorbarCB);

    AddColorbarList2(LISTBOX_colorbar, "rainbow");
    AddColorbarList2(LISTBOX_colorbar, "linear");
    AddColorbarList2(LISTBOX_colorbar, "divergent");
    AddColorbarList2(LISTBOX_colorbar, "circular");
    AddColorbarList2(LISTBOX_colorbar, "deprecated");
    AddColorbarList2(LISTBOX_colorbar, "user");
    LISTBOX_colorbar->set_int_val(colorbartype);
  }
  EDITTEXT_colorbar_label  = glui_colorbar->add_edittext_to_panel(PANEL_cb1,_("Label"),GLUI_EDITTEXT_TEXT,colorbar_label,COLORBAR_LABEL,ColorbarCB);
  BUTTON_update=glui_colorbar->add_button_to_panel(PANEL_cb1,_("Update label"),COLORBAR_UPDATE,ColorbarCB);
  BUTTON_autonodes=glui_colorbar->add_button_to_panel(PANEL_cb1,_("Distribute nodes uniformly"),COLORBAR_UNIFORM,ColorbarCB);
  PANEL_cb11r     = glui_colorbar->add_panel_to_panel(PANEL_cb1,"",GLUI_PANEL_NONE);
  BUTTON_prev     = glui_colorbar->add_button_to_panel(PANEL_cb11r, _("Previous"), COLORBAR_PREV, ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb11r,false);
  BUTTON_next     = glui_colorbar->add_button_to_panel(PANEL_cb11r, _("Next"),     COLORBAR_NEXT, ColorbarCB);
#ifdef pp_COLOR_TOGGLE
  int i;

  PANEL_toggle_cb = glui_colorbar->add_panel(_("toggle colorbars"));
  LISTBOX_colorbar1=glui_colorbar->add_listbox_to_panel(PANEL_toggle_cb,"colorbar 1", &index_colorbar1, COLORBAR_LISTA, ColorbarCB);
  for(i=0;i<ncolorbars;i++){
    colorbardata *cbi;

    cbi = colorbarinfo + i;
    LISTBOX_colorbar1->add_item(i,cbi->label);
  }
  LISTBOX_colorbar1->set_int_val(index_colorbar1);

  LISTBOX_colorbar2=glui_colorbar->add_listbox_to_panel(PANEL_toggle_cb,"colorbar 2",&index_colorbar2, COLORBAR_LISTB, ColorbarCB);
  for(i=0;i<ncolorbars;i++){
    colorbardata *cbi;

    cbi = colorbarinfo + i;
    LISTBOX_colorbar2->add_item(i,cbi->label);
  }
  LISTBOX_colorbar2->set_int_val(index_colorbar2);

  BUTTON_colorbar1 = glui_colorbar->add_button_to_panel(PANEL_toggle_cb, _("colorbar 1"), COLORBAR_CB1, ColorbarCB);
  ColorbarCB(COLORBAR_LISTA);

  BUTTON_colorbar2 = glui_colorbar->add_button_to_panel(PANEL_toggle_cb, _("colorbar 2"), COLORBAR_CB2, ColorbarCB);
  ColorbarCB(COLORBAR_LISTB);
#endif
  PANEL_point = glui_colorbar->add_panel(_("Node"));

  PANEL_cb5 = glui_colorbar->add_panel_to_panel(PANEL_point,"",GLUI_PANEL_NONE);

  BUTTON_node_prev=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Previous"),COLORBAR_NODE_PREV,ColorbarCB);
  BUTTON_deletepoint=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Delete"),COLORBAR_DELETEPOINT,ColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb5,false);

  BUTTON_node_next=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Next"),COLORBAR_NODE_NEXT,ColorbarCB);
  BUTTON_addpoint=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Insert"),COLORBAR_ADDPOINT,ColorbarCB);

  PANEL_cb4 = glui_colorbar->add_panel_to_panel(PANEL_point,"",GLUI_PANEL_NONE);
  SPINNER_colorindex=  glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("node index"),  GLUI_SPINNER_INT,&cb_colorindex,  COLORBAR_COLORINDEX,ColorbarCB);
  SPINNER_colorindex->set_int_limits(0,255);
  SPINNER_rgb[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb4, _("red"), GLUI_SPINNER_INT, cb_rgb, COLORBAR_RGB, ColorbarCB);
  SPINNER_rgb[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("green"),GLUI_SPINNER_INT,cb_rgb+1,COLORBAR_RGB,ColorbarCB);
  SPINNER_rgb[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("blue"), GLUI_SPINNER_INT,cb_rgb+2,COLORBAR_RGB,ColorbarCB);

  SPINNER_rgb[0]->set_int_limits(0,255);
  SPINNER_rgb[1]->set_int_limits(0,255);
  SPINNER_rgb[2]->set_int_limits(0,255);

#ifdef pp_COLOR_CIE
  PANEL_cb12 = glui_colorbar->add_panel("rgb<->CIELab");
  cb_frgb2[0] = 0.0;
  cb_frgb2[1] = 0.0;
  cb_frgb2[2] = 0.0;
  SPINNER_rgb2[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb12, _("red"),   GLUI_SPINNER_FLOAT,   cb_frgb2,     COLORBAR_RGB2, ColorbarCB);
  SPINNER_rgb2[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb12, _("green"), GLUI_SPINNER_FLOAT,   cb_frgb2 + 1, COLORBAR_RGB2, ColorbarCB);
  SPINNER_rgb2[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb12, _("blue"),  GLUI_SPINNER_FLOAT,   cb_frgb2 + 2, COLORBAR_RGB2, ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb12,false);
  SPINNER_Lab2[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb12, _("L"),     GLUI_SPINNER_FLOAT, cb_lab2,     COLORBAR_LAB2, ColorbarCB);
  SPINNER_Lab2[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb12, _("a"),     GLUI_SPINNER_FLOAT, cb_lab2 + 1, COLORBAR_LAB2, ColorbarCB);
  SPINNER_Lab2[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb12, _("b"),     GLUI_SPINNER_FLOAT, cb_lab2 + 2, COLORBAR_LAB2, ColorbarCB);
  ColorbarCB(COLORBAR_RGB2);
  SPINNER_rgb2[0]->set_float_limits(0.0, 255.0);
  SPINNER_rgb2[1]->set_float_limits(0.0, 255.0);
  SPINNER_rgb2[2]->set_float_limits(0.0, 255.0);
  SPINNER_Lab2[0]->set_float_limits(0.0,100.0);
  SPINNER_Lab2[1]->set_float_limits(-128.0,128.0);
  SPINNER_Lab2[2]->set_float_limits(-128.0,128.0);
#endif
  ColorbarGlobal2Local();

  PANEL_cb10 = glui_colorbar->add_panel("",GLUI_PANEL_NONE);
  BUTTON_colorbar_save=glui_colorbar->add_button_to_panel(PANEL_cb10,_("Save settings"),COLORBAR_SAVE,ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb10,false);
  BUTTON_colorbar_close=glui_colorbar->add_button_to_panel(PANEL_cb10,_("Close"),COLORBAR_CLOSE,ColorbarCB);
#ifdef pp_CLOSEOFF
  BUTTON_colorbar_close->disable();
#endif

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

  BUTTON_node_next->enable();
  BUTTON_node_prev->enable();

  strcpy(colorbar_label,cbi->label);
  EDITTEXT_colorbar_label->set_text(colorbar_label);
  icolorbar=LISTBOX_colorbar->get_int_val();
  if(icolorbar>=ndefaultcolorbars){
    BUTTON_delete->enable();
    EDITTEXT_colorbar_label->enable();
    BUTTON_update->enable();
    BUTTON_autonodes->enable();
    SPINNER_rgb[0]->enable();
    SPINNER_rgb[1]->enable();
    SPINNER_rgb[2]->enable();
    BUTTON_addpoint->enable();
    BUTTON_deletepoint->enable();
    SPINNER_colorindex->enable();
  }
  else{
    BUTTON_delete->disable();
    EDITTEXT_colorbar_label->disable();
    BUTTON_update->disable();
    BUTTON_autonodes->disable();
    SPINNER_rgb[0]->disable();
    SPINNER_rgb[1]->disable();
    SPINNER_rgb[2]->disable();
    BUTTON_addpoint->disable();
    BUTTON_deletepoint->disable();
    SPINNER_colorindex->disable();
  }
  rgb_local = cbi->rgb_node+3*colorbarpoint;
  SPINNER_rgb[0]->set_int_val(  (int)(rgb_local[0]));
  SPINNER_rgb[1]->set_int_val((int)(rgb_local[1]));
  SPINNER_rgb[2]->set_int_val( (int)(rgb_local[2]));

  UpdateExtremeVals();

  ExtremeCB(COLORBAR_EXTREME);
}
