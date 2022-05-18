#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

#define DEVICE_sensorsize      20
#define SHOWDEVICEVALS         26
#define SLICE_PLOT             31
#define SHOWDEVICEPLOT         29
#define COLORDEVICEVALS        27
//#define DEVICE_devicetypes     28  put in smokeviewdefs.h
#define SAVE_SETTINGS_OBJECTS  99
#define DEVICE_close            3
#define DEVICE_show_orientation 4
#define DEVICE_NBUCKETS         5
#define DEVICE_SHOWBEAM         6
#define DEVICE_RADIUS           7
//#define HRRPUV_PLOT            30 put in smokeviewdef.h
#define DEVICE_TIMEAVERAGE     31
#define FUEL_HOC               32
#define RESET_FUEL_HOC         33

#ifdef pp_PLOT2D_NEW
#define GENPLOT_devID1        101
#define GENPLOT_devtype1      102
#define GENPLOT_ADDDEV1       103
#define GENPLOT_ADDHRR1       104
#define GENPLOT_COMP1         105
#define GENPLOT_CLEAR1        106
#define GENPLOT_CLEARALL      107
#define GENPLOT_HRR1          108
#define GENPLOT_SHOW1         109
#define GENPLOT_XYZ           110

#define PLOT2D_DEV              0
#define PLOT2D_HRR              1
#endif

#define WINDROSE_SHOW_FIRST   996
#define WINDROSE_SHOW_NEXT    997
#define WINDROSE_DXYZ         995
#define WINDROSE_SETPOS       993
#define WINDROSE_DT           998
#define WINDROSE_DTMINMAX     994
#define WINDROSE_UPDATE       999
#define WINDROSE_SHOWHIDEALL 1000

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

#ifdef pp_PLOT2D_NEW
GLUI_Button *BUTTON_add_dev=NULL ;
GLUI_Button *BUTTON_add_hrr=NULL ;
GLUI_Button *BUTTON_remove_plot=NULL ;
#endif
GLUI_Button *BUTTON_open_down=NULL ;
GLUI_Button *BUTTON_device_1=NULL;
GLUI_Button *BUTTON_device_2=NULL;
GLUI_Button *BUTTON_update_windrose = NULL;
GLUI_Button *BUTTON_reset_fuel_hoc = NULL;

#ifdef pp_PLOT2D_NEW
GLUI_Checkbox *CHECKBOX_show_genplot = NULL;
#endif
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
GLUI_Checkbox *CHECKBOX_vis_hrr_plot=NULL;

GLUI_EditText *EDIT_filter=NULL;

#ifdef pp_PLOT2D_NEW
GLUI_Listbox *LIST_devID1 = NULL;
GLUI_Listbox *LIST_devtype1 = NULL;
GLUI_Listbox *LIST_hrr1 = NULL;
GLUI_Listbox *LIST_plotlist1 = NULL;
#endif
GLUI_Listbox *LIST_devicetypes = NULL;
GLUI_Listbox *LIST_open=NULL;
GLUI_Listbox *LIST_hrrdata=NULL;

#ifdef pp_PLOT2D_NEW
GLUI_Panel *PANEL_plotdevice_select = NULL;
GLUI_Panel *PANEL_plotgeneral_device = NULL;
GLUI_Panel *PANEL_plotgeneral_hrr = NULL;
GLUI_Panel *PANEL_plotgeneral_plot = NULL;
GLUI_Panel *PANEL_plotgeneral_position = NULL;
#endif
GLUI_Panel *PANEL_hrr_min = NULL;
GLUI_Panel *PANEL_hrr_max = NULL;
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

GLUI_RadioGroup *RADIO_windrose_ttype = NULL;
GLUI_RadioGroup *RADIO_windrose_merge_type=NULL;
GLUI_RadioGroup *RADIO_vectortype=NULL;
GLUI_RadioGroup *RADIO_scale_windrose=NULL;
GLUI_RadioGroup *RADIO_windstate_windrose = NULL;
GLUI_RadioGroup *RADIO_vis_device_plot = NULL;

#ifdef pp_PLOT2D_NEW
GLUI_Rollout *ROLLOUT_plotgeneral = NULL;
#endif
GLUI_Rollout *ROLLOUT_plotslice=NULL;
GLUI_Rollout *ROLLOUT_plothrr=NULL;
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

#ifdef pp_PLOT2D_NEW
GLUI_Spinner *SPINNER_genplot_x = NULL;
GLUI_Spinner *SPINNER_genplot_y = NULL;
GLUI_Spinner *SPINNER_genplot_z = NULL;
#endif
GLUI_Spinner *SPINNER_fuel_hoc = NULL;
GLUI_Spinner *SPINNER_size_factor = NULL;
GLUI_Spinner *SPINNER_slice_x = NULL;
GLUI_Spinner *SPINNER_slice_y = NULL;
GLUI_Spinner *SPINNER_slice_z = NULL;
GLUI_Spinner *SPINNER_device_time_average = NULL;
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

#define OBJECTS_ROLLOUT     0
#define FLOWVECTORS_ROLLOUT 1
#define WINDROSE_ROLLOUT    2
#define PLOT2D_ROLLOUT      3

procdata deviceprocinfo[4];
int ndeviceprocinfo = 0;

/* ------------------ Device_Rollout_CB ------------------------ */

void Device_Rollout_CB(int var){
  ToggleRollout(deviceprocinfo, ndeviceprocinfo, var);
}

/* ------------------ UpdateDeviceTypes ------------------------ */

extern "C" void UpdateDeviceTypes(int val){
  devicetypes_index = val;
  LIST_devicetypes->set_int_val(val);
  updatemenu = 1;
}

/* ------------------ UpdateVisHrrPlot ------------------------ */

extern "C" void UpdateVisHrrPlot(void){
  if(CHECKBOX_vis_hrr_plot!=NULL)CHECKBOX_vis_hrr_plot->set_int_val(vis_hrr_plot);
}

/* ------------------ UpdateDeviceShow ------------------------ */

extern "C" void UpdateDeviceShow(void){
  if(CHECKBOX_showdevice_val!=NULL)CHECKBOX_showdevice_val->set_int_val(showdevice_val);
  if(RADIO_vis_device_plot!=NULL)RADIO_vis_device_plot->set_int_val(vis_device_plot);
}

/* ------------------ UpdateSliceXYZ ------------------------ */

