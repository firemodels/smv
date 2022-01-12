#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOscript.h"
#include "MALLOCC.h"
#include "glui_smoke.h"
#include "glui_bounds.h"
#include "histogram.h"

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
#define BOUND_UPDATE_COLORS            110
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

#define PERCENTILE_DISABLED 0
#define PERCENTILE_ENABLED  1

#define HIDE_CACHE_CHECKBOX 0
#define SHOW_CACHE_CHECKBOX 1

int update_hist_bounds;

/* ------------------ bounds_dialog class ------------------------ */

class bounds_dialog{
  public:

  // variables
  cpp_boundsdata bounds, *all_bounds, *all_bounds_save;
  int   nall_bounds, research_mode_cpp, percentile_mode_cpp, percentile_enabled;
  int percentile_draw;
  float percentile_min_cpp100, percentile_max_cpp100;
  float plot_min_cpp, plot_max_cpp;
  int hist_left_percen_cpp, hist_down_percen_cpp, hist_length_percen_cpp, hist_show_labels_cpp;

  // widgets
  GLUI_EditText    *EDIT_valmin, *EDIT_valmax, *EDIT_chopmin, *EDIT_chopmax;
  GLUI_Checkbox    *CHECKBOX_set_chopmin, *CHECKBOX_set_chopmax, *CHECKBOX_cache;
  GLUI_Checkbox    *CHECKBOX_research_mode, *CHECKBOX_percentile_mode, *CHECKBOX_percentile_draw;
  GLUI_Checkbox    *CHECKBOX_hist_show_labels;
  GLUI_RadioGroup  *RADIO_set_valtype,  *RADIO_set_valmin, *RADIO_set_valmax;
  GLUI_RadioButton *RADIO_button_loaded_min, *RADIO_button_loaded_max;
  GLUI_RadioButton *RADIO_button_all_min, *RADIO_button_all_max;
  GLUI_RadioButton *RADIO_button_percentile_min, *RADIO_button_percentile_max;
  GLUI_Button      *BUTTON_update_colors, *BUTTON_reload_data;
  GLUI_Panel *PANEL_min, *PANEL_max;
  GLUI_Spinner *SPINNER_percentile_min, *SPINNER_percentile_max;
  GLUI_Spinner *SPINNER_plot_min, *SPINNER_plot_max;
  GLUI_Spinner *SPINNER_hist_left_percen, *SPINNER_hist_down_percen, *SPINNER_hist_length_percen;
  GLUI_Spinner *SPINNER_colorbar_digits;
  GLUI_StaticText  *STATIC_min_unit, *STATIC_max_unit, *STATIC_chopmin_unit, *STATIC_chopmax_unit;
  GLUI_StaticText *STATIC_percentile_00, *STATIC_percentile_50, *STATIC_percentile_100;
  GLUI_Rollout     *ROLLOUT_main_bound, *ROLLOUT_truncate, *ROLLOUT_percentiles;

  // routines
  //bounds_dialog(void);

  void CB(int var);
  int  get_cache_flag(void);
  void get_global_minmax(char *label, float *valmin, float *valmax);
  int  get_min(char *label, int *set_valmin, float *valmin);
  void get_min_all(int *set_valmin, float *valmin, int *nvals);
  int  get_max(char *label, int *set_valmax, float *valmax);
  int  get_percentile_draw(void);
  void set_percentile_draw(int val);
  void get_max_all(int *set_valmax, float *valmax, int *nvals);
  int  get_nvaltypes(void);
  int  get_valtype(void);
  int  in_research_mode(void);
  int  in_percentile_mode(void);
  void enabledisable_percentile_draw(int flag);
  cpp_boundsdata *get_bounds_data(void);

  void setup(const char *file_type, GLUI_Rollout *ROLLOUT_dialog, cpp_boundsdata *bounds, int nbounds,
             int *cache_flag, int cache_enable, int percentile_enable,
             void Callback(int var), GLUI_Update_CB PROC_CB, procdata *procinfo, int *nprocinfo);
  void setupNoGraphics(const char *file_type, cpp_boundsdata *bounds, int nbounds);
  void set_cache_flag(int cache_flag);
  int  set_chopmin(char *label, int set_valmin, float valmin);
  int  set_chopmax(char *label, int set_valmax, float valmax);
  int  set_min(char *label, int set_valmin, float valmin);
  void set_min_all(int *set_valmin, float *valmin, int nvals);
  int  set_max(char *label, int set_valmax, float valmax);
  void set_max_all(int *set_valmax, float *valmax, int nvals);
  void set_plot_minmax(float p_min, float p_max);
  void set_plot_minmax_clamp(float p_min, float p_max);
  void set_percentile_minmax(float p_min, float p_max);
  void set_research_mode(int flag);
  void set_percentile_mode(int flag);

  void set_colorbar_digits(int ndigits);
  void set_percentiles(float val_00, float per_valmin, float val_50, float per_valmax, float val_100);
  int  set_valtype(char *label);
  void set_valtype_index(int index);
  void SaveBounds(void);
  void RestoreBounds(void);
  void update_plot_parms(void);

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

/* ------------------ enabledisable_percentile_draw ------------------------ */

void bounds_dialog::enabledisable_percentile_draw(int flag){
  if(flag==1){
    CHECKBOX_percentile_draw->enable();
  }
  else{
    CHECKBOX_percentile_draw->disable();
    percentile_draw = 0;
    CHECKBOX_percentile_draw->set_int_val(percentile_draw);
  }
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

/* ------------------ set_percentile_minmax ------------------------ */

void bounds_dialog::set_percentile_minmax(float p_min, float p_max){
  p_min = SmvRound(p_min, 4);
  p_max = SmvRound(p_max, 4);
  if(SPINNER_percentile_min!=NULL)SPINNER_percentile_min->set_float_val(p_min);
  if(SPINNER_percentile_max!=NULL)SPINNER_percentile_max->set_float_val(p_max);
}

/* ------------------ set_cache_flag ------------------------ */

void bounds_dialog::set_cache_flag(int cache_flag){
  if(cache_flag!=1)cache_flag = 0;
  bounds.cache = cache_flag;
#ifdef pp_SHOW_CACHE
  if(CHECKBOX_cache!=NULL){
    CHECKBOX_cache->set_int_val(cache_flag);
    CB(BOUND_CACHE_DATA);
  }
#else
  CB(BOUND_CACHE_DATA);
#endif
}

  /* ------------------ get_bounds_data ------------------------ */

cpp_boundsdata *bounds_dialog::get_bounds_data(void){
  return &bounds;
}

/* ------------------ get_cache_flag ------------------------ */

int bounds_dialog::get_cache_flag(void){
  return bounds.cache;
}

/* ------------------ get_percentile_draw ------------------------ */

int bounds_dialog::get_percentile_draw(void){
  return percentile_draw;
}

/* ------------------ set_percentile_draw ------------------------ */

void bounds_dialog::set_percentile_draw(int val){
  percentile_draw = val;
  if(CHECKBOX_percentile_draw!=NULL)CHECKBOX_percentile_draw->set_int_val(percentile_draw);
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

  /* ------------------ in_percentile_mode ------------------------ */

int bounds_dialog::in_percentile_mode(void){
  int i;

  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    if(boundi->set_valmin!=BOUND_PERCENTILE_MIN)return 0;
    if(boundi->set_valmax!=BOUND_PERCENTILE_MAX)return 0;
  }
  return 1;
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
  if(research_mode_cpp==1){
    set_percentile_mode(0);
  }
  CHECKBOX_research_mode->set_int_val(research_mode_cpp);
  CB(BOUND_CACHE_DATA);
}

  /* ------------------ set_percentile_mode ------------------------ */

void bounds_dialog::set_percentile_mode(int flag){
  int i;

  if(flag==1){
    for(i = 0; i<nall_bounds; i++){
      cpp_boundsdata *boundi;

      boundi = all_bounds+i;
      boundi->set_valmin = BOUND_PERCENTILE_MIN;
      boundi->set_valmax = BOUND_PERCENTILE_MAX;
    }
    CB(BOUND_VAL_TYPE);
  }
  percentile_mode = flag;
  percentile_mode_cpp = flag;
  if(percentile_mode_cpp==1){
    set_research_mode(0);
  }
  CHECKBOX_percentile_mode->set_int_val(percentile_mode_cpp);
  CB(BOUND_CACHE_DATA);
}

/* ------------------ set_colorbar_digits ------------------------ */

void bounds_dialog::set_colorbar_digits(int ndigits){
  if(SPINNER_colorbar_digits!=NULL)SPINNER_colorbar_digits->set_int_val(ndigits);
}

/* ------------------ set_plot_parms ------------------------ */

void bounds_dialog::update_plot_parms(void){
  if(hist_left_percen != hist_left_percen_cpp){
    hist_left_percen_cpp = hist_left_percen;
    SPINNER_hist_left_percen->set_int_val(hist_left_percen);
  }
  if(hist_down_percen != hist_down_percen_cpp){
    hist_down_percen_cpp = hist_down_percen;
    SPINNER_hist_down_percen->set_int_val(hist_down_percen);
  }
  if(hist_length_percen != hist_length_percen_cpp){
    hist_length_percen_cpp = hist_length_percen;
    SPINNER_hist_length_percen->set_int_val(hist_length_percen);
  }
  if(hist_show_labels != hist_show_labels_cpp){
    hist_show_labels_cpp = hist_show_labels;
    CHECKBOX_hist_show_labels->set_int_val(hist_show_labels);
  }
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

void bounds_dialog::setup(const char *file_type, GLUI_Rollout *ROLLOUT_dialog, cpp_boundsdata *bounds_arg, int nbounds_arg, int *cache_flag, int cache_enable, int percentile_enabled_arg,
                          void Callback(int var),
                          GLUI_Update_CB PROC_CB, procdata *procinfo, int *nprocinfo){
  GLUI_Rollout *ROLLOUT_bound;
  GLUI_Panel *PANEL_bound2, *PANEL_minmax, *PANEL_minmax2, *PANEL_buttons;
  GLUI_Panel *PANEL_truncate_min, *PANEL_truncate_max, *PANEL_edit_min, *PANEL_edit_max;
  int i;
  char label1[64], label2[64];

  all_bounds = bounds_arg;
  nall_bounds = nbounds_arg;
  update_ini = 1;

  hist_left_percen_cpp   = hist_left_percen;
  hist_down_percen_cpp   = hist_down_percen;
  hist_length_percen_cpp = hist_length_percen;
  hist_show_labels_cpp   = hist_show_labels;

  plot_min_cpp = 0.0;
  plot_max_cpp = 1.0;
  update_hist_bounds = 1;

  NewMemory((void **)&all_bounds_save, nall_bounds*sizeof(cpp_boundsdata));
  SaveBounds();

  research_mode_cpp = research_mode;
  percentile_enabled = percentile_enabled_arg;

// bound min/max
  ROLLOUT_main_bound = glui_bounds->add_rollout_to_panel(ROLLOUT_dialog, _("Bound/Truncate data"), false, 0, PROC_CB);
  INSERT_ROLLOUT(ROLLOUT_main_bound, glui_bounds);
  ADDPROCINFO(procinfo, *nprocinfo, ROLLOUT_main_bound, 0, glui_bounds);

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
#ifdef pp_SHOW_CACHE
    CHECKBOX_cache = glui_bounds->add_checkbox_to_panel(PANEL_minmax, "Cache data", &(bounds.cache), BOUND_CACHE_DATA, Callback);
    if(cache_enable==0){
      CHECKBOX_cache->disable();
    }
#endif
  }

  strcpy(label1, "global(loaded ");
  strcat(label1, file_type);
  strcat(label1, " files)");

  strcpy(label2, "global(all ");
  strcat(label2, file_type);
  strcat(label2, " files)");

  RADIO_button_percentile_max = NULL;
  RADIO_button_percentile_min = NULL;

  CHECKBOX_research_mode   = glui_bounds->add_checkbox_to_panel(PANEL_minmax, _("global bounds for all data (research mode)"), &research_mode,      BOUND_RESEARCH_MODE,   Callback);
  CHECKBOX_percentile_mode = glui_bounds->add_checkbox_to_panel(PANEL_minmax, _("percentile bounds for all data"),             &percentile_mode,    BOUND_PERCENTILE_MODE, Callback);

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
  RADIO_button_all_min    = glui_bounds->add_radiobutton_to_group(RADIO_set_valmin, label2);
  if(cache_flag!=NULL&&percentile_enabled==1){
    RADIO_button_percentile_min = glui_bounds->add_radiobutton_to_group(RADIO_set_valmin, "percentile");
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
  RADIO_button_all_max    = glui_bounds->add_radiobutton_to_group(RADIO_set_valmax, label2);
  if(cache_flag!=NULL&&percentile_enabled==1){
    RADIO_button_percentile_max = glui_bounds->add_radiobutton_to_group(RADIO_set_valmax, "percentile");
  }

  BUTTON_update_colors = NULL;
  SPINNER_percentile_min = NULL;
  SPINNER_percentile_max = NULL;
  SPINNER_plot_min = NULL;
  SPINNER_plot_max = NULL;
  PANEL_buttons = NULL;
  CHECKBOX_percentile_draw = NULL;
  if(cache_flag!=NULL){
    if(percentile_enabled==1){
      GLUI_Panel *PANEL_drawA, *PANEL_drawB;

      percentile_draw = 0;
      ROLLOUT_percentiles = glui_bounds->add_rollout_to_panel(PANEL_minmax, "data distribution", false);

      percentile_min_cpp100 = CLAMP(percentile_level_min, 0.0, 1.0)*100.0;
      percentile_max_cpp100 = CLAMP(percentile_level_max, percentile_level_min,1.0)*100.0;

      PANEL_drawA            = glui_bounds->add_panel_to_panel(ROLLOUT_percentiles, "values:   percentiles");
      STATIC_percentile_100  = glui_bounds->add_statictext_to_panel(PANEL_drawA, "");
      SPINNER_percentile_max = glui_bounds->add_spinner_to_panel(PANEL_drawA,    "max", GLUI_SPINNER_FLOAT, &percentile_max_cpp100, BOUND_PERCENTILE_MAXVAL, Callback);
      STATIC_percentile_50   = glui_bounds->add_statictext_to_panel(PANEL_drawA, "");
      SPINNER_percentile_min = glui_bounds->add_spinner_to_panel(PANEL_drawA,    "min", GLUI_SPINNER_FLOAT, &percentile_min_cpp100, BOUND_PERCENTILE_MINVAL, Callback);
      STATIC_percentile_00   = glui_bounds->add_statictext_to_panel(PANEL_drawA, "");

      glui_bounds->add_column_to_panel(ROLLOUT_percentiles, false);

      PANEL_drawB = glui_bounds->add_panel_to_panel(ROLLOUT_percentiles, "plot bounds/position");
      CHECKBOX_hist_show_labels = glui_bounds->add_checkbox_to_panel(PANEL_drawB, _("show labels"),             &hist_show_labels_cpp, BOUND_HIST_LABELS, Callback);
      CHECKBOX_percentile_draw  = glui_bounds->add_checkbox_to_panel(PANEL_drawB, _("show plot"),               &percentile_draw, BOUND_PERCENTILE_DRAW, Callback);
      SPINNER_plot_max          = glui_bounds->add_spinner_to_panel(PANEL_drawB, _("max:"), GLUI_SPINNER_FLOAT, &plot_max_cpp, BOUND_PLOT_MINMAX, Callback);
      SPINNER_plot_min          = glui_bounds->add_spinner_to_panel(PANEL_drawB, _("min:"), GLUI_SPINNER_FLOAT, &plot_min_cpp, BOUND_PLOT_MINMAX, Callback);

      glui_bounds->add_column_to_panel(PANEL_drawB, false);
      SPINNER_hist_length_percen = glui_bounds->add_spinner_to_panel(PANEL_drawB, _("size:"), GLUI_SPINNER_INT, &hist_length_percen_cpp, BOUND_LENGTH_PERCEN, Callback);
      SPINNER_hist_left_percen   = glui_bounds->add_spinner_to_panel(PANEL_drawB, _("left:"), GLUI_SPINNER_INT, &hist_left_percen_cpp, BOUND_LEFT_PERCEN, Callback);
      SPINNER_hist_down_percen   = glui_bounds->add_spinner_to_panel(PANEL_drawB, _("bottom:"), GLUI_SPINNER_INT, &hist_down_percen_cpp, BOUND_DOWN_PERCEN, Callback);
      SPINNER_hist_left_percen->set_int_limits(0, 100);
      SPINNER_hist_down_percen->set_int_limits(0, 100);
      SPINNER_hist_length_percen->set_int_limits(0, 100);

      SPINNER_percentile_max->set_float_limits(percentile_min_cpp100, 100.0);
      SPINNER_percentile_min->set_float_limits(0.0, percentile_max_cpp100);

      // glui_bounds->add_button_to_panel(ROLLOUT_percentiles, "Update", BOUND_COMPUTE_PERCENTILES, Callback);
    }
    PANEL_buttons = glui_bounds->add_panel_to_panel(PANEL_minmax, "", GLUI_PANEL_NONE);
    BUTTON_update_colors      = glui_bounds->add_button_to_panel(PANEL_buttons, "Update colors", BOUND_UPDATE_COLORS, Callback);
    glui_bounds->add_column_to_panel(PANEL_buttons, false);
  }
  if(PANEL_buttons==NULL){
    BUTTON_reload_data = glui_bounds->add_button_to_panel(PANEL_minmax, "Reload data", BOUND_RELOAD_DATA, Callback);
  }
  else{
    BUTTON_reload_data = glui_bounds->add_button_to_panel(PANEL_buttons, "Reload data", BOUND_RELOAD_DATA, Callback);
  }

//*** chop above/below

  ROLLOUT_truncate = glui_bounds->add_rollout_to_panel(PANEL_bound2, "Truncate data",false);

  PANEL_truncate_max = glui_bounds->add_panel_to_panel(ROLLOUT_truncate, "", GLUI_PANEL_NONE);
  EDIT_chopmax = glui_bounds->add_edittext_to_panel(PANEL_truncate_max, "", GLUI_EDITTEXT_FLOAT, &(bounds.chopmax), BOUND_CHOPMAX, Callback);
  glui_bounds->add_column_to_panel(PANEL_truncate_max, false);
  STATIC_chopmax_unit = glui_bounds->add_statictext_to_panel(PANEL_truncate_max, "");
  STATIC_chopmax_unit->set_w(10);
  glui_bounds->add_column_to_panel(PANEL_truncate_max, false);
  CHECKBOX_set_chopmax = glui_bounds->add_checkbox_to_panel(PANEL_truncate_max, _("Above"), &(bounds.set_chopmax), BOUND_SETCHOPMAX, Callback);

  PANEL_truncate_min = glui_bounds->add_panel_to_panel(ROLLOUT_truncate, "", GLUI_PANEL_NONE);
  EDIT_chopmin = glui_bounds->add_edittext_to_panel(PANEL_truncate_min, "", GLUI_EDITTEXT_FLOAT, &(bounds.chopmin), BOUND_CHOPMIN, Callback);
  glui_bounds->add_column_to_panel(PANEL_truncate_min, false);
  STATIC_chopmin_unit = glui_bounds->add_statictext_to_panel(PANEL_truncate_min, "");
  STATIC_chopmin_unit->set_w(10);
  glui_bounds->add_column_to_panel(PANEL_truncate_min, false);
  CHECKBOX_set_chopmin = glui_bounds->add_checkbox_to_panel(PANEL_truncate_min, _("Below"), &(bounds.set_chopmin), BOUND_SETCHOPMIN, Callback);

  Callback(BOUND_VAL_TYPE);
  Callback(BOUND_SETCHOPMIN);
  Callback(BOUND_SETCHOPMAX);
  if(cache_flag!=NULL&&percentile_enabled==1){
    Callback(BOUND_CACHE_DATA);
    Callback(BOUND_PERCENTILE_MINVAL);
    Callback(BOUND_PERCENTILE_MAXVAL);
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

/* ------------------ set_percentiles ------------------------ */

void bounds_dialog::set_percentiles(float val_00, float per_valmin, float val_50, float per_valmax, float val_100){
  char val_label[sizeof(GLUI_String)];
#define PAD_LENGTH 12

  Float2String(val_label, val_00, 4, force_fixedpoint);
  strcat(val_label,":");
  PadString(val_label,"0 %", PAD_LENGTH);
  STATIC_percentile_00->set_name(val_label);

  Float2String(val_label, per_valmin, 4, force_fixedpoint);
  strcat(val_label,":");
  PadString(val_label,"", PAD_LENGTH);
  SPINNER_percentile_min->edittext->set_name(val_label);

  Float2String(val_label, val_50, 4, force_fixedpoint);
  strcat(val_label,":");
  PadString(val_label,"50 %", PAD_LENGTH);
  STATIC_percentile_50->set_name(val_label);

  Float2String(val_label, per_valmax, 4, force_fixedpoint);
  strcat(val_label,":");
  PadString(val_label,"", PAD_LENGTH);
  SPINNER_percentile_max->edittext->set_name(val_label);

  Float2String(val_label, val_100, 4, force_fixedpoint);
  strcat(val_label,":");
  PadString(val_label,"100 %", PAD_LENGTH);
  STATIC_percentile_100->set_name(val_label);
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

  boundi = all_bounds+i;
  boundi->set_valtype = i;
  RADIO_set_valtype->set_int_val(i);
  CB(BOUND_VAL_TYPE);
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

  nvals = MIN(nall_bounds, nvals);
  for(i = 0; i<nall_bounds; i++){
    cpp_boundsdata *boundi;

    boundi = all_bounds+i;
    boundi->set_valmin = set_valmin[i];
    boundi->valmin[boundi->set_valmin] = valmin[i];
  }
}

/* ------------------ set_max_all ------------------------ */

void bounds_dialog::set_max_all(int *set_valmax, float *valmax, int nvals){
  int i;

  nvals = MIN(nall_bounds, nvals);
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
      memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(RADIO_set_valmin!=NULL)RADIO_set_valmin->set_int_val(BOUND_SET_MIN);
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      if(InPercentileMode()!=percentile_mode_cpp){
        SetPercentileMode(1-percentile_mode_cpp);
      }
      break;
    case BOUND_VALMAX:
      bounds.valmax[BOUND_SET_MAX] = bounds.glui_valmax;
      bounds.set_valmax = BOUND_SET_MAX;
      memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(RADIO_set_valmax!=NULL)RADIO_set_valmax->set_int_val(BOUND_SET_MAX);
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      if(InPercentileMode()!=percentile_mode_cpp){
        SetPercentileMode(1-percentile_mode_cpp);
      }
      break;

      // min/max radio buttons
    case BOUND_SETVALMIN:
      bounds.glui_valmin = all_boundsi->valmin[bounds.set_valmin];
      if(EDIT_valmin!=NULL)EDIT_valmin->set_float_val(bounds.glui_valmin);
      memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      if(InPercentileMode()!=percentile_mode_cpp){
        SetPercentileMode(1-percentile_mode_cpp);
      }
      break;
    case BOUND_SETVALMAX:
      bounds.glui_valmax = all_boundsi->valmax[bounds.set_valmax];
      if(EDIT_valmax!=NULL)EDIT_valmax->set_float_val(bounds.glui_valmax);
      memcpy(all_boundsi, &bounds, sizeof(cpp_boundsdata));
      if(InResearchMode()!=research_mode_cpp){
        SetResearchMode(1-research_mode_cpp);
      }
      if(InPercentileMode()!=percentile_mode_cpp){
        SetPercentileMode(1-percentile_mode_cpp);
      }
      break;

      // chop dialog boxes
    case BOUND_CHOPMIN:
      all_boundsi->chopmin = bounds.chopmin;
      update_chop_colors = 1;
      break;
    case BOUND_CHOPMAX:
      all_boundsi->chopmax = bounds.chopmax;
      update_chop_colors = 1;
      break;
    case BOUND_SETCHOPMIN:
      all_boundsi->set_chopmin = bounds.set_chopmin;
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
      all_boundsi->set_chopmax = bounds.set_chopmax;
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
      {
        int i, cache_val = 0, enable_update_colors;

#ifndef pp_SHOW_CACHE
        cache_val = cache_file_data;
#endif

        if(CHECKBOX_cache!=NULL)cache_val = CHECKBOX_cache->get_int_val();

        for(i = 0; i<nall_bounds; i++){
          cpp_boundsdata *boundi;

          boundi = all_bounds+i;
          boundi->cache = bounds.cache;
        }
        if(ROLLOUT_percentiles!=NULL){
          if(cache_val==1&&percentile_enabled==1){
            ROLLOUT_percentiles->enable();
          }
          else{
            ROLLOUT_percentiles->close();
            ROLLOUT_percentiles->disable();
          }
        }
        if(cache_val==1){
          if(RADIO_button_percentile_min!=NULL)RADIO_button_percentile_min->enable();
          if(RADIO_button_percentile_max!=NULL)RADIO_button_percentile_max->enable();
        }
        else{
          if(RADIO_button_percentile_min!=NULL)RADIO_button_percentile_min->disable();
          if(RADIO_button_percentile_max!=NULL)RADIO_button_percentile_max->disable();
        }
        if(BUTTON_update_colors!=NULL){
          enable_update_colors = 0;
#ifdef pp_SHOW_CACHE
          if(CHECKBOX_cache!=NULL&&CHECKBOX_cache->get_int_val()==1)enable_update_colors = 1;
#else
          if(cache_val==1)enable_update_colors = 1;
#endif
          if(enable_update_colors==1){
            BUTTON_update_colors->enable();
          }
          else{
            BUTTON_update_colors->disable();
          }
        }
      }
      break;

      // update colors, reload data buttons - handle in calling routine
    case BOUND_UPDATE_COLORS:
      break;
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
      update_percentile_mode = 1;
      CB(BOUND_CACHE_DATA);
      break;
    case BOUND_COLORBAR_DIGITS:
      update_colorbar_digits = 1;
      updatemenu = 1;
      break;
    case BOUND_COMPUTE_PERCENTILES:
      break;
    case BOUND_PLOT_MINMAX:
      gmin_draw = plot_min_cpp;
      gmax_draw = plot_max_cpp;
      break;
    case BOUND_PERCENTILE_MINVAL:
      SPINNER_percentile_max->set_float_limits(percentile_min_cpp100+0.0001, 100.0);
      update_hist_bounds = 0;
      SetPercentileMinMax(percentile_min_cpp100, percentile_max_cpp100);
      update_hist_bounds = 1;
      percentile_level_min = percentile_min_cpp100/100.0;
      break;
    case BOUND_PERCENTILE_MAXVAL:
      SPINNER_percentile_min->set_float_limits(0.0,percentile_max_cpp100-0.0001);
      update_hist_bounds = 0;
      SetPercentileMinMax(percentile_min_cpp100, percentile_max_cpp100);
      update_hist_bounds = 1;
      percentile_level_max = percentile_max_cpp100/100.0;
      break;
    case BOUND_PERCENTILE_DRAW:
      if(CHECKBOX_percentile_draw!=NULL&&CHECKBOX_percentile_draw->get_int_val()!=percentile_draw){
        CHECKBOX_percentile_draw->set_int_val(percentile_draw);
      }
      break;
    case BOUND_LEFT_PERCEN:
      hist_left_percen = hist_left_percen_cpp;

      hist_length_percen = MIN(MIN(100-hist_left_percen_cpp, hist_length_percen_cpp), 100-hist_down_percen_cpp);
      hist_length_percen_cpp = hist_length_percen;
      SPINNER_hist_length_percen->set_int_val(hist_length_percen);
      break;
    case BOUND_DOWN_PERCEN:
      hist_down_percen = hist_down_percen_cpp;

      hist_length_percen = MIN(MIN(100-hist_left_percen_cpp, hist_length_percen_cpp), 100-hist_down_percen_cpp);
      hist_length_percen_cpp = hist_length_percen;
      SPINNER_hist_length_percen->set_int_val(hist_length_percen);
      break;
    case BOUND_LENGTH_PERCEN:
      hist_length_percen = hist_length_percen_cpp;

      hist_left_percen = MIN(hist_left_percen, 100 - hist_length_percen);
      hist_left_percen_cpp = hist_left_percen;
      SPINNER_hist_left_percen->set_int_val(hist_left_percen);

      hist_down_percen = MIN(hist_down_percen, 100 - hist_length_percen);
      hist_down_percen_cpp = hist_down_percen;
      SPINNER_hist_down_percen->set_int_val(hist_down_percen);
      break;
    case BOUND_HIST_LABELS:
      hist_show_labels       = hist_show_labels_cpp;
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
};

bounds_dialog patchboundsCPP, partboundsCPP, plot3dboundsCPP, sliceboundsCPP;

/* ------------------ SliceBoundsCPPSetupNoGraphics ------------------------ */

extern "C" void SliceBoundsSetupNoGraphics(void){
  sliceboundsCPP.setupNoGraphics("slice", slicebounds_cpp, nslicebounds_cpp);
}

/* ------------------ SetResearchMode ------------------------ */

extern "C" void SetResearchMode(int flag){
  if(npatchinfo>0)patchboundsCPP.set_research_mode(flag);
  if(nsliceinfo>0)sliceboundsCPP.set_research_mode(flag);
  if(npartinfo>0)partboundsCPP.set_research_mode(flag);
  if(nplot3dinfo>0)plot3dboundsCPP.set_research_mode(flag);
}

/* ------------------ SetColorbarDigitsCPP ------------------------ */

extern "C" void SetColorbarDigitsCPP(int ndigits){
  if(npatchinfo>0)patchboundsCPP.set_colorbar_digits(ndigits);
  if(nsliceinfo>0)sliceboundsCPP.set_colorbar_digits(ndigits);
  if(npartinfo>0)partboundsCPP.set_colorbar_digits(ndigits);
  if(nplot3dinfo>0)plot3dboundsCPP.set_colorbar_digits(ndigits);
}

/* ------------------ SetPercentileMode ------------------------ */

extern "C" void SetPercentileMode(int flag){
  if(npatchinfo>0)patchboundsCPP.set_percentile_mode(flag);
  if(nsliceinfo>0)sliceboundsCPP.set_percentile_mode(flag);
  if(npartinfo>0)partboundsCPP.set_percentile_mode(flag);
  if(nplot3dinfo>0)plot3dboundsCPP.set_percentile_mode(flag);
}

  /* ------------------ InPercentilehMode ------------------------ */

int InPercentileMode(void){
  if(npatchinfo>0&&patchboundsCPP.in_percentile_mode()==0)return 0;
  if(npartinfo>0&&partboundsCPP.in_percentile_mode()==0)return 0;
  if(nplot3dinfo>0&&plot3dboundsCPP.in_percentile_mode()==0)return 0;
  if(nsliceinfo>0&&sliceboundsCPP.in_percentile_mode()==0)return 0;
  return 1;
}

  /* ------------------ InResearchMode ------------------------ */

int InResearchMode(void){
  if(npatchinfo>0&&patchboundsCPP.in_research_mode()==0)return 0;
  if(npartinfo>0&&partboundsCPP.in_research_mode()==0)return 0;
  if(nplot3dinfo>0&&plot3dboundsCPP.in_research_mode()==0)return 0;
  if(nsliceinfo>0&&sliceboundsCPP.in_research_mode()==0)return 0;
  return 1;
}

/* ------------------ SetPercentileMinMax ------------------------ */

extern "C" void SetPercentileMinMax(float p_min, float p_max){
  if(npatchinfo>0)patchboundsCPP.set_percentile_minmax(p_min, p_max);
  if(nsliceinfo>0)sliceboundsCPP.set_percentile_minmax(p_min, p_max);
  if(npartinfo>0)partboundsCPP.set_percentile_minmax(p_min, p_max);
  if(nplot3dinfo>0)plot3dboundsCPP.set_percentile_minmax(p_min, p_max);
}

/* ------------------ SetPercentiles ------------------------ */

extern "C" void SetPercentiles(int type, float val_00, float per_valmin, float val_50, float per_valmax, float val_100){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)patchboundsCPP.set_percentiles(val_00, per_valmin, val_50, per_valmax, val_100);
      break;
    case BOUND_PART:
      if(npartinfo>0)partboundsCPP.set_percentiles(val_00, per_valmin, val_50, per_valmax, val_100);
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)plot3dboundsCPP.set_percentiles(val_00, per_valmin, val_50, per_valmax, val_100);
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)sliceboundsCPP.set_percentiles(val_00, per_valmin, val_50, per_valmax, val_100);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ GetBoundsData ------------------------ */

extern "C" cpp_boundsdata *GetBoundsData(int type){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)return patchboundsCPP.get_bounds_data();
      break;
    case BOUND_PART:
      if(npartinfo>0)return partboundsCPP.get_bounds_data();
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)return plot3dboundsCPP.get_bounds_data();
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)return sliceboundsCPP.get_bounds_data();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return NULL;
}

