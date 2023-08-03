#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

GLUI *glui_colorbar=NULL;

GLUI_Panel *PANEL_toggle = NULL;
GLUI_Panel *PANEL_cb_display = NULL;
GLUI_Panel *PANEL_cb1=NULL;
GLUI_Panel *PANEL_cb2R2=NULL;
GLUI_Panel *PANEL_cb4=NULL;
GLUI_Panel *PANEL_point=NULL;
GLUI_Panel *PANEL_cb5=NULL;
GLUI_Panel *PANEL_cb10=NULL;
GLUI_Panel *PANEL_cb11r=NULL;
GLUI_Panel *PANEL_cb12 = NULL;
GLUI_Panel *PANEL_cb13 = NULL;
GLUI_Panel *PANEL_cb14 = NULL;

GLUI_Listbox *LISTBOX_colorbar_edit=NULL;
GLUI_Listbox *LISTBOX_colorbar_toggle_edit1 = NULL;
GLUI_Listbox *LISTBOX_colorbar_toggle_edit2 = NULL;
extern GLUI_Listbox *LISTBOX_colorbar_toggle_bound1;
extern GLUI_Listbox *LISTBOX_colorbar_toggle_bound2;
extern GLUI_Listbox *LISTBOX_colorbar_toggle_bound2;

GLUI_Spinner *SPINNER_rgb[3];
GLUI_Spinner *SPINNER_Lab2[3];
GLUI_Spinner *SPINNER_rgb2[3];
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
GLUI_Button *BUTTON_colorbar_close=NULL;
GLUI_RadioGroup *RADIO_colorbar_coord_type;

GLUI_Checkbox *CHECKBOX_cb_interp = NULL;

GLUI_EditText *EDITTEXT_colorbar_label    =NULL;
GLUI_EditText *EDITTEXT_colorbar_filename = NULL;

GLUI_StaticText *STATICTEXT_left=NULL, *STATICTEXT_right=NULL;

int cb_usecolorbar_extreme;

#define COLORBAR_LIST                 0
#define COLORBAR_CLOSE                1
#define COLORBAR_NODE_NEXT            3
#define COLORBAR_NODE_PREV            4
#define COLORBAR_NEW                  5
#define COLORBAR_ADDPOINT             7
#define COLORBAR_DELETEPOINT          8
#define COLORBAR_SAVE_INI             9
#define COLORBAR_LABEL               10
#define COLORBAR_UPDATE              11
#define COLORBAR_COLORINDEX          12
#define COLORBAR_DELETE              14
#define COLORBAR_EXTREME             16
#define COLORBAR_PREV                21
#define COLORBAR_NEXT                22
#define COLORBAR_ADJUST_LAB          23
#define COLORBAR_ADJUST_L            33
#define COLORBAR_REVERT              24
#define COLORBAR_LAB2                26
#define COLORBAR_RGB2                27
#define COLORBAR_SAVE                32

/* ------------------ UpdateColorbarEdit ------------------------ */

extern "C" void UpdateColorbarEdit(void){
  if(LISTBOX_colorbar_edit != NULL)LISTBOX_colorbar_edit->set_int_val(colorbartype);
  if(LISTBOX_colorbar_toggle_edit1!=NULL)LISTBOX_colorbar_toggle_edit1->set_int_val(index_colorbar1);
  if(LISTBOX_colorbar_toggle_edit2!=NULL)LISTBOX_colorbar_toggle_edit2->set_int_val(index_colorbar2);
}

/* ------------------ UpdateColorbarType ------------------------ */