extern "C" void UpdateSliceXYZ(void){
  if(SPINNER_slice_x!=NULL)SPINNER_slice_x->set_float_val(slice_xyz[0]);
  if(SPINNER_slice_y!=NULL)SPINNER_slice_y->set_float_val(slice_xyz[1]);
  if(SPINNER_slice_z!=NULL)SPINNER_slice_z->set_float_val(slice_xyz[2]);
}

/* ------------------ UpdateWindRoseDevices ------------------------ */

extern "C" void UpdateWindRoseDevices(int option){
  int i,icheckboxes;

  icheckboxes = 0;
  for(i = 0; i<nzwindtreeinfo; i++){
    treedevicedata *treei;
    int j;

    treei = zwindtreeinfo[i];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;
      vdevicedata *vd;

      vdevsorti = vdevices_sorted+j;
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
    if(windrosez_showhide!=NULL)UpdateWindRoseDevices(UPDATE_WINDROSE_SHOWHIDE);
  }
}

/* ------------------ UpdateShowbeamAsLine ------------------------ */

extern "C" void UpdateShowbeamAsLine(void){
  if(CHECKBOX_showbeam_as_line!=NULL)CHECKBOX_showbeam_as_line->set_int_val(showbeam_as_line);
}

/* ------------------ UpdateDeviceSize ------------------------ */

extern "C" void UpdateDeviceSize(void){
  if(sensorrelsize<sensorrelsizeMIN)sensorrelsize = sensorrelsizeMIN;
  if(SPINNER_sensorrelsize!=NULL)SPINNER_sensorrelsize->set_float_val(sensorrelsize);
}

/* ------------------ UpdateDeviceOrientation ------------------------ */

extern "C" void UpdateDeviceOrientation(void){
  if(CHECKBOX_device_orientation!=NULL)CHECKBOX_device_orientation->set_int_val(show_device_orientation);
}

/* ------------------ UpdateShowWindRoses ------------------------ */

void UpdateShowWindRoses(void) {
  int i;
  int icheckbox;

  icheckbox=0;
  for (i = 0; i < nzwindtreeinfo; i++) {
    treedevicedata *treei;
    int j;
    int idev;

    treei = zwindtreeinfo[i];

    idev = 0;
    for (j = treei->first; j <= treei->last; j++) {
      vdevicesortdata *vdevsorti;
      vdevicedata *vd;

      vdevsorti = vdevices_sorted + j;
      vd = vdevsorti->vdeviceinfo;
      if(vd->udev==NULL&&vd->vdev==NULL&&vd->wdev==NULL&&
         vd->angledev == NULL&&vd->veldev == NULL)continue;

      if(vdevsorti->dir == ZDIR&&vd->unique != 0) {
        int kk, idev_hat;
        //idev = istart + k*iskip
        kk = (idev-windrose_first)/windrose_next;
        idev_hat = windrose_first + kk*windrose_next;
        if(kk>=0&&idev==idev_hat){
          vd->display = 1;
        }
        else {
          vd->display = 0;
        }
        CHECKBOX_showz_windrose[icheckbox]->set_int_val(vd->display);
        idev++;
        icheckbox++;
      }
    }
  }
}

#ifdef pp_PLOT2D_NEW

/* ------------------ AdddPlot2D ------------------------ */
  int *device_index, ndevice_index;
  int *hrr_index, nhrr_index;

void AddPlot2D(int type){
  int i, have_index, nindex;
  int offset = 0, index;

  if(type == PLOT2D_HRR){
    offset = ndeviceinfo;
    index = LIST_hrr1->get_int_val();
    }
  else{
    index = LIST_devID1->get_int_val();
  }
  have_index = 0;
  for(i=0; i<plot2dinfo->ncurve_index; i++){
    if(plot2dinfo->curve_index[i] == index+offset){
      have_index = 1;
      break;
    }
  }
  if(have_index == 0){
    char label[255];

    nindex = plot2dinfo->ncurve_index;
    plot2dinfo->curve_index[nindex] = offset + index;
    nindex++;
    plot2dinfo->ncurve_index = nindex;
    if(type == PLOT2D_DEV){
      strcpy(label, "dev/");
      strcat(label, deviceinfo[index].labelptr);
    }
    else{
      strcpy(label, "hrr/");
      strcat(label, hrrinfo[index].label.shortlabel);
    }
    LIST_plotlist1->add_item(offset + index, label);
    LIST_plotlist1->set_int_val(offset + index);
  }
}

/* ------------------ RemovePlot2D ------------------------ */

void RemovePlot2D(int index){
  if(index<0){
    int i;

    for(i=0;i<plot2dinfo->ncurve_index; i++){
      LIST_plotlist1->delete_item(plot2dinfo->curve_index[i]);
    }
    plot2dinfo->ncurve_index = 0;
    LIST_plotlist1->set_int_val(-1);
  }
  else{
    int i, ii;

    LIST_plotlist1->delete_item(index);

    ii = 0;
    for(i = 0; i < plot2dinfo->ncurve_index; i++){
      if(plot2dinfo->curve_index[i] == index)continue;
      if(i != ii)plot2dinfo->curve_index[ii] = plot2dinfo->curve_index[i];
      ii++;
    }
    (plot2dinfo->ncurve_index)--;
    if(plot2dinfo->ncurve_index > 0){
      LIST_plotlist1->set_int_val(plot2dinfo->curve_index[0]);
    }
    else{
      LIST_plotlist1->set_int_val(-1);
    }
  }
}

/* ------------------ UpdateDevList ------------------------ */

void UpdateDevList(GLUI_Listbox *LIST_dev, int devtype_index){
  int i;

  if(LIST_dev == NULL)return;
  for(i = 0; i < ndeviceinfo; i++){
    devicedata *devicei;

    devicei = deviceinfo + i;
    if(devicei->inlist1 == 1){
      devicei->inlist1 = 0;
      LIST_dev->delete_item(i);
    }
  }
  for(i = 0; i < ndeviceinfo; i++){
    devicedata *devicei;
    int inlist;

    inlist = 0;
    devicei = deviceinfo + i;
    if(devtype_index == -1 || strcmp(devicetypes[devtype_index]->quantity, devicei->quantity) == 0)inlist = 1;
    if(inlist == 1 && devicei->inlist1 == 0){
      devicei->inlist1 = 1;
      LIST_dev->add_item(i, devicei->deviceID);
    }
  }
}
/* ------------------ GenPlotCB ------------------------ */