/* ------------------ GetGlobalBoundsMinMax ------------------------ */

extern "C" void GetGlobalBoundsMinMax(int type, char *label, float *valmin, float *valmax){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)patchboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_PART:
      if(npartinfo>0)partboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)plot3dboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)sliceboundsCPP.get_global_minmax(label, valmin, valmax);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetCacheFlag ------------------------ */

extern "C" void SetCacheFlag(int type, int cache_flag){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)patchboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_PART:
      if(npartinfo>0)partboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)plot3dboundsCPP.set_cache_flag(cache_flag);
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)sliceboundsCPP.set_cache_flag(cache_flag);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ GetCacheFlag ------------------------ */

extern "C" int GetCacheFlag(int type){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)return patchboundsCPP.get_cache_flag();
      break;
    case BOUND_PART:
      if(npartinfo>0)return partboundsCPP.get_cache_flag();
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)return plot3dboundsCPP.get_cache_flag();
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)return sliceboundsCPP.get_cache_flag();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GetPercentileDraw ------------------------ */

extern "C" int GetPercentileDraw(int type){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)return patchboundsCPP.get_percentile_draw();
      break;
    case BOUND_PART:
      if(npartinfo>0)return partboundsCPP.get_percentile_draw();
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)return plot3dboundsCPP.get_percentile_draw();
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)return sliceboundsCPP.get_percentile_draw();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ DisablePartPercentileDrawOff ------------------------ */

extern "C" void EnableDisablePartPercentileDraw(int flag){
  if(npartloaded>0){
    partboundsCPP.enabledisable_percentile_draw(flag);
  }
}

/* ------------------ SetPercentileDrawOff ------------------------ */

extern "C" void SetPercentileDrawOff(void){
  if(npatchloaded==0){
    patchboundsCPP.set_percentile_draw(0);
    PatchBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }
  if(npartloaded==0){
    partboundsCPP.set_percentile_draw(0);
    PartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }
  if(nplot3dloaded==0){
    plot3dboundsCPP.set_percentile_draw(0);
    Plot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }
  if(nsliceloaded==0){
    sliceboundsCPP.set_percentile_draw(0);
    SliceBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }
}

/* ------------------ SetPercentileDraw ------------------------ */

extern "C" void SetPercentileDraw(int type, int val){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0){
        patchboundsCPP.set_percentile_draw(val);
        if(val==1){
          if(npartinfo>0)partboundsCPP.set_percentile_draw(0);
          if(nplot3dinfo>0)plot3dboundsCPP.set_percentile_draw(0);
          if(nsliceinfo>0)sliceboundsCPP.set_percentile_draw(0);
        }
      }
      break;
    case BOUND_PART:
      if(npartinfo>0){
        partboundsCPP.set_percentile_draw(val);
        if(val==1){
          if(npatchinfo>0)patchboundsCPP.set_percentile_draw(0);
          if(nplot3dinfo>0)plot3dboundsCPP.set_percentile_draw(0);
          if(nsliceinfo>0)sliceboundsCPP.set_percentile_draw(0);
        }
      }
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0){
        plot3dboundsCPP.set_percentile_draw(val);
        if(val==1){
          if(npartinfo>0)partboundsCPP.set_percentile_draw(0);
          if(npatchinfo>0)patchboundsCPP.set_percentile_draw(0);
          if(nsliceinfo>0)sliceboundsCPP.set_percentile_draw(0);
        }
      }
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0){
        sliceboundsCPP.set_percentile_draw(val);
        if(val==1){
          if(npartinfo>0)partboundsCPP.set_percentile_draw(0);
          if(npatchinfo>0)patchboundsCPP.set_percentile_draw(0);
          if(nplot3dinfo>0)plot3dboundsCPP.set_percentile_draw(0);
        }
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ UpdateGluiBounds ------------------------ */

extern "C" void UpdateGluiBounds(void){
  if(npatchinfo>0){
    patchboundsCPP.CB(BOUND_VAL_TYPE);
    patchboundsCPP.CB(BOUND_SETCHOPMIN);
    patchboundsCPP.CB(BOUND_SETCHOPMAX);
  }

  if(npartinfo>0){
    partboundsCPP.CB(BOUND_VAL_TYPE);
    partboundsCPP.CB(BOUND_SETCHOPMIN);
    partboundsCPP.CB(BOUND_SETCHOPMAX);
  }

  if(nplot3dinfo>0){
    plot3dboundsCPP.CB(BOUND_VAL_TYPE);
    plot3dboundsCPP.CB(BOUND_SETCHOPMIN);
    plot3dboundsCPP.CB(BOUND_SETCHOPMAX);
  }

  if(nsliceinfo>0){
    sliceboundsCPP.CB(BOUND_VAL_TYPE);
    sliceboundsCPP.CB(BOUND_SETCHOPMIN);
    sliceboundsCPP.CB(BOUND_SETCHOPMAX);
  }
}

/* ------------------ GetValtype ------------------------ */

extern "C" int GetValType(int type){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)return patchboundsCPP.get_valtype();
      break;
    case BOUND_PART:
      if(npartinfo>0)return partboundsCPP.get_valtype();
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)return plot3dboundsCPP.get_valtype();
      break;
    case BOUND_SLICE:
      if(npatchinfo>0)return sliceboundsCPP.get_valtype();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ GetNValtypes ------------------------ */

extern "C" int GetNValtypes(int type){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)return patchboundsCPP.get_nvaltypes();
      break;
    case BOUND_PART:
      if(npartinfo>0)return partboundsCPP.get_nvaltypes();
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)return plot3dboundsCPP.get_nvaltypes();
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)return sliceboundsCPP.get_nvaltypes();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return 0;
}

/* ------------------ SetValTypeIndex ------------------------ */

extern "C" void SetValTypeIndex(int type, int valtype_index){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0)patchboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_PART:
      if(npartinfo>0)partboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0)plot3dboundsCPP.set_valtype_index(valtype_index);
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0)sliceboundsCPP.set_valtype_index(valtype_index);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ GetMinMax ------------------------ */

