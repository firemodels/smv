#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "IOobjects.h"

#define NEW_CURVE      0
#define EXISTING_CURVE 1

#define GENPLOT_ADD_PLOT            101
#define GENPLOT_REM_PLOT            102
#define GENPLOT_REM_ALL_PLOTS       136
#define GENPLOT_SHOW_PLOT           103
#define GENPLOT_SELECT_PLOT         104
#define GENPLOT_ADD_DEV_PLOTS       105
#define GENPLOT_REM_DEV_PLOTS       106
#define GENPLOT_SELECT_DEV_PLOTS    107
#define GENPLOT_RESET_DEV_PLOTS     108

#define GENPLOT_PLOT_SIZE           110
#define GENPLOT_SET_PLOTPOS         111
#define GENPLOT_PLOT_LABEL          112
#define GENPLOT_PLOT_MINMAX         113
#define GENPLOT_RESET_BOUNDS        114
#define GENPLOT_PLOT_DIST           115
#define GENPLOT_UPDATE              116
#define GENPLOT_PLOT_TMINMAX        118
#define GENPLOT_SHOW_PLOTS          119

#define GENPLOT_CSV_FILETYPE        121
#define GENPLOT_CURVE_UNIT          122

#define GENPLOT_ADD_CURVE           131
#define GENPLOT_REM_CURVE           132
#define GENPLOT_REM_ALLCURVES       133
#define GENPLOT_REM_SELECTEDCURVE   134
#define GENPLOT_SELECT_CURVE        135

#define GENPLOT_CURVE_FACTOR        141

#define GENPLOT_XYZ                 151
#define GENPLOT_RESET_FUEL_HOC      152
#define GENPLOT_RESET_FUEL_1P0      153
#define GENPLOT_USE_FOREGROUND_COLOR 154

#define GENPLOT_SAVE                161
#define GENPLOT_CLOSE               162

#define DEVICE_WINDROSE_SHOW_FIRST   996
#define DEVICE_WINDROSE_SHOW_NEXT    997
#define DEVICE_WINDROSE_DXYZ         995
#define DEVICE_WINDROSE_SETPOS       993
#define DEVICE_WINDROSE_DT           998
#define DEVICE_WINDROSE_DTMINMAX     994
#define DEVICE_WINDROSE_UPDATE       999
#define DEVICE_WINDROSE_SHOWHIDEALL 1000

#define OPEN_UP 0
#define OPEN_DOWN 1
#define OPEN_FILEINDEX 2
#define OPEN_OPEN 3
#define OPEN_CANCEL 4
#define OPEN_FILTER 5
#define OPEN_APPLY_FILTER 6
#define OPEN_UPDATE_LIST 7

class CGluiOpen {
};

int gluiopen_file_index=0;
int gluiopen_nfilelist=0;
char gluiopen_path_dir[1024];
filelistdata *gluiopen_filelist;
char gluiopen_filter[sizeof(GLUI_String)];
char gluiopen_filter2[sizeof(GLUI_String)];

GLUI *glui_device=NULL;
GLUI *glui_plot2d = NULL;

GLUI_EditText *EDIT_plot_label   = NULL;
GLUI_EditText *EDIT_scaled_label = NULL;
GLUI_EditText *EDIT_scaled_unit  = NULL;
GLUI_EditText *EDIT_xaxis_label  = NULL;

GLUI_Button *BUTTON_plot_position = NULL;
GLUI_Button *BUTTON_add_plot = NULL;
GLUI_Button *BUTTON_rem_plot = NULL;
GLUI_Button *BUTTON_rem_all_plots = NULL;
GLUI_Button *BUTTON_open_down=NULL;
GLUI_Button *BUTTON_device_2=NULL;
GLUI_Button *BUTTON_plot2d_2=NULL;
GLUI_Button *BUTTON_update_windrose = NULL;
GLUI_Button *BUTTON_reset_plot2d_bounds = NULL;

GLUI_Checkbox *CHECKBOX_genplot_use_usermin = NULL;
GLUI_Checkbox *CHECKBOX_genplot_use_usermax = NULL;
GLUI_Checkbox *CHECKBOX_show_genplot        = NULL;
GLUI_Checkbox *CHECKBOX_show_plot_title          = NULL;
GLUI_Checkbox *CHECKBOX_show_curve_labels   = NULL;
GLUI_Checkbox *CHECKBOX_show_xaxis_bounds    = NULL;
GLUI_Checkbox *CHECKBOX_show_xaxis_labels    = NULL;
GLUI_Checkbox *CHECKBOX_show_yaxis_bounds    = NULL;
GLUI_Checkbox *CHECKBOX_show_yaxis_units     = NULL;
GLUI_Checkbox *CHECKBOX_show_curve_values   = NULL;
GLUI_Checkbox *CHECKBOX_genplot_use_valmin[3];
GLUI_Checkbox *CHECKBOX_genplot_use_valmax[3];
GLUI_Checkbox *CHECKBOX_device_1=NULL;
GLUI_Checkbox *CHECKBOX_showdevice_val=NULL;
GLUI_Checkbox *CHECKBOX_device_3=NULL;
GLUI_Checkbox *CHECKBOX_device_4=NULL;
GLUI_Checkbox *CHECKBOX_device_5=NULL;
GLUI_Checkbox *CHECKBOX_device_6=NULL;
GLUI_Checkbox *CHECKBOX_device_orientation = NULL;
GLUI_Checkbox *CHECKBOX_vis_xtree = NULL;
GLUI_Checkbox *CHECKBOX_vis_ytree = NULL;
GLUI_Checkbox *CHECKBOX_vis_ztree = NULL;
GLUI_Checkbox *CHECKBOX_showbeam_as_line = NULL;
GLUI_Checkbox *CHECKBOX_use_beamcolor = NULL;
GLUI_Checkbox **CHECKBOX_showz_windrose;
GLUI_Checkbox *CHECKBOX_curve_apply_factor = NULL;

GLUI_EditText *EDIT_filter=NULL;

GLUI_Listbox *LIST_csvfile = NULL;
GLUI_Listbox *LIST_csvID    = NULL;
GLUI_Listbox *LIST_curve_unit = NULL;
GLUI_Listbox *LIST_curve_compartments = NULL;
GLUI_Listbox *LIST_plots = NULL;
GLUI_Listbox *LIST_plotcurves = NULL;
GLUI_Listbox *LIST_open=NULL;
GLUI_Listbox *LIST_plot_add_dev = NULL;

GLUI_Panel *PANEL_allplotproperties = NULL;
GLUI_Panel *PANEL_plottitle = NULL;
GLUI_Panel *PANEL_plotother = NULL;
GLUI_Panel *PANEL_newplot = NULL;
GLUI_Panel *PANEL_plot_bounds2 = NULL;
GLUI_Panel *PANEL_curve_factor = NULL;
GLUI_Panel *PANEL_plot2d_label3 = NULL;
GLUI_Panel *PANEL_bound1 = NULL;
GLUI_Panel *PANEL_bound2 = NULL;
GLUI_Panel *PANEL_bound3 = NULL;
GLUI_Panel *PANEL_csv1 = NULL;
GLUI_Panel *PANEL_curve_color = NULL;
GLUI_Panel *PANEL_curve_usermin = NULL;
GLUI_Panel *PANEL_curve_usermax = NULL;
GLUI_Panel *PANEL_modify_curve = NULL;
GLUI_Panel *PANEL_plot_xlabels=NULL;
GLUI_Panel *PANEL_plot_ylabels=NULL;
GLUI_Panel *PANEL_plot_labels2 = NULL;
GLUI_Panel *PANEL_plot1 = NULL;
GLUI_Panel *PANEL_add_curve = NULL;
GLUI_Panel *PANEL_add_curve1 = NULL;
GLUI_Panel *PANEL_plots = NULL;
GLUI_Panel *PANEL_genplot = NULL;
GLUI_Panel *PANEL_plot_position = NULL;
GLUI_Panel *PANEL_objects=NULL;
GLUI_Panel *PANEL_vectors=NULL;
GLUI_Panel *PANEL_arrow_base=NULL;
GLUI_Panel *PANEL_arrow_height=NULL;
GLUI_Panel *PANEL_label3=NULL;
GLUI_Panel *PANEL_vector_type=NULL;
GLUI_Panel *PANEL_beam=NULL;
GLUI_Panel *PANEL_orientation=NULL;
GLUI_Panel *PANEL_wr1=NULL;
GLUI_Panel *PANEL_windrose_merge = NULL;
GLUI_Panel *PANEL_windrose_merget = NULL;
GLUI_Panel *PANEL_windrose_mergexyz = NULL;
GLUI_Panel *PANEL_plotproperties1=NULL;
GLUI_Panel *PANEL_plotproperties2=NULL;

GLUI_RadioGroup *RADIO_windrose_ttype = NULL;
GLUI_RadioGroup *RADIO_windrose_merge_type=NULL;
GLUI_RadioGroup *RADIO_vectortype=NULL;
GLUI_RadioGroup *RADIO_scale_windrose=NULL;
GLUI_RadioGroup *RADIO_windstate_windrose = NULL;

GLUI_Rollout *ROLLOUT_devplots = NULL;
GLUI_Rollout *ROLLOUT_plot_bounds = NULL;
GLUI_Rollout *ROLLOUT_curve_properties = NULL;
GLUI_Rollout *ROLLOUT_positions = NULL;
GLUI_Rollout *ROLLOUT_plotdevice = NULL;
GLUI_Rollout *ROLLOUT_plotproperties = NULL;
GLUI_Rollout *ROLLOUT_values = NULL;
GLUI_Rollout *ROLLOUT_device2Dplots=NULL;
GLUI_Rollout *ROLLOUT_showhide_windrose = NULL;
GLUI_Rollout *ROLLOUT_show_windrose2 = NULL;
GLUI_Rollout *ROLLOUT_scale_windrose = NULL;
GLUI_Rollout *ROLLOUT_2Dplots = NULL;
GLUI_Rollout *ROLLOUT_velocityvectors = NULL;
GLUI_Rollout *ROLLOUT_smvobjects=NULL;
GLUI_Rollout *ROLLOUT_arrow_dimensions = NULL;
GLUI_Rollout *ROLLOUT_windrose = NULL;
GLUI_Rollout **ROLLOUT_showz_windrose;
GLUI_Rollout *ROLLOUT_trees = NULL;

GLUI_Spinner *SPINNER_genplot_x = NULL;
GLUI_Spinner *SPINNER_genplot_y = NULL;
GLUI_Spinner *SPINNER_genplot_z = NULL;
GLUI_Spinner *SPINNER_genplot_red = NULL;
GLUI_Spinner *SPINNER_genplot_green = NULL;
GLUI_Spinner *SPINNER_genplot_blue = NULL;
GLUI_Spinner *SPINNER_genplot_linewidth = NULL;
GLUI_Spinner *SPINNER_genplot_valmin[3];
GLUI_Spinner *SPINNER_genplot_valmax[3];
GLUI_Spinner *SPINNER_curve_factor = NULL;
GLUI_Spinner *SPINNER_size_factor = NULL;
GLUI_Spinner *SPINNER_plot2d_time_average = NULL;
GLUI_Spinner *SPINNER_windrose_merge_dxyzt[6];
GLUI_Spinner *SPINNER_sensorrelsize=NULL;
GLUI_Spinner *SPINNER_orientation_scale=NULL;
GLUI_Spinner *SPINNER_beam_line_width = NULL;
GLUI_Spinner *SPINNER_mintreesize = NULL;
GLUI_Spinner *SPINNER_beam_color[3];
GLUI_Spinner *SPINNER_nr_windrose = NULL;
GLUI_Spinner *SPINNER_ntheta_windrose = NULL;
GLUI_Spinner *SPINNER_radius_windrose = NULL;
GLUI_Spinner *SPINNER_scale_increment_windrose = NULL;
GLUI_Spinner *SPINNER_scale_max_windrose = NULL;
GLUI_Spinner *SPINNER_windrose_first=NULL;
GLUI_Spinner *SPINNER_windrose_next=NULL;
GLUI_Spinner *SPINNER_plot2d_xaxis_position=NULL;

#define OBJECTS_ROLLOUT     0
#define FLOWVECTORS_ROLLOUT 1
#define WINDROSE_ROLLOUT    2
#define PLOT2D_ROLLOUT      3

procdata deviceprocinfo[4];
int ndeviceprocinfo = 0;

#define PLOT2D_NEW_ROLLOUT     0
#define PLOT2D_CURVE_ROLLOUT   1
#define PLOT2D_BOUNDS_ROLLOUT  2

procdata plot2dprocinfo[3];
int nplot2dprocinfo = 0;

/* ------------------ GLUIUpdatePlot2DSize ------------------------ */

extern "C" void GLUIUpdatePlot2DSize(void){
  if(SPINNER_size_factor!=NULL)SPINNER_size_factor->set_float_val(plot2d_size_factor);
  if(SPINNER_plot2d_time_average!=NULL)SPINNER_plot2d_time_average->set_float_val(plot2d_time_average);
}

/* ------------------ Device_Rollout_CB ------------------------ */

void Device_Rollout_CB(int var){
  GLUIToggleRollout(deviceprocinfo, ndeviceprocinfo, var);
}

/* ------------------ Plot2D_Rollout_CB ------------------------ */

void Plot2D_Rollout_CB(int var){
  GLUIToggleRollout(plot2dprocinfo, nplot2dprocinfo, var);
}

/* ------------------ GLUIUpdateDeviceTypes ------------------------ */

 extern "C" void GLUIUpdateDeviceTypes(int val){
  devicetypes_index = val;
  updatemenu = 1;
}

/* ------------------ GLUIUpdateDeviceShow ------------------------ */

extern "C" void GLUIUpdateDeviceShow(void){
  if(CHECKBOX_showdevice_val!=NULL)CHECKBOX_showdevice_val->set_int_val(showdevice_val);
}

/* ------------------ GLUIUpdateWindRoseDevices ------------------------ */