void GenPlotCB(int var){
  switch (var){
    char label[256];

    case GENPLOT_devID1:
      strcpy(label, "Add ");
      strcat(label, deviceinfo[deviceID1_index].deviceID);
      strcat(label, " to plot");
      BUTTON_add_dev->set_name(label);
      break;
    case GENPLOT_devtype1:
      UpdateDevList(LIST_devID1, devtype1_index);
      break;
    case GENPLOT_ADDDEV1:
      AddPlot2D(PLOT2D_DEV);
      GenPlotCB(GENPLOT_COMP1);
      break;
    case GENPLOT_ADDHRR1:
      AddPlot2D(PLOT2D_HRR);
      GenPlotCB(GENPLOT_COMP1);
      break;
    case GENPLOT_COMP1:
      if(BUTTON_remove_plot != NULL){
        strcpy(label, "Remove ");
        strcat(label, LIST_plotlist1->curr_text);
        strcat(label, " from plot");
        BUTTON_remove_plot->set_name(label);
      }
      break;
    case GENPLOT_CLEAR1:
      RemovePlot2D(plot_component1);
      break;
    case GENPLOT_CLEARALL:
      RemovePlot2D(-1);
      break;
    case GENPLOT_HRR1:
      strcpy(label, "Add ");
      strcat(label, hrrinfo[hrr1_index].label.shortlabel);
      strcat(label, " to plot");
      BUTTON_add_hrr->set_name(label);
      break;
    case GENPLOT_SHOW1:
      plot2dinfo[0].show = show_genplot1;
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      update_times = 1;
      break;
    case GENPLOT_XYZ:
      memcpy(plot2dinfo[0].xyz, genplot_xyz, 3*sizeof(float));
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}
#endif

/* ------------------ DeviceCB ------------------------ */

extern "C" void DeviceCB(int var){
  int i;

  updatemenu = 1;
  if(var==SLICE_PLOT){
    Slice2Device();
  }
  if(var==HRRPUV_PLOT){
    vis_hrr_plot = 1-vis_hrr_plot;
    ShowObjectsMenu(PLOT_HRRPUV);
    return;
  }
  if(var==HRRPUV2_PLOT){
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    update_times = 1;
    update_avg = 1;
  }
  if(var == WINDROSE_UPDATE){
    if(viswindrose==1)DeviceData2WindRose(nr_windrose, ntheta_windrose);
    return;
  }
  if(var==WINDROSE_SETPOS){
    for(i = 0; i<4; i++){
      if(windrose_merge_dxyzt[i]<0.0){
        windrose_merge_dxyzt[i] = 0.0;
        SPINNER_windrose_merge_dxyzt[i]->set_float_val(windrose_merge_dxyzt[i]);
      }
    }
    return;
  }
  if(var == WINDROSE_DT){
    if(windrose_ttype==WINDROSE_USE_NEITHER){
      windrose_ttype=WINDROSE_USE_DT;
      RADIO_windrose_ttype->set_int_val(windrose_ttype);
    }
    if(windrose_merge_type==WINDROSE_POINT&&windrose_merge_dxyzt[3]>0.0){
      windrose_merge_type=WINDROSE_SLIDING;
      RADIO_windrose_merge_type->set_int_val(windrose_merge_type);
    }
    DeviceCB(WINDROSE_SETPOS);
    return;
  }
  if(var == WINDROSE_DTMINMAX){
    if(windrose_ttype==WINDROSE_USE_NEITHER){
      windrose_ttype=WINDROSE_USE_TMINMAX;
      RADIO_windrose_ttype->set_int_val(windrose_ttype);
    }
    if(windrose_merge_type==WINDROSE_POINT&&(windrose_merge_dxyzt[4]>0.0||windrose_merge_dxyzt[5]>0.0)){
      windrose_merge_type=WINDROSE_SLIDING;
      RADIO_windrose_merge_type->set_int_val(windrose_merge_type);
    }
    DeviceCB(WINDROSE_SETPOS);
    return;
  }
  if(var == WINDROSE_DXYZ){
    DeviceCB(WINDROSE_SETPOS);
    return;
  }
  if(var == WINDROSE_SHOW_FIRST){
    if(windrose_first<0){
      windrose_first=0;
      SPINNER_windrose_first->set_int_val(windrose_first);
    }
    UpdateShowWindRoses();
    return;
  }
  if(var == WINDROSE_SHOW_NEXT){
    if(windrose_next<1){
      windrose_next=1;
      SPINNER_windrose_next->set_int_val(windrose_next);
    }
    UpdateShowWindRoses();
    return;
  }
  if(var>=WINDROSE_SHOWHIDEALL&&var<WINDROSE_SHOWHIDEALL+nzwindtreeinfo){
    int iztree, j;
    treedevicedata *treei;

    iztree = var-WINDROSE_SHOWHIDEALL;
    treei = zwindtreeinfo[iztree];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;

      vdevsorti = vdevices_sorted+j;
      if(vdevsorti->dir==ZDIR){
        vdevicedata *vd;

        vd = vdevsorti->vdeviceinfo;
        //if(vd->unique==0)continue;
        vd->display = 1;
      }
    }
    UpdateWindRoseDevices(UPDATE_WINDROSE_CHECKBOX);
    return;
  }
  if(var>=WINDROSE_SHOWHIDEALL+nzwindtreeinfo&&var<WINDROSE_SHOWHIDEALL+2*nzwindtreeinfo){
    int iztree, j;
    treedevicedata *treei;

    iztree = var-WINDROSE_SHOWHIDEALL - nzwindtreeinfo;
    treei = zwindtreeinfo[iztree];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;

      vdevsorti = vdevices_sorted+j;
      if(vdevsorti->dir==ZDIR){
        vdevicedata *vd;

        vd = vdevsorti->vdeviceinfo;
        //if(vd->unique==0)continue;
        vd->display = 0;
      }
    }
    UpdateWindRoseDevices(UPDATE_WINDROSE_CHECKBOX);
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
    for(i = 0; i<ndeviceinfo; i++){
      devicedata *devicei;

      devicei = deviceinfo+i;
      devicei->update_avg = 1;
    }
    update_avg = 1;
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
  case RESET_FUEL_HOC:
    fuel_hoc = fuel_hoc_default;
    SPINNER_fuel_hoc->set_float_val(fuel_hoc);
    DeviceCB(FUEL_HOC);
    break;;
  case FUEL_HOC:
    if(fuel_hoc<0.0){
      fuel_hoc = 0.0;
      SPINNER_fuel_hoc->set_float_val(fuel_hoc);
    }
    UpdateHoc();
    break;
  case SHOWDEVICEPLOT:
    {
      int vis_device_plot_temp;

      vis_device_plot_temp = vis_device_plot;
      switch (vis_device_plot_temp){
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
          ASSERT(FFALSE);
          break;
      }
    }
    break;
  case SHOWDEVICEVALS:
  case COLORDEVICEVALS:
    update_times=1;
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
  case SAVE_SETTINGS_OBJECTS:
    WriteIni(LOCAL_INI, NULL);
    break;
  case DEVICE_close:
    HideGluiDevice();
    break;
  default:
    ASSERT(FFALSE);
  }
}