extern "C" void GetMinMax(int type, char *label, int *set_valmin, float *valmin, int *set_valmax, float *valmax){
  switch(type){
    case BOUND_PATCH:
      if(npatchinfo>0){
        patchboundsCPP.set_valtype(label);
        patchboundsCPP.get_min(label, set_valmin, valmin);
        patchboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PART:
      if(npartinfo>0){
        partboundsCPP.set_valtype(label);
        partboundsCPP.get_min(label, set_valmin, valmin);
        partboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_PLOT3D:
      if(nplot3dinfo>0){
        plot3dboundsCPP.set_valtype(label);
        plot3dboundsCPP.get_min(label, set_valmin, valmin);
        plot3dboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    case BOUND_SLICE:
      if(nsliceinfo>0){
        sliceboundsCPP.set_valtype(label);
        sliceboundsCPP.get_min(label, set_valmin, valmin);
        sliceboundsCPP.get_max(label, set_valmax, valmax);
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ GetMinMaxAll ------------------------ */

extern "C" void GetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int *nall){
  switch(type){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetMinMax ------------------------ */

extern "C" void SetMinMax(int type, char *label, int set_valmin, float valmin, int set_valmax, float valmax){
  switch(type){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetMin ------------------------ */

extern "C" void SetMin(int type, char *label, int set_valmin, float valmin){
  switch(type){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetMax ------------------------ */

extern "C" void SetMax(int type, char *label, int set_valmax, float valmax){
  switch(type){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetMinMaxAll ------------------------ */

extern "C" void SetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int nall){
  switch(type){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetChopMin ------------------------ */

extern "C" void SetChopMin(int type, char *label, int set_valmin, float valmin){
  switch(type){
    case BOUND_PATCH:
      patchboundsCPP.set_chopmin(label, set_valmin, valmin);
      break;
    case BOUND_PART:
      partboundsCPP.set_chopmin(label, set_valmin, valmin);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.set_chopmin(label, set_valmin, valmin);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.set_chopmin(label, set_valmin, valmin);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ SetChopMax ------------------------ */

extern "C" void SetChopMax(int type, char *label, int set_valmax, float valmax){
  switch(type){
    case BOUND_PATCH:
      patchboundsCPP.set_chopmax(label, set_valmax, valmax);
      break;
    case BOUND_PART:
      partboundsCPP.set_chopmax(label, set_valmax, valmax);
      break;
    case BOUND_PLOT3D:
      plot3dboundsCPP.set_chopmax(label, set_valmax, valmax);
      break;
    case BOUND_SLICE:
      sliceboundsCPP.set_chopmax(label, set_valmax, valmax);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ slice callback: SliceBoundsCPP_CB ------------------------ */

extern "C" void SliceBoundsCPP_CB(int var){
  int ii, last_slice;
  cpp_boundsdata *bounds;
  float per_valmin, per_valmax;

  sliceboundsCPP.CB(var);
  switch(var){
    case BOUND_VAL_TYPE:
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
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
      bounds = GetBoundsData(BOUND_SLICE);
      if(var==BOUND_COMPUTE_PERCENTILES||bounds->hist==NULL){
        ComputeLoadedSliceHist(bounds->label,&(bounds->hist));
      }
      if(bounds->hist!=NULL&&bounds->hist->defined==1){
        float per_00, per_50, per_100;

        GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
        GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
        SetMin(BOUND_SLICE, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
        SetMax(BOUND_SLICE, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

        GetHistogramValProc(bounds->hist, 0.0, &per_00);
        GetHistogramValProc(bounds->hist, 0.5, &per_50);
        GetHistogramValProc(bounds->hist, 1.0, &per_100);
        SetPercentiles(BOUND_SLICE, per_00, per_valmin, per_50, per_valmax, per_100);
      }
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 0;
      SliceBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 1;
      break;
    case BOUND_PERCENTILE_DRAW:
      if(GetPercentileDraw(BOUND_SLICE)==1){
        bounds = GetBoundsData(BOUND_SLICE);
        if(bounds->hist==NULL){
          ComputeLoadedSliceHist(bounds->label,&(bounds->hist));
        }
        if(bounds->hist!=NULL&&bounds->hist->defined==1){
          float hist_min, hist_max;
          float per_00, per_50, per_100;

          GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
          GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
          SetMin(BOUND_SLICE, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
          SetMax(BOUND_SLICE, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

          GetHistogramValProc(bounds->hist, 0.0, &per_00);
          GetHistogramValProc(bounds->hist, 0.5, &per_50);
          GetHistogramValProc(bounds->hist, 1.0, &per_100);
          SetPercentiles(BOUND_SLICE, per_00, per_valmin, per_50, per_valmax, per_100);

          histogram_draw = bounds->hist;
          histogram_label1 = cslice_label;
          histogram_label2 = bounds->label;
          xmin_draw = per_valmin;
          xmax_draw = per_valmax;
          hist_min = bounds->hist->val_min;
          hist_max = bounds->hist->val_max;
          for(ii = 0; ii<nsliceinfo; ii++){
            slicedata *slicei;

            slicei = sliceinfo+ii;
            if(strcmp(slicei->label.unit, bounds->unit)==0&&slicei->valmin_fds<=slicei->valmax_fds){
              hist_min = MIN(hist_min, slicei->valmin_fds);
              hist_max = MAX(hist_max, slicei->valmax_fds);
            }
          }
          if(update_hist_bounds==1){
            gmin_draw = hist_min;
            gmax_draw = hist_max;
            if(nsliceinfo>0){
              sliceboundsCPP.set_plot_minmax_clamp(hist_min, hist_max);
              sliceboundsCPP.set_plot_minmax(hist_min, hist_max);
            }
          }
          SetPercentileDraw(BOUND_PATCH, 0);
          SetPercentileDraw(BOUND_PART, 0);
          SetPercentileDraw(BOUND_PLOT3D, 0);
        }
      }
      else{
        histogram_draw = NULL;
        histogram_label1 = NULL;
        histogram_label2 = NULL;
      }
      break;
    case BOUND_CACHE_DATA:
      cache_slice_data = GetCacheFlag(BOUND_SLICE);
      break;
    case BOUND_UPDATE_COLORS:
      if(nslice_loaded==0)break;
#ifdef pp_RESEARCH_DEBUG
      printf("*** updating slice file colors\n");
#endif
      SetLoadedSliceBounds(NULL, 0);

      last_slice = slice_loaded_list[nslice_loaded - 1];
      for(ii = nslice_loaded - 1; ii >= 0; ii--){
        int i;
        slicedata *sd;

        i = slice_loaded_list[ii];
        sd = sliceinfo + i;
        if(sd->vloaded==0&&sd->display==0)continue;
        if(sd->slicefile_labelindex == slicefile_labelindex){
          last_slice = i;
          break;
        }
      }
      float timer_update_slice_colors;
      INIT_PRINT_TIMER(timer_update_slice_colors);
      PRINT_TIMER(timer_update_slice_colors, "null");
      UpdateSliceColors(last_slice);
      PRINT_TIMER(timer_update_slice_colors, "updateslicecolors");
      break;
    case BOUND_RELOAD_DATA:
      SetLoadedSliceBounds(NULL, 0);
#ifdef pp_THREAD
      LockUnlockCompress(1);
#endif
      SetLoadedSliceBounds(NULL, 0);
      ReloadAllVectorSliceFiles();
      ReloadAllSliceFiles();
#ifdef pp_THREAD
      LockUnlockCompress(0);
#endif
      SliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
      break;
    case BOUND_RESEARCH_MODE:
      if(npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
      if(npartinfo>0)partboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_PERCENTILE_MODE);
      break;
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      if(npartinfo>0)partboundsCPP.update_plot_parms();
      if(npatchinfo>0)patchboundsCPP.update_plot_parms();
      if(nplot3dinfo>0)plot3dboundsCPP.update_plot_parms();
      if(nsliceinfo>0)sliceboundsCPP.update_plot_parms();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ HavePlot3DData ------------------------ */

int HavePlot3DData(void){
  int i;

  for(i = 0; i<nplot3dinfo; i++){
    meshdata *meshi;
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    if(plot3di->blocknumber<0)return 0;
    meshi = meshinfo+plot3di->blocknumber;
    if(meshi->qdata==NULL)return 0;
  }
  return 1;
}

/* ------------------ plot3d callback: Plot3DBoundsCPP_CB ------------------------ */

extern "C" void Plot3DBoundsCPP_CB(int var){
  int i, iplot3d;
  cpp_boundsdata *all_bounds, *bounds;

  plot3dboundsCPP.CB(var);
  all_bounds = plot3dboundsCPP.all_bounds;
  iplot3d = GetValType(BOUND_PLOT3D);
  bounds = all_bounds+iplot3d;
  switch(var){
    case BOUND_VAL_TYPE:
      plotn = GetValType(BOUND_PLOT3D)+1;
      UpdateAllPlotSlices();
      Plot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      break;
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
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
#ifdef pp_RESEARCH_DEBUG
        printf("*** updating plot3d colors\n");
#endif
        UpdateAllPlot3DColors();
      }
      else{
        Plot3DBoundsCPP_CB(BOUND_RELOAD_DATA);
      }
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
      if(var==BOUND_COMPUTE_PERCENTILES||bounds->hist==NULL){
        MergePlot3DHistograms();
      }
      if(bounds!=NULL&&bounds->hist!=NULL&&bounds->hist->defined==1){
        float per_00, per_50, per_100;
        float per_valmin, per_valmax;

        SetPercentilePlot3DBounds();

        GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
        GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
        SetMin(BOUND_PLOT3D, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
        SetMax(BOUND_PLOT3D, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

        GetHistogramValProc(bounds->hist, 0.0, &per_00);
        GetHistogramValProc(bounds->hist, 0.5, &per_50);
        GetHistogramValProc(bounds->hist, 1.0, &per_100);
        SetPercentiles(BOUND_PLOT3D, per_00, per_valmin, per_50, per_valmax, per_100);
      }
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 0;
      Plot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 1;
      break;
    case BOUND_PERCENTILE_DRAW:
      if(bounds!=NULL&&bounds->hist!=NULL&&bounds->hist->defined==1){
        float per_valmin, per_valmax;
        float per_00, per_50, per_100;

        GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
        GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
        GetHistogramValProc(bounds->hist, 0.5, &(bounds->hist->median));
        SetMin(BOUND_PLOT3D, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
        SetMax(BOUND_PLOT3D, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

        GetHistogramValProc(bounds->hist, 0.0, &per_00);
        GetHistogramValProc(bounds->hist, 0.5, &per_50);
        GetHistogramValProc(bounds->hist, 1.0, &per_100);
        SetPercentiles(BOUND_PLOT3D, per_00, per_valmin, per_50, per_valmax, per_100);

        if(GetPercentileDraw(BOUND_PLOT3D)==1){
          float hist_min, hist_max;

          histogram_draw = bounds->hist;
          histogram_label1 = cplot3d_label;
          histogram_label2 = bounds->label;
          xmin_draw = per_valmin;
          xmax_draw = per_valmax;
          hist_min = bounds->hist->val_min;
          hist_max = bounds->hist->val_max;

          for(i = 0; i<plot3dboundsCPP.nall_bounds; i++){
            cpp_boundsdata *boundi;

            boundi = plot3dboundsCPP.all_bounds+i;
            if(strcmp(boundi->unit, bounds->unit)==0&&boundi->hist!=NULL){
              hist_min = MIN(hist_min, boundi->hist->val_min);
              hist_max = MAX(hist_max, boundi->hist->val_max);
            }
          }
          SetMin(BOUND_PLOT3D, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
          SetMax(BOUND_PLOT3D, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);
          if(update_hist_bounds==1){
            gmin_draw = hist_min;
            gmax_draw = hist_max;
            if(nplot3dinfo>0){
              plot3dboundsCPP.set_plot_minmax_clamp(hist_min, hist_max);
              plot3dboundsCPP.set_plot_minmax(hist_min, hist_max);
            }
          }
          SetPercentileDraw(BOUND_PART, 0);
          SetPercentileDraw(BOUND_PATCH, 0);
          SetPercentileDraw(BOUND_SLICE, 0);
        }
        else{
          histogram_draw = NULL;
          histogram_label1 = NULL;
          histogram_label2 = NULL;
        }
      }
      break;
    case BOUND_RELOAD_DATA:
#ifdef pp_RESEARCH_DEBUG
      printf("*** reloading plot3d data\n");
#endif
      LoadPlot3dMenu(RELOAD_ALL);
      break;
    case BOUND_RESEARCH_MODE:
      if(npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
      if(npartinfo>0)partboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_PERCENTILE_MODE);
      break;
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      if(npartinfo>0)partboundsCPP.update_plot_parms();
      if(npatchinfo>0)patchboundsCPP.update_plot_parms();
      if(nplot3dinfo>0)plot3dboundsCPP.update_plot_parms();
      if(nsliceinfo>0)sliceboundsCPP.update_plot_parms();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ part callback: PartBoundsCPP_CB ------------------------ */

extern "C" void PartBoundsCPP_CB(int var){
  cpp_boundsdata *all_bounds, *bounds;

  partboundsCPP.CB(var);
  all_bounds = partboundsCPP.all_bounds;
  ipart5prop = GetValType(BOUND_PART);
  bounds = all_bounds+ipart5prop;

  switch(var){
    case BOUND_VAL_TYPE:
      ipart5prop = GetValType(BOUND_PART);
      ParticlePropShowMenu(ipart5prop);
      PartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      break;
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
    case BOUND_CHOPMIN:
    case BOUND_CHOPMAX:
    case BOUND_SETCHOPMIN:
    case BOUND_SETCHOPMAX:
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
      if(var==BOUND_COMPUTE_PERCENTILES||bounds->hist==NULL){
        MergePartHistograms();
      }
      if(bounds->hist!=NULL&&bounds->hist->defined==1){
        float per_00, per_50, per_100;
        float per_valmin, per_valmax;

        SetPercentilePartBounds();

        GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
        GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
       // SetMin(BOUND_PART, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
       // SetMax(BOUND_PART, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

        GetHistogramValProc(bounds->hist, 0.0, &per_00);
        GetHistogramValProc(bounds->hist, 0.5, &per_50);
        GetHistogramValProc(bounds->hist, 1.0, &per_100);
        SetPercentiles(BOUND_PART, per_00, per_valmin, per_50, per_valmax, per_100);
      }
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 0;
      PartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 1;
      break;
    case BOUND_PERCENTILE_DRAW:
      if(bounds!=NULL&&bounds->hist!=NULL&&bounds->hist->defined==1){
        int i;
        float per_valmin, per_valmax;
        float per_00, per_50, per_100;

        GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
        GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
        GetHistogramValProc(bounds->hist, 0.5, &(bounds->hist->median));
   //     SetMin(BOUND_PART, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
   //     SetMax(BOUND_PART, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

        GetHistogramValProc(bounds->hist, 0.0, &per_00);
        GetHistogramValProc(bounds->hist, 0.5, &per_50);
        GetHistogramValProc(bounds->hist, 1.0, &per_100);
        SetPercentiles(BOUND_PART, per_00, per_valmin, per_50, per_valmax, per_100);

        if(GetPercentileDraw(BOUND_PART)==1&&ipart5prop!=0){
          float hist_min, hist_max;

          histogram_draw = bounds->hist;
          histogram_label1 = cpart_label;
          histogram_label2 = bounds->label;
          xmin_draw = per_valmin;
          xmax_draw = per_valmax;
          SetPercentileDraw(BOUND_SLICE,  0);
          SetPercentileDraw(BOUND_PATCH,  0);
          SetPercentileDraw(BOUND_PLOT3D, 0);
          hist_min = bounds->hist->val_min;
          hist_max = bounds->hist->val_max;
          for(i = 0; i<partboundsCPP.nall_bounds; i++){
            cpp_boundsdata *boundi;

            boundi = partboundsCPP.all_bounds+i;
            if(strcmp(boundi->unit, bounds->unit)==0&&boundi->hist!=NULL){
              hist_min = MIN(hist_min, boundi->hist->val_min);
              hist_max = MAX(hist_max, boundi->hist->val_max);
            }
          }
          if(update_hist_bounds==1){
            gmin_draw = hist_min;
            gmax_draw = hist_max;
            if(npartinfo>0){
              partboundsCPP.set_plot_minmax_clamp(hist_min, hist_max);
              partboundsCPP.set_plot_minmax(hist_min, hist_max);
            }
          }
         // SetMin(BOUND_PART, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
         // SetMax(BOUND_PART, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);
        }
        else{
          histogram_draw   = NULL;
          histogram_label1 = NULL;
          histogram_label2 = NULL;
        }
      }
      break;
    case BOUND_CACHE_DATA:
      if(GetCacheFlag(BOUND_PART)==0){
        ClosePartFiles();
      }
      break;
    case BOUND_UPDATE_COLORS:
      UpdatePartColors(NULL);
      break;
    case BOUND_RELOAD_DATA:
      if(npartinfo>0){
#ifdef pp_RESEARCH_DEBUG
        int doit=0, i;

        for(i = 0; i<npartinfo; i++){
          partdata *parti;

          parti = partinfo+i;
          if(parti->loaded==1){
            doit = 1;
            break;
          }
        }
        if(doit==1)printf("*** reloading particle file data\n");
#endif
        LoadParticleMenu(PARTFILE_RELOADALL);
        LoadEvacMenu(EVACFILE_RELOADALL);
      }
      break;
    case BOUND_RESEARCH_MODE:
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_PERCENTILE_MODE);
      break;
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      if(npartinfo>0)partboundsCPP.update_plot_parms();
      if(npatchinfo>0)patchboundsCPP.update_plot_parms();
      if(nplot3dinfo>0)plot3dboundsCPP.update_plot_parms();
      if(nsliceinfo>0)sliceboundsCPP.update_plot_parms();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ HavePatchData ------------------------ */

int HavePatchData(void){
  int i;

  for(i = 0; i<npatchinfo; i++){
    patchdata *patchi;
    meshdata *meshi;

    patchi = patchinfo+i;
    if(patchi->loaded==0)continue;
    switch(patchi->patch_filetype){
      case PATCH_STRUCTURED_NODE_CENTER:
      case PATCH_STRUCTURED_CELL_CENTER:
        meshi = meshinfo+patchi->blocknumber;
        if(meshi->patchval==NULL||meshi->cpatchval==NULL)return 0;
        break;
      case PATCH_GEOMETRY_BOUNDARY:
        if(patchi->geom_vals==NULL)return 0;
        break;
      case PATCH_GEOMETRY_SLICE:
        break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }
  return 1;
}

/* ------------------ patch callback: PatchBoundsCPP_CB ------------------------ */

extern "C" void PatchBoundsCPP_CB(int var){
  int i;
  cpp_boundsdata *bounds;
  float per_valmin, per_valmax;
  float global_min, global_max;

  patchboundsCPP.CB(var);
  switch(var){
    case BOUND_VAL_TYPE:
    case BOUND_VALMIN:
    case BOUND_VALMAX:
    case BOUND_SETVALMIN:
    case BOUND_SETVALMAX:
    case BOUND_CHOPMIN:
    case BOUND_CHOPMAX:
    case BOUND_SETCHOPMIN:
    case BOUND_SETCHOPMAX:
      break;
    case BOUND_PERCENTILE_MINVAL:
    case BOUND_PERCENTILE_MAXVAL:
    case BOUND_COMPUTE_PERCENTILES:
    case BOUND_COMPUTE_ONLY_PERCENTILES:
      bounds = GetBoundsData(BOUND_PATCH);

      if(var==BOUND_COMPUTE_PERCENTILES||bounds->hist==NULL){
        GetGlobalBoundsMinMax(BOUND_PATCH, bounds->label, &global_min, &global_max);
        ComputeLoadedPatchHist(bounds->label, &(bounds->hist), &global_min, &global_max);
      }
      if(bounds->hist!=NULL&&bounds->hist->defined==1){
        float per_00, per_50, per_100;

        GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
        GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
        SetMin(BOUND_PATCH, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
        SetMax(BOUND_PATCH, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

        GetHistogramValProc(bounds->hist, 0.0, &per_00);
        GetHistogramValProc(bounds->hist, 0.5, &per_50);
        GetHistogramValProc(bounds->hist, 1.0, &per_100);
        SetPercentiles(BOUND_PATCH, per_00, per_valmin, per_50, per_valmax, per_100);
      }
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 0;
      PatchBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
      if(var==BOUND_PERCENTILE_MAXVAL||var==BOUND_PERCENTILE_MINVAL)update_hist_bounds = 1;
      break;
    case BOUND_CACHE_DATA:
      cache_boundary_data = GetCacheFlag(BOUND_PATCH);
      break;
    case BOUND_PERCENTILE_DRAW:
      if(GetPercentileDraw(BOUND_PATCH)==1){
        bounds = GetBoundsData(BOUND_PATCH);
        if(bounds->hist==NULL){
          GetGlobalBoundsMinMax(BOUND_PATCH, bounds->label, &global_min, &global_max);
          ComputeLoadedPatchHist(bounds->label, &(bounds->hist), &global_min, &global_max);
        }
        if(bounds->hist!=NULL&&bounds->hist->defined==1){
          float hist_min, hist_max;
          float per_00, per_50, per_100;

          GetHistogramValProc(bounds->hist, percentile_level_min, &per_valmin);
          GetHistogramValProc(bounds->hist, percentile_level_max, &per_valmax);
          GetHistogramValProc(bounds->hist, 0.5,                  &(bounds->hist->median));
          SetMin(BOUND_PATCH, bounds->label, BOUND_PERCENTILE_MIN, per_valmin);
          SetMax(BOUND_PATCH, bounds->label, BOUND_PERCENTILE_MAX, per_valmax);

          GetHistogramValProc(bounds->hist, 0.0, &per_00);
          GetHistogramValProc(bounds->hist, 0.5, &per_50);
          GetHistogramValProc(bounds->hist, 1.0, &per_100);
          SetPercentiles(BOUND_PATCH, per_00, per_valmin, per_50, per_valmax, per_100);

          histogram_draw = bounds->hist;
          histogram_label1 = cbound_label;
          histogram_label2 = bounds->label;
          xmin_draw = per_valmin;
          xmax_draw = per_valmax;
          hist_min = bounds->hist->val_min;
          hist_max = bounds->hist->val_max;
          for(i = 0; i<npatchinfo; i++){
            patchdata *patchi;

            patchi = patchinfo+i;
            if(strcmp(patchi->label.unit, bounds->unit)==0&&patchi->valmin_fds<=patchi->valmax_fds){
              hist_min = MIN(hist_min, patchi->valmin_fds);
              hist_max = MAX(hist_max, patchi->valmax_fds);
            }
          }
          if(update_hist_bounds==1){
            gmin_draw = hist_min;
            gmax_draw = hist_max;
            if(npatchinfo>0){
              patchboundsCPP.set_plot_minmax_clamp(hist_min, hist_max);
              patchboundsCPP.set_plot_minmax(hist_min, hist_max);
            }
          }
          SetPercentileDraw(BOUND_SLICE,  0);
          SetPercentileDraw(BOUND_PART,   0);
          SetPercentileDraw(BOUND_PLOT3D, 0);
        }
      }
      else{
        histogram_draw   = NULL;
        histogram_label1 = NULL;
        histogram_label2 = NULL;
      }
      break;
    case BOUND_UPDATE_COLORS:
      if(HavePatchData()==1){
#ifdef pp_RESEARCH_DEBUG
        if(npatchloaded>0)printf("*** updating boundary file colors");
#endif
        SetLoadedPatchBounds(NULL, 0);
        UpdateAllBoundaryColors();
      }
      else{
        PatchBoundsCPP_CB(BOUND_RELOAD_DATA);
      }
      break;
    case BOUND_RELOAD_DATA:
#ifdef pp_RESEARCH_DEBUG
      printf("*** reloading boundary file data\n");
#endif
      SetLoadedPatchBounds(NULL, 0);
      for(i = 0; i<npatchinfo; i++){
        patchdata *patchi;

        patchi = patchinfo+i;
        patchi->finalize = 0;
      }
      for(i = npatchinfo-1; i>=0;  i--){
        patchdata *patchi;

        patchi = patchinfo+i;
        if(patchi->loaded==0)continue;
        patchi->finalize = 1;
        break;
      }
      for(i = 0; i<npatchinfo; i++){
        patchdata *patchi;
        int errorcode;

        patchi = patchinfo + i;
        if(patchi->loaded == 0)continue;
        ReadBoundary(i,LOAD,&errorcode);
      }
      break;
    case BOUND_RESEARCH_MODE:
      if(npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
      if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
      break;
    case BOUND_PERCENTILE_MODE:
      if(npartinfo>0)partboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(npatchinfo>0)patchboundsCPP.CB(BOUND_PERCENTILE_MODE);
      if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_PERCENTILE_MODE);
      break;
    case BOUND_LEFT_PERCEN:
    case BOUND_DOWN_PERCEN:
    case BOUND_LENGTH_PERCEN:
    case BOUND_HIST_LABELS:
      if(npartinfo>0)partboundsCPP.update_plot_parms();
      if(npatchinfo>0)patchboundsCPP.update_plot_parms();
      if(nplot3dinfo>0)plot3dboundsCPP.update_plot_parms();
      if(nsliceinfo>0)sliceboundsCPP.update_plot_parms();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ UpdatdateResearchModeCPP ------------------------ */

extern "C" void UpdatdateResearchModeCPP(void){
  if(npatchinfo>0)patchboundsCPP.CB(BOUND_RESEARCH_MODE);
  if(npartinfo>0)partboundsCPP.CB(BOUND_RESEARCH_MODE);
  if(nplot3dinfo>0)plot3dboundsCPP.CB(BOUND_RESEARCH_MODE);
  if(nsliceinfo>0)sliceboundsCPP.CB(BOUND_RESEARCH_MODE);
}

/* ------------------ SetLoadedSliceBounds ------------------------ */

void SetLoadedSliceBounds(int *list, int nlist){
  int set_valmin, set_valmax;
  float valmin_dlg, valmax_dlg;
  float valmin, valmax;
  char *label=NULL;
  int i;

  if(list==NULL){
    for(i = 0; i<nsliceinfo; i++){
      slicedata *slicei;

      slicei = sliceinfo+i;
      if(slicei->loaded==1&&slicei->display==1){
        label = slicei->label.shortlabel;
        break;
      }
    }
  }
  else{
    slicedata *slicei;

    slicei = sliceinfo+list[0];
    label = slicei->label.shortlabel;
  }
  if(label==NULL)return;

  valmin = 1.0;
  valmax = 0.0;
  if(list==NULL)nlist = 0;
  for(i = 0; i<nlist; i++){
    slicedata *slicei;

    slicei = sliceinfo+list[i];
    if(valmin>valmax){
      valmin = slicei->valmin_fds;
      valmax = slicei->valmax_fds;
    }
    else{
      valmin = MIN(valmin, slicei->valmin_fds);
      valmax = MAX(valmax, slicei->valmax_fds);
    }
  }
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(slicei->loaded==0)continue;
    if(strcmp(slicei->label.shortlabel, label)!=0)continue;
    if(valmin>valmax){
      valmin = slicei->valmin_fds;
      valmax = slicei->valmax_fds;
    }
    else{
      valmin = MIN(valmin, slicei->valmin_fds);
      valmax = MAX(valmax, slicei->valmax_fds);
    }
  }
  if(valmin<=valmax){
    GetMinMax(BOUND_SLICE, label, &set_valmin, &valmin_dlg, &set_valmax, &valmax_dlg);
    if(set_valmin!=BOUND_LOADED_MIN){
      valmin = valmin_dlg;
    }
    if(set_valmax!=BOUND_LOADED_MAX){
      valmax = valmax_dlg;
    }
    SetMinMax(BOUND_SLICE, label, set_valmin, valmin, set_valmax, valmax);
  }
}

/* ------------------ SetLoadedPatchBounds ------------------------ */

void SetLoadedPatchBounds(int *list, int nlist){
  int set_valmin, set_valmax;
  float valmin_dlg, valmax_dlg;
  float valmin, valmax;
  char *label=NULL;
  patchdata *patchi;
  int i;

  if(list==NULL){
    for(i = 0; i<npatchinfo; i++){
      patchi = patchinfo+i;
      if(patchi->loaded==1&&patchi->display==1){
        label = patchi->label.shortlabel;
        break;
      }
    }
  }
  else{
    patchi = patchinfo+list[0];
    label = patchi->label.shortlabel;
  }
  if(label==NULL)return;

  valmin = 1.0;
  valmax = 0.0;
  if(list==NULL)nlist = 0;
  for(i = 0; i<nlist; i++){
    patchi = patchinfo+list[i];
    if(valmin>valmax){
      valmin = patchi->valmin_fds;
      valmax = patchi->valmax_fds;
    }
    else{
      valmin = MIN(valmin, patchi->valmin_fds);
      valmax = MAX(valmax, patchi->valmax_fds);
    }
  }
  for(i = 0; i<npatchinfo; i++){
    patchi = patchinfo+i;
    if(patchi->loaded==0)continue;
    if(strcmp(patchi->label.shortlabel, label)!=0)continue;
    if(valmin>valmax){
      valmin = patchi->valmin_fds;
      valmax = patchi->valmax_fds;
    }
    else{
      valmin = MIN(valmin, patchi->valmin_fds);
      valmax = MAX(valmax, patchi->valmax_fds);
    }
  }
  if(valmin<=valmax){
    GetMinMax(BOUND_PATCH, label, &set_valmin, &valmin_dlg, &set_valmax, &valmax_dlg);
    if(set_valmin!=BOUND_LOADED_MIN){
      valmin = valmin_dlg;
    }
    if(set_valmax!=BOUND_LOADED_MAX){
      valmax = valmax_dlg;
    }
    SetMinMax(BOUND_PATCH, label, set_valmin, valmin, set_valmax, valmax);
  }
}

/* ------------------ SetLoadedPlot3DBounds ------------------------ */

void SetLoadedPlot3DBounds(int *list, int nlist){
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
    if(list==NULL)nlist = 0;
    for(i = 0; i<nlist; i++){
      float *valmin_fds, *valmax_fds;
      plot3ddata *plot3di;

      plot3di = plot3dinfo+list[i];
      valmin_fds = plot3di->valmin_fds;
      valmax_fds = plot3di->valmax_fds;
      if(valmin[j]>valmax[j]){
        valmin[j] = valmin_fds[j];
        valmax[j] = valmax_fds[j];
      }
      else{
        valmin[j] = MIN(valmin[j], valmin_fds[j]);
        valmax[j] = MAX(valmax[j], valmax_fds[j]);
      }
    }

    for(i = 0; i<nplot3dinfo; i++){
      float *valmin_fds, *valmax_fds;
      plot3ddata *plot3di;

      plot3di = plot3dinfo+i;
      if(plot3di->loaded==0)continue;
      valmin_fds = plot3di->valmin_fds;
      valmax_fds = plot3di->valmax_fds;
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

  GetMinMaxAll(BOUND_PLOT3D, set_valmin, valmin_dlg, set_valmax, valmax_dlg, &nall);
  for(i=0;i<nall;i++){
    if(set_valmin[i]!=BOUND_LOADED_MIN){
      valmin[i] = valmin_dlg[i];
    }
    if(set_valmax[i]!=BOUND_LOADED_MAX){
      valmax[i] = valmax_dlg[i];
    }
  }
  SetMinMaxAll(BOUND_PLOT3D, set_valmin, valmin, set_valmax, valmax, nall);
  FREEMEMORY(valmin);
  FREEMEMORY(valmax);
  FREEMEMORY(set_valmin);
  FREEMEMORY(set_valmax);
  FREEMEMORY(valmin_dlg);
  FREEMEMORY(valmax_dlg);
}

/* ------------------ SetPercentilePlot3DBounds ------------------------ */

 void SetPercentilePlot3DBounds(void){
  int i;
  cpp_boundsdata *all_bounds;

  if(full_plot3D_histograms==NULL)return;

  all_bounds = plot3dboundsCPP.all_bounds;

  for(i = 0; i<MAXPLOT3DVARS; i++){
    float vmin, vmax;
    cpp_boundsdata *boundsi;

    boundsi = all_bounds+i;
    boundsi->hist = full_plot3D_histograms+i;

    GetHistogramValProc(boundsi->hist, percentile_level_min, &vmin);
    GetHistogramValProc(boundsi->hist, percentile_level_max, &vmax);
    boundsi->valmin[BOUND_PERCENTILE_MIN] = vmin;
    boundsi->valmax[BOUND_PERCENTILE_MAX] = vmax;
  }
}

/* ------------------ SetPercentilePartBounds ------------------------ */

void SetPercentilePartBounds(void){
  int *set_valmin, *set_valmax, nall;
  float *valmin, *valmax;
  int i;
  cpp_boundsdata *all_bounds;

  if(full_part_histogram==NULL)return;

  all_bounds = partboundsCPP.all_bounds;

  NewMemory((void **)&set_valmin, npart5prop*sizeof(int));
  NewMemory((void **)&set_valmax, npart5prop*sizeof(int));
  NewMemory((void **)&valmin, npart5prop*sizeof(float));
  NewMemory((void **)&valmax, npart5prop*sizeof(float));

  GetMinMaxAll(BOUND_PART, set_valmin, valmin, set_valmax, valmax, &nall);
  valmin[0] = 0.0;
  valmax[0] = 1.0;
  for(i = 1; i<npart5prop; i++){
    float vmin, vmax;
    cpp_boundsdata *boundsi;

    boundsi = all_bounds+i;
    boundsi->hist = full_part_histogram+i;

    GetHistogramValProc(boundsi->hist, percentile_level_min, &vmin);
    GetHistogramValProc(boundsi->hist, percentile_level_max, &vmax);
    boundsi->valmin[BOUND_PERCENTILE_MIN] = vmin;
    boundsi->valmax[BOUND_PERCENTILE_MAX] = vmax;
    valmin[i] = vmin;
    valmax[i] = vmax;
  }
  FREEMEMORY(set_valmin);
  FREEMEMORY(set_valmax);
  FREEMEMORY(valmin);
  FREEMEMORY(valmax);
}

/* ------------------ SetLoadedPartBounds ------------------------ */

void SetLoadedPartBounds(int *list, int nlist){
  float *valmin_dlg, *valmax_dlg, *valmin, *valmax;
  int *set_valmin, *set_valmax, nall;
  int i, j;
  int npart_types;


  npart_types = GetNValtypes(BOUND_PART);

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
      float *valmin_fds, *valmax_fds;
      partdata *parti;

      parti = partinfo+list[i];
      if(parti->have_bound_file==NO)continue;
      valmin_fds = parti->valmin_fds;
      valmax_fds = parti->valmax_fds;
      if(valmin[j]>valmax[j]){
        valmin[j] = valmin_fds[j];
        valmax[j] = valmax_fds[j];
      }
      else{
        valmin[j] = MIN(valmin[j], valmin_fds[j]);
        valmax[j] = MAX(valmax[j], valmax_fds[j]);
      }
    }

    for(i = 0; i<npartinfo; i++){
      float *valmin_fds, *valmax_fds;
      partdata *parti;

      parti = partinfo+i;
      if(parti->loaded==0||parti->have_bound_file==NO)continue;
      valmin_fds = parti->valmin_fds;
      valmax_fds = parti->valmax_fds;
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

  GetMinMaxAll(BOUND_PART, set_valmin, valmin_dlg, set_valmax, valmax_dlg, &nall);
  for(i = 0; i<nall; i++){
    if(set_valmin[i]!=BOUND_LOADED_MIN){
      valmin[i] = valmin_dlg[i];
    }
    if(set_valmax[i]!=BOUND_LOADED_MAX){
      valmax[i] = valmax_dlg[i];
    }
  }
  SetMinMaxAll(BOUND_PART, set_valmin, valmin, set_valmax, valmax, nall);
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

int cb_up_rgb[3], cb_down_rgb[3];

GLUI_Listbox *LIST_colorbar2 = NULL;

GLUI_Rollout *ROLLOUT_zone_bound=NULL;
GLUI_Rollout *ROLLOUT_coloring=NULL;

#ifdef pp_MEMDEBUG
#define MEMCHECK 1
#endif

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
GLUI_Button *BUTTON_EVAC = NULL;
GLUI_Button *BUTTON_PART = NULL;
GLUI_Button *BUTTON_SLICE = NULL;
GLUI_Button *BUTTON_VSLICE = NULL;
GLUI_Button *BUTTON_PLOT3D = NULL;
GLUI_Button *BUTTON_3DSMOKE = NULL;
GLUI_Button *BUTTON_BOUNDARY = NULL;
GLUI_Button *BUTTON_ISO = NULL;

GLUI_Listbox *LIST_colortable = NULL;
GLUI_Listbox *LIST_iso_colorbar = NULL;

#ifdef pp_MEMDEBUG
GLUI_Rollout *ROLLOUT_memcheck=NULL;
#endif
GLUI_Rollout *ROLLOUT_boundary_duplicates;
GLUI_Rollout *ROLLOUT_iso_settings;
GLUI_Rollout *ROLLOUT_iso_bounds;
GLUI_Rollout *ROLLOUT_iso_color;
GLUI_Rollout *ROLLOUT_script = NULL;
GLUI_Rollout *ROLLOUT_config = NULL;
GLUI_Rollout *ROLLOUT_autoload=NULL;
GLUI_Rollout *ROLLOUT_compress=NULL;
GLUI_Rollout *ROLLOUT_plot3d=NULL,*ROLLOUT_part=NULL,*ROLLOUT_slice=NULL,*ROLLOUT_bound=NULL,*ROLLOUT_iso=NULL;
GLUI_Rollout *ROLLOUT_iso_colors = NULL;
GLUI_Rollout *ROLLOUT_smoke3d=NULL,*ROLLOUT_volsmoke3d=NULL;
GLUI_Rollout *ROLLOUT_time=NULL,*ROLLOUT_colorbar=NULL;
GLUI_Rollout *ROLLOUT_outputpatchdata=NULL;
GLUI_Rollout *ROLLOUT_slice_settings = NULL;
GLUI_Rollout *ROLLOUT_filebounds = NULL;
GLUI_Rollout *ROLLOUT_showhide = NULL;
GLUI_Rollout *ROLLOUT_slice_average = NULL;
GLUI_Rollout *ROLLOUT_slice_histogram = NULL;
GLUI_Rollout *ROLLOUT_line_contour = NULL;
GLUI_Rollout *ROLLOUT_slicedups = NULL;
GLUI_Rollout *ROLLOUT_vector = NULL;
GLUI_Rollout *ROLLOUT_isosurface = NULL;
GLUI_Rollout *ROLLOUT_boundary_settings = NULL;
GLUI_Rollout *ROLLOUT_particle_settings=NULL;
GLUI_Rollout *ROLLOUT_particle_histogram = NULL;
GLUI_Panel *PANEL_keep_bound_data = NULL;
GLUI_Panel *PANEL_keep_plot3d_data = NULL;
GLUI_Rollout *ROLLOUT_extreme2 = NULL;
GLUI_Rollout *ROLLOUT_split = NULL;
GLUI_Rollout *ROLLOUT_files = NULL;
GLUI_Panel *PANEL_vector1=NULL, *PANEL_vector2=NULL;

GLUI_Panel *PANEL_partread = NULL;

GLUI_Panel *PANEL_slice_misc=NULL, *PANEL_slice_vector=NULL, *PANEL_showslice=NULL;
GLUI_Panel *PANEL_plot3d=NULL;
GLUI_Panel *PANEL_boundary_temp_threshold=NULL;
GLUI_Panel *PANEL_slice_buttonsA = NULL;
GLUI_Panel *PANEL_slice_buttonsB = NULL;
GLUI_Panel *PANEL_boundary_outline_type = NULL;
GLUI_Panel *PANEL_iso1 = NULL;
GLUI_Panel *PANEL_iso2 = NULL;
GLUI_Panel *PANEL_geomexp = NULL;
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
GLUI_Panel *PANEL_evac_direction=NULL;
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
GLUI_Panel *PANEL_time2=NULL;
GLUI_Panel *PANEL_time1a=NULL;
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

#ifdef pp_IIMEBAR_DIGITS
GLUI_Spinner *SPINNER_ntimebar_digits = NULL;
#endif
GLUI_Spinner *SPINNER_sliceval_ndigits = NULL;
GLUI_Spinner *SPINNER_npartthread_ids = NULL;
GLUI_Spinner *SPINNER_iso_outline_ioffset = NULL;
GLUI_Spinner *SPINNER_histogram_width_factor = NULL;
GLUI_Spinner *SPINNER_histogram_nbuckets=NULL;
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
GLUI_Spinner *SPINNER_smoke3dzipstep=NULL;
GLUI_Spinner *SPINNER_slicezipstep=NULL;
GLUI_Spinner *SPINNER_isozipstep=NULL;
GLUI_Spinner *SPINNER_boundzipstep=NULL;
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
GLUI_Spinner *SPINNER_labels_transparency_data = NULL;
GLUI_Spinner *SPINNER_down_red = NULL, *SPINNER_down_green = NULL, *SPINNER_down_blue = NULL;
GLUI_Spinner *SPINNER_up_red = NULL, *SPINNER_up_green = NULL, *SPINNER_up_blue = NULL;
GLUI_Spinner *SPINNER_colorsplit[12];
GLUI_Spinner *SPINNER_valsplit[3];

GLUI_Listbox *LIST_scriptlist=NULL;
GLUI_Listbox *LIST_ini_list=NULL;

GLUI_EditText *EDIT_iso_valmin=NULL;
GLUI_EditText *EDIT_iso_valmax=NULL;
GLUI_EditText *EDIT_zone_min=NULL, *EDIT_zone_max=NULL;
GLUI_EditText *EDIT_ini=NULL;
GLUI_EditText *EDIT_renderdir=NULL;
GLUI_EditText *EDIT_rendersuffix=NULL;

GLUI_Checkbox* CHECKBOX_visColorbarHorizontal2 = NULL;
GLUI_Checkbox* CHECKBOX_visColorbarVertical2 = NULL;
GLUI_Checkbox *CHECKBOX_show_boundary_outline=NULL;
GLUI_Checkbox *CHECKBOX_part_multithread = NULL;
GLUI_Checkbox *CHECKBOX_partfast = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_shaded = NULL;
GLUI_Checkbox *CHECKBOX_show_vector_slice = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_outlines = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_points = NULL;
GLUI_Checkbox *CHECKBOX_show_slice_values = NULL;

GLUI_Checkbox *CHECKBOX_show_iso_shaded=NULL;
GLUI_Checkbox *CHECKBOX_show_iso_outline=NULL;
GLUI_Checkbox *CHECKBOX_show_iso_points=NULL;

GLUI_Checkbox *CHECKBOX_slice_load_incremental=NULL;
GLUI_Checkbox *CHECKBOX_histogram_show_numbers=NULL;
GLUI_Checkbox *CHECKBOX_histogram_show_graph=NULL;
GLUI_Checkbox *CHECKBOX_histogram_show_outline=NULL;
GLUI_Checkbox *CHECKBOX_color_vector_black = NULL;
GLUI_Checkbox *CHECKBOX_show_node_slices_and_vectors=NULL;
GLUI_Checkbox *CHECKBOX_show_cell_slices_and_vectors=NULL;
GLUI_Checkbox *CHECKBOX_showpatch_both=NULL;
GLUI_Checkbox *CHECKBOX_showchar=NULL, *CHECKBOX_showonlychar;
GLUI_Checkbox *CHECKBOX_script_step=NULL;
GLUI_Checkbox *CHECKBOX_show_evac_slices=NULL;
GLUI_Checkbox *CHECKBOX_constant_coloring=NULL;
GLUI_Checkbox *CHECKBOX_show_evac_color=NULL;
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
GLUI_Checkbox *CHECKBOX_colorbar_autoflip = NULL;
GLUI_Checkbox *CHECKBOX_labels_shadedata = NULL;
GLUI_Checkbox *CHECKBOX_labels_shade = NULL;
GLUI_Checkbox *CHECKBOX_transparentflag = NULL;
GLUI_Checkbox *CHECKBOX_use_lighting = NULL;
GLUI_Checkbox *CHECKBOX_show_extreme_mindata = NULL;
GLUI_Checkbox *CHECKBOX_show_extreme_maxdata = NULL;

GLUI_RadioGroup *RADIO_iso_setmin=NULL;
GLUI_RadioGroup *RADIO_iso_setmax=NULL;
GLUI_RadioGroup *RADIO_transparency_option=NULL;
GLUI_RadioGroup *RADIO_slice_celltype=NULL;
GLUI_RadioGroup *RADIO_slice_edgetype=NULL;
GLUI_RadioGroup *RADIO_boundary_edgetype = NULL;
GLUI_RadioGroup *RADIO_boundaryslicedup = NULL;
GLUI_RadioGroup *RADIO_slicedup = NULL;
GLUI_RadioGroup *RADIO_vectorslicedup = NULL;
GLUI_RadioGroup *RADIO_histogram_static=NULL;
GLUI_RadioGroup *RADIO_showhide = NULL;
GLUI_RadioGroup *RADIO_contour_type = NULL;
GLUI_RadioGroup *RADIO_zone_setmin=NULL, *RADIO_zone_setmax=NULL;
GLUI_RadioGroup *RADIO_bf=NULL, *RADIO_p3=NULL,*RADIO_slice=NULL;
GLUI_RadioGroup *RADIO_part5=NULL;
GLUI_RadioGroup *RADIO_plot3d_isotype=NULL;
GLUI_RadioGroup *RADIO_plot3d_display=NULL;
GLUI_RadioGroup *RADIO2_plot3d_display = NULL;
GLUI_RadioButton *RADIO_button_cutcell = NULL;


#ifdef pp_MEMDEBUG
GLUI_RadioGroup *RADIO_memcheck=NULL;
#endif

GLUI_RadioButton *RADIOBUTTON_plot3d_iso_hidden=NULL;
GLUI_RadioButton *RADIOBUTTON_zone_permin=NULL;
GLUI_RadioButton *RADIOBUTTON_zone_permax=NULL;

#define FLIP                  19
#define CB_USE_LIGHTING      120
#define COLORBAR_EXTREME_RGB  15
#define COLORBAR_EXTREME      16
// #define SPLIT_COLORBAR         1 now defined in smokeviewdefs.h

//*** boundprocinfo entries
#define ZONE_ROLLOUT     0
#define SMOKE3D_ROLLOUT  1
#define BOUNDARY_ROLLOUT 2
#define ISO_ROLLOUT      3
#define PART_ROLLOUT     4
#define EVAC_ROLLOUT     5
#define PLOT3D_ROLLOUT   6
#define SLICE_ROLLOUT    7
#define TIME_ROLLOUT     8

procdata  boundprocinfo[9];
int      nboundprocinfo = 0;

//*** isoprocinfo entries
#define ISO_ROLLOUT_BOUNDS   0
#define ISO_ROLLOUT_SETTINGS 1
#define ISO_ROLLOUT_COLOR    2
procdata  isoprocinfo[3];
int      nisoprocinfo=0;


//*** sliceprocinfo entries
#define SLICE_BOUND             0
#define SLICE_CHOP              1
#define SLICE_AVERAGE_ROLLOUT   2
#define SLICE_VECTOR_ROLLOUT    3
#define LINE_CONTOUR_ROLLOUT    4
#define SLICE_HISTOGRAM_ROLLOUT 5
#define SLICE_DUP_ROLLOUT       6
#define SLICE_SETTINGS_ROLLOUT  7
procdata  sliceprocinfo[8];
int      nsliceprocinfo=0;

//*** plot3dprocinfo entries
#define PLOT3D_BOUND              0
#define PLOT3D_CHOP               1
#define PLOT3D_VECTOR_ROLLOUT     2
#define PLOT3D_ISOSURFACE_ROLLOUT 3

procdata  plot3dprocinfo[4];
int      nplot3dprocinfo=0;


//*** fileprocinfo entries
#define LOAD_ROLLOUT       0
#define SHOWHIDE_ROLLOUT   1
#define COMPRESS_ROLLOUT   2
#define SCRIPT_ROLLOUT     3
#define CONFIG_ROLLOUT     4
#define FILEBOUNDS_ROLLOUT 5
#define COLORING_ROLLOUT   6
#define MEMCHECK_ROLLOUT   7

procdata  fileprocinfo[8];
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

#ifdef pp_REFRESH
/* ------------------ RefreshGluiDialogs ------------------------ */

extern "C" void RefreshGluiDialogs(void){
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

/* ------------------ UpdateSliceSkip ------------------------ */

extern "C" void UpdateSliceSkip(void){
  SliceBoundCB(SLICE_SKIP);
}

/* ------------------ UpdateColorbarControls2 ------------------------ */

extern "C" void UpdateColorbarControls2(void){
  if(CHECKBOX_visColorbarVertical2 != NULL && CHECKBOX_visColorbarVertical2->get_int_val() != visColorbarVertical)CHECKBOX_visColorbarVertical2->set_int_val(visColorbarVertical);
  if(CHECKBOX_visColorbarHorizontal2 != NULL && CHECKBOX_visColorbarHorizontal2->get_int_val() != visColorbarHorizontal)CHECKBOX_visColorbarHorizontal2->set_int_val(visColorbarHorizontal);
}

/* ------------------ UpdatePartType ------------------------ */

extern "C" void UpdatePartType(void){
  if(RADIO_part5!=NULL)RADIO_part5->set_int_val(ipart5prop);
}

/* ------------------ UpdateTransparency ------------------------ */

extern "C" void UpdateTransparency(void){
  if(CHECKBOX_transparentflag!=NULL)CHECKBOX_transparentflag->set_int_val(use_transparency_data);
}

/* ------------------ Plot3DBounds2Glui ------------------------ */

extern "C" void Plot3DBounds2Glui(void){
  glui_p3min = p3min_all[list_p3_index];
  glui_p3max = p3max_all[list_p3_index];
}

/* ------------------ UpdateUseLighting ------------------------ */

extern "C" void UpdateUseLighting(void){
  CHECKBOX_use_lighting->set_int_val(use_lighting);
}

/* ------------------ ExtremeCB ------------------------ */

extern "C" void ExtremeCB(int var){
  colorbardata *cbi;
  unsigned char *rgb_nodes;
  int i;

  switch(var){
  case COLORBAR_EXTREME:
    if(show_extreme_mindata==1){
      if(SPINNER_down_red!=NULL)SPINNER_down_red->enable();
      if(SPINNER_down_green!=NULL)SPINNER_down_green->enable();
      if(SPINNER_down_blue!=NULL)SPINNER_down_blue->enable();
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
    }
    else{
      if(SPINNER_up_red!=NULL)SPINNER_up_red->disable();
      if(SPINNER_up_green!=NULL)SPINNER_up_green->disable();
      if(SPINNER_up_blue!=NULL)SPINNER_up_blue->disable();
    }
    if(colorbartype<0||colorbartype>=ncolorbars)return;
    cbi = colorbarinfo+colorbartype;
    RemapColorbar(cbi);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    updatemenu = 1;
    break;
  case COLORBAR_EXTREME_RGB:
    if(colorbartype<0||colorbartype>=ncolorbars)return;
    cbi = colorbarinfo+colorbartype;

    rgb_nodes = rgb_above_max;
    for(i = 0; i<3; i++){
      rgb_nodes[i] = cb_up_rgb[i];
    }
    rgb_nodes = rgb_below_min;
    for(i = 0; i<3; i++){
      rgb_nodes[i] = cb_down_rgb[i];
    }
    RemapColorbar(cbi);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    break;
  default:
    break;
  }
}

/* ------------------ SplitCB ------------------------ */

extern "C" void SplitCB(int var){
  int isplit, i;
  float denom;

  switch(var){
  case SPLIT_COLORBAR:
    if(split_colorbar==NULL)break;
    denom = splitvals[2]-splitvals[0];
    if(denom==0.0)denom = 1.0;
    isplit = CLAMP(255*(splitvals[1]-splitvals[0])/denom, 0, 254);
    split_colorbar->index_node[1] = isplit;
    split_colorbar->index_node[2] = isplit+1;

    for(i = 0; i<12; i++){
      split_colorbar->rgb_node[i] = colorsplit[i]&0xFF;
    }
    RemapColorbar(split_colorbar);
    UpdateColorbarSplits(split_colorbar);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}


/* ------------------ UpdateExtremeVals ------------------------ */

extern "C" void UpdateExtremeVals(void){
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

/* ------------------ UpdateExtreme ------------------------ */

extern "C" void UpdateExtreme(void){
  if(CHECKBOX_show_extreme_mindata!=NULL){
    CHECKBOX_show_extreme_mindata->set_int_val(show_extreme_mindata);
  }
  if(CHECKBOX_show_extreme_maxdata!=NULL){
    CHECKBOX_show_extreme_maxdata->set_int_val(show_extreme_maxdata);
  }
  ExtremeCB(COLORBAR_EXTREME);
}

/* ------------------ GetColorbarListIndex ------------------------ */

extern "C" int GetColorbarListIndex(void){
  return LIST_colorbar2->get_int_val();
}

/* ------------------ UpdateColorbarList2 ------------------------ */

extern "C" void UpdateColorbarList2(void){
  if(LIST_colorbar2!=NULL)LIST_colorbar2->set_int_val(selectedcolorbar_index2);
}

/* ------------------ AddColorbarList2 ------------------------ */

extern "C" void AddColorbarList2(int index, char *label){
  LIST_colorbar2->add_item(index, label);
}

/* ------------------ SetColorbarListIndex ------------------------ */

extern "C" void SetColorbarListIndex(int val){
  if(LIST_colorbar2!=NULL)LIST_colorbar2->set_int_val(val);
}

/* ------------------ SetColorbarDigits ------------------------ */

extern "C" void SetColorbarDigits(void){
  SPINNER_ncolorlabel_digits->set_int_val(ncolorlabel_digits);
  CHECKBOX_fixedpoint->set_int_val(force_fixedpoint);
  CHECKBOX_exponential->set_int_val(force_exponential);
}

/* ------------------ UpdateColorbarFlip ------------------------ */

extern "C" void UpdateColorbarFlip(void){
  CHECKBOX_colorbar_flip->set_int_val(colorbar_flip);
  CHECKBOX_colorbar_autoflip->set_int_val(colorbar_autoflip);
}

/* ------------------ SetLabelControls2 ------------------------ */

  extern "C" void SetLabelControls2(){
    if(CHECKBOX_labels_shade!=NULL)CHECKBOX_labels_shade->set_int_val(setbw);
    if(CHECKBOX_labels_shadedata != NULL)CHECKBOX_labels_shadedata->set_int_val(setbwdata);
  }

/* ------------------ UpdateGluiPartFast ------------------------ */

extern "C" void UpdateGluiPartFast(void){
  if(CHECKBOX_partfast!=NULL)CHECKBOX_partfast->set_int_val(partfast);
  if(CHECKBOX_part_multithread!=NULL)CHECKBOX_part_multithread->set_int_val(part_multithread);
  PartBoundCB(PARTFAST);
}

/* ------------------ UpdateListIsoColorobar ------------------------ */

extern "C" void UpdateListIsoColorobar(void){
  if(LIST_iso_colorbar!=NULL)LIST_iso_colorbar->set_int_val(iso_colorbar_index);
}


/* ------------------ UpdateGluiIsoBounds ------------------------ */

extern "C" void UpdateGluiIsoBounds(void){
  if(setisomin==PERCENTILE_MIN||setisomin==GLOBAL_MIN){
    if(setisomin==PERCENTILE_MIN)glui_iso_valmin=iso_percentile_min;
    if(setisomin==GLOBAL_MIN)glui_iso_valmin=iso_global_min;
    if(EDIT_iso_valmin!=NULL)EDIT_iso_valmin->set_float_val(glui_iso_valmin);
  }
  if(setisomax==PERCENTILE_MAX||setisomax==GLOBAL_MAX){
    if(setisomax==PERCENTILE_MAX)glui_iso_valmax=iso_percentile_max;
    if(setisomax==GLOBAL_MAX)glui_iso_valmax=iso_global_max;
    if(EDIT_iso_valmax!=NULL)EDIT_iso_valmax->set_float_val(glui_iso_valmax);
  }
}

/* ------------------ UpdateVectorpointsize ------------------------ */

extern "C" void UpdateVectorpointsize(void){
  if(SPINNER_vectorpointsize!=NULL)SPINNER_vectorpointsize->set_int_val(vectorpointsize);
}

/* ------------------ UpdateSliceDupDialog ------------------------ */

extern "C" void UpdateSliceDupDialog(void){
  if(RADIO_boundaryslicedup != NULL)RADIO_boundaryslicedup->set_int_val(boundaryslicedup_option);
  if(RADIO_slicedup != NULL)RADIO_slicedup->set_int_val(slicedup_option);
  if(RADIO_vectorslicedup != NULL)RADIO_vectorslicedup->set_int_val(vectorslicedup_option);
}

/* ------------------ UpdateImmersedControls ------------------------ */

extern "C" void UpdateImmersedControls(void){
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

/* ------------------ UpdateHistogramType ------------------------ */

extern "C" void UpdateHistogramType(void){
  RADIO_histogram_static->set_int_val(histogram_static);
  CHECKBOX_histogram_show_graph->set_int_val(histogram_show_graph);
  CHECKBOX_histogram_show_numbers->set_int_val(histogram_show_numbers);
  CHECKBOX_histogram_show_outline->set_int_val(histogram_show_outline);
}

/* ------------------ UpdateShowSliceInObst ------------------------ */

extern "C" void UpdateShowSliceInObst(int var){
  show_slice_in_obst = var;
  if(show_slice_in_obst==GAS_AND_SOLID){
    show_slice_in_gas   = 1;
    show_slice_in_solid = 1;
  }
  else if(show_slice_in_obst==ONLY_IN_GAS){
    show_slice_in_gas   = 1;
    show_slice_in_solid = 0;
  }
  else if(show_slice_in_obst==ONLY_IN_SOLID){
    show_slice_in_gas   = 0;
    show_slice_in_solid = 1;
  }
  else{
    show_slice_in_gas   = 1;
    show_slice_in_solid = 0;
  }
  show_slice_shaded[IN_GAS_GLUI]   = show_slice_in_gas;
  show_slice_shaded[IN_SOLID_GLUI] = show_slice_in_solid;
  ImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
}

/* ------------------ UpdateIsoColorlevel ------------------------ */

extern "C" void UpdateIsoColorlevel(void){
  IsoBoundCB(ISO_LEVEL);
  IsoBoundCB(ISO_COLORS);
}

/* ------------------ ParticleRolloutCB ------------------------ */

void ParticleRolloutCB(int var){
  ToggleRollout(particleprocinfo, nparticleprocinfo, var);
}

/* ------------------ Plot3dRolloutCB ------------------------ */

void Plot3dRolloutCB(int var){
  ToggleRollout(plot3dprocinfo, nplot3dprocinfo, var);
}

/* ------------------ SliceRolloutCB ------------------------ */

void SliceRolloutCB(int var){
  ToggleRollout(sliceprocinfo, nsliceprocinfo, var);
}

/* ------------------ IsoRolloutCB ------------------------ */

void IsoRolloutCB(int var){
  ToggleRollout(isoprocinfo, nisoprocinfo, var);
}

/* ------------------ BoundRolloutCB ------------------------ */

void BoundRolloutCB(int var){
  ToggleRollout(boundprocinfo, nboundprocinfo, var);
  if(nzoneinfo>0){
    if(var==ZONE_ROLLOUT){
      SliceBoundCB(SETZONEVALMINMAX);
    }
    if(var==SLICE_ROLLOUT){
      list_slice_index = CLAMP(list_slice_index,0,nlist_slice_index-1);
      if(RADIO_slice!=NULL)RADIO_slice->set_int_val(list_slice_index);
      SliceBoundCB(FILETYPE_INDEX);
    }
  }
}

/* ------------------ SubBoundRolloutCB ------------------------ */

void SubBoundRolloutCB(int var){
  ToggleRollout(subboundprocinfo, nsubboundprocinfo, var);
}

/* ------------------ FileRolloutCB ------------------------ */

void FileRolloutCB(int var){
  ToggleRollout(fileprocinfo, nfileprocinfo, var);
}

/* ------------------ UpdateGluiZoneBounds ------------------------ */

extern "C" void UpdateGluiZoneBounds(void){
  if(EDIT_zone_min!=NULL)EDIT_zone_min->set_float_val(zonemin);
  if(EDIT_zone_max!=NULL)EDIT_zone_max->set_float_val(zonemax);
}
/* ------------------ UpdateGluiVecFactor ------------------------ */

extern "C" void UpdateGluiVecFactor(void){
  if(SPINNER_plot3d_vectorlinelength!=NULL)SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
  if(SPINNER_vectorlinelength!=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
}

/* ------------------ UpdateResearchMode ------------------------ */

extern "C" void UpdateResearchMode(void){
  SliceBoundCB(RESEARCH_MODE);
}
/* ------------------ UpdateScriptStop ------------------------ */

extern "C" void UpdateScriptStop(void){
  if(BUTTON_script_start!=NULL)BUTTON_script_start->enable();
  if(BUTTON_script_stop!=NULL)BUTTON_script_stop->disable();
  if(BUTTON_script_runscript!=NULL)BUTTON_script_runscript->enable();
  if(EDIT_renderdir!=NULL)EDIT_renderdir->enable();
}

/* ------------------ UpdateScriptStart ------------------------ */

extern "C" void UpdateScriptStart(void){
  if(BUTTON_script_start!=NULL)BUTTON_script_start->disable();
  if(BUTTON_script_stop!=NULL)BUTTON_script_stop->enable();
  if(BUTTON_script_runscript!=NULL)BUTTON_script_runscript->disable();
  if(EDIT_renderdir!=NULL)EDIT_renderdir->disable();
}

/* ------------------ UpdateScriptStep ------------------------ */

extern "C" void UpdateScriptStep(void){
  CHECKBOX_script_step->set_int_val(script_step);
  if(script_step==1){
    BUTTON_step->enable();
  }
  else{
    BUTTON_step->disable();
  }
}

/* ------------------ UpdateEvacParms ------------------------ */

extern "C" void UpdateEvacParms(void){
  if(CHECKBOX_show_evac_slices!=NULL)CHECKBOX_show_evac_slices->set_int_val(show_evac_slices);
  if(CHECKBOX_constant_coloring!=NULL)CHECKBOX_constant_coloring->set_int_val(constant_evac_coloring);
  if(CHECKBOX_data_coloring!=NULL)CHECKBOX_data_coloring->set_int_val(data_evac_coloring);
  if(CHECKBOX_show_evac_color!=NULL)CHECKBOX_show_evac_color->set_int_val(show_evac_colorbar);
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

/* ------------------ UpdateColorTableList ------------------------ */

extern "C" void UpdateColorTableList(int ncolortableinfo_old){
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


  for(i = -1; i<ncolortableinfo; i++){
    if(i==-1){
      LIST_colortable->add_item(i, "Custom");
    }
    else{
      colortabledata *cti;

      cti = colortableinfo+order[i];
      LIST_colortable->add_item(i, cti->label);
    }
  }
  if(ncolortableinfo>0){
    FREEMEMORY(order);
  }
}

/* ------------------ FileShowCB ------------------------ */

extern "C" void FileShowCB(int var){
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
      ASSERT(FFALSE);
      break;
    }
    break;
  case FILESHOW_evac:
    switch(showhide_option){
    case SHOWALL_FILES:
      EvacShowMenu(SHOWALL_EVAC);
      break;
    case SHOWONLY_FILE:
      EvacShowMenu(SHOWALL_EVAC);
      if(npartloaded != 0)ParticleShowMenu(HIDEALL_PARTICLE);
      if(nsmoke3dloaded != 0)Smoke3DShowMenu(HIDEALL_SMOKE3D);
      if(nisoloaded != 0)IsoShowMenu(HIDEALL_ISO);
      if(nsliceloaded != 0)ShowHideSliceMenu(GLUI_HIDEALL_SLICE);
      if(nvsliceloaded != 0)ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
      if(npatchloaded != 0)ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
      break;
    case HIDEALL_FILES:
      EvacShowMenu(HIDEALL_EVAC);
      break;
    default:
      ASSERT(FFALSE);
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
      if(nevacloaded != 0)EvacShowMenu(HIDEALL_EVAC);
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
      ASSERT(FFALSE);
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
      if(nevacloaded != 0)EvacShowMenu(HIDEALL_EVAC);
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
      ASSERT(FFALSE);
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
      if(nevacloaded != 0)EvacShowMenu(HIDEALL_EVAC);
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
      ASSERT(FFALSE);
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
      if(nevacloaded != 0)EvacShowMenu(HIDEALL_EVAC);
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
      ASSERT(FFALSE);
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
      if(nevacloaded != 0)EvacShowMenu(HIDEALL_EVAC);
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
      ASSERT(FFALSE);
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
      if(nevacloaded != 0)EvacShowMenu(HIDEALL_EVAC);
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
      ASSERT(FFALSE);
      break;
    }
    break;
  default:
    break;
  }
}

#ifdef pp_MEMDEBUG

/* ------------------ MemcheckCB ------------------------ */

void MemcheckCB(int var){
  switch(var){
  case MEMCHECK:
    set_memcheck(list_memcheck_index);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}
#endif

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
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ ImmersedBoundCB ------------------------ */

#define SHOW_POLYGON_EDGES 0
#define SHOW_TRIANGLE_EDGES 1
#define HIDE_EDGES 2
extern "C" void ImmersedBoundCB(int var){
  updatemenu = 1;
  switch(var){
    int i;

  case IMMERSED_SWITCH_CELLTYPE:
    glui_show_vector_slice   = show_vector_slice[slice_celltype];
    glui_slice_edgetype      = slice_edgetypes[slice_celltype];
    glui_show_slice_shaded   = show_slice_shaded[slice_celltype];
    glui_show_slice_outlines = show_slice_outlines[slice_celltype];
    glui_show_slice_points   = show_slice_points[slice_celltype];
    glui_show_slice_values   = show_slice_values[slice_celltype];
    for(i=0;i<3;i++){
      switch(slice_edgetypes[i]){
        case OUTLINE_POLYGON:
        case OUTLINE_TRIANGLE:
          show_slice_outlines[i]=1;
          break;
        case OUTLINE_HIDDEN:
          show_slice_outlines[i]=0;
          break;
	default:
	  ASSERT(FFALSE);
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
    ImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
    break;
  case HIDE_ALL_SLICE_VALUES:
    show_slice_values[0] = 0;
    show_slice_values[1] = 0;
    show_slice_values[2] = 0;
    show_slice_values_all_regions = 0;
    ImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
    break;
  case IMMERSED_SET_DRAWTYPE:
    if(glui_show_slice_outlines == 0){
      glui_slice_edgetype = OUTLINE_HIDDEN;
    }
    else{
      if(glui_slice_edgetype == OUTLINE_HIDDEN)glui_slice_edgetype = OUTLINE_TRIANGLE;
    }
    show_vector_slice[slice_celltype]   = glui_show_vector_slice;
    slice_edgetypes[slice_celltype]     = glui_slice_edgetype;
    show_slice_shaded[slice_celltype]   = glui_show_slice_shaded;
    show_slice_outlines[slice_celltype] = glui_show_slice_outlines;
    show_slice_points[slice_celltype]   = glui_show_slice_points;
    show_slice_values[slice_celltype] = glui_show_slice_values;
    if(RADIO_slice_edgetype!=NULL)RADIO_slice_edgetype->set_int_val(glui_slice_edgetype);
    break;
  case IMMERSED_SWITCH_EDGETYPE:
    switch (glui_slice_edgetype){
    case SHOW_POLYGON_EDGES:
    case SHOW_TRIANGLE_EDGES:
      glui_show_slice_outlines=1;
      break;
    case HIDE_EDGES:
      glui_show_slice_outlines=0;
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
    ImmersedBoundCB(IMMERSED_SET_DRAWTYPE);
    if(CHECKBOX_show_slice_outlines!=NULL)CHECKBOX_show_slice_outlines->set_int_val(glui_show_slice_outlines);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ BoundBoundCB ------------------------ */

extern "C" void BoundBoundCB(int var){
  int i;

  switch(var){
  case SHOW_BOUNDARY_OUTLINE:
    if(ngeom_data==0)break;
    if(show_boundary_outline==1&&boundary_edgetype==OUTLINE_HIDDEN)boundary_edgetype = OUTLINE_POLYGON;
    if(show_boundary_outline==0&&boundary_edgetype!=OUTLINE_HIDDEN)boundary_edgetype = OUTLINE_HIDDEN;
    if(boundary_edgetype!=RADIO_boundary_edgetype->get_int_val())RADIO_boundary_edgetype->set_int_val(boundary_edgetype);
    break;
  case BOUNDARY_EDGETYPE:
    if(boundary_edgetype==OUTLINE_HIDDEN&&show_boundary_outline==1)show_boundary_outline=0;
    if(boundary_edgetype!=OUTLINE_HIDDEN&&show_boundary_outline==0)show_boundary_outline=1;
    if(show_boundary_outline!=CHECKBOX_show_boundary_outline->get_int_val())CHECKBOX_show_boundary_outline->set_int_val(show_boundary_outline);
    break;
  case UPDATE_BOUNDARYSLICEDUPS:
    UpdateBoundarySliceDups();
    updatemenu = 1;
    break;
  case SHOWPATCH_BOTH:
    updatefacelists = 1;
    updatehiddenfaces = 1;
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
  case FRAMELOADING:
    boundframestep = boundframeskip + 1;
    boundzipstep = boundzipskip + 1;
    updatemenu = 1;
    break;
  case CHOPUPDATE:
    UpdateChopColors();
    break;
  case SETCHOPMINVAL:
    UpdateChopColors();
    if (patchlabellist != NULL)GLUI2GlobalBoundaryBounds(patchlabellist[list_patch_index]);
    switch(setpatchchopmin){
    case DISABLE:
      break;
    case ENABLE:
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
    UpdateHideBoundarySurface();
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
      ASSERT(FFALSE);
      break;
    }
    UpdateHideBoundarySurface();
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
    UpdateHideBoundarySurface();
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
    GetGlobalPatchBounds();
    if(patchlabellist != NULL)Global2GLUIBoundaryBounds(patchlabellist[list_patch_index]);
    UpdateAllBoundaryColors();
    break;
  case FILE_RELOAD:
    if(npatchinfo>0){
//      BoundBoundCB(FILE_UPDATE);
      for(i = 0;i < npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo + i;
        if(patchi->loaded == 0)continue;
        LoadBoundaryMenu(i);
      }
    }
    break;
  case COMPRESS_FILES:
    CompressSVZip();
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
  case STARTUP:
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
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ Smoke3dBoundCB ------------------------ */

void Smoke3dBoundCB(int var){
  switch(var){
  case FRAMELOADING:
    smoke3dframestep = smoke3dframeskip + 1;
    smoke3dzipstep = smoke3dzipskip + 1;
    updatemenu = 1;
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ TimeBoundCB ------------------------ */

void TimeBoundCB(int var){

  updatemenu = 1;
  switch(var){
  case SET_TIME:
    SetTimeVal(glui_time);
    break;
  case TBOUNDS:
    if(use_tload_begin == 1 || use_tload_end == 1 || use_tload_skip == 1){
      UpdateTBounds();
    }
    break;
#ifdef pp_TIMEBAR_DIGITS
  case TIMEBAR_DIGITS:
    break;
#endif
  case TBOUNDS_USE:
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
    UpdateTBounds();
    break;
  case RELOAD_ALL_DATA:
    ReloadMenu(RELOAD_ALL_NOW);
    break;
  case RELOAD_INCREMENTAL_DATA:
    ReloadMenu(RELOAD_INCREMENTAL_NOW);
    break;
  default:
    ASSERT(FFALSE);
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
    GluiScriptEnable();
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
#ifdef WIN32
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
#ifdef WIN32
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
      strcpy(script_renderfile, fdsprefix);
      strcat(script_renderfile, "_");
      strcat(script_renderfile, suffix);
      strcpy(label, _("Render: "));
      strcat(label, script_renderfile);
    }
    else{
      strcpy(label, _("Render"));
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
      strcpy(label, _("Run:"));
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
    if(strcmp(ini_filename, caseini_filename) == 0){
      ReadIni(NULL);
    }
    else if(id >= 0){
      char *script_filename2;

      if(strlen(ini_filename) == 0)break;
      script_filename2 = script_filename;
      strcpy(script_filename, ini_filename);
      windowresized = 0;
      ReadIni(script_filename2);
    }
    if(scriptoutstream != NULL){
      fprintf(scriptoutstream, "LOADINIFILE\n");
      fprintf(scriptoutstream, " %s\n", ini_filename);
    }
  }
  break;
  case SCRIPT_STEP:
    UpdateScriptStep();
    updatemenu = 1;
    break;
  case SCRIPT_EDIT_INI:
    strcpy(label, _("Save "));
    strcat(label, fdsprefix);
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
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ GluiBoundsSetup ------------------------ */

extern "C" void GluiBoundsSetup(int main_window){
  int i;
  int have_part, have_evac;

  if(glui_bounds!=NULL){
    glui_bounds->close();
    glui_bounds=NULL;
  }
  overwrite_all=0;
  glui_bounds = GLUI_Master.create_glui( "Files/Data/Coloring",0,0,0 );
  glui_bounds->hide();

  ROLLOUT_files = glui_bounds->add_rollout("Files", false);

  ROLLOUT_autoload = glui_bounds->add_rollout_to_panel(ROLLOUT_files,_("Auto load"), false, LOAD_ROLLOUT, FileRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_autoload, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_autoload, LOAD_ROLLOUT, glui_bounds);

  glui_bounds->add_checkbox_to_panel(ROLLOUT_autoload, _("Auto load at startup"),
    &loadfiles_at_startup, STARTUP, BoundBoundCB);
  glui_bounds->add_button_to_panel(ROLLOUT_autoload, _("Save auto load file list"), SAVE_FILE_LIST, BoundBoundCB);
  glui_bounds->add_button_to_panel(ROLLOUT_autoload, _("Auto load now"), LOAD_FILES, BoundBoundCB);

  // -------------- Show/Hide Loaded files -------------------

  if(npartinfo > 0 || nsliceinfo > 0 || nvsliceinfo > 0 || nisoinfo > 0 || npatchinfo || nsmoke3dinfo > 0 || nplot3dinfo > 0){
    ROLLOUT_showhide = glui_bounds->add_rollout_to_panel(ROLLOUT_files,_("Show/Hide"), false, SHOWHIDE_ROLLOUT, FileRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_showhide, glui_bounds);
    ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_showhide, SHOWHIDE_ROLLOUT, glui_bounds);

    RADIO_showhide = glui_bounds->add_radiogroup_to_panel(ROLLOUT_showhide, &showhide_option);
    glui_bounds->add_radiobutton_to_group(RADIO_showhide, _("Show"));
    glui_bounds->add_radiobutton_to_group(RADIO_showhide, _("Show only"));
    glui_bounds->add_radiobutton_to_group(RADIO_showhide, _("Hide"));

    glui_bounds->add_column_to_panel(ROLLOUT_showhide, false);

    if(nevac > 0){}
    if(npartinfo > 0 && nevac != npartinfo)BUTTON_PART = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Particle", FILESHOW_particle, FileShowCB);
    if(nevac > 0)BUTTON_EVAC = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Evacuation", FILESHOW_evac, FileShowCB);
    if(nsliceinfo > 0)BUTTON_SLICE = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Slice", FILESHOW_slice, FileShowCB);
    if(nvsliceinfo > 0)BUTTON_VSLICE = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Vector", FILESHOW_vslice, FileShowCB);
    if(nisoinfo > 0)BUTTON_ISO = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Isosurface", FILESHOW_isosurface, FileShowCB);
    if(npatchinfo > 0)BUTTON_BOUNDARY = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Boundary", FILESHOW_boundary, FileShowCB);
    if(nsmoke3dinfo > 0)BUTTON_3DSMOKE = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "3D smoke/fire", FILESHOW_3dsmoke, FileShowCB);
    if(nplot3dinfo > 0)BUTTON_PLOT3D = glui_bounds->add_button_to_panel(ROLLOUT_showhide, "Plot3D", FILESHOW_plot3d, FileShowCB);
    glui_bounds->add_button_to_panel(ROLLOUT_showhide, "File Sizes", FILESHOW_sizes, FileShowCB);


    UpdateShowHideButtons();
  }


#ifdef pp_COMPRESS
  if(smokezippath != NULL && (npatchinfo > 0 || nsmoke3dinfo > 0 || nsliceinfo > 0)){
    ROLLOUT_compress = glui_bounds->add_rollout_to_panel(ROLLOUT_files,_("Compress"), false, COMPRESS_ROLLOUT, FileRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_compress, glui_bounds);
    ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_compress, COMPRESS_ROLLOUT, glui_bounds);

    CHECKBOX_erase_all = glui_bounds->add_checkbox_to_panel(ROLLOUT_compress, _("Erase compressed files"),
      &erase_all, ERASE, BoundBoundCB);
    CHECKBOX_overwrite_all = glui_bounds->add_checkbox_to_panel(ROLLOUT_compress, _("Overwrite compressed files"),
      &overwrite_all, OVERWRITE, BoundBoundCB);
    CHECKBOX_compress_autoloaded = glui_bounds->add_checkbox_to_panel(ROLLOUT_compress, _("Compress only autoloaded files"),
      &compress_autoloaded, COMPRESS_AUTOLOADED, BoundBoundCB);
    if(nsliceinfo > 0){
      SPINNER_slicezipstep = glui_bounds->add_spinner_to_panel(ROLLOUT_compress, _("Slice frame Skip"), GLUI_SPINNER_INT, &slicezipskip,
        FRAMELOADING, SliceBoundCB);
      SPINNER_slicezipstep->set_int_limits(0, 100);
    }
    if(nisoinfo > 0){
      SPINNER_isozipstep = glui_bounds->add_spinner_to_panel(ROLLOUT_compress, _("Compressed file frame skip"), GLUI_SPINNER_INT, &isozipskip,
        FRAMELOADING, IsoBoundCB);
      SPINNER_isozipstep->set_int_limits(0, 100);
    }
    if(nsmoke3dinfo > 0){
      SPINNER_smoke3dzipstep = glui_bounds->add_spinner_to_panel(ROLLOUT_compress, _("3D smoke frame skip"), GLUI_SPINNER_INT, &smoke3dzipskip,
        FRAMELOADING, Smoke3dBoundCB);
      SPINNER_smoke3dzipstep->set_int_limits(0, 100);
    }
    if(npatchinfo > 0){
      SPINNER_boundzipstep = glui_bounds->add_spinner_to_panel(ROLLOUT_compress, _("Boundary file frame skip"),
        GLUI_SPINNER_INT, &boundzipskip, FRAMELOADING, BoundBoundCB);
      SPINNER_boundzipstep->set_int_limits(0, 100);
    }
    BUTTON_compress = glui_bounds->add_button_to_panel(ROLLOUT_compress, _("Run smokezip"), COMPRESS_FILES, BoundBoundCB);
  }
#endif

  ROLLOUT_script = glui_bounds->add_rollout_to_panel(ROLLOUT_files,_("Scripts"), false, SCRIPT_ROLLOUT, FileRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_script, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_script, SCRIPT_ROLLOUT, glui_bounds);

  PANEL_script1 = glui_bounds->add_panel_to_panel(ROLLOUT_script, _("Script files"), false);
  PANEL_record = glui_bounds->add_panel_to_panel(PANEL_script1, _("Record"), true);

  PANEL_script1a = glui_bounds->add_panel_to_panel(PANEL_record, "", false);
  BUTTON_script_start = glui_bounds->add_button_to_panel(PANEL_script1a, _("Start"), SCRIPT_START, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script1a, false);
  BUTTON_script_stop = glui_bounds->add_button_to_panel(PANEL_script1a, _("Stop"), SCRIPT_STOP, ScriptCB);
  BUTTON_script_stop->disable();

  PANEL_run = glui_bounds->add_panel_to_panel(PANEL_script1, _("Run"), true);
  PANEL_script1b = glui_bounds->add_panel_to_panel(PANEL_run, "", false);
  BUTTON_script_runscript = glui_bounds->add_button_to_panel(PANEL_script1b, _("Run script"), SCRIPT_RUNSCRIPT, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script1b, false);
  CHECKBOX_script_step = glui_bounds->add_checkbox_to_panel(PANEL_run, _("Step through script"), &script_step, SCRIPT_STEP, ScriptCB);
  BUTTON_step = glui_bounds->add_button_to_panel(PANEL_run, _("Next"), SCRIPT_STEP_NOW, ScriptCB);
  UpdateScriptStep();
  glui_bounds->add_button_to_panel(PANEL_run, _("Cancel script"), SCRIPT_CANCEL_NOW, ScriptCB);

  LIST_scriptlist = glui_bounds->add_listbox_to_panel(PANEL_script1b, _("Select:"), &script_index, SCRIPT_LIST, ScriptCB);
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
  INSERT_ROLLOUT(ROLLOUT_config, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_config, CONFIG_ROLLOUT, glui_bounds);

  PANEL_script2a = glui_bounds->add_panel_to_panel(ROLLOUT_config, "", false);
  EDIT_ini = glui_bounds->add_edittext_to_panel(PANEL_script2a, "suffix:", GLUI_EDITTEXT_TEXT, script_inifile_suffix, SCRIPT_EDIT_INI, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script2a, false);
  BUTTON_script_setsuffix = glui_bounds->add_button_to_panel(PANEL_script2a, _("Set"), SCRIPT_SETSUFFIX, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script2a, false);
  BUTTON_script_saveini = glui_bounds->add_button_to_panel(PANEL_script2a, _("Save:"), SCRIPT_SAVEINI, ScriptCB);
  ScriptCB(SCRIPT_EDIT_INI);

  PANEL_script2b = glui_bounds->add_panel_to_panel(ROLLOUT_config, "", false);
  ini_index = -2;
  LIST_ini_list = glui_bounds->add_listbox_to_panel(PANEL_script2b, _("Select:"), &ini_index);
  {
    inifiledata *inifile;

    for(inifile = first_inifile.next; inifile->next != NULL; inifile = inifile->next){
      if(inifile->file != NULL&&FILE_EXISTS(inifile->file) == YES){
        if(ini_index == -2)ini_index = inifile->id;
        LIST_ini_list->add_item(inifile->id, inifile->file);
      }
    }
  }
  glui_bounds->add_column_to_panel(PANEL_script2b, false);
  BUTTON_ini_load = glui_bounds->add_button_to_panel(PANEL_script2b, _("Load"), SCRIPT_LOADINI, ScriptCB);

  PANEL_script3 = glui_bounds->add_panel_to_panel(ROLLOUT_script, _("Render"), true);
  EDIT_renderdir = glui_bounds->add_edittext_to_panel(PANEL_script3, _("directory:"),
    GLUI_EDITTEXT_TEXT, script_renderdir, SCRIPT_RENDER_DIR, ScriptCB);
  EDIT_renderdir->set_w(260);
  PANEL_script1c = glui_bounds->add_panel_to_panel(PANEL_script3, "", false);
  BUTTON_script_render = glui_bounds->add_button_to_panel(PANEL_script1c, _("Render"), SCRIPT_RENDER, ScriptCB);
  glui_bounds->add_column_to_panel(PANEL_script1c, false);
  EDIT_rendersuffix = glui_bounds->add_edittext_to_panel(PANEL_script1c, _("suffix:"),
    GLUI_EDITTEXT_TEXT, script_renderfilesuffix, SCRIPT_RENDER_SUFFIX, ScriptCB);
  EDIT_rendersuffix->set_w(130);
  ScriptCB(SCRIPT_RENDER_SUFFIX);

// ----------------------------------- Bounds ----------------------------------------

  ROLLOUT_filebounds = glui_bounds->add_rollout(_("Data"), false, FILEBOUNDS_ROLLOUT, FileRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_filebounds, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_filebounds, FILEBOUNDS_ROLLOUT, glui_bounds);

  /*  zone (cfast) */

  if(nzoneinfo>0){
    ROLLOUT_zone_bound = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds,_("Zone/slice temperatures"),false,ZONE_ROLLOUT,BoundRolloutCB);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_zone_bound, ZONE_ROLLOUT, glui_bounds);

    PANEL_zone_a = glui_bounds->add_panel_to_panel(ROLLOUT_zone_bound,"",GLUI_PANEL_NONE);

    EDIT_zone_min = glui_bounds->add_edittext_to_panel(PANEL_zone_a,"",GLUI_EDITTEXT_FLOAT,&zonemin,ZONEVALMINMAX,SliceBoundCB);
    glui_bounds->add_column_to_panel(PANEL_zone_a,false);

    RADIO_zone_setmin = glui_bounds->add_radiogroup_to_panel(PANEL_zone_a,&setzonemin,SETZONEVALMINMAX,SliceBoundCB);
    RADIOBUTTON_zone_permin=glui_bounds->add_radiobutton_to_group(RADIO_zone_setmin,_("percentile min"));
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmin,_("set min"));
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmin,_("global min"));

    PANEL_zone_b = glui_bounds->add_panel_to_panel(ROLLOUT_zone_bound,"",GLUI_PANEL_NONE);

    EDIT_zone_max = glui_bounds->add_edittext_to_panel(PANEL_zone_b,"",GLUI_EDITTEXT_FLOAT,&zonemax,ZONEVALMINMAX,SliceBoundCB);
    glui_bounds->add_column_to_panel(PANEL_zone_b,false);

    RADIO_zone_setmax = glui_bounds->add_radiogroup_to_panel(PANEL_zone_b,&setzonemax,SETZONEVALMINMAX,SliceBoundCB);
    RADIOBUTTON_zone_permax=glui_bounds->add_radiobutton_to_group(RADIO_zone_setmax,_("percentile max"));
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmax,_("set max"));
    glui_bounds->add_radiobutton_to_group(RADIO_zone_setmax,_("global max"));

    RADIOBUTTON_zone_permin->disable();
    RADIOBUTTON_zone_permax->disable();
    SliceBoundCB(SETZONEVALMINMAX);
  }

  // ----------------------------------- 3D smoke ----------------------------------------

  if(nsmoke3dinfo>0||nvolrenderinfo>0){
    ROLLOUT_smoke3d = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds,_("3D smoke"),false,SMOKE3D_ROLLOUT,BoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_smoke3d, glui_bounds);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_smoke3d, SMOKE3D_ROLLOUT, glui_bounds);
  }

  // ----------------------------------- Boundary ----------------------------------------

  if(npatchinfo>0){
    glui_active=1;
    ROLLOUT_bound = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds,_("Boundary"),false,BOUNDARY_ROLLOUT,BoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_bound, glui_bounds);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_bound, BOUNDARY_ROLLOUT, glui_bounds);

    patchboundsCPP.setup("boundary", ROLLOUT_bound, patchbounds_cpp, npatchbounds_cpp, &cache_boundary_data, SHOW_CACHE_CHECKBOX, PERCENTILE_ENABLED, PatchBoundsCPP_CB,
                         SubBoundRolloutCB, subboundprocinfo, &nsubboundprocinfo);

    ROLLOUT_outputpatchdata = glui_bounds->add_rollout_to_panel(ROLLOUT_bound,_("Output data"),false,
             BOUNDARY_OUTPUT_ROLLOUT,SubBoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_outputpatchdata, glui_bounds);
    ADDPROCINFO(subboundprocinfo, nsubboundprocinfo, ROLLOUT_outputpatchdata, BOUNDARY_OUTPUT_ROLLOUT, glui_bounds);

    glui_bounds->add_checkbox_to_panel(ROLLOUT_outputpatchdata,_("Output data to file"),&output_patchdata);

    PANEL_outputpatchdata = glui_bounds->add_panel_to_panel(ROLLOUT_outputpatchdata,"",GLUI_PANEL_NONE);

    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"tmin",GLUI_SPINNER_FLOAT,&patchout_tmin);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"xmin",GLUI_SPINNER_FLOAT,&patchout_xmin);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"ymin",GLUI_SPINNER_FLOAT,&patchout_ymin);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"zmin",GLUI_SPINNER_FLOAT,&patchout_zmin);

    glui_bounds->add_column_to_panel(PANEL_outputpatchdata,false);

    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"tmax",GLUI_SPINNER_FLOAT,&patchout_tmax);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"xmax",GLUI_SPINNER_FLOAT,&patchout_xmax);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"ymax",GLUI_SPINNER_FLOAT,&patchout_ymax);
    glui_bounds->add_spinner_to_panel(PANEL_outputpatchdata,"zmax",GLUI_SPINNER_FLOAT,&patchout_zmax);

    ROLLOUT_boundary_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_bound, _("Settings"),false, BOUNDARY_SETTINGS_ROLLOUT, SubBoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_boundary_settings, glui_bounds);
    ADDPROCINFO(subboundprocinfo, nsubboundprocinfo, ROLLOUT_boundary_settings, BOUNDARY_SETTINGS_ROLLOUT, glui_bounds);

    if(ngeom_data > 0){
      glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, _("shaded"), &show_boundary_shaded);
      CHECKBOX_show_boundary_outline=glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, _("outline"), &show_boundary_outline, SHOW_BOUNDARY_OUTLINE, BoundBoundCB);
      glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, _("points"), &show_boundary_points);

      PANEL_boundary_outline_type = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings,"outline type");
      RADIO_boundary_edgetype = glui_bounds->add_radiogroup_to_panel(PANEL_boundary_outline_type, &boundary_edgetype, BOUNDARY_EDGETYPE, BoundBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_boundary_edgetype, _("polygon"));
      glui_bounds->add_radiobutton_to_group(RADIO_boundary_edgetype, _("triangle"));
      glui_bounds->add_radiobutton_to_group(RADIO_boundary_edgetype, _("none"));
      BoundBoundCB(BOUNDARY_EDGETYPE);
      BoundBoundCB(SHOW_BOUNDARY_OUTLINE);

      PANEL_geomexp = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings,"experimental");
      glui_bounds->add_checkbox_to_panel(PANEL_geomexp, _("smooth normals"), &geomdata_smoothnormals);
      glui_bounds->add_checkbox_to_panel(PANEL_geomexp, _("smooth color/data"), &geomdata_smoothcolors);
      glui_bounds->add_checkbox_to_panel(PANEL_geomexp, _("lighting"), &geomdata_lighting);

      glui_bounds->add_spinner_to_panel(ROLLOUT_boundary_settings, "line width", GLUI_SPINNER_FLOAT, &geomboundary_linewidth);
      glui_bounds->add_spinner_to_panel(ROLLOUT_boundary_settings, "point size", GLUI_SPINNER_FLOAT, &geomboundary_pointsize);
      glui_bounds->add_separator_to_panel(ROLLOUT_boundary_settings);
    }
    if(activate_threshold==1){
      PANEL_boundary_temp_threshold = glui_bounds->add_panel_to_panel(ROLLOUT_boundary_settings, _("Temperature threshold"));
      CHECKBOX_showchar = glui_bounds->add_checkbox_to_panel(PANEL_boundary_temp_threshold, _("Show"), &vis_threshold, SHOWCHAR, BoundBoundCB);
      CHECKBOX_showonlychar = glui_bounds->add_checkbox_to_panel(PANEL_boundary_temp_threshold, _("Show only threshold"), &vis_onlythreshold, SHOWCHAR, BoundBoundCB);
      {
        char label[256];

        strcpy(label, "Temperature (");
        strcat(label, (const char *)degC);
        strcat(label, ") ");
        glui_bounds->add_spinner_to_panel(PANEL_boundary_temp_threshold, label, GLUI_SPINNER_FLOAT, &temp_threshold);
      }
      BoundBoundCB(SHOWCHAR);
    }
    CHECKBOX_showpatch_both = glui_bounds->add_checkbox_to_panel(ROLLOUT_boundary_settings, _("Display exterior data"), &showpatch_both, SHOWPATCH_BOTH, BoundBoundCB);


    if(nboundaryslicedups > 0){
      ROLLOUT_boundary_duplicates = glui_bounds->add_rollout_to_panel(ROLLOUT_bound, "Duplicates", false,BOUNDARY_DUPLICATE_ROLLOUT,SubBoundRolloutCB);
      INSERT_ROLLOUT(ROLLOUT_boundary_duplicates, glui_bounds);
      ADDPROCINFO(subboundprocinfo, nsubboundprocinfo, ROLLOUT_boundary_duplicates, BOUNDARY_DUPLICATE_ROLLOUT, glui_bounds);

      RADIO_boundaryslicedup = glui_bounds->add_radiogroup_to_panel(ROLLOUT_boundary_duplicates, &boundaryslicedup_option,UPDATE_BOUNDARYSLICEDUPS,BoundBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_boundaryslicedup, _("Keep all"));
      glui_bounds->add_radiobutton_to_group(RADIO_boundaryslicedup, _("Keep fine"));
      glui_bounds->add_radiobutton_to_group(RADIO_boundaryslicedup, _("Keep coarse"));
    }
  }

  // ----------------------------------- Isosurface ----------------------------------------

  if(nisoinfo>0){
    ROLLOUT_iso = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds, "Isosurface", false, ISO_ROLLOUT, BoundRolloutCB);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_iso, ISO_ROLLOUT, glui_bounds);

    if(niso_bounds>0){
      ROLLOUT_iso_bounds = glui_bounds->add_rollout_to_panel(ROLLOUT_iso, "Bound data", true, ISO_ROLLOUT_BOUNDS, IsoRolloutCB);
      INSERT_ROLLOUT(ROLLOUT_iso_bounds, glui_bounds);
      ADDPROCINFO(isoprocinfo, nisoprocinfo, ROLLOUT_iso_bounds, ISO_ROLLOUT_BOUNDS, glui_bounds);

      PANEL_iso1 = glui_bounds->add_panel_to_panel(ROLLOUT_iso_bounds, "", GLUI_PANEL_NONE);
      EDIT_iso_valmin = glui_bounds->add_edittext_to_panel(PANEL_iso1, "", GLUI_EDITTEXT_FLOAT, &glui_iso_valmin, ISO_VALMIN, IsoBoundCB);
      glui_bounds->add_column_to_panel(PANEL_iso1, false);
      RADIO_iso_setmin = glui_bounds->add_radiogroup_to_panel(PANEL_iso1, &setisomin, ISO_SETVALMIN, IsoBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmin, _("percentile min"));
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmin, _("set min"));
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmin, _("global min"));
      IsoBoundCB(ISO_SETVALMIN);

      PANEL_iso2 = glui_bounds->add_panel_to_panel(ROLLOUT_iso_bounds, "", GLUI_PANEL_NONE);
      EDIT_iso_valmax = glui_bounds->add_edittext_to_panel(PANEL_iso2, "", GLUI_EDITTEXT_FLOAT, &glui_iso_valmax, ISO_VALMAX, IsoBoundCB);
      glui_bounds->add_column_to_panel(PANEL_iso2, false);
      RADIO_iso_setmax = glui_bounds->add_radiogroup_to_panel(PANEL_iso2, &setisomax, ISO_SETVALMAX, IsoBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmax, _("percentile max"));
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmax, _("set max"));
      glui_bounds->add_radiobutton_to_group(RADIO_iso_setmax, _("global max"));
      IsoBoundCB(ISO_SETVALMAX);
    }

    ROLLOUT_iso_color = glui_bounds->add_rollout_to_panel(ROLLOUT_iso, "Color/transparency", false, ISO_ROLLOUT_COLOR, IsoRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_iso_color, glui_bounds);
    ADDPROCINFO(isoprocinfo, nisoprocinfo, ROLLOUT_iso_color, ISO_ROLLOUT_COLOR, glui_bounds);

    RADIO_transparency_option = glui_bounds->add_radiogroup_to_panel(ROLLOUT_iso_color, &iso_transparency_option,ISO_TRANSPARENCY_OPTION,IsoBoundCB);
    glui_bounds->add_radiobutton_to_group(RADIO_transparency_option, _("transparent(constant)"));
    glui_bounds->add_radiobutton_to_group(RADIO_transparency_option, _("transparent(varying)"));
    glui_bounds->add_radiobutton_to_group(RADIO_transparency_option, _("opaque"));
    IsoBoundCB(ISO_TRANSPARENCY_OPTION);

    PANEL_iso_alllevels = glui_bounds->add_panel_to_panel(ROLLOUT_iso_color, "All levels", true);

    SPINNER_iso_transparency = glui_bounds->add_spinner_to_panel(PANEL_iso_alllevels, "alpha", GLUI_SPINNER_INT, &glui_iso_transparency, ISO_TRANSPARENCY, IsoBoundCB);
    glui_bounds->add_button_to_panel(PANEL_iso_alllevels, _("Apply"), GLOBAL_ALPHA, IsoBoundCB);

    PANEL_iso_eachlevel = glui_bounds->add_panel_to_panel(ROLLOUT_iso_color, "Each level", true);
    SPINNER_iso_level = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "level:", GLUI_SPINNER_INT, &glui_iso_level, ISO_LEVEL, IsoBoundCB);
    SPINNER_iso_level->set_int_limits(1, MAX_ISO_COLORS);
    LIST_colortable = glui_bounds->add_listbox_to_panel(PANEL_iso_eachlevel, _("Color:"), &i_colortable_list, COLORTABLE_LIST, IsoBoundCB);
    SPINNER_iso_colors[0] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "red:", GLUI_SPINNER_INT, glui_iso_colors+0, ISO_COLORS, IsoBoundCB);
    SPINNER_iso_colors[1] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "green:", GLUI_SPINNER_INT, glui_iso_colors+1, ISO_COLORS, IsoBoundCB);
    SPINNER_iso_colors[2] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "blue:", GLUI_SPINNER_INT, glui_iso_colors+2, ISO_COLORS, IsoBoundCB);
    SPINNER_iso_colors[3] = glui_bounds->add_spinner_to_panel(PANEL_iso_eachlevel, "alpha:", GLUI_SPINNER_INT, glui_iso_colors+3, ISO_COLORS, IsoBoundCB);

    UpdateColorTableList(-1);

    SPINNER_iso_colors[0]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_iso_colors[1]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_iso_colors[2]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_iso_colors[3]->set_int_limits(1, 255, GLUI_LIMIT_CLAMP);
    IsoBoundCB(ISO_LEVEL);
    IsoBoundCB(ISO_COLORS);

    if(ncolorbars>0){
      LIST_iso_colorbar = glui_bounds->add_listbox_to_panel(ROLLOUT_iso_color, "colormap:", &iso_colorbar_index, ISO_COLORBAR_LIST, IsoBoundCB);
      for(i = 0; i<ncolorbars; i++){
        colorbardata *cbi;

        cbi = colorbarinfo+i;
        cbi->label_ptr = cbi->label;
        LIST_iso_colorbar->add_item(i, cbi->label_ptr);
      }
      LIST_iso_colorbar->set_int_val(iso_colorbar_index);
      IsoBoundCB(ISO_COLORBAR_LIST);
    }
    glui_bounds->add_spinner_to_panel(ROLLOUT_iso_color, "min:", GLUI_SPINNER_FLOAT, &iso_valmin);
    glui_bounds->add_spinner_to_panel(ROLLOUT_iso_color, "max:", GLUI_SPINNER_FLOAT, &iso_valmax);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_color,_("Show"),&show_iso_color);

    ROLLOUT_iso_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_iso, "Settings", true, ISO_ROLLOUT_SETTINGS, IsoRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_iso_settings, glui_bounds);
    ADDPROCINFO(isoprocinfo, nisoprocinfo, ROLLOUT_iso_settings, ISO_ROLLOUT_SETTINGS, glui_bounds);

    visAIso = show_iso_shaded*1+show_iso_outline*2+show_iso_points*4;
    CHECKBOX_show_iso_shaded = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, _("shaded"), &show_iso_shaded, ISO_SURFACE, IsoBoundCB);
    CHECKBOX_show_iso_outline = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, _("outline"), &show_iso_outline, ISO_OUTLINE, IsoBoundCB);
    CHECKBOX_show_iso_points = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, _("points"), &show_iso_points, ISO_POINTS, IsoBoundCB);

    SPINNER_isolinewidth = glui_bounds->add_spinner_to_panel(ROLLOUT_iso_settings, _("line width"), GLUI_SPINNER_FLOAT, &isolinewidth);
    SPINNER_isolinewidth->set_float_limits(1.0, 10.0);
    SPINNER_iso_outline_ioffset = glui_bounds->add_spinner_to_panel(ROLLOUT_iso_settings, "outline offset", GLUI_SPINNER_INT, &iso_outline_ioffset, ISO_OUTLINE_IOFFSET, IsoBoundCB);
    SPINNER_iso_outline_ioffset->set_int_limits(0, 200);
    SPINNER_isopointsize = glui_bounds->add_spinner_to_panel(ROLLOUT_iso_settings, _("point size"), GLUI_SPINNER_FLOAT, &isopointsize);
    SPINNER_isopointsize->set_float_limits(1.0, 10.0);

    glui_bounds->add_separator_to_panel(ROLLOUT_iso_settings);

#ifdef pp_BETA
    CHECKBOX_sort2 = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, _("Sort transparent surfaces:"), &sort_iso_triangles, SORT_SURFACES, SliceBoundCB);
#endif
    CHECKBOX_smooth2 = glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, _("Smooth isosurfaces"), &smooth_iso_normal, SMOOTH_SURFACES, SliceBoundCB);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_iso_settings, _("wrapup in background"), &iso_multithread);
  }

  /* Particle File Bounds  */

  have_part = 0;
  have_evac = 0;
  if(npartinfo > 0 && nevac != npartinfo)have_part = 1;
  if(nevac > 0)have_evac = 1;
  if(have_part==1||have_evac==1){
    char label[100];

    strcpy(label, "");
    if(have_part == 1)strcat(label, "Particle");
    if(have_part == 1 && have_evac == 1)strcat(label, "/");
    if(have_evac == 1)strcat(label, "Evac");

    glui_active=1;
    ROLLOUT_part = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds,label,false,PART_ROLLOUT,BoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_part, glui_bounds);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_part, PART_ROLLOUT, glui_bounds);

    partboundsCPP.setup("particle", ROLLOUT_part, partbounds_cpp, npartbounds_cpp, &cache_part_data, SHOW_CACHE_CHECKBOX, PERCENTILE_ENABLED, PartBoundsCPP_CB,
                        ParticleRolloutCB, particleprocinfo, &nparticleprocinfo);

    ROLLOUT_particle_settings = glui_bounds->add_rollout_to_panel(ROLLOUT_part,"Settings",false,PARTICLE_SETTINGS, ParticleRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_particle_settings, glui_bounds);
    ADDPROCINFO(particleprocinfo, nparticleprocinfo, ROLLOUT_particle_settings, PARTICLE_SETTINGS, glui_bounds);

    SPINNER_partpointsize=glui_bounds->add_spinner_to_panel(ROLLOUT_particle_settings,_("Particle size"),GLUI_SPINNER_FLOAT,&partpointsize);
    SPINNER_partpointsize->set_float_limits(1.0,100.0);
    SPINNER_streaklinewidth=glui_bounds->add_spinner_to_panel(ROLLOUT_particle_settings,_("Streak line width"),GLUI_SPINNER_FLOAT,&streaklinewidth);
    SPINNER_streaklinewidth->set_float_limits(1.0,100.0);

    SPINNER_partstreaklength=glui_bounds->add_spinner_to_panel(ROLLOUT_particle_settings,_("Streak length (s)"),GLUI_SPINNER_FLOAT,&float_streak5value,STREAKLENGTH,PartBoundCB);
    SPINNER_partstreaklength->set_float_limits(0.0,tmax_part);

    CHECKBOX_showtracer=glui_bounds->add_checkbox_to_panel(ROLLOUT_particle_settings,_("Always show tracers"),&show_tracers_always,TRACERS,PartBoundCB);

    PANEL_partread=glui_bounds->add_panel_to_panel(ROLLOUT_particle_settings,_("Particle loading"));
    CHECKBOX_partfast = glui_bounds->add_checkbox_to_panel(PANEL_partread, _("Fast loading"), &partfast, PARTFAST, PartBoundCB);
    CHECKBOX_part_multithread = glui_bounds->add_checkbox_to_panel(PANEL_partread, _("Parallel loading"), &part_multithread);
    SPINNER_npartthread_ids = glui_bounds->add_spinner_to_panel(PANEL_partread, _("Files loaded at once"), GLUI_SPINNER_INT, &npartthread_ids);
    if(npartinfo>1){
      SPINNER_npartthread_ids->set_int_limits(1,MIN(npartinfo,MAX_THREADS));
    }
    else{
      SPINNER_npartthread_ids->set_int_limits(1,1);
    }
    PartBoundCB(PARTFAST);
  }
  PartBoundCB(FILETYPE_INDEX);

  if(have_evac==1){
    glui_active=1;

    glui_bounds->add_checkbox_to_panel(ROLLOUT_part,_("Select avatar"),&select_avatar);
    CHECKBOX_show_evac_slices=glui_bounds->add_checkbox_to_panel(ROLLOUT_part,_("Show slice menus"),&show_evac_slices,SHOW_EVAC_SLICES,SliceBoundCB);
    PANEL_evac_direction=glui_bounds->add_panel_to_panel(ROLLOUT_part,_("Direction vectors"));
    CHECKBOX_constant_coloring=glui_bounds->add_checkbox_to_panel(PANEL_evac_direction,_("Constant coloring"),&constant_evac_coloring,SHOW_EVAC_SLICES,SliceBoundCB);
    CHECKBOX_data_coloring=glui_bounds->add_checkbox_to_panel(PANEL_evac_direction,_("Data coloring"),&data_evac_coloring,DATA_EVAC_COLORING,SliceBoundCB);
    CHECKBOX_show_evac_color=glui_bounds->add_checkbox_to_panel(PANEL_evac_direction,_("Show colorbar (when data coloring)"),&show_evac_colorbar,SHOW_EVAC_SLICES,SliceBoundCB);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_part,_("View from selected Avatar"),&view_from_selected_avatar);
  }

  // ----------------------------------- Plot3D ----------------------------------------

  if(nplot3dinfo>0){
    glui_active=1;
    ROLLOUT_plot3d = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds,"Plot3D",false,PLOT3D_ROLLOUT,BoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_plot3d, glui_bounds);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_plot3d, PLOT3D_ROLLOUT, glui_bounds);

    plot3dboundsCPP.setup("PLOT3D", ROLLOUT_plot3d, plot3dbounds_cpp, nplot3dbounds_cpp, &cache_plot3d_data, SHOW_CACHE_CHECKBOX, PERCENTILE_ENABLED, Plot3DBoundsCPP_CB,
                          Plot3dRolloutCB, plot3dprocinfo, &nplot3dprocinfo);

    PANEL_plot3d = glui_bounds->add_panel_to_panel(ROLLOUT_plot3d,"", GLUI_PANEL_NONE);

    ROLLOUT_vector = glui_bounds->add_rollout_to_panel(PANEL_plot3d,_("Vector"),false,PLOT3D_VECTOR_ROLLOUT, Plot3dRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_vector, glui_bounds);
    ADDPROCINFO(plot3dprocinfo, nplot3dprocinfo, ROLLOUT_vector, PLOT3D_VECTOR_ROLLOUT, glui_bounds);

    glui_bounds->add_checkbox_to_panel(ROLLOUT_vector,_("Show vectors"),&visVector,UPDATEPLOT,Plot3DBoundCB);
    SPINNER_plot3d_vectorpointsize=glui_bounds->add_spinner_to_panel(ROLLOUT_vector,_("point size"),GLUI_SPINNER_FLOAT,&vectorpointsize,UPDATE_VECTOR,Plot3DBoundCB);
    SPINNER_plot3d_vectorpointsize->set_float_limits(1.0,10.0);
    SPINNER_plot3d_vectorlinewidth=glui_bounds->add_spinner_to_panel(ROLLOUT_vector,_("vector width"),GLUI_SPINNER_FLOAT,&vectorlinewidth,UPDATE_VECTOR,Plot3DBoundCB);
    SPINNER_plot3d_vectorlinewidth->set_float_limits(1.0,10.0);
    SPINNER_plot3d_vectorlinelength=glui_bounds->add_spinner_to_panel(ROLLOUT_vector,_("vector length"),GLUI_SPINNER_FLOAT,&vecfactor,UPDATE_VECTOR,Plot3DBoundCB);
    SPINNER_plot3dvectorskip=glui_bounds->add_spinner_to_panel(ROLLOUT_vector,_("Vector skip"),GLUI_SPINNER_INT,&vectorskip,PLOT3D_VECTORSKIP,Plot3DBoundCB);
    SPINNER_plot3dvectorskip->set_int_limits(1,10);

    glui_bounds->add_column_to_panel(PANEL_plot3d, false);

    ROLLOUT_isosurface = glui_bounds->add_rollout_to_panel(PANEL_plot3d,"Isosurface",false,PLOT3D_ISOSURFACE_ROLLOUT, Plot3dRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_isosurface, glui_bounds);
    ADDPROCINFO(plot3dprocinfo, nplot3dprocinfo, ROLLOUT_isosurface, PLOT3D_ISOSURFACE_ROLLOUT, glui_bounds);

    PANEL_pan1 = glui_bounds->add_panel_to_panel(ROLLOUT_isosurface,"",GLUI_PANEL_NONE);

    glui_bounds->add_checkbox_to_panel(PANEL_pan1,"Show isosurface",&visiso,PLOTISO,Plot3DBoundCB);
    SPINNER_plot3dpointsize=glui_bounds->add_spinner_to_panel(PANEL_pan1,_("Point size"),GLUI_SPINNER_FLOAT,
      &plot3dpointsize);
    SPINNER_plot3dpointsize->set_float_limits(1.0,10.0);

    SPINNER_plot3dlinewidth=glui_bounds->add_spinner_to_panel(PANEL_pan1,_("Line width"),GLUI_SPINNER_FLOAT,
      &plot3dlinewidth);
    SPINNER_plot3dlinewidth->set_float_limits(1.0,10.0);
//    glui_bounds->add_column_to_panel(ROLLOUT_isosurface);
    PANEL_pan2 = glui_bounds->add_panel_to_panel(ROLLOUT_isosurface,"",GLUI_PANEL_NONE);
    RADIO_plot3d_isotype=glui_bounds->add_radiogroup_to_panel(PANEL_pan2,&p3dsurfacetype,PLOTISOTYPE,Plot3DBoundCB);
    RADIOBUTTON_plot3d_iso_hidden=glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype,_("Hidden"));
    glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype,_("shaded"));
    glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype,_("outline"));
    glui_bounds->add_radiobutton_to_group(RADIO_plot3d_isotype,_("points"));
    RADIOBUTTON_plot3d_iso_hidden->disable();

    glui_p3min=p3min_all[0];
    glui_p3max=p3max_all[0];
    p3chopmin_temp=p3chopmin[0];
    p3chopmax_temp=p3chopmax[0];
    glui_bounds->add_column_to_panel(ROLLOUT_plot3d,false);
  }

  // ----------------------------------- Slice ----------------------------------------

  if(nsliceinfo>0){
    glui_active=1;
    ROLLOUT_slice = glui_bounds->add_rollout_to_panel(ROLLOUT_filebounds,"Slice",false,SLICE_ROLLOUT,BoundRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_slice, glui_bounds);
    ADDPROCINFO(boundprocinfo, nboundprocinfo, ROLLOUT_slice, SLICE_ROLLOUT, glui_bounds);

    sliceboundsCPP.setup("slice", ROLLOUT_slice, slicebounds_cpp, nslicebounds_cpp, &cache_slice_data, HIDE_CACHE_CHECKBOX, PERCENTILE_ENABLED, SliceBoundsCPP_CB,
                         SliceRolloutCB, sliceprocinfo, &nsliceprocinfo);

    PANEL_slice_buttonsA = glui_bounds->add_panel_to_panel(ROLLOUT_slice,"",false);

    ROLLOUT_slice_histogram = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsA, _("Histogram"), false, SLICE_HISTOGRAM_ROLLOUT, SliceRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_slice_histogram, glui_bounds);
    ADDPROCINFO(sliceprocinfo, nsliceprocinfo, ROLLOUT_slice_histogram, SLICE_HISTOGRAM_ROLLOUT, glui_bounds);

    RADIO_histogram_static = glui_bounds->add_radiogroup_to_panel(ROLLOUT_slice_histogram,&histogram_static);
    glui_bounds->add_radiobutton_to_group(RADIO_histogram_static,_("each time"));
    glui_bounds->add_radiobutton_to_group(RADIO_histogram_static,_("all times"));
    SPINNER_histogram_width_factor=glui_bounds->add_spinner_to_panel(ROLLOUT_slice_histogram, _("val at left"), GLUI_SPINNER_FLOAT,&histogram_width_factor);
    SPINNER_histogram_width_factor->set_float_limits(1.0,100.0);
    SPINNER_histogram_nbuckets=glui_bounds->add_spinner_to_panel(ROLLOUT_slice_histogram, _("bins"), GLUI_SPINNER_INT,&histogram_nbuckets,UPDATE_HISTOGRAM,SliceBoundCB);
    SPINNER_histogram_nbuckets->set_int_limits(3,255);
    CHECKBOX_histogram_show_numbers = glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_histogram, _("percentages"), &histogram_show_numbers, INIT_HISTOGRAM, SliceBoundCB);
    CHECKBOX_histogram_show_graph=glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_histogram, _("graph"), &histogram_show_graph, INIT_HISTOGRAM, SliceBoundCB);
    CHECKBOX_histogram_show_outline=glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_histogram, _("outline"), &histogram_show_outline);

    glui_bounds->add_column_to_panel(PANEL_slice_buttonsA, false);
    ROLLOUT_slice_average=glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsA,_("Average data"),false,SLICE_AVERAGE_ROLLOUT,SliceRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_slice_average, glui_bounds);
    ADDPROCINFO(sliceprocinfo, nsliceprocinfo, ROLLOUT_slice_average, SLICE_AVERAGE_ROLLOUT, glui_bounds);

    CHECKBOX_average_slice=glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_average,_("Average slice data"),&slice_average_flag);
    SPINNER_sliceaverage=glui_bounds->add_spinner_to_panel(ROLLOUT_slice_average,_("Time interval"),GLUI_SPINNER_FLOAT,&slice_average_interval);
    SPINNER_sliceaverage->set_float_limits(0.0,MAX(120.0,tour_tstop));
    glui_bounds->add_button_to_panel(ROLLOUT_slice_average,_("Reload"),FILE_RELOAD,SliceBoundCB);

    glui_bounds->add_column_to_panel(PANEL_slice_buttonsA, false);

    ROLLOUT_line_contour = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsA, _("Line contours"), false, LINE_CONTOUR_ROLLOUT, SliceRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_line_contour, glui_bounds);
    ADDPROCINFO(sliceprocinfo, nsliceprocinfo, ROLLOUT_line_contour, LINE_CONTOUR_ROLLOUT, glui_bounds);

    glui_bounds->add_column_to_panel(PANEL_slice_buttonsA, false);

    slice_line_contour_min = 0.0;
    slice_line_contour_max=1.0;
    SPINNER_line_contour_min=glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour,_("Min"),GLUI_SPINNER_FLOAT,
      &slice_line_contour_min,LINE_CONTOUR_VALUE,SliceBoundCB);
    SPINNER_line_contour_max=glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour,_("Max"),GLUI_SPINNER_FLOAT,
      &slice_line_contour_max,LINE_CONTOUR_VALUE,SliceBoundCB);
    slice_line_contour_num=1;
    SPINNER_line_contour_num=glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour,_("Number of contours"),GLUI_SPINNER_INT,
      &slice_line_contour_num,LINE_CONTOUR_VALUE,SliceBoundCB);
    SPINNER_line_contour_width=glui_bounds->add_spinner_to_panel(ROLLOUT_line_contour,_("contour width"),GLUI_SPINNER_FLOAT,&slice_line_contour_width);
    SPINNER_line_contour_width->set_float_limits(1.0,10.0);
      RADIO_contour_type = glui_bounds->add_radiogroup_to_panel(ROLLOUT_line_contour,&slice_contour_type);
    glui_bounds->add_radiobutton_to_group(RADIO_contour_type,_("line"));
