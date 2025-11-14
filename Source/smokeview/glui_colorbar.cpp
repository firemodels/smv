#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>
#include <ctype.h>
#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "glui_motion.h"

#include "colorbars.h"

GLUI *glui_colorbar=NULL;

GLUI_Rollout *ROLLOUT_cb_simple  = NULL;
GLUI_Rollout *ROLLOUT_cb_general = NULL;
GLUI_Rollout *ROLLOUT_cb_display = NULL;

GLUI_Panel *PANEL_cb_toggle   = NULL;
GLUI_Panel *PANEL_cb_select   = NULL;
GLUI_Panel *PANEL_cb_select1  = NULL;
GLUI_Panel *PANEL_cb_select2  = NULL;
GLUI_Panel *PANEL_cb_select3  = NULL;
GLUI_Panel *PANEL_cb_general1 = NULL;
GLUI_Panel *PANEL_cb_general2 = NULL;
GLUI_Panel *PANEL_cb_edit     = NULL;
GLUI_Panel *PANEL_cb_save     = NULL;
GLUI_Panel *PANEL_cb_display1 = NULL;
GLUI_Panel *PANEL_cb_simple   = NULL;
GLUI_Panel *PANEL_cb_equalize = NULL;
GLUI_Panel *PANEL_cb_csv      = NULL;
GLUI_Panel *PANEL_cb_eqcsv    = NULL;

GLUI_Listbox *LISTBOX_cb_edit         = NULL;
GLUI_Listbox *LISTBOX_cb_toggle_edit1 = NULL;
GLUI_Listbox *LISTBOX_cb_toggle_edit2 = NULL;
extern GLUI_Listbox *LISTBOX_cb_toggle_bound1;
extern GLUI_Listbox *LISTBOX_cb_toggle_bound2;
extern GLUI_Listbox *LISTBOX_cb_toggle_bound2;

GLUI_Spinner *SPINNER_cb_simple_rgb[15];
GLUI_Spinner *SPINNER_cb_rgb[3];
GLUI_Spinner *SPINNER_cb_Lab2[3];
GLUI_Spinner *SPINNER_cb_rgb2[3];
GLUI_Spinner *SPINNER_cb_colorindex=NULL;

GLUI_Button *BUTTON_cb_node_next    = NULL;
GLUI_Button *BUTTON_cb_node_prev    = NULL;
GLUI_Button *BUTTON_cb_next         = NULL;
GLUI_Button *BUTTON_cb_prev         = NULL;
GLUI_Button *BUTTON_cb_save_as      = NULL;
GLUI_Button *BUTTON_cb_new          = NULL;
GLUI_Button *BUTTON_cb_delete       = NULL;
GLUI_Button *BUTTON_cb_addpoint     = NULL;
GLUI_Button *BUTTON_cb_deletepoint  = NULL;
GLUI_Button *BUTTON_cb_savesettings = NULL;
GLUI_Button *BUTTON_cb_update       = NULL;
GLUI_Button *BUTTON_cb_close        = NULL;

GLUI_RadioGroup *RADIO_cb_coord_type        = NULL;
GLUI_RadioGroup *RADIO_cb_simple_type       = NULL;
GLUI_RadioButton *RADIOBUTTON_cb_gtr_5nodes = NULL;

GLUI_Checkbox *CHECKBOX_cb_interp    = NULL;
GLUI_Checkbox *CHECKBOX_cb_plot_dist = NULL;

GLUI_EditText *EDITTEXT_cb_label    = NULL;
GLUI_EditText *EDITTEXT_cb_filename = NULL;

GLUI_StaticText *STATICTEXT_cb_node_label = NULL;
GLUI_StaticText *STATICTEXT_cb_adjusted   = NULL;

//#define COLORBAR_LIST                 0 // defined in smokeviewdefs.h
#define COLORBAR_CLOSE                1
#define COLORBAR_NODE_NEXT            3
#define COLORBAR_NODE_PREV            4
#define COLORBAR_COPY                 5
#define COLORBAR_ADDPOINT             7
#define COLORBAR_DELETEPOINT          8
#define COLORBAR_SAVE_INI             9
#define COLORBAR_LABEL               10
#define COLORBAR_COLORINDEX          12
#define COLORBAR_DELETE              14
#define COLORBAR_EXTREME             16
#define COLORBAR_PREV                21
#define COLORBAR_NEXT                22
#define COLORBAR_ADJUST_LAB          23
#define COLORBAR_REVERT              24
#define COLORBAR_LAB2                26
#define COLORBAR_RGB2                27
#define COLORBAR_SAVE_CSV            32
#define COLORBAR_NEW                 34
#define COLORBAR_SIMPLE_RGB          35
#define COLORBAR_SIMPLE_TYPE         36
#define COLORBAR_SIMPLE_ABLE         37
#define COLORBAR_SAVE_AS             38
#define COLORBAR_S0_RGB              39
#define COLORBAR_S1_RGB              40
#define COLORBAR_S2_RGB              41
#define COLORBAR_S3_RGB              42
#define COLORBAR_S4_RGB              43
#define COLORBAR_LAB2GEN             44
#define COLORBAR_CSV_FILENAME        45

/* ------------------ GetCBSimpleType ------------------------ */

int GetCBSimpleType(colorbardata *cbi){
  if(cbi->nnodes > 5)return 6;
  if(cbi->nnodes == 4){
    if(cbi->node_index[1] == cbi->node_index[2])return 5;
    if(cbi->node_index[1] == cbi->node_index[2]-1)return 5;
  }
  return cbi->nnodes - 1;
}

/* ------------------ GLUIUpdateNodeLabel ------------------------ */

void GLUIUpdateNodeLabel(colorbardata *cbi){
  char label_nodes[sizeof(GLUI_String)];

  if(cbi->adjusted == 1){
    strcpy(label_nodes, "Equalized: yes");
    STATICTEXT_cb_adjusted->set_name(label_nodes);
  }
  else{
    strcpy(label_nodes, "Equalized: no");
    STATICTEXT_cb_adjusted->set_name(label_nodes);
  }
  sprintf(label_nodes, "nodes: %i", cbi->nnodes);
  STATICTEXT_cb_node_label->set_name(label_nodes);
}

/* ------------------ ColorbarSimple2General ------------------------ */

