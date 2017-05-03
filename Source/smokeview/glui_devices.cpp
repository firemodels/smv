#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

#define DEVICE_sensorsize 20
#define SHOWDEVICEVALS 26
#define COLORDEVICEVALS 27
#define DEVICE_devicetypes 28
#define SAVE_SETTINGS 99
#define DEVICE_close 3
#define DEVICE_show_orientation 4
#define DEVICE_NBUCKETS 5
#define DEVICE_SHOWBEAM 6
#define DEVICE_RADIUS 7
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

GLUI_Button *BUTTON_open_down=NULL ;
GLUI_Button *BUTTON_device_1=NULL;
GLUI_Button *BUTTON_device_2=NULL;

GLUI_Checkbox *CHECKBOX_device_1=NULL;
GLUI_Checkbox *CHECKBOX_device_2=NULL;
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
GLUI_Checkbox **CHECKBOX_show_windrose;

GLUI_EditText *EDIT_filter=NULL;

GLUI_Listbox *LIST_open=NULL;

GLUI_Panel *PANEL_objects=NULL;
GLUI_Panel *PANEL_vectors=NULL;
GLUI_Panel *PANEL_arrow_base=NULL;
GLUI_Panel *PANEL_arrow_height=NULL;
GLUI_Panel *PANEL_devicevis=NULL;
GLUI_Panel *PANEL_label3=NULL;
GLUI_Panel *PANEL_vector_type=NULL;
GLUI_Panel *PANEL_beam=NULL;
GLUI_Panel *PANEL_orientation=NULL;
GLUI_Panel *PANEL_wr1=NULL;
GLUI_Panel *PANEL_show_windrose = NULL;

GLUI_RadioGroup *RADIO_devicetypes=NULL;
GLUI_RadioGroup *RADIO_vectortype=NULL;
GLUI_RadioGroup *RADIO_scale_windrose=NULL;
GLUI_RadioGroup *RADIO_windstate_windrose = NULL;

GLUI_Panel *ROLLOUT_properties = NULL;
GLUI_Panel *ROLLOUT_scale_windrose = NULL;
GLUI_Rollout *ROLLOUT_devicevalues = NULL;
GLUI_Rollout *ROLLOUT_velocityvectors = NULL;
GLUI_Rollout *ROLLOUT_smvobjects=NULL;
GLUI_Rollout *ROLLOUT_arrow_dimensions = NULL;
GLUI_Rollout *ROLLOUT_windrose = NULL;
GLUI_Rollout **ROLLOUT_show_windrose;
GLUI_Rollout *ROLLOUT_trees = NULL;

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

#define OBJECTS_ROLLOUT     0
#define FLOWVECTORS_ROLLOUT 1
#define WINDROSE_ROLLOUT    2
#define DEVICE_ROLLOUT      3

procdata deviceprocinfo[4];
int ndeviceprocinfo = 0;

/* ------------------ Device_Rollout_CB ------------------------ */

void Device_Rollout_CB(int var){
  toggle_rollout(deviceprocinfo, ndeviceprocinfo, var);
}

/* ------------------ UpdateWindRoseDevices ------------------------ */

extern "C" void UpdateWindRoseDevices(int option){
  int i,icheckboxes;

  icheckboxes = 0;
  for(i = 0; i<nztreedeviceinfo; i++){
    treedevicedata *treei;
    int j;

    treei = ztreedeviceinfo[i];
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;
      vdevicedata *vd;
      float *xyz;

      vdevsorti = vdevices_sorted+j;
      vd = vdevsorti->vdeviceinfo;
      xyz = NULL;
      if(xyz==NULL&&vd->udev!=NULL)xyz = vd->udev->xyz;
      if(xyz==NULL&&vd->vdev!=NULL)xyz = vd->vdev->xyz;
      if(xyz==NULL&&vd->wdev!=NULL)xyz = vd->wdev->xyz;
      if(xyz!=NULL&&vdevsorti->dir == ZDIR&&vd->unique != 0){

      // when reading ini file
        if(option==UPDATE_WINDROSE_DEVICE&&windrose_showhide!=NULL){
          vd->display=windrose_showhide[icheckboxes];
        }

        // when updating checkboxes
        if(option==UPDATE_WINDROSE_CHECKBOX&&CHECKBOX_show_windrose!=NULL&&
             CHECKBOX_show_windrose[icheckboxes]!=NULL){
          CHECKBOX_show_windrose[icheckboxes]->set_int_val(vd->display);
        }

        // when writing ini file
        if(option==UPDATE_WINDROSE_SHOWHIDE&&windrose_showhide!=NULL){
          windrose_showhide[icheckboxes]=vd->display;
        }
        icheckboxes++;
      }
    }
  }
  nwindrose_showhide=icheckboxes;
  update_windrose_showhide = 0;
  if(option == UPDATE_WINDROSE_SHOWHIDE&&windrose_showhide == NULL){
    NewMemory((void **)&windrose_showhide, nwindrose_showhide * sizeof(int));
    if(windrose_showhide!=NULL)UpdateWindRoseDevices(UPDATE_WINDROSE_SHOWHIDE);
  }
}