#ifdef _DEBUG
    glui_bounds->add_radiobutton_to_group(RADIO_contour_type,_("stepped"));
#endif

    BUTTON_update_line_contour=glui_bounds->add_button_to_panel(ROLLOUT_line_contour,_("Update contours"),UPDATE_LINE_CONTOUR_VALUE,SliceBoundCB);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_line_contour,_("Show contours"),&vis_slice_contours);

    PANEL_slice_buttonsB = glui_bounds->add_panel_to_panel(ROLLOUT_slice, "", false);
    if(n_embedded_meshes>0){
      glui_bounds->add_column_to_panel(PANEL_slice_buttonsB, false);
      CHECKBOX_skip_subslice=glui_bounds->add_checkbox_to_panel(PANEL_slice_buttonsB,_("Skip coarse sub-slice"),&skip_slice_in_embedded_mesh);
    }

    if(nslicedups>0){
      glui_bounds->add_column_to_panel(PANEL_slice_buttonsB, false);
      ROLLOUT_slicedups = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsB, _("Duplicates"), false, SLICE_DUP_ROLLOUT, SliceRolloutCB);
      INSERT_ROLLOUT(ROLLOUT_slicedups, glui_bounds);
      ADDPROCINFO(sliceprocinfo, nsliceprocinfo, ROLLOUT_slicedups, SLICE_DUP_ROLLOUT, glui_bounds);

      PANEL_slicedup = glui_bounds->add_panel_to_panel(ROLLOUT_slicedups,"slices",true);
      RADIO_slicedup = glui_bounds->add_radiogroup_to_panel(PANEL_slicedup, &slicedup_option,UPDATE_SLICEDUPS,SliceBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_slicedup, _("Keep all"));
      glui_bounds->add_radiobutton_to_group(RADIO_slicedup, _("Keep fine"));
      glui_bounds->add_radiobutton_to_group(RADIO_slicedup, _("Keep coarse"));

      PANEL_vectorslicedup = glui_bounds->add_panel_to_panel(ROLLOUT_slicedups,"vector slices",true);
      RADIO_vectorslicedup = glui_bounds->add_radiogroup_to_panel(PANEL_vectorslicedup, &vectorslicedup_option, UPDATE_SLICEDUPS, SliceBoundCB);
      glui_bounds->add_radiobutton_to_group(RADIO_vectorslicedup, _("Keep all"));
      glui_bounds->add_radiobutton_to_group(RADIO_vectorslicedup, _("Keep fine"));
      glui_bounds->add_radiobutton_to_group(RADIO_vectorslicedup, _("Keep coarse"));
    }

    glui_bounds->add_column_to_panel(PANEL_slice_buttonsB, false);
    ROLLOUT_slice_settings = glui_bounds->add_rollout_to_panel(PANEL_slice_buttonsB, "Settings",false,SLICE_SETTINGS_ROLLOUT,SliceRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_slice_settings, glui_bounds);
    ADDPROCINFO(sliceprocinfo, nsliceprocinfo, ROLLOUT_slice_settings, SLICE_SETTINGS_ROLLOUT, glui_bounds);

    PANEL_immersed = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "show slice", true);
    PANEL_immersed_region = glui_bounds->add_panel_to_panel(PANEL_immersed, "region", true);
    RADIO_slice_celltype = glui_bounds->add_radiogroup_to_panel(PANEL_immersed_region, &slice_celltype, IMMERSED_SWITCH_CELLTYPE, ImmersedBoundCB);
    glui_bounds->add_radiobutton_to_group(RADIO_slice_celltype, "gas");
    glui_bounds->add_radiobutton_to_group(RADIO_slice_celltype, "solid");


    RADIO_button_cutcell = glui_bounds->add_radiobutton_to_group(RADIO_slice_celltype, "cut cell");
    if(ngeom_data==0)RADIO_button_cutcell->disable();

    PANEL_immersed_outlinetype = glui_bounds->add_panel_to_panel(PANEL_immersed, "outline type", true);
    RADIO_slice_edgetype = glui_bounds->add_radiogroup_to_panel(PANEL_immersed_outlinetype, &glui_slice_edgetype, IMMERSED_SWITCH_EDGETYPE, ImmersedBoundCB);
    glui_bounds->add_radiobutton_to_group(RADIO_slice_edgetype, _("polygon"));
    glui_bounds->add_radiobutton_to_group(RADIO_slice_edgetype, _("triangle"));
    glui_bounds->add_radiobutton_to_group(RADIO_slice_edgetype, _("none"));

    glui_bounds->add_column_to_panel(PANEL_immersed, false);
    PANEL_immersed_drawas = glui_bounds->add_panel_to_panel(PANEL_immersed, "draw slices using", true);
    CHECKBOX_show_slice_shaded = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas,   "solid colors",  &glui_show_slice_shaded,   IMMERSED_SET_DRAWTYPE, ImmersedBoundCB);
    CHECKBOX_show_slice_outlines = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas, "outlines", &glui_show_slice_outlines, IMMERSED_SET_DRAWTYPE, ImmersedBoundCB);
    CHECKBOX_show_slice_points = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas,   "points",  &glui_show_slice_points,   IMMERSED_SET_DRAWTYPE, ImmersedBoundCB);
    CHECKBOX_show_slice_values = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas,   "values",  &glui_show_slice_values,   IMMERSED_SET_DRAWTYPE, ImmersedBoundCB);
    CHECKBOX_show_vector_slice = glui_bounds->add_checkbox_to_panel(PANEL_immersed_drawas,   "vectors", &glui_show_vector_slice,   IMMERSED_SET_DRAWTYPE, ImmersedBoundCB);

    ImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
    ImmersedBoundCB(IMMERSED_SWITCH_EDGETYPE);

    glui_bounds->add_button_to_panel(PANEL_immersed, _("show values(all regions)"), SHOW_ALL_SLICE_VALUES, ImmersedBoundCB);
    glui_bounds->add_button_to_panel(PANEL_immersed, _("hide values(all regions)"), HIDE_ALL_SLICE_VALUES, ImmersedBoundCB);
    SPINNER_sliceval_ndigits = glui_bounds->add_spinner_to_panel(PANEL_immersed, _("digits"), GLUI_SPINNER_INT, &sliceval_ndigits);
    SPINNER_sliceval_ndigits->set_int_limits(0, 10);

    PANEL_showslice = glui_bounds->add_panel_to_panel(PANEL_immersed, "show vectors and", true);
    CHECKBOX_show_node_slices_and_vectors = glui_bounds->add_checkbox_to_panel(PANEL_showslice, _("node centered slices"), &show_node_slices_and_vectors);
    CHECKBOX_show_node_slices_and_vectors = glui_bounds->add_checkbox_to_panel(PANEL_showslice, _("cell centered slices"), &show_cell_slices_and_vectors);


    PANEL_slice_vector = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "vector properties", true);

    PANEL_vector1 = glui_bounds->add_panel_to_panel(PANEL_slice_vector, "", false);
    SPINNER_vectorpointsize = glui_bounds->add_spinner_to_panel(PANEL_vector1, _("point size"), GLUI_SPINNER_FLOAT,
      &vectorpointsize,UPDATE_VECTOR,SliceBoundCB);
    SPINNER_vectorpointsize->set_float_limits(1.0,20.0);
    SPINNER_vectorlinewidth=glui_bounds->add_spinner_to_panel(PANEL_vector1,_("width"),GLUI_SPINNER_FLOAT,&vectorlinewidth,UPDATE_VECTOR,SliceBoundCB);
    SPINNER_vectorlinewidth->set_float_limits(1.0,20.0);
    SPINNER_vectorlinelength = glui_bounds->add_spinner_to_panel(PANEL_vector1, _("length"), GLUI_SPINNER_FLOAT, &vecfactor, UPDATE_VECTOR, SliceBoundCB);
    SPINNER_slicevectorskip = glui_bounds->add_spinner_to_panel(PANEL_vector1, _("skip"), GLUI_SPINNER_INT, &vectorskip, SLICE_VECTORSKIP, SliceBoundCB);
    SPINNER_slicevectorskip->set_int_limits(1, 10);

    glui_bounds->add_column_to_panel(PANEL_slice_vector, false);
    PANEL_vector2 = glui_bounds->add_panel_to_panel(PANEL_slice_vector, "", false);
    glui_bounds->add_checkbox_to_panel(PANEL_vector2, "uniform spacing", &vec_uniform_spacing);
    glui_bounds->add_checkbox_to_panel(PANEL_vector2, "uniform length", &vec_uniform_length);

    CHECKBOX_color_vector_black = glui_bounds->add_checkbox_to_panel(PANEL_vector2, _("use foreground color"), &color_vector_black);

    if(ngeom_data > 0)glui_bounds->add_column_to_panel(ROLLOUT_slice_settings, false);

    if(show_slice_in_obst==ONLY_IN_GAS){
      show_slice_in_gas   = 1;
      show_slice_in_solid = 0;
    }
    else if(show_slice_in_obst==GAS_AND_SOLID){
      show_slice_in_gas   = 1;
      show_slice_in_solid = 1;
    }
    else if(show_slice_in_obst==ONLY_IN_SOLID){
      show_slice_in_gas   = 0;
      show_slice_in_solid = 1;
    }
    else{
      show_slice_in_gas   = 1;
      show_slice_in_solid = 0;
    }

    if(ngeom_data == 0)glui_bounds->add_column_to_panel(ROLLOUT_slice_settings, false);

    PANEL_slice_smoke = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "slice(fire)", true);
    glui_bounds->add_checkbox_to_panel(PANEL_slice_smoke, _("max blending"), &slices3d_max_blending);
    glui_bounds->add_checkbox_to_panel(PANEL_slice_smoke, _("show all 3D slices"), &showall_3dslices);

    PANEL_slice_misc = glui_bounds->add_panel_to_panel(ROLLOUT_slice_settings, "", true);
    SPINNER_transparent_level = glui_bounds->add_spinner_to_panel(PANEL_slice_misc, _("Transparent level"), GLUI_SPINNER_FLOAT, &transparent_level, TRANSPARENTLEVEL, SliceBoundCB);
    SPINNER_transparent_level->set_float_limits(0.0, 1.0);
    glui_bounds->add_spinner_to_panel(PANEL_slice_misc, "slice offset", GLUI_SPINNER_FLOAT, &slice_dz);
    for(i = 0; i<nmeshes; i++){
      meshdata *meshi;

      meshi = meshinfo+i;
      max_slice_skip = MAX(max_slice_skip, meshi->ibar/2);
      max_slice_skip = MAX(max_slice_skip, meshi->jbar/2);
      max_slice_skip = MAX(max_slice_skip, meshi->kbar/2);

    }
    SPINNER_slice_skip = glui_bounds->add_spinner_to_panel(PANEL_slice_misc, "data skip", GLUI_SPINNER_INT, &slice_skip, SLICE_SKIP, SliceBoundCB);
    SliceBoundCB(SLICE_SKIP);
    glui_bounds->add_checkbox_to_panel(PANEL_slice_misc, _("Output data (press r)"), &output_slicedata);
    if(nfedinfo>0){
      glui_bounds->add_checkbox_to_panel(PANEL_slice_misc, "Regenerate FED data", &regenerate_fed);
    }

    if(nterraininfo>0){
      glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_settings, _("terrain slice overlap"), &terrain_slice_overlap);
    }