extern "C" void GLUIUpdateWindRoseDevices(int option){
  int i,icheckboxes;

  icheckboxes = 0;
  for(i = 0; i<nzwindtreeinfo; i++){
    treedevicedata *treei;
    int j;

    treei = zwindtreeinfo[i];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;
      vdevicedata *vd;

      vdevsorti = global_scase.devicecoll.vdevices_sorted+j;
      vd = vdevsorti->vdeviceinfo;
      if(vd->udev==NULL&&vd->vdev==NULL&&vd->wdev==NULL&&
         vd->angledev == NULL&&vd->veldev == NULL)continue;

      if(vdevsorti->dir == ZDIR&&vd->unique != 0){

      // when reading ini file
        if(option==UPDATE_WINDROSE_DEVICE&&windrosez_showhide!=NULL){
          vd->display=windrosez_showhide[icheckboxes];
        }

        // when updating checkboxes
        if(option==UPDATE_WINDROSE_CHECKBOX&&CHECKBOX_showz_windrose!=NULL&&
             CHECKBOX_showz_windrose[icheckboxes]!=NULL){
          CHECKBOX_showz_windrose[icheckboxes]->set_int_val(vd->display);
        }

        // when writing ini file
        if(option==UPDATE_WINDROSE_SHOWHIDE&&windrosez_showhide!=NULL){
          windrosez_showhide[icheckboxes]=vd->display;
        }
        icheckboxes++;
      }
    }
  }
  nwindrosez_showhide=icheckboxes;
  update_windrose_showhide = 0;
  if(option == UPDATE_WINDROSE_SHOWHIDE&&windrosez_showhide == NULL){
    NewMemory((void **)&windrosez_showhide, nwindrosez_showhide * sizeof(int));
    if(windrosez_showhide!=NULL)GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_SHOWHIDE);
  }
}

/* ------------------ GLUIUpdateShowbeamAsLine ------------------------ */

extern "C" void GLUIUpdateShowbeamAsLine(void){
  if(CHECKBOX_showbeam_as_line!=NULL)CHECKBOX_showbeam_as_line->set_int_val(showbeam_as_line);
}

/* ------------------ GLUIUpdateDeviceSize ------------------------ */

extern "C" void GLUIUpdateDeviceSize(void){
  if(sensorrelsize<sensorrelsizeMIN)sensorrelsize = sensorrelsizeMIN;
  if(SPINNER_sensorrelsize!=NULL)SPINNER_sensorrelsize->set_float_val(sensorrelsize);
}

/* ------------------ GLUIUpdateDeviceOrientation ------------------------ */

extern "C" void GLUIUpdateDeviceOrientation(void){
  if(CHECKBOX_device_orientation!=NULL)CHECKBOX_device_orientation->set_int_val(show_device_orientation);
}

/* ------------------ UpdateShowWindRoses ------------------------ */

void UpdateShowWindRoses(void){
  int i;
  int icheckbox;

  icheckbox=0;
  for(i = 0; i < nzwindtreeinfo; i++){
    treedevicedata *treei;
    int j;
    int idev;

    treei = zwindtreeinfo[i];

    idev = 0;
    for(j = treei->first; j <= treei->last; j++){
      vdevicesortdata *vdevsorti;
      vdevicedata *vd;

      vdevsorti = global_scase.devicecoll.vdevices_sorted + j;
      vd = vdevsorti->vdeviceinfo;
      if(vd->udev==NULL&&vd->vdev==NULL&&vd->wdev==NULL&&
         vd->angledev == NULL&&vd->veldev == NULL)continue;

      if(vdevsorti->dir == ZDIR&&vd->unique != 0){
        int kk, idev_hat;
        //idev = istart + k*iskip
        kk = (idev-windrose_first)/windrose_next;
        idev_hat = windrose_first + kk*windrose_next;
        if(kk>=0&&idev==idev_hat){
          vd->display = 1;
        }
        else{
          vd->display = 0;
        }
        CHECKBOX_showz_windrose[icheckbox]->set_int_val(vd->display);
        idev++;
        icheckbox++;
      }
    }
  }
}

/* ------------------ RemoveCurve ------------------------ */

void RemoveCurve(plot2ddata *plot2di, int index){
  int i;
  curvedata *curve;

  for(i = 0; i < PLOT2D_MAX_CURVES; i++){
    LIST_plotcurves->delete_item(i);
  }
  if(index < 0){
    plot2di->ncurves = 0;
    LIST_plotcurves->set_int_val(-1);
    return;
  }
  curve = plot2di->curve+index;
  if(index >= 0){
    FREEMEMORY(curve->vals);
  }
//    (0,...,i-1,i+1,...,n-1)
  if(plot2di->ncurves>index+1){
    memmove(curve, curve+1, (plot2di->ncurves-index-1)*sizeof(curvedata));
  }
  (plot2di->ncurves)--;
  for(i = 0; i < plot2di->ncurves; i++){
    char *label;

    label = GetPlotShortLabel(plot2di, i);
    LIST_plotcurves->add_item(i, label);
  }
  LIST_plotcurves->set_int_val(-1);
  index=-1;
  if(plot2di->ncurves > 0)index = CLAMP(index-1, 0, plot2di->ncurves-1);
  LIST_plotcurves->set_int_val(index);
}

/* ------------------ InCSVPlot ------------------------ */

int InCSVPlot(plot2ddata *plot2di, char *c_type, int index){
  int i;

  for(i = 0; i < plot2di->ncurves; i++){
    if(plot2di->curve[i].csv_col_index != index)continue;
    if(strcmp(plot2di->curve[i].c_type, c_type)!=0)continue;
    return 1;
  }
  return 0;
}

/* ------------------ UpdateCurveControls ------------------------ */

void UpdateCurveControls(char *unit){
  SPINNER_genplot_red->set_int_val(glui_curve.color[0]);
  SPINNER_genplot_green->set_int_val(glui_curve.color[1]);
  SPINNER_genplot_blue->set_int_val(glui_curve.color[2]);
  SPINNER_genplot_linewidth->set_float_val(glui_curve.linewidth);
  SPINNER_curve_factor->set_float_val(glui_curve.curve_factor);
  CHECKBOX_curve_apply_factor->set_int_val(glui_curve.apply_curve_factor);
  EDIT_scaled_label->set_text(glui_curve.scaled_label);
  EDIT_scaled_unit->set_text(glui_curve.scaled_unit);
}

/* ------------------ AddCSVCurve ------------------------ */

void AddCSVCurve(plot2ddata *plot2di, int index, int option){
  int have_plot, nplots;
  csvfiledata *csvfi;
  csvdata *csvi;
  char *c_type;

  csvi = GetCsvCurve(index, &csvfi);
  if(csvfi == NULL || csvi == NULL)return;
  c_type = csvfi->c_type;

  have_plot = 0;
  if(option == NEW_CURVE)have_plot = InCSVPlot(plot2di, c_type, index);
  nplots = plot2di->ncurves;
  if(have_plot == 0 && nplots < PLOT2D_MAX_CURVES){
    char label[255];
    curvedata *curve;

    curve = plot2di->curve + nplots;
    curve->vmax = csvi->valmax;
    curve->vmin = csvi->valmin;
    curve->csv_col_index = index;
    curve->quantity = NULL;
    if(option==NEW_CURVE){
      char *shortlabel, *unit;

      curve->csv_file_index     = glui_csv_file_index;
      curve->color[0]           = glui_curve_default.color[0];
      curve->color[1]           = glui_curve_default.color[1];
      curve->color[2]           = glui_curve_default.color[2];
      curve->use_foreground_color = glui_curve_default.use_foreground_color;
      curve->linewidth          = glui_curve_default.linewidth;
      curve->curve_factor       = glui_curve_default.curve_factor;
      curve->apply_curve_factor = glui_curve_default.apply_curve_factor;
      curve->vals               = glui_curve_default.vals;
      curve->update_avg         = glui_curve_default.update_avg;

      shortlabel = GetPlotShortLabel2(plot2di, curve);
      if(shortlabel!=curve->scaled_label)strcpy(curve->scaled_label, shortlabel);

      unit = GetPlotUnit2(plot2di, curve);
      if(unit!=curve->scaled_unit)strcpy(curve->scaled_unit,  unit);
    }
    else{
      csvfi  = global_scase.csvcoll.csvfileinfo+curve->csv_file_index;
      c_type = curve->c_type;
      csvi   = csvfi->csvinfo+curve->csv_col_index;
    }
    if(c_type!=curve->c_type)strcpy(curve->c_type, c_type);
    if(strcmp(c_type, "devc")==0){
      curve->quantity = global_scase.devicecoll.deviceinfo[curve->csv_col_index-1].quantity;
    }
    plot2di->ncurves = nplots+1;
    strcpy(label, c_type);
    strcat(label, "/");
    if(csvi->label.shortlabel!=NULL)strcat(label, csvi->label.shortlabel);

    LIST_plotcurves->add_item(nplots, label);
    LIST_plotcurves->set_int_val(nplots);
    memcpy(&glui_curve, curve, sizeof(curvedata));
    nplots++;
    UpdateCurveControls(csvi->label.unit);
  }
}

/* ------------------ MakeCurveList ------------------------ */

void MakeCurveList(plot2ddata *plot2di, int option){
  int i;

  if(option == 1)LIST_plotcurves->add_item(-1, "");
  for(i = 0; i < plot2di->ncurves_ini; i++){
    int curv_index;

    curv_index = plot2di->curve[i].csv_col_index_ini;
    AddCSVCurve(plot2di, curv_index, EXISTING_CURVE);
    GLUIGenPlotCB(GENPLOT_SELECT_CURVE);
  }
}

/* ------------------ Plot2D2Glui ------------------------ */

void Plot2D2Glui(int index){
  if(index >= nplot2dinfo)return;
  RemoveCurve(glui_plot2dinfo, -1);

  memcpy(glui_plot2dinfo, plot2dinfo + index, sizeof(plot2ddata));

  int i;
  for(i=0;i<glui_plot2dinfo->ncurves; i++){
    glui_plot2dinfo->curve[i].csv_col_index_ini = glui_plot2dinfo->curve[i].csv_col_index;
  }
  glui_plot2dinfo->ncurves_ini = glui_plot2dinfo->ncurves;
  glui_plot2dinfo->ncurves = 0;
  MakeCurveList(glui_plot2dinfo, 0);
  SPINNER_genplot_x->set_float_val(glui_plot2dinfo->xyz[0]);
  SPINNER_genplot_y->set_float_val(glui_plot2dinfo->xyz[1]);
  SPINNER_genplot_z->set_float_val(glui_plot2dinfo->xyz[2]);

  if(glui_plot2dinfo->curve_index >= 0){
    curvedata *curve;
    curve = glui_plot2dinfo->curve + glui_plot2dinfo->curve_index;
    memcpy(&glui_curve, curve, sizeof(curvedata));
  }
  else{
    memcpy(&glui_curve, &glui_curve_default, sizeof(curvedata));
  }
  CHECKBOX_show_genplot->set_int_val(glui_plot2dinfo->show);
  EDIT_plot_label->set_text(glui_plot2dinfo->plot_label);
  for(i = 0; i < 2; i++){
    CHECKBOX_genplot_use_valmin[i]->set_int_val(glui_plot2dinfo->use_valmin[i]);
    CHECKBOX_genplot_use_valmax[i]->set_int_val(glui_plot2dinfo->use_valmax[i]);
    SPINNER_genplot_valmin[i]->set_float_val(glui_plot2dinfo->valmin[i]);
    SPINNER_genplot_valmax[i]->set_float_val(glui_plot2dinfo->valmax[i]);
  }
  UpdateCurveControls(NULL);
}

/* ------------------ GetPlotLabel ------------------------ */

void GetPlotLabel(char *label, int size){
  int index, i;

  for(index = 1;; index++){
    int skip;

//    sprintf(label, "plot %i", index);
    snprintf(label, size, "plot %i", index);
    skip = 0;
    for(i = 0; i<nplot2dinfo-1; i++){
      plot2ddata *plot2di;

      plot2di = plot2dinfo+i;
      if(strcmp(plot2di->plot_label, label)==0){
        skip = 1;
        break;
      }
    }
    if(skip==0)return;
  }
}

/* ------------------ AddPlot ------------------------ */

void AddPlot(void){
  char label[32];

  nplot2dinfo++;
  if(nplot2dinfo==1){
    NewMemory((void **)&plot2dinfo,   nplot2dinfo*sizeof(plot2ddata));
  }
  else{
    ResizeMemory((void **)&plot2dinfo,   nplot2dinfo*sizeof(plot2ddata));
  }
  iplot2dinfo = nplot2dinfo - 1;
  InitPlot2D(plot2dinfo + iplot2dinfo, nplot2dinfo);
  Plot2D2Glui(iplot2dinfo);
  GetPlotLabel(label, 32);
  strcpy(plot2dinfo[iplot2dinfo].plot_label, label);
  LIST_plots->add_item(iplot2dinfo, label);
  LIST_plots->set_int_val(iplot2dinfo);
  memcpy(&glui_curve, &glui_curve_default, sizeof(curvedata));
  UpdateCurveControls(NULL);
  }

/* ------------------ RemovePlot ------------------------ */

void RemovePlot(int i){
  int ii;

  if(nplot2dinfo <= 0||i==-1)return;
  for(ii = 0; ii < nplot2dinfo; ii++){
    plot2ddata *plot2di;

    plot2di = plot2dinfo + ii;
    LIST_plots->delete_item(plot2di->plot_label);
  }
  for(ii=i+1;ii<nplot2dinfo;ii++){
    memcpy(plot2dinfo+ii-1, plot2dinfo+ii, sizeof(plot2ddata));
  }
  nplot2dinfo--;
  if(nplot2dinfo==0){
    FREEMEMORY(plot2dinfo);
  }
  if(nplot2dinfo>0){
    iplot2dinfo = 0;
    for(ii = 0; ii < nplot2dinfo; ii++){
      plot2ddata *plot2di;

      plot2di = plot2dinfo + ii;
      LIST_plots->add_item(ii, plot2di->plot_label);
    }
    Plot2D2Glui(iplot2dinfo);
    LIST_plots->set_int_val(-1);
    iplot2dinfo = 0;
    LIST_plots->set_int_val(iplot2dinfo);
  }
  else{
    LIST_plots->set_int_val(-1);
  }
}