extern "C" void UpdateColorbarType(void){
  if(LISTBOX_colorbar_edit!=NULL)LISTBOX_colorbar_edit->set_int_val(colorbartype);
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

/* ------------------ SetColorbarListEdit ------------------------ */

extern "C" void SetColorbarListEdit(int val){
  if(LISTBOX_colorbar_edit!=NULL)LISTBOX_colorbar_edit->set_int_val(val);
}

/* ------------------ Colorbar2File ------------------------ */

void Colorbar2File(colorbardata *cbi, char *file, char *label){
  FILE *stream = NULL;
  int i;

  // values consistent with http://colormine.org/convert/rgb-to-lab

  if(file != NULL && strlen(file) > 0 && label != NULL && strlen(label) > 0)stream = fopen(file, "w");
  if(stream == NULL)return;
  fprintf(stream, "name,%s\n", label);
  for(i = 0;i < 256;i++){
    float *rgbi;
    int rgb255[3];
    float cie[3];
    unsigned char rgbc[3];

    rgbi = cbi->colorbar + 3 * i;
    rgb255[0] = rgbi[0] * 255.0;
    rgb255[1] = rgbi[1] * 255.0;
    rgb255[2] = rgbi[2] * 255.0;
    rgbc[0] = (unsigned char)CLAMP(rgb255[0], 0, 255);
    rgbc[1] = (unsigned char)CLAMP(rgb255[1], 0, 255);
    rgbc[2] = (unsigned char)CLAMP(rgb255[2], 0, 255);
    Rgb2CIE(rgbc, cie);
    fprintf(stream, "%i,%i,%i,%f,%f,%f\n", rgb255[0], rgb255[1], rgb255[2], cie[0], cie[1], cie[2]);
  }
  fclose(stream);
}

/* ------------------ ColorbarCB ------------------------ */

extern "C" void ColorbarCB(int var){
  colorbardata *cbi;
  unsigned char *rgb_nodes;
  int i;
  unsigned char rgb_local[3];

  switch(var){
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
      LISTBOX_colorbar_edit->delete_item(colorbartype);
      LISTBOX_colorbar_edit->add_item(colorbartype, colorbar_label);
      LISTBOX_colorbar_edit->set_int_val(0);
      LISTBOX_colorbar_edit->set_int_val(colorbartype);
      updatemenu = 1;
    }
    break;
  case COLORBAR_SAVE_INI:
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
  case COLORBAR_TOGGLE:
    colorbar_toggle = 1 - colorbar_toggle;
    if(colorbar_toggle == 0){
      LISTBOX_colorbar_edit->set_int_val(index_colorbar1);
    }
    else{
      LISTBOX_colorbar_edit->set_int_val(index_colorbar2);
    }
    ColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_RGB:
    if(colorbartype < 0 || colorbartype >= ncolorbars)return;
    cbi = colorbarinfo + colorbartype;
    if(colorbarpoint<0 || colorbarpoint>cbi->nnodes - 1)return;
    rgb_nodes = cbi->rgb_node + 3 * colorbarpoint;

    memcpy(rgb_nodes, cb_rgb, 3);
    RemapColorbar(cbi);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    break;
  case COLORBAR_LIST:
    int list_index;

    list_index = LISTBOX_colorbar_edit->get_int_val();
    if(list_index<0)break;
    colorbartype = list_index;
    if(show_firecolormap!=0)fire_colorbar_index= colorbartype;
    SetColorbarListBound(colorbartype);
    ColorbarMenu(colorbartype);
    ColorbarGlobal2Local();
    break;
  case COLORBAR_LISTA:
    if(LISTBOX_colorbar_toggle_edit1!=NULL)LISTBOX_colorbar_toggle_edit1->set_int_val(index_colorbar1);
    if(LISTBOX_colorbar_toggle_bound1!=NULL)LISTBOX_colorbar_toggle_bound1->set_int_val(index_colorbar1);
    break;
  case COLORBAR_LISTB:
    if(LISTBOX_colorbar_toggle_edit2!=NULL)LISTBOX_colorbar_toggle_edit2->set_int_val(index_colorbar2);
    if(LISTBOX_colorbar_toggle_bound2!=NULL)LISTBOX_colorbar_toggle_bound2->set_int_val(index_colorbar2);
    break;
  case COLORBAR_CLOSE:
    HideGluiColorbar();
    break;
  case COLORBAR_PREV:
  case COLORBAR_NEXT:
    colorbartype = colorbar_list_inverse[colorbartype];
    if(var==COLORBAR_PREV)colorbartype--;
    if(var==COLORBAR_NEXT)colorbartype++;
    if(colorbartype<0)colorbartype= max_LISTBOX_colorbar_edit;
    if(colorbartype> max_LISTBOX_colorbar_edit)colorbartype=0;
    colorbartype = colorbar_list_sorted[colorbartype];
    LISTBOX_colorbar_edit->set_int_val(colorbartype);
    ColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_NODE_NEXT:
  case COLORBAR_NODE_PREV:
  case COLORBAR_SET:
    if(colorbartype < 0 || colorbartype >= ncolorbars)return;
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
    colorbartype = AddColorbar(colorbartype);
    UpdateCurrentColorbar(colorbarinfo + colorbartype);
    UpdateColorbarSplits(current_colorbar);
    ColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_ADJUST_LAB:
    AdjustColorBar(colorbarinfo + colorbartype, COLOR_DIST_LAB);
    ColorbarCB(COLORBAR_RGB);
    break;
  case COLORBAR_ADJUST_L:
    AdjustColorBar(colorbarinfo + colorbartype, COLOR_DIST_L);
    ColorbarCB(COLORBAR_RGB);
    break;
  case COLORBAR_REVERT:
    RevertColorBar(colorbarinfo + colorbartype);
    ColorbarCB(COLORBAR_RGB);
    break;
  case COLORBAR_SAVE:
    cbi = colorbarinfo + colorbartype;
    Colorbar2File(cbi, colorbar_filename, colorbar_label);
    break;
  case COLORBAR_DELETE:
    if(colorbartype >= ndefaultcolorbars&&colorbartype < ncolorbars){
      colorbardata *cb_from, *cb_to;

      for(i = colorbartype;i < ncolorbars - 1;i++){
        cb_to = colorbarinfo + i;
        cb_from = cb_to + 1;
        memcpy(cb_to, cb_from, sizeof(colorbardata));
      }
      for(i = colorbartype;i < ncolorbars;i++){
        LISTBOX_colorbar_edit->delete_item(i);
      }
      ncolorbars--;
      for(i = colorbartype;i < ncolorbars;i++){
        cbi = colorbarinfo + i;
        LISTBOX_colorbar_edit->add_item(i, cbi->label);
      }
      if(colorbartype == ncolorbars)colorbartype--;
      LISTBOX_colorbar_edit->set_int_val(0);
      ColorbarCB(COLORBAR_LIST);
      UpdateColorbarDialogs();
    }
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ AddColorbarListEdit ------------------------ */

void AddColorbarListEdit(GLUI_Listbox *LIST_cbar, int index, char *label_arg, int *max_index){
  char cbar_type[256];
  int i, nitems=0;

 
  for(i = 0; i < ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbarinfo + i;
    if(strcmp(cbi->ctype, label_arg) != 0)continue;
    nitems++;
    break;
  }
  if(nitems == 0)return;
  strcpy(cbar_type, "----------");
  strcat(cbar_type, label_arg);
  strcat(cbar_type, "----------");
  LIST_cbar->add_item(index, cbar_type);
  for(i = 0; i < ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbarinfo + colorbar_list_sorted[i];
    if(strcmp(cbi->ctype, label_arg) != 0)continue;
    LIST_cbar->add_item(colorbar_list_sorted[i], cbi->label);
    *max_index = MAX(colorbar_list_sorted[i], *max_index);
  }
}

/* ------------------ UpdateColorbarListEdit ------------------------ */

extern "C" void UpdateColorbarListEdit(int flag, int del){
  int i;
  char label[64];
  GLUI_Listbox *LISTBOX_cb;

  switch(flag){
  case 1:
    LISTBOX_cb = LISTBOX_colorbar_edit;
    break;
  case 2:
    LISTBOX_cb = LISTBOX_colorbar_toggle_edit1;
    break;
  case 3:
    LISTBOX_cb = LISTBOX_colorbar_toggle_edit2;
    break;
  default:
    LISTBOX_cb = LISTBOX_colorbar_edit;
    ASSERT(FFALSE);
    break;
  }
  if(LISTBOX_cb == NULL)return;
  if(del == CB_DELETE){
    for(i = -7; i < ncolorbars; i++){
      LISTBOX_cb->delete_item(i);
    }
  }
  strcpy(label, "rainbow");      AddColorbarListEdit(LISTBOX_cb, -1, label, &max_LISTBOX_colorbar_edit);
  strcpy(label, "original");     AddColorbarListEdit(LISTBOX_cb, -2, label, &max_LISTBOX_colorbar_edit);
  strcpy(label, "linear");       AddColorbarListEdit(LISTBOX_cb, -3, label, &max_LISTBOX_colorbar_edit);
  strcpy(label, "divergent");    AddColorbarListEdit(LISTBOX_cb, -4, label, &max_LISTBOX_colorbar_edit);
  strcpy(label, "circular");     AddColorbarListEdit(LISTBOX_cb, -5, label, &max_LISTBOX_colorbar_edit);
  strcpy(label, "deprecated");   AddColorbarListEdit(LISTBOX_cb, -6, label, &max_LISTBOX_colorbar_edit);
  strcpy(label, "user defined"); AddColorbarListEdit(LISTBOX_cb, -7, label, &max_LISTBOX_colorbar_edit);
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

  if(colorbar_filename != NULL){
    FREEMEMORY(colorbar_filename);
  }
  NewMemory((void **)&colorbar_filename, sizeof(GLUI_String));
  strcpy(colorbar_filename, "colorbar.csv");

  if(glui_colorbar!=NULL){
    glui_colorbar->close();
    glui_colorbar=NULL;
  }
  glui_colorbar = GLUI_Master.create_glui(_("Customize Colorbar"),0,0,0);
  if(showcolorbar_dialog==0)glui_colorbar->hide();

  PANEL_cb1 = glui_colorbar->add_panel(_("Colorbar"));
  PANEL_cb2R2 = glui_colorbar->add_panel_to_panel(PANEL_cb1,"",GLUI_PANEL_NONE);
  BUTTON_delete=glui_colorbar->add_button_to_panel(PANEL_cb2R2,"Delete",COLORBAR_DELETE,ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb2R2, false);
  BUTTON_new=glui_colorbar->add_button_to_panel(PANEL_cb2R2,"Copy Colorbar",COLORBAR_NEW,ColorbarCB);
  colorbar_hidescene=1;
#ifdef pp_COLOR_HIDE
  glui_colorbar->add_checkbox_to_panel(PANEL_cb2R2,_("Hide scene"),&colorbar_hidescene);
#endif
  if(ncolorbars>0){
    colorbartype=0;

    LISTBOX_colorbar_edit=glui_colorbar->add_listbox_to_panel(PANEL_cb1,"",&colorbartype,COLORBAR_LIST,ColorbarCB);
    UpdateColorbarListEdit(1,CB_KEEP);
    LISTBOX_colorbar_edit->set_int_val(colorbartype_default);
  }
  PANEL_cb13 = glui_colorbar->add_panel_to_panel(PANEL_cb1, "", GLUI_PANEL_NONE);
  EDITTEXT_colorbar_label  = glui_colorbar->add_edittext_to_panel(PANEL_cb13,_("Label:"),GLUI_EDITTEXT_TEXT,colorbar_label,COLORBAR_LABEL,ColorbarCB);
  EDITTEXT_colorbar_label->set_w(200);
  glui_colorbar->add_column_to_panel(PANEL_cb13, false);
  BUTTON_update=glui_colorbar->add_button_to_panel(PANEL_cb13,_("Update label"),COLORBAR_UPDATE,ColorbarCB);
  PANEL_cb11r     = glui_colorbar->add_panel_to_panel(PANEL_cb1,"",GLUI_PANEL_NONE);
  BUTTON_prev     = glui_colorbar->add_button_to_panel(PANEL_cb11r, _("Previous"), COLORBAR_PREV, ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb11r,false);
  BUTTON_next     = glui_colorbar->add_button_to_panel(PANEL_cb11r, _("Next"),     COLORBAR_NEXT, ColorbarCB);

  PANEL_point = glui_colorbar->add_panel(_("Node"));
  PANEL_cb5 = glui_colorbar->add_panel_to_panel(PANEL_point,"",GLUI_PANEL_NONE);

  BUTTON_node_prev=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Previous"),COLORBAR_NODE_PREV,ColorbarCB);
  BUTTON_deletepoint=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Delete"),COLORBAR_DELETEPOINT,ColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb5,false);

  BUTTON_node_next=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Next"),COLORBAR_NODE_NEXT,ColorbarCB);
  BUTTON_addpoint=glui_colorbar->add_button_to_panel(PANEL_cb5,_("Insert Before"),COLORBAR_ADDPOINT,ColorbarCB);

  PANEL_cb4 = glui_colorbar->add_panel_to_panel(PANEL_point,"",GLUI_PANEL_NONE);
  SPINNER_colorindex=  glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("node index"),  GLUI_SPINNER_INT,&cb_colorindex,  COLORBAR_COLORINDEX,ColorbarCB);
  SPINNER_colorindex->set_int_limits(0,255);
  SPINNER_rgb[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb4, _("red"), GLUI_SPINNER_INT, cb_rgb, COLORBAR_RGB, ColorbarCB);
  SPINNER_rgb[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("green"),GLUI_SPINNER_INT,cb_rgb+1,COLORBAR_RGB,ColorbarCB);
  SPINNER_rgb[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb4,_("blue"), GLUI_SPINNER_INT,cb_rgb+2,COLORBAR_RGB,ColorbarCB);

  SPINNER_rgb[0]->set_int_limits(0,255);
  SPINNER_rgb[1]->set_int_limits(0,255);
  SPINNER_rgb[2]->set_int_limits(0,255);

  PANEL_toggle = glui_colorbar->add_panel("Toggle colorbar");
  LISTBOX_colorbar_toggle_edit1 = glui_colorbar->add_listbox_to_panel(PANEL_toggle, "", &index_colorbar1, COLORBAR_LISTA, ColorbarCB);
  UpdateColorbarListEdit(2, CB_KEEP);
  LISTBOX_colorbar_toggle_edit1->set_int_val(index_colorbar1);

  LISTBOX_colorbar_toggle_edit2 = glui_colorbar->add_listbox_to_panel(PANEL_toggle, "", &index_colorbar2, COLORBAR_LISTB, ColorbarCB);
  UpdateColorbarListEdit(3, CB_KEEP);
  LISTBOX_colorbar_toggle_edit2->set_int_val(index_colorbar2);

  glui_colorbar->add_button_to_panel(PANEL_toggle, _("toggle"), COLORBAR_TOGGLE, ColorbarCB);
  ColorbarCB(COLORBAR_LISTA);
  ColorbarCB(COLORBAR_LISTB);

  PANEL_cb_display = glui_colorbar->add_panel("Display");
  RADIO_colorbar_coord_type = glui_colorbar->add_radiogroup_to_panel(PANEL_cb_display,&colorbar_coord_type);
  glui_colorbar->add_radiobutton_to_group(RADIO_colorbar_coord_type, "rgb");
  glui_colorbar->add_radiobutton_to_group(RADIO_colorbar_coord_type, "CIELab");
  glui_colorbar->add_checkbox_to_panel(PANEL_cb_display,"Show CIELab equal distance bars", &show_Lab_dist_bars);
  PANEL_cb14 = glui_colorbar->add_panel_to_panel(PANEL_cb_display,"", GLUI_PANEL_NONE);
  glui_colorbar->add_button_to_panel(PANEL_cb14, "Equal Lab distance",    COLORBAR_ADJUST_LAB, ColorbarCB);
  glui_colorbar->add_button_to_panel(PANEL_cb14, "Revert",                COLORBAR_REVERT,     ColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb14, false);
  glui_colorbar->add_button_to_panel(PANEL_cb14, "Equal L distance",      COLORBAR_ADJUST_L,   ColorbarCB);
  glui_colorbar->add_button_to_panel(PANEL_cb14, "Save",          COLORBAR_SAVE,       ColorbarCB);
  EDITTEXT_colorbar_filename = glui_colorbar->add_edittext_to_panel(PANEL_cb_display, "filename:", GLUI_EDITTEXT_TEXT, colorbar_filename);
  EDITTEXT_colorbar_filename->set_w(200);
  UpdateColorbarEdit();

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
  ColorbarGlobal2Local();

  PANEL_cb10 = glui_colorbar->add_panel("",GLUI_PANEL_NONE);
  glui_colorbar->add_button_to_panel(PANEL_cb10,_("Save settings"),COLORBAR_SAVE_INI,ColorbarCB);
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
  icolorbar=LISTBOX_colorbar_edit->get_int_val();

  if(icolorbar>=ndefaultcolorbars){
    BUTTON_delete->enable();
    EDITTEXT_colorbar_label->enable();
    BUTTON_update->enable();
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
