#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#endif
#include GLUT_H

#include "string_util.h"
#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "IOvolsmoke.h"
#include "readhvac.h"
#include "readobject.h"
#include "readsmvfile.h"
#include "IOobjects.h"
#include "IOscript.h"
#include "viewports.h"
#include "colorbars.h"
#include "paths.h"

void LoadHVACMenu(int value);
void LoadPlot2DMenu(int value);

int       part_file_count;
FILE_SIZE part_load_size;
float     part_load_time;

#ifdef WIN32
#include <direct.h>
#endif

#define ABOUT_DATA_TRANSFER_TEST      2

#define GEOM_Vents                   15
#define GEOM_Compartments            16
#define GEOM_OutlineA                -1
#define GEOM_OutlineB                -2
#define GEOM_OutlineC                -3
#define GEOM_TriangleCount           14
#define GEOM_ShowAll                 11
#define GEOM_HideAll                 13
#define GEOM_BOUNDING_BOX_MOUSE_DOWN  9
#define SKY_OUTLINE                   4

#define MENU_TERRAIN_SHOW_SURFACE      -1
#define MENU_TERRAIN_SHOW_LINES        -2
#define MENU_TERRAIN_SHOW_POINTS       -3
#define MENU_TERRAIN_BOUNDING_BOX_AUTO -6

#define MENU_KEEP_ALL -2
#define MENU_KEEP_FINE -3
#define MENU_KEEP_COARSE -4

#define MENU_SLICECOLORDEFER    -5
#define MENU_SPLITSLICES       -10
#define MENU_SPLITSLICES_DEBUG -11

#define MENU_LOAD_SPECIFIED    -12
#define MENU_LOADALL_XorYorZ   -13
#define MENU_LOADALL_XandYandZ -14

#define MENU_SLICE_FILE_SIZES -9

#define MENU_OPTION_TRAINERMENU 2

#define MENU_BNDF_MIRROR              -5
#define MENU_BNDF_OPEN                -9

#define MENU_DUMMY3 -2

#define MENU_ERASECOMPRESS 1
#define MENU_OVERWRITECOMPRESS 2
#define MENU_COMPRESSNOW 3
#define MENU_COMPRESSAUTOLOAD 4
#define MENU_COMPRESS_SETTINGS 5

#define MENU_TRAINER_CLEAR 998
#define MENU_MAIN_QUIT 3

#define MENU_READCASEINI    -1
#define MENU_READINI         1
#define MENU_WRITEINI        2
#define MENU_WRITECASEINI    3
#define MENU_READSVO         4
#define MENU_CONFIG_SETTINGS 5
#define MENU_REVERT_WRITEINI 6

#define MENU_DUMMY2 -1

#define MENU_PLOT3D_DUMMY 997
#define MENU_PLOT3D_Z 1
#define MENU_PLOT3D_Y 2
#define MENU_PLOT3D_X 3
#define MENU_PLOT3D_CONT 4
#define MENU_PLOT3D_SHOWALL 5
#define MENU_PLOT3D_HIDEALL 6

#define MENU_PLOT2D_LOAD    0
#define MENU_PLOT2D_LOADCSV 1
#define MENU_PLOT2D_UNLOAD  2

#define MENU_MAIN_TRAINERTOGGLE 997

#define MENU_UNLOADSMOKE3D_UNLOADALLSOOT -1
#define MENU_UNLOADSMOKE3D_UNLOADALLFIRE -2
#define MENU_UNLOADSMOKE3D_UNLOADALLTEMP -3
#define MENU_UNLOADSMOKE3D_UNLOADALLCO2 -4

#define MENU_UNLOADTERRAIN_UNLOADALL -10
#define MENU_UNLOADTERRAIN_DUMMY -1

#define MENU_LOADTERRAIN_LOADALL -9
#define MENU_LOADTERRAIN_UNLOAD -10
#define MENU_LOADTERRAIN_DUMMY -1

#define MENU_LOADVSLICE_SHOWALL -20

#define MENU_LOADVSLICE_SETTINGS -21
#define MENU_ISO_SETTINGS         -3
#define MENU_BOUNDARY_SETTINGS    -7
#define MENU_PART_SETTINGS        -4
#define MENU_PLOT3D_SETTINGS      -4
#define MENU_VOLSMOKE_SETTINGS    -4
#define MENU_SLICE_SETTINGS       -6
#define MENU_PART_PARTFAST        -7
#define MENU_PART_NUM_FILE_SIZE   -8

#define MENU_PARTICLE_UNLOAD_ALL -1
#define MENU_PARTICLE_DUMMY -2
#define MENU_PARTICLE_ALLMESHES -11

#define MENU_AVATAR_DEFINED -1

#define MENU_PARTSHOW_PARTICLES 1
#define MENU_PARTSHOW_DROPLETS 2
#define MENU_PARTSHOW_SHOWALL 3
#define MENU_PARTSHOW_HIDEALL 4
#define MENU_PARTSHOW_STATIC 5

#define MENU_PROP_DUMMY -1
#define MENU_PROP_SHOWALL -2
#define MENU_PROP_HIDEALL -3
#define MENU_PROP_HIDEPART -4
#define MENU_PROP_HIDEAVATAR -5
#define MENU_PROP_TRACERS -6

#define MENU_STREAK_HIDE -2
#define MENU_STREAK_HEAD -3

#define MENU_VECTOR_SHOW -2

#define MENU_SURFACE_SMOOTH 0
#define MENU_SURFACE_FACET 1
#define MENU_SURFACE_OUTLINE 2
#define MENU_SURFACE_POINTS 3

#define MENU_ISOSHOW_SHOWALL 99
#define MENU_ISOSHOW_HIDEALL 98
#define MENU_ISOSHOW_ALLSOLID 94
#define MENU_ISOSHOW_ALLTRANSPARENT 95
#define MENU_ISOSHOW_MINSOLID 96
#define MENU_ISOSHOW_MAXSOLID 97

#define MENU_ISOSHOW_SOLID   1
#define MENU_ISOSHOW_OUTLINE 2
#define MENU_ISOSHOW_POINTS  3
#define MENU_ISOSHOW_SMOOTH  4
#define MENU_ISOSHOW_NORMALS 5
#define MENU_ISOSHOW_OUTPUT  6

#define MENU_ZONE_2DTEMP2 21
#define MENU_ZONE_2DTEMP 6
#define MENU_ZONE_2DHAZARD 5
#define MENU_ZONE_3DSMOKE 7
#define MENU_ZONE_ZPLANE 1
#define MENU_ZONE_YPLANE 2
#define MENU_ZONE_XPLANE 24
#define MENU_ZONE_LAYERHIDE 4
#define MENU_ZONE_VENTS 14
#define MENU_ZONE_HVENTS 15
#define MENU_ZONE_VVENTS 16
#define MENU_ZONE_MVENTS 17
#define MENU_ZONE_FIRES 18
#define MENU_ZONE_VENT_SLAB 19
#define MENU_ZONE_VENT_PROFILE 20
#define MENU_ZONE_WALLS 22
#define MENU_ZONE_TARGETS 23

#define MENU_SHOWSLICE_IN_GAS -16
#define MENU_SHOWSLICE_IN_GASANDSOLID -17
#define MENU_SHOWSLICE_IN_SOLID -18
#define MENU_SHOWSLICE_NODESLICEANDVECTORS -15
#define MENU_SHOWSLICE_CELLSLICEANDVECTORS -19
#define MENU_SHOWSLICE_TERRAIN -13
#define MENU_SHOWSLICE_OFFSET -12

#define MENU_VENT_OPEN 14
#define MENU_VENT_EXTERIOR 16
#define MENU_VENT_OTHER 21
#define MENU_VENT_OUTLINE 15
#define MENU_VENT_TWOINTERIOR 18
#define MENU_VENT_TWOEXTERIOR 19
#define MENU_VENT_TRANSPARENT 20
#define MENU_VENT_CIRCLE 23
#define MENU_VENT_RECTANGLE 24
#define MENU_VENT_CIRCLEHIDE 25
#define MENU_VENT_CIRCLEOUTLINE 26

#define MENU_TIMEVIEW             -103
#define SAVE_VIEWPOINT            -101
//#define SAVE_CURRENT_VIEWPOINT    -115 movied to smokeviewdefs.h
#define SAVE_VIEWPOINT_AS_STARTUP -106
#define MENU_STARTUPVIEW          -102
#define MENU_OUTLINEVIEW          -104
#define MENU_SIZEPRESERVING       -105
#define MENU_VIEWPOINT_SETTINGS   -107
#define MENU_VIEWPOINT_TOPVIEW    -108
#define MENU_DUMMY                -999
//#define MENU_VIEW_XMIN            -109 moved to smokeviewdefs.h
//#define MENU_VIEW_XMAX            -110
//#define MENU_VIEW_YMIN            -111
//#define MENU_VIEW_YMAX            -112
//#define MENU_VIEW_ZMIN            -113
//#define MENU_VIEW_ZMAX            -114

#define MENU_SHOWHIDE_PRINT 16
#define MENU_SHOWHIDE_PARTICLES 1
#define MENU_SHOWHIDE_SENSOR 9
#define MENU_SHOWHIDE_SENSOR_NORM 14
#define MENU_SHOWHIDE_OFFSET 12

#define MENU_UNITS_RESET -1
#define MENU_UNITS_SHOWALL -3
#define MENU_UNITS_HMS -2

#define MENU_HELP_SMV_ISSUES    -1
#define MENU_HELP_FDS_ISSUES    -2
#define MENU_HELP_DOWNLOADS     -3
#define MENU_HELP_DOCUMENTATION -4
#define MENU_HELP_FDSWEB        -5
#define MENU_HELP_FORUM         -6
#define MENU_HELP_RELEASENOTES  -7

#define GRID_yz 1
#define GRID_xz 2
#define GRID_xy 3
#define GRID_showall 4
#define GRID_hideall 5
#define GRID_grid 7
#define GRID_probe 8

#define OBJECT_SHOWALL              -1
#define OBJECT_HIDEALL              -2
#define OBJECT_SELECT               -3
#define OBJECT_OUTLINE              -4
#define OBJECT_ORIENTATION          -5
#define OBJECT_MISSING              -6
#define OBJECT_SHOWBEAM             -7
#define OBJECT_BOX                 -14
//#define OBJECT_PLOT_SHOW_ALL        -8  put in smokeviewdefs.h
//#define OBJECT_PLOT_SHOW_SELECTED  -11  put in smokeviewdefs.h
#define OBJECT_VALUES               -9
#define MENU_DEVICE_SETTINGS       -10
//#define OBJECT_PLOT_SHOW_TREE_ALL  -12 put in smokeviewdefs.h
//#define PLOT_HRRPUV                -13 put in smokeviewdef.h

#define ISO_COLORS 4

#define MENU_HVAC_CONNECTION_IGNORE    -999
#define MENU_HVAC_SHOWALL_CONNECTIONS  -1
#define MENU_HVAC_HIDEALL_CONNECTIONS  -2
#define MENU_HVAC_CONNECTION_VIEW      -3
#define MENU_HVAC_NETWORK_VIEW         -4

#define MENU_HVAC_SHOW_NODE_IGNORE        -1
#define MENU_HVAC_SHOWALL_NETWORKS        -2
#define MENU_HVAC_HIDEALL_NETWORKS        -3
#define MENU_HVAC_METRO_VIEW              -4
#define MENU_HVAC_DIALOG_HVAC             -5
#define MENU_HVAC_SHOW_COMPONENT_TEXT     -6
#define MENU_HVAC_SHOW_COMPONENT_SYMBOLS  -7
#define MENU_HVAC_SHOW_COMPONENT_HIDE     -8
#define MENU_HVAC_SHOW_FILTER_TEXT        -9
#define MENU_HVAC_SHOW_FILTER_SYMBOLS    -10
#define MENU_HVAC_SHOW_FILTER_HIDE       -11
#define MENU_HVAC_SHOW_DUCT_IDS          -12
#define MENU_HVAC_SHOW_NODE_IDS          -13
// defined in smokeviewdefs.h (so it can also be used in IOscript.c )
//#define MENU_HVAC_HIDE_ALL_VALUES        -15
#define MENU_HVAC_CELL_VIEW              -16

/* ------------------ PrintFileLoadTimes ------------------------ */

void PrintFileLoadTimes(int file_count, FILE_SIZE load_size, float load_time){
  if(file_count > 1){
    float rate;
    char crate[32];
    strcpy(crate, "");
    if(load_time > 0.0){
      rate = load_size * 8.0 / load_time;
      if(rate > 1000000000.0){
        rate /= 1000000000.0;
        sprintf(crate, "%.2f Gbs", rate);
      }
      else if(rate > 1000000.0){
        rate /= 1000000.0;
        sprintf(crate, "%.2f Mbs", rate);
      }
      else{
        rate /= 1000.0;
        sprintf(crate, "%.2f Kbs", rate);
      }
    }
    if(load_size > 1000000000){
      PRINTF("Loaded %.2f GB in %.2f s (%s)\n", (float)load_size / 1000000000., load_time, crate);
    }
    else if(load_size > 1000000){
      PRINTF("Loaded %.2f MB in %.2f s (%s)\n", (float)load_size / 1000000., load_time, crate);
    }
    else{
      PRINTF("Loaded %.2f kB in %.2f s (%s)\n", (float)load_size / 1000., load_time, crate);
    }
    printf("\n");
  }
}

#ifdef WIN32

/* ------------------ OpenSMVFile ------------------------ */

void OpenSMVFile(char *filebuffer,int filebufferlength,int *openfile){
  char stringFilter[]="Smokeview Files (*.smv)\0*.smv\0\0\0";
  char strTitle[80]="Select Smokeview Case";
  int buffersize;
  char smv_directory[1024];
  OPENFILENAME openfinfo;

  *openfile=0;
  buffersize=sizeof(OPENFILENAME);

  STRCPY(filebuffer,"");
  openfinfo.lStructSize=(unsigned long)buffersize;
  openfinfo.hwndOwner=NULL;
  openfinfo.lpstrFilter=stringFilter;
  openfinfo.lpstrCustomFilter=NULL;
  openfinfo.lpstrFile=filebuffer;
  openfinfo.nMaxFile=(unsigned long)filebufferlength;
  openfinfo.lpstrFileTitle=NULL;
  openfinfo.nMaxFileTitle=80;
  openfinfo.lpstrInitialDir=NULL;
  openfinfo.lpstrTitle=strTitle;
  openfinfo.Flags=0;
  openfinfo.lpstrDefExt=NULL;

  if(GetOpenFileName(&openfinfo)){
    STRCPY(smv_directory,"");
    strncat(smv_directory,filebuffer,openfinfo.nFileOffset);
    if( _chdir( smv_directory )  ){
      PRINTF( "Unable to locate the directory: %s\n", smv_directory );
    }
    else{
      *openfile=1;
    }
  }
}
#endif

/* ----------------------- ComparePatchLabels ----------------------------- */

int ComparePatchLabels(const void *arg1, const void *arg2){
  patchdata *patchi, *patchj;
  char *labeli, *labelj;

  patchi = *(patchdata **)arg1;
  patchj = *(patchdata **)arg2;
  labeli = patchi->label.longlabel;
  labelj = patchj->label.longlabel;

  return strcmp(labeli, labelj);
}

/* ----------------------- PrintFileSizes ----------------------------- */

void PrintFileSizes(char *type, float val, float val2){
  char label[100], label2[100], *labelptr=NULL, *labelptr2=NULL;

  if(val>0.0)labelptr=GetFloatFileSizeLabel(val, label);
  if(val2>0.0)labelptr2=GetFloatFileSizeLabel(val2, label2);
  if(val>0.0){
    if(val2>0.0){
      if(labelptr!=NULL&&labelptr2!=NULL)printf("  %s/compressed: %s/%s\n", type,labelptr,labelptr2);
    }
    else{
      if(labelptr != NULL)printf("  %s: %s\n", type,labelptr);
    }
  }
}
/* ------------------ GetFileSizes ------------------------ */

void GetFileSizes(void){
  int i;

  printf("\n");
  if(global_scase.smoke3dcoll.nsmoke3dinfo>0){
    float hrrpuv = 0.0, soot = 0.0, temp = 0.0, co2 = 0.0;
    float hrrpuv2 = 0.0, soot2 = 0.0, temp2 = 0.0, co22 = 0.0;

    for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
      smoke3ddata *smoke3di;
      FILE_SIZE file_size, compressed_file_size;

      smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;

      file_size = GetFileSizeSMV(smoke3di->reg_file);
      compressed_file_size = GetFileSizeSMV(smoke3di->comp_file);

      if(smoke3di->type==SOOT_index){
        soot += file_size;
        soot2 += compressed_file_size;
      }
      else if(smoke3di->type==HRRPUV_index){
        hrrpuv += file_size;
        hrrpuv2 += compressed_file_size;
      }
      else if(smoke3di->type==TEMP_index){
        temp += file_size;
        temp2 += compressed_file_size;
      }
      else if(smoke3di->type==CO2_index){
        co2 += file_size;
        co22 += compressed_file_size;
      }
    }
    printf("3d smoke file sizes:\n");
    PrintFileSizes("soot",soot,soot2);
    PrintFileSizes("hrrpuv",hrrpuv,hrrpuv2);
    PrintFileSizes("temp",temp,temp2);
    PrintFileSizes("co2",co2,co22);
  }
  else{
    printf("3d smoke file sizes: no files found\n");
  }

  printf("\n");
  if(global_scase.npartinfo>0){
    float part = 0.0;
    char label[100];

    for(i = 0; i<global_scase.npartinfo; i++){
      partdata *parti;
      FILE_SIZE file_size;

      parti = global_scase.partinfo+i;
      file_size = GetFileSizeSMV(parti->file);
      part += file_size;
    }
    printf("particle files: %s\n", GetFloatFileSizeLabel(part, label));
  }
  else{
    printf("particle files: no files found\n");
  }

  printf("\n");
  if(global_scase.npatchinfo>0){
    patchdata **patchlist;
    float sum = 0.0, compressed_sum=0;

    printf("boundary files sizes: \n");
    NewMemory((void **)&patchlist, global_scase.npatchinfo*sizeof(patchdata *));
    for(i = 0; i<global_scase.npatchinfo; i++){
      patchlist[i] = global_scase.patchinfo+i;
    }
    qsort((patchdata **)patchlist, (size_t)global_scase.npatchinfo, sizeof(patchdata *), ComparePatchLabels);
    for(i = 0; i<global_scase.npatchinfo; i++){
      patchdata *patchi, *patchim1;
      FILE_SIZE file_size, compressed_file_size;

      patchi = patchlist[i];
      file_size = GetFileSizeSMV(patchi->reg_file);
      compressed_file_size = GetFileSizeSMV(patchi->comp_file);

      if(i>0)patchim1 = patchlist[i-1];
      if(i==0||strcmp(patchim1->label.longlabel, patchi->label.longlabel)==0){
        sum += file_size;
        compressed_sum += compressed_file_size;
      }
      else{
        PrintFileSizes(patchim1->label.longlabel,sum,compressed_sum);
        sum = file_size;
        compressed_sum = compressed_file_size;
      }
      if(i==global_scase.npatchinfo-1){
        PrintFileSizes(patchi->label.longlabel,sum,compressed_sum);
      }
    }
  }
  else{
    printf("boundary files sizes: no files found\n");
  }
}

/* ------------------ HideAllSmoke ------------------------ */

void HideAllSmoke(void){
  int i;
  for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->loaded == 1)smoke3di->display = 0;
  }
  for(i = 0; i < global_scase.nisoinfo; i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded == 1)isoi->display = 0;
  }
}

/* ------------------ HideAllSlices ------------------------ */

void HideAllSlices(void){
  int i;

  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  for(i = 0; i < global_scase.slicecoll.nsliceinfo; i++){
    global_scase.slicecoll.sliceinfo[i].display = 0;
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ ShowAllSmoke ------------------------ */

void ShowAllSmoke(void){
  int i;
  for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->loaded == 1)smoke3di->display = 1;
  }
  for(i = 0; i < global_scase.nisoinfo; i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded == 1)isoi->display = 1;
  }
}

/* ------------------ ShowMultiSliceMenu ------------------------ */

void ShowMultiSliceMenu(int value){
  int mdisplay;
  int i;

  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  switch(value){
  case SHOW_ALL:
  case HIDE_ALL:
    ShowHideSliceMenu(value);
    return;
  case MENU_SHOWSLICE_IN_GAS:
    GLUISliceInObstMenu2Dialog(ONLY_IN_GAS);
    break;
  case MENU_SHOWSLICE_IN_GASANDSOLID:
    GLUISliceInObstMenu2Dialog(GAS_AND_SOLID);
    break;
  case MENU_SHOWSLICE_IN_SOLID:
    GLUISliceInObstMenu2Dialog(ONLY_IN_SOLID);
    break;
  case -12:
    offset_slice = 1 - offset_slice;
    break;
  default:
    if(value<=-20){
      value = -20 - value;
      ShowBoundaryMenu(value+1000);
    }
    else{
      multislicedata *mslicei;
      slicedata *sd;

      mslicei = global_scase.slicecoll.multisliceinfo+value;
      sd = global_scase.slicecoll.sliceinfo+mslicei->islices[0];
      if(slicefile_labelindex==sd->slicefile_labelindex){
        if(plotstate!=DYNAMIC_PLOTS){
          plotstate = DYNAMIC_PLOTS;
          mdisplay = 1;
        }
        else{
          if(mslicei->display == -1){
            mdisplay = 0;
          }
          else{
            mdisplay = 1 - mslicei->display;
          }
        }
      }
      else{
        plotstate = DYNAMIC_PLOTS;
        sd = global_scase.slicecoll.sliceinfo+mslicei->islices[0];
        slicefile_labelindex = sd->slicefile_labelindex;
        mdisplay = 1;
      }
      for(i = 0; i<mslicei->nslices; i++){
        sd = global_scase.slicecoll.sliceinfo+mslicei->islices[i];
        if(sd->loaded==0)continue;
        sd->display = mdisplay;
      }
    }
    break;
  }
  UpdateSliceFilenum();
  plotstate = GetPlotState(DYNAMIC_PLOTS);

  UpdateShow();
}

/* ------------------ ShowAllSlices ------------------------ */

void ShowAllSlices(char *type1, char *type2){
  int i;

  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(trainer_showall_mslice == 1){
    for(i = 0; i < global_scase.slicecoll.nsliceinfo; i++){
      slicedata *slicei;

      slicei = global_scase.slicecoll.sliceinfo + i;
      slicei->display = 0;
      if(slicei->loaded == 0)continue;
      if(
        (type1 != NULL&&STRCMP(slicei->label.longlabel, type1) == 0) ||
        (type2 != NULL&&STRCMP(slicei->label.longlabel, type2) == 0)
       ){
        global_scase.slicecoll.sliceinfo[i].display = 1;
        slicefile_labelindex = slicei->slicefile_labelindex;
      }
    }
  }
  else{
    int msliceindex;

    if(trainerload == 2){
      if(trainerload == trainerload_old){
        trainer_temp_index++;
        if(trainer_temp_index > trainer_temp_n - 1){
          trainer_temp_index = 0;
        }
      }
      msliceindex = trainer_temp_indexes[trainer_temp_index];
    }
    else{
      if(trainerload == trainerload_old){
        trainer_oxy_index++;
        if(trainer_oxy_index > trainer_oxy_n - 1){
          trainer_oxy_index = 0;
        }
      }
      msliceindex = trainer_oxy_indexes[trainer_oxy_index];
    }
    ShowMultiSliceMenu(HIDE_ALL);
    ShowMultiSliceMenu(msliceindex);
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ TrainerViewMenu ------------------------ */

void TrainerViewMenu(int value){
  switch(value){
  case MENU_TRAINER_smoke:   // realistic
    HideAllSlices();
    trainerload=1;
    ShowAllSmoke();
    trainerload_old=1;
    break;
  case MENU_TRAINER_temp:  // temperature slices
    HideAllSmoke();
    trainerload=2;
    ShowAllSlices("TEMPERATURE",NULL);
    trainerload_old=2;
    break;
  case MENU_TRAINER_oxy:  //  oxygen slices
    HideAllSmoke();
    trainerload=3;
    ShowAllSlices("OXYGEN","OXYGEN VOLUME FRACTION");
    trainerload_old=3;
    break;
  case MENU_TRAINER_CLEAR: // unload
    LoadUnloadMenu(UNLOADALL);
    trainerload=0;
    trainerload_old=0;
    break;
  default:
    assert(FFALSE);
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ MainMenu ------------------------ */

void MainMenu(int value){

  if(value==MENU_MAIN_QUIT){
    if(scriptoutstream!=NULL){
      ScriptMenu(SCRIPT_STOP_RECORDING);
    }
    SMV_EXIT(0);
  }
  if(value==MENU_MAIN_TRAINERTOGGLE){
    trainer_mode=1-trainer_mode;
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ StaticVariableMenu ------------------------ */

void StaticVariableMenu(int value){

  plotn=value;
  plotstate=STATIC_PLOTS;
  visGrid = NOGRID_NOPROBE;
  if(visiso==1){
    UpdateShowStep(1,ISO);
  }
  UpdateSurface();
  if(visx_all==1){
    UpdateShowStep(1,XDIR);
  }
  if(visy_all==1){
    UpdateShowStep(1,YDIR);
  }
  if(visz_all==1){
    UpdateShowStep(1,ZDIR);
  }
  if(visx_all==0&&visy_all==0&&visz_all==0){
    UpdateShowStep(1,YDIR);
  }
  UpdateAllPlotSlices();
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUIUpdatePlot3dListIndex();
#define BOUND_PERCENTILE_DRAW          120
  GLUIPlot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
}

/* ------------------ IsoVariableMenu ------------------------ */

void IsoVariableMenu(int value){
  if(nplot3dloaded>0){
    plotn=value;
    if(visx_all==1){
      UpdateShowStep(1,XDIR);
    }
    if(visy_all==1){
      UpdateShowStep(1,YDIR);
    }
    if(visz_all==1){
      UpdateShowStep(1,ZDIR);
    }
    UpdateShowStep(1,ISO);
    UpdateSurface();
    plotstate=STATIC_PLOTS;
    UpdatePlotSlice(XDIR);
    UpdatePlotSlice(YDIR);
    UpdatePlotSlice(ZDIR);
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    GLUIUpdatePlot3dListIndex();
  }
}

/* ------------------ LabelMenu ------------------------ */

void LabelMenu(int value){
  updatemenu=1;
  if(value == MENU_DUMMY)return;
  GLUTPOSTREDISPLAY;
  switch(value){
  case MENU_LABEL_SETTINGS:
    GLUIShowDisplay(DIALOG_DISPLAY);
    break;
    // vis_colorbar                        state
    //    0/COLORBAR_HIDDEN                hidden
    //    1/COLORBAR_SHOW_VERTICAL         vertical
    //    2->max/COLORBAR_SHOW_HORIZONTAL  horizontal
  case MENU_LABEL_colorbar_vertical:
  case MENU_LABEL_colorbar_horizontal:
    if(value == MENU_LABEL_colorbar_vertical){
      visColorbarVertical = 1 - visColorbarVertical;
      if(visColorbarVertical == 1)visColorbarHorizontal = 0;
    }
    else{
      visColorbarHorizontal = 1 - visColorbarHorizontal;
      if(visColorbarHorizontal == 1)visColorbarVertical = 0;
    }
    GLUIUpdateColorbarControls();
    GLUIUpdateColorbarControls2();
    vis_colorbar = GetColorbarState();
    break;
  case MENU_LABEL_timebar:
    visTimebar        = 1-visTimebar;
    visTimelabel      = 1-visTimebar;
    visFrameTimelabel = 1-visTimebar;
    visFramelabel     = 1-visTimebar;
    LabelMenu(MENU_LABEL_timelabel);
    LabelMenu(MENU_LABEL_frametimelabel);
    LabelMenu(MENU_LABEL_framelabel);
    break;
  case MENU_LABEL_framerate:
    visFramerate = 1 - visFramerate;
    break;
   case MENU_LABEL_ShowAll:
    visUSERticks=1;
    visColorbarVertical=1;
    visColorbarHorizontal=0;
    visTimebar=1;
    vis_title_smv_version = 1;
    vis_title_fds = 1;
    vis_title_CHID = 1;
    vis_title_gversion =1;
    visFramerate=1;
#ifdef pp_memstatus
    visAvailmemory=1;
#endif
    visaxislabels=1;
    visTimelabel=1;
    visFramelabel=1;
    visLabels=1;
    visMeshlabel=1;
    vis_slice_average=1;
    if(global_scase.ntickinfo>0)visFDSticks=1;
    visgridloc=1;
    vis_hrr_label=1;
    visFramelabel=1;
    break;
   case MENU_LABEL_HideAll:
    visUSERticks=0;
    visColorbarVertical=0;
    visColorbarHorizontal=0;
    visTimebar=0;
    vis_title_smv_version = 0;
    vis_title_fds = 0;
    vis_title_CHID = 0;
    vis_title_gversion =0;
    visFramerate=0;
    visaxislabels=0;
    visLabels=0;
    visTimelabel=0;
    visFramelabel=0;
    visMeshlabel=0;
    vis_hrr_label=0;
    if(global_scase.ntickinfo>0)visFDSticks=0;
    visgridloc=0;
    vis_slice_average=0;
#ifdef pp_memstatus
    visAvailmemory=0;
#endif
    break;
   case MENU_LABEL_northangle:
     vis_northangle = 1-vis_northangle;
     break;
   case MENU_LABEL_axis:
    visaxislabels = 1 - visaxislabels;
    GLUIUpdateVisAxisLabels();
    break;
   case MENU_LABEL_textlabels:
     visLabels = 1 - visLabels;
     break;
   case MENU_LABEL_timelabel:
     visTimelabel=1-visTimelabel;
     break;
   case MENU_LABEL_frametimelabel:
     visFrameTimelabel = 1-visFrameTimelabel;
     GLUIUpdateFrameTimelabel();
     break;
   case MENU_LABEL_framelabel:
     visFramelabel=1-visFramelabel;
     if(visFramelabel==1){
       vis_hrr_label=0;
     }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateShow();
    update_times = 1;
     break;
   case MENU_LABEL_meshlabel:
     visMeshlabel=1-visMeshlabel;
     break;
#ifdef pp_memstatus
   case MENU_LABEL_memload:
     visAvailmemory = 1 - visAvailmemory;
     break;
#endif
#ifdef pp_MEMDEBUG
   case MENU_LABEL_memusage:
     visUsagememory = 1 - visUsagememory;
#ifdef pp_memstatus
     if(visUsagememory==1)visAvailmemory=0;
#endif
     break;
#endif
   case MENU_LABEL_fdsticks:
     visFDSticks=1-visFDSticks;
     break;
   case MENU_LABEL_hmslabel:
     vishmsTimelabel = 1 - vishmsTimelabel;
     break;
   case MENU_LABEL_grid:
     visgridloc = 1 - visgridloc;
     break;
   case MENU_LABEL_sliceaverage:
     vis_slice_average = 1 - vis_slice_average;
     break;
   case MENU_LABEL_hrr:
     vis_hrr_label=1-vis_hrr_label;
     break;
   case MENU_LABEL_userticks:
     visUSERticks = 1 - visUSERticks;
     break;
   default:
     assert(FFALSE);
     break;
  }
  GLUISetLabelControls();
}

/* ------------------ SmokeColorbarMenu ------------------------ */

void SmokeColorbarMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;

  value = CLAMP(value, 0, colorbars.ncolorbars - 1);
  colorbars.fire_colorbar_index=value;
  fire_colorbar = colorbars.colorbarinfo + value;
  UpdateRGBColors(colorbar_select_index);
  if(FlowDir>0){
    Keyboard('-',FROM_SMOKEVIEW);
    Keyboard(' ',FROM_SMOKEVIEW);
  }
  else{
    Keyboard(' ',FROM_SMOKEVIEW);
    Keyboard('-',FROM_SMOKEVIEW);
  }
  GLUTPOSTREDISPLAY;
}

/* ------------------ ColorbarDigitMenu ------------------------ */

void ColorbarDigitMenu(int value){
  ncolorlabel_digits = value;
  update_colorbar_digits = 1;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

  /* ------------------ ColorbarMenu ------------------------ */

void ColorbarMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value<0){
    switch(value){
    case COLORBAR_AUTOFLIP:
      colorbar_autoflip = 1 - colorbar_autoflip;
      update_flipped_colorbar = 1;
      GLUIUpdateColorbarFlip();
      break;
    case COLORBAR_FLIP:
      colorbar_flip=1-colorbar_flip;
      GLUIUpdateColorbarFlip();
      break;
    case COLORBAR_RESET:
      show_extreme_mindata=0;
      show_extreme_maxdata=0;
      colorbar_flip=0;
      contour_type=SHADED_CONTOURS;
      setbw=0;
      GLUIUpdateExtreme();
      UpdateRGBColors(colorbar_select_index);
      break;
    case COLORBAR_HIGHLIGHT_BELOW:
      show_extreme_mindata=1-show_extreme_mindata;
      GLUIUpdateExtreme();
      UpdateRGBColors(colorbar_select_index);
      break;
    case COLORBAR_HIGHLIGHT_ABOVE:
      show_extreme_maxdata=1-show_extreme_maxdata;
      GLUIUpdateExtreme();
      UpdateRGBColors(colorbar_select_index);
      break;
    case COLORBAR_TOGGLE_BW_DATA:
      setbwdata = 1 - setbwdata;
      if(setbwdata==1&&colorbars.bw_colorbar_index>=0){
        colorbartype_save=colorbartype;
        ColorbarMenu(colorbars.bw_colorbar_index);
      }
      else{
        if(colorbartype_save>-1)ColorbarMenu(colorbartype_save);
      }
      GLUIIsoBoundCB(ISO_COLORS);
      break;
    case COLORBAR_TOGGLE_BW:
      setbw=1-setbw;
      InitRGB();
      GLUISetLabelControls();
      break;
    case USE_LIGHTING:
      GLUIUpdateUseLighting();
      break;
    case TOGGLE_LIGHTING:
      use_lighting = 1 - use_lighting;
      GLUIUpdateUseLighting();
      break;
   case COLORBAR_TRANSPARENT:
     use_transparency_data=1-use_transparency_data;
     UpdateRGBColors(colorbar_select_index);
     GLUISetLabelControls();
     GLUIUpdateTransparency();
     break;
   case COLORBAR_DECIMAL:
     force_decimal = 1 - force_decimal;
     update_colorbar_digits = 1;
     break;
   case COLORBAR_CONTINUOUS:
     contour_type=SHADED_CONTOURS;
     UpdateRGBColors(colorbar_select_index);
     break;
   case COLORBAR_STEPPED:
     contour_type=STEPPED_CONTOURS;
     UpdateRGBColors(colorbar_select_index);
     break;
   case COLORBAR_LINES:
     contour_type=LINE_CONTOURS;
     UpdateRGBColors(colorbar_select_index);
     break;
   case COLORBAR_HORIZONTAL:
     LabelMenu(MENU_LABEL_colorbar_horizontal);
     break;
   case COLORBAR_VERTICAL:
     LabelMenu(MENU_LABEL_colorbar_vertical);
     break;
   case MENU_COLORBAR_SETTINGS:
     GLUIShowBounds(DIALOG_COLORING);
     break;
   default:
     assert(FFALSE);
     break;
   }
  }
  if(value>=0){
    colorbartype=value;
    colorbars.iso_colorbar_index=value;
    iso_colorbar = colorbars.colorbarinfo + colorbars.iso_colorbar_index;
    update_texturebar=1;
    GLUIUpdateListIsoColorobar();
    UpdateCurrentColorbar(colorbars.colorbarinfo+colorbartype);
    GLUIUpdateColorbarType();
    GLUISetColorbarListBound(colorbartype);
    if(colorbartype == colorbars.bw_colorbar_index&&colorbars.bw_colorbar_index>=0){
      setbwdata = 1;
    }
    else{
      setbwdata = 0;
    }
    GLUIIsoBoundCB(ISO_COLORS);
    GLUISetLabelControls();
    char *ext, cblabel[1024];
    strcpy(cblabel,colorbars.colorbarinfo[colorbartype].menu_label);
    ext = strrchr(cblabel,'.');
    if(ext!=NULL)*ext=0;
  }
  if(value>-10){
    UpdateRGBColors(colorbar_select_index);
  }
}

/* ------------------ Smoke3DShowMenu ------------------------ */

void Smoke3DShowMenu(int value){
  smoke3ddata *smoke3di;
  int i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value<0){
    switch(value){
    case SHOW_ALL:
      plotstate=DYNAMIC_PLOTS;
      show_3dsmoke=1;
      Smoke3DShowMenu(SET_SMOKE3D);
      break;
    case HIDE_ALL:
      show_3dsmoke=0;
      Smoke3DShowMenu(SET_SMOKE3D);
      break;
    case TOGGLE_SMOKE3D:
      show_3dsmoke = 1 - show_3dsmoke;
      Smoke3DShowMenu(SET_SMOKE3D);
      break;
    case SET_SMOKE3D:
      for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
        smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
        if(smoke3di->loaded==1)smoke3di->display=show_3dsmoke;
      }
    break;
    default:
      assert(FFALSE);
    }
  }
  else{
    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + value;
    if(plotstate!=DYNAMIC_PLOTS){
      plotstate=DYNAMIC_PLOTS;
      smoke3di->display=1;
    }
    else{
      smoke3di->display = 1 - smoke3di->display;
    }
  }
}

/* ------------------ IsoShowMenu ------------------------ */

void IsoShowMenu(int value){
  int i;
  int nisolevels, *showlevels;
  isodata *isoi;

  nisolevels=loaded_isomesh->nisolevels;
  showlevels=loaded_isomesh->showlevels;

  switch(value){
  case  MENU_ISOSHOW_SMOOTH:
    smooth_iso_normal=1-smooth_iso_normal;
    break;
  case MENU_ISOSHOW_OUTPUT:
    OutputAllIsoBounds();
    break;
  case MENU_ISOSHOW_NORMALS:
    show_iso_normal = 1 - show_iso_normal;
    break;
  case MENU_ISOSHOW_SOLID:
  case MENU_ISOSHOW_OUTLINE:
  case MENU_ISOSHOW_POINTS:
    if(value == MENU_ISOSHOW_SOLID)show_iso_shaded=1-show_iso_shaded;
    if(value == MENU_ISOSHOW_OUTLINE)show_iso_outline = 1 - show_iso_outline;
    if(value == MENU_ISOSHOW_POINTS)show_iso_points = 1 - show_iso_points;
    visAIso=show_iso_shaded*1+show_iso_outline*2+show_iso_points*4;
    if(visAIso!=0){
      plotstate=DYNAMIC_PLOTS;
    }
    GLUIUpdateIsotype();
    break;
   case MENU_ISOSHOW_ALLSOLID:
    transparent_state=ALL_SOLID;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo + global_scase.surfcoll.nsurfinfo + 1 + i;
      surfi->transparent_level=1.0;
    }
    use_transparency_data=0;
    break;
   case MENU_ISOSHOW_ALLTRANSPARENT:
    transparent_state=ALL_TRANSPARENT;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo + global_scase.surfcoll.nsurfinfo + 1 + i;
      surfi->transparent_level=transparent_level;
    }
    use_transparency_data=1;
    break;
   case MENU_ISOSHOW_MINSOLID:
    transparent_state=MIN_SOLID;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo + global_scase.surfcoll.nsurfinfo + 1 + i;
      surfi->transparent_level=transparent_level;
    }
    global_scase.surfcoll.surfinfo[global_scase.surfcoll.nsurfinfo+1].transparent_level=1.0;
    use_transparency_data=1;
    break;
   case MENU_ISOSHOW_MAXSOLID:
    transparent_state=MAX_SOLID;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo + global_scase.surfcoll.nsurfinfo + 1 + i;
      surfi->transparent_level=transparent_level;
    }
    use_transparency_data=1;
    global_scase.surfcoll.surfinfo[global_scase.surfcoll.nsurfinfo+1+loaded_isomesh->nisolevels-1].transparent_level=1.0;
    break;
   case MENU_ISOSHOW_HIDEALL:
    show_iso_shaded=0;
    show_iso_outline=0;
    show_iso_points=0;
    visAIso=show_iso_shaded*1+show_iso_outline*2+show_iso_points*4;
    for(i=0;i<nisolevels;i++){
      showlevels[i]=0;
    }
    break;
   case MENU_ISOSHOW_SHOWALL:
    show_iso_shaded=1;
    show_iso_outline=0;
    show_iso_points=0;
    visAIso=show_iso_shaded*1+show_iso_outline*2+show_iso_points*4;
    for(i=0;i<nisolevels;i++){
      showlevels[i]=1;
    }
    break;
   default:
    if(value>99&&value<999&&value-100<nisolevels){
     showlevels[value-100] = 1 - showlevels[value-100];
    }
    else if(value>=1000&&value<=10000){      // we can only have 9900 isosurface files
     isoi = global_scase.isoinfo + value - 1000;          // hope that is enough!!
     if(plotstate!=DYNAMIC_PLOTS){
       plotstate=DYNAMIC_PLOTS;
       isoi->display=1;
       iisotype=isoi->type;
     }
     else{
       if(isoi->type==iisotype){
         isoi->display = 1 - isoi->display;
         UpdateIsoType();
       }
       else{
         isoi->display=1;
         iisotype=isoi->type;
       }
     }
     UpdateShow();
    }
    else if(value>=SHOWALL_ISO){
      if(value==SHOWALL_ISO){
        plotstate=DYNAMIC_PLOTS;
        show_isofiles = 1;
      }
      else if(value==HIDEALL_ISO){
        show_isofiles = 0;
      }
      else if(value==TOGGLE_ISO){
        show_isofiles = 1 - show_isofiles;
      }
      for(i=0;i<global_scase.nisoinfo;i++){
        global_scase.isoinfo[i].display=show_isofiles;
      }
      UpdateShow();
    }
  }
  UpdateIsoShowLevels(&global_scase, loaded_isomesh);
  UpdateIsoTriangles(1);

  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ ShowVSliceMenu ------------------------ */

void ShowVSliceMenu(int value){
  int i;
  vslicedata *vd;

  if(value == MENU_DUMMY)return;
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  if(value==SHOW_ALL||value==GLUI_SHOWALL_VSLICE||value==GLUI_HIDEALL_VSLICE){
    if(value == SHOW_ALL)showall_slices = 1 - showall_slices;
    if(value == GLUI_SHOWALL_VSLICE)showall_slices = 1;
    if(value == GLUI_HIDEALL_VSLICE)showall_slices = 0;
    for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
      vd = global_scase.slicecoll.vsliceinfo+i;
      if(vd->loaded==0)continue;
      vd->display= showall_slices;
    }
    UpdateTimes();
    return;
  }
  if(value==MENU_SHOWSLICE_IN_GAS){
    GLUISliceInObstMenu2Dialog(ONLY_IN_GAS);
    return;
  }
  else if(value==MENU_SHOWSLICE_IN_GASANDSOLID){
    GLUISliceInObstMenu2Dialog(GAS_AND_SOLID);
    return;
  }
  else if(value==MENU_SHOWSLICE_IN_SOLID){
    GLUISliceInObstMenu2Dialog(ONLY_IN_SOLID);
    return;
  }
  if(value == MENU_SHOWSLICE_OFFSET){
    offset_slice=1-offset_slice;
    return;
  }
  if(value == MENU_SHOWSLICE_NODESLICEANDVECTORS){
    show_node_slices_and_vectors=1-show_node_slices_and_vectors;
    return;
  }
  if(value == MENU_SHOWSLICE_CELLSLICEANDVECTORS){
    show_cell_slices_and_vectors=1-show_cell_slices_and_vectors;
    return;
  }
  vd = global_scase.slicecoll.vsliceinfo + value;
  if(slicefile_labelindex==global_scase.slicecoll.sliceinfo[vd->ival].slicefile_labelindex){
    if(plotstate!=DYNAMIC_PLOTS){
      plotstate=DYNAMIC_PLOTS;
      vd->display=1;
    }
    else{
      vd->display = 1 - vd->display;
    }
    if(vd->iu!=-1){
      slicedata *sd;

      sd = global_scase.slicecoll.sliceinfo+vd->iu;
      sd->vloaded=vd->display;
    }
    if(vd->iv!=-1){
      slicedata *sd;

      sd = global_scase.slicecoll.sliceinfo+vd->iv;
      sd->vloaded=vd->display;
    }
    if(vd->iw!=-1){
      slicedata *sd;

      sd = global_scase.slicecoll.sliceinfo+vd->iw;
      sd->vloaded=vd->display;
    }
    if(vd->ival!=-1){
      slicedata *sd;

      sd = global_scase.slicecoll.sliceinfo+vd->ival;
      sd->vloaded=vd->display;
    }
  }
  else{
    slicefile_labelindex = global_scase.slicecoll.sliceinfo[vd->ival].slicefile_labelindex;
    vd->display=1;
  }
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  UpdateShow();
}

/* ------------------ SliceSKipMenu ------------------------ */

void SliceSkipMenu(int value){
  slice_skip = value;
  GLUIUpdateSliceSkip();
  updatemenu=1;
}

/* ------------------ ShowHideSliceMenu ------------------------ */

void ShowHideSliceMenu(int value){
  int i;

  if(value == MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value<0){
    switch(value){
    case GLUI_HIDEALL:
    case GLUI_SHOWALL:
    case SHOW_ALL:
      if(value == GLUI_SHOWALL)showall_slices = 1;
      if(value == GLUI_HIDEALL)showall_slices = 0;
      if(value == SHOW_ALL)showall_slices = 1-showall_slices;
      for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
        global_scase.slicecoll.sliceinfo[i].display=showall_slices;
      }
      break;
    case MENU_SHOWSLICE_IN_GAS:
      GLUISliceInObstMenu2Dialog(ONLY_IN_GAS);
      break;
    case MENU_SHOWSLICE_IN_GASANDSOLID:
      GLUISliceInObstMenu2Dialog(GAS_AND_SOLID);
      break;
    case MENU_SHOWSLICE_IN_SOLID:
      GLUISliceInObstMenu2Dialog(ONLY_IN_SOLID);
      break;
    case MENU_SHOWSLICE_OFFSET:
      offset_slice=1-offset_slice;
      break;
    case MENU_SHOWSLICE_TERRAIN:
      planar_terrain_slice=1-planar_terrain_slice;
      break;
    case MENU_SHOWSLICE_NODESLICEANDVECTORS:
      show_node_slices_and_vectors=1-show_node_slices_and_vectors;
      return;
    case MENU_SHOWSLICE_CELLSLICEANDVECTORS:
      show_cell_slices_and_vectors=1-show_cell_slices_and_vectors;
      return;
    default:
      if(value<=-20){
        value = -20 - value;
        ShowBoundaryMenu(value+1000);
      }
    }
  }
  else{
    slicedata *sd;

    sd = global_scase.slicecoll.sliceinfo + value;
    if(slicefile_labelindex==sd->slicefile_labelindex){
      if(plotstate!=DYNAMIC_PLOTS){
        plotstate=DYNAMIC_PLOTS;
        sd->display=1;
      }
      else{
        sd->display = 1 - sd->display;
      }
    }
    else{
      plotstate=DYNAMIC_PLOTS;
      slicefile_labelindex = sd->slicefile_labelindex;
      sd->display=1;
    }
  }
  update_flipped_colorbar = 1;
  UpdateSliceFilenum();
  plotstate=GetPlotState(DYNAMIC_PLOTS);

  UpdateShow();
}

/* ------------------ ShowHideMenu ------------------------ */

void ShowHideMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  switch(value){
#ifdef pp_MEMPRINT
  case MENU_SHOWHIDE_PRINT:
    PrintMemoryInfo;
    break;
#endif
  case MENU_SHOWHIDE_FLIP:
   background_flip = 1-background_flip;
   UpdateRGBColors(colorbar_select_index);
   GLUISetLabelControls();
   GLUISetColorControls();
   GLUIUpdateBackgroundFlip(background_flip);
   GLUIUpdateBackgroundFlip2(background_flip);
   break;
  case MENU_SHOWHIDE_PARTICLES:
    if(plotstate==DYNAMIC_PLOTS){
      visParticles=1-visParticles;
    }
    else{
      plotstate=DYNAMIC_PLOTS;
      visParticles=1;
    }
    UpdateTimes();
    break;
  case MENU_SHOWHIDE_SENSOR:
    visSensor=1-visSensor;
    break;
  case MENU_SHOWHIDE_SENSOR_NORM:
    visSensorNorm=1-visSensorNorm;
    break;
  case MENU_SHOWHIDE_OFFSET:
    if(titlesafe_offset==0){
      titlesafe_offset=titlesafe_offsetBASE;
    }
    else{
      titlesafe_offset=0;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ ViewpointMenu ------------------------ */

void ViewpointMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  switch(value){
  case TOGGLE_TITLE_SAFE:
    if(titlesafe_offset==0){
      titlesafe_offset=titlesafe_offsetBASE;
    }
    else{
      titlesafe_offset=0;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ DialogMenu ------------------------ */

void DialogMenu(int value){
  GLUTPOSTREDISPLAY;
  switch(value){
  case DIALOG_SHOOTER:
    GLUIShowShooter();
    break;
  case DIALOG_TRAINER:
    GLUIShowTrainer();
    break;
  case DIALOG_2DPLOTS:
    if(csv_loaded==0)LoadPlot2DMenu(MENU_PLOT2D_LOADCSV);
    GLUIShowPlot2D();
    break;
  case DIALOG_DEVICE:
    GLUIShowDevice();
    break;
  case DIALOG_3DSMOKE:
  case DIALOG_AUTOLOAD:
  case DIALOG_BOUNDS:
  case DIALOG_CONFIG:
  case DIALOG_SCRIPT:
  case DIALOG_SHOWFILES:
  case DIALOG_SMOKEZIP:
  case DIALOG_TIME:
    GLUIShowBounds(value);
    break;
  case DIALOG_MOTION:
  case DIALOG_RENDER:
  case DIALOG_MOVIE:
  case DIALOG_MOVIE_BATCH:
  case DIALOG_SCALING:
  case DIALOG_VIEW:
  case DIALOG_WINDOW_PROPERTIES:
    GLUIShowMotion(value);
    break;
  case DIALOG_USER_TICKS:
  case DIALOG_FONTS:
  case DIALOG_LABELS_TICKS:
  case DIALOG_DISPLAY:
    GLUIShowDisplay(value);
    break;
  case DIALOG_TOUR_SHOW:
    GLUIShowTour();
    break;
  case DIALOG_TOUR_HIDE:
    GLUIHideTour();
    break;
  case DIALOG_CLIP:
    GLUIShowClip();
    break;
  case DIALOG_STEREO:
    GLUIShowStereo();
    break;
  case DIALOG_COLORBAR:
    GLUIShowColorbar();
    break;
  case DIALOG_HVAC:
    GLUIShowHVAC();
    break;
  case DIALOG_GEOMETRY_CLOSE:
    GLUIHideGeometry();
    GLUIUpdateTrainerOutline();
    break;
  case DIALOG_GEOMETRY_OPEN:
    if(global_scase.fds_filein!=NULL&&updategetobstlabels==1){
      CheckMemoryOff;
      GetObstLabels(global_scase.fds_filein);
      CheckMemoryOn;
      updategetobstlabels=0;
    }
    GLUIShowGeometry();
    visBlocks=visBLOCKNormal;
    GLUIUpdateTrainerOutline();
    break;
  case DIALOG_TERRAIN:
    GLUIShowTerrain();
    break;
  case DIALOG_SHRINKALL:
    GLUIShrinkDialogs();
    break;
  case DIALOG_HIDEALL:
    GLUIHideTerrain();
    GLUIHideShooter();
    GLUIHideDisplay();
    GLUIHideBounds();
    GLUIHideMotion();
    GLUIHideTour();
    GLUIHideClip();
    GLUIHideStereo();
    GLUIHideColorbar();
    if(showedit_dialog==1)DialogMenu(DIALOG_GEOMETRY_CLOSE);
    GLUIHideTrainer();
    GLUIHideDevice();
    GLUIHidePlot2D();
    break;
  default:
    assert(FFALSE);
    break;
  }
  updatemenu=1;
#ifdef pp_REFRESH
  refresh_glui_dialogs = 1;
  SetMainWindow();
  GLUIRefreshDialogs();
  glutPostRedisplay();
#endif
}

/* ------------------ ZoomMenu ------------------------ */

void ZoomMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;
  if(opengldefined==1){
    GLUTPOSTREDISPLAY;
  }
  zoomindex=value;
  if(zoomindex==-1){
    if(zoom<zooms[0]){
      zoom=zooms[0];
      zoomindex=0;
    }
    if(zoomindex!=MAX_ZOOMS&&zoom>zooms[MAX_ZOOMS-1]){
      zoom=zooms[MAX_ZOOMS-1];
      zoomindex=MAX_ZOOMS-1;
    }
    if(projection_type!= PROJECTION_PERSPECTIVE){
      camera_current->projection_type=projection_type;
      SetViewPoint(RESTORE_EXTERIOR_VIEW);
      GLUIUpdateProjectionType();
    }
  }
  else if(zoomindex==UPDATE_PROJECTION){
    camera_current->projection_type=projection_type;
    GLUIUpdateProjectionType();
    UpdateCameraYpos(camera_current, 2);
    if(projection_type== PROJECTION_ORTHOGRAPHIC){
      camera_current->eye[1]=camera_current->isometric_y;
    }
  }
  else{
    if(zoomindex<0)zoomindex=ZOOMINDEX_ONE;
    if(zoomindex>MAX_ZOOMS)zoomindex=ZOOMINDEX_ONE;
    zoom=zooms[zoomindex];
    if(projection_type!= PROJECTION_PERSPECTIVE){
      SetViewPoint(RESTORE_EXTERIOR_VIEW_ZOOM);
      camera_current->projection_type=projection_type;
      GLUIUpdateProjectionType();
    }
  }
  camera_current->zoom=zoom;
  GLUIUpdateZoom();
}

/* ------------------ ApertureMenu ------------------------ */

void ApertureMenu(int value){
  updatemenu=1;
  if(opengldefined==1){
    GLUTPOSTREDISPLAY;
  }
  apertureindex = CLAMP(value, 0, 4);
  aperture=apertures[apertureindex];
}

/* ------------------ FontMenu ------------------------ */

void FontMenu(int value){
  updatemenu=1;
  if(opengldefined==1){
    GLUTPOSTREDISPLAY;
  }
  switch(value){
  case MENU_FONT_SETTINGS:
    GLUIShowDisplay(DIALOG_FONTS);
    break;
  case SMALL_FONT:
    fontindex=SMALL_FONT;
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      font_ptr          = (void *)GLUT_BITMAP_HELVETICA_24;
      colorbar_font_ptr = (void *)GLUT_BITMAP_HELVETICA_20;
    }
    else{
      font_ptr          = GLUT_BITMAP_HELVETICA_12;
      colorbar_font_ptr = GLUT_BITMAP_HELVETICA_10;
  }
#else
    font_ptr          = GLUT_BITMAP_HELVETICA_12;
    colorbar_font_ptr = GLUT_BITMAP_HELVETICA_12;
#endif
    font_height          = 12;
    colorbar_font_height = 10;
    break;
  case LARGE_FONT:
    fontindex=LARGE_FONT;
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      font_ptr          = (void *)GLUT_BITMAP_HELVETICA_36;
      colorbar_font_ptr = (void *)GLUT_BITMAP_HELVETICA_36;
    }
    else{
      font_ptr          = GLUT_BITMAP_HELVETICA_18;
      colorbar_font_ptr = GLUT_BITMAP_HELVETICA_18;
    }
#else
    font_ptr            = GLUT_BITMAP_HELVETICA_18;
    colorbar_font_ptr   = GLUT_BITMAP_HELVETICA_18;
#endif
    font_height          = 18;
    colorbar_font_height = 18;
    break;
  case SCALED_FONT:
    fontindex=SCALED_FONT;
    break;
  default:
    assert(FFALSE);
  }
  GLUIUpdateFontIndex();
  GLUISetLabelControls();
}

/* ------------------ UnitsMenu ------------------------ */

void UnitsMenu(int value){
  int unitclass, unit_index;
  int i;

  unitclass = value/1000;
  unit_index = value - unitclass*1000;
  unitclasses[unitclass].unit_index=unit_index;
  if(value==MENU_UNITS_RESET){
    for(i=0;i<nunitclasses;i++){
      unitclasses[i].unit_index=0;
    }
  }
  else if(value==MENU_UNITS_HMS){
    vishmsTimelabel = 1 - vishmsTimelabel;
    GLUISetLabelControls();

  }
  else if(value==MENU_UNITS_SHOWALL){
    show_all_units = 1 - show_all_units;
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ OptionMenu ------------------------ */

void OptionMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value == MENU_OPTION_TRAINERMENU){
    trainer_mode=1;
    if(showtrainer_dialog==0){
      GLUIShowTrainer();
    }
    FontMenu(LARGE_FONT);
  }
}

/* ------------------ GetNextViewLabel ------------------------ */

void GetNextViewLabel(char *label){
  int i;

  for(i=1;;i++){
    char view[256];

    sprintf(view,"view %i",i);
    if(GetCamera(view)==NULL){
      strcpy(label,view);
      return;
    }
  }
}

/* ------------------ ResetMenu ------------------------ */

void ResetMenu(int value){
  char view_label[256];

  if(value==MENU_DUMMY)return;
  switch(value){
  case MENU_VIEWPOINT_SETTINGS:
    GLUIShowMotion(DIALOG_VIEW);
    break;
  case MENU_SIZEPRESERVING:
    projection_type = 1 - projection_type;
    GLUISceneMotionCB(PROJECTION);
    break;
  case MENU_OUTLINEVIEW:
    if(visBlocks==visBLOCKOutline){
      BlockageMenu(visBLOCKAsInput);
    }
    else{
      BlockageMenu(visBLOCKOutline);
    }
    break;
  case MENU_TIMEVIEW:
    UpdateTimes();
    break;
  case MENU_VIEW_XMIN:
  case MENU_VIEW_XMAX:
  case MENU_VIEW_YMIN:
  case MENU_VIEW_YMAX:
  case MENU_VIEW_ZMIN:
  case MENU_VIEW_ZMAX:
    SetCameraView(camera_current, value);
    break;
  case SAVE_VIEWPOINT_AS_STARTUP:
    ResetMenu(SAVE_VIEWPOINT);
    ResetMenu(MENU_STARTUPVIEW);
    update_startup_view = 3;
    break;
  case MENU_VIEWPOINT_TOPVIEW:
    SetViewZMAXPersp();
    break;
  case SAVE_CURRENT_VIEWPOINT:
  case SAVE_VIEWPOINT:
    {
      cameradata *ca;

      if(value==SAVE_CURRENT_VIEWPOINT){
        strcpy(view_label, "current");
      }
      else{
        GetNextViewLabel(view_label);
      }
      GLUIAddListView(view_label);
      ca = GetCamera(view_label);
      if(ca != NULL){
        ResetMenu(ca->view_id);
      }
      update_saving_viewpoint = 2;
      strcpy(viewpoint_label_saved, view_label);
    }
    break;
  case MENU_STARTUPVIEW:
    if(selected_view==MENU_DUMMY)ResetMenu(SAVE_VIEWPOINT);
    GLUISetStartupView();
    break;
  default:
    assert(value>=-5);
    if(value<100000){
      GLUIResetView(value);
      if(scriptoutstream!=NULL){
        fprintf(scriptoutstream,"SETVIEWPOINT\n");
        fprintf(scriptoutstream," %s\n",camera_current->name);
      }
    }
    break;
  }
  //updatezoommenu=1; // updating zoom causes a bug when restoring views from the menu
                      // kept commented code in for future reference
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ ResetDefaultMenu ------------------------ */

void ResetDefaultMenu(int var){
  if(var == XYZ_CENTER){
    float xyz_center[3];

    xyz_center[0] = (xbarFDS + xbar0FDS)/2.0;
    xyz_center[1] = (ybarFDS + ybar0FDS)/2.0;
    xyz_center[2] = zbar0FDS;
    GLUISetPosXYZFDS(xyz_center);
    return;
  }
  if(var==2){
    int i;

    use_geom_factors = 1 - use_geom_factors;
    updatemenu = 1;
    GLUIUpdateUseGeomFactors();
    for(i = 0; i<ncameras_sorted; i++){
      cameradata *ca;

      ca = cameras_sorted[i];
      if(ca->view_id>1)continue;
      if(ca->view_id==selected_view){
        ResetDefaultMenu(selected_view);
        break;
      }
    }
    return;
  }
  ResetMenu(var);
  switch(var){
    case 0:
    case -1:
      UpdateCameraYpos(camera_current, 1);
      break;
    case 1:
    case -2:
    case -3:
      UpdateCameraYpos(camera_current, 2);
      break;
    case -4:
    case -5:
      UpdateCameraYpos(camera_current, 3);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ RenderState ------------------------ */

void RenderState(int onoff){
  int scale = 1;

  if(onoff==RENDER_ON){

    if(render_status == RENDER_ON)return;
    render_status = RENDER_ON;
    render_firsttime = YES;
    update_screeninfo = 1;
    saveW=screenWidth;
    saveH=screenHeight;
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      scale = 2;
    }
#endif
    if(renderW==0||renderH==0){
      ResizeWindow(screenWidth/scale, screenHeight/scale);
    }
    else{
      if(renderW>max_screenWidth){
        ResizeWindow(max_screenWidth/scale, max_screenHeight/scale);
      }
      else{
        ResizeWindow(renderW/scale, renderH/scale);
      }
    }
  }
  else{
    int width_low, height_low, width_high, height_high;

    if(render_status==RENDER_OFF)return;
    render_status = RENDER_OFF;
    render_firsttime = NO;
    GLUIEnable360Zoom();
    screenWidth  = saveW/scale;
    screenHeight = saveH/scale;
    SetScreenSize(&screenWidth, &screenHeight);
    ResizeWindow(saveW/scale, saveH/scale);
    ResetRenderResolution(&width_low, &height_low, &width_high, &height_high);
    GLUIUpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
  }
}

/* ------------------ SkipMenu ------------------------ */

void SkipMenu(int value){
  render_skip=value;
  GLUTPOSTREDISPLAY;
  updatemenu=1;
  UpdateRenderListSkip();
}

/* ------------------ RenderMenu ------------------------ */

void RenderMenu(int value){
  slicedata *sd;
  int i;
  meshdata *meshi;

  if(value==MENU_DUMMY)return;
  updatemenu=1;
  if(value>=11000)return;
  if(opengldefined==1){
    GLUTPOSTREDISPLAY;
  }
  if(value>=10000&&value<=10005){
    resolution_multiplier=value-10000;
    GLUIUpdateResolutionMultiplier();
    return;
  }
  switch(value){
  case MENU_RENDER_SETTINGS:
    GLUIShowMotion(DIALOG_RENDER);
    break;
  case RenderCustom:
    render_window_size = value;
    renderW = script_render_width;
    renderH = script_render_height;
    render_size_index = value;
    break;
  case Render320:
    render_window_size=value;
    renderW=320;
    renderH=240;
    render_size_index=value;
    break;
  case Render640:
    render_window_size=value;
    renderW=640;
    renderH=480;
    render_size_index=value;
    break;
  case RenderWindow:
    render_window_size=value;
    renderW=0;
    renderH=0;
    render_size_index=value;
    break;
  case RENDER_CURRENT_SINGLE:
    Keyboard('r',FROM_SMOKEVIEW);
     break;
  case RENDER_CURRENT_360:
    LabelMenu(MENU_LABEL_HideAll);
    GetViewportInfo();
    RenderMenu(RENDER_CURRENT_SINGLE);
    Keyboard('R', FROM_SMOKEVIEW_ALT);
    break;
  case RENDER_CURRENT_MULTIPLE:
    if(resolution_multiplier==1){
      RenderMenu(RENDER_CURRENT_SINGLE);
      return;
    }
    Keyboard('R',FROM_SMOKEVIEW);
    break;
  case RenderJSON:
  case RenderJSONALL:
    {

      char *htmlobst_filename = CasePathHtmlObst(&global_scase);
      char *htmlslicenode_filename = CasePathHtmlSliceNode(&global_scase);
      char *htmlslicecell_filename = CasePathHtmlSliceCell(&global_scase);
      int json_option;

      json_option = HTML_CURRENT_TIME;
      if(value==RenderJSONALL)json_option = HTML_ALL_TIMES;
      if(Obst2Data(htmlobst_filename)!=0){
        printf("blockage data output to %s\n",htmlobst_filename);
      }
      else{
        printf("no blockage data to output\n");
      }
      if(SliceNode2Data(htmlslicenode_filename, json_option)!=0){
        printf("node centered slice file data output to %s\n", htmlslicenode_filename);
      }
      else{
        printf("no node centered slice file data to output\n");
      }
      if(SliceCell2Data(htmlslicecell_filename, json_option)!=0){
        printf("cell centered slice file data output to %s\n", htmlslicecell_filename);
      }
      else{
        printf("no cell centered slice file data to output\n");
      }
      FREEMEMORY(htmlobst_filename);
      FREEMEMORY(htmlslicenode_filename);
      FREEMEMORY(htmlslicecell_filename);
      break;
    }
    break;
    case RenderHTML: {
      char *html_filename = CasePathHtml(&global_scase);
      Smv2Html(html_filename, HTML_CURRENT_TIME,
               FROM_SMOKEVIEW);
      FREEMEMORY(html_filename);
    }
    break;
    case RenderHTMLALL: {
      char *html_filename = CasePathHtml(&global_scase);
      Smv2Html(html_filename, HTML_ALL_TIMES, FROM_SMOKEVIEW);
      FREEMEMORY(html_filename);

    } break;
  case RenderCancel:
    RenderState(RENDER_OFF);
    break;
  case Render360:
    render_mode = 1-render_mode;
    updatemenu = 1;
    break;
  case RenderStart360:
    RenderCB(RENDER_START_360);
    break;
  case RenderStartORIGRES:
    render_mode = RENDER_NORMAL;
    resolution_multiplier=1;
    RenderMenu(RenderStart);
    break;
  case RenderStartHIGHRES:
    render_mode = RENDER_NORMAL;
    resolution_multiplier=glui_resolution_multiplier;
    RenderMenu(RenderStart);
    break;
  case RenderStart:
    if(RenderTime==0&&touring==0){
      RenderMenu(RENDER_CURRENT_MULTIPLE);
      return;
    }
    if(render_skip == RENDER_CURRENT_SINGLE){
      UpdateFrameNumber(0);
    }
    else{
      if(stept == 0)Keyboard('t', FROM_SMOKEVIEW);
      ResetItimes0();
      for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
        sd=global_scase.slicecoll.sliceinfo+i;
        sd->itime=0;
      }
      for(i=0;i<global_scase.meshescoll.nmeshes;i++){
        meshi=global_scase.meshescoll.meshinfo+i;
        meshi->patch_itime=0;
      }
    }
    RenderState(RENDER_ON);
    UpdateTimeLabels();
    FlowDir=1;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"RENDERALL\n");
      fprintf(scriptoutstream," %i\n",render_skip);
      fprintf(scriptoutstream,"\n");
    }
    render_times = RENDER_ALLTIMES;
    break;
  case RenderLABELframenumber:
    render_label_type=RENDER_LABEL_FRAMENUM;
    GLUIUpdateFileLabel(render_label_type);
    break;
  case RenderLABELtime:
    render_label_type=RENDER_LABEL_TIME;
    GLUIUpdateFileLabel(render_label_type);
    break;
  case RenderPNG:
     render_filetype=PNG;
     updatemenu=1;
     break;
  case RenderJPEG:
     render_filetype=JPEG;
     updatemenu=1;
     break;
  default:
     assert(FFALSE);
     break;
  }
  GLUIUpdateResolutionMultiplier();
}

/* ------------------ ParticleShowMenu ------------------------ */

void ParticleShowMenu(int value){
  partdata *parti;
  int i;

  if(global_scase.npartinfo==0)return;
  if(value==MENU_DUMMY)return;
  if(value<0){
    value = -value;
    value--;
    parti = global_scase.partinfo + value;
    parti->display = 1 - parti->display;
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    return;
  }
  if(plotstate==DYNAMIC_PLOTS){
    switch(value){
      case MENU_PARTSHOW_PARTICLES:
        if(visSmokePart==2){
          visSmokePart=0;
        }
        else{
          visSmokePart=2;
        }
        break;
      case MENU_PARTSHOW_DROPLETS:
        visSprinkPart = 1 - visSprinkPart;
        break;
      case MENU_PARTSHOW_SHOWALL:
        visSprinkPart=1;
        visSmokePart=2;
        for(i=0;i<global_scase.npartinfo;i++){
          parti = global_scase.partinfo + i;
          if(parti->loaded==0)continue;
          parti->display=1;
        }
        break;
      case MENU_PARTSHOW_STATIC:
        break;
      case MENU_PARTSHOW_HIDEALL:
        visSprinkPart=0;
        visSmokePart=0;
        for(i=0;i<global_scase.npartinfo;i++){
          parti = global_scase.partinfo + i;
          if(parti->loaded==0)continue;
          parti->display=0;
        }
        break;
      default:
        assert(FFALSE);
        break;
    }
    if(visSprinkPart==1||visSmokePart!=0){
      visParticles=1;
    }
    else{
      visParticles=0;
    }
  }
  else{
    switch(value){
      case 1:
        visSmokePart = 2;
        break;
      case 2:
        visSprinkPart = 1;
        break;
      case 3:
        visSprinkPart=1;
        visSmokePart=2;
        for(i=0;i<global_scase.npartinfo;i++){
          parti = global_scase.partinfo + i;
          if(parti->loaded==0)continue;
          parti->display=1;
        }
        break;
      case 5:
        break;
      default:
        assert(FFALSE);
        break;
    }
    if(visSmokePart!=0||visSprinkPart==1){
      visParticles=1;
    }
  }
  updatemenu=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  GLUTPOSTREDISPLAY;
}

#define MENU_FRAMERATE_Realtime 2001
#define MENU_FRAMERATE_2xRealtime 2002
#define MENU_FRAMERATE_4xRealtime 2004

/* ------------------ FrameRateMenu ------------------------ */

void FrameRateMenu(int value){
  updateUpdateFrameRateMenu=0;
  realtime_flag=0;
  frameinterval=1;
  if(value > 0){
    switch(value){
    case MENU_FRAMERATE_Realtime:
      if(nglobal_times>0){
        if(global_times!=NULL)frameinterval=1000.*(global_times[nglobal_times-1]-global_times[0])/nglobal_times;
      }
      realtime_flag=1;
      break;
    case MENU_FRAMERATE_2xRealtime:
      if(global_times!=NULL)frameinterval=1000.*(global_times[nglobal_times-1]-global_times[0])/nglobal_times;
      frameinterval /= 2.0;
      realtime_flag=2;
      break;
    case MENU_FRAMERATE_4xRealtime:
      if(global_times!=NULL)frameinterval=1000.*(global_times[nglobal_times-1]-global_times[0])/nglobal_times;
      frameinterval /= 4.0;
      realtime_flag=4;
      break;
    default:
      frameinterval = 1000./value;
      if(frameinterval<1.0){frameinterval = 0.0;}
      break;
    }
    if(global_times==NULL&&realtime_flag!=0)updateUpdateFrameRateMenu=1;
  }
  else{
    Keyboard('t',FROM_SMOKEVIEW);
    RenderState(RENDER_OFF);
    FlowDir=1;
  }
  frameratevalue=value;
  updatemenu=1;
  if(opengldefined==1){
    GLUTPOSTREDISPLAY;
  }
  ResetGLTime();
}

/* ------------------ IsoSurfaceTypeMenu ------------------------ */

void IsoSurfaceTypeMenu(int value){
  if(nplot3dloaded>0){
    switch(value){
    case MENU_SURFACE_SMOOTH:
      p3dsurfacesmooth=1;
      p3dsurfacetype=SURFACE_SOLID;
      break;
    case MENU_SURFACE_FACET:
      p3dsurfacesmooth=0;
      p3dsurfacetype=SURFACE_SOLID;
      break;
    case MENU_SURFACE_OUTLINE:
      p3dsurfacetype=SURFACE_OUTLINE;
      break;
    case MENU_SURFACE_POINTS:
      p3dsurfacetype=SURFACE_POINTS;
      break;
    default:
      assert(FFALSE);
      break;
    }
    GLUIUpdatePlot3Dtype();
    updatemenu=1;
    GLUTPOSTREDISPLAY;
  }
}

/* ------------------ IsoSurfaceMenu ------------------------ */

void IsoSurfaceMenu(int value){
  if(nplot3dloaded>0){
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    if(value==1){
      UpdateShowStep(0,ISO);
    }
    if(value==2){
      p3dsurfacesmooth = 1 - p3dsurfacesmooth;
    }
  }
}

/* ------------------ LevelMenu ------------------------ */

void LevelMenu(int value){
  if(nplot3dloaded>0){
    plotiso[plotn-1]=value;
    UpdateShowStep(1,ISO);
    UpdateSurface();
    updatemenu=1;
    GLUTPOSTREDISPLAY;
  }
}

/* ------------------ OpenUrl ------------------------ */

#ifdef pp_OSX
void OpenUrl(char *url){
  char command[1000];

  strcpy(command,"open ");
  strcat(command,url);
  system(command);
}
#endif
#ifdef pp_LINUX
void OpenUrl(char *url){
  char command[1000];

  strcpy(command,"xdg-open ");
  strcat(command,url);
  strcat(command," >& /dev/null");
  system(command);
}
#endif

/* ------------------ HelpMenu ------------------------ */

#ifdef WIN32
#define OPENURL(url) ShellExecute(NULL,"open", url,NULL,NULL,SW_SHOWNORMAL)
#else
#define OPENURL(url) OpenUrl(url)
#endif

void HelpMenu(int value){
  switch(value){
    case MENU_HELP_FORUM:
      OPENURL("https://github.com/firemodels/fds/discussions");
      break;
    case MENU_HELP_FDS_ISSUES:
      OPENURL("https://github.com/firemodels/fds/issues");
       break;
    case MENU_HELP_SMV_ISSUES:
      OPENURL("https://github.com/firemodels/smv/issues");
      break;
    case MENU_HELP_DOWNLOADS:
      OPENURL("https://pages.nist.gov/fds-smv/downloads.html");
      break;
    case MENU_HELP_RELEASENOTES:
      OPENURL("https://pages.nist.gov/fds-smv/smv_readme.html");
      break;
    case MENU_HELP_DOCUMENTATION:
      OPENURL("https://pages.nist.gov/fds-smv/manuals.html");
      break;
    case MENU_HELP_FDSWEB:
      OPENURL("https://pages.nist.gov/fds-smv");
      break;
    case MENU_DUMMY:
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ VectorSkipMenu ------------------------ */

void VectorSkipMenu(int value){
  if(value==-1)return; /* dummy label in menu */
  if(value==MENU_VECTOR_SHOW){       /* toggle vector visibility */
    visVector=1-visVector;
    if(vectorspresent==0)visVector=0;
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    return;
  }
  vectorskip=value;
  visVector=1;
  updatemenu=1;
  update_vectorskip = 1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ TextureShowMenu ------------------------ */

void TextureShowMenu(int value){
  texturedata *texti;
  int i;
  int texture_flag=0;
  int loadall_textures;

  updatefacelists=1;
  if(value>=0){
    texti = global_scase.texture_coll.textureinfo + value;
    texti->display = 1-texti->display;
    for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
      texti = global_scase.texture_coll.textureinfo + i;
      if(texti->loaded==0||texti->used==0)continue;
      if(texti->display==0){
        showall_textures = 0;
        texture_flag =     1;
        break;
      }
    }
    if(texture_flag==0)showall_textures=1;
  }
  else{
    switch(value){
    case MENU_TEXTURE_SHOWALL2:
      // load all textures if none are loaded
      loadall_textures = 1;
      for(i = 0; i < global_scase.texture_coll.ntextureinfo; i++){
        texti = global_scase.texture_coll.textureinfo + i;
        if(texti->loaded == 1 && texti->used == 1&&texti->display==1){
          loadall_textures = 0;
          break;
        }
      }
      // if loadall_textures==1 then fall through and run MENU_TEXTURE_SHOWALL block
      if(loadall_textures == 0)break;
    case MENU_TEXTURE_SHOWALL:
      for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
        texti = global_scase.texture_coll.textureinfo + i;
        if(texti->loaded==0||texti->used==0)continue;
        texti->display=1;
      }
      showall_textures=1;
      break;
    case MENU_TEXTURE_HIDEALL:
      for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
        texti = global_scase.texture_coll.textureinfo + i;
        if(texti->loaded==0||texti->used==0)continue;
        texti->display=0;
      }
      showall_textures=0;
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
  visGeomTextures=0;
  for(i=0;i<global_scase.ngeominfo;i++){
    geomdata *geomi;
    surfdata *surf;
    texturedata *textii=NULL;

    geomi = global_scase.geominfo + i;
    surf = geomi->surfgeom;
    if(global_scase.terrain_texture_coll.terrain_textures!=NULL){
      textii = global_scase.terrain_texture_coll.terrain_textures+iterrain_textures;
    }
    else{
      if(surf!=NULL)textii = surf->textureinfo;
    }
    if(textii!=NULL&&textii->display==1){
      visGeomTextures = 1;
      break;
    }
  }

  for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
    texti = global_scase.texture_coll.textureinfo + i;
    if(texti->loaded==1&&texti->used==1&&texti->display==1){
      if(value!=visBLOCKOutline&&value!=visBLOCKSolidOutline&&value!=visBLOCKHide){
        BlockageMenu(visBLOCKAsInput);
        break;
      }
    }
  }
  GLUIUpdateTextureDisplay();
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ Plot3DShowMenu ------------------------ */

void Plot3DShowMenu(int value){
  int i;

  switch(value){
  case MENU_PLOT3D_DUMMY:
    return;
    break;
  case MENU_PLOT3D_Z:
      visz_all=1-visz_all;
      break;
  case MENU_PLOT3D_Y:
      visy_all=1-visy_all;
      break;
  case MENU_PLOT3D_X:
      visx_all=1-visx_all;
      break;
  case MENU_PLOT3D_CONT:
      switch(contour_type){
        case SHADED_CONTOURS:
          contour_type=STEPPED_CONTOURS;
          break;
        case STEPPED_CONTOURS:
        case LINE_CONTOURS:
          contour_type=SHADED_CONTOURS;
          break;
        default:
          assert(FFALSE);
          break;
      }
      break;
  case MENU_PLOT3D_SHOWALL:
      visx_all=1;
      visy_all=1;
      visz_all=1;
      break;
  case MENU_PLOT3D_HIDEALL:
      visx_all=0;
      visy_all=0;
      visz_all=0;
      plotstate=DYNAMIC_PLOTS;
      break;
   case HIDEALL_PLOT3D:
     show_plot3dfiles = 0;
     Plot3DShowMenu(DISPLAY_PLOT3D);
     break;
   case SHOWALL_PLOT3D:
     show_plot3dfiles = 1;
     Plot3DShowMenu(DISPLAY_PLOT3D);
     break;
   case TOGGLESHOW_PLOT3D:
     show_plot3dfiles = 1 - show_plot3dfiles;
     Plot3DShowMenu(DISPLAY_PLOT3D);
     break;
   case DISPLAY_PLOT3D:
     for(i=0;i<global_scase.nplot3dinfo;i++){
       if(global_scase.plot3dinfo[i].loaded==1)global_scase.plot3dinfo[i].display=show_plot3dfiles;
     }
     break;
   default:
     if(value>=1000){
       if(plotstate==STATIC_PLOTS){
         global_scase.plot3dinfo[value-1000].display=1-global_scase.plot3dinfo[value-1000].display;
       }
       else{
         global_scase.plot3dinfo[value-1000].display=1;
       }
     }
     break;
  }
  plotstate=GetPlotState(STATIC_PLOTS);
  if(plotstate==STATIC_PLOTS&&visiso==1){
    UpdateSurface();
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}


/* ------------------ GridDigitsMenu ------------------------ */

void GridDigitsMenu(int value){
  ngridloc_digits = value;
  GLUIUpdateGridLocation();
}

  /* ------------------ GridSliceMenu ------------------------ */

void GridSliceMenu(int value){
  switch(value){
  case GRID_xy:
    visz_all=1-visz_all;
    if(visz_all==1&&visGrid==0)visGrid=1;
    break;
  case GRID_xz:
    visy_all=1-visy_all;
    if(visy_all==1&&visGrid==0)visGrid=1;
    break;
  case GRID_yz:
    visx_all=1-visx_all;
    if(visx_all==1&&visGrid==0)visGrid=1;
    break;
  case GRID_showall:
    visx_all=1;
    visy_all=1;
    visz_all=1;
    visGrid= GRID_NOPROBE;
    break;
  case GRID_hideall:
    visx_all=0;
    visy_all=0;
    visz_all=0;
    break;
  case MENU_DUMMY:
    break;
  case GRID_grid:
    switch(visGrid){
      case GRID_PROBE:
        visGrid=NOGRID_PROBE;
        break;
      case GRID_NOPROBE:
        visGrid=NOGRID_NOPROBE;
        break;
      case NOGRID_PROBE:
        visGrid=GRID_PROBE;
        break;
      case NOGRID_NOPROBE:
        visGrid=GRID_NOPROBE;
        break;
      default:
        assert(FFALSE);
        break;
    }
    break;
  case GRID_probe:
    switch(visGrid){
      case GRID_PROBE:
        visGrid=GRID_NOPROBE;
        break;
      case GRID_NOPROBE:
        visGrid=GRID_PROBE;
        break;
      case NOGRID_PROBE:
        visGrid=NOGRID_NOPROBE;
        break;
      case NOGRID_NOPROBE:
        visGrid=NOGRID_PROBE;
        break;
      default:
        assert(FFALSE);
        break;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

#ifdef pp_COMPRESS

/* ------------------ CompressMenu ------------------------ */

void CompressMenu(int value){
  if(value==MENU_DUMMY)return;
  switch(value){
  case MENU_CONFIG_SETTINGS:
    GLUIShowBounds(DIALOG_SMOKEZIP);
    break;
  case MENU_ERASECOMPRESS:
    erase_all=1;
    overwrite_all=0;
    GLUIUpdateOverwrite();
    if(compress_threads == NULL){
      compress_threads = THREADinit(&n_compress_threads, &use_compress_threads, Compress);
    }
    THREADrun(compress_threads);
    break;
  case MENU_OVERWRITECOMPRESS:
    erase_all=0;
    overwrite_all=1-overwrite_all;
    GLUIUpdateOverwrite();
    break;
  case MENU_COMPRESSNOW:
    erase_all=0;
    if(compress_threads == NULL){
      compress_threads = THREADinit(&n_compress_threads, &use_compress_threads, Compress);
    }
    THREADrun(compress_threads);
    break;
  case MENU_COMPRESSAUTOLOAD:
    compress_autoloaded=1-compress_autoloaded;
    GLUIUpdateOverwrite();
    break;
  default:
    assert(FFALSE);
    break;
  }
  updatemenu=1;
}
#endif

/* ------------------ IniSubMenu ------------------------ */

void IniSubMenu(int value){
  if(value==MENU_READCASEINI){
    ReadIni(NULL);
  }
  else{
    char *ini_filename;
    char *script_filename2;

    ini_filename = GetIniFileName(value);
    if(ini_filename==NULL||strlen(ini_filename)==0)return;
    script_filename2=script_filename;
    strcpy(script_filename,ini_filename);
    windowresized=0;
    ReadIni(script_filename2);
  }
}

/* ------------------ SmokeviewIniMenu ------------------------ */

void SmokeviewIniMenu(int value){
  switch(value){
  case MENU_READINI:
    ReadIni(NULL);
    UpdateRGBColors(colorbar_select_index);
    break;
  case MENU_REVERT_WRITEINI:
    ReadBinIni();
    break;
  case MENU_WRITEINI:
    WriteIni(GLOBAL_INI,NULL);
    WriteIni(LOCAL_INI,NULL);
    break;
  case MENU_WRITECASEINI:
    WriteIni(LOCAL_INI,NULL);
    break;
  case MENU_READSVO:
    ReadDefaultObjectCollection(&global_scase.objectscoll, global_scase.fdsprefix, global_scase.isZoneFireModel);
    break;
  case MENU_DUMMY:
    break;
  case MENU_CONFIG_SETTINGS:
    GLUIShowBounds(DIALOG_CONFIG);
    break;
  default:
    assert(FFALSE);
    break;
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ PeriodicReloads ------------------------ */

void PeriodicReloads(int value){
  if(periodic_reloads!=0){
    if(load_incremental==1)LoadUnloadMenu(RELOAD_INCREMENTAL_ALL);
    if(load_incremental==0)LoadUnloadMenu(RELOADALL);
    glutTimerFunc((unsigned int)value,PeriodicReloads,value);
  }
}

#ifdef pp_REFRESH
/* ------------------ PeriodicRefresh ------------------------ */

void PeriodicRefresh(int value){
  update_refresh = 0;
  if(periodic_refresh!=0){
    GLUTPOSTREDISPLAY;
    if(glui_refresh_rate>0){
      glutTimerFunc((unsigned int)value, PeriodicRefresh, refresh_interval);
    }
  }
}
#endif

/* ------------------ ScriptMenu2 ------------------------ */

void ScriptMenu2(int value){
  script_step=1;
  GLUIUpdateScriptStep();
  ScriptMenu(value);
}

/* ------------------ ScriptMenu ------------------------ */

void ScriptMenu(int value){
  int error_code;
  scriptfiledata *scriptfile;
  char newscriptfilename[1024];

  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  switch(value){
    case MENU_SCRIPT_SETTINGS:
      GLUIShowBounds(DIALOG_SCRIPT);
      break;
    case SCRIPT_STEP:
      script_step=1-script_step;
      break;
    case SCRIPT_CANCEL:
      script_defer_loading = 0;
      use_iso_threads = use_iso_threads_save;
      current_script_command=NULL;
      runscript=0;
      first_frame_index=0;
      script_startframe=-1;
      script_skipframe=-1;
      script_step=0;
      GLUIScriptEnable();
      break;
    case SCRIPT_CONTINUE:
      script_step=0;
      break;
    case SCRIPT_START_RECORDING2:
      ScriptMenu(SCRIPT_START_RECORDING);
      script_defer_loading = 1;
      break;
    case SCRIPT_START_RECORDING:
      script_defer_loading = 0;
      GLUIUpdateScriptStart();
      GetNewScriptFileName(newscriptfilename);
      script_recording = InsertScriptFile(newscriptfilename);
      scriptoutstream=fopen(newscriptfilename,"w");
      if(scriptoutstream!=NULL){
        PRINTF("Script recorder on\n");
        script_recording->recording=1;
        {
          char *renderdir;

          TrimBack(script_renderdir);
          renderdir = TrimFront(script_renderdir);
          if(strlen(renderdir)>0&&strcmp(renderdir,".")!=0){
            fprintf(scriptoutstream,"RENDERDIR\n");
            fprintf(scriptoutstream," %s\n",renderdir);
          }
          else{
            fprintf(scriptoutstream,"RENDERDIR\n");
            fprintf(scriptoutstream," .\n");
          }
        }
        fprintf(scriptoutstream,"XSCENECLIP\n");
        fprintf(scriptoutstream," %i %f %i %f\n",clipinfo.clip_xmin,clipinfo.xmin,clipinfo.clip_xmax,clipinfo.xmax);
        fprintf(scriptoutstream,"YSCENECLIP\n");
        fprintf(scriptoutstream," %i %f %i %f\n",clipinfo.clip_ymin,clipinfo.ymin,clipinfo.clip_ymax,clipinfo.ymax);
        fprintf(scriptoutstream,"ZSCENECLIP\n");
        fprintf(scriptoutstream," %i %f %i %f\n",clipinfo.clip_zmin,clipinfo.zmin,clipinfo.clip_zmax,clipinfo.zmax);
        fprintf(scriptoutstream,"SCENECLIP\n");
        fprintf(scriptoutstream," %i\n",clip_mode);
      }
      else{
        script_recording->recording=0;
        script_recording=NULL;
        fprintf(stderr,"*** Error: The script file %s could not be opened for writing.",newscriptfilename);
      }
      break;
    case SCRIPT_STOP_RECORDING:
      if(script_recording!=NULL){
        script_recording->recording=0;
        GLUIAddScriptList(script_recording->file,script_recording->id);
        script_recording=NULL;
      }
      if(scriptoutstream!=NULL){
        fclose(scriptoutstream);
        scriptoutstream=NULL;
        PRINTF("Script recorder off\n");
      }
      GLUIUpdateScriptStop();
      break;
    default:
      for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
        char *file;

        file=scriptfile->file;
        if(file==NULL)continue;
        if(scriptfile->id!=value)continue;
        error_code= CompileScript(file);
        switch(error_code){
        case 0:
          StartScript();
          break;
        case 1:
          fprintf(stderr, "***error: unable to open script file");
          if(file != NULL)fprintf(stderr, ": %s", file);
          fprintf(stderr, "\n");
        case 2:
          break;
        default:
          assert(FFALSE);
          break;
        }
        if(error_code != 0){
          fprintf(stderr, "          script aborted\n");
          if(from_commandline == 1)SMV_EXIT(1);
        }
        break;
      }
      break;
  }
}

/* ------------------ ReLoadMenu ------------------------ */

void ReloadMenu(int value){
  int msecs;

  if(value == MENU_DUMMY)return;
  updatemenu=1;
  periodic_reload_value=value;
  switch(value){
  case STOP_RELOADING:
    periodic_reloads=0;
    break;
  case RELOAD_SWITCH:
    if(load_incremental==1)LoadUnloadMenu(RELOAD_INCREMENTAL_ALL);
    if(load_incremental==0)LoadUnloadMenu(RELOADALL);
  break;
  case RELOAD_ALL_NOW:
    LoadUnloadMenu(RELOADALL);
    break;
  case RELOAD_INCREMENTAL_NOW:
    LoadUnloadMenu(RELOAD_INCREMENTAL_ALL);
    break;
  case RELOAD_SMV_FILE:
    UpdateSMVDynamic(smv_filename);
    break;
  case RELOAD_MODE_ALL:
    load_incremental = 0;
    break;
  default:
    periodic_reloads=1;
    msecs = value*60*1000;
    glutTimerFunc((unsigned int)msecs,PeriodicReloads,msecs);
    break;
  }
}

/* ------------------ OutputTImingStats ------------------------ */

#define GIGA 1073741824
#define MEGA 1048576
void OutputTimingStats(int i, float timer, char *label){
  float rate;

  rate = (float)i * (float)GIGA * (float)8 / timer;
  if(rate > (float)GIGA){
    printf("%s stats: time: %.2f s, rate: %.2f Gb/s\n", label, timer, rate / (float)GIGA);
  }
  else if(rate > (float)MEGA){
    printf("%s stats: time: %.2f s, rate: %.2f Mb/s\n", label, timer, rate/(float)MEGA);
  }
  else{
    printf("%s stats: time: %.2f s, rate: %.2f b/s\n",  label, timer, rate);
  }
}

/* ------------------ MemoryTest ------------------------ */

void MemoryTest(void){
  unsigned char *buffer1 = NULL, *buffer2 = NULL;
  int diskread = 0, diskwrite = 1;
  int i;
  int value=1;

  NewMemory((void **)&buffer1, GIGA);
  NewMemory((void **)&buffer2, GIGA);
  memset(buffer1, value, GIGA);
  for(i = 1;i <= 4;i++){
    FILE *stream;
    float mem_timer, diskwrite_timer, diskread_timer;
    int j;

    START_TIMER(mem_timer);
    for(j=0;j<i;j++){
      memcpy(buffer2, buffer1, GIGA);
    }
    STOP_TIMER(mem_timer);

    diskread = 0;
    diskwrite = 0;
    stream = fopen("test.bin", "wb");
    if(stream != NULL){
      diskwrite = 1;
      START_TIMER(diskwrite_timer);
      for(j=0;j<i;j++){
        fwrite(buffer1, 1, GIGA, stream);
      }
      STOP_TIMER(diskwrite_timer);
      fclose(stream);
      stream = fopen("test.bin", "rb");
      if(stream != NULL){
        diskread = 1;
        START_TIMER(diskread_timer);
        for(j = 0;j < i;j++){
          fread(buffer1, 1, GIGA, stream);
        }
        STOP_TIMER(diskread_timer);
      }
    }
    printf("%i GB\n", i);
    OutputTimingStats(i, mem_timer, "memory");
    if(diskread == 1){
      OutputTimingStats(i, diskread_timer, "disk read");
    }
    if(diskwrite == 1){
      OutputTimingStats(i, diskwrite_timer, "disk write");
    }
    if(stream!=NULL){
      fclose(stream);
      stream = fopen("test.bin", "wb");
      if(stream != NULL){
        fwrite(buffer1, 1, 1, stream);
        fclose(stream);
      }
    }
    printf("\n");
  }
  FREEMEMORY(buffer1);
  FREEMEMORY(buffer2);
}

/* ------------------ AboutMenu ------------------------ */

void AboutMenu(int value){
  if(value == ABOUT_DATA_TRANSFER_TEST)MemoryTest();
}

/* ------------------ LoadVolsmoke3DMenu ------------------------ */

void LoadVolsmoke3DMenu(int value){
  if(value == MENU_DUMMY)return;
  updatemenu = 1;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value >= 0){
    meshdata *meshi;
    volrenderdata *vr;

    update_smokecolorbar = 1;
    meshi = global_scase.meshescoll.meshinfo + value;
    vr = meshi->volrenderinfo;
    if(vr->smokeslice != NULL&&vr->fireslice != NULL){
      if(scriptoutstream != NULL){
        fprintf(scriptoutstream, "LOADVOLSMOKE\n");
        fprintf(scriptoutstream, " %i\n", value);
      }
      if(scriptoutstream==NULL||script_defer_loading==0){
        if(read_vol_mesh==VOL_READNONE){
          read_vol_mesh = value;
          ReadVolsmokeAllFramesAllMeshes();
        }
        else{
          fprintf(stderr, "*** Warning: 3D smoke is currently being loaded\n");
          fprintf(stderr, "   Load data when this is complete.\n");
        }
      }
    }
  }
  else if(value == UNLOAD_ALL){  // unload all
    if(read_vol_mesh == VOL_READNONE){
      UnLoadVolsmoke3DMenu(value);
    }
    else{
      if(read_vol_mesh == VOL_UNLOAD){
        fprintf(stderr, "*** Warning: data is currently being unloaded\n");
      }
      else{
        fprintf(stderr, "*** Warning: data is currently being loaded\n");
      }
      fprintf(stderr, "    Continue when this is complete.\n");
    }
  }
  else if(value == LOAD_ALL){  // load all
    update_smokecolorbar = 1;
    if(scriptoutstream != NULL){
      fprintf(scriptoutstream, "LOADVOLSMOKE\n");
      fprintf(scriptoutstream, " -1\n");
    }
    if(read_vol_mesh == VOL_READNONE){
      read_vol_mesh = VOL_READALL;
      if(scriptoutstream==NULL||script_defer_loading==0){
        ReadVolsmokeAllFramesAllMeshes();
      }
    }
    else{
      if(read_vol_mesh == VOL_UNLOAD){
        fprintf(stderr, "*** Warning: data is currently being unloaded\n");
      }
      else{
        fprintf(stderr, "*** Warning: data is currently being loaded\n");
      }
      fprintf(stderr, "    Continue when this is complete.\n");
    }
  }
  else if(value==MENU_VOLSMOKE_SETTINGS){
    GLUIShowBoundsDialog(DLG_3DSMOKE);
  }
  updatemenu = 1;
  ForceIdle();
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ UnloadAllSliceFiles ------------------------ */

void UnloadAllSliceFiles(char *longlabel){
  int i, errorcode;
  int len;

  if(longlabel!=NULL)len = strlen(longlabel);
  for(i=0; i<global_scase.slicecoll.nvsliceinfo; i++){
    vslicedata *vslicei;
    char *label2;
    int len2;

    vslicei = global_scase.slicecoll.vsliceinfo+i;
    if(vslicei->loaded==0)continue;
    if(vslicei->val==NULL)continue;
    label2 = vslicei->val->label.longlabel;
    len2 = strlen(label2);
    if(longlabel==NULL||strncmp(label2,longlabel,MIN(len,len2))!=0){
      ReadVSlice(i,ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
    }
  }
  for(i=0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;
    char *label2;
    int len2;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(slicei->loaded==0||slicei->vloaded==1)continue;
    label2 = slicei->label.longlabel;
    len2 = strlen(label2);
    if(longlabel==NULL||strncmp(label2, longlabel,MIN(len,len2))!=0){
      ReadSlice("", i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
    }
  }
}

/* ------------------ ReloadAllVectorSliceFiles ------------------------ */

void ReloadAllVectorSliceFiles(int load_flag){
  int i, errorcode;

  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    slicei->uvw = 0;
  }
  for(i = 0; i<global_scase.slicecoll.nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo+i;
    vslicei->reload = 0;
    if(vslicei->loaded==1&&vslicei->display==1)vslicei->reload = 1;
    if(vslicei->iu>=0)global_scase.slicecoll.sliceinfo[vslicei->iu].uvw = 1;
    if(vslicei->iv>=0)global_scase.slicecoll.sliceinfo[vslicei->iv].uvw = 1;
    if(vslicei->iw>=0)global_scase.slicecoll.sliceinfo[vslicei->iw].uvw = 1;
  }

    //*** reload vector slice files

#ifndef pp_SLICEFRAME
  for(i = 0; i<global_scase.slicecoll.nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo+i;
    if(vslicei->reload==1){
      ReadVSlice(i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
    }
  }
#endif
  int lastslice=0;

  for(i = global_scase.slicecoll.nvsliceinfo-1; i>=0; i--){
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo+i;
    if(vslicei->reload==1){
      lastslice = i;
      break;
    }
  }
  for(i = 0; i<global_scase.slicecoll.nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo+i;
    if(vslicei->reload==1){
      if(i==lastslice){
        ReadVSlice(i, ALL_FRAMES, NULL, load_flag, SET_SLICECOLOR, &errorcode);
      }
      else{
        ReadVSlice(i, ALL_FRAMES, NULL, load_flag, DEFER_SLICECOLOR, &errorcode);
      }
    }
  }
}

/* ------------------ ReloadAllSliceFiles ------------------------ */

void ReloadAllSliceFiles(int load_flag){
  int ii;
#ifndef pp_SLICEFRAME
  int file_count = 0;
  float load_size = 0.0;
#endif
  float load_time;
  slicedata **reload_slicelist;

  if(global_scase.slicecoll.nsliceinfo == 0)return;
  NewMemory((void **)&reload_slicelist, global_scase.slicecoll.nsliceinfo*sizeof(slicedata *));
  slicefile_labelindex_save = slicefile_labelindex;
  START_TIMER(load_time);

  for(ii=0; ii<global_scase.slicecoll.nsliceinfo; ii++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+ii;
    reload_slicelist[ii] = NULL;
    if(slicei->loaded==1&&slicei->display==1){ // don't reload a slice file that is part of a vector slice
      if(slicei->vloaded==0){
        reload_slicelist[ii] = slicei;
      }
    }
  }
  for(ii = 0; ii < global_scase.slicecoll.nsliceinfo; ii++){
    slicedata *slicei;
    int i;
    int errorcode;

    slicei = reload_slicelist[ii];
    if(slicei==NULL)continue;
    i = slicei-global_scase.slicecoll.sliceinfo;

    if(slicei->slice_filetype == SLICE_GEOM){
#ifdef pp_SLICEFRAME
      ReadGeomData(slicei->patchgeom, slicei, load_flag, ALL_FRAMES, NULL, 0, &errorcode);
#else
      load_size+=ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, 0, &errorcode);
#endif
    }
    else{
#ifdef pp_SLICEFRAME
      ReadSlice(slicei->file, i, ALL_FRAMES, NULL, load_flag, DEFER_SLICECOLOR, &errorcode);
#else
      load_size += ReadSlice(slicei->file, i, ALL_FRAMES, NULL, LOAD, DEFER_SLICECOLOR, &errorcode);
#endif
    }
#ifndef pp_SLICEFRAME
    file_count++;
#endif
  }
  STOP_TIMER(load_time);
  FREEMEMORY(reload_slicelist);
#ifndef pp_SLICEFRAME
  PrintFileLoadTimes(file_count,load_size,load_time);
#endif
  slicefile_labelindex = slicefile_labelindex_save;
}

/* ------------------ LoadPlot2DMenu ------------------------ */

void LoadPlot2DMenu(int value){
  switch(value){
  case MENU_PLOT2D_LOADCSV:
    InitializeDeviceCsvData(UNLOAD);
    InitializeDeviceCsvData(LOAD);
    csv_loaded = 1;
    plot2d_show_plots = 1;
    updatemenu = 1;
    printf("CSV data loaded\n");
    break;
  case MENU_PLOT2D_LOAD:
    LoadPlot2DMenu(MENU_PLOT2D_LOADCSV);
    GLUIShowPlot2D();
    printf("CSV data loaded\n");
    break;
  case MENU_PLOT2D_UNLOAD:
    if(csv_loaded == 1){
      InitializeDeviceCsvData(UNLOAD);
      csv_loaded = 0;
      plot2d_show_plots = 0;
      updatemenu = 1;
      GLUIHidePlot2D();
      printf("CSV data unloaded\n");
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ UnloadSmoke3D ------------------------ */

void UnloadSmoke3D(smoke3ddata *smoke3di){
  smoke3di->request_load = 0;
  smoke3di->soot_density_loaded = 0;
  if(smoke3di->loaded == 0)return;
  FreeSmoke3D(&global_scase, smoke3di);
  smoke3di->loaded  = 0;
  smoke3di->display = 0;
}

/* ------------------ UnloadAllSmoke3D ------------------------ */

void UnloadAllSmoke3D(int type){
  int i;

  if(global_scase.smoke3dcoll.nsmoke3dinfo > 0){
    for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
      smoke3ddata *smoke3di;

      smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
      if(type == -1 || smoke3di->type == type)smoke3di->request_load = 0;
      if(smoke3di->loaded == 0)continue;
      if(type == -1 || smoke3di->type == type){
        UnloadSmoke3D(smoke3di);
      }
    }
    SmokeWrapup();
  }
}

/* ------------------ FileSize2Label ------------------------ */

char *FileSize2Label(char *label, FILE_SIZE bytes){
  char vallabel[256];

  if(bytes >= 0 && bytes < 1000){
    sprintf(label, "%iB", (int)bytes);
  }
  else if(bytes >= 1000 && bytes < 1000000){
    Float2String(vallabel, (float)bytes/1000.0, ncolorlabel_digits, force_fixedpoint);
    sprintf(label, "%sKB", vallabel);
  }
  else if(bytes >= 1000000 && bytes < 1000000000){
    Float2String(vallabel, (float)bytes/1000000.0, ncolorlabel_digits, force_fixedpoint);
    sprintf(label, "%sMB", vallabel);
  }
  else{
    Float2String(vallabel, (float)bytes/1000000000.0, ncolorlabel_digits, force_fixedpoint);
    sprintf(label, "%sGB", vallabel);
  }
  return label;
}

/* ------------------ Plot3DSummary ------------------------ */

void Plot3DSummary(char *label, int count, FILE_SIZE file_size, float timer){
  char size_label[256], time_label[512], time_label2[256];

  sprintf(label, "PLOT3D: loaded %i files, %s", count, FileSize2Label(size_label, file_size));
  Float2String(time_label2, timer, ncolorlabel_digits, force_fixedpoint);
  sprintf(time_label, " in %ss", time_label2);
  strcat(label, time_label);
}

/* ------------------ LoadUnloadMenu ------------------------ */

void LoadUnloadMenu(int value){
  int errorcode;
  int i;

  float load_time;
  int load_flag;

  if(value==MENU_DUMMY)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  switch(value){
  case UNLOADALL:
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"UNLOADALL\n");
    }
    if(global_scase.hvaccoll.nhvacinfo>0){
      LoadHVACMenu(MENU_HVAC_UNLOAD);
    }
    if(nvolrenderinfo>0){
      LoadVolsmoke3DMenu(UNLOAD_ALL);
    }

    LoadVSliceMenu2(UNLOAD_ALL);

    for(i = 0; i < global_scase.slicecoll.nsliceinfo; i++){
      slicedata *slicei;

      slicei = global_scase.slicecoll.sliceinfo + i;
      if(slicei->loaded == 1){
        if(slicei->slice_filetype == SLICE_GEOM){
          ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0, &errorcode);
        }
        else{
          ReadSlice(slicei->file, i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
        }
      }
    }
    for(i = 0; i<global_scase.nplot3dinfo; i++){
      ReadPlot3D("",i,UNLOAD,&errorcode);
    }
    for(i=0;i<global_scase.npatchinfo;i++){
      ReadBoundary(i,UNLOAD,&errorcode);
    }
    for(i=0;i<global_scase.npartinfo;i++){
      ReadPart("",i,UNLOAD,&errorcode);
    }
    for(i=0;i<global_scase.nisoinfo;i++){
      ReadIso("",i,UNLOAD,NULL,&errorcode);
    }
    for(i=0;i<global_scase.nzoneinfo;i++){
      ReadZone(i,UNLOAD,&errorcode);
    }
    if(global_scase.smoke3dcoll.nsmoke3dinfo > 0){
      UnloadAllSmoke3D(-1);
    }
    if(nvolrenderinfo>0){
      UnLoadVolsmoke3DMenu(UNLOAD_ALL);
    }
    if(showdevice_val==1||vis_device_plot!=DEVICE_PLOT_HIDDEN){
      vis_device_plot = DEVICE_PLOT_HIDDEN;
      showdevice_val = 0;
      GLUIUpdateDeviceShow();
    }
    LoadPlot2DMenu(MENU_PLOT2D_UNLOAD);
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    break;
  case RELOADALL:
  case RELOAD_INCREMENTAL_ALL:
#ifdef pp_FRAME
    if(value == RELOADALL){
      load_flag = LOAD;
    }
    else{
      load_flag = RELOAD;
    }
#else
    load_flag = LOAD;
#endif
    THREADcontrol(compress_threads, THREAD_LOCK);
    char *hrr_csv_filename = CasePathHrrCsv(&global_scase);
    if(FileExistsCaseDir(&global_scase, hrr_csv_filename) == YES) {
      ReadHRR(&global_scase, LOAD);
    }
    FREEMEMORY(hrr_csv_filename);

    //*** reload hvac file
      if(global_scase.hvaccoll.hvacductvalsinfo!=NULL&&global_scase.hvaccoll.hvacductvalsinfo->loaded==1){
        LoadHVACMenu(MENU_HVAC_LOAD);
      }

    //*** reload vector slice and slice files

    slicefile_labelindex_save = slicefile_labelindex;
    START_TIMER(load_time);
    SetLoadedSliceBounds(NULL, 0);
    ReloadAllVectorSliceFiles(load_flag);
    ReloadAllSliceFiles(load_flag);
    GLUIHVACSliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
    STOP_TIMER(load_time);
    slicefile_labelindex=slicefile_labelindex_save;

    //*** reload plot3d files

    for(i = 0; i<global_scase.nplot3dinfo; i++){
      global_scase.plot3dinfo[i].finalize=0;
    }
    for(i = global_scase.nplot3dinfo-1; i>=0; i--){
      if(global_scase.plot3dinfo[i].loaded==1){
        global_scase.plot3dinfo[i].finalize = 1;
        break;
      }
    }
    int plot3d_loaded = 0;
    FILE_SIZE total_plot3d_filesize = 0;
    int file_count=0;
    float plot3d_timer;
    START_TIMER(plot3d_timer);
    for(i=0;i<global_scase.nplot3dinfo;i++){
      if(global_scase.plot3dinfo[i].loaded==1){
        plot3d_loaded = 1;
        total_plot3d_filesize += ReadPlot3D(global_scase.plot3dinfo[i].file,i,LOAD,&errorcode);
        file_count++;
      }
    }
    STOP_TIMER(plot3d_timer);
    if(file_count>0){
      char label[256];

      Plot3DSummary(label, file_count, total_plot3d_filesize, plot3d_timer);
      printf("%s\n",label);
    }

    //*** reload boundary files

    for(i = 0;i < global_scase.npatchinfo;i++){
      patchdata *patchi;

      patchi = global_scase.patchinfo + i;
      assert(patchi->loaded==0||patchi->loaded==1);
      if(patchi->loaded == 1){
#ifdef pp_BOUNDFRAME
        if(patchi->structured == YES){
          PRINTF("Loading %s(%s)", patchi->file, patchi->label.shortlabel);
        }
        ReadBoundary(i, load_flag, &errorcode);
#else
        ReadBoundary(i, LOAD,&errorcode);
#endif
      }
    }

    //*** reload 3d smoke files

    for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
      smoke3ddata *smoke3di;

      smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
      if(smoke3di->request_load==1){
#ifdef pp_SMOKEFRAME
        ReadSmoke3D(ALL_SMOKE_FRAMES, i, load_flag, FIRST_TIME, &errorcode);
#else
        ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
#endif
      }
    }

    //*** reload particle files

#ifdef pp_PARTFRAME
    if(value == RELOADALL){
      LoadAllPartFilesMT(LOAD_ALL_PART_FILES);
    }
    else{
      LoadAllPartFilesMT(RELOAD_LOADED_PART_FILES);
    }
#else
    int npartloaded_local = 0;
    for(i=0;i<global_scase.npartinfo;i++){
      partdata *parti;

      parti = global_scase.partinfo+i;
      if(parti->loaded==1){
        npartloaded_local = 1;
        break;
      }
    }
    if(npartloaded_local>0)LoadParticleMenu(MENU_PARTICLE_ALLMESHES);
#endif

    //*** reload isosurface files

    update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
    CancelUpdateTriangles();
    for(i = 0; i<global_scase.nisoinfo; i++){
      isodata *isoi;

      isoi = global_scase.isoinfo + i;
      if(isoi->loaded==0)continue;
#ifdef pp_ISOFRAME
      ReadIso(isoi->file, i, load_flag, NULL, &errorcode);
      if(isoi->frameinfo==NULL||isoi->frameinfo->frames_read>0)printf("\n");
#else
      ReadIso(isoi->file,i,LOAD,NULL,&errorcode);
      printf("\n");
#endif
    }
    if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)ReadIsoGeomWrapup(BACKGROUND);
    update_readiso_geom_wrapup = UPDATE_ISO_OFF;

    // reload .smv file only if a plot3d file is loaded
    if(plot3d_loaded==1)UpdateSMVDynamic(smv_filename);

    //*** reload csv data

    if(csv_loaded == 1){
      LoadPlot2DMenu(MENU_PLOT2D_LOADCSV);
    }

    updatemenu=1;
    GLUTPOSTREDISPLAY;
    THREADcontrol(compress_threads, THREAD_UNLOCK);
    break;
  case SHOWFILES:
    GLUTPOSTREDISPLAY;
    showfiles=1-showfiles;
    updatemenu=1;
    SetSliceParmInfo(&sliceparminfo);
    UpdateSliceMenuLabels(&sliceparminfo);
    UpdateVsliceMenuLabels(&sliceparminfo);
    UpdateSmoke3dMenuLabels();
    UpdateBoundaryMenuLabels();
    UpdateIsoMenuLabels();
    UpdatePartMenuLabels();
    UpdateTourMenuLabels();
    UpdatePlot3dMenuLabels();
    break;
  case COMPUTE_SMV_BOUNDS:
    bounds_each_mesh = 1-bounds_each_mesh;
    updatemenu = 1;
    break;
  case SHOW_BOUND_DIFFS:
    show_bound_diffs = 1-show_bound_diffs;
    updatemenu = 1;
    break;
  case CACHE_FILE_DATA:
    cache_file_data = 1-cache_file_data;
    cache_plot3d_data = cache_file_data;
    cache_boundary_data = cache_file_data;
    cache_part_data = cache_file_data;
    GLUISetCacheFlag(BOUND_PATCH, cache_file_data);
    GLUISetCacheFlag(BOUND_PLOT3D, cache_file_data);
    GLUISetCacheFlag(BOUND_PART, cache_file_data);
#define BOUND_CACHE_DATA               112
    GLUIPatchBoundsCPP_CB(BOUND_CACHE_DATA);
    GLUIHVACSliceBoundsCPP_CB(BOUND_CACHE_DATA);
    GLUIPartBoundsCPP_CB(BOUND_CACHE_DATA);
    updatemenu = 1;
    break;
  case REDIRECT:
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    redirect=1-redirect;
    if(LOG_FILENAME!=NULL){
      fclose(LOG_FILENAME);
      LOG_FILENAME=NULL;
    }
    if(redirect==1){
      char *log_filename = CasePathLogFile(&global_scase);
      LOG_FILENAME=fopen(log_filename,"w");
      FREEMEMORY(log_filename);
      if(LOG_FILENAME==NULL)redirect=0;
    }
    if(redirect==1){
      SetStdOut(LOG_FILENAME);
    }
    else{
      SetStdOut(stdout);
    }
    break;
  case LOAD_WHEN_LOADED:
    load_only_when_unloaded = 1 - load_only_when_unloaded;
    GLUIUpdateLoadWhenLoaded();
    updatemenu = 1;
    break;
  default:
    assert(FFALSE);
    break;
  }
  GLUTSETCURSOR(GLUT_CURSOR_RIGHT_ARROW);
}

/* ------------------ TourMenu ------------------------ */

void TourMenu(int value){
  tourdata *touri;
  int i;

  if(value==MENU_DUMMY)return;
  touring=0;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  switch(value){
  case MENU_TOUR_SETTINGS:
    DialogMenu(DIALOG_TOUR_SHOW);
    break;
  case MENU_TOUR_NEW:
    GLUIAddNewTour();
    DialogMenu(DIALOG_TOUR_SHOW);
    break;
  case MENU_TOUR_CLEARALL:
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){  // clear all tours
      touri = global_scase.tourcoll.tourinfo + i;
      touri->display=touri->display2;
    }
    if(viewtourfrompath==1){
      SetViewPoint(RESTORE_EXTERIOR_VIEW);
    }
    from_glui_trainer=0;
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      touri = global_scase.tourcoll.tourinfo + i;
      if(touri->display==1){
        selected_tour=touri;
        break;
      }
    }
    selected_tour=NULL;
    break;
  case MENU_TOUR_MANUAL:
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){  // clear all tours
      touri = global_scase.tourcoll.tourinfo + i;
      touri->display=0;
    }
    if(viewtourfrompath==1){
      SetViewPoint(RESTORE_EXTERIOR_VIEW);
    }
    from_glui_trainer=0;
    selected_tour=NULL;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"UNLOADTOUR\n");
    }
    DialogMenu(DIALOG_TOUR_HIDE);
    break;
  case MENU_TOUR_SHOWDIALOG:
    edittour=1-edittour;
    if(edittour==1&&showtour_dialog==0){
      GLUIShowTour();
    }
    break;
  case MENU_TOUR_SHOWALL:               // show all tours
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      touri = global_scase.tourcoll.tourinfo + i;
      touri->display=1;
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    break;
  case MENU_TOUR_VIEWFROMROUTE:               // view from route
    viewtourfrompath = 1 - viewtourfrompath;
    if(viewtourfrompath==0)SetViewPoint(RESTORE_EXTERIOR_VIEW);
    break;
  case MENU_TOUR_DEFAULT:
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      touri = global_scase.tourcoll.tourinfo + i;
      touri->display=0;
    }
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
    DefaultTour();
    break;
  default:
    if(value<-22){
      tourlocus_type=2;
      global_scase.objectscoll.iavatar_types=(-value-23);
      if(selectedtour_index>=0&&selectedtour_index<global_scase.tourcoll.ntourinfo){
        global_scase.tourcoll.tourinfo[selectedtour_index].glui_avatar_index=global_scase.objectscoll.iavatar_types;
      }
    }

    //  show one tour

    if(value>=0&&value<global_scase.tourcoll.ntourinfo){
      int j;

      touri = global_scase.tourcoll.tourinfo + value;
      touri->display = 1 - touri->display;
      if(touri->display==1){
        selectedtour_index=value;
        selected_frame=touri->first_frame.next;
        selected_tour=touri;
      }
      else{
        for(j=0;j<global_scase.tourcoll.ntourinfo;j++){
          tourdata *tourj;

          tourj = global_scase.tourcoll.tourinfo + j;
          if(touri==tourj||tourj->display==0)continue;
          selectedtour_index=j;
          selected_frame=tourj->first_frame.next;
          selected_tour=tourj;
          break;
        }
      }
    }
    break;
  }
  UpdateViewTour();
  GLUIDeleteTourList();
  GLUICreateTourList();
  GLUIUpdateTourControls();
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  if(value!=-5&&value!=-4)UpdateTimes();
  callfrom_tourglui=0;

}

/* ------------------ TourCopyMenu ------------------------ */

#define TOUR_INSERT_COPY 33
void TourCopyMenu(int value){
  if(value==-1){
    TourMenu(MENU_TOUR_NEW);
  }
  else{
    selectedtour_index=value;
    TourCB(TOUR_INSERT_COPY);
    DialogMenu(DIALOG_TOUR_SHOW);
  }
}

/* ------------------ SetTour ------------------------ */

void SetTour(tourdata *thetour){
  int tournumber;

  if(thetour==NULL)return;
  tournumber = thetour - global_scase.tourcoll.tourinfo;
  TourMenu(tournumber);
}

/* ------------------ UpdateStreakValue ------------------------ */

void UpdateStreakValue(float value){
  partdata *parti=NULL;
  int i;

  streak_index=-1;
  for(i=0;i<nstreak_rvalue;i++){
    if(ABS(value-streak_rvalue[i])<0.01){
      streak_index=i;
      float_streak5value=streak_rvalue[i];
      break;
    }
  }
  for(i=0;i<global_scase.npartinfo;i++){
    parti = global_scase.partinfo + i;
    if(parti->loaded==1)break;
  }
  if(parti!=NULL&&parti->loaded==1&&parti->ntimes>1){
    for(i=0;i<parti->ntimes-1;i++){
      if(parti->times[i]<=value&&value<parti->times[i+1]){
        streak5step=i;
        break;
      }
    }
  }
}
/* ------------------ ParticleStreakShowMenu ------------------------ */

void ParticleStreakShowMenu(int value){
  float rvalue;

  if(value==-1)return;
  if(value==MENU_STREAK_HIDE){
    SetStreakShow(0);
    streak5step=0;
  }
  else if(value==MENU_STREAK_HEAD){
    showstreakhead=1-showstreakhead;
  }
  else{
    SetStreakShow(1);
    streak5step=0;
    rvalue=streak_rvalue[value];
    UpdateStreakValue(rvalue-0.001);
    GLUIUpdateStreakValue(rvalue);

  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ Particle5ShowMenu ------------------------ */

void Particle5ShowMenu(int value){
}

/* ------------------ PropMenu ------------------------ */

void PropMenu(int value){
  int iprop, iobject;

  // value = iobject*npropinfo + iprop

  iprop = value%global_scase.propcoll.npropinfo;
  iobject = value / global_scase.propcoll.npropinfo;
  if(iprop >= 0 && iprop < global_scase.propcoll.npropinfo){
    propdata *propi;

    propi = global_scase.propcoll.propinfo + iprop;
    if(iobject >= 0 && iobject < propi->nsmokeview_ids){
      int i;

      propi->smokeview_id = propi->smokeview_ids[iobject];
      propi->smv_object = propi->smv_objects[iobject];
      updatemenu = 1;
      GetIndepVarIndices(propi->smv_object,
        propi->vars_indep, propi->nvars_indep,
        propi->vars_indep_index);

      for(i = 0;i < global_scase.npartclassinfo;i++){
        partclassdata *partclassi;

        partclassi = global_scase.partclassinfo + i;
        UpdatePartClassDepend(partclassi);

      }
      GLUTPOSTREDISPLAY;
    }
  }
}

/* ------------------ ParticlePropShowMenu ------------------------ */

void ParticlePropShowMenu(int value){
  partpropdata *propi;

  int propvalue;

  propvalue = (-value)/10000-1;
  value = -((-value)%10000);

  if(value==MENU_PROP_DUMMY)return;

  if(value>=0){
    int iprop;
    int i;

    part5show=1;
    parttype=0;
    iprop = value;
    for(i=0;i<npart5prop;i++){
      propi = part5propinfo + i;
      propi->display=0;
    }

    propi = part5propinfo + iprop;
    last_prop_display=iprop;
    ipart5prop = iprop;
    GLUISetValTypeIndex(BOUND_PART, ipart5prop);

    propi->display=1;
    part5colorindex=iprop;

    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"PARTCLASSCOLOR\n");
      fprintf(scriptoutstream," %s\n",propi->label->longlabel);
    }
    current_property = propi;
    if(iprop!=0){
      parttype=1;
    }
    global_prop_index = iprop;
    partshortlabel=propi->label->shortlabel;
    partunitlabel=propi->label->unit;
#define FILETYPE_INDEX 5
#define BOUND_PERCENTILE_DRAW          120
    GLUIPartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }
  else if(value==MENU_PROP_SHOWALL){
    if(current_property!=NULL){
      unsigned char *vis;
      int i;

      vis = current_property->class_vis;
      for(i=0;i< global_scase.npartclassinfo;i++){
        vis[i]=1;
      }
    }
  }
  else if(value==MENU_PROP_HIDEALL){
    if(current_property!=NULL){
      unsigned char *vis;
      int i;

      vis = current_property->class_vis;
      for(i=0;i< global_scase.npartclassinfo;i++){
        vis[i]=0;
      }
    }

  }
  else if(value==MENU_PROP_HIDEPART){
    int i;
    int unhide=1;

    for(i=0;i<npart5prop;i++){
      propi = part5propinfo + i;
      if(propi->display==1)unhide=0;
      propi->display=0;
    }
    part5show=0;
    parttype=0;
    if(unhide==1&&last_prop_display>=0){
      ParticlePropShowMenu(last_prop_display);
    }
  }
  else if(value==MENU_PROP_HIDEAVATAR){
    part5show=0;
    parttype=0;
  }
  else if(value==MENU_PROP_TRACERS){
    show_tracers_always=1-show_tracers_always;
    GLUIUpdateTracers();
  }
  else{
    int iclass;
    int vistype;

    iclass =  (-value - 10)/5;
    vistype = (-value - 10)%5;
    if(vistype==0){
      if(current_property!=NULL){
        unsigned char *vis;

        vis = current_property->class_vis;
        vis[iclass] = 1 - vis[iclass];
        if(scriptoutstream!=NULL){
          fprintf(scriptoutstream,"PARTCLASSTYPE\n");
          fprintf(scriptoutstream," %s\n",current_property->label->longlabel);
        }
      }
    }
    else{
      partclassdata *partclassj;

      partclassj = global_scase.partclassinfo + iclass;
      partclassj->vis_type=vistype;
      PropMenu(propvalue);
    }
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ UnloadAllPartFiles ------------------------ */

void UnloadAllPartFiles(void){
  int i;

  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;
    int errorcode;

    parti = global_scase.partinfo+i;
    if(parti->loaded==0)continue;
    ReadPart(parti->file, i, UNLOAD, &errorcode);
  }
}

/* ------------------ LoadAllPartFiles ------------------------ */

void LoadAllPartFiles(int partnum){
  int i;

  for(i = 0;i<global_scase.npartinfo;i++){
    partdata *parti;
    int errorcode;
    FILE_SIZE file_size;

    parti = global_scase.partinfo+i;
    if(parti->file == NULL)continue;
#ifdef pp_PARTFRAME
    if(partnum != RELOAD_LOADED_PART_FILES && partnum != LOAD_ALL_PART_FILES){
      IF_NOT_USEMESH_CONTINUE(parti->loaded, parti->blocknumber);
    }
#else
    IF_NOT_USEMESH_CONTINUE(parti->loaded,parti->blocknumber);
#endif
    if(parti->skipload==1)continue;
    if(partnum>=0&&i!=partnum)continue;  //  load only particle file with file index partnum
    THREADcontrol(partload_threads, THREAD_LOCK);                      //  or load all particle files
    if(parti->loadstatus==FILE_UNLOADED
#ifdef pp_PARTFRAME
      || partnum==RELOAD_LOADED_PART_FILES || partnum == LOAD_ALL_PART_FILES
#endif
     ){
      if(partnum==LOAD_ALL_PART_FILES||(partnum==RELOAD_LOADED_PART_FILES&&parti->loaded==1)||partnum==i){
        parti->loadstatus = FILE_LOADING;
        THREADcontrol(partload_threads, THREAD_UNLOCK);
#ifdef pp_PARTFRAME
        if(partnum == RELOAD_LOADED_PART_FILES){
          file_size = ReadPart(parti->file, i, RELOAD, &errorcode);
        }
        else{
          file_size = ReadPart(parti->file, i, LOAD, &errorcode);
        }
#else
        file_size = ReadPart(parti->file, i, LOAD, &errorcode);
#endif
        THREADcontrol(partload_threads, THREAD_LOCK);
        parti->loadstatus = FILE_LOADED;
        part_load_size += file_size;
        part_file_count++;
        parti->file_size = file_size;
      }
    }
    THREADcontrol(partload_threads, THREAD_UNLOCK);
  }
}

/* ------------------ SetupPart ------------------------ */

void SetupPart(int value){
  int i;
  int *list = NULL, nlist = 0;

  NewMemory((void **)&list, global_scase.npartinfo*sizeof(int));
  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo+i;
    if(
      load_only_when_unloaded == 0 &&
      parti->loaded == 1){
      int errorcode;

      ReadPart("", i, UNLOAD, &errorcode);
    }
    if(parti->loaded==1)continue;
    if(parti->loaded==0&&value==PARTFILE_RELOADALL)continue;  // don't reload a file that is not currently loaded
    if(parti->loaded==0&&value>=0&&value!=i)continue;         // if value>0 only load file with index  value
    list[nlist++] = i;
  }
  SetLoadedPartBounds(list, nlist);
  FREEMEMORY(list);
  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo+i;
    parti->finalize = 0;
    parti->skipload = 1;
    parti->loadstatus = FILE_UNLOADED;
    parti->boundstatus = PART_BOUND_UNDEFINED;
    if(parti->loaded==1)continue;
    if(parti->loaded==0&&value==PARTFILE_RELOADALL)continue;  // don't reload a file that is not currently loaded
    if(parti->loaded==0&&value>=0&&value!=i)continue;         // if value>0 only load file with index  value
    parti->skipload = 0;
  }

  if(value>=0){
    partdata *parti;

    assert(value >= 0 && value < global_scase.npartinfo);
    value = CLAMP(value, 0, global_scase.npartinfo - 1);
    parti = global_scase.partinfo+value;
    parti->finalize = 1;
  }
  else{
    for(i = global_scase.npartinfo-1; i>=0; i--){
      partdata *parti;

      parti = global_scase.partinfo+i;
      if(parti->skipload==1)continue;
      parti->finalize = 1;
      break;
    }
  }
}

/* ------------------ MtLoadAllPartFiles ------------------------ */

void *MtLoadAllPartFiles(void *arg){
  int *valptr;

  valptr = (int *)(arg);
  LoadAllPartFiles(*valptr);
#ifdef pp_PARTFRAME
  return NULL;
#else
  THREAD_EXIT(partload_threads);
#endif
}

/* ------------------ LoadAllPartFilesMT ------------------------ */

void LoadAllPartFilesMT(int partnum){
  int i;

  INIT_PRINT_TIMER(part_load_timer);
#ifdef pp_PARTFRAME
  LoadAllPartFiles(partnum);
#else
  if(partload_threads == NULL){
    partload_threads = THREADinit(&n_partload_threads, &use_partload_threads, MtLoadAllPartFiles);
  }
  int partnuminfo[1];
  partnuminfo[0] = partnum;
  THREADruni(partload_threads, (unsigned char *)partnuminfo, 0);
  THREADcontrol(partload_threads, THREAD_JOIN);
#endif
  PRINT_TIMER(part_load_timer, "LoadAllPartFilesMT");

  INIT_PRINT_TIMER(part_timer);
  if(partnum < 0){
    for(i = 0; i < global_scase.npartinfo; i++){
      partdata *parti;

      parti = global_scase.partinfo + i;
      parti->finalize = 0;
    }
    for(i = global_scase.npartinfo - 1; i >= 0; i--){
      partdata *parti;

      parti = global_scase.partinfo + i;
      if(parti->loaded == 1){
        parti->finalize = 1;
        FinalizePartLoad(parti);
        break;
      }
    }
  }
  else{
    FinalizePartLoad(global_scase.partinfo + partnum);
  }
  PRINT_TIMER(part_timer, "finalize particle time");
}

/* ------------------ LoadParticleMenu ------------------------ */

void LoadParticleMenu(int value){
  int errorcode,i;

  if(value==MENU_DUMMY)return;
  part_load_size = 0;
  part_file_count = 0;
  global_part_boundsize = 0;
  global_have_global_bound_file = 0;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    char  *partfile;
    partdata *parti;

    parti = global_scase.partinfo + value;
    partfile = parti->file;
    parti->finalize = 1;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOADFILE\n");
      fprintf(scriptoutstream," %s\n",partfile);
    }
    npartframes_max=GetMinPartFrames(PARTFILE_RELOADALL);
    npartframes_max=MAX(GetMinPartFrames(value),npartframes_max);
    if(scriptoutstream==NULL||script_defer_loading==0){
      SetupPart(value);                                                // load only particle file with index value
      LoadAllPartFilesMT(value);
      if(global_scase.partinfo[value].file_size == 0.0)printf("***warning: particle file has no particles\n");
    }
  }
  else{
    if(value==MENU_PARTICLE_UNLOAD_ALL){
      for(i=0;i<global_scase.npartinfo;i++){
        ReadPart("", i, UNLOAD, &errorcode);
      }
    }
    else if(value==MENU_PART_SETTINGS){
      GLUIShowBoundsDialog(DLG_PART);
    }
    else if(value==MENU_PART_PARTFAST){
      updatemenu = 1;
      partfast = 1-partfast;
      if(partfast==0)printf("fast loading: off\n");
      if(partfast==1)printf("fast loading: on\n");
      GLUIUpdatePartFast();
    }
    else if(value == MENU_PART_NUM_FILE_SIZE){
      int total = 0;
      for(i = 0;i < global_scase.npartinfo;i++){
        partdata *parti;

        parti = global_scase.partinfo + i;
        total += parti->npoints_file;
      }
      printf("Particle number/file size: %i/", total);
      FILE_SIZE total_size;

      total_size=0;
      for(i = 0; i < global_scase.npartinfo; i++){
        partdata *parti;

        parti = global_scase.partinfo + i;
        total_size += GetFileSizeSMV(parti->reg_file);
      }
      if(total_size > 1000000000){
        PRINTF("%.1f GB\n", (float)total_size / 1000000000.);
      }
      else if(total_size > 1000000){
        PRINTF("%.1f MB\n", (float)total_size / 1000000.);
      }
      else{
        PRINTF("%.0f kB\n", (float)total_size / 1000.);
      }
    }
    else{
      if(scriptoutstream!=NULL){
        fprintf(scriptoutstream,"LOADPARTICLES\n");
      }
      if(value == PARTFILE_LOADALL){
        SetupPart(value);
      }
      if(value==PARTFILE_LOADALL){
        npartframes_max=GetMinPartFrames(PARTFILE_LOADALL);
      }
      else{
        npartframes_max=GetMinPartFrames(PARTFILE_RELOADALL);
      }
      if(scriptoutstream==NULL||script_defer_loading==0){

        SetupPart(value);

        // unload particle files

        if(
          load_only_when_unloaded==0&&
          value!=PARTFILE_RELOADALL){
          UnloadAllPartFiles();
        }

        // load particle files unless we are reloading and they were not loaded before

        START_TIMER(part_load_time);
        int have_particles = 0, load_particles=0;
        for(i = 0; i<global_scase.npartinfo; i++){
          partdata *parti;

          parti = global_scase.partinfo+i;
          parti->file_size = 0;
          if(parti->skipload==0)load_particles = 1;
        }
        if(load_particles==1){
          LoadAllPartFilesMT(LOAD_ALL_PART_FILES);
          for(i = 0; i<global_scase.npartinfo; i++){
            partdata *parti;

            parti = global_scase.partinfo+i;
            if(parti->file_size>0){
              have_particles = 1;
              break;
            }
          }
          STOP_TIMER(part_load_time);
#ifndef pp_PARTFRAME
          PrintFileLoadTimes(part_file_count,part_load_size,part_load_time);
#endif
          if(have_particles==0)printf("***warning: particle files have no particles\n");
        }

        force_redisplay = 1;
        UpdateFrameNumber(0);
      }
    }
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ ZoneMenu ------------------------ */

void ZoneMenu(int value){
  int i,errorcode;
  if(value>=0){
    if(scriptoutstream!=NULL){
      zonedata *zonei;

      zonei = global_scase.zoneinfo + value;
      fprintf(scriptoutstream,"LOADFILE\n");
      fprintf(scriptoutstream," %s\n",zonei->file);
    }
    ReadZone(value,LOAD,&errorcode);
  }
  else{
    for(i=0;i<global_scase.nzoneinfo;i++){
      ReadZone(i,UNLOAD,&errorcode);
    }
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ UnloadVSliceMenu ------------------------ */

void UnloadVSliceMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    ReadVSlice(value,ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR, &errorcode);
  }
  else if(value==UNLOAD_ALL){
    int lastslice=0;

    for(i = global_scase.slicecoll.nvsliceinfo-1; i>=0; i--){
      vslicedata *vslicei;

      vslicei = global_scase.slicecoll.vsliceinfo+i;
      if(vslicei->loaded==1){
        lastslice = i;
        break;
      }
    }
    for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
      vslicedata *vslicei;

      vslicei = global_scase.slicecoll.vsliceinfo+i;
      if(vslicei->loaded==1){
        if(lastslice==i){
          ReadVSlice(i, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR, &errorcode);
        }
        else{
          ReadVSlice(i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
        }
      }
    }
  }
  else if(value==-2){
    assert(FFALSE);
  }
}

/* ------------------ UnloadBoundaryMenu ------------------------ */

void UnloadBoundaryMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    ReadBoundary(value,UNLOAD,&errorcode);
  }
  else{
    for(i=0;i<global_scase.npatchinfo;i++){
      patchdata *patchi;

      patchi = global_scase.patchinfo+i;
      if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label, "INCLUDE_GEOM")!=0){
        ReadBoundary(i,UNLOAD,&errorcode);
      }
    }
  }
}

/* ------------------ UnloadPlot3dMenu ------------------------ */

void UnloadPlot3dMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    ReadPlot3D("",value,UNLOAD,&errorcode);
  }
  else{
    for(i=0;i<global_scase.nplot3dinfo;i++){
      ReadPlot3D("",i,UNLOAD,&errorcode);
    }
  }
}

/* ------------------ LoadVSliceMenu2 ------------------------ */

FILE_SIZE LoadVSliceMenu2(int value){
  int errorcode;
  int i;
  FILE_SIZE return_filesize = 0;

  if(value==MENU_DUMMY)return 0;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value==UNLOAD_ALL){
    int lastslice=0;

    for(i=global_scase.slicecoll.nvsliceinfo-1;i>=0;i--){
      vslicedata *vslicei;

      vslicei = global_scase.slicecoll.vsliceinfo + i;
      if(vslicei->loaded==1){
        lastslice = i;
        break;
      }
    }
    for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
      vslicedata *vslicei;

      vslicei = global_scase.slicecoll.vsliceinfo + i;
      if(vslicei->loaded==1){
        if(lastslice==i){
          ReadVSlice(i,ALL_FRAMES, NULL,  UNLOAD, SET_SLICECOLOR, &errorcode);
        }
        else{
          ReadVSlice(i,ALL_FRAMES, NULL,  UNLOAD, DEFER_SLICECOLOR, &errorcode);
        }
      }
    }
    return 0;
  }
  else if(value==MENU_LOADVSLICE_SETTINGS){
    GLUIShowBoundsDialog(DLG_SLICE);
  }
  else if(value>=0){
    vslicedata *vslicei;
    slicedata *slicei;

    return_filesize = ReadVSlice(value, ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR, &errorcode);
    vslicei = global_scase.slicecoll.vsliceinfo + value;
    slicei = vslicei->val;
    if(script_multivslice==0&&slicei!=NULL&&scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOADVSLICEM\n");
      fprintf(scriptoutstream," %s\n", slicei->label.longlabel);
      fprintf(scriptoutstream," %i %f\n", slicei->idir, slicei->position_orig);
      fprintf(scriptoutstream," %i\n", slicei->blocknumber+1);
    }
  }
  else{
    int submenutype;
    char *submenulabel;
    vslicedata *vslicei;
    slicedata *slicei;
    int dir;
#ifndef pp_SLICEFRAME
    int file_count = 0;
    float load_size = 0.0;
#endif
    float load_time;
    int lastslice=0;

    value = -(1000 + value);
    submenutype=value/4;
    dir=value%4;
    submenutype=global_scase.subvslice_menuindex[submenutype];
    vslicei = global_scase.slicecoll.vsliceinfo + submenutype;
    slicei = global_scase.slicecoll.sliceinfo + vslicei->ival;
    submenulabel = slicei->label.longlabel;
    START_TIMER(load_time);

    for(i = global_scase.slicecoll.nvsliceinfo-1; i>=0; i--){
      char *longlabel;

      vslicei = global_scase.slicecoll.vsliceinfo+i;
      slicei = global_scase.slicecoll.sliceinfo+vslicei->ival;
      longlabel = slicei->label.longlabel;
      if(strcmp(longlabel, submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicei->idir)continue;
      lastslice = i;
      break;
    }
    for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
      char *longlabel;

      vslicei = global_scase.slicecoll.vsliceinfo + i;
      slicei=global_scase.slicecoll.sliceinfo + vslicei->ival;
      longlabel = slicei->label.longlabel;
      if(strcmp(longlabel,submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicei->idir)continue;
#ifndef pp_SLICEFRAME
      file_count++;
#endif
#ifdef pp_SLICEFRAME
      if(lastslice==i){
        ReadVSlice(i,ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR, &errorcode);
      }
      else{
        ReadVSlice(i,ALL_FRAMES, NULL, LOAD, DEFER_SLICECOLOR, &errorcode);
      }
#else
      if(lastslice==i){
        load_size+=ReadVSlice(i,ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR, &errorcode);
      }
      else{
        load_size+=ReadVSlice(i,ALL_FRAMES, NULL, LOAD, DEFER_SLICECOLOR, &errorcode);
      }
#endif
    }
    STOP_TIMER(load_time);
#ifndef pp_SLICEFRAME
    PrintFileLoadTimes(file_count,load_size,load_time);
#endif
  }
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
  return return_filesize;
}


/* ------------------ LoadVSliceMenu ------------------------ */

void LoadVSliceMenu(int value){
  LoadVSliceMenu2(value);
}

/* ------------------ UnloadSliceMenu ------------------------ */

void UnloadSliceMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+value;

    if(slicei->slice_filetype==SLICE_GEOM){
      ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0, &errorcode);
    }
    else{
      ReadSlice("", value, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR, &errorcode);
    }
  }
  if(value<=-3){
    UnloadBoundaryMenu(-3-value);
  }
  else{
    if(value==UNLOAD_ALL){
      for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo+i;
        if(slicei->slice_filetype == SLICE_GEOM){
          ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0, &errorcode);
        }
        else{
          ReadSlice("",i, ALL_FRAMES, NULL, UNLOAD,DEFER_SLICECOLOR,&errorcode);
        }
      }
      for(i=0;i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
          UnloadBoundaryMenu(i);
        }
      }
    }
  }
}

/* ------------------ UnLoadMultiVSliceMenu ------------------------ */

void UnloadMultiVSliceMenu(int value){
  int i;
  multivslicedata *mvslicei;

  if(value>=0){
    mvslicei = global_scase.slicecoll.multivsliceinfo + value;
    for(i=0;i<mvslicei->nvslices;i++){
      UnloadSliceMenu(mvslicei->ivslices[i]);
    }
  }
  else{
    UnloadSliceMenu(UNLOAD_ALL);
  }
}

/* ------------------ UnLoadMultiSliceMenu ------------------------ */

void UnloadMultiSliceMenu(int value){
  int i;
  multislicedata *mslicei;

  if(value>=0){
    mslicei = global_scase.slicecoll.multisliceinfo + value;
    for(i=0;i<mslicei->nslices;i++){
      UnloadSliceMenu(mslicei->islices[i]);
    }
  }
  else if(value<=-3){
    UnloadSliceMenu(value);
  }
  else{
    UnloadSliceMenu(UNLOAD_ALL);
  }
}

/* ------------------ ShowVolsmoke3DMenu ------------------------ */

void ShowVolsmoke3DMenu(int value){
  int i;

  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + value;
    vr = meshi->volrenderinfo;
    if(vr->fireslice!=NULL||vr->smokeslice!=NULL){
      if(vr->loaded==1){
        vr->display=1-vr->display;
        PRINTF("%s vis state:%i\n",meshi->label,vr->display);
      }
    }
  }
  else{
    if(value==HIDE_VOLSMOKE){  // hide all
      show_volsmokefiles=0;
    }
    else if(value==SHOW_VOLSMOKE){  // show all
      show_volsmokefiles=1;
    }
    else if(value==TOGGLE_VOLSMOKE){  // show all
      show_volsmokefiles=1-show_volsmokefiles;
    }
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = global_scase.meshescoll.meshinfo + i;
      vr = meshi->volrenderinfo;
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==1){
        vr->display=show_volsmokefiles;
        PRINTF("%s vis state:%i\n",meshi->label,vr->display);
      }
    }
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ UnLoadVolsmoke3DMenu ------------------------ */

void UnLoadVolsmoke3DMenu(int value){
  int i;

  if(value==MENU_DUMMY)return;
  read_vol_mesh=VOL_UNLOAD;
  updatemenu=1;
  if(value<0){
    if(value==UNLOAD_ALL){
      for(i=0;i<global_scase.meshescoll.nmeshes;i++){
        meshdata *meshi;
        volrenderdata *vr;

        meshi = global_scase.meshescoll.meshinfo + i;
        vr = meshi->volrenderinfo;
        if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
        if(vr->loaded==1){
          UnloadVolsmokeAllFrames(vr);
        }
      }
    }
  }
  else{
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + value;
    vr = meshi->volrenderinfo;
    if(vr->fireslice!=NULL||vr->smokeslice!=NULL){
      UnloadVolsmokeAllFrames(vr);
    }
  }
  updatemenu=1;
  read_vol_mesh=VOL_READNONE;
  GLUTPOSTREDISPLAY;
}

/* ------------------ UnLoadSmoke3DMenu ------------------------ */

void UnLoadSmoke3DMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;
  if(value<0){
    value= -value-1;
    UnloadAllSmoke3D(value);
  }
  else{
    UnloadSmoke3D(global_scase.smoke3dcoll.smoke3dinfo + value);
    SmokeWrapup();
  }
}

/* ------------------ IsSmokeType ------------------------ */

int IsSmokeType(smoke3ddata *smoke3di, int type){
  if(smoke3di->type == SOOT_index   && (type&1) != 0)return 1;
  if(smoke3di->type == HRRPUV_index && (type&2) != 0)return 1;
  if(smoke3di->type == TEMP_index   && (type&4) != 0)return 1;
  if(smoke3di->type == CO2_index    && (type&8) != 0)return 1;
  return 0;
}

/* ------------------ LoadSmoke3D ------------------------ */

FILE_SIZE LoadSmoke3D(int type, int frame, int *count, float *time_value){
  int last_smoke = 0, i, file_count=0,errorcode;
  FILE_SIZE load_size=0;

  if(load_only_when_unloaded == 0){
    for(i = global_scase.smoke3dcoll.nsmoke3dinfo - 1; i >= 0; i--){
      smoke3ddata *smoke3di;

      smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
      if(smoke3di->loaded==1&&IsSmokeType(smoke3di, type) == 1){
        ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
      }
    }
  }
  for(i = global_scase.smoke3dcoll.nsmoke3dinfo-1; i>=0; i--){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    IF_NOT_USEMESH_CONTINUE(smoke3di->loaded,smoke3di->blocknumber);
    if(IsSmokeType(smoke3di, type) == 1){
    last_smoke = i;
    break;
    }
  }
  smoke3d_compression_type = COMPRESSED_UNKNOWN;
  for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    IF_NOT_USEMESH_CONTINUE(smoke3di->loaded,smoke3di->blocknumber);
    if(IsSmokeType(smoke3di, type) == 1){
      file_count++;
      smoke3di->finalize = 0;
      if(i == last_smoke)smoke3di->finalize = 1;
      load_size += ReadSmoke3D(frame, i, LOAD, FIRST_TIME, &errorcode);
    }
  }
  if(verbose_output == 0)PRINTF("\n");
  *count = file_count;
  return load_size;
}

/* ------------------ LoadSmoke3DMenu ------------------------ */

void LoadSmoke3DMenu(int value){
  int i,errorcode;
  int file_count;
  float load_time;
#ifndef pp_SMOKEFRAME
  float load_size;
#endif

#define MENU_DUMMY_SMOKE           -9
#define MENU_SMOKE_SETTINGS        -4
#define MENU_SMOKE_FILE_SIZES     -10

  if(value == MENU_DUMMY_SMOKE)return;
  START_TIMER(load_time);
#ifndef pp_SMOKEFRAME
  load_size = 0.0;
#endif
  file_count=0;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    if(scriptoutstream!=NULL){
      char *file;

      file = global_scase.smoke3dcoll.smoke3dinfo[value].file;
      fprintf(scriptoutstream,"LOADFILE\n");
      fprintf(scriptoutstream," %s\n",file);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      smoke3ddata *smoke3di;

      smoke3di = global_scase.smoke3dcoll.smoke3dinfo + value;
      smoke3di->finalize = 1;
      if(smoke3di->extinct>0.0){ // only load one smoke type at a time
        int j, add_blank=0;

        for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
          smoke3ddata *smoke3dj;

          smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
          if(smoke3dj->loaded==1&&smoke3dj->extinct>0.0&&smoke3di->type!=smoke3dj->type){
            PRINTF("Unloading %s(%s)\n", smoke3dj->file, smoke3dj->label.shortlabel);
            ReadSmoke3D(ALL_SMOKE_FRAMES, j, UNLOAD, FIRST_TIME, &errorcode);
            add_blank = 1;
          }
        }
        if(add_blank==1)printf("\n");
      }
      if(load_only_when_unloaded == 0){
        ReadSmoke3D(ALL_SMOKE_FRAMES, value, UNLOAD, FIRST_TIME, &errorcode);
      }
      for(i = 0;i < 1;i++){
        IF_NOT_USEMESH_CONTINUE(smoke3di->loaded, smoke3di->blocknumber);
        ReadSmoke3D(ALL_SMOKE_FRAMES, value, LOAD, FIRST_TIME, &errorcode);
      }
    }
  }
  else if(value==UNLOAD_ALL){
    for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
    }
  }
  else if(value == MENU_SMOKE_SETTINGS){
    GLUIShowBoundsDialog(DLG_3DSMOKE);
  }
  else if(value ==MENU_SMOKE_FILE_SIZES){
    if(global_scase.smoke3dcoll.nsmoke3dtypes>0){
      int ii;

      PRINTF("3D smoke file sizes:\n");
      for(ii = 0; ii<global_scase.smoke3dcoll.nsmoke3dtypes; ii++){
        FILE_SIZE total_size;
        char *smoke_type;

        total_size=0;
        smoke_type = NULL;
        for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
          smoke3ddata *smoke3di;

          smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
          if(smoke3di->type == ii){
            total_size += GetFileSizeSMV(smoke3di->reg_file);
            smoke_type = smoke3di->label.longlabel;
          }
        }
        if(smoke_type != NULL){
          printf("  %s: ", smoke_type);
          if(total_size > 1000000000){
            PRINTF("%.1f GB\n", (float)total_size / 1000000000.);
          }
          else if(total_size > 1000000){
            PRINTF("%.1f MB\n", (float)total_size / 1000000.);
          }
          else{
            PRINTF("%.0f kB\n", (float)total_size / 1000.);
          }
        }
      }
    }
  }
  else if(value<=-100){
    smoke3ddata *smoke3di;

    value = -(value + 100);
    smoke3di = global_scase.smoke3dcoll.smoke3dtypes[value].smoke3d;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOAD3DSMOKE\n");
      fprintf(scriptoutstream," %s\n",smoke3di->label.longlabel);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      if(smoke3di->extinct>0.0){ // only load one smoke type at a time
        int j, add_blank=0;

        for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
          smoke3ddata *smoke3dj;

          smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
          if(smoke3dj->loaded==1&&smoke3dj->extinct>0.0&&smoke3di->type!=smoke3dj->type){
            PRINTF("Unloading %s(%s)\n", smoke3dj->file, smoke3dj->label.shortlabel);
            ReadSmoke3D(ALL_SMOKE_FRAMES, j, UNLOAD, FIRST_TIME, &errorcode);
            add_blank = 1;
          }
        }
        if(add_blank==1)printf("\n");
      }
      int type;
      type = 1;
      if(smoke3di->type == HRRPUV_index)type = 2;
      if(smoke3di->type == TEMP_index)type = 4;
      if(smoke3di->type == CO2_index)type = 8;
#ifdef pp_SMOKEFRAME
      LoadSmoke3D(type, ALL_SMOKE_FRAMES, &file_count, NULL);
#else
      load_size=LoadSmoke3D(type, ALL_SMOKE_FRAMES, &file_count, NULL);
#endif
    }
  }
  STOP_TIMER(load_time);
#ifndef pp_SMOKEFRAME
  PrintFileLoadTimes(file_count, load_size, load_time);
#endif
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ AnySmoke ------------------------ */

int AnySmoke(void){

  if(global_scase.smoke3dcoll.nsmoke3dinfo>0)return 1;
  return 0;
}

/* ------------------ AnySlices ------------------------ */

int AnySlices(const char *type){
  int i;

  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    if(STRCMP(global_scase.slicecoll.sliceinfo[i].label.longlabel,type)==0)return 1;
  }
  return 0;
}

/* ------------------ LoadSlicei ------------------------ */

FILE_SIZE LoadSlicei(int set_slicecolor, int value, int time_frame, float *time_value){
  slicedata *slicei;
  int errorcode;
  FILE_SIZE return_filesize=0;

  SNIFF_ERRORS("LoadSlicei: start");
  slicei = global_scase.slicecoll.sliceinfo + value;
  slicei->loading=1;
  if(script_multislice == 0 && scriptoutstream != NULL){
    fprintf(scriptoutstream, "LOADSLICEM\n");
    fprintf(scriptoutstream, " %s\n", slicei->label.longlabel);
    fprintf(scriptoutstream, " %i %f\n", slicei->idir, slicei->position_orig);
    fprintf(scriptoutstream, " %i\n", slicei->blocknumber + 1);
  }
  if(scriptoutstream==NULL||script_defer_loading==0){
    {
      if(slicei->slice_filetype == SLICE_GEOM){
        return_filesize = ReadGeomData(slicei->patchgeom, slicei, LOAD, time_frame, time_value, 0, &errorcode);
      }
      else{
        return_filesize = ReadSlice(slicei->file, value, time_frame, time_value, LOAD, set_slicecolor, &errorcode);
      }
    }
  }
  slicei->loading=0;
  CheckMemory;
  return return_filesize;
}

/* ------------------ LoadAllSliceFiles ------------------------ */

FILE_SIZE LoadAllSliceFiles(int last_slice, char *submenulabel, int dir, int *fcount){
  int i, file_count=0, errorcode;
  FILE_SIZE load_size = 0;

  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    char *longlabel;
    int set_slicecolor;
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    longlabel = slicei->label.longlabel;
    if(strcmp(longlabel, submenulabel)!=0)continue;
    if(dir!=0&&dir!=slicei->idir)continue;
    set_slicecolor = DEFER_SLICECOLOR;
    if(i==last_slice)set_slicecolor = SET_SLICECOLOR;
    if(slicei->slice_filetype==SLICE_GEOM){
      load_size += ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, 0, &errorcode);
    }
    else{
      load_size += ReadSlice(slicei->file, i, ALL_FRAMES, NULL, LOAD, set_slicecolor, &errorcode);
    }
    file_count++;
  }
  *fcount = file_count;
  return load_size;
}

/* ------------------ LoadSliceMenu ------------------------ */

void LoadSliceMenu(int value){
  int errorcode,i;
#ifndef pp_SLICEFRAME
  float load_time, load_size = 0.0;
  int file_count=0;
#endif

  SNIFF_ERRORS("LoadSliceMenu: start");
  if(value==MENU_DUMMY)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    SetLoadedSliceBounds(&value, 1);
    LoadSlicei(SET_SLICECOLOR,value, ALL_FRAMES, NULL);
  }
  else{
    switch(value){
      slicedata *slicei;
#ifndef pp_SLICEFRAME
      int submenutype;
      char *submenulabel;
      int dir;
      int last_slice;
#endif

      case UNLOAD_ALL:
        for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
          slicei = global_scase.slicecoll.sliceinfo + i;
          if(slicei->loaded == 1){
            if(slicei->slice_filetype == SLICE_GEOM){
              ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0, &errorcode);
            }
            else{
              ReadSlice("",i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
            }
          }
        }
        UpdateTimes();
        break;
      case MENU_SHOWSLICE_IN_GAS:
        GLUISliceInObstMenu2Dialog(ONLY_IN_GAS);
        break;
      case  MENU_SHOWSLICE_IN_GASANDSOLID:
        GLUISliceInObstMenu2Dialog(GAS_AND_SOLID);
        break;
      case MENU_SHOWSLICE_IN_SOLID:
        GLUISliceInObstMenu2Dialog(ONLY_IN_SOLID);
        break;
      case MENU_SLICE_SETTINGS:
        GLUIShowBoundsDialog(DLG_SLICE);
        break;
#ifdef pp_SLICEFRAME
      default:
        break;
#else
      default:
        value = -(1000 + value);
        submenutype=value/4;
        submenutype=global_scase.subslice_menuindex[submenutype];
        slicei = global_scase.slicecoll.sliceinfo + submenutype;
        submenulabel = slicei->label.longlabel;
        dir=value%4;
        last_slice = global_scase.slicecoll.nsliceinfo - 1;
        for(i = global_scase.slicecoll.nsliceinfo-1; i>=0; i--){
          char *longlabel;

          slicei = global_scase.slicecoll.sliceinfo + i;
          longlabel = slicei->label.longlabel;
          if(strcmp(longlabel, submenulabel) != 0)continue;
          if(dir != 0 && dir != slicei->idir)continue;
          last_slice = i;
          break;
        }
        START_TIMER(load_time);
        load_size = LoadAllSliceFiles(last_slice, submenulabel, dir, &file_count);
        STOP_TIMER(load_time);
        PrintFileLoadTimes(file_count,load_size,load_time);
        break;
#endif
      }
  }
  updatemenu=1;
  CheckMemory;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadMultiVSliceMenu ------------------------ */

void LoadMultiVSliceMenu(int value){
  int i;
#ifndef pp_SLICEFRAME
  int file_count = 0;
  float load_size = 0.0;
#endif
  float load_time;

  if(value==MENU_DUMMY)return;
  if(value>=0){
    multivslicedata *mvslicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo + value;
    if(scriptoutstream!=NULL){
      if(mvslicei->nvslices>0){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + mvslicei->ivslices[0];
        fprintf(scriptoutstream,"LOADVSLICE\n");
        fprintf(scriptoutstream," %s\n",slicei->label.longlabel);
        fprintf(scriptoutstream," %i %f\n",slicei->idir,slicei->position_orig);
        script_multivslice=1;
      }
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      char *longlabel=NULL;
      vslicedata *vslice1;

      vslice1 = global_scase.slicecoll.vsliceinfo+mvslicei->ivslices[0];
      if(vslice1->ival>=0)longlabel = global_scase.slicecoll.sliceinfo[vslice1->ival].label.longlabel;
      UnloadAllSliceFiles(longlabel); // unload all vector slices except for the type being loaded now
      if(load_only_when_unloaded == 0){
        for(i = 0; i < mvslicei->nvslices; i++){
          UnloadVSliceMenu(mvslicei->ivslices[i]);
        }
      }

      START_TIMER(load_time);
      for(i = 0; i<mvslicei->nvslices; i++){
        vslicedata *vslicei;

        vslicei = global_scase.slicecoll.vsliceinfo+mvslicei->ivslices[i];
        vslicei->finalize = 0;
      }
      for(i = mvslicei->nvslices-1; i>=0; i--){
        vslicedata *vslicei;

        vslicei = global_scase.slicecoll.vsliceinfo+mvslicei->ivslices[i];
        IF_NOT_USEMESH_CONTINUE(vslicei->loaded,global_scase.slicecoll.sliceinfo[vslicei->ival].blocknumber);
        if(vslicei->skip==0&&vslicei->loaded==0){
          vslicei->finalize = 1;
          break;
        }
      }
      for(i = 0; i<mvslicei->nvslices; i++){
        vslicedata *vslicei;

        vslicei = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[i];
        IF_NOT_USEMESH_CONTINUE(vslicei->loaded,global_scase.slicecoll.sliceinfo[vslicei->ival].blocknumber);
        if(vslicei->skip==0&&vslicei->loaded==0){
#ifdef pp_SLICEFRAME
          LoadVSliceMenu2(mvslicei->ivslices[i]);
#else
          load_size+=LoadVSliceMenu2(mvslicei->ivslices[i]);
          file_count++;
#endif
        }
        if(vslicei->skip==1&&vslicei->loaded==1)UnloadVSliceMenu(mvslicei->ivslices[i]);
      }
      STOP_TIMER(load_time);
#ifndef pp_SLICEFRAME
      PrintFileLoadTimes(file_count,load_size,load_time);
#endif
    }
    script_multivslice=0;
  }
  else if(value<=-1000){ // load multi v slice
    int submenutype, dir;
    char *submenulabel;
    slicedata *slicei;

    value = -(1000 + value);
    submenutype=value/4-1;
    dir=value%4;
    submenutype=global_scase.msubvslice_menuindex[submenutype];

    slicei = global_scase.slicecoll.sliceinfo + submenutype;
    submenulabel = slicei->label.longlabel;
    START_TIMER(load_time);
    for(i = 0; i<global_scase.slicecoll.nmultivsliceinfo; i++){
      char *longlabel;
      multivslicedata *mvslicei;
      slicedata *slicej;
      vslicedata *vslicej;

      mvslicei = global_scase.slicecoll.multivsliceinfo + i;
      vslicej = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[0];
      slicej = global_scase.slicecoll.sliceinfo+vslicej->ival;
      longlabel = slicej->label.longlabel;
      if(strcmp(longlabel,submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicej->idir)continue;
      LoadMultiVSliceMenu(i);
#ifndef pp_SLICEFRAME
      file_count++;
#endif
    }
    STOP_TIMER(load_time);
#ifndef pp_SLICEFRAME
    PrintFileLoadTimes(file_count,load_size,load_time);
#endif
  }
  else{
    switch(value){
      case UNLOAD_ALL:
        LoadVSliceMenu(UNLOAD_ALL);
        break;

      case MENU_KEEP_ALL:
      if(vectorslicedup_option!=SLICEDUP_KEEPALL){
        vectorslicedup_option = SLICEDUP_KEEPALL;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateVSliceDups();
        GLUIUpdateSliceDupDialog();
      }
      break;

      case  MENU_KEEP_COARSE:
      if(vectorslicedup_option!=SLICEDUP_KEEPCOARSE){
        vectorslicedup_option = SLICEDUP_KEEPCOARSE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateVSliceDups();
        GLUIUpdateSliceDupDialog();
      }
      break;

      case MENU_KEEP_FINE:
      if(vectorslicedup_option!=SLICEDUP_KEEPFINE){
        vectorslicedup_option = SLICEDUP_KEEPFINE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateVSliceDups();
        GLUIUpdateSliceDupDialog();
      }
      break;
      case MENU_LOADVSLICE_SETTINGS:
        GLUIShowBoundsDialog(DLG_SLICE);
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
}

/* ------------------ LoadAllMSlicesMT ------------------------ */

FILE_SIZE LoadAllMSlicesMT(int last_slice, multislicedata *mslicei, int *fcount){
  FILE_SIZE file_size = 0;
  int file_count = 0;
  int i;

  file_count = 0;
  file_size = 0;
  for(i = 0; i < mslicei->nslices; i++){
    slicedata *slicei;
    int set_slicecolor;

    slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[i];
    set_slicecolor = DEFER_SLICECOLOR;
    IF_NOT_USEMESH_CONTINUE(slicei->loaded,slicei->blocknumber);

    slicei->finalize = 0;
    if(last_slice == mslicei->islices[i]){
      slicei->finalize = 1;
      set_slicecolor = SET_SLICECOLOR;
    }
    if(slicei->skipdup == 0 && last_slice != mslicei->islices[i]){
      file_size += LoadSlicei(set_slicecolor, mslicei->islices[i], ALL_FRAMES, NULL);
      file_count++;
    }
  }
  file_size += LoadSlicei(SET_SLICECOLOR, last_slice, ALL_FRAMES, NULL);
  file_count++;
  *fcount = file_count;
  return file_size;
}

/* ------------------ LoadAllMSlices ------------------------ */

FILE_SIZE LoadAllMSlices(int last_slice, multislicedata *mslicei){
  float load_time;
  FILE_SIZE file_size = 0;
  int file_count=0;

  START_TIMER(load_time);

  SetLoadedSliceBounds(mslicei->islices, mslicei->nslices);
  file_size = LoadAllMSlicesMT(last_slice, mslicei, &file_count);
  STOP_TIMER(load_time);
#ifndef pp_SLICEFRAME
  PrintFileLoadTimes(file_count,(float)file_size,load_time);
#endif
  return file_size;
}

/* ------------------ LoadMultiSliceMenu2 ------------------------ */

void LoadMultiSliceMenu(int value){
  int i;

  if(value==MENU_DUMMY)return;
  if(value>=0){
    multislicedata *mslicei;

    mslicei = global_scase.slicecoll.multisliceinfo + value;
    if(scriptoutstream!=NULL){
      if(mslicei->nslices>0){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[0];
        fprintf(scriptoutstream,"LOADSLICE\n");
        fprintf(scriptoutstream," %s\n",slicei->label.longlabel);
        fprintf(scriptoutstream," %i %f\n",slicei->idir,slicei->position_orig);
        script_multislice=1;
      }
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      FILE_SIZE total_size=0;
      int last_slice;

      char *longlabel;

      longlabel = global_scase.slicecoll.sliceinfo[mslicei->islices[0]].label.longlabel;
      UnloadAllSliceFiles(longlabel); // unload all slice and vector slices not of type 'longlabel'
      if(load_only_when_unloaded == 0){ // unload slice being loaded if it is already loaded and of the same type
        for(i = 0;i<mslicei->nslices; i++){
          UnloadSliceMenu(mslicei->islices[i]);
        }
      }
      last_slice = -1;
      for(i = mslicei->nslices-1; i >=0; i--){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[i];
        IF_NOT_USEMESH_CONTINUE(slicei->loaded,slicei->blocknumber);
        if(slicei->slice_filetype==SLICE_TERRAIN&&slicei->have_agl_data==0)continue;
        if(slicei->skipdup== 0){
          last_slice = mslicei->islices[i];
          break;
        }
      }
      for(i = 0; i < mslicei->nslices; i++){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[i];
        if(slicei->skipdup== 1 && slicei->loaded == 1){
          UnloadSliceMenu(mslicei->islices[i]);
        }
      }
      if(last_slice >= 0){
        total_size = LoadAllMSlices(last_slice, mslicei);
        if(compute_slice_file_sizes == 1){
          PRINTF(" file size: ");
          if(total_size > 1000000000){
            PRINTF("%.1f GB\n", (float)total_size / 1000000000.);
          }
          else if(total_size > 1000000){
            PRINTF("%.1f MB\n", (float)total_size / 1000000.);
          }
          else{
            PRINTF("%.0f kB\n", (float)total_size / 1000.);
          }
          PRINTF(" load time: %f s\n", (float)total_size * 8.0 / 1000000000.0);
          PRINTF("   (assuming a gigabit network connection)\n");
        }
      }
    }
    script_multislice=0;
  }
  else if(value<=-1000){
    int submenutype, dir, errorcode;
    char *submenulabel;
    slicedata *slicei;
    float load_time;
#ifndef pp_SLICEFRAME
    float load_size = 0.0;
    int file_count = 0;
#endif

    value = -(1000 + value);
    submenutype=value/4;
    dir=value%4;
    submenutype=global_scase.msubslice_menuindex[submenutype];
    slicei = global_scase.slicecoll.sliceinfo + submenutype;
    submenulabel = slicei->label.longlabel;
    START_TIMER(load_time);
    for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
      char *longlabel;

      slicei = global_scase.slicecoll.sliceinfo + i;
      if(slicei->skipdup== 1)continue;
      longlabel = slicei->label.longlabel;
      if(strcmp(longlabel,submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicei->idir)continue;
      if(dir!=0&&slicei->volslice==1)continue;
#ifdef pp_SLICEFRAME
      if(slicei->slice_filetype == SLICE_GEOM){
        ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, 0, &errorcode);
      }
      else{
        ReadSlice(slicei->file,i, ALL_FRAMES, NULL, LOAD,SET_SLICECOLOR,&errorcode);
      }
#else
      if(slicei->slice_filetype == SLICE_GEOM){
        load_size += ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, 0, &errorcode);
      }
      else{
        load_size += ReadSlice(slicei->file,i, ALL_FRAMES, NULL, LOAD,SET_SLICECOLOR,&errorcode);
      }
      file_count++;
#endif
    }
    UpdateTimes();
    STOP_TIMER(load_time);
#ifndef pp_SLICEFRAME
    PrintFileLoadTimes(file_count,load_size,load_time);
#endif
  }
  else{
    switch(value){
      case UNLOAD_ALL:
        LoadSliceMenu(UNLOAD_ALL);
        break;
      case MENU_KEEP_ALL:
        if(slicedup_option!=SLICEDUP_KEEPALL){
          slicedup_option = SLICEDUP_KEEPALL;
          updatemenu = 1;
          GLUTPOSTREDISPLAY;
          SetSliceParmInfo(&sliceparminfo);
          UpdateSliceDups(&sliceparminfo);
          GLUIUpdateSliceDupDialog();
        }
        break;
      case  MENU_KEEP_COARSE:
        if(slicedup_option!=SLICEDUP_KEEPCOARSE){
          slicedup_option = SLICEDUP_KEEPCOARSE;
          updatemenu = 1;
          GLUTPOSTREDISPLAY;
          SetSliceParmInfo(&sliceparminfo);
          UpdateSliceDups(&sliceparminfo);
          GLUIUpdateSliceDupDialog();
        }
        break;
      case MENU_KEEP_FINE:
        if(slicedup_option!=SLICEDUP_KEEPFINE){
          slicedup_option = SLICEDUP_KEEPFINE;
          updatemenu = 1;
          GLUTPOSTREDISPLAY;
          SetSliceParmInfo(&sliceparminfo);
          UpdateSliceDups(&sliceparminfo);
          GLUIUpdateSliceDupDialog();
        }
        break;
      case MENU_SPLITSLICES:
        sortslices = 1 - sortslices;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        GLUIUpdateSortSlices();
        break;
      case MENU_SPLITSLICES_DEBUG:
        sortslices_debug = 1 - sortslices_debug;
        if(sortslices_debug == 1)sortslices = 1;
        GLUIUpdateSortSlices();
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        break;
      case MENU_SLICECOLORDEFER:
        use_set_slicecolor = 1 - use_set_slicecolor;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        break;
      case MENU_SLICE_FILE_SIZES:
        compute_slice_file_sizes = 1-compute_slice_file_sizes;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        break;
      case MENU_SLICE_SETTINGS:
        GLUIShowBoundsDialog(DLG_SLICE);
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
}

/* ------------------ LoadMultiSliceMenu ------------------------ */

void LoadAllMultiSliceMenu(void){
  int i;
  char *label;

  label = slicebounds_cpp[sliceload_boundtype].label;
  for(i = 0; i < global_scase.slicecoll.nmultisliceinfo; i++){
    multislicedata *mslicei;
    slicedata *slicei;

    mslicei = global_scase.slicecoll.multisliceinfo + i;
    slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[0];
    if(slicei->volslice == 1)continue;
    if(sliceload_dir == 0 && slicei->idir != 1)continue;
    if(sliceload_dir == 1 && slicei->idir != 2)continue;
    if(sliceload_dir == 2 && slicei->idir != 3)continue;
    if(sliceload_filetype == 0 && slicei->slice_filetype!=SLICE_NODE_CENTER)continue;
    if(sliceload_filetype == 1 && slicei->slice_filetype!=SLICE_CELL_CENTER)continue;
    if(strcmp(label, slicei->label.shortlabel) != 0)continue;
    LoadMultiSliceMenu(i);
  }
}

/* ------------------ LoadAllMultiVSliceMenu ------------------------ */

void LoadAllMultiVSliceMenu(void){
  int i;
  char *label;

  label = slicebounds_cpp[sliceload_boundtype].label;
  for(i = 0; i < global_scase.slicecoll.nmultivsliceinfo; i++){
    multivslicedata *mvslicei;
    slicedata *slicei;
    vslicedata *vslicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo + i;
    vslicei = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[0];
    slicei = global_scase.slicecoll.sliceinfo + vslicei->ival;
    if(slicei->volslice == 1)continue;
    if(sliceload_dir == 0 && slicei->idir != 1)continue;
    if(sliceload_dir == 1 && slicei->idir != 2)continue;
    if(sliceload_dir == 2 && slicei->idir != 3)continue;
    if(sliceload_filetype == 0 && slicei->slice_filetype!=SLICE_NODE_CENTER)continue;
    if(sliceload_filetype == 1 && slicei->slice_filetype!=SLICE_CELL_CENTER)continue;
    if(strcmp(label, slicei->label.shortlabel) != 0)continue;
    LoadMultiVSliceMenu(i);
  }
}

/* ------------------ Plot3DListMenu ------------------------ */

void Plot3DListMenu(int value){
  int i;
  float delta_time=0.5;

  value = CLAMP(value, 0, nplot3dtimelist-1);
  iplot3dtimelist = value;
  if(scriptoutstream!=NULL){
    fprintf(scriptoutstream,"LOADPLOT3D\n");
    fprintf(scriptoutstream," %f\n",plot3dtimelist[value]);
  }
  if(nplot3dtimelist>1)delta_time = (plot3dtimelist[1]-plot3dtimelist[0])/2.0;

  for(i = 0; i < global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    plot3di->loadnow = 0;
    if(
      load_only_when_unloaded == 0 &&
      plot3di->loaded == 1){
      int errorcode;

      ReadPlot3D("", i, UNLOAD, &errorcode);
    }
  }
  for(i = 0; i < global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    IF_NOT_USEMESH_CONTINUE(plot3di->loaded,plot3di->blocknumber);
    if(ABS(plot3di->time-plot3dtimelist[value])<delta_time){
      if(plot3di->loaded==0){
        plot3di->loadnow = 1;
        plot3di->finalize = 0;
      }
    }
    else{
      if(plot3di->loaded == 1){
        int errorcode;

        ReadPlot3D("", i, UNLOAD, &errorcode);
      }
    }
  }
  SetLoadedPlot3DBounds();
  for(i = global_scase.nplot3dinfo-1; i>=0; i--){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loadnow==0)continue;
    plot3di->finalize = 1;
    break;
  }
  FILE_SIZE total_plot3d_filesize = 0;
  int file_count=0;
  float plot3d_timer;
  START_TIMER(plot3d_timer);
  for(i=0;i<global_scase.nplot3dinfo;i++){
    int errorcode;
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(plot3di->loadnow==0)continue;
    total_plot3d_filesize += ReadPlot3D(plot3di->file, i, LOAD, &errorcode);
    file_count++;
  }
  STOP_TIMER(plot3d_timer);
  if(file_count>0){
    char label[256];

    Plot3DSummary(label, file_count, total_plot3d_filesize, plot3d_timer);
    printf("%s\n",label);
  }
  printf("\n");
}

/* ------------------ UpdateMenu ------------------------ */

void UpdateMenu(void){
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadAllPlot3D ------------------------ */

int LoadAllPlot3D(float time){
  int i;
  int errorcode;
  int count=0;

  for(i = 0; i < global_scase.nplot3dinfo; i++){
    if(global_scase.plot3dinfo[i].loaded == 1){
      ReadPlot3D("", i, UNLOAD, &errorcode);
    }
  }
  for(i = 0; i < global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    plot3di->finalize = 0;
  }
  for(i = global_scase.nplot3dinfo - 1; i >=0; i--){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(ABS(plot3di->time - time) < 0.5){
      plot3di->finalize = 1;
      break;
    }
  }
  FILE_SIZE total_plot3d_filesize = 0;
  int file_count=0;
  float plot3d_timer;
  START_TIMER(plot3d_timer);
  for(i = 0; i < global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(ABS(plot3di->time - time) > 0.5)continue;
    total_plot3d_filesize += ReadPlot3D(plot3di->file, plot3di - global_scase.plot3dinfo, LOAD, &errorcode);
    file_count++;
    if(errorcode==0)count++;
  }
  STOP_TIMER(plot3d_timer);
  if(file_count>0){
    char label[256];

    Plot3DSummary(label, file_count, total_plot3d_filesize, plot3d_timer);
    printf("%s\n",label);
  }
  return count;
}

/* ------------------ LoadPlot3DMenu ------------------------ */

void LoadPlot3dMenu(int value){
  int errorcode;
  int i;

  if(value==MENU_PLOT3D_DUMMY)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    if(scriptoutstream!=NULL&&loadplot3dall==0){
      fprintf(scriptoutstream,"LOADPLOT3D\n");
      fprintf(scriptoutstream," %i %f\n",
        global_scase.plot3dinfo[value].blocknumber+1,global_scase.plot3dinfo[value].time);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      for(i = 0;i < global_scase.nplot3dinfo;i++){
        plot3ddata *plot3di;

        plot3di = global_scase.plot3dinfo + i;
        plot3di->loadnow = 0;
      }
      global_scase.plot3dinfo[value].loadnow = 1;
      SetLoadedPlot3DBounds();
      global_scase.plot3dinfo[value].finalize = 1;
      for(i = 0; i < global_scase.nplot3dinfo; i++){
        plot3ddata *plot3di;

        plot3di = global_scase.plot3dinfo + i;
        if(plot3di->loaded == 1){
          if(
            load_only_when_unloaded == 0 ||
            i != value){
            ReadPlot3D("", i, UNLOAD, &errorcode);
          }
        }
      }
      FILE_SIZE total_plot3d_filesize = 0;
      int file_count=0;
      float plot3d_timer;
      START_TIMER(plot3d_timer);
      for(i = 0;i < 1;i++){
        plot3ddata *plot3di;

        plot3di = global_scase.plot3dinfo + value;
        IF_NOT_USEMESH_CONTINUE(plot3di->loaded, plot3di->blocknumber);
        total_plot3d_filesize += ReadPlot3D(plot3di->file, value, LOAD, &errorcode);
        file_count++;
      }
      STOP_TIMER(plot3d_timer);
      if(file_count>0){
        char label[256];

        Plot3DSummary(label, file_count, total_plot3d_filesize, plot3d_timer);
        printf("%s\n",label);
      }
    }
  }
  else if(value==RELOAD_ALL){
    for(i = 0; i<global_scase.nplot3dinfo; i++){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo+i;
      plot3di->loadnow = 0;
      plot3di->finalize = 0;
      if(plot3di->loaded==1){
        plot3di->loadnow = 1;
      }
    }
    for(i = global_scase.nplot3dinfo - 1; i >= 0; i--){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo + i;
      if(plot3di->loadnow == 1){
        plot3di->finalize = 1;
        break;
      }
    }
    SetLoadedPlot3DBounds();
    for(i = 0; i<global_scase.nplot3dinfo; i++){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo + i;
      if(plot3di->loaded==1){
        ReadPlot3D("", plot3di-global_scase.plot3dinfo, UNLOAD, &errorcode);
      }
    }
    FILE_SIZE total_plot3d_filesize = 0;
    int file_count=0;
    float plot3d_timer;
    START_TIMER(plot3d_timer);
    for(i = 0; i<global_scase.nplot3dinfo; i++){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo + i;
      if(plot3di->loadnow == 1){
        total_plot3d_filesize += ReadPlot3D(plot3di->file, plot3di-global_scase.plot3dinfo, LOAD, &errorcode);
        file_count++;
      }
    }
    STOP_TIMER(plot3d_timer);
    if(file_count>0){
      char label[256];

      Plot3DSummary(label, file_count, total_plot3d_filesize, plot3d_timer);
      printf("%s\n",label);
    }
  }
  else if(value==UNLOAD_ALL){
    for(i=0;i<global_scase.nplot3dinfo;i++){
      ReadPlot3D("",i,UNLOAD,&errorcode);
    }
  }
  else if(value==MENU_PLOT3D_SETTINGS){
    GLUIShowBoundsDialog(DLG_PLOT3D);
  }
  else{
    value+=100000;
    loadplot3dall=1;
    Plot3DListMenu(value);
    loadplot3dall=0;
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadIsoi ------------------------ */

FILE_SIZE LoadIsoI(int value){
  char *file;
  isodata *isoi;
  int errorcode;
  FILE_SIZE return_filesize = 0;
  float total_time;

  START_TIMER(total_time);
  THREADcontrol(isosurface_threads, THREAD_JOIN);
  ReadIsoFile=1;
  isoi = global_scase.isoinfo + value;
  file=isoi->file;
  isoi->loading=1;
  if(script_iso==0&&scriptoutstream!=NULL){
    fprintf(scriptoutstream,"LOADISOM\n");
    fprintf(scriptoutstream, " %s\n", isoi->surface_label.longlabel);
    fprintf(scriptoutstream, " %i\n", isoi->blocknumber+1);
  }
  CancelUpdateTriangles();
  if(scriptoutstream==NULL||script_defer_loading==0){
    return_filesize=ReadIso(file,value,LOAD,NULL,&errorcode);
    if(update_readiso_geom_wrapup == UPDATE_ISO_ONE_NOW)ReadIsoGeomWrapup(BACKGROUND);
  }
  isoi->loading=0;
  STOP_TIMER(total_time);
  PRINTF(" - %.1f MB/%.1f s\n",(float)return_filesize/1000000.,total_time);
  return return_filesize;
}

  /* ------------------ LoadAllIsos ------------------------ */

void LoadAllIsos(int iso_type){
  int i;
#ifndef pp_ISOFRAME
  int file_count=0;
  float load_size=0.0;
#endif
  float load_time=0.0;

  if(load_only_when_unloaded == 0){
    for(i = 0; i < global_scase.nisoinfo; i++){
      isodata *isoi;

      isoi = global_scase.isoinfo + i;
      if(iso_type == isoi->type&&isoi->blocknumber >= 0){
        meshdata *meshi;

        meshi = global_scase.meshescoll.meshinfo + isoi->blocknumber;
        UnloadIso(meshi);
      }
    }
  }
  START_TIMER(load_time);
  CancelUpdateTriangles();
  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    isoi->finalize = 0;
  }
  for(i = global_scase.nisoinfo-1;i>=0;i--){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    IF_NOT_USEMESH_CONTINUE(isoi->loaded, isoi->blocknumber);
    if(iso_type==isoi->type){
      isoi->finalize = 1;
      break;
    }
  }
  for(i = 0; i < global_scase.nisoinfo; i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    IF_NOT_USEMESH_CONTINUE(isoi->loaded,isoi->blocknumber);
    if(iso_type==isoi->type){
#ifdef pp_ISOFRAME
      LoadIsoI(i);
#else
      load_size+=LoadIsoI(i);
      file_count++;
#endif
    }
  }
  STOP_TIMER(load_time);
#ifndef pp_ISOFRAME
  PrintFileLoadTimes(file_count,load_size,load_time);
#endif
}

/* ------------------ LoadIsoMenu ------------------------ */

void LoadIsoMenu(int value){
  int errorcode;
  int i;
  int ii;

  THREADcontrol(isosurface_threads, THREAD_JOIN);
  if(value==MENU_DUMMY3)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    if(load_only_when_unloaded == 0){
      for(i = 0;i < global_scase.nisoinfo;i++){
        isodata *isoi;

        isoi = global_scase.isoinfo + i;
        isoi->finalize = 0;
        if(isoi->loaded == 1)ReadIso("", i, UNLOAD, NULL, &errorcode);
      }
    }
    for(i=0;i<1;i++){
      isodata *isoi;

      isoi = global_scase.isoinfo + value;
      isoi->finalize = 1;
      IF_NOT_USEMESH_CONTINUE(isoi->loaded,isoi->blocknumber);
      LoadIsoI(value);
    }
  }
  if(value==-1){
    for(i=0;i<global_scase.nisoinfo;i++){
      isodata *isoi;

      isoi = global_scase.isoinfo + i;
      if(isoi->loaded==1)ReadIso("",i,UNLOAD,NULL,&errorcode);
    }
  }
  if(value==MENU_ISO_SETTINGS){
    GLUIShowBoundsDialog(DLG_ISO);
    return;
  }
  if(value<=-10){
    isodata *isoi;

    ii = -(value + 10);
    isoi = global_scase.isoinfo + ii;
    if(scriptoutstream!=NULL){
      script_iso=1;
      fprintf(scriptoutstream,"LOADISO\n");
      fprintf(scriptoutstream," %s\n",isoi->surface_label.longlabel);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
      LoadAllIsos(isoi->type);
      if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)ReadIsoGeomWrapup(BACKGROUND);
      update_readiso_geom_wrapup = UPDATE_ISO_OFF;
    }
    script_iso=0;
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadBoundaryMenu ------------------------ */

int InPatchList(patchdata *patchj, patchdata *patchi){
  if(strcmp(patchj->label.longlabel, patchi->label.longlabel)!=0)return 0;
  if(patchj->patch_filetype!=patchi->patch_filetype)return 0;
  return 1;
}

/* ------------------ LoadBoundaryMenu ------------------------ */

void LoadBoundaryMenu(int value){
  int errorcode;
  int i;

  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    if(load_only_when_unloaded == 0){
      for(i = 0;i < global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 1){
          ReadBoundary(i, UNLOAD, &errorcode);
        }
      }
    }
    if(scriptoutstream!=NULL){
      patchdata *patchi;

      patchi = global_scase.patchinfo + value;
      fprintf(scriptoutstream,"// LOADFILE\n");
      fprintf(scriptoutstream,"//  %s\n",patchi->file);
      fprintf(scriptoutstream, "LOADBOUNDARYM\n");
      fprintf(scriptoutstream, " %s\n", patchi->label.longlabel);
      fprintf(scriptoutstream, " %i\n", patchi->blocknumber+1);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      for(i = 0;i < 1;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + value;
        IF_NOT_USEMESH_CONTINUE(patchi->loaded, patchi->blocknumber);
        THREADcontrol(compress_threads, THREAD_LOCK);
        SetLoadedPatchBounds(&value, 1);
        if(patchi->structured == YES){
          PRINTF("Loading %s(%s)", patchi->file, patchi->label.shortlabel);
        }
        ReadBoundary(value, LOAD, &errorcode);
        THREADcontrol(compress_threads, THREAD_UNLOCK);
      }
    }
  }
  else if(value<=-10){
    patchdata *patchj;

    value = -(value + 10);
    patchj = global_scase.patchinfo + value;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOADBOUNDARY\n");
      fprintf(scriptoutstream," %s\n",patchj->label.longlabel);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
#ifndef pp_BOUNDFRAME
      int file_count=0;
      float load_size=0.0;
#endif
      float load_time=0.0;

      START_TIMER(load_time);

      // only perform wrapup operations when loading last boundary file
      for(i = 0; i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        patchi->finalize = 0;
        if(patchi->loaded == 1
          && load_only_when_unloaded == 0
         ){
          ReadBoundary(i, UNLOAD, &errorcode);
        }
      }
      int *list=NULL, nlist=0;

      NewMemory((void **)&list,global_scase.npatchinfo*sizeof(int));
      nlist=0;
      for(i = 0; i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        if(InPatchList(patchj, patchi)==1){
          list[nlist++]=i;
        }
      }
      SetLoadedPatchBounds(list, nlist);
      FREEMEMORY(list);
      for(i = global_scase.npatchinfo-1; i>=0; i--){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        IF_NOT_USEMESH_CONTINUE(patchi->loaded,patchi->blocknumber);
        if(FileExistsOrig(patchi->reg_file) == NO)continue;
        if(InPatchList(patchj, patchi)==1){
          THREADcontrol(compress_threads, THREAD_LOCK);
          patchi->finalize = 1;
          THREADcontrol(compress_threads, THREAD_UNLOCK);
          break;
        }
      }
      for(i=0;i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        IF_NOT_USEMESH_CONTINUE(patchi->loaded,patchi->blocknumber);
        if(InPatchList(patchj, patchi)==1){
          THREADcontrol(compress_threads, THREAD_LOCK);
          if(patchi->structured == YES){
            PRINTF("Loading %s(%s)", patchi->file, patchi->label.shortlabel);
          }
#ifdef pp_BOUNDFRAME
          ReadBoundary(i, LOAD, &errorcode);
#else
          load_size+=ReadBoundary(i, LOAD, &errorcode);
#endif
          if(patchi->structured!=NO&&patchi->finalize==1){
            UpdateTriangles(GEOM_STATIC, GEOM_UPDATE_ALL);
          }
#ifndef pp_BOUNDFRAME
          file_count++;
#endif
          THREADcontrol(compress_threads, THREAD_UNLOCK);
        }
      }
      STOP_TIMER(load_time);
#ifndef pp_BOUNDFRAME
      PrintFileLoadTimes(file_count,load_size,load_time);
#endif
    }
    force_redisplay=1;
    UpdateFrameNumber(0);
  }
  else{
    switch(value){
    case MENU_BNDF_MIRROR:
      show_mirror_boundary = 1 - show_mirror_boundary;
      updatemenu = 1;
      break;
    case MENU_BNDF_OPEN:
      show_open_boundary = 1 - show_open_boundary;
      updatemenu = 1;
      break;
    case MENU_BOUNDARY_SETTINGS:
      GLUIShowBoundsDialog(DLG_BOUNDARY);
      break;
      case MENU_KEEP_ALL:
      if(boundaryslicedup_option!=SLICEDUP_KEEPALL){
        boundaryslicedup_option = SLICEDUP_KEEPALL;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateBoundarySliceDups();
        GLUIUpdateSliceDupDialog();
      }
      break;

      case  MENU_KEEP_COARSE:
      if(boundaryslicedup_option!=SLICEDUP_KEEPCOARSE){
        boundaryslicedup_option = SLICEDUP_KEEPCOARSE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateBoundarySliceDups();
        GLUIUpdateSliceDupDialog();
      }
      break;

      case MENU_KEEP_FINE:
      if(boundaryslicedup_option!=SLICEDUP_KEEPFINE){
        boundaryslicedup_option = SLICEDUP_KEEPFINE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateBoundarySliceDups();
        GLUIUpdateSliceDupDialog();
      }
      break;
    default:
      for(i=0;i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label, "INCLUDE_GEOM")!=0){
          ReadBoundary(i, UNLOAD, &errorcode);
        }
      }
      break;
    }
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ GetInternalFaceShow ------------------------ */

int GetInternalFaceShow(void){
  int show = 1;

  if(boundary_loaded == 1){
    cpp_boundsdata *bounds;
    bounds = GLUIGetBoundsData(BOUND_PATCH);
    if(bounds->set_chopmax == 1 || bounds->set_chopmin == 1){
      update_bound_chop_data = 1;
      show = 1;
    }
    else{
      update_bound_chop_data = 0;
      if(show_all_interior_patch_data == 0){
        show = 0;
      }
      else{
        show = 1;
      }
    }
  }
  else{
    show = 1;
  }
  return show;
}

/* ------------------ ShowInternalBlockages ------------------------ */

void ShowInternalBlockages(void){
  int show;

  show = GetInternalFaceShow();
  hide_internal_blockages = 1 - show;
  if(show == 0){
    outline_state=OUTLINE_NONE;
    solid_state=visBLOCKHide;
  }
  else{
    if(update_showblock_ini == 1){
      update_showblock_ini = 0;
      visBlocks     = visBlocks_ini;
      solid_state   = solid_state_ini;
      outline_state = outline_state_ini;
    }
    else{
      visBlocks = visBLOCKNormal;
      solid_state = visBLOCKNormal;
      outline_state = OUTLINE_NONE;
    }
    GeometryMenu(17 + TERRAIN_HIDDEN);
  }
  updatemenu = 1;
  global_scase.updatefaces = 1;
  updatefacelists = 1;
}

/* ------------------ ShowBoundaryMenu ------------------------ */

void ShowBoundaryMenu(int value){
  updatemenu=1;
  updatefacelists=1;
  GLUTPOSTREDISPLAY;
  if(value>=1000){
    patchdata *patchj;
    int i;

    patchj = global_scase.patchinfo + value-1000;
    patchj->display = 1 - patchj->display;
    for(i=0;i<global_scase.npatchinfo;i++){
      patchdata *patchi;

      patchi = global_scase.patchinfo + i;
      if(patchi->loaded == 0)continue;
      if(strcmp(patchi->label.longlabel,patchj->label.longlabel)==0)patchi->display=patchj->display;
    }
    UpdateBoundaryType();
  }
  if(value==SHOW_CHAR){
    vis_threshold = 1 - vis_threshold;
    GLUIUpdateChar();
  }
  if(value==GLUI_SHOWALL_BOUNDARY||value==GLUI_HIDEALL_BOUNDARY){
    if(value == GLUI_SHOWALL_BOUNDARY){
      show_boundaryfiles = 1;
      ShowBoundaryMenu(SHOW_INTERIOR_WALL_MENU);
      ShowBoundaryMenu(SHOW_EXTERIOR_WALL_MENU);
    }
    if(value == GLUI_HIDEALL_BOUNDARY){
      show_boundaryfiles = 0;
      ShowBoundaryMenu(HIDE_INTERIOR_WALL_MENU);
      ShowBoundaryMenu(HIDE_EXTERIOR_WALL_MENU);
    }
    updatefacelists = 1;
    global_scase.updatefaces = 1;
    ShowInternalBlockages();
    update_patch_vis = 1;
  }
  if(value<0){
    if(value==SHOW_EXTERIOR_WALL_MENU||value==HIDE_EXTERIOR_WALL_MENU){
      int i,val;

      if(value==SHOW_EXTERIOR_WALL_MENU){
        val = 1;
      }
      else{
        val = 0;
      }
      for(i = 0;i < global_scase.npatchinfo;i++){
        int n;

        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        for(n = 0;n < patchi->npatches;n++){
          patchfacedata *pfi;

          pfi = patchi->patchfaceinfo + n;
          if(pfi->type != INTERIORwall){
            pfi->vis = val;
          }
        }
      }
      for(i=1;i<7;i++){
        vis_boundary_type[i]=val;
      }
      update_patch_vis = 1;
    }
    else if(value==INTERIOR_WALL_MENU){
      int i;

      hide_all_interior_patch_data    = show_all_interior_patch_data;
      show_all_interior_patch_data    = 1 - show_all_interior_patch_data;
      vis_boundary_type[INTERIORwall] = show_all_interior_patch_data;
      for(i = 0;i < global_scase.npatchinfo;i++){
        patchdata *patchi;
        int n;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        for(n = 0;n < patchi->npatches;n++){
          patchfacedata *pfi;

          pfi = patchi->patchfaceinfo + n;
          if(pfi->type == INTERIORwall){
            pfi->vis = show_all_interior_patch_data;
          }
        }
      }
      ShowInternalBlockages();
      UpdateShowIntPatch(hide_all_interior_patch_data, show_all_interior_patch_data);
    }
    else if(value == SHOW_INTERIOR_WALL_MENU || value == HIDE_INTERIOR_WALL_MENU){
      if(value == SHOW_INTERIOR_WALL_MENU)show_all_interior_patch_data = 1;
      if(value == HIDE_INTERIOR_WALL_MENU)show_all_interior_patch_data = 0;
      ShowBoundaryMenu(INTERIOR_WALL_MENU);
    }
    if(value==INI_EXTERIORwallmenu){
      int i;

      for(i = 0;i < global_scase.npatchinfo;i++){
        int n;

        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        for(n = 0;n < patchi->npatches;n++){
          patchfacedata *pfi;

          pfi = patchi->patchfaceinfo + n;
          if(pfi->type != INTERIORwall){
            pfi->vis = vis_boundary_type[pfi->type];
          }
        }
      }
    }
    else if(value != DUMMYwallmenu){
      int i;

      value = -(value + 2); /* map xxxwallmenu to xxxwall */
      for(i = 0;i < global_scase.npatchinfo;i++){
        patchdata *patchi;
        int n;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        for(n = 0;n < patchi->npatches;n++){
          patchfacedata *pfi;

          pfi = patchi->patchfaceinfo + n;
          if(pfi->type == value){
            pfi->vis = 1 - pfi->vis;
            vis_boundary_type[value] = pfi->vis;
          }
        }
      }
    }
    update_patch_vis = 1;
  }
  plotstate=GetPlotState(DYNAMIC_PLOTS);
}

/* ------------------ VentMenu ------------------------ */

void VentMenu(int value){
  if(value==-1)return;
  switch(value){
  case SHOW_ALL_VENTS: // show all vents
    visVents=1;
    visOpenVents=1;
    visDummyVents=1;
    global_scase.visOtherVents=1;
    visCircularVents=VENT_CIRCLE;
    break;
  case MENU_VENT_OPEN:
    visOpenVents=1-visOpenVents;
    break;
  case MENU_VENT_OUTLINE:
    visOpenVentsAsOutline = 1 - visOpenVentsAsOutline;
    break;
  case MENU_VENT_EXTERIOR:
    visDummyVents = 1 - visDummyVents;
    break;
  case MENU_VENT_TWOINTERIOR:
     show_bothsides_int=1-show_bothsides_int;
     global_scase.updatefaces=1;
     break;
  case MENU_VENT_TWOEXTERIOR:
     show_bothsides_ext = 1 - show_bothsides_ext;
     global_scase.updatefaces=1;
     break;
  case MENU_VENT_TRANSPARENT:
     show_transparent_vents=1-show_transparent_vents;
     global_scase.updatefaces=1;
     break;
  case MENU_VENT_OTHER:
     global_scase.visOtherVents=1-global_scase.visOtherVents;
     break;
   case HIDE_ALL_VENTS: // Hide all vents
     visVents=0;
     visOpenVents=0;
     visDummyVents=0;
     global_scase.visOtherVents=0;
     visCircularVents=VENT_HIDE;
     break;
   case MENU_VENT_CIRCLE:
     visCircularVents=VENT_CIRCLE;
     break;
   case MENU_VENT_RECTANGLE:
     visCircularVents=VENT_RECTANGLE;
     break;
   case MENU_VENT_CIRCLEHIDE:
     visCircularVents=VENT_HIDE;
     break;
   case MENU_VENT_CIRCLEOUTLINE:
     circle_outline=1-circle_outline;
     break;
  default:
    assert(FFALSE);
    break;
  }
  updatefacelists=1;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}
#define GEOMETRY_SOLID                   0
#define GEOMETRY_OUTLINE                 1
#define GEOMETRY_SOLIDOUTLINE            2
#define GEOMETRY_INTERIOR_SOLID         23
#define GEOMETRY_INTERIOR_OUTLINE       24
#define GEOMETRY_HIDE                    7
#define GEOMETRY_SHOWNORMAL              3
#define GEOMETRY_SORTFACES               6
#define GEOMETRY_SMOOTHNORMAL            4
#define GEOMETRY_HILIGHTSKINNY           5
#define GEOMETRY_HIDEALL                 8
#define GEOMETRY_INSIDE_DOMAIN          25
#define GEOMETRY_OUTSIDE_DOMAIN         15
#define GEOMETRY_VOLUMES_INTERIOR       18
#define GEOMETRY_VOLUMES_EXTERIOR       19
#define GEOMETRY_DUMMY                -999
#define GEOMETRY_TERRAIN_SHOW_TOP       22

/* ------------------ ImmersedMenu ------------------------ */

void ImmersedMenu(int value){
  if(value==GEOMETRY_DUMMY)return;
  updatemenu=1;
  switch(value){
    case GEOM_TriangleCount:
      show_triangle_count=1-show_triangle_count;
      break;
    case GEOMETRY_TERRAIN_SHOW_TOP:
      terrain_showonly_top = 1 - terrain_showonly_top;
      GLUIUpdateShowOnlyTop();
      break;
    case GEOMETRY_SOLIDOUTLINE:
      if(show_faces_shaded==1&&show_faces_outline==1){
        show_faces_shaded=1;
        show_faces_outline=0;
      }
      else{
        show_faces_shaded=1;
        show_faces_outline=1;
      }
      break;
    case GEOMETRY_SOLID:
      if(show_faces_shaded==1&&show_faces_outline==1){
        show_faces_shaded=1;
        show_faces_outline=0;
      }
      else if(show_faces_shaded==1&&show_faces_outline==0){
        show_faces_shaded=0;
        show_faces_outline=1;
      }
      else if(show_faces_shaded==0&&show_faces_outline==1){
        show_faces_shaded=1;
        show_faces_outline=0;
      }
      else{
        show_faces_shaded=1;
        show_faces_outline=0;
      }
      break;
    case GEOMETRY_OUTLINE:
      if(show_faces_shaded==1&&show_faces_outline==1){
        show_faces_shaded=0;
        show_faces_outline=1;
      }
      else if(show_faces_shaded==1&&show_faces_outline==0){
        show_faces_shaded=0;
        show_faces_outline=1;
      }
      else if(show_faces_shaded==0&&show_faces_outline==1){
        show_faces_shaded=1;
        show_faces_outline=0;
      }
      else{
        show_faces_shaded=0;
        show_faces_outline=1;
      }
      break;
    case GEOMETRY_SHOWNORMAL:
      show_geom_normal=1-show_geom_normal;
      break;
    case GEOMETRY_SMOOTHNORMAL:
      smooth_geom_normal=1-smooth_geom_normal;
      break;
    case GEOMETRY_HILIGHTSKINNY:
      hilight_skinny = 1 - hilight_skinny;
      break;
    case GEOMETRY_SORTFACES:
      sort_geometry=1-sort_geometry;
      break;
    case GEOMETRY_HIDE:
      show_faces_shaded=0;
      show_faces_outline=0;
      break;
    case GEOMETRY_HIDEALL:
      ImmersedMenu(GEOMETRY_HIDE);
      show_geom_normal = 0;
      break;
    case MENU_DUMMY:
      break;
    case GEOMETRY_INSIDE_DOMAIN:
      showgeom_inside_domain = 1 - showgeom_inside_domain;
      GLUIUpdateWhereFaceVolumes();
      break;
    case GEOMETRY_OUTSIDE_DOMAIN:
      showgeom_outside_domain = 1 - showgeom_outside_domain;
      GLUIUpdateWhereFaceVolumes();
      break;
    default:
      assert(FFALSE);
      break;
  }
  GLUIUpdateGeometryControls();

  GLUTPOSTREDISPLAY;
}

/* ------------------ BlockageMenu ------------------------ */

void BlockageMenu(int value){
  int change_state=0;

  if(value==ANIMATE_BLOCKAGES){
    animate_blockages = 1 - animate_blockages;
    if(animate_blockages==1){
      global_scase.tourcoll.tourinfo->display = 0;
      show_avatar = 0;
    }
    if(animate_blockages==0){
      global_scase.tourcoll.tourinfo->display = 1;
    }
    TourMenu(0);
    updatemenu = 1;
    return;
  }
  if(value==visLightFaces){
    light_faces = 1 - light_faces;
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    return;
  }
  if(solid_state<0)solid_state=visBlocks;
  if(outline_state<0)outline_state=OUTLINE_NONE;
  switch(value){
    case GEOM_BOUNDING_BOX_MOUSE_DOWN:
      if(hide_scene==1){
        hide_scene = 0;
      }
      else{
        hide_scene = 1;
      }
      GLUIUpdateGeomBoundingBox();
      break;
    case visBLOCKOutlineColor:
      outline_color_flag = 1 - outline_color_flag;
      global_scase.updatefaces=1;
      break;
    case visBLOCKOnlyOutline:
      if(outline_state!=OUTLINE_ONLY){
        outline_state=OUTLINE_ONLY;
      }
      else{
        outline_state=OUTLINE_NONE;
      }
      change_state=1;
      GeometryMenu(17 + TERRAIN_HIDDEN);
      break;
    case visCADOpaque:
      viscadopaque = 1 - viscadopaque;
      break;
    case visBLOCKAddOutline:
      if(outline_state!=OUTLINE_ADDED){
        outline_state=OUTLINE_ADDED;
        if(solid_state==visBLOCKHide)solid_state=visBLOCKAsInput;
      }
      else{
        outline_state=OUTLINE_NONE;
      }
      GeometryMenu(17 + TERRAIN_HIDDEN);
      change_state=1;
      break;
    case visBLOCKAsInput:
      solid_state=visBLOCKAsInput;
      if(outline_state==OUTLINE_ONLY)outline_state=OUTLINE_ADDED;
      GeometryMenu(17 + TERRAIN_HIDDEN);
      change_state=1;
      break;
    case visBLOCKNormal:
      solid_state=visBLOCKNormal;
      if(outline_state==OUTLINE_ONLY)outline_state=OUTLINE_ADDED;
      GeometryMenu(17 + TERRAIN_HIDDEN);
      change_state=1;
      break;
    case visBLOCKHide:
      outline_state=OUTLINE_NONE;
      solid_state=visBLOCKHide;
      change_state=1;
      break;
    case BLOCKlocation_grid:
    case BLOCKlocation_exact:
    case BLOCKlocation_cad:
      break;
    default:
      assert(FFALSE);
      break;
  }
  if(change_state==1){
    switch(outline_state){
      case OUTLINE_NONE:
        value=solid_state;
        break;
      case OUTLINE_ONLY:
        value=visBLOCKOutline;
        break;
      case OUTLINE_ADDED:
        switch(solid_state){
          case visBLOCKAsInput:
            value=visBLOCKAsInputOutline;
          break;
          case visBLOCKNormal:
            value=visBLOCKSolidOutline;
            break;
          case BLOCKAGE_HIDDEN:
            value=visBLOCKOutline;
            break;
          default:
            assert(FFALSE);
            break;
        }
        break;
      default:
        assert(FFALSE);
        break;
    }
  }

  switch(value){
   case visCADOpaque:
   case visBLOCKOutlineColor:
     break;
   case visBLOCKAsInputOutline:
   case visBLOCKAsInput:
     visBlocks=value;
     GLUIUpdateTrainerOutline();
     break;
   case visBLOCKNormal:
   case visBLOCKOutline:
   case visBLOCKHide:
   case visBLOCKSolidOutline:
     visBlocks=value;
     if(value==visBLOCKSolidOutline||visBLOCKold==visBLOCKSolidOutline)global_scase.updatefaces=1;
     GLUIUpdateTrainerOutline();
     break;
   case BLOCKlocation_grid:
   case BLOCKlocation_exact:
   case BLOCKlocation_cad:
     if(NCADGeom(&global_scase.cadgeomcoll) == 0){
       if(value == BLOCKlocation_grid){
         blocklocation_menu = BLOCKlocation_exact;
       }
       else{
         blocklocation_menu = BLOCKlocation_grid;
       }
     }
     else{
       blocklocation_menu = value;
     }
     Keyboard('q', FROM_SMOKEVIEW);
     break;
   case BLOCKtexture_cad:
     visCadTextures=1-visCadTextures;
     break;
   case visBLOCKTransparent:
     visTransparentBlockage=1-visTransparentBlockage;
     break;
   default:
     if(value<0){
       value=-value-1;
       if(value>=0&&value<=global_scase.propcoll.npropinfo-1){
         propdata *propi;

         propi = global_scase.propcoll.propinfo + value;
         propi->blockvis=1-propi->blockvis;
       }
     }
     else{
       assert(FFALSE);
     }
     break;
  }
  visBLOCKold=value;
  updatemenu=1;
  updatefacelists=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ TranslateTypeMenu ------------------------ */

void TranslateTypeMenu(int value){
  assert(value>=0&&value<=2);
  translation_type = CLAMP(value,0,2);
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  switch(translation_type){
  case TRANSLATE_XY_option:
    printf("translate left/right and front/back\n");
    break;
  case TRANSLATE_X_option:
    printf("translate only left/right\n");
    break;
  case TRANSLATE_Y_option:
    printf("translate only front/back\n");
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ RotateTypeMenu ------------------------ */

void RotateTypeMenu(int value){
  if(value==MENU_DUMMY)return;
  if(value==MENU_MOTION_SETTINGS){
    GLUIShowMotion(DIALOG_MOTION);
    return;
  }
  else if(value == MENU_MOTION_SHOW_VECTORS){
	showgravity_vector = 1-showgravity_vector;
	GLUIUpdateShowGravityVector();
  }
  else if(value == MENU_MOTION_GRAVITY_VECTOR){
    gvec_down = 1;
#define USE_GVEC 28
    GLUISceneMotionCB(USE_GVEC);
  }
  else if(value==MENU_MOTION_Z_VECTOR){
#define ZAXIS_UP 41
    GLUISceneMotionCB(ZAXIS_UP);
  }
  else{
    rotation_type = value;
    GLUIUpdateRotationType(rotation_type);
    GLUIRotationTypeCB(rotation_type);
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ TitleMenu ------------------------ */

void TitleMenu(int value){
  if(value == MENU_DUMMY)return;
  GLUTPOSTREDISPLAY;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  switch(value){
  case MENU_TITLE_title_smv_version:
    vis_title_smv_version =1-vis_title_smv_version;
    break;
  case MENU_TITLE_chid:
    vis_title_CHID=1-vis_title_CHID;
    break;
  case MENU_TITLE_title_fds:
    vis_title_fds=1-vis_title_fds;
    break;
  case MENU_TITLE_gversion:
    vis_title_gversion =1-vis_title_gversion;
    break;
  case MENU_TITLE_show_all:
    vis_title_smv_version = 1;
    vis_title_CHID        = 1;
    vis_title_fds         = 1;
    vis_title_gversion    = 1;
    break;
  case MENU_TITLE_hide_all:
    vis_title_smv_version = 0;
    vis_title_CHID        = 0;
    vis_title_fds         = 0;
    vis_title_gversion    = 0;
    break;
  default:
    assert(FFALSE);
    break;
  }
  GLUISetLabelControls();
}

/* ------------------ ShowADeviceType ------------------------ */

void ShowADeviceType(void){
  int i;

  for(i=0;i<global_scase.objectscoll.nobject_defs;i++){
    sv_object *obj_typei;

    obj_typei = global_scase.objectscoll.object_defs[i];
    if(obj_typei->used_by_device==1&&obj_typei->visible==1)return;
  }
  for(i=0;i<global_scase.objectscoll.nobject_defs;i++){
    sv_object *obj_typei;

    obj_typei = global_scase.objectscoll.object_defs[i];
    if(obj_typei->used_by_device==1){
      obj_typei->visible=1;
      return;
    }
  }
}

/* ------------------ DeviceTypeMenu ------------------------ */

void DeviceTypeMenu(int val){
  GLUIUpdateDeviceTypes(val);
  GLUIDeviceCB(DEVICE_devicetypes);
}

/* ------------------ ShowDevicesMenu ------------------------ */

void ShowDevicesMenu(int value){
  if(value==MENU_DUMMY||global_scase.devicecoll.ndeviceinfo<=0)return;
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  if(value==MENU_DEVICES_SHOWALL||value==MENU_DEVICES_HIDEALL){
    int i;

    for(i=0; i<global_scase.devicecoll.ndeviceinfo; i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo + i;
      if(value==MENU_DEVICES_SHOWALL){
        devicei->show = 1;
      }
      else{
        devicei->show = 0;
      }
    }
    return;
  }
  {
    int ival, itype;
    devicedata *devicei;

    if(value >= 3 * global_scase.devicecoll.ndeviceinfo)return;
    ival = value % global_scase.devicecoll.ndeviceinfo;
    itype = value / global_scase.devicecoll.ndeviceinfo;

    devicei = global_scase.devicecoll.deviceinfo + ival;
    //             0 <= value <   ndeviceinfo : toggle
    //   ndeviceinfo <= value < 2*ndeviceinfo : show
    // 2*ndeviceinfo <= value < 3*ndeviceinfo : hide
    if(itype == 0)devicei->show = 1 - devicei->show;
    if(itype == 1)devicei->show = 1;
    if(itype == 2)devicei->show = 0;
  }
}

/* ------------------ ShowObjectsMenu ------------------------ */

void ShowObjectsMenu(int value){
  sv_object *objecti;
  int i;

  if(value>=0&&value<global_scase.objectscoll.nobject_defs){
    objecti = global_scase.objectscoll.object_defs[value];
    objecti->visible = 1 - objecti->visible;
    if(showdevice_val==1||vis_device_plot!=DEVICE_PLOT_HIDDEN){
      update_times = 1;
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      GLUIUpdateDeviceShow();
    }
  }
  else if(value == OBJECT_MISSING){
    updatemenu = 1;
    show_missing_objects = 1 - show_missing_objects;
  }
  else if(value==OBJECT_SHOWALL){
    for(i=0;i<global_scase.objectscoll.nobject_defs;i++){
      objecti = global_scase.objectscoll.object_defs[i];
      objecti->visible=1;
    }
  }
  else if(value==OBJECT_HIDEALL){
    for(i=0;i<global_scase.objectscoll.nobject_defs;i++){
      objecti = global_scase.objectscoll.object_defs[i];
      objecti->visible=0;
    }
  }
  else if(value==OBJECT_SELECT){
    select_device=1-select_device;
  }
  else if(value==OBJECT_PLOT_SHOW_TREE_ALL){
    update_times=1;
    if(vis_device_plot==DEVICE_PLOT_SHOW_TREE_ALL){
      vis_device_plot = DEVICE_PLOT_HIDDEN;
    }
    else{
      vis_device_plot = DEVICE_PLOT_SHOW_TREE_ALL;
      select_device = 1;
      ShowADeviceType();
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    GLUIUpdateDeviceShow();
  }
  else if(value==PLOT_HRRPUV){
    vis_hrr_plot = 1-vis_hrr_plot;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateShow();
    update_times = 1;
  }
  else if(value==OBJECT_PLOT_SHOW_ALL){
    update_times=1;
    if(vis_device_plot==DEVICE_PLOT_SHOW_ALL){
      vis_device_plot = DEVICE_PLOT_HIDDEN;
    }
    else{
      vis_device_plot = DEVICE_PLOT_SHOW_ALL;
      select_device = 1;
      ShowADeviceType();
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    GLUIUpdateDeviceShow();
  }
  else if(value==OBJECT_PLOT_SHOW_SELECTED){
    update_times = 1;
    if(vis_device_plot==DEVICE_PLOT_SHOW_SELECTED){
      vis_device_plot = DEVICE_PLOT_HIDDEN;
    }
    else{
      vis_device_plot = DEVICE_PLOT_SHOW_SELECTED;
      select_device = 1;
      ShowADeviceType();
    }
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    GLUIUpdateDeviceShow();
  }
  else if(value==MENU_DEVICE_SETTINGS){
    GLUIShowPlotDevice();
  }
  else if(value==OBJECT_VALUES){
    update_times=1;
    showdevice_val = 1 - showdevice_val;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    GLUIUpdateDeviceShow();
  }
  else if(value==OBJECT_OUTLINE){
    object_outlines=1-object_outlines;
  }
  else if(value==OBJECT_BOX){
    object_box = 1-object_box;
  }
  else if(value==OBJECT_ORIENTATION){
    show_device_orientation=1-show_device_orientation;
    GLUIUpdateDeviceOrientation();
  }
  else if(value == OBJECT_SHOWBEAM){
    showbeam_as_line = 1 - showbeam_as_line;
    GLUIUpdateShowbeamAsLine();
  }
  else if(value==MENU_DUMMY){
  }
  else{
    device_sphere_segments=ABS(value);
    InitSphere(device_sphere_segments,2*device_sphere_segments);
    InitCircle(2*device_sphere_segments,&object_circ);
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}


/* ------------------ TerrainGeomShowMenu ------------------------ */

void TerrainGeomShowMenu(int value){
  if(value==MENU_DUMMY)return;
  if(value>=0){
    texturedata *texti;

    if(value>=0&&value<global_scase.terrain_texture_coll.nterrain_textures){
      texti = global_scase.terrain_texture_coll.terrain_textures+value;
      texti->display = 1-texti->display;
      GLUIUpdateTerrainTexture(value);
    }
  }
  else{
    switch(value){
    case MENU_TERRAIN_SHOW_SURFACE:
      terrain_show_geometry_surface = 1 - terrain_show_geometry_surface;
      break;
    case MENU_TERRAIN_SHOW_LINES:
      terrain_show_geometry_outline = 1 - terrain_show_geometry_outline;
      break;
    case MENU_TERRAIN_SHOW_POINTS:
      terrain_show_geometry_points = 1-terrain_show_geometry_points;
      break;
    case MENU_TERRAIN_BOUNDING_BOX_AUTO:
      if(hide_scene==1){
        hide_scene = 0;
      }
      else{
        hide_scene = 1;
      }
      GLUIUpdateGeomBoundingBox();
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

  /* ------------------ ZoneShowMenu ------------------------ */

void ZoneShowMenu(int value){
  switch(value){
  case MENU_DUMMY:
    return;
  case MENU_ZONE_XPLANE:
    if(zonecolortype == ZONESMOKE_COLOR)zonecolortype = ZONETEMP_COLOR;
    if(visZonePlane==ZONE_XPLANE){
      visZonePlane = ZONE_HIDDEN;
      visZone=0;
    }
    else{
      visZonePlane = ZONE_XPLANE;
      visZone=1;
    }
    break;
  case MENU_ZONE_YPLANE:
    if(zonecolortype==ZONESMOKE_COLOR)zonecolortype = ZONETEMP_COLOR;
    if(visZonePlane==ZONE_YPLANE){
      visZonePlane = ZONE_HIDDEN;
      visZone=0;
    }
    else{
      visZonePlane = ZONE_YPLANE;
      visZone=1;
    }
    break;
  case MENU_ZONE_ZPLANE:
    if(zonecolortype==ZONESMOKE_COLOR)zonecolortype = ZONETEMP_COLOR;
    if(visZonePlane==ZONE_ZPLANE){
      visZonePlane = ZONE_HIDDEN;
      visZone=0;
    }
    else{
      visZonePlane = ZONE_ZPLANE;
      visZone=1;
    }
    break;
  case MENU_ZONE_LAYERHIDE:
    visZone=ZONE_HIDDEN;
    visSZone=0;
    break;
  case MENU_ZONE_2DHAZARD:
    zonecolortype=ZONEHAZARD_COLOR;
    visSZone=0;
    if(visZonePlane == ZONE_HIDDEN)visZonePlane = ZONE_YPLANE;
    visZone=1;
    break;
  case MENU_ZONE_2DTEMP:
  case MENU_ZONE_2DTEMP2:
    show_zonelower = 1;
    if(value==MENU_ZONE_2DTEMP)show_zonelower = 0;
    zonecolortype=ZONETEMP_COLOR;
    visSZone=0;
    if(visZonePlane == ZONE_HIDDEN)visZonePlane = ZONE_YPLANE;
    visZone=1;
    break;
  case MENU_ZONE_3DSMOKE:
    zonecolortype=ZONESMOKE_COLOR;
    visSZone=1;
    visZone=1;
    break;
  case MENU_ZONE_WALLS:
    vis_wall_data = 1-vis_wall_data;
    break;
  case MENU_ZONE_TARGETS:
    vis_target_data = 1-vis_target_data;
    break;
  case MENU_ZONE_VENTS:
    visVentFlow=1-visVentFlow;
    if(visVentFlow==1){
      visVentHFlow=1;
      visVentVFlow=1;
      visVentMFlow=1;
    }
    else{
      visVentHFlow=0;
      visVentVFlow=0;
      visVentMFlow=0;
    }
    break;
#define VISVENTFLOW     if((global_scase.nzhvents>0&&visVentHFlow==1)||(global_scase.nzvvents>0&&visVentVFlow==1)||(global_scase.nzmvents>0&&visVentMFlow==1)){\
      visVentFlow = 1;\
    }\
    else{\
      visVentFlow = 0;\
    }
  case MENU_ZONE_HVENTS:
    visVentHFlow = 1-visVentHFlow;
    VISVENTFLOW;
    break;
  case MENU_ZONE_VVENTS:
    visVentVFlow = 1-visVentVFlow;
    VISVENTFLOW;
    break;
  case MENU_ZONE_MVENTS:
    visVentMFlow = 1-visVentMFlow;
    VISVENTFLOW;
    break;
  case MENU_ZONE_VENT_SLAB:
    visventslab = 1 - visventslab;
    if(visventslab==1)visventprofile=0;
    if(visventprofile==1||visventslab==1){
      visVentHFlow=1;
    }
    else{
      visVentHFlow=0;
    }
    VISVENTFLOW;
    break;
  case MENU_ZONE_VENT_PROFILE:
    visventprofile = 1 - visventprofile;
    if(visventprofile==1)visventslab=0;
    if(visventprofile==1||visventslab==1){
      visVentHFlow=1;
    }
    else{
      visVentHFlow=0;
    }
    VISVENTFLOW;
    break;
  case MENU_ZONE_FIRES:
    viszonefire=1-viszonefire;
    break;
  default:
    assert(FFALSE);
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ GetHVACConnectState ------------------------ */

int GetHVACConnectState(int index){
  int i;

  for(i = 0;i < global_scase.hvaccoll.nhvacconnectinfo;i++){
    hvacconnectdata *hi;

    hi = global_scase.hvaccoll.hvacconnectinfo + i;
    if(hi->index == index)return hi->display;
  }
  return 1;
}

/* ------------------ HVACConnectMenu ------------------------ */

void HVACConnectMenu(int var){
  int i;

  if(var==MENU_HVAC_CONNECTION_VIEW){
    hvac_show_networks = hvac_show_connections;
    hvac_show_connections = 1 - hvac_show_connections;
    updatemenu = 1;
    GLUTPOSTREDISPLAY;
    return;
  }
  hvac_show_networks    = 0;
  hvac_show_connections = 1;
  if(var >= 0){
    global_scase.hvaccoll.hvacconnectinfo[var].display = 1 - global_scase.hvaccoll.hvacconnectinfo[var].display;
  }
  else if(var == MENU_HVAC_SHOWALL_CONNECTIONS){
    for(i = 0;i < global_scase.hvaccoll.nhvacconnectinfo;i++){
      global_scase.hvaccoll.hvacconnectinfo[i].display = 1;
    }
  }
  else if(var == MENU_HVAC_HIDEALL_CONNECTIONS){
    for(i = 0;i < global_scase.hvaccoll.nhvacconnectinfo;i++){
      global_scase.hvaccoll.hvacconnectinfo[i].display = 0;
    }
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ HVACNetworkMenu ------------------------ */

void HVACNetworkMenu(int value){
  int i;

  if(value==MENU_HVAC_SHOW_NODE_IGNORE)return;
  if(value==MENU_HVAC_NETWORK_VIEW){
    hvac_show_connections = hvac_show_networks;
    hvac_show_networks = 1 - hvac_show_networks;
    updatemenu = 1;
    GLUTPOSTREDISPLAY;
    return;
  }
  hvac_show_networks    = 1;
  hvac_show_connections = 0;
  if(value>=0&&value<global_scase.hvaccoll.nhvacinfo){
    hvacdata *hvaci;

    hvaci = global_scase.hvaccoll.hvacinfo + value;
    hvaci->display = 1 - hvaci->display;
  }
  else{
    switch(value){
    case MENU_HVAC_SHOWALL_NETWORKS:
      for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
        hvacdata *hvaci;

        hvaci = global_scase.hvaccoll.hvacinfo + i;
        hvaci->display = 1;
      }
      break;
    case MENU_HVAC_HIDEALL_NETWORKS:
      for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
        hvacdata *hvaci;

        hvaci = global_scase.hvaccoll.hvacinfo + i;
        hvaci->display = 0;
      }
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ SetHVACNodeValIndex ------------------------ */

void SetHVACNodeValIndex(int value){
  int i, return_val;

  return_val = -1;
  if(global_scase.hvaccoll.hvacnodevalsinfo == NULL){
    global_scase.hvaccoll.hvacnodevar_index = return_val;
    return;
  }
  for(i = 0;i < global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars;i++){
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + i;
    if(value == i){
      hi->vis = 1 - hi->vis;
      if(hi->vis == 1)return_val = i;
    }
    else{
      hi->vis = 0;
    }
  }
  global_scase.hvaccoll.hvacnodevar_index = return_val;
}

/* ------------------ SetAllHVACDucts ------------------------ */

void SetHVACDuct(void){
  int i;

  global_scase.hvaccoll.hvacductvalsinfo->duct_vars[0].vis = 1;
  for(i = 1;i < global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;i++){
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + i;
     hi->vis = 0;
  }
  global_scase.hvaccoll.hvacductvar_index = 0;
  if(IsHVACVisible(&global_scase.hvaccoll)==0){
    for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
      hvacdata *hvaci;

      hvaci = global_scase.hvaccoll.hvacinfo + i;
      hvaci->display = 1;
    }
  }
}

/* ------------------ SetHVACDuctValIndex ------------------------ */

void SetHVACDuctValIndex(int value){
  int i, return_val;

  return_val = -1;
  if(global_scase.hvaccoll.hvacductvalsinfo == NULL){
    global_scase.hvaccoll.hvacductvar_index = return_val;
    return;
  }
  for(i = 0;i < global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;i++){
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + i;
    if(value == i){
      hi->vis = 1 - hi->vis;
      if(hi->vis == 1)return_val = i;
    }
    else{
      hi->vis = 0;
    }
  }
  global_scase.hvaccoll.hvacductvar_index = return_val;
}

/* ------------------ HVACNodeValueMenu ------------------------ */

void HVACNodeValueMenu(int value){
  int i;

  if(global_scase.hvaccoll.hvacductvalsinfo->times==NULL){
    ReadHVACData(LOAD);
  }
  SetHVACNodeValIndex(value);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  if(global_scase.hvaccoll.hvacductvar_index >= 0 || global_scase.hvaccoll.hvacnodevar_index >= 0){
    if(IsHVACVisible(&global_scase.hvaccoll)==0){
      for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
        hvacdata *hvaci;

        hvaci = global_scase.hvaccoll.hvacinfo + i;
        hvaci->display = 1;
      }
    }
  }
  GLUISetValTypeIndex(BOUND_HVACNODE, value);
  updatemenu = 1;
  GLUIUpdateHVACVarLists();
  GLUIHVACNodeBoundsCPP_CB(BOUND_UPDATE_COLORS);
  GLUTPOSTREDISPLAY;
}

/* ------------------ HVACDuctValueMenu ------------------------ */

void HVACDuctValueMenu(int value){
  int i;

  if(global_scase.hvaccoll.hvacductvalsinfo->times==NULL){
    ReadHVACData(LOAD);
  }
  SetHVACDuctValIndex(value);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  if(global_scase.hvaccoll.hvacductvar_index >= 0 || global_scase.hvaccoll.hvacnodevar_index >= 0){
    if(IsHVACVisible(&global_scase.hvaccoll)==0){
      for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
        hvacdata *hvaci;

        hvaci = global_scase.hvaccoll.hvacinfo + i;
        hvaci->display = 1;
      }
    }
  }
  updatemenu = 1;
  GLUIUpdateHVACVarLists();
  GLUISetValTypeIndex(BOUND_HVACDUCT, value);
  GLUIHVACDuctBoundsCPP_CB(BOUND_UPDATE_COLORS);//
  GLUTPOSTREDISPLAY;
}

/* ------------------ LoadHVACMenu ------------------------ */

void LoadHVACMenu(int value){
  switch(value){
    case MENU_HVAC_LOAD:
      ReadHVACData(LOAD);
      SetHVACDuct();
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      UpdateTimes();
      GLUIHVACDuctBoundsCPP_CB(BOUND_UPDATE_COLORS);//
      GLUTPOSTREDISPLAY;
      break;
    case MENU_HVAC_UNLOAD:
      SetHVACNodeValIndex(-1);
      SetHVACDuctValIndex(-1);
      ReadHVACData(UNLOAD);
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      UpdateTimes();
      GLUTPOSTREDISPLAY;
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ HVACMenu ------------------------ */

void HVACMenu(int value){
  int i;

  if(value==MENU_HVAC_SHOW_NODE_IGNORE)return;
  if(value==MENU_HVAC_HIDE_ALL_VALUES){
    SetHVACNodeValIndex(-1);
    SetHVACDuctValIndex(-1);
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    updatemenu = 1;
    GLUTPOSTREDISPLAY;
    return;
  }
  switch(value){
  case MENU_HVAC_SHOW_DUCT_IDS:
      glui_hvac->show_duct_labels = 1 - glui_hvac->show_duct_labels;
      break;
    case MENU_HVAC_SHOW_NODE_IDS:
      glui_hvac->show_node_labels = 1 - glui_hvac->show_node_labels;
      break;
    case MENU_HVAC_SHOW_COMPONENT_TEXT:
      glui_hvac->show_component = 0;
      break;
    case MENU_HVAC_SHOW_COMPONENT_SYMBOLS:
      glui_hvac->show_component = 1;
      break;
    case MENU_HVAC_SHOW_COMPONENT_HIDE:
      glui_hvac->show_component = 2;
      break;
    case MENU_HVAC_SHOW_FILTER_TEXT:
      glui_hvac->show_filters = 0;
      break;
    case MENU_HVAC_SHOW_FILTER_SYMBOLS:
      glui_hvac->show_filters = 1;
      break;
    case MENU_HVAC_SHOW_FILTER_HIDE:
      glui_hvac->show_filters = 2;
      break;
    case MENU_HVAC_METRO_VIEW:
      hvac_metro_view = 1 - hvac_metro_view;
      GLUIUpdateHVACViews();
      break;
    case MENU_HVAC_CELL_VIEW:
      hvac_cell_view = 1 - hvac_cell_view;
      GLUIUpdateHVACViews();
      break;
    case MENU_HVAC_DIALOG_HVAC:
      DialogMenu(DIALOG_HVAC);
      break;
    default:
      assert(FFALSE);
      break;
  }
  for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
    char *labelsave;
    hvacdata *hvaci;
    int display;

    hvaci = global_scase.hvaccoll.hvacinfo + i;
    labelsave = hvaci->network_name;
    display   = hvaci->display;
    memcpy(hvaci, glui_hvac, sizeof(hvacdata));
    hvaci->network_name = labelsave;
    hvaci->display      = display;
  }
  GLUIHVAC2Glui(0);
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ ToggleMetroMode ------------------------ */

void ToggleMetroMode(void){
  HVACMenu(MENU_HVAC_METRO_VIEW);
}

  /* ------------------ HaveBoundaryArrival ------------------------ */

int HaveBoundaryArrival(void){
  int i;

  for(i = 0; i < global_scase.npatchinfo; i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->loaded == 1 && patchi->display == 1 && strcmp(patchi->label.shortlabel, "t_a") == 0){
      return 1;
    }
  }
  return 0;
}

/* ------------------ GeometryOutlineMenu ------------------------ */

void GeometryOutlineMenu(int value){
  if(global_scase.isZoneFireModel == 1)return;
  switch(value){
  case SKY_OUTLINE:
    visSkyboxoutline = 1 - visSkyboxoutline;
    GLUIUpdateVisSkyboxOutline();
    break;
  case GEOM_OutlineA:
    if(outline_mode != SCENE_OUTLINE_HIDDEN){
      outline_mode = SCENE_OUTLINE_HIDDEN;
    }
    else{
      outline_mode = SCENE_OUTLINE_SCENE;
    }
    break;
  case GEOM_OutlineB:
    if(outline_mode == SCENE_OUTLINE_MESH){
      outline_mode = SCENE_OUTLINE_HIDDEN;
    }
    else{
      outline_mode = SCENE_OUTLINE_MESH;
    }
    break;
  case GEOM_OutlineC:
    if(outline_mode == SCENE_OUTLINE_SCENE){
      outline_mode = SCENE_OUTLINE_HIDDEN;
    }
    else{
      outline_mode = SCENE_OUTLINE_SCENE;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
  if(outline_mode == SCENE_OUTLINE_HIDDEN)PRINTF("outline mode: hidden\n", outline_mode);
  if(outline_mode == SCENE_OUTLINE_MESH)PRINTF("outline mode: mesh\n", outline_mode);
  if(outline_mode == SCENE_OUTLINE_SCENE)PRINTF("outline mode: scene\n", outline_mode);
  updatefacelists = 1;
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

  /* ------------------ GeometryMenu ------------------------ */

void GeometryMenu(int value){

  switch(value){
  case GEOM_OutlineC:
    GeometryOutlineMenu(value);
    break;
  case 5:
    global_scase.visFloor=1-global_scase.visFloor;
    break;
  case 6:
    global_scase.visWalls=1-global_scase.visWalls;
    break;
  case 7:
    global_scase.visCeiling=1-global_scase.visCeiling;
    break;
  case 17+TERRAIN_TOP:
    terrain_showonly_top = 1 - terrain_showonly_top;
    GLUIUpdateTerrain();
    break;
  case 17+TERRAIN_SURFACE:
  case 17+TERRAIN_IMAGE:
  case 17+TERRAIN_HIDDEN:
    if(value == 17 + TERRAIN_HIDDEN && from_read_boundary == 1){
      from_read_boundary = 0;
      if(HaveBoundaryArrival() == 1)break;
    }
    global_scase.visTerrainType = value-17;
    GLUIUpdateTerrain();
    if(global_scase.visTerrainType == TERRAIN_HIDDEN){
      if(update_ini_vents == 0){
        if(global_scase.visOtherVents != global_scase.visOtherVentsSAVE){
          global_scase.visOtherVents = global_scase.visOtherVentsSAVE;
        }
      }
      else{
        update_ini_vents = 0;
      }
    }
    else{
      if(global_scase.visOtherVents!=0){
        global_scase.visOtherVentsSAVE=global_scase.visOtherVents;
        global_scase.visOtherVents=0;
      }
      BlockageMenu(visBLOCKHide);
    }
    if(global_scase.visTerrainType==TERRAIN_SURFACE){
      planar_terrain_slice=0;
    }
    else{
      planar_terrain_slice=1;
    }
    break;
  case GEOM_ShowAll:
    show_faces_shaded=1;
    global_scase.visFloor = 1;
    outline_mode = SCENE_OUTLINE_SCENE;
    BlockageMenu(visBLOCKAsInput);
    VentMenu(SHOW_ALL_VENTS);
    break;
  case GEOM_HideAll:
    outline_mode = SCENE_OUTLINE_HIDDEN;
    global_scase.visFloor=0;
    global_scase.visWalls=0;
    global_scase.visCeiling=0;
    VentMenu(HIDE_ALL_VENTS);
    visGrid = NOGRID_NOPROBE;
    BlockageMenu(visBLOCKHide);
    ImmersedMenu(GEOMETRY_HIDEALL);
    break;
  case GEOM_Vents:
    visVents=1-visVents;
    break;
  case GEOM_Compartments:
    visCompartments = 1 - visCompartments;
    break;
  case GEOM_BOUNDING_BOX_MOUSE_DOWN:
    if(hide_scene==1){
      hide_scene = 0;
    }
    else{
      hide_scene = 1;
    }
    GLUIUpdateGeomBoundingBox();
    break;
  default:
    assert(FFALSE);
    break;
  }
  updatefacelists=1;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}


/* ------------------ GeometryMainMenu ------------------------ */

void GeometryMainMenu(int value){
  if(value==BLOCKlocation_grid||value==BLOCKlocation_exact||value==BLOCKlocation_cad){
    BlockageMenu(value);
    return;
  }
  GeometryMenu(value);
}

/* ------------------ GetNumActiveDevices ------------------------ */

int GetNumActiveDevices(void){
  int num_activedevices = 0;

  if(global_scase.objectscoll.nobject_defs > 0){
    int i;

    for(i = 0; i < global_scase.objectscoll.nobject_defs; i++){
      sv_object *obj_typei;

      obj_typei = global_scase.objectscoll.object_defs[i];
      if(obj_typei->used_by_device == 1)num_activedevices++;
    }
  }
  return num_activedevices;
}

/* ------------------ GetNTotalVents ------------------------ */

int GetNTotalVents(void){
  int ntotal_vents = 0;
  int i;

  for(i = 0; i < global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    ntotal_vents += meshi->nvents;
  }
  return ntotal_vents;
}

/* ------------------ IsBoundaryType ------------------------ */

int IsBoundaryType(int type){
  int i;

  for(i = 0; i < global_scase.npatchinfo; i++){
    patchdata *patchi;
    int n;

    patchi = global_scase.patchinfo + i;
    for(n = 0; n < patchi->npatches; n++){
      patchfacedata *pfi;

      pfi = patchi->patchfaceinfo + n;
      if(pfi->type == type)return 1;
    }
  }
  return 0;
}

/* ------------------ IsoLoadState ------------------------ */

void IsoLoadState(isodata *isoi, int  *load_state){
  int i, total=0, loaded=0;

  for(i=0; i<global_scase.nisoinfo; i++){
    isodata *isoii;

    isoii = global_scase.isoinfo + i;
    if(strcmp(isoi->surface_label.longlabel, isoii->surface_label.longlabel)!=0)continue;
    total++;
    if(isoii->loaded==1)loaded++;
  }
  if(loaded==0){
    *load_state = 0;
  }
  else{
    if(loaded<total){
      *load_state = 1;
    }
    else{
      *load_state = 2;
    }
  }
}

/* ------------------ PatchLoadState ------------------------ */

void PatchLoadState(patchdata *patchi, int  *load_state){
  int i, total=0, loaded=0;

  for(i=0; i<global_scase.npatchinfo; i++){
    patchdata *patchii;

    patchii = global_scase.patchinfo + i;
    if(strcmp(patchi->label.longlabel, patchii->label.longlabel)!=0)continue;
    total++;
    if(patchii->loaded==1)loaded++;
  }
  if(loaded==0){
    *load_state = 0;
  }
  else{
    if(loaded<total){
      *load_state = 1;
    }
    else{
      *load_state = 2;
    }
  }
}

/* ------------------ Plot3DLoadState ------------------------ */

void Plot3DLoadState(float time, int  *load_state){
  int i, total=0, loaded=0;

  for(i=0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(ABS(plot3di->time-time)>0.1)continue;
    total++;
    if(plot3di->loaded==1)loaded++;
  }
  if(loaded==0){
    *load_state = 0;
  }
  else{
    if(loaded<total){
      *load_state = 1;
    }
    else{
      *load_state = 2;
    }
  }
}

/* ------------------ PartLoadState ------------------------ */

void PartLoadState(int  *load_state){
  int i, total=0, loaded=0;

  for(i=0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    total++;
    if(parti->loaded==1)loaded++;
  }
  if(loaded==0){
    *load_state = 0;
  }
  else{
    if(loaded<total){
      *load_state = 1;
    }
    else{
      *load_state = 2;
    }
  }
}

#define CREATEMENU(menu,Menu) menu=glutCreateMenu(Menu);\
  assert(nmenus<MAXMENUS);\
  if(nmenus<MAXMENUS){\
    menuinfo[nmenus].menuvar=menu;\
    menuinfo[nmenus++].status = 1;\
  }

#define GLUTADDSUBMENU(menu_label,menu_value) glutAddSubMenu(menu_label,menu_value)

/* ------------------ InitShowSliceMenu ------------------------ */

void InitShowSliceMenu(int *showhideslicemenuptr, int patchgeom_slice_showhide){
  if(nsliceloaded>0||patchgeom_slice_showhide==1){
    int showhideslicemenu;

    CREATEMENU(showhideslicemenu, ShowHideSliceMenu);
    *showhideslicemenuptr = showhideslicemenu;
    // loaded slice entries
    if(have_multislice==0){
      glutAddMenuEntry("-", MENU_DUMMY);
      if(HaveTerrainSlice()==1){
        if(planar_terrain_slice==1)glutAddMenuEntry(_("*Planar terrain slice"), MENU_SHOWSLICE_TERRAIN);
        if(planar_terrain_slice==0)glutAddMenuEntry(_("Planar terrain slice"), MENU_SHOWSLICE_TERRAIN);
      }

      if(nsliceloaded>0){
        glutAddMenuEntry(_("Show in:"), MENU_DUMMY);
        if(global_scase.show_slice_in_obst==ONLY_IN_GAS){
          glutAddMenuEntry(_("  *gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
          glutAddMenuEntry(_("  gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
        }
        if(global_scase.show_slice_in_obst==GAS_AND_SOLID){
          glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
          glutAddMenuEntry(_("  *gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
        }
        if(global_scase.show_slice_in_obst==ONLY_IN_SOLID){
          glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  *solid"), MENU_SHOWSLICE_IN_SOLID);
          glutAddMenuEntry(_("  gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
        }
        if(global_scase.show_slice_in_obst==NEITHER_GAS_NOR_SOLID){
          glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
          glutAddMenuEntry(_("  gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
        }
      }
      if(nsliceloaded>0){
        if(offset_slice==1)glutAddMenuEntry(_("*Offset slice"), MENU_SHOWSLICE_OFFSET);
        if(offset_slice==0)glutAddMenuEntry(_("Offset slice"), MENU_SHOWSLICE_OFFSET);
        if(show_node_slices_and_vectors==1)glutAddMenuEntry(_("*Show node centered slices and vectors"), MENU_SHOWSLICE_NODESLICEANDVECTORS);
        if(show_node_slices_and_vectors==0)glutAddMenuEntry(_("Show node centered slices and vectors"), MENU_SHOWSLICE_NODESLICEANDVECTORS);
        if(show_cell_slices_and_vectors==1)glutAddMenuEntry(_("*Show cell centered slices and vectors"), MENU_SHOWSLICE_CELLSLICEANDVECTORS);
        if(show_cell_slices_and_vectors==0)glutAddMenuEntry(_("Show cell centered slices and vectors"), MENU_SHOWSLICE_CELLSLICEANDVECTORS);
      }
      if(nsliceloaded>0&&sd_shown!=NULL){
        char menulabel[1024];

        glutAddMenuEntry("-", MENU_DUMMY);
        STRCPY(menulabel, "");
        if(showall_slices==1)STRCAT(menulabel, "*");
        STRCAT(menulabel, sd_shown->label.longlabel);
        glutAddMenuEntry(menulabel, SHOW_ALL);
        glutAddMenuEntry("", MENU_DUMMY);
      }
    }
  }
}

/* ------------------ InitShowMultiSliceMenu ------------------------ */

void InitShowMultiSliceMenu(int *showmultislicemenuptr, int showhideslicemenu, int patchgeom_slice_showhide){
  if(have_multislice==1||patchgeom_slice_showhide==1){
    int ii, i;
    patchdata *patchim1 = NULL;
    int showmultislicemenu;

    CREATEMENU(showmultislicemenu, ShowMultiSliceMenu);
    *showmultislicemenuptr = showmultislicemenu;
    for(i = 0; i<global_scase.slicecoll.nmultisliceinfo; i++){
      slicedata *sd;
      char menulabel[1024];
      multislicedata *mslicei;

      mslicei = global_scase.slicecoll.multisliceinfo+i;
      if(mslicei->loaded==0)continue;
      sd = global_scase.slicecoll.sliceinfo+mslicei->islices[0];
      STRCPY(menulabel, "");
      if(plotstate==DYNAMIC_PLOTS&&mslicei->display!=0&&sd->slicefile_labelindex==slicefile_labelindex){
        if(mslicei->display==1){
          STRCAT(menulabel, "*");
        }
        else if(mslicei->display==-1){
          STRCAT(menulabel, "#");
        }
      }
      STRCAT(menulabel, mslicei->menulabel2);
      if(sd->slicelabel!=NULL){
        STRCAT(menulabel, " - ");
        STRCAT(menulabel, sd->slicelabel);
      }
      glutAddMenuEntry(menulabel, i);
    }
    // loaded geometry slice entries
    for(ii = 0; ii<global_scase.npatchinfo; ii++){
      patchdata *patchi;

      i = patchorderindex[ii];
      patchi = global_scase.patchinfo+i;
      if(patchi->loaded==1&&patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
        if(patchim1==NULL||strcmp(patchi->label.longlabel, patchim1->label.longlabel)!=0){
          char mlabel[128];

          strcpy(mlabel, "");
          if(patchi->display==1)strcat(mlabel, "*");
          strcat(mlabel, patchi->label.longlabel);
          glutAddMenuEntry(mlabel, -20-i);
        }
        patchim1 = patchi;
      }
    }
    if(nsliceloaded>0){
      glutAddMenuEntry(_("  Show in:"), MENU_DUMMY);
      if(global_scase.show_slice_in_obst==ONLY_IN_GAS){
        glutAddMenuEntry(_("    *gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
      if(global_scase.show_slice_in_obst==GAS_AND_SOLID){
        glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("    *gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
      if(global_scase.show_slice_in_obst==ONLY_IN_SOLID){
        glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("    *solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
      if(global_scase.show_slice_in_obst==NEITHER_GAS_NOR_SOLID){
        glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("  gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
    }
    if(nsliceloaded>0){
      if(offset_slice==1)glutAddMenuEntry(_("*Offset slice"), MENU_SHOWSLICE_OFFSET);
      if(offset_slice==0)glutAddMenuEntry(_("Offset slice"), MENU_SHOWSLICE_OFFSET);
    }
  }
}

/* ------------------ InitUnloadSliceMenu ------------------------ */

void InitUnloadSliceMenu(int *unloadslicemenuptr){
  int i;
  int unloadslicemenu;

  CREATEMENU(unloadslicemenu,UnloadSliceMenu);
  *unloadslicemenuptr = unloadslicemenu;
  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *sd;
    char menulabel[1024];

    sd = global_scase.slicecoll.sliceinfo + global_scase.sliceorderindex[i];
    if(sd->loaded==1){
      STRCPY(menulabel,sd->menulabel2);
      glutAddMenuEntry(menulabel,global_scase.sliceorderindex[i]);
    }
  }
  for(i = 0;i<global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==1&&patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
      glutAddMenuEntry(patchi->label.longlabel, -3-i);
      geom_slice_loaded++;
    }
  }

  glutAddMenuEntry("-",MENU_DUMMY);
  glutAddMenuEntry(_("Unload all"), UNLOAD_ALL);
}

/* ------------------ InitSliceSkipMenu ------------------------ */

void InitSliceSkipMenu(int *sliceskipmenuptr){
  int i, sliceskipmenu;

  CREATEMENU(sliceskipmenu, SliceSkipMenu);
  *sliceskipmenuptr = sliceskipmenu;

  for(i = 1; i<=4; i++){
    char skiplabel[20];

    if(max_slice_skip>0&&i>max_slice_skip)break;
    if(slice_skip==i){
      sprintf(skiplabel, "*%i", i);
    }
    else{
      sprintf(skiplabel, "%i", i);
    }
    glutAddMenuEntry(skiplabel, i);
  }
  for(i =6; i<=8; i+=2){
    char skiplabel[20];

    if(max_slice_skip>0&&i>max_slice_skip)break;
    if(slice_skip==i){
      sprintf(skiplabel, "*%i", i);
    }
    else{
      sprintf(skiplabel, "%i", i);
    }
    glutAddMenuEntry(skiplabel, i);
  }
  for(i = 10; i<=20; i+=5){
    char skiplabel[20];

    if(max_slice_skip>0&&i>max_slice_skip)break;
    if(slice_skip==i){
      sprintf(skiplabel, "*%i", i);
    }
    else{
      sprintf(skiplabel, "%i", i);
    }
    glutAddMenuEntry(skiplabel, i);
  }
  for(i = 30; i<=max_slice_skip; i+=10){
    char skiplabel[20];

    if(max_slice_skip>0&&i>max_slice_skip)break;
    if(slice_skip==i){
      sprintf(skiplabel, "*%i", i);
    }
    else{
      sprintf(skiplabel, "%i", i);
    }
    glutAddMenuEntry(skiplabel, i);
  }
}

/* ------------------ InitUnloadMultiSliceMenu ------------------------ */

void InitUnloadMultiSliceMenu(int *unloadmultislicemenuptr){
  int i, unloadmultislicemenu;

  CREATEMENU(unloadmultislicemenu, UnloadMultiSliceMenu);
  *unloadmultislicemenuptr = unloadmultislicemenu;

  for(i = 0; i<global_scase.slicecoll.nmultisliceinfo; i++){
    multislicedata *mslicei;

    mslicei = global_scase.slicecoll.multisliceinfo+i;
    if(mslicei->loaded!=0){
      glutAddMenuEntry(mslicei->menulabel2, i);
    }
  }
  for(i = 0; i<global_scase.npatchinfo; i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==1&&patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
      glutAddMenuEntry(patchi->label.longlabel, -3-i);
    }
  }
  glutAddMenuEntry(_("Unload all"), UNLOAD_ALL);
}

/* ------------------ InitLoadMultiSubMenu ------------------------ */

void InitLoadMultiSubMenu(int **loadsubmslicemenuptr, int *nmultisliceloadedptr){
  int i, *loadsubmslicemenu, nloadsubmslicemenu;
  int nmultisliceloaded;

  nmultisliceloaded = 0;
  nloadsubmslicemenu = 1;
  for(i = 1;i<global_scase.slicecoll.nmultisliceinfo;i++){
    slicedata *sd, *sdim1;

    sd = global_scase.slicecoll.sliceinfo+(global_scase.slicecoll.multisliceinfo+i)->islices[0];
    sdim1 = global_scase.slicecoll.sliceinfo+(global_scase.slicecoll.multisliceinfo+i-1)->islices[0];
    if(strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0)nloadsubmslicemenu++;
  }
  loadsubmslicemenu = *loadsubmslicemenuptr;
  NEWMEM(loadsubmslicemenu, nloadsubmslicemenu*sizeof(int));
  *loadsubmslicemenuptr = loadsubmslicemenu;
  for(i = 0;i<nloadsubmslicemenu;i++){
    loadsubmslicemenu[i] = 0;
  }
  nloadsubmslicemenu = 0;
  for(i = 0;i<global_scase.slicecoll.nmultisliceinfo;i++){
    slicedata *sd, *sdim1;
    char menulabel[1024];
    multislicedata *mslicei,*msliceim1;

    if(i>0){
      msliceim1 = global_scase.slicecoll.multisliceinfo+i-1;
      sdim1 = global_scase.slicecoll.sliceinfo+msliceim1->islices[0];
    }
    mslicei = global_scase.slicecoll.multisliceinfo+i;
    sd = global_scase.slicecoll.sliceinfo+mslicei->islices[0];

    if(i==0||strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0){
      global_scase.msubslice_menuindex[nloadsubmslicemenu]=mslicei->islices[0];
      CREATEMENU(loadsubmslicemenu[nloadsubmslicemenu], LoadMultiSliceMenu);
      nloadsubmslicemenu++;
    }
    STRCPY(menulabel, "");
    if(mslicei->loadable == 0)strcat(menulabel, "***");
    if(mslicei->loaded==1){
      STRCAT(menulabel, "*");
      nmultisliceloaded++;
    }
    else if(mslicei->loaded==-1){
      STRCAT(menulabel, "#");
      nmultisliceloaded++;
    }
    STRCAT(menulabel, mslicei->menulabel);
    if(sd->slicelabel!=NULL){
      STRCAT(menulabel, " - ");
      STRCAT(menulabel, sd->slicelabel);
    }
    if(mslicei->slice_filetype==SLICE_GEOM){
      strcat(menulabel, " - geometry");
    }
    if(sd->compression_type == COMPRESSED_ZLIB){
      strcat(menulabel, "(ZLIB)");
    }
    if(mslicei->loadable == 0)strcat(menulabel, " - unavailable, no meshes with this data were selected");
    glutAddMenuEntry(menulabel, i);
  }
  *loadsubmslicemenuptr = loadsubmslicemenu;
  *nmultisliceloadedptr = nmultisliceloaded;
}

/* ------------------ InitDuplicateSliceMenu ------------------------ */

void InitDuplicateSliceMenu(int *duplicateslicemenuptr){
  int duplicateslicemenu;

      CREATEMENU(duplicateslicemenu, LoadMultiSliceMenu);
      *duplicateslicemenuptr = duplicateslicemenu;
      if(slicedup_option==SLICEDUP_KEEPALL){
        glutAddMenuEntry(_("  *keep all"), MENU_KEEP_ALL);
      }
      else{
        glutAddMenuEntry(_("  keep all"), MENU_KEEP_ALL);
      }
      if(slicedup_option==SLICEDUP_KEEPFINE){
        glutAddMenuEntry(_("  *keep fine"), MENU_KEEP_FINE);
      }
      else{
        glutAddMenuEntry(_("  keep fine"), MENU_KEEP_FINE);
      }
      if(slicedup_option==SLICEDUP_KEEPCOARSE){
        glutAddMenuEntry(_("  *keep coarse"), MENU_KEEP_COARSE);
      }
      else{
        glutAddMenuEntry(_("  keep coarse"), MENU_KEEP_COARSE);
      }
}

/* ----------------------- CompareSubSliceMenu ----------------------------- */

int CompareSubSliceMenu(const void *arg1, const void *arg2){
  subslicemenudata *subslicemenui, *subslicemenuj;
  char *labeli, *labelj;

  subslicemenui = (subslicemenudata  *)arg1;
  subslicemenuj = (subslicemenudata  *)arg2;
  labeli = subslicemenui->menulabel;
  labelj = subslicemenuj->menulabel;
  return strcmp(labeli, labelj);
}

/* ------------------ InitSubSliceMenuInfo ------------------------ */

void InitSubSliceMenuInfo(){
  int i;

  if(global_scase.slicecoll.nsliceinfo == 0 || subslicemenuinfo != NULL)return;
  INIT_PRINT_TIMER(subslicemenu_timer);
  NewMemory((void **)&subslicemenuinfo, global_scase.slicecoll.nsliceinfo * sizeof(subslicemenudata));
  nsubslicex = 0;
  nsubslicey = 0;
  nsubslicez = 0;
  nsubslicexyz = 0;
  for(i = 0;i<global_scase.slicecoll.nmultisliceinfo;i++){
    slicedata *sd, *sdim1;
    subslicemenudata *si;

    sd = global_scase.slicecoll.sliceinfo+(global_scase.slicecoll.multisliceinfo+i)->islices[0];
    if(i>0)sdim1 = global_scase.slicecoll.sliceinfo+(global_scase.slicecoll.multisliceinfo+i-1)->islices[0];

    if(i==0||strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0){
      si = subslicemenuinfo + nsubslicemenuinfo;
      si->menulabel  = sd->label.longlabel;
      si->shortlabel = sd->label.shortlabel;
      si->havex      = 0;
      si->havey      = 0;
      si->havez      = 0;
      si->havexyz    = 0;
      if(sd->slice_filetype == SLICE_NODE_CENTER || sd->slice_filetype == SLICE_CELL_CENTER){
        si->slicetype = sd->slice_filetype-1;
      }
      else{
        si->slicetype = SLICE_UNKNOWN;
      }
      nsubslicemenuinfo++;
    }
    si = subslicemenuinfo + nsubslicemenuinfo-1;
    if(si->slicetype!=SLICE_UNKNOWN&&sd->volslice==0){
      if(sd->idir == 1){
        si->havex = 1;
        nsubslicex++;
      }
      if(sd->idir == 2){
        si->havey = 1;
        nsubslicey++;
      }
      if(sd->idir == 3){
        si->havez = 1;
        nsubslicez++;
      }
      if(si->havex == 1 || si->havey == 1 || si->havez == 1){
        si->havexyz = 1;
        nsubslicexyz++;
      }
    }
  }
  ResizeMemory((void **)&subslicemenuinfo, nsubslicemenuinfo * sizeof(subslicemenudata));
  qsort((subslicemenudata *)subslicemenuinfo, (int)nsubslicemenuinfo, sizeof(subslicemenudata), CompareSubSliceMenu);
  PRINT_TIMER(subslicemenu_timer, "Initialize load all slice menus");
}

/* ------------------ InitSubVectorSliceMenuInfo ------------------------ */

void InitSubVectorSliceMenuInfo(){
  int i;

  if(global_scase.slicecoll.nmultivsliceinfo == 0 || subvectorslicemenuinfo != NULL)return;
  INIT_PRINT_TIMER(subvectorslicemenu_timer);
  NewMemory((void **)&subvectorslicemenuinfo, global_scase.slicecoll.nmultivsliceinfo * sizeof(subslicemenudata));
  nsubvectorslicex = 0;
  nsubvectorslicey = 0;
  nsubvectorslicez = 0;
  nsubvectorslicexyz = 0;
  for(i = 0; i<global_scase.slicecoll.nmultivsliceinfo; i++){
    vslicedata *vi, *vim1;
    slicedata *si, *sim1;
    subslicemenudata *vd;

    vi = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i)->ivslices[0];
    si = global_scase.slicecoll.sliceinfo+vi->ival;

    if(i>0){
      vim1 = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i-1)->ivslices[0];
      sim1 = global_scase.slicecoll.sliceinfo+vim1->ival;
    }
    if(i==0||(i>0&&strcmp(si->label.longlabel, sim1->label.longlabel)!=0)){
      vd = subvectorslicemenuinfo + nsubvectorslicemenuinfo;
      vd->menulabel  = si->label.longlabel;
      vd->shortlabel = si->label.shortlabel;
      vd->havex      = 0;
      vd->havey      = 0;
      vd->havez      = 0;
      vd->havexyz    = 0;
      if(si->slice_filetype == SLICE_NODE_CENTER || si->slice_filetype == SLICE_CELL_CENTER){
        vd->slicetype = si->slice_filetype-1;
      }
      else{
        vd->slicetype = SLICE_UNKNOWN;
      }
      nsubvectorslicemenuinfo++;
    }
    vd = subvectorslicemenuinfo + nsubvectorslicemenuinfo - 1;
    if(vd->slicetype!=SLICE_UNKNOWN&&si->volslice==0){
      if(si->idir == 1){
        vd->havex = 1;
        nsubvectorslicex++;
      }
      if(si->idir == 2){
        vd->havey = 1;
        nsubvectorslicey++;
      }
      if(si->idir == 3){
        vd->havez = 1;
        nsubvectorslicez++;
      }
      if(vd->havex == 1 || vd->havey == 1 || vd->havez == 1){
        vd->havexyz = 1;
        nsubvectorslicexyz++;
      }
    }
  }
  ResizeMemory((void **)&subvectorslicemenuinfo, nsubvectorslicemenuinfo * sizeof(subslicemenudata));
  qsort((subslicemenudata *)subvectorslicemenuinfo, (int)nsubvectorslicemenuinfo, sizeof(subslicemenudata), CompareSubSliceMenu);
  PRINT_TIMER(subvectorslicemenu_timer, "Initialize load all vector slice menus");
}

/* ------------------ GetSliceBoundType ------------------------ */

int GetSliceBoundType(char *label){
  int i;

  for(i=0;i<nslicebounds_cpp;i++){
    if(strcmp(label, slicebounds_cpp[i].label) == 0)return i;
  }
  return -1;
}

/* ------------------ LoadAllSlices ------------------------ */

void LoadAllSlices(int var){
  int bound_type;
  int quantity_index;

  sliceload_dir = var%4;
  quantity_index = var / 4;
  bound_type = GetSliceBoundType(subslicemenuinfo[quantity_index].shortlabel);
  if(bound_type<0)return;
  if(subslicemenuinfo[quantity_index].slicetype == SLICE_UNKNOWN)return;
  sliceload_filetype = subslicemenuinfo[quantity_index].slicetype;
  sliceload_boundtype = bound_type;
  sliceload_isvector = 0;
  GLUIUpdateLoadAllSlices();
  LoadAllMultiSliceMenu();
}

/* ------------------ LoadAllVectorSlices ------------------------ */

void LoadAllVectorSlices(int var){
  int bound_type;
  int quantity_index;

  sliceload_dir = var % 4;
  quantity_index = var / 4;
  bound_type = GetSliceBoundType(subvectorslicemenuinfo[quantity_index].shortlabel);
  if(bound_type < 0)return;
  if(subvectorslicemenuinfo[quantity_index].slicetype == SLICE_UNKNOWN)return;
  sliceload_filetype = subvectorslicemenuinfo[quantity_index].slicetype;
  sliceload_boundtype = bound_type;
  sliceload_isvector = 1;
  GLUIUpdateLoadAllSlices();
  LoadAllMultiVSliceMenu();
}

/* ------------------ InitLoadMultiSliceMenu ------------------------ */

void InitLoadMultiSliceMenu(int *loadmultislicemenuptr, int *loadsubmslicemenu, int *loadsubpatchmenu_s,
                            int *nsubpatchmenus_s, int sliceskipmenu, int sliceloadoptionmenu, int duplicateslicemenu,
                            int loadslicemenu, int nmultisliceloaded, int unloadmultislicemenu
                            , int *loadsubslicexmenuptr, int *loadsubsliceymenuptr, int *loadsubslicezmenuptr, int *loadsubslicexyzmenuptr
                           ){
  int i, loadmultislicemenu;
  int nloadsubmslicemenu;
  int iloadsubpatchmenu_s;
  int loadsubslicexmenu=0, loadsubsliceymenu=0, loadsubslicezmenu=0, loadsubslicexyzmenu=0;

  InitSubSliceMenuInfo();
  if(nsubslicemenuinfo>0){

//*** load all x slices

    CREATEMENU(loadsubslicexmenu,   LoadAllSlices);
    *loadsubslicexmenuptr = loadsubslicexmenu;
    for(i=0;i<nsubslicemenuinfo;i++){
      subslicemenudata *si;

      si = subslicemenuinfo + i;
      if(si->havex==1)glutAddMenuEntry(si->menulabel,4*i);
    }

//*** load all y slices

    CREATEMENU(loadsubsliceymenu,   LoadAllSlices);
    *loadsubsliceymenuptr = loadsubsliceymenu;
    for(i = 0;i < nsubslicemenuinfo;i++){
      subslicemenudata *si;

      si = subslicemenuinfo + i;
      if(si->havey == 1)glutAddMenuEntry(si->menulabel, 4*i+1);
    }

//*** load all z slices

    CREATEMENU(loadsubslicezmenu,   LoadAllSlices);
    *loadsubslicezmenuptr = loadsubslicezmenu;
    for(i = 0;i < nsubslicemenuinfo;i++){
      subslicemenudata *si;

      si = subslicemenuinfo + i;
      if(si->havez == 1)glutAddMenuEntry(si->menulabel, 4*i+2);
    }

//*** load all x,y,z slices

    CREATEMENU(loadsubslicexyzmenu, LoadAllSlices);
    *loadsubslicexyzmenuptr = loadsubslicexyzmenu;
    for(i = 0;i < nsubslicemenuinfo;i++){
      subslicemenudata *si;

      si = subslicemenuinfo + i;
      if(si->havexyz == 1)glutAddMenuEntry(si->menulabel, 4*i+3);
    }
  }
  CREATEMENU(loadmultislicemenu, LoadMultiSliceMenu);
  *loadmultislicemenuptr = loadmultislicemenu;
  nloadsubmslicemenu = 0;
  for(i = 0;i<global_scase.slicecoll.nmultisliceinfo;i++){
    slicedata *sd, *sdim1;

    sd = global_scase.slicecoll.sliceinfo+(global_scase.slicecoll.multisliceinfo+i)->islices[0];
    if(i>0)sdim1 = global_scase.slicecoll.sliceinfo+(global_scase.slicecoll.multisliceinfo+i-1)->islices[0];

    if(i==0||strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0){
      char mlabel[1024];

      STRCPY(mlabel, sd->label.longlabel);
      GLUTADDSUBMENU(mlabel, loadsubmslicemenu[nloadsubmslicemenu]);
      nloadsubmslicemenu++;
    }
  }
  if(global_scase.meshescoll.nmeshes>0){
    int ii;

    iloadsubpatchmenu_s = 0;
    for(ii = 0;ii<global_scase.npatchinfo;ii++){
      int im1;
      patchdata *patchi, *patchim1;

      i = patchorderindex[ii];
      if(ii>0){
        im1 = patchorderindex[ii-1];
        patchim1 = global_scase.patchinfo+im1;
      }
      patchi = global_scase.patchinfo+i;
      if(ii==0||strcmp(patchi->menulabel_base, patchim1->menulabel_base)!=0){
        if(nsubpatchmenus_s[iloadsubpatchmenu_s]>0){
          GLUTADDSUBMENU(patchi->menulabel_base, loadsubpatchmenu_s[iloadsubpatchmenu_s]);
        }
        iloadsubpatchmenu_s++;
      }
    }
  }
  if(nsubslicemenuinfo>0&&(nsubslicex>0||nsubslicey>0||nsubslicez>0||nsubslicexyz>0)){
    glutAddMenuEntry("-", MENU_DUMMY);
    if(nsubslicex>0)GLUTADDSUBMENU("Load all x slices",     loadsubslicexmenu);
    if(nsubslicey>0)GLUTADDSUBMENU("Load all y slices",     loadsubsliceymenu);
    if(nsubslicez>0)GLUTADDSUBMENU("Load all z slices",     loadsubslicezmenu);
    if(nsubslicexyz>0)GLUTADDSUBMENU("Load all x,y,z slices", loadsubslicexyzmenu);
  }
  if(global_scase.slicecoll.nmultisliceinfo>0)glutAddMenuEntry("-", MENU_DUMMY);

  GLUTADDSUBMENU(_("Skip"), sliceskipmenu);
  if(sortslices == 1){
    glutAddMenuEntry(_("*Sort slices(back to front)"), MENU_SPLITSLICES);
  }
  else{
    glutAddMenuEntry(_("Sort slices(back to front)"), MENU_SPLITSLICES);
  }
#ifdef _DEBUG
  if(sortslices == 1){
    glutAddMenuEntry(_("*Sort slices(debug)"), MENU_SPLITSLICES_DEBUG);
  }
  else{
    glutAddMenuEntry(_("Sort slices(debug)"), MENU_SPLITSLICES_DEBUG);
  }
#endif
  if(use_set_slicecolor == 1){
    glutAddMenuEntry(_("*Defer slice coloring"), MENU_SLICECOLORDEFER);
  }
  else{
    glutAddMenuEntry(_("Defer slice coloring"), MENU_SLICECOLORDEFER);
  }
  if(nslicedups > 0){
    GLUTADDSUBMENU(_("Duplicate slices"), duplicateslicemenu);
  }
  if(compute_slice_file_sizes==1){
    glutAddMenuEntry(_("*Show slice file sizes"), MENU_SLICE_FILE_SIZES);
  }
  else{
    glutAddMenuEntry(_("Show slice file sizes"), MENU_SLICE_FILE_SIZES);
  }
  glutAddMenuEntry("-", MENU_DUMMY);
  glutAddMenuEntry(_("Settings..."), MENU_SLICE_SETTINGS);
  if(nmultisliceloaded+geom_slice_loaded>1){
    GLUTADDSUBMENU(_("Unload"), unloadmultislicemenu);
  }
  else{
    glutAddMenuEntry(_("Unload"), UNLOAD_ALL);
  }
}

/* ------------------ InitUnloadVSLiceMenu ------------------------ */

void InitUnloadVSLiceMenu(int *unloadvslicemenuptr){
  int ii, unloadvslicemenu;

  CREATEMENU(unloadvslicemenu,UnloadVSliceMenu);
  *unloadvslicemenuptr = unloadvslicemenu;
  for(ii=0;ii<global_scase.slicecoll.nvsliceinfo;ii++){
    vslicedata *vd;
    int i;

    i = global_scase.vsliceorderindex[ii];
    vd = global_scase.slicecoll.vsliceinfo + i;
    if(vd->loaded==0)continue;
    glutAddMenuEntry(vd->menulabel2,i);
  }
  glutAddMenuEntry("-",MENU_DUMMY);
  glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);
}

/* ------------------ InitVSliceLoadMenu ------------------------ */

void InitVSliceLoadMenu(int *vsliceloadmenuptr, int *loadsubvslicemenu, int unloadvslicemenu){
  int vsliceloadmenu;

  CREATEMENU(vsliceloadmenu, LoadVSliceMenu);
  *vsliceloadmenuptr = vsliceloadmenu;
}

/* ------------------ InitDuplicateVectorSliceMenu ------------------------ */

void InitDuplicateVectorSliceMenu(int *duplicatevectorslicemenuptr){
  int duplicatevectorslicemenu;

  CREATEMENU(duplicatevectorslicemenu, LoadMultiVSliceMenu);
  *duplicatevectorslicemenuptr = duplicatevectorslicemenu;
  if(vectorslicedup_option==SLICEDUP_KEEPALL){
    glutAddMenuEntry(_("  *keep all"), MENU_KEEP_ALL);
  }
  else{
    glutAddMenuEntry(_("  keep all"), MENU_KEEP_ALL);
  }
  if(vectorslicedup_option==SLICEDUP_KEEPFINE){
    glutAddMenuEntry(_("  *keep fine"), MENU_KEEP_FINE);
  }
  else{
    glutAddMenuEntry(_("  keep fine"), MENU_KEEP_FINE);
  }
  if(vectorslicedup_option==SLICEDUP_KEEPCOARSE){
    glutAddMenuEntry(_("  *keep coarse"), MENU_KEEP_COARSE);
  }
  else{
    glutAddMenuEntry(_("  keep coarse"), MENU_KEEP_COARSE);
  }
}

/* ------------------ InitMultiVectorUnloadSliceMenu ------------------------ */

void InitMultiVectorUnloadSliceMenu(int *unloadmultivslicemenuptr){
  int unloadmultivslicemenu, i;

  CREATEMENU(unloadmultivslicemenu, UnloadMultiVSliceMenu);
  *unloadmultivslicemenuptr = unloadmultivslicemenu;
  for(i = 0; i<global_scase.slicecoll.nmultivsliceinfo; i++){
    multivslicedata *mvslicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo+i;
    if(mvslicei->loaded!=0){
      glutAddMenuEntry(mvslicei->menulabel2, i);
    }
  }
  glutAddMenuEntry(_("Unload all"), UNLOAD_ALL);
}


/* ------------------ InitMultiVectorSubMenu ------------------------ */

void InitMultiVectorSubMenu(int **loadsubmvslicemenuptr){
  int i, *loadsubmvslicemenu;
  int nloadsubmvslicemenu;


  nloadsubmvslicemenu = 1;
  for(i = 1; i<global_scase.slicecoll.nmultivsliceinfo; i++){
    vslicedata *vi, *vim1;
    slicedata *si, *sim1;

    vi = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i)->ivslices[0];
    vim1 = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i-1)->ivslices[0];
    si = global_scase.slicecoll.sliceinfo+vi->ival;
    sim1 = global_scase.slicecoll.sliceinfo+vim1->ival;
    if(strcmp(si->label.longlabel, sim1->label.longlabel)!=0){
      nloadsubmvslicemenu++;
    }
  }
  loadsubmvslicemenu = *loadsubmvslicemenuptr;
  NEWMEM(loadsubmvslicemenu, nloadsubmvslicemenu * sizeof(int));
  *loadsubmvslicemenuptr = loadsubmvslicemenu;
  for(i = 0; i<nloadsubmvslicemenu; i++){
    loadsubmvslicemenu[i] = 0;
  }

  nloadsubmvslicemenu = 0;
  for(i = 0; i<global_scase.slicecoll.nmultivsliceinfo; i++){
    vslicedata *vi, *vim1;
    slicedata *si, *sim1;
    char menulabel[1024];
    multivslicedata *mvslicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo+i;

    if(i>0){
      vim1 = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i-1)->ivslices[0];
      sim1 = global_scase.slicecoll.sliceinfo+vim1->ival;
    }
    vi = global_scase.slicecoll.vsliceinfo+mvslicei->ivslices[0];
    si = global_scase.slicecoll.sliceinfo+vi->ival;
    if(i==0||strcmp(si->label.longlabel, sim1->label.longlabel)!=0){
      CREATEMENU(loadsubmvslicemenu[nloadsubmvslicemenu], LoadMultiVSliceMenu);
      global_scase.msubvslice_menuindex[nloadsubmvslicemenu] = vi->ival;
    }

    STRCPY(menulabel, "");
    if(mvslicei->loadable == 0)strcat(menulabel, "***");
    if(mvslicei->loaded==1){
      STRCAT(menulabel, "*");
    }
    else if(mvslicei->loaded==-1){
      STRCAT(menulabel, "#");
    }
    else{
    }
    STRCAT(menulabel, mvslicei->menulabel);
    if(si->slicelabel!=NULL){
      STRCAT(menulabel, " - ");
      STRCAT(menulabel, si->slicelabel);
    }
    if(si->compression_type == COMPRESSED_ZLIB){
      STRCAT(menulabel, "(ZLIB)");
    }
    if(mvslicei->loadable == 0)strcat(menulabel, " - unavailable, no meshes with this data were selected");
    glutAddMenuEntry(menulabel, i);
    if(i==0||strcmp(si->label.longlabel, sim1->label.longlabel)!=0){
      nloadsubmvslicemenu++;
    }
  }
}

/* ------------------ InitMultiVectorLoadMenu ------------------------ */

void InitMultiVectorLoadMenu(int *loadmultivslicemenuptr, int *loadsubmvslicemenu, int duplicatevectorslicemenu,
                             int vsliceloadmenu, int sliceloadoptionmenu, int unloadmultivslicemenu
                             , int *loadsubvectorslicexmenuptr, int *loadsubvectorsliceymenuptr, int *loadsubvectorslicezmenuptr, int *loadsubvectorslicexyzmenuptr
                            ){
  int loadmultivslicemenu;
  int nloadsubmvslicemenu;
  int i;
  int loadsubvectorslicexmenu, loadsubvectorsliceymenu, loadsubvectorslicezmenu, loadsubvectorslicexyzmenu;

  InitSubVectorSliceMenuInfo();

//*** load all x vector slices

  CREATEMENU(loadsubvectorslicexmenu,   LoadAllVectorSlices);
  *loadsubvectorslicexmenuptr = loadsubvectorslicexmenu;
  for(i=0;i<nsubvectorslicemenuinfo;i++){
    subslicemenudata *si;

    si = subvectorslicemenuinfo + i;
    if(si->havex==1)glutAddMenuEntry(si->menulabel,4*i);
  }

//*** load all y vector slices

  CREATEMENU(loadsubvectorsliceymenu,   LoadAllVectorSlices);
  *loadsubvectorsliceymenuptr = loadsubvectorsliceymenu;
  for(i=0;i<nsubvectorslicemenuinfo;i++){
    subslicemenudata *si;

    si = subvectorslicemenuinfo + i;
    if(si->havey==1)glutAddMenuEntry(si->menulabel,4*i+1);
  }

//*** load all z vector slices

  CREATEMENU(loadsubvectorslicezmenu,   LoadAllVectorSlices);
  *loadsubvectorslicezmenuptr = loadsubvectorslicezmenu;
  for(i=0;i<nsubvectorslicemenuinfo;i++){
    subslicemenudata *si;

    si = subvectorslicemenuinfo + i;
    if(si->havez==1)glutAddMenuEntry(si->menulabel,4*i+2);
  }

//*** load all x,y,z vector slices

  CREATEMENU(loadsubvectorslicexyzmenu,   LoadAllVectorSlices);
  *loadsubvectorslicexyzmenuptr = loadsubvectorslicexyzmenu;
  for(i=0;i<nsubvectorslicemenuinfo;i++){
    subslicemenudata *si;

    si = subvectorslicemenuinfo + i;
    if(si->havexyz==1)glutAddMenuEntry(si->menulabel,4*i+3);
  }
  nloadsubmvslicemenu = 0;
  CREATEMENU(loadmultivslicemenu, LoadMultiVSliceMenu);
  *loadmultivslicemenuptr = loadmultivslicemenu;
  for(i = 0; i<global_scase.slicecoll.nmultivsliceinfo; i++){
    vslicedata *vi, *vim1;
    slicedata *si, *sim1;

    vi = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i)->ivslices[0];
    si = global_scase.slicecoll.sliceinfo+vi->ival;
    if(i>0){
      vim1 = global_scase.slicecoll.vsliceinfo+(global_scase.slicecoll.multivsliceinfo+i-1)->ivslices[0];
      sim1 = global_scase.slicecoll.sliceinfo+vim1->ival;
    }
    if(i==0||(i>0&&strcmp(si->label.longlabel, sim1->label.longlabel)!=0)){
      char mlabel[1024];

      STRCPY(mlabel, si->label.longlabel);
      GLUTADDSUBMENU(mlabel, loadsubmvslicemenu[nloadsubmvslicemenu]);
      nloadsubmvslicemenu++;
    }
  }
  if(nsubvectorslicemenuinfo>0&&(nsubvectorslicex>0||nsubvectorslicey>0||nsubvectorslicez>0||nsubvectorslicexyz>0)){
    glutAddMenuEntry("-", MENU_DUMMY);
    if(nsubvectorslicex>0)GLUTADDSUBMENU("Load all x vector slices",       loadsubvectorslicexmenu);
    if(nsubvectorslicey>0)GLUTADDSUBMENU("Load all y vector slices",       loadsubvectorsliceymenu);
    if(nsubvectorslicez>0)GLUTADDSUBMENU("Load all z vector slices",       loadsubvectorslicezmenu);
    if(nsubvectorslicexyz>0)GLUTADDSUBMENU("Load all x,y,z vector slices", loadsubvectorslicexyzmenu);
  }

  if(nslicedups > 0){
    GLUTADDSUBMENU(_("Duplicate vector slices"), duplicatevectorslicemenu);
  }
  glutAddMenuEntry("-", MENU_DUMMY);
  glutAddMenuEntry(_("Settings..."), MENU_LOADVSLICE_SETTINGS);
  if(nvsliceloaded>1){
    GLUTADDSUBMENU(_("Unload"),unloadmultivslicemenu);
  }
  else{
    glutAddMenuEntry(_("Unload"), UNLOAD_ALL);
  }
}

/* ------------------ InitPatchSubMenus ------------------------ */

void InitPatchSubMenus(int **loadsubpatchmenu_sptr, int **nsubpatchmenus_sptr){
  int ii, nloadsubpatchmenu_s, *loadsubpatchmenu_s, *nsubpatchmenus_s, iloadsubpatchmenu_s;

// count patch submenus

  have_geom_slice_menus=0;
  nloadsubpatchmenu_s = 0;
  for(ii = 0;ii<global_scase.npatchinfo;ii++){
    int im1;
    patchdata *patchi, *patchim1;
    int i;

    i = patchorderindex[ii];
    if(ii>0){
      im1 = patchorderindex[ii-1];
      patchim1 = global_scase.patchinfo+im1;
    }
    patchi = global_scase.patchinfo+i;
    if(ii==0||strcmp(patchi->menulabel_base, patchim1->menulabel_base)!=0){
      nloadsubpatchmenu_s++;
    }
  }

// create patch submenus

  if(nloadsubpatchmenu_s > 0){
    loadsubpatchmenu_s  = *loadsubpatchmenu_sptr;
    NEWMEM(loadsubpatchmenu_s, nloadsubpatchmenu_s * sizeof(int));
    *loadsubpatchmenu_sptr = loadsubpatchmenu_s;

    nsubpatchmenus_s  = *nsubpatchmenus_sptr;
    NEWMEM(nsubpatchmenus_s, nloadsubpatchmenu_s * sizeof(int));
    *nsubpatchmenus_sptr = nsubpatchmenus_s;
  }
  for(ii = 0;ii<nloadsubpatchmenu_s;ii++){
    loadsubpatchmenu_s[ii] = 0;
    nsubpatchmenus_s[ii] = 0;
  }

  iloadsubpatchmenu_s = 0;
  for(ii = 0;ii<global_scase.npatchinfo;ii++){
    int im1, i;
    patchdata *patchi, *patchim1;

    i = patchorderindex[ii];
    if(ii>0){
      im1 = patchorderindex[ii-1];
      patchim1 = global_scase.patchinfo+im1;
    }
    patchi = global_scase.patchinfo+i;
    if(ii==0||strcmp(patchi->menulabel_base, patchim1->menulabel_base)!=0){
      CREATEMENU(loadsubpatchmenu_s[iloadsubpatchmenu_s], LoadBoundaryMenu);
      iloadsubpatchmenu_s++;
    }
    if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label, "INCLUDE_GEOM")!=0)continue;
    if(nsubpatchmenus_s[iloadsubpatchmenu_s - 1] == 0 ||
      strcmp(patchi->menulabel_suffix, patchim1->menulabel_suffix) != 0){
      nsubpatchmenus_s[iloadsubpatchmenu_s-1]++;
      have_geom_slice_menus=1;
      glutAddMenuEntry(patchi->menulabel_suffix, -i-10);
    }
  }
}

/* ------------------ MakeSubColorbarMenu ------------------------ */

int MakeSubColorbarMenu(int *submenuptr, int *nmenusptr, char *ctype, void (*CBMenu)(int)){
  int i, nitems=0, submenu;

  for(i = 0; i < colorbars.ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + i;
    if(strcmp(cbi->colorbar_type, ctype) != 0)continue;
    nitems++;
    break;
  }
  if(nitems == 0)return 0;

  CREATEMENU(submenu, CBMenu);
  *submenuptr = submenu;
  char ccolorbarmenu[256];

  for(i = 0; i < colorbars.ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + colorbar_list_sorted[i];
    if(strcmp(cbi->colorbar_type, ctype) != 0)continue;
    strcpy(ccolorbarmenu, "");
    if(colorbartype == colorbar_list_sorted[i]){
      strcat(ccolorbarmenu, "*");
      strcat(ccolorbarmenu, cbi->menu_label);
    }
    else{
      strcat(ccolorbarmenu, cbi->menu_label);
    }
    char *ext;
    ext = strrchr(ccolorbarmenu, '.');
    if(ext != NULL)*ext = 0;
    glutAddMenuEntry(ccolorbarmenu, colorbar_list_sorted[i]);
  }
  return 1;
}

/* ------------------ MakeColorbarMenu ------------------------ */

void MakeColorbarMenu(int *menuptr,
                      int *submenu1ptr, int *submenu2ptr, int *submenu3ptr,
                      int *submenu4ptr, int *submenu5ptr, int *submenu6ptr,
                      int *submenu7ptr,
                      void (*CBMenu)(int)){
  int menu = 0;
  int submenu1 = 0, submenu2 = 0, submenu3 = 0;
  int submenu4 = 0, submenu5 = 0, submenu6 = 0;
  int submenu7 = 0;

  MakeSubColorbarMenu(&submenu3, &nmenus, "rainbow",      CBMenu);
  MakeSubColorbarMenu(&submenu7, &nmenus, "original",     CBMenu);
  MakeSubColorbarMenu(&submenu1, &nmenus, "linear",       CBMenu);
  MakeSubColorbarMenu(&submenu4, &nmenus, "divergent",    CBMenu);
  MakeSubColorbarMenu(&submenu2, &nmenus, "circular",     CBMenu);
  MakeSubColorbarMenu(&submenu6, &nmenus, "deprecated",   CBMenu);
  MakeSubColorbarMenu(&submenu5, &nmenus, "user defined", CBMenu);

  CREATEMENU(menu, CBMenu);
  if(submenu3 > 0)GLUTADDSUBMENU("rainbow",      submenu3);
  if(submenu7 > 0)GLUTADDSUBMENU("original",     submenu7);
  if(submenu1 > 0)GLUTADDSUBMENU("linear",       submenu1);
  if(submenu4 > 0)GLUTADDSUBMENU("divergent",    submenu4);
  if(submenu2 > 0)GLUTADDSUBMENU("circular",     submenu2);
  if(submenu6 > 0)GLUTADDSUBMENU("deprecated",   submenu6);
  if(submenu5 > 0)GLUTADDSUBMENU("user defined", submenu5);
  *submenu1ptr = submenu1;
  *submenu2ptr = submenu2;
  *submenu3ptr = submenu3;
  *submenu4ptr = submenu4;
  *submenu5ptr = submenu5;
  *submenu6ptr = submenu6;
  *submenu7ptr = submenu7;
  *menuptr     = menu;
}

/* ------------------ InitMenus ------------------------ */

void InitMenus(void){
  int i;
  int nmultisliceloaded;
  int showhide_data = 0;
  int patchgeom_slice_showhide;


static int filesdialogmenu = 0, viewdialogmenu = 0, datadialogmenu = 0, windowdialogmenu=0;
static int labelmenu=0, titlemenu=0, colorbarmenu=0, colorbarsmenu=0, colorbarshademenu, smokecolorbarmenu=0, showhidemenu=0,colorbardigitmenu=0;
static int optionmenu=0, rotatetypemenu=0, translatetypemenu=0;
static int colorbars_submenu1 = 0, colorbars_submenu2 = 0, colorbars_submenu3 = 0;
static int colorbars_submenu4 = 0, colorbars_submenu5 = 0, colorbars_submenu6 = 0;
static int colorbars_submenu7 = 0;
static int smokecolorbars_submenu1 = 0, smokecolorbars_submenu2 = 0, smokecolorbars_submenu3 = 0;
static int smokecolorbars_submenu4 = 0, smokecolorbars_submenu5 = 0, smokecolorbars_submenu6 = 0;
static int smokecolorbars_submenu7 = 0;
static int resetmenu=0, defaultviewmenu=0, frameratemenu=0, rendermenu=0, smokeviewinimenu=0, inisubmenu=0, resolutionmultipliermenu=0;
static int terrain_geom_showmenu = 0;
static int render_resolutionmenu=0, render_filetypemenu=0, render_filesuffixmenu=0, render_skipmenu=0;
static int render_startmenu = 0;
#ifdef pp_COMPRESS
static int compressmenu=0;
#endif
static int showhideslicemenu=0, sliceskipmenu=0, showvslicemenu=0;
static int loadsubslicexmenu=0, loadsubsliceymenu=0, loadsubslicezmenu=0, loadsubslicexyzmenu=0;
static int loadsubvectorslicexmenu=0, loadsubvectorsliceymenu=0, loadsubvectorslicezmenu=0, loadsubvectorslicexyzmenu=0;
static int plot3dshowmenu=0, staticvariablemenu=0, helpmenu=0, webhelpmenu=0, keyboardhelpmenu=0, mousehelpmenu=0;
static int vectorskipmenu=0,unitsmenu=0;
static int sliceloadoptionmenu = 0, vectorsliceloadoptionmenu = 0;
static int isosurfacemenu=0, isovariablemenu=0, levelmenu=0;
static int fontmenu=0, aperturemenu=0,dialogmenu=0,zoommenu=0;
static int gridslicemenu=0, griddigitsmenu=0, blockagemenu=0, immersedmenu=0, loadpatchmenu=0, ventmenu=0, circularventmenu=0;
static int loadisomenu=0, isosurfacetypemenu=0,showpatchextmenu=0;
static int geometrymenu=0, geometryoutlinemenu=0, loadunloadmenu=0, reloadmenu=0, fileinfomenu=0, aboutmenu=0, disclaimermenu=0, terrain_obst_showmenu=0;
static int scriptmenu=0;
static int hvacmenu = 0, hvacnetworkmenu, showcomponentmenu = 0, showfiltermenu = 0, connectivitymenu = 0;
static int hvacvaluemenu = 0, hvacnodevaluemenu = 0, hvacductvaluemenu = 0;
static int scriptlistmenu=0,scriptsteplistmenu=0,scriptrecordmenu=0;
static int loadplot3dmenu=0, unloadvslicemenu=0, unloadslicemenu=0;
static int loadsmoke3dmenu = 0;
static int loadvolsmoke3dmenu=0,showvolsmoke3dmenu=0;
static int unloadsmoke3dmenu=0,unloadvolsmoke3dmenu=0;
static int loadslicemenu=0, loadmultislicemenu = 0, loadhvacmenu = 0;
static int *loadsubvslicemenu=NULL, nloadsubvslicemenu=0;
static int *loadsubpatchmenu_b = NULL, *nsubpatchmenus_b=NULL, iloadsubpatchmenu_b=0, nloadsubpatchmenu_b = 0;
static int *loadsubpatchmenu_s = NULL, *nsubpatchmenus_s=NULL, nloadsubpatchmenu_s = 0;
static int *loadsubmslicemenu=NULL, nloadsubmslicemenu=0;
static int *loadsubmvslicemenu=NULL, nloadsubmvslicemenu=0;
static int *loadsubplot3dmenu=NULL, nloadsubplot3dmenu=0;
static int loadmultivslicemenu=0, unloadmultivslicemenu=0;
static int duplicatevectorslicemenu=0, duplicateslicemenu=0, duplicateboundaryslicemenu=0;
static int unloadmultislicemenu=0, vsliceloadmenu=0, staticslicemenu=0;
static int particlemenu=0, showpatchmenu=0, zonemenu=0, isoshowmenu=0, isolevelmenu=0, smoke3dshowmenu=0;
static int particlepropshowmenu=0;
static int *particlepropshowsubmenu=NULL;
static int particlestreakshowmenu=0;
static int tourmenu=0,tourcopymenu=0;
static int trainerviewmenu=0,mainmenu=0,zoneshowmenu=0,particleshowmenu=0;
static int showobjectsmenu=0,showdevicesmenu=0,showobjectsplotmenu=0,devicetypemenu=0,spheresegmentmenu=0,propmenu=0;
static int showmultislicemenu=0;
static int textureshowmenu=0;
#ifdef _DEBUG
static int menu_count=0;
#endif

//*** destroy existing menus
  updatemenu=0;
  GLUIUpdateShowHideButtons();
  GLUTPOSTREDISPLAY;

  for(i = 0; i < nmenus; i++){
    menudata *menui;

    menui = menuinfo + i;
    if(menui->menuvar > 0 && menui->status == 1){
      glutDestroyMenu(menui->menuvar);
      menui->status = 0;
    }
  }
  nmenus = 0;

  for(i=0;i<global_scase.slicecoll.nmultisliceinfo;i++){
    multislicedata *mslicei;
    int j;

    mslicei = global_scase.slicecoll.multisliceinfo + i;
    mslicei->loaded=0;
    mslicei->display=0;
    mslicei->loadable = 0;
    for(j=0;j<mslicei->nslices;j++){
      slicedata *sd;
      meshdata *meshi;

      sd = global_scase.slicecoll.sliceinfo + mslicei->islices[j];
      meshi = global_scase.meshescoll.meshinfo + sd->blocknumber;
      if(meshi->use == 1)mslicei->loadable = 1;
      if(sd->loaded==1)mslicei->loaded++;
      if(sd->display==1)mslicei->display++;
    }
    if(mslicei->loaded>0&&mslicei->loaded<mslicei->nslices){
      mslicei->loaded=-1;
    }
    else if(mslicei->loaded==mslicei->nslices){
      mslicei->loaded=1;
    }
    if(mslicei->display>0&&mslicei->display<mslicei->nslices){
      mslicei->display=-1;
    }
    else if(mslicei->display==mslicei->nslices){
      mslicei->display=1;
    }
  }
  for(i=0;i<global_scase.slicecoll.nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    int j;

    mvslicei = global_scase.slicecoll.multivsliceinfo + i;
    mvslicei->loaded   = 0;
    mvslicei->display  = 0;
    mvslicei->loadable = 0;
    for(j=0;j<mvslicei->nvslices;j++){
      vslicedata *vd;
      meshdata *meshi;
      slicedata *valslice;

      vd = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[j];
      valslice = global_scase.slicecoll.sliceinfo + vd->ival;
      meshi = global_scase.meshescoll.meshinfo + valslice->blocknumber;
      if(meshi->use==1)mvslicei->loadable = 1;
      if(vd->loaded==1)mvslicei->loaded++;
      if(vd->display==1)mvslicei->display++;
    }
    if(mvslicei->loaded>0&&mvslicei->loaded<mvslicei->nvslices){
      mvslicei->loaded=-1;
    }
    else if(mvslicei->loaded==mvslicei->nvslices){
      mvslicei->loaded=1;
    }
    if(mvslicei->display>0&&mvslicei->display<mvslicei->nvslices){
      mvslicei->display=-1;
    }
    else if(mvslicei->display==mvslicei->nvslices){
      mvslicei->display=1;
    }
  }

  if(nloadsubpatchmenu_b > 0){
    FREEMEMORY(loadsubpatchmenu_b);
    FREEMEMORY(nsubpatchmenus_b);
  }
  if(nloadsubpatchmenu_s>0){
    FREEMEMORY(loadsubpatchmenu_s);
    FREEMEMORY(nsubpatchmenus_s);
  }
  if(nloadsubmslicemenu>0){
    FREEMEMORY(loadsubmslicemenu);
  }
  if(nloadsubvslicemenu>0){
    FREEMEMORY(loadsubvslicemenu);
  }
  FREEMEMORY(particlepropshowsubmenu);
  if(nloadsubmvslicemenu>0){
    FREEMEMORY(loadsubmvslicemenu);
  }
  if(nloadsubplot3dmenu>0){
    FREEMEMORY(loadsubplot3dmenu);
  }

  patchgeom_slice_showhide = 0;
  for(i=0;i<global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==1){
      if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
        patchgeom_slice_showhide = 1;
      }
    }
  }

/* --------------------------------patch menu -------------------------- */
  if(global_scase.npatchinfo>0){
    int ii;
    char menulabel[1024];
    int next_total=0;

    CREATEMENU(showpatchextmenu, ShowBoundaryMenu);
    for(i=1;i<7;i++){
      next_total+=vis_boundary_type[i];
    }
    if(next_total == 6){
      show_all_exterior_patch_data = 1;
      hide_all_exterior_patch_data = 0;
      glutAddMenuEntry(_("*Show all"),  SHOW_EXTERIOR_WALL_MENU);
      glutAddMenuEntry(_("Hide all"),   HIDE_EXTERIOR_WALL_MENU);
    }
    else if(next_total == 0){
      show_all_exterior_patch_data = 0;
      hide_all_exterior_patch_data = 1;
      glutAddMenuEntry(_("Show all"),  SHOW_EXTERIOR_WALL_MENU);
      glutAddMenuEntry(_("*Hide all"), HIDE_EXTERIOR_WALL_MENU);
    }
    else{
      show_all_exterior_patch_data = 0;
      hide_all_exterior_patch_data = 0;
      glutAddMenuEntry(_("#Show all"),  SHOW_EXTERIOR_WALL_MENU);
      glutAddMenuEntry(_("#Hide all"),  HIDE_EXTERIOR_WALL_MENU);
    }
    UpdateShowExtPatch(show_all_exterior_patch_data, hide_all_exterior_patch_data);
    if(IsBoundaryType(FRONTwall) == 1){
      if(vis_boundary_type[FRONTwall] == 1)glutAddMenuEntry(_("*Front"), FRONTwallmenu);
      if(vis_boundary_type[FRONTwall] == 0)glutAddMenuEntry(_("Front"), FRONTwallmenu);
    }
    if(IsBoundaryType(BACKwall) == 1){
      if(vis_boundary_type[BACKwall] == 1)glutAddMenuEntry(_("*Back"), BACKwallmenu);
      if(vis_boundary_type[BACKwall] == 0)glutAddMenuEntry(_("Back"), BACKwallmenu);
    }
    if(IsBoundaryType(LEFTwall) == 1){
      if(vis_boundary_type[LEFTwall] == 1)glutAddMenuEntry(_("*Left"), LEFTwallmenu);
      if(vis_boundary_type[LEFTwall] == 0)glutAddMenuEntry(_("Left"), LEFTwallmenu);
    }
    if(IsBoundaryType(RIGHTwall) == 1){
      if(vis_boundary_type[RIGHTwall] == 1)glutAddMenuEntry(_("*Right"), RIGHTwallmenu);
      if(vis_boundary_type[RIGHTwall] == 0)glutAddMenuEntry(_("Right"), RIGHTwallmenu);
    }
    if(IsBoundaryType(UPwall) == 1){
      if(vis_boundary_type[UPwall] == 1)glutAddMenuEntry(_("*Up"), UPwallmenu);
      if(vis_boundary_type[UPwall] == 0)glutAddMenuEntry(_("Up"), UPwallmenu);
    }
    if(IsBoundaryType(DOWNwall) == 1){
      if(vis_boundary_type[DOWNwall] == 1)glutAddMenuEntry(_("*Down"), DOWNwallmenu);
      if(vis_boundary_type[DOWNwall] == 0)glutAddMenuEntry(_("Down"), DOWNwallmenu);
    }

    CREATEMENU(showpatchmenu,ShowBoundaryMenu);
    if(npatchloaded>0){
      patchdata *patchi=NULL, *patchim1=NULL;

      for(ii = 0;ii<global_scase.npatchinfo;ii++){

        i = patchorderindex[ii];
        patchi = global_scase.patchinfo+i;
        if(patchi->loaded==0)continue;
        if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0)continue;
        if(ii>0){
          if(patchim1!=NULL){
            if(strcmp(patchi->label.longlabel,patchim1->label.longlabel)==0){
              patchim1 = patchi;
              continue;
            }
          }
        }
        patchim1 = patchi;
        strcpy(menulabel,"");
        if(patchi->display==1){
          strcat(menulabel,"*");
        }
        strcat(menulabel,patchi->label.longlabel);
        if(patchi->structured == NO){
          if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label, "INCLUDE_GEOM")!=0){
            glutAddMenuEntry(menulabel, 1000+i);
          }
        }
        else{
          if(show_boundaryfiles==1){
            glutAddMenuEntry("*Show all", GLUI_SHOWALL_BOUNDARY);
            glutAddMenuEntry("Hide all",  GLUI_HIDEALL_BOUNDARY);
          }
          else{
            glutAddMenuEntry("Show all",  GLUI_SHOWALL_BOUNDARY);
            glutAddMenuEntry("*Hide all", GLUI_HIDEALL_BOUNDARY);
          }
        }
      }
    }
    npatchloaded=0;
    {
      int local_do_threshold=0;

      for(i = 0;i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo+i;
        if(patchi->loaded==0)continue;
        if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0)continue;
        npatchloaded++;
      }
      for(ii=0;ii<global_scase.npatchinfo;ii++){
        patchdata *patchi;

        i = patchorderindex[ii];
        patchi = global_scase.patchinfo+i;
        if(patchi->loaded==0)continue;
        if(activate_threshold==1){
          if(
            strncmp(patchi->label.shortlabel,"TEMP",4) == 0||
            strncmp(patchi->label.shortlabel,"temp",4) == 0
           ){
            local_do_threshold=1;
          }
        }
      }
      if(activate_threshold == 1 && local_do_threshold == 1){
        glutAddMenuEntry("-",DUMMYwallmenu);
        if(vis_threshold==1)glutAddMenuEntry("*char",SHOW_CHAR);
        if(vis_threshold==0)glutAddMenuEntry("char",SHOW_CHAR);
      }
    }
    GLUTADDSUBMENU(_("Exterior"), showpatchextmenu);
    if(vis_boundary_type[INTERIORwall]==1)glutAddMenuEntry(_("*Interior"), INTERIOR_WALL_MENU);
    if(vis_boundary_type[INTERIORwall]==0)glutAddMenuEntry(_("Interior"),  INTERIOR_WALL_MENU);
  }

  /* --------------------------------terrain menu -------------------------- */


  if(global_scase.terrain_texture_coll.nterrain_textures>0){
    CREATEMENU(terrain_geom_showmenu, TerrainGeomShowMenu);
    for(i = 0; i<global_scase.terrain_texture_coll.nterrain_textures; i++){
      texturedata *texti;

      texti = global_scase.terrain_texture_coll.terrain_textures+i;
      if(texti->loaded==1){
        char tlabel[256];

        strcpy(tlabel, "  ");
        if(texti->display==1)strcat(tlabel, "*");
        strcat(tlabel, texti->file);
        if(texti->display==1)glutAddMenuEntry(tlabel, i);
        if(texti->display==0)glutAddMenuEntry(tlabel, i);
      }
    }
  }

/* --------------------------------surface menu -------------------------- */

  CREATEMENU(immersedmenu, ImmersedMenu);
  glutAddMenuEntry(_("What"),GEOMETRY_DUMMY);
  if(show_faces_shaded==1&&show_faces_outline==1){
    glutAddMenuEntry(_("   *Solid and outline"),GEOMETRY_SOLIDOUTLINE);
  }
  else{
    glutAddMenuEntry(_("   Solid and outline"),GEOMETRY_SOLIDOUTLINE);
  }
  if(show_faces_shaded==1&&show_faces_outline==0){
    glutAddMenuEntry(_("   *Solid only"),GEOMETRY_SOLID);
  }
  else{
    glutAddMenuEntry(_("   Solid only"),GEOMETRY_SOLID);
  }
  if(show_faces_outline==1&&show_faces_shaded==0){
    glutAddMenuEntry(_("   *Outline only"),GEOMETRY_OUTLINE);
  }
  else{
    glutAddMenuEntry(_("   Outline only"),GEOMETRY_OUTLINE);
  }
  if(show_faces_shaded == 0 && show_faces_outline == 0){
    glutAddMenuEntry(_("   *Hide"),GEOMETRY_HIDE);
  }
  else{
    glutAddMenuEntry(_("   Hide"),GEOMETRY_HIDE);
  }
  glutAddMenuEntry(_("Where"),GEOMETRY_DUMMY);
  if(showgeom_inside_domain == 1){
    glutAddMenuEntry(_("   *Inside FDS domain"), GEOMETRY_INSIDE_DOMAIN);
  }
  else{
    glutAddMenuEntry(_("   Inside Domain domain"), GEOMETRY_INSIDE_DOMAIN);
  }
  if(showgeom_outside_domain == 1){
    glutAddMenuEntry(_("   *Outside FDS domain"), GEOMETRY_OUTSIDE_DOMAIN);
  }
  else{
    glutAddMenuEntry(_("   Outside FDS domain"), GEOMETRY_OUTSIDE_DOMAIN);
  }
  glutAddMenuEntry("-", GEOMETRY_DUMMY);
  if(global_scase.terrain_texture_coll.nterrain_textures>0){
    GLUTADDSUBMENU(_("Terrain images"), terrain_geom_showmenu);
  }
  if(terrain_nindices>0){
    if(terrain_showonly_top==1)glutAddMenuEntry(_("*Show only top surface"), GEOMETRY_TERRAIN_SHOW_TOP);
    if(terrain_showonly_top==0)glutAddMenuEntry(_("Show only top surface"), GEOMETRY_TERRAIN_SHOW_TOP);
  }
#ifdef _DEBUG
  if(show_triangle_count==1)glutAddMenuEntry(_("*Triangle count"), GEOM_TriangleCount);
  if(show_triangle_count==0)glutAddMenuEntry(_("Triangle count"),  GEOM_TriangleCount);
#endif
  if(show_geom_normal==1){
    glutAddMenuEntry(_("*Show normal"), GEOMETRY_SHOWNORMAL);
  }
  else{
    glutAddMenuEntry(_("Show normal"), GEOMETRY_SHOWNORMAL);
  }
  if(smooth_geom_normal==1){
    glutAddMenuEntry(_("*Smooth normal"), GEOMETRY_SMOOTHNORMAL);
  }
  else{
    glutAddMenuEntry(_("Smooth normal"), GEOMETRY_SMOOTHNORMAL);
  }
  if(sort_geometry==1){
    glutAddMenuEntry(_("*Sort faces"), GEOMETRY_SORTFACES);
  }
  else{
    glutAddMenuEntry(_("Sort faces"), GEOMETRY_SORTFACES);
  }
  if(hilight_skinny==1){
    glutAddMenuEntry(_("*Hilight skinny triangles"), GEOMETRY_HILIGHTSKINNY);
  }
  else{
    glutAddMenuEntry(_("Hilight skinny triangles"), GEOMETRY_HILIGHTSKINNY);
  }

  /* --------------------------------blockage menu -------------------------- */

  CREATEMENU(blockagemenu,BlockageMenu);
  glutAddMenuEntry(_("View Method:"),MENU_DUMMY);
  if(global_scase.tourcoll.tourinfo!=NULL&&global_scase.have_animate_blockages == 1){
    if(animate_blockages == 1)glutAddMenuEntry(_("   *Animate blockages"), ANIMATE_BLOCKAGES);
    if(animate_blockages==0)glutAddMenuEntry(_("   Animate blockages"),    ANIMATE_BLOCKAGES);
  }
  if(visBlocks==visBLOCKAsInput||visBlocks==visBLOCKAsInputOutline){
    glutAddMenuEntry(_("   *Defined in input file"),visBLOCKAsInput);
  }
   else{
    glutAddMenuEntry(_("   Defined in input file"),visBLOCKAsInput);
  }
  if(visBlocks==visBLOCKNormal||visBlocks==visBLOCKSolidOutline){
    glutAddMenuEntry(_("   *Solid"),visBLOCKNormal);
    if(ntransparentblocks>0){
      if(visTransparentBlockage==1){
         glutAddMenuEntry(_("      *Transparent"),visBLOCKTransparent);
      }
      else{
         glutAddMenuEntry(_("      Transparent"),visBLOCKTransparent);
      }
    }
  }
  else{
    glutAddMenuEntry(_("   Solid"),visBLOCKNormal);
  }
  if(outline_state==OUTLINE_ONLY){
    glutAddMenuEntry(_("   *Outline only"),visBLOCKOnlyOutline);
  }
  else{
    glutAddMenuEntry(_("   Outline only"),visBLOCKOnlyOutline);
  }
  if(outline_state==OUTLINE_ADDED){
    glutAddMenuEntry(_("   *Outline added"),visBLOCKAddOutline);
  }
  else{
    glutAddMenuEntry(_("   Outline added"),visBLOCKAddOutline);
  }
  if(NCADGeom(&global_scase.cadgeomcoll)>0){
    if(viscadopaque==1){
      glutAddMenuEntry(_("   *Cad surface drawn opaque"),visCADOpaque);
    }
    else{
      glutAddMenuEntry(_("   Cad surface drawn opaque"),visCADOpaque);
    }
  }
  if(visBlocks==visBLOCKHide){
    glutAddMenuEntry(_("   *Hidden"),visBLOCKHide);
  }
  else{
    glutAddMenuEntry(_("   Hidden"),visBLOCKHide);
  }
  if(light_faces == 1){
    glutAddMenuEntry(_("   *Light faces"), visLightFaces);
  }
  else{
    glutAddMenuEntry(_("   Light faces"), visLightFaces);
  }
  glutAddMenuEntry("-",MENU_DUMMY);
  glutAddMenuEntry(_(" Outline color:"),MENU_DUMMY);
  if(outline_color_flag==1){
    glutAddMenuEntry(_("   use blockage"),visBLOCKOutlineColor);
    glutAddMenuEntry(_("   *use foreground"),visBLOCKOutlineColor);
  }
  else{
    glutAddMenuEntry(_("   *use blockage"),visBLOCKOutlineColor);
    glutAddMenuEntry(_("   use foreground"),visBLOCKOutlineColor);
  }
  {
    int nblockprop=0;

    for(i=0;i<global_scase.propcoll.npropinfo;i++){
      propdata *propi;

      propi = global_scase.propcoll.propinfo + i;
      if(propi->inblockage==1)nblockprop++;
    }
    if(nblockprop>0){
      char propmenulabel[255];

      glutAddMenuEntry("-",MENU_DUMMY);
      glutAddMenuEntry(_("Show/Hide blockage types:"),MENU_DUMMY);
      for(i=0;i<global_scase.propcoll.npropinfo;i++){
        propdata *propi;

        propi = global_scase.propcoll.propinfo + i;
        if(propi->inblockage==1){
          strcpy(propmenulabel,"    ");
          if(propi->blockvis==1)strcat(propmenulabel,"*");
          strcat(propmenulabel,propi->label);
          glutAddMenuEntry(propmenulabel,-i-1);
        }
      }
    }
  }

/* --------------------------------level menu -------------------------- */

  if(global_scase.nplot3dinfo>0){
    CREATEMENU(levelmenu,LevelMenu);
    for(i=1;i<global_scase.nrgb-1;i++){
      if(colorlabeliso!=NULL){
        char *colorlabel;
        char levellabel2[256];

        colorlabel=&colorlabeliso[plotn-1][global_scase.nrgb-2-i][0];
        strcpy(levellabel2,"");
        if(plotiso[plotn-1]==global_scase.nrgb-2-i&&visiso==1){
          strcat(levellabel2,"*");
        }
        strcat(levellabel2,colorlabel);
        glutAddMenuEntry(levellabel2,global_scase.nrgb-2-i);
      }
      else{
        char chari[20];

        if(plotiso[plotn-1]==i&&visiso==1){
          sprintf(chari,"*%i",i+1);
        }
        else{
          sprintf(chari,"%i",i+1);
        }
        glutAddMenuEntry(chari,i+1);
      }
    }
  }

/* --------------------------------static variable menu -------------------------- */

  if(global_scase.nplot3dinfo>0){
    int n;

    CREATEMENU(staticvariablemenu,StaticVariableMenu);
    for(n=0;n<numplot3dvars;n++){
      char *p3label;

      p3label = global_scase.plot3dinfo[0].label[n].shortlabel;
      if(plotn-1==n){
        char menulabel[1024];

        STRCPY(menulabel,"*");
        STRCAT(menulabel,p3label);
        glutAddMenuEntry(menulabel,n+1);
      }
      else{
        glutAddMenuEntry(p3label,n+1);
      }
    }
  }

/* --------------------------------iso variable menu -------------------------- */

  if(global_scase.nplot3dinfo>0){
    int n;

    CREATEMENU(isovariablemenu,IsoVariableMenu);
    for(n=0;n<numplot3dvars;n++){
      char *p3label;

      p3label = global_scase.plot3dinfo[0].label[n].shortlabel;
      if(plotn-1==n&&visiso==1){
        char menulabel[1024];

        STRCPY(menulabel,"*");
        STRCAT(menulabel,p3label);
        glutAddMenuEntry(menulabel,n+1);
      }
      else{
        glutAddMenuEntry(p3label,n+1);
      }
    }
  }

/* --------------------------------iso surface menu -------------------------- */
  if(global_scase.nplot3dinfo>0){
    CREATEMENU(isosurfacetypemenu,IsoSurfaceTypeMenu);
    if(p3dsurfacesmooth==1&&p3dsurfacetype==SURFACE_SOLID){
      glutAddMenuEntry(_("*Smooth"),MENU_SURFACE_SMOOTH);
    }
     else{
       glutAddMenuEntry(_("Smooth"),MENU_SURFACE_SMOOTH);
     }
     if(p3dsurfacesmooth==0&&p3dsurfacetype==SURFACE_SOLID){
       glutAddMenuEntry(_("*Facets"),MENU_SURFACE_FACET);
     }
    else{
      glutAddMenuEntry(_("Facets"),MENU_SURFACE_FACET);
    }
    if(p3dsurfacetype==SURFACE_OUTLINE)glutAddMenuEntry(_("*Triangles"),SURFACE_OUTLINE);
    if(p3dsurfacetype!=SURFACE_OUTLINE)glutAddMenuEntry(_("Triangles"),SURFACE_OUTLINE);
    if(p3dsurfacetype == SURFACE_POINTS)glutAddMenuEntry(_("*Points"), SURFACE_POINTS);
    if(p3dsurfacetype != SURFACE_POINTS)glutAddMenuEntry(_("Points"), SURFACE_POINTS);

    CREATEMENU(isosurfacemenu,IsoSurfaceMenu);
    GLUTADDSUBMENU(_("Solution variable"),isovariablemenu);
    GLUTADDSUBMENU(_("Solution value"),levelmenu);
    GLUTADDSUBMENU(_("Surface type"),isosurfacetypemenu);
    glutAddMenuEntry(_("Hide"),1);
  }

/* --------------------------------vector skip menu -------------------------- */

  if(global_scase.nplot3dinfo>0){
    CREATEMENU(vectorskipmenu,VectorSkipMenu);
    if(visVector==1)glutAddMenuEntry(_("*Show"),MENU_VECTOR_SHOW);
    if(visVector!=1)glutAddMenuEntry(_("Show"),MENU_VECTOR_SHOW);
    glutAddMenuEntry(_("Frequency:"),-1);
    if(vectorskip==1)glutAddMenuEntry(_("*All"),1);
    if(vectorskip!=1)glutAddMenuEntry(_("All"),1);
    if(vectorskip==2)glutAddMenuEntry(_("*Every 2nd"),2);
    if(vectorskip!=2)glutAddMenuEntry(_("Every 2nd"),2);
    if(vectorskip==3)glutAddMenuEntry(_("*Every 3rd"),3);
    if(vectorskip!=3)glutAddMenuEntry(_("Every 3rd"),3);
    if(vectorskip==4)glutAddMenuEntry(_("*Every 4th"),4);
    if(vectorskip!=4)glutAddMenuEntry(_("Every 4th"),4);
  }

  if(ntextures_loaded_used>0){
    int ntextures_used;

    CREATEMENU(textureshowmenu,TextureShowMenu);
    ntextures_used=0;
    for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
      texturedata *texti;
      char menulabel[1024];

      texti = global_scase.texture_coll.textureinfo + i;
      if(texti->loaded == 0 || texti->used == 0)continue;
      if(global_scase.terrain_texture_coll.terrain_textures != NULL){
        if(texti >= global_scase.terrain_texture_coll.terrain_textures && texti < global_scase.terrain_texture_coll.terrain_textures + global_scase.terrain_texture_coll.nterrain_textures)continue;
      }
      ntextures_used++;
      if(texti->display==1){
        STRCPY(menulabel,"*");
        STRCAT(menulabel,texti->file);
        glutAddMenuEntry(menulabel,i);
      }
      else{
        STRCPY(menulabel,texti->file);
        glutAddMenuEntry(menulabel,i);
      }
    }
    if(ntextures_used>1){
      glutAddMenuEntry("-",MENU_DUMMY);
      glutAddMenuEntry(_("Show all"),MENU_TEXTURE_SHOWALL);
      glutAddMenuEntry(_("Hide all"),MENU_TEXTURE_HIDEALL);
    }
  }

/* --------------------------------Plot3d Show menu -------------------------- */
  if(global_scase.nplot3dinfo>0){
    CREATEMENU(staticslicemenu,Plot3DShowMenu);
    GLUTADDSUBMENU(_("Solution variable"),staticvariablemenu);
    if(visz_all==1)glutAddMenuEntry(_("*xy plane"), MENU_PLOT3D_Z);
    if(visz_all==0)glutAddMenuEntry(_("xy plane"), MENU_PLOT3D_Z);
    if(visy_all==1)glutAddMenuEntry(_("*xz plane"), MENU_PLOT3D_Y);
    if(visy_all==0)glutAddMenuEntry(_("xz plane"), MENU_PLOT3D_Y);
    if(visx_all==1)glutAddMenuEntry(_("*yz plane"), MENU_PLOT3D_X);
    if(visx_all==0)glutAddMenuEntry(_("yz plane"), MENU_PLOT3D_X);
    if(vectorspresent==1)GLUTADDSUBMENU(_("Flow vectors"),vectorskipmenu);
    if(contour_type==SHADED_CONTOURS){
      glutAddMenuEntry(_("*Continuous contours"), MENU_PLOT3D_CONT);
    }
    if(contour_type!=SHADED_CONTOURS){
      glutAddMenuEntry(_("Continuous contours"), MENU_PLOT3D_CONT);
    }
    glutAddMenuEntry(_("Show all planes"), MENU_PLOT3D_SHOWALL);
    glutAddMenuEntry(_("Hide all planes"), MENU_PLOT3D_HIDEALL);

    CREATEMENU(plot3dshowmenu,Plot3DShowMenu);
    if(nplot3dloaded>0){
      int ii;
      plot3ddata *plot3di;
      char menulabel[1024];

      for(ii = 0;ii<global_scase.nplot3dinfo;ii++){
        i = plot3dorderindex[ii];
        plot3di = global_scase.plot3dinfo+i;
        if(plot3di->loaded==0)continue;
        strcpy(menulabel, "");
        if(show_plot3dfiles==1)strcat(menulabel, "*");
        strcat(menulabel, plot3di->timelabel);
        strcat(menulabel, ", ");
        strcat(menulabel, plot3di->longlabel);
        glutAddMenuEntry(menulabel, TOGGLESHOW_PLOT3D);
        break;
      }
    }
    GLUTADDSUBMENU(_("2D contours"),staticslicemenu);
    if(cache_plot3d_data==1){
      GLUTADDSUBMENU(_("3D contours"),isosurfacemenu);
    }
  }

/* --------------------------------grid digits menu -------------------------- */

  CREATEMENU(griddigitsmenu, GridDigitsMenu);
  for(i = GRIDLOC_NDECIMALS_MIN; i<=GRIDLOC_NDECIMALS_MAX;i++){
    char digit_label[10];

    if(i==ngridloc_digits){
      sprintf(digit_label, "*%i", i);
    }
    else{
      sprintf(digit_label, "%i", i);
    }
    glutAddMenuEntry(digit_label, i);
  }

  CREATEMENU(gridslicemenu,GridSliceMenu);
  if(visGrid==GRID_NOPROBE||visGrid==GRID_PROBE){
    glutAddMenuEntry(_("*show grid"),GRID_grid);
  }
  else{
    glutAddMenuEntry(_("show grid"),GRID_grid);
  }
  if(visGrid==GRID_PROBE||visGrid==NOGRID_PROBE){
    glutAddMenuEntry(_("*show grid location"),GRID_probe);
  }
  else{
    glutAddMenuEntry(_("show grid location"),GRID_probe);
  }
  GLUTADDSUBMENU(_("grid location digits"), griddigitsmenu);

  glutAddMenuEntry("-",MENU_DUMMY);
  if(visz_all==1){
    glutAddMenuEntry(_("*xy plane"),GRID_xy);
  }
  else{
    glutAddMenuEntry(_("xy plane"),GRID_xy);
  }
  if(visy_all==1){
    glutAddMenuEntry(_("*xz plane"),GRID_xz);
  }
  else{
    glutAddMenuEntry(_("xz plane"),GRID_xz);
  }
  if(visx_all==1){
    glutAddMenuEntry(_("*yz plane"),GRID_yz);
  }
  else{
    glutAddMenuEntry(_("yz plane"),GRID_yz);
  }
  if(visx_all==0||visy_all==0||visz_all==0){
    glutAddMenuEntry(_("Show all planes"),GRID_showall);
  }
  if(visx_all==1||visy_all==1||visz_all==1){
    glutAddMenuEntry(_("Hide all planes"),GRID_hideall);
  }

  CREATEMENU(circularventmenu,VentMenu);
  if(visCircularVents==VENT_CIRCLE){
    glutAddMenuEntry(_("*As circle"), MENU_VENT_CIRCLE);
    glutAddMenuEntry(_("As rectangle"), MENU_VENT_RECTANGLE);
    glutAddMenuEntry(_("Hide"), MENU_VENT_CIRCLEHIDE);
  }
  if(visCircularVents==VENT_RECTANGLE){
    glutAddMenuEntry(_("As circle"), MENU_VENT_CIRCLE);
    glutAddMenuEntry(_("*As rectangle"), MENU_VENT_RECTANGLE);
    glutAddMenuEntry(_("Hide"), MENU_VENT_CIRCLEHIDE);
  }
  if(visCircularVents==VENT_HIDE){
    glutAddMenuEntry(_("As circle"), MENU_VENT_CIRCLE);
    glutAddMenuEntry(_("As rectangle"), MENU_VENT_RECTANGLE);
    glutAddMenuEntry(_("*Hide"), MENU_VENT_CIRCLEHIDE);
  }
  glutAddMenuEntry("-",MENU_DUMMY2);
  if(circle_outline == 1)glutAddMenuEntry(_("*Outline"), MENU_VENT_CIRCLEOUTLINE);
  if(circle_outline == 0)glutAddMenuEntry(_("Outline"), MENU_VENT_CIRCLEOUTLINE);

/* --------------------------------vent menu -------------------------- */

  CREATEMENU(ventmenu,VentMenu);
  if(GetNTotalVents()>0){
    if(nopenvents>0){
      if(visOpenVents == 1)glutAddMenuEntry(_("*Open"), MENU_VENT_OPEN);
      if(visOpenVents == 0)glutAddMenuEntry(_("Open"), MENU_VENT_OPEN);
    }
    if(global_scase.ndummyvents>0){
      if(visDummyVents == 1)glutAddMenuEntry(_("*Exterior"), MENU_VENT_EXTERIOR);
      if(visDummyVents == 0)glutAddMenuEntry(_("Exterior"), MENU_VENT_EXTERIOR);
    }
    if(global_scase.ncvents>0){
      if(visCircularVents!=VENT_HIDE)GLUTADDSUBMENU(_("*Circular"),circularventmenu);
      if(visCircularVents==VENT_HIDE)GLUTADDSUBMENU(_("Circular"),circularventmenu);
    }
    if(GetNTotalVents()>nopenvents+global_scase.ndummyvents){
      if(global_scase.visOtherVents == 1)glutAddMenuEntry(_("*Other"), MENU_VENT_OTHER);
      if(global_scase.visOtherVents == 0)glutAddMenuEntry(_("Other"), MENU_VENT_OTHER);
    }
    if(visOpenVents==1&&visDummyVents==1&&global_scase.visOtherVents==1){
      glutAddMenuEntry(_("*Show all"),SHOW_ALL_VENTS);
    }
    else{
      glutAddMenuEntry(_("Show all"),SHOW_ALL_VENTS);
    }
    if(visOpenVents==0&&visDummyVents==0&&global_scase.visOtherVents==0){
      glutAddMenuEntry(_("*Hide all"),HIDE_ALL_VENTS);
    }
    else{
      glutAddMenuEntry(_("Hide all"),HIDE_ALL_VENTS);
    }
    glutAddMenuEntry("-",MENU_DUMMY2);
    if(nopenvents_nonoutline>0){
      if(visOpenVentsAsOutline == 1)glutAddMenuEntry(_("*Open vents as outlines"), MENU_VENT_OUTLINE);
      if(visOpenVentsAsOutline == 0)glutAddMenuEntry(_("Open vents as outlines"), MENU_VENT_OUTLINE);
    }
    if(have_vents_int==1){
      if(show_bothsides_int == 1)glutAddMenuEntry(_("*Two sided (interior)"), MENU_VENT_TWOINTERIOR);
      if(show_bothsides_int == 0)glutAddMenuEntry(_("Two sided (interior)"), MENU_VENT_TWOINTERIOR);
    }
    if(show_bothsides_ext == 1)glutAddMenuEntry(_("*Two sided (exterior)"), MENU_VENT_TWOEXTERIOR);
    if(show_bothsides_ext == 0)glutAddMenuEntry(_("Two sided (exterior)"), MENU_VENT_TWOEXTERIOR);
    if(global_scase.nvent_transparent>0){
      if(show_transparent_vents == 1)glutAddMenuEntry(_("*Transparent"), MENU_VENT_TRANSPARENT);
      if(show_transparent_vents == 0)glutAddMenuEntry(_("Transparent"), MENU_VENT_TRANSPARENT);
    }
  }

/* --------------------------------terrain_obst_showmenu -------------------------- */

  CREATEMENU(terrain_obst_showmenu, GeometryMenu);
  if(terrain_showonly_top==1)glutAddMenuEntry(_("*Show only top surface"), 17 + TERRAIN_TOP);
  if(terrain_showonly_top==0)glutAddMenuEntry(_("Show only top surface"),  17 + TERRAIN_TOP);
  if(global_scase.visTerrainType==TERRAIN_SURFACE)glutAddMenuEntry(_("*3D surface"),17+TERRAIN_SURFACE);
  if(global_scase.visTerrainType!=TERRAIN_SURFACE)glutAddMenuEntry(_("3D surface"),17+TERRAIN_SURFACE);
  if(global_scase.terrain_texture_coll.terrain_textures!=NULL){ // &&terrain_texture->loaded==1
    if(global_scase.visTerrainType==TERRAIN_IMAGE)glutAddMenuEntry(_("*Image"),17+TERRAIN_IMAGE);
    if(global_scase.visTerrainType!=TERRAIN_IMAGE)glutAddMenuEntry(_("Image"),17+TERRAIN_IMAGE);
  }
  if(global_scase.visTerrainType==TERRAIN_HIDDEN)glutAddMenuEntry(_("*Hidden"),17+TERRAIN_HIDDEN);
  if(global_scase.visTerrainType!=TERRAIN_HIDDEN)glutAddMenuEntry(_("Hidden"),17+TERRAIN_HIDDEN);

  if(global_scase.objectscoll.nobject_defs>0){
    int multiprop;

    multiprop=0;
    for(i=0;i<global_scase.propcoll.npropinfo;i++){
      propdata *propi;

      propi = global_scase.propcoll.propinfo + i;
      if(propi->nsmokeview_ids>1)multiprop=1;
    }
    if(multiprop==1){
      for(i=0;i<global_scase.propcoll.npropinfo;i++){
        propdata *propi;

        propi = global_scase.propcoll.propinfo + i;
        CREATEMENU(propi->menu_id,PropMenu);
        if(propi->nsmokeview_ids>1){
          int jj;
          char menulabel[1024];

          for(jj=0;jj<propi->nsmokeview_ids;jj++){
            strcpy(menulabel,"");
            if(propi->smokeview_ids[jj]==propi->smokeview_id){
              strcat(menulabel,"*");
            }
            strcat(menulabel,propi->smokeview_ids[jj]);
            glutAddMenuEntry(menulabel,jj*global_scase.propcoll.npropinfo+i);
          }
        }
      }
      CREATEMENU(propmenu,PropMenu);
      for(i=0;i<global_scase.propcoll.npropinfo;i++){
        propdata *propi;

        propi = global_scase.propcoll.propinfo + i;
        if(propi->nsmokeview_ids>1){
          GLUTADDSUBMENU(propi->label,propi->menu_id);
        }
      }
    }

    CREATEMENU(spheresegmentmenu,ShowObjectsMenu);
    if(device_sphere_segments==6){
      glutAddMenuEntry("   *6",-6);
    }
    else{
      glutAddMenuEntry("   6",-6);
    }
    if(device_sphere_segments==12){
      glutAddMenuEntry("   *12",-12);
    }
    else{
      glutAddMenuEntry("   12",-12);
    }
    if(device_sphere_segments==24){
      glutAddMenuEntry("   *24",-24);
    }
    else{
      glutAddMenuEntry("   24",-24);
    }
    if(device_sphere_segments==48){
      glutAddMenuEntry("   *48",-48);
    }
    else{
      glutAddMenuEntry("   48",-48);
    }
  }

  if(ndevicetypes>0){
    CREATEMENU(devicetypemenu,DeviceTypeMenu);
    for(i=0;i<ndevicetypes;i++){
      char qlabel[64];

      strcpy(qlabel, "");
      if(devicetypes_index==i)strcat(qlabel,"*");
      strcat(qlabel, devicetypes[i]->quantity);
      glutAddMenuEntry(qlabel, i);
    }
  }
  if(global_scase.objectscoll.nobject_defs>0||global_scase.hrrptr!=NULL){
    CREATEMENU(showobjectsplotmenu,ShowObjectsMenu);
    if(ndevicetypes>0){
      GLUTADDSUBMENU(_("quantity"),devicetypemenu);
    }
    if(global_scase.objectscoll.nobject_defs>0){
      if(vis_device_plot==DEVICE_PLOT_SHOW_ALL)glutAddMenuEntry(      "*All devices",           OBJECT_PLOT_SHOW_ALL);
      if(vis_device_plot!=DEVICE_PLOT_SHOW_ALL)glutAddMenuEntry(      "All devices",            OBJECT_PLOT_SHOW_ALL);
      if(vis_device_plot==DEVICE_PLOT_SHOW_SELECTED)glutAddMenuEntry( "*Selected devices",      OBJECT_PLOT_SHOW_SELECTED);
      if(vis_device_plot!=DEVICE_PLOT_SHOW_SELECTED)glutAddMenuEntry( "Selected devices",       OBJECT_PLOT_SHOW_SELECTED);
    }
    if(global_scase.hrrptr!=NULL){
      if(vis_hrr_plot==1)glutAddMenuEntry("*HRRPUV", PLOT_HRRPUV);
      if(vis_hrr_plot==0)glutAddMenuEntry("HRRPUV", PLOT_HRRPUV);
    }

    if(showdevice_val==1)glutAddMenuEntry(_("*Show values"), OBJECT_VALUES);
    if(showdevice_val==0)glutAddMenuEntry(_("Show values"),  OBJECT_VALUES);
    glutAddMenuEntry(_("Settings..."), MENU_DEVICE_SETTINGS);
  }
  if(global_scase.objectscoll.nobject_defs>0){
    if(global_scase.devicecoll.ndeviceinfo > 0){
      int showall=1, hideall=1;

      CREATEMENU(showdevicesmenu, ShowDevicesMenu);
      for(i = 0; i < global_scase.devicecoll.ndeviceinfo; i++){
        devicedata *devicei;
        char devicelabel[256];

        devicei = global_scase.devicecoll.deviceinfo + i;
        strcpy(devicelabel, "");
        if(devicei->show==1){
          strcat(devicelabel,"*");
          hideall=0;
        }
        else{
          showall=0;
        }
        strcat(devicelabel, devicei->deviceID);
        glutAddMenuEntry(devicelabel, i);
      }
      glutAddMenuEntry("-",MENU_DUMMY);
      if(showall==1)glutAddMenuEntry("*Show all", MENU_DEVICES_SHOWALL);
      if(showall==0)glutAddMenuEntry("Show all", MENU_DEVICES_SHOWALL);
      if(hideall==1)glutAddMenuEntry("*Hide all", MENU_DEVICES_HIDEALL);
      if(hideall==0)glutAddMenuEntry("Hide all", MENU_DEVICES_HIDEALL);
    }

    CREATEMENU(showobjectsmenu,ShowObjectsMenu);
    for(i=0;i<global_scase.objectscoll.nobject_defs;i++){
      sv_object *obj_typei;

      obj_typei = global_scase.objectscoll.object_defs[i];
      if(obj_typei->used_by_device==1){
        char obj_menu[256];

        strcpy(obj_menu,"");
        if(obj_typei->visible==1){
          strcat(obj_menu,"*");
        }
        strcat(obj_menu,obj_typei->label);
        glutAddMenuEntry(obj_menu,i);
      }
    }
    if(global_scase.have_missing_objects == 1&&global_scase.isZoneFireModel==0){
      if(show_missing_objects==1)glutAddMenuEntry(_("*undefined"),OBJECT_MISSING);
      if(show_missing_objects == 0)glutAddMenuEntry(_("undefined"),OBJECT_MISSING);
    }
    if(global_scase.devicecoll.ndeviceinfo>0){
      glutAddMenuEntry("-",MENU_DUMMY);
      if(select_device==1){
        glutAddMenuEntry(_("*Select"),OBJECT_SELECT);
      }
      else{
        glutAddMenuEntry(_("Select"),OBJECT_SELECT);
      }
    }
    if(object_outlines==0)glutAddMenuEntry(_("Outline"),OBJECT_OUTLINE);
    if(object_outlines==1)glutAddMenuEntry(_("*Outline"),OBJECT_OUTLINE);
    if(global_scase.have_object_box==1){
      if(object_box==0)glutAddMenuEntry(_("Show XB"), OBJECT_BOX);
      if(object_box==1)glutAddMenuEntry(_("*Show XB"), OBJECT_BOX);
    }
    glutAddMenuEntry(_("Show all"),OBJECT_SHOWALL);
    glutAddMenuEntry(_("Hide all"),OBJECT_HIDEALL);
    if(show_device_orientation==1){
      glutAddMenuEntry(_("*Show orientation"),OBJECT_ORIENTATION);
    }
    else{
      glutAddMenuEntry(_("Show orientation"),OBJECT_ORIENTATION);
    }
    if(global_scase.have_beam == 1){
      if(showbeam_as_line == 1){
        glutAddMenuEntry(_("*Show beam as line"), OBJECT_SHOWBEAM);
      }
      else{
        glutAddMenuEntry(_("Show beam as line"), OBJECT_SHOWBEAM);
      }
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    if(global_scase.devicecoll.ndeviceinfo > 0){
      GLUTADDSUBMENU(_("Show/Hide devices"), showdevicesmenu);
    }
    GLUTADDSUBMENU(_("Segments"),spheresegmentmenu);
    if(global_scase.objectscoll.nobject_defs>0&&global_scase.devicecoll.ndeviceinfo>0){
      glutAddMenuEntry("-",MENU_DUMMY);
      GLUTADDSUBMENU(_("Plot data"),showobjectsplotmenu);
    }
    glutAddMenuEntry(_("Settings..."), MENU_DEVICE_SETTINGS);

  }

  /* --------------------------------hvac menu -------------------------- */

  if(global_scase.hvaccoll.nhvacinfo > 0){
    int show_all_networks=1;
    int hide_all_networks=1;

    if(global_scase.hvaccoll.nhvacconnectinfo > 0){
      int show_all_connections=1;
      int hide_all_connections=1;

      CREATEMENU(connectivitymenu, HVACConnectMenu);
      if(hvac_show_connections==1){
        glutAddMenuEntry("*connection view", MENU_HVAC_CONNECTION_VIEW);
      }
      else{
        glutAddMenuEntry("connection view",  MENU_HVAC_CONNECTION_VIEW);
      }
      glutAddMenuEntry("-", MENU_HVAC_SHOW_NODE_IGNORE);
      for(i=0;i<global_scase.hvaccoll.nhvacconnectinfo;i++){
        char label[32];
        hvacconnectdata *hi;

        hi = global_scase.hvaccoll.hvacconnectinfo + i;

        if(hi->display==1){
          sprintf(label, "*%i", hi->index);
          hide_all_connections = 0;
        }
        else{
          sprintf(label, "%i", hi->index);
          show_all_connections = 0;
        }
        glutAddMenuEntry(label,   i);
      }
      if(show_all_connections==1){
        glutAddMenuEntry("*Show all", MENU_HVAC_SHOWALL_CONNECTIONS);
      }
      else{
        glutAddMenuEntry("Show all", MENU_HVAC_SHOWALL_CONNECTIONS);
      }
      if(hide_all_connections==1){
        glutAddMenuEntry("*Hide all", MENU_HVAC_HIDEALL_CONNECTIONS);
      }
      else{
        glutAddMenuEntry("Hide all", MENU_HVAC_HIDEALL_CONNECTIONS);
      }
    }

    if(global_scase.hvaccoll.nhvaccomponents > 0){
      CREATEMENU(showcomponentmenu, HVACMenu);
      if(glui_hvac->show_component == 0){
        glutAddMenuEntry("*text",   MENU_HVAC_SHOW_COMPONENT_TEXT);
        glutAddMenuEntry("symbols", MENU_HVAC_SHOW_COMPONENT_SYMBOLS);
        glutAddMenuEntry("hide",    MENU_HVAC_SHOW_COMPONENT_HIDE);
      }
      else if(glui_hvac->show_component == 1){
        glutAddMenuEntry("text",     MENU_HVAC_SHOW_COMPONENT_TEXT);
        glutAddMenuEntry("*symbols", MENU_HVAC_SHOW_COMPONENT_SYMBOLS);
        glutAddMenuEntry("hide",     MENU_HVAC_SHOW_COMPONENT_HIDE);
      }
      else{
        glutAddMenuEntry("text",    MENU_HVAC_SHOW_COMPONENT_TEXT);
        glutAddMenuEntry("symbols", MENU_HVAC_SHOW_COMPONENT_SYMBOLS);
        glutAddMenuEntry("*hide",   MENU_HVAC_SHOW_COMPONENT_HIDE);
      }
    }

    if(global_scase.hvaccoll.nhvacfilters > 0){
      CREATEMENU(showfiltermenu, HVACMenu);
      if(glui_hvac->show_filters == 0){
        glutAddMenuEntry("*text", MENU_HVAC_SHOW_FILTER_TEXT);
        glutAddMenuEntry("symbols", MENU_HVAC_SHOW_FILTER_SYMBOLS);
        glutAddMenuEntry("hide", MENU_HVAC_SHOW_FILTER_HIDE);
      }
      else if(glui_hvac->show_filters == 1){
        glutAddMenuEntry("text", MENU_HVAC_SHOW_FILTER_TEXT);
        glutAddMenuEntry("*symbols", MENU_HVAC_SHOW_FILTER_SYMBOLS);
        glutAddMenuEntry("hide", MENU_HVAC_SHOW_FILTER_HIDE);
      }
      else{
        glutAddMenuEntry("text", MENU_HVAC_SHOW_FILTER_TEXT);
        glutAddMenuEntry("symbols", MENU_HVAC_SHOW_FILTER_SYMBOLS);
        glutAddMenuEntry("*hide", MENU_HVAC_SHOW_FILTER_HIDE);
      }
    }

    CREATEMENU(hvacnetworkmenu, HVACNetworkMenu);
    if(global_scase.hvaccoll.nhvacinfo>1){
      if(hvac_show_networks==1){
        glutAddMenuEntry("*network view", MENU_HVAC_NETWORK_VIEW);
      }
      else{
        glutAddMenuEntry("network view", MENU_HVAC_NETWORK_VIEW);
      }
      glutAddMenuEntry("-", MENU_HVAC_SHOW_NODE_IGNORE);
    }
    for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
      hvacdata *hvaci;
      char label[256];

      hvaci = global_scase.hvaccoll.hvacinfo + i;
      strcpy(label, "");
      if(hvaci->display == 1){
        strcat(label, "*");
        hide_all_networks = 0;
      }
      else{
        show_all_networks = 0;
      }
      strcat(label, hvaci->network_name);
      glutAddMenuEntry(label, i);
    }
    if(global_scase.hvaccoll.nhvacinfo > 1){
      if(show_all_networks == 1){
        glutAddMenuEntry("*show all", MENU_HVAC_SHOWALL_NETWORKS);
      }
      else{
        glutAddMenuEntry("show all", MENU_HVAC_SHOWALL_NETWORKS);
      }
      if(hide_all_networks == 1){
        glutAddMenuEntry("*hide all", MENU_HVAC_HIDEALL_NETWORKS);
      }
      else{
        glutAddMenuEntry("hide all", MENU_HVAC_HIDEALL_NETWORKS);
      }
    }
    int doit_ducts=0, doit_nodes=0;

    if(global_scase.hvaccoll.hvacductvalsinfo != NULL&&global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars>0)doit_ducts = 1;
    if(global_scase.hvaccoll.hvacnodevalsinfo != NULL&&global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars>0)doit_nodes = 1;
    if(doit_nodes==1){
      CREATEMENU(hvacnodevaluemenu, HVACNodeValueMenu);
      for(i = 0;i < global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars;i++){
        char label[255], *labeli;
        hvacvaldata *hi;

        hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + i;

        labeli = hi->label.longlabel;
        strcpy(label, "");
        if(global_scase.hvaccoll.hvacnodevar_index == i)strcat(label, "*");
        strcat(label, labeli);
        glutAddMenuEntry(label, i);
      }
    }
    if(doit_ducts==1){
      CREATEMENU(hvacductvaluemenu, HVACDuctValueMenu);
      for(i = 0;i < global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;i++){
        char label[255], *labeli;
        hvacvaldata *hi;

        hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + i;

        labeli = hi->label.longlabel;
        strcpy(label, "");
        if(global_scase.hvaccoll.hvacductvar_index == i)strcat(label, "*");
        strcat(label, labeli);
        glutAddMenuEntry(label, i);
      }
    }
    if(doit_ducts==1|| doit_nodes ==1){
      CREATEMENU(hvacvaluemenu, HVACMenu);

      if(doit_ducts==1)GLUTADDSUBMENU(_("Duct"), hvacductvaluemenu);
      if(doit_nodes==1)GLUTADDSUBMENU(_("Node"), hvacnodevaluemenu);
      glutAddMenuEntry("Hide all", MENU_HVAC_HIDE_ALL_VALUES);
    }

    CREATEMENU(hvacmenu, HVACMenu);
    if(doit_ducts==1||doit_nodes==1)GLUTADDSUBMENU(_("Values"), hvacvaluemenu);
    GLUTADDSUBMENU(_("Networks"), hvacnetworkmenu);
    if(global_scase.hvaccoll.nhvacconnectinfo > 0){
      GLUTADDSUBMENU(_("Connections"), connectivitymenu);
    }
    glutAddMenuEntry("Ducts", MENU_HVAC_SHOW_NODE_IGNORE);
    if(glui_hvac->show_duct_labels == 1){
      glutAddMenuEntry("   *IDs", MENU_HVAC_SHOW_DUCT_IDS);
    }
    else{
      glutAddMenuEntry("   IDs", MENU_HVAC_SHOW_DUCT_IDS);
    }
    if(global_scase.hvaccoll.nhvaccomponents > 0){
      GLUTADDSUBMENU(_(  "   Components"), showcomponentmenu);
    }
    glutAddMenuEntry("Nodes", MENU_HVAC_SHOW_NODE_IGNORE);
    if(glui_hvac->show_node_labels == 1){
      glutAddMenuEntry("   *IDs", MENU_HVAC_SHOW_NODE_IDS);
    }
    else{
      glutAddMenuEntry("   IDs", MENU_HVAC_SHOW_NODE_IDS);
    }
    if(global_scase.hvaccoll.nhvacfilters > 0){
      GLUTADDSUBMENU(_("   Filters"), showfiltermenu);
    }
    glutAddMenuEntry("-", MENU_HVAC_SHOW_NODE_IGNORE);
    if(hvac_metro_view==1){
      glutAddMenuEntry("*metro view", MENU_HVAC_METRO_VIEW);
    }
    else{
      glutAddMenuEntry("metro view", MENU_HVAC_METRO_VIEW);
    }
    if(hvac_cell_view == 1){
      glutAddMenuEntry("*cell view", MENU_HVAC_CELL_VIEW);
    }
    else{
      glutAddMenuEntry("cell view", MENU_HVAC_CELL_VIEW);
    }
    glutAddMenuEntry(_("Settings..."), MENU_HVAC_DIALOG_HVAC);
  }

  /* --------------------------------geometryoutlinemenu menu -------------------------- */

  CREATEMENU(geometryoutlinemenu,GeometryOutlineMenu);
  if(global_scase.isZoneFireModel==0){
    char skylabel[32] = "", label[128];
    char label2a[128] = "", label2b[128] = "mesh";
    char label3a[128] = "", label3b[128] = "scene";

    if(outline_mode == SCENE_OUTLINE_MESH)strcpy(label2a,   "*");
    if(outline_mode == SCENE_OUTLINE_SCENE)strcpy(label3a,  "*");
    if(skyboxinfo != NULL)strcpy(skylabel,"(FDS scene)");

    if(global_scase.meshescoll.nmeshes > 1){
      glutAddMenuEntry(ConcatLabels(label2a, label2b, skylabel, label), GEOM_OutlineB);
    }
    glutAddMenuEntry(ConcatLabels(label3a, label3b, skylabel, label), GEOM_OutlineC);
  }
  else{
    outline_mode = SCENE_OUTLINE_HIDDEN;
  }
  if(skyboxinfo!=NULL){
    if(visSkyboxoutline==1)glutAddMenuEntry(_("*Outline(skybox images)"), SKY_OUTLINE);
    if(visSkyboxoutline==0)glutAddMenuEntry(_("Outline(skybox images)"),  SKY_OUTLINE);
  }

  /* --------------------------------geometry menu -------------------------- */

  CREATEMENU(geometrymenu,GeometryMainMenu);
  if(global_scase.ntotal_blockages>0)GLUTADDSUBMENU(_("Obstacles"),blockagemenu);
  if(global_scase.ngeominfo>0){
    GLUTADDSUBMENU(_("Immersed"), immersedmenu);
  }
  if(GetNTotalVents()>0)GLUTADDSUBMENU(_("Surfaces"), ventmenu);
  if(global_scase.nrooms > 0){
    if(visCompartments == 1){
      glutAddMenuEntry(_("*Compartments"), GEOM_Compartments);
    }
    else{
      glutAddMenuEntry(_("Compartments"), GEOM_Compartments);
    }
  }
  if(global_scase.nzvents > 0){
    if(visVents == 1){
      glutAddMenuEntry(_("*Vents"), GEOM_Vents);
    }
    else{
      glutAddMenuEntry(_("Vents"), GEOM_Vents);
    }
  }
  GLUTADDSUBMENU(_("Grid"),gridslicemenu);
  if(global_scase.meshescoll.nmeshes > 1||skyboxinfo!=NULL){
    if(outline_mode != SCENE_OUTLINE_HIDDEN || (skyboxinfo != NULL && visSkyboxoutline == 1)){
      GLUTADDSUBMENU("*Outline",geometryoutlinemenu);
    }
    else{
      GLUTADDSUBMENU(_("Outline"),geometryoutlinemenu);
    }
  }
  else{
    if(outline_mode==SCENE_OUTLINE_HIDDEN)glutAddMenuEntry("Outline",  GEOM_OutlineC);
    if(outline_mode!=SCENE_OUTLINE_HIDDEN)glutAddMenuEntry("*Outline", GEOM_OutlineC);
  }
  if(hide_scene == 1)glutAddMenuEntry(_("*Show only bounding box when mouse is down"), GEOM_BOUNDING_BOX_MOUSE_DOWN);
  if(hide_scene != 1)glutAddMenuEntry(_("Show only bounding box when mouse is down"), GEOM_BOUNDING_BOX_MOUSE_DOWN);
  if(NCADGeom(&global_scase.cadgeomcoll) == 0){
    if(blocklocation == BLOCKlocation_grid){
      glutAddMenuEntry("Geometry positions(*as computed,as input)", BLOCKlocation_grid);
    }
    if(blocklocation == BLOCKlocation_exact){
      glutAddMenuEntry("Geometry positions(as computed,*as input)", BLOCKlocation_exact);
    }
  }
  else{
    if(blocklocation == BLOCKlocation_grid){
      glutAddMenuEntry("Locations:*actual",   BLOCKlocation_grid);
      glutAddMenuEntry("Locations:requested", BLOCKlocation_exact);
      glutAddMenuEntry("Locations:cad",       BLOCKlocation_cad);
    }
    if(blocklocation == BLOCKlocation_exact){
      glutAddMenuEntry("Locations:actual",     BLOCKlocation_grid);
      glutAddMenuEntry("Locations:*requested", BLOCKlocation_exact);
      glutAddMenuEntry("Locations:cad",        BLOCKlocation_cad);
    }
    if(blocklocation == BLOCKlocation_cad){
      glutAddMenuEntry("Locations:actual", BLOCKlocation_grid);
      glutAddMenuEntry("Locations:requested", BLOCKlocation_exact);
      glutAddMenuEntry("Locations:*cad", BLOCKlocation_cad);
    }
    {
      cadgeomdata *cd;
      cadlookdata *cdi;
      int showtexturemenu;

      showtexturemenu = 0;
      for(i = 0; i < NCADGeom(&global_scase.cadgeomcoll); i++){
        int j;

        cd = global_scase.cadgeomcoll.cadgeominfo + i;
        for(j = 0; j < cd->ncadlookinfo; j++){
          cdi = cd->cadlookinfo + j;
          if(cdi->textureinfo.loaded == 1){
            showtexturemenu = 1;
            break;
          }
        }
        if(showtexturemenu == 1)break;
      }
      if(showtexturemenu == 1){
        if(visCadTextures == 1){
          glutAddMenuEntry(_(" *Show CAD textures"), BLOCKtexture_cad);
        }
        else{
          glutAddMenuEntry(_(" Show CAD textures"), BLOCKtexture_cad);
        }
      }
    }
  }
  glutAddMenuEntry(_("Show all"), GEOM_ShowAll);
  glutAddMenuEntry(_("Hide all"), GEOM_HideAll);

  CREATEMENU(titlemenu, TitleMenu);
  if(vis_title_smv_version == 1)glutAddMenuEntry(_("*Smokeview version, build data"), MENU_TITLE_title_smv_version);
  if(vis_title_smv_version == 0)glutAddMenuEntry(_("Smokeview version, build date"), MENU_TITLE_title_smv_version);
  if(vis_title_gversion== 1)glutAddMenuEntry(_("*FDS, Smokeview version"), MENU_TITLE_gversion);
  if(vis_title_gversion== 0)glutAddMenuEntry(_("FDS, Smokeview version"), MENU_TITLE_gversion);
  if(global_scase.fds_title!=NULL){
    if(vis_title_fds == 1)glutAddMenuEntry(_("*Input file title"), MENU_TITLE_title_fds);
    if(vis_title_fds == 0)glutAddMenuEntry(_("Input file title"), MENU_TITLE_title_fds);
  }
  if(vis_title_CHID == 1)glutAddMenuEntry(_("*CHID"), MENU_TITLE_chid);
  if(vis_title_CHID == 0)glutAddMenuEntry(_("CHID"), MENU_TITLE_chid);
  glutAddMenuEntry("-", MENU_DUMMY);
  glutAddMenuEntry(_("Show All"), MENU_TITLE_show_all);
  glutAddMenuEntry(_("Hide All"), MENU_TITLE_hide_all);

/* --------------------------------label menu -------------------------- */

  CREATEMENU(labelmenu, LabelMenu);
  if(visColorbarVertical==1)glutAddMenuEntry(_("*Colorbar(vertical)"),MENU_LABEL_colorbar_vertical);
  if(visColorbarVertical==0)glutAddMenuEntry(_("Colorbar(vertical)"), MENU_LABEL_colorbar_vertical);
  if(visColorbarHorizontal == 1)glutAddMenuEntry(_("*Colorbar(horizontal)"), MENU_LABEL_colorbar_horizontal);
  if(visColorbarHorizontal == 0)glutAddMenuEntry(_("Colorbar(horizontal)"), MENU_LABEL_colorbar_horizontal);
  if(visTimebar==1)glutAddMenuEntry(_("*Time bar"),MENU_LABEL_timebar);
  if(visTimebar==0)glutAddMenuEntry(_("Time bar"),MENU_LABEL_timebar);
  if(visFramelabel == 1)glutAddMenuEntry(_("   *Frame"), MENU_LABEL_framelabel);
  if(visFramelabel == 0)glutAddMenuEntry(_("   Frame"), MENU_LABEL_framelabel);
  if(visTimelabel == 1)glutAddMenuEntry(_("   *Time"), MENU_LABEL_timelabel);
  if(visTimelabel == 0)glutAddMenuEntry(_("   Time"), MENU_LABEL_timelabel);
  if(visFrameTimelabel==1)glutAddMenuEntry(_("   *Frame/time label"), MENU_LABEL_frametimelabel);
  if(visFrameTimelabel==0)glutAddMenuEntry(_("   Frame/time label"), MENU_LABEL_frametimelabel);
  GLUTADDSUBMENU(_("Titles"),titlemenu);

  glutAddMenuEntry("-", MENU_DUMMY);

  if(visaxislabels == 1)glutAddMenuEntry(_("*Axis"), MENU_LABEL_axis);
  if(visaxislabels == 0)glutAddMenuEntry(_("Axis"), MENU_LABEL_axis);
  if(global_scase.have_northangle==1){
    if(vis_northangle==1)glutAddMenuEntry(_("*North"), MENU_LABEL_northangle);
    if(vis_northangle==0)glutAddMenuEntry(_("North"), MENU_LABEL_northangle);
  }
  if(global_scase.ntickinfo>0){
    if(visFDSticks == 0)glutAddMenuEntry(_("FDS generated ticks"), MENU_LABEL_fdsticks);
    if(visFDSticks == 1)glutAddMenuEntry(_("*FDS generated ticks"), MENU_LABEL_fdsticks);
  }

  if(visFramerate == 1)glutAddMenuEntry(_("*Frame rate"), MENU_LABEL_framerate);
  if(visFramerate == 0)glutAddMenuEntry(_("Frame rate"), MENU_LABEL_framerate);
  if(visgridloc == 1)glutAddMenuEntry(_("*Grid locations"), MENU_LABEL_grid);
  if(visgridloc == 0)glutAddMenuEntry(_("Grid locations"), MENU_LABEL_grid);

  if(global_scase.hrrptr != NULL){
    if(vis_hrr_label == 1)glutAddMenuEntry(_("*HRR"), MENU_LABEL_hrr);
    if(vis_hrr_label == 0)glutAddMenuEntry(_("HRR"), MENU_LABEL_hrr);
  }
#ifdef pp_memstatus
  if(visAvailmemory == 1)glutAddMenuEntry(_("*Memory load"), MENU_LABEL_memload);
  if(visAvailmemory == 0)glutAddMenuEntry(_("Memory load"), MENU_LABEL_memload);
#endif
#ifdef pp_MEMDEBUG
  if(visUsagememory == 1)glutAddMenuEntry(_("*Memory usage"), MENU_LABEL_memusage);
  if(visUsagememory == 0)glutAddMenuEntry(_("Memory usage"), MENU_LABEL_memusage);
#endif
  if(visMeshlabel == 1)glutAddMenuEntry(_("*Mesh"), MENU_LABEL_meshlabel);
  if(visMeshlabel == 0)glutAddMenuEntry(_("Mesh"), MENU_LABEL_meshlabel);
  if(vis_slice_average == 1)glutAddMenuEntry(_("*Slice average"), MENU_LABEL_sliceaverage);
  if(vis_slice_average == 0)glutAddMenuEntry(_("Slice average"), MENU_LABEL_sliceaverage);
  if(LabelGetNUserLabels(&global_scase.labelscoll) > 0){
    if(visLabels == 1)glutAddMenuEntry(_("*Text labels"), MENU_LABEL_textlabels);
    if(visLabels == 0)glutAddMenuEntry(_("Text labels"), MENU_LABEL_textlabels);
  }

  if(visUSERticks == 1)glutAddMenuEntry(_("*User settable ticks"), MENU_LABEL_userticks);
  if(visUSERticks == 0)glutAddMenuEntry(_("User settable ticks"), MENU_LABEL_userticks);

  glutAddMenuEntry("-", MENU_DUMMY);
  glutAddMenuEntry(_("Show all"), MENU_LABEL_ShowAll);
  glutAddMenuEntry(_("Hide all"), MENU_LABEL_HideAll);
  glutAddMenuEntry(_("Settings..."), MENU_LABEL_SETTINGS);

  /* --------------------------------translate type menu -------------------------- */

  CREATEMENU(translatetypemenu, TranslateTypeMenu);
  if(translation_type==TRANSLATE_XY_option){
    glutAddMenuEntry("*translate left/right and front/back", TRANSLATE_XY_option);
    glutAddMenuEntry("translate only front/back", TRANSLATE_Y_option);
    glutAddMenuEntry("translate only left/right", TRANSLATE_X_option);
  }
  if(translation_type==TRANSLATE_Y_option){
    glutAddMenuEntry("translate left/right and front/back", TRANSLATE_XY_option);
    glutAddMenuEntry("*translate only front/back", TRANSLATE_Y_option);
    glutAddMenuEntry("translate only left/right", TRANSLATE_X_option);
  }
  if(translation_type==TRANSLATE_X_option){
    glutAddMenuEntry("translate left/right and front/back", TRANSLATE_XY_option);
    glutAddMenuEntry("translate only front/back", TRANSLATE_Y_option);
    glutAddMenuEntry("*translate only left/right", TRANSLATE_X_option);
  }

  /* --------------------------------rotate type menu -------------------------- */

  CREATEMENU(rotatetypemenu,RotateTypeMenu);
  glutAddMenuEntry(_("Scene centered:"),MENU_DUMMY);
  switch(rotation_type){
  case EYE_CENTERED:
    glutAddMenuEntry(_("  2 axis"),ROTATION_2AXIS);
    glutAddMenuEntry(_("  Level (1 axis)"),ROTATION_1AXIS);
    glutAddMenuEntry(_("  3 axis"),ROTATION_3AXIS);
    glutAddMenuEntry(_("*Eye centered"),EYE_CENTERED);
    break;
  case ROTATION_2AXIS:
    glutAddMenuEntry(_("  *2 axis"),ROTATION_2AXIS);
    glutAddMenuEntry(_("  Level (1 axis)"),ROTATION_1AXIS);
    glutAddMenuEntry(_("  3 axis"),ROTATION_3AXIS);
    glutAddMenuEntry(_("Eye centered"),EYE_CENTERED);
    break;
  case ROTATION_1AXIS:
    glutAddMenuEntry(_("  2 axis"),ROTATION_2AXIS);
    glutAddMenuEntry(_("  *Level (1 axis)"),ROTATION_1AXIS);
    glutAddMenuEntry(_("  3 axis"),ROTATION_3AXIS);
    glutAddMenuEntry(_("Eye centered"),EYE_CENTERED);
    break;
  case ROTATION_3AXIS:
    glutAddMenuEntry(_("  2 axis"),ROTATION_2AXIS);
    glutAddMenuEntry(_("  Level (1 axis)"),ROTATION_1AXIS);
    glutAddMenuEntry(_("  *3 axis"),ROTATION_3AXIS);
    glutAddMenuEntry(_("Eye centered"),EYE_CENTERED);
    break;
  default:
    assert(FFALSE);
    break;
  }
  glutAddMenuEntry(_("Direction vectors:"), MENU_DUMMY);
  if(showgravity_vector==1)glutAddMenuEntry(_("  *show gravity, axis vectors"), MENU_MOTION_SHOW_VECTORS);
  if(showgravity_vector==0)glutAddMenuEntry(_("  show gravity, axis vectors"), MENU_MOTION_SHOW_VECTORS);
  glutAddMenuEntry(_("  gravity vector down"), MENU_MOTION_GRAVITY_VECTOR);
  glutAddMenuEntry(_("  z vector up"), MENU_MOTION_Z_VECTOR);
  glutAddMenuEntry(_("Settings..."), MENU_MOTION_SETTINGS);

/* --------------------------------zone show menu -------------------------- */

  if(global_scase.nzoneinfo>0&&(ReadZoneFile==1||global_scase.nzvents>0)){
    CREATEMENU(zoneshowmenu,ZoneShowMenu);
    glutAddMenuEntry(_("Layers"),MENU_DUMMY);
    glutAddMenuEntry(_("   Representation:"),MENU_DUMMY);
    if(visZone==1&&zonecolortype==ZONETEMP_COLOR){
      if(show_zonelower==1){
        glutAddMenuEntry(_("      Temperature(upper)"), MENU_ZONE_2DTEMP);
        glutAddMenuEntry(_("      *Temperature(upper/lower)"), MENU_ZONE_2DTEMP2);
      }
      else{
        glutAddMenuEntry(_("      *Temperature(upper)"), MENU_ZONE_2DTEMP);
        glutAddMenuEntry(_("      Temperature(upper/lower)"), MENU_ZONE_2DTEMP2);
      }
      glutAddMenuEntry(_("      Hazard"), MENU_ZONE_2DHAZARD);
      glutAddMenuEntry(_("      Smoke"), MENU_ZONE_3DSMOKE);
    }
    else if(visZone==1&&zonecolortype==ZONEHAZARD_COLOR){
      glutAddMenuEntry(_("      Temperature(upper)"), MENU_ZONE_2DTEMP);
      glutAddMenuEntry(_("      Temperature(upper/lower)"), MENU_ZONE_2DTEMP2);
      glutAddMenuEntry(_("      *Hazard"), MENU_ZONE_2DHAZARD);
      glutAddMenuEntry(_("      Smoke"), MENU_ZONE_3DSMOKE);
    }
    else{
      glutAddMenuEntry(_("      Temperature(upper)"), MENU_ZONE_2DTEMP);
      glutAddMenuEntry(_("      Temperature(upper/lower)"), MENU_ZONE_2DTEMP2);
      glutAddMenuEntry(_("      Hazard"), MENU_ZONE_2DHAZARD);
      glutAddMenuEntry(_("      *Smoke"), MENU_ZONE_3DSMOKE);
    }
    glutAddMenuEntry(_("   Orientation:"), MENU_DUMMY);
    if(visZone==1){
      if(zonecolortype==ZONESMOKE_COLOR){
        glutAddMenuEntry(_("      Vertical(X plane)"),   MENU_ZONE_XPLANE);
        glutAddMenuEntry(_("      Vertical(Y plane)"),   MENU_ZONE_YPLANE);
        glutAddMenuEntry(_("      Horizontal(Z plane)"), MENU_ZONE_ZPLANE);
      }
      else{
        if(visZonePlane == ZONE_XPLANE){
          glutAddMenuEntry(_("      *Vertical(X plane)"),  MENU_ZONE_XPLANE);
          glutAddMenuEntry(_("      Vertical(Y plane)"),   MENU_ZONE_YPLANE);
          glutAddMenuEntry(_("      Horizontal(Z plane)"), MENU_ZONE_ZPLANE);
        }
        if(visZonePlane == ZONE_YPLANE){
          glutAddMenuEntry(_("      Vertical(X plane)"),   MENU_ZONE_XPLANE);
          glutAddMenuEntry(_("      *Vertical(Y plane)"),  MENU_ZONE_YPLANE);
          glutAddMenuEntry(_("      Horizontal(Z plane)"), MENU_ZONE_ZPLANE);
        }
        if(visZonePlane == ZONE_ZPLANE){
          glutAddMenuEntry(_("      Vertical(X plane)"),    MENU_ZONE_XPLANE);
          glutAddMenuEntry(_("      Vertical(Y plane)"),    MENU_ZONE_YPLANE);
          glutAddMenuEntry(_("      *Horizontal(Z plane)"), MENU_ZONE_ZPLANE);
        }
      }
    }
    else{
      glutAddMenuEntry(_("      Vertical(X plane)"),   MENU_ZONE_XPLANE);
      glutAddMenuEntry(_("      Vertical(Y plane)"),   MENU_ZONE_YPLANE);
      glutAddMenuEntry(_("      Horizontal(Z plane)"), MENU_ZONE_ZPLANE);
    }
    if(visZone==0){
      glutAddMenuEntry(_("   *Hide"), MENU_ZONE_LAYERHIDE);
    }
    else{
      glutAddMenuEntry(_("   Hide"), MENU_ZONE_LAYERHIDE);
    }
    if(have_wall_data==1){
      if(vis_wall_data==1){
        glutAddMenuEntry(_("*Walls"), MENU_ZONE_WALLS);
      }
      else{
        glutAddMenuEntry(_("Walls"), MENU_ZONE_WALLS);
      }
    }
    if(have_target_data==1){
      if(vis_target_data==1){
        glutAddMenuEntry(_("*Target"), MENU_ZONE_TARGETS);
      }
      else{
        glutAddMenuEntry(_("Targets"), MENU_ZONE_TARGETS);
      }
    }
    if(global_scase.nzvents>0){
      if(visVentFlow==1){
        glutAddMenuEntry(_("*Vent flow"), MENU_ZONE_VENTS);
      }
      else{
        glutAddMenuEntry(_("Vent flow"), MENU_ZONE_VENTS);
      }
      if(global_scase.nzhvents>0){
        if(visVentHFlow == 1){
          glutAddMenuEntry(_("   *Horizontal"), MENU_ZONE_HVENTS);
        }
        else{
          glutAddMenuEntry(_("   Horizontal"), MENU_ZONE_HVENTS);
        }
        if(have_ventslab_flow==0){
          if(visventprofile==1){
            glutAddMenuEntry(_("      *velocity profile"), MENU_ZONE_VENT_PROFILE);
          }
          else{
            glutAddMenuEntry(_("      velocity profile"), MENU_ZONE_VENT_PROFILE);
          }
        }
        else{
          if(visventslab==1){
            glutAddMenuEntry(_("      *mass slab"), MENU_ZONE_VENT_SLAB);
          }
          else{
            glutAddMenuEntry(_("      mass slab"), MENU_ZONE_VENT_SLAB);
          }
          if(visventprofile==1){
            glutAddMenuEntry(_("      *velocity profile"), MENU_ZONE_VENT_PROFILE);
          }
          else{
            glutAddMenuEntry(_("      velocity profile"), MENU_ZONE_VENT_PROFILE);
          }
        }
      }
      if(global_scase.nzvvents>0){
        if(visVentVFlow==1){
          glutAddMenuEntry(_("   *Vertical"), MENU_ZONE_VVENTS);
        }
        else{
          glutAddMenuEntry(_("   Vertical"), MENU_ZONE_VVENTS);
        }
      }
      if(global_scase.nzmvents>0){
        if(visVentMFlow==1){
          glutAddMenuEntry(_("   *Mechanical"), MENU_ZONE_MVENTS);
        }
        else{
          glutAddMenuEntry(_("   Mechanical"), MENU_ZONE_MVENTS);
        }
      }
    }
    if(global_scase.nfires>0){
      if(viszonefire==1){
        glutAddMenuEntry(_("*Fires"), MENU_ZONE_FIRES);
      }
      else{
        glutAddMenuEntry(_("Fires"), MENU_ZONE_FIRES);
      }
    }
  }

  /* --------------------------------particle class show menu -------------------------- */

  if(global_scase.npartclassinfo>0){
    int ntypes;

    CREATEMENU(particlestreakshowmenu,ParticleStreakShowMenu);
    {
      int iii;
      char streaklabel[1024];

      streak_rvalue[nstreak_rvalue-1]=tmax_part;
      for(iii=0;iii<nstreak_rvalue;iii++){
        if(iii==streak_index){
          sprintf(streaklabel,"*%f",streak_rvalue[iii]);
        }
        else{
          sprintf(streaklabel,"%f",streak_rvalue[iii]);
        }
        TrimZeros(streaklabel);
        strcat(streaklabel," s");
        glutAddMenuEntry(streaklabel,iii);
      }
    }
    glutAddMenuEntry("-",MENU_DUMMY2);
    if(showstreakhead==1){
      glutAddMenuEntry(_("*Particle head"),MENU_STREAK_HEAD);
    }
    else{
      glutAddMenuEntry(_("Particle head"),MENU_STREAK_HEAD);
    }
    glutAddMenuEntry(_("Hide"),MENU_STREAK_HIDE);

// allocate memory for particle property sub-menus

    if(npart5prop*global_scase.npartclassinfo>0){
      NewMemory((void **)&particlepropshowsubmenu,npart5prop*global_scase.npartclassinfo*sizeof(int));
    }

      ntypes=0;
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        int j;

        propi = part5propinfo + i;
        if(propi->display==0)continue;
        for(j=0;j<global_scase.npartclassinfo;j++){
          partclassdata *partclassj;
          char menulabel[1024];

          if(propi->class_present[j]==0)continue;
          partclassj = global_scase.partclassinfo + j;
          CREATEMENU(particlepropshowsubmenu[ntypes],ParticlePropShowMenu);
          ntypes++;
          if(propi->class_vis[j]==1){
            strcpy(menulabel,"  *");
          }
          else{
            strcpy(menulabel,"  ");
          }
          strcat(menulabel,partclassj->name);
          if(partclassj->col_diameter>=0||partclassj->col_length>=0||partclassj->device_name!=NULL||
             (partclassj->prop!=NULL&&partclassj->prop->smokeview_id!=NULL)||
             (partclassj->col_u_vel>=0&&partclassj->col_v_vel>=0&&partclassj->col_w_vel>=0)
           ){
            if(propi->class_vis[j]==1){
              strcpy(menulabel,_("using:"));
            }
            else{
              strcpy(menulabel,_("using:"));
            }
            glutAddMenuEntry(menulabel,-10-5*j);
            if(partclassj->vis_type==PART_POINTS){
              glutAddMenuEntry(_("    *points"),-10-5*j-PART_POINTS);
            }
            else{
              glutAddMenuEntry(_("    points"),-10-5*j-PART_POINTS);
            }
            if(partclassj->col_diameter>=0||partclassj->device_name!=NULL){
              if(partclassj->vis_type==PART_SPHERES){
                glutAddMenuEntry(_("    *spheres"),-10-5*j-PART_SPHERES);
              }
              else{
                glutAddMenuEntry(_("    spheres"),-10-5*j-PART_SPHERES);
              }
            }
            if(partclassj->col_length>=0||partclassj->device_name!=NULL||
              (partclassj->col_u_vel>=0&&partclassj->col_v_vel>=0&&partclassj->col_w_vel>=0)){
              if(partclassj->vis_type==PART_LINES){
                glutAddMenuEntry(_("    *Lines"),-10-5*j-PART_LINES);
              }
              else{
                glutAddMenuEntry(_("    Lines"),-10-5*j-PART_LINES);
              }
            }
            if(
              (partclassj->smv_device!=NULL&&partclassj->device_name!=NULL)||
              (partclassj->prop!=NULL&&partclassj->prop->smokeview_id!=NULL)
             ){
              if(partclassj->device_name!=NULL){
                strcpy(menulabel,"    ");
                if(partclassj->vis_type==PART_SMV_DEVICE){
                  strcat(menulabel,"*");
                }
                strcat(menulabel,partclassj->device_name);
                glutAddMenuEntry(menulabel,-10-5*j-PART_SMV_DEVICE);
              }
              else if(partclassj->prop!=NULL&&partclassj->prop->smokeview_id!=NULL){
                int iii;
                propdata *propclass;

              // value = iobject*npropinfo + iprop
                propclass=partclassj->prop;
                for(iii=0;iii<propclass->nsmokeview_ids;iii++){
                  int propvalue, showvalue, menuvalue;

                  propvalue = iii*global_scase.propcoll.npropinfo + (propclass-global_scase.propcoll.propinfo);
                  showvalue = -10-5*j-PART_SMV_DEVICE;
                  menuvalue = (-1-propvalue)*10000 + showvalue;
                  // propvalue = (-menuvalue)/10000-1;
                  // showvalue = -((-menuvalue)%10000)
                  strcpy(menulabel,"    ");
                  if(partclassj->vis_type==PART_SMV_DEVICE&&propclass->smokeview_ids[iii]==propclass->smokeview_id){
                    strcat(menulabel,"*");
                  }
                  strcat(menulabel,propclass->smokeview_ids[iii]);
                  glutAddMenuEntry(menulabel,menuvalue);
                }
              }
            }
          }
          else{
            glutAddMenuEntry(menulabel,-10-5*j);
          }
        }
      }

    CREATEMENU(particlepropshowmenu,ParticlePropShowMenu);
    if(npart5prop>=0){
      glutAddMenuEntry(_("Color with:"),MENU_PROP_DUMMY);
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        char menulabel[1024];

        propi = part5propinfo + i;
        if(propi->display==1){
          strcpy(menulabel,"  *");
        }
        else{
          strcpy(menulabel,"  ");
        }
        strcat(menulabel,propi->label->longlabel);
        glutAddMenuEntry(menulabel,i);
      }

      if(part5show==0)glutAddMenuEntry(_("  *Hide"), MENU_PROP_HIDEPART);
      if(part5show==1)glutAddMenuEntry(_("  Hide"), MENU_PROP_HIDEPART);
      glutAddMenuEntry("-",MENU_PROP_DUMMY);

      glutAddMenuEntry(_("Draw"),MENU_PROP_DUMMY);
      ntypes=0;
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        int j;

        propi = part5propinfo + i;
        if(propi->display==0)continue;
        for(j=0;j<global_scase.npartclassinfo;j++){
          partclassdata *partclassj;
          char menulabel[1024];

          if(propi->class_present[j]==0)continue;
          partclassj = global_scase.partclassinfo + j;
          ntypes++;
          if(propi->class_vis[j]==1){
            strcpy(menulabel,"  *");
          }
          else{
            strcpy(menulabel,"  ");
          }
          strcat(menulabel,partclassj->name);
          GLUTADDSUBMENU(menulabel,particlepropshowsubmenu[ntypes-1]);
        }
      }

      if(ntypes>1){
        glutAddMenuEntry(_("  Show all"),MENU_PROP_SHOWALL);
        glutAddMenuEntry(_("  Hide all"),MENU_PROP_HIDEALL);
      }
      glutAddMenuEntry("-",MENU_PROP_DUMMY);
      if(streak5show==1){
        GLUTADDSUBMENU(_("*Streaks"),particlestreakshowmenu);
      }
      else{
        GLUTADDSUBMENU(_("Streaks"),particlestreakshowmenu);
      }
      glutAddMenuEntry("-",MENU_PROP_DUMMY);
      if(show_tracers_always==0)glutAddMenuEntry(_("Show tracers always"),MENU_PROP_TRACERS);
      if(show_tracers_always==1)glutAddMenuEntry(_("*Show tracers always"), MENU_PROP_TRACERS);
    }
  }

/* --------------------------------particle show menu -------------------------- */

  if(global_scase.npartinfo>0){
    int ii;
    int showall;

    CREATEMENU(particleshowmenu,ParticleShowMenu);
    for(ii=0;ii<global_scase.npartinfo;ii++){
      partdata *parti;
      char menulabel[1024];

      i = partorderindex[ii];
      parti = global_scase.partinfo + i;
      if(parti->loaded==0)continue;
      STRCPY(menulabel,"");
      if(parti->display==1)STRCAT(menulabel,"*");
      STRCAT(menulabel,parti->menulabel);
      glutAddMenuEntry(menulabel,-1-i);
    }

    glutAddMenuEntry("-",MENU_DUMMY);
    if(plotstate==DYNAMIC_PLOTS&&visSmokePart!=0){
      if(visSmokePart==2)glutAddMenuEntry(_("*Particles"),MENU_PARTSHOW_PARTICLES);
      if(visSmokePart==1)glutAddMenuEntry(_("#Particles"), MENU_PARTSHOW_PARTICLES);
    }
    else{
      glutAddMenuEntry(_("Particles"), MENU_PARTSHOW_PARTICLES);
    }
    if(havesprinkpart==1){
      if(plotstate==DYNAMIC_PLOTS&&visSprinkPart==1){
        glutAddMenuEntry(_("*Droplets"), MENU_PARTSHOW_DROPLETS);
      }
      else{
        glutAddMenuEntry(_("Droplets"), MENU_PARTSHOW_DROPLETS);
      }
    }
    showall=0;
    if(plotstate==DYNAMIC_PLOTS){
      if(visSprinkPart==1&&visSmokePart!=0)showall=1;
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    if(showall==1){
      glutAddMenuEntry(_("*Show all"), MENU_PARTSHOW_SHOWALL);
    }
    else{
      glutAddMenuEntry(_("Show all"), MENU_PARTSHOW_SHOWALL);
    }
    if(plotstate==DYNAMIC_PLOTS){
      int hideall;

      hideall=1;
      if(visSmokePart!=0)hideall=0;
      if(havesprinkpart==1&&visSprinkPart==1)hideall=0;
      if(hideall==1){
        glutAddMenuEntry(_("*Hide all"), MENU_PARTSHOW_HIDEALL);
      }
      else{
        glutAddMenuEntry(_("Hide all"), MENU_PARTSHOW_HIDEALL);
      }
    }
  }

/* -------------------------------- colorbarmenu -------------------------- */

  if(nsmoke3dloaded>0||nvolrenderinfo>0){
    MakeColorbarMenu(&smokecolorbarmenu,
                     &smokecolorbars_submenu1, &smokecolorbars_submenu2, &smokecolorbars_submenu3,
                     &smokecolorbars_submenu4, &smokecolorbars_submenu5, &smokecolorbars_submenu6,
                     &smokecolorbars_submenu7,
                     SmokeColorbarMenu);

  }


  /* --------------------------------smoke3d showmenu -------------------------- */

  if(nsmoke3dloaded>0){
    {
      if(nsmoke3dloaded>0){
        CREATEMENU(smoke3dshowmenu, Smoke3DShowMenu);
        if(show_3dsmoke==1)glutAddMenuEntry(_("*Show"), TOGGLE_SMOKE3D);
        if(show_3dsmoke==0)glutAddMenuEntry(_("Show"), TOGGLE_SMOKE3D);
        GLUTADDSUBMENU(_("Smoke colorbar"),smokecolorbarmenu);
      }
    }
  }

/* --------------------------------iso level menu -------------------------- */

  if(loaded_isomesh!=NULL&&global_scase.nisoinfo>0&&ReadIsoFile==1){
    CREATEMENU(isolevelmenu,IsoShowMenu);
    if(loaded_isomesh->nisolevels>0&&loaded_isomesh->showlevels!=NULL){
      int showflag,hideflag;
      showflag=1;
      hideflag=1;
      for(i=0;i<loaded_isomesh->nisolevels;i++){
        char levellabel[1024];

        if(loaded_isomesh->showlevels[i]==1){
          sprintf(levellabel,"*%f ",loaded_isomesh->isolevels[i]);
          hideflag=0;
        }
        else{
          showflag=0;
          sprintf(levellabel,"%f ",loaded_isomesh->isolevels[i]);
        }
        if(loaded_isomesh->isofilenum!=-1){
          STRCAT(levellabel,global_scase.isoinfo[loaded_isomesh->isofilenum].surface_label.unit);
        }
        else{
          STRCAT(levellabel,"");
        }
        glutAddMenuEntry(levellabel,100+i);

      }
      if(showflag == 1)glutAddMenuEntry(_("*Show all levels"), MENU_ISOSHOW_SHOWALL);
      if(showflag == 0)glutAddMenuEntry(_("Show all levels"), MENU_ISOSHOW_SHOWALL);
      if(hideflag == 1)glutAddMenuEntry(_("*Hide all levels"), MENU_ISOSHOW_HIDEALL);
      if(hideflag == 0)glutAddMenuEntry(_("Hide all levels"), MENU_ISOSHOW_HIDEALL);
      glutAddMenuEntry("---",93);
      if(transparent_state == ALL_SOLID)glutAddMenuEntry(_("*All levels solid"), MENU_ISOSHOW_ALLSOLID);
      if(transparent_state != ALL_SOLID)glutAddMenuEntry(_("All levels solid"), MENU_ISOSHOW_ALLSOLID);
      if(transparent_state == ALL_TRANSPARENT)glutAddMenuEntry(_("*All levels transparent"), MENU_ISOSHOW_ALLTRANSPARENT);
      if(transparent_state != ALL_TRANSPARENT)glutAddMenuEntry(_("All levels transparent"), MENU_ISOSHOW_ALLTRANSPARENT);
      if(transparent_state == MIN_SOLID)glutAddMenuEntry(_("*Minimum level solid"), MENU_ISOSHOW_MINSOLID);
      if(transparent_state != MIN_SOLID)glutAddMenuEntry(_("Minimum level solid"), MENU_ISOSHOW_MINSOLID);
      if(transparent_state == MAX_SOLID)glutAddMenuEntry(_("*Maximum level solid"), MENU_ISOSHOW_MAXSOLID);
      if(transparent_state != MAX_SOLID)glutAddMenuEntry(_("Maximum level solid"), MENU_ISOSHOW_MAXSOLID);
    }
    else{
      glutAddMenuEntry(_("Show"), MENU_ISOSHOW_SHOWALL);
      if(visAIso == 0)glutAddMenuEntry(_("*Hide"), MENU_ISOSHOW_HIDEALL);
      if(visAIso != 0)glutAddMenuEntry(_("Hide"), MENU_ISOSHOW_HIDEALL);
    }

/* --------------------------------iso show menu -------------------------- */

    if(global_scase.nisoinfo>0&&ReadIsoFile==1){
      meshdata *hmesh;
      isodata *iso2;

      iso2 = NULL;
      CREATEMENU(isoshowmenu, IsoShowMenu);
      if(iso2!=NULL){
        char menulabel[1024];

        STRCPY(menulabel,"");
        if(show_isofiles==1)STRCAT(menulabel,"*");
        STRCAT(menulabel,iso2->surface_label.longlabel);
        glutAddMenuEntry(menulabel,TOGGLE_ISO);
        glutAddMenuEntry("-", MENU_DUMMY);
      }
      if((visAIso & 1) == 1)glutAddMenuEntry(_("*Solid"), MENU_ISOSHOW_SOLID);
      if((visAIso & 1) != 1)glutAddMenuEntry(_("Solid"), MENU_ISOSHOW_SOLID);
      if((visAIso & 2) == 2)glutAddMenuEntry(_("*Outline"), MENU_ISOSHOW_OUTLINE);
      if((visAIso & 2) != 2)glutAddMenuEntry(_("Outline"), MENU_ISOSHOW_OUTLINE);
      if((visAIso & 4) == 4)glutAddMenuEntry(_("*Points"), MENU_ISOSHOW_POINTS);
      if((visAIso & 4) != 4)glutAddMenuEntry(_("Points"), MENU_ISOSHOW_POINTS);
      hmesh=global_scase.meshescoll.meshinfo+highlight_mesh;
      if(hmesh->isofilenum!=-1){
        char levellabel[1024];

        STRCPY(levellabel,global_scase.isoinfo[hmesh->isofilenum].surface_label.shortlabel);
        STRCAT(levellabel," ");
        STRCAT(levellabel,_("Levels"));
        GLUTADDSUBMENU(levellabel,isolevelmenu);
      }
      if(global_scase.niso_compressed==0){
        if(smooth_iso_normal == 1)glutAddMenuEntry(_("*Smooth"), MENU_ISOSHOW_SMOOTH);
        if(smooth_iso_normal == 0)glutAddMenuEntry(_("Smooth"), MENU_ISOSHOW_SMOOTH);
      }
      if(show_iso_normal == 1)glutAddMenuEntry(_("*Show normals"), MENU_ISOSHOW_NORMALS);
      if(show_iso_normal == 0)glutAddMenuEntry(_("Show normals"), MENU_ISOSHOW_NORMALS);
      glutAddMenuEntry(_("Output bounds"), MENU_ISOSHOW_OUTPUT);
    }
  }

  /* -------------------------------- colorbarmenu -------------------------- */

  CREATEMENU(colorbardigitmenu, ColorbarDigitMenu);
  for(i = COLORBAR_NDECIMALS_MIN; i<=COLORBAR_NDECIMALS_MAX; i++){
    char label[10];

    if(i==ncolorlabel_digits){
      sprintf(label, "%s%i", "*", i);
    }
    else{
      sprintf(label, "%i",  i);
    }
    glutAddMenuEntry(label, i);
  }

  /* -------------------------------- colorbarmenu -------------------------- */

  CREATEMENU(colorbarshademenu,ColorbarMenu);
  if(visColorbarVertical==1)glutAddMenuEntry(_("*Vertical"),COLORBAR_VERTICAL);
  if(visColorbarVertical==0)glutAddMenuEntry(_("Vertical"),COLORBAR_VERTICAL);
  if(visColorbarHorizontal==1)glutAddMenuEntry(_("*Horizontal"),COLORBAR_HORIZONTAL);
  if(visColorbarHorizontal==0)glutAddMenuEntry(_("Horizontal"),COLORBAR_HORIZONTAL);
  glutAddMenuEntry("-", MENU_DUMMY);
  if(contour_type==SHADED_CONTOURS){
    glutAddMenuEntry(_("*Continuous"),COLORBAR_CONTINUOUS);
    glutAddMenuEntry(_("Stepped"),COLORBAR_STEPPED);
    glutAddMenuEntry(_("Lines"),COLORBAR_LINES);
  }
  else if(contour_type==STEPPED_CONTOURS){
    glutAddMenuEntry(_("Continuous"),COLORBAR_CONTINUOUS);
    glutAddMenuEntry(_("*Stepped"),COLORBAR_STEPPED);
    glutAddMenuEntry(_("Lines"),COLORBAR_LINES);
  }else if(contour_type==LINE_CONTOURS){
    glutAddMenuEntry(_("Continuous"),COLORBAR_CONTINUOUS);
    glutAddMenuEntry(_("Stepped"),COLORBAR_STEPPED);
    glutAddMenuEntry(_("*Lines"),COLORBAR_LINES);
  }
  glutAddMenuEntry("-",MENU_DUMMY);
  if(show_extreme_maxdata == 1){
    glutAddMenuEntry(_("  *Highlight data above specified max"), COLORBAR_HIGHLIGHT_ABOVE);
  }
  else{
    glutAddMenuEntry(_("  Highlight data above specified max"), COLORBAR_HIGHLIGHT_ABOVE);
  }
  if(show_extreme_mindata == 1){
    glutAddMenuEntry(_("  *Highlight data below specified min"), COLORBAR_HIGHLIGHT_BELOW);
  }
  else{
    glutAddMenuEntry(_("  Highlight data below specified min"), COLORBAR_HIGHLIGHT_BELOW);
  }
  if(colorbar_flip == 1){
    glutAddMenuEntry(_("  *Flip"), COLORBAR_FLIP);
  }
  else{
    glutAddMenuEntry(_("  Flip"), COLORBAR_FLIP);
  }
  if(colorbar_autoflip == 1){
    glutAddMenuEntry(_("  *Auto flip"), COLORBAR_AUTOFLIP);
  }
  else{
    glutAddMenuEntry(_("  Auto flip"), COLORBAR_AUTOFLIP);
  }
  MakeColorbarMenu(&colorbarsmenu,
                   &colorbars_submenu1, &colorbars_submenu2, &colorbars_submenu3,
                   &colorbars_submenu4, &colorbars_submenu5, &colorbars_submenu6,
                   &colorbars_submenu7,
                   ColorbarMenu);

/* -------------------------------- colorbarmenu -------------------------- */

  CREATEMENU(colorbarmenu,ColorbarMenu);
  GLUTADDSUBMENU(_("Colorbar"),colorbarsmenu);
  GLUTADDSUBMENU(_("Colorbar type"), colorbarshademenu);
  GLUTADDSUBMENU(_("Colorbar digits"), colorbardigitmenu);
  if(force_decimal == 1){
    glutAddMenuEntry(_("  *Force decimal in colorbar labels"), COLORBAR_DECIMAL);
  }
  else{
    glutAddMenuEntry(_("  Force decimal in colorbar labels"), COLORBAR_DECIMAL);
  }
  if(use_transparency_data==1){
    glutAddMenuEntry(_("  *Transparent (data)"),COLORBAR_TRANSPARENT);
  }
  else{
    glutAddMenuEntry(_("  Transparent (data)"),COLORBAR_TRANSPARENT);
  }
  if(setbwdata == 1)glutAddMenuEntry(_("*Black/White (data)"), COLORBAR_TOGGLE_BW_DATA);
  if(setbwdata == 0)glutAddMenuEntry(_("Black/White (data)"), COLORBAR_TOGGLE_BW_DATA);
  if(setbw == 1)glutAddMenuEntry(_("*Black/White (geometry)"), COLORBAR_TOGGLE_BW);
  if(setbw == 0)glutAddMenuEntry(_("Black/White (geometry)"), COLORBAR_TOGGLE_BW);
  glutAddMenuEntry(_("  Reset"), COLORBAR_RESET);
  if(use_lighting==1)glutAddMenuEntry(_("*Lighting"), TOGGLE_LIGHTING);
  if(use_lighting==0)glutAddMenuEntry(_("Lighting"),  TOGGLE_LIGHTING);
  glutAddMenuEntry(_("Settings..."), MENU_COLORBAR_SETTINGS);

/* --------------------------------showVslice menu -------------------------- */
  if(nvsliceloaded==0){
    vd_shown=NULL;
  }
  if(global_scase.slicecoll.nvsliceinfo>0&&nvsliceloaded>0){
    CREATEMENU(showvslicemenu,ShowVSliceMenu);
    if(vd_shown!=NULL&&nvsliceloaded!=0){
      char menulabel[1024];
      slicedata *slice_shown;

      STRCPY(menulabel, "");
      if(showall_slices==1)STRCAT(menulabel, "*");
      slice_shown = global_scase.slicecoll.sliceinfo+vd_shown->ival;
      STRCAT(menulabel, slice_shown->label.longlabel);
      STRCAT(menulabel, ", ");
      STRCAT(menulabel, slice_shown->cdir);
      if(slice_shown->slicelabel!=NULL){
        STRCAT(menulabel, " - ");
        STRCAT(menulabel, slice_shown->slicelabel);
      }
      glutAddMenuEntry(menulabel,SHOW_ALL);
    }
    glutAddMenuEntry(_("  Show in:"), MENU_DUMMY);
    if(global_scase.show_slice_in_obst==ONLY_IN_GAS){
      glutAddMenuEntry(_("    *gas"),  MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(global_scase.show_slice_in_obst==GAS_AND_SOLID){
      glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("    *gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(global_scase.show_slice_in_obst==ONLY_IN_SOLID){
      glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("    *solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(global_scase.show_slice_in_obst==NEITHER_GAS_NOR_SOLID){
      glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("  gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(show_node_slices_and_vectors == 1)glutAddMenuEntry(_("*Show node centered slices and vectors"), MENU_SHOWSLICE_NODESLICEANDVECTORS);
    if(show_node_slices_and_vectors == 0)glutAddMenuEntry(_("Show node centered slices and vectors"), MENU_SHOWSLICE_NODESLICEANDVECTORS);
    if(show_cell_slices_and_vectors == 1)glutAddMenuEntry(_("*Show cell/face centered slices and vectors"), MENU_SHOWSLICE_CELLSLICEANDVECTORS);
    if(show_cell_slices_and_vectors == 0)glutAddMenuEntry(_("Show cell/face centered slices and vectors"), MENU_SHOWSLICE_CELLSLICEANDVECTORS);
    if(offset_slice == 1)glutAddMenuEntry(_("*Offset vector slice"), MENU_SHOWSLICE_OFFSET);
    if(offset_slice == 0)glutAddMenuEntry(_("Offset vector slice"), MENU_SHOWSLICE_OFFSET);
  }

/* --------------------------------showslice menu -------------------------- */
  if(nsliceloaded==0)sd_shown=NULL;
  InitShowSliceMenu(&showhideslicemenu, patchgeom_slice_showhide);
  InitShowMultiSliceMenu(&showmultislicemenu, showhideslicemenu, patchgeom_slice_showhide);

  CREATEMENU(tourcopymenu, TourCopyMenu);
  glutAddMenuEntry("Path through domain", -1);
  for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
    tourdata *touri;

    touri = global_scase.tourcoll.tourinfo + i;
    glutAddMenuEntry(touri->menulabel, i);
  }

    /* --------------------------------tour menu -------------------------- */

  CREATEMENU(tourmenu,TourMenu);

  GLUTADDSUBMENU(_("New"),tourcopymenu);
  if(global_scase.tourcoll.ntourinfo>0){
    glutAddMenuEntry("-",MENU_DUMMY);
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      tourdata *touri;
      int glui_avatar_index_local;
      char menulabel[1024];

      touri =global_scase.tourcoll. tourinfo + i;
      if(touri->display==1){
        STRCPY(menulabel,"");
        if(selectedtour_index==i){
          STRCAT(menulabel,"(editing)");
        }
        STRCAT(menulabel,"*");
        STRCAT(menulabel,touri->menulabel);
      }
      else{
        STRCPY(menulabel,touri->menulabel);
      }
      glui_avatar_index_local = touri->glui_avatar_index;
      if(glui_avatar_index_local>=0&&glui_avatar_index_local<global_scase.objectscoll.navatar_types){
        sv_object *avatari;

        avatari=global_scase.objectscoll.avatar_types[glui_avatar_index_local];
        strcat(menulabel,"(");
        strcat(menulabel,avatari->label);
        strcat(menulabel,")");
      }
      glutAddMenuEntry(menulabel,i);
    }
    if(selectedtour_index>=0){
      char menulabel[1024];

      glutAddMenuEntry("-", MENU_DUMMY);
      strcpy(menulabel,"");
      if(viewtourfrompath==1)strcat(menulabel,"*");
      strcat(menulabel,"View from ");
      strcat(menulabel,global_scase.tourcoll.tourinfo[selectedtour_index].label);
      glutAddMenuEntry(menulabel,MENU_TOUR_VIEWFROMROUTE);
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    glutAddMenuEntry(_("Show all"),MENU_TOUR_SHOWALL);
    glutAddMenuEntry(_("Hide all"),MENU_TOUR_MANUAL);
    glutAddMenuEntry(_("Settings..."), MENU_TOUR_SETTINGS);
  }

 /* --------------------------------Show Volume smoke menu -------------------------- */

  if(nvolsmoke3dloaded>0){
    char vlabel[256];

    CREATEMENU(showvolsmoke3dmenu,ShowVolsmoke3DMenu);
    strcpy(vlabel, "");
    if(show_volsmokefiles==1)strcat(vlabel, "*");
    strcat(vlabel,_("Show"));
    glutAddMenuEntry(vlabel,TOGGLE_VOLSMOKE);
    GLUTADDSUBMENU(_("Smoke colorbar"),smokecolorbarmenu);
  }

  CREATEMENU(aperturemenu,ApertureMenu);
  if(apertureindex==0)glutAddMenuEntry("*30",0);
  if(apertureindex!=0)glutAddMenuEntry("30",0);
  if(apertureindex==1)glutAddMenuEntry("*45",1);
  if(apertureindex!=1)glutAddMenuEntry("45",1);
  if(apertureindex==2)glutAddMenuEntry("*60",2);
  if(apertureindex!=2)glutAddMenuEntry("60",2);
  if(apertureindex==3)glutAddMenuEntry("*75",3);
  if(apertureindex!=3)glutAddMenuEntry("75",3);
  if(apertureindex==4)glutAddMenuEntry("*90",4);
  if(apertureindex!=4)glutAddMenuEntry("90",4);

  CREATEMENU(zoommenu,ZoomMenu);
  for(i = 0; i<MAX_ZOOMS+1; i++){
    char label[64];

    if(zooms[i]>0.0){
      if(zoomindex==i){
        sprintf(label, "*%f", zooms[i]);
      }
      else{
        sprintf(label, "%f", zooms[i]);
      }
      TrimZeros(label);
      glutAddMenuEntry(label, i);
    }
  }

  /* --------------------------------default view menu -------------------------- */
  SortCameras();
  CREATEMENU(defaultviewmenu, ResetDefaultMenu);
  if(have_geom_factors==1){
    if(use_geom_factors==1)glutAddMenuEntry("*include geometry",2);
    if(use_geom_factors==0)glutAddMenuEntry("include geometry", 2);
  }
  for(i = 0; i<ncameras_sorted; i++){
    cameradata *ca;
    char line[256];

    ca = cameras_sorted[i];
    if(ca->view_id>1)continue;
    strcpy(line, "");
    if(ca->view_id==selected_view){
      strcat(line, "*");
    }
    if(trainer_mode==1&&strcmp(ca->name, _("external"))==0){
      strcat(line, _("Outside"));
    }
    else{
      strcat(line, ca->name);
      if(strcmp(ca->name, viewpoint_label_startup)==0){
        strcat(line, " (startup view)");
      }
    }
    glutAddMenuEntry(line, ca->view_id);
  }
  glutAddMenuEntry("center", XYZ_CENTER);

  /* --------------------------------reset menu -------------------------- */

  CREATEMENU(resetmenu,ResetMenu);
  if(trainer_mode==1){
    if(visBlocks==visBLOCKOutline){
      glutAddMenuEntry(_("*Outline"),MENU_OUTLINEVIEW);
    }
    else{
      glutAddMenuEntry(_("Outline"),MENU_OUTLINEVIEW);
    }
    glutAddMenuEntry("-",MENU_DUMMY);
  }
  if(trainer_mode==0){
    char view_label[255], menu_label[255];
    char *current_view = NULL;
    cameradata *ca;

    for(ca = camera_list_first.next; ca->next!=NULL; ca = ca->next){
      if(ca->view_id==selected_view){
        current_view = ca->name;
        break;
      }
    }
    GetNextViewLabel(view_label);
    strcpy(menu_label, _("Save viewpoint as"));
    strcat(menu_label, " ");
    strcat(menu_label,view_label);

    glutAddMenuEntry(menu_label,SAVE_VIEWPOINT);
    if(current_view != NULL){
      if(strcmp(current_view, viewpoint_label_startup)!=0){
        strcpy(menu_label, _("Apply"));
        strcat(menu_label, " ");
        strcat(menu_label, current_view);
        strcat(menu_label, " ");
        strcat(menu_label, _("at startup"));
        glutAddMenuEntry(menu_label, MENU_STARTUPVIEW);
      }
    }
    else{
      strcpy(menu_label, _("Save viewpoint as"));
      strcat(menu_label, " ");
      strcat(menu_label, view_label);
      strcat(menu_label, " ");
      strcat(menu_label, _("and apply at startup"));
      glutAddMenuEntry(menu_label, SAVE_VIEWPOINT_AS_STARTUP);
    }
    GLUTADDSUBMENU(_("Zoom"),zoommenu);
    if(projection_type == PROJECTION_ORTHOGRAPHIC)glutAddMenuEntry(_("Switch to perspective view       ALT v"),MENU_SIZEPRESERVING);
    if(projection_type == PROJECTION_PERSPECTIVE)glutAddMenuEntry(_("Switch to size preserving view   ALT v"),MENU_SIZEPRESERVING);
    glutAddMenuEntry("-",MENU_DUMMY);
  }

  for(i = 0; i < ncameras_sorted;i++){
    cameradata *ca;
    char line[256];

    ca = cameras_sorted[i];
    if(ca->view_id<=1)continue;
    strcpy(line,"");
    if(ca->view_id==selected_view){
      strcat(line,"*");
    }
    if(trainer_mode==1&&strcmp(ca->name,_("external"))==0){
      strcat(line,_("Outside"));
    }
    else{
      strcat(line,ca->name);
      if(strcmp(ca->name, viewpoint_label_startup)==0){
        strcat(line," (startup view)");
      }
    }
    glutAddMenuEntry(line,ca->view_id);
  }
  if(trainer_mode==0&&showtime==1){
    glutAddMenuEntry("-",MENU_DUMMY);
    glutAddMenuEntry(_("Time"),MENU_TIMEVIEW);
  }
  glutAddMenuEntry(_("Settings..."), MENU_VIEWPOINT_SETTINGS);

  /* --------------------------------showhide menu -------------------------- */

  showhide_data = 0;
  CREATEMENU(showhidemenu,ShowHideMenu);
  GLUTADDSUBMENU(_("Color"), colorbarmenu);
  GLUTADDSUBMENU(_("Geometry"),geometrymenu);
  if(global_scase.hvaccoll.nhvacinfo>0)GLUTADDSUBMENU(_("HVAC"), hvacmenu);
  if(global_scase.nterraininfo>0&&global_scase.ngeominfo==0){
    GLUTADDSUBMENU(_("Terrain"), terrain_obst_showmenu);
  }
  if(GetNumActiveDevices()>0||global_scase.ncvents>0){
    GLUTADDSUBMENU(_("Devices"), showobjectsmenu);
  }
  GLUTADDSUBMENU(_("Labels"),labelmenu);

  GLUTADDSUBMENU(_("Viewpoints (default)"), defaultviewmenu);
  GLUTADDSUBMENU(_("Viewpoints (user)"), resetmenu);
  glutAddMenuEntry("-", MENU_DUMMY);
  if(nsmoke3dloaded>0){
    showhide_data = 1;
    GLUTADDSUBMENU(_("3D smoke"), smoke3dshowmenu);
  }
  if(nvolsmoke3dloaded>0){
    char vlabel[256];

    showhide_data = 1;
    strcpy(vlabel, _("3D smoke (Volume rendered)"));
    GLUTADDSUBMENU(vlabel, showvolsmoke3dmenu);
  }
  if(npatchloaded>0){
    showhide_data = 1;
    GLUTADDSUBMENU(_("Boundary"), showpatchmenu);
  }
  if(npart5loaded>0){
    showhide_data = 1;
    GLUTADDSUBMENU(_("Particles"), particlepropshowmenu);
  }
  if(ReadIsoFile==1){
    int niso_loaded=0;

    for(i=0;i<global_scase.nisoinfo;i++){
      isodata *isoi;

      isoi = global_scase.isoinfo + i;
      if(isoi->loaded==1)niso_loaded++;
    }

    if(niso_loaded>0){
     GLUTADDSUBMENU(_("Isosurfaces"),isoshowmenu);
    }
    showhide_data = 1;
  }
  if(nplot3dloaded>0){
    showhide_data = 1;
    GLUTADDSUBMENU(_("Plot3D"), plot3dshowmenu);
  }

  if(nvsliceloaded>0){
    showhide_data = 1;
    GLUTADDSUBMENU(_("Vector slice"), showvslicemenu);
  }
  if(nsliceloaded>0||patchgeom_slice_showhide==1){
    showhide_data = 1;
    if(have_multislice==1){
      GLUTADDSUBMENU(_("Slice"),showmultislicemenu);
    }
    else{
      GLUTADDSUBMENU(_("Slice"), showhideslicemenu);
    }
  }

  if(global_scase.nzoneinfo>0&&(ReadZoneFile==1||global_scase.nzvents>0)){
    showhide_data = 1;
    GLUTADDSUBMENU(_("Zone"), zoneshowmenu);
  }
  if(global_scase.objectscoll.nobject_defs>0){
    int num_activedevices=0;

    for(i = 0; i<global_scase.objectscoll.nobject_defs; i++){
      sv_object *obj_typei;

      obj_typei = global_scase.objectscoll.object_defs[i];
      if(obj_typei->used==1)num_activedevices++;
    }

    if(num_activedevices>0){
      /*
      if(isZoneFireModel==0||(isZoneFireModel==1&&num_activedevices>1)){
        GLUTADDSUBMENU(_("Objects"),showobjectsmenu);
      }
      */
    }
    else{
      for(i=0;i<global_scase.objectscoll.nobject_defs;i++){
        sv_object *obj_typei;

        obj_typei = global_scase.objectscoll.object_defs[i];
        if(obj_typei->used==1){
          char obj_menu[256];

          strcpy(obj_menu,"");
          if(obj_typei->visible==1){
            strcat(obj_menu,"*");
          }
          strcat(obj_menu,obj_typei->label);
          showhide_data = 1;
          glutAddMenuEntry(obj_menu, i);
        }
      }
    }
  }
  if(global_scase.ntc_total>0){
    showhide_data = 1;
    if(global_scase.isZoneFireModel==1){
      if(visSensor==1)glutAddMenuEntry(_("*Targets"), MENU_SHOWHIDE_SENSOR);
      if(visSensor==0)glutAddMenuEntry(_("Targets"), MENU_SHOWHIDE_SENSOR);
      if(global_scase.hasSensorNorm==1){
        if(visSensorNorm==1)glutAddMenuEntry(_("*Target orientation"), MENU_SHOWHIDE_SENSOR_NORM);
        if(visSensorNorm==0)glutAddMenuEntry(_("Target orientation"), MENU_SHOWHIDE_SENSOR_NORM);
      }
    }
    else{
      if(visSensor==1)glutAddMenuEntry(_("*Thermocouples"), MENU_SHOWHIDE_SENSOR);
      if(visSensor==0)glutAddMenuEntry(_("Thermocouples"), MENU_SHOWHIDE_SENSOR);
      if(global_scase.hasSensorNorm==1){
        if(visSensorNorm==1)glutAddMenuEntry(_("*Thermocouple norms"), MENU_SHOWHIDE_SENSOR_NORM);
        if(visSensorNorm==0)glutAddMenuEntry(_("Thermocouple norms"), MENU_SHOWHIDE_SENSOR_NORM);
      }
    }
  }
  if(showhide_data==1)glutAddMenuEntry("-", MENU_DUMMY);
  if(background_flip==1){
    glutAddMenuEntry(_("*Flip background"), MENU_SHOWHIDE_FLIP);
  }
  else{
    glutAddMenuEntry(_("Flip background"), MENU_SHOWHIDE_FLIP);
  }
  if(titlesafe_offset==0)glutAddMenuEntry(_("Offset window"), MENU_SHOWHIDE_OFFSET);
  if(titlesafe_offset!=0)glutAddMenuEntry(_("*Offset window"),MENU_SHOWHIDE_OFFSET);
  if(ntextures_loaded_used>global_scase.terrain_texture_coll.nterrain_textures){
    GLUTADDSUBMENU(_("Textures"),textureshowmenu);
  }
#ifdef pp_MEMPRINT
  glutAddMenuEntry("Show Memory block info",MENU_SHOWHIDE_PRINT);
#endif


/* --------------------------------frame rate menu -------------------------- */

  CREATEMENU(frameratemenu,FrameRateMenu);
  if(frameratevalue==1)glutAddMenuEntry("*1 FPS",1);
  if(frameratevalue!=1)glutAddMenuEntry("1 FPS",1);
  if(frameratevalue==2)glutAddMenuEntry("*2 FPS",2);
  if(frameratevalue!=2)glutAddMenuEntry("2 FPS",2);
  if(frameratevalue==4)glutAddMenuEntry("*4 FPS",4);
  if(frameratevalue!=4)glutAddMenuEntry("4 FPS",4);
  if(frameratevalue==8)glutAddMenuEntry("*8 FPS",8);
  if(frameratevalue!=8)glutAddMenuEntry("8 FPS",8);
  if(frameratevalue==10)glutAddMenuEntry("*10 FPS",10);
  if(frameratevalue!=10)glutAddMenuEntry("10 FPS",10);
  if(frameratevalue==15)glutAddMenuEntry("*15 FPS",15);
  if(frameratevalue!=15)glutAddMenuEntry("15 FPS",15);
  if(frameratevalue==30)glutAddMenuEntry("*30 FPS",30);
  if(frameratevalue!=30)glutAddMenuEntry("30 FPS",30);
  if(frameratevalue==2001)glutAddMenuEntry(_("*Real time"),MENU_FRAMERATE_Realtime);
  if(frameratevalue!=2001)glutAddMenuEntry(_("Real time"), MENU_FRAMERATE_Realtime);
  if(frameratevalue==2002)glutAddMenuEntry(_("*2 x Real time"), MENU_FRAMERATE_2xRealtime);
  if(frameratevalue!=2002)glutAddMenuEntry(_("2 x Real time"), MENU_FRAMERATE_2xRealtime);
  if(frameratevalue==2004)glutAddMenuEntry(_("*4 x Real time"), MENU_FRAMERATE_4xRealtime);
  if(frameratevalue!=2004)glutAddMenuEntry(_("4 x Real time"), MENU_FRAMERATE_4xRealtime);
  if(frameratevalue!=1000)glutAddMenuEntry(_("Unlimited"),1000);
  if(frameratevalue==1000)glutAddMenuEntry(_("*Unlimited"),1000);
  if(frameratevalue<0){
    glutAddMenuEntry(_("*Step"),-1);
  }
  else{
    glutAddMenuEntry(_("Step"),-1);
  }

/* --------------------------------render menu -------------------------- */
  {
    char renderwindow[1024];
    char renderwindow2[1024];
    char renderwindow3[1024];
    char rendertemp[1024];
    int render_current=0;

    strcpy(renderwindow,"  ");
    if(renderW==320)strcat(renderwindow,"*");
    strcat(renderwindow,"320x240");

    strcpy(renderwindow2,"  ");
    if(renderW==640)strcat(renderwindow2,"*");
    strcat(renderwindow2,"640x480");

    sprintf(rendertemp,"%i%s%i (current)",screenWidth,"x",screenHeight);
    strcpy(renderwindow3,"  ");
    if(renderW!=320&&renderW!=640&&renderW!=2*screenWidth){
      render_current=1;
      strcat(renderwindow3,"*");
    }
    strcat(renderwindow3,rendertemp);

    CREATEMENU(render_skipmenu,SkipMenu);
    for(i = 0;i<NRENDER_SKIPS;i++){
      char skiplabel[128];

      strcpy(skiplabel, "  ");
      if(render_skip==render_skips[i])strcat(skiplabel, "*");
      strcat(skiplabel, crender_skips[i]);
      glutAddMenuEntry(skiplabel, render_skips[i]);
    }

    CREATEMENU(resolutionmultipliermenu,RenderMenu);
    for(i = 2;i<=10;i++){
      char render_label[256];
      int render_index;

      render_index = 10000+MIN(5, i);
      if(resolution_multiplier==i){
        sprintf(render_label, "  *%ix", i);
        glutAddMenuEntry(render_label, render_index);
      }
      else if(i<=5){
        sprintf(render_label, "  %ix", i);
        glutAddMenuEntry(render_label, render_index);
      }
    }

    CREATEMENU(render_filesuffixmenu, RenderMenu);
    if(render_label_type==RENDER_LABEL_FRAMENUM){
      glutAddMenuEntry(_("  *Frame number"),RenderLABELframenumber);
      glutAddMenuEntry(_("  Time"),RenderLABELtime);
    }
    if(render_label_type==RENDER_LABEL_TIME){
      glutAddMenuEntry(_("  Frame number"),RenderLABELframenumber);
      glutAddMenuEntry(_("  *Time"),RenderLABELtime);
    }

    CREATEMENU(render_filetypemenu, RenderMenu);
    if(render_filetype==PNG){
      glutAddMenuEntry("  *PNG",RenderPNG);
      glutAddMenuEntry("  JPEG",RenderJPEG);
    }
    if(render_filetype==JPEG){
      glutAddMenuEntry("  PNG",RenderPNG);
      glutAddMenuEntry("  *JPEG",RenderJPEG);
    }
    if(render_filetype==IMAGE_NONE){
      glutAddMenuEntry("  PNG",RenderPNG);
      glutAddMenuEntry("  JPEG",RenderJPEG);
    }

    CREATEMENU(render_startmenu,RenderMenu);
    {
      char sizeORIGRES[128], sizeHIGHRES[128];
      int width, height, factor;

      if(render_current==1){
        width = screenWidth;
        height = screenHeight;
      }
      else{
        width = renderW;
        height = renderH;
      }

      factor = glui_resolution_multiplier;
      sprintf(sizeORIGRES, "%ix%i", width, height);
      sprintf(sizeHIGHRES, "%ix%i", width*factor, height*factor);
      glutAddMenuEntry(sizeORIGRES, RenderStartORIGRES);
      glutAddMenuEntry(sizeHIGHRES, RenderStartHIGHRES);
      {
        char rend_label[100];
        unsigned char deg360[] = {'3','6','0',0};

        sprintf(rend_label,"%s - %ix%i",deg360,nwidth360, nheight360);
        glutAddMenuEntry(rend_label, RenderStart360);
      }
    }

    CREATEMENU(render_resolutionmenu, RenderMenu);
    glutAddMenuEntry(renderwindow,    Render320);
    glutAddMenuEntry(renderwindow2,   Render640);
    glutAddMenuEntry(renderwindow3,   RenderWindow);

    CREATEMENU(rendermenu,RenderMenu);
    GLUTADDSUBMENU(_("Start rendering"),  render_startmenu);
    glutAddMenuEntry(_("Stop rendering"), RenderCancel);
    glutAddMenuEntry("-", MENU_DUMMY);
    glutAddMenuEntry(_("Render html(current)"), RenderHTML);
    glutAddMenuEntry(_("Render html(all)"),     RenderHTMLALL);
    glutAddMenuEntry(_("Render json(current)"),  RenderJSON);
    glutAddMenuEntry(_("Render json(all)"),     RenderJSONALL);

    glutAddMenuEntry("-", MENU_DUMMY);

    {
    char skip_label[128];

    if(render_skip==1){
      strcpy(skip_label,"Frames/all");
    }
    else if(render_skip==RENDER_CURRENT_SINGLE){
      sprintf(skip_label, "Frames/Current");
    }
    else{
      sprintf(skip_label, "Frames/%i", render_skip);
    }
    GLUTADDSUBMENU(skip_label, render_skipmenu);
}
    GLUTADDSUBMENU(_("Image size"),  render_resolutionmenu);
    if(render_current==1){
      char res_menu[128];

      sprintf(res_menu, "Image size multiplier/%ix", resolution_multiplier);
      GLUTADDSUBMENU(res_menu, resolutionmultipliermenu);
    }
    GLUTADDSUBMENU(_("Image type"),        render_filetypemenu);
    GLUTADDSUBMENU(_("Image suffix"), render_filesuffixmenu);
    glutAddMenuEntry(_("Settings..."), MENU_RENDER_SETTINGS);
    GLUIUpdateRender();
  }

  /* --------------------------------filesdialog menu -------------------------- */

  CREATEMENU(filesdialogmenu, DialogMenu);
  glutAddMenuEntry(_("Auto load data files..."), DIALOG_AUTOLOAD);
#ifdef pp_COMPRESS
  if(smokezippath!=NULL&&(global_scase.npatchinfo>0||global_scase.smoke3dcoll.nsmoke3dinfo>0||global_scase.slicecoll.nsliceinfo>0)){
#ifdef pp_DIALOG_SHORTCUTS
    glutAddMenuEntry(_("Compress data files...  ALT z"), DIALOG_SMOKEZIP);
#else
    glutAddMenuEntry(_("Compress data files..."), DIALOG_SMOKEZIP);
#endif
  }
#endif
  glutAddMenuEntry(_("Save/load configuration files..."), DIALOG_CONFIG);
  glutAddMenuEntry(_("Render images..."), DIALOG_RENDER);
  THREADcontrol(ffmpeg_threads, THREAD_LOCK);
  if(have_slurm==1&&have_ffmpeg==1){
    glutAddMenuEntry(_("Make movies(local)..."), DIALOG_MOVIE);
    glutAddMenuEntry(_("Make movies(cluster)..."), DIALOG_MOVIE_BATCH);
  }
  if(have_slurm==0&&have_ffmpeg==1){
    glutAddMenuEntry(_("Make movies..."), DIALOG_MOVIE);
  }
  THREADcontrol(ffmpeg_threads, THREAD_UNLOCK);
  glutAddMenuEntry(_("Record/run scripts..."), DIALOG_SCRIPT);

  /* --------------------------------viewdialog menu -------------------------- */

  CREATEMENU(viewdialogmenu, DialogMenu);
#ifdef pp_DIALOG_SHORTCUTS
  glutAddMenuEntry("Clipping...  ALT c", DIALOG_CLIP);
  glutAddMenuEntry(_("Tours...  ALT t"), DIALOG_TOUR_SHOW);
  glutAddMenuEntry("Edit Colorbar...  ALT C", DIALOG_COLORBAR);
  if(global_scase.isZoneFireModel==0 && have_geometry_dialog==1){
    glutAddMenuEntry(_("Examine geometry...  ALT e"), DIALOG_GEOMETRY_OPEN);
  }
#else
  glutAddMenuEntry("Clip scene...", DIALOG_CLIP);
  glutAddMenuEntry(_("Tours..."), DIALOG_TOUR_SHOW);
  glutAddMenuEntry("Edit Colorbar...  ", DIALOG_COLORBAR);
  if(global_scase.isZoneFireModel == 0 && have_geometry_dialog == 1){
    glutAddMenuEntry(_("Examine geometry...  "), DIALOG_GEOMETRY_OPEN);
  }
#endif
  if(global_scase.nterraininfo>0&&global_scase.ngeominfo==0){
    glutAddMenuEntry(_("Terrain..."), DIALOG_TERRAIN);
  }
  if(global_scase.hvaccoll.nhvacinfo > 0){
    glutAddMenuEntry(_("HVAC settings..."), DIALOG_HVAC);
  }
  char stereo_label[32];
  if(have_vr == 1){
    strcpy(stereo_label, "Stereo/VR settings...");
  }
  else{
    strcpy(stereo_label, "Stereo settings...");
  }
  glutAddMenuEntry(stereo_label, DIALOG_STEREO);
  if(trainer_active==1){
    glutAddMenuEntry(_("Trainer..."), DIALOG_TRAINER);
  }

  /* --------------------------------data dialog menu -------------------------- */

  CREATEMENU(datadialogmenu, DialogMenu);
  if(global_scase.devicecoll.ndeviceinfo>0&&GetNumActiveDevices()>0){
    glutAddMenuEntry(_("Devices/Objects"), DIALOG_DEVICE);
  }
  if((global_scase.csvcoll.ncsvfileinfo>0&&have_ext==0)||(global_scase.csvcoll.ncsvfileinfo>1&&have_ext==1)){
    glutAddMenuEntry(_("2D plots"), DIALOG_2DPLOTS);
  }
  glutAddMenuEntry(_("Show/Hide data files..."), DIALOG_SHOWFILES);
  glutAddMenuEntry(_("Particle tracking..."),    DIALOG_SHOOTER);

  /* --------------------------------window menu -------------------------- */

  CREATEMENU(windowdialogmenu, DialogMenu);
  glutAddMenuEntry(_("Fonts..."),             DIALOG_FONTS);
  glutAddMenuEntry(_("User ticks..."),        DIALOG_USER_TICKS);
  glutAddMenuEntry(_("Labels + Ticks..."),    DIALOG_LABELS_TICKS);
  glutAddMenuEntry(_("Window properties..."), DIALOG_WINDOW_PROPERTIES);
  glutAddMenuEntry(_("Scaling..."),           DIALOG_SCALING);

  /* --------------------------------dialog menu -------------------------- */

  CREATEMENU(dialogmenu,DialogMenu);

  glutAddMenuEntry(_("Display...  ALT D"),            DIALOG_DISPLAY);
  glutAddMenuEntry(_("Files/Data/Coloring... ALT b"), DIALOG_BOUNDS);
  glutAddMenuEntry(_("Motion/View/Render...  ALT m"), DIALOG_MOTION);
  glutAddMenuEntry(_("Viewpoints... ALT g"),          DIALOG_VIEW);

  glutAddMenuEntry("-",MENU_DUMMY2);

  GLUTADDSUBMENU(_("Data"),   datadialogmenu);
  GLUTADDSUBMENU(_("Files"),  filesdialogmenu);
  GLUTADDSUBMENU(_("View"),   viewdialogmenu);
  GLUTADDSUBMENU(_("Window"), windowdialogmenu);

  glutAddMenuEntry("-",MENU_DUMMY2);
#ifdef pp_DIALOG_SHORTCUTS
  glutAddMenuEntry(_("Shrink all dialogs ALT X"), DIALOG_SHRINKALL);
  glutAddMenuEntry(_("Close all dialogs  ALT x"), DIALOG_HIDEALL);
#else
  glutAddMenuEntry(_("Shrink all dialogs "), DIALOG_SHRINKALL);
  glutAddMenuEntry(_("Close all dialogs  "), DIALOG_HIDEALL);
#endif

  /* -------------------------------- font menu -------------------------- */

  CREATEMENU(fontmenu,FontMenu);
  switch(fontindex){
  case SMALL_FONT:
    glutAddMenuEntry(_("*Small"),SMALL_FONT);
    glutAddMenuEntry(_("Large"),LARGE_FONT);
    glutAddMenuEntry(_("Scaled"),SCALED_FONT);
    break;
  case LARGE_FONT:
    glutAddMenuEntry(_("Small"),SMALL_FONT);
    glutAddMenuEntry(_("*Large"),LARGE_FONT);
    glutAddMenuEntry(_("Scaled"),SCALED_FONT);
    break;
  case SCALED_FONT:
    glutAddMenuEntry(_("Small"),SMALL_FONT);
    glutAddMenuEntry(_("Large"),LARGE_FONT);
    glutAddMenuEntry(_("*Scaled"),SCALED_FONT);
    break;
  default:
    assert(FFALSE);
    break;
  }
  glutAddMenuEntry(_("Settings..."), MENU_FONT_SETTINGS);

  /* -------------------------------- units menu -------------------------- */

  if(nunitclasses>0){
    for(i=0;i<nunitclasses;i++){
      f_units *uci;
      int j;

      uci = unitclasses + i;
      CheckMemory;

      CREATEMENU(uci->submenuid,UnitsMenu);

      for(j=0;j<uci->nunits;j++){
        char menulabel[1024];

        if(uci->unit_index==j){
          strcpy(menulabel,"*");
          strcat(menulabel,uci->units[j].unit);
        }
        else{
          strcpy(menulabel,uci->units[j].unit);
        }
        if(global_scase.smokediff==1&&uci->diff_index==j&&uci->units[j].rel_defined==1){
          strcat(menulabel," rel to ");
          strcat(menulabel,uci->units[j].rel_val);
          strcat(menulabel," ");
          strcat(menulabel,uci->units[0].unit);
        }
        glutAddMenuEntry(menulabel,1000*i+j);
      }
    }
    CREATEMENU(unitsmenu,UnitsMenu);
    for(i=0;i<nunitclasses;i++){
      f_units *uci;

      uci = unitclasses + i;
      if(uci->visible==1||show_all_units==1||strcmp(uci->unitclass,"Distance")==0){
        GLUTADDSUBMENU(uci->unitclass,uci->submenuid);
      }
    }
    if(vishmsTimelabel==0)glutAddMenuEntry(_("time (h:m:s)"), MENU_UNITS_HMS);
    if(vishmsTimelabel==1)glutAddMenuEntry(_("*time (h:m:s)"), MENU_UNITS_HMS);
    if(show_all_units==1)glutAddMenuEntry(_("*show all units"), MENU_UNITS_SHOWALL);
    if(show_all_units==0)glutAddMenuEntry(_("show all units"), MENU_UNITS_SHOWALL);
    glutAddMenuEntry(_("Reset"), MENU_UNITS_RESET);
  }

/* --------------------------------option menu -------------------------- */

  CREATEMENU(optionmenu,OptionMenu);
  if(nunitclasses>0)GLUTADDSUBMENU(_("Display Units"),unitsmenu);
  GLUTADDSUBMENU(_("Rotation options"),rotatetypemenu);
  GLUTADDSUBMENU(_("Translation options"), translatetypemenu);
  GLUTADDSUBMENU(_("Max frame rate"),frameratemenu);
  GLUTADDSUBMENU(_("Render"),rendermenu);
  GLUTADDSUBMENU(_("Tours"),tourmenu);
  GLUTADDSUBMENU(_("Font"),fontmenu);
  if(trainer_active==1)glutAddMenuEntry(_("Trainer menu"),MENU_OPTION_TRAINERMENU);

/* -------------------------------- about menu -------------------------- */

  CREATEMENU(disclaimermenu,AboutMenu);
  glutAddMenuEntry("The US Department of Commerce makes no warranty, expressed or",MENU_DUMMY);
  glutAddMenuEntry("implied, to users of Smokeview, and accepts no responsibility",MENU_DUMMY);
  glutAddMenuEntry("for its use. Users of Smokeview assume sole responsibility under",MENU_DUMMY);
  glutAddMenuEntry("Federal law for determining the appropriateness of its use in any",MENU_DUMMY);
  glutAddMenuEntry("particular application; for any conclusions drawn from the results",MENU_DUMMY);
  glutAddMenuEntry("of its use; and for any actions taken or not taken as a result of",MENU_DUMMY);
  glutAddMenuEntry("analysis performed using this tools.",MENU_DUMMY);
  glutAddMenuEntry("",MENU_DUMMY);
  glutAddMenuEntry("Smokeview and the companion program FDS is intended for use only",MENU_DUMMY);
  glutAddMenuEntry("by those competent in the fields of fluid dynamics, thermodynamics,",MENU_DUMMY);
  glutAddMenuEntry("combustion, and heat transfer, and is intended only to supplement",MENU_DUMMY);
  glutAddMenuEntry("the informed judgment of the qualified user. These software packages",MENU_DUMMY);
  glutAddMenuEntry("may or may not have predictive capability when applied to a specific",MENU_DUMMY);
  glutAddMenuEntry("set of factual circumstances.  Lack of accurate predictions could lead",MENU_DUMMY);
  glutAddMenuEntry("to erroneous conclusions with regard to fire safety.  All results",MENU_DUMMY);
  glutAddMenuEntry("should be evaluated by an informed user.",1);

/* -------------------------------- about menu -------------------------- */

  CREATEMENU(aboutmenu,AboutMenu);
  glutAddMenuEntry(release_title,1);
  {
#ifdef pp_GPU
    char version_label[256];
#endif
    char menulabel[1024];
    char compiler_version_label[1024];

    sprintf(menulabel,"  Smokeview build: %s",smv_githash);
    glutAddMenuEntry(menulabel,1);
    strcpy(compiler_version_label, _("    Compiler version:"));
    strcat(compiler_version_label, " ");
    strcat(compiler_version_label, pp_COMPVER);
    glutAddMenuEntry(compiler_version_label, 1);
    if(global_scase.fds_version!=NULL){
      sprintf(menulabel, "  FDS version: %s", global_scase.fds_version);
      glutAddMenuEntry(menulabel, 1);
    }
    if(global_scase.fds_githash!=NULL){
      sprintf(menulabel,"  FDS build: %s",global_scase.fds_githash);
      glutAddMenuEntry(menulabel,1);
    }
#ifdef pp_GPU
    strcpy(version_label,_("  OpenGL version:"));
    strcat(version_label," ");
    strcat(version_label,(char *)glGetString(GL_VERSION));
    glutAddMenuEntry(version_label,1);
    if(gpuactive==1){
      if(usegpu==1){
        strcpy(menulabel,_("  GPU activated. (Press G to deactivate)"));
      }
      else{
        strcpy(menulabel,_("  GPU available but not in use. (Press G to activate)"));
      }
    }
    else{
      strcpy(menulabel,_("  GPU not available"));
    }
    glutAddMenuEntry(menulabel,1);
#endif
#ifdef WIN32
    glutAddMenuEntry("  Platform: WIN64", 1);
#endif
#ifdef pp_OSX
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      glutAddMenuEntry("  Platform: OSX64(high res fonts)", 1);
    }
    else{
      glutAddMenuEntry("  Platform: OSX64", 1);
    }
#else
    glutAddMenuEntry("  Platform: OSX64", 1);
#endif
#endif
#ifdef pp_LINUX
    glutAddMenuEntry("  Platform: LINUX64", 1);
#endif
    GLUTADDSUBMENU(_("Disclaimer"),disclaimermenu);
    glutAddMenuEntry("Data transfer test", ABOUT_DATA_TRANSFER_TEST);
  }

  /* --------------------------------web help menu -------------------------- */

  CREATEMENU(webhelpmenu,HelpMenu);

#ifdef WIN32
  glutAddMenuEntry(_("Downloads"),               MENU_HELP_DOWNLOADS);
  glutAddMenuEntry(_("Documentation"),           MENU_HELP_DOCUMENTATION);
  glutAddMenuEntry(_("Discussion forum"),        MENU_HELP_FORUM);
  glutAddMenuEntry(_("FDS issue tracker"),       MENU_HELP_FDS_ISSUES);
  glutAddMenuEntry(_("Smokeview issue tracker"), MENU_HELP_SMV_ISSUES);
  glutAddMenuEntry(_("Release notes"),           MENU_HELP_RELEASENOTES);
  glutAddMenuEntry(_("Home page"),               MENU_HELP_FDSWEB);
#endif
#ifdef pp_OSX
  glutAddMenuEntry(_("Downloads"),               MENU_HELP_DOWNLOADS);
  glutAddMenuEntry(_("Documentation"),           MENU_HELP_DOCUMENTATION);
  glutAddMenuEntry(_("Discussion forum"),        MENU_HELP_FORUM);
  glutAddMenuEntry(_("FDS issue tracker"),       MENU_HELP_FDS_ISSUES);
  glutAddMenuEntry(_("Smokeview issue tracker"), MENU_HELP_SMV_ISSUES);
  glutAddMenuEntry(_("Release notes"),           MENU_HELP_RELEASENOTES);
  glutAddMenuEntry(_("Home page"),               MENU_HELP_FDSWEB);
#endif
#ifdef pp_LINUX
  glutAddMenuEntry(_("Downloads: https://pages.nist.gov/fds-smv/"),                                   MENU_HELP_DOWNLOADS);
  glutAddMenuEntry(_("Documentation:  https://pages.nist.gov/fds-smv/manuals.html"),                  MENU_HELP_DOCUMENTATION);
  glutAddMenuEntry(_("Discussion forum: https://github.com/firemodels/fds/discussions"),              MENU_HELP_FORUM);
  glutAddMenuEntry(_("FDS issue tracker: https://github.com/firemodels/fds/issues"),                  MENU_HELP_FDS_ISSUES);
  glutAddMenuEntry(_("Smokeview issue tracker: https://github.com/firemodels/smv/issues"),            MENU_HELP_SMV_ISSUES);
  glutAddMenuEntry(_("Release notes: https://pages.nist.gov/fds-smv/smv_readme.html"),                MENU_HELP_RELEASENOTES);
  glutAddMenuEntry(_("Home page: https://pages.nist.gov/fds-smv/"),                                   MENU_HELP_FDSWEB);
#endif

  /* --------------------------------keyboard help menu -------------------------- */

  CREATEMENU(keyboardhelpmenu,HelpMenu);
  if(plotstate==DYNAMIC_PLOTS){
    glutAddMenuEntry(_("Animation"),MENU_DUMMY);
    glutAddMenuEntry(_("  0: reset animation to the initial time"), MENU_DUMMY);
    glutAddMenuEntry(_("  1-9: number of frames to skip"), MENU_DUMMY);
    glutAddMenuEntry(_("  a/ALT a: increase/decrease flow vector length by 1.5"), MENU_DUMMY);
    glutAddMenuEntry(_("  H: toggle  slice and vector slice visibility"), MENU_DUMMY);
    glutAddMenuEntry(_("  I: toggle  visibility of slices in blockages"), MENU_DUMMY);
    glutAddMenuEntry(_("  N: force bound update when loading files (assume fds is running)"), MENU_DUMMY);
    glutAddMenuEntry(_("  p,P: increment particle variable displayed"), MENU_DUMMY);
    glutAddMenuEntry(_("  s,S: increase/decrease interval between adjacent vectors"), MENU_DUMMY);
    glutAddMenuEntry(_("  t: set/unset single time step mode"), MENU_DUMMY);
    glutAddMenuEntry(_("  T: time display between 'Time s' and 'h:m:s'"), MENU_DUMMY);
    if(global_scase.cellcenter_slice_active==1){
      glutAddMenuEntry(_("     (also, toggles cell center display on/off)"), MENU_DUMMY);
      glutAddMenuEntry(_("  @: display FDS values in cell centered slices"), MENU_DUMMY);
    }
    glutAddMenuEntry("  $: force 3D smoke/fire to be opaque", MENU_DUMMY);
    glutAddMenuEntry(_("  u: reload files"), MENU_DUMMY);
    glutAddMenuEntry(_("  [,]: decrease/increase particle size"), MENU_DUMMY);
  }
  if(rotation_type==EYE_CENTERED){
    glutAddMenuEntry(_("Motion"), MENU_DUMMY);
    glutAddMenuEntry(_("   left/right cursor: rotate left/right"), MENU_DUMMY);
    glutAddMenuEntry(_("      up/down cursor: move forward/backward"), MENU_DUMMY);
    glutAddMenuEntry(_(" CTRL:up/down cursor: move forward/backward 5 times slower"), MENU_DUMMY);
    glutAddMenuEntry(_(" SHIFT: left/right cursor: rotate 90 degrees"), MENU_DUMMY);
    glutAddMenuEntry(_("    ALT:left/right cursor: slide left/right"), MENU_DUMMY);
    glutAddMenuEntry(_("    ALT:   up/down cursor: slide up/down"), MENU_DUMMY);
    glutAddMenuEntry(_("     INSERT/HOME/PageUP: tilt down/reset/tilt up"), MENU_DUMMY);
  }
  if(plotstate==STATIC_PLOTS){
    glutAddMenuEntry(_("Plot3D"), MENU_DUMMY);
    glutAddMenuEntry(_("  a/ALT a: increase/decrease flow vector length by 1.5"), MENU_DUMMY);
    glutAddMenuEntry(_("  c: toggle between continuous and 2D stepped contours"), MENU_DUMMY);
    glutAddMenuEntry(_("  i: toggle iso-surface visibility"), MENU_DUMMY);
    glutAddMenuEntry(_("  p,P: increment plot3d variable displayed"), MENU_DUMMY);
    glutAddMenuEntry(_("  s,S: increase/decrease interval between adjacent vectors"), MENU_DUMMY);
    glutAddMenuEntry(_("  v: toggle flow vector visiblity"), MENU_DUMMY);
    glutAddMenuEntry(_("  x,y,z: toggle contour plot visibility along x, y and z axis"), MENU_DUMMY);
    glutAddMenuEntry(_("  {,}: load previous/next time Plot3D files"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("Misc"), MENU_DUMMY);
  glutAddMenuEntry(_("  A: toggle between plot types (device and HRRPUV)"), MENU_DUMMY);
  glutAddMenuEntry(_("  B: when the mouse is down, hide OBSTS and geometry - draw an outline instead"), MENU_DUMMY);
  glutAddMenuEntry(_("  e: toggle between view rotation types: scene centered 2 axis, 1 axis, 3 axis and eye centered"), MENU_DUMMY);
  if(global_scase.ntotal_blockages>0||global_scase.isZoneFireModel==1){
    glutAddMenuEntry(_("  g: toggle grid visibility modes"), MENU_DUMMY);
  }
  if(global_scase.devicecoll.ndeviceinfo > 0 && GetNumActiveDevices() > 0){
    glutAddMenuEntry("  j/ALT j: increase/decrease object size", MENU_DUMMY);
  }
  if(global_scase.have_cface_normals == CFACE_NORMALS_YES){
    glutAddMenuEntry(_("  n: display cface normal vectors"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("  M: toggle command line clipping"), MENU_DUMMY);
  if(global_scase.ntotal_blockages > 0){
    glutAddMenuEntry(_("  O: toggle blockage view (normal <--> outline)"), MENU_DUMMY);
    glutAddMenuEntry(_("  ALT o: cycle between blockage view types"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("  q: display blockage locations as specified by user or by FDS"), MENU_DUMMY);
  glutAddMenuEntry(_("  r/R: render the current scene to an image file"), MENU_DUMMY);
  glutAddMenuEntry("             r: image has the same resolution as the scene", MENU_DUMMY);
  {
    char render_label[1024];
    unsigned char deg360[] = {'3', '6', '0', 0};

    sprintf(render_label, "            R: image has %i times the resolution of of scene", MAX(2, resolution_multiplier));
    glutAddMenuEntry(render_label, MENU_DUMMY);
    sprintf(render_label, "    ALT R: %s view - all view directions are shown in a 1024x512 image", deg360);
    glutAddMenuEntry(render_label, MENU_DUMMY);
  }
  glutAddMenuEntry("  ALT r: toggle research mode (global min/max for coloring data, turn off axis label smoothing)", MENU_DUMMY);
  if(n_embedded_meshes > 0){
    glutAddMenuEntry(_("  ALT u: toggle coarse slice display in embedded mesh"), MENU_DUMMY);
  }
  glutAddMenuEntry("      U: toggle toggle between original and fast blockage drawing", MENU_DUMMY);
  glutAddMenuEntry(_("  W: toggle clipping modes - use ALT c to specify clipping planes"), MENU_DUMMY);
  glutAddMenuEntry(_("  -/space bar: decrement/increment time step, 2D contour planes, 3D contour levels"), MENU_DUMMY);
  glutAddMenuEntry("", MENU_DUMMY);
  glutAddMenuEntry(_("  ALT v: toggle projection mode (perspective and size preserving)"), MENU_DUMMY);
  if(clip_commandline==1){
    glutAddMenuEntry(_("  x/y/z: toggle lower x/y/z clip planes"), MENU_DUMMY);
    glutAddMenuEntry(_("  X/Y/Z: toggle upper x/y/z clip planes"), MENU_DUMMY);
  }
  {
    char *caseini_filename = CasePathCaseIni(&global_scase);
    if(caseini_filename != NULL && strlen(caseini_filename) > 0){
      char inilabel[512];

      sprintf(inilabel, "  #: save settings to %s", caseini_filename);
      glutAddMenuEntry(inilabel, MENU_DUMMY);
    }
    else{
      glutAddMenuEntry(_("  #: save settings (create casename.ini file)"), MENU_DUMMY);
    }
    FREEMEMORY(caseini_filename);
  }
  if(global_scase.ngeominfo){
    glutAddMenuEntry(_("  =: toggle vertex selected in examine geometry dialog"), MENU_DUMMY);
    glutAddMenuEntry(_("  Z: toggle rotation center between FDS and FDS+GEOM center"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("  !: snap scene to closest 45 degree view direction"), MENU_DUMMY);
  glutAddMenuEntry(_("  ~: level the scene"),2);
  glutAddMenuEntry(_("  &&: toggle line anti-aliasing (draw lines smoothly)"), MENU_DUMMY);
  glutAddMenuEntry(_("  /: toggle parallel particle loading"), MENU_DUMMY);

  /* --------------------------------mouse help menu -------------------------- */

  CREATEMENU(mousehelpmenu,HelpMenu);
  switch(rotation_type){
    case ROTATION_2AXIS:
      glutAddMenuEntry(_("horizontal/vertical: rotate about z, x axis"), MENU_DUMMY);
      break;
    case ROTATION_1AXIS:
      glutAddMenuEntry(_("horizontal: rotate about z axis"), MENU_DUMMY);
      break;
    case ROTATION_3AXIS:
      glutAddMenuEntry(_("horizontal/vertical: rotate about z, x axis (click near scene center)"), MENU_DUMMY);
      glutAddMenuEntry(_("clock/counter clockwise: rotate about y axis (click near scene edge)"), MENU_DUMMY);
      break;
    case EYE_CENTERED:
      glutAddMenuEntry(_("horizontal/vertical: rotate about user location"), MENU_DUMMY);
      break;
    default:
      assert(FFALSE);
      break;
  }
  switch(rotation_type){
    case EYE_CENTERED:
      break;
    case ROTATION_2AXIS:
    case ROTATION_1AXIS:
    case ROTATION_3AXIS:
      glutAddMenuEntry(_("CTRL horizontal/vertical: translate along x, y axis"), MENU_DUMMY);
      break;
    default:
      assert(FFALSE);
      break;
  }
  glutAddMenuEntry(_("ALT vertical: translate along z axis"), MENU_DUMMY);
  if(SHOW_gslice_data==1){
    glutAddMenuEntry(_("double-click: rotate/translate 3D node-centered slice"), MENU_DUMMY);
  }

  /* --------------------------------help menu -------------------------- */

  CREATEMENU(helpmenu,HelpMenu);
  GLUTADDSUBMENU(_("Online"),webhelpmenu);
  GLUTADDSUBMENU(_("Shortcuts"),keyboardhelpmenu);
  GLUTADDSUBMENU(_("Mouse"),mousehelpmenu);
  GLUTADDSUBMENU(_("About"),aboutmenu);

  /* --------------------------------hvac menu -------------------------- */

  if(global_scase.hvaccoll.nhvacinfo > 0 && global_scase.hvaccoll.hvacductvalsinfo!=NULL){
    char menulabel[1024];

    CREATEMENU(loadhvacmenu, LoadHVACMenu);
    strcpy(menulabel, "");
    if(global_scase.hvaccoll.hvacductvalsinfo->times!=NULL)strcat(menulabel, "*");
    strcat(menulabel, global_scase.hvaccoll.hvacductvalsinfo->file);
    glutAddMenuEntry(menulabel, MENU_HVAC_LOAD);
    glutAddMenuEntry("Unload",  MENU_HVAC_UNLOAD);
  }

  /* --------------------------------particle menu -------------------------- */

  if(global_scase.npartinfo>0){
    int ii;
    int doit = 0;

    if(global_scase.meshescoll.nmeshes==1){
      CREATEMENU(particlemenu,LoadParticleMenu);
      doit = 1;
    }
    if(doit == 1){
      for(ii = 0;ii < global_scase.npartinfo;ii++){
        char menulabel[1024];

        i = partorderindex[ii];
        if(global_scase.partinfo[i].loaded == 1){
          STRCPY(menulabel, "*");
          STRCAT(menulabel, global_scase.partinfo[i].menulabel);
        }
        else{
          STRCPY(menulabel, global_scase.partinfo[i].menulabel);
        }
        glutAddMenuEntry(menulabel, i);
      }
    }
    if(global_scase.meshescoll.nmeshes>1){
      char menulabel[1024];

      CREATEMENU(particlemenu,LoadParticleMenu);
      if(global_scase.npartinfo > 0){
        int part_load_state;

        PartLoadState(&part_load_state);
        strcpy(menulabel, "");
        if(part_load_state==1)strcat(menulabel, "#");
        if(part_load_state==2)strcat(menulabel, "*");
        strcat(menulabel, "Particles");
        glutAddMenuEntry(menulabel, MENU_PARTICLE_ALLMESHES);
      }
      glutAddMenuEntry("-",MENU_DUMMY);
      if(partfast==1)glutAddMenuEntry(_("*Fast loading"), MENU_PART_PARTFAST);
      if(partfast==0)glutAddMenuEntry(_("Fast loading"), MENU_PART_PARTFAST);
      if(global_scase.meshescoll.nmeshes>1){
      }
    }
    glutAddMenuEntry("Particle number/file size", MENU_PART_NUM_FILE_SIZE);
    glutAddMenuEntry("-",MENU_DUMMY);
    glutAddMenuEntry(_("Settings..."),     MENU_PART_SETTINGS);
    glutAddMenuEntry(_("Unload"), MENU_PARTICLE_UNLOAD_ALL);
  }

  if(global_scase.slicecoll.nvsliceinfo>0){

  //*** setup vector slice menus

    InitUnloadVSLiceMenu(&unloadvslicemenu);
    InitVSliceLoadMenu(&vsliceloadmenu, loadsubvslicemenu, unloadvslicemenu);
    if(nslicedups > 0){
      InitDuplicateVectorSliceMenu(&duplicatevectorslicemenu);
    }

  //*** setup multi vector slice menus

    if(have_multivslice==1){
      InitMultiVectorUnloadSliceMenu(&unloadmultivslicemenu);
      InitMultiVectorSubMenu(&loadsubmvslicemenu);
      InitMultiVectorLoadMenu(&loadmultivslicemenu, loadsubmvslicemenu, duplicatevectorslicemenu, vsliceloadmenu,
                              vectorsliceloadoptionmenu, unloadmultivslicemenu
                              ,&loadsubvectorslicexmenu, &loadsubvectorsliceymenu, &loadsubvectorslicezmenu, &loadsubvectorslicexyzmenu
      );
    }
  }

  //*** setup patch submenus for slices

  InitPatchSubMenus(&loadsubpatchmenu_s, &nsubpatchmenus_s);

  //*** setup slice menus

  if(global_scase.slicecoll.nsliceinfo>0||have_geom_slice_menus==1){
    InitUnloadSliceMenu(&unloadslicemenu);
    InitSliceSkipMenu(&sliceskipmenu);
  }

  //*** setup multi slice menus

  if(have_multislice==1||have_geom_slice_menus==1){
    nmultisliceloaded = 0;
    InitUnloadMultiSliceMenu(&unloadmultislicemenu);
    InitLoadMultiSubMenu(&loadsubmslicemenu, &nmultisliceloaded);

    if(have_multislice==1){
      InitDuplicateSliceMenu(&duplicateslicemenu);
    }
    InitLoadMultiSliceMenu(&loadmultislicemenu, loadsubmslicemenu, loadsubpatchmenu_s, nsubpatchmenus_s,
                           sliceskipmenu, sliceloadoptionmenu, duplicateslicemenu, loadslicemenu, nmultisliceloaded, unloadmultislicemenu
                           ,&loadsubslicexmenu, &loadsubsliceymenu, &loadsubslicezmenu, &loadsubslicexyzmenu
                           );
  }


/* --------------------------------unload and load 3d vol smoke menus -------------------------- */

    if(nvolsmoke3dloaded>0){
      CREATEMENU(unloadvolsmoke3dmenu,UnLoadVolsmoke3DMenu);
      if(nvolsmoke3dloaded>1){
        char vlabel[256];

        strcpy(vlabel,_("3D smoke (Volume rendered)"));
        glutAddMenuEntry(vlabel,UNLOAD_ALL);
      }
      for(i=0;i<global_scase.meshescoll.nmeshes;i++){
        meshdata *meshi;
        volrenderdata *vr;

        meshi = global_scase.meshescoll.meshinfo + i;
        vr = meshi->volrenderinfo;
        if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
        if(vr->loaded==0)continue;
        glutAddMenuEntry(meshi->label,i);
      }
    }
    if(nvolrenderinfo>0){
      char vlabel[256];
      CREATEMENU(loadvolsmoke3dmenu,LoadVolsmoke3DMenu);

      strcpy(vlabel,_("3D smoke (Volume rendered)"));
      glutAddMenuEntry(vlabel,LOAD_ALL);
      glutAddMenuEntry("-", MENU_DUMMY);
      glutAddMenuEntry(_("Settings..."), MENU_VOLSMOKE_SETTINGS);
      if(nvolsmoke3dloaded==1)glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
      if(nvolsmoke3dloaded>1)GLUTADDSUBMENU(_("Unload"),unloadvolsmoke3dmenu);
    }

    /* --------------------------------unload and load 3d smoke menus -------------------------- */

      if(nsmoke3dloaded>0){
        CREATEMENU(unloadsmoke3dmenu,UnLoadSmoke3DMenu);
        for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dtypes; i++){
          int j, doit, is_zlib;

          doit = 0;
          is_zlib = 0;
          for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
            if(global_scase.smoke3dcoll.smoke3dinfo[j].loaded==1&&global_scase.smoke3dcoll.smoke3dinfo[j].type==i){
              if(global_scase.smoke3dcoll.smoke3dinfo[j].compression_type == COMPRESSED_ZLIB){
                is_zlib = 1;
              }
              doit = 1;
              break;
            }
          }
          if(doit == 1){
            char smvmenulabel[256];

            strcpy(smvmenulabel, global_scase.smoke3dcoll.smoke3dtypes[i].longlabel);
            if(is_zlib == 1){
              strcat(smvmenulabel, "(ZLIB)");
            }
            glutAddMenuEntry(smvmenulabel, -(i + 1));
          }
        }
      }
    {
      if(global_scase.smoke3dcoll.nsmoke3dinfo>0){
        if(global_scase.meshescoll.nmeshes==1){
          CREATEMENU(loadsmoke3dmenu,LoadSmoke3DMenu);
        }

        int ii;
        for(ii = 0; ii<global_scase.smoke3dcoll.nsmoke3dtypes; ii++){
          if(global_scase.meshescoll.nmeshes>1){
            CREATEMENU(global_scase.smoke3dcoll.smoke3dtypes[ii].menu_id, LoadSmoke3DMenu);
          }
          for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
            char menulabel[256];
            smoke3ddata *smoke3di;

            smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
#ifdef pp_SMOKE3D_FORCE
            if(smoke3di->dummy == 1)continue;
#endif
            if(smoke3di->type!=ii)continue;
            strcpy(menulabel, "");
            if(smoke3di->loaded==1){
              strcat(menulabel, "*");
            }
            strcat(menulabel, smoke3di->menulabel);
            glutAddMenuEntry(menulabel, i);
          }
        }
        if(global_scase.meshescoll.nmeshes>1){
          CREATEMENU(loadsmoke3dmenu,LoadSmoke3DMenu);
          // multi mesh smoke menus items
          for(ii = 0; ii<global_scase.smoke3dcoll.nsmoke3dtypes; ii++){
            int jj;
            int ntotal, nloaded;
            char menulabel[256];
            int is_zlib;

            ntotal=0;
            nloaded=0;
            is_zlib = 0;
            for(jj=0;jj<global_scase.smoke3dcoll.nsmoke3dinfo;jj++){
#ifdef pp_SMOKE3D_FORCE
              if(global_scase.smoke3dcoll.smoke3dinfo[jj].dummy == 1)continue;
#endif
              if(global_scase.smoke3dcoll.smoke3dinfo[jj].type==ii){
                if(global_scase.smoke3dcoll.smoke3dinfo[jj].loaded==1)nloaded++;
                if(global_scase.smoke3dcoll.smoke3dinfo[jj].compression_type == COMPRESSED_ZLIB){
                  is_zlib = 1;
                }
                ntotal++;
              }
            }
            strcpy(menulabel,"");
            if(nloaded==ntotal){
              strcat(menulabel, "*");
            }
            else if(nloaded>0&&nloaded<ntotal){
              strcat(menulabel, "#");
            }
            strcat(menulabel, global_scase.smoke3dcoll.smoke3dtypes[ii].longlabel);
            strcat(menulabel, global_scase.smoke3dcoll.smoke3dtypes[ii].smoke3d->cextinct);
            if(is_zlib == 1){
              strcat(menulabel, "(ZLIB)");
            }
            glutAddMenuEntry(menulabel,-ii-100);
          }
        }

        glutAddMenuEntry("-", MENU_DUMMY3);
        glutAddMenuEntry(_("3D smoke file sizes"), MENU_SMOKE_FILE_SIZES);
        glutAddMenuEntry("-", MENU_DUMMY3);
        glutAddMenuEntry(_("Settings..."), MENU_SMOKE_SETTINGS);
        if(nsmoke3dloaded > 1){
          GLUTADDSUBMENU(_("Unload"), unloadsmoke3dmenu);
        }
        else{
          glutAddMenuEntry(_("Unload"), UNLOAD_ALL);
        }
      }
    }

    /* --------------------------------plot3d menu -------------------------- */

    if(global_scase.nplot3dinfo>0){
      plot3ddata *plot3dim1, *plot3di;
      char menulabel[1024];
      int ii;

      nloadsubplot3dmenu=1;
      for(ii=1;ii<global_scase.nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        im1 = plot3dorderindex[ii-1];
        plot3di = global_scase.plot3dinfo + i;
        plot3dim1 = global_scase.plot3dinfo + im1;
        if(ABS(plot3di->time-plot3dim1->time)>0.1)nloadsubplot3dmenu++;
      }
      NewMemory((void **)&loadsubplot3dmenu,nloadsubplot3dmenu*sizeof(int));
      for(i=0;i<nloadsubplot3dmenu;i++){
        loadsubplot3dmenu[i]=0;
      }

      nloadsubplot3dmenu=0;
      i = plot3dorderindex[0];
      plot3di = global_scase.plot3dinfo + i;
      CREATEMENU(loadsubplot3dmenu[nloadsubplot3dmenu],LoadPlot3dMenu);
      strcpy(menulabel,"");
      if(plot3di->loaded==1){
        strcat(menulabel,"*");
      }
      strcat(menulabel,plot3di->menulabel);
      glutAddMenuEntry(menulabel,i);
      nloadsubplot3dmenu++;

      for(ii=1;ii<global_scase.nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        im1 = plot3dorderindex[ii-1];
        plot3di = global_scase.plot3dinfo + i;
        plot3dim1 = global_scase.plot3dinfo + im1;
        if(ABS(plot3di->time-plot3dim1->time)>0.1){
          CREATEMENU(loadsubplot3dmenu[nloadsubplot3dmenu],LoadPlot3dMenu);
          nloadsubplot3dmenu++;
        }
        strcpy(menulabel,"");
        if(plot3di->loaded==1){
          strcat(menulabel,"*");
        }
        strcat(menulabel,plot3di->menulabel);
        glutAddMenuEntry(menulabel,i);
      }
      nloadsubplot3dmenu=0;
      nloadsubplot3dmenu=0;
      CREATEMENU(loadplot3dmenu,LoadPlot3dMenu);
      for(ii=0;ii<global_scase.nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        plot3di = global_scase.plot3dinfo + i;
        if(ii==0){
          int plot3d_load_state;
          char prefix[3];

          strcpy(menulabel,plot3di->longlabel);
          glutAddMenuEntry(menulabel,MENU_PLOT3D_DUMMY);
          Plot3DLoadState(plot3di->time, &plot3d_load_state);
          strcpy(prefix, "");
          if(plot3d_load_state==1)strcat(prefix,"#");
          if(plot3d_load_state==2)strcat(prefix,"*");
          sprintf(menulabel,"  %s%f", prefix, plot3di->time);
          TrimZeros(menulabel);
          strcat(menulabel," s");
          if(global_scase.meshescoll.nmeshes>1){
            glutAddMenuEntry(menulabel,-100000+nloadsubplot3dmenu);
          }
          else{
            strcpy(menulabel,"  ");
            if(plot3di->loaded==1){
              strcat(menulabel,"*");
            }
            strcat(menulabel,plot3di->menulabel);
            glutAddMenuEntry(menulabel,i);
          }
          nloadsubplot3dmenu++;
        }
        if(ii!=0){
          int plot3d_load_state;

          i = plot3dorderindex[ii];
          im1 = plot3dorderindex[ii-1];
          plot3di = global_scase.plot3dinfo + i;
          plot3dim1 = global_scase.plot3dinfo + im1;
          if(strcmp(plot3di->longlabel,plot3dim1->longlabel)!=0){
            glutAddMenuEntry(plot3di->longlabel,MENU_PLOT3D_DUMMY);
          }
          if(ABS(plot3di->time-plot3dim1->time)>0.1){
            char prefix[3];

            Plot3DLoadState(plot3di->time, &plot3d_load_state);
            strcpy(prefix, "");
            if(plot3d_load_state==1)strcat(prefix,"#");
            if(plot3d_load_state==2)strcat(prefix,"*");
            sprintf(menulabel,"  %s%f",prefix, plot3di->time);
            TrimZeros(menulabel);
            strcat(menulabel," s");
            if(global_scase.meshescoll.nmeshes>1){
              glutAddMenuEntry(menulabel,-100000+nloadsubplot3dmenu);
            }
            else{
              strcpy(menulabel,"  ");
              if(plot3di->loaded==1){
                strcat(menulabel,"*");
              }
              strcat(menulabel,plot3di->menulabel);
              glutAddMenuEntry(menulabel,i);
            }
            nloadsubplot3dmenu++;
          }
        }
        if(ii==global_scase.nplot3dinfo-1){
          glutAddMenuEntry("-", MENU_PLOT3D_DUMMY);
          glutAddMenuEntry(_("Settings..."), MENU_PLOT3D_SETTINGS);
          glutAddMenuEntry(_("Unload"), UNLOAD_ALL);
        }
      }
    }

/* --------------------------------load patch menu -------------------------- */

    if(global_scase.npatchinfo>0){
      int ii;

      nloadpatchsubmenus=0;

      if(global_scase.meshescoll.nmeshes>1&&loadpatchsubmenus==NULL){
        NewMemory((void **)&loadpatchsubmenus,global_scase.npatchinfo*sizeof(int));
      }

      if(global_scase.meshescoll.nmeshes>1){
        CREATEMENU(loadpatchsubmenus[nloadpatchsubmenus],LoadBoundaryMenu);
        nloadpatchsubmenus++;
      }
      else{
        CREATEMENU(loadpatchmenu,LoadBoundaryMenu);
      }

      for(ii=0;ii<global_scase.npatchinfo;ii++){
        patchdata *patchim1, *patchi;
        char menulabel[1024];

        i = patchorderindex[ii];
        patchi = global_scase.patchinfo + i;
        if(ii>0){
          patchim1 = global_scase.patchinfo + patchorderindex[ii-1];
          if(global_scase.meshescoll.nmeshes>1&&strcmp(patchim1->label.longlabel,patchi->label.longlabel)!=0){
            CREATEMENU(loadpatchsubmenus[nloadpatchsubmenus],LoadBoundaryMenu);
            nloadpatchsubmenus++;
          }
        }
        if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label,"INCLUDE_GEOM")!=0){
          STRCPY(menulabel, "");
          if(patchi->loaded==1)STRCAT(menulabel,"*");
          STRCAT(menulabel,patchi->menulabel);
          glutAddMenuEntry(menulabel,i);
        }
      }
      if(nboundaryslicedups>0){
        GLUTADDSUBMENU(_("Duplicate boundary slices"),duplicateboundaryslicemenu);
      }
      glutAddMenuEntry("-",MENU_DUMMY3);
      glutAddMenuEntry(_("Settings..."), MENU_BOUNDARY_SETTINGS);
      glutAddMenuEntry(_("Unload"),UNLOAD_ALL);

      if(nboundaryslicedups>0){
        CREATEMENU(duplicateboundaryslicemenu,LoadBoundaryMenu);
        if(boundaryslicedup_option == SLICEDUP_KEEPALL){
          glutAddMenuEntry(_("  *keep all"), MENU_KEEP_ALL);
        }
        else{
          glutAddMenuEntry(_("  keep all"), MENU_KEEP_ALL);
        }
        if(boundaryslicedup_option == SLICEDUP_KEEPFINE){
          glutAddMenuEntry(_("  *keep fine"), MENU_KEEP_FINE);
        }
        else{
          glutAddMenuEntry(_("  keep fine"), MENU_KEEP_FINE);
        }
        if(boundaryslicedup_option == SLICEDUP_KEEPCOARSE){
          glutAddMenuEntry(_("  *keep coarse"), MENU_KEEP_COARSE);
        }
        else{
          glutAddMenuEntry(_("  keep coarse"), MENU_KEEP_COARSE);
        }
      }
      if(global_scase.meshescoll.nmeshes>1){

// count patch submenus

        nloadsubpatchmenu_b=0;
        for(ii=0;ii<global_scase.npatchinfo;ii++){
          int im1;
          patchdata *patchi, *patchim1;

          i = patchorderindex[ii];
          if(ii>0){
            im1 = patchorderindex[ii-1];
            patchim1=global_scase.patchinfo + im1;
          }
          patchi = global_scase.patchinfo + i;
          if(ii==0||strcmp(patchi->menulabel_base,patchim1->menulabel_base)!=0){
            nloadsubpatchmenu_b++;
          }
        }

// create patch submenus

        if(nloadsubpatchmenu_b > 0){
          NewMemory((void **)&loadsubpatchmenu_b, nloadsubpatchmenu_b * sizeof(int));
          NewMemory((void **)&nsubpatchmenus_b, nloadsubpatchmenu_b * sizeof(int));
        }
        for(i=0;i<nloadsubpatchmenu_b;i++){
          loadsubpatchmenu_b[i]=0;
          nsubpatchmenus_b[i]=0;
        }

        iloadsubpatchmenu_b=0;
        for(ii=0;ii<global_scase.npatchinfo;ii++){
          int im1;
          patchdata *patchi, *patchim1;

          i = patchorderindex[ii];
          if(ii>0){
            im1 = patchorderindex[ii-1];
            patchim1=global_scase.patchinfo + im1;
          }
          patchi = global_scase.patchinfo + i;
          if(ii==0||strcmp(patchi->menulabel_base,patchim1->menulabel_base)!=0){
            CREATEMENU(loadsubpatchmenu_b[iloadsubpatchmenu_b],LoadBoundaryMenu);
            iloadsubpatchmenu_b++;
          }
          if(ii==0||strcmp(patchi->menulabel_suffix,patchim1->menulabel_suffix)!=0){
            if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label,"INCLUDE_GEOM")!=0){
              char menulabel[256];

              nsubpatchmenus_b[iloadsubpatchmenu_b-1]++;
              strcpy(menulabel, patchi->menulabel_suffix);
              if(patchi->compression_type == COMPRESSED_ZLIB){
                strcat(menulabel, "(ZLIB)");
              }
              glutAddMenuEntry(menulabel,-i-10);
            }
          }
        }

// call patch submenus from main patch menu

        CREATEMENU(loadpatchmenu,LoadBoundaryMenu);
        iloadsubpatchmenu_b=0;
        for(ii=0;ii<global_scase.npatchinfo;ii++){
          int im1;
          patchdata *patchi, *patchim1;

          i = patchorderindex[ii];
          if(ii>0){
            im1 = patchorderindex[ii-1];
            patchim1=global_scase.patchinfo + im1;
          }
          patchi = global_scase.patchinfo + i;
          if(ii==0||strcmp(patchi->menulabel_base,patchim1->menulabel_base)!=0){
            int nsubmenus;

            nsubmenus = nsubpatchmenus_b[iloadsubpatchmenu_b];
            if(nsubmenus>1){
              GLUTADDSUBMENU(patchi->menulabel_base,loadsubpatchmenu_b[iloadsubpatchmenu_b]);
            }
            else if(nsubmenus==1){
              char menulabel[1024];
              int patch_load_state;

              PatchLoadState(patchi, &patch_load_state);
              strcpy(menulabel, "");
              if(patch_load_state==1)strcat(menulabel, "#");
              if(patch_load_state==2)strcat(menulabel, "*");
              strcat(menulabel, patchi->label.longlabel);
              if(patchi->compression_type == COMPRESSED_ZLIB){
                strcat(menulabel, "(ZLIB)");
              }
              glutAddMenuEntry(menulabel,-i-10);
            }
            iloadsubpatchmenu_b++;
          }
        }
      }
//*** these same lines also appear above (except for nmeshes>1 line)
      glutAddMenuEntry("-",MENU_DUMMY3);

      if(nboundaryslicedups>0){
        GLUTADDSUBMENU(_("Duplicate boundary slices"),duplicateboundaryslicemenu);
      }
      glutAddMenuEntry(_("Settings..."), MENU_BOUNDARY_SETTINGS);
      glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
    }

/* --------------------------------load iso menu -------------------------- */

    if(global_scase.nisoinfo>0){
      int ii;

      if(global_scase.nisoinfo>0){
        if(isosubmenus==NULL){
          NewMemory((void **)&isosubmenus,global_scase.nisoinfo*sizeof(int));
        }
        nisosubmenus=0;

        CREATEMENU(isosubmenus[nisosubmenus],LoadIsoMenu);
        nisosubmenus++;
      }

      if(global_scase.meshescoll.nmeshes==1){
        CREATEMENU(loadisomenu,LoadIsoMenu);
      }
      for(ii=0;ii<global_scase.nisoinfo;ii++){
        isodata *iso1, *iso2;
        char menulabel[1024];

        i = isoorderindex[ii];
        if(ii>0){
          iso1 = global_scase.isoinfo + isoorderindex[ii-1];
          iso2 = global_scase.isoinfo + isoorderindex[ii];
          if(global_scase.meshescoll.nmeshes>1&&strcmp(iso1->surface_label.longlabel,iso2->surface_label.longlabel)!=0){
            CREATEMENU(isosubmenus[nisosubmenus],LoadIsoMenu);
            nisosubmenus++;
          }
        }
        if(global_scase.isoinfo[i].loaded==1){
          STRCPY(menulabel,"*");
          STRCAT(menulabel,global_scase.isoinfo[i].menulabel);
        }
        else{
          STRCPY(menulabel,global_scase.isoinfo[i].menulabel);
        }
        glutAddMenuEntry(menulabel,i);
      }

      {
        int useitem;
        isodata *isoi, *isoj;

        if(global_scase.meshescoll.nmeshes>1){
          CREATEMENU(loadisomenu,LoadIsoMenu);
          for(i=0;i<global_scase.nisoinfo;i++){
            int j;

            useitem=i;
            isoi = global_scase.isoinfo + i;
            for(j=0;j<i;j++){
              isoj = global_scase.isoinfo + j;
              if(strcmp(isoi->surface_label.longlabel,isoj->surface_label.longlabel)==0){
                useitem=-1;
                break;
              }
            }
            if(useitem!=-1){
              char menulabel[1024];
              int load_state;

              IsoLoadState(isoi, &load_state);
              strcpy(menulabel,"");
              if(load_state==1)strcat(menulabel, "#");
              if(load_state==2)strcat(menulabel, "*");
              strcat(menulabel, isoi->surface_label.longlabel);
              glutAddMenuEntry(menulabel,-useitem-10);
            }
          }
          glutAddMenuEntry("-", MENU_DUMMY3);
        }
      }
      glutAddMenuEntry(_("Settings..."), MENU_ISO_SETTINGS);
      glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
    }

/* --------------------------------zone menu -------------------------- */

    if(global_scase.nzoneinfo>0){
      CREATEMENU(zonemenu,ZoneMenu);
      for(i=0;i<global_scase.nzoneinfo;i++){
        zonedata *zonei;
        char menulabel[1024];
        int n;

        zonei = global_scase.zoneinfo + i;
        STRCPY(menulabel, "");
        if(zonei->loaded==1)STRCAT(menulabel,"*");
        STRCAT(menulabel,zonei->file);
        STRCAT(menulabel,", ");
        for(n=0;n<3;n++){
          STRCAT(menulabel,zonei->label[n].shortlabel);
          STRCAT(menulabel,", ");
        }
        STRCAT(menulabel,zonei->label[3].shortlabel);
        glutAddMenuEntry(menulabel,i);
      }
      glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
    }

/* -------------------------------- compress menu -------------------------- */

#ifdef pp_COMPRESS
    if(smokezippath != NULL && (global_scase.npatchinfo > 0 || global_scase.smoke3dcoll.nsmoke3dinfo > 0 || global_scase.slicecoll.nsliceinfo > 0)){
    CREATEMENU(compressmenu,CompressMenu);
    glutAddMenuEntry(_("Compression options"),MENU_DUMMY);  // -c
    if(overwrite_all==1){
      glutAddMenuEntry(_("  *Overwrite compressed files"),MENU_OVERWRITECOMPRESS);  // -f
    }
    else{
      glutAddMenuEntry(_("  Overwrite compressed files"),MENU_OVERWRITECOMPRESS);  // -f
    }
    if(compress_autoloaded==1){
      glutAddMenuEntry(_("  *Compress only autoloaded files"),MENU_COMPRESSAUTOLOAD);  // -f
    }
    else{
      glutAddMenuEntry(_("  Compress only autoloaded files"),MENU_COMPRESSAUTOLOAD);  // -f
    }
    glutAddMenuEntry("-",MENU_DUMMY);  // -c
    glutAddMenuEntry(_("Compress now"),MENU_COMPRESSNOW);
    glutAddMenuEntry(_("Erase compressed files"),MENU_ERASECOMPRESS);  // -c
    glutAddMenuEntry(_("Settings..."), MENU_COMPRESS_SETTINGS);
  }
#endif


/* --------------------------------inisub menu -------------------------- */
  {
    int n_inifiles;
    inifiledata *inifile;

    n_inifiles=0;
    for(inifile=first_inifile.next;inifile->next!=NULL;inifile=inifile->next){
      if(inifile->file!=NULL&&FILE_EXISTS(inifile->file)==YES){
        n_inifiles++;
      }
    }
    if(n_inifiles>0){
      CREATEMENU(inisubmenu,IniSubMenu);
      char *caseini_filename = CasePathCaseIni(&global_scase);
      if(FILE_EXISTS(caseini_filename)==YES){
        glutAddMenuEntry(caseini_filename,MENU_READCASEINI);
      }
      FREEMEMORY(caseini_filename);
      for(inifile=first_inifile.next;inifile->next!=NULL;inifile=inifile->next){
        if(inifile->file!=NULL&&FILE_EXISTS(inifile->file)==YES){
          glutAddMenuEntry(inifile->file,inifile->id);
        }
      }
    }
  }

/* --------------------------------smokeviewini menu -------------------------- */

    CREATEMENU(smokeviewinimenu,SmokeviewIniMenu);
   {
    inifiledata *inifile;
    int n_inifiles;

    n_inifiles=0;
    for(inifile=first_inifile.next;inifile->next!=NULL;inifile=inifile->next){
      if(inifile->file!=NULL&&FILE_EXISTS(inifile->file)==YES){
        n_inifiles++;
      }
    }
    char *global_ini_path = GetSystemIniPath();
    char *user_ini_path = GetUserIniPath();
    char *caseini_filename = CasePathCaseIni(&global_scase);
    if( n_inifiles>0||FILE_EXISTS(user_ini_path)==YES||FILE_EXISTS(caseini_filename)==YES||FILE_EXISTS(global_ini_path)==YES){
      if(n_inifiles==0){
        glutAddMenuEntry(_("Read ini files"),MENU_READINI);
      }
      else{
        GLUTADDSUBMENU(_("Read ini files"),inisubmenu);
      }
    }
    FREEMEMORY(caseini_filename);
    FREEMEMORY(global_ini_path);
    FREEMEMORY(user_ini_path);
   }


    {
      char caselabel[255];

      char *caseini_filename = CasePathCaseIni(&global_scase);
      STRCPY(caselabel, _("Save settings (this case - "));
      STRCAT(caselabel, caseini_filename);
      STRCAT(caselabel, ")");
      FREEMEMORY(caseini_filename);

      glutAddMenuEntry(caselabel,MENU_WRITECASEINI);
    }

    glutAddMenuEntry("-", MENU_DUMMY);
    if(global_scase.devicecoll.ndeviceinfo>0){
      glutAddMenuEntry(_("Read .svo files"),MENU_READSVO);
    }
    glutAddMenuEntry("Save settings (all cases - smokeview.ini)", MENU_WRITEINI);
    glutAddMenuEntry("Revert settings to installation defaults", MENU_REVERT_WRITEINI);
    glutAddMenuEntry(_("Settings..."), MENU_CONFIG_SETTINGS);

    CREATEMENU(reloadmenu,ReloadMenu);
    glutAddMenuEntry(_("smv"), RELOAD_SMV_FILE);
    glutAddMenuEntry("-", MENU_DUMMY);
    glutAddMenuEntry(_("When:"), MENU_DUMMY);
    glutAddMenuEntry(_("  now"),RELOAD_SWITCH);
    if(periodic_reload_value==1)glutAddMenuEntry(_("   *every minute"),1);
    if(periodic_reload_value!=1)glutAddMenuEntry(_("   every minute"),1);
    if(periodic_reload_value==5)glutAddMenuEntry(_("   *every 5 minutes"),5);
    if(periodic_reload_value!=5)glutAddMenuEntry(_("   every 5 minutes"),5);
    if(periodic_reload_value==10)glutAddMenuEntry(_("   *every 10 minutes"),10);
    if(periodic_reload_value!=10)glutAddMenuEntry(_("   every 10 minutes"),10);
    glutAddMenuEntry(_("Cancel"),STOP_RELOADING);


    {
      int nscripts;

      nscripts=0;
      if(script_recording==NULL){
        scriptfiledata *scriptfile;

        for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
          char *file;
          int len;

          file=scriptfile->file;
          if(file==NULL)continue;
          len = strlen(file);
          if(len<=0)continue;
          if(FILE_EXISTS(file)==NO)continue;

          nscripts++;
        }

        if(nscripts>0){
          CREATEMENU(scriptlistmenu,ScriptMenu);
          for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
            char *file;
            int len;
            char menulabel[1024];

            file=scriptfile->file;
            if(file==NULL)continue;
            len = strlen(file);
            if(len<=0)continue;
            if(FILE_EXISTS(file)==NO)continue;

            strcpy(menulabel,"  ");
            strcat(menulabel,file);
            glutAddMenuEntry(menulabel,scriptfile->id);
          }
          CREATEMENU(scriptsteplistmenu,ScriptMenu2);
          for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
            char *file;
            int len;
            char menulabel[1024];

            file=scriptfile->file;
            if(file==NULL)continue;
            len = strlen(file);
            if(len<=0)continue;
            if(FILE_EXISTS(file)==NO)continue;

            strcpy(menulabel,"  ");
            strcat(menulabel,file);
            glutAddMenuEntry(menulabel,scriptfile->id);
          }
        }
      }

      CREATEMENU(scriptrecordmenu,ScriptMenu);
      if(script_recording==NULL){
        glutAddMenuEntry(_("Start"),SCRIPT_START_RECORDING);
        glutAddMenuEntry(_("Start (disable file loading)"),SCRIPT_START_RECORDING2);
      }
      glutAddMenuEntry(_("Stop"),SCRIPT_STOP_RECORDING);

      CREATEMENU(scriptmenu,ScriptMenu);
      if(nscripts>0){
        GLUTADDSUBMENU(_("Run"),scriptlistmenu);
        GLUTADDSUBMENU(_("Step (using ^)"),scriptsteplistmenu);
        if(script_step==1)glutAddMenuEntry(_("Continue"),SCRIPT_CONTINUE);
        if(script_step==1||current_script_command!=NULL)glutAddMenuEntry(_("Cancel"),SCRIPT_CANCEL);
      }
      GLUTADDSUBMENU(_("Record"),scriptrecordmenu);
      glutAddMenuEntry(_("Settings..."), MENU_SCRIPT_SETTINGS);
    }

    CREATEMENU(fileinfomenu, LoadUnloadMenu);
    if(cache_file_data==1)glutAddMenuEntry(_("*Cache file data"), CACHE_FILE_DATA);
    if(cache_file_data==0)glutAddMenuEntry(_("Cache file data"), CACHE_FILE_DATA);
    if(showfiles==1)glutAddMenuEntry("*Show file names", SHOWFILES);
    if(showfiles==0)glutAddMenuEntry("Show file names", SHOWFILES);
    glutAddMenuEntry(                       "Show file bounds:", MENU_DUMMY);
    if(bounds_each_mesh==1)glutAddMenuEntry("  *for each mesh", COMPUTE_SMV_BOUNDS);
    if(bounds_each_mesh==0)glutAddMenuEntry("   for each mesh", COMPUTE_SMV_BOUNDS);
    if(show_bound_diffs==1)glutAddMenuEntry("  *differences (fds - smokeview)", SHOW_BOUND_DIFFS);
    if(show_bound_diffs==0)glutAddMenuEntry("   differences (fds - smokeview)", SHOW_BOUND_DIFFS);

/* --------------------------------loadunload menu -------------------------- */
    {
      char loadmenulabel[100];
      char steplabel[100];

      CREATEMENU(loadunloadmenu,LoadUnloadMenu);
      strcpy(steplabel,_("error: steplabel not defined"));

      // 3d smoke
      int n_smoke3d_present = 0;
#ifdef pp_SMOKE3D_FORCE
      for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
        smoke3ddata *smoke3di;

        smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
        if(smoke3di->dummy == 1)continue;
        n_smoke3d_present = 1;
        break;
      }
#else
      if(global_scase.smoke3dcoll.nsmoke3dinfo > 0)n_smoke3d_present = 1;
#endif
      if(n_smoke3d_present==1){
        strcpy(loadmenulabel,_("3D smoke"));
        if(tload_step > 1){
          sprintf(steplabel,"/Skip %i",tload_skip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadsmoke3dmenu);
      }

      // volume rendered smoke

      if(nvolrenderinfo>0&&global_scase.smokediff==0){
        char vlabel[256];

        strcpy(vlabel,_("3D smoke (Volume rendered)"));
        GLUTADDSUBMENU(vlabel,loadvolsmoke3dmenu);
      }

      // terrain

      if(global_scase.manual_terrain==1&&global_scase.nterraininfo>0){
 //       GLUTADDSUBMENU(_("Terrain"),loadterrainmenu);
      }

      // slice

      if(have_multislice==1||have_geom_slice_menus==1){
        strcpy(loadmenulabel, _("Slice"));
        if(tload_step > 1){
          sprintf(steplabel, "/Skip %i", tload_skip);
          strcat(loadmenulabel, steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel, loadmultislicemenu);
      }
      else if(global_scase.slicecoll.nsliceinfo > 0){
        strcpy(loadmenulabel, "Slice");
        if(tload_step > 1){
          sprintf(steplabel, "/Skip %i", tload_skip);
          strcat(loadmenulabel, steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel, loadslicemenu);
      }

      // vector slice

      if(have_multivslice==1){
        strcpy(loadmenulabel,_("Vector Slice"));
        if(tload_step > 1){
          sprintf(steplabel,"/Skip %i",tload_skip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadmultivslicemenu);
      }
      else if(global_scase.slicecoll.nvsliceinfo>0){
        strcpy(loadmenulabel,_("Vector slice"));
        if(tload_step > 1){
          sprintf(steplabel,"/Skip %i",tload_skip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,vsliceloadmenu);
      }

      // isosurface

      if(global_scase.nisoinfo>0){
        strcpy(loadmenulabel,"Isosurface");
        if(tload_step > 1){
          sprintf(steplabel,"/Skip %i",tload_skip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadisomenu);
      }

      // boundary

      if(global_scase.npatchinfo>0){
        strcpy(loadmenulabel,"Boundary");
        if(tload_step > 1){
          sprintf(steplabel,"/Skip %i",tload_skip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadpatchmenu);
      }

      // particle

      if(global_scase.npartinfo>0){
        strcpy(loadmenulabel,"Particles");
        if(tload_step > 1){
          sprintf(steplabel,"/Skip Frame %i",tload_skip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,particlemenu);
      }

      // plot3d

      if(global_scase.nplot3dinfo>0)GLUTADDSUBMENU(_("Plot3D"),loadplot3dmenu);

      // zone fire

      if(global_scase.nzoneinfo>0){
        strcpy(loadmenulabel,"Zone fire");
        GLUTADDSUBMENU(loadmenulabel,zonemenu);
      }

      if(glui_active==1){
        glutAddMenuEntry("-",MENU_DUMMY);
      }
      if(global_scase.hvaccoll.nhvacinfo > 0 && global_scase.hvaccoll.hvacductvalsinfo!=NULL){
        GLUTADDSUBMENU("HVAC", loadhvacmenu);
      }
      GLUTADDSUBMENU(_("Configuration files"),smokeviewinimenu);
      GLUTADDSUBMENU(_("Scripts"),scriptmenu);
#ifdef pp_COMPRESS
      if(smokezippath!=NULL&&(global_scase.npatchinfo>0||global_scase.smoke3dcoll.nsmoke3dinfo>0||global_scase.slicecoll.nsliceinfo>0)){
        GLUTADDSUBMENU(_("Compression"),compressmenu);
      }
#endif
      GLUTADDSUBMENU(_("Misc"),fileinfomenu);
      {
        char menulabel[1024];

        strcpy(menulabel,"");
        if(redirect==1)strcat(menulabel,"*");
        strcat(menulabel,"Redirect messages to ");
        char *log_filename = CasePathLogFile(&global_scase);
        strcat(menulabel,log_filename);
        FREEMEMORY(log_filename);
        glutAddMenuEntry(menulabel,REDIRECT);
      }

      GLUTADDSUBMENU(_("Reload"),reloadmenu);
      if(load_only_when_unloaded==1){
        glutAddMenuEntry(_("*Load a file only if unloaded"), LOAD_WHEN_LOADED);
      }
      else{
        glutAddMenuEntry(_("Load a file only if unloaded"), LOAD_WHEN_LOADED);
      }
      glutAddMenuEntry(_("Unload all"),UNLOADALL);
    }

    /* --------------------------------main menu -------------------------- */
    if(trainer_mode==1){
      CREATEMENU(trainerviewmenu,TrainerViewMenu);
      if(AnySmoke()==1){
        if(trainerload==1)glutAddMenuEntry(_("*Realistic"),MENU_TRAINER_smoke);
        if(trainerload!=1)glutAddMenuEntry(_("Realistic"),MENU_TRAINER_smoke);
      }
      if(AnySlices("TEMPERATURE")==1){
        if(trainerload==2)glutAddMenuEntry(_("*Temperature"),MENU_TRAINER_temp);
        if(trainerload!=2)glutAddMenuEntry(_("Temperature"),MENU_TRAINER_temp);
      }
      if(AnySlices("oxygen")==1||
         AnySlices("oxygen VOLUME FRACTION")==1){
        if(trainerload==3)glutAddMenuEntry(_("*Oxygen"),MENU_TRAINER_oxy);
        if(trainerload!=3)glutAddMenuEntry(_("Oxygen"),MENU_TRAINER_oxy);
      }
      glutAddMenuEntry(_("Clear"),MENU_TRAINER_CLEAR);
    }

    CREATEMENU(mainmenu,MainMenu);
    if(trainer_mode==0){
      GLUTADDSUBMENU(_("Load/Unload"),loadunloadmenu);
      GLUTADDSUBMENU(_("Show/Hide"),showhidemenu);
      GLUTADDSUBMENU(_("Options"),optionmenu);
      GLUTADDSUBMENU(_("Dialogs"),dialogmenu);
      GLUTADDSUBMENU(_("Help"),helpmenu);
      glutAddMenuEntry(_("Quit"),MENU_MAIN_QUIT);
    }
    if(trainer_active==1){
      if(trainer_mode==1){
        glutAddMenuEntry(_("Smokeview menus"),MENU_MAIN_TRAINERTOGGLE);
      }
      else{
        glutAddMenuEntry(_("Trainer menus"),MENU_MAIN_TRAINERTOGGLE);
      }
    }
#ifdef _DEBUG
    PRINTF("Updated menus count: %i nmenus: %i\n", menu_count++, nmenus);
    if(updatemenu==1)PRINTF("menu updated again\n");
#endif

}

/* ------------------ MenuStatusCB ------------------------ */

void MenuStatusCB(int status, int x, int y){
  float *eye_xyz;

  menustatus=status;

  /* keep scene from "bouncing" around when leaving a menu */
  start_xyz0[0]=x;
  start_xyz0[1]=y;
  /*touring=0;*/
  mouse_down_xy0[0]=x; mouse_down_xy0[1]=y;
  eye_xyz = camera_current->eye;
  eye_xyz0[0]=eye_xyz[0];
  eye_xyz0[1]=eye_xyz[1];
  eye_xyz0[2]=eye_xyz[2];
}
