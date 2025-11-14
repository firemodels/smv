#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewdefs.h"
#include "smokeviewvars.h"
#include "IOscript.h"
#include "dmalloc.h"
#include "glui_smoke.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "histogram.h"

#include "colorbars.h"
#include "paths.h"

void SetResearchMode(int flag);
int GetCacheFlag(int type);
int GetValType(int type);
void SliceInObstDialog2Menu(void);
void GLUIPlot3DBoundCB(int var);

GLUI *glui_bounds=NULL;

#define BOUND_VAL_TYPE                 101
#define BOUND_VALMIN                   102
#define BOUND_VALMAX                   103
#define BOUND_SETVALMIN                104
#define BOUND_SETVALMAX                105
#define BOUND_CHOPMIN                  106
#define BOUND_CHOPMAX                  107
#define BOUND_SETCHOPMIN               108
#define BOUND_SETCHOPMAX               109
//#define BOUND_UPDATE_COLORS            110 // defined in smokeviewdefs.h
#define BOUND_RELOAD_DATA              111
#define BOUND_CACHE_DATA               112
#define BOUND_RESEARCH_MODE            115
#define BOUND_COMPUTE_PERCENTILES      116
#define BOUND_PERCENTILE_MINVAL        117
#define BOUND_PERCENTILE_MAXVAL        118
#define BOUND_COMPUTE_ONLY_PERCENTILES 119
#define BOUND_PERCENTILE_DRAW          120
#define BOUND_LEFT_PERCEN              121
#define BOUND_DOWN_PERCEN              122
#define BOUND_LENGTH_PERCEN            123
#define BOUND_HIST_LABELS              124
#define BOUND_PERCENTILE_MODE          125
#define BOUND_PLOT_MINMAX              126
#define BOUND_COLORBAR_DIGITS          127
//#define BOUND_DONTUPDATE_COLORS        128  // moved to datadefs.h
#define SET_PERCENTILE_MIN_VAL         129
#define SET_PERCENTILE_MAX_VAL         130
#define SET_PERCENTILE_MIN_LEVEL       131
#define SET_PERCENTILE_MAX_LEVEL       132

#define PERCENTILE_DISABLED 0
#define PERCENTILE_ENABLED  1

#define HIDE_CACHE_CHECKBOX 0
#define SHOW_CACHE_CHECKBOX 1

int update_hist_bounds;

GLUI_Panel *PANEL_toggle_cba = NULL;
GLUI_Listbox *LISTBOX_cb_toggle_bound1 = NULL;
GLUI_Listbox *LISTBOX_cb_toggle_bound2 = NULL;
GLUI_Listbox *LISTBOX_sliceload=NULL;
extern GLUI_Spinner *SPINNER_slice_skip2;

/* ------------------ bounds_dialog class ------------------------ */

class bounds_dialog{
  public:

  // variables
  cpp_boundsdata bounds, *all_bounds, *all_bounds_save;
  int   nall_bounds, research_mode_cpp;
  float percentile_min_level, percentile_max_level;
  float plot_min_cpp, plot_max_cpp;

  // widgets
  GLUI_EditText    *EDIT_valmin, *EDIT_valmax, *EDIT_chopmin, *EDIT_chopmax;
  GLUI_Checkbox    *CHECKBOX_set_chopmin, *CHECKBOX_set_chopmax, *CHECKBOX_cache;
  GLUI_Checkbox    *CHECKBOX_research_mode;
  GLUI_Checkbox    *CHECKBOX_hist_show_labels;
  GLUI_RadioGroup  *RADIO_set_valtype,  *RADIO_set_valmin, *RADIO_set_valmax;
  GLUI_RadioButton *RADIO_button_loaded_min, *RADIO_button_loaded_max;
  GLUI_RadioButton *RADIO_button_all_min, *RADIO_button_all_max;
  GLUI_Button      *BUTTON_reload_data;
  GLUI_Panel *PANEL_min, *PANEL_max;
  GLUI_Panel *PANEL_percentile_min, *PANEL_percentile_max;
  GLUI_Button  *BUTTON_percentile_min, *BUTTON_percentile_max;
  GLUI_Spinner *SPINNER_percentile_min_level, *SPINNER_percentile_max_level;
  GLUI_Spinner *SPINNER_plot_min, *SPINNER_plot_max;
  GLUI_Spinner *SPINNER_hist_left_percen, *SPINNER_hist_down_percen, *SPINNER_hist_length_percen;
  GLUI_Spinner *SPINNER_colorbar_digits;
  GLUI_StaticText  *STATIC_min_unit, *STATIC_max_unit, *STATIC_chopmin_unit, *STATIC_chopmax_unit;
  GLUI_Rollout *ROLLOUT_main_bound, *ROLLOUT_truncate;

  // routines
  bounds_dialog(){
    percentile_min_level = 1.0;
    percentile_max_level = 99.0;
  }

  void CB(int var);
  int  get_cache_flag(void);
  void get_global_minmax(char *label, float *valmin, float *valmax);
  int  get_min(char *label, int *set_valmin, float *valmin);
  void get_min_all(int *set_valmin, float *valmin, int *nvals);
  int  get_max(char *label, int *set_valmax, float *valmax);
  void get_max_all(int *set_valmax, float *valmax, int *nvals);
  int  get_nvaltypes(void);
  int  get_valtype(void);
  int  in_research_mode(void);
  cpp_boundsdata *get_bounds_data(void);

  void setup(const char *file_type, GLUI_Rollout *ROLLOUT_dialog, cpp_boundsdata *bounds, int nbounds,
    int *cache_flag, int cache_enable,
    void Callback(int var), GLUI_Update_CB PROC_CB, procdata *procinfo, int *nprocinfo);
  void setupNoGraphics(const char *file_type, cpp_boundsdata *bounds, int nbounds);
  void set_cache_flag(int cache_flag);
  int  get_chopmin(char *label, int *set_valmin, float *valmin);
  int  get_chopmax(char *label, int *set_valmax, float *valmax);
  int  set_chopmin(char *label, int set_valmin, float valmin);
  int  set_chopmax(char *label, int set_valmax, float valmax);
  int  set_min(char *label, int set_valmin, float valmin);
  void set_min_all(int *set_valmin, float *valmin, int nvals);
  void get_global_minmax_all(float *valmin, float *valmax, int nvals);
  void get_loaded_minmax_all(float *valmin, float *valmax, int nvals);
  void set_global_minmax_all(float *valmin, float *valmax, int nvals);
  void set_loaded_minmax_all(float *valmin, float *valmax, int nvals);
  int  set_max(char *label, int set_valmax, float valmax);
  void set_max_all(int *set_valmax, float *valmax, int nvals);
  void set_plot_minmax(float p_min, float p_max);
  void set_plot_minmax_clamp(float p_min, float p_max);
  void set_research_mode(int flag);
  void open_minmax(void);

  void set_colorbar_digits(int ndigits);
  int  set_valtype(char *label);
  void set_valtype_index(int index);
  void SaveBounds(void);
  void RestoreBounds(void);
};

int InResearchMode(void);
int InPercentileMode(void);

/* ------------------ bounds_dialog ------------------------ */

//bounds_dialog::bounds_dialog(void){
//}

/* ------------------ SmvRound ------------------------ */

float SmvRound(float val, int n){
  float return_val;
  char c_val[32];

  Float2String(c_val, val, n, force_fixedpoint);
  sscanf(c_val, "%f", &return_val);

  return return_val;
}

/* ------------------ set_plot_minmax ------------------------ */

void bounds_dialog::set_plot_minmax(float p_min, float p_max){
  float p_min_round, p_max_round;

  p_min_round = SmvRound(p_min, 4);
  p_max_round = SmvRound(p_max, 4);
  if(SPINNER_plot_min!=NULL)SPINNER_plot_min->set_float_val(p_min_round);
  if(SPINNER_plot_max!=NULL)SPINNER_plot_max->set_float_val(p_max_round);
}

/* ------------------ set_plot_minmax_clamp ------------------------ */

void bounds_dialog::set_plot_minmax_clamp(float p_min, float p_max){
  p_min = SmvRound(p_min, 4);
  p_max = SmvRound(p_max, 4);
  if(SPINNER_plot_min!=NULL){
    SPINNER_plot_min->set_float_limits(p_min, p_max);
  }
  if(SPINNER_plot_max!=NULL){
    SPINNER_plot_max->set_float_limits(p_min, p_max);
  }
}

/* ------------------ set_cache_flag ------------------------ */

void bounds_dialog::set_cache_flag(int cache_flag){
  if(cache_flag!=1)cache_flag = 0;
  bounds.cache = cache_flag;
  CB(BOUND_CACHE_DATA);
}

  /* ------------------ get_bounds_data ------------------------ */

cpp_boundsdata *bounds_dialog::get_bounds_data(void){
  return &bounds;
}

/* ------------------ get_cache_flag ------------------------ */

int bounds_dialog::get_cache_flag(void){
  return bounds.cache;
}

  /* ------------------ in_research_mode ------------------------ */

int bounds_dialog::in_research_mode(void){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(boundi->set_valmin!=BOUND_LOADED_MIN&&boundi->set_valmin!=BOUND_GLOBAL_MIN)return 0;
    if(boundi->set_valmax!=BOUND_LOADED_MAX&&boundi->set_valmax!=BOUND_GLOBAL_MAX)return 0;
  }
  return 1;
}

/* ------------------ open_minmax ------------------------ */

void bounds_dialog::open_minmax(void){
  ROLLOUT_main_bound->open();
}

  /* ------------------ set_research_mode ------------------------ */

void bounds_dialog::set_research_mode(int flag){
  int i;

  if(flag==1){
    for(i = 0; i<nall_bounds; i++){
      cpp_boundsdata *boundi;

      boundi = all_bounds+i;
      boundi->set_valmin = BOUND_LOADED_MIN;
      boundi->set_valmax = BOUND_LOADED_MAX;
    }
    CB(BOUND_VAL_TYPE);
  }
  research_mode     = flag;
  research_mode_cpp = flag;
  CHECKBOX_research_mode->set_int_val(research_mode_cpp);
  CB(BOUND_CACHE_DATA);
}

/* ------------------ set_colorbar_digits ------------------------ */

void bounds_dialog::set_colorbar_digits(int ndigits){
  if(SPINNER_colorbar_digits!=NULL)SPINNER_colorbar_digits->set_int_val(ndigits);
}

/* ------------------ SaveBounds ------------------------ */

void bounds_dialog::SaveBounds(void){
  memcpy(all_bounds_save, all_bounds, nall_bounds*sizeof(cpp_boundsdata));
}

/* ------------------ RestoreBounds ------------------------ */

void bounds_dialog::RestoreBounds(void){
  memcpy(all_bounds, all_bounds_save, nall_bounds*sizeof(cpp_boundsdata));
}

void bounds_dialog::setupNoGraphics(const char *file_type, cpp_boundsdata *bounds_arg, int nbounds_arg){

  all_bounds = bounds_arg;
  nall_bounds = nbounds_arg;
  update_ini = 1;

  NewMemory((void **)&all_bounds_save, nall_bounds*sizeof(cpp_boundsdata));
  SaveBounds();
}

/* ------------------ setup ------------------------ */

void bounds_dialog::setup(const char *file_type, GLUI_Rollout *ROLLOUT_dialog, cpp_boundsdata *bounds_arg, int nbounds_arg, int *cache_flag, int cache_enable,
  void Callback(int var),
  GLUI_Update_CB PROC_CB, procdata *procinfo, int *nprocinfo){
  GLUI_Rollout *ROLLOUT_bound;
  GLUI_Panel *PANEL_bound2, *PANEL_minmax, *PANEL_minmax2, *PANEL_buttons;
  GLUI_Panel *PANEL_truncate_min, *PANEL_truncate_max, *PANEL_edit_min, *PANEL_edit_max;
  int i;
  char label1[256], label2[256];

  all_bounds = bounds_arg;
  nall_bounds = nbounds_arg;
  bounds.chopmax = 0.0;
  bounds.chopmin = 0.0;
  bounds.set_chopmax = 0;
  bounds.set_chopmin = 0;
  update_ini = 1;

  plot_min_cpp = 0.0;
  plot_max_cpp = 1.0;
  update_hist_bounds = 1;

  NewMemory((void **)&all_bounds_save, nall_bounds*sizeof(cpp_boundsdata));
  SaveBounds();

  research_mode_cpp = research_mode;

// bound min/max
  char main_label[256];
  if(strcmp(file_type, "hvac") == 0){
    strcpy(main_label, "Bound data");
  }
  else{
    strcpy(main_label, "Bound/Truncate data");
  }
  ROLLOUT_main_bound = glui_bounds->add_rollout_to_panel(ROLLOUT_dialog, main_label, false, 0, PROC_CB);
  TOGGLE_ROLLOUT(procinfo, *nprocinfo, ROLLOUT_main_bound, 0, glui_bounds);

  PANEL_bound2 = glui_bounds->add_panel_to_panel(ROLLOUT_main_bound, "", GLUI_PANEL_NONE);

  RADIO_set_valtype = glui_bounds->add_radiogroup_to_panel(PANEL_bound2, &(bounds.set_valtype), BOUND_VAL_TYPE, Callback);
  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    glui_bounds->add_radiobutton_to_group(RADIO_set_valtype, boundi->label);
  }

  glui_bounds->add_column_to_panel(PANEL_bound2, false);

  ROLLOUT_bound            = glui_bounds->add_rollout_to_panel(PANEL_bound2, "Bound data");
  PANEL_minmax             = glui_bounds->add_panel_to_panel(ROLLOUT_bound, "", GLUI_PANEL_NONE);

  CHECKBOX_cache = NULL;
  if(cache_flag!=NULL){
    bounds.cache = *cache_flag;
  }

  if(strcmp(file_type, "hvac") == 0){
    strcpy(label1, "global");
  }
  else{
    strcpy(label1, "global(loaded ");
    strcat(label1, file_type);
    strcat(label1, " files)");
  }

  strcpy(label2, "global(all ");
  strcat(label2, file_type);
  strcat(label2, " files)");

  CHECKBOX_research_mode   = glui_bounds->add_checkbox_to_panel(PANEL_minmax, "global bounds for all data (research mode)", &research_mode,      BOUND_RESEARCH_MODE,   Callback);
  SPINNER_colorbar_digits  = glui_bounds->add_spinner_to_panel(PANEL_minmax,    "colorbar label digits", GLUI_SPINNER_INT,     &ncolorlabel_digits, BOUND_COLORBAR_DIGITS, Callback);
  SPINNER_colorbar_digits->set_int_limits(COLORBAR_NDECIMALS_MIN, COLORBAR_NDECIMALS_MAX, GLUI_LIMIT_CLAMP);

  PANEL_minmax2 = glui_bounds->add_panel_to_panel(PANEL_minmax, "", GLUI_PANEL_NONE);

  PANEL_min = glui_bounds->add_panel_to_panel(PANEL_minmax2, "min");
  PANEL_edit_min = glui_bounds->add_panel_to_panel(PANEL_min, "", GLUI_PANEL_NONE);
  EDIT_valmin = glui_bounds->add_edittext_to_panel(PANEL_edit_min, "", GLUI_EDITTEXT_FLOAT, &(bounds.glui_valmin), BOUND_VALMIN, Callback);
  glui_bounds->add_column_to_panel(PANEL_edit_min, false);
  STATIC_min_unit = glui_bounds->add_statictext_to_panel(PANEL_edit_min, "");
  STATIC_min_unit->set_w(10);

  RADIO_set_valmin = glui_bounds->add_radiogroup_to_panel(PANEL_min, &(bounds.set_valmin), BOUND_SETVALMIN, Callback);
  glui_bounds->add_radiobutton_to_group(RADIO_set_valmin, "set");
  RADIO_button_loaded_min = glui_bounds->add_radiobutton_to_group(RADIO_set_valmin, label1);
  if(strcmp(file_type, "hvac") != 0){
    RADIO_button_all_min = glui_bounds->add_radiobutton_to_group(RADIO_set_valmin, label2);
  }
  if(strcmp(file_type,"hvac")!=0){
    PANEL_percentile_min = glui_bounds->add_panel_to_panel(PANEL_min, "percentile");
    BUTTON_percentile_min = glui_bounds->add_button_to_panel(PANEL_percentile_min, "Set", SET_PERCENTILE_MIN_VAL, Callback);
    SPINNER_percentile_min_level = glui_bounds->add_spinner_to_panel(PANEL_percentile_min, "level %", GLUI_SPINNER_FLOAT, &percentile_min_level, SET_PERCENTILE_MIN_LEVEL, Callback);
    SPINNER_percentile_min_level->set_float_limits(0.0, 100.0);
  }

  glui_bounds->add_column_to_panel(PANEL_minmax2, false);

  PANEL_max = glui_bounds->add_panel_to_panel(PANEL_minmax2, "max");
  PANEL_edit_max = glui_bounds->add_panel_to_panel(PANEL_max, "", GLUI_PANEL_NONE);
  EDIT_valmax = glui_bounds->add_edittext_to_panel(PANEL_edit_max, "", GLUI_EDITTEXT_FLOAT, &(bounds.glui_valmax), BOUND_VALMAX, Callback);
  glui_bounds->add_column_to_panel(PANEL_edit_max, false);
  STATIC_max_unit = glui_bounds->add_statictext_to_panel(PANEL_edit_max, "");
  STATIC_max_unit->set_w(10);

  RADIO_set_valmax = glui_bounds->add_radiogroup_to_panel(PANEL_max, &(bounds.set_valmax), BOUND_SETVALMAX, Callback);
  glui_bounds->add_radiobutton_to_group(RADIO_set_valmax, "set");
  RADIO_button_loaded_max = glui_bounds->add_radiobutton_to_group(RADIO_set_valmax, label1);
  if(strcmp(file_type, "hvac") != 0){
    RADIO_button_all_max = glui_bounds->add_radiobutton_to_group(RADIO_set_valmax, label2);
  }
  if(strcmp(file_type,"hvac")!=0){
    PANEL_percentile_max = glui_bounds->add_panel_to_panel(PANEL_max, "percentile");
    BUTTON_percentile_max = glui_bounds->add_button_to_panel(PANEL_percentile_max, "Set", SET_PERCENTILE_MAX_VAL, Callback);
    SPINNER_percentile_max_level = glui_bounds->add_spinner_to_panel(PANEL_percentile_max, "level %", GLUI_SPINNER_FLOAT, &percentile_max_level, SET_PERCENTILE_MAX_LEVEL, Callback);
    SPINNER_percentile_max_level->set_float_limits(0.0, 100.0);
  }
  SPINNER_plot_min = NULL;
  SPINNER_plot_max = NULL;
  PANEL_buttons = NULL;
  if(cache_flag!=NULL){
    PANEL_buttons = glui_bounds->add_panel_to_panel(PANEL_minmax, "", GLUI_PANEL_NONE);
    glui_bounds->add_column_to_panel(PANEL_buttons, false);
  }
  if(PANEL_buttons==NULL){
    BUTTON_reload_data = glui_bounds->add_button_to_panel(PANEL_minmax, "Reload data", BOUND_RELOAD_DATA, Callback);
  }
  else{
    BUTTON_reload_data = glui_bounds->add_button_to_panel(PANEL_buttons, "Reload data", BOUND_RELOAD_DATA, Callback);
  }

//*** chop above/below

  if(strcmp(file_type, "hvac") != 0){
    ROLLOUT_truncate = glui_bounds->add_rollout_to_panel(PANEL_bound2, "Truncate data", false);

    PANEL_truncate_max = glui_bounds->add_panel_to_panel(ROLLOUT_truncate, "", GLUI_PANEL_NONE);
    EDIT_chopmax = glui_bounds->add_edittext_to_panel(PANEL_truncate_max, "", GLUI_EDITTEXT_FLOAT, &(bounds.chopmax), BOUND_CHOPMAX, Callback);
    glui_bounds->add_column_to_panel(PANEL_truncate_max, false);
    STATIC_chopmax_unit = glui_bounds->add_statictext_to_panel(PANEL_truncate_max, "");
    STATIC_chopmax_unit->set_w(10);
    glui_bounds->add_column_to_panel(PANEL_truncate_max, false);
    CHECKBOX_set_chopmax = glui_bounds->add_checkbox_to_panel(PANEL_truncate_max, "Above", &(bounds.set_chopmax), BOUND_SETCHOPMAX, Callback);

    PANEL_truncate_min = glui_bounds->add_panel_to_panel(ROLLOUT_truncate, "", GLUI_PANEL_NONE);
    EDIT_chopmin = glui_bounds->add_edittext_to_panel(PANEL_truncate_min, "", GLUI_EDITTEXT_FLOAT, &(bounds.chopmin), BOUND_CHOPMIN, Callback);
    glui_bounds->add_column_to_panel(PANEL_truncate_min, false);
    STATIC_chopmin_unit = glui_bounds->add_statictext_to_panel(PANEL_truncate_min, "");
    STATIC_chopmin_unit->set_w(10);
    glui_bounds->add_column_to_panel(PANEL_truncate_min, false);
    CHECKBOX_set_chopmin = glui_bounds->add_checkbox_to_panel(PANEL_truncate_min, "Below", &(bounds.set_chopmin), BOUND_SETCHOPMIN, Callback);

    Callback(BOUND_VAL_TYPE);
    Callback(BOUND_SETCHOPMIN);
    Callback(BOUND_SETCHOPMAX);
  }
  update_ini = 1;
}

/* ------------------ PadString ------------------------ */

void PadString(char *label1, const char *label2, int length){
  int i;

  TrimBack(label1);
  for(i=strlen(label1);i<length;i++){
    strcat(label1, " ");
  }
  strcat(label1,label2);
}

/* ------------------ set_valtype ------------------------ */

int bounds_dialog::set_valtype(char *label){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      boundi->set_valtype = i;
      if(RADIO_set_valtype!=NULL)RADIO_set_valtype->set_int_val(i);
      CB(BOUND_VAL_TYPE);
      return 1;
    }
  }
  return 0;
}

/* ------------------ get_nvaltypes ------------------------ */

int bounds_dialog::get_nvaltypes(void){
  return nall_bounds;
}

/* ------------------ set_valtype_index ------------------------ */

void bounds_dialog::set_valtype_index(int index){
  int i;
  cpp_boundsdata *boundi;

  i = CLAMP(index, 0, nall_bounds-1);

  if(all_bounds != NULL){
    boundi = all_bounds + i;
    boundi->set_valtype = i;
    RADIO_set_valtype->set_int_val(i);
    CB(BOUND_VAL_TYPE);
  }
}

/* ------------------ get_valtype ------------------------ */

int bounds_dialog::get_valtype(void){
  return bounds.set_valtype;
}

/* ------------------ set_chopmin ------------------------ */

int bounds_dialog::set_chopmin(char *label, int set_valmin, float valmin){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      boundi->set_chopmin = set_valmin;
      boundi->chopmin     = valmin;
      return 1;
    }
  }
  return 0;
}

/* ------------------ get_chopmin ------------------------ */

int bounds_dialog::get_chopmin(char *label, int *set_chopmin, float *chopmin){
  int i;

  for(i = 0; i < nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds + i;
    if(strcmp(boundi->label, label) == 0){
      *set_chopmin = boundi->set_chopmin;
      *chopmin = boundi->chopmin;
      return 1;
    }
  }
  return 0;
}

/* ------------------ get_chopmax ------------------------ */

int bounds_dialog::get_chopmax(char *label, int *set_chopmax, float *chopmax){
  int i;

  for(i = 0; i < nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds + i;
    if(strcmp(boundi->label, label) == 0){
      *set_chopmax = boundi->set_chopmax;
      *chopmax     = boundi->chopmax;
      return 1;
    }
  }
  return 0;
}

/* ------------------ set_chopmax ------------------------ */

int bounds_dialog::set_chopmax(char *label, int set_valmax, float valmax){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      boundi->set_chopmax = set_valmax;
      boundi->chopmax     = valmax;
      return 1;
    }
  }
  return 0;
}

/* ------------------ get_min ------------------------ */

int bounds_dialog::get_min(char *label, int *set_valmin, float *valmin){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      *set_valmin = boundi->set_valmin;
      *valmin = boundi->valmin[boundi->set_valmin];
      return 1;
    }
  }
  *valmin = 1.0;
  return 0;
}

/* ------------------ get_min_all ------------------------ */

void bounds_dialog::get_min_all(int *set_valmin, float *valmin, int *nvals){
  int i;

  *nvals = nall_bounds;
  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    set_valmin[i] = boundi->set_valmin;
    valmin[i] = boundi->valmin[boundi->set_valmin];
  }
}

/* ------------------ get_max_all ------------------------ */

void bounds_dialog::get_max_all(int *set_valmax, float *valmax, int *nvals){
  int i;

  *nvals = nall_bounds;
  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    set_valmax[i] = boundi->set_valmax;
    valmax[i] = boundi->valmax[boundi->set_valmax];
  }
}

/* ------------------ get_global_minmax ------------------------ */

void bounds_dialog::get_global_minmax(char *label, float *valmin, float *valmax){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      *valmin = boundi->valmin[BOUND_GLOBAL_MIN];
      *valmax = boundi->valmax[BOUND_GLOBAL_MAX];
      return;
    }
  }
}

/* ------------------ set_min_all ------------------------ */

void bounds_dialog::set_min_all(int *set_valmin, float *valmin, int nvals){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    boundi->set_valmin = set_valmin[i];
    boundi->valmin[boundi->set_valmin] = valmin[i];
  }
}

/* ------------------ get_global_minmax_all ------------------------ */

void bounds_dialog::get_global_minmax_all(float *valmin, float *valmax, int nvals){
  int i, set_valmin, set_valmax;

  set_valmin = BOUND_GLOBAL_MIN;
  set_valmax = BOUND_GLOBAL_MAX;
  for(i = 0; i < nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds + i;
    valmin[i] = boundi->valmin[set_valmin];
    valmax[i] = boundi->valmax[set_valmax];
  }
}

/* ------------------ get_loaded_minmax_all ------------------------ */

void bounds_dialog::get_loaded_minmax_all(float *valmin, float *valmax, int nvals){
  int i, set_valmin, set_valmax;

  set_valmin = BOUND_LOADED_MIN;
  set_valmax = BOUND_LOADED_MAX;
  for(i = 0; i < nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds + i;
    valmin[i] = boundi->valmin[set_valmin];
    valmax[i] = boundi->valmax[set_valmax];
  }
}

/* ------------------ set_global_minmax_all ------------------------ */

void bounds_dialog::set_global_minmax_all(float *valmin, float *valmax, int nvals){
  int i, set_valmin, set_valmax;

  set_valmin = BOUND_GLOBAL_MIN;
  set_valmax = BOUND_GLOBAL_MAX;
  for(i = 0; i < nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds + i;
    boundi->valmin[set_valmin] = valmin[i];
    boundi->valmax[set_valmax] = valmax[i];
  }
}

/* ------------------ set_loaded_minmax_all ------------------------ */

void bounds_dialog::set_loaded_minmax_all(float *valmin, float *valmax, int nvals){
  int i, set_valmin, set_valmax;

  set_valmin = BOUND_LOADED_MIN;
  set_valmax = BOUND_LOADED_MAX;
  for(i = 0; i < nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds + i;
    boundi->valmin[set_valmin] = valmin[i];
    boundi->valmax[set_valmax] = valmax[i];
  }
}

/* ------------------ set_max_all ------------------------ */

void bounds_dialog::set_max_all(int *set_valmax, float *valmax, int nvals){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    boundi->set_valmax = set_valmax[i];
    boundi->valmax[boundi->set_valmax] = valmax[i];
  }
}

/* ------------------ set_min ------------------------ */

int bounds_dialog::set_min(char *label, int set_valmin, float valmin){
  int i;

  if(strcmp(label, "")==0){
    for(i = 0; i<nall_bounds; i++){
      cpp_boundsdata *boundi;

      boundi = all_bounds+i;
      boundi->set_valmin = set_valmin;
      if(RADIO_set_valmin!=NULL&&RADIO_set_valmin!=NULL)RADIO_set_valmin->set_int_val(set_valmin);
    }
    return 1;
  }
  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      boundi->set_valmin         = set_valmin;
      boundi->valmin[set_valmin] = valmin;
      if(EDIT_valmin!=NULL)EDIT_valmin->set_float_val(valmin);
      if(RADIO_set_valmin!=NULL)RADIO_set_valmin->set_int_val(set_valmin);
      return 1;
    }
  }
  return 0;
}

/* ------------------ get_max ------------------------ */

int bounds_dialog::get_max(char *label, int *set_valmax, float *valmax){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      *set_valmax = boundi->set_valmax;
      *valmax = boundi->valmax[boundi->set_valmax];
      return 1;
    }
  }
  *valmax = 0.0;
  return 0;
}

/* ------------------ set_max ------------------------ */

int bounds_dialog::set_max(char *label, int set_valmax, float valmax){
  int i;

  if(strcmp(label, "")==0){
    for(i = 0; i<nall_bounds; i++){
      cpp_boundsdata *boundi;

      boundi = all_bounds+i;
      boundi->set_valmax = set_valmax;
      if(RADIO_set_valmax!=NULL&&RADIO_set_valmax!=NULL)RADIO_set_valmax->set_int_val(set_valmax);
    }
    return 1;
  }
  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(strcmp(boundi->label, label)==0){
      boundi->set_valmax         = set_valmax;
      boundi->valmax[set_valmax] = valmax;
      if(EDIT_valmax!=NULL)EDIT_valmax->set_float_val(valmax);
      if(RADIO_set_valmax!=NULL)RADIO_set_valmax->set_int_val(set_valmax);
      return 1;
    }
  }
  return 0;
}

/* ------------------ CB ------------------------ */

void bounds_dialog::CB(int var){
  cpp_boundsdata *all_boundsi;
  int valtype_save;

  all_boundsi = all_bounds + bounds.set_valtype;
  switch(var){

    // quantity radio button
    case BOUND_VAL_TYPE:
      if(all_bounds == NULL)break;
      valtype_save = bounds.set_valtype;
      memcpy(&bounds, all_boundsi, sizeof(cpp_boundsdata));
      bounds.set_valtype = valtype_save;

      bounds.glui_valmin = bounds.valmin[bounds.set_valmin];
      if(EDIT_valmin!=NULL)EDIT_valmin->set_float_val(bounds.glui_valmin);
      if(STATIC_min_unit!=NULL)STATIC_min_unit->set_name(bounds.unit);
      if(RADIO_set_valmin!=NULL)RADIO_set_valmin->set_int_val(bounds.set_valmin);

      bounds.glui_valmax = bounds.valmax[bounds.set_valmax];
      if(EDIT_valmax!=NULL)EDIT_valmax->set_float_val(bounds.glui_valmax);
      if(STATIC_max_unit!=NULL)STATIC_max_unit->set_name(bounds.unit);
      if(RADIO_set_valmax!=NULL)RADIO_set_valmax->set_int_val(bounds.set_valmax);

      if(EDIT_chopmin!=NULL)EDIT_chopmin->set_float_val(bounds.chopmin);
      if(CHECKBOX_set_chopmin!=NULL)CHECKBOX_set_chopmin->set_int_val(bounds.set_chopmin);
      if(STATIC_chopmin_unit!=NULL)STATIC_chopmin_unit->set_name(bounds.unit);

      if(EDIT_chopmax!=NULL)EDIT_chopmax->set_float_val(bounds.chopmax);
      if(CHECKBOX_set_chopmax!=NULL)CHECKBOX_set_chopmax->set_int_val(bounds.set_chopmax);
      if(STATIC_chopmax_unit!=NULL)STATIC_chopmax_unit->set_name(bounds.unit);

      CB(BOUND_SETCHOPMIN);
      CB(BOUND_SETCHOPMAX);

      if(CHECKBOX_cache!=NULL)CHECKBOX_cache->set_int_val(bounds.cache);
      update_chop_colors = 1;
      break;

      // min/max edit boxes
    case BOUND_VALMIN:
      bounds.valmin[BOUND_SET_MIN] = bounds.glui_valmin;
      bounds.set_valmin            = BOUND_SET_MIN;
      if(all_bounds != NULL)memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(RADIO_set_valmin!=NULL)RADIO_set_valmin->set_int_val(BOUND_SET_MIN);
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      break;
    case BOUND_VALMAX:
      bounds.valmax[BOUND_SET_MAX] = bounds.glui_valmax;
      bounds.set_valmax = BOUND_SET_MAX;
      if(all_bounds != NULL)memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(RADIO_set_valmax!=NULL)RADIO_set_valmax->set_int_val(BOUND_SET_MAX);
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      break;

      // min/max radio buttons
    case BOUND_SETVALMIN:
      if(all_bounds != NULL)bounds.glui_valmin = all_boundsi->valmin[bounds.set_valmin];
      if(EDIT_valmin!=NULL)EDIT_valmin->set_float_val(bounds.glui_valmin);
      if(all_bounds != NULL)memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      break;
    case BOUND_SETVALMAX:
      if(all_bounds != NULL)bounds.glui_valmax = all_boundsi->valmax[bounds.set_valmax];
      if(EDIT_valmax!=NULL)EDIT_valmax->set_float_val(bounds.glui_valmax);
      if(all_bounds != NULL)memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      break;
      case SET_PERCENTILE_MIN_VAL:
        CB(BOUND_VALMIN);
        break;
      case SET_PERCENTILE_MAX_VAL:
        CB(BOUND_VALMAX);
        break;
      case SET_PERCENTILE_MIN_LEVEL:
      if(percentile_min_level>percentile_max_level){
        percentile_min_level = percentile_max_level;
        SPINNER_percentile_min_level->set_float_val(percentile_min_level);
      }
      break;
      case SET_PERCENTILE_MAX_LEVEL:
      if(percentile_max_level<percentile_min_level){
        percentile_max_level = percentile_min_level;
        SPINNER_percentile_max_level->set_float_val(percentile_max_level);
      }
      break;

      // chop dialog boxes
    case BOUND_CHOPMIN:
      if(all_bounds != NULL)all_boundsi->chopmin = bounds.chopmin;
      update_chop_colors = 1;
      break;
    case BOUND_CHOPMAX:
      if(all_bounds != NULL)all_boundsi->chopmax = bounds.chopmax;
      update_chop_colors = 1;
      break;
    case BOUND_SETCHOPMIN:
      if(all_bounds != NULL)all_boundsi->set_chopmin = bounds.set_chopmin;
      if(EDIT_chopmin!=NULL){
        if(bounds.set_chopmin==1){
          EDIT_chopmin->enable();
        }
        else{
          EDIT_chopmin->disable();
        }
      }
      update_chop_colors = 1;
      break;
    case BOUND_SETCHOPMAX:
      if(all_bounds != NULL)all_boundsi->set_chopmax = bounds.set_chopmax;
      if(EDIT_chopmax!=NULL){
        if(bounds.set_chopmax==1){
          EDIT_chopmax->enable();
        }
        else{
          EDIT_chopmax->disable();
        }
      }
      update_chop_colors = 1;
      break;

      // keep data checkbox
    case BOUND_CACHE_DATA:
      if(all_bounds != NULL){
        int i;

        for(i = 0; i<nall_bounds; i++){
          cpp_boundsdata *boundi;

          boundi = all_bounds+i;
          boundi->cache = bounds.cache;
        }
      }
      break;

      // update colors, reload data buttons - handle in calling routine
    case BOUND_DONTUPDATE_COLORS:
    case BOUND_UPDATE_COLORS:
    case BOUND_RELOAD_DATA:
      break;
    case BOUND_RESEARCH_MODE:
      if(research_mode==1){
        SaveBounds();
      }
      else{
        RestoreBounds();
        CB(BOUND_VAL_TYPE);
      }
      update_research_mode = 1;
      CB(BOUND_CACHE_DATA);
      break;
    case BOUND_PERCENTILE_MODE:
    case BOUND_COMPUTE_PERCENTILES:
      break;
    case BOUND_COLORBAR_DIGITS:
      update_colorbar_digits = 1;
      updatemenu = 1;
      break;
    case BOUND_PLOT_MINMAX:
      gmin_draw = plot_min_cpp;
      gmax_draw = plot_max_cpp;
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_PERCENTILE_DRAW:
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      break;
    default:
      assert(FFALSE);
      break;
  }
};

bounds_dialog patchboundsCPP, partboundsCPP, plot3dboundsCPP, sliceboundsCPP;
bounds_dialog hvacductboundsCPP, hvacnodeboundsCPP;

/* ------------------ GLUISliceBoundsSetupNoGraphics ------------------------ */

extern "C" void GLUISliceBoundsSetupNoGraphics(void){
  sliceboundsCPP.setupNoGraphics("slice", slicebounds_cpp, nslicebounds_cpp);
}

/* ------------------ SetResearchMode ------------------------ */

void SetResearchMode(int flag){
  if(global_scase.npatchinfo>0)patchboundsCPP.set_research_mode(flag);
  if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.set_research_mode(flag);
  if(global_scase.npartinfo>0)partboundsCPP.set_research_mode(flag);
  if(global_scase.nplot3dinfo>0)plot3dboundsCPP.set_research_mode(flag);
  if(nhvacductbounds>0)hvacductboundsCPP.set_research_mode(flag);
  if(nhvacnodebounds > 0)hvacnodeboundsCPP.set_research_mode(flag);
}

/* ------------------ GLUISetColorbarDigitsCPP ------------------------ */

extern "C" void GLUISetColorbarDigitsCPP(int ndigits){
  if(global_scase.npatchinfo>0)patchboundsCPP.set_colorbar_digits(ndigits);
  if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.set_colorbar_digits(ndigits);
  if(global_scase.npartinfo>0)partboundsCPP.set_colorbar_digits(ndigits);
  if(global_scase.nplot3dinfo>0)plot3dboundsCPP.set_colorbar_digits(ndigits);
}

  /* ------------------ InResearchMode ------------------------ */

int InResearchMode(void){
  if(global_scase.npatchinfo>0&&patchboundsCPP.in_research_mode()==0)return 0;
  if(global_scase.npartinfo>0&&partboundsCPP.in_research_mode()==0)return 0;
  if(global_scase.nplot3dinfo>0&&plot3dboundsCPP.in_research_mode()==0)return 0;
  if(global_scase.slicecoll.nsliceinfo>0&&sliceboundsCPP.in_research_mode()==0)return 0;
  if(nhvacductbounds>0&&hvacductboundsCPP.in_research_mode()==0)return 0;
  if(nhvacnodebounds > 0 && hvacnodeboundsCPP.in_research_mode() == 0)return 0;
  return 1;
}

/* ------------------ GLUIGetBoundsData ------------------------ */