void ColorbarGeneral2Simple(colorbardata *cbi){
  int i;

  GLUIUpdateNodeLabel(cbi);
  update_colorbar_dialog = 1;

  if(cbi->nnodes > 5||cbi->nnodes<2){
    for(i = 0;i < 15;i++){
      SPINNER_cb_simple_rgb[i]->disable();
    }
    ROLLOUT_cb_general->open();
    if(cbi->nnodes > 5)colorbar_simple_type = 6;
    if(cbi->nnodes < 2 )colorbar_simple_type = 0;
    RADIO_cb_simple_type->set_int_val(colorbar_simple_type);
    return;
  }
  ROLLOUT_cb_simple->open();
  colorbar_simple_type = GetCBSimpleType(cbi);

  switch(cbi->nnodes){
  default:
    assert(FFALSE);
    break;
  case 2:
    for(i = 0;i < 3;i++){
      cb_simple_rgb[i] = (int)cbi->node_rgb[i];
    }
    for(i = 0;i < 3;i++){
      cb_simple_rgb[12+i] = (int)cbi->node_rgb[3+i];
    }
    break;
  case 3:
    for(i = 0;i < 6;i++){
      cb_simple_rgb[i] = (int)cbi->node_rgb[i];
    }
    for(i = 0;i < 3;i++){
      cb_simple_rgb[12 + i] = (int)cbi->node_rgb[6+i];
    }
    break;
  case 4:
    for(i = 0;i < 9;i++){
      cb_simple_rgb[i] = (int)cbi->node_rgb[i];
    }
    for(i = 0;i < 3;i++){
      cb_simple_rgb[12 + i] = (int)cbi->node_rgb[9+i];
    }
    break;
  case 5:
    for(i = 0;i < 15;i++){
      cb_simple_rgb[i] = (int)cbi->node_rgb[i];
    }
    break;
  }
  for(i = 0;i < 15;i++){
    SPINNER_cb_simple_rgb[i]->set_int_val(cb_simple_rgb[i]);
  }
  RADIO_cb_simple_type->set_int_val(colorbar_simple_type);
}

/* ------------------ ColorbarSimple2General ------------------------ */

void ColorbarSimple2General(colorbardata *cbi){
  int node_rgb[1024 * 3];
  int i;

  for(i = 0;i < 1024 * 3;i++){
    node_rgb[i] = 0;
  }
  switch(colorbar_simple_type){
  case 0: // constant (1 node)
    cbi->nnodes = 1;
    memcpy(node_rgb,   cb_simple_rgb, 3 * sizeof(int));
    memcpy(node_rgb+3, cb_simple_rgb, 3 * sizeof(int));
    cbi->node_index[0] = 0;
    cbi->node_index[1] = 255;
    break;
  case 1: // linear (2 nodes)
    cbi->nnodes = 2;
    memcpy(node_rgb,     cb_simple_rgb,    3 * sizeof(int));
    memcpy(node_rgb + 3, cb_simple_rgb+12, 3 * sizeof(int));
    cbi->node_index[0] = 0;
    cbi->node_index[1] = 255;
    break;
  case 2: // 3 nodes
    cbi->nnodes = 3;
    memcpy(node_rgb,     cb_simple_rgb,      3 * sizeof(int));
    memcpy(node_rgb + 3, cb_simple_rgb + 3,  3 * sizeof(int));
    memcpy(node_rgb + 6, cb_simple_rgb + 12, 3 * sizeof(int));
    cbi->node_index[0] = 0;
    cbi->node_index[1] = 127;
    cbi->node_index[2] = 255;
    break;
  case 3: // 4 nodes
    cbi->nnodes = 4;
    memcpy(node_rgb,     cb_simple_rgb,      3 * sizeof(int));
    memcpy(node_rgb + 3, cb_simple_rgb + 3,  3 * sizeof(int));
    memcpy(node_rgb + 6, cb_simple_rgb + 6,  3 * sizeof(int));
    memcpy(node_rgb + 9, cb_simple_rgb + 12, 3 * sizeof(int));
    cbi->node_index[0] = 0;
    cbi->node_index[1] = 85;
    cbi->node_index[2] = 170;
    cbi->node_index[3] = 255;
    break;
  case 4: // 5 nodes
    cbi->nnodes = 5;
    memcpy(node_rgb, cb_simple_rgb, 15 * sizeof(int));
    cbi->node_index[0] = 0;
    cbi->node_index[1] = 64;
    cbi->node_index[2] = 128;
    cbi->node_index[3] = 192;
    cbi->node_index[4] = 255;
    break;
  case 5: // split
    cbi->nnodes = 4;
    memcpy(node_rgb,     cb_simple_rgb,     3 * sizeof(int));
    memcpy(node_rgb + 3, cb_simple_rgb + 3, 3 * sizeof(int));
    memcpy(node_rgb + 6, cb_simple_rgb + 6, 3 * sizeof(int));
    memcpy(node_rgb + 9, cb_simple_rgb + 12, 3 * sizeof(int));
    cbi->node_index[0] = 0;
    cbi->node_index[1] = 127;
    cbi->node_index[2] = 128;
    cbi->node_index[3] = 255;
    break;
  default:
    assert(FFALSE);
    break;
  }
  for(i=0;i<3*cbi->nnodes;i++){
    cbi->node_rgb[i] = (unsigned char)node_rgb[i];
  }
}

/* ------------------ GLUIUpdateColorbarEdit ------------------------ */

extern "C" void GLUIUpdateColorbarEdit(void){
  if(LISTBOX_cb_edit != NULL)LISTBOX_cb_edit->set_int_val(colorbartype);
  if(LISTBOX_cb_toggle_edit1!=NULL)LISTBOX_cb_toggle_edit1->set_int_val(index_colorbar1);
  if(LISTBOX_cb_toggle_edit2!=NULL)LISTBOX_cb_toggle_edit2->set_int_val(index_colorbar2);
}

/* ------------------ GLUIUpdateColorbarType ------------------------ */

extern "C" void GLUIUpdateColorbarType(void){
  if(LISTBOX_cb_edit!=NULL)LISTBOX_cb_edit->set_int_val(colorbartype);
}

/* ------------------ GLUIHideColorbar ------------------------ */