/* ------------------ Glui2Plot2d ------------------------ */

void Glui2Plot2D(int index){
  if(plot2dinfo != NULL){
    plot2ddata *plot2di;

    plot2di = plot2dinfo + index;
    memcpy(plot2di, glui_plot2dinfo, sizeof(plot2ddata));
  }
}

/* ------------------ UpdatePlotList ------------------------ */

void UpdatePlotList(void){
  int i;

  for(i = 0; i < nplot2dinfo; i++){
    plot2ddata *plot2di;

    plot2di = plot2dinfo + i;
    LIST_plots->delete_item(i);
    LIST_plots->add_item(i, plot2di->plot_label);
  }
}

/* ------------------ EnableDisablePlot2D ------------------------ */

void EnableDisablePlot2D(void){
  if(plot2d_dialogs_defined == 0)return;
  if(nplot2dinfo == 0){
    ROLLOUT_curve_properties->disable();
    ROLLOUT_plot_bounds->disable();
  }
  else{
    PANEL_add_curve->enable();
    ROLLOUT_plot_bounds->enable();
    if(glui_plot2dinfo->ncurves == 0){
      ROLLOUT_curve_properties->disable();
    }
    else{
      ROLLOUT_curve_properties->enable();
    }
  }
}

/* ------------------ GLUIUpdateDeviceAdd ------------------------ */

extern "C" void GLUIUpdateDeviceAdd(void){
  if(LIST_plot_add_dev!=NULL){
    LIST_plot_add_dev->set_int_val(idevice_add);
  }
}

/* ------------------ GLUIShowPlot2D ------------------------ */

extern "C" void GLUIShowPlot2D(void){
  if(glui_plot2d != NULL){
    glui_plot2d->show();
    EnableDisablePlot2D();
  }
}

/* ------------------ GLUIHidePlot2D ------------------------ */

extern "C" void GLUIHidePlot2D(void){
  if(glui_plot2d != NULL){
    glui_plot2d->hide();
    EnableDisablePlot2D();
  }
}

/* ------------------ GetCsvUnit ------------------------ */

char *GetCsvUnit(void){
  int unit_id;

  unit_id = LIST_curve_unit->get_int_val();
  if(unit_id>=0){
    csvdata *csvunit;

    csvunit = GetCsvCurve(unit_id, NULL);
    if(csvunit==NULL)return NULL;
    if(csvunit->dimensionless==1)return dimensionless;
    return csvunit->label.unit;
  }
  else{
    return NULL;
  }
}

/* ------------------ FilterList ------------------------ */

void FilterList(void){
  int i;

  char unit_label[256];
  int unit_id;
  int compartment_id;

  for(i=0; i<plot2d_max_columns; i++){
    LIST_csvID->delete_item(i);
  }
  {
    csvfiledata *csvfi;

    GetCsvCurve(0, &csvfi);
    if(csvfi == NULL)return;
    unit_id = LIST_curve_unit->get_int_val();
    strcpy(unit_label, "all");
    if(unit_id >= 0){
      csvdata *csvunit;

      csvunit = GetCsvCurve(unit_id, NULL);
      if(csvunit!=NULL&&csvunit->dimensionless == 0){
        strcpy(unit_label, csvunit->label.unit);
      }
      else{
        strcpy(unit_label, "dimensionless");
      }
    }
    compartment_id = -1;
    if(global_scase.isZoneFireModel==1){
      compartment_id = LIST_curve_compartments->get_int_val();
    }
    for(i = 0; i < csvfi->ncsvinfo; i++){
      csvdata *csvi;
      int doit;

      csvi = GetCsvCurve(i, NULL);
      if(csvi==NULL||csvi->skip != 0)continue;
      doit = 0;
      if(strcmp(unit_label, "all") == 0)doit = 1;
      if(doit==0&&csvi->dimensionless == 1 && strcmp(unit_label, "dimensionless")==0)doit = 1;
      if(doit==0&&csvi->dimensionless == 0 && strcmp(unit_label, csvi->label.unit) == 0)doit = 1;
      if(doit==0)continue;
      if(global_scase.isZoneFireModel==1&&compartment_id>=0){
        if(
          strcmp(csvfi->c_type, "compartments") == 0 ||
          strcmp(csvfi->c_type, "masses") == 0       ||
          strcmp(csvfi->c_type, "walls") == 0       ||
          strcmp(csvfi->c_type, "zone") == 0
        ){
          char *label;
          int ilabel;

          doit = 0;
          label = csvi->label.shortlabel;
          if(strlen(label)>=5&&strncmp(label,"HSLAB",5)==0)continue;
          label = strchr(label, '_');
          if(label != NULL){
            label++;
            sscanf(label, "%i", &ilabel);
            if(compartment_id == ilabel-1)doit = 1;
          }

        }
      }
      if(doit==1)LIST_csvID->add_item(i, csvi->label.shortlabel);
    }
  }
}

/* ------------------ GetCsvType ------------------------ */

char *GetCsvType(void){
  csvfiledata *csvfi;

  if(glui_csv_file_index>=0){
    csvfi = global_scase.csvcoll.csvfileinfo+glui_csv_file_index;
    return csvfi->c_type;
  }
  else{
    return NULL;
  }
}

/* ------------------ UpdateCurveLabels ------------------------ */

void UpdateCurveLabels(void){
  char label[256], *unit, *c_type;

  c_type = GetCsvType();
  unit = GetCsvUnit();
  strcpy(label, "");
  if(c_type!=NULL)strcat(label, c_type);
  strcat(label, " curve");
  if(unit==NULL){
    strcat(label, "(any unit):");
  }
  else{
    strcat(label, "(");
    strcat(label, unit);
    strcat(label, ")");
  }
  LIST_csvID->set_name(label);

}

/* ------------------ UpdateCsvList ------------------------ */

void UpdateCsvList(void){
  int i;
  char label[256];
  char label2[256];
  csvfiledata *csvfi;

  for(i=0; i<plot2d_max_columns; i++){
    LIST_csvID->delete_item(i);
  }
  GetCsvCurve(0, &csvfi);
  if(csvfi == NULL)return;
  for(i = 0; i < csvfi->ncsvinfo; i++){
    csvdata *csvi;

    csvi = GetCsvCurve(i, NULL);
    if(csvi==NULL||csvi->skip==1)continue;
    LIST_csvID->add_item(i, csvi->label.shortlabel);
  }
  strcpy(label, "add");
  if(plot2dinfo != NULL){
    strcat(label, " curves to ");
    strcat(label, plot2dinfo[iplot2dinfo].plot_label);
  }
  else{
    strcat(label, " curves");
  }
  PANEL_add_curve->set_name(label);

  strcpy(label2, "");
  strcat(label2, csvfi->c_type);
  strcat(label2, " curve:");
  LIST_csvID->set_name(label2);

  for(i=0; i<plot2d_max_columns; i++){
    LIST_curve_unit->delete_item(i);
  }
  for(i = 0; i < csvfi->ncsvinfo; i++){
    csvdata *csvi;
    int dup_unit, j;

    csvi = GetCsvCurve(i, NULL);
    if(csvi == NULL)continue;
    dup_unit = 0;
    for(j=0; j<i; j++){
      csvdata *csvj;

      csvj = GetCsvCurve(j, NULL);
      if(csvj == NULL)continue;
      if(csvi->dimensionless==0){
        if(strcmp(csvi->label.unit, csvj->label.unit) == 0){
          dup_unit = 1;
          break;
        }
      }
      else{
        if(csvj->dimensionless==1){
          dup_unit = 1;
          break;
        }
      }
    }
    if(dup_unit == 0){
      if(csvi->dimensionless==0){
        LIST_curve_unit->add_item(i, csvi->label.unit);
      }
      else{
        LIST_curve_unit->add_item(i, "dimensionless");
      }
    }
  }
  strcpy(label, "add");
  if(plot2dinfo != NULL){
    strcat(label, " curves to ");
    strcat(label, plot2dinfo[iplot2dinfo].plot_label);
  }
  else{
    strcat(label, " curves");
  }
  PANEL_add_curve->set_name(label);

  UpdateCurveLabels();

  LIST_curve_unit->set_int_val(-1);

  if(BUTTON_plot_position!=NULL){
    if(strcmp(csvfi->c_type, "devc")==0){
      BUTTON_plot_position->enable();
    }
    else{
      BUTTON_plot_position->disable();
    }
  }
}

/* ------------------ SetPlot2DBoundLabels ------------------------ */
void SetPlot2DBoundLabels(plot2ddata *plot2di){
  int i;
  char *axis_right_unit = NULL, *axis_left_unit = NULL;

  if(plot2dinfo == NULL){
    PANEL_bound1->set_name("bound1 (left axis)");
    PANEL_bound2->set_name("bound2 (right axis)");
    PANEL_bound1->disable();
    PANEL_bound2->disable();
    PANEL_bound3->disable();
    return;
  }
  for(i = 0; i < plot2di->ncurves; i++){
    char *unit;

    unit = GetPlotUnit(plot2di, i);
    if(axis_left_unit == NULL){
      axis_left_unit = unit;
      continue;
    }
    if(strcmp(unit, axis_left_unit) != 0){
      axis_right_unit = unit;
      break;
    }
  }
  char label[256];

  if(axis_left_unit != NULL){
    strcpy(label, axis_left_unit);
    strcat(label, " (left axis)");
    PANEL_bound1->enable();
    PANEL_bound3->enable();
  }
  else{
    strcpy(label, "bound (left axis)");
    PANEL_bound1->disable();
    PANEL_bound3->disable();
  }
  PANEL_bound1->set_name(label);

  if(axis_right_unit != NULL){
    strcpy(label, axis_right_unit);
    strcat(label, " (right axis)");
    PANEL_bound2->enable();
    PANEL_bound3->enable();
  }
  else{
    strcpy(label, "bound (right axis)");
    PANEL_bound2->disable();
  }
  PANEL_bound2->set_name(label);
  if(axis_left_unit!=NULL||axis_right_unit!=NULL){
    BUTTON_reset_plot2d_bounds->enable();
  }
  else{
    BUTTON_reset_plot2d_bounds->enable();
  }
}

/* ------------------ InDevList ------------------------ */

int InDevList(devicedata *devi, int n){
  int j;

  for(j = 0; j<n; j++){
    devicedata *devj;

    devj = global_scase.devicecoll.deviceinfo+j;
    if(strcmp(devi->quantity, devj->quantity)==0&&devj->inlist==1)return 1;
  }
  return 0;
}

/* ------------------ UpdatePlotDevList ------------------------ */

void UpdatePlotDevList(void){
  int i;

  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devi;

    devi = global_scase.devicecoll.deviceinfo + i;
    devi->inlist = 0;
  }
  LIST_plot_add_dev->delete_item(-1);
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devi;

    devi = global_scase.devicecoll.deviceinfo+i;
    devi->inlist = 1 - InDevList(devi, i);
    LIST_plot_add_dev->delete_item(i);
  }
  LIST_plot_add_dev->add_item(-1, "");
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devi;

    devi = global_scase.devicecoll.deviceinfo+i;
    if(devi->inlist==1){
      char label[64];

      strcpy(label, "");
      strcat(label, devi->quantity);
      LIST_plot_add_dev->add_item(i, label);
    }
  }
}

/* ------------------ SetPlot2DShowLabel ------------------------ */

void SetPlot2DShowLabel(void){
  if(iplot2dinfo >= 0&&iplot2dinfo<nplot2dinfo && CHECKBOX_show_genplot!=NULL){
    plot2ddata *plot2di;
    char show_label[100];

    plot2di = plot2dinfo+iplot2dinfo;
    strcpy(show_label, "show ");
    strcat(show_label, plot2di->plot_label);
    CHECKBOX_show_genplot->set_name(show_label);
  }
}

/* ------------------ GLUIGenPlotCB ------------------------ */