extern "C" cpp_boundsdata *GLUIGetBoundsData(int type){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)return hvacductboundsCPP.get_bounds_data();
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)return hvacnodeboundsCPP.get_bounds_data();
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)return patchboundsCPP.get_bounds_data();
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)return partboundsCPP.get_bounds_data();
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)return plot3dboundsCPP.get_bounds_data();
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)return sliceboundsCPP.get_bounds_data();
      break;
    default:
      assert(FFALSE);
      break;
  }
  return NULL;
}

/* ------------------ GLUIGetGlobalBoundsMinMax ------------------------ */

extern "C" void GLUIGetGlobalBoundsMinMax(int type, char *label, float *valmin, float *valmax){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)hvacductboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)patchboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)partboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)plot3dboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUISetCacheFlag ------------------------ */

extern "C" void GLUISetCacheFlag(int type, int cache_flag){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)hvacductboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)patchboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)partboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)plot3dboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.set_cache_flag(cache_flag);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GetCacheFlag ------------------------ */

int GetCacheFlag(int type){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)return hvacductboundsCPP.get_cache_flag();
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)return hvacnodeboundsCPP.get_cache_flag();
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)return patchboundsCPP.get_cache_flag();
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)return partboundsCPP.get_cache_flag();
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)return plot3dboundsCPP.get_cache_flag();
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)return sliceboundsCPP.get_cache_flag();
      break;
    default:
      assert(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GLUIUpdateBounds ------------------------ */

extern "C" void GLUIUpdateBounds(void){
  if(nhvacductbounds > 0){
    hvacductboundsCPP.CB(BOUND_VAL_TYPE);
  }
  if(nhvacnodebounds > 0){
    hvacnodeboundsCPP.CB(BOUND_VAL_TYPE);
  }
  if(global_scase.npatchinfo>0){
    patchboundsCPP.CB(BOUND_VAL_TYPE);
    patchboundsCPP.CB(BOUND_SETCHOPMIN);
    patchboundsCPP.CB(BOUND_SETCHOPMAX);
  }

  if(global_scase.npartinfo>0){
    partboundsCPP.CB(BOUND_VAL_TYPE);
    partboundsCPP.CB(BOUND_SETCHOPMIN);
    partboundsCPP.CB(BOUND_SETCHOPMAX);
  }

  if(global_scase.nplot3dinfo>0){
    plot3dboundsCPP.CB(BOUND_VAL_TYPE);
    plot3dboundsCPP.CB(BOUND_SETCHOPMIN);
    plot3dboundsCPP.CB(BOUND_SETCHOPMAX);
  }

  if(global_scase.slicecoll.nsliceinfo>0){
    sliceboundsCPP.CB(BOUND_VAL_TYPE);
    sliceboundsCPP.CB(BOUND_SETCHOPMIN);
    sliceboundsCPP.CB(BOUND_SETCHOPMAX);
  }
}

/* ------------------ GetValtype ------------------------ */

int GetValType(int type){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)return hvacductboundsCPP.get_valtype();
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)return hvacnodeboundsCPP.get_valtype();
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)return patchboundsCPP.get_valtype();
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)return partboundsCPP.get_valtype();
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)return plot3dboundsCPP.get_valtype();
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)return sliceboundsCPP.get_valtype();
      break;
    default:
      assert(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GLUIGetGetChopMin ------------------------ */

extern "C" int GLUIGetChopMin(int type, char *label, int *set_chopmin, float *chopmin){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)return hvacductboundsCPP.get_chopmin(label, set_chopmin, chopmin);
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)return hvacnodeboundsCPP.get_chopmin(label, set_chopmin, chopmin);
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)return patchboundsCPP.get_chopmin(label, set_chopmin, chopmin);
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)return partboundsCPP.get_chopmin(label, set_chopmin, chopmin);
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)return plot3dboundsCPP.get_chopmin(label, set_chopmin, chopmin);
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)return sliceboundsCPP.get_chopmin(label, set_chopmin, chopmin);
      break;
    default:
      assert(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GLUIGetGetChopMax ------------------------ */

extern "C" int GLUIGetChopMax(int type, char *label, int *set_chopmax, float *chopmax){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)return hvacductboundsCPP.get_chopmax(label, set_chopmax, chopmax);
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)return hvacnodeboundsCPP.get_chopmax(label, set_chopmax, chopmax);
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)return patchboundsCPP.get_chopmax(label, set_chopmax, chopmax);
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)return partboundsCPP.get_chopmax(label, set_chopmax, chopmax);
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)return plot3dboundsCPP.get_chopmax(label, set_chopmax, chopmax);
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)return sliceboundsCPP.get_chopmax(label, set_chopmax, chopmax);
      break;
    default:
      assert(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GLUISetChopMin ------------------------ */

extern "C" int GLUISetChopMin(int type, char *label, int set_chopmin, float chopmin){
  switch(type){
  case BOUND_HVACDUCT:
    if(nhvacductbounds > 0)return hvacductboundsCPP.set_chopmin(label, set_chopmin, chopmin);
    break;
  case BOUND_HVACNODE:
    if(nhvacnodebounds > 0)return hvacnodeboundsCPP.set_chopmin(label, set_chopmin, chopmin);
    break;
  case BOUND_PATCH:
    if(global_scase.npatchinfo > 0)return patchboundsCPP.set_chopmin(label, set_chopmin, chopmin);
    break;
  case BOUND_PART:
    if(global_scase.npartinfo > 0)return partboundsCPP.set_chopmin(label, set_chopmin, chopmin);
    break;
  case BOUND_PLOT3D:
    if(global_scase.nplot3dinfo > 0)return plot3dboundsCPP.set_chopmin(label, set_chopmin, chopmin);
    break;
  case BOUND_SLICE:
    if(global_scase.slicecoll.nsliceinfo > 0)return sliceboundsCPP.set_chopmin(label, set_chopmin, chopmin);
    break;
  default:
    assert(FFALSE);
    break;
  }
  return 0;
}

/* ------------------ GLUISetChopMax ------------------------ */

extern "C" int GLUISetChopMax(int type, char *label, int set_chopmax, float chopmax){
  switch(type){
  case BOUND_HVACDUCT:
    if(nhvacductbounds > 0)return hvacductboundsCPP.set_chopmax(label, set_chopmax, chopmax);
    break;
  case BOUND_HVACNODE:
    if(nhvacnodebounds > 0)return hvacnodeboundsCPP.set_chopmax(label, set_chopmax, chopmax);
    break;
  case BOUND_PATCH:
    if(global_scase.npatchinfo > 0)return patchboundsCPP.set_chopmax(label, set_chopmax, chopmax);
    break;
  case BOUND_PART:
    if(global_scase.npartinfo > 0)return partboundsCPP.set_chopmax(label, set_chopmax, chopmax);
    break;
  case BOUND_PLOT3D:
    if(global_scase.nplot3dinfo > 0)return plot3dboundsCPP.set_chopmax(label, set_chopmax, chopmax);
    break;
  case BOUND_SLICE:
    if(global_scase.slicecoll.nsliceinfo > 0)return sliceboundsCPP.set_chopmax(label, set_chopmax, chopmax);
    break;
  default:
    assert(FFALSE);
    break;
  }
  return 0;
}

/* ------------------ GLUIGetNValtypes ------------------------ */

extern "C" int GLUIGetNValtypes(int type){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0)return hvacductboundsCPP.get_nvaltypes();
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)return hvacnodeboundsCPP.get_nvaltypes();
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)return patchboundsCPP.get_nvaltypes();
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)return partboundsCPP.get_nvaltypes();
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)return plot3dboundsCPP.get_nvaltypes();
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)return sliceboundsCPP.get_nvaltypes();
      break;
    default:
      assert(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GLUISetValTypeIndex ------------------------ */

extern "C" void GLUISetValTypeIndex(int type, int valtype_index){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds > 0)hvacductboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0)patchboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0)partboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0)plot3dboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.set_valtype_index(valtype_index);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIGetOnlyMinMax ------------------------ */