extern "C" void GLUIHideColorbar(void){
  GLUICloseRollouts(glui_colorbar);
  viscolorbarpath=0;
  if(show_extreme_mindata_save==1){
    show_extreme_mindata=1;
    GLUIUpdateExtreme();
  }
  if(show_extreme_maxdata_save==1){
    show_extreme_maxdata=1;
    GLUIUpdateExtreme();
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

/* ------------------ GLUIShowColorbar ------------------------ */

extern "C" void GLUIShowColorbar(void){
// show colorbar dialog box and redefine initial view point
  viscolorbarpath=1;
  show_extreme_mindata_save = show_extreme_mindata;
  if(show_extreme_mindata==1){
    show_extreme_mindata=0;
    GLUIUpdateExtreme();
  }
  show_extreme_maxdata_save = show_extreme_maxdata;
  if(show_extreme_maxdata==1){
    show_extreme_maxdata=0;
    GLUIUpdateExtreme();
  }
  if(glui_colorbar!=NULL){
    ReshapeCB(screenWidth,screenHeight);
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
    glui_colorbar->show();
  }
}

/* ------------------ GLUISetColorbarListEdit ------------------------ */

extern "C" void GLUISetColorbarListEdit(int val){
  if(LISTBOX_cb_edit!=NULL)LISTBOX_cb_edit->set_int_val(val);
}

/* ------------------ Colorbar2File ------------------------ */

void Colorbar2File(colorbardata *cbi, char *file, char *label){
  FILE *stream = NULL;
  int i;

  // values consistent with http://colormine.org/convert/rgb-to-lab

  if(file != NULL && strlen(file) > 0 && label != NULL && strlen(label) > 0)stream = FOPEN(file, "w");
  if(stream == NULL)return;
  fprintf(stream, "name,%s\n", label);
  for(i = 0;i < 256;i++){
    float *rgbi;
    int rgb255[3];
    float lab[3];
    unsigned char rgbc[3];

    rgbi = cbi->colorbar_rgb + 3 * i;
    rgb255[0] = rgbi[0] * 255.0;
    rgb255[1] = rgbi[1] * 255.0;
    rgb255[2] = rgbi[2] * 255.0;
    rgbc[0] = (unsigned char)CLAMP(rgb255[0], 0, 255);
    rgbc[1] = (unsigned char)CLAMP(rgb255[1], 0, 255);
    rgbc[2] = (unsigned char)CLAMP(rgb255[2], 0, 255);
    Rgb2Lab(rgbc, lab);
    fprintf(stream, "%i,%i,%i,%f,%f,%f\n", rgb255[0], rgb255[1], rgb255[2], lab[0], lab[1], lab[2]);
  }
  fclose(stream);
}

/* ------------------ GetNewColorbarName ------------------------ */

void GetNewColorbarName(char *base, char *label, int labellen){
  int i;
  char labelcopy[sizeof(GLUI_String)];

  for(i = 1;;i++){
    int j;

    if(i == 1){
      snprintf(labelcopy, sizeof(GLUI_String), "%s", base);
    }
    else{
      snprintf(labelcopy, sizeof(GLUI_String), "%s %i", base, i);
    }
    strcpy(label, labelcopy);

    int dup = 0;
    for(j = 0;j < colorbars.ncolorbars;j++){
      colorbardata *cbj;

      cbj = colorbars.colorbarinfo + j;
      if(strcmp(label, cbj->menu_label) == 0){
        dup = 1;
        break;
      }
    }
    if(dup == 0)return;
  }
}

/* ------------------ ColorbarSimple ------------------------ */

void ColorbarSimple(int node){
  ColorbarSimple2General(colorbars.colorbarinfo + colorbartype);
  colorbarpoint = node;
  memcpy(cb_rgb, cb_simple_rgb + 3*colorbarpoint, 3*sizeof(int));
  SPINNER_cb_rgb[0]->set_int_val(cb_rgb[0]);
  SPINNER_cb_rgb[1]->set_int_val(cb_rgb[1]);
  SPINNER_cb_rgb[2]->set_int_val(cb_rgb[2]);
  cb_colorindex = colorbars.colorbarinfo[colorbartype].node_index[colorbarpoint];
  SPINNER_cb_colorindex->set_int_val(cb_colorindex);
  GLUIColorbarCB(COLORBAR_RGB);
}

/* ------------------ FilterCSVFilename ------------------------ */

void FilterCSVFilename(char *file){
  char *csv, *newfile;
  int i, lenfile;


  lenfile = strlen(file);
  {
    char *c;

    c = file + strlen(file) - 1;
    if(isalnum(*c) == 0)*c=0;
  }
  for(i = 0;i < lenfile;i++){
    char *c;

    c = file + i;
    if(i==0&&isalpha(*c) == 0)*c = 'a';
    if(i>0&&isalnum(*c) == 0 && *c !='.')*c = '_';
  }
  newfile = file;
  *newfile++ = file[0];
  lenfile = strlen(file);
  for(i = 1;i < lenfile;i++){
    if(file[i - 1] != '_')*newfile++ = file[i];
    if(file[i - 1] == '_' && file[i]!='_')*newfile++ = file[i];
  }
  *newfile = 0;
  csv = strstr(file, ".csv");
  if(csv==NULL){
    strcat(file, ".csv");
  }
  EDITTEXT_cb_filename->set_text(file);
}

/* ------------------ GLUIColorbarCB ------------------------ */

extern "C" void GLUIColorbarCB(int var){
  colorbardata *cbi;
  unsigned char *nodes_rgb;
  int i, type;
  unsigned char rgb_local[3];

  update_colorbar_dialog = 1;
  switch(var){
  case COLORBAR_COLORINDEX:
    if(colorbartype < colorbars.ncolorbars){
      cbi = colorbars.colorbarinfo + colorbartype;
      UpdateCurrentColorbar(cbi);

      cbi->node_index[colorbarpoint] = cb_colorindex;
      cbi->can_adjust = 0;

      GLUIColorbarGlobal2Local();
      RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
      UpdateRGBColors(colorbar_select_index);
      ColorbarGeneral2Simple(cbi);
      cbi->adjusted = 0;
    }
    break;
  case COLORBAR_LABEL:
    if(colorbartype < colorbars.ncolorbars){
      char *clabel;

      cbi = colorbars.colorbarinfo + colorbartype;
      clabel = EDITTEXT_cb_label->get_text();
      strcpy(cbi->menu_label, clabel);
      LISTBOX_cb_edit->delete_item(colorbartype);
      LISTBOX_cb_edit->add_item(colorbartype, colorbar_label);
      int save;
      save = colorbartype;
      LISTBOX_cb_edit->set_int_val(0);
      LISTBOX_cb_edit->set_int_val(save);
      UpdateColorbarDialogs();
      updatemenu = 1;
    }
    break;
  case COLORBAR_SAVE_INI:
    updatemenu = 1;
    WriteIni(LOCAL_INI, NULL);
    break;
  case COLORBAR_ADDPOINT:
    if(colorbartype >= colorbars.ncolorbars)return;
    cbi = colorbars.colorbarinfo + colorbartype;
    if(colorbarpoint <= 0 || colorbarpoint > cbi->nnodes - 1)return;

    cbi->nnodes++;
    if(colorbarpoint < 1)colorbarpoint = 1;
    cbi->nodehilight = colorbarpoint;

    int nsize;

    nsize = (cbi->nnodes - colorbarpoint - 1);
    memmove(cbi->node_index + colorbarpoint + 1, cbi->node_index + colorbarpoint, nsize);
    memmove(cbi->node_rgb + 3*colorbarpoint + 3, cbi->node_rgb + 3*colorbarpoint, 3*nsize);
    {
      unsigned char *rnew;
      unsigned char *inew, *ibef, *iaft;
      float lab1[3], lab2[3], labnew[3], fnew[3];

      rnew = cbi->node_rgb + 3 * colorbarpoint;
      Rgb2Lab(rnew - 3, lab1);
      Rgb2Lab(rnew + 3, lab2);
      labnew[0] = (lab1[0] + lab2[0])/2.0;
      labnew[1] = (lab1[1] + lab2[1])/2.0;
      labnew[2] = (lab1[2] + lab2[2])/2.0;
      Lab2Rgb(rnew, fnew, labnew);

      inew = cbi->node_index + colorbarpoint;
      ibef = inew - 1;
      iaft = inew + 1;
      *inew = (*ibef + *iaft) / 2;
    }

    GLUIColorbarGlobal2Local();
    RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
    UpdateRGBColors(colorbar_select_index);

    if(colorbarpoint == cbi->nnodes)colorbarpoint = cbi->nnodes - 1;
    ColorbarGeneral2Simple(colorbars.colorbarinfo + colorbartype);
    GLUIColorbarCB(COLORBAR_SIMPLE_ABLE);
    break;
  case COLORBAR_DELETEPOINT:
    if(colorbartype >= colorbars.ncolorbars)return;
    cbi = colorbars.colorbarinfo + colorbartype;
    if(colorbarpoint<0 || colorbarpoint>cbi->nnodes - 1)return;
    if(cbi->nnodes <= 2)return;

    if(cbi->nnodes <= 1)return;
    for(i = colorbarpoint + 1;i < cbi->nnodes;i++){
      unsigned char *rgb1, *rgb2_local;

      cbi->node_index[i - 1] = cbi->node_index[i];
      rgb2_local = cbi->node_rgb + 3 * i;
      rgb1 = rgb2_local - 3;
      rgb1[0] = rgb2_local[0];
      rgb1[1] = rgb2_local[1];
      rgb1[2] = rgb2_local[2];
    }
    cbi->nnodes--;
    if(colorbarpoint == cbi->nnodes){
      colorbarpoint = cbi->nnodes - 1;
      cbi->node_index[colorbarpoint] = 255;
    }
    if(colorbarpoint == 0)cbi->node_index[colorbarpoint] = 0;
    RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
    UpdateRGBColors(colorbar_select_index);
    nodes_rgb = cbi->node_rgb + 3 * colorbarpoint;
    for(i = 0;i < 3;i++){
      cb_rgb[i] = nodes_rgb[i];
      SPINNER_cb_rgb[i]->set_int_val(cb_rgb[i]);
    }
    SPINNER_cb_colorindex->set_int_val(cbi->node_index[colorbarpoint]);
    cb_colorindex = cbi->node_index[colorbarpoint];
    ColorbarGeneral2Simple(colorbars.colorbarinfo + colorbartype);
    GLUIColorbarCB(COLORBAR_SIMPLE_ABLE);
    break;
  case COLORBAR_RGB2:
    FRgb2Lab(cb_frgb2, cb_lab2);
    SPINNER_cb_Lab2[0]->set_float_val(cb_lab2[0]);
    SPINNER_cb_Lab2[1]->set_float_val(cb_lab2[1]);
    SPINNER_cb_Lab2[2]->set_float_val(cb_lab2[2]);
    break;
  case COLORBAR_LAB2:
    Lab2Rgb(rgb_local, cb_frgb2, cb_lab2);
    SPINNER_cb_rgb2[0]->set_float_val(cb_frgb2[0]);
    SPINNER_cb_rgb2[1]->set_float_val(cb_frgb2[1]);
    SPINNER_cb_rgb2[2]->set_float_val(cb_frgb2[2]);
    break;
  case COLORBAR_TOGGLE:
    toggle_on = 1;
    ROLLOUT_cb_general->open();
    ROLLOUT_cb_simple->open();
    colorbar_toggle = 1 - colorbar_toggle;
    if(colorbar_toggle == 0){
      LISTBOX_cb_edit->set_int_val(index_colorbar1);
    }
    else{
      LISTBOX_cb_edit->set_int_val(index_colorbar2);
    }
    GLUIColorbarCB(COLORBAR_LIST);
    toggle_on = 0;
    break;
  case COLORBAR_SIMPLE_ABLE:
    if(colorbars.colorbarinfo[colorbartype].nnodes > 5)break;
    switch(colorbar_simple_type){
      default:
      assert(FFALSE);
      break;
      case 0:
      for(type=0;type<5;type++){
        for(i=0;i<3;i++){
          int ii;

          ii = 3*type + i;
          if(type==0){
            SPINNER_cb_simple_rgb[ii]->enable();
          }
          else{
            SPINNER_cb_simple_rgb[ii]->disable();
          }
        }
      }
      break;
      case 1:
      for(type = 0;type < 5;type++){
        for(i=0;i<3;i++){
          int ii;

          ii = 3*type + i;
          if(type==0||type==4){
            SPINNER_cb_simple_rgb[ii]->enable();
          }
          else{
            SPINNER_cb_simple_rgb[ii]->disable();
          }
        }
      }
      break;
      case 2:
      for(type = 0;type < 5;type++){
        for(i=0;i<3;i++){
          int ii;

          ii = 3*type + i;
          if(type == 0 || type==1||type == 4){
            SPINNER_cb_simple_rgb[ii]->enable();
          }
          else{
            SPINNER_cb_simple_rgb[ii]->disable();
          }
        }
      }
      break;
      case 3:
      for(type=0;type<5;type++){
        for(i=0;i<3;i++){
          int ii;

          ii = 3*type + i;
          if(type == 0 || type==1||type==2||type == 4){
            SPINNER_cb_simple_rgb[ii]->enable();
          }
          else{
            SPINNER_cb_simple_rgb[ii]->disable();
          }
        }
      }
      break;
      case 4:
      for(type = 0;type < 5;type++){
        for(i=0;i<3;i++){
          int ii;

          ii = 3*type + i;
          if(type<=4){
            SPINNER_cb_simple_rgb[ii]->enable();
          }
          else{
            SPINNER_cb_simple_rgb[ii]->disable();
          }
        }
      }
      break;
      case 5:
      for(type = 0;type < 5;type++){
        for(i=0;i<3;i++){
          int ii;

          ii = 3*type + i;
          if(type<=2||type==4){
            SPINNER_cb_simple_rgb[ii]->enable();
          }
          else{
            SPINNER_cb_simple_rgb[ii]->disable();
          }
        }
      }
      break;
    }
    break;
  case COLORBAR_SIMPLE_TYPE:
    GLUIColorbarCB(COLORBAR_SIMPLE_ABLE);
    ColorbarSimple2General(colorbars.colorbarinfo + colorbartype);
    GLUIColorbarCB(COLORBAR_LIST);
    ColorbarSimple(0);
    GLUTPOSTREDISPLAY;
    break;
  case COLORBAR_RGB:
    if(colorbartype < 0 || colorbartype >= colorbars.ncolorbars)return;
    cbi = colorbars.colorbarinfo + colorbartype;
    if(colorbarpoint<0 || colorbarpoint>cbi->nnodes - 1)return;
    if(colorbarpoint<=4){
      int index;

      if(colorbarpoint == cbi->nnodes - 1){
        index = 4;
      }
      else{
        index = colorbarpoint;
      }
      memcpy(cb_simple_rgb + 3*index, cb_rgb, 3*sizeof(int));
      SPINNER_cb_simple_rgb[3*index + 0]->set_int_val(cb_rgb[0]);
      SPINNER_cb_simple_rgb[3*index + 1]->set_int_val(cb_rgb[1]);
      SPINNER_cb_simple_rgb[3*index + 2]->set_int_val(cb_rgb[2]);
    }
    nodes_rgb = cbi->node_rgb + 3 * colorbarpoint;
    for(i = 0;i < 3;i++){
      nodes_rgb[i] = cb_rgb[i];
    }
    RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
    UpdateRGBColors(colorbar_select_index);
    break;
  case COLORBAR_S0_RGB:
    ColorbarSimple(0);
    break;
  case COLORBAR_S1_RGB:
    ColorbarSimple(1);
    break;
  case COLORBAR_S2_RGB:
    ColorbarSimple(2);
    break;
  case COLORBAR_S3_RGB:
    ColorbarSimple(3);
    break;
  case COLORBAR_S4_RGB:
    ColorbarSimple(4);
    break;
  case COLORBAR_CSV_FILENAME:
    FilterCSVFilename(colorbar_filename);
    break;
  case COLORBAR_LAB2GEN:
    cb_rgb[0] = CLAMP((int)(cb_frgb2[0] + 0.5), 0, 255);
    cb_rgb[1] = CLAMP((int)(cb_frgb2[1] + 0.5), 0, 255);
    cb_rgb[2] = CLAMP((int)(cb_frgb2[2] + 0.5), 0, 255);
    SPINNER_cb_rgb[0]->set_int_val(cb_rgb[0]);
    SPINNER_cb_rgb[1]->set_int_val(cb_rgb[1]);
    SPINNER_cb_rgb[2]->set_int_val(cb_rgb[2]);
    GLUIColorbarCB(COLORBAR_RGB);
    ROLLOUT_cb_general->open();
    break;
  case COLORBAR_LIST:
    int list_index;

    if(LISTBOX_cb_edit == NULL)break;
    list_index = LISTBOX_cb_edit->get_int_val();
    if(list_index<0)break;
    colorbartype = list_index;
    cbi = colorbars.colorbarinfo + colorbartype;
    if(show_firecolormap!=0)colorbars.fire_colorbar_index= colorbartype;
    GLUISetColorbarListBound(colorbartype);
    ColorbarMenu(colorbartype);
    GLUIColorbarGlobal2Local();
    ColorbarGeneral2Simple(cbi);
    GLUIColorbarCB(COLORBAR_SIMPLE_ABLE);

    char button_label[sizeof(GLUI_String)];
    strcpy(button_label, "Copy to ");
    strcat(button_label, colorbars.colorbarinfo[colorbartype].menu_label);
    strcat(button_label, "_copy");
    BUTTON_cb_save_as->set_name(button_label);
    if(colorbartype < colorbars.ndefaultcolorbars){
      BUTTON_cb_delete ->disable();
    }
    else{
      BUTTON_cb_delete ->enable();
    }
    strcpy(colorbar_filename, cbi->menu_label);
    FilterCSVFilename(colorbar_filename);
    break;
  case COLORBAR_LISTA:
    if(LISTBOX_cb_toggle_edit1!=NULL)LISTBOX_cb_toggle_edit1->set_int_val(index_colorbar1);
    if(LISTBOX_cb_toggle_bound1!=NULL)LISTBOX_cb_toggle_bound1->set_int_val(index_colorbar1);
    break;
  case COLORBAR_LISTB:
    if(LISTBOX_cb_toggle_edit2!=NULL)LISTBOX_cb_toggle_edit2->set_int_val(index_colorbar2);
    if(LISTBOX_cb_toggle_bound2!=NULL)LISTBOX_cb_toggle_bound2->set_int_val(index_colorbar2);
    break;
  case COLORBAR_CLOSE:
    vis_colorbar_dists_plot = 0;
    if(CHECKBOX_cb_plot_dist!=NULL)CHECKBOX_cb_plot_dist->set_int_val(0);
    GLUISliceBoundCB(COLORBAR_PLOT2D);
    GLUIHideColorbar();
    break;
  case COLORBAR_PREV:
  case COLORBAR_NEXT:
    colorbartype = colorbar_list_inverse[colorbartype];
    if(var==COLORBAR_PREV)colorbartype--;
    if(var==COLORBAR_NEXT)colorbartype++;
    if(colorbartype<0)colorbartype= max_LISTBOX_cb_edit;
    if(colorbartype> max_LISTBOX_cb_edit)colorbartype=0;
    colorbartype = colorbar_list_sorted[colorbartype];
    LISTBOX_cb_edit->set_int_val(colorbartype);
    GLUIColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_NODE_NEXT:
  case COLORBAR_NODE_PREV:
  case COLORBAR_SET:
    if(colorbartype < 0 || colorbartype >= colorbars.ncolorbars)return;
    cbi = colorbars.colorbarinfo + colorbartype;
    if(var == COLORBAR_NODE_NEXT){
      colorbarpoint++;
      if(colorbarpoint > cbi->nnodes - 1)colorbarpoint = 0;
    }
    else if(var == COLORBAR_NODE_PREV){
      colorbarpoint--;
      if(colorbarpoint < 0)colorbarpoint = cbi->nnodes - 1;
    }
    cbi->nodehilight = colorbarpoint;
    GLUIColorbarGlobal2Local();
    break;
  case COLORBAR_COPY:
    if(colorbartype < 0 || colorbartype >= colorbars.ncolorbars)return;
    colorbartype = AddColorbar(colorbartype);
    UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
    GLUIColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_NEW:
    colorbartype = colorbars.bw_colorbar_index;
    GLUIColorbarCB(COLORBAR_COPY);
    char newlabel[sizeof(GLUI_String)], temp_label[sizeof(GLUI_String)];
    strcpy(temp_label, "new");
    GetNewColorbarName(temp_label, newlabel, sizeof(GLUI_String));
    EDITTEXT_cb_label->set_text(newlabel);
    GLUIColorbarCB(COLORBAR_LABEL);
    GLUIUpdateColorbarType();
    break;
  case COLORBAR_SAVE_AS:
    if(colorbartype < colorbars.ndefaultcolorbars){
      int cb_save;

      cb_save = colorbartype;
      GLUIColorbarCB(COLORBAR_COPY);
      memcpy(colorbars.colorbarinfo + cb_save, colorbarcopyinfo + cb_save, sizeof(colorbardata));
    }
    break;
  case COLORBAR_ADJUST_LAB:
    AdjustColorBar(colorbars.colorbarinfo + colorbartype);
    // As we have used AdjustColorBar we should set the flat to update the
    // colorbar dialog.
    update_colorbar_dialog = 1;
    GLUIColorbarCB(COLORBAR_RGB);
    break;
  case COLORBAR_REVERT:
    RevertColorBar(colorbars.colorbarinfo + colorbartype);
    GLUIColorbarCB(COLORBAR_LIST);
    break;
  case COLORBAR_SAVE_CSV:
    cbi = colorbars.colorbarinfo + colorbartype;
    Colorbar2File(cbi, colorbar_filename, colorbar_label);
    break;
  case COLORBAR_DELETE:
    if(colorbartype >= colorbars.ndefaultcolorbars&&colorbartype < colorbars.ncolorbars){
      colorbardata *cb_from, *cb_to;

      for(i = colorbartype;i < colorbars.ncolorbars - 1;i++){
        cb_to = colorbars.colorbarinfo + i;
        cb_from = cb_to + 1;
        memcpy(cb_to, cb_from, sizeof(colorbardata));
      }
      for(i = colorbartype;i < colorbars.ncolorbars;i++){
        LISTBOX_cb_edit->delete_item(i);
      }
      colorbars.ncolorbars--;
      for(i = colorbartype;i < colorbars.ncolorbars;i++){
        cbi = colorbars.colorbarinfo + i;
        LISTBOX_cb_edit->add_item(i, cbi->menu_label);
      }
      if(colorbartype == colorbars.ncolorbars)colorbartype--;
      LISTBOX_cb_edit->set_int_val(0);
      GLUIColorbarCB(COLORBAR_LIST);
      UpdateColorbarDialogs();
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ AddColorbarListEdit ------------------------ */

void AddColorbarListEdit(GLUI_Listbox *LIST_cbar, int index, char *label_arg, int *max_index){
  char cbar_type[256];
  int i, nitems=0;


  for(i = 0; i < colorbars.ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + i;
    if(strcmp(cbi->colorbar_type, label_arg) != 0)continue;
    nitems++;
    break;
  }
  if(nitems == 0)return;
  strcpy(cbar_type, "----------");
  strcat(cbar_type, label_arg);
  strcat(cbar_type, "----------");
  LIST_cbar->add_item(index, cbar_type);
  for(i = 0; i < colorbars.ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + colorbar_list_sorted[i];
    if(strcmp(cbi->colorbar_type, label_arg) != 0)continue;
    LIST_cbar->add_item(colorbar_list_sorted[i], cbi->menu_label);
    *max_index = MAX(colorbar_list_sorted[i], *max_index);
  }
}

/* ------------------ GLUIUpdateColorbarListEdit ------------------------ */

extern "C" void GLUIUpdateColorbarListEdit(int flag, int del){
  int i;
  char label[64];
  GLUI_Listbox *LISTBOX_cb=NULL;

  switch(flag){
  case 1:
    LISTBOX_cb = LISTBOX_cb_edit;
    break;
  case 2:
    LISTBOX_cb = LISTBOX_cb_toggle_edit1;
    break;
  case 3:
    LISTBOX_cb = LISTBOX_cb_toggle_edit2;
    break;
  default:
    assert(FFALSE);
    break;
  }
  if(LISTBOX_cb == NULL)return;
  if(del == CB_DELETE){
    for(i = -7; i < colorbars.ncolorbars; i++){
      LISTBOX_cb->delete_item(i);
    }
  }
  strcpy(label, "rainbow");      AddColorbarListEdit(LISTBOX_cb, -1, label, &max_LISTBOX_cb_edit);
  strcpy(label, "original");     AddColorbarListEdit(LISTBOX_cb, -2, label, &max_LISTBOX_cb_edit);
  strcpy(label, "linear");       AddColorbarListEdit(LISTBOX_cb, -3, label, &max_LISTBOX_cb_edit);
  strcpy(label, "divergent");    AddColorbarListEdit(LISTBOX_cb, -4, label, &max_LISTBOX_cb_edit);
  strcpy(label, "circular");     AddColorbarListEdit(LISTBOX_cb, -5, label, &max_LISTBOX_cb_edit);
  strcpy(label, "deprecated");   AddColorbarListEdit(LISTBOX_cb, -6, label, &max_LISTBOX_cb_edit);
  strcpy(label, "user defined"); AddColorbarListEdit(LISTBOX_cb, -7, label, &max_LISTBOX_cb_edit);
}

/* ------------------ GLUIUpdateColorbarList ------------------------ */

extern "C" void GLUIUpdateColorbarList(void){
  GLUIColorbarCB(COLORBAR_LIST);
  GLUIColorbarCB(COLORBAR_LISTA);
  GLUIColorbarCB(COLORBAR_LISTB);
  GLUIColorbarCB(COLORBAR_SIMPLE_TYPE);
}

/* ------------------ GLUIColorbarSetup ------------------------ */

extern "C" void GLUIColorbarSetup(int main_window){
  cb_valmin=0.0;
  cb_valmax=100.0;
  cb_val=50.0;
  cb_colorindex=128;

  if(colorbar_label!=NULL){
    FREEMEMORY(colorbar_label);
  }
  NewMemory((void **)&colorbar_label,sizeof(GLUI_String));
  strcpy(colorbar_label,"New colorbar");

  if(colorbar_filename != NULL){
    FREEMEMORY(colorbar_filename);
  }
  NewMemory((void **)&colorbar_filename, sizeof(GLUI_String));
  strcpy(colorbar_filename, "colorbar.csv");

  if(glui_colorbar!=NULL){
    glui_colorbar->close();
    glui_colorbar=NULL;
  }
  glui_colorbar = GLUI_Master.create_glui("Edit Colorbar",0,dialogX0,dialogY0);
  glui_colorbar->hide();

  PANEL_cb_select = glui_colorbar->add_panel("Colorbar");
  PANEL_cb_select1 = glui_colorbar->add_panel_to_panel(PANEL_cb_select,"",GLUI_PANEL_NONE);
  BUTTON_cb_delete=glui_colorbar->add_button_to_panel(PANEL_cb_select1,"Delete",COLORBAR_DELETE,GLUIColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb_select1, false);
  BUTTON_cb_save_as = glui_colorbar->add_button_to_panel(PANEL_cb_select1, "Save",      COLORBAR_SAVE_AS, GLUIColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb_select1, false);
  glui_colorbar->add_button_to_panel(PANEL_cb_select1,"New",COLORBAR_NEW,GLUIColorbarCB);
  if(colorbars.ncolorbars>0){
    colorbartype=0;

    LISTBOX_cb_edit=glui_colorbar->add_listbox_to_panel(PANEL_cb_select,"",&colorbartype,COLORBAR_LIST,GLUIColorbarCB);
    GLUIUpdateColorbarListEdit(1,CB_KEEP);
    LISTBOX_cb_edit->set_int_val(colorbartype_default);
  }
  PANEL_cb_select2 = glui_colorbar->add_panel_to_panel(PANEL_cb_select, "", GLUI_PANEL_NONE);
  EDITTEXT_cb_label  = glui_colorbar->add_edittext_to_panel(PANEL_cb_select2,"Label:",GLUI_EDITTEXT_TEXT,colorbar_label,COLORBAR_LABEL,GLUIColorbarCB);
  EDITTEXT_cb_label->set_w(200);
  glui_colorbar->add_column_to_panel(PANEL_cb_select2, false);
  BUTTON_cb_update=glui_colorbar->add_button_to_panel(PANEL_cb_select2,"Update label",COLORBAR_LABEL,GLUIColorbarCB);
  PANEL_cb_select3     = glui_colorbar->add_panel_to_panel(PANEL_cb_select,"",GLUI_PANEL_NONE);
  BUTTON_cb_prev     = glui_colorbar->add_button_to_panel(PANEL_cb_select3, "Previous", COLORBAR_PREV, GLUIColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb_select3,false);
  BUTTON_cb_next     = glui_colorbar->add_button_to_panel(PANEL_cb_select3, "Next",     COLORBAR_NEXT, GLUIColorbarCB);

  glui_colorbar->add_checkbox_to_panel(PANEL_cb_select, "Show scene", &colorbar_showscene);

  char label_nodes[sizeof(GLUI_String)];
  strcpy(label_nodes, "nodes");
  STATICTEXT_cb_node_label = glui_colorbar->add_statictext_to_panel(PANEL_cb_select, label_nodes);

  PANEL_cb_edit = glui_colorbar->add_panel("Edit colorbar nodes");

  ROLLOUT_cb_simple = glui_colorbar->add_rollout_to_panel(PANEL_cb_edit, "1->5 nodes");
  char column_label[sizeof(GLUI_String)];

  int i;

  for(i=0;i<15;i++){
    cb_simple_rgb[i] = 0;
  }

  PANEL_cb_simple = glui_colorbar->add_panel_to_panel(ROLLOUT_cb_simple,"",GLUI_PANEL_NONE);

  strcpy(column_label, "               red");
  glui_colorbar->add_statictext_to_panel(PANEL_cb_simple, column_label);

  SPINNER_cb_simple_rgb[12] = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "node 5", GLUI_SPINNER_INT, cb_simple_rgb + 12, COLORBAR_S4_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[9]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "node 4", GLUI_SPINNER_INT, cb_simple_rgb + 9 , COLORBAR_S3_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[6]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "node 3", GLUI_SPINNER_INT, cb_simple_rgb + 6,  COLORBAR_S2_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[3]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "node 2", GLUI_SPINNER_INT, cb_simple_rgb + 3,  COLORBAR_S1_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[0]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "node 1", GLUI_SPINNER_INT, cb_simple_rgb + 0,  COLORBAR_S0_RGB, GLUIColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb_simple,false);
  strcpy(column_label, "               green");
  glui_colorbar->add_statictext_to_panel(PANEL_cb_simple, column_label);
  SPINNER_cb_simple_rgb[13] = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 13, COLORBAR_S4_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[10] = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 10, COLORBAR_S3_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[7]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 7,  COLORBAR_S2_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[4]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 4,  COLORBAR_S1_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[1]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 1,  COLORBAR_S0_RGB, GLUIColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb_simple,false);
  strcpy(column_label, "               blue");
  glui_colorbar->add_statictext_to_panel(PANEL_cb_simple, column_label);
  SPINNER_cb_simple_rgb[14] = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 14, COLORBAR_S4_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[11] = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 11, COLORBAR_S3_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[8]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 8,  COLORBAR_S2_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[5]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 5,  COLORBAR_S1_RGB, GLUIColorbarCB);
  SPINNER_cb_simple_rgb[2]  = glui_colorbar->add_spinner_to_panel(PANEL_cb_simple, "", GLUI_SPINNER_INT, cb_simple_rgb + 2,  COLORBAR_S0_RGB, GLUIColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb_simple, false);
  strcpy(column_label, "               ");
  glui_colorbar->add_statictext_to_panel(PANEL_cb_simple, column_label);
  RADIO_cb_simple_type = glui_colorbar->add_radiogroup_to_panel(PANEL_cb_simple,&colorbar_simple_type, COLORBAR_SIMPLE_TYPE, GLUIColorbarCB);
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, "constant (1 node)");
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, "linear (2 nodes)");
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, "3 nodes");
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, "4 nodes");
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, "5 nodes");
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, "split/divergent");
  RADIOBUTTON_cb_gtr_5nodes = glui_colorbar->add_radiobutton_to_group(RADIO_cb_simple_type, ">5 nodes");
  RADIOBUTTON_cb_gtr_5nodes->disable();

  ROLLOUT_cb_general = glui_colorbar->add_rollout_to_panel(PANEL_cb_edit, "General(2->256 nodes)");
  ROLLOUT_cb_general->close();
  PANEL_cb_general1 = glui_colorbar->add_panel_to_panel(ROLLOUT_cb_general,"",GLUI_PANEL_NONE);

  BUTTON_cb_node_prev=glui_colorbar->add_button_to_panel(PANEL_cb_general1,"Previous",COLORBAR_NODE_PREV,GLUIColorbarCB);
  BUTTON_cb_deletepoint=glui_colorbar->add_button_to_panel(PANEL_cb_general1,"Delete",COLORBAR_DELETEPOINT,GLUIColorbarCB);

  glui_colorbar->add_column_to_panel(PANEL_cb_general1,false);

  BUTTON_cb_node_next=glui_colorbar->add_button_to_panel(PANEL_cb_general1,"Next",COLORBAR_NODE_NEXT,GLUIColorbarCB);
  BUTTON_cb_addpoint=glui_colorbar->add_button_to_panel(PANEL_cb_general1,"Insert Before",COLORBAR_ADDPOINT,GLUIColorbarCB);

  PANEL_cb_general2 = glui_colorbar->add_panel_to_panel(ROLLOUT_cb_general,"",GLUI_PANEL_NONE);
  SPINNER_cb_colorindex=  glui_colorbar->add_spinner_to_panel(PANEL_cb_general2,"node index",  GLUI_SPINNER_INT, &cb_colorindex,  COLORBAR_COLORINDEX,GLUIColorbarCB);
  SPINNER_cb_colorindex->set_int_limits(0,255);
  SPINNER_cb_rgb[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb_general2, "red",  GLUI_SPINNER_INT, cb_rgb,   COLORBAR_RGB, GLUIColorbarCB);
  SPINNER_cb_rgb[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb_general2,"green", GLUI_SPINNER_INT, cb_rgb+1, COLORBAR_RGB, GLUIColorbarCB);
  SPINNER_cb_rgb[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb_general2,"blue",  GLUI_SPINNER_INT, cb_rgb+2, COLORBAR_RGB, GLUIColorbarCB);


  SPINNER_cb_rgb[0]->set_int_limits(0,255);
  SPINNER_cb_rgb[1]->set_int_limits(0,255);
  SPINNER_cb_rgb[2]->set_int_limits(0,255);

  ROLLOUT_cb_display = glui_colorbar->add_rollout("Display");
  ROLLOUT_cb_display->close();
  RADIO_cb_coord_type = glui_colorbar->add_radiogroup_to_panel(ROLLOUT_cb_display,&colorbar_coord_type);
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_coord_type, "RGB");
  glui_colorbar->add_radiobutton_to_group(RADIO_cb_coord_type, "CIELab");
  glui_colorbar->add_checkbox_to_panel(ROLLOUT_cb_display,"Show CIELab equal distance bars", &show_Lab_dist_bars);
  CHECKBOX_cb_plot_dist = glui_colorbar->add_checkbox_to_panel(ROLLOUT_cb_display, "Show CIELab distance plot", &vis_colorbar_dists_plot, COLORBAR_PLOT2D, GLUISliceBoundCB);

  PANEL_cb_toggle = glui_colorbar->add_panel_to_panel(ROLLOUT_cb_display, "Toggle");
  LISTBOX_cb_toggle_edit1 = glui_colorbar->add_listbox_to_panel(PANEL_cb_toggle, "", &index_colorbar1, COLORBAR_LISTA, GLUIColorbarCB);
  GLUIUpdateColorbarListEdit(2, CB_KEEP);
  LISTBOX_cb_toggle_edit1->set_int_val(index_colorbar1);

  LISTBOX_cb_toggle_edit2 = glui_colorbar->add_listbox_to_panel(PANEL_cb_toggle, "", &index_colorbar2, COLORBAR_LISTB, GLUIColorbarCB);
  GLUIUpdateColorbarListEdit(3, CB_KEEP);
  LISTBOX_cb_toggle_edit2->set_int_val(index_colorbar2);

  glui_colorbar->add_button_to_panel(PANEL_cb_toggle, "toggle", COLORBAR_TOGGLE, GLUIColorbarCB);
  update_colorbar_list = 1;

  PANEL_cb_display1 = glui_colorbar->add_panel_to_panel(ROLLOUT_cb_display, "RGB<->CIELab");
  cb_frgb2[0] = 0.0;
  cb_frgb2[1] = 0.0;
  cb_frgb2[2] = 0.0;
  SPINNER_cb_rgb2[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb_display1, "red",   GLUI_SPINNER_FLOAT, cb_frgb2,     COLORBAR_RGB2, GLUIColorbarCB);
  SPINNER_cb_rgb2[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb_display1, "green", GLUI_SPINNER_FLOAT, cb_frgb2 + 1, COLORBAR_RGB2, GLUIColorbarCB);
  SPINNER_cb_rgb2[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb_display1, "blue",  GLUI_SPINNER_FLOAT, cb_frgb2 + 2, COLORBAR_RGB2, GLUIColorbarCB);
  glui_colorbar->add_button_to_panel(PANEL_cb_display1, "Copy red/green/blue to colorbar", COLORBAR_LAB2GEN, GLUIColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb_display1, false);
  SPINNER_cb_Lab2[0] = glui_colorbar->add_spinner_to_panel(PANEL_cb_display1, "L", GLUI_SPINNER_FLOAT, cb_lab2,     COLORBAR_LAB2, GLUIColorbarCB);
  SPINNER_cb_Lab2[1] = glui_colorbar->add_spinner_to_panel(PANEL_cb_display1, "a", GLUI_SPINNER_FLOAT, cb_lab2 + 1, COLORBAR_LAB2, GLUIColorbarCB);
  SPINNER_cb_Lab2[2] = glui_colorbar->add_spinner_to_panel(PANEL_cb_display1, "b", GLUI_SPINNER_FLOAT, cb_lab2 + 2, COLORBAR_LAB2, GLUIColorbarCB);
  GLUIColorbarCB(COLORBAR_RGB2);
  SPINNER_cb_rgb2[0]->set_float_limits(0.0, 255.0);
  SPINNER_cb_rgb2[1]->set_float_limits(0.0, 255.0);
  SPINNER_cb_rgb2[2]->set_float_limits(0.0, 255.0);
  SPINNER_cb_Lab2[0]->set_float_limits(0.0, 100.0);
  SPINNER_cb_Lab2[1]->set_float_limits(-128.0, 128.0);
  SPINNER_cb_Lab2[2]->set_float_limits(-128.0, 128.0);
  GLUIColorbarGlobal2Local();

  PANEL_cb_eqcsv = glui_colorbar->add_panel_to_panel(ROLLOUT_cb_display,"", GLUI_PANEL_NONE);
  PANEL_cb_csv = glui_colorbar->add_panel_to_panel(PANEL_cb_eqcsv,"");
  glui_colorbar->add_button_to_panel(PANEL_cb_csv, "Save colorbar as csv file",              COLORBAR_SAVE_CSV,       GLUIColorbarCB);
  EDITTEXT_cb_filename = glui_colorbar->add_edittext_to_panel(PANEL_cb_csv, "csv filename:", GLUI_EDITTEXT_TEXT, colorbar_filename, COLORBAR_CSV_FILENAME, GLUIColorbarCB);
  EDITTEXT_cb_filename->set_w(200);

  glui_colorbar->add_column_to_panel(PANEL_cb_eqcsv, false);
  PANEL_cb_equalize = glui_colorbar->add_panel_to_panel(PANEL_cb_eqcsv,"");
  glui_colorbar->add_button_to_panel(PANEL_cb_equalize, "Equalize color distances",    COLORBAR_ADJUST_LAB, GLUIColorbarCB);
  STATICTEXT_cb_adjusted = glui_colorbar->add_statictext_to_panel(PANEL_cb_equalize, "");

  GLUIUpdateColorbarEdit();

  PANEL_cb_save = glui_colorbar->add_panel("",GLUI_PANEL_NONE);
  glui_colorbar->add_button_to_panel(PANEL_cb_save,"Save settings",COLORBAR_SAVE_INI,GLUIColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb_save,false);
  glui_colorbar->add_button_to_panel(PANEL_cb_save, "Revert", COLORBAR_REVERT, GLUIColorbarCB);
  glui_colorbar->add_column_to_panel(PANEL_cb_save, false);
  BUTTON_cb_close=glui_colorbar->add_button_to_panel(PANEL_cb_save,"Close",COLORBAR_CLOSE,GLUIColorbarCB);