/* ------------------ UpdateGluiDevices ------------------------ */

extern "C" void UpdateGluiDevices(void){
  DeviceCB(SHOWDEVICEPLOT);
  DeviceCB(SHOWDEVICEVALS);
  DeviceCB(COLORDEVICEVALS);
  DeviceCB(DEVICE_devicetypes);
}

/* ------------------ GetDeviceTminTmax ------------------------ */

float GetDeviceTminTmax(void){
  float return_val=1.0;
  int first = 1, i;

  for(i = 0; i<ndeviceinfo; i++){
    devicedata *devicei;
    float *times;

    devicei = deviceinfo+i;
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

/* ------------------ GluiDeviceSetup ------------------------ */

extern "C" void GluiDeviceSetup(int main_window){

  if(glui_device!=NULL){
    glui_device->close();
    glui_device=NULL;
  }
  glui_device = GLUI_Master.create_glui("Devices/Objects/2D plots",0,0,0);
  glui_device->hide();

  if(ndeviceinfo>0){
    int i;

    PANEL_objects = glui_device->add_panel("Devices/Objects/2D plots", false);

    ROLLOUT_smvobjects = glui_device->add_rollout_to_panel(PANEL_objects, _("Objects"), false, OBJECTS_ROLLOUT, Device_Rollout_CB);
    INSERT_ROLLOUT(ROLLOUT_smvobjects, glui_device);
    ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_smvobjects, OBJECTS_ROLLOUT, glui_device);

    SPINNER_sensorrelsize = glui_device->add_spinner_to_panel(ROLLOUT_smvobjects, _("Scale"), GLUI_SPINNER_FLOAT, &sensorrelsize, DEVICE_sensorsize, DeviceCB);
    CHECKBOX_device_3 = glui_device->add_checkbox_to_panel(ROLLOUT_smvobjects, _("Outline"), &object_outlines);
    CHECKBOX_device_orientation = glui_device->add_checkbox_to_panel(ROLLOUT_smvobjects, _("Orientation"), &show_device_orientation, DEVICE_show_orientation, DeviceCB);
    SPINNER_orientation_scale = glui_device->add_spinner_to_panel(ROLLOUT_smvobjects, _("Orientation scale"), GLUI_SPINNER_FLOAT, &orientation_scale);
    SPINNER_orientation_scale->set_float_limits(0.1, 10.0);
    if(have_beam){
      PANEL_beam = glui_device->add_panel_to_panel(ROLLOUT_smvobjects, "Beam sensor", true);
      CHECKBOX_showbeam_as_line = glui_device->add_checkbox_to_panel(PANEL_beam, _("Show beam as line"), &showbeam_as_line, DEVICE_SHOWBEAM, DeviceCB);
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

    if(GetNumActiveDevices()>0||isZoneFireModel==1){
      ROLLOUT_velocityvectors = glui_device->add_rollout_to_panel(PANEL_objects, "Flow vectors", false, FLOWVECTORS_ROLLOUT, Device_Rollout_CB);
      INSERT_ROLLOUT(ROLLOUT_velocityvectors, glui_device);
      ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_velocityvectors, FLOWVECTORS_ROLLOUT, glui_device);

      if(nvdeviceinfo==0)ROLLOUT_velocityvectors->disable();
      CHECKBOX_device_1 = glui_device->add_checkbox_to_panel(ROLLOUT_velocityvectors, _("Show"), &showvdevice_val);
      PANEL_vector_type = glui_device->add_panel_to_panel(ROLLOUT_velocityvectors, _("type"), true);
      RADIO_vectortype = glui_device->add_radiogroup_to_panel(PANEL_vector_type, &vectortype);
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("line"));
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("arrow"));
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("object"));
      glui_device->add_radiobutton_to_group(RADIO_vectortype, _("profile"));
      ROLLOUT_arrow_dimensions = glui_device->add_rollout_to_panel(ROLLOUT_velocityvectors, _("Dimensions"), false);
      INSERT_ROLLOUT(ROLLOUT_arrow_dimensions, glui_device);
      PANEL_arrow_base = glui_device->add_panel_to_panel(ROLLOUT_arrow_dimensions, _("base"), true);
      glui_device->add_spinner_to_panel(PANEL_arrow_base, _("length"), GLUI_SPINNER_FLOAT, &vector_baselength);
      glui_device->add_spinner_to_panel(PANEL_arrow_base, _("diameter"), GLUI_SPINNER_FLOAT, &vector_basediameter);
      PANEL_arrow_height = glui_device->add_panel_to_panel(ROLLOUT_arrow_dimensions, _("head"), true);
      glui_device->add_spinner_to_panel(PANEL_arrow_height, _("length"), GLUI_SPINNER_FLOAT, &vector_headlength);
      glui_device->add_spinner_to_panel(PANEL_arrow_height, _("diameter"), GLUI_SPINNER_FLOAT, &vector_headdiameter);

      if(ntreedeviceinfo>0){
        ROLLOUT_trees = glui_device->add_rollout_to_panel(ROLLOUT_velocityvectors, "Device trees", false);
        INSERT_ROLLOUT(ROLLOUT_trees, glui_device);
        SPINNER_mintreesize = glui_device->add_spinner_to_panel(ROLLOUT_trees, _("min size"), GLUI_SPINNER_INT, &mintreesize);
        SPINNER_mintreesize->set_int_limits(2, MAX(2, max_device_tree));
        CHECKBOX_vis_xtree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _A(_("Show"), " x"), &vis_xtree);
        CHECKBOX_vis_ytree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _A(_("Show"), " y"), &vis_ytree);
        CHECKBOX_vis_ztree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _A(_("Show"), " z"), &vis_ztree);
      }

      ROLLOUT_windrose = glui_device->add_rollout_to_panel(PANEL_objects, "Windrose", false, WINDROSE_ROLLOUT, Device_Rollout_CB);
      INSERT_ROLLOUT(ROLLOUT_windrose, glui_device);
      ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_windrose, WINDROSE_ROLLOUT, glui_device);


      glui_device->add_checkbox_to_panel(ROLLOUT_windrose, _("show"), &viswindrose, WINDROSE_UPDATE, DeviceCB);

      SPINNER_nr_windrose = glui_device->add_spinner_to_panel(ROLLOUT_windrose, _("radii"), GLUI_SPINNER_INT, &nr_windrose, DEVICE_NBUCKETS, DeviceCB);
      SPINNER_nr_windrose->set_int_limits(3, 72, GLUI_LIMIT_CLAMP);
      SPINNER_ntheta_windrose = glui_device->add_spinner_to_panel(ROLLOUT_windrose, _("angles"), GLUI_SPINNER_INT, &ntheta_windrose, DEVICE_NBUCKETS, DeviceCB);
      SPINNER_ntheta_windrose->set_int_limits(3, 72, GLUI_LIMIT_CLAMP);
      SPINNER_radius_windrose = glui_device->add_spinner_to_panel(ROLLOUT_windrose, _("radius"), GLUI_SPINNER_FLOAT, &radius_windrose, DEVICE_RADIUS, DeviceCB);
      RADIO_windstate_windrose = glui_device->add_radiogroup_to_panel(ROLLOUT_windrose, &windstate_windrose);
      glui_device->add_radiobutton_to_group(RADIO_windstate_windrose, "heading");
      glui_device->add_radiobutton_to_group(RADIO_windstate_windrose, "direction");

      ROLLOUT_show_windrose2 = glui_device->add_rollout_to_panel(ROLLOUT_windrose, "merge data", false);
      PANEL_windrose_mergexyz = glui_device->add_panel_to_panel(ROLLOUT_show_windrose2, "space", true);
      SPINNER_windrose_merge_dxyzt[0] = glui_device->add_spinner_to_panel(PANEL_windrose_mergexyz, "dx", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt, WINDROSE_DXYZ, DeviceCB);
      SPINNER_windrose_merge_dxyzt[1] = glui_device->add_spinner_to_panel(PANEL_windrose_mergexyz, "dy", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+1, WINDROSE_DXYZ, DeviceCB);
      SPINNER_windrose_merge_dxyzt[2] = glui_device->add_spinner_to_panel(PANEL_windrose_mergexyz, "dz", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+2, WINDROSE_DXYZ, DeviceCB);

      PANEL_windrose_merget = glui_device->add_panel_to_panel(ROLLOUT_show_windrose2, "time", true);
      SPINNER_windrose_merge_dxyzt[3] = glui_device->add_spinner_to_panel(PANEL_windrose_merget, "dt", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+3, WINDROSE_DT, DeviceCB);
      SPINNER_windrose_merge_dxyzt[4] = glui_device->add_spinner_to_panel(PANEL_windrose_merget, "tmin", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+4, WINDROSE_DTMINMAX, DeviceCB);
      SPINNER_windrose_merge_dxyzt[5] = glui_device->add_spinner_to_panel(PANEL_windrose_merget, "tmax", GLUI_SPINNER_FLOAT, windrose_merge_dxyzt+5, WINDROSE_DTMINMAX, DeviceCB);

      PANEL_windrose_merge = glui_device->add_panel_to_panel(PANEL_windrose_merget, "type", true);

      RADIO_windrose_merge_type = glui_device->add_radiogroup_to_panel(PANEL_windrose_merge, &windrose_merge_type, WINDROSE_DT, DeviceCB);
      glui_device->add_radiobutton_to_group(RADIO_windrose_merge_type, _("point"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_merge_type, _("stepped"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_merge_type, _("sliding"));

      RADIO_windrose_ttype = glui_device->add_radiogroup_to_panel(PANEL_windrose_merget, &windrose_ttype, WINDROSE_SETPOS, DeviceCB);
      glui_device->add_radiobutton_to_group(RADIO_windrose_ttype, _("use dt"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_ttype, _("use tmin/tmax"));
      glui_device->add_radiobutton_to_group(RADIO_windrose_ttype, _("neither"));

      BUTTON_update_windrose = glui_device->add_button_to_panel(ROLLOUT_show_windrose2, _("Update"), WINDROSE_UPDATE, DeviceCB);

      if(windrose_xy_active==1||windrose_xz_active==1||windrose_yz_active==1){
        PANEL_orientation = glui_device->add_panel_to_panel(ROLLOUT_windrose, "orientation", true);
        if(windrose_xy_active==1)glui_device->add_checkbox_to_panel(PANEL_orientation, "xy", &windrose_xy_vis);
        if(windrose_xz_active==1)glui_device->add_checkbox_to_panel(PANEL_orientation, "xz", &windrose_xz_vis);
        if(windrose_yz_active==1)glui_device->add_checkbox_to_panel(PANEL_orientation, "yz", &windrose_yz_vis);
      }

      if(nzwindtreeinfo>0){
        int icheckboxes;

        ROLLOUT_showhide_windrose = glui_device->add_rollout_to_panel(ROLLOUT_windrose, "trees", false);
        INSERT_ROLLOUT(ROLLOUT_showhide_windrose, glui_device);

        SPINNER_windrose_first = glui_device->add_spinner_to_panel(ROLLOUT_showhide_windrose, _("first"), GLUI_SPINNER_INT, &windrose_first, WINDROSE_SHOW_FIRST, DeviceCB);
        SPINNER_windrose_next = glui_device->add_spinner_to_panel(ROLLOUT_showhide_windrose, _("skip"), GLUI_SPINNER_INT, &windrose_next, WINDROSE_SHOW_NEXT, DeviceCB);

        NewMemory((void **)&ROLLOUT_showz_windrose, nzwindtreeinfo*sizeof(GLUI_Rollout *));

        nwindrosez_checkboxes = 0;
        for(i = 0; i<nzwindtreeinfo; i++){
          treedevicedata *treei;
          int j;

          treei = zwindtreeinfo[i];
          for(j = treei->first; j<=treei->last; j++){
            vdevicesortdata *vdevsorti;
            vdevicedata *vd;

            vdevsorti = vdevices_sorted+j;
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
        UpdateWindRoseDevices(UPDATE_WINDROSE_DEVICE);

        icheckboxes = 0;
        for(i = 0; i<nzwindtreeinfo; i++){
          char roselabel[1024], xlabel[256], ylabel[256];
          float *xyz;
          treedevicedata *treei;
          int j;

          treei = zwindtreeinfo[i];
          xyz = treei->xyz;

          sprintf(xlabel, "%f", xyz[0]);
          sprintf(ylabel, "%f", xyz[1]);
          TrimZeros(xlabel);
          TrimZeros(ylabel);
          sprintf(roselabel, "x=%s, y=%s", xlabel, ylabel);
          ROLLOUT_showz_windrose[i] = glui_device->add_rollout_to_panel(ROLLOUT_showhide_windrose, roselabel, false);
          INSERT_ROLLOUT(ROLLOUT_showz_windrose[i], glui_device);
          glui_device->add_button_to_panel(ROLLOUT_showz_windrose[i], _("Show all"), WINDROSE_SHOWHIDEALL+i, DeviceCB);
          glui_device->add_button_to_panel(ROLLOUT_showz_windrose[i], _("Hide all"), WINDROSE_SHOWHIDEALL+nzwindtreeinfo+i, DeviceCB);

          for(j = treei->first; j<=treei->last; j++){
            vdevicesortdata *vdevsorti;
            vdevicedata *vd;

            vdevsorti = vdevices_sorted+j;
            vd = vdevsorti->vdeviceinfo;
            xyz = NULL;
            if(xyz==NULL&&vd->udev!=NULL)xyz = vd->udev->xyz;
            if(xyz==NULL&&vd->vdev!=NULL)xyz = vd->vdev->xyz;
            if(xyz==NULL&&vd->wdev!=NULL)xyz = vd->wdev->xyz;
            if(xyz==NULL&&vd->veldev!=NULL)xyz = vd->veldev->xyz;
            if(xyz==NULL&&vd->angledev!=NULL)xyz = vd->angledev->xyz;
            if(xyz!=NULL&&vdevsorti->dir==ZDIR&&vd->unique!=0){
              char zlabel[256];

              sprintf(zlabel, "z=%f", xyz[2]);
              TrimZeros(zlabel);
              CHECKBOX_showz_windrose[icheckboxes++] = glui_device->add_checkbox_to_panel(ROLLOUT_showz_windrose[i], zlabel, &vd->display);
            }
          }
        }
      }

      ROLLOUT_scale_windrose = glui_device->add_rollout_to_panel(ROLLOUT_windrose, _("scale"), false);
      INSERT_ROLLOUT(ROLLOUT_scale_windrose, glui_device);
      glui_device->add_checkbox_to_panel(ROLLOUT_scale_windrose, _("show scale"), &showref_windrose);
      glui_device->add_checkbox_to_panel(ROLLOUT_scale_windrose, _("show labels"), &showlabels_windrose);
      RADIO_scale_windrose = glui_device->add_radiogroup_to_panel(ROLLOUT_scale_windrose, &scale_windrose);
      glui_device->add_radiobutton_to_group(RADIO_scale_windrose, _("local"));
      glui_device->add_radiobutton_to_group(RADIO_scale_windrose, _("global"));
      SPINNER_scale_increment_windrose = glui_device->add_spinner_to_panel(ROLLOUT_scale_windrose, _("increment"), GLUI_SPINNER_INT, &scale_increment_windrose);
      SPINNER_scale_increment_windrose->set_int_limits(1, 50);
      SPINNER_scale_max_windrose = glui_device->add_spinner_to_panel(ROLLOUT_scale_windrose, _("max"), GLUI_SPINNER_INT, &scale_max_windrose);
      SPINNER_scale_max_windrose->set_int_limits(0, 100);


      UpdateGluiDevices();
    }
  }
  if(nhrrinfo>0||ndevicetypes>0||nsliceinfo>0){
    int i;
    ROLLOUT_device2Dplots = glui_device->add_rollout(_("2D plots"), false, PLOT2D_ROLLOUT, Device_Rollout_CB);
    INSERT_ROLLOUT(ROLLOUT_device2Dplots, glui_device);
    ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_device2Dplots, PLOT2D_ROLLOUT, glui_device);

#ifdef pp_PLOT2D_NEW
    if(nhrrinfo>0||ndevicetypes>0){
      ROLLOUT_plotgeneral = glui_device->add_rollout_to_panel(ROLLOUT_device2Dplots, "device+hrr", false);
        if(ndevicetypes>0){
        PANEL_plotgeneral_device = glui_device->add_panel_to_panel(ROLLOUT_plotgeneral, "device data");
        PANEL_plotdevice_select = glui_device->add_panel_to_panel(PANEL_plotgeneral_device, "", false);
        LIST_devID1 = glui_device->add_listbox_to_panel(PANEL_plotdevice_select, "select device:", &deviceID1_index, GENPLOT_devID1, GenPlotCB);
        glui_device->add_column_to_panel(PANEL_plotdevice_select,false);
        for(i = 0; i < ndeviceinfo; i++){
          devicedata *devicei;

          devicei = deviceinfo + i;
          devicei->inlist1 = 1;
          LIST_devID1->add_item(i, devicei->deviceID);
        }
        devicetypes_index = CLAMP(devicetypes_index, 0, ndevicetypes-1);
        LIST_devtype1 = glui_device->add_listbox_to_panel(PANEL_plotdevice_select, "device types:", &devtype1_index, GENPLOT_devtype1, GenPlotCB);
        LIST_devtype1->add_item(-1, "All");
        for(i = 0; i<ndevicetypes; i++){
          LIST_devtype1->add_item(i, devicetypes[i]->quantity);
        }
        BUTTON_add_dev = glui_device->add_button_to_panel(PANEL_plotgeneral_device, _("Add to plot"), GENPLOT_ADDDEV1, GenPlotCB);
        GenPlotCB(GENPLOT_devtype1);
        GenPlotCB(GENPLOT_devID1);
      }

      if(nhrrinfo>0){
        PANEL_plotgeneral_hrr = glui_device->add_panel_to_panel(ROLLOUT_plotgeneral, "hrr data");
        LIST_hrr1 = glui_device->add_listbox_to_panel(PANEL_plotgeneral_hrr, "select hrr quantity:", &hrr1_index, GENPLOT_HRR1, GenPlotCB);
        for(i = 0; i<nhrrinfo+nhrrhcinfo; i++){
          hrrdata *hi;

          hi = hrrinfo+i;
          if(hi->label.shortlabel!=NULL){
            if(strcmp(hi->label.shortlabel, "Time")==0)continue;
            LIST_hrr1->add_item(i, hi->label.shortlabel);
          }
        }
        BUTTON_add_hrr = glui_device->add_button_to_panel(PANEL_plotgeneral_hrr, _("Add to plot"), GENPLOT_ADDHRR1, GenPlotCB);
        GenPlotCB(GENPLOT_HRR1);
      }

      PANEL_plotgeneral_plot = glui_device->add_panel_to_panel(ROLLOUT_plotgeneral, "plot");
      LIST_plotlist1 = glui_device->add_listbox_to_panel(PANEL_plotgeneral_plot, "curve:", &plot_component1, GENPLOT_COMP1,    GenPlotCB);
      LIST_plotlist1->add_item(-1, "");
      for(i = 0; i < plot2dinfo->ncurve_index_ini; i++){
        int curv_index;

        curv_index = plot2dinfo->curve_index_ini[i];
        if(curv_index < ndeviceinfo){
          LIST_devID1->set_int_val(curv_index);
          AddPlot2D(PLOT2D_DEV);
        }
        else{
          LIST_hrr1->set_int_val(curv_index-ndeviceinfo);
          AddPlot2D(PLOT2D_HRR);
        }
        GenPlotCB(GENPLOT_COMP1);
      }

      BUTTON_remove_plot = glui_device->add_button_to_panel(PANEL_plotgeneral_plot, _("Remove from plot"),                      GENPLOT_CLEAR1,   GenPlotCB);
      glui_device->add_button_to_panel(PANEL_plotgeneral_plot, _("Remove all from plot"),                     GENPLOT_CLEARALL, GenPlotCB);
      PANEL_plotgeneral_position = glui_device->add_panel_to_panel(PANEL_plotgeneral_plot, "plot position");
      SPINNER_genplot_x = glui_device->add_spinner_to_panel(PANEL_plotgeneral_position, "x", GLUI_SPINNER_FLOAT, genplot_xyz+0, GENPLOT_XYZ, GenPlotCB);
      SPINNER_genplot_y = glui_device->add_spinner_to_panel(PANEL_plotgeneral_position, "y", GLUI_SPINNER_FLOAT, genplot_xyz+1, GENPLOT_XYZ, GenPlotCB);
      SPINNER_genplot_z = glui_device->add_spinner_to_panel(PANEL_plotgeneral_position, "z", GLUI_SPINNER_FLOAT, genplot_xyz+2, GENPLOT_XYZ, GenPlotCB);

      float plot_xyz_delta;
      plot_xyz_delta = MAX(ABS(xbarFDS - xbar0FDS), ABS(ybarFDS - ybar0FDS));
      plot_xyz_delta = MAX(ABS(zbarFDS - zbar0FDS), plot_xyz_delta)/5.0;
      SPINNER_genplot_x->set_float_limits(xbar0FDS - plot_xyz_delta, xbarFDS + plot_xyz_delta);
      SPINNER_genplot_y->set_float_limits(ybar0FDS - plot_xyz_delta, ybarFDS + plot_xyz_delta);
      SPINNER_genplot_z->set_float_limits(zbar0FDS - plot_xyz_delta, zbarFDS + plot_xyz_delta);
      CHECKBOX_show_genplot = glui_device->add_checkbox_to_panel(PANEL_plotgeneral_plot,"show plot", &show_genplot1,  GENPLOT_SHOW1, GenPlotCB);
      GenPlotCB(GENPLOT_devtype1);
      GenPlotCB(GENPLOT_SHOW1);
    }
#endif

    if(ndevicetypes>0){
      ROLLOUT_plotdevice = glui_device->add_rollout_to_panel(ROLLOUT_device2Dplots, "device", false);
      RADIO_vis_device_plot = glui_device->add_radiogroup_to_panel(ROLLOUT_plotdevice, &vis_device_plot, SHOWDEVICEPLOT, DeviceCB);
      glui_device->add_radiobutton_to_group(RADIO_vis_device_plot, "hide");
      glui_device->add_radiobutton_to_group(RADIO_vis_device_plot, "show selected");
      glui_device->add_radiobutton_to_group(RADIO_vis_device_plot, "show all");
      glui_device->add_radiobutton_to_group(RADIO_vis_device_plot, "show all (trees)");
      devicetypes_index = CLAMP(devicetypes_index, 0, ndevicetypes-1);
      LIST_devicetypes = glui_device->add_listbox_to_panel(ROLLOUT_plotdevice, "type:", &devicetypes_index, DEVICE_devicetypes, DeviceCB);
      for(i = 0; i<ndevicetypes; i++){
        LIST_devicetypes->add_item(i, devicetypes[i]->quantity);
      }
      glui_device->add_spinner_to_panel(ROLLOUT_plotdevice, _("x offset"), GLUI_SPINNER_FLOAT, plot2d_xyz_offset);
      glui_device->add_spinner_to_panel(ROLLOUT_plotdevice, _("y offset"), GLUI_SPINNER_FLOAT, plot2d_xyz_offset+1);
      glui_device->add_spinner_to_panel(ROLLOUT_plotdevice, _("z offset"), GLUI_SPINNER_FLOAT, plot2d_xyz_offset+2);

      ROLLOUT_values = glui_device->add_rollout_to_panel(ROLLOUT_plotdevice, "values", false);
      CHECKBOX_showdevice_val = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Values"), &showdevice_val,  SHOWDEVICEVALS,  DeviceCB);
      CHECKBOX_device_1 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Id"),           &showdevice_id,   SHOWDEVICEVALS,  DeviceCB);
      CHECKBOX_device_5 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Type"),         &showdevice_type, SHOWDEVICEVALS,  DeviceCB);
      CHECKBOX_device_6 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Unit"),         &showdevice_unit, SHOWDEVICEVALS,  DeviceCB);
      CHECKBOX_device_4 = glui_device->add_checkbox_to_panel(ROLLOUT_values, _("Color"),        &colordevice_val, COLORDEVICEVALS, DeviceCB);
      glui_device->add_spinner_to_panel(ROLLOUT_values, "min", GLUI_SPINNER_FLOAT, &device_valmin);
      glui_device->add_spinner_to_panel(ROLLOUT_values, "max", GLUI_SPINNER_FLOAT, &device_valmax);
    }

    if(nhrrinfo>0){
      ROLLOUT_plothrr = glui_device->add_rollout_to_panel(ROLLOUT_device2Dplots, "hrr", false);
      CHECKBOX_vis_hrr_plot = glui_device->add_checkbox_to_panel(ROLLOUT_plothrr, _("show"), &vis_hrr_plot, HRRPUV2_PLOT, DeviceCB);
      LIST_hrrdata = glui_device->add_listbox_to_panel(ROLLOUT_plothrr, "type:", &glui_hrr, DEVICE_TIMEAVERAGE, DeviceCB);
      for(i = 0; i<nhrrinfo+nhrrhcinfo; i++){
        hrrdata *hi;

        hi = hrrinfo+i;
        if(hi->label.shortlabel!=NULL){
          if(strcmp(hi->label.shortlabel, "Time")==0)continue;
          LIST_hrrdata->add_item(i, hi->label.shortlabel);
        }
      }
      LIST_hrrdata->set_int_val(glui_hrr);

      PANEL_hrr_min = glui_device->add_panel_to_panel(ROLLOUT_plothrr, "",false);
      glui_device->add_spinner_to_panel(PANEL_hrr_min, "min", GLUI_SPINNER_FLOAT, &plot2d_hrr_min);
      glui_device->add_column_to_panel(PANEL_hrr_min,false);
      glui_device->add_checkbox_to_panel(PANEL_hrr_min, "set", &use_plot2d_hrr_min);

      PANEL_hrr_max = glui_device->add_panel_to_panel(ROLLOUT_plothrr, "",false);
      glui_device->add_spinner_to_panel(PANEL_hrr_max, "max", GLUI_SPINNER_FLOAT, &plot2d_hrr_max);
      glui_device->add_column_to_panel(PANEL_hrr_max,false);
      glui_device->add_checkbox_to_panel(PANEL_hrr_max, "set", &use_plot2d_hrr_max);

      if(have_mlr==1){
        SPINNER_fuel_hoc = glui_device->add_spinner_to_panel(ROLLOUT_plothrr, _("HOC (kJ/kg)"), GLUI_SPINNER_FLOAT, &fuel_hoc, FUEL_HOC, DeviceCB);
        glui_device->add_checkbox_to_panel(ROLLOUT_plothrr, _("HRR and HOC*MLR_..."), &hoc_hrr);
        BUTTON_reset_fuel_hoc = glui_device->add_button_to_panel(ROLLOUT_plothrr, _("Reset HOC"), RESET_FUEL_HOC, DeviceCB);
      }
    }
    if(nsliceinfo>0){
      ROLLOUT_plotslice = glui_device->add_rollout_to_panel(ROLLOUT_device2Dplots, "slice", false);
      glui_device->add_checkbox_to_panel(ROLLOUT_plotslice, _("show"), &vis_slice_plot, SLICE_PLOT, DeviceCB);
      glui_device->add_checkbox_to_panel(ROLLOUT_plotslice, _("slice bounds"), &slice_plot_bound_option, SLICE_PLOT, DeviceCB);
      SPINNER_slice_x = glui_device->add_spinner_to_panel(ROLLOUT_plotslice, "x", GLUI_SPINNER_FLOAT, slice_xyz+0, SLICE_PLOT, DeviceCB);
      SPINNER_slice_y = glui_device->add_spinner_to_panel(ROLLOUT_plotslice, "y", GLUI_SPINNER_FLOAT, slice_xyz+1, SLICE_PLOT, DeviceCB);
      SPINNER_slice_z = glui_device->add_spinner_to_panel(ROLLOUT_plotslice, "z", GLUI_SPINNER_FLOAT, slice_xyz+2, SLICE_PLOT, DeviceCB);
      SPINNER_slice_x->set_float_limits(xbar0FDS, xbarFDS);
      SPINNER_slice_y->set_float_limits(ybar0FDS, ybarFDS);
      SPINNER_slice_z->set_float_limits(zbar0FDS, zbarFDS);
    }

    ROLLOUT_plotproperties = glui_device->add_rollout_to_panel(ROLLOUT_device2Dplots, "properties", false);
    glui_device->add_checkbox_to_panel(ROLLOUT_plotproperties, _("plot labels"), &showd_plot2d_labels);
    SPINNER_size_factor = glui_device->add_spinner_to_panel(ROLLOUT_plotproperties, _("size factor"), GLUI_SPINNER_FLOAT, &plot2d_size_factor);
                          glui_device->add_spinner_to_panel(ROLLOUT_plotproperties, _("font spacing"), GLUI_SPINNER_FLOAT, &plot2d_font_spacing);
    SPINNER_size_factor->set_float_limits(0.0, 1.0);
    glui_device->add_spinner_to_panel(ROLLOUT_plotproperties, _("line width"), GLUI_SPINNER_FLOAT, &plot2d_line_width);
    glui_device->add_spinner_to_panel(ROLLOUT_plotproperties, _("point size"), GLUI_SPINNER_FLOAT, &plot2d_point_size);
    {
      float dev_tmax;

      dev_tmax = GetDeviceTminTmax();
      SPINNER_device_time_average = glui_device->add_spinner_to_panel(ROLLOUT_plotproperties, _("smoothing interval (s)"), GLUI_SPINNER_FLOAT, &device_time_average, DEVICE_TIMEAVERAGE, DeviceCB);
      SPINNER_device_time_average->set_float_limits(0.0, dev_tmax);
    }
  }

  PANEL_label3 = glui_device->add_panel("",false);
  glui_device->add_column_to_panel(PANEL_label3,false);

  BUTTON_device_1=glui_device->add_button_to_panel(PANEL_label3,_("Save settings"),SAVE_SETTINGS_OBJECTS,DeviceCB);
  glui_device->add_column_to_panel(PANEL_label3,false);

  BUTTON_device_2=glui_device->add_button_to_panel(PANEL_label3,_("Close"),DEVICE_close,DeviceCB);
#ifdef pp_CLOSEOFF
  BUTTON_device_2->disable();
#endif

  glui_device->set_main_gfx_window( main_window );
}

/* ------------------ HideGluiDevice ------------------------ */

extern "C" void HideGluiDevice(void){
  CloseRollouts(glui_device);
}

/* ------------------ ShowGluiPlotDevice ------------------------ */

extern "C" void ShowGluiPlotDevice(void){
  if(glui_device!=NULL){
    glui_device->show();
    if(ROLLOUT_device2Dplots!=NULL)ROLLOUT_device2Dplots->open();
  }

}

/* ------------------ ShowGlui2DPlots ------------------------ */

extern "C" void ShowGlui2DPlots(void){
  if(glui_device!=NULL&&ROLLOUT_device2Dplots!=NULL){
    glui_device->show();
    ROLLOUT_device2Dplots->open();
  }
}

/* ------------------ ShowGluiDevice ------------------------ */

extern "C" void ShowGluiDevice(void){
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
      gluiopen_nfilelist=GetFileListSize(gluiopen_path_dir,gluiopen_filter);
      if(gluiopen_nfilelist==0){
        LIST_open->add_item(0,"");
      }
      MakeFileList(gluiopen_path_dir, gluiopen_filter,gluiopen_nfilelist,NO,&gluiopen_filelist);
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
      ASSERT(FFALSE);
      break;
  }
}