void GLUIGenPlotCB(int var){
  char label[256];
  int index;
  curvedata *curve;
  int curve_id;

  GLUTPOSTREDISPLAY;
  switch(var){
    case GENPLOT_RESET_BOUNDS:
      plot2ddata *plot2dii;
      int i;

      if(plot2dinfo == NULL)return;
      plot2dii = plot2dinfo + iplot2dinfo;
      GetPlot2DBounds(plot2dii, plot2dii->valmin, plot2dii->valmax);
      for(i=0;i<2;i++){
        SPINNER_genplot_valmin[i]->set_float_val(plot2dii->valmin[i]);
        SPINNER_genplot_valmax[i]->set_float_val(plot2dii->valmax[i]);
      }
      break;
    case GENPLOT_PLOT_TMINMAX:
      GLUIUpdateBoundTbounds();
      break;
    case GENPLOT_PLOT_MINMAX:
      Glui2Plot2D(iplot2dinfo);
      GLUIDeviceCB(DEVICE_TIMEAVERAGE);
      break;
    case GENPLOT_ADD_CURVE:
      curve_id = LIST_csvID->get_int_val();
      AddCSVCurve(glui_plot2dinfo, curve_id, NEW_CURVE);
      Glui2Plot2D(iplot2dinfo);
      EnableDisablePlot2D();
      GLUIGenPlotCB(GENPLOT_RESET_BOUNDS);
      SetPlot2DBoundLabels(plot2dinfo + iplot2dinfo);
      GLUIDeviceCB(DEVICE_TIMEAVERAGE);
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      update_times = 1;
      break;
    case GENPLOT_CSV_FILETYPE:
      UpdateCsvList();
      LIST_csvID->set_int_val(-1);
      UpdateCurveLabels();
      GLUIGenPlotCB(GENPLOT_CURVE_UNIT);
      break;
    case GENPLOT_CURVE_UNIT:
      FilterList();
      LIST_csvID->set_int_val(-1);
      UpdateCurveLabels();
      break;
    case GENPLOT_SELECT_CURVE:
      index = glui_plot2dinfo->curve_index;
      if(index < 0)break;
      curve = glui_plot2dinfo->curve + index;
      memcpy(&glui_curve, curve, sizeof(curvedata));
      char *unit;
      unit = GetPlotUnit(glui_plot2dinfo, index);
      UpdateCurveControls(unit);
      if(BUTTON_plot_position != NULL){
        if(glui_plot2dinfo->curve_index<global_scase.devicecoll.ndeviceinfo){
        }
        else{
          strcpy(label, "Set to device location");
          BUTTON_plot_position->set_name(label);
        }
      }
      if(glui_remove_selected_curve==1){
        GLUIGenPlotCB(GENPLOT_REM_CURVE);
      }
      SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      break;
    case GENPLOT_REM_CURVE:
      RemoveCurve(glui_plot2dinfo, glui_plot2dinfo->curve_index);
      Glui2Plot2D(iplot2dinfo);
      EnableDisablePlot2D();
      SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      update_times = 1;
      break;
    case GENPLOT_REM_SELECTEDCURVE:
      glui_remove_selected_curve = 1;
      GLUIGenPlotCB(GENPLOT_SELECT_CURVE);
      glui_remove_selected_curve = 0;
      Glui2Plot2D(iplot2dinfo);
      EnableDisablePlot2D();
      break;
    case GENPLOT_REM_ALLCURVES:
      RemoveCurve(glui_plot2dinfo, -1);
      Glui2Plot2D(iplot2dinfo);
      EnableDisablePlot2D();
      SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      break;
    case GENPLOT_SHOW_PLOT:
      Glui2Plot2D(iplot2dinfo);
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      update_times = 1;
      break;
    case GENPLOT_SHOW_PLOTS:
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      update_times = 1;
      break;
    case GENPLOT_USE_FOREGROUND_COLOR:
      if(glui_curve.use_foreground_color==1){
        glui_curve.color[0] = foregroundcolor[0]*255;
        glui_curve.color[1] = foregroundcolor[1]*255;
        glui_curve.color[2] = foregroundcolor[2]*255;
        SPINNER_genplot_red->set_int_val(glui_curve.color[0]);
        SPINNER_genplot_green->set_int_val(glui_curve.color[1]);
        SPINNER_genplot_blue->set_int_val(glui_curve.color[2]);
        GLUIGenPlotCB(GENPLOT_XYZ);
      }
      break;
    case GENPLOT_XYZ:
      index = glui_plot2dinfo->curve_index;
      curve = glui_plot2dinfo->curve+index;
      memcpy(curve, &glui_curve, sizeof(curvedata));
      Glui2Plot2D(iplot2dinfo);
      GLUIDeviceCB(DEVICE_TIMEAVERAGE);
#define COLOR_EPS 5
      glui_curve.use_foreground_color = 0;
      if(
        ABS(glui_curve.color[0]-foregroundcolor[0]*255)<=COLOR_EPS&&
        ABS(glui_curve.color[1]-foregroundcolor[1]*255)<=COLOR_EPS&&
        ABS(glui_curve.color[2]-foregroundcolor[2]*255)<=COLOR_EPS
        ){
        glui_curve.use_foreground_color = 1;
        break;
      }
      if(
        ABS(glui_curve.color[0]-backgroundcolor[0]*255)<=COLOR_EPS&&
        ABS(glui_curve.color[1]-backgroundcolor[1]*255)<=COLOR_EPS&&
        ABS(glui_curve.color[2]-backgroundcolor[2]*255)<=COLOR_EPS
        ){
        glui_curve.use_foreground_color = 1;
      }
      break;
    case GENPLOT_PLOT_SIZE:
      if(plot2d_size_factor<0.0){
        plot2d_size_factor = 0.0;
        GLUIUpdatePlot2DSize();
      }
      break;
    case GENPLOT_PLOT_LABEL:
      UpdatePlotList();
      Glui2Plot2D(iplot2dinfo);
      {
        int iplot2dinfo_save = iplot2dinfo;
        LIST_plots->set_int_val(-1);
        LIST_plots->set_int_val(iplot2dinfo_save);
      }
      GLUIDeviceCB(DEVICE_TIMEAVERAGE);
      SetPlot2DShowLabel();
      break;
    case GENPLOT_SET_PLOTPOS:
      if(glui_plot2dinfo->curve_index<global_scase.devicecoll.ndeviceinfo){
        float *plot_xyz;

        plot_xyz = global_scase.devicecoll.deviceinfo[glui_plot2dinfo->curve_index].xyz;
        memcpy(glui_plot2dinfo->xyz, plot_xyz, 3 * sizeof(float));
        SPINNER_genplot_x->set_float_val(plot_xyz[0]);
        SPINNER_genplot_y->set_float_val(plot_xyz[1]);
        SPINNER_genplot_z->set_float_val(plot_xyz[2]);
        Glui2Plot2D(iplot2dinfo);
      }
      break;
    case GENPLOT_PLOT_DIST:
      iplot2dinfo = 0;
      LIST_plots->set_int_val(iplot2dinfo);
      memcpy(plot2d_xyzstart, glui_plot2dinfo->xyz, 3*sizeof(float));
      GLUIGenPlotCB(GENPLOT_SELECT_PLOT);
      if(nplot2dinfo==1){
        memcpy(plot2dinfo[0].xyz, plot2d_xyzstart, 3*sizeof(float));
      }
      else{
        for(i = 0; i<nplot2dinfo; i++){
          plot2ddata *plot2di;
          float dxyz[3], xyzval[3];
          int j;

          plot2di = plot2dinfo+i;
          for(j=0;j<3;j++){

            dxyz[j] = (plot2d_xyzend[j]-plot2d_xyzstart[j])/(float)(nplot2dinfo-1);
            xyzval[j] = plot2d_xyzstart[j]+(float)i*dxyz[j];
          }
          memcpy(plot2di->xyz, xyzval, 3*sizeof(float));
        }
        for(i = 0; i<nplot2dinfo; i++){
          plot2ddata *plot2di;

          plot2di = plot2dinfo+i;
          if(plot2di->plot_index==glui_plot2dinfo->plot_index){
            memcpy(glui_plot2dinfo->xyz, plot2di->xyz, 3*sizeof(float));
            SPINNER_genplot_x->set_float_val(glui_plot2dinfo->xyz[0]);
            SPINNER_genplot_y->set_float_val(glui_plot2dinfo->xyz[1]);
            SPINNER_genplot_z->set_float_val(glui_plot2dinfo->xyz[2]);
          }
        }
      }
      break;
    case GENPLOT_SELECT_PLOT:
      if(iplot2dinfo >= 0&&iplot2dinfo<nplot2dinfo){
        plot2ddata *plot2di;

        plot2di = plot2dinfo+iplot2dinfo;
        Plot2D2Glui(iplot2dinfo);
        strcpy(label, "Remove plot: ");
        strcat(label, plot2di->plot_label);
        BUTTON_rem_plot->set_name(label);
        strcpy(label, "curve properties(");
        strcat(label, plot2di->plot_label);
        strcat(label, ")");
        ROLLOUT_curve_properties->set_name(label);
        GetCsvCurve(0, NULL);
        strcpy(label, "add");
        if(plot2dinfo != NULL){
          strcat(label, " curves to ");
          strcat(label, plot2di->plot_label);
        }
        else{
          strcat(label, " curves");
        }
        PANEL_add_curve->set_name(label);
        SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
        SetPlot2DShowLabel();
      }
      break;
    case GENPLOT_REM_DEV_PLOTS:
      int stop;

      for(;;){

        stop = 0;
        for(i = 0; i<nplot2dinfo; i++){
          plot2ddata *plot2di;
          curvedata *curvei;

          plot2di = plot2dinfo+i;
          if(plot2di->ncurves!=1)continue;
          curvei = plot2di->curve;
          if(strcmp(curvei->c_type,"devc")==0&&plot2di->mult_devc==1){
            iplot2dinfo = i;
            GLUIGenPlotCB(GENPLOT_REM_PLOT);
            stop = 1;
            break;
          }
        }
        if(stop==0)break;
      }
      UpdatePlotDevList();
      break;
    case GENPLOT_RESET_DEV_PLOTS:
      char *dev_pos;

      dev_pos = global_scase.devicecoll.deviceinfo[idevice_add].quantity;
      for(i = 0; i<nplot2dinfo; i++){
        plot2ddata *plot2di;
        curvedata *curvei;

        plot2di = plot2dinfo+i;
        if(plot2di->ncurves!=1)continue;
        curvei = plot2di->curve;
        if(strcmp(curvei->c_type,"devc")==0&&curvei->quantity!=NULL&&strcmp(curvei->quantity, dev_pos)==0){
          iplot2dinfo = i;
          GLUIGenPlotCB(GENPLOT_SELECT_PLOT);
          memcpy(plot2di->xyz, plot2di->xyz_orig, 3*sizeof(float));
          SPINNER_genplot_x->set_float_val(plot2di->xyz[0]);
          SPINNER_genplot_y->set_float_val(plot2di->xyz[1]);
          SPINNER_genplot_z->set_float_val(plot2di->xyz[2]);
          GLUIGenPlotCB(GENPLOT_XYZ);
        }
      }
      break;
    case GENPLOT_ADD_DEV_PLOTS:
      char *dev_quant;

      GLUIGenPlotCB(GENPLOT_REM_DEV_PLOTS);
      dev_quant = global_scase.devicecoll.deviceinfo[idevice_add].quantity;
      glui_csv_file_index = 0;
      LIST_csvfile->set_int_val(glui_csv_file_index);
      GLUIGenPlotCB(GENPLOT_CSV_FILETYPE);
      for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
        devicedata *devi;

        devi = global_scase.devicecoll.deviceinfo + i;
        if(strcmp(devi->quantity, dev_quant)!=0)continue;
        GLUIGenPlotCB(GENPLOT_ADD_PLOT);

        icsv_cols = devi - global_scase.devicecoll.deviceinfo +1;
        LIST_csvID->set_int_val(icsv_cols);
        GLUIGenPlotCB(GENPLOT_ADD_CURVE);

        SPINNER_genplot_x->set_float_val(devi->xyz[0]);
        SPINNER_genplot_y->set_float_val(devi->xyz[1]);
        SPINNER_genplot_z->set_float_val(devi->xyz[2]);
        GLUIGenPlotCB(GENPLOT_XYZ);
        glui_plot2dinfo->mult_devc = 1;
        memcpy(glui_plot2dinfo->xyz_orig,        devi->xyz, 3*sizeof(float));
        memcpy(plot2dinfo[iplot2dinfo].xyz_orig, devi->xyz, 3*sizeof(float));
        CHECKBOX_show_genplot->set_int_val(1);
        GLUIGenPlotCB(GENPLOT_SHOW_PLOT);
      }
      UpdatePlotDevList();
      LIST_plot_add_dev->set_int_val(idevice_add);
      break;
    case GENPLOT_SELECT_DEV_PLOTS:
      break;
    case GENPLOT_ADD_PLOT:
      AddPlot();
      {
        int iplot2dinfo_save = iplot2dinfo;
        LIST_plots->set_int_val(-1);
        LIST_plots->set_int_val(iplot2dinfo_save);
      }
      strcpy(label, "Remove plot: ");
      strcat(label, plot2dinfo[iplot2dinfo].plot_label);
      BUTTON_rem_plot->set_name(label);
      EnableDisablePlot2D();
      GLUIGenPlotCB(GENPLOT_SELECT_PLOT);
      SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      LIST_csvID->set_int_val(-1);
      if(nplot2dinfo == 1){
        glui_csv_file_index = 0;
        LIST_csvfile->set_int_val(0);
        GLUIGenPlotCB(GENPLOT_CSV_FILETYPE);
      }
      break;
    case GENPLOT_REM_ALL_PLOTS:
      while(nplot2dinfo != 0){
        iplot2dinfo = 0;
        GLUIGenPlotCB(GENPLOT_REM_PLOT);
      }
      break;
    case GENPLOT_REM_PLOT:
      RemovePlot(iplot2dinfo);
      if(iplot2dinfo>=0&&iplot2dinfo<nplot2dinfo){
        int iplot2dinfo_save = iplot2dinfo;
        LIST_plots->set_int_val(-1);
        LIST_plots->set_int_val(iplot2dinfo_save);
        strcpy(label, "Remove plot: ");
        strcat(label, plot2dinfo[iplot2dinfo].plot_label);
        SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      }
      else if(plot2dinfo!=NULL){
        strcpy(label, "Remove plot");
        SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      }
      BUTTON_rem_plot->set_name(label);
      EnableDisablePlot2D();
      GLUIGenPlotCB(GENPLOT_SELECT_PLOT);
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      update_times = 1;
      break;
    case GENPLOT_RESET_FUEL_1P0:
      SPINNER_curve_factor->set_float_val(1.0);
      GLUIGenPlotCB(GENPLOT_CURVE_FACTOR);
      break;
    case GENPLOT_RESET_FUEL_HOC:
      SPINNER_curve_factor->set_float_val(MAX(0.0,global_scase.fuel_hoc_default));
      GLUIGenPlotCB(GENPLOT_CURVE_FACTOR);
      break;
    case GENPLOT_CURVE_FACTOR:
      if(glui_curve.curve_factor<0.0){
        glui_curve.curve_factor = 0.00001;
        SPINNER_curve_factor->set_float_val(glui_curve.curve_factor);
      }
      index = glui_plot2dinfo->curve_index;
      curve = glui_plot2dinfo->curve+index;
      {
        int file_index, curv_index;

        file_index = curve->csv_file_index;
        curv_index = curve->csv_col_index;
        memcpy(curve, &glui_curve, sizeof(curvedata));
        curve->csv_file_index = file_index;
        curve->csv_col_index = curv_index;
      }
      Glui2Plot2D(iplot2dinfo);
      GLUIDeviceCB(DEVICE_TIMEAVERAGE);
      SetPlot2DBoundLabels(plot2dinfo+iplot2dinfo);
      break;
    case GENPLOT_SAVE:
      WriteIni(LOCAL_INI, NULL);
      break;
    case GENPLOT_CLOSE:
      glui_plot2d->hide();
      break;
    case GENPLOT_UPDATE:
      break;
    default:
      assert(FFALSE);
      break;
  }
  ForceIdle();
}