#ifdef pp_CLOSEOFF
  BUTTON_colorbar_close->disable();
#endif
  if(colorbars.ncolorbars > 0){
    GLUIColorbarCB(COLORBAR_LIST);
  }

  glui_colorbar->set_main_gfx_window( main_window );
}

/* ------------------ GLUIColorbarGlobal2Local ------------------------ */

extern "C" void GLUIColorbarGlobal2Local(void){
  colorbardata *cbi;
  unsigned char *rgb_local;

  if(colorbartype<0||colorbartype>=colorbars.ncolorbars)return;

  cbi = colorbars.colorbarinfo + colorbartype;
  colorbarpoint=cbi->nodehilight;

  if(SPINNER_cb_colorindex == NULL)return;
  SPINNER_cb_colorindex->set_int_val(cbi->node_index[colorbarpoint]);

  BUTTON_cb_node_next->enable();
  BUTTON_cb_node_prev->enable();

  strcpy(colorbar_label,cbi->menu_label);
  EDITTEXT_cb_label->set_text(colorbar_label);

  BUTTON_cb_delete->enable();
  EDITTEXT_cb_label->enable();
  BUTTON_cb_update->enable();
  SPINNER_cb_rgb[0]->enable();
  SPINNER_cb_rgb[1]->enable();
  SPINNER_cb_rgb[2]->enable();
  BUTTON_cb_addpoint->enable();
  BUTTON_cb_deletepoint->enable();
  SPINNER_cb_colorindex->enable();

  rgb_local = cbi->node_rgb+3*colorbarpoint;
  SPINNER_cb_rgb[0]->set_int_val(  (int)(rgb_local[0]));
  SPINNER_cb_rgb[1]->set_int_val((int)(rgb_local[1]));
  SPINNER_cb_rgb[2]->set_int_val( (int)(rgb_local[2]));

  GLUIUpdateExtremeVals();

  GLUIExtremeCB(COLORBAR_EXTREME);
}