#ifdef pp_SMOKETEST
    glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_settings, _("opacity adjustment"), &slice_opacity_adjustment);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_settings, _("sort slices"), &sort_slices);
    glui_bounds->add_checkbox_to_panel(ROLLOUT_slice_settings, _("show sorted slice labels"), &show_sort_labels);
#endif
  }

  // ----------------------------------- Time ----------------------------------------

  ROLLOUT_time = glui_bounds->add_rollout("Time", false, TIME_ROLLOUT, BoundRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_time, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_time, TIME_ROLLOUT, glui_bounds);

  PANEL_time1a = glui_bounds->add_panel_to_panel(ROLLOUT_time, "", false);
  SPINNER_timebounds = glui_bounds->add_spinner_to_panel(PANEL_time1a, _("Time:"), GLUI_SPINNER_FLOAT, &glui_time);
  glui_bounds->add_spinner_to_panel(PANEL_time1a, _("Offset:"), GLUI_SPINNER_FLOAT, &timeoffset);
  glui_bounds->add_column_to_panel(PANEL_time1a, false);
  SPINNER_timebounds->set_float_limits(0.0, 3600.0*24);
  BUTTON_SETTIME = glui_bounds->add_button_to_panel(PANEL_time1a, _("Set"), SET_TIME, TimeBoundCB);

  PANEL_time2 = glui_bounds->add_panel_to_panel(ROLLOUT_time, _("Data loading"), true);

  PANEL_time2a = glui_bounds->add_panel_to_panel(PANEL_time2, "", false);
  SPINNER_tload_begin = glui_bounds->add_spinner_to_panel(PANEL_time2a, _("min time"), GLUI_SPINNER_FLOAT, &tload_begin, TBOUNDS, TimeBoundCB);
  glui_bounds->add_column_to_panel(PANEL_time2a, false);
  CHECKBOX_use_tload_begin = glui_bounds->add_checkbox_to_panel(PANEL_time2a, "", &use_tload_begin, TBOUNDS_USE, TimeBoundCB);

  PANEL_time2b = glui_bounds->add_panel_to_panel(PANEL_time2, "", false);
  SPINNER_tload_end = glui_bounds->add_spinner_to_panel(PANEL_time2b, _("max time"), GLUI_SPINNER_FLOAT, &tload_end, TBOUNDS, TimeBoundCB);
  glui_bounds->add_column_to_panel(PANEL_time2b, false);
  CHECKBOX_use_tload_end = glui_bounds->add_checkbox_to_panel(PANEL_time2b, "", &use_tload_end, TBOUNDS_USE, TimeBoundCB);

  PANEL_time2c = glui_bounds->add_panel_to_panel(PANEL_time2, "", false);
  SPINNER_tload_skip = glui_bounds->add_spinner_to_panel(PANEL_time2c, _("frame skip"), GLUI_SPINNER_INT, &tload_skip, TBOUNDS, TimeBoundCB);
  glui_bounds->add_column_to_panel(PANEL_time2c, false);
  CHECKBOX_use_tload_skip = glui_bounds->add_checkbox_to_panel(PANEL_time2c, "", &use_tload_skip, TBOUNDS_USE, TimeBoundCB);
  SPINNER_tload_skip->set_int_limits(0, 1000);