/* ------------------ GLUIDeviceCB ------------------------ */

extern "C" void GLUIDeviceCB(int var){
  int i;

  updatemenu = 1;
  if(var==DEVICE_HRRPUV_PLOT){
    vis_hrr_plot = 1-vis_hrr_plot;
    ShowObjectsMenu(PLOT_HRRPUV);
    return;
  }
  if(var==DEVICE_HRRPUV2_PLOT){
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    update_times = 1;
    update_avg = 1;
  }
  if(var == DEVICE_WINDROSE_UPDATE){
    if(viswindrose==1)DeviceData2WindRose(nr_windrose, ntheta_windrose);
    return;
  }
  if(var==DEVICE_WINDROSE_SETPOS){
    for(i = 0; i<4; i++){
      if(windrose_merge_dxyzt[i]<0.0){
        windrose_merge_dxyzt[i] = 0.0;
        SPINNER_windrose_merge_dxyzt[i]->set_float_val(windrose_merge_dxyzt[i]);
      }
    }
    return;
  }
  if(var == DEVICE_WINDROSE_DT){
    if(windrose_ttype==WINDROSE_USE_NEITHER){
      windrose_ttype=WINDROSE_USE_DT;
      RADIO_windrose_ttype->set_int_val(windrose_ttype);
    }
    if(windrose_merge_type==WINDROSE_POINT&&windrose_merge_dxyzt[3]>0.0){
      windrose_merge_type=WINDROSE_SLIDING;
      RADIO_windrose_merge_type->set_int_val(windrose_merge_type);
    }
    GLUIDeviceCB(DEVICE_WINDROSE_SETPOS);
    return;
  }
  if(var == DEVICE_WINDROSE_DTMINMAX){
    if(windrose_ttype==WINDROSE_USE_NEITHER){
      windrose_ttype=WINDROSE_USE_TMINMAX;
      RADIO_windrose_ttype->set_int_val(windrose_ttype);
    }
    if(windrose_merge_type==WINDROSE_POINT&&(windrose_merge_dxyzt[4]>0.0||windrose_merge_dxyzt[5]>0.0)){
      windrose_merge_type=WINDROSE_SLIDING;
      RADIO_windrose_merge_type->set_int_val(windrose_merge_type);
    }
    GLUIDeviceCB(DEVICE_WINDROSE_SETPOS);
    return;
  }
  if(var == DEVICE_WINDROSE_DXYZ){
    GLUIDeviceCB(DEVICE_WINDROSE_SETPOS);
    return;
  }
  if(var == DEVICE_WINDROSE_SHOW_FIRST){
    if(windrose_first<0){
      windrose_first=0;
      SPINNER_windrose_first->set_int_val(windrose_first);
    }
    UpdateShowWindRoses();
    return;
  }
  if(var == DEVICE_WINDROSE_SHOW_NEXT){
    if(windrose_next<1){
      windrose_next=1;
      SPINNER_windrose_next->set_int_val(windrose_next);
    }
    UpdateShowWindRoses();
    return;
  }
  if(var>=DEVICE_WINDROSE_SHOWHIDEALL&&var<DEVICE_WINDROSE_SHOWHIDEALL+nzwindtreeinfo){
    int iztree, j;
    treedevicedata *treei;

    iztree = var-DEVICE_WINDROSE_SHOWHIDEALL;
    treei = zwindtreeinfo[iztree];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;

      vdevsorti = global_scase.devicecoll.vdevices_sorted+j;
      if(vdevsorti->dir==ZDIR){
        vdevicedata *vd;

        vd = vdevsorti->vdeviceinfo;
        //if(vd->unique==0)continue;
        vd->display = 1;
      }
    }
    GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_CHECKBOX);
    return;
  }
  if(var>=DEVICE_WINDROSE_SHOWHIDEALL+nzwindtreeinfo&&var<DEVICE_WINDROSE_SHOWHIDEALL+2*nzwindtreeinfo){
    int iztree, j;
    treedevicedata *treei;

    iztree = var-DEVICE_WINDROSE_SHOWHIDEALL - nzwindtreeinfo;
    treei = zwindtreeinfo[iztree];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;

      vdevsorti = global_scase.devicecoll.vdevices_sorted+j;
      if(vdevsorti->dir==ZDIR){
        vdevicedata *vd;

        vd = vdevsorti->vdeviceinfo;
        //if(vd->unique==0)continue;
        vd->display = 0;
      }
    }
    GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_CHECKBOX);
    return;
  }
  switch(var){
  case DEVICE_SHOWBEAM:
    updatemenu = 1;
    break;
  case DEVICE_RADIUS:
    if(radius_windrose<0.0){
      SPINNER_radius_windrose->set_float_val(0.0);
    }
    break;
  case DEVICE_NBUCKETS:
    if(viswindrose==1)DeviceData2WindRose(nr_windrose, ntheta_windrose);
    break;
  case DEVICE_show_orientation:
    updatemenu = 1;
    break;
  case DEVICE_TIMEAVERAGE:
    for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo+i;
      devicei->update_avg = 1;
    }
    for(i = 0; i<nplot2dinfo; i++){
      plot2ddata *plot2di;
      int j;

      plot2di = plot2dinfo+i;
      for(j = 0; j<plot2di->ncurves; j++){
        curvedata *curvej;

        curvej = plot2di->curve + j;
        curvej->update_avg = 1;
      }
    }
    update_avg = 1;
    GLUIUpdatePlot2DSize2();
    break;
  case DEVICE_devicetypes:
    for(i = 0;i < ndevicetypes;i++){
      devicetypes[i]->type2vis = 0;
    }
    if(ndevicetypes > 0){
      devicetypes[devicetypes_index]->type2vis = 1;
      UpdateColorDevices();
    }
    break;
  case DEVICE_SHOWDEVICEPLOT:
    {
      int vis_device_plot_temp;

      vis_device_plot_temp = vis_device_plot;
      switch(vis_device_plot_temp){
        case DEVICE_PLOT_HIDDEN:
          vis_device_plot = DEVICE_PLOT_SHOW_ALL;
          ShowObjectsMenu(OBJECT_PLOT_SHOW_ALL);
          break;
        case DEVICE_PLOT_SHOW_SELECTED:
          vis_device_plot = DEVICE_PLOT_HIDDEN;
          ShowObjectsMenu(OBJECT_PLOT_SHOW_SELECTED);
          break;
        case DEVICE_PLOT_SHOW_ALL:
          vis_device_plot = DEVICE_PLOT_HIDDEN;
          ShowObjectsMenu(OBJECT_PLOT_SHOW_ALL);
          break;
        case DEVICE_PLOT_SHOW_TREE_ALL:
          vis_device_plot = DEVICE_PLOT_HIDDEN;
          ShowObjectsMenu(OBJECT_PLOT_SHOW_TREE_ALL);
          break;
        default:
          assert(FFALSE);
          break;
      }
    }
    break;
  case DEVICE_SHOWDEVICEVALS:
  case DEVICE_COLORDEVICEVALS:
    update_times=1;
    if(showdevice_val==1){
      vis_device_plot = 1;
    }
    else{
      vis_device_plot = 0;
    }
    GLUIDeviceCB(DEVICE_SHOWDEVICEPLOT);
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    updatemenu=1;
    break;
  case DEVICE_sensorsize:
    if(sensorrelsize < sensorrelsizeMIN){
      sensorrelsize = sensorrelsizeMIN;
      if(SPINNER_sensorrelsize != NULL){
        SPINNER_sensorrelsize->set_float_val(sensorrelsize);
      }
    }
    break;
  case DEVICE_SAVE_SETTINGS_OBJECTS:
    WriteIni(LOCAL_INI, NULL);
    break;
  case DEVICE_close:
    GLUIHideDevice();
    break;
  default:
    assert(FFALSE);
  }
}

/* ------------------ GLUIUpdateDevices ------------------------ */

extern "C" void GLUIUpdateDevices(void){
  GLUIDeviceCB(DEVICE_SHOWDEVICEPLOT);
  GLUIDeviceCB(DEVICE_SHOWDEVICEVALS);
  GLUIDeviceCB(DEVICE_COLORDEVICEVALS);
  GLUIDeviceCB(DEVICE_devicetypes);
}

/* ------------------ GetDeviceTminTmax ------------------------ */

float GetDeviceTminTmax(void){
  float return_val=1.0;
  int first = 1, i;

  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devicei;
    float *times;

    devicei = global_scase.devicecoll.deviceinfo+i;
    times = devicei->times;
    if(times!=NULL&&devicei->nvals>0){
      float tval;

      tval = times[devicei->nvals-1];
      if(first==1){
        first = 0;
        return_val = tval;
      }
      else{
        return_val = MAX(return_val, tval);
      }
    }
  }
  return return_val;
}

/* ------------------ HaveExt ------------------------ */

int HaveExt(void){
  int i;

  for(i = 0; i<global_scase.csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = global_scase.csvcoll.csvfileinfo+i;
    if(strcmp(csvfi->c_type, "ext")==0)return 1;
  }
  return 0;
}

/* ------------------ GLUIUpdatePlot2DINI ------------------------ */

extern "C" void GLUIUpdatePlot2DINI(void){
  if(nplot2dini > 0){
    int i;

    nplot2dinfo = nplot2dini;
    FREEMEMORY(plot2dinfo);
    NewMemory((void **)&plot2dinfo, nplot2dinfo * sizeof(plot2ddata));
    memcpy(plot2dinfo, plot2dini, nplot2dinfo * sizeof(plot2ddata));
    for(i = 0; i < nplot2dini; i++){
      plot2ddata *plot2di;

      plot2di = plot2dinfo + i;
      LIST_plots->add_item(i, plot2di->plot_label);
    }
    LIST_plots->set_int_val(0);
    GLUIGenPlotCB(GENPLOT_SELECT_PLOT);
  }
}

/* ------------------ UpdateCSVFileTypes ------------------------ */

void UpdateCSVFileTypes(void){
  int i;

  for(i = 0; i<global_scase.csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = global_scase.csvcoll.csvfileinfo+i;
    if(strcmp(csvfi->c_type, "ext")!=0 && csvfi->glui_defined==0 && csvfi->defined == CSV_DEFINED && LIST_csvfile != NULL){
      csvfi->glui_defined = CSV_DEFINED;
      LIST_csvfile->add_item(i, csvfi->c_type);
    }
  }
}

/* ------------------ GLUIUpdatePlot2DTbounds ------------------------ */

extern "C" void GLUIUpdatePlot2DTbounds(void){
  use_tload_end2 = use_tload_end;
  use_tload_begin2 = use_tload_begin;
  tload_end2 = global_scase.tload_end;
  tload_begin2 = global_scase.tload_begin;
  if(CHECKBOX_genplot_use_valmax[2]!=NULL)CHECKBOX_genplot_use_valmax[2]->set_int_val(use_tload_end2);
  if(CHECKBOX_genplot_use_valmin[2]!=NULL)CHECKBOX_genplot_use_valmin[2]->set_int_val(use_tload_begin2);
  if(SPINNER_genplot_valmax[2]!=NULL)SPINNER_genplot_valmax[2]->set_float_val(tload_end2);
  if(SPINNER_genplot_valmin[2]!=NULL)SPINNER_genplot_valmin[2]->set_float_val(tload_begin2);
}

/* ------------------ GLUIPlot2DSetup ------------------------ */