extern "C" void GLUIGetOnlyMinMax(int type, char *label, int *set_valmin, float *valmin, int *set_valmax, float *valmax){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0){
        hvacductboundsCPP.get_min(label, set_valmin, valmin);
        hvacductboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0){
        hvacnodeboundsCPP.get_min(label, set_valmin, valmin);
        hvacnodeboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0){
        patchboundsCPP.get_min(label, set_valmin, valmin);
        patchboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0){
        partboundsCPP.get_min(label, set_valmin, valmin);
        partboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0){
        plot3dboundsCPP.get_min(label, set_valmin, valmin);
        plot3dboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0){
        sliceboundsCPP.get_min(label, set_valmin, valmin);
        sliceboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIGetMinMax ------------------------ */

extern "C" void GLUIGetMinMax(int type, char *label, int *set_valmin, float *valmin, int *set_valmax, float *valmax){
  switch(type){
    case BOUND_HVACDUCT:
      if(nhvacductbounds>0){
        hvacductboundsCPP.set_valtype(label);
        hvacductboundsCPP.get_min(label, set_valmin, valmin);
        hvacductboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_HVACNODE:
      if(nhvacnodebounds > 0){
        hvacnodeboundsCPP.set_valtype(label);
        hvacnodeboundsCPP.get_min(label, set_valmin, valmin);
        hvacnodeboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PATCH:
      if(global_scase.npatchinfo>0){
        patchboundsCPP.set_valtype(label);
        patchboundsCPP.get_min(label, set_valmin, valmin);
        patchboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PART:
      if(global_scase.npartinfo>0){
        partboundsCPP.set_valtype(label);
        partboundsCPP.get_min(label, set_valmin, valmin);
        partboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PLOT3D:
      if(global_scase.nplot3dinfo>0){
        plot3dboundsCPP.set_valtype(label);
        plot3dboundsCPP.get_min(label, set_valmin, valmin);
        plot3dboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_SLICE:
      if(global_scase.slicecoll.nsliceinfo>0){
        sliceboundsCPP.set_valtype(label);
        sliceboundsCPP.get_min(label, set_valmin, valmin);
        sliceboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIGetMinMaxAll ------------------------ */

extern "C" void GLUIGetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int *nall){
  switch(type){
    case BOUND_HVACDUCT:
      hvacductboundsCPP.get_min_all(set_valmin, valmin, nall);
      hvacductboundsCPP.get_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_HVACNODE:
      hvacnodeboundsCPP.get_min_all(set_valmin, valmin, nall);
      hvacnodeboundsCPP.get_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_PATCH:
      patchboundsCPP.get_min_all(set_valmin, valmin, nall);
      patchboundsCPP.get_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_PART:
      partboundsCPP.get_min_all(set_valmin, valmin, nall);
      partboundsCPP.get_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.get_min_all(set_valmin, valmin, nall);
      plot3dboundsCPP.get_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.get_min_all(set_valmin, valmin, nall);
      sliceboundsCPP.get_max_all(set_valmax, valmax, nall);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUISetMinMax ------------------------ */

extern "C" void GLUISetMinMax(int type, char *label, int set_valmin, float valmin, int set_valmax, float valmax){
  switch(type){
    case BOUND_HVACDUCT:
      hvacductboundsCPP.set_min(label, set_valmin, valmin);
      hvacductboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_HVACNODE:
      hvacnodeboundsCPP.set_min(label, set_valmin, valmin);
      hvacnodeboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_PATCH:
      patchboundsCPP.set_min(label, set_valmin, valmin);
      patchboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_PART:
      partboundsCPP.set_min(label, set_valmin, valmin);
      partboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.set_min(label, set_valmin, valmin);
      plot3dboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.set_min(label, set_valmin, valmin);
      sliceboundsCPP.set_max(label, set_valmax, valmax);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUISetMin ------------------------ */

extern "C" void GLUISetMin(int type, char *label, int set_valmin, float valmin){
  switch(type){
    case BOUND_HVACDUCT:
      hvacductboundsCPP.set_min(label, set_valmin, valmin);
      break;
    case BOUND_HVACNODE:
      hvacnodeboundsCPP.set_min(label, set_valmin, valmin);
      break;
    case BOUND_PATCH:
      patchboundsCPP.set_min(label, set_valmin, valmin);
      break;
    case BOUND_PART:
      partboundsCPP.set_min(label, set_valmin, valmin);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.set_min(label, set_valmin, valmin);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.set_min(label, set_valmin, valmin);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUISetMax ------------------------ */

extern "C" void GLUISetMax(int type, char *label, int set_valmax, float valmax){
  switch(type){
    case BOUND_HVACDUCT:
      hvacductboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_HVACNODE:
      hvacnodeboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_PATCH:
      patchboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_PART:
      partboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.set_max(label, set_valmax, valmax);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.set_max(label, set_valmax, valmax);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUISetMinMaxAll ------------------------ */

extern "C" void GLUISetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int nall){
  switch(type){
    case BOUND_HVACDUCT:
      hvacductboundsCPP.set_min_all(set_valmin, valmin, nall);
      hvacductboundsCPP.set_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_HVACNODE:
      hvacnodeboundsCPP.set_min_all(set_valmin, valmin, nall);
      hvacnodeboundsCPP.set_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_PATCH:
      patchboundsCPP.set_min_all(set_valmin, valmin, nall);
      patchboundsCPP.set_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_PART:
      partboundsCPP.set_min_all(set_valmin, valmin, nall);
      partboundsCPP.set_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.set_min_all(set_valmin, valmin, nall);
      plot3dboundsCPP.set_max_all(set_valmax, valmax, nall);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.set_min_all(set_valmin, valmin, nall);
      sliceboundsCPP.set_max_all(set_valmax, valmax, nall);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIGetGlobalMinMaxAll ------------------------ */

extern "C" void GLUIGetGlobalMinMaxAll(int type, float *valmin, float *valmax, int nall){
  switch(type){
  case BOUND_HVACDUCT:
    hvacductboundsCPP.get_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_HVACNODE:
    hvacnodeboundsCPP.get_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PATCH:
    patchboundsCPP.get_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PART:
    partboundsCPP.get_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PLOT3D:
    plot3dboundsCPP.get_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_SLICE:
    sliceboundsCPP.get_global_minmax_all(valmin, valmax, nall);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIGetLoadedMinMaxAll ------------------------ */

extern "C" void GLUIGetLoadedMinMaxAll(int type, float *valmin, float *valmax, int nall){
  switch(type){
  case BOUND_HVACDUCT:
    hvacductboundsCPP.get_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_HVACNODE:
    hvacnodeboundsCPP.get_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PATCH:
    patchboundsCPP.get_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PART:
    partboundsCPP.get_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PLOT3D:
    plot3dboundsCPP.get_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_SLICE:
    sliceboundsCPP.get_loaded_minmax_all(valmin, valmax, nall);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUISetGlobalMinMaxAll ------------------------ */

extern "C" void GLUISetGlobalMinMaxAll(int type, float *valmin, float *valmax, int nall){
  switch(type){
  case BOUND_HVACDUCT:
    hvacductboundsCPP.set_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_HVACNODE:
    hvacnodeboundsCPP.set_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PATCH:
    patchboundsCPP.set_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PART:
    partboundsCPP.set_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PLOT3D:
    plot3dboundsCPP.set_global_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_SLICE:
    sliceboundsCPP.set_global_minmax_all(valmin, valmax, nall);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUISetLoadedMinMaxAll ------------------------ */

extern "C" void GLUISetLoadedMinMaxAll(int type, float *valmin, float *valmax, int nall){
  switch(type){
  case BOUND_HVACDUCT:
    hvacductboundsCPP.set_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_HVACNODE:
    hvacnodeboundsCPP.set_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PATCH:
    patchboundsCPP.set_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PART:
    partboundsCPP.set_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_PLOT3D:
    plot3dboundsCPP.set_loaded_minmax_all(valmin, valmax, nall);
    break;
  case BOUND_SLICE:
    sliceboundsCPP.set_loaded_minmax_all(valmin, valmax, nall);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ HVAC callback: GLUIHVACDuctBoundsCPP_CB ------------------------ */

extern "C" void GLUIHVACDuctBoundsCPP_CB(int var){
  if(nhvacductbounds == 0)return;
  hvacductboundsCPP.CB(var);
  switch(var){
  case BOUND_RELOAD_DATA:
    ReadHVACData(LOAD);
    break;
  case BOUND_RESEARCH_MODE:
    if(global_scase.npartinfo > 0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
    if(global_scase.npatchinfo > 0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
    if(global_scase.nplot3dinfo > 0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
    if(global_scase.slicecoll.nsliceinfo > 0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
    GLUIHVACDuctBoundsCPP_CB(BOUND_UPDATE_COLORS);
    break;
  case BOUND_VALMAX:
  case BOUND_VALMIN:
  case BOUND_SETVALMIN:
  case BOUND_SETVALMAX:
  case BOUND_UPDATE_COLORS:
    int valtype;

    hvacductboundsCPP.CB(var);
    valtype = GetValType(BOUND_HVACDUCT);
    UpdateHVACDuctColorLabels(valtype);
    break;
  default:
    break;
  }
}

/* ------------------ HVAC callback: GLUIHVACNodeBoundsCPP_CB ------------------------ */

extern "C" void GLUIHVACNodeBoundsCPP_CB(int var){
  if(nhvacnodebounds == 0)return;
  hvacnodeboundsCPP.CB(var);
  switch(var){
  case BOUND_RELOAD_DATA:
    ReadHVACData(LOAD);
    break;
  case BOUND_RESEARCH_MODE:
    if(global_scase.npartinfo > 0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
    if(global_scase.npatchinfo > 0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
    if(global_scase.nplot3dinfo > 0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
    if(global_scase.slicecoll.nsliceinfo > 0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
    GLUIHVACNodeBoundsCPP_CB(BOUND_UPDATE_COLORS);
    break;
  case BOUND_VALMAX:
  case BOUND_VALMIN:
  case BOUND_SETVALMIN:
  case BOUND_SETVALMAX:
  case BOUND_UPDATE_COLORS:
    int valtype;

    hvacnodeboundsCPP.CB(var);
    valtype = GetValType(BOUND_HVACNODE);
    UpdateHVACNodeColorLabels(valtype);
    break;
  default:
    break;
  }
}

/* ------------------ GLUIUpdateHVACDuctType ------------------------ */

extern "C" void GLUIUpdateHVACDuctType(void){
  GLUIHVACDuctBoundsCPP_CB(BOUND_VAL_TYPE);
}

/* ------------------ slice callback: GLUIHVACSliceBoundsCPP_CB ------------------------ */

extern "C" void GLUIHVACSliceBoundsCPP_CB(int var){
  int ii, last_slice;
  cpp_boundsdata *bounds;

  sliceboundsCPP.CB(var);
  switch(var){
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
      SetLoadedSliceBounds(NULL, 0);
      UpdateSliceBounds2();
      break;
    case BOUND_VAL_TYPE:
    case BOUND_CHOPMIN:
    case BOUND_CHOPMAX:
    case BOUND_SETCHOPMIN:
    case BOUND_SETCHOPMAX:
    case BOUND_COLORBAR_DIGITS:
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
    case BOUND_PERCENTILE_DRAW:
      break;
    case BOUND_CACHE_DATA:
      cache_slice_data = GetCacheFlag(BOUND_SLICE);
      break;
    case BOUND_UPDATE_COLORS:
      if(nslice_loaded==0)break;
      SetLoadedSliceBounds(NULL, 0);

      last_slice = slice_loaded_list[nslice_loaded - 1];
      for(ii = nslice_loaded - 1; ii >= 0; ii--){
        int i;
        slicedata *sd;

        i = slice_loaded_list[ii];
        sd = global_scase.slicecoll.sliceinfo + i;
        if(sd->vloaded==0&&sd->display==0)continue;
        if(sd->slicefile_labelindex == slicefile_labelindex){
          last_slice = i;
          break;
        }
      }
      INIT_PRINT_TIMER(timer_update_slice_colors);
      UpdateSliceColors(last_slice);
      PRINT_TIMER(timer_update_slice_colors, "updateslicecolors");
      break;
    case BOUND_RELOAD_DATA:
      SetLoadedSliceBounds(NULL, 0);
      THREADcontrol(compress_threads, THREAD_LOCK);
      SetLoadedSliceBounds(NULL, 0);
      ReloadAllVectorSliceFiles(LOAD);
      ReloadAllSliceFiles(LOAD);
      THREADcontrol(compress_threads, THREAD_UNLOCK);
      GLUIHVACSliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
      break;
    case BOUND_RESEARCH_MODE:
      if(global_scase.npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacductbounds>0)hvacductboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      break;
    case SET_PERCENTILE_MIN_LEVEL:
    case SET_PERCENTILE_MAX_LEVEL:
      break;
    case SET_PERCENTILE_MIN_VAL:
    case SET_PERCENTILE_MAX_VAL:
      float valmin, valmax;
      int i, hist_update, slice_loaded;

      hist_update = 0;
      slice_loaded = 0;
      bounds = GLUIGetBoundsData(BOUND_SLICE);
      for(i = 0;i < global_scase.slicecoll.nsliceinfo;i++){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + i;
        if(slicei->loaded == 0)continue;
        slice_loaded = 1;
        if(slicei->hist_update == 1)hist_update = 1;
        slicei->hist_update = 0;
      }
      if(hist_update == 1||bounds->hist==NULL){
        float global_min, global_max;

        GLUIGetGlobalBoundsMinMax(BOUND_SLICE, bounds->label, &global_min, &global_max);
        ComputeLoadedSliceHist(bounds->label, global_min, global_max);
        MergeLoadedSliceHist(bounds->label, &(bounds->hist));
      }
      if(slice_loaded == 1){
        if(var == SET_PERCENTILE_MIN_VAL || var == SET_PERCENTILE_MIN_LEVEL){
          GetHistogramValProc(bounds->hist, sliceboundsCPP.percentile_min_level / 100.0, &valmin);
          GLUISetMin(BOUND_SLICE, bounds->label, 0, valmin);
          GLUIHVACSliceBoundsCPP_CB(BOUND_VALMIN);
        }
        if(var == SET_PERCENTILE_MAX_VAL || var == SET_PERCENTILE_MAX_LEVEL){
          GetHistogramValProc(bounds->hist, sliceboundsCPP.percentile_max_level / 100.0, &valmax);
          GLUISetMax(BOUND_SLICE, bounds->label, 0, valmax);
          GLUIHVACSliceBoundsCPP_CB(BOUND_VALMAX);
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ HavePlot3DData ------------------------ */

int HavePlot3DData(void){
  int i;

  for(i = 0; i<global_scase.nplot3dinfo; i++){
    meshdata *meshi;
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    if(plot3di->blocknumber<0)return 0;
    meshi = global_scase.meshescoll.meshinfo+plot3di->blocknumber;
    if(meshi->qdata==NULL)return 0;
  }
  return 1;
}

/* ------------------ plot3d callback: GLUIPlot3DBoundsCPP_CB ------------------------ */

extern "C" void GLUIPlot3DBoundsCPP_CB(int var){
  cpp_boundsdata *bounds;

  plot3dboundsCPP.CB(var);
  switch(var){
    case BOUND_VAL_TYPE:
      plotn = GetValType(BOUND_PLOT3D)+1;
      UpdateAllPlotSlices();
      GLUIPlot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      break;
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
      UpdateAllPlot3DColors(0);
      break;
    case BOUND_CHOPMIN:
    case BOUND_CHOPMAX:
    case BOUND_SETCHOPMIN:
    case BOUND_SETCHOPMAX:
      break;
    case BOUND_CACHE_DATA:
      cache_plot3d_data = GetCacheFlag(BOUND_PLOT3D);
      break;
    case BOUND_UPDATE_COLORS:
      if(HavePlot3DData()==1){
        UpdateAllPlot3DColors(1);
      }
      else{
        GLUIPlot3DBoundsCPP_CB(BOUND_RELOAD_DATA);
      }
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
      break;
    case BOUND_PERCENTILE_DRAW:
      break;
    case BOUND_RELOAD_DATA:
      LoadPlot3dMenu(RELOAD_ALL);
      break;
    case BOUND_RESEARCH_MODE:
      if(global_scase.npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacductbounds>0)hvacductboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      break;
    case SET_PERCENTILE_MIN_LEVEL:
    case SET_PERCENTILE_MAX_LEVEL:
      break;
    case SET_PERCENTILE_MIN_VAL:
    case SET_PERCENTILE_MAX_VAL:
      float valmin, valmax;
      int hist_update, i, plot3d_loaded;

      hist_update = 0;
      plot3d_loaded = 0;
      bounds = GLUIGetBoundsData(BOUND_PLOT3D);
      for(i = 0;i < global_scase.nplot3dinfo;i++){
        plot3ddata *plot3di;

        plot3di = global_scase.plot3dinfo + i;
        if(plot3di->loaded == 0)continue;
        plot3d_loaded = 1;
        if(plot3di->hist_update == 1)hist_update = 1;
        plot3di->hist_update = 0;
      }
      if(hist_update == 1){
        for(i = 0;i < global_scase.nplot3dinfo;i++){
          plot3ddata *plot3di;

          plot3di = global_scase.plot3dinfo + i;
          if(plot3di->loaded == 0)continue;
          GetPlot3DHists(plot3di);
        }
        MergePlot3DHistograms();
      }
      if(plot3d_loaded == 1){
        if(var == SET_PERCENTILE_MIN_VAL || var == SET_PERCENTILE_MIN_LEVEL){
          GetHistogramValProc(full_plot3D_histograms + plotn - 1, plot3dboundsCPP.percentile_min_level / 100.0, &valmin);
          GLUISetMin(BOUND_PLOT3D, bounds->label, 0, valmin);
          GLUIPlot3DBoundsCPP_CB(BOUND_VALMIN);
        }
        if(var == SET_PERCENTILE_MAX_VAL || var == SET_PERCENTILE_MAX_LEVEL){
          GetHistogramValProc(full_plot3D_histograms + plotn - 1, plot3dboundsCPP.percentile_max_level / 100.0, &valmax);
          GLUISetMax(BOUND_PLOT3D, bounds->label, 0, valmax);
          GLUIPlot3DBoundsCPP_CB(BOUND_VALMAX);
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ part callback: GLUIPartBoundsCPP_CB ------------------------ */

extern "C" void GLUIPartBoundsCPP_CB(int var){
  cpp_boundsdata *bounds;

  partboundsCPP.CB(var);
  ipart5prop = GetValType(BOUND_PART);

  switch(var){
    case BOUND_COLORBAR_DIGITS:
      break;
    case BOUND_VAL_TYPE:
      ipart5prop = GetValType(BOUND_PART);
      ParticlePropShowMenu(ipart5prop);
      GLUIPartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      break;
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
    case BOUND_VALMIN:
    case BOUND_VALMAX:
      UpdatePartColors(NULL, 0);
      break;
    case BOUND_CHOPMIN:
    case BOUND_CHOPMAX:
    case BOUND_SETCHOPMIN:
    case BOUND_SETCHOPMAX:
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
      break;
    case BOUND_PERCENTILE_DRAW:
      break;
    case BOUND_CACHE_DATA:
      if(GetCacheFlag(BOUND_PART)==0){
        ClosePartFiles();
      }
      break;
    case BOUND_UPDATE_COLORS:
      UpdatePartColors(NULL, 1);
      break;
    case BOUND_RELOAD_DATA:
      if(global_scase.npartinfo>0){
        LoadParticleMenu(PARTFILE_RELOADALL);
      }
      break;
    case BOUND_RESEARCH_MODE:
      if(global_scase.npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacductbounds>0)hvacductboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.CB(BOUND_RESEARCH_MODE);
      UpdatePartColors(NULL, 0);
      break;
    case BOUND_PERCENTILE_MODE:
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      break;
    case SET_PERCENTILE_MIN_LEVEL:
    case SET_PERCENTILE_MAX_LEVEL:
      break;
    case SET_PERCENTILE_MIN_VAL:
    case SET_PERCENTILE_MAX_VAL:
      float valmin, valmax;
      int i, hist_update, part_loaded;

      hist_update = 0;
      part_loaded = 0;
      bounds = GLUIGetBoundsData(BOUND_PART);
      for(i = 0;i < global_scase.npartinfo;i++){
        partdata *parti;

        parti = global_scase.partinfo + i;
        if(parti->loaded == 0)continue;
        part_loaded = 1;
        if(parti->hist_update == 1)hist_update = 1;
        parti->hist_update = 0;
      }
      if(hist_update == 1 || bounds->hist == NULL){
        GeneratePartHistograms();
      }
      int partprop_index;

      partprop_index = GetPartPropIndexS(bounds->label);
      if(part_loaded==1){
        if(partprop_index >= 0){
          if(var == SET_PERCENTILE_MIN_VAL || var == SET_PERCENTILE_MIN_LEVEL){
            GetHistogramValProc(full_part_histogram + partprop_index, partboundsCPP.percentile_min_level / 100.0, &valmin);
            GLUISetMin(BOUND_PART, bounds->label, 0, valmin);
            GLUIPartBoundsCPP_CB(BOUND_VALMIN);
          }
          if(var == SET_PERCENTILE_MAX_VAL || var == SET_PERCENTILE_MAX_LEVEL){
            GetHistogramValProc(full_part_histogram + partprop_index, partboundsCPP.percentile_max_level / 100.0, &valmax);
            GLUISetMax(BOUND_PART, bounds->label, 0, valmax);
            GLUIPartBoundsCPP_CB(BOUND_VALMAX);
          }
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ HavePatchData ------------------------ */

int HavePatchData(void){
  int i;

  for(i = 0; i<global_scase.npatchinfo; i++){
    patchdata *patchi;
    meshdata *meshi;

    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==0)continue;
    switch(patchi->patch_filetype){
      case PATCH_STRUCTURED_NODE_CENTER:
      case PATCH_STRUCTURED_CELL_CENTER:
        meshi = global_scase.meshescoll.meshinfo+patchi->blocknumber;
        if(meshi->patchval==NULL||meshi->cpatchval==NULL)return 0;
        break;
      case PATCH_GEOMETRY_BOUNDARY:
        if(patchi->geom_vals==NULL)return 0;
        break;
      case PATCH_GEOMETRY_SLICE:
        break;
    default:
      assert(FFALSE);
      break;
    }
  }
  return 1;
}

/* ------------------ patch callback: GLUIPatchBoundsCPP_CB ------------------------ */

extern "C" void GLUIPatchBoundsCPP_CB(int var){
  int i;
  cpp_boundsdata *bounds;

  bounds = patchboundsCPP.get_bounds_data();
  patchboundsCPP.CB(var);
  switch(var){
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
      UpdateAllBoundaryColors(0);
      break;
    case BOUND_SETCHOPMIN:
    case BOUND_SETCHOPMAX:
    case BOUND_CHOPMIN:
    case BOUND_CHOPMAX:
      updatefacelists = 1;
      global_scase.updatefaces = 1;
      if(bounds->set_chopmax == 1 || bounds->set_chopmin == 1){
        update_bound_chop_data = 1;
        hide_internal_blockages = 0;
      }
      else{
        update_bound_chop_data = 0;
      }
      break;
    case BOUND_VAL_TYPE:
    case BOUND_COLORBAR_DIGITS:
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
      break;
    case BOUND_CACHE_DATA:
      cache_boundary_data = GetCacheFlag(BOUND_PATCH);
      break;
    case BOUND_PERCENTILE_DRAW:
      break;
    case BOUND_DONTUPDATE_COLORS:
    case BOUND_UPDATE_COLORS:
      if(HavePatchData()==1){
        SetLoadedPatchBounds(NULL, 0);
        if(var==BOUND_DONTUPDATE_COLORS){
          UpdateAllBoundaryColors(0);
        }
        else{
          UpdateAllBoundaryColors(1);
        }
      }
      else{
        GLUIPatchBoundsCPP_CB(BOUND_RELOAD_DATA);
      }
      break;
    case BOUND_RELOAD_DATA:
      SetLoadedPatchBounds(NULL, 0);
      for(i = 0; i<global_scase.npatchinfo; i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        patchi->finalize = 0;
      }
      for(i = global_scase.npatchinfo-1; i>=0;  i--){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        if(patchi->loaded==0)continue;
        patchi->finalize = 1;
        break;
      }
      for(i = 0; i<global_scase.npatchinfo; i++){
        patchdata *patchi;
        int errorcode;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        ReadBoundary(i,LOAD,&errorcode);
      }
      break;
    case BOUND_RESEARCH_MODE:
      if(global_scase.npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacductbounds>0)hvacductboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nhvacnodebounds > 0)hvacnodeboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      break;
    case SET_PERCENTILE_MIN_LEVEL:
    case SET_PERCENTILE_MAX_LEVEL:
      break;
    case SET_PERCENTILE_MIN_VAL:
    case SET_PERCENTILE_MAX_VAL:
      float valmin, valmax;
      int hist_update, bound_loaded;

      hist_update = 0;
      bound_loaded = 0;
      bounds = GLUIGetBoundsData(BOUND_PATCH);
      for(i = 0;i < global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        bound_loaded = 1;
        if(patchi->hist_update == 1)hist_update = 1;
        patchi->hist_update = 0;
      }
      if(hist_update == 1||bounds->hist==NULL){
        float global_min, global_max;

        GLUIGetGlobalBoundsMinMax(BOUND_PATCH, bounds->label, &global_min, &global_max);
        ComputeLoadedPatchHist(bounds->label, &(bounds->hist), &global_min, &global_max);
      }
      if(bound_loaded == 1){
        if(var == SET_PERCENTILE_MIN_VAL || var == SET_PERCENTILE_MIN_LEVEL){
          GetHistogramValProc(bounds->hist, patchboundsCPP.percentile_min_level / 100.0, &valmin);
          GLUISetMin(BOUND_PATCH, bounds->label, 0, valmin);
          GLUIPatchBoundsCPP_CB(BOUND_VALMIN);
        }
        if(var == SET_PERCENTILE_MAX_VAL || var == SET_PERCENTILE_MAX_LEVEL){
          GetHistogramValProc(bounds->hist, patchboundsCPP.percentile_max_level / 100.0, &valmax);
          GLUISetMax(BOUND_PATCH, bounds->label, 0, valmax);
          GLUIPatchBoundsCPP_CB(BOUND_VALMAX);
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIUpdatdateResearchModeCPP ------------------------ */

extern "C" void GLUIUpdatdateResearchModeCPP(void){
  if(global_scase.npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
  if(global_scase.npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
  if(global_scase.nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
  if(global_scase.slicecoll.nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
}

/* ------------------ SetLoadedSliceBounds ------------------------ */

void SetLoadedSliceBounds(int *list, int nlist){
  int set_valmin, set_valmax;
  float valmin_dlg, valmax_dlg;
  float valmin, valmax;
  char *label=NULL;
  int i;

  if(list==NULL){
    for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
      slicedata *slicei;

      slicei = global_scase.slicecoll.sliceinfo+i;
      if(slicei->loaded==1&&slicei->display==1){
        label = slicei->label.shortlabel;
        break;
      }
    }
  }
  else{
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+list[0];
    label = slicei->label.shortlabel;
  }
  if(label==NULL)return;

  valmin = 1.0;
  valmax = 0.0;
  if(list==NULL)nlist = 0;
  for(i = 0; i<nlist; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+list[i];
    if(valmin>valmax){
      valmin = slicei->valmin_slice;
      valmax = slicei->valmax_slice;
    }
    else{
      valmin = MIN(valmin, slicei->valmin_slice);
      valmax = MAX(valmax, slicei->valmax_slice);
    }
  }
  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(slicei->loaded==0)continue;
    if(strcmp(slicei->label.shortlabel, label)!=0)continue;
    if(valmin>valmax){
      valmin = slicei->valmin_slice;
      valmax = slicei->valmax_slice;
    }
    else{
      valmin = MIN(valmin, slicei->valmin_slice);
      valmax = MAX(valmax, slicei->valmax_slice);
    }
  }
  if(valmin<=valmax){
    set_valmin = BOUND_LOADED_MIN;
    set_valmax = BOUND_LOADED_MAX;
    GLUIGetMinMax(BOUND_SLICE, label, &set_valmin, &valmin_dlg, &set_valmax, &valmax_dlg);
    valmin = valmin_dlg;
    valmax = valmax_dlg;
    GLUISetMinMax(BOUND_SLICE, label, set_valmin, valmin, set_valmax, valmax);
  }
}

/* ------------------ SetLoadedPatchBounds ------------------------ */

void SetLoadedPatchBounds(int *list, int nlist){
  int set_valmin=0, set_valmax=0;
  float valmin_dlg, valmax_dlg;
  float valmin, valmax;
  char *label=NULL;
  patchdata *patchi;
  int i;

  if(list==NULL){
    for(i = 0; i<global_scase.npatchinfo; i++){
      patchi = global_scase.patchinfo+i;
      if(patchi->loaded==1&&patchi->display==1){
        label = patchi->label.shortlabel;
        break;
      }
    }
  }
  else{
    patchi = global_scase.patchinfo+list[0];
    label = patchi->label.shortlabel;
  }
  if(label==NULL)return;

  valmin = 1.0;
  valmax = 0.0;
  if(list==NULL)nlist = 0;
  for(i = 0; i<nlist; i++){
    patchi = global_scase.patchinfo+list[i];
    if(valmin>valmax){
      valmin = patchi->valmin_patch;
      valmax = patchi->valmax_patch;
    }
    else{
      valmin = MIN(valmin, patchi->valmin_patch);
      valmax = MAX(valmax, patchi->valmax_patch);
    }
  }
  for(i = 0; i<global_scase.npatchinfo; i++){
    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==0)continue;
    if(strcmp(patchi->label.shortlabel, label)!=0)continue;
    if(valmin>valmax){
      valmin = patchi->valmin_patch;
      valmax = patchi->valmax_patch;
    }
    else{
      valmin = MIN(valmin, patchi->valmin_patch);
      valmax = MAX(valmax, patchi->valmax_patch);
    }
  }
  if(valmin<=valmax){
    GLUIGetMinMax(BOUND_PATCH, label, &set_valmin, &valmin_dlg, &set_valmax, &valmax_dlg);
    if(set_valmin!=BOUND_LOADED_MIN){
      valmin = valmin_dlg;
    }
    if(set_valmax!=BOUND_LOADED_MAX){
      valmax = valmax_dlg;
    }
    GLUISetMinMax(BOUND_PATCH, label, set_valmin, valmin, set_valmax, valmax);
  }
}

/* ------------------ SetLoadedPlot3DBounds ------------------------ */

void SetLoadedPlot3DBounds(void){
  float *valmin_dlg, *valmax_dlg, *valmin, *valmax;
  int *set_valmin, *set_valmax, nall;
  int i,j;

  NewMemoryMemID((void **)&valmin,     MAXPLOT3DVARS*sizeof(float), -1);
  NewMemoryMemID((void **)&valmax,     MAXPLOT3DVARS*sizeof(float), -1);
  NewMemoryMemID((void **)&valmin_dlg, MAXPLOT3DVARS*sizeof(float), -1);
  NewMemoryMemID((void **)&valmax_dlg, MAXPLOT3DVARS*sizeof(float), -1);
  NewMemoryMemID((void **)&set_valmin, MAXPLOT3DVARS*sizeof(int),   -1);
  NewMemoryMemID((void **)&set_valmax, MAXPLOT3DVARS*sizeof(int),   -1);

  for(j = 0; j<MAXPLOT3DVARS; j++){
    valmin[j] = 1.0;
    valmax[j] = 0.0;
    for(i = 0; i <global_scase.nplot3dinfo; i++){
      float *valmin_fds, *valmax_fds;
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo+i;
      if(plot3di->loadnow == 0)continue;
      valmin_fds = plot3di->valmin_plot3d;
      valmax_fds = plot3di->valmax_plot3d;
      if(valmin[j]>valmax[j]){
        valmin[j] = valmin_fds[j];
        valmax[j] = valmax_fds[j];
      }
      else{
        valmin[j] = MIN(valmin[j], valmin_fds[j]);
        valmax[j] = MAX(valmax[j], valmax_fds[j]);
      }
    }

    for(i = 0; i<global_scase.nplot3dinfo; i++){
      float *valmin_fds, *valmax_fds;
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo+i;
      if(plot3di->loaded==0)continue;
      valmin_fds = plot3di->valmin_plot3d;
      valmax_fds = plot3di->valmax_plot3d;
      if(valmin[j]>valmax[j]){
        valmin[j] = valmin_fds[j];
        valmax[j] = valmax_fds[j];
      }
      else{
        valmin[j] = MIN(valmin[j], valmin_fds[j]);
        valmax[j] = MAX(valmax[j], valmax_fds[j]);
      }
    }
  }

  GLUIGetMinMaxAll(BOUND_PLOT3D, set_valmin, valmin_dlg, set_valmax, valmax_dlg, &nall);
  for(i=0;i<nall;i++){
    if(set_valmin[i]!=BOUND_LOADED_MIN){
      valmin[i] = valmin_dlg[i];
    }
    if(set_valmax[i]!=BOUND_LOADED_MAX){
      valmax[i] = valmax_dlg[i];
    }
  }
  GLUISetMinMaxAll(BOUND_PLOT3D, set_valmin, valmin, set_valmax, valmax, nall);
  FREEMEMORY(valmin);
  FREEMEMORY(valmax);
  FREEMEMORY(set_valmin);
  FREEMEMORY(set_valmax);
  FREEMEMORY(valmin_dlg);
  FREEMEMORY(valmax_dlg);
}

/* ------------------ SetLoadedPartBounds ------------------------ */

void SetLoadedPartBounds(int *list, int nlist){
  float *valmin_dlg, *valmax_dlg, *valmin, *valmax;
  int *set_valmin, *set_valmax, nall;
  int i, j;
  int npart_types;


  npart_types = GLUIGetNValtypes(BOUND_PART);

  NewMemory((void **)&valmin,     npart_types*sizeof(float));
  NewMemory((void **)&valmax,     npart_types*sizeof(float));
  NewMemory((void **)&valmin_dlg, npart_types*sizeof(float));
  NewMemory((void **)&valmax_dlg, npart_types*sizeof(float));
  NewMemory((void **)&set_valmin, npart_types*sizeof(int));
  NewMemory((void **)&set_valmax, npart_types*sizeof(int));

  for(j = 0; j<npart_types; j++){
    if(j==0){ // skip over 'uniform'
      valmin[j] = 0.0;
      valmax[j] = 1.0;
      continue;
    }
    else{
      valmin[j] = 1.0;
      valmax[j] = 0.0;
    }
    if(list==NULL)nlist = 0;
    for(i = 0; i<nlist; i++){
      float *valmin_part, *valmax_part;
      partdata *parti;

      parti = global_scase.partinfo+list[i];
      if(parti->have_bound_file==NO)continue;
      valmin_part = parti->valmin_part;
      valmax_part = parti->valmax_part;
      if(valmin[j]>valmax[j]){
        valmin[j] = valmin_part[j];
        valmax[j] = valmax_part[j];
      }
      else{
        valmin[j] = MIN(valmin[j], valmin_part[j]);
        valmax[j] = MAX(valmax[j], valmax_part[j]);
      }
    }

    for(i = 0; i<global_scase.npartinfo; i++){
      float *valmin_part, *valmax_part;
      partdata *parti;

      parti = global_scase.partinfo+i;
      if(parti->loaded==0||parti->have_bound_file==NO)continue;
      valmin_part = parti->valmin_part;
      valmax_part = parti->valmax_part;
      if(valmin[j]>valmax[j]){
        valmin[j] = valmin_part[j];
        valmax[j] = valmax_part[j];
      }
      else{
        valmin[j] = MIN(valmin[j], valmin_part[j]);
        valmax[j] = MAX(valmax[j], valmax_part[j]);
      }
    }
  }

  GLUIGetMinMaxAll(BOUND_PART, set_valmin, valmin_dlg, set_valmax, valmax_dlg, &nall);
  for(i = 0; i<nall; i++){
    if(set_valmin[i]!=BOUND_LOADED_MIN){
      valmin[i] = valmin_dlg[i];
    }
    if(set_valmax[i]!=BOUND_LOADED_MAX){
      valmax[i] = valmax_dlg[i];
    }
  }
  GLUISetMinMaxAll(BOUND_PART, set_valmin, valmin, set_valmax, valmax, nall);
  FREEMEMORY(valmin);
  FREEMEMORY(valmax);
  FREEMEMORY(set_valmin);
  FREEMEMORY(set_valmax);
  FREEMEMORY(valmin_dlg);
  FREEMEMORY(valmax_dlg);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

#ifdef pp_REFRESH
extern GLUI *glui_clip, *glui_colorbar, *glui_labels, *glui_geometry, *glui_motion, *glui_device;
extern GLUI *glui_shooter, *glui_tour, *glui_stereo, *glui_trainer;
#endif

#define MEMCHECK 1

GLUI_Button *BUTTON_globalalpha = NULL;
GLUI_Button *BUTTON_updatebound = NULL;
GLUI_Button *BUTTON_compress=NULL;
GLUI_Button *BUTTON_step=NULL;
GLUI_Button *BUTTON_script_stop=NULL;
GLUI_Button *BUTTON_script_start=NULL;
GLUI_Button *BUTTON_script_saveini=NULL;
GLUI_Button *BUTTON_script_render=NULL;
GLUI_Button *BUTTON_update_line_contour=NULL;
GLUI_Button *BUTTON_ini_load=NULL;
GLUI_Button *BUTTON_script_setsuffix=NULL;
GLUI_Button *BUTTON_script_runscript=NULL;
GLUI_Button *BUTTON_SETTIME=NULL;
GLUI_Button *BUTTON_PART = NULL;
GLUI_Button *BUTTON_SLICE = NULL;
GLUI_Button *BUTTON_VSLICE = NULL;
GLUI_Button *BUTTON_PLOT3D = NULL;
GLUI_Button *BUTTON_3DSMOKE = NULL;
GLUI_Button *BUTTON_BOUNDARY = NULL;
GLUI_Button *BUTTON_ISO = NULL;
GLUI_Button *BUTTON_OUTPUT_PLOT2D=NULL;

GLUI_Listbox *LISTBOX_cb_bound = NULL;
GLUI_Listbox *LIST_colortable = NULL;
GLUI_Listbox *LIST_iso_colorbar = NULL;

GLUI_Rollout *ROLLOUT_zone_bound=NULL;
GLUI_Rollout *ROLLOUT_coloring=NULL;
GLUI_Rollout *ROLLOUT_view_options = NULL;
GLUI_Rollout *ROLLOUT_vismesh_blockages = NULL;
GLUI_Rollout *ROLLOUT_vismesh_data = NULL;
GLUI_Rollout *ROLLOUT_memcheck=NULL;
GLUI_Rollout *ROLLOUT_boundary_duplicates;
GLUI_Rollout *ROLLOUT_iso_settings;
GLUI_Rollout *ROLLOUT_iso_bounds;
GLUI_Rollout *ROLLOUT_iso_color;
GLUI_Rollout *ROLLOUT_script = NULL;
GLUI_Rollout *ROLLOUT_config = NULL;
GLUI_Rollout *ROLLOUT_compress=NULL;
GLUI_Rollout *ROLLOUT_plot3d=NULL,*ROLLOUT_part=NULL,*ROLLOUT_slice=NULL,*ROLLOUT_bound=NULL,*ROLLOUT_iso=NULL;
GLUI_Rollout *ROLLOUT_hvacduct=NULL, *ROLLOUT_hvacnode=NULL;
GLUI_Rollout *ROLLOUT_iso_colors = NULL;
GLUI_Rollout *ROLLOUT_smoke3d=NULL,*ROLLOUT_volsmoke3d=NULL;
GLUI_Rollout *ROLLOUT_time=NULL,*ROLLOUT_colorbar=NULL;
GLUI_Rollout *ROLLOUT_outputpatchdata=NULL;
GLUI_Rollout *ROLLOUT_slice_settings = NULL;
GLUI_Rollout *ROLLOUT_filebounds = NULL;
GLUI_Rollout *ROLLOUT_showhide = NULL;
GLUI_Rollout *ROLLOUT_slice_average = NULL;
GLUI_Rollout *ROLLOUT_line_contour = NULL;
GLUI_Rollout *ROLLOUT_slicedups = NULL;
GLUI_Rollout *ROLLOUT_slice_2d_plots = NULL;
GLUI_Rollout *ROLLOUT_vector = NULL;
GLUI_Rollout *ROLLOUT_isosurface = NULL;
GLUI_Rollout *ROLLOUT_boundary_settings = NULL;
GLUI_Rollout *ROLLOUT_particle_settings=NULL;
GLUI_Rollout *ROLLOUT_particle_histogram = NULL;
GLUI_Rollout *ROLLOUT_files = NULL;
GLUI_Rollout *ROLLOUT_split=NULL;
GLUI_Rollout *ROLLOUT_autoload = NULL;
GLUI_Rollout *ROLLOUT_time1a = NULL;
GLUI_Rollout *ROLLOUT_box_specify = NULL;
GLUI_Rollout *ROLLOUT_time2=NULL;

GLUI_Panel *PANEL_keep_bound_data = NULL;
GLUI_Panel *PANEL_keep_plot3d_data = NULL;
GLUI_Panel *PANEL_vector1=NULL, *PANEL_vector2=NULL;
GLUI_Panel *PANEL_partread = NULL;
GLUI_Panel *PANEL_slice_misc=NULL, *PANEL_slice_vector=NULL, *PANEL_showslice=NULL;
GLUI_Panel *PANEL_slice_xyz=NULL;
GLUI_Panel *PANEL_slice_filetype=NULL;
GLUI_Panel *PANEL_sliceload_option = NULL;
GLUI_Panel *PANEL_slicevalues = NULL;
GLUI_Panel *PANEL_plot3d=NULL;
GLUI_Panel *PANEL_setmesh = NULL;
GLUI_Panel *PANEL_mesh1 = NULL;
GLUI_Panel *PANEL_mesh2 = NULL;
GLUI_Panel *PANEL_addmesh = NULL;
GLUI_Panel *PANEL_addremovemesh = NULL;
GLUI_Panel *PANEL_boundary_temp_threshold=NULL;
GLUI_Panel *PANEL_boundary_exterior_data = NULL;
GLUI_Panel *PANEL_boundary_interior_data = NULL;
GLUI_Panel *PANEL_slice_buttonsA = NULL;
GLUI_Panel *PANEL_boundary_outline_type = NULL;
GLUI_Panel *PANEL_iso1 = NULL;
GLUI_Panel *PANEL_iso2 = NULL;
GLUI_Panel *PANEL_slice_smoke = NULL;
GLUI_Panel *PANEL_immersed = NULL;
GLUI_Panel *PANEL_immersed_region = NULL;
GLUI_Panel *PANEL_immersed_drawas = NULL;
GLUI_Panel *PANEL_immersed_outlinetype = NULL;
GLUI_Panel *PANEL_where = NULL;
GLUI_Panel *PANEL_slicedup = NULL;
GLUI_Panel *PANEL_vectorslicedup = NULL;
GLUI_Panel *PANEL_iso_eachlevel = NULL;
GLUI_Panel *PANEL_iso_alllevels = NULL;
GLUI_Panel *PANEL_bounds = NULL;
GLUI_Panel *PANEL_zone_a=NULL, *PANEL_zone_b=NULL;
GLUI_Panel *PANEL_pan1=NULL;
GLUI_Panel *PANEL_pan2=NULL;
GLUI_Panel *PANEL_run=NULL;
GLUI_Panel *PANEL_record=NULL;
GLUI_Panel *PANEL_script1=NULL;
GLUI_Panel *PANEL_script1a=NULL;
GLUI_Panel *PANEL_script1b=NULL;
GLUI_Panel *PANEL_script1c=NULL;
GLUI_Panel *PANEL_script2a=NULL;
GLUI_Panel *PANEL_script2b=NULL;
GLUI_Panel *PANEL_script3=NULL;
GLUI_Panel *PANEL_transparency2=NULL;
GLUI_Panel *PANEL_mesh     = NULL;
GLUI_Panel *PANEL_mesh_min = NULL;
GLUI_Panel *PANEL_meshxyz[6];
GLUI_Panel *PANEL_mesh_max = NULL;
GLUI_Panel *PANEL_mesh_minmax = NULL;
GLUI_Panel *PANEL_time2a=NULL;
GLUI_Panel *PANEL_time2b=NULL;
GLUI_Panel *PANEL_time2c=NULL;
GLUI_Panel *PANEL_outputpatchdata=NULL;
GLUI_Panel *PANEL_cb11 = NULL;
GLUI_Panel *PANEL_colorbar_properties = NULL;
GLUI_Panel *PANEL_coloring = NULL;
GLUI_Panel *PANEL_split1L = NULL, *PANEL_split1H = NULL;
GLUI_Panel *PANEL_split2L = NULL, *PANEL_split2H = NULL;
GLUI_Panel *PANEL_split3 = NULL;
GLUI_Panel *PANEL_extreme = NULL, *PANEL_cb8 = NULL, *PANEL_cb7 = NULL;
GLUI_Panel *PANEL_extreme_min = NULL, *PANEL_extreme_max = NULL;
GLUI_Panel *PANEL_slice_plot2da = NULL;
GLUI_Panel *PANEL_slice_plot2db = NULL;
GLUI_Panel *PANEL_slice_plot2dc = NULL;
GLUI_Panel *PANEL_slice_plot2dd = NULL;
GLUI_Panel *PANEL_slice_plot2de = NULL;
GLUI_Panel *PANEL_slice_plot2df = NULL;
GLUI_Panel *PANEL_loadbounds = NULL;
GLUI_Panel *PANEL_intersection_box = NULL;
GLUI_Panel *PANEL_read_test = NULL;
GLUI_Panel *PANEL_geom_vis2 = NULL;
GLUI_Panel *PANEL_geom_vis3 = NULL;
GLUI_Panel *PANEL_data_vis2 = NULL;
GLUI_Panel *PANEL_data_vis3 = NULL;

GLUI_Spinner *SPINNER_partdrawskip = NULL;
GLUI_Spinner *SPINNER_sliceval_ndigits = NULL;
GLUI_Spinner *SPINNER_n_part_threads = NULL;
GLUI_Spinner *SPINNER_iso_outline_ioffset = NULL;
GLUI_Spinner *SPINNER_iso_level = NULL;
GLUI_Spinner *SPINNER_iso_colors[4];
GLUI_Spinner *SPINNER_iso_transparency;
GLUI_Spinner *SPINNER_transparent_level = NULL;
GLUI_Spinner *SPINNER_slice_skip = NULL;
GLUI_Spinner *SPINNER_line_contour_num=NULL;
GLUI_Spinner *SPINNER_line_contour_width=NULL;
GLUI_Spinner *SPINNER_line_contour_min=NULL;
GLUI_Spinner *SPINNER_line_contour_max=NULL;
GLUI_Spinner *SPINNER_timebounds=NULL;
GLUI_Spinner *SPINNER_tload_begin=NULL;
GLUI_Spinner *SPINNER_tload_end=NULL;
GLUI_Spinner *SPINNER_tload_skip=NULL;
GLUI_Spinner *SPINNER_plot3d_vectorpointsize=NULL,*SPINNER_plot3d_vectorlinewidth=NULL,*SPINNER_plot3d_vectorlinelength=NULL;
GLUI_Spinner *SPINNER_sliceaverage=NULL;
GLUI_Spinner *SPINNER_zipstep=NULL;
GLUI_Spinner *SPINNER_partstreaklength=NULL;
GLUI_Spinner *SPINNER_partpointsize=NULL;
GLUI_Spinner *SPINNER_isopointsize=NULL;
GLUI_Spinner *SPINNER_isolinewidth=NULL;
GLUI_Spinner *SPINNER_plot3dpointsize=NULL;
GLUI_Spinner *SPINNER_plot3dlinewidth=NULL;
GLUI_Spinner *SPINNER_streaklinewidth=NULL;
GLUI_Spinner *SPINNER_vectorpointsize=NULL;
GLUI_Spinner *SPINNER_vectorlinewidth=NULL;
GLUI_Spinner *SPINNER_vectorlinelength=NULL;
GLUI_Spinner *SPINNER_slicevectorskip=NULL;
GLUI_Spinner *SPINNER_plot3dvectorskip=NULL;
GLUI_Spinner *SPINNER_colorbar_shift = NULL;
GLUI_Spinner *SPINNER_ncolorlabel_digits = NULL;
GLUI_Spinner *SPINNER_ncolorlabel_padding = NULL;
GLUI_Spinner *SPINNER_colorbar_selection_width = NULL;
GLUI_Spinner *SPINNER_colorbar_select_index = NULL;
GLUI_Spinner *SPINNER_colorbar_linewidth = NULL;
GLUI_Spinner *SPINNER_labels_transparency_data = NULL;
GLUI_Spinner *SPINNER_down_red = NULL, *SPINNER_down_green = NULL, *SPINNER_down_blue = NULL;
GLUI_Spinner *SPINNER_up_red = NULL, *SPINNER_up_green = NULL, *SPINNER_up_blue = NULL;
GLUI_Spinner *SPINNER_colorsplit[12];
GLUI_Spinner *SPINNER_valsplit[3];
GLUI_Spinner *SPINNER_max_mem_GB = NULL;

GLUI_Listbox *LIST_scriptlist=NULL;
GLUI_Listbox *LIST_ini_list=NULL;

GLUI_EditText *EDIT_iso_valmin=NULL;
GLUI_EditText *EDIT_iso_valmax=NULL;
GLUI_EditText *EDIT_zone_min=NULL, *EDIT_zone_max=NULL;
GLUI_EditText *EDIT_ini=NULL;
GLUI_EditText *EDIT_renderdir=NULL;
GLUI_EditText *EDIT_rendersuffix=NULL;

GLUI_Checkbox *CHECKBOX_sliceload_isvector = NULL;
GLUI_Checkbox *CHECKBOX_sortslices = NULL;
GLUI_Checkbox *CHECKBOX_sortslices_debug = NULL;
GLUI_Checkbox *CHECKBOX_visColorbarHorizontal2 = NULL;
GLUI_Checkbox *CHECKBOX_visColorbarVertical2 = NULL;
GLUI_Checkbox *CHECKBOX_show_boundary_outline=NULL;
GLUI_Checkbox *CHECKBOX_show_all_exterior_patch_data = NULL;
GLUI_Checkbox *CHECKBOX_hide_all_exterior_patch_data = NULL;
GLUI_Checkbox *CHECKBOX_show_all_interior_patch_data = NULL;
GLUI_Checkbox *CHECKBOX_hide_all_interior_patch_data = NULL;
GLUI_Checkbox *CHECKBOX_show_exterior_walls[7];
GLUI_Checkbox *CHECKBOX_show_mesh_geom[256];
GLUI_Checkbox *CHECKBOX_show_mesh_data[256];

GLUI_Checkbox *CHECKBOX_use_partload_threads = NULL;
GLUI_Checkbox *CHECKBOX_partfast = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_shaded = NULL;
GLUI_Checkbox *CHECKBOX_show_vector_slice = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_outlines = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_points = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_values = NULL;
GLUI_Checkbox *CHECKBOX_vis_slice_plot = NULL;
extern GLUI_Checkbox *CHECKBOX_cb_plot_dist;

GLUI_Checkbox *CHECKBOX_show_iso_shaded=NULL;
GLUI_Checkbox *CHECKBOX_show_iso_outline=NULL;
GLUI_Checkbox *CHECKBOX_show_iso_points=NULL;

GLUI_Rollout *ROLLOUT_plotslice = NULL;
GLUI_Spinner *SPINNER_slice_x   = NULL;
GLUI_Spinner *SPINNER_slice_y   = NULL;
GLUI_Spinner *SPINNER_slice_z   = NULL;
GLUI_Spinner *SPINNER_slice_dx = NULL;
GLUI_Spinner *SPINNER_slice_dy = NULL;
GLUI_Spinner *SPINNER_slice_dz = NULL;
GLUI_Spinner *SPINNER_size_factor2         = NULL;
GLUI_Spinner *SPINNER_plot2d_dt = NULL;
GLUI_Spinner *SPINNER_meshclip[6];
GLUI_Spinner *SPINNER_set_mesh = NULL;

GLUI_Checkbox *CHECKBOX_slice_load_incremental=NULL;
GLUI_Checkbox *CHECKBOX_color_vector_black = NULL;
GLUI_Checkbox *CHECKBOX_show_node_slices_and_vectors=NULL;
GLUI_Checkbox *CHECKBOX_show_cell_slices_and_vectors=NULL;
GLUI_Checkbox *CHECKBOX_showchar=NULL, *CHECKBOX_showonlychar;
GLUI_Checkbox *CHECKBOX_script_step=NULL;
GLUI_Checkbox *CHECKBOX_constant_coloring=NULL;
GLUI_Checkbox *CHECKBOX_data_coloring=NULL;
GLUI_Checkbox *CHECKBOX_sort2=NULL;
GLUI_Checkbox *CHECKBOX_smooth2=NULL;
GLUI_Checkbox *CHECKBOX_overwrite_all=NULL;
GLUI_Checkbox *CHECKBOX_compress_autoloaded=NULL;
GLUI_Checkbox *CHECKBOX_erase_all=NULL;
GLUI_Checkbox *CHECKBOX_multi_task=NULL;
GLUI_Checkbox *CHECKBOX_showtracer=NULL;
GLUI_Checkbox *CHECKBOX_cellcenter_slice_interp=NULL;
GLUI_Checkbox *CHECKBOX_skip_subslice=NULL;
GLUI_Checkbox *CHECKBOX_turb_slice=NULL;
GLUI_Checkbox *CHECKBOX_average_slice=NULL;
GLUI_Checkbox *CHECKBOX_use_tload_begin=NULL;
GLUI_Checkbox *CHECKBOX_use_tload_end=NULL;
GLUI_Checkbox *CHECKBOX_use_tload_skip=NULL;
GLUI_Checkbox *CHECKBOX_colorbar_flip = NULL;
GLUI_Checkbox *CHECKBOX_fixedpoint=NULL;
GLUI_Checkbox *CHECKBOX_exponential=NULL;
GLUI_Checkbox *CHECKBOX_force_decimal=NULL;
GLUI_Checkbox *CHECKBOX_force_zero_pad = NULL;
GLUI_Checkbox *CHECKBOX_colorbar_autoflip = NULL;
GLUI_Checkbox *CHECKBOX_labels_shadedata = NULL;
GLUI_Checkbox *CHECKBOX_labels_shade = NULL;
GLUI_Checkbox *CHECKBOX_transparentflag = NULL;
GLUI_Checkbox *CHECKBOX_use_lighting = NULL;
GLUI_Checkbox *CHECKBOX_show_extreme_mindata = NULL;
GLUI_Checkbox *CHECKBOX_show_extreme_maxdata = NULL;
GLUI_Checkbox *CHECKBOX_use_meshclip[6];
GLUI_Checkbox *CHECKBOX_show_intersection_box=NULL;
GLUI_Checkbox *CHECKBOX_show_intersected_meshes = NULL;
GLUI_Checkbox *CHECKBOX_load_only_when_unloaded = NULL;

GLUI_RadioGroup *RADIO_intersect_option=NULL;
GLUI_RadioGroup *RADIO_filetype=NULL;
GLUI_RadioGroup *RADIO_sliceload_dir=NULL;
GLUI_RadioGroup *RADIO_iso_setmin=NULL;
GLUI_RadioGroup *RADIO_iso_setmax=NULL;
GLUI_RadioGroup *RADIO_transparency_option=NULL;
GLUI_RadioGroup *RADIO_slice_celltype=NULL;
GLUI_RadioGroup *RADIO_slice_edgetype=NULL;
GLUI_RadioGroup *RADIO_boundary_edgetype = NULL;
GLUI_RadioGroup *RADIO_boundaryslicedup = NULL;
GLUI_RadioGroup *RADIO_slicedup = NULL;
GLUI_RadioGroup *RADIO_vectorslicedup = NULL;
GLUI_RadioGroup *RADIO_showhide = NULL;
GLUI_RadioGroup *RADIO_contour_type = NULL;
GLUI_RadioGroup *RADIO_zone_setmin=NULL, *RADIO_zone_setmax=NULL;
GLUI_RadioGroup *RADIO_bf=NULL, *RADIO_p3=NULL,*RADIO_slice=NULL;
GLUI_RadioGroup *RADIO_part5=NULL;
GLUI_RadioGroup *RADIO_plot3d_isotype=NULL;
GLUI_RadioGroup *RADIO_plot3d_display=NULL;
GLUI_RadioGroup *RADIO2_plot3d_display = NULL;
GLUI_RadioButton *RADIO_button_cutcell = NULL;

GLUI_RadioButton *RADIOBUTTON_iso_percentile_min = NULL;
GLUI_RadioButton *RADIOBUTTON_iso_percentile_max = NULL;
GLUI_RadioButton *RADIOBUTTON_sliceload_x=NULL;
GLUI_RadioButton *RADIOBUTTON_sliceload_y=NULL;
GLUI_RadioButton *RADIOBUTTON_sliceload_z=NULL;
GLUI_RadioButton *RADIOBUTTON_sliceload_xyz=NULL;
GLUI_RadioButton *RADIOBUTTON_plot3d_iso_hidden=NULL;
GLUI_RadioButton *RADIOBUTTON_zone_permin=NULL;
GLUI_RadioButton *RADIOBUTTON_zone_permax=NULL;

#define FLIP                  19
#define CB_USE_LIGHTING      120
#define COLORBAR_EXTREME      16
// #define SPLIT_COLORBAR         1 now defined in smokeviewdefs.h
#define COLORBAR_SHOWSPLIT    17

//*** boundprocinfo entries
#define ZONE_ROLLOUT     0
#define SMOKE3D_ROLLOUT  1
#define BOUNDARY_ROLLOUT 2
#define ISO_ROLLOUT      3
#define PART_ROLLOUT     4
#define PLOT3D_ROLLOUT   6
#define SLICE_ROLLOUT_BOUNDS    7
#define HVACDUCT_ROLLOUT 8
#define HVACNODE_ROLLOUT 9
#define TIME_ROLLOUT     10
#define VIEWOPTIONS_ROLLOUT 11

procdata  boundprocinfo[12];
int      nboundprocinfo = 0;

#define MESHBLOCKAGE_ROLLOUT 0
#define MESHDATA_ROLLOUT 1

procdata  viewprocinfo[2];
int      nviewprocinfo = 0;

//*** loadprocinfo entries
#define LOAD_AUTO_ROLLOUT       0
#define LOAD_TIMESET_ROLLOUT    1
#define LOAD_TIMEBOUND_ROLLOUT  2
#define LOAD_SPACEBOUND_ROLLOUT 3

procdata  loadprocinfo[4];
int      nloadprocinfo = 0;

//*** isoprocinfo entries
#define ISO_ROLLOUT_BOUNDS   0
#define ISO_ROLLOUT_SETTINGS 1
#define ISO_ROLLOUT_COLOR    2
procdata  isoprocinfo[3];
int      nisoprocinfo=0;


//*** hvacductprocinfo entries
procdata  hvacductprocinfo[1];
int      nhvacductprocinfo = 0;

//*** hvacnodeprocinfo entries
procdata  hvacnodeprocinfo[1];
int      nhvacnodeprocinfo = 0;

//*** sliceprocinfo entries
#define SLICE_BOUND             0
#define SLICE_CHOP              1
#define SLICE_AVERAGE_ROLLOUT   2
#define SLICE_VECTOR_ROLLOUT    3
#define LINE_CONTOUR_ROLLOUT    4
#define SLICE_HISTOGRAM_ROLLOUT 5
#define SLICE_DUP_ROLLOUT       6
#define SLICE_SETTINGS_ROLLOUT  7
#define SLICE_2D_PLOTS          8
#define SLICE_PLOT2D_ROLLOUT    9
procdata  sliceprocinfo[10];
int      nsliceprocinfo=0;

//*** plot3dprocinfo entries
#define PLOT3D_BOUND              0
#define PLOT3D_CHOP               1
#define PLOT3D_VECTOR_ROLLOUT     2
#define PLOT3D_ISOSURFACE_ROLLOUT 3

procdata  plot3dprocinfo[4];
int      nplot3dprocinfo=0;



//*** filedatacolprocinfo entries
#define FILE_ROLLOUT     0
#define DATA_ROLLOUT     1
#define COLORING_ROLLOUT 2
#define MEMCHECK_ROLLOUT 3

procdata  filedatacolprocinfo[4];
int      nfiledatacolprocinfo = 0;

//*** fileprocinfo entries
#define SHOWHIDE_ROLLOUT   0
#define COMPRESS_ROLLOUT   1
#define SCRIPT_ROLLOUT     2
#define CONFIG_ROLLOUT     3

procdata  fileprocinfo[4];
int      nfileprocinfo = 0;

//*** particleprocinfo entries
#define PARTICLE_BOUND             0
#define PARTICLE_CHOP              1
#define PARTICLE_SETTINGS          2

procdata particleprocinfo[3];
int      nparticleprocinfo=0;

//*** subboundprocinfo entries
#define BOUNDARY_BOUND             0
#define BOUNDARY_CHOP              1
#define BOUNDARY_OUTPUT_ROLLOUT    2
#define BOUNDARY_DUPLICATE_ROLLOUT 3
#define BOUNDARY_SETTINGS_ROLLOUT  4

procdata  subboundprocinfo[5];
int       nsubboundprocinfo=0;

/* ------------------ UpdateShowExtPatch ------------------------ */

extern "C" void UpdateShowExtPatch(int show_option, int hide_option){
  if(CHECKBOX_show_all_exterior_patch_data != NULL)CHECKBOX_show_all_exterior_patch_data->set_int_val(show_option);
  if(CHECKBOX_hide_all_exterior_patch_data != NULL)CHECKBOX_hide_all_exterior_patch_data->set_int_val(hide_option);
}

/* ------------------ UpdateShowIntPatch ------------------------ */

extern "C" void UpdateShowIntPatch(int show_option, int hide_option){
  if(CHECKBOX_show_all_interior_patch_data != NULL)CHECKBOX_show_all_interior_patch_data->set_int_val(show_option);
  if(CHECKBOX_hide_all_interior_patch_data != NULL)CHECKBOX_hide_all_interior_patch_data->set_int_val(hide_option);
}

/* ------------------ UpdateColorbarSelectionIndex ------------------------ */

extern "C" void UpdateColorbarSelectionIndex(int val){
  colorbar_select_index = val;
  if(SPINNER_colorbar_select_index != NULL)SPINNER_colorbar_select_index->set_int_val(val);
}

/* ------------------ GLUIUpdateLoadAllSlices ------------------------ */

extern "C" void GLUIUpdateLoadAllSlices(void){
  if(RADIO_filetype!=NULL)RADIO_filetype->set_int_val(sliceload_filetype);
  if(LISTBOX_sliceload!=NULL)LISTBOX_sliceload->set_int_val(sliceload_boundtype);
  if(CHECKBOX_sliceload_isvector!=NULL)CHECKBOX_sliceload_isvector->set_int_val(sliceload_isvector);
  if(RADIO_sliceload_dir!=NULL)RADIO_sliceload_dir->set_int_val(sliceload_dir);
}

/* ------------------ GLUIUpdatePartPointSize ------------------------ */

extern "C" void GLUIUpdatePartPointSize(void){
  SPINNER_partpointsize->set_float_val(partpointsize);
}

/* ------------------ GLUIUpdateLoadWhenLoaded ------------------------ */

extern "C" void GLUIUpdateLoadWhenLoaded(void){
  CHECKBOX_load_only_when_unloaded->set_int_val(load_only_when_unloaded);
}

/* ------------------ GLUIUpdatePlotLabel ------------------------ */

extern "C" void GLUIUpdatePlotLabel(void){
  GLUISliceBoundCB(SLICE_PLOT_LABEL);
}

/* ------------------ GLUIUpdateSortSlices ------------------------ */

extern "C" void GLUIUpdateSortSlices(void){
  CHECKBOX_sortslices->set_int_val(sortslices);
  CHECKBOX_sortslices_debug->set_int_val(sortslices_debug);
}

/* ------------------ GLUIUpdatePlot2DSize2 ------------------------ */

extern "C" void GLUIUpdatePlot2DSize2(void){
  if(SPINNER_size_factor2!=NULL)SPINNER_size_factor2->set_float_val(plot2d_size_factor);
  if(SPINNER_plot2d_dt !=NULL)SPINNER_plot2d_dt->set_float_val(plot2d_time_average);
}

#ifdef pp_REFRESH
/* ------------------ GLUIRefreshDialogs ------------------------ */

extern "C" void GLUIRefreshDialogs(void){
  if(glui_bounds!=NULL)glui_bounds->refresh();
  if(glui_clip!=NULL)glui_clip->refresh();
  if(glui_colorbar!=NULL)glui_colorbar->refresh();
  if(glui_labels!=NULL)glui_labels->refresh();
  if(glui_geometry!=NULL)glui_geometry->refresh();
  if(glui_motion!=NULL)glui_motion->refresh();
  if(glui_device!=NULL)glui_device->refresh();
  if(glui_shooter!=NULL)glui_shooter->refresh();
  if(glui_stereo!=NULL)glui_stereo->refresh();
  if(glui_tour!=NULL)glui_tour->refresh();
  if(glui_trainer!=NULL)glui_trainer->refresh();
}
#endif

/* ------------------ GLUIUpdateSliceSkip ------------------------ */

extern "C" void GLUIUpdateSliceSkip(void){
  GLUISliceBoundCB(SLICE_SKIP);
}

/* ------------------ GLUIUpdateColorbarControls2 ------------------------ */

extern "C" void GLUIUpdateColorbarControls2(void){
  if(CHECKBOX_visColorbarVertical2 != NULL && CHECKBOX_visColorbarVertical2->get_int_val() != visColorbarVertical)CHECKBOX_visColorbarVertical2->set_int_val(visColorbarVertical);
  if(CHECKBOX_visColorbarHorizontal2 != NULL && CHECKBOX_visColorbarHorizontal2->get_int_val() != visColorbarHorizontal)CHECKBOX_visColorbarHorizontal2->set_int_val(visColorbarHorizontal);
}

/* ------------------ GLUIUpdateTransparency ------------------------ */

extern "C" void GLUIUpdateTransparency(void){
  if(CHECKBOX_transparentflag!=NULL)CHECKBOX_transparentflag->set_int_val(use_transparency_data);
}

/* ------------------ GLUIUpdateUseLighting ------------------------ */

extern "C" void GLUIUpdateUseLighting(void){
  if(CHECKBOX_use_lighting!=NULL)CHECKBOX_use_lighting->set_int_val(use_lighting);
}

/* ------------------ GLUISplitCB ------------------------ */

extern "C" void GLUISplitCB(int var){
  int isplit, i;
  float denom;

  switch(var){
  case COLORBAR_SHOWSPLIT:
    LISTBOX_cb_bound->set_int_val(colorbars.split_colorbar_index);
    GLUISliceBoundCB(COLORBAR_LIST2);
    visColorbarVertical = 1;
    updatemenu = 1;
    break;
  case SPLIT_COLORBAR:
    if(split_colorbar == NULL)break;
    denom = splitvals[2] - splitvals[0];
    if(denom == 0.0)denom = 1.0;
    isplit = CLAMP(255*(splitvals[1] - splitvals[0]) / denom, 0, 254);
    split_colorbar->node_index[1] = isplit;
    split_colorbar->node_index[2] = isplit + 1;

    for(i = 0; i < 12; i++){
      split_colorbar->node_rgb[i] = colorsplit[i] & 0xFF;
    }
    RemapColorbar(split_colorbar, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
    UpdateRGBColors(colorbar_select_index);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIExtremeCB ------------------------ */

extern "C" void GLUIExtremeCB(int var){
  colorbardata *cbi;
  int i;

  switch(var){
  case COLORBAR_EXTREME:
    if(colorbartype<0||colorbartype>=colorbars.ncolorbars)return;
    cbi = colorbars.colorbarinfo + colorbartype;
    if(show_extreme_mindata==1){
      if(SPINNER_down_red!=NULL)SPINNER_down_red->enable();
      if(SPINNER_down_green!=NULL)SPINNER_down_green->enable();
      if(SPINNER_down_blue!=NULL)SPINNER_down_blue->enable();
      for(i = 0; i<3; i++){
        rgb_below_min[i] = glui_down_rgb[i];
      }
    }
    else{
      if(SPINNER_down_red!=NULL)SPINNER_down_red->disable();
      if(SPINNER_down_green!=NULL)SPINNER_down_green->disable();
      if(SPINNER_down_blue!=NULL)SPINNER_down_blue->disable();
    }
    if(show_extreme_maxdata==1){
      if(SPINNER_up_red!=NULL)SPINNER_up_red->enable();
      if(SPINNER_up_green!=NULL)SPINNER_up_green->enable();
      if(SPINNER_up_blue!=NULL)SPINNER_up_blue->enable();
      for(i = 0; i<3; i++){
        rgb_above_max[i] = glui_up_rgb[i];
      }
    }
    else{
      if(SPINNER_up_red!=NULL)SPINNER_up_red->disable();
      if(SPINNER_up_green!=NULL)SPINNER_up_green->disable();
      if(SPINNER_up_blue!=NULL)SPINNER_up_blue->disable();
    }
    RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
    UpdateRGBColors(colorbar_select_index);
    updatemenu = 1;
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateExtremeVals ------------------------ */

extern "C" void GLUIUpdateExtremeVals(void){
  unsigned char *rgb_local;

  rgb_local = rgb_below_min;
  if(SPINNER_down_red!=NULL)SPINNER_down_red->set_int_val((int)(rgb_local[0]));
  if(SPINNER_down_green!=NULL)SPINNER_down_green->set_int_val((int)(rgb_local[1]));
  if(SPINNER_down_blue!=NULL)SPINNER_down_blue->set_int_val((int)(rgb_local[2]));

  rgb_local = rgb_above_max;
  if(SPINNER_up_red!=NULL)SPINNER_up_red->set_int_val((int)(rgb_local[0]));
  if(SPINNER_up_green!=NULL)SPINNER_up_green->set_int_val((int)(rgb_local[1]));
  if(SPINNER_up_blue!=NULL)SPINNER_up_blue->set_int_val((int)(rgb_local[2]));
}

/* ------------------ GLUIUpdateExtreme ------------------------ */

extern "C" void GLUIUpdateExtreme(void){
  if(CHECKBOX_show_extreme_mindata!=NULL){
    CHECKBOX_show_extreme_mindata->set_int_val(show_extreme_mindata);
  }
  if(CHECKBOX_show_extreme_maxdata!=NULL){
    CHECKBOX_show_extreme_maxdata->set_int_val(show_extreme_maxdata);
  }
  GLUIExtremeCB(COLORBAR_EXTREME);
}

/* ------------------ GLUIUpdateColorbarBound ------------------------ */

extern "C" void GLUIUpdateColorbarBound(void){
  if(LISTBOX_cb_bound != NULL)LISTBOX_cb_bound->set_int_val(colorbartype);
  if(LISTBOX_cb_toggle_bound1!=NULL)LISTBOX_cb_toggle_bound1->set_int_val(index_colorbar1);
  if(LISTBOX_cb_toggle_bound2!=NULL)LISTBOX_cb_toggle_bound2->set_int_val(index_colorbar2);
}

/* ------------------ GLUISetColorbarListBound ------------------------ */

extern "C" void GLUISetColorbarListBound(int val){
  if(LISTBOX_cb_bound!=NULL)LISTBOX_cb_bound->set_int_val(val);
}

/* ------------------ GLUISetColorbarDigits ------------------------ */

extern "C" void GLUISetColorbarDigits(void){
  SPINNER_ncolorlabel_digits->set_int_val(ncolorlabel_digits);
  CHECKBOX_fixedpoint->set_int_val(force_fixedpoint);
  CHECKBOX_exponential->set_int_val(force_exponential);
  CHECKBOX_force_decimal->set_int_val(force_decimal);
  CHECKBOX_force_zero_pad->set_int_val(force_zero_pad);
}

/* ------------------ GLUIUpdateColorbarFlip ------------------------ */

extern "C" void GLUIUpdateColorbarFlip(void){
  CHECKBOX_colorbar_flip->set_int_val(colorbar_flip);
  CHECKBOX_colorbar_autoflip->set_int_val(colorbar_autoflip);
}

/* ------------------ GLUISetLabelControls2 ------------------------ */

extern "C" void GLUISetLabelControls2(){
  if(CHECKBOX_labels_shade!=NULL)CHECKBOX_labels_shade->set_int_val(setbw);
  if(CHECKBOX_labels_shadedata != NULL)CHECKBOX_labels_shadedata->set_int_val(setbwdata);
}

/* ------------------ GLUIUpdatePartFast ------------------------ */

extern "C" void GLUIUpdatePartFast(void){
  if(CHECKBOX_partfast!=NULL)CHECKBOX_partfast->set_int_val(partfast);
  if(CHECKBOX_use_partload_threads!=NULL)CHECKBOX_use_partload_threads->set_int_val(use_partload_threads);
  PartBoundCB(PARTFAST);
}

/* ------------------ GLUIUpdateListIsoColorobar ------------------------ */

extern "C" void GLUIUpdateListIsoColorobar(void){
  if(LIST_iso_colorbar!=NULL)LIST_iso_colorbar->set_int_val(colorbars.iso_colorbar_index);
}


/* ------------------ GLUIUpdateIsoBounds ------------------------ */

extern "C" void GLUIUpdateIsoBounds(void){
  boundsdata *sb = NULL;
  char label[256];

  if(iisottype<0|| iisottype >=niso_bounds)return;
  sb = isobounds + iisottype;
  setisomin = sb->dlg_setvalmin;
  setisomax = sb->dlg_setvalmax;
  strcpy(label, "Isosurface - colored by ");
  strcat(label, sb->label->longlabel);
  strcat(label, "(");
  strcat(label, sb->label->unit);
  strcat(label, ")");
  if(ROLLOUT_iso!=NULL)ROLLOUT_iso->set_name(label);

  if(setisomin == GLOBAL_MIN || setisomin == SET_MIN){
    if(setisomin == GLOBAL_MIN){
      glui_iso_valmin = iso_global_min;
      if(EDIT_iso_valmin != NULL)EDIT_iso_valmin->disable();
    }
    if(setisomin == SET_MIN){
      glui_iso_valmin = sb->dlg_valmin;
      if(EDIT_iso_valmin != NULL)EDIT_iso_valmin->enable();
    }
    if(EDIT_iso_valmin != NULL)EDIT_iso_valmin->set_float_val(glui_iso_valmin);
    if(RADIO_iso_setmin!=NULL&&sb!=NULL)RADIO_iso_setmin->set_int_val(setisomin);
  }
  if(setisomax == GLOBAL_MAX || setisomin == SET_MAX){
    if(setisomax == GLOBAL_MAX){
      glui_iso_valmax = iso_global_max;
      if(EDIT_iso_valmin != NULL)EDIT_iso_valmax->disable();
    }
    if(setisomax == SET_MAX){
      glui_iso_valmax = sb->dlg_valmax;
      if(EDIT_iso_valmax!=NULL)EDIT_iso_valmax->enable();
    }
    if(EDIT_iso_valmax != NULL)EDIT_iso_valmax->set_float_val(glui_iso_valmax);
    if(RADIO_iso_setmax!=NULL&&sb!=NULL)RADIO_iso_setmax->set_int_val(setisomax);
  }
}
/* ------------------ GLUIUpdateVectorpointsize ------------------------ */


extern "C" void GLUIUpdateVectorpointsize(void){
  if(SPINNER_vectorpointsize!=NULL)SPINNER_vectorpointsize->set_int_val(vectorpointsize);
}

/* ------------------ GLUIUpdateSliceDupDialog ------------------------ */


extern "C" void GLUIUpdateSliceDupDialog(void){
  if(RADIO_boundaryslicedup != NULL)RADIO_boundaryslicedup->set_int_val(boundaryslicedup_option);
  if(RADIO_slicedup != NULL)RADIO_slicedup->set_int_val(slicedup_option);
  if(RADIO_vectorslicedup != NULL)RADIO_vectorslicedup->set_int_val(vectorslicedup_option);
}

/* ------------------ UpdateIsoControls ------------------------ */

void UpdateIsoControls(void){
  if(use_transparency_data==1){
    if(SPINNER_iso_colors[3] != NULL)SPINNER_iso_colors[3]->enable();
    if(SPINNER_iso_transparency != NULL)SPINNER_iso_transparency->enable();
  }
  else{
    if(SPINNER_iso_colors[3] != NULL)SPINNER_iso_colors[3]->disable();
    if(SPINNER_iso_transparency != NULL)SPINNER_iso_transparency->disable();
  }
}

/* ------------------ GLUISliceInObstMenu2Dialog ------------------------ */

extern "C" void GLUISliceInObstMenu2Dialog(int var){
  global_scase.show_slice_in_obst = var;
  if(global_scase.show_slice_in_obst==GAS_AND_SOLID){
    show_slice_in_gas   = 1;
    show_slice_in_solid = 1;
  }
  else if(global_scase.show_slice_in_obst==ONLY_IN_GAS){
    show_slice_in_gas   = 1;
    show_slice_in_solid = 0;
  }
  else if(global_scase.show_slice_in_obst==ONLY_IN_SOLID){
    show_slice_in_gas   = 0;
    show_slice_in_solid = 1;
  }
  else{
    show_slice_in_gas   = 0;
    show_slice_in_solid = 0;
  }
  show_slice_shaded[IN_GAS_GLUI]   = show_slice_in_gas;
  show_slice_shaded[IN_SOLID_GLUI] = show_slice_in_solid;
  show_vector_slice[IN_GAS_GLUI]   = show_slice_in_gas;
  show_vector_slice[IN_SOLID_GLUI] = show_slice_in_solid;
  GLUIImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
}

/* ------------------ SliceInObstDialog2Menu ------------------------ */

void SliceInObstDialog2Menu(void){
  if(show_slice_shaded[IN_GAS_GLUI] == 1 && show_slice_shaded[IN_SOLID_GLUI] == 1){
    global_scase.show_slice_in_obst = GAS_AND_SOLID;
  }
  else if(show_slice_shaded[IN_GAS_GLUI] == 1 && show_slice_shaded[IN_SOLID_GLUI] == 0){
    global_scase.show_slice_in_obst = ONLY_IN_GAS;
  }
  else if(show_slice_shaded[IN_GAS_GLUI] == 0 && show_slice_shaded[IN_SOLID_GLUI] == 1){
    global_scase.show_slice_in_obst = ONLY_IN_SOLID;
  }
  else{
    global_scase.show_slice_in_obst = NEITHER_GAS_NOR_SOLID;
  }
  updatemenu = 1;
}

/* ------------------ GLUIUpdateIsoColorlevel ------------------------ */

extern "C" void GLUIUpdateIsoColorlevel(void){
  GLUIIsoBoundCB(ISO_LEVEL);
  GLUIIsoBoundCB(ISO_COLORS);
}

/* ------------------ ParticleRolloutCB ------------------------ */

void ParticleRolloutCB(int var){
  GLUIToggleRollout(particleprocinfo, nparticleprocinfo, var);
}

/* ------------------ Plot3dRolloutCB ------------------------ */

void Plot3dRolloutCB(int var){
  GLUIToggleRollout(plot3dprocinfo, nplot3dprocinfo, var);
}

/* ------------------ HVACRolloutCB ------------------------ */

void HVACRolloutCB(int var){
}

/* ------------------ SliceRolloutCB ------------------------ */

void SliceRolloutCB(int var){
  GLUIToggleRollout(sliceprocinfo, nsliceprocinfo, var);
}

/* ------------------ IsoRolloutCB ------------------------ */

void IsoRolloutCB(int var){
  GLUIToggleRollout(isoprocinfo, nisoprocinfo, var);
}

/* ------------------ LoadRolloutCB ------------------------ */

void LoadRolloutCB(int var){
  GLUIToggleRollout(loadprocinfo, nloadprocinfo, var);
}

/* ------------------ BoundRolloutCB ------------------------ */

void BoundRolloutCB(int var){
  GLUIToggleRollout(boundprocinfo, nboundprocinfo, var);
  if(global_scase.nzoneinfo>0){
    if(var==ZONE_ROLLOUT){
      GLUISliceBoundCB(SETZONEVALMINMAX);
    }
    if(var==SLICE_ROLLOUT_BOUNDS){
      list_slice_index = CLAMP(list_slice_index,0,nlist_slice_index-1);
      if(RADIO_slice!=NULL)RADIO_slice->set_int_val(list_slice_index);
      GLUISliceBoundCB(FILETYPE_INDEX);
    }
  }
}

/* ------------------ ViewRolloutCB ------------------------ */

void ViewRolloutCB(int var){
  GLUIToggleRollout(viewprocinfo, nviewprocinfo, var);
}

/* ------------------ SubBoundRolloutCB ------------------------ */

void SubBoundRolloutCB(int var){
  GLUIToggleRollout(subboundprocinfo, nsubboundprocinfo, var);
}

/* ------------------ FileDataColRolloutCB ------------------------ */

void FileDataColRolloutCB(int var){
  GLUIToggleRollout(filedatacolprocinfo, nfiledatacolprocinfo, var);
}

/* ------------------ FileRolloutCB ------------------------ */

void FileRolloutCB(int var){
  GLUIToggleRollout(fileprocinfo, nfileprocinfo, var);
}

/* ------------------ GLUIUpdateZoneBounds ------------------------ */

extern "C" void GLUIUpdateZoneBounds(void){
  if(EDIT_zone_min!=NULL)EDIT_zone_min->set_float_val(zonemin);
  if(EDIT_zone_max!=NULL)EDIT_zone_max->set_float_val(zonemax);
}
/* ------------------ GLUIUpdateVecFactor ------------------------ */

extern "C" void GLUIUpdateVecFactor(void){
  if(SPINNER_plot3d_vectorlinelength!=NULL)SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
  if(SPINNER_vectorlinelength!=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
}

/* ------------------ GLUIUpdateResearchMode ------------------------ */

extern "C" void GLUIUpdateResearchMode(void){
  GLUISliceBoundCB(RESEARCH_MODE);
}

/* ------------------ GLUIUpdateScriptStop ------------------------ */

extern "C" void GLUIUpdateScriptStop(void){
  if(BUTTON_script_start!=NULL)BUTTON_script_start->enable();
  if(BUTTON_script_stop!=NULL)BUTTON_script_stop->disable();
  if(BUTTON_script_runscript!=NULL)BUTTON_script_runscript->enable();
  if(EDIT_renderdir!=NULL)EDIT_renderdir->enable();
}

/* ------------------ GLUIUpdateScriptStart ------------------------ */

extern "C" void GLUIUpdateScriptStart(void){
  if(BUTTON_script_start!=NULL)BUTTON_script_start->disable();
  if(BUTTON_script_stop!=NULL)BUTTON_script_stop->enable();
  if(BUTTON_script_runscript!=NULL)BUTTON_script_runscript->disable();
  if(EDIT_renderdir!=NULL)EDIT_renderdir->disable();
}

/* ------------------ GLUIUpdateScriptStep ------------------------ */

extern "C" void GLUIUpdateScriptStep(void){
  CHECKBOX_script_step->set_int_val(script_step);
  if(script_step==1){
    BUTTON_step->enable();
  }
  else{
    BUTTON_step->disable();
  }
}

/* ------------------ ColorTableCompare ------------------------ */

int ColorTableCompare(const void *arg1, const void *arg2){
  colortabledata *cti, *ctj;
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;

  cti = colortableinfo + i;
  ctj = colortableinfo + j;

  return(strcmp(cti->label, ctj->label));
}

/* ------------------ GLUIUpdateColorTableList ------------------------ */

extern "C" void GLUIUpdateColorTableList(int ncolortableinfo_old){
  int i, *order=NULL;

  if(LIST_colortable==NULL)return;
  for(i = -1; i<ncolortableinfo_old; i++){
    LIST_colortable->delete_item(i);
  }

  if(ncolortableinfo>0){
    NewMemory((void **)&order, ncolortableinfo*sizeof(int));
    for(i = 0; i < ncolortableinfo; i++){
      order[i] = i;
    }
    qsort((int *)order, (size_t)ncolortableinfo, sizeof(int), ColorTableCompare);
  }

  if(LIST_colortable != NULL){
    for(i = -1; i < ncolortableinfo; i++){
      if(i == -1){
        LIST_colortable->add_item(i, "Custom");
      }
      else{
        if(order != NULL){
          colortabledata *cti;

          cti = colortableinfo + order[i];
          LIST_colortable->add_item(i, cti->label);
        }
      }
    }
  }
  if(ncolortableinfo>0){
    FREEMEMORY(order);
  }
}

/* ------------------ FileShowCB ------------------------ */

void FileShowCB(int var){
  updatemenu = 1;
  switch(var){
  case FILESHOW_sizes:
    GetFileSizes();
    break;
  case  FILESHOW_plot3d:
    switch(showhide_option){
    case SHOWALL_FILES:
    case SHOWONLY_FILE:
      Plot3DShowMenu(SHOWALL_PLOT3D);
      break;
    case HIDEALL_FILES:
      Plot3DShowMenu(HIDEALL_PLOT3D);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case  FILESHOW_particle:
    switch(showhide_option){
    case SHOWALL_FILES:
      ParticleShowMenu(SHOWALL_PARTICLE);
      break;
    case SHOWONLY_FILE:
      ParticleShowMenu(SHOWALL_PARTICLE);
      if(nsmoke3dloaded != 0)Smoke3DShowMenu(HIDEALL_SMOKE3D);
      if(nisoloaded != 0)IsoShowMenu(HIDEALL_ISO);
      if(nsliceloaded != 0)ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      if(nvsliceloaded != 0)ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      if(npatchloaded != 0)ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      break;
    case HIDEALL_FILES:
      ParticleShowMenu(HIDEALL_PARTICLE);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case  FILESHOW_slice:
    switch(showhide_option){
    case SHOWALL_FILES:
      ShowHideSliceMenu(GLUI_SHOWALL_SLICE);
      break;
    case SHOWONLY_FILE:
      ShowHideSliceMenu(GLUI_SHOWALL_SLICE);
      if(nvsliceloaded != 0)ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      if(npatchloaded != 0)ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      if(nsmoke3dloaded != 0)Smoke3DShowMenu(HIDEALL_SMOKE3D);
      if(nisoloaded != 0)IsoShowMenu(HIDEALL_ISO);
      if(npartloaded != 0)ParticleShowMenu(HIDEALL_PARTICLE);
      break;
    case HIDEALL_FILES:
      ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case  FILESHOW_vslice:
    switch(showhide_option){
    case SHOWALL_FILES:
      ShowVSliceMenu(GLUI_SHOWALL_VSLICE);
      break;
    case SHOWONLY_FILE:
      ShowVSliceMenu(GLUI_SHOWALL_VSLICE);
      if(npatchloaded != 0)ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      if(nsmoke3dloaded != 0)Smoke3DShowMenu(HIDEALL_SMOKE3D);
      if(nisoloaded != 0)IsoShowMenu(HIDEALL_ISO);
      if(npartloaded != 0)ParticleShowMenu(HIDEALL_PARTICLE);
      if(nsliceloaded != 0)ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      break;
    case HIDEALL_FILES:
      ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case  FILESHOW_boundary:
    switch(showhide_option){
    case SHOWALL_FILES:
      ShowBoundaryMenu(GLUI_SHOWALL_BOUNDARY);
      break;
    case SHOWONLY_FILE:
      ShowBoundaryMenu(GLUI_SHOWALL_BOUNDARY);
      if(nsmoke3dloaded != 0)Smoke3DShowMenu(HIDEALL_SMOKE3D);
      if(npartloaded != 0)ParticleShowMenu(HIDEALL_PARTICLE);
      if(nvsliceloaded != 0)ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      if(nsliceloaded != 0)ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      if(nisoloaded != 0)IsoShowMenu(HIDEALL_ISO);
      break;
    case HIDEALL_FILES:
      ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case  FILESHOW_3dsmoke:
    switch(showhide_option){
    case SHOWALL_FILES:
      Smoke3DShowMenu(SHOWALL_SMOKE3D);
      break;
    case SHOWONLY_FILE:
      Smoke3DShowMenu(SHOWALL_SMOKE3D);
      if(npatchloaded != 0)ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      if(npartloaded != 0)ParticleShowMenu(HIDEALL_PARTICLE);
      if(nvsliceloaded != 0)ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      if(nsliceloaded != 0)ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      if(nisoloaded != 0)IsoShowMenu(HIDEALL_ISO);
      break;
    case HIDEALL_FILES:
      Smoke3DShowMenu(HIDEALL_SMOKE3D);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case  FILESHOW_isosurface:
    switch(showhide_option){
    case SHOWALL_FILES:
      IsoShowMenu(SHOWALL_ISO);
      break;
    case SHOWONLY_FILE:
      IsoShowMenu(SHOWALL_ISO);
      if(nsmoke3dloaded != 0)Smoke3DShowMenu(HIDEALL_SMOKE3D);
      if(npatchloaded != 0)ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      if(npartloaded != 0)ParticleShowMenu(HIDEALL_PARTICLE);
      if(nvsliceloaded != 0)ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      if(nsliceloaded != 0)ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      break;
    case HIDEALL_FILES:
      IsoShowMenu(HIDEALL_ISO);
      break;
    default:
      assert(FFALSE);
      break;
    }
    break;
  default:
    break;
  }
}

/* ------------------ MemcheckCB ------------------------ */

void MemcheckCB(int var){
  switch(var){
  case MEMCHECK:
    if(max_mem_GB<0.0){
      max_mem_GB = 0.0;
      SPINNER_max_mem_GB->set_float_val(0.0);
    }
    SetMemCheck(max_mem_GB);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ BoundsDlgCB ------------------------ */

void BoundsDlgCB(int var){
  switch(var){
  case CLOSE_BOUNDS:
#ifndef pp_CLOSEOFF
    glui_bounds->hide();
    updatemenu = 1;
#endif
    break;
  case SAVE_SETTINGS_BOUNDS:
    WriteIni(LOCAL_INI, NULL);
    break;
  case COMPRESS_FILES:
    PRINTF("compressing\n");
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIImmersedBoundCB ------------------------ */

#define SHOW_POLYGON_EDGES 0
#define SHOW_TRIANGLE_EDGES 1
#define HIDE_EDGES 2

extern "C" void GLUIImmersedBoundCB(int var){
  int i;

  updatemenu = 1;
  switch(var){
  case IMMERSED_SWITCH_CELLTYPE:
    glui_show_vector_slice   = show_vector_slice[slice_celltype];
    glui_slice_edgetype      = slice_edgetypes[slice_celltype];
    glui_show_slice_shaded   = show_slice_shaded[slice_celltype];
    glui_show_slice_outlines = show_slice_outlines[slice_celltype];
    glui_show_slice_points   = show_slice_points[slice_celltype];
    glui_show_slice_values   = show_slice_values[slice_celltype];
    for(i=0;i<3;i++){
      switch(slice_edgetypes[i]){
        case GEOM_OUTLINE_POLYGON:
        case GEOM_OUTLINE_TRIANGLE:
          show_slice_outlines[i]=1;
          break;
        case GEOM_OUTLINE_HIDDEN:
          show_slice_outlines[i]=0;
          break;
	      default:
	        assert(FFALSE);
	        break;
      }
    }
    if(RADIO_slice_edgetype!=NULL)RADIO_slice_edgetype->set_int_val(glui_slice_edgetype);
    if(CHECKBOX_show_vector_slice!=NULL)CHECKBOX_show_vector_slice->set_int_val(glui_show_vector_slice);
    if(CHECKBOX_show_slice_shaded!=NULL)CHECKBOX_show_slice_shaded->set_int_val(glui_show_slice_shaded);
    if(CHECKBOX_show_slice_outlines!=NULL)CHECKBOX_show_slice_outlines->set_int_val(glui_show_slice_outlines);
    if(CHECKBOX_show_slice_points!=NULL)CHECKBOX_show_slice_points->set_int_val(glui_show_slice_points);
    if(CHECKBOX_show_slice_values!=NULL)CHECKBOX_show_slice_values->set_int_val(glui_show_slice_values);

    break;
  case SHOW_ALL_SLICE_VALUES:
    show_slice_values[0] = 1;
    show_slice_values[1] = 1;
    show_slice_values[2] = 1;
    show_slice_values_all_regions = 1;
    GLUIImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
    break;
  case HIDE_ALL_SLICE_VALUES:
    show_slice_values[0] = 0;
    show_slice_values[1] = 0;
    show_slice_values[2] = 0;
    show_slice_values_all_regions = 0;
    GLUIImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
    break;
  case IMMERSED_SET_DRAWTYPE:
    if(glui_show_slice_outlines == 0){
      glui_slice_edgetype = GEOM_OUTLINE_HIDDEN;
    }
    else{
      if(glui_slice_edgetype == GEOM_OUTLINE_HIDDEN)glui_slice_edgetype = GEOM_OUTLINE_TRIANGLE;
    }
    show_vector_slice[slice_celltype]   = glui_show_vector_slice;
    slice_edgetypes[slice_celltype]     = glui_slice_edgetype;
    show_slice_shaded[slice_celltype]   = glui_show_slice_shaded;
    show_slice_outlines[slice_celltype] = glui_show_slice_outlines;
    show_slice_points[slice_celltype]   = glui_show_slice_points;
    show_slice_values[slice_celltype]   = glui_show_slice_values;
    if(RADIO_slice_edgetype!=NULL)RADIO_slice_edgetype->set_int_val(glui_slice_edgetype);
    SliceInObstDialog2Menu();
    break;
  case IMMERSED_SWITCH_EDGETYPE:
    switch(glui_slice_edgetype){
    case SHOW_POLYGON_EDGES:
    case SHOW_TRIANGLE_EDGES:
      glui_show_slice_outlines=1;
      break;
    case HIDE_EDGES:
      glui_show_slice_outlines=0;
      break;
    default:
      assert(FFALSE);
      break;
    }
    GLUIImmersedBoundCB(IMMERSED_SET_DRAWTYPE);
    if(CHECKBOX_show_slice_outlines!=NULL)CHECKBOX_show_slice_outlines->set_int_val(glui_show_slice_outlines);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ BoundBoundCB ------------------------ */

extern "C" void BoundBoundCB(int var){
  int i;

  SNIFF_ERRORS("BoundBoundCB: start");
  switch(var){
  case SHOW_BOUNDARY_OUTLINE:
    if(global_scase.ngeom_data==0)break;
    if(show_boundary_outline==1&&boundary_edgetype==GEOM_OUTLINE_HIDDEN)boundary_edgetype = GEOM_OUTLINE_POLYGON;
    if(show_boundary_outline==0&&boundary_edgetype!=GEOM_OUTLINE_HIDDEN)boundary_edgetype = GEOM_OUTLINE_HIDDEN;
    if(boundary_edgetype!=RADIO_boundary_edgetype->get_int_val())RADIO_boundary_edgetype->set_int_val(boundary_edgetype);
    break;
  case BOUNDARY_EDGETYPE:
    if(boundary_edgetype==GEOM_OUTLINE_HIDDEN&&show_boundary_outline==1)show_boundary_outline=0;
    if(boundary_edgetype!=GEOM_OUTLINE_HIDDEN&&show_boundary_outline==0)show_boundary_outline=1;
    if(show_boundary_outline!=CHECKBOX_show_boundary_outline->get_int_val())CHECKBOX_show_boundary_outline->set_int_val(show_boundary_outline);
    break;
  case UPDATE_BOUNDARYSLICEDUPS:
    UpdateBoundarySliceDups();
    updatemenu = 1;
    break;
  case SHOW_ALL_INTERIOR_PATCH_DATA:
    hide_all_interior_patch_data = 1 - hide_all_interior_patch_data;
    show_all_interior_patch_data = 1 - hide_all_interior_patch_data;
    ShowBoundaryMenu(INTERIOR_WALL_MENU);
    break;
  case HIDE_ALL_INTERIOR_PATCH_DATA:
    show_all_interior_patch_data = 1 - show_all_interior_patch_data;
    hide_all_interior_patch_data = 1 - show_all_interior_patch_data;
    ShowBoundaryMenu(INTERIOR_WALL_MENU);
    break;
  case SHOW_ALL_EXTERIOR_PATCH_DATA:
    if(show_all_exterior_patch_data==1){
      ShowBoundaryMenu(SHOW_EXTERIOR_WALL_MENU);
      for(i=1; i<7; i++){
        if(CHECKBOX_show_exterior_walls[i]!=NULL)CHECKBOX_show_exterior_walls[i]->set_int_val(vis_boundary_type[i]);
      }
      if(hide_all_exterior_patch_data == 1){
        hide_all_exterior_patch_data = 0;
        if(CHECKBOX_hide_all_exterior_patch_data!=NULL)CHECKBOX_hide_all_exterior_patch_data->set_int_val(0);
      }
    }
    else{
      ShowBoundaryMenu(HIDE_EXTERIOR_WALL_MENU);
      for(i = 1; i < 7; i++){
        if(CHECKBOX_show_exterior_walls[i]!=NULL)CHECKBOX_show_exterior_walls[i]->set_int_val(vis_boundary_type[i]);
      }
      if(CHECKBOX_hide_all_exterior_patch_data!=NULL)CHECKBOX_hide_all_exterior_patch_data->set_int_val(1);
    }
    updatemenu = 1;
    break;
  case HIDE_ALL_EXTERIOR_PATCH_DATA:
    if(hide_all_exterior_patch_data==1){
      ShowBoundaryMenu(HIDE_EXTERIOR_WALL_MENU);
      for(i=1; i<7; i++){
        if(CHECKBOX_show_exterior_walls[i]!=NULL)CHECKBOX_show_exterior_walls[i]->set_int_val(vis_boundary_type[i]);
      }
      if(show_all_exterior_patch_data == 1){
        show_all_exterior_patch_data = 0;
        if(CHECKBOX_show_all_exterior_patch_data!=NULL)CHECKBOX_show_all_exterior_patch_data->set_int_val(0);
      }
    }
    else{
      ShowBoundaryMenu(SHOW_EXTERIOR_WALL_MENU);
      for(i = 1; i < 7; i++){
        if(CHECKBOX_show_exterior_walls[i]!=NULL)CHECKBOX_show_exterior_walls[i]->set_int_val(vis_boundary_type[i]);
      }
      if(CHECKBOX_show_all_exterior_patch_data!=NULL)CHECKBOX_show_all_exterior_patch_data->set_int_val(1);
    }
    break;
  case SHOW_EXTERIOR_PATCH_DATA:
    {
      int show_all_ext = 1;
      int hide_all_ext = 1;
      for(i=1; i<7; i++){
        if(vis_boundary_type[i] == 1)hide_all_ext = 0;
        if(vis_boundary_type[i] == 0)show_all_ext = 0;
        if(CHECKBOX_show_exterior_walls[i]!=NULL)CHECKBOX_show_exterior_walls[i]->set_int_val(vis_boundary_type[i]);
      }
      if(show_all_ext == 1){
        show_all_exterior_patch_data = 1;
        if(CHECKBOX_show_all_exterior_patch_data!=NULL)CHECKBOX_show_all_exterior_patch_data->set_int_val(1);
        if(CHECKBOX_hide_all_exterior_patch_data!=NULL)CHECKBOX_hide_all_exterior_patch_data->set_int_val(0);
        BoundBoundCB(SHOW_ALL_EXTERIOR_PATCH_DATA);
      }
      else if(hide_all_ext == 1){
        hide_all_exterior_patch_data = 1;
        if(CHECKBOX_show_all_exterior_patch_data!=NULL)CHECKBOX_show_all_exterior_patch_data->set_int_val(0);
        if(CHECKBOX_hide_all_exterior_patch_data!=NULL)CHECKBOX_hide_all_exterior_patch_data->set_int_val(1);
        BoundBoundCB(HIDE_ALL_EXTERIOR_PATCH_DATA);
      }
      else{
        for(i = 0;i < global_scase.npatchinfo;i++){
          patchdata *patchi;
          int n;

          patchi = global_scase.patchinfo + i;
          if(patchi->loaded == 0)continue;
          for(n = 0;n < patchi->npatches;n++){
            patchfacedata *pfi;
            int wall_index;

            pfi = patchi->patchfaceinfo + n;
            wall_index = pfi->type;
            if(wall_index>=1 && wall_index<=6){
              pfi->vis = vis_boundary_type[wall_index];
            }
          }
        }
      }
    }
    updatemenu = 1;
    updatefacelists = 1;
    break;
  case CACHE_DATA:
    if(PANEL_keep_bound_data !=NULL){
      if(cache_boundary_data==0){
        PANEL_keep_bound_data->disable();
      }
      else{
        PANEL_keep_bound_data->enable();
      }
    }
    break;
  case VALMAX:
  case VALMIN:
    if(patchlabellist!=NULL)GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    break;
  case HIDEPATCHSURFACE:
    updatefacelists = 1;
    break;
  case CHOPUPDATE:
    UpdateChopColors();
    break;
  case SETCHOPMINVAL:
    UpdateChopColors();
    if(patchlabellist != NULL)GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    switch(setpatchchopmin){
    case DISABLE:
      break;
    case ENABLE:
      break;
    default:
      assert(FFALSE);
      break;
    }
    updatefacelists = 1;
    break;
  case SETCHOPMAXVAL:
    UpdateChopColors();
    GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    switch(setpatchchopmax){
    case DISABLE:
      break;
    case ENABLE:
      break;
    default:
      assert(FFALSE);
      break;
    }
    updatefacelists = 1;
    break;
  case CHOPVALMIN:
    GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    UpdateChopColors();
    break;
  case CHOPVALMAX:
    GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    UpdateChopColors();
    break;
  case SHOWCHAR:
    if(CHECKBOX_showchar != NULL&&CHECKBOX_showonlychar != NULL){
      if(vis_threshold == 1){
        CHECKBOX_showonlychar->enable();
      }
      else{
        CHECKBOX_showonlychar->disable();
      }
    }
    updatemenu = 1;
    updatefacelists = 1;
    break;
  case FILETYPE_INDEX:
    GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index_old]);
    Global2GLUIBoundaryBounds(patchlabellist[list_patch_index]);

    BoundBoundCB(SETVALMIN);
    BoundBoundCB(SETVALMAX);

    list_patch_index_old = list_patch_index;
    updatefacelists = 1;
    break;
  case SETVALMIN:
    BoundBoundCB(FILE_UPDATE);
    break;
  case SETVALMAX:
    BoundBoundCB(FILE_UPDATE);
    break;
  case FILE_UPDATE:
    if(patchlabellist!=NULL)GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    BoundBoundCB(UPDATE_DATA_COLORS);
    break;
  case UPDATE_DATA_COLORS:
    GetGlobalPatchBounds(1,SET_MINMAX_FLAG,NULL);
    if(patchlabellist != NULL)Global2GLUIBoundaryBounds(patchlabellist[list_patch_index]);
    UpdateAllBoundaryColors(1);
    break;
  case FILE_RELOAD:
    if(global_scase.npatchinfo>0){
//      BoundBoundCB(FILE_UPDATE);
      for(i = 0;i < global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        LoadBoundaryMenu(i);
      }
    }
    break;
  case COMPRESS_FILES:
    if(compress_threads == NULL){
      compress_threads = THREADinit(&n_compress_threads, &use_compress_threads, Compress);
    }
    THREADrun(compress_threads);
    break;
  case COMPRESS_AUTOLOADED:
    updatemenu = 1;
    break;
  case OVERWRITE:
    if(overwrite_all == 1){
      CHECKBOX_erase_all->set_int_val(0);
    }
    updatemenu = 1;
    break;
  case ERASE:
    if(erase_all == 1){
      CHECKBOX_overwrite_all->set_int_val(0);
    }
    updatemenu = 1;
    break;
  case BOUND_STARTUP:
    BoundsDlgCB(SAVE_SETTINGS_BOUNDS);
    break;
  case SAVE_FILE_LIST:
    Set3DSmokeStartup();
    BoundsDlgCB(SAVE_SETTINGS_BOUNDS);
    break;
  case LOAD_FILES:
    LoadFiles();
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ CheckBounds ------------------------ */

void CheckBounds(int var){
  if((var==-1||var==0)&&use_meshclip[0] == 0){
    meshclip_save[0] = meshclip[0];
    meshclip[0] = xbar0FDS;
    SPINNER_meshclip[0]->set_float_val(meshclip[0]);
  }
  if((var == -1 || var == 1) && use_meshclip[1] == 0){
    meshclip_save[1] = meshclip[1];
    meshclip[1] = xbarFDS;
    SPINNER_meshclip[1]->set_float_val(meshclip[1]);
  }
  if((var == -1 || var == 2) && use_meshclip[2] == 0){
    meshclip_save[2] = meshclip[2];
    meshclip[2] = ybar0FDS;
    SPINNER_meshclip[2]->set_float_val(meshclip[2]);
  }
  if((var == -1 || var == 3) && use_meshclip[3] == 0){
    meshclip_save[3] = meshclip[3];
    meshclip[3] = ybarFDS;
    SPINNER_meshclip[3]->set_float_val(meshclip[3]);
  }
  if((var == -1 || var == 4) && use_meshclip[4] == 0){
    meshclip_save[4] = meshclip[4];
    meshclip[4] = zbar0FDS;
    SPINNER_meshclip[4]->set_float_val(meshclip[4]);
  }
  if((var == -1 || var == 5) && use_meshclip[5] == 0){
    meshclip_save[5] = meshclip[5];
    meshclip[5] = zbarFDS;
    SPINNER_meshclip[5]->set_float_val(meshclip[5]);
  }
}

/* ------------------ UpdateBoundaryFiles ------------------------ */

void UpdateBoundaryFiles(void){
  int i;

  for(i = 0;i < global_scase.npatchinfo;i++){
    patchdata *patchi;
    meshdata *meshi;

    patchi = global_scase.patchinfo + i;
    if(patchi->loaded == 0 || patchi->blocknumber < 0)continue;
    meshi = global_scase.meshescoll.meshinfo + patchi->blocknumber;
    if(meshi->use == 1 && patchi->display == 0){
      patchi->display = 1;
      updatefacelists = 1;
    }
    else if(meshi->use == 0 && patchi->display == 1){
      patchi->display = 0;
      updatefacelists = 1;
    }
  }
}

/* ------------------ TimeBoundCB ------------------------ */

void MeshBoundCB(int var){
  int i;

  GLUTPOSTREDISPLAY;
  switch(var){
  case USEMESH_SET_ALL:
    meshclip[0] = xbar0FDS;
    meshclip[1] = xbarFDS;
    meshclip[2] = ybar0FDS;
    meshclip[3] = ybarFDS;
    meshclip[4] = zbar0FDS;
    meshclip[5] = zbarFDS;
    for(i = 0;i < 6;i++){
      use_meshclip[i] = 1;
      CHECKBOX_use_meshclip[i]->set_int_val(use_meshclip[i]);
      SPINNER_meshclip[i]->set_float_val(meshclip[i]);
      SPINNER_meshclip[i]->enable();
    }
    MeshBoundCB(USEMESH_XYZ);
    break;
  case USEMESH_REMOVE_ALL:
    for(i = 0; i < global_scase.meshescoll.nmeshes; i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      meshi->use = 0;
    }
    break;
  case USEMESH_SET_ONE:
    {
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + set_mesh - 1;
      meshclip[0] = meshi->boxmin_fds[0];
      meshclip[2] = meshi->boxmin_fds[1];
      meshclip[4] = meshi->boxmin_fds[2];
      meshclip[1] = meshi->boxmax_fds[0];
      meshclip[3] = meshi->boxmax_fds[1];
      meshclip[5] = meshi->boxmax_fds[2];
      meshi->use = 1;
    }
    for(i = 0;i < 6;i++){
      use_meshclip[i] = 1;
      CHECKBOX_use_meshclip[i]->set_int_val(use_meshclip[i]);
      SPINNER_meshclip[i]->set_float_val(meshclip[i]);
      SPINNER_meshclip[i]->enable();
    }
    break;
  case USEMESH_REMOVE_ONE:
  {
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + set_mesh - 1;
    meshi->use = 0;
  }
  break;
  case USEMESH_DRAW_BOX:
    break;
  case USEMESH_LOAD_WHEN_LOADED:
    updatemenu = 1;
    break;
  case USEMESH_DRAW_MESH:
    break;
  case USEMESH_XYZ:
    updatemenu = 1;
    for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      meshi->use = 1;
    }
    if(use_meshclip[0] == 0 && use_meshclip[1] == 0 && use_meshclip[2] == 0 &&
       use_meshclip[3] == 0 && use_meshclip[4] == 0 && use_meshclip[5] == 0)break;
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      float meshclip_min[3],  meshclip_max[3];
      int use_meshclip_min[3], use_meshclip_max[3];

      meshi = global_scase.meshescoll.meshinfo + i;
      meshclip_min[0] = meshclip[0];
      meshclip_min[1] = meshclip[2];
      meshclip_min[2] = meshclip[4];
      meshclip_max[0] = meshclip[1];
      meshclip_max[1] = meshclip[3];
      meshclip_max[2] = meshclip[5];
      use_meshclip_min[0] = use_meshclip[0];
      use_meshclip_min[1] = use_meshclip[2];
      use_meshclip_min[2] = use_meshclip[4];
      use_meshclip_max[0] = use_meshclip[1];
      use_meshclip_max[1] = use_meshclip[3];
      use_meshclip_max[2] = use_meshclip[5];
      if(use_meshclip_min[0] == 0)meshclip_min[0] = xbar0FDS;
      if(use_meshclip_max[0] == 0)meshclip_max[0] = xbarFDS;
      if(use_meshclip_min[1] == 0)meshclip_min[1] = ybar0FDS;
      if(use_meshclip_max[1] == 0)meshclip_max[1] = ybarFDS;
      if(use_meshclip_min[2] == 0)meshclip_min[2] = zbar0FDS;
      if(use_meshclip_max[2] == 0)meshclip_max[2] = zbarFDS;
      if(glui_mesh_intersection_option == 0){
        if(use_meshclip_min[0] == 1 && meshclip_min[0] + MESH_EPS > meshi->boxmax_fds[0]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_max[0] == 1 && meshclip_max[0] - MESH_EPS < meshi->boxmin_fds[0]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_min[1] == 1 && meshclip_min[1] + MESH_EPS > meshi->boxmax_fds[1]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_max[1] == 1 && meshclip_max[1] - MESH_EPS < meshi->boxmin_fds[1]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_min[2] == 1 && meshclip_min[2] + MESH_EPS > meshi->boxmax_fds[2]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_max[2] == 1 && meshclip_max[2] - MESH_EPS < meshi->boxmin_fds[2]){
          meshi->use = 0;
          continue;
        }
      }
      else{
        if(use_meshclip_max[0] == 1 && meshclip_max[0] + MESH_EPS < meshi->boxmax_fds[0]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_min[0] == 1 && meshclip_min[0] - MESH_EPS > meshi->boxmin_fds[0]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_max[1] == 1 && meshclip_max[1] + MESH_EPS < meshi->boxmax_fds[1]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_min[1] == 1 && meshclip_min[1] - MESH_EPS > meshi->boxmin_fds[1]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_max[2] == 1 && meshclip_max[2] + MESH_EPS < meshi->boxmax_fds[2]){
          meshi->use = 0;
          continue;
        }
        if(use_meshclip_min[2] == 1 && meshclip_min[2] - MESH_EPS > meshi->boxmin_fds[2]){
          meshi->use = 0;
          continue;
        }
      }
    }
    UpdateBoundaryFiles();
    break;
  case USEMESH_USE_XYZ_ALL:
    for(i = 0;i < 6;i++){
      if(use_meshclip[i] == 1){
        SPINNER_meshclip[i]->enable();
      }
      else{
        SPINNER_meshclip[i]->disable();
      }
    }
    CheckBounds(-1);
    MeshBoundCB(USEMESH_XYZ);
    break;
  case USEMESH_USE_XYZ + 0:
  case USEMESH_USE_XYZ + 1:
  case USEMESH_USE_XYZ + 2:
  case USEMESH_USE_XYZ + 3:
  case USEMESH_USE_XYZ + 4:
  case USEMESH_USE_XYZ + 5:
    i = var - USEMESH_USE_XYZ;
    if(use_meshclip[i]==1){
      meshclip[i] = meshclip_save[i];
      SPINNER_meshclip[i]->set_float_val(meshclip[i]);
      SPINNER_meshclip[i]->enable();
    }
    else{
      SPINNER_meshclip[i]->disable();
    }
    CheckBounds(i);
    MeshBoundCB(USEMESH_XYZ);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateMeshBounds ------------------------ */

extern "C" void GLUIUpdateMeshBounds(void){
  int i;

  for(i = 0;i < 6;i++){
    SPINNER_meshclip[i]->set_float_val(meshclip[i]);
    CHECKBOX_use_meshclip[i]->set_int_val(use_meshclip[i]);
  }
  CHECKBOX_show_intersection_box->set_int_val(show_intersection_box);
  CHECKBOX_show_intersected_meshes->set_int_val(show_intersected_meshes);
  MeshBoundCB(USEMESH_XYZ);
  MeshBoundCB(USEMESH_USE_XYZ);
}

/* ------------------ TimeBoundCB ------------------------ */

void TimeBoundCB(int var){

  updatemenu = 1;
  switch(var){
  case SET_TIME:
    SetTimeVal(glui_time);
    break;
  case TBOUNDS_USE:
    GLUIUpdatePlot2DTbounds();
    if(use_tload_begin == 1){
      SPINNER_tload_begin->enable();
    }
    else{
      SPINNER_tload_begin->disable();
    }
    if(use_tload_end == 1){
      SPINNER_tload_end->enable();
    }
    else{
      SPINNER_tload_end->disable();
    }
    if(use_tload_skip == 1){
      SPINNER_tload_skip->enable();
    }
    else{
      SPINNER_tload_skip->disable();
    }
    GLUIUpdateTBounds();
    UpdateTimes();
    updatemenu = 1;
    break;
  case TBOUNDS:
    GLUIUpdatePlot2DTbounds();
    GLUIUpdateTBounds();
    UpdateTimes();
    updatemenu = 1;
    break;
  case SET_FDS_TIMES:
    use_tload_begin = 1;
    use_tload_end   = 1;
    global_scase.tload_begin     = global_scase.global_tbegin;
    global_scase.tload_end       = global_scase.global_tend;
    SPINNER_tload_begin->set_float_val(global_scase.tload_begin);
    SPINNER_tload_end->set_float_val(global_scase.tload_end);
    CHECKBOX_use_tload_begin->set_int_val(use_tload_begin);
    CHECKBOX_use_tload_end->set_int_val(use_tload_end);
    TimeBoundCB(TBOUNDS);
    TimeBoundCB(TBOUNDS_USE);
    break;
  case RELOAD_ALL_DATA:
    ReloadMenu(RELOAD_ALL_NOW);
    break;
  case RELOAD_INCREMENTAL_DATA:
    ReloadMenu(RELOAD_INCREMENTAL_NOW);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ ScriptCB ------------------------ */

void ScriptCB(int var){
  char label[1024];
  char *name;
  int id;
  int len, i;
  int set_renderlabel;

  switch(var){
  case SCRIPT_STEP_NOW:
    Keyboard('^', FROM_SMOKEVIEW);
    break;
  case SCRIPT_CANCEL_NOW:
    current_script_command = NULL;
    runscript = 0;
    first_frame_index = 0;
    script_startframe = -1;
    script_skipframe = -1;
    script_step = 0;
    GLUIScriptEnable();
    render_status = RENDER_OFF;
    break;
  case SCRIPT_RENDER_DIR:
    strcpy(label, script_renderdir);
    TrimBack(label);
    name = TrimFront(label);
    set_renderlabel = 0;
    if(name != NULL&&strlen(name) != strlen(script_renderdir)){
      strcpy(script_renderdir, name);
      set_renderlabel = 1;
    }
    name = script_renderdir;
    len = strlen(script_renderdir);
    if(len == 0)break;
    for(i = 0;i < len;i++){
#ifdef _WIN32
      if(name[i] == '/'){
        set_renderlabel = 1;
        name[i] = '\\';
      }
#else
      if(name[i] == '\\'){
        set_renderlabel = 1;
        name[i] = '/';
      }
#endif
    }
#ifdef _WIN32
    if(name[len - 1] != '\\'){
      set_renderlabel = 1;
      strcat(name, dirseparator);
    }
#else
    if(name[len - 1] != '/'){
      set_renderlabel = 1;
      strcat(name, dirseparator);
    }
#endif
    if(set_renderlabel == 1){
      EDIT_renderdir->set_text(script_renderdir);
    }
    break;
  case SCRIPT_RENDER:
    Keyboard('r', FROM_SMOKEVIEW);
    break;
  case SCRIPT_RENDER_SUFFIX:
  {
    char *suffix;

    TrimBack(script_renderfilesuffix);
    suffix = TrimFront(script_renderfilesuffix);
    strcpy(script_renderfile, "");
    if(strlen(suffix) > 0){
      strcpy(script_renderfile, global_scase.fdsprefix);
      strcat(script_renderfile, "_");
      strcat(script_renderfile, suffix);
      strcpy(label, "Render: ");
      strcat(label, script_renderfile);
    }
    else{
      strcpy(label, "Render");
    }
    BUTTON_script_render->set_name(label);
  }
  break;
  case SCRIPT_START:
    ScriptMenu(SCRIPT_START_RECORDING);
    break;
  case SCRIPT_STOP:
    ScriptMenu(SCRIPT_STOP_RECORDING);
    break;
  case SCRIPT_RUNSCRIPT:
    name = 5 + BUTTON_script_runscript->name;
    PRINTF("running script: %s\n", name);
    ScriptMenu(LIST_scriptlist->get_int_val());
    break;
  case SCRIPT_LIST:
    id = LIST_scriptlist->get_int_val();
    name = GetScriptFileName(id);
    if(name != NULL&&strlen(name) > 0){
      strcpy(label, "Run:");
      strcat(label, name);
      BUTTON_script_runscript->set_name(label);
    }
    break;
  case SCRIPT_SAVEINI:
    name = 5 + BUTTON_script_saveini->name;
    if(strlen(name) > 0){
      inifiledata *inifile;

      strcpy(script_filename, name);
      inifile = InsertIniFile(name);
      WriteIni(SCRIPT_INI, script_filename);
      if(inifile != NULL&&LIST_ini_list != NULL){
        LIST_ini_list->add_item(inifile->id, inifile->file);
      }
    }
    WriteIni(LOCAL_INI, NULL);
    break;
  case SCRIPT_LOADINI:
  {
    char *ini_filename;

    id = LIST_ini_list->get_int_val();
    ini_filename = GetIniFileName(id);
    if(ini_filename == NULL)break;
    char *caseini_filename = CasePathCaseIni(&global_scase);
    if(strcmp(ini_filename, caseini_filename) == 0){
      ReadIni(NULL);
    }
    else if(id >= 0){
      char *script_filename2;

      if(strlen(ini_filename) == 0) {
        FREEMEMORY(caseini_filename);
        break;
      }
      script_filename2 = script_filename;
      strcpy(script_filename, ini_filename);
      windowresized = 0;
      ReadIni(script_filename2);
    }
    if(scriptoutstream != NULL){
      fprintf(scriptoutstream, "LOADINIFILE\n");
      fprintf(scriptoutstream, " %s\n", ini_filename);
    }
    FREEMEMORY(caseini_filename);
  }
  break;
  case SCRIPT_STEP:
    GLUIUpdateScriptStep();
    updatemenu = 1;
    break;
  case SCRIPT_EDIT_INI:
    strcpy(label, "Save ");
    strcat(label, global_scase.fdsprefix);
    TrimBack(script_inifile_suffix);
    if(strlen(script_inifile_suffix) > 0){
      strcat(label, "_");
      strcat(label, script_inifile_suffix);
    }
    strcat(label, ".ini");
    BUTTON_script_saveini->set_name(label);
    break;
  case SCRIPT_SETSUFFIX:
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateBoundTbounds ------------------------ */

extern "C" void GLUIUpdateBoundTbounds(void){
  use_tload_end   = use_tload_end2;
  use_tload_begin = use_tload_begin2;
  global_scase.tload_end       = tload_end2;
  global_scase.tload_begin     = tload_begin2;
  if(CHECKBOX_use_tload_begin!=NULL)CHECKBOX_use_tload_begin->set_int_val(use_tload_begin);
  if(CHECKBOX_use_tload_end!=NULL)CHECKBOX_use_tload_end->set_int_val(use_tload_end);
  if(SPINNER_tload_end!=NULL)SPINNER_tload_end->set_float_val(global_scase.tload_end);
  if(SPINNER_tload_begin!=NULL)SPINNER_tload_begin->set_float_val(global_scase.tload_begin);
}

/* ------------------ GLUIUpdateSliceXYZ ------------------------ */

extern "C" void GLUIUpdateSliceXYZ(void){
  if(SPINNER_slice_x!=NULL)SPINNER_slice_x->set_float_val(slice_xyz[0]);
  if(SPINNER_slice_y!=NULL)SPINNER_slice_y->set_float_val(slice_xyz[1]);
  if(SPINNER_slice_z!=NULL)SPINNER_slice_z->set_float_val(slice_xyz[2]);
}

/* ------------------ AddColorbarListBound ------------------------ */

void AddColorbarListBound(GLUI_Listbox *LIST_cbar, int index, char *label_arg, int *max_index){
  char cbar_type[256];
  int i, nitems = 0;


  for(i = 0; i < colorbars.ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + i;
    if(strcmp(cbi->colorbar_type, label_arg) != 0)continue;
    nitems++;
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

#define SLICE_LOADALL   1
#define SLICE_UNLOADALL 2

/* ------------------ SliceLoadCB ------------------------ */

void LoadAllMultiSliceMenu(void);
void LoadAllMultiVSliceMenu(void);

void SliceLoadCB(int var){
  switch(var){
  case SLICE_LOADALL:
    if(sliceload_isvector==1){
      LoadAllMultiVSliceMenu();
    }
    else{
      LoadAllMultiSliceMenu();
    }
    break;
  case SLICE_UNLOADALL:
    UnloadSliceMenu(UNLOAD_ALL);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateColorbarListBound ------------------------ */

extern "C" void GLUIUpdateColorbarListBound(int flag){
  int i;
  char label[64];
  GLUI_Listbox *LIST_cb=NULL;

  switch(flag){
    case 1:
     LIST_cb = LISTBOX_cb_bound;
     break;
    case 2:
     LIST_cb = LISTBOX_cb_toggle_bound1;
     break;
    case 3:
     LIST_cb = LISTBOX_cb_toggle_bound2;
     break;
    default:
     assert(FFALSE);
     break;
  }

  if(LIST_cb == NULL)return;
  for(i=-7;i<colorbars.ncolorbars;i++){
   LIST_cb->delete_item(i);
  }
  strcpy(label, "rainbow");      AddColorbarListBound(LIST_cb, -1, label, &max_LISTBOX_cb_bound);
  strcpy(label, "original");     AddColorbarListBound(LIST_cb, -2, label, &max_LISTBOX_cb_bound);
  strcpy(label, "linear");       AddColorbarListBound(LIST_cb, -3, label, &max_LISTBOX_cb_bound);
  strcpy(label, "divergent");    AddColorbarListBound(LIST_cb, -4, label, &max_LISTBOX_cb_bound);
  strcpy(label, "circular");     AddColorbarListBound(LIST_cb, -5, label, &max_LISTBOX_cb_bound);
  strcpy(label, "deprecated");   AddColorbarListBound(LIST_cb, -6, label, &max_LISTBOX_cb_bound);
  strcpy(label, "user defined"); AddColorbarListBound(LIST_cb, -7, label, &max_LISTBOX_cb_bound);
}

/* ------------------ GLUIShowHideGeomDataCB ------------------------ */

#define SHOW_ALL_MESH_GEOM 0
#define HIDE_ALL_MESH_GEOM 1
#define SHOW_ALL_MESH_DATA 2
#define HIDE_ALL_MESH_DATA 3

void GLUIShowHideGeomDataCB(int var){
  int i;
  int nn=MIN(global_scase.meshescoll.nmeshes,256);

  switch (var){
    case SHOW_ALL_MESH_GEOM:
      for(i=0;i<nn;i++){
        CHECKBOX_show_mesh_geom[i]->set_int_val(1);
      }
      break;
    case HIDE_ALL_MESH_GEOM:
      for(i=0;i<nn;i++){
        CHECKBOX_show_mesh_geom[i]->set_int_val(0);
      }
      break;
    case SHOW_ALL_MESH_DATA:
      for(i=0;i<nn;i++){
        CHECKBOX_show_mesh_data[i]->set_int_val(1);
      }
      break;
    case HIDE_ALL_MESH_DATA:
      for(i=0;i<nn;i++){
        CHECKBOX_show_mesh_data[i]->set_int_val(0);
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ AddMeshCheckbox ------------------------ */

void AddMeshCheckbox(int icol,int nm, GLUI_Panel *PANEL, GLUI_Checkbox **CHECKBOX, int option){
  if(icol<=nm){
    int i;

    for(i=icol-1;i<nm;i+=8){
      meshdata *meshi;
      char label[340];

      meshi = global_scase.meshescoll.meshinfo + i;
      sprintf(label, "%i", i + 1);
      if(option == 1){
        CHECKBOX[i] = glui_bounds->add_checkbox_to_panel(PANEL, label, &meshi->blockvis);
      }
      else{
        CHECKBOX[i] = glui_bounds->add_checkbox_to_panel(PANEL, label, &meshi->datavis);
      }
    }
    if(icol!=MIN(8,nm)){
      glui_bounds->add_column_to_panel(PANEL, false);
    }
  }
}

/* ------------------ GLUIBoundsSetup ------------------------ */

extern "C" void GLUIBoundsSetup(int main_window){
  int i;
  int have_part;

  if(glui_bounds!=NULL){
    glui_bounds->close();
    glui_bounds=NULL;
  }
  overwrite_all=0;
  glui_bounds = GLUI_Master.create_glui( "Files/Data/Coloring",0,dialogX0,dialogY0);
  glui_bounds->hide();

  ROLLOUT_files = glui_bounds->add_rollout("Files", false, FILE_ROLLOUT, FileDataColRolloutCB);
  TOGGLE_ROLLOUT(filedatacolprocinfo, nfiledatacolprocinfo, ROLLOUT_files, FILE_ROLLOUT, glui_bounds);

  // -------------- Show/Hide Loaded files -------------------

  if(global_scase.npartinfo > 0 || global_scase.slicecoll.nsliceinfo > 0 || global_scase.slicecoll.nvsliceinfo > 0 || global_scase.nisoinfo > 0 || global_scase.npatchinfo || global_scase.smoke3dcoll.nsmoke3dinfo > 0 || global_scase.nplot3dinfo > 0){
    ROLLOUT_showhide = glui_bounds->add_rollout_to_panel(ROLLOUT_files,"Show/Hide", false, SHOWHIDE_ROLLOUT, FileRolloutCB);
    TOGGLE_ROLLOUT(fileprocinfo, nfileprocinfo, ROLLOUT_showhide, SHOWHIDE_ROLLOUT, glui_bounds);

    RADIO_showhide = glui_bounds->add_radiogroup_to_panel(ROLLOUT_showhide, &showhide_option);
    glui_bounds->add_radiobutton_to_group(RADIO_showhide, "Show");
    glui_bounds->add_radiobutton_to_group(RADIO_showhide, "Show only");
    glui_bounds->add_radiobutton_to_group(RADIO_showhide, "Hide");

    glui_bounds->add_column_to_panel(ROLLOUT_showhide, false);

    if(global_scase.npartinfo > 0)BUTTON_PART = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Particle", FILESHOW_particle, FileShowCB);
    if(global_scase.slicecoll.nsliceinfo > 0)BUTTON_SLICE = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Slice", FILESHOW_slice, FileShowCB);
    if(global_scase.slicecoll.nvsliceinfo > 0)BUTTON_VSLICE = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Vector", FILESHOW_vslice, FileShowCB);
    if(global_scase.nisoinfo > 0)BUTTON_ISO = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Isosurface", FILESHOW_isosurface, FileShowCB);
    if(global_scase.npatchinfo > 0)BUTTON_BOUNDARY = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Boundary", FILESHOW_boundary, FileShowCB);
    if(global_scase.smoke3dcoll.nsmoke3dinfo > 0)BUTTON_3DSMOKE = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "3D smoke/fire", FILESHOW_3dsmoke, FileShowCB);
    if(global_scase.nplot3dinfo > 0)BUTTON_PLOT3D = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Plot3D", FILESHOW_plot3d, FileShowCB);
    glui_bounds->add_button_to_panel(ROLLOUT_showhide, "File Sizes", FILESHOW_sizes, FileShowCB);


    GLUIUpdateShowHideButtons();
  }

  if(smokezippath != NULL && (global_scase.npatchinfo > 0 || global_scase.smoke3dcoll.nsmoke3dinfo > 0 || global_scase.slicecoll.nsliceinfo > 0)){
    ROLLOUT_compress = glui_bounds->add_rollout_to_panel(ROLLOUT_files, "Compress", false, COMPRESS_ROLLOUT, FileRolloutCB);
    TOGGLE_ROLLOUT(fileprocinfo, nfileprocinfo, ROLLOUT_compress, COMPRESS_ROLLOUT, glui_bounds);

    CHECKBOX_erase_all = glui_bounds->add_checkbox_to_panel(ROLLOUT_compress, "Erase compressed files",
      &erase_all, ERASE, BoundBoundCB);
    CHECKBOX_overwrite_all = glui_bounds->add_checkbox_to_panel(ROLLOUT_compress, "Overwrite compressed files",
      &overwrite_all, OVERWRITE, BoundBoundCB);
    CHECKBOX_compress_autoloaded = glui_bounds->add_checkbox_to_panel(ROLLOUT_compress, "Compress only autoloaded files",
      &compress_autoloaded, COMPRESS_AUTOLOADED, BoundBoundCB);
    SPINNER_zipstep = glui_bounds->add_spinner_to_panel(ROLLOUT_compress, "Frame Skip", GLUI_SPINNER_INT, &tload_zipskip,
        FRAMELOADING, GLUISliceBoundCB);
    SPINNER_zipstep->set_int_limits(0, 100);
    BUTTON_compress = glui_bounds->add_button_to_panel(ROLLOUT_compress, "Run smokezip", COMPRESS_FILES, BoundBoundCB);
  }

  ROLLOUT_script = glui_bounds->add_rollout_to_panel(ROLLOUT_files, "Scripts", false, SCRIPT_ROLLOUT, FileRolloutCB);
  TOGGLE_ROLLOUT(fileprocinfo, nfileprocinfo, ROLLOUT_script, SCRIPT_ROLLOUT, glui_bounds);

  PANEL_script1 = glui_bounds->add_panel_to_panel(ROLLOUT_script, "Script files", false);
  PANEL_record = glui_bounds->add_panel_to_panel(PANEL_script1, "Record", true);

  PANEL_script1a = glui_bounds->add_panel_to_panel(PANEL_record, "", false);
  BUTTON_script_start = glui_bounds->add_button_to_panel(PANEL_script1a, "Start", SCRIPT_START, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script1a, false);
  BUTTON_script_stop = glui_bounds->add_button_to_panel(PANEL_script1a, "Stop", SCRIPT_STOP, ScriptCB);
  BUTTON_script_stop->disable();

  PANEL_run = glui_bounds->add_panel_to_panel(PANEL_script1, "Run", true);
  PANEL_script1b = glui_bounds->add_panel_to_panel(PANEL_run, "", false);
  BUTTON_script_runscript = glui_bounds->add_button_to_panel(PANEL_script1b, "Run script", SCRIPT_RUNSCRIPT, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script1b, false);
  CHECKBOX_script_step = glui_bounds->add_checkbox_to_panel(PANEL_run, "Step through script", &script_step, SCRIPT_STEP, ScriptCB);
  BUTTON_step = glui_bounds->add_button_to_panel(PANEL_run, "Next", SCRIPT_STEP_NOW, ScriptCB);
  GLUIUpdateScriptStep();
  glui_bounds->add_button_to_panel(PANEL_run, "Cancel script", SCRIPT_CANCEL_NOW, ScriptCB);

  LIST_scriptlist = glui_bounds->add_listbox_to_panel(PANEL_script1b, "Select:", &script_index, SCRIPT_LIST, ScriptCB);
  {
    scriptfiledata *scriptfile;

    for(scriptfile = first_scriptfile.next; scriptfile->next != NULL; scriptfile = scriptfile->next){
      char *file;
      int len;

      file = scriptfile->file;
      if(file == NULL)continue;
      if(FILE_EXISTS(file) == NO)continue;
      len = strlen(file);
      if(len <= 0)continue;

      LIST_scriptlist->add_item(scriptfile->id, file);
    }
    ScriptCB(SCRIPT_LIST);
  }

  ROLLOUT_config = glui_bounds->add_rollout_to_panel(ROLLOUT_files, "Config", false, CONFIG_ROLLOUT, FileRolloutCB);
  TOGGLE_ROLLOUT(fileprocinfo, nfileprocinfo, ROLLOUT_config, CONFIG_ROLLOUT, glui_bounds);

  PANEL_script2a = glui_bounds->add_panel_to_panel(ROLLOUT_config, "", false);
  EDIT_ini = glui_bounds->add_edittext_to_panel(PANEL_script2a, "suffix:", GLUI_EDITTEXT_TEXT, script_inifile_suffix, SCRIPT_EDIT_INI, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script2a, false);
  BUTTON_script_setsuffix = glui_bounds->add_button_to_panel(PANEL_script2a, "Set", SCRIPT_SETSUFFIX, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script2a, false);
  BUTTON_script_saveini = glui_bounds->add_button_to_panel(PANEL_script2a, "Save:", SCRIPT_SAVEINI, ScriptCB);
  ScriptCB(SCRIPT_EDIT_INI);

  PANEL_script2b = glui_bounds->add_panel_to_panel(ROLLOUT_config, "", false);
  ini_index = -2;
  LIST_ini_list = glui_bounds->add_listbox_to_panel(PANEL_script2b, "Select:", &ini_index);
  {
    inifiledata *inifile;

    for(inifile = first_inifile.next; inifile->next != NULL; inifile = inifile->next){
      if(inifile->file != NULL && FILE_EXISTS(inifile->file) == YES){
        if(ini_index == -2)ini_index = inifile->id;
        LIST_ini_list->add_item(inifile->id, inifile->file);
      }
    }
  }
  glui_bounds->add_column_to_panel(PANEL_script2b, false);
  BUTTON_ini_load = glui_bounds->add_button_to_panel(PANEL_script2b, "Load", SCRIPT_LOADINI, ScriptCB);

  PANEL_script3 = glui_bounds->add_panel_to_panel(ROLLOUT_script, "Render", true);
  EDIT_renderdir = glui_bounds->add_edittext_to_panel(PANEL_script3, "directory:",
    GLUI_EDITTEXT_TEXT, script_renderdir, SCRIPT_RENDER_DIR, ScriptCB);
  EDIT_renderdir->set_w(260);
  PANEL_script1c = glui_bounds->add_panel_to_panel(PANEL_script3, "", false);
  BUTTON_script_render = glui_bounds->add_button_to_panel(PANEL_script1c, "Render", SCRIPT_RENDER, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script1c, false);
  EDIT_rendersuffix = glui_bounds->add_edittext_to_panel(PANEL_script1c, "suffix:",
    GLUI_EDITTEXT_TEXT, script_renderfilesuffix, SCRIPT_RENDER_SUFFIX, ScriptCB);
  EDIT_rendersuffix->set_w(130);
  ScriptCB(SCRIPT_RENDER_SUFFIX);

  // ----------------------------------- Bounds ----------------------------------------

  ROLLOUT_filebounds = glui_bounds->add_rollout("Data", false, DATA_ROLLOUT, FileDataColRolloutCB);
  TOGGLE_ROLLOUT(filedatacolprocinfo, nfiledatacolprocinfo, ROLLOUT_filebounds, DATA_ROLLOUT, glui_bounds);

  /*  zone (cfast) */

  if(global_scase.nzoneinfo > 0){
    ROLLOUT_zone_bound = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Zone/slice temperatures", false, ZONE_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_zone_bound, ZONE_ROLLOUT, glui_bounds);

    PANEL_zone_a = glui_bounds->add_panel_to_panel(ROLLOUT_zone_bound, "", GLUI_PANEL_NONE);

    EDIT_zone_min = glui_bounds->add_edittext_to_panel(PANEL_zone_a, "", GLUI_EDITTEXT_FLOAT, &zonemin, ZONEVALMINMAX, GLUISliceBoundCB);
    glui_bounds->add_column_to_panel(PANEL_zone_a, false);

    RADIO_zone_setmin = glui_bounds->add_radiogroup_to_panel(PANEL_zone_a, &setzonemin, SETZONEVALMINMAX, GLUISliceBoundCB);
    RADIOBUTTON_zone_permin = glui_bounds->add_radiobutton_to_group(RADIO_zone_setmin, "percentile min");
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmin, "set min");
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmin, "global min");

    PANEL_zone_b = glui_bounds->add_panel_to_panel(ROLLOUT_zone_bound, "", GLUI_PANEL_NONE);

    EDIT_zone_max = glui_bounds->add_edittext_to_panel(PANEL_zone_b, "", GLUI_EDITTEXT_FLOAT, &zonemax, ZONEVALMINMAX, GLUISliceBoundCB);
    glui_bounds->add_column_to_panel(PANEL_zone_b, false);

    RADIO_zone_setmax = glui_bounds->add_radiogroup_to_panel(PANEL_zone_b, &setzonemax, SETZONEVALMINMAX, GLUISliceBoundCB);
    RADIOBUTTON_zone_permax = glui_bounds->add_radiobutton_to_group(RADIO_zone_setmax, "percentile max");
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmax, "set max");
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmax, "global max");

    RADIOBUTTON_zone_permin->disable();
    RADIOBUTTON_zone_permax->disable();
    GLUISliceBoundCB(SETZONEVALMINMAX);
  }

  // ----------------------------------- 3D smoke ----------------------------------------


  if(global_scase.smoke3dcoll.nsmoke3dinfo > 0 || nvolrenderinfo > 0){
    ROLLOUT_smoke3d = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "3D smoke", false, SMOKE3D_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_smoke3d, SMOKE3D_ROLLOUT, glui_bounds);
  }

  // ----------------------------------- Boundary ----------------------------------------

  if(global_scase.npatchinfo > 0){
    glui_active = 1;
    ROLLOUT_bound = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Boundary", false, BOUNDARY_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_bound, BOUNDARY_ROLLOUT, glui_bounds);

    patchboundsCPP.setup("boundary", ROLLOUT_bound, patchbounds_cpp, npatchbounds_cpp, &cache_boundary_data, SHOW_CACHE_CHECKBOX, GLUIPatchBoundsCPP_CB,
      SubBoundRolloutCB, subboundprocinfo, &nsubboundprocinfo);

    ROLLOUT_outputpatchdata = glui_bounds->add_rollout_to_panel(ROLLOUT_bound, "Output data", false,
      BOUNDARY_OUTPUT_ROLLOUT, SubBoundRolloutCB);
    TOGGLE_ROLLOUT(subboundprocinfo, nsubboundprocinfo, ROLLOUT_outputpatchdata, BOUNDARY_OUTPUT_ROLLOUT, glui_bounds);

    glui_bounds->add_checkbox_to_panel(ROLLOUT_outputpatchdata, "Output data to file", &output_patchdata);

    PANEL_outputpatchdata = glui_bounds->add_panel_to_panel(ROLLOUT_outputpatchdata, "", GLUI_PANEL_NONE);

    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "tmin", GLUI_SPINNER_FLOAT, &patchout_tmin);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "xmin", GLUI_SPINNER_FLOAT, &patchout_xmin);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "ymin", GLUI_SPINNER_FLOAT, &patchout_ymin);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "zmin", GLUI_SPINNER_FLOAT, &patchout_zmin);

    glui_bounds->add_column_to_panel(PANEL_outputpatchdata, false);

    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "tmax", GLUI_SPINNER_FLOAT, &patchout_tmax);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "xmax", GLUI_SPINNER_FLOAT, &patchout_xmax);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "ymax", GLUI_SPINNER_FLOAT, &patchout_ymax);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata, "zmax", GLUI_SPINNER_FLOAT, &patchout_zmax);

    ROLLOUT_boundary_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_bound, "Settings", false, BOUNDARY_SETTINGS_ROLLOUT, SubBoundRolloutCB);
    TOGGLE_ROLLOUT(subboundprocinfo, nsubboundprocinfo, ROLLOUT_boundary_settings, BOUNDARY_SETTINGS_ROLLOUT, glui_bounds);

    if(global_scase.ngeom_data > 0){
      glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, "shaded", &show_boundary_shaded);
      CHECKBOX_show_boundary_outline = glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, "outline", &show_boundary_outline, SHOW_BOUNDARY_OUTLINE, BoundBoundCB);
      glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, "points", &show_boundary_points);

      PANEL_boundary_outline_type = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings, "outline type");
      RADIO_boundary_edgetype = glui_bounds->add_radiogroup_to_panel(PANEL_boundary_outline_type, &boundary_edgetype, BOUNDARY_EDGETYPE, BoundBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_boundary_edgetype, "polygon");
      glui_bounds->add_radiobutton_to_group(RADIO_boundary_edgetype, "triangle");
      glui_bounds->add_radiobutton_to_group(RADIO_boundary_edgetype, "none");
      BoundBoundCB(BOUNDARY_EDGETYPE);
      BoundBoundCB(SHOW_BOUNDARY_OUTLINE);

      glui_bounds->add_spinner_to_panel(ROLLOUT_boundary_settings, "line width", GLUI_SPINNER_FLOAT, &geomboundary_linewidth);
      glui_bounds->add_spinner_to_panel(ROLLOUT_boundary_settings, "point size", GLUI_SPINNER_FLOAT, &geomboundary_pointsize);
      glui_bounds->add_separator_to_panel(ROLLOUT_boundary_settings);
    }
    if(activate_threshold == 1){
      PANEL_boundary_temp_threshold = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings, "Temperature threshold");
      CHECKBOX_showchar = glui_bounds->add_checkbox_to_panel(PANEL_boundary_temp_threshold, "Show", &vis_threshold, SHOWCHAR, BoundBoundCB);
      CHECKBOX_showonlychar = glui_bounds->add_checkbox_to_panel(PANEL_boundary_temp_threshold, "Show only threshold", &vis_onlythreshold, SHOWCHAR, BoundBoundCB);
      {
        char label[256];

        strcpy(label, "Temperature (");
        strcat(label, (const char *)degC);
        strcat(label, ") ");
        glui_bounds->add_spinner_to_panel(PANEL_boundary_temp_threshold, label, GLUI_SPINNER_FLOAT, &temp_threshold);
      }
      BoundBoundCB(SHOWCHAR);
    }

    PANEL_boundary_exterior_data = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings, "exterior data");
    CHECKBOX_show_all_exterior_patch_data = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "Show all", &show_all_exterior_patch_data, SHOW_ALL_EXTERIOR_PATCH_DATA, BoundBoundCB);
    CHECKBOX_show_exterior_walls[LEFTwall] = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "left wall", vis_boundary_type + LEFTwall, SHOW_EXTERIOR_PATCH_DATA, BoundBoundCB);
    CHECKBOX_show_exterior_walls[FRONTwall] = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "front wall", vis_boundary_type + FRONTwall, SHOW_EXTERIOR_PATCH_DATA, BoundBoundCB);
    CHECKBOX_show_exterior_walls[DOWNwall] = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "lower wall", vis_boundary_type + DOWNwall, SHOW_EXTERIOR_PATCH_DATA, BoundBoundCB);
    glui_bounds->add_column_to_panel(PANEL_boundary_exterior_data, false);

    CHECKBOX_hide_all_exterior_patch_data = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "Hide all", &hide_all_exterior_patch_data, HIDE_ALL_EXTERIOR_PATCH_DATA, BoundBoundCB);
    CHECKBOX_show_exterior_walls[RIGHTwall] = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "right wall", vis_boundary_type + RIGHTwall, SHOW_EXTERIOR_PATCH_DATA, BoundBoundCB);
    CHECKBOX_show_exterior_walls[BACKwall] = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "back wall", vis_boundary_type + BACKwall, SHOW_EXTERIOR_PATCH_DATA, BoundBoundCB);
    CHECKBOX_show_exterior_walls[UPwall] = glui_bounds->add_checkbox_to_panel(PANEL_boundary_exterior_data, "upper wall", vis_boundary_type + UPwall, SHOW_EXTERIOR_PATCH_DATA, BoundBoundCB);

    PANEL_boundary_interior_data = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings, "interior data");
    CHECKBOX_show_all_interior_patch_data = glui_bounds->add_checkbox_to_panel(PANEL_boundary_interior_data, "Show all", &hide_all_interior_patch_data, SHOW_ALL_INTERIOR_PATCH_DATA, BoundBoundCB);
    glui_bounds->add_column_to_panel(PANEL_boundary_interior_data, false);
    CHECKBOX_hide_all_interior_patch_data = glui_bounds->add_checkbox_to_panel(PANEL_boundary_interior_data, "Hide all", &show_all_interior_patch_data, HIDE_ALL_INTERIOR_PATCH_DATA, BoundBoundCB);

    glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, "output patch info when loading", &outout_patch_faces);

    if(nboundaryslicedups > 0){
      ROLLOUT_boundary_duplicates = glui_bounds->add_rollout_to_panel(ROLLOUT_bound, "Duplicates", false, BOUNDARY_DUPLICATE_ROLLOUT, SubBoundRolloutCB);
      TOGGLE_ROLLOUT(subboundprocinfo, nsubboundprocinfo, ROLLOUT_boundary_duplicates, BOUNDARY_DUPLICATE_ROLLOUT, glui_bounds);

      RADIO_boundaryslicedup = glui_bounds->add_radiogroup_to_panel(ROLLOUT_boundary_duplicates, &boundaryslicedup_option, UPDATE_BOUNDARYSLICEDUPS, BoundBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_boundaryslicedup, "Keep all");
      glui_bounds->add_radiobutton_to_group(RADIO_boundaryslicedup, "Keep fine");
      glui_bounds->add_radiobutton_to_group(RADIO_boundaryslicedup, "Keep coarse");
    }
  }

  // ----------------------------------- Isosurface ----------------------------------------

  if(global_scase.nisoinfo > 0){
    ROLLOUT_iso = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Isosurface", false, ISO_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_iso, ISO_ROLLOUT, glui_bounds);

    if(niso_bounds > 0){
      ROLLOUT_iso_bounds = glui_bounds->add_rollout_to_panel(ROLLOUT_iso, "Bound data", true, ISO_ROLLOUT_BOUNDS, IsoRolloutCB);
      TOGGLE_ROLLOUT(isoprocinfo, nisoprocinfo, ROLLOUT_iso_bounds, ISO_ROLLOUT_BOUNDS, glui_bounds);

      PANEL_iso1 = glui_bounds->add_panel_to_panel(ROLLOUT_iso_bounds, "", GLUI_PANEL_NONE);
      EDIT_iso_valmin = glui_bounds->add_edittext_to_panel(PANEL_iso1, "", GLUI_EDITTEXT_FLOAT, &glui_iso_valmin, ISO_VALMIN, GLUIIsoBoundCB);
      glui_bounds->add_column_to_panel(PANEL_iso1, false);

      RADIO_iso_setmin = glui_bounds->add_radiogroup_to_panel(PANEL_iso1, &setisomin, ISO_SETVALMIN, GLUIIsoBoundCB);
      RADIOBUTTON_iso_percentile_min = glui_bounds->add_radiobutton_to_group(RADIO_iso_setmin, "percentile min");
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmin, "set min");
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmin, "global min");
      GLUIIsoBoundCB(ISO_SETVALMIN);
      RADIOBUTTON_iso_percentile_min->disable();

      PANEL_iso2 = glui_bounds->add_panel_to_panel(ROLLOUT_iso_bounds, "", GLUI_PANEL_NONE);
      EDIT_iso_valmax = glui_bounds->add_edittext_to_panel(PANEL_iso2, "", GLUI_EDITTEXT_FLOAT, &glui_iso_valmax, ISO_VALMAX, GLUIIsoBoundCB);
      glui_bounds->add_column_to_panel(PANEL_iso2, false);

      RADIO_iso_setmax = glui_bounds->add_radiogroup_to_panel(PANEL_iso2, &setisomax, ISO_SETVALMAX, GLUIIsoBoundCB);
      RADIOBUTTON_iso_percentile_max = glui_bounds->add_radiobutton_to_group(RADIO_iso_setmax, "percentile max");
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmax, "set max");
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmax, "global max");
      GLUIIsoBoundCB(ISO_SETVALMAX);
      RADIOBUTTON_iso_percentile_max->disable();
    }

    ROLLOUT_iso_color = glui_bounds->add_rollout_to_panel(ROLLOUT_iso, "Color/transparency", false, ISO_ROLLOUT_COLOR, IsoRolloutCB);
    TOGGLE_ROLLOUT(isoprocinfo, nisoprocinfo, ROLLOUT_iso_color, ISO_ROLLOUT_COLOR, glui_bounds);

    RADIO_transparency_option = glui_bounds->add_radiogroup_to_panel(ROLLOUT_iso_color, &iso_transparency_option, ISO_TRANSPARENCY_OPTION, GLUIIsoBoundCB);
    glui_bounds->add_radiobutton_to_group(RADIO_transparency_option, "transparent(constant)");
    glui_bounds->add_radiobutton_to_group(RADIO_transparency_option, "transparent(varying)");
    glui_bounds->add_radiobutton_to_group(RADIO_transparency_option, "opaque");
    GLUIIsoBoundCB(ISO_TRANSPARENCY_OPTION);

    PANEL_iso_alllevels = glui_bounds->add_panel_to_panel(ROLLOUT_iso_color, "All levels", true);

    SPINNER_iso_transparency = glui_bounds->add_spinner_to_panel(PANEL_iso_alllevels, "alpha", GLUI_SPINNER_INT, &glui_iso_transparency, ISO_TRANSPARENCY, GLUIIsoBoundCB);
    glui_bounds->add_button_to_panel(PANEL_iso_alllevels, "Apply", GLOBAL_ALPHA, GLUIIsoBoundCB);

    PANEL_iso_eachlevel = glui_bounds->add_panel_to_panel(ROLLOUT_iso_color, "Each level", true);
    SPINNER_iso_level = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "level:", GLUI_SPINNER_INT, &glui_iso_level, ISO_LEVEL, GLUIIsoBoundCB);
    SPINNER_iso_level->set_int_limits(1, MAX_ISO_COLORS);
    LIST_colortable = glui_bounds->add_listbox_to_panel(PANEL_iso_eachlevel, "Color:", &i_colortable_list, COLORTABLE_LIST, GLUIIsoBoundCB);
    SPINNER_iso_colors[0] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "red:", GLUI_SPINNER_INT, glui_iso_colors + 0, ISO_COLORS, GLUIIsoBoundCB);
    SPINNER_iso_colors[1] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "green:", GLUI_SPINNER_INT, glui_iso_colors + 1, ISO_COLORS, GLUIIsoBoundCB);
    SPINNER_iso_colors[2] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "blue:", GLUI_SPINNER_INT, glui_iso_colors + 2, ISO_COLORS, GLUIIsoBoundCB);
    SPINNER_iso_colors[3] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "alpha:", GLUI_SPINNER_INT, glui_iso_colors + 3, ISO_COLORS, GLUIIsoBoundCB);

    GLUIUpdateColorTableList(-1);

    SPINNER_iso_colors[0]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_iso_colors[1]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_iso_colors[2]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_iso_colors[3]->set_int_limits(1, 255, GLUI_LIMIT_CLAMP);
    GLUIIsoBoundCB(ISO_LEVEL);
    GLUIIsoBoundCB(ISO_COLORS);

    if(colorbars.ncolorbars > 0){
      LIST_iso_colorbar = glui_bounds->add_listbox_to_panel(ROLLOUT_iso_color, "colormap:", &colorbars.iso_colorbar_index, ISO_COLORBAR_LIST, GLUIIsoBoundCB);
      for(i = 0; i < colorbars.ncolorbars; i++){
        colorbardata *cbi;

        cbi = colorbars.colorbarinfo + i;
        LIST_iso_colorbar->add_item(i, cbi->menu_label);
      }
      LIST_iso_colorbar->set_int_val(colorbars.iso_colorbar_index);
      GLUIIsoBoundCB(ISO_COLORBAR_LIST);
    }
    glui_bounds->add_spinner_to_panel(ROLLOUT_iso_color, "min:", GLUI_SPINNER_FLOAT, &iso_valmin);
    glui_bounds->add_spinner_to_panel(ROLLOUT_iso_color, "max:", GLUI_SPINNER_FLOAT, &iso_valmax);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_color, "Show", &show_iso_color);

    ROLLOUT_iso_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_iso, "Settings", true, ISO_ROLLOUT_SETTINGS, IsoRolloutCB);
    TOGGLE_ROLLOUT(isoprocinfo, nisoprocinfo, ROLLOUT_iso_settings, ISO_ROLLOUT_SETTINGS, glui_bounds);

    visAIso = show_iso_shaded * 1 + show_iso_outline * 2 + show_iso_points * 4;
    CHECKBOX_show_iso_shaded = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, "shaded", &show_iso_shaded, ISO_SURFACE, GLUIIsoBoundCB);
    CHECKBOX_show_iso_outline = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, "outline", &show_iso_outline, ISO_OUTLINE, GLUIIsoBoundCB);
    CHECKBOX_show_iso_points = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, "points", &show_iso_points, ISO_POINTS, GLUIIsoBoundCB);

    SPINNER_isolinewidth = glui_bounds->add_spinner_to_panel(ROLLOUT_iso_settings, "line width", GLUI_SPINNER_FLOAT, &isolinewidth);
    SPINNER_isolinewidth->set_float_limits(1.0, 10.0);
    SPINNER_iso_outline_ioffset = glui_bounds->add_spinner_to_panel(ROLLOUT_iso_settings, "outline offset", GLUI_SPINNER_INT, &iso_outline_ioffset, ISO_OUTLINE_IOFFSET, GLUIIsoBoundCB);
    SPINNER_iso_outline_ioffset->set_int_limits(0, 200);
    SPINNER_isopointsize = glui_bounds->add_spinner_to_panel(ROLLOUT_iso_settings, "point size", GLUI_SPINNER_FLOAT, &isopointsize);
    SPINNER_isopointsize->set_float_limits(1.0, 10.0);

    glui_bounds->add_separator_to_panel(ROLLOUT_iso_settings);

    CHECKBOX_sort2 = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, "Sort transparent surfaces:", &sort_iso_triangles, SORT_SURFACES, GLUISliceBoundCB);
    CHECKBOX_smooth2 = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, "Smooth isosurfaces", &smooth_iso_normal, SMOOTH_SURFACES, GLUISliceBoundCB);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, "wrapup in background", &use_iso_threads);
    glui_bounds->add_button_to_panel(ROLLOUT_iso_settings, "Output isosurface bounds", ISO_BOUNDS_OUTPUT, GLUISliceBoundCB);
  }

  /* Particle File Bounds  */

  have_part = 0;
  if(global_scase.npartinfo > 0)have_part = 1;
  if(have_part == 1){
    char label[100];

    strcpy(label, "");
    strcat(label, "Particle");

    glui_active = 1;
    ROLLOUT_part = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, label, false, PART_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_part, PART_ROLLOUT, glui_bounds);

    partboundsCPP.setup("particle", ROLLOUT_part, partbounds_cpp, npartbounds_cpp, &cache_part_data, SHOW_CACHE_CHECKBOX, GLUIPartBoundsCPP_CB,
      ParticleRolloutCB, particleprocinfo, &nparticleprocinfo);

    ROLLOUT_particle_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_part, "Settings", false, PARTICLE_SETTINGS, ParticleRolloutCB);
    TOGGLE_ROLLOUT(particleprocinfo, nparticleprocinfo, ROLLOUT_particle_settings, PARTICLE_SETTINGS, glui_bounds);

    SPINNER_partpointsize = glui_bounds->add_spinner_to_panel(ROLLOUT_particle_settings, "Particle size", GLUI_SPINNER_FLOAT, &partpointsize);
    SPINNER_partpointsize->set_float_limits(PART_MIN_SIZE, PART_MAX_SIZE);
    SPINNER_streaklinewidth = glui_bounds->add_spinner_to_panel(ROLLOUT_particle_settings, "Streak line width", GLUI_SPINNER_FLOAT, &streaklinewidth);
    SPINNER_streaklinewidth->set_float_limits(PART_MIN_WIDTH, PART_MAX_WIDTH);

    SPINNER_partstreaklength = glui_bounds->add_spinner_to_panel(ROLLOUT_particle_settings, "Streak length (s)", GLUI_SPINNER_FLOAT, &float_streak5value, STREAKLENGTH, PartBoundCB);
    SPINNER_partstreaklength->set_float_limits(0.0, tmax_part);

    CHECKBOX_showtracer = glui_bounds->add_checkbox_to_panel(ROLLOUT_particle_settings, "Always show tracers", &show_tracers_always, TRACERS, PartBoundCB);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_particle_settings, "Show selected particle tag", &select_part);

    PANEL_partread = glui_bounds->add_panel_to_panel(ROLLOUT_particle_settings, "Particle loading");
    CHECKBOX_partfast = glui_bounds->add_checkbox_to_panel(PANEL_partread, "Fast loading", &partfast, PARTFAST, PartBoundCB);
    CHECKBOX_use_partload_threads = glui_bounds->add_checkbox_to_panel(PANEL_partread, "Parallel loading", &use_partload_threads);
    SPINNER_n_part_threads = glui_bounds->add_spinner_to_panel(PANEL_partread, "Files loaded at once:", GLUI_SPINNER_INT, &n_partload_threads);
    if(global_scase.npartinfo > 1){
      SPINNER_n_part_threads->set_int_limits(1, MIN(global_scase.npartinfo, MAX_THREADS));
    }
    else{
      SPINNER_n_part_threads->set_int_limits(1, 1);
    }
    SPINNER_partdrawskip = glui_bounds->add_spinner_to_panel(PANEL_partread, "Draw skip:", GLUI_SPINNER_INT, &partdrawskip, PARTSKIP, PartBoundCB);
    PartBoundCB(PARTFAST);
    PartBoundCB(PARTSKIP);
  }
  PartBoundCB(FILETYPE_INDEX);

  // ----------------------------------- Plot3D ----------------------------------------

  if(global_scase.nplot3dinfo > 0){
    glui_active = 1;
    ROLLOUT_plot3d = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Plot3D", false, PLOT3D_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_plot3d, PLOT3D_ROLLOUT, glui_bounds);

    plot3dboundsCPP.setup("PLOT3D", ROLLOUT_plot3d, plot3dbounds_cpp, nplot3dbounds_cpp, &cache_plot3d_data, SHOW_CACHE_CHECKBOX, GLUIPlot3DBoundsCPP_CB,
      Plot3dRolloutCB, plot3dprocinfo, &nplot3dprocinfo);

    PANEL_plot3d = glui_bounds->add_panel_to_panel(ROLLOUT_plot3d, "", GLUI_PANEL_NONE);

    ROLLOUT_vector = glui_bounds->add_rollout_to_panel(PANEL_plot3d, "Vector", false, PLOT3D_VECTOR_ROLLOUT, Plot3dRolloutCB);
    TOGGLE_ROLLOUT(plot3dprocinfo, nplot3dprocinfo, ROLLOUT_vector, PLOT3D_VECTOR_ROLLOUT, glui_bounds);

    glui_bounds->add_checkbox_to_panel(ROLLOUT_vector, "Show vectors", &visVector, UPDATEPLOT, GLUIPlot3DBoundCB);
    SPINNER_plot3d_vectorpointsize = glui_bounds->add_spinner_to_panel(ROLLOUT_vector, "point size", GLUI_SPINNER_FLOAT, &vectorpointsize, UPDATE_VECTOR, GLUIPlot3DBoundCB);
    SPINNER_plot3d_vectorpointsize->set_float_limits(1.0, 10.0);
    SPINNER_plot3d_vectorlinewidth = glui_bounds->add_spinner_to_panel(ROLLOUT_vector, "vector width", GLUI_SPINNER_FLOAT, &vectorlinewidth, UPDATE_VECTOR, GLUIPlot3DBoundCB);
    SPINNER_plot3d_vectorlinewidth->set_float_limits(1.0, 10.0);
    SPINNER_plot3d_vectorlinelength = glui_bounds->add_spinner_to_panel(ROLLOUT_vector, "vector length multiplier", GLUI_SPINNER_FLOAT, &vecfactor, UPDATE_VECTOR, GLUIPlot3DBoundCB);
    SPINNER_plot3dvectorskip = glui_bounds->add_spinner_to_panel(ROLLOUT_vector, "Vector skip", GLUI_SPINNER_INT, &vectorskip, PLOT3D_VECTORSKIP, GLUIPlot3DBoundCB);

    glui_bounds->add_column_to_panel(PANEL_plot3d, false);

    ROLLOUT_isosurface = glui_bounds->add_rollout_to_panel(PANEL_plot3d, "Isosurface", false, PLOT3D_ISOSURFACE_ROLLOUT, Plot3dRolloutCB);
    TOGGLE_ROLLOUT(plot3dprocinfo, nplot3dprocinfo, ROLLOUT_isosurface, PLOT3D_ISOSURFACE_ROLLOUT, glui_bounds);

    PANEL_pan1 = glui_bounds->add_panel_to_panel(ROLLOUT_isosurface, "", GLUI_PANEL_NONE);

    glui_bounds->add_checkbox_to_panel(PANEL_pan1, "Show isosurface", &visiso, PLOTISO, GLUIPlot3DBoundCB);
    SPINNER_plot3dpointsize = glui_bounds->add_spinner_to_panel(PANEL_pan1, "Point size", GLUI_SPINNER_FLOAT,
      &plot3dpointsize);
    SPINNER_plot3dpointsize->set_float_limits(1.0, 10.0);

    SPINNER_plot3dlinewidth = glui_bounds->add_spinner_to_panel(PANEL_pan1, "Line width", GLUI_SPINNER_FLOAT,
      &plot3dlinewidth);
    SPINNER_plot3dlinewidth->set_float_limits(1.0, 10.0);
    //    glui_bounds->add_column_to_panel(ROLLOUT_isosurface);
    PANEL_pan2 = glui_bounds->add_panel_to_panel(ROLLOUT_isosurface, "", GLUI_PANEL_NONE);
    RADIO_plot3d_isotype = glui_bounds->add_radiogroup_to_panel(PANEL_pan2, &p3dsurfacetype, PLOTISOTYPE, GLUIPlot3DBoundCB);
    RADIOBUTTON_plot3d_iso_hidden = glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype, "Hidden");
    glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype, "shaded");
    glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype, "outline");
    glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype, "points");
    RADIOBUTTON_plot3d_iso_hidden->disable();

    glui_p3min = p3min_all[0];
    glui_p3max = p3max_all[0];
    p3chopmin_temp = p3chopmin[0];
    p3chopmax_temp = p3chopmax[0];
    glui_bounds->add_column_to_panel(ROLLOUT_plot3d, false);
  }

  // ----------------------------------- HVAC ducts ----------------------------------------

  if(global_scase.hvaccoll.nhvacinfo > 0 && nhvacductbounds_cpp > 0){
    glui_active = 1;
    ROLLOUT_hvacduct = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "HVAC ducts", false, HVACDUCT_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_hvacduct, HVACDUCT_ROLLOUT, glui_bounds);

    hvacductboundsCPP.setup("hvac", ROLLOUT_hvacduct, hvacductbounds_cpp, nhvacductbounds_cpp, &cache_hvac_data, HIDE_CACHE_CHECKBOX, GLUIHVACDuctBoundsCPP_CB,
      HVACRolloutCB, hvacductprocinfo, &nhvacductprocinfo);
    GLUIHVACDuctBoundsCPP_CB(BOUND_VAL_TYPE);
  }

  // ----------------------------------- HVAC nodes ----------------------------------------

  if(global_scase.hvaccoll.nhvacinfo > 0 && nhvacnodebounds_cpp > 0){
    glui_active = 1;
    ROLLOUT_hvacnode = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "HVAC nodes", false, HVACNODE_ROLLOUT, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_hvacnode, HVACNODE_ROLLOUT, glui_bounds);
    hvacnodeboundsCPP.setup("hvac", ROLLOUT_hvacnode, hvacnodebounds_cpp, nhvacnodebounds_cpp, &cache_hvac_data, HIDE_CACHE_CHECKBOX, GLUIHVACNodeBoundsCPP_CB,
      HVACRolloutCB, hvacnodeprocinfo, &nhvacnodeprocinfo);
    GLUIHVACNodeBoundsCPP_CB(BOUND_VAL_TYPE);
  }

  // ----------------------------------- Slice ----------------------------------------

  if(global_scase.slicecoll.nsliceinfo > 0){
    glui_active = 1;
    ROLLOUT_slice = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Slice", false, SLICE_ROLLOUT_BOUNDS, BoundRolloutCB);
    TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_slice, SLICE_ROLLOUT_BOUNDS, glui_bounds);

    sliceboundsCPP.setup("slice", ROLLOUT_slice, slicebounds_cpp, nslicebounds_cpp, &cache_slice_data, HIDE_CACHE_CHECKBOX, GLUIHVACSliceBoundsCPP_CB,
      SliceRolloutCB, sliceprocinfo, &nsliceprocinfo);

    PANEL_slice_buttonsA = glui_bounds->add_panel_to_panel(ROLLOUT_slice, "", false);

    ROLLOUT_slice_average = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsA, "Average data", false, SLICE_AVERAGE_ROLLOUT, SliceRolloutCB);
    TOGGLE_ROLLOUT(sliceprocinfo, nsliceprocinfo, ROLLOUT_slice_average, SLICE_AVERAGE_ROLLOUT, glui_bounds);

    CHECKBOX_average_slice = glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_average, "Average slice data", &slice_average_flag);
    SPINNER_sliceaverage = glui_bounds->add_spinner_to_panel(ROLLOUT_slice_average, "Time interval", GLUI_SPINNER_FLOAT, &slice_average_interval);
    SPINNER_sliceaverage->set_float_limits(0.0, MAX(120.0, global_scase.tourcoll.tour_tstop));
    glui_bounds->add_button_to_panel(ROLLOUT_slice_average, "Reload", FILE_RELOAD, GLUISliceBoundCB);

    ROLLOUT_line_contour = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsA, "Line contours", false, LINE_CONTOUR_ROLLOUT, SliceRolloutCB);
    TOGGLE_ROLLOUT(sliceprocinfo, nsliceprocinfo, ROLLOUT_line_contour, LINE_CONTOUR_ROLLOUT, glui_bounds);

    slice_line_contour_min = 0.0;
    slice_line_contour_max = 1.0;
    SPINNER_line_contour_min = glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour, "Min", GLUI_SPINNER_FLOAT,
      &slice_line_contour_min, LINE_CONTOUR_VALUE, GLUISliceBoundCB);
    SPINNER_line_contour_max = glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour, "Max", GLUI_SPINNER_FLOAT,
      &slice_line_contour_max, LINE_CONTOUR_VALUE, GLUISliceBoundCB);
    slice_line_contour_num = 1;
    SPINNER_line_contour_num = glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour, "Number of contours", GLUI_SPINNER_INT,
      &slice_line_contour_num, LINE_CONTOUR_VALUE, GLUISliceBoundCB);
    SPINNER_line_contour_width = glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour, "contour width", GLUI_SPINNER_FLOAT, &slice_line_contour_width);
    SPINNER_line_contour_width->set_float_limits(1.0, 10.0);
    RADIO_contour_type = glui_bounds->add_radiogroup_to_panel(ROLLOUT_line_contour, &slice_contour_type);
    glui_bounds->add_radiobutton_to_group(RADIO_contour_type, "line");
#ifdef _DEBUG
    glui_bounds->add_radiobutton_to_group(RADIO_contour_type, "stepped");
#endif

    BUTTON_update_line_contour = glui_bounds->add_button_to_panel(ROLLOUT_line_contour, "Update contours", UPDATE_LINE_CONTOUR_VALUE, GLUISliceBoundCB);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_line_contour, "Show contours", &vis_slice_contours);

    if(n_embedded_meshes > 0){
      CHECKBOX_skip_subslice = glui_bounds->add_checkbox_to_panel(ROLLOUT_slice, "Skip coarse sub-slice", &skip_slice_in_embedded_mesh);
    }

    if(global_scase.slicecoll.nsliceinfo > 0){
      ROLLOUT_plotslice = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsA, "2D plots", false, SLICE_PLOT2D_ROLLOUT, SliceRolloutCB);
      TOGGLE_ROLLOUT(sliceprocinfo, nsliceprocinfo, ROLLOUT_plotslice, SLICE_PLOT2D_ROLLOUT, glui_bounds);

      PANEL_slice_plot2dd = glui_bounds->add_panel_to_panel(ROLLOUT_plotslice, "", false);
      PANEL_slice_plot2de = glui_bounds->add_panel_to_panel(PANEL_slice_plot2dd, "", false);
      CHECKBOX_vis_slice_plot = glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2de, "show plot", &vis_slice_plot, SLICE_PLOT2D, GLUISliceBoundCB);
      glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2de, "use specified min/max", &slice_plot_bound_option, SLICE_PLOT2D, GLUISliceBoundCB);
      SPINNER_size_factor2 = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2de, "plot size(rel)", GLUI_SPINNER_FLOAT, &plot2d_size_factor, SLICE_SIZE, GLUISliceBoundCB);
      SPINNER_size_factor2->set_float_limits(0.0, 1.0);

      glui_bounds->add_column_to_panel(PANEL_slice_plot2dd, false);
      PANEL_slice_plot2de = glui_bounds->add_panel_to_panel(PANEL_slice_plot2dd, "", false);
      glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2de, "show title", &show_plot2d_title);
      glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2de, "show x axis labels", &show_plot2d_xlabels);
      glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2de, "show y axis labels", &show_plot2d_ylabels);
      glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2de, "show data position", &show_plot2d_slice_position);

      PANEL_slice_plot2dc = glui_bounds->add_panel_to_panel(ROLLOUT_plotslice, "", false);
      PANEL_slice_plot2da = glui_bounds->add_panel_to_panel(PANEL_slice_plot2dc, "position");
      SPINNER_slice_x = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2da, "x", GLUI_SPINNER_FLOAT, slice_xyz + 0, SLICE_PLOT2D, GLUISliceBoundCB);
      SPINNER_slice_y = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2da, "y", GLUI_SPINNER_FLOAT, slice_xyz + 1, SLICE_PLOT2D, GLUISliceBoundCB);
      SPINNER_slice_z = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2da, "z", GLUI_SPINNER_FLOAT, slice_xyz + 2, SLICE_PLOT2D, GLUISliceBoundCB);
      SPINNER_slice_x->set_float_limits(xbar0FDS, xbarFDS);
      SPINNER_slice_y->set_float_limits(ybar0FDS, ybarFDS);
      SPINNER_slice_z->set_float_limits(zbar0FDS, zbarFDS);
      glui_bounds->add_column_to_panel(PANEL_slice_plot2dc, false);
      PANEL_slice_plot2db = glui_bounds->add_panel_to_panel(PANEL_slice_plot2dc, "average data");
      glui_bounds->add_checkbox_to_panel(PANEL_slice_plot2db, "average over dt,dx,dy,dz", &average_plot2d_slice_region, SLICE_PLOT2D, GLUISliceBoundCB);
      SPINNER_plot2d_dt = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2db, "dt", GLUI_SPINNER_FLOAT, &plot2d_time_average, SLICE_SIZE, GLUISliceBoundCB);
      SPINNER_slice_dx = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2db, "dx", GLUI_SPINNER_FLOAT, slice_dxyz + 0, SLICE_DPLOT, GLUISliceBoundCB);
      SPINNER_slice_dy = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2db, "dy", GLUI_SPINNER_FLOAT, slice_dxyz + 1, SLICE_DPLOT, GLUISliceBoundCB);
      SPINNER_slice_dz = glui_bounds->add_spinner_to_panel(PANEL_slice_plot2db, "dz", GLUI_SPINNER_FLOAT, slice_dxyz + 2, SLICE_DPLOT, GLUISliceBoundCB);

      BUTTON_OUTPUT_PLOT2D = glui_bounds->add_button_to_panel(ROLLOUT_plotslice, "Output data", SLICE_PLOT_CSV, GLUISliceBoundCB);
      GLUISliceBoundCB(SLICE_PLOT_LABEL);
    }

    if(nslicedups > 0){
      ROLLOUT_slicedups = glui_bounds->add_rollout_to_panel(ROLLOUT_slice, "Duplicates", false, SLICE_DUP_ROLLOUT, SliceRolloutCB);
      TOGGLE_ROLLOUT(sliceprocinfo, nsliceprocinfo, ROLLOUT_slicedups, SLICE_DUP_ROLLOUT, glui_bounds);

      PANEL_slicedup = glui_bounds->add_panel_to_panel(ROLLOUT_slicedups, "slices", true);
      RADIO_slicedup = glui_bounds->add_radiogroup_to_panel(PANEL_slicedup, &slicedup_option, UPDATE_SLICEDUPS, GLUISliceBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_slicedup, "Keep all");
      glui_bounds->add_radiobutton_to_group(RADIO_slicedup, "Keep fine");
      glui_bounds->add_radiobutton_to_group(RADIO_slicedup, "Keep coarse");

      PANEL_vectorslicedup = glui_bounds->add_panel_to_panel(ROLLOUT_slicedups, "vector slices", true);
      RADIO_vectorslicedup = glui_bounds->add_radiogroup_to_panel(PANEL_vectorslicedup, &vectorslicedup_option, UPDATE_SLICEDUPS, GLUISliceBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_vectorslicedup, "Keep all");
      glui_bounds->add_radiobutton_to_group(RADIO_vectorslicedup, "Keep fine");
      glui_bounds->add_radiobutton_to_group(RADIO_vectorslicedup, "Keep coarse");
    }

    ROLLOUT_slice_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_slice, "Settings", false, SLICE_SETTINGS_ROLLOUT, SliceRolloutCB);
    TOGGLE_ROLLOUT(sliceprocinfo, nsliceprocinfo, ROLLOUT_slice_settings, SLICE_SETTINGS_ROLLOUT, glui_bounds);

    PANEL_immersed = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "show slice", true);
    PANEL_immersed_region = glui_bounds->add_panel_to_panel(PANEL_immersed, "region", true);
    RADIO_slice_celltype = glui_bounds->add_radiogroup_to_panel(PANEL_immersed_region, &slice_celltype, IMMERSED_SWITCH_CELLTYPE, GLUIImmersedBoundCB);
    glui_bounds->add_radiobutton_to_group(RADIO_slice_celltype, "gas");
    glui_bounds->add_radiobutton_to_group(RADIO_slice_celltype, "solid");


    RADIO_button_cutcell = glui_bounds->add_radiobutton_to_group(RADIO_slice_celltype, "cut cell");
    if(global_scase.ngeom_data == 0)RADIO_button_cutcell->disable();

    PANEL_immersed_outlinetype = glui_bounds->add_panel_to_panel(PANEL_immersed, "outline type", true);
    RADIO_slice_edgetype = glui_bounds->add_radiogroup_to_panel(PANEL_immersed_outlinetype, &glui_slice_edgetype, IMMERSED_SWITCH_EDGETYPE, GLUIImmersedBoundCB);
    glui_bounds->add_radiobutton_to_group(RADIO_slice_edgetype, "polygon");
    glui_bounds->add_radiobutton_to_group(RADIO_slice_edgetype, "triangle");
    glui_bounds->add_radiobutton_to_group(RADIO_slice_edgetype, "none");

    glui_bounds->add_column_to_panel(PANEL_immersed, false);
    PANEL_immersed_drawas = glui_bounds->add_panel_to_panel(PANEL_immersed, "draw slices using", true);
    CHECKBOX_show_slice_shaded   = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas, "solid colors", &glui_show_slice_shaded,   IMMERSED_SET_DRAWTYPE, GLUIImmersedBoundCB);
    CHECKBOX_show_slice_outlines = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas, "outlines",     &glui_show_slice_outlines, IMMERSED_SET_DRAWTYPE, GLUIImmersedBoundCB);
    CHECKBOX_show_slice_points   = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas, "points",       &glui_show_slice_points,   IMMERSED_SET_DRAWTYPE, GLUIImmersedBoundCB);
    CHECKBOX_show_slice_values   = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas, "values",       &glui_show_slice_values,   IMMERSED_SET_DRAWTYPE, GLUIImmersedBoundCB);
    CHECKBOX_show_vector_slice   = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas, "vectors",      &glui_show_vector_slice,   IMMERSED_SET_DRAWTYPE, GLUIImmersedBoundCB);

    GLUIImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
    GLUIImmersedBoundCB(IMMERSED_SWITCH_EDGETYPE);
    GLUIImmersedBoundCB(IMMERSED_SET_DRAWTYPE);

    PANEL_slicevalues = glui_bounds->add_panel_to_panel(PANEL_immersed, "slice values", true);
    glui_bounds->add_button_to_panel(PANEL_slicevalues, "show all regions", SHOW_ALL_SLICE_VALUES, GLUIImmersedBoundCB);
    glui_bounds->add_button_to_panel(PANEL_slicevalues, "hide all regions", HIDE_ALL_SLICE_VALUES, GLUIImmersedBoundCB);
    SPINNER_sliceval_ndigits = glui_bounds->add_spinner_to_panel(PANEL_slicevalues, "digits (0 all)", GLUI_SPINNER_INT, &sliceval_ndigits);
    SPINNER_sliceval_ndigits->set_int_limits(0, 10);

    PANEL_slice_vector = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "vector properties", true);

    PANEL_vector1 = glui_bounds->add_panel_to_panel(PANEL_slice_vector, "", false);
    SPINNER_vectorpointsize = glui_bounds->add_spinner_to_panel(PANEL_vector1, "point size", GLUI_SPINNER_FLOAT,
      &vectorpointsize, UPDATE_VECTOR, GLUISliceBoundCB);
    SPINNER_vectorpointsize->set_float_limits(1.0, 20.0);
    SPINNER_vectorlinewidth = glui_bounds->add_spinner_to_panel(PANEL_vector1, "width", GLUI_SPINNER_FLOAT, &vectorlinewidth, UPDATE_VECTOR, GLUISliceBoundCB);
    SPINNER_vectorlinewidth->set_float_limits(1.0, 20.0);
    SPINNER_vectorlinelength = glui_bounds->add_spinner_to_panel(PANEL_vector1, "length", GLUI_SPINNER_FLOAT, &vecfactor, UPDATE_VECTOR, GLUISliceBoundCB);
    SPINNER_slicevectorskip = glui_bounds->add_spinner_to_panel(PANEL_vector1, "skip", GLUI_SPINNER_INT, &vectorskip, SLICE_VECTORSKIP, GLUISliceBoundCB);

    glui_bounds->add_column_to_panel(PANEL_slice_vector, false);
    PANEL_vector2 = glui_bounds->add_panel_to_panel(PANEL_slice_vector, "", false);
    glui_bounds->add_checkbox_to_panel(PANEL_vector2, "uniform spacing", &vec_uniform_spacing, VEC_UNIFORM_SPACING, GLUISliceBoundCB);
    glui_bounds->add_checkbox_to_panel(PANEL_vector2, "uniform length", &vec_uniform_length);
    CHECKBOX_color_vector_black = glui_bounds->add_checkbox_to_panel(PANEL_vector2, "use foreground color", &color_vector_black);

    PANEL_showslice = glui_bounds->add_panel_to_panel(PANEL_vector2, "show vectors and", true);
    CHECKBOX_show_node_slices_and_vectors = glui_bounds->add_checkbox_to_panel(PANEL_showslice, "node centered slices", &show_node_slices_and_vectors);
    CHECKBOX_show_node_slices_and_vectors = glui_bounds->add_checkbox_to_panel(PANEL_showslice, "cell centered slices", &show_cell_slices_and_vectors);

    if(global_scase.ngeom_data > 0)glui_bounds->add_column_to_panel(ROLLOUT_slice_settings, false);

    if(global_scase.show_slice_in_obst == ONLY_IN_GAS){
      show_slice_in_gas = 1;
      show_slice_in_solid = 0;
    }
    else if(global_scase.show_slice_in_obst == GAS_AND_SOLID){
      show_slice_in_gas = 1;
      show_slice_in_solid = 1;
    }
    else if(global_scase.show_slice_in_obst == ONLY_IN_SOLID){
      show_slice_in_gas = 0;
      show_slice_in_solid = 1;
    }
    else{
      show_slice_in_gas = 1;
      show_slice_in_solid = 0;
    }

    if(global_scase.ngeom_data == 0)glui_bounds->add_column_to_panel(ROLLOUT_slice_settings, false);

    PANEL_slice_smoke = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "slice(fire)", true);
    glui_bounds->add_checkbox_to_panel(PANEL_slice_smoke, "max blending", &slices3d_max_blending);
    glui_bounds->add_checkbox_to_panel(PANEL_slice_smoke, "show all 3D slices", &showall_3dslices);

    PANEL_slice_misc = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "", true);
    SPINNER_transparent_level = glui_bounds->add_spinner_to_panel(PANEL_slice_misc, "Transparent level", GLUI_SPINNER_FLOAT, &transparent_level, TRANSPARENTLEVEL, GLUISliceBoundCB);
    SPINNER_transparent_level->set_float_limits(0.0, 1.0);
    glui_bounds->add_spinner_to_panel(PANEL_slice_misc, "slice offset", GLUI_SPINNER_FLOAT, &slice_dz);
    CHECKBOX_sortslices = glui_bounds->add_checkbox_to_panel(PANEL_slice_misc, "sort slices(back to front)", &sortslices, SORTSLICES, GLUISliceBoundCB);
    CHECKBOX_sortslices_debug = glui_bounds->add_checkbox_to_panel(PANEL_slice_misc, "sort slices(debug)", &sortslices_debug, SORTSLICES_DEBUG, GLUISliceBoundCB);
    for(i = 0; i < global_scase.meshescoll.nmeshes; i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      max_slice_skip = MAX(max_slice_skip, meshi->ibar / 2);
      max_slice_skip = MAX(max_slice_skip, meshi->jbar / 2);
      max_slice_skip = MAX(max_slice_skip, meshi->kbar / 2);

    }
    SPINNER_slice_skip = glui_bounds->add_spinner_to_panel(PANEL_slice_misc, "data skip", GLUI_SPINNER_INT, &slice_skip, SLICE_SKIP, GLUISliceBoundCB);
    GLUISliceBoundCB(SLICE_SKIP);
    glui_bounds->add_checkbox_to_panel(PANEL_slice_misc, "Output data (press r)", &output_slicedata);

    if(global_scase.nterraininfo > 0){
      glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_settings, "terrain slice overlap", &terrain_slice_overlap);
      glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_settings, "actual agl offset", &agl_offset_actual);
    }
    PANEL_sliceload_option = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "", true);
    glui_bounds->add_button_to_panel(PANEL_sliceload_option, "Load all", SLICE_LOADALL, SliceLoadCB);
    CHECKBOX_sliceload_isvector = glui_bounds->add_checkbox_to_panel(PANEL_sliceload_option, "vector slice", &sliceload_isvector);
    PANEL_slice_xyz = glui_bounds->add_panel_to_panel(PANEL_sliceload_option, "orientation", true);
    RADIO_sliceload_dir = glui_bounds->add_radiogroup_to_panel(PANEL_slice_xyz, &sliceload_dir);
    RADIOBUTTON_sliceload_x = glui_bounds->add_radiobutton_to_group(RADIO_sliceload_dir, "x");
    RADIOBUTTON_sliceload_y = glui_bounds->add_radiobutton_to_group(RADIO_sliceload_dir, "y");
    RADIOBUTTON_sliceload_z = glui_bounds->add_radiobutton_to_group(RADIO_sliceload_dir, "z");
    RADIOBUTTON_sliceload_xyz = glui_bounds->add_radiobutton_to_group(RADIO_sliceload_dir, "x,y,z");
    PANEL_slice_filetype = glui_bounds->add_panel_to_panel(PANEL_sliceload_option, "file type", true);
    RADIO_filetype = glui_bounds->add_radiogroup_to_panel(PANEL_slice_filetype, &sliceload_filetype);
    glui_bounds->add_radiobutton_to_group(RADIO_filetype, "node centered");
    glui_bounds->add_radiobutton_to_group(RADIO_filetype, "cell centered");
    LISTBOX_sliceload = glui_bounds->add_listbox_to_panel(PANEL_sliceload_option, "quantity:", &sliceload_boundtype);
    for(i = 0;i < nslicebounds_cpp;i++){
      LISTBOX_sliceload->add_item(i, slicebounds_cpp[i].label);
    }
    glui_bounds->add_button_to_panel(PANEL_sliceload_option, "Unload all", SLICE_UNLOADALL, SliceLoadCB);
  }

  // ----------------------------------- Time ----------------------------------------

  ROLLOUT_time = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Load options", false, TIME_ROLLOUT, BoundRolloutCB);
  TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_time, TIME_ROLLOUT, glui_bounds);

  PANEL_loadbounds = glui_bounds->add_panel_to_panel(ROLLOUT_time, "", GLUI_PANEL_NONE);

  ROLLOUT_autoload = glui_bounds->add_rollout_to_panel(PANEL_loadbounds, "Auto load", false, LOAD_AUTO_ROLLOUT, LoadRolloutCB);
  TOGGLE_ROLLOUT(loadprocinfo, nloadprocinfo, ROLLOUT_autoload, LOAD_AUTO_ROLLOUT, glui_bounds);

  glui_bounds->add_checkbox_to_panel(ROLLOUT_autoload, "Auto load at startup", &loadfiles_at_startup, BOUND_STARTUP, BoundBoundCB);
  glui_bounds->add_button_to_panel(ROLLOUT_autoload, "Save auto load file list", SAVE_FILE_LIST, BoundBoundCB);
  glui_bounds->add_button_to_panel(ROLLOUT_autoload, "Auto load now", LOAD_FILES, BoundBoundCB);

  ROLLOUT_time1a = glui_bounds->add_rollout_to_panel(PANEL_loadbounds, "Set time", false, LOAD_TIMESET_ROLLOUT, LoadRolloutCB);
  TOGGLE_ROLLOUT(loadprocinfo, nloadprocinfo, ROLLOUT_time1a, LOAD_TIMESET_ROLLOUT, glui_bounds);

  SPINNER_timebounds = glui_bounds->add_spinner_to_panel(ROLLOUT_time1a, "Time:", GLUI_SPINNER_FLOAT, &glui_time);
  glui_bounds->add_spinner_to_panel(ROLLOUT_time1a, "Offset:", GLUI_SPINNER_FLOAT, &timeoffset);
  BUTTON_SETTIME = glui_bounds->add_button_to_panel(ROLLOUT_time1a, "Set", SET_TIME, TimeBoundCB);

  ROLLOUT_time2 = glui_bounds->add_rollout_to_panel(PANEL_loadbounds, "Set time limits", false, LOAD_TIMEBOUND_ROLLOUT, LoadRolloutCB);
  TOGGLE_ROLLOUT(loadprocinfo, nloadprocinfo, ROLLOUT_time2, LOAD_TIMEBOUND_ROLLOUT, glui_bounds);

  glui_bounds->add_button_to_panel(ROLLOUT_time2, "Use FDS start/end times", SET_FDS_TIMES, TimeBoundCB);

  PANEL_time2a = glui_bounds->add_panel_to_panel(ROLLOUT_time2, "", false);
  SPINNER_tload_begin = glui_bounds->add_spinner_to_panel(PANEL_time2a, "min time", GLUI_SPINNER_FLOAT, &global_scase.tload_begin, TBOUNDS, TimeBoundCB);
  glui_bounds->add_column_to_panel(PANEL_time2a, false);
  CHECKBOX_use_tload_begin = glui_bounds->add_checkbox_to_panel(PANEL_time2a, "", &use_tload_begin, TBOUNDS_USE, TimeBoundCB);

  PANEL_time2b = glui_bounds->add_panel_to_panel(ROLLOUT_time2, "", false);
  SPINNER_tload_end = glui_bounds->add_spinner_to_panel(PANEL_time2b, "max time", GLUI_SPINNER_FLOAT, &global_scase.tload_end, TBOUNDS, TimeBoundCB);
  glui_bounds->add_column_to_panel(PANEL_time2b, false);
  CHECKBOX_use_tload_end = glui_bounds->add_checkbox_to_panel(PANEL_time2b, "", &use_tload_end, TBOUNDS_USE, TimeBoundCB);

  PANEL_time2c = glui_bounds->add_panel_to_panel(ROLLOUT_time2, "", false);
  SPINNER_tload_skip = glui_bounds->add_spinner_to_panel(PANEL_time2c, "frame skip", GLUI_SPINNER_INT, &tload_skip, TBOUNDS, TimeBoundCB);
  glui_bounds->add_column_to_panel(PANEL_time2c, false);
  CHECKBOX_use_tload_skip = glui_bounds->add_checkbox_to_panel(PANEL_time2c, "", &use_tload_skip, TBOUNDS_USE, TimeBoundCB);
  SPINNER_tload_skip->set_int_limits(0, 1000);

  glui_bounds->add_button_to_panel(ROLLOUT_time2, "Reload all data", RELOAD_ALL_DATA, TimeBoundCB);

  TimeBoundCB(TBOUNDS_USE);
  TimeBoundCB(TBOUNDS);

  ROLLOUT_box_specify = glui_bounds->add_rollout_to_panel(ROLLOUT_time, "Set spatial limits", false, LOAD_SPACEBOUND_ROLLOUT, LoadRolloutCB);
  TOGGLE_ROLLOUT(loadprocinfo, nloadprocinfo, ROLLOUT_box_specify, LOAD_SPACEBOUND_ROLLOUT, glui_bounds);

  PANEL_intersection_box = glui_bounds->add_panel_to_panel(ROLLOUT_box_specify, "", false);
  PANEL_intersection_box->set_alignment(GLUI_ALIGN_LEFT);
  CHECKBOX_show_intersected_meshes = glui_bounds->add_checkbox_to_panel(PANEL_intersection_box, "Show selected meshes", &show_intersected_meshes, USEMESH_DRAW_MESH, MeshBoundCB);
  glui_bounds->add_checkbox_to_panel(PANEL_intersection_box, "Show selected mesh indices", &show_mesh_labels);
  CHECKBOX_load_only_when_unloaded = glui_bounds->add_checkbox_to_panel(PANEL_intersection_box, "Load a file only if unloaded", &load_only_when_unloaded, USEMESH_LOAD_WHEN_LOADED, MeshBoundCB);

  PANEL_mesh = glui_bounds->add_panel_to_panel(ROLLOUT_box_specify, "Select meshes by specifying the intersection box");
  RADIO_intersect_option = glui_bounds->add_radiogroup_to_panel(PANEL_mesh, &glui_mesh_intersection_option, USEMESH_XYZ, MeshBoundCB);
  glui_bounds->add_radiobutton_to_group(RADIO_intersect_option, "Select meshes that intersect the box");
  glui_bounds->add_radiobutton_to_group(RADIO_intersect_option, "Select meshes that are completely within the box");
  CHECKBOX_show_intersection_box = glui_bounds->add_checkbox_to_panel(PANEL_mesh, "Show intersection box", &show_intersection_box, USEMESH_DRAW_BOX, MeshBoundCB);

  PANEL_mesh_minmax = glui_bounds->add_panel_to_panel(PANEL_mesh, "", false);
  PANEL_meshxyz[0] = glui_bounds->add_panel_to_panel(PANEL_mesh_minmax, "", false);
  PANEL_meshxyz[2] = glui_bounds->add_panel_to_panel(PANEL_mesh_minmax, "", false);
  PANEL_meshxyz[4] = glui_bounds->add_panel_to_panel(PANEL_mesh_minmax, "", false);
  glui_bounds->add_column_to_panel(PANEL_mesh_minmax, false);
  PANEL_meshxyz[1] = glui_bounds->add_panel_to_panel(PANEL_mesh_minmax, "", false);
  PANEL_meshxyz[3] = glui_bounds->add_panel_to_panel(PANEL_mesh_minmax, "", false);
  PANEL_meshxyz[5] = glui_bounds->add_panel_to_panel(PANEL_mesh_minmax, "", false);

  char lbl[6][6];
  strcpy(lbl[0], "xmin");
  strcpy(lbl[1], "xmax");
  strcpy(lbl[2], "ymin");
  strcpy(lbl[3], "ymax");
  strcpy(lbl[4], "zmin");
  strcpy(lbl[5], "zmax");
  for(i = 0;i < 6;i++){
    SPINNER_meshclip[i] = glui_bounds->add_spinner_to_panel(PANEL_meshxyz[i], lbl[i], GLUI_SPINNER_FLOAT, meshclip + i, USEMESH_XYZ, MeshBoundCB);
    glui_bounds->add_column_to_panel(PANEL_meshxyz[i], false);
    CHECKBOX_use_meshclip[i] = glui_bounds->add_checkbox_to_panel(PANEL_meshxyz[i], "", use_meshclip + i, USEMESH_USE_XYZ + i, MeshBoundCB);
  }
  SPINNER_meshclip[0]->set_float_limits(xbar0FDS, xbarFDS);
  SPINNER_meshclip[1]->set_float_limits(xbar0FDS, xbarFDS);
  SPINNER_meshclip[2]->set_float_limits(ybar0FDS, ybarFDS);
  SPINNER_meshclip[3]->set_float_limits(ybar0FDS, ybarFDS);
  SPINNER_meshclip[4]->set_float_limits(zbar0FDS, zbarFDS);
  SPINNER_meshclip[5]->set_float_limits(zbar0FDS, zbarFDS);

  PANEL_setmesh = glui_bounds->add_panel_to_panel(ROLLOUT_box_specify, "or by selecting mesh(es) directly", true);

  PANEL_addmesh = glui_bounds->add_panel_to_panel(PANEL_setmesh, "", false);
  SPINNER_set_mesh = glui_bounds->add_spinner_to_panel(PANEL_addmesh, "mesh:", GLUI_SPINNER_INT, &set_mesh);
  SPINNER_set_mesh->set_int_limits(1, global_scase.meshescoll.nmeshes);
  glui_bounds->add_column_to_panel(PANEL_addmesh, false);
  glui_bounds->add_button_to_panel(PANEL_addmesh, "Add", USEMESH_SET_ONE, MeshBoundCB);
  glui_bounds->add_column_to_panel(PANEL_addmesh, false);
  glui_bounds->add_button_to_panel(PANEL_addmesh, "Remove", USEMESH_REMOVE_ONE, MeshBoundCB);

  PANEL_addremovemesh = glui_bounds->add_panel_to_panel(PANEL_setmesh, "", false);
  glui_bounds->add_button_to_panel(PANEL_addremovemesh, "Add all", USEMESH_SET_ALL, MeshBoundCB);
  glui_bounds->add_column_to_panel(PANEL_addremovemesh, false);
  glui_bounds->add_button_to_panel(PANEL_addremovemesh, "Remove all", USEMESH_REMOVE_ALL, MeshBoundCB);
  MeshBoundCB(USEMESH_USE_XYZ_ALL);
  glui_meshclip_defined = 1;

  ROLLOUT_view_options = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "View options", false, VIEWOPTIONS_ROLLOUT, BoundRolloutCB);
  TOGGLE_ROLLOUT(boundprocinfo, nboundprocinfo, ROLLOUT_view_options, VIEWOPTIONS_ROLLOUT, glui_bounds);

  ROLLOUT_vismesh_blockages = glui_bounds->add_rollout_to_panel(ROLLOUT_view_options, "View blockages by mesh", false, MESHBLOCKAGE_ROLLOUT, ViewRolloutCB);
  TOGGLE_ROLLOUT(viewprocinfo, nviewprocinfo, ROLLOUT_vismesh_blockages, MESHBLOCKAGE_ROLLOUT, glui_bounds);

  int nn = MIN(global_scase.meshescoll.nmeshes, 256);
  PANEL_geom_vis2 = glui_bounds->add_panel_to_panel(ROLLOUT_vismesh_blockages, "", GLUI_PANEL_NONE);
  for(i = 1;i < 9;i++){
    AddMeshCheckbox(i, nn, PANEL_geom_vis2, CHECKBOX_show_mesh_geom, 1);
  }

  PANEL_geom_vis3 = glui_bounds->add_panel_to_panel(ROLLOUT_vismesh_blockages, "", GLUI_PANEL_NONE);
  glui_bounds->add_button_to_panel(PANEL_geom_vis3, "Show all",     SHOW_ALL_MESH_GEOM, GLUIShowHideGeomDataCB);
  glui_bounds->add_column_to_panel(PANEL_geom_vis3, false);
  glui_bounds->add_button_to_panel(PANEL_geom_vis3, "Hide all",     HIDE_ALL_MESH_GEOM, GLUIShowHideGeomDataCB);

  ROLLOUT_vismesh_data = glui_bounds->add_rollout_to_panel(ROLLOUT_view_options, "View data by mesh", false, MESHDATA_ROLLOUT, ViewRolloutCB);
  TOGGLE_ROLLOUT(viewprocinfo, nviewprocinfo, ROLLOUT_vismesh_data, MESHDATA_ROLLOUT, glui_bounds);
  PANEL_data_vis2 = glui_bounds->add_panel_to_panel(ROLLOUT_vismesh_data, "", GLUI_PANEL_NONE);
  for(i = 1;i < 9;i++){
    AddMeshCheckbox(i, nn, PANEL_data_vis2, CHECKBOX_show_mesh_data, 2);
  }
  PANEL_data_vis3 = glui_bounds->add_panel_to_panel(ROLLOUT_vismesh_data, "", GLUI_PANEL_NONE);
  glui_bounds->add_button_to_panel(PANEL_data_vis3, "Show all",     SHOW_ALL_MESH_DATA, GLUIShowHideGeomDataCB);
  glui_bounds->add_column_to_panel(PANEL_data_vis3, false);
  glui_bounds->add_button_to_panel(PANEL_data_vis3, "Hide all",     HIDE_ALL_MESH_DATA, GLUIShowHideGeomDataCB);

  // -------------- Data coloring -------------------

  ROLLOUT_coloring = glui_bounds->add_rollout("Coloring", false, COLORING_ROLLOUT, FileDataColRolloutCB);
  TOGGLE_ROLLOUT(filedatacolprocinfo, nfiledatacolprocinfo, ROLLOUT_coloring, COLORING_ROLLOUT, glui_bounds);

  PANEL_cb11 = glui_bounds->add_panel_to_panel(ROLLOUT_coloring, "", GLUI_PANEL_NONE);

  PANEL_colorbar_properties = glui_bounds->add_panel_to_panel(PANEL_cb11, "Colorbar");

  if(colorbars.ncolorbars>0){
    colorbartype = -1;
    LISTBOX_cb_bound = glui_bounds->add_listbox_to_panel(PANEL_colorbar_properties, "", &colorbartype, COLORBAR_LIST2, GLUISliceBoundCB);
    GLUIUpdateColorbarListBound(1);
    LISTBOX_cb_bound->set_int_val(colorbartype_default);

    glui_bounds->add_button_to_panel(PANEL_colorbar_properties, "Next",     COLORBAR_LIST2_NEXT, GLUISliceBoundCB);
    glui_bounds->add_button_to_panel(PANEL_colorbar_properties, "Previous", COLORBAR_LIST2_PREV, GLUISliceBoundCB);
  }

  CHECKBOX_visColorbarVertical2   = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "vertical",   &visColorbarVertical,   LABELS_vcolorbar, GLUILabelsCB);
  CHECKBOX_visColorbarHorizontal2 = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "horizontal", &visColorbarHorizontal, LABELS_hcolorbar, GLUILabelsCB);
  RADIO2_plot3d_display = glui_bounds->add_radiogroup_to_panel(PANEL_colorbar_properties, &contour_type, UPDATEPLOT, GLUIPlot3DBoundCB);
  glui_bounds->add_radiobutton_to_group(RADIO2_plot3d_display, "Continuous");
  glui_bounds->add_radiobutton_to_group(RADIO2_plot3d_display, "Stepped");
  glui_bounds->add_radiobutton_to_group(RADIO2_plot3d_display, "Line");
  SPINNER_colorbar_linewidth = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, "line width:", GLUI_SPINNER_INT, &colorbar_linewidth, UPDATEPLOT, GLUIPlot3DBoundCB);
  SPINNER_colorbar_linewidth->set_int_limits(1, 10);
  CHECKBOX_colorbar_flip = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "flip", &colorbar_flip, FLIP, GLUILabelsCB);
  CHECKBOX_colorbar_autoflip = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "Auto flip", &colorbar_autoflip, FLIP, GLUILabelsCB);
  SPINNER_colorbar_shift = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, "shift:", GLUI_SPINNER_FLOAT, &colorbar_shift, LABELS_colorbar_shift, GLUILabelsCB);
  SPINNER_colorbar_shift->set_float_limits(COLORBAR_SHIFT_MIN, COLORBAR_SHIFT_MAX);

  glui_bounds->add_separator_to_panel(PANEL_colorbar_properties);

  SPINNER_colorbar_select_index = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, "selection index:", GLUI_SPINNER_INT, &colorbar_select_index,  COLORINDEX, GLUISliceBoundCB);
  SPINNER_colorbar_select_index->set_int_limits(-1, 255);
  SPINNER_colorbar_selection_width = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, "selection width:",    GLUI_SPINNER_INT, &colorbar_selection_width, COLORBAND,  GLUISliceBoundCB);
  SPINNER_colorbar_selection_width->set_int_limits(COLORBAR_SELECTION_WIDTH_MIN, COLORBAR_SELECTION_WIDTH_MAX);
  SPINNER_ncolorlabel_digits = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, "max label digits:", GLUI_SPINNER_INT, &ncolorlabel_digits, COLORLABEL_DIGITS, GLUISliceBoundCB);
  SPINNER_ncolorlabel_digits->set_int_limits(COLORBAR_NDECIMALS_MIN, COLORBAR_NDECIMALS_MAX, GLUI_LIMIT_CLAMP);
  SPINNER_ncolorlabel_padding = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, "extra space:", GLUI_SPINNER_INT, &ncolorlabel_padding, COLORLABEL_DIGITS, GLUISliceBoundCB);
  SPINNER_ncolorlabel_padding->set_int_limits(0, 8, GLUI_LIMIT_CLAMP);
  CHECKBOX_fixedpoint     = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "force fixed point labels", &force_fixedpoint,  COLORLABEL_DIGITS,   GLUISliceBoundCB);
  CHECKBOX_exponential    = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "force exponential labels", &force_exponential, FORCE_EXPONENTIAL,   GLUISliceBoundCB);
  CHECKBOX_force_decimal  = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "force decimal in labels",  &force_decimal,     COLORLABEL_DIGITS,   GLUISliceBoundCB);
  CHECKBOX_force_zero_pad = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "force zero padding", &force_zero_pad,          COLORLABEL_ZERO_PAD, GLUISliceBoundCB);

  glui_bounds->add_column_to_panel(PANEL_cb11, false);

  PANEL_coloring = glui_bounds->add_panel_to_panel(PANEL_cb11, "Coloring");
  CHECKBOX_labels_shade = glui_bounds->add_checkbox_to_panel(PANEL_coloring, "Black/White (geometry)", &setbw, LABELS_shade, GLUILabelsCB);
  CHECKBOX_labels_shadedata = glui_bounds->add_checkbox_to_panel(PANEL_coloring, "Black/White (data)", &setbwdata, LABELS_shadedata, GLUILabelsCB);
  CHECKBOX_transparentflag = glui_bounds->add_checkbox_to_panel(PANEL_coloring, "Transparent (data)",
    &use_transparency_data, DATA_transparent, GLUISliceBoundCB);
  SPINNER_labels_transparency_data = glui_bounds->add_spinner_to_panel(PANEL_coloring, "level",
    GLUI_SPINNER_FLOAT, &transparent_level, TRANSPARENTLEVEL, GLUISliceBoundCB);
  SPINNER_labels_transparency_data->set_w(0);
  SPINNER_labels_transparency_data->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);
  CHECKBOX_use_lighting = glui_bounds->add_checkbox_to_panel(PANEL_coloring, "Lighting", &use_lighting, CB_USE_LIGHTING, GLUILabelsCB);

    PANEL_toggle_cba = glui_bounds->add_panel_to_panel(PANEL_cb11, "toggle colorbars");
    LISTBOX_cb_toggle_bound1 = glui_bounds->add_listbox_to_panel(PANEL_toggle_cba, "", &index_colorbar1, COLORBAR_LISTA, GLUIColorbarCB);
    GLUIUpdateColorbarListBound(2);
    LISTBOX_cb_toggle_bound1->set_int_val(index_colorbar1);

    LISTBOX_cb_toggle_bound2 = glui_bounds->add_listbox_to_panel(PANEL_toggle_cba, "", &index_colorbar2, COLORBAR_LISTB, GLUIColorbarCB);
    GLUIUpdateColorbarListBound(3);
    LISTBOX_cb_toggle_bound2->set_int_val(index_colorbar2);

    glui_bounds->add_button_to_panel(PANEL_toggle_cba, "toggle", COLORBAR_TOGGLE, GLUIColorbarCB);
    GLUIColorbarCB(COLORBAR_LISTA);
    GLUIColorbarCB(COLORBAR_LISTB);
    GLUIUpdateColorbarBound();
  PANEL_extreme = glui_bounds->add_panel_to_panel(PANEL_cb11, "Highlight extreme data");

  PANEL_extreme_min = glui_bounds->add_panel_to_panel(PANEL_extreme, "", GLUI_PANEL_NONE);
  CHECKBOX_show_extreme_mindata = glui_bounds->add_checkbox_to_panel(PANEL_extreme_min, "Color below min", &show_extreme_mindata, COLORBAR_EXTREME,
  GLUIExtremeCB);

  SPINNER_down_red   = glui_bounds->add_spinner_to_panel(PANEL_extreme_min, "red",   GLUI_SPINNER_INT, glui_down_rgb,   COLORBAR_EXTREME, GLUIExtremeCB);
  SPINNER_down_green = glui_bounds->add_spinner_to_panel(PANEL_extreme_min, "green", GLUI_SPINNER_INT, glui_down_rgb+1, COLORBAR_EXTREME, GLUIExtremeCB);
  SPINNER_down_blue  = glui_bounds->add_spinner_to_panel(PANEL_extreme_min, "blue",  GLUI_SPINNER_INT, glui_down_rgb+2, COLORBAR_EXTREME, GLUIExtremeCB);
  SPINNER_down_red->set_int_limits(0, 255);
  SPINNER_down_green->set_int_limits(0, 255);
  SPINNER_down_blue->set_int_limits(0, 255);

  glui_bounds->add_column_to_panel(PANEL_extreme, false);

  PANEL_extreme_max = glui_bounds->add_panel_to_panel(PANEL_extreme, "", GLUI_PANEL_NONE);

  CHECKBOX_show_extreme_maxdata = glui_bounds->add_checkbox_to_panel(PANEL_extreme_max, "Color above max", &show_extreme_maxdata, COLORBAR_EXTREME, GLUIExtremeCB);

  SPINNER_up_red   = glui_bounds->add_spinner_to_panel(PANEL_extreme_max, "red",   GLUI_SPINNER_INT, glui_up_rgb,   COLORBAR_EXTREME, GLUIExtremeCB);
  SPINNER_up_green = glui_bounds->add_spinner_to_panel(PANEL_extreme_max, "green", GLUI_SPINNER_INT, glui_up_rgb+1, COLORBAR_EXTREME, GLUIExtremeCB);
  SPINNER_up_blue  = glui_bounds->add_spinner_to_panel(PANEL_extreme_max, "blue",  GLUI_SPINNER_INT, glui_up_rgb+2, COLORBAR_EXTREME, GLUIExtremeCB);
  SPINNER_up_red->set_int_limits(0, 255);
  SPINNER_up_green->set_int_limits(0, 255);
  SPINNER_up_blue->set_int_limits(0, 255);

  if(use_data_extremes==0){
    CHECKBOX_show_extreme_maxdata->set_int_val(0);
    CHECKBOX_show_extreme_mindata->set_int_val(0);
    GLUIExtremeCB(COLORBAR_EXTREME);
  }
  GLUIColorbarGlobal2Local();

  ROLLOUT_split = glui_bounds->add_rollout_to_panel(ROLLOUT_coloring, "Modify split colorbar", false);
  PANEL_split1H = glui_bounds->add_panel_to_panel(ROLLOUT_split, "color above split");

  SPINNER_colorsplit[3] = glui_bounds->add_spinner_to_panel(PANEL_split1H, "red",   GLUI_SPINNER_INT, colorsplit+6, SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[4] = glui_bounds->add_spinner_to_panel(PANEL_split1H, "green", GLUI_SPINNER_INT, colorsplit+7, SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[5] = glui_bounds->add_spinner_to_panel(PANEL_split1H, "blue",  GLUI_SPINNER_INT, colorsplit+8, SPLIT_COLORBAR, GLUISplitCB);

  PANEL_split1L = glui_bounds->add_panel_to_panel(ROLLOUT_split, "color at min");

  SPINNER_colorsplit[0] = glui_bounds->add_spinner_to_panel(PANEL_split1L, "red",   GLUI_SPINNER_INT, colorsplit,   SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[1] = glui_bounds->add_spinner_to_panel(PANEL_split1L, "green", GLUI_SPINNER_INT, colorsplit+1, SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[2] = glui_bounds->add_spinner_to_panel(PANEL_split1L, "blue",  GLUI_SPINNER_INT, colorsplit+2, SPLIT_COLORBAR, GLUISplitCB);

  glui_bounds->add_column_to_panel(ROLLOUT_split, false);

  PANEL_split2H = glui_bounds->add_panel_to_panel(ROLLOUT_split, "color at max");

  SPINNER_colorsplit[9]  = glui_bounds->add_spinner_to_panel(PANEL_split2H, "red",   GLUI_SPINNER_INT, colorsplit+9,  SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[10] = glui_bounds->add_spinner_to_panel(PANEL_split2H, "green", GLUI_SPINNER_INT, colorsplit+10, SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[11] = glui_bounds->add_spinner_to_panel(PANEL_split2H, "blue",  GLUI_SPINNER_INT, colorsplit+11, SPLIT_COLORBAR, GLUISplitCB);

  PANEL_split2L = glui_bounds->add_panel_to_panel(ROLLOUT_split, "color below split");

  SPINNER_colorsplit[6] = glui_bounds->add_spinner_to_panel(PANEL_split2L, "red",   GLUI_SPINNER_INT, colorsplit+3, SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[7] = glui_bounds->add_spinner_to_panel(PANEL_split2L, "green", GLUI_SPINNER_INT, colorsplit+4, SPLIT_COLORBAR, GLUISplitCB);
  SPINNER_colorsplit[8] = glui_bounds->add_spinner_to_panel(PANEL_split2L, "blue",  GLUI_SPINNER_INT, colorsplit+5, SPLIT_COLORBAR, GLUISplitCB);

  glui_bounds->add_column_to_panel(ROLLOUT_split, false);

  PANEL_split3 = glui_bounds->add_panel_to_panel(ROLLOUT_split, "set split location");

  glui_bounds->add_spinner_to_panel(PANEL_split3, "max val",   GLUI_SPINNER_FLOAT, splitvals+2, SPLIT_COLORBAR, GLUISplitCB);
  glui_bounds->add_spinner_to_panel(PANEL_split3, "split val", GLUI_SPINNER_FLOAT, splitvals+1, SPLIT_COLORBAR, GLUISplitCB);
  glui_bounds->add_spinner_to_panel(PANEL_split3, "min val",   GLUI_SPINNER_FLOAT, splitvals,   SPLIT_COLORBAR, GLUISplitCB);
  glui_bounds->add_button_to_panel(ROLLOUT_split, "Show split colorbar", COLORBAR_SHOWSPLIT, GLUISplitCB);

  for(i = 0; i<12; i++){
    SPINNER_colorsplit[i]->set_int_limits(0, 255);
  }
  GLUISplitCB(SPLIT_COLORBAR);

  // ----------------------------------- Memory check ----------------------------------------

  ROLLOUT_memcheck = glui_bounds->add_rollout("Memory check",false,MEMCHECK_ROLLOUT,FileDataColRolloutCB);
  TOGGLE_ROLLOUT(filedatacolprocinfo, nfiledatacolprocinfo, ROLLOUT_memcheck, MEMCHECK_ROLLOUT, glui_bounds);

  SPINNER_max_mem_GB = glui_bounds->add_spinner_to_panel(ROLLOUT_memcheck, "max memory (0 unlimited) GB", GLUI_SPINNER_FLOAT, &max_mem_GB, MEMCHECK, MemcheckCB);
  MemcheckCB(MEMCHECK);

  glui_bounds->add_button("Save settings", SAVE_SETTINGS_BOUNDS, BoundsDlgCB);
#ifdef pp_CLOSEOFF
  GLUI_Button *BUTTON_button1=glui_bounds->add_button("Close", CLOSE_BOUNDS, BoundsDlgCB);
  BUTTON_button1->disable();
#else
  glui_bounds->add_button("Close", CLOSE_BOUNDS, BoundsDlgCB);
#endif

  glui_bounds->set_main_gfx_window( main_window );
}

/* ------------------ GLUICompressOnOff ------------------------ */

extern "C" void GLUICompressOnOff(int flag){
  switch(flag){
    case OFF:
      if(BUTTON_compress!=NULL)BUTTON_compress->disable();
      if(CHECKBOX_overwrite_all!=NULL)CHECKBOX_overwrite_all->disable();
      if(CHECKBOX_erase_all!=NULL)CHECKBOX_erase_all->disable();
      if(CHECKBOX_multi_task!=NULL)CHECKBOX_multi_task->disable();
      break;
    case ON:
      if(BUTTON_compress!=NULL)BUTTON_compress->enable();
      if(CHECKBOX_overwrite_all!=NULL)CHECKBOX_overwrite_all->enable();
      if(CHECKBOX_erase_all!=NULL)CHECKBOX_erase_all->enable();
      if(CHECKBOX_multi_task!=NULL)CHECKBOX_multi_task->enable();
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIPlot3DBoundCB ------------------------ */

extern "C" void GLUIPlot3DBoundCB(int var){
  int i;

  SNIFF_ERRORS("GLUIPlot3DBoundCB: start");
  switch(var){
  case PLOT3D_VECTORSKIP:
    if(vectorskip < 1){
      vectorskip = 1;
      if(SPINNER_plot3dvectorskip != NULL)SPINNER_slicevectorskip->set_int_val(vectorskip);
    }
    if(SPINNER_slicevectorskip!=NULL)SPINNER_slicevectorskip->set_int_val(vectorskip);
    break;
  case UPDATE_VECTOR_FROM_SMV:
    if(vecfactor<0.0)vecfactor = 0.0;
    if(SPINNER_vectorpointsize!=NULL&&SPINNER_vectorlinewidth!=NULL&&SPINNER_vectorlinelength!=NULL){
      SPINNER_vectorpointsize->set_float_val(vectorpointsize);
      SPINNER_vectorlinewidth->set_float_val(vectorlinewidth);
      SPINNER_vectorlinelength->set_float_val(vecfactor);
      if(SPINNER_plot3d_vectorlinelength!=NULL)SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
    }
    GLUIPlot3DBoundCB(UPDATE_VECTOR);
    break;
  case UPDATE_VECTOR:
    if(vecfactor<0.0){
      vecfactor = 0.0;
      if(SPINNER_vectorlinelength !=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
      if(SPINNER_plot3d_vectorlinelength!=NULL)SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
    }
    UpdatePlotSlice(XDIR);
    UpdatePlotSlice(YDIR);
    UpdatePlotSlice(ZDIR);
    break;
  case CHOPUPDATE:
    UpdateChopColors();
    break;
  case SETCHOPMINVAL:
    UpdateChopColors();
    break;
  case SETCHOPMAXVAL:
    UpdateChopColors();
    break;
  case CHOPVALMIN:
    p3chopmin[list_p3_index]=p3chopmin_temp;
    setp3chopmin[list_p3_index]=setp3chopmin_temp;

    UpdateChopColors();
    break;
  case CHOPVALMAX:
    p3chopmax[list_p3_index]=p3chopmax_temp;
    setp3chopmax[list_p3_index]=setp3chopmax_temp;

    UpdateChopColors();
    break;
  case PLOTISO:
    visiso = 1 - visiso;
    HandleIso();
    glutPostRedisplay();
    break;
  case PLOTISOTYPE:
    updatemenu=1;
    break;
  case UPDATEPLOT:
    UpdateRGBColors(colorbar_select_index);
    updatemenu=1;
    glutPostRedisplay();
    break;
  case FILETYPE_INDEX:
   p3min_all[list_p3_index_old]=glui_p3min;
   p3max_all[list_p3_index_old]=glui_p3max;
   setp3min_all[list_p3_index_old]=glui_setp3min;
   setp3max_all[list_p3_index_old]=glui_setp3max;
   p3chopmin[list_p3_index_old]=p3chopmin_temp;
   p3chopmax[list_p3_index_old]=p3chopmax_temp;
   setp3chopmin[list_p3_index_old]=setp3chopmin_temp;
   setp3chopmax[list_p3_index_old]=setp3chopmax_temp;

   glui_p3min=p3min_all[list_p3_index];
   glui_p3max=p3max_all[list_p3_index];
   glui_setp3min=setp3min_all[list_p3_index];
   glui_setp3max=setp3max_all[list_p3_index];
   p3chopmin_temp=p3chopmin[list_p3_index];
   p3chopmax_temp=p3chopmax[list_p3_index];
   setp3chopmin_temp=setp3chopmin[list_p3_index];
   setp3chopmax_temp=setp3chopmax[list_p3_index];
   if(global_scase.plot3dinfo!=NULL){
     plot3dmin_unit = (unsigned char *)global_scase.plot3dinfo->label[list_p3_index].unit;
     plot3dmax_unit = plot3dmin_unit;
   }

   list_p3_index_old=list_p3_index;
   GLUIPlot3DBoundCB(SETCHOPMINVAL);
   GLUIPlot3DBoundCB(SETCHOPMAXVAL);
   GLUIPlot3DBoundCB(SETVALMIN);
   GLUIPlot3DBoundCB(SETVALMAX);
   break;
  case SETVALMIN:
   switch(glui_setp3min){
    case PERCENTILE_MIN:
      break;
    case GLOBAL_MIN:
      GetLoadedPlot3dBounds(NULL, p3min_loaded, p3max_loaded);
      glui_p3min = p3min_loaded[list_p3_index];
      break;
    case SET_MIN:
    case CHOP_MIN:
      break;
    default:
      assert(FFALSE);
      break;
   }
   break;
  case SETVALMAX:
     switch(glui_setp3max){
      case PERCENTILE_MAX:
        break;
      case GLOBAL_MAX:
        GetLoadedPlot3dBounds(NULL, p3min_loaded, p3max_loaded);
        glui_p3max = p3max_loaded[list_p3_index];
        break;
      case SET_MAX:
      case CHOP_MAX:
        break;
      default:
        assert(FFALSE);
        break;
     }
   break;
  case CACHE_DATA:
    if(PANEL_keep_plot3d_data !=NULL){
      if(cache_plot3d_data==0){
        PANEL_keep_plot3d_data->disable();
      }
      else{
        PANEL_keep_plot3d_data->enable();
      }
    }
    break;
  case FILE_UPDATE:
   p3min_all[list_p3_index] =    glui_p3min;
   p3max_all[list_p3_index] =    glui_p3max;
   setp3min_all[list_p3_index] = glui_setp3min;
   setp3max_all[list_p3_index] = glui_setp3max;
   break;
  case UPDATE_DATA_COLORS:
    GLUIPlot3DBoundCB(FILE_UPDATE);
    UpdateAllPlot3DColors(1);
    break;
  case FILE_RELOAD:
   GLUIPlot3DBoundCB(FILE_UPDATE);
   for(i=0;i<global_scase.nplot3dinfo;i++){
     if(global_scase.plot3dinfo[i].loaded==0)continue;
     LoadPlot3dMenu(i);
   }
   break;
  case VALMIN:
    break;
  case VALMAX:
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateTracers ------------------------ */

extern "C" void GLUIUpdateTracers(void){
  if(CHECKBOX_showtracer==NULL)return;
  CHECKBOX_showtracer->set_int_val(show_tracers_always);
}


/* ------------------ GLUIUpdateIsotype ------------------------ */

extern "C" void GLUIUpdateIsotype(void){
  CHECKBOX_show_iso_shaded->set_int_val(visAIso&1);
  CHECKBOX_show_iso_outline->set_int_val((visAIso&2)/2);
  CHECKBOX_show_iso_points->set_int_val((visAIso&4)/4);
}


/* ------------------ GLUIUpdatePlot3Dtype ------------------------ */


extern "C" void GLUIUpdatePlot3Dtype(void){
  RADIO_plot3d_isotype->set_int_val(p3dsurfacetype);
}
/* ------------------ GLUIUpdateChar ------------------------ */

extern "C" void GLUIUpdateChar(void){
  if(CHECKBOX_showchar==NULL)return;
  if(canshow_threshold==1){
    CHECKBOX_showchar->enable();
  }
  else{
    CHECKBOX_showchar->disable();
  }
  CHECKBOX_showchar->set_int_val(vis_threshold);
  BoundBoundCB(SHOWCHAR);
}

/* ------------------ GLUIUpdatePlot3dListIndex ------------------------ */

extern "C" void GLUIUpdatePlot3dListIndex(void){
  GLUISetValTypeIndex(BOUND_PLOT3D, plotn-1);
}

/* ------------------ GLUIIsoBoundCB ------------------------ */

extern "C" void GLUIIsoBoundCB(int var){
  int i;
  float *iso_color;
  boundsdata *sb;

  switch(var){
    case ISO_SETVALMIN:
    case ISO_SETVALMAX:
    case ISO_VALMIN:
    case ISO_VALMAX:
      if(iisottype<0)return;
      sb = isobounds + iisottype;
      break;
    default:
      break;
  }
  switch(var){
  case ISO_OUTLINE_IOFFSET:
    iso_outline_offset = (float)iso_outline_ioffset/1000.0;
  break;
  case ISO_COLORBAR_LIST:
    iso_colorbar = colorbars.colorbarinfo + colorbars.iso_colorbar_index;
    ColorbarMenu(colorbars.iso_colorbar_index);
    updatemenu = 1;
    update_texturebar = 1;
    break;
  case ISO_TRANSPARENCY_OPTION:
    switch(iso_transparency_option){
      case ISO_TRANSPARENT_CONSTANT:
        use_transparency_data=1;
        iso_opacity_change=0;
        break;
      case ISO_TRANSPARENT_VARYING:
        use_transparency_data=1;
        iso_opacity_change=1;
        break;
      case ISO_OPAQUE:
        use_transparency_data=0;
        iso_opacity_change=1;
        break;
      default:
	assert(FFALSE);
	break;
    }
    GLUISliceBoundCB(DATA_transparent);
    break;
  case COLORTABLE_LIST:
    if(i_colortable_list>=0){
      colortabledata *cti;

      cti = colortableinfo+i_colortable_list;
      glui_iso_colors[0] = cti->color[0];
      glui_iso_colors[1] = cti->color[1];
      glui_iso_colors[2] = cti->color[2];
      glui_iso_colors[3] = cti->color[3];
      GLUIIsoBoundCB(ISO_COLORS);
      if(SPINNER_iso_colors[0]!=NULL)SPINNER_iso_colors[0]->set_int_val(glui_iso_colors[0]);
      if(SPINNER_iso_colors[1]!=NULL)SPINNER_iso_colors[1]->set_int_val(glui_iso_colors[1]);
      if(SPINNER_iso_colors[2]!=NULL)SPINNER_iso_colors[2]->set_int_val(glui_iso_colors[2]);
      if(SPINNER_iso_colors[3]!=NULL)SPINNER_iso_colors[3]->set_int_val(glui_iso_colors[3]);
    }
    break;

  case ISO_LEVEL:
    iso_color = iso_colors+4*(glui_iso_level-1);
    glui_iso_colors[0] = CLAMP(255*iso_color[0]+0.1, 0, 255);
    glui_iso_colors[1] = CLAMP(255*iso_color[1]+0.1, 0, 255);
    glui_iso_colors[2] = CLAMP(255*iso_color[2]+0.1, 0, 255);
    glui_iso_colors[3] = CLAMP(255*iso_color[3]+0.1, 1, 255);

    if(SPINNER_iso_colors[0]!=NULL)SPINNER_iso_colors[0]->set_int_val(glui_iso_colors[0]);
    if(SPINNER_iso_colors[1]!=NULL)SPINNER_iso_colors[1]->set_int_val(glui_iso_colors[1]);
    if(SPINNER_iso_colors[2]!=NULL)SPINNER_iso_colors[2]->set_int_val(glui_iso_colors[2]);
    if(SPINNER_iso_colors[3]!=NULL)SPINNER_iso_colors[3]->set_int_val(glui_iso_colors[3]);
    if(LIST_colortable != NULL){
      i_colortable_list = CLAMP(GetColorTableIndex(glui_iso_colors), -1, ncolortableinfo - 1);
      LIST_colortable->set_int_val(i_colortable_list);
    }
    break;
  case GLOBAL_ALPHA:
    for(i = 0; i < MAX_ISO_COLORS; i++){
      iso_colors[4*i + 3] = iso_transparency;
    }
    if(SPINNER_iso_colors[3]!=NULL)SPINNER_iso_colors[3]->set_int_val(glui_iso_transparency);
    GLUIIsoBoundCB(ISO_COLORS);
    break;
  case ISO_TRANSPARENCY:
    if(glui_iso_transparency < 0 || glui_iso_transparency > 255) {
      glui_iso_transparency = CLAMP(glui_iso_transparency, 0, 255);
      SPINNER_iso_transparency->set_int_val(glui_iso_transparency);
    }
    iso_transparency = CLAMP(((float)glui_iso_transparency + 0.1) / 255.0,0.0,1.0);
    break;
  case ISO_COLORS:
    iso_color = iso_colors+4*(glui_iso_level-1);
    iso_color[0] = ((float)glui_iso_colors[0]+0.1)/255.0;
    iso_color[1] = ((float)glui_iso_colors[1]+0.1)/255.0;
    iso_color[2] = ((float)glui_iso_colors[2]+0.1)/255.0;
    iso_color[3] = ((float)glui_iso_colors[3]+0.1)/255.0;

    for(i = 0; i < MAX_ISO_COLORS; i++){
      float graylevel;

      graylevel = TOBW(iso_colors+4*i);
      iso_colorsbw[4*i + 0] = graylevel;
      iso_colorsbw[4*i + 1] = graylevel;
      iso_colorsbw[4*i + 2] = graylevel;
      iso_colorsbw[4*i + 3] = iso_colors[4*i + 3];
    }
    UpdateIsoColors();
    if(LIST_colortable!=NULL){
      i_colortable_list = CLAMP(GetColorTableIndex(glui_iso_colors), -1, ncolortableinfo-1);
      LIST_colortable->set_int_val(i_colortable_list);
    }
    break;
  case ISO_SURFACE:
  case  ISO_OUTLINE:
  case ISO_POINTS:
    visAIso= 1*show_iso_shaded + 2*show_iso_outline + 4*show_iso_points;
    updatemenu=1;
    break;

  case ISO_SETVALMIN:
    switch(setisomin){
      case SET_MIN:
        if(sb->edit_valmin_defined==1)glui_iso_valmin = sb->edit_valmin;
        iso_valmin=glui_iso_valmin;
        EDIT_iso_valmin->enable();
        break;
      case PERCENTILE_MIN:
        EDIT_iso_valmin->disable();
        break;
      case GLOBAL_MIN:
        iso_valmin = iso_global_min;
        EDIT_iso_valmin->disable();
        break;
      default:
        assert(FFALSE);
        break;
    }
    sb->dlg_setvalmin = setisomin;
    glui_iso_valmin=iso_valmin;
    EDIT_iso_valmin->set_float_val(glui_iso_valmin);
    glutPostRedisplay();
    break;

  case ISO_SETVALMAX:
    switch(setisomax){
      case SET_MAX:
        if(sb->edit_valmax_defined==1)glui_iso_valmax = sb->edit_valmax;
        iso_valmax=glui_iso_valmax;
        EDIT_iso_valmax->enable();
        break;
      case PERCENTILE_MAX:
        EDIT_iso_valmax->disable();
        break;
      case GLOBAL_MAX:
        iso_valmax = iso_global_max;
        EDIT_iso_valmax->disable();
        break;
      default:
        assert(FFALSE);
        break;
    }
    sb->dlg_setvalmax = setisomax;
    glui_iso_valmax = iso_valmax;
    EDIT_iso_valmax->set_float_val(glui_iso_valmax);
    glutPostRedisplay();
    break;
  case ISO_VALMIN:
    if(setisomin==1){
      sb->edit_valmin = glui_iso_valmin;
      sb->edit_valmin_defined = 1;
    }
    iso_valmin=glui_iso_valmin;
    sb->dlg_valmin = iso_valmin;
    glutPostRedisplay();
    break;
  case ISO_VALMAX:
    if(setisomax==1){
      sb->edit_valmax = glui_iso_valmax;
      sb->edit_valmax_defined = 1;
    }
    iso_valmax=glui_iso_valmax;
    sb->dlg_valmax = iso_valmax;
    glutPostRedisplay();
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIAddScriptList ------------------------ */

extern "C" void GLUIAddScriptList(char *file, int id){
  if(file!=NULL&&strlen(file)>0&&LIST_scriptlist!=NULL){
    LIST_scriptlist->add_item(id,file);
  }
}

/* ------------------ GLUIScriptEnable ------------------------ */

extern "C" void GLUIScriptEnable(void){
    BUTTON_script_start->enable();
    BUTTON_script_stop->enable();
    BUTTON_script_runscript->enable();
    LIST_scriptlist->enable();
    BUTTON_script_saveini->enable();
    BUTTON_script_setsuffix->enable();
    EDIT_ini->enable();
  }

/* ------------------ GLUIScriptDisable ------------------------ */


extern "C"  void GLUIScriptDisable(void){
    BUTTON_script_start->disable();
    BUTTON_script_stop->disable();
    BUTTON_script_runscript->disable();
    LIST_scriptlist->disable();
    BUTTON_script_saveini->disable();
    BUTTON_script_setsuffix->disable();
    EDIT_ini->disable();
  }

/* ------------------ GLUIUpdateBoundaryListIndex ------------------------ */

extern "C" void GLUIUpdateBoundaryListIndex(int patchfilenum){
  int i;
  if(RADIO_bf==NULL)return;
  for(i=0;i<npatch2;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + patchfilenum;
    if(strcmp(patchlabellist[i],patchi->label.shortlabel)==0){
      if(RADIO_bf == NULL){
        list_patch_index = i;
      }
      else{
        RADIO_bf->set_int_val(i);
      }
      list_patch_index_old=list_patch_index;
      Global2GLUIBoundaryBounds(patchlabellist[i]);
      return;
    }
  }
}

/* ------------------ GLUIUpdateStreakValue ------------------------ */

extern "C" void GLUIUpdateStreakValue(float rvalue){
  float_streak5value=rvalue;
  if(SPINNER_partstreaklength!=NULL){
    SPINNER_partstreaklength->set_float_val(rvalue);
    SPINNER_partstreaklength->set_float_limits(0.0,tmax_part);
  }
}

/* ------------------ GLUIIncrementPartPropIndex ------------------------ */

extern "C" void GLUIIncrementPartPropIndex(void){
  if(npart5prop>0){
    ipart5prop++;
    if(ipart5prop>npart5prop-1)ipart5prop = 0;
    GLUISetValTypeIndex(BOUND_PART, ipart5prop);
    ParticlePropShowMenu(ipart5prop);
  }
}

/* ------------------ PartBoundCB ------------------------ */

void PartBoundCB(int var){
  partpropdata *prop_new, *prop_old;

  SNIFF_ERRORS("PartBoundCB: start");
  prop_new = part5propinfo + ipart5prop;
  prop_old = part5propinfo + ipart5prop_old;
  if(part5propinfo==NULL)return;
  switch(var){
  case VALMIN:
    if(setpartmin==SET_MIN)prop_new->user_min = glui_partmin;
    break;
  case VALMAX:
    if(setpartmax==SET_MAX)prop_new->user_max = glui_partmax;
    break;
  case FILETYPE_INDEX:

    // save data from controls

    prop_old->setvalmin=setpartmin;
    prop_old->setvalmax=setpartmax;
    if(setpartmin==SET_MIN){
      prop_old->user_min = glui_partmin;
    }
    if(setpartmax==SET_MAX){
      prop_old->user_max = glui_partmax;
    }
    prop_old->setchopmin=setpartchopmin;
    prop_old->setchopmax=setpartchopmax;
    prop_old->chopmin=partchopmin;
    prop_old->chopmax=partchopmax;

    // copy data to controls

    setpartmin=prop_new->setvalmin;
    if(setpartmin==PERCENTILE_MIN)setpartmin = GLOBAL_MIN;

    setpartmax = prop_new->setvalmax;
    if(setpartmax==PERCENTILE_MAX)setpartmax = GLOBAL_MAX;

    PartBoundCB(SETVALMIN);
    PartBoundCB(SETVALMAX);
    if(ipart5prop>0&&glui_partmin>glui_partmax){
      glui_partmin = part5propinfo[ipart5prop].dlg_global_valmin;
      glui_partmax = part5propinfo[ipart5prop].dlg_global_valmax;
      part5propinfo[ipart5prop].user_min = glui_partmin;
      part5propinfo[ipart5prop].user_max = glui_partmax;
    }

    setpartchopmin=prop_new->setchopmin;
    setpartchopmax=prop_new->setchopmax;
    partchopmin=prop_new->chopmin;
    partchopmax=prop_new->chopmax;

    partmin_unit = (unsigned char *)prop_new->label->unit;
    partmax_unit = partmin_unit;

    // update controls

    ipart5prop_old = ipart5prop;

    break;
  case STREAKLENGTH:
    UpdateStreakValue(float_streak5value-0.001);
    if(float_streak5value==0.0){
      SetStreakShow(0);
    }
    else{
      SetStreakShow(1);
    }
    updatemenu=1;
    break;
  case PARTSKIP:
    if(partdrawskip < 1){
      partdrawskip = 1;
      if(SPINNER_partdrawskip!=NULL)SPINNER_partdrawskip->set_float_val(partdrawskip);
    }
    break;
  case TRACERS:
  case PARTFAST:
    if(global_scase.npartinfo<=1){
      CHECKBOX_use_partload_threads->disable();
      SPINNER_n_part_threads->disable();
      use_partload_threads = 0;
      CHECKBOX_use_partload_threads->set_int_val(use_partload_threads);
    }
    else{
      CHECKBOX_use_partload_threads->set_int_val(use_partload_threads);
    }
    updatemenu=1;
    break;
  case CHOPUPDATE:
    UpdateChopColors();
    break;
  case SETCHOPMINVAL:
    prop_new->setchopmin=setpartchopmin;
    prop_new->chopmin=partchopmin;
    UpdateChopColors();
    break;
  case SETCHOPMAXVAL:
    prop_new->setchopmax=setpartchopmax;
    prop_new->chopmax=partchopmax;
    UpdateChopColors();
    break;
  case CHOPVALMIN:
    prop_new->setchopmin=setpartchopmin;
    prop_new->chopmin=partchopmin;
    UpdateChopColors();
    break;
  case CHOPVALMAX:
    prop_new->setchopmax=setpartchopmax;
    prop_new->chopmax=partchopmax;
    UpdateChopColors();
    break;
  case SETVALMIN:
    if(setpartmin_old==SET_MIN){
      if(prop_old!=NULL)prop_old->user_min = glui_partmin;
    }
    setpartmin_old=setpartmin;
    if(prop_new!=NULL)prop_new->setvalmin = setpartmin;
    if(prop_new!=NULL)prop_new->valmin= glui_partmin;
   break;
  case SETVALMAX:
    if(setpartmax_old==SET_MAX){
      if(prop_old!=NULL)prop_old->user_max= glui_partmax;
    }
    setpartmax_old=setpartmax;
    if(prop_new!=NULL)prop_new->setvalmax = setpartmax;
    if(prop_new!=NULL)prop_new->valmax = glui_partmax;
   break;
  case FILE_RELOAD:

    if(research_mode==1&&npartloaded>0){
      setpartmin = GLOBAL_MIN;
      PartBoundCB(SETVALMIN);

      setpartmax = GLOBAL_MAX;
      PartBoundCB(SETVALMAX);
    }

    {
      int prop_index_SAVE;

     prop_index_SAVE= global_prop_index;
     PartBoundCB(FILETYPE_INDEX);
     LoadParticleMenu(PARTFILE_RELOADALL);
     ParticlePropShowMenu(prop_index_SAVE);
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ SliceBounds2Glui ------------------------ */

void SliceBounds2Glui(int slicetype){
  if(slicetype >= 0 && slicetype < nslicebounds){
    slice_line_contour_min = slicebounds[slicetype].line_contour_min;
    slice_line_contour_max = slicebounds[slicetype].line_contour_max;
    slice_line_contour_num = slicebounds[slicetype].line_contour_num;
    glui_slicemin          = slicebounds[slicetype].dlg_valmin;
    glui_slicemax          = slicebounds[slicetype].dlg_valmax;
    glui_slicechopmin = slicebounds[slicetype].chopmin;
    glui_slicechopmax = slicebounds[slicetype].chopmax;
    glui_setslicechopmin = slicebounds[slicetype].setchopmin;
    glui_setslicechopmax = slicebounds[slicetype].setchopmax;
    glui_slicemin_unit = (unsigned char*)slicebounds[slicetype].label->unit;
    glui_slicemax_unit = glui_slicemin_unit;
  }
}

/* ------------------ GLUISliceBoundCB ------------------------ */

extern "C" void GLUISliceBoundCB(int var){
  int error,i;
  int ii;
  slicedata *sd;
  int last_slice;
  int use_slice;

  SNIFF_ERRORS("GLUISliceBoundCB: start");
  updatemenu=1;
  switch(var){
    case COLORBAR_LIST2_NEXT:
    case COLORBAR_LIST2_PREV:
      colorbartype = colorbar_list_inverse[colorbartype];
      if(var==COLORBAR_LIST2_NEXT)colorbartype++;
      if(var==COLORBAR_LIST2_PREV)colorbartype--;
      if(colorbartype<0)colorbartype= max_LISTBOX_cb_bound;
      if(colorbartype> max_LISTBOX_cb_bound)colorbartype=0;
      colorbartype = colorbar_list_sorted[colorbartype];
      LISTBOX_cb_bound->set_int_val(colorbartype);
      GLUISliceBoundCB(COLORBAR_LIST2);
      break;
    case SLICE_SIZE:
      update_avg = 1;
      GLUIDeviceCB(DEVICE_TIMEAVERAGE);
      if(plot2d_size_factor<0){
        plot2d_size_factor = 0.0;
        GLUIUpdatePlot2DSize2();
      }
      GLUIUpdatePlot2DSize();
      break;
    case COLORBAR_PLOT2D:
      if(vis_colorbar_dists_plot==1&&vis_slice_plot == 1){
        vis_slice_plot = 0;
        CHECKBOX_vis_slice_plot->set_int_val(vis_slice_plot);
      }
      break;
    case SLICE_PLOT2D:
      if(vis_colorbar_dists_plot==1&&vis_slice_plot == 1){
        vis_colorbar_dists_plot = 0;
        CHECKBOX_cb_plot_dist->set_int_val(vis_colorbar_dists_plot);
      }
      Slice2Device();
      break;
    case SLICE_DPLOT:
      if(slice_dxyz[0]<0.0){
        slice_dxyz[0] = 0.0;
        if(SPINNER_slice_dx!=NULL)SPINNER_slice_dx->set_float_val(0.0);
      }
      if(slice_dxyz[1]<0.0){
        slice_dxyz[1] = 0.0;
        if(SPINNER_slice_dy!=NULL)SPINNER_slice_dy->set_float_val(0.0);
      }
      if(slice_dxyz[2]<0.0){
        slice_dxyz[2] = 0.0;
        if(SPINNER_slice_dz!=NULL)SPINNER_slice_dz->set_float_val(0.0);
      }
      Slice2Device();
      break;
    case SLICE_PLOT_CSV:
      GLUISliceBoundCB(SLICE_PLOT_FILENAME);
      if(strlen(slice_plot_filename) > 0)slice_plot_csv = 1;
      break;
    case SLICE_PLOT_FILENAME:
      use_slice = 0;
      strcpy(slice_plot_filename, global_scase.fdsprefix);
      strcat(slice_plot_filename, "_slice");
      for(i = 0; i < global_scase.slicecoll.nsliceinfo; i++){
        slicedata *slicei;
        devicedata *devicei;

        slicei = global_scase.slicecoll.sliceinfo + i;
        devicei = &(slicei->vals2d);
        if(slicei->loaded == 0 || devicei->valid == 0)continue;
        strcat(slice_plot_filename, "_");
        strcat(slice_plot_filename, slicei->label.shortlabel);
        strcat(slice_plot_filename, ".csv");
        use_slice = 1;
        break;
      }
      if(use_slice == 0){
        strcpy(slice_plot_filename, "");
      }
      break;
    case SLICE_PLOT_LABEL:
      GLUISliceBoundCB(SLICE_PLOT_FILENAME);
      strcpy(slice_plot_label, "Output data");
      if(strlen(slice_plot_filename)>0){
        strcat(slice_plot_label, " to ");
        strcat(slice_plot_label, slice_plot_filename);
      }
      if(BUTTON_OUTPUT_PLOT2D!=NULL)BUTTON_OUTPUT_PLOT2D->set_name(slice_plot_label);
      break;
    case UPDATE_HISTOGRAM:
    case FRAMES_HISTOGRAM:
    case INIT_HISTOGRAM:
      break;
    case DATA_transparent:
      GLUIUpdateTransparency();
      UpdateChopColors();
      UpdateIsoControls();
      break;
    case COLORBAR_EXTREME2:
      GLUIUpdateExtreme();
      break;
    case UPDATE_SLICEDUPS:
    updatemenu = 1;
    break;
    case SLICE_VECTORSKIP:
      if(vectorskip < 1){
        vectorskip = 1;
        if(SPINNER_slicevectorskip != NULL)SPINNER_slicevectorskip->set_int_val(vectorskip);
      }
      if(SPINNER_plot3dvectorskip!=NULL)SPINNER_plot3dvectorskip->set_int_val(vectorskip);
      update_vectorskip = 1;
      break;
    case ZONEVALMINMAX:
      GetZoneColors(zonetu, nzonetotal, izonetu, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      GetZoneColors(zonetl, nzonetotal, izonetl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zonefl==1)GetZoneColors(zonefl, nzonetotal, izonefl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zonelw==1)GetZoneColors(zonelw, nzonetotal, izonelw, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zoneuw==1)GetZoneColors(zoneuw, nzonetotal, izoneuw, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zonecl==1)GetZoneColors(zonecl, nzonetotal, izonecl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      if(have_target_data==1)GetZoneColors(zonetargets, nzonetotal_targets, izonetargets, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      zoneusermin=zonemin;
      zoneusermax=zonemax;
      break;
    case SETZONEVALMINMAX:
      if(setzonemin==SET_MIN){
        zonemin=zoneusermin;
        EDIT_zone_min->set_float_val(zonemin);
      }
      else{
        EDIT_zone_min->set_float_val(zoneglobalmin);
      }
      if(setzonemax==SET_MAX){
        zonemax = zoneusermax;
        EDIT_zone_max->set_float_val(zonemax);
      }
      else{
        EDIT_zone_max->set_float_val(zoneglobalmax);
      }
      GetZoneColors(zonetu, nzonetotal, izonetu,zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      GetZoneColors(zonetl, nzonetotal, izonetl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
      break;
    case COLORBAR_LIST2:
      int list_index;

      list_index = LISTBOX_cb_bound->get_int_val();
      if(list_index<0)break;
      colorbartype = list_index;
      GLUISetColorbarListEdit(colorbartype);
      ColorbarMenu(colorbartype);
      GLUIColorbarGlobal2Local();
      if(colorbartype == colorbars.bw_colorbar_index&&colorbars.bw_colorbar_index>=0){
        setbwdata = 1;
        ColorbarMenu(colorbars.bw_colorbar_index);
      }
      else{
        setbwdata = 0;
      }
      GLUIIsoBoundCB(ISO_COLORS);
      GLUISetLabelControls();
      break;
    case RESEARCH_MODE:
      SetResearchMode(research_mode);
      GLUIHVACDuctBoundsCPP_CB(BOUND_UPDATE_COLORS);
      GLUIHVACNodeBoundsCPP_CB(BOUND_UPDATE_COLORS);
      GLUIHVACSliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
      GLUIPatchBoundsCPP_CB(BOUND_UPDATE_COLORS);
      GLUIPartBoundsCPP_CB(BOUND_UPDATE_COLORS);
      if(nplot3dloaded>0)GLUIPlot3DBoundsCPP_CB(BOUND_UPDATE_COLORS);
      if(research_mode==1)PRINTF("\nresearch mode on, using global bounds\n\n");
      if(research_mode==0)PRINTF("research mode off\n");
      GLUISliceBoundCB(FILE_UPDATE);
      break;
    case ISO_BOUNDS_OUTPUT:
      OutputAllIsoBounds();
      break;
    case SMOOTH_SURFACES:
      CHECKBOX_smooth2->set_int_val(smooth_iso_normal);
      break;
    case SORT_SURFACES:
      sort_geometry=sort_iso_triangles;
      for(i=global_scase.surfcoll.nsurfinfo;i<global_scase.surfcoll.nsurfinfo+MAX_ISO_COLORS+1;i++){
        surfdata *surfi;

        surfi = global_scase.surfcoll.surfinfo + i;
        surfi->transparent_level=transparent_level;
      }
      CHECKBOX_sort2->set_int_val(sort_iso_triangles);
      GLUIIsoBoundCB(GLOBAL_ALPHA);
      break;
    case COLORINDEX:
      HandleColorbarIndex(colorbar_select_index);
      break;
    case COLORBAND:
      UpdateRGBColors(colorbar_select_index);
      if(colorbar_select_index == -1){
        HandleColorbarIndex(colorbar_select_index);
      }
      break;
    case FORCE_EXPONENTIAL:
      if(force_exponential==1&&force_fixedpoint==1)force_fixedpoint=0;
      updatemenu = 1;
      update_colorbar_digits = 1;
      break;
    case COLORLABEL_DIGITS:
      if(force_exponential==1&&force_fixedpoint==1)force_exponential=0;
      if(force_decimal==0){
        force_zero_pad = 0;
        CHECKBOX_force_zero_pad->set_int_val(force_zero_pad);
      }
      if(force_exponential == 1){
        force_zero_pad = 0;
        force_decimal = 1;
        CHECKBOX_force_zero_pad->set_int_val(force_zero_pad);
        CHECKBOX_force_decimal->set_int_val(force_decimal);
      }
      updatemenu = 1;
      update_colorbar_digits = 1;
      break;
    case COLORLABEL_ZERO_PAD:
      if(force_zero_pad == 1){
        force_decimal = 1;
        CHECKBOX_force_decimal->set_int_val(force_decimal);
      }
      GLUISliceBoundCB(COLORLABEL_DIGITS);
      break;
    case SLICE_OPTION:
      updatemenu = 1;
      break;
    case SLICE_SKIP:
      slice_skip = CLAMP(slice_skip,1,max_slice_skip);
      if(SPINNER_slice_skip!=NULL)SPINNER_slice_skip->set_int_val(slice_skip);
      if(SPINNER_slice_skip2!=NULL)SPINNER_slice_skip2->set_int_val(slice_skip);
      slice_skipx = slice_skip;
      slice_skipy = slice_skip;
      slice_skipz = slice_skip;
      break;
    case SORTSLICES_DEBUG:
      if(sortslices_debug == 1 && sortslices == 0){
        sortslices = 1;
        CHECKBOX_sortslices->set_int_val(sortslices);
      }
      GLUTPOSTREDISPLAY;
      updatemenu = 1;
      break;
    case SORTSLICES:
      if(sortslices_debug == 1 && sortslices == 0){
        sortslices_debug = 0;
        CHECKBOX_sortslices_debug->set_int_val(sortslices_debug);
      }
      GLUTPOSTREDISPLAY;
      updatemenu = 1;
      break;
    case TRANSPARENTLEVEL:
      for(i=global_scase.surfcoll.nsurfinfo;i<global_scase.surfcoll.nsurfinfo+MAX_ISO_COLORS+1;i++){
        surfdata *surfi;

        surfi = global_scase.surfcoll.surfinfo + i;
        surfi->transparent_level=transparent_level;
      }
      UpdateRGBColors(colorbar_select_index);
      if(SPINNER_transparent_level!=NULL)SPINNER_transparent_level->set_float_val(transparent_level);
      if(SPINNER_labels_transparency_data != NULL)SPINNER_labels_transparency_data->set_float_val(transparent_level);
      if(transparent_level > 0.999){
        CHECKBOX_transparentflag->set_int_val(0);
      }
      else{
        CHECKBOX_transparentflag->set_int_val(1);
      }
      GLUTPOSTREDISPLAY;
      break;
    case LINE_CONTOUR_VALUE:
      if(slice_line_contour_num<1){
        slice_line_contour_num=1;
        SPINNER_line_contour_num->set_int_val(slice_line_contour_num);
      }
      if(slice_line_contour_num==1&&slice_line_contour_min!=slice_line_contour_max){
        slice_line_contour_max=slice_line_contour_min;
        SPINNER_line_contour_max->set_float_val(slice_line_contour_max);
      }
      slicebounds[list_slice_index].line_contour_min=slice_line_contour_min;
      slicebounds[list_slice_index].line_contour_max=slice_line_contour_max;
      slicebounds[list_slice_index].line_contour_num=slice_line_contour_num;
      break;
    case UPDATE_LINE_CONTOUR_VALUE:
      UpdateSliceContours(list_slice_index,slice_line_contour_min, slice_line_contour_max,slice_line_contour_num);
      break;
  case UPDATE_VECTOR_FROM_SMV:
    if(vecfactor<0.0)vecfactor = 0.0;
    if(SPINNER_plot3d_vectorpointsize!=NULL&&SPINNER_plot3d_vectorlinewidth!=NULL&&SPINNER_plot3d_vectorlinelength!=NULL){
      SPINNER_plot3d_vectorpointsize->set_float_val(vectorpointsize);
      SPINNER_plot3d_vectorlinewidth->set_float_val(vectorlinewidth);
      if(SPINNER_vectorlinelength !=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
      SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
    }
    GLUISliceBoundCB(UPDATE_VECTOR);
    break;
  case VEC_UNIFORM_SPACING:
    update_vectorskip = 1;
    break;
  case UPDATE_VECTOR:
    if(vecfactor<0.0){
      vecfactor = 0.0;
      if(SPINNER_vectorlinelength !=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
      if(SPINNER_plot3d_vectorlinelength!=NULL)SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
    }
    break;
  case FRAMELOADING:
    tload_zipstep = tload_zipskip + 1;
    if(use_tload_skip==0){
      tload_step = 1;
    }
    else{
      tload_step = tload_skip + 1;
    }
    updatemenu=1;
    break;
  case CHOPUPDATE:
    UpdateChopColors();
    break;
  case SETCHOPMINVAL:
    UpdateChopColors();
    break;
  case SETCHOPMAXVAL:
    UpdateChopColors();
    break;
  case CHOPVALMIN:
    UpdateChopColors();
    break;
  case CHOPVALMAX:
    UpdateChopColors();
    break;
  case SETVALMIN:
  case SETVALMAX:
  case VALMIN:
  case VALMAX:
    break;
  case FILETYPE_INDEX:
    SliceBounds2Glui(list_slice_index);
    SPINNER_line_contour_min->set_float_val(slice_line_contour_min);
    SPINNER_line_contour_max->set_float_val(slice_line_contour_max);
    SPINNER_line_contour_num->set_int_val(slice_line_contour_num);
    if(ROLLOUT_zone_bound!=NULL){
      int slice_index;

      if(RADIO_slice!=NULL){
        slice_index = RADIO_slice->get_int_val();
        if(strcmp(slicebounds[slice_index].shortlabel, "TEMP")==0){
          BoundRolloutCB(ZONE_ROLLOUT);
        }
      }
    }
    break;
  case SET_GLOBAL_BOUNDS:
      GLUISliceBoundCB(SETVALMIN);

      GLUISliceBoundCB(SETVALMAX);
    break;
  case UPDATE_DATA_COLORS:
  case FILE_UPDATE:
    if(research_mode==1){
      GLUISliceBoundCB(SET_GLOBAL_BOUNDS);
    }
    slice_fileupdate++;
    if(slice_fileupdate>1){
      slice_fileupdate--;
      break;
    }
  last_slice = -1;
    for(ii = nslice_loaded - 1; ii >= 0; ii--){
      i = slice_loaded_list[ii];
      sd = global_scase.slicecoll.sliceinfo + i;
      if(sd->slicefile_labelindex == slicefile_labelindex){
        last_slice = i;
        break;
      }
    }
    for(ii = 0; ii < nslice_loaded; ii++){
      i = slice_loaded_list[ii];
      sd = global_scase.slicecoll.sliceinfo + i;
      if(sd->slicefile_labelindex == slicefile_labelindex){
        int set_slicecolor;

        set_slicecolor = DEFER_SLICECOLOR;
        assert(last_slice!=1);
        if(i == last_slice)set_slicecolor = SET_SLICECOLOR;
        ReadSlice("", i, ALL_FRAMES, NULL, RESETBOUNDS, set_slicecolor, &error);
      }
    }
    slice_fileupdate--;
    use_slice_glui_bounds = 0;
    break;
  case FILE_RELOAD:
    if(research_mode==1){
      GLUISliceBoundCB(SET_GLOBAL_BOUNDS);
    }
    SetLoadedSliceBounds(NULL, 0);
    ReloadAllVectorSliceFiles(RELOAD);
    ReloadAllSliceFiles(RELOAD);
    GLUIHVACSliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIShowBounds ------------------------ */

extern "C" void GLUIShowBounds(int menu_id){
  switch(menu_id){
  case DIALOG_BOUNDS:
    GLUIUpdateChar();
    FileDataColRolloutCB(DATA_ROLLOUT);
    break;
  case DIALOG_SHOWFILES:
    if(ROLLOUT_showhide == NULL)break;
    if(ROLLOUT_showhide->is_open==1){
      ROLLOUT_showhide->close();
    }
    else{
      if(ROLLOUT_files->is_open == 0){
        FileDataColRolloutCB(FILE_ROLLOUT);
      }
      FileRolloutCB(SHOWHIDE_ROLLOUT);
    }
    break;
  case DIALOG_CONFIG:
    if(ROLLOUT_config->is_open==0){
      if(ROLLOUT_files->is_open == 0){
        FileDataColRolloutCB(FILE_ROLLOUT);
      }
      FileRolloutCB(CONFIG_ROLLOUT);
    }
    else{
      ROLLOUT_config->close();
    }
    break;
  case DIALOG_AUTOLOAD:
    if(ROLLOUT_autoload->is_open==0){
      if(ROLLOUT_filebounds->is_open == 0) {
        FileDataColRolloutCB(DATA_ROLLOUT);
      }
      if(ROLLOUT_time->is_open == 0){
        BoundRolloutCB(TIME_ROLLOUT);
      }
      LoadRolloutCB(LOAD_AUTO_ROLLOUT);
    }
    else {
      ROLLOUT_autoload->close();
    }
    break;
  case DIALOG_TIME:
    BoundRolloutCB(TIME_ROLLOUT);
    break;
  case DIALOG_SCRIPT:
    FileRolloutCB(SCRIPT_ROLLOUT);
    break;
  case DIALOG_SMOKEZIP:
    FileRolloutCB(COMPRESS_ROLLOUT);
    break;
  case DIALOG_3DSMOKE:
    FileDataColRolloutCB(DATA_ROLLOUT);
    BoundRolloutCB(SMOKE3D_ROLLOUT);
    break;
  case DIALOG_COLORING:
    FileRolloutCB(COLORING_ROLLOUT);
    break;
  default:
    assert(FFALSE);
    break;
  }
  glui_bounds->show();
}

/* ------------------ GLUIShowBoundsDialog ------------------------ */

extern "C" void GLUIShowBoundsDialog(int type){
  if(type==DIALOG_3DSMOKE){
    GLUIShowBounds(DIALOG_3DSMOKE);
  }
  else{
    GLUIShowBounds(DIALOG_BOUNDS);
  }
  switch(type){
    case DLG_3DSMOKE:
      if(ROLLOUT_smoke3d!=NULL)ROLLOUT_smoke3d->open();
      break;
    case DLG_BOUNDARY:
      if(ROLLOUT_bound!=NULL)ROLLOUT_bound->open();
      if(ROLLOUT_boundary_settings!=NULL)ROLLOUT_boundary_settings->open();
      break;
    case DLG_SLICE:
      if(ROLLOUT_slice != NULL)ROLLOUT_slice->open();
      if(ROLLOUT_slice_settings != NULL)ROLLOUT_slice_settings->open();
      break;
    case DLG_HVACDUCT:
      if(ROLLOUT_hvacduct != NULL)ROLLOUT_hvacduct->open();
      hvacductboundsCPP.open_minmax();
      break;
    case DLG_HVACNODE:
      if(ROLLOUT_hvacnode != NULL)ROLLOUT_hvacnode->open();
      hvacnodeboundsCPP.open_minmax();
      break;
    case DLG_PART:
      if(ROLLOUT_part!=NULL)ROLLOUT_part->open();
      if(ROLLOUT_particle_settings!=NULL)ROLLOUT_particle_settings->open();
      break;
    case DLG_PLOT3D:
      if(ROLLOUT_plot3d!=NULL)ROLLOUT_plot3d->open();
      break;
    case DLG_ISO:
      if(ROLLOUT_iso!=NULL)ROLLOUT_iso->open();
      if(ROLLOUT_iso_settings!=NULL)ROLLOUT_iso_settings->open();
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIUpdateOverwrite ------------------------ */

extern "C" void GLUIUpdateOverwrite(void){
  if(CHECKBOX_overwrite_all!=NULL)CHECKBOX_overwrite_all->set_int_val(overwrite_all);
  if(CHECKBOX_compress_autoloaded!=NULL)CHECKBOX_compress_autoloaded->set_int_val(compress_autoloaded);
}

/* ------------------ GLUIHideBounds ------------------------ */

extern "C" void GLUIHideBounds(void){
  GLUICloseRollouts(glui_bounds);
}

/* ------------------ GLUIUpdateVectorWidgets ------------------------ */

extern "C" void GLUIUpdateVectorWidgets(void){
  GLUIPlot3DBoundCB(UPDATE_VECTOR_FROM_SMV);
  GLUISliceBoundCB(UPDATE_VECTOR_FROM_SMV);
}

/* ------------------ GLUIUpdatePlot3dDisplay ------------------------ */

extern "C" void GLUIUpdatePlot3dDisplay(void){
  if(RADIO_plot3d_display!=NULL)RADIO_plot3d_display->set_int_val(contour_type);
}

/* ------------------ GLUIUpdateTimeBounds ------------------------ */

extern "C" void GLUIUpdateTimeBounds(float time_min, float time_max){
  if(SPINNER_timebounds!=NULL){
    SPINNER_timebounds->set_float_limits(time_min,time_max);
  }
}

/* ------------------ GLUIUpdateTBounds ------------------------ */

extern "C" void GLUIUpdateTBounds(void){
  GLUISliceBoundCB(FRAMELOADING);
}

/* ------------------ GLUIUpdateShowHideButtons ------------------------ */

extern "C" void GLUIUpdateShowHideButtons(void){

//  if(CHECKBOX_label_3 != NULL){
//    CHECKBOX_label_3->set_int_val(hide_overlaps);
//  }
  if(BUTTON_PART != NULL){
    if(npartloaded == 0){
      BUTTON_PART->disable();
    }
    else{
      BUTTON_PART->enable();
    }
  }

  if(BUTTON_SLICE != NULL){
    if(nsliceloaded == 0){
      BUTTON_SLICE->disable();
    }
    else{
      BUTTON_SLICE->enable();
    }
  }

  if(BUTTON_VSLICE != NULL){
    if(nvsliceloaded == 0){
      BUTTON_VSLICE->disable();
    }
    else{
      BUTTON_VSLICE->enable();
    }
  }

  if(BUTTON_ISO != NULL){
    if(nisoloaded == 0){
      BUTTON_ISO->disable();
    }
    else{
      BUTTON_ISO->enable();
    }
  }

  if(BUTTON_BOUNDARY != NULL){
    if(npatchloaded == 0){
      BUTTON_BOUNDARY->disable();
    }
    else{
      BUTTON_BOUNDARY->enable();
    }
  }

  if(BUTTON_3DSMOKE != NULL){
    if(nsmoke3dloaded == 0){
      BUTTON_3DSMOKE->disable();
    }
    else{
      BUTTON_3DSMOKE->enable();
    }
  }

  if(BUTTON_PLOT3D != NULL){
    if(nplot3dloaded == 0){
      BUTTON_PLOT3D->disable();
    }
    else{
      BUTTON_PLOT3D->enable();
    }
  }

  if(nplot3dloaded == 0 && nsmoke3dloaded == 0 && nisoloaded == 0 && nsliceloaded == 0 && npartloaded == 0 && npatchloaded == 0){
    if(RADIO_showhide != NULL)RADIO_showhide->disable();
  }
  else{
    if(RADIO_showhide != NULL)RADIO_showhide->enable();
  }
}