/* ------------------ UpdateShowbeamAsLine ------------------------ */

extern "C" void UpdateShowbeamAsLine(void){
  if(CHECKBOX_showbeam_as_line!=NULL)CHECKBOX_showbeam_as_line->set_int_val(showbeam_as_line);
}

/* ------------------ update_device_size ------------------------ */

extern "C" void update_device_size(void){
  if(sensorrelsize<sensorrelsizeMIN)sensorrelsize = sensorrelsizeMIN;
  if(SPINNER_sensorrelsize!=NULL)SPINNER_sensorrelsize->set_float_val(sensorrelsize);
}

/* ------------------ update_device_orientation ------------------------ */

extern "C" void update_device_orientation(void){
  if(CHECKBOX_device_orientation!=NULL)CHECKBOX_device_orientation->set_int_val(show_device_orientation);
}

/* ------------------ Device_CB ------------------------ */

void Device_CB(int var){
  int i;

  updatemenu = 1;
  if(var>=WINDROSE_SHOWHIDEALL&&var<WINDROSE_SHOWHIDEALL+nztreedeviceinfo){
    int iztree, j;
    treedevicedata *treei;

    iztree = var-WINDROSE_SHOWHIDEALL;
    treei = ztreedeviceinfo[iztree];
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
  if(var>=WINDROSE_SHOWHIDEALL+nztreedeviceinfo&&var<WINDROSE_SHOWHIDEALL+2*nztreedeviceinfo){
    int iztree, j;
    treedevicedata *treei;

    iztree = var-WINDROSE_SHOWHIDEALL - nztreedeviceinfo;
    treei = ztreedeviceinfo[iztree];
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
    DeviceData2WindRose(nr_windrose, ntheta_windrose, NOT_FIRST_TIME);
    break;
  case DEVICE_show_orientation:
    updatemenu = 1;
    break;
  case DEVICE_devicetypes:
    for(i = 0;i < ndevicetypes;i++){
      devicetypes[i]->type2vis = 0;
    }
    if(ndevicetypes > 0){
      devicetypes[devicetypes_index]->type2vis = 1;
      update_colordevs();
    }
    break;
  case SHOWDEVICEVALS:
  case COLORDEVICEVALS:
    if(PANEL_devicevis != NULL){
      if(colordeviceval == 1 || showdeviceval == 1){
        PANEL_devicevis->enable();
      }
      else{
        PANEL_devicevis->disable();
      }
    }

    break;
  case DEVICE_sensorsize:
    if(sensorrelsize < sensorrelsizeMIN){
      sensorrelsize = sensorrelsizeMIN;
      if(SPINNER_sensorrelsize != NULL){
        SPINNER_sensorrelsize->set_float_val(sensorrelsize);
      }
    }
    break;
  case SAVE_SETTINGS:
    WriteINI(LOCAL_INI, NULL);
    break;
  case DEVICE_close:
    hide_glui_device();
    break;
  default:
    ASSERT(FFALSE);
  }
}

/* ------------------ update_glui_devices ------------------------ */

extern "C" void update_glui_devices(void){
  Device_CB(SHOWDEVICEVALS);
  Device_CB(COLORDEVICEVALS);
  Device_CB(DEVICE_devicetypes);
}

/* ------------------ glui_device_setup ------------------------ */

extern "C" void glui_device_setup(int main_window){

  update_glui_device=0;
  if(glui_device!=NULL){
    glui_device->close();
    glui_device=NULL;
  }
  glui_device = GLUI_Master.create_glui("Device/Objects",0,0,0);
  glui_device->hide();

  if(ndeviceinfo>0){
    int i;

    PANEL_objects = glui_device->add_panel("Devices/Objects",false);

    ROLLOUT_smvobjects = glui_device->add_rollout_to_panel(PANEL_objects,"Objects",false, OBJECTS_ROLLOUT, Device_Rollout_CB);
    ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_smvobjects, OBJECTS_ROLLOUT);

    SPINNER_sensorrelsize=glui_device->add_spinner_to_panel(ROLLOUT_smvobjects,_d("Scale"),GLUI_SPINNER_FLOAT,&sensorrelsize,DEVICE_sensorsize,Device_CB);
    CHECKBOX_device_3=glui_device->add_checkbox_to_panel(ROLLOUT_smvobjects,_d("Outline"),&object_outlines);
    CHECKBOX_device_orientation=glui_device->add_checkbox_to_panel(ROLLOUT_smvobjects,_d("Orientation"),&show_device_orientation,DEVICE_show_orientation,Device_CB);
    SPINNER_orientation_scale=glui_device->add_spinner_to_panel(ROLLOUT_smvobjects,_d("Orientation scale"),GLUI_SPINNER_FLOAT,&orientation_scale);
    SPINNER_orientation_scale->set_float_limits(0.1,10.0);
    if(have_beam){
      PANEL_beam = glui_device->add_panel_to_panel(ROLLOUT_smvobjects, "Beam sensor", true);
      CHECKBOX_showbeam_as_line=glui_device->add_checkbox_to_panel(PANEL_beam,_d("Show beam as line"),&showbeam_as_line,DEVICE_SHOWBEAM,Device_CB);
      SPINNER_beam_line_width = glui_device->add_spinner_to_panel(PANEL_beam, _d("line width"), GLUI_SPINNER_FLOAT, &beam_line_width);
      SPINNER_beam_line_width->set_float_limits(1.0, 20.0);
      CHECKBOX_use_beamcolor = glui_device->add_checkbox_to_panel(PANEL_beam, _d("Use color"), &use_beamcolor);
      SPINNER_beam_color[0] = glui_device->add_spinner_to_panel(PANEL_beam, _d("red"), GLUI_SPINNER_INT, beam_color);
      SPINNER_beam_color[1] = glui_device->add_spinner_to_panel(PANEL_beam, _d("green"), GLUI_SPINNER_INT, beam_color+1);
      SPINNER_beam_color[2] = glui_device->add_spinner_to_panel(PANEL_beam, _d("blue"), GLUI_SPINNER_INT, beam_color+2);
      SPINNER_beam_color[0]->set_int_limits(0,255);
      SPINNER_beam_color[1]->set_int_limits(0, 255);
      SPINNER_beam_color[2]->set_int_limits(0, 255);
    }

    if(GetNumActiveDevices()>0||isZoneFireModel==1){
      ROLLOUT_velocityvectors = glui_device->add_rollout_to_panel(PANEL_objects, "Flow vectors", false, FLOWVECTORS_ROLLOUT, Device_Rollout_CB);
      ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_velocityvectors, FLOWVECTORS_ROLLOUT);

      if(nvdeviceinfo==0)ROLLOUT_velocityvectors->disable();
      CHECKBOX_device_1=glui_device->add_checkbox_to_panel(ROLLOUT_velocityvectors,_d("Show"),&showvdeviceval);
      PANEL_vector_type=glui_device->add_panel_to_panel(ROLLOUT_velocityvectors,"type",true);
      RADIO_vectortype=glui_device->add_radiogroup_to_panel(PANEL_vector_type,&vectortype);
      glui_device->add_radiobutton_to_group(RADIO_vectortype,"line");
      glui_device->add_radiobutton_to_group(RADIO_vectortype,"arrow");
      glui_device->add_radiobutton_to_group(RADIO_vectortype,"object");
      glui_device->add_radiobutton_to_group(RADIO_vectortype, "profile");
      ROLLOUT_arrow_dimensions = glui_device->add_rollout_to_panel(ROLLOUT_velocityvectors, "Dimensions", false);
      PANEL_arrow_base=glui_device->add_panel_to_panel(ROLLOUT_arrow_dimensions,"base",true);
      glui_device->add_spinner_to_panel(PANEL_arrow_base,_d("length"),GLUI_SPINNER_FLOAT,&vector_baselength);
      glui_device->add_spinner_to_panel(PANEL_arrow_base,_d("diameter"),GLUI_SPINNER_FLOAT,&vector_basediameter);
      PANEL_arrow_height=glui_device->add_panel_to_panel(ROLLOUT_arrow_dimensions,"head",true);
      glui_device->add_spinner_to_panel(PANEL_arrow_height,_d("length"),GLUI_SPINNER_FLOAT,&vector_headlength);
      glui_device->add_spinner_to_panel(PANEL_arrow_height,_d("diameter"),GLUI_SPINNER_FLOAT,&vector_headdiameter);

      if(ntreedeviceinfo>0){
        ROLLOUT_trees = glui_device->add_rollout_to_panel(ROLLOUT_velocityvectors, "Device trees", false);
        SPINNER_mintreesize = glui_device->add_spinner_to_panel(ROLLOUT_trees, _d("min size"), GLUI_SPINNER_INT, &mintreesize);
        SPINNER_mintreesize->set_int_limits(2, MAX(2, max_device_tree));
        CHECKBOX_vis_xtree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _d("Show x"), &vis_xtree);
        CHECKBOX_vis_ytree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _d("Show y"), &vis_ytree);
        CHECKBOX_vis_ztree = glui_device->add_checkbox_to_panel(ROLLOUT_trees, _d("Show z"), &vis_ztree);
      }

      ROLLOUT_windrose = glui_device->add_rollout_to_panel(PANEL_objects, "Windrose", false, WINDROSE_ROLLOUT, Device_Rollout_CB);
      ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_windrose, WINDROSE_ROLLOUT);


      PANEL_show_windrose = glui_device->add_panel_to_panel(ROLLOUT_windrose, "show", true);
      glui_device->add_checkbox_to_panel(PANEL_show_windrose, _d("show"), &viswindrose);

      PANEL_orientation = glui_device->add_panel_to_panel(PANEL_show_windrose, "orientation", true);
      glui_device->add_checkbox_to_panel(PANEL_orientation, _d("xy"), &visxy_windrose);
      glui_device->add_checkbox_to_panel(PANEL_orientation, _d("xz"), &visxz_windrose);
      glui_device->add_checkbox_to_panel(PANEL_orientation, _d("yz"), &visyz_windrose);

      if(nztreedeviceinfo>0){
        int icheckboxes;

        NewMemory((void **)&ROLLOUT_show_windrose, nztreedeviceinfo*sizeof(GLUI_Rollout *));

        nwindrose_checkboxes=0;
        for(i = 0; i<nztreedeviceinfo; i++){
          treedevicedata *treei;
          int j;

          treei = ztreedeviceinfo[i];
          for(j = treei->first; j <= treei->last; j++){
            vdevicesortdata *vdevsorti;
            vdevicedata *vd;
            float *xyz;

            vdevsorti = vdevices_sorted + j;
            vd = vdevsorti->vdeviceinfo;
            xyz = NULL;
            if(xyz==NULL&&vd->udev!=NULL)xyz = vd->udev->xyz;
            if(xyz==NULL&&vd->vdev!=NULL)xyz = vd->vdev->xyz;
            if(xyz==NULL&&vd->wdev!=NULL)xyz = vd->wdev->xyz;
            if(xyz!=NULL&&vdevsorti->dir == ZDIR&&vd->unique != 0)nwindrose_checkboxes++;
          }
        }
        NewMemory((void **)&CHECKBOX_show_windrose, nwindrose_checkboxes*sizeof(GLUI_Checkbox *));
        for(i=0;i<nwindrose_checkboxes;i++){
          CHECKBOX_show_windrose[i] = NULL;
        }
        UpdateWindRoseDevices(UPDATE_WINDROSE_DEVICE);

        icheckboxes = 0;
        for(i = 0; i<nztreedeviceinfo; i++){
          char roselabel[256], xlabel[256], ylabel[256];
          float *xyz;
          treedevicedata *treei;
          int j;

          treei = ztreedeviceinfo[i];
          xyz = treei->xyz;

          sprintf(xlabel, "%f", xyz[0]);
          sprintf(ylabel, "%f", xyz[1]);
          TrimZeros(xlabel);
          TrimZeros(ylabel);
          sprintf(roselabel, "x=%s, y=%s", xlabel, ylabel);
          ROLLOUT_show_windrose[i] = glui_device->add_rollout_to_panel(PANEL_show_windrose, roselabel, false);
          glui_device->add_button_to_panel(ROLLOUT_show_windrose[i],"Show all",WINDROSE_SHOWHIDEALL+i,Device_CB);
          glui_device->add_button_to_panel(ROLLOUT_show_windrose[i],"Hide all",WINDROSE_SHOWHIDEALL+nztreedeviceinfo+i,Device_CB);

          for(j = treei->first; j<=treei->last; j++){
            vdevicesortdata *vdevsorti;
            vdevicedata *vd;

            vdevsorti = vdevices_sorted+j;
            vd = vdevsorti->vdeviceinfo;
            xyz = NULL;
            if(xyz==NULL&&vd->udev!=NULL)xyz = vd->udev->xyz;
            if(xyz==NULL&&vd->vdev!=NULL)xyz = vd->vdev->xyz;
            if(xyz==NULL&&vd->wdev!=NULL)xyz = vd->wdev->xyz;
            if(xyz!=NULL&&vdevsorti->dir==ZDIR&&vd->unique!=0){
              char zlabel[256];

              sprintf(zlabel, "z=%f", xyz[2]);
              TrimZeros(zlabel);
              CHECKBOX_show_windrose[icheckboxes++] = glui_device->add_checkbox_to_panel(ROLLOUT_show_windrose[i],zlabel,&vd->display);
            }
          }
        }
      }

      ROLLOUT_properties=glui_device->add_rollout_to_panel(ROLLOUT_windrose,"properties",false);
      SPINNER_nr_windrose = glui_device->add_spinner_to_panel(ROLLOUT_properties, _d("radii"), GLUI_SPINNER_INT, &nr_windrose, DEVICE_NBUCKETS, Device_CB);
      SPINNER_nr_windrose->set_int_limits(3, 72, GLUI_LIMIT_CLAMP);
      SPINNER_ntheta_windrose = glui_device->add_spinner_to_panel(ROLLOUT_properties, _d("angles"), GLUI_SPINNER_INT, &ntheta_windrose, DEVICE_NBUCKETS, Device_CB);
      SPINNER_ntheta_windrose->set_int_limits(3, 72, GLUI_LIMIT_CLAMP);
      SPINNER_radius_windrose = glui_device->add_spinner_to_panel(ROLLOUT_properties, _d("radius"), GLUI_SPINNER_FLOAT, &radius_windrose, DEVICE_RADIUS, Device_CB);
      RADIO_windstate_windrose = glui_device->add_radiogroup_to_panel(ROLLOUT_properties, &windstate_windrose);
      glui_device->add_radiobutton_to_group(RADIO_windstate_windrose, "heading");
      glui_device->add_radiobutton_to_group(RADIO_windstate_windrose, "direction");

      ROLLOUT_scale_windrose=glui_device->add_rollout_to_panel(ROLLOUT_windrose,"scale",false);
      glui_device->add_checkbox_to_panel(ROLLOUT_scale_windrose, _d("show scale"), &showref_windrose);
      glui_device->add_checkbox_to_panel(ROLLOUT_scale_windrose, _d("show labels"), &showlabels_windrose);
      RADIO_scale_windrose=glui_device->add_radiogroup_to_panel(ROLLOUT_scale_windrose,&scale_windrose);
      glui_device->add_radiobutton_to_group(RADIO_scale_windrose,"local");
      glui_device->add_radiobutton_to_group(RADIO_scale_windrose,"global");
      SPINNER_scale_increment_windrose = glui_device->add_spinner_to_panel(ROLLOUT_scale_windrose, _d("increment"), GLUI_SPINNER_INT, &scale_increment_windrose);
      SPINNER_scale_increment_windrose->set_int_limits(1, 50);
      SPINNER_scale_max_windrose = glui_device->add_spinner_to_panel(ROLLOUT_scale_windrose, _d("max"), GLUI_SPINNER_INT, &scale_max_windrose);
      SPINNER_scale_max_windrose->set_int_limits(0, 100);

      ROLLOUT_devicevalues = glui_device->add_rollout_to_panel(PANEL_objects,"Device values",false, DEVICE_ROLLOUT, Device_Rollout_CB);
      ADDPROCINFO(deviceprocinfo, ndeviceprocinfo, ROLLOUT_devicevalues, DEVICE_ROLLOUT);


      CHECKBOX_device_2 = glui_device->add_checkbox_to_panel(ROLLOUT_devicevalues, _d("Values"), &showdeviceval, SHOWDEVICEVALS, Device_CB);
      CHECKBOX_device_5 = glui_device->add_checkbox_to_panel(ROLLOUT_devicevalues, _d("Type"), &showdevicetype, SHOWDEVICEVALS, Device_CB);
      CHECKBOX_device_6 = glui_device->add_checkbox_to_panel(ROLLOUT_devicevalues, _d("Unit"), &showdeviceunit, SHOWDEVICEVALS, Device_CB);
      CHECKBOX_device_4=glui_device->add_checkbox_to_panel(ROLLOUT_devicevalues,_d("Color"),&colordeviceval,COLORDEVICEVALS,Device_CB);
      glui_device->add_spinner_to_panel(ROLLOUT_devicevalues,"min",GLUI_SPINNER_FLOAT,&device_valmin);
      glui_device->add_spinner_to_panel(ROLLOUT_devicevalues,"max",GLUI_SPINNER_FLOAT,&device_valmax);

      PANEL_devicevis=glui_device->add_panel_to_panel(ROLLOUT_devicevalues,"",false);
      devicetypes_index=CLAMP(devicetypes_index,0,ndevicetypes-1);
      RADIO_devicetypes=glui_device->add_radiogroup_to_panel(PANEL_devicevis,&devicetypes_index,DEVICE_devicetypes,Device_CB);
      for(i=0;i<ndevicetypes;i++){
        glui_device->add_radiobutton_to_group(RADIO_devicetypes,devicetypes[i]->quantity);
      }

      update_glui_devices();
    }
  }

  PANEL_label3 = glui_device->add_panel("",false);
  glui_device->add_column_to_panel(PANEL_label3,false);

  BUTTON_device_1=glui_device->add_button_to_panel(PANEL_label3,_d("Save settings"),SAVE_SETTINGS,Device_CB);
  glui_device->add_column_to_panel(PANEL_label3,false);

  BUTTON_device_2=glui_device->add_button_to_panel(PANEL_label3,_d("Close"),DEVICE_close,Device_CB);

  glui_device->set_main_gfx_window( main_window );
}

/* ------------------ hide_glui_device ------------------------ */

extern "C" void hide_glui_device(void){
  if(glui_device!=NULL)glui_device->hide();
  updatemenu=1;
}

/* ------------------ show_glui_device ------------------------ */

extern "C" void show_glui_device(void){
  if(glui_device!=NULL)glui_device->show();
}

/* ------------------ Open_CB ------------------------ */

void Open_CB(int var){
  int i;
  filelistdata *filei;
  char *open_filter_ptr;


  switch(var){
    case OPEN_UP:
      strcat(gluiopen_path_dir,dirseparator);
      strcat(gluiopen_path_dir,"..");
      Open_CB(OPEN_UPDATE_LIST);
      break;
    case OPEN_DOWN:
      if(gluiopen_filelist==NULL)break;
      filei = gluiopen_filelist + gluiopen_file_index;
      if(filei->type==1){
        strcat(gluiopen_path_dir,dirseparator);
        strcat(gluiopen_path_dir,filei->file);
        Open_CB(OPEN_UPDATE_LIST);
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
        Open_CB(OPEN_UPDATE_LIST);
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
      Open_CB(OPEN_UPDATE_LIST);
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