extern "C" void GLUIPlot2DSetup(int main_window){
  if(glui_plot2d!=NULL){
    glui_plot2d->close();
    glui_plot2d = NULL;
  }

  have_ext = HaveExt();
  if((global_scase.csvcoll.ncsvfileinfo>0&&have_ext==0)||(global_scase.csvcoll.ncsvfileinfo>1&&have_ext==1)){
    int i;

    glui_plot2d = GLUI_Master.create_glui("2D plots", 0, dialogX0, dialogY0);
    glui_plot2d->hide();

    PANEL_genplot = glui_plot2d->add_panel("", 0);
    PANEL_newplot = glui_plot2d->add_panel_to_panel(PANEL_genplot,"", 0);

    PANEL_plots = glui_plot2d->add_panel_to_panel(PANEL_newplot, "add/remove/select plot");
    BUTTON_add_plot = glui_plot2d->add_button_to_panel(PANEL_plots, _("New plot"), GENPLOT_ADD_PLOT, GLUIGenPlotCB);

    BUTTON_rem_plot = glui_plot2d->add_button_to_panel(PANEL_plots, _("Remove"), GENPLOT_REM_PLOT, GLUIGenPlotCB);
    BUTTON_rem_all_plots = glui_plot2d->add_button_to_panel(PANEL_plots, _("Remove all"), GENPLOT_REM_ALL_PLOTS, GLUIGenPlotCB);
    LIST_plots = glui_plot2d->add_listbox_to_panel(PANEL_plots, "select:", &iplot2dinfo, GENPLOT_SELECT_PLOT, GLUIGenPlotCB);
    LIST_plots->add_item(-1, "");
    CHECKBOX_show_genplot  = glui_plot2d->add_checkbox_to_panel(PANEL_plots, "show", &(glui_plot2dinfo->show), GENPLOT_SHOW_PLOT, GLUIGenPlotCB);
    glui_plot2d->add_checkbox_to_panel(PANEL_plots, "show plots", &plot2d_show_plots, GENPLOT_SHOW_PLOTS, GLUIGenPlotCB);

    if(global_scase.devicecoll.ndeviceinfo>0){
      ROLLOUT_devplots = glui_plot2d->add_rollout_to_panel(PANEL_plots, "add plots at device locations", 0);
      for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
        devicedata *devi;

        devi = global_scase.devicecoll.deviceinfo+i;
        devi->inlist = 0;
      }
      for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
        devicedata *devi;

        devi = global_scase.devicecoll.deviceinfo+i;
        devi->inlist = 1-InDevList(devi, i);
      }
      LIST_plot_add_dev = glui_plot2d->add_listbox_to_panel(ROLLOUT_devplots,    "Add:",    &idevice_add,  GENPLOT_ADD_DEV_PLOTS,  GLUIGenPlotCB);
      glui_plot2d->add_button_to_panel(ROLLOUT_devplots, _("Remove"), GENPLOT_REM_DEV_PLOTS, GLUIGenPlotCB);
      glui_plot2d->add_button_to_panel(ROLLOUT_devplots, _("Reset Positions"), GENPLOT_RESET_DEV_PLOTS, GLUIGenPlotCB);
      GLUIGenPlotCB(GENPLOT_SELECT_DEV_PLOTS);
      UpdatePlotDevList();
      LIST_plot_add_dev->set_int_val(-1);
    }
    glui_plot2d->add_column_to_panel(PANEL_newplot, false);

    PANEL_add_curve = glui_plot2d->add_panel_to_panel(PANEL_newplot, "");
    PANEL_add_curve1 = PANEL_add_curve;
    PANEL_csv1 = glui_plot2d->add_panel_to_panel(PANEL_add_curve1, "", 0);
    LIST_csvfile = glui_plot2d->add_listbox_to_panel(PANEL_csv1, "csv file type:", &glui_csv_file_index, GENPLOT_CSV_FILETYPE, GLUIGenPlotCB);
    UpdateCSVFileTypes();
    LIST_csvID = glui_plot2d->add_listbox_to_panel(PANEL_add_curve1,      "curves:", &icsv_cols,  GENPLOT_ADD_CURVE,  GLUIGenPlotCB);
    LIST_csvID->add_item(-1, "");

    LIST_curve_unit = glui_plot2d->add_listbox_to_panel(PANEL_add_curve1, "unit:",    &icsv_units, GENPLOT_CURVE_UNIT, GLUIGenPlotCB);
    LIST_curve_unit->add_item(-1, "any");

    if(global_scase.isZoneFireModel==1){
      LIST_curve_compartments = glui_plot2d->add_listbox_to_panel(PANEL_add_curve1, "compartment:", &icsv_compartments, GENPLOT_CURVE_UNIT, GLUIGenPlotCB);
      for(i = 0;i < global_scase.nrooms;i++){
        char label[100];

        snprintf(label, sizeof(label), "%i", i + 1);
        LIST_curve_compartments->add_item(i, label);
      }
      LIST_curve_compartments->add_item(-1, "any");
      LIST_curve_compartments->set_int_val(-1);
    }

    GLUIGenPlotCB(GENPLOT_CSV_FILETYPE);
    GLUIGenPlotCB(GENPLOT_CURVE_UNIT);


    ROLLOUT_plotproperties = glui_plot2d->add_rollout("plot properties(all plots)", true, PLOT2D_NEW_ROLLOUT, Plot2D_Rollout_CB);
    TOGGLE_ROLLOUT(plot2dprocinfo, nplot2dprocinfo, ROLLOUT_plotproperties, PLOT2D_NEW_ROLLOUT, glui_plot2d);

    PANEL_plotproperties1 = glui_plot2d->add_panel_to_panel(ROLLOUT_plotproperties, "", GLUI_PANEL_NONE);
    PANEL_plot_position = glui_plot2d->add_panel_to_panel(PANEL_plotproperties1, "position");
    SPINNER_genplot_x = glui_plot2d->add_spinner_to_panel(PANEL_plot_position, "x0", GLUI_SPINNER_FLOAT, glui_plot2dinfo->xyz+0, GENPLOT_XYZ, GLUIGenPlotCB);
    SPINNER_genplot_y = glui_plot2d->add_spinner_to_panel(PANEL_plot_position, "y0", GLUI_SPINNER_FLOAT, glui_plot2dinfo->xyz+1, GENPLOT_XYZ, GLUIGenPlotCB);
    SPINNER_genplot_z = glui_plot2d->add_spinner_to_panel(PANEL_plot_position, "z0", GLUI_SPINNER_FLOAT, glui_plot2dinfo->xyz+2, GENPLOT_XYZ, GLUIGenPlotCB);
   // BUTTON_plot_position = glui_plot2d->add_button_to_panel(PANEL_plot_position, _("Set to device location"), GENPLOT_SET_PLOTPOS, GLUIGenPlotCB);
    plot2d_xyzstart[0] = xbar0FDS;
    plot2d_xyzend[0]   = xbar0FDS;
    plot2d_xyzstart[1] = ybar0FDS;
    plot2d_xyzend[1]   = ybar0FDS;
    plot2d_xyzstart[2] = zbar0FDS;
    plot2d_xyzend[2]   = zbarFDS-1.3*SCALE2FDS(plot2d_size_factor);
    ROLLOUT_positions = glui_plot2d->add_rollout_to_panel(PANEL_plot_position, "distribute positions", 0);
    glui_plot2d->add_spinner_to_panel(ROLLOUT_positions, "x1", GLUI_SPINNER_FLOAT, plot2d_xyzend);
    glui_plot2d->add_spinner_to_panel(ROLLOUT_positions, "y1", GLUI_SPINNER_FLOAT, plot2d_xyzend+1);
    glui_plot2d->add_spinner_to_panel(ROLLOUT_positions, "z1", GLUI_SPINNER_FLOAT, plot2d_xyzend+2);
    glui_plot2d->add_button_to_panel(ROLLOUT_positions, _("Apply x0->x1, y0->y1, z0->z1"), GENPLOT_PLOT_DIST, GLUIGenPlotCB);

    glui_plot2d->add_column_to_panel(PANEL_plotproperties1, false);

    PANEL_plotother = glui_plot2d->add_panel_to_panel(PANEL_plotproperties1, "misc");
    glui_plot2d->add_spinner_to_panel(PANEL_plotother, _("frame line width"), GLUI_SPINNER_FLOAT, &plot2d_frame_width,                       GENPLOT_UPDATE,    GLUIGenPlotCB);
    SPINNER_size_factor = glui_plot2d->add_spinner_to_panel(PANEL_plotother, _("plot size(relative)"), GLUI_SPINNER_FLOAT, &plot2d_size_factor,   GENPLOT_PLOT_SIZE, GLUIGenPlotCB);
    glui_plot2d->add_spinner_to_panel(PANEL_plotother, _("vertical font spacing"), GLUI_SPINNER_FLOAT, &plot2d_font_spacing,                 GENPLOT_UPDATE,    GLUIGenPlotCB);
    SPINNER_plot2d_time_average = glui_plot2d->add_spinner_to_panel(PANEL_plotother, _("smoothing interval (s)"), GLUI_SPINNER_FLOAT, &plot2d_time_average, DEVICE_TIMEAVERAGE, GLUIDeviceCB);

    PANEL_plotproperties2 = glui_plot2d->add_panel_to_panel(ROLLOUT_plotproperties, "", GLUI_PANEL_NONE);

    PANEL_plottitle = glui_plot2d->add_panel_to_panel(PANEL_plotproperties2, "title");
    CHECKBOX_show_plot_title = glui_plot2d->add_checkbox_to_panel(PANEL_plottitle, "show", &plot2d_show_plot_title, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    EDIT_plot_label = glui_plot2d->add_edittext_to_panel(PANEL_plottitle, "edit:", GLUI_EDITTEXT_TEXT, glui_plot2dinfo->plot_label, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    glui_plot2d->add_button_to_panel(PANEL_plottitle, _("Apply"), GENPLOT_PLOT_LABEL, GLUIGenPlotCB);

    glui_plot2d->add_column_to_panel(PANEL_plotproperties2, false);
    strcpy(plot2d_xaxis_label, "time");
    PANEL_plot_xlabels            = glui_plot2d->add_panel_to_panel(PANEL_plotproperties2, "x axis labels");
    CHECKBOX_show_xaxis_bounds    = glui_plot2d->add_checkbox_to_panel(PANEL_plot_xlabels, "bounds",
                                    &plot2d_show_xaxis_bounds, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    CHECKBOX_show_xaxis_labels    = glui_plot2d->add_checkbox_to_panel(PANEL_plot_xlabels, "show label",
                                    &plot2d_show_xaxis_labels, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    EDIT_xaxis_label              = glui_plot2d->add_edittext_to_panel(PANEL_plot_xlabels, "label:", GLUI_EDITTEXT_TEXT,
                                    plot2d_xaxis_label,       GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    SPINNER_plot2d_xaxis_position = glui_plot2d->add_spinner_to_panel(PANEL_plot_xlabels,  "position", GLUI_SPINNER_FLOAT,
                                    &plot2d_xaxis_position,    GENPLOT_PLOT_LABEL, GLUIGenPlotCB);

    glui_plot2d->add_column_to_panel(PANEL_plotproperties2, false);
    PANEL_plot_ylabels = glui_plot2d->add_panel_to_panel(PANEL_plotproperties2, "y axis labels");
    PANEL_plot_labels2 = glui_plot2d->add_panel_to_panel(PANEL_plot_ylabels, "", 0);

    CHECKBOX_show_yaxis_bounds = glui_plot2d->add_checkbox_to_panel(PANEL_plot_labels2, "bounds",        &plot2d_show_yaxis_bounds, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    CHECKBOX_show_yaxis_units  = glui_plot2d->add_checkbox_to_panel(PANEL_plot_labels2, "units",         &plot2d_show_yaxis_units,  GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    CHECKBOX_show_curve_labels = glui_plot2d->add_checkbox_to_panel(PANEL_plot_labels2, "quantity",      &plot2d_show_curve_labels, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);
    CHECKBOX_show_curve_values = glui_plot2d->add_checkbox_to_panel(PANEL_plot_labels2, "values",        &plot2d_show_curve_values, GENPLOT_PLOT_LABEL, GLUIGenPlotCB);

    memcpy(&glui_curve, &glui_curve_default, sizeof(curvedata));

    ROLLOUT_curve_properties = glui_plot2d->add_rollout("curve properties", false, PLOT2D_CURVE_ROLLOUT, Plot2D_Rollout_CB);
    TOGGLE_ROLLOUT(plot2dprocinfo, nplot2dprocinfo, ROLLOUT_curve_properties, PLOT2D_CURVE_ROLLOUT, glui_plot2d);

    if(nplot2dinfo>0&&glui_plot2dinfo->ncurves==0)ROLLOUT_curve_properties->disable();
    LIST_plotcurves = glui_plot2d->add_listbox_to_panel(ROLLOUT_curve_properties, "select:", &glui_plot2dinfo->curve_index, GENPLOT_SELECT_CURVE, GLUIGenPlotCB);
    RemoveCurve(glui_plot2dinfo, -1);
    MakeCurveList(glui_plot2dinfo, 1);

    glui_plot2d->add_button_to_panel(ROLLOUT_curve_properties, _("Remove selected curve"), GENPLOT_REM_SELECTEDCURVE, GLUIGenPlotCB);
    glui_plot2d->add_button_to_panel(ROLLOUT_curve_properties, _("Remove all curves"), GENPLOT_REM_ALLCURVES, GLUIGenPlotCB);

    PANEL_curve_factor = glui_plot2d->add_panel_to_panel(ROLLOUT_curve_properties, "scale curve");
    glui_curve.curve_factor       = 1.0;
    glui_curve.apply_curve_factor = 0;
    SPINNER_curve_factor = glui_plot2d->add_spinner_to_panel(PANEL_curve_factor,     "factor", GLUI_SPINNER_FLOAT, &glui_curve.curve_factor, GENPLOT_CURVE_FACTOR, GLUIGenPlotCB);
    CHECKBOX_curve_apply_factor = glui_plot2d->add_checkbox_to_panel(PANEL_curve_factor, "Multiply selected curve by factor", &glui_curve.apply_curve_factor,   GENPLOT_CURVE_FACTOR, GLUIGenPlotCB);

    EDIT_scaled_label = glui_plot2d->add_edittext_to_panel(PANEL_curve_factor, "scaled label:", GLUI_EDITTEXT_TEXT, glui_curve.scaled_label, GENPLOT_CURVE_FACTOR, GLUIGenPlotCB);
    EDIT_scaled_unit  = glui_plot2d->add_edittext_to_panel(PANEL_curve_factor, "scaled unit:",  GLUI_EDITTEXT_TEXT, glui_curve.scaled_unit,  GENPLOT_CURVE_FACTOR, GLUIGenPlotCB);

    glui_plot2d->add_button_to_panel(PANEL_curve_factor, "Reset factor(HOC)",   GENPLOT_RESET_FUEL_HOC, GLUIGenPlotCB);
    glui_plot2d->add_button_to_panel(PANEL_curve_factor, "Reset factor(1.0)",   GENPLOT_RESET_FUEL_1P0, GLUIGenPlotCB);

    glui_plot2d->add_column_to_panel(ROLLOUT_curve_properties, false);

    PANEL_curve_color     = glui_plot2d->add_panel_to_panel(ROLLOUT_curve_properties, "color");
    SPINNER_genplot_red   = glui_plot2d->add_spinner_to_panel(PANEL_curve_color, "red",   GLUI_SPINNER_INT, glui_curve.color+0, GENPLOT_XYZ, GLUIGenPlotCB);
    SPINNER_genplot_green = glui_plot2d->add_spinner_to_panel(PANEL_curve_color, "green", GLUI_SPINNER_INT, glui_curve.color+1, GENPLOT_XYZ, GLUIGenPlotCB);
    SPINNER_genplot_blue  = glui_plot2d->add_spinner_to_panel(PANEL_curve_color, "blue",  GLUI_SPINNER_INT, glui_curve.color+2, GENPLOT_XYZ, GLUIGenPlotCB);
    SPINNER_genplot_red->set_int_limits(0, 255);
    SPINNER_genplot_green->set_int_limits(0, 255);
    SPINNER_genplot_blue->set_int_limits(0, 255);
    glui_plot2d->add_button_to_panel(PANEL_curve_color, "Apply colors", GENPLOT_XYZ, GLUIGenPlotCB);

    SPINNER_genplot_linewidth = glui_plot2d->add_spinner_to_panel(ROLLOUT_curve_properties, "line width", GLUI_SPINNER_FLOAT, &(glui_curve.linewidth), GENPLOT_XYZ, GLUIGenPlotCB);
    SPINNER_genplot_linewidth->set_float_limits(1.0, 10.0);

    ROLLOUT_plot_bounds = glui_plot2d->add_rollout("plot bounds(all plots)", false, PLOT2D_BOUNDS_ROLLOUT, Plot2D_Rollout_CB);
    TOGGLE_ROLLOUT(plot2dprocinfo, nplot2dprocinfo, ROLLOUT_plot_bounds, PLOT2D_BOUNDS_ROLLOUT, glui_plot2d);

    PANEL_plot_bounds2 = glui_plot2d->add_panel_to_panel(ROLLOUT_plot_bounds, "", 0);

    PANEL_bound3 = glui_plot2d->add_panel_to_panel(PANEL_plot_bounds2, "time");

    CHECKBOX_genplot_use_valmax[2] = glui_plot2d->add_checkbox_to_panel(PANEL_bound3, "use max",                 &use_tload_end2,   GENPLOT_PLOT_TMINMAX, GLUIGenPlotCB);
    SPINNER_genplot_valmax[2]      = glui_plot2d->add_spinner_to_panel(PANEL_bound3,  "max", GLUI_SPINNER_FLOAT, &tload_end2,       GENPLOT_PLOT_TMINMAX, GLUIGenPlotCB);
    CHECKBOX_genplot_use_valmin[2] = glui_plot2d->add_checkbox_to_panel(PANEL_bound3, "use min",                 &use_tload_begin2, GENPLOT_PLOT_TMINMAX, GLUIGenPlotCB);
    SPINNER_genplot_valmin[2]      = glui_plot2d->add_spinner_to_panel(PANEL_bound3,  "min", GLUI_SPINNER_FLOAT, &tload_begin2,     GENPLOT_PLOT_TMINMAX, GLUIGenPlotCB);

    glui_plot2d->add_column_to_panel(PANEL_plot_bounds2, false);

    PANEL_bound1 = glui_plot2d->add_panel_to_panel(PANEL_plot_bounds2, "bound1");

    CHECKBOX_genplot_use_valmax[0] = glui_plot2d->add_checkbox_to_panel(PANEL_bound1, "use max",                 glui_plot2dinfo->use_valmax, GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);
    SPINNER_genplot_valmax[0]      = glui_plot2d->add_spinner_to_panel(PANEL_bound1,  "max", GLUI_SPINNER_FLOAT, glui_plot2dinfo->valmax,     GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);
    CHECKBOX_genplot_use_valmin[0] = glui_plot2d->add_checkbox_to_panel(PANEL_bound1, "use min",                 glui_plot2dinfo->use_valmin, GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);
    SPINNER_genplot_valmin[0]      = glui_plot2d->add_spinner_to_panel(PANEL_bound1,  "min", GLUI_SPINNER_FLOAT, glui_plot2dinfo->valmin,     GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);

    glui_plot2d->add_column_to_panel(PANEL_plot_bounds2, false);

    PANEL_bound2 = glui_plot2d->add_panel_to_panel(PANEL_plot_bounds2, "bound2");

    CHECKBOX_genplot_use_valmax[1] = glui_plot2d->add_checkbox_to_panel(PANEL_bound2, "use max",                 glui_plot2dinfo->use_valmax+1, GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);
    SPINNER_genplot_valmax[1]      = glui_plot2d->add_spinner_to_panel(PANEL_bound2,  "max", GLUI_SPINNER_FLOAT, glui_plot2dinfo->valmax+1,     GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);
    CHECKBOX_genplot_use_valmin[1] = glui_plot2d->add_checkbox_to_panel(PANEL_bound2, "use min",                 glui_plot2dinfo->use_valmin+1, GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);
    SPINNER_genplot_valmin[1]      = glui_plot2d->add_spinner_to_panel(PANEL_bound2,  "min", GLUI_SPINNER_FLOAT, glui_plot2dinfo->valmin+1,     GENPLOT_PLOT_MINMAX, GLUIGenPlotCB);

    BUTTON_reset_plot2d_bounds = glui_plot2d->add_button_to_panel(ROLLOUT_plot_bounds, "Reset bounds", GENPLOT_RESET_BOUNDS, GLUIGenPlotCB);

    SetPlot2DBoundLabels(plot2dinfo);

    if(update_plot2dini == 1){
      update_plot2dini = 0;
      GLUIUpdatePlot2DINI();
    }
    GLUIGenPlotCB(GENPLOT_SHOW_PLOT);
    plot2d_dialogs_defined = 1;
    EnableDisablePlot2D();

    PANEL_plot2d_label3 = glui_plot2d->add_panel("", false);
    glui_plot2d->add_column_to_panel(PANEL_plot2d_label3, false);

    glui_plot2d->add_button_to_panel(PANEL_plot2d_label3, _("Save settings"), GENPLOT_SAVE, GLUIGenPlotCB);
    glui_plot2d->add_column_to_panel(PANEL_plot2d_label3, false);

    BUTTON_plot2d_2 = glui_plot2d->add_button_to_panel(PANEL_plot2d_label3, _("Close"), GENPLOT_CLOSE, GLUIGenPlotCB);
#ifdef pp_CLOSEOFF
    BUTTON_plot2d_2->disable();
#endif
  }
}

/* ------------------ GLUIDeviceSetup ------------------------ */

extern "C" void GLUIDeviceSetup(int main_window){

  if(glui_device!=NULL){
    glui_device->close();
    glui_device=NULL;
  }
  glui_device = GLUI_Master.create_glui("Devices/Objects",0,dialogX0,dialogY0);
  glui_device->hide();

  if(global_scase.devicecoll.ndeviceinfo>0){
    int i;

    PANEL_objects = glui_device->add_panel("Devices/Objects", false);

    ROLLOUT_smvobjects = glui_device->add_rollout_to_panel(PANEL_objects, _("Objects"), false, OBJECTS_ROLLOUT, Device_Rollout_CB);
    TOGGLE_ROLLOUT(deviceprocinfo, ndeviceprocinfo, ROLLOUT_smvobjects, OBJECTS_ROLLOUT, glui_device);

    SPINNER_sensorrelsize = glui_device->add_spinner_to_panel(ROLLOUT_smvobjects, _("Scale"), GLUI_SPINNER_FLOAT, &sensorrelsize, DEVICE_sensorsize, GLUIDeviceCB);
    CHECKBOX_device_3 = glui_device->add_checkbox_to_panel(ROLLOUT_smvobjects, _("Outline"), &object_outlines);
    CHECKBOX_device_orientation = glui_device->add_checkbox_to_panel(ROLLOUT_smvobjects, _("Orientation"), &show_device_orientation, DEVICE_show_orientation, GLUIDeviceCB);
    SPINNER_orientation_scale = glui_device->add_spinner_to_panel(ROLLOUT_smvobjects, _("Orientation scale"), GLUI_SPINNER_FLOAT, &orientation_scale);
    SPINNER_orientation_scale->set_float_limits(0.1, 10.0);
    if(global_scase.have_beam){
      PANEL_beam = glui_device->add_panel_to_panel(ROLLOUT_smvobjects, "Beam sensor", true);
      CHECKBOX_showbeam_as_line = glui_device->add_checkbox_to_panel(PANEL_beam, _("Show beam as line"), &showbeam_as_line, DEVICE_SHOWBEAM, GLUIDeviceCB);
      SPINNER_beam_line_width = glui_device->add_spinner_to_panel(PANEL_beam, _("line width"), GLUI_SPINNER_FLOAT, &beam_line_width);
      SPINNER_beam_line_width->set_float_limits(1.0, 20.0);
      CHECKBOX_use_beamcolor = glui_device->add_checkbox_to_panel(PANEL_beam, _("Use color"), &use_beamcolor);
      SPINNER_beam_color[0] = glui_device->add_spinner_to_panel(PANEL_beam, _("red"), GLUI_SPINNER_INT, beam_color);
      SPINNER_beam_color[1] = glui_device->add_spinner_to_panel(PANEL_beam, _("green"), GLUI_SPINNER_INT, beam_color+1);
      SPINNER_beam_color[2] = glui_device->add_spinner_to_panel(PANEL_beam, _("blue"), GLUI_SPINNER_INT, beam_color+2);
      SPINNER_beam_color[0]->set_int_limits(0, 255);
      SPINNER_beam_color[1]->set_int_limits(0, 255);
      SPINNER_beam_color[2]->set_int_limits(0, 255);
    }

    if(GetNumActiveDevices()>0||global_scase.isZoneFireModel==1){
      ROLLOUT_velocityvectors = glui_device->add_rollout_to_panel(PANEL_objects, "Flow vectors", false, FLOWVECTORS_ROLLOUT, Device_Rollout_CB);
      TOGGLE_ROLLOUT(deviceprocinfo, ndeviceprocinfo, ROLLOUT_velocityvectors, FLOWVECTORS_ROLLOUT, glui_device);

      if(global_scase.devicecoll.nvdeviceinfo==0)ROLLOUT_velocityvectors->disable();
      CHECKBOX_device_1 = glui_device->add_checkbox_to_panel(ROLLOUT_velocityvectors, _("Show"), &showvdevice_val);
      PANEL_vector_type = glui_device->add_panel_to_panel(ROLLOUT_velocityvectors, _("type"), true);
      RADIO_vectortype = glui_device->add_radiogroup_to_panel(PANEL_vector_type, &vectortype);
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("line"));
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("arrow"));
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("object"));
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("profile"));
      ROLLOUT_arrow_dimensions = glui_device->add_rollout_to_panel(ROLLOUT_velocityvectors, _("Dimensions"), false);
      PANEL_arrow_base = glui_device->add_panel_to_panel(ROLLOUT_arrow_dimensions, _("base"), true);
      glui_device->add_spinner_to_panel(PANEL_arrow_base, _("length"), GLUI_SPINNER_FLOAT, &vector_baselength);
      glui_device->add_spinner_to_panel(PANEL_arrow_base, _("diameter"), GLUI_SPINNER_FLOAT, &vector_basediameter);
      PANEL_arrow_height = glui_device->add_panel_to_panel(ROLLOUT_arrow_dimensions, _("head"), true);
      glui_device->add_spinner_to_panel(PANEL_arrow_height, _("length"), GLUI_SPINNER_FLOAT, &vector_headlength);
      glui_device->add_spinner_to_panel(PANEL_arrow_height, _("diameter"), GLUI_SPINNER_FLOAT, &vector_headdiameter);

      if(ntreedeviceinfo>0){
        ROLLOUT_trees = glui_device->add_rollout_to_panel(ROLLOUT_velocityvectors, "Device trees", false);
        SPINNER_mintreesize = glui_device->add_spinner_to_panel(ROLLOUT_trees, _("min size"), GLUI_SPINNER_INT, &mintreesize);
        SPINNER_mintreesize->set_int_limits(2, MAX(2, max_device_tree));
        CHECKBOX_vis_xtree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _A(_("Show"), " x"), &vis_xtree);
        CHECKBOX_vis_ytree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _A(_("Show"), " y"), &vis_ytree);
        CHECKBOX_vis_ztree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _A(_("Show"), " z"), &vis_ztree);
      }

      ROLLOUT_windrose = glui_device->add_rollout_to_panel(PANEL_objects, "Wind rose", false, WINDROSE_ROLLOUT, Device_Rollout_CB);
      TOGGLE_ROLLOUT(deviceprocinfo, ndeviceprocinfo, ROLLOUT_windrose, WINDROSE_ROLLOUT, glui_device);

      glui_device->add_checkbox_to_panel(ROLLOUT_windrose, _("show"), &viswindrose, DEVICE_WINDROSE_UPDATE, GLUIDeviceCB);

      SPINNER_nr_windrose = glui_device->add_spinner_to_panel(ROLLOUT_windrose, _("radii"), GLUI_SPINNER_INT, &nr_windrose, DEVICE_NBUCKETS, GLUIDeviceCB);
      SPINNER_nr_windrose->set_int_limits(3, 72, GLUI_LIMIT_CLAMP);
      SPINNER_ntheta_windrose = glui_device->add_spinner_to_panel(ROLLOUT_windrose, _("angles"), GLUI_SPINNER_INT, &ntheta_windrose, DEVICE_NBUCKETS, GLUIDeviceCB);
      SPINNER_ntheta_windrose->set_int_limits(3, 72, GLUI_LIMIT_CLAMP);
      SPINNER_radius_windrose = glui_device->add_spinner_to_panel(ROLLOUT_windrose, _("radius"), GLUI_SPINNER_FLOAT, &radius_windrose, DEVICE_RADIUS, GLUIDeviceCB);
      RADIO_windstate_windrose = glui_device->add_radiogroup_to_panel(ROLLOUT_windrose, &windstate_windrose);
      glui_device->add_radiobutton_to_group(RADIO_windstate_windrose, "heading");
      glui_device->add_radiobutton_to_group(RADIO_windstate_windrose, "direction");

      ROLLOUT_show_windrose2 = glui_device->add_rollout_to_panel(ROLLOUT_windrose, "merge data", false);
      PANEL_windrose_mergexyz = glui_device->add_panel_to_panel(ROLLOUT_show_windrose2, "space", true);
      SPINNER_windrose_merge_dxyzt[0] = glui_device->add_spinner_to_panel(PANEL_windrose_mergexyz, "dx", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt, DEVICE_WINDROSE_DXYZ, GLUIDeviceCB);
      SPINNER_windrose_merge_dxyzt[1] = glui_device->add_spinner_to_panel(PANEL_windrose_mergexyz, "dy", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+1, DEVICE_WINDROSE_DXYZ, GLUIDeviceCB);
      SPINNER_windrose_merge_dxyzt[2] = glui_device->add_spinner_to_panel(PANEL_windrose_mergexyz, "dz", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+2, DEVICE_WINDROSE_DXYZ, GLUIDeviceCB);

      PANEL_windrose_merget = glui_device->add_panel_to_panel(ROLLOUT_show_windrose2, "time", true);
      SPINNER_windrose_merge_dxyzt[3] = glui_device->add_spinner_to_panel(PANEL_windrose_merget, "dt", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+3, DEVICE_WINDROSE_DT, GLUIDeviceCB);
      SPINNER_windrose_merge_dxyzt[4] = glui_device->add_spinner_to_panel(PANEL_windrose_merget, "tmin", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+4, DEVICE_WINDROSE_DTMINMAX, GLUIDeviceCB);
      SPINNER_windrose_merge_dxyzt[5] = glui_device->add_spinner_to_panel(PANEL_windrose_merget, "tmax", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+5, DEVICE_WINDROSE_DTMINMAX, GLUIDeviceCB);

      PANEL_windrose_merge = glui_device->add_panel_to_panel(PANEL_windrose_merget, "type", true);

      RADIO_windrose_merge_type = glui_device->add_radiogroup_to_panel(PANEL_windrose_merge, &windrose_merge_type, DEVICE_WINDROSE_DT, GLUIDeviceCB);
      glui_device->add_radiobutton_to_group(RADIO_windrose_merge_type, _("point"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_merge_type, _("stepped"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_merge_type, _("sliding"));

      RADIO_windrose_ttype = glui_device->add_radiogroup_to_panel(PANEL_windrose_merget, &windrose_ttype, DEVICE_WINDROSE_SETPOS, GLUIDeviceCB);
      glui_device->add_radiobutton_to_group(RADIO_windrose_ttype, _("use dt"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_ttype, _("use tmin/tmax"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_ttype, _("neither"));

      BUTTON_update_windrose = glui_device->add_button_to_panel(ROLLOUT_show_windrose2, _("Update"), DEVICE_WINDROSE_UPDATE, GLUIDeviceCB);

      if(windrose_xy_active==1||windrose_xz_active==1||windrose_yz_active==1){
        PANEL_orientation = glui_device->add_panel_to_panel(ROLLOUT_windrose, "orientation", true);
        if(windrose_xy_active==1)glui_device->add_checkbox_to_panel(PANEL_orientation, "xy", &windrose_xy_vis);
        if(windrose_xz_active==1)glui_device->add_checkbox_to_panel(PANEL_orientation, "xz", &windrose_xz_vis);
        if(windrose_yz_active==1)glui_device->add_checkbox_to_panel(PANEL_orientation, "yz", &windrose_yz_vis);
      }

      if(nzwindtreeinfo>0){
        int icheckboxes;

        ROLLOUT_showhide_windrose = glui_device->add_rollout_to_panel(ROLLOUT_windrose, "trees", false);

        SPINNER_windrose_first = glui_device->add_spinner_to_panel(ROLLOUT_showhide_windrose, _("first"), GLUI_SPINNER_INT, &windrose_first, DEVICE_WINDROSE_SHOW_FIRST, GLUIDeviceCB);
        SPINNER_windrose_next = glui_device->add_spinner_to_panel(ROLLOUT_showhide_windrose, _("skip"), GLUI_SPINNER_INT, &windrose_next, DEVICE_WINDROSE_SHOW_NEXT, GLUIDeviceCB);

        NewMemory((void **)&ROLLOUT_showz_windrose, nzwindtreeinfo*sizeof(GLUI_Rollout *));

        nwindrosez_checkboxes = 0;
        for(i = 0; i<nzwindtreeinfo; i++){
          treedevicedata *treei;
          int j;

          treei = zwindtreeinfo[i];
          for(j = treei->first; j<=treei->last; j++){
            vdevicesortdata *vdevsorti;
            vdevicedata *vd;

            vdevsorti = global_scase.devicecoll.vdevices_sorted+j;
            vd = vdevsorti->vdeviceinfo;
            if(vd->udev==NULL&&vd->vdev==NULL&&vd->wdev==NULL&&
               vd->angledev==NULL&&vd->veldev==NULL)continue;
            if(vdevsorti->dir==ZDIR&&vd->unique!=0)nwindrosez_checkboxes++;
          }
        }
        NewMemory((void **)&CHECKBOX_showz_windrose, nwindrosez_checkboxes*sizeof(GLUI_Checkbox *));
        for(i = 0; i<nwindrosez_checkboxes; i++){
          CHECKBOX_showz_windrose[i] = NULL;
        }
        GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_DEVICE);

        icheckboxes = 0;
        for(i = 0; i<nzwindtreeinfo; i++){
          char roselabel[1024], xlabel[256], ylabel[256];
          float *xyz;
          treedevicedata *treei;
          int j;

          treei = zwindtreeinfo[i];
          xyz = treei->xyz;

          //sprintf(xlabel, "%f", xyz[0]);
          snprintf(xlabel, sizeof(xlabel), "%f", xyz[0]);
          //sprintf(ylabel, "%f", xyz[1]);
          snprintf(ylabel, sizeof(ylabel), "%f", xyz[1]);
          TrimZeros(xlabel);
          TrimZeros(ylabel);
          //sprintf(roselabel, "x=%s, y=%s", xlabel, ylabel);
          snprintf(roselabel, sizeof(roselabel), "x=%s, y=%s", xlabel, ylabel);
          ROLLOUT_showz_windrose[i] = glui_device->add_rollout_to_panel(ROLLOUT_showhide_windrose, roselabel, false);
          glui_device->add_button_to_panel(ROLLOUT_showz_windrose[i], _("Show all"), DEVICE_WINDROSE_SHOWHIDEALL+i, GLUIDeviceCB);
          glui_device->add_button_to_panel(ROLLOUT_showz_windrose[i], _("Hide all"), DEVICE_WINDROSE_SHOWHIDEALL+nzwindtreeinfo+i, GLUIDeviceCB);

          for(j = treei->first; j<=treei->last; j++){
            vdevicesortdata *vdevsorti;
            vdevicedata *vd;

            vdevsorti = global_scase.devicecoll.vdevices_sorted+j;
            vd = vdevsorti->vdeviceinfo;
            xyz = NULL;
            if(xyz==NULL&&vd->udev!=NULL)xyz = vd->udev->xyz;
            if(xyz==NULL&&vd->vdev!=NULL)xyz = vd->vdev->xyz;
            if(xyz==NULL&&vd->wdev!=NULL)xyz = vd->wdev->xyz;
            if(xyz==NULL&&vd->veldev!=NULL)xyz = vd->veldev->xyz;
            if(xyz==NULL&&vd->angledev!=NULL)xyz = vd->angledev->xyz;
            if(xyz!=NULL&&vdevsorti->dir==ZDIR&&vd->unique!=0){
              char zlabel[256];

              //sprintf(zlabel, "z=%f", xyz[2]);
              snprintf(zlabel, sizeof(zlabel), "z=%f", xyz[2]);
              TrimZeros(zlabel);
              CHECKBOX_showz_windrose[icheckboxes++] = glui_device->add_checkbox_to_panel(ROLLOUT_showz_windrose[i], zlabel, &vd->display);
            }
          }
        }
      }

      ROLLOUT_scale_windrose = glui_device->add_rollout_to_panel(ROLLOUT_windrose, _("scale"), false);
      glui_device->add_checkbox_to_panel(ROLLOUT_scale_windrose, _("show scale"), &showref_windrose);
      glui_device->add_checkbox_to_panel(ROLLOUT_scale_windrose, _("show labels"), &showlabels_windrose);
      RADIO_scale_windrose = glui_device->add_radiogroup_to_panel(ROLLOUT_scale_windrose, &scale_windrose);
      glui_device->add_radiobutton_to_group(RADIO_scale_windrose, _("local"));
      glui_device->add_radiobutton_to_group(RADIO_scale_windrose, _("global"));
      SPINNER_scale_increment_windrose = glui_device->add_spinner_to_panel(ROLLOUT_scale_windrose, _("increment"), GLUI_SPINNER_INT, &scale_increment_windrose);
      SPINNER_scale_increment_windrose->set_int_limits(1, 50);
      SPINNER_scale_max_windrose = glui_device->add_spinner_to_panel(ROLLOUT_scale_windrose, _("max"), GLUI_SPINNER_INT, &scale_max_windrose);
      SPINNER_scale_max_windrose->set_int_limits(0, 100);

      GLUIUpdateDevices();
    }
  }

  if(ndevicetypes>0){
    ROLLOUT_values = glui_device->add_rollout_to_panel(PANEL_objects, "devc values", false);
    CHECKBOX_showdevice_val = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Values"), &showdevice_val, DEVICE_SHOWDEVICEVALS, GLUIDeviceCB);
    CHECKBOX_device_1 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Id"), &showdevice_id, DEVICE_SHOWDEVICEVALS, GLUIDeviceCB);
    CHECKBOX_device_5 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Type"), &showdevice_type, DEVICE_SHOWDEVICEVALS, GLUIDeviceCB);
    CHECKBOX_device_6 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Unit"), &showdevice_unit, DEVICE_SHOWDEVICEVALS, GLUIDeviceCB);
    CHECKBOX_device_4 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Color"), &colordevice_val, DEVICE_COLORDEVICEVALS, GLUIDeviceCB);
    glui_device->add_spinner_to_panel(ROLLOUT_values, "min", GLUI_SPINNER_FLOAT, &device_valmin);
    glui_device->add_spinner_to_panel(ROLLOUT_values, "max", GLUI_SPINNER_FLOAT, &device_valmax);
  }

  PANEL_label3 = glui_device->add_panel("",false);
  glui_device->add_column_to_panel(PANEL_label3,false);

  glui_device->add_button_to_panel(PANEL_label3,_("Save settings"),DEVICE_SAVE_SETTINGS_OBJECTS,GLUIDeviceCB);
  glui_device->add_column_to_panel(PANEL_label3,false);

  BUTTON_device_2=glui_device->add_button_to_panel(PANEL_label3,_("Close"),DEVICE_close,GLUIDeviceCB);
#ifdef pp_CLOSEOFF
  BUTTON_device_2->disable();
#endif

  glui_device->set_main_gfx_window( main_window );
}

/* ------------------ GLUIHideDevice ------------------------ */

extern "C" void GLUIHideDevice(void){
  GLUICloseRollouts(glui_device);
}

/* ------------------ GLUIShowPlotDevice ------------------------ */

extern "C" void GLUIShowPlotDevice(void){
  if(glui_device!=NULL){
    glui_device->show();
    if(ROLLOUT_device2Dplots!=NULL)ROLLOUT_device2Dplots->open();
  }

}

/* ------------------ GLUIShowDevice ------------------------ */

extern "C" void GLUIShowDevice(void){
  if(glui_device!=NULL&&ROLLOUT_smvobjects!=NULL){
    glui_device->show();
    ROLLOUT_smvobjects->open();
  }
}
/* ------------------ OpenCB ------------------------ */

void OpenCB(int var){
  int i;
  filelistdata *filei;
  char *open_filter_ptr;


  switch(var){
    case OPEN_UP:
      strcat(gluiopen_path_dir,dirseparator);
      strcat(gluiopen_path_dir,"..");
      OpenCB(OPEN_UPDATE_LIST);
      break;
    case OPEN_DOWN:
      if(gluiopen_filelist==NULL)break;
      filei = gluiopen_filelist + gluiopen_file_index;
      if(filei->type==1){
        strcat(gluiopen_path_dir,dirseparator);
        strcat(gluiopen_path_dir,filei->file);
        OpenCB(OPEN_UPDATE_LIST);
      }
      break;
    case OPEN_FILEINDEX:
      PRINTF("in OPEN_FILEINDEX\n");
      if(gluiopen_filelist==NULL)break;
      filei = gluiopen_filelist + gluiopen_file_index;
      if(filei->type==1){
        BUTTON_open_down->enable();
      }
      else{
        BUTTON_open_down->disable();
      }
      break;
    case OPEN_OPEN:
      if(gluiopen_filelist==NULL)break;
      filei = gluiopen_filelist + gluiopen_file_index;
      if(filei->type==1){
        strcat(gluiopen_path_dir,dirseparator);
        strcat(gluiopen_path_dir,filei->file);
        OpenCB(OPEN_UPDATE_LIST);
      }
      else{
        PRINTF("opening file: %s\n",filei->file);
      }
      break;
    case OPEN_CANCEL:
      break;
    case OPEN_FILTER:
      break;
    case OPEN_APPLY_FILTER:
      strcpy(gluiopen_filter2,gluiopen_filter);
      TrimBack(gluiopen_filter2);
      open_filter_ptr = TrimFront(gluiopen_filter2);
      EDIT_filter->set_text(open_filter_ptr);
      OpenCB(OPEN_UPDATE_LIST);
      break;
    case OPEN_UPDATE_LIST:
      LIST_open->delete_item("");
      for(i=0;i<gluiopen_nfilelist;i++){
        char label[1024];

        strcpy(label,"");
        if(gluiopen_filelist[i].type==1){
          strcat(label,"> ");
        }
        strcat(label,gluiopen_filelist[i].file);
        LIST_open->delete_item(label);
      }
      FreeFileList(gluiopen_filelist,&gluiopen_nfilelist);
      gluiopen_nfilelist=GetFileListSize(gluiopen_path_dir,gluiopen_filter,FILE_MODE);
      if(gluiopen_nfilelist==0){
        LIST_open->add_item(0,"");
      }
      MakeFileList(gluiopen_path_dir, gluiopen_filter,gluiopen_nfilelist,NO,&gluiopen_filelist, FILE_MODE);
      if(gluiopen_nfilelist>0&&gluiopen_filelist[0].type==1){
        BUTTON_open_down->enable();
      }
      else{
        BUTTON_open_down->disable();
      }
      for(i=0;i<gluiopen_nfilelist;i++){
        char label[1024];

        strcpy(label,"");
        if(gluiopen_filelist[i].type==1){
          strcat(label,"> ");
        }
        strcat(label,gluiopen_filelist[i].file);
        LIST_open->add_item(i,label);
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}