#ifdef pp_TIMEBAR_DIGITS
  SPINNER_ntimebar_digits = glui_bounds->add_spinner_to_panel(PANEL_time2, _("timebar digits:"), GLUI_SPINNER_INT, &ntimebar_digits, TIMEBAR_DIGITS, TimeBoundCB);
  SPINNER_ntimebar_digits->set_int_limits(3, 8, GLUI_LIMIT_CLAMP);
#endif

  glui_bounds->add_button_to_panel(PANEL_time2, _("Reload all data"), RELOAD_ALL_DATA, TimeBoundCB);
#ifdef pp_LOAD_NEWDATA
  glui_bounds->add_button_to_panel(PANEL_time2, _("Reload new data"), RELOAD_INCREMENTAL_DATA, TimeBoundCB);
#endif

  TimeBoundCB(TBOUNDS_USE);


  // -------------- Data coloring -------------------

  ROLLOUT_coloring = glui_bounds->add_rollout("Coloring", false, COLORING_ROLLOUT, FileRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_coloring, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_coloring, COLORING_ROLLOUT, glui_bounds);

  PANEL_cb11 = glui_bounds->add_panel_to_panel(ROLLOUT_coloring, "", GLUI_PANEL_NONE);

  PANEL_colorbar_properties = glui_bounds->add_panel_to_panel(PANEL_cb11, _("Colorbar"));


  if(ncolorbars>0){
    selectedcolorbar_index2 = -1;
    LIST_colorbar2 = glui_bounds->add_listbox_to_panel(PANEL_colorbar_properties, "", &selectedcolorbar_index2, COLORBAR_LIST2, SliceBoundCB);

    for(i = 0; i<ncolorbars; i++){
      colorbardata *cbi;

      cbi = colorbarinfo+i;
      cbi->label_ptr = cbi->label;
      LIST_colorbar2->add_item(i, cbi->label_ptr);
    }
    LIST_colorbar2->set_int_val(colorbartype);
  }

  CHECKBOX_visColorbarVertical2   = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "vertical",   &visColorbarVertical,   LABELS_vcolorbar, LabelsCB);
  CHECKBOX_visColorbarHorizontal2 = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, "horizontal", &visColorbarHorizontal, LABELS_hcolorbar, LabelsCB);
  RADIO2_plot3d_display = glui_bounds->add_radiogroup_to_panel(PANEL_colorbar_properties, &contour_type, UPDATEPLOT, Plot3DBoundCB);
  glui_bounds->add_radiobutton_to_group(RADIO2_plot3d_display, _("Continuous"));
  glui_bounds->add_radiobutton_to_group(RADIO2_plot3d_display, _("Stepped"));
  glui_bounds->add_radiobutton_to_group(RADIO2_plot3d_display, _("Line"));
  CHECKBOX_colorbar_flip = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, _("flip"), &colorbar_flip, FLIP, LabelsCB);
  CHECKBOX_colorbar_autoflip = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, _("Auto flip"), &colorbar_autoflip, FLIP, LabelsCB);
  SPINNER_colorbar_shift = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, _("shift:"), GLUI_SPINNER_FLOAT, &colorbar_shift, LABELS_colorbar_shift, LabelsCB);
  SPINNER_colorbar_shift->set_float_limits(COLORBAR_SHIFT_MIN, COLORBAR_SHIFT_MAX);

  glui_bounds->add_separator_to_panel(PANEL_colorbar_properties);

  SPINNER_colorbar_selection_width = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, _("Selection width:"), GLUI_SPINNER_INT, &colorbar_selection_width, COLORBAND, SliceBoundCB);
  SPINNER_colorbar_selection_width->set_int_limits(COLORBAR_SELECTION_WIDTH_MIN, COLORBAR_SELECTION_WIDTH_MAX);
  SPINNER_ncolorlabel_digits = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, _("colorbar digits:"), GLUI_SPINNER_INT, &ncolorlabel_digits, COLORLABEL_DIGITS, SliceBoundCB);
  SPINNER_ncolorlabel_digits->set_int_limits(COLORBAR_NDECIMALS_MIN, COLORBAR_NDECIMALS_MAX, GLUI_LIMIT_CLAMP);
  SPINNER_ncolorlabel_padding = glui_bounds->add_spinner_to_panel(PANEL_colorbar_properties, _("padding:"), GLUI_SPINNER_INT, &ncolorlabel_padding, COLORLABEL_DIGITS, SliceBoundCB);
  SPINNER_ncolorlabel_padding->set_int_limits(0, 8, GLUI_LIMIT_CLAMP);
  CHECKBOX_fixedpoint  = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, _("force fixed point labels"), &force_fixedpoint,  COLORLABEL_DIGITS, SliceBoundCB);
  CHECKBOX_exponential = glui_bounds->add_checkbox_to_panel(PANEL_colorbar_properties, _("force exponential labels"), &force_exponential, FORCE_EXPONENTIAL, SliceBoundCB);

  glui_bounds->add_column_to_panel(PANEL_cb11, false);

  PANEL_coloring = glui_bounds->add_panel_to_panel(PANEL_cb11, _("Coloring"));
  CHECKBOX_labels_shade = glui_bounds->add_checkbox_to_panel(PANEL_coloring, _("Black/White (geometry)"), &setbw, LABELS_shade, LabelsCB);
  CHECKBOX_labels_shadedata = glui_bounds->add_checkbox_to_panel(PANEL_coloring, _("Black/White (data)"), &setbwdata, LABELS_shadedata, LabelsCB);
  CHECKBOX_transparentflag = glui_bounds->add_checkbox_to_panel(PANEL_coloring, _("Transparent (data)"),
    &use_transparency_data, DATA_transparent, SliceBoundCB);
  SPINNER_labels_transparency_data = glui_bounds->add_spinner_to_panel(PANEL_coloring, _("level"),
    GLUI_SPINNER_FLOAT, &transparent_level, TRANSPARENTLEVEL, SliceBoundCB);
  SPINNER_labels_transparency_data->set_w(0);
  SPINNER_labels_transparency_data->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);
  CHECKBOX_use_lighting = glui_bounds->add_checkbox_to_panel(PANEL_coloring, _("Lighting"), &use_lighting, CB_USE_LIGHTING, LabelsCB);


  PANEL_extreme = glui_bounds->add_panel_to_panel(ROLLOUT_coloring, "", GLUI_PANEL_NONE);

  if(use_data_extremes==1){
    ROLLOUT_extreme2 = glui_bounds->add_rollout_to_panel(PANEL_extreme, "Highlight extreme data", false);
    INSERT_ROLLOUT(ROLLOUT_extreme2, glui_bounds);
  }
  else{
    ROLLOUT_extreme2 = glui_bounds->add_rollout_to_panel(PANEL_extreme, "Highlight extreme data (not supported with OpenGL 1.x)", false);
    INSERT_ROLLOUT(ROLLOUT_extreme2, glui_bounds);
  }

  PANEL_extreme_min = glui_bounds->add_panel_to_panel(ROLLOUT_extreme2, "", GLUI_PANEL_NONE);
  CHECKBOX_show_extreme_mindata = glui_bounds->add_checkbox_to_panel(PANEL_extreme_min, _("Color below min"), &show_extreme_mindata, COLORBAR_EXTREME, ExtremeCB);

  SPINNER_down_red = glui_bounds->add_spinner_to_panel(PANEL_extreme_min, _("red"), GLUI_SPINNER_INT, cb_down_rgb, COLORBAR_EXTREME_RGB, ExtremeCB);
  SPINNER_down_green = glui_bounds->add_spinner_to_panel(PANEL_extreme_min, _("green"), GLUI_SPINNER_INT, cb_down_rgb+1, COLORBAR_EXTREME_RGB, ExtremeCB);
  SPINNER_down_blue = glui_bounds->add_spinner_to_panel(PANEL_extreme_min, _("blue"), GLUI_SPINNER_INT, cb_down_rgb+2, COLORBAR_EXTREME_RGB, ExtremeCB);
  SPINNER_down_red->set_int_limits(0, 255);
  SPINNER_down_green->set_int_limits(0, 255);
  SPINNER_down_blue->set_int_limits(0, 255);

  glui_bounds->add_column_to_panel(ROLLOUT_extreme2, false);

  PANEL_extreme_max = glui_bounds->add_panel_to_panel(ROLLOUT_extreme2, "", GLUI_PANEL_NONE);

  CHECKBOX_show_extreme_maxdata = glui_bounds->add_checkbox_to_panel(PANEL_extreme_max, _("Color above max"), &show_extreme_maxdata, COLORBAR_EXTREME, ExtremeCB);

  SPINNER_up_red = glui_bounds->add_spinner_to_panel(PANEL_extreme_max, _("red"), GLUI_SPINNER_INT, cb_up_rgb, COLORBAR_EXTREME_RGB, ExtremeCB);
  SPINNER_up_green = glui_bounds->add_spinner_to_panel(PANEL_extreme_max, _("green"), GLUI_SPINNER_INT, cb_up_rgb+1, COLORBAR_EXTREME_RGB, ExtremeCB);
  SPINNER_up_blue = glui_bounds->add_spinner_to_panel(PANEL_extreme_max, _("blue"), GLUI_SPINNER_INT, cb_up_rgb+2, COLORBAR_EXTREME_RGB, ExtremeCB);
  SPINNER_up_red->set_int_limits(0, 255);
  SPINNER_up_green->set_int_limits(0, 255);
  SPINNER_up_blue->set_int_limits(0, 255);

  if(use_data_extremes==0){
    ROLLOUT_extreme2->disable();
    CHECKBOX_show_extreme_maxdata->set_int_val(0);
    CHECKBOX_show_extreme_mindata->set_int_val(0);
    ExtremeCB(COLORBAR_EXTREME_RGB);
  }
  ColorbarGlobal2Local();

  ROLLOUT_split = glui_bounds->add_rollout_to_panel(ROLLOUT_coloring, "Define split colorbar", false);
  INSERT_ROLLOUT(ROLLOUT_split, glui_bounds);
  PANEL_split1H = glui_bounds->add_panel_to_panel(ROLLOUT_split, "color below split");

  SPINNER_colorsplit[3] = glui_bounds->add_spinner_to_panel(PANEL_split1H, _("red"), GLUI_SPINNER_INT, colorsplit+3, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[4] = glui_bounds->add_spinner_to_panel(PANEL_split1H, _("green"), GLUI_SPINNER_INT, colorsplit+4, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[5] = glui_bounds->add_spinner_to_panel(PANEL_split1H, _("blue"), GLUI_SPINNER_INT, colorsplit+5, SPLIT_COLORBAR, SplitCB);

  PANEL_split1L = glui_bounds->add_panel_to_panel(ROLLOUT_split, "min color");

  SPINNER_colorsplit[0] = glui_bounds->add_spinner_to_panel(PANEL_split1L, _("red"), GLUI_SPINNER_INT, colorsplit, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[1] = glui_bounds->add_spinner_to_panel(PANEL_split1L, _("green"), GLUI_SPINNER_INT, colorsplit+1, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[2] = glui_bounds->add_spinner_to_panel(PANEL_split1L, _("blue"), GLUI_SPINNER_INT, colorsplit+2, SPLIT_COLORBAR, SplitCB);

  glui_bounds->add_column_to_panel(ROLLOUT_split, false);

  PANEL_split2H = glui_bounds->add_panel_to_panel(ROLLOUT_split, "max color");

  SPINNER_colorsplit[9] = glui_bounds->add_spinner_to_panel(PANEL_split2H, _("red"), GLUI_SPINNER_INT, colorsplit+9, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[10] = glui_bounds->add_spinner_to_panel(PANEL_split2H, _("green"), GLUI_SPINNER_INT, colorsplit+10, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[11] = glui_bounds->add_spinner_to_panel(PANEL_split2H, _("blue"), GLUI_SPINNER_INT, colorsplit+11, SPLIT_COLORBAR, SplitCB);

  PANEL_split2L = glui_bounds->add_panel_to_panel(ROLLOUT_split, "color above split");

  SPINNER_colorsplit[6] = glui_bounds->add_spinner_to_panel(PANEL_split2L, _("red"), GLUI_SPINNER_INT, colorsplit+6, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[7] = glui_bounds->add_spinner_to_panel(PANEL_split2L, _("green"), GLUI_SPINNER_INT, colorsplit+7, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[8] = glui_bounds->add_spinner_to_panel(PANEL_split2L, _("blue"), GLUI_SPINNER_INT, colorsplit+8, SPLIT_COLORBAR, SplitCB);

  glui_bounds->add_column_to_panel(ROLLOUT_split, false);

  PANEL_split3 = glui_bounds->add_panel_to_panel(ROLLOUT_split, "vals");

  glui_bounds->add_spinner_to_panel(PANEL_split3, _("max"), GLUI_SPINNER_FLOAT, splitvals+2, SPLIT_COLORBAR, SplitCB);
  glui_bounds->add_spinner_to_panel(PANEL_split3, _("split"), GLUI_SPINNER_FLOAT, splitvals+1, SPLIT_COLORBAR, SplitCB);
  glui_bounds->add_spinner_to_panel(PANEL_split3, _("min"), GLUI_SPINNER_FLOAT, splitvals, SPLIT_COLORBAR, SplitCB);

  for(i = 0; i<12; i++){
    SPINNER_colorsplit[i]->set_int_limits(0, 255);
  }
  SplitCB(SPLIT_COLORBAR);

  // ----------------------------------- Memory check ----------------------------------------

#ifdef pp_MEMDEBUG
  ROLLOUT_memcheck = glui_bounds->add_rollout(_("Memory check"),false,MEMCHECK_ROLLOUT,FileRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_memcheck, glui_bounds);
  ADDPROCINFO(fileprocinfo, nfileprocinfo, ROLLOUT_memcheck, MEMCHECK_ROLLOUT, glui_bounds);

  list_memcheck_index = 0;
  RADIO_memcheck = glui_bounds->add_radiogroup_to_panel(ROLLOUT_memcheck,&list_memcheck_index,MEMCHECK, MemcheckCB);
  glui_bounds->add_radiobutton_to_group(RADIO_memcheck,_("Unlimited"));
  glui_bounds->add_radiobutton_to_group(RADIO_memcheck,"1 GB");
  glui_bounds->add_radiobutton_to_group(RADIO_memcheck,"2 GB");
  glui_bounds->add_radiobutton_to_group(RADIO_memcheck,"4 GB");
  glui_bounds->add_radiobutton_to_group(RADIO_memcheck,"8 GB");
#endif

  glui_bounds->add_button(_("Save settings"), SAVE_SETTINGS_BOUNDS, BoundsDlgCB);
#ifdef pp_CLOSEOFF
  GLUI_Button *BUTTON_button1=glui_bounds->add_button(_("Close"), CLOSE_BOUNDS, BoundsDlgCB);
  BUTTON_button1->disable();
#else
  glui_bounds->add_button(_("Close"), CLOSE_BOUNDS, BoundsDlgCB);
#endif

  glui_bounds->set_main_gfx_window( main_window );
}

/* ------------------ CompressOnOff ------------------------ */

extern "C" void CompressOnOff(int flag){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ Plot3DBoundCB ------------------------ */

extern "C" void Plot3DBoundCB(int var){
  int i;

  switch(var){
  case PLOT3D_VECTORSKIP:
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
    Plot3DBoundCB(UPDATE_VECTOR);
    break;
  case UPDATE_VECTOR:
    if(vecfactor<0.0){
      vecfactor = 0.0;
      if(SPINNER_vectorlinewidth!=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
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
    UpdateRGBColors(COLORBAR_INDEX_NONE);
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
   if(plot3dinfo!=NULL){
     plot3dmin_unit = (unsigned char *)plot3dinfo->label[list_p3_index].unit;
     plot3dmax_unit = plot3dmin_unit;
   }

   list_p3_index_old=list_p3_index;
   Plot3DBoundCB(SETCHOPMINVAL);
   Plot3DBoundCB(SETCHOPMAXVAL);
   Plot3DBoundCB(SETVALMIN);
   Plot3DBoundCB(SETVALMAX);
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
      ASSERT(FFALSE);
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
        ASSERT(FFALSE);
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
    Plot3DBoundCB(FILE_UPDATE);
    UpdateAllPlot3DColors();
    break;
  case FILE_RELOAD:
   Plot3DBoundCB(FILE_UPDATE);
   for(i=0;i<nplot3dinfo;i++){
     if(plot3dinfo[i].loaded==0)continue;
     LoadPlot3dMenu(i);
   }
   break;
  case VALMIN:
    break;
  case VALMAX:
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ UpdateTracers ------------------------ */

extern "C" void UpdateTracers(void){
  if(CHECKBOX_showtracer==NULL)return;
  CHECKBOX_showtracer->set_int_val(show_tracers_always);
}


/* ------------------ UpdateGluiIsotype ------------------------ */

extern "C" void UpdateGluiIsotype(void){
  CHECKBOX_show_iso_shaded->set_int_val(visAIso&1);
  CHECKBOX_show_iso_outline->set_int_val((visAIso&2)/2);
  CHECKBOX_show_iso_points->set_int_val((visAIso&4)/4);
}


/* ------------------ UpdateGluiPlot3Dtype ------------------------ */

extern "C" void UpdateGluiPlot3Dtype(void){
  RADIO_plot3d_isotype->set_int_val(p3dsurfacetype);
}
/* ------------------ UpdateChar ------------------------ */

extern "C" void UpdateChar(void){
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

/* ------------------ UpdatePlot3dListIndex ------------------------ */

extern "C" void UpdatePlot3dListIndex(void){
  SetValTypeIndex(BOUND_PLOT3D, plotn-1);
}

/* ------------------ GetColorTableIndex ------------------------ */

int GetColorTableIndex(int *color){
  int i;

  if(colortableinfo==NULL)return -1;
  for(i=0;i<ncolortableinfo;i++){
    colortabledata *cti;

    cti = colortableinfo + i;
    if(color[0]==cti->color[0]&&color[1]==cti->color[1]&&color[2]==cti->color[2])return i;
  }
  return -1;
}

/* ------------------ GetColorTable ------------------------ */

colortabledata *GetColorTable(char *label){
  int i;

  if(label==NULL||strlen(label)==0)return NULL;
  for(i=0;i<ncolortableinfo;i++){
    colortabledata *cti;

    cti = colortableinfo + i;
    if(strcmp(label,cti->label)==0)return cti;
  }
  return NULL;
}

/* ------------------ IsoBoundCB ------------------------ */

extern "C" void IsoBoundCB(int var){
  int i;
  float *iso_color;

  switch(var){
  case ISO_OUTLINE_IOFFSET:
    iso_outline_offset = (float)iso_outline_ioffset/1000.0;
  break;
  case ISO_COLORBAR_LIST:
    iso_colorbar = colorbarinfo + iso_colorbar_index;
    ColorbarMenu(iso_colorbar_index);
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
	ASSERT(FFALSE);
	break;
    }
    SliceBoundCB(DATA_transparent);
    break;
  case COLORTABLE_LIST:
    if(i_colortable_list>=0){
      colortabledata *cti;

      cti = colortableinfo+i_colortable_list;
      glui_iso_colors[0] = cti->color[0];
      glui_iso_colors[1] = cti->color[1];
      glui_iso_colors[2] = cti->color[2];
      glui_iso_colors[3] = cti->color[3];
      IsoBoundCB(ISO_COLORS);
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
      iso_colors[4 * i + 3] = iso_transparency;
    }
    if(SPINNER_iso_colors[3]!=NULL)SPINNER_iso_colors[3]->set_int_val(glui_iso_transparency);
    IsoBoundCB(ISO_COLORS);
    break;
  case ISO_TRANSPARENCY:
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
      iso_colorsbw[4 * i + 0] = graylevel;
      iso_colorsbw[4 * i + 1] = graylevel;
      iso_colorsbw[4 * i + 2] = graylevel;
      iso_colorsbw[4 * i + 3] = iso_colors[4 * i + 3];
    }
    UpdateIsoColors();
    if(LIST_colortable!=NULL){
      i_colortable_list = CLAMP(GetColorTableIndex(glui_iso_colors), -1, ncolortableinfo-1);
      LIST_colortable->set_int_val(i_colortable_list);
    }
    break;
  case FRAMELOADING:
    isoframestep_global=isoframeskip_global+1;
    isozipstep=isozipskip+1;
    updatemenu=1;
    break;
  case ISO_SURFACE:
  case  ISO_OUTLINE:
  case ISO_POINTS:
    visAIso= 1*show_iso_shaded + 2*show_iso_outline + 4*show_iso_points;
    updatemenu=1;
    break;
  case ISO_SETVALMIN:
    switch (setisomin){
      case SET_MIN:
        iso_valmin=glui_iso_valmin;
        EDIT_iso_valmin->enable();
        break;
      case PERCENTILE_MIN:
        iso_valmin = iso_percentile_min;
        EDIT_iso_valmin->disable();
        break;
      case GLOBAL_MIN:
        iso_valmin = iso_global_min;
        EDIT_iso_valmin->disable();
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    glui_iso_valmin=iso_valmin;
    EDIT_iso_valmin->set_float_val(glui_iso_valmin);
    glutPostRedisplay();
    break;
  case ISO_SETVALMAX:
    switch (setisomax){
      case SET_MAX:
        iso_valmax=glui_iso_valmax;
        EDIT_iso_valmax->enable();
        break;
      case PERCENTILE_MAX:
        iso_valmax = iso_percentile_max;
        EDIT_iso_valmax->disable();
        break;
      case GLOBAL_MAX:
        iso_valmax = iso_global_max;
        EDIT_iso_valmax->disable();
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    glui_iso_valmax = iso_valmax;
    EDIT_iso_valmax->set_float_val(glui_iso_valmax);
    glutPostRedisplay();
    break;
  case ISO_VALMIN:
    iso_valmin=glui_iso_valmin;
    glutPostRedisplay();
    break;
  case ISO_VALMAX:
    iso_valmax=glui_iso_valmax;
    glutPostRedisplay();
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ AddScriptList ------------------------ */

extern "C" void AddScriptList(char *file, int id){
  if(file!=NULL&&strlen(file)>0&&LIST_scriptlist!=NULL){
    LIST_scriptlist->add_item(id,file);
  }
}

/* ------------------ GluiScriptEnable ------------------------ */

extern "C" void GluiScriptEnable(void){
    BUTTON_script_start->enable();
    BUTTON_script_stop->enable();
    BUTTON_script_runscript->enable();
    LIST_scriptlist->enable();
    BUTTON_script_saveini->enable();
    BUTTON_script_setsuffix->enable();
    EDIT_ini->enable();
  }

/* ------------------ GluiScriptDisable ------------------------ */

extern "C"  void GluiScriptDisable(void){
    BUTTON_script_start->disable();
    BUTTON_script_stop->disable();
    BUTTON_script_runscript->disable();
    LIST_scriptlist->disable();
    BUTTON_script_saveini->disable();
    BUTTON_script_setsuffix->disable();
    EDIT_ini->disable();
  }

/* ------------------ UpdateBoundaryListIndex ------------------------ */

extern "C" void UpdateBoundaryListIndex(int patchfilenum){
  int i;
  if(RADIO_bf==NULL)return;
  for(i=0;i<npatch2;i++){
    patchdata *patchi;

    patchi = patchinfo + patchfilenum;
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

/* ------------------ UpdateBoundaryListIndex2 ------------------------ */

extern "C" void UpdateBoundaryListIndex2(char *label){
  int i;
  for(i=0;i<npatch2;i++){
    if(strcmp(patchlabellist[i],label)==0){
      UpdateBoundaryListIndex(patchlabellist_index[i]);
      break;
    }
  }
}

/* ------------------ UpdateGluiStreakValue ------------------------ */

extern "C" void UpdateGluiStreakValue(float rvalue){
  float_streak5value=rvalue;
  if(SPINNER_partstreaklength!=NULL){
    SPINNER_partstreaklength->set_float_val(rvalue);
    SPINNER_partstreaklength->set_float_limits(0.0,tmax_part);
  }
}

/* ------------------ IncrementPartPropIndex ------------------------ */

extern "C" void IncrementPartPropIndex(void){
  if(npart5prop>0){
    ipart5prop++;
    if(ipart5prop>npart5prop-1)ipart5prop = 0;
    SetValTypeIndex(BOUND_PART, ipart5prop);
    ParticlePropShowMenu(ipart5prop);
  }
}

/* ------------------ PartBoundCB ------------------------ */

void PartBoundCB(int var){
  partpropdata *prop_new, *prop_old;

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
      streak5show=0;
    }
    else{
      streak5show=1;
    }
    updatemenu=1;
    break;
  case TRACERS:
  case PARTFAST:
    if(npartinfo<=1||nevac>0){
      CHECKBOX_part_multithread->disable();
      SPINNER_npartthread_ids->disable();
      part_multithread = 0;
      if(nevac>0){
        partfast = 0;
        CHECKBOX_partfast->set_int_val(partfast);
        CHECKBOX_partfast->disable();
      }
      CHECKBOX_part_multithread->set_int_val(part_multithread);
    }
    else{
      CHECKBOX_part_multithread->enable();
      SPINNER_npartthread_ids->enable();
      CHECKBOX_part_multithread->set_int_val(part_multithread);
    }
    updatemenu=1;
    break;
  case FRAMELOADING:
    partframestep=partframeskip+1;
    evacframestep=evacframeskip+1;
    evacframestep=evacframeskip+1;
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
     LoadEvacMenu(EVACFILE_RELOADALL);
     ParticlePropShowMenu(prop_index_SAVE);
    }
    break;
  default:
    ASSERT(FFALSE);
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

/* ------------------ SliceBoundCB ------------------------ */

extern "C" void SliceBoundCB(int var){
  int error,i;
  int ii;
  slicedata *sd;
  int last_slice;

  updatemenu=1;
  if(var==COLORBAR_LIST2){
    selectedcolorbar_index= GetColorbarListIndex();
    UpdateColorbarList();
    ColorbarMenu(selectedcolorbar_index);
    ColorbarGlobal2Local();
  }
  switch(var){
    case UPDATE_HISTOGRAM:
      update_slice_hists = 1;
      histograms_defined = 0;
      break;
    case INIT_HISTOGRAM:
      if(histogram_show_graph == 1 || histogram_show_numbers == 1){
        update_slice_hists = 1;
        visColorbarVertical = 1;
      }
      break;
    case DATA_transparent:
      UpdateTransparency();
      UpdateChopColors();
      UpdateIsoControls();
      break;
    case COLORBAR_EXTREME2:
      UpdateExtreme();
      break;
    case UPDATE_SLICEDUPS:
    updatemenu = 1;
    break;
    case SLICE_VECTORSKIP:
      if(SPINNER_plot3dvectorskip!=NULL)SPINNER_plot3dvectorskip->set_int_val(vectorskip);
      break;
    case ZONEVALMINMAX:
      GetZoneColors(zonetu, nzonetotal, izonetu, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      GetZoneColors(zonetl, nzonetotal, izonetl, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zonefl==1)GetZoneColors(zonefl, nzonetotal, izonefl, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zonelw==1)GetZoneColors(zonelw, nzonetotal, izonelw, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zoneuw==1)GetZoneColors(zoneuw, nzonetotal, izoneuw, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      if(have_zonecl==1)GetZoneColors(zonecl, nzonetotal, izonecl, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      if(have_target_data==1)GetZoneColors(zonetargets, nzonetotal_targets, izonetargets, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
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
      GetZoneColors(zonetu, nzonetotal, izonetu,zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      GetZoneColors(zonetl, nzonetotal, izonetl, zonemin, zonemax, nrgb, nrgb_full, colorlabelzone, colorvalueszone, zonelevels256);
      break;
    case COLORBAR_LIST2:
      if(selectedcolorbar_index2 == bw_colorbar_index){
        setbwdata = 1;
        ColorbarMenu(bw_colorbar_index);
      }
      else{
        setbwdata = 0;
      }
      IsoBoundCB(ISO_COLORS);
      SetLabelControls();
      break;
    case RESEARCH_MODE:
     SetResearchMode(research_mode);
      {

        // slice files

        SliceBoundsCPP_CB(BOUND_UPDATE_COLORS);

        // boundary files

        PatchBoundsCPP_CB(BOUND_UPDATE_COLORS);

        // particle files

        PartBoundsCPP_CB(BOUND_RELOAD_DATA);

        // plot3d files

        if(nplot3dloaded>0){
          Plot3DBoundsCPP_CB(BOUND_UPDATE_COLORS);
        }
        if(research_mode==1)PRINTF("\nresearch mode on, using global bounds\n\n");
      }
      if(research_mode==0){
        PRINTF("research mode off\n");
      }
      SliceBoundCB(FILE_UPDATE);
      break;
    case SMOOTH_SURFACES:
      CHECKBOX_smooth2->set_int_val(smooth_iso_normal);
      break;
    case SORT_SURFACES:
      sort_geometry=sort_iso_triangles;
      for(i=nsurfinfo;i<nsurfinfo+MAX_ISO_COLORS+1;i++){
        surfdata *surfi;

        surfi = surfinfo + i;
        surfi->transparent_level=transparent_level;
      }
      CHECKBOX_sort2->set_int_val(sort_iso_triangles);
      IsoBoundCB(GLOBAL_ALPHA);
      break;
    case SHOW_EVAC_SLICES:
      data_evac_coloring = 1-constant_evac_coloring;
      UpdateSliceMenuShow();
      if(CHECKBOX_data_coloring!=NULL)CHECKBOX_data_coloring->set_int_val(data_evac_coloring);
      break;
    case DATA_EVAC_COLORING:
      constant_evac_coloring = 1-data_evac_coloring;
      UpdateSliceMenuShow();
      if(CHECKBOX_constant_coloring!=NULL)CHECKBOX_constant_coloring->set_int_val(constant_evac_coloring);
      break;
    case COLORBAND:
      UpdateRGBColors(colorbar_select_index);
      break;
    case FORCE_EXPONENTIAL:
      if(force_exponential==1&&force_fixedpoint==1)force_fixedpoint=0;
      updatemenu = 1;
      update_colorbar_digits = 1;
      break;
    case COLORLABEL_DIGITS:
      if(force_exponential==1&&force_fixedpoint==1)force_exponential=0;
      updatemenu = 1;
      update_colorbar_digits = 1;
      break;
    case SLICE_SKIP:
      slice_skip = CLAMP(slice_skip,1,max_slice_skip);
      SPINNER_slice_skip->set_int_val(slice_skip);
      slice_skipx = slice_skip;
      slice_skipy = slice_skip;
      slice_skipz = slice_skip;
      break;
    case TRANSPARENTLEVEL:
      for(i=nsurfinfo;i<nsurfinfo+MAX_ISO_COLORS+1;i++){
        surfdata *surfi;

        surfi = surfinfo + i;
        surfi->transparent_level=transparent_level;
      }
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      if(SPINNER_transparent_level!=NULL)SPINNER_transparent_level->set_float_val(transparent_level);
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
      if(SPINNER_vectorlinewidth!=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
      SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
    }
    SliceBoundCB(UPDATE_VECTOR);
    break;
  case UPDATE_VECTOR:
    if(vecfactor<0.0){
      vecfactor = 0.0;
      if(SPINNER_vectorlinewidth!=NULL)SPINNER_vectorlinelength->set_float_val(vecfactor);
      if(SPINNER_plot3d_vectorlinelength!=NULL)SPINNER_plot3d_vectorlinelength->set_float_val(vecfactor);
    }
    break;
  case FRAMELOADING:
    sliceframestep=sliceframeskip+1;
    slicezipstep=slicezipskip+1;
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
    break;
  case SETVALMAX:
    break;
  case VALMIN:
    if(is_fed_colorbar==1&&glui_setslicemin==1&&ABS(glui_slicemin)>0.001){
        printf("***warning: min/max bounds for the FED colorbar are set to 0.0 and 3.0 respectively.\n");
      printf("   To use different min/max bounds, change the colorbar.\n");
      glui_slicemin = 0.0;
    }
    break;
  case VALMAX:
    if(is_fed_colorbar==1&&glui_setslicemax==1&&ABS(glui_slicemax-3.0)>0.001){
        printf("***warning: min/max bounds for the FED colorbar are set to 0.0 and 3.0 respectively.\n");
      printf("   To use different min/max bounds, change the colorbar.\n");
      glui_slicemax = 3.0;
    }
    break;
  case FILETYPE_INDEX:
    if(slice_bounds_dialog==1&&list_slice_index==fire_line_index){
      slice_bounds_dialog=0;
    }
    if(slice_bounds_dialog==0&&list_slice_index!=fire_line_index){
      slice_bounds_dialog=1;
    }
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
      SliceBoundCB(SETVALMIN);

      SliceBoundCB(SETVALMAX);
    break;
  case UPDATE_DATA_COLORS:
  case FILE_UPDATE:
    if(research_mode==1){
      SliceBoundCB(SET_GLOBAL_BOUNDS);
    }
    slice_fileupdate++;
    if(slice_fileupdate>1){
      slice_fileupdate--;
      break;
    }
    for(ii = nslice_loaded - 1; ii >= 0; ii--){
      i = slice_loaded_list[ii];
      sd = sliceinfo + i;
      if(sd->slicefile_labelindex == slicefile_labelindex){
        last_slice = i;
        break;
      }
    }
    for(ii = 0; ii < nslice_loaded; ii++){
      i = slice_loaded_list[ii];
      sd = sliceinfo + i;
      if(sd->slicefile_labelindex == slicefile_labelindex){
        int set_slicecolor;

        set_slicecolor = DEFER_SLICECOLOR;
        if(i == last_slice)set_slicecolor = SET_SLICECOLOR;
        ReadSlice("", i, ALL_FRAMES, NULL, RESETBOUNDS, set_slicecolor, &error);
      }
    }
    slice_fileupdate--;
    use_slice_glui_bounds = 0;
    break;
  case FILE_RELOAD:
    if(research_mode==1){
      SliceBoundCB(SET_GLOBAL_BOUNDS);
    }
    SetLoadedSliceBounds(NULL, 0);
    ReloadAllVectorSliceFiles();
    ReloadAllSliceFiles();
    SliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ ShowGluiBounds ------------------------ */

extern "C" void ShowGluiBounds(int menu_id){
  if(menu_id==DIALOG_BOUNDS){

    UpdateChar();
    FileRolloutCB(FILEBOUNDS_ROLLOUT);
  }
  else if(menu_id == DIALOG_SHOWFILES){
    FileRolloutCB(SHOWHIDE_ROLLOUT);
  }
  else if(menu_id==DIALOG_CONFIG){
    FileRolloutCB(CONFIG_ROLLOUT);
  }
  else if(menu_id==DIALOG_AUTOLOAD){
    FileRolloutCB(LOAD_ROLLOUT);
  }
  else if(menu_id==DIALOG_TIME){
    FileRolloutCB(TIME_ROLLOUT);
  }
  else if(menu_id==DIALOG_SCRIPT){
    FileRolloutCB(SCRIPT_ROLLOUT);
  }
  else if(menu_id == DIALOG_SMOKEZIP){
    FileRolloutCB(COMPRESS_ROLLOUT);
  }
  else if(menu_id == DIALOG_3DSMOKE){
    FileRolloutCB(FILEBOUNDS_ROLLOUT);
    BoundRolloutCB(SMOKE3D_ROLLOUT);
  }
  glui_bounds->show();
}

/* ------------------ ShowBoundsDialog ------------------------ */

extern "C" void ShowBoundsDialog(int type){
  if(type==DIALOG_3DSMOKE){
    ShowGluiBounds(DIALOG_3DSMOKE);
  }
  else{
    ShowGluiBounds(DIALOG_BOUNDS);
  }
  switch (type){
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ UpdateOverwrite ------------------------ */

extern "C" void UpdateOverwrite(void){
  if(CHECKBOX_overwrite_all!=NULL)CHECKBOX_overwrite_all->set_int_val(overwrite_all);
  if(CHECKBOX_compress_autoloaded!=NULL)CHECKBOX_compress_autoloaded->set_int_val(compress_autoloaded);
}

/* ------------------ HideGluiBounds ------------------------ */

extern "C" void HideGluiBounds(void){
  CloseRollouts(glui_bounds);
}

/* ------------------ UpdateVectorWidgets ------------------------ */

extern "C" void UpdateVectorWidgets(void){
  Plot3DBoundCB(UPDATE_VECTOR_FROM_SMV);
  SliceBoundCB(UPDATE_VECTOR_FROM_SMV);
}

/* ------------------ UpdatePlot3dDisplay ------------------------ */

extern "C" void UpdatePlot3dDisplay(void){
  if(RADIO_plot3d_display!=NULL)RADIO_plot3d_display->set_int_val(contour_type);
}

/* ------------------ UpdateGluiTimeBounds ------------------------ */

extern "C" void UpdateGluiTimeBounds(float time_min, float time_max){
  if(SPINNER_timebounds!=NULL){
    SPINNER_timebounds->set_float_limits(time_min,time_max);
  }
}

/* ------------------ UpdateTBounds ------------------------ */

extern "C" void UpdateTBounds(void){
  settmin_p=use_tload_begin;
  settmax_p=use_tload_end;
  tmin_p=tload_begin;
  tmax_p=tload_end;

  settmin_s=use_tload_begin;
  settmax_s=use_tload_end;
  tmin_s=tload_begin;
  tmax_s=tload_end;

  settmin_i=use_tload_begin;
  settmax_i=use_tload_end;
  tmin_i=tload_begin;
  tmax_i=tload_end;

  settmin_s=use_tload_begin;
  settmax_s=use_tload_end;
  tmin_s=tload_begin;
  tmax_s=tload_end;

  settmin_b=use_tload_begin;
  settmax_b=use_tload_end;
  tmin_b=tload_begin;
  tmax_b=tload_end;

  if(use_tload_skip==1){
    smoke3dframeskip=tload_skip;
    boundframeskip=tload_skip;
    isoframeskip_global=tload_skip;
    partframeskip=tload_skip;
    evacframeskip=tload_skip;
    sliceframeskip=tload_skip;
  }
  else{
    smoke3dframeskip=0;
    boundframeskip=0;
    isoframeskip_global=0;
    partframeskip=0;
    evacframeskip=0;
    sliceframeskip=0;
  }

  Smoke3dBoundCB(FRAMELOADING);
  BoundBoundCB(FRAMELOADING);
  IsoBoundCB(FRAMELOADING);
  PartBoundCB(FRAMELOADING);
  SliceBoundCB(FRAMELOADING);
}

/* ------------------ UpdateShowHideButtons ------------------------ */

extern "C" void UpdateShowHideButtons(void){

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
