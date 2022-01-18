#include "options.h"
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
#include "IOvolsmoke.h"

int       part_file_count;
FILE_SIZE part_load_size;
float     part_load_time;

#ifdef WIN32
#include <direct.h>
#endif

#define PRINT_PROCESSTIMES(file_count,load_size,load_time) \
  if(file_count>1){\
    if(load_size>1000000000){\
      PRINTF("Processed %.1f GB in %.1f s\n",(float)load_size/1000000000.,load_time);\
    }\
    else if(load_size>1000000){\
      PRINTF("Processed %.1f MB in %.1f s\n",(float)load_size/1000000.,load_time);\
    }\
    else{\
      PRINTF("Processed %.0f kB in %.1f s\n",(float)load_size/1000.,load_time);\
    }\
  }

#define PRINT_LOADTIMES(file_count,load_size,load_time) \
  if(file_count>1){\
    if(load_size>1000000000){\
      PRINTF("Loaded %.1f GB in %.1f s\n",(float)load_size/1000000000.,load_time);\
    }\
    else if(load_size>1000000){\
      PRINTF("Loaded %.1f MB in %.1f s\n",(float)load_size/1000000.,load_time);\
    }\
    else{\
      PRINTF("Loaded %.0f kB in %.1f s\n",(float)load_size/1000.,load_time);\
    }\
    printf("\n");\
  }

#define GEOM_Vents                   15
#define GEOM_Compartments            16
#define GEOM_Outline                  3
#define GEOM_TriangleCount           14
#define GEOM_ShowAll                 11
#define GEOM_HideAll                 13
#define GEOM_BOUNDING_BOX_ALWAYS     10
#define GEOM_BOUNDING_BOX_MOUSE_DOWN  9

#define MENU_TERRAIN_SHOW_SURFACE      -1
#define MENU_TERRAIN_SHOW_LINES        -2
#define MENU_TERRAIN_SHOW_POINTS       -3
#define MENU_TERRAIN_BOUNDING_BOX      -5
#define MENU_TERRAIN_BOUNDING_BOX_AUTO -6

#define MENU_KEEP_ALL -2
#define MENU_KEEP_FINE -3
#define MENU_KEEP_COARSE -4

#define MENU_SLICECOLORDEFER -5

#define MENU_SLICE_FILE_SIZES -9

#define MENU_OPTION_TRAINERMENU 2

#define MENU_UPDATEBOUNDS             -6
#define MENU_BNDF_SHOW_MESH_INTERFACE -8
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

#define MENU_READCASEINI -1
#define MENU_READINI 1
#define MENU_WRITEINI 2
#define MENU_WRITECASEINI 3
#define MENU_READSVO 4
#define MENU_CONFIG_SETTINGS 5

#define MENU_DUMMY2 -1

#define MENU_PLOT3D_DUMMY 997
#define MENU_PLOT3D_Z 1
#define MENU_PLOT3D_Y 2
#define MENU_PLOT3D_X 3
#define MENU_PLOT3D_CONT 4
#define MENU_PLOT3D_SHOWALL 5
#define MENU_PLOT3D_HIDEALL 6

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

#define MENU_EVAC_UNLOADALL -1
#define MENU_EVAC_DUMMY -2

#define MENU_PARTICLE_UNLOAD_ALL -1
#define MENU_PARTICLE_DUMMY -2
#define MENU_PARTICLE_ALLMESHES -11

#define MENU_UNLOADEVAC_UNLOADALL -1

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

#define MENU_ISOSHOW_SOLID 1
#define MENU_ISOSHOW_OUTLINE 2
#define MENU_ISOSHOW_POINTS 3
#define MENU_ISOSHOW_SMOOTH 4
#define MENU_ISOSHOW_NORMALS 5

#define MENU_ZONE_2DTEMP2 21
#define MENU_ZONE_2DTEMP 6
#define MENU_ZONE_2DHAZARD 5
#define MENU_ZONE_3DSMOKE 7
#define MENU_ZONE_HORIZONTAL 1
#define MENU_ZONE_VERTICAL 2
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
#define MENU_SHOWSLICE_FEDAREA -14

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

#define MENU_SHOWHIDE_EVAC 13
#define MENU_SHOWHIDE_PRINT 16
#define MENU_SHOWHIDE_PARTICLES 1
#define MENU_SHOWHIDE_SENSOR 9
#define MENU_SHOWHIDE_SENSOR_NORM 14
#define MENU_SHOWHIDE_OFFSET 12

#define MENU_UNITS_RESET -1
#define MENU_UNITS_SHOWALL -3
#define MENU_UNITS_HMS -2

#define MENU_HELP_ISSUES -1
#define MENU_HELP_DOWNLOADS -2
#define MENU_HELP_DOCUMENTATION -3
#define MENU_HELP_FDSWEB -4
#define MENU_HELP_FORUM -5
#define MENU_HELP_RELEASENOTES -6

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
    if( _chdir( smv_directory )   ){
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
  char label[100], label2[100], *labelptr, *labelptr2;

  if(val>0.0)labelptr=GetFloatFileSizeLabel(val, label);
  if(val2>0.0)labelptr2=GetFloatFileSizeLabel(val2, label2);
  if(val>0.0){
    if(val2>0.0){
      printf("  %s/compressed: %s/%s\n", type,labelptr,labelptr2);
    }
    else{
      printf("  %s: %s\n", type,labelptr);
    }
  }
}
/* ------------------ GetFileSizes ------------------------ */

void GetFileSizes(void){
  int i;

  printf("\n");
  if(nsmoke3dinfo>0){
    float hrrpuv = 0.0, soot = 0.0, temp = 0.0, co2 = 0.0;
    float hrrpuv2 = 0.0, soot2 = 0.0, temp2 = 0.0, co22 = 0.0;

    for(i = 0; i<nsmoke3dinfo; i++){
      smoke3ddata *smoke3di;
      FILE_SIZE file_size, compressed_file_size;

      smoke3di = smoke3dinfo+i;

      file_size = GetFileSizeSMV(smoke3di->reg_file);
      compressed_file_size = GetFileSizeSMV(smoke3di->comp_file);

      switch(smoke3di->type){
      case SOOT:
        soot  += file_size;
        soot2 += compressed_file_size;
        break;
      case HRRPUV:
        hrrpuv  += file_size;
        hrrpuv2 += compressed_file_size;
        break;
      case TEMP:
        temp  += file_size;
        temp2 += compressed_file_size;
        break;
      case CO2:
        co2  += file_size;
        co22 += compressed_file_size;
        break;
      default:
        ASSERT(FFALSE);
        break;
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
  if(npartinfo>0){
    float part = 0.0;
    char label[100];

    for(i = 0; i<npartinfo; i++){
      partdata *parti;
      FILE_SIZE file_size;

      parti = partinfo+i;
      file_size = GetFileSizeSMV(parti->file);
      part += file_size;
    }
    printf("particle files: %s\n", GetFloatFileSizeLabel(part, label));
  }
  else{
    printf("particle files: no files found\n");
  }

  printf("\n");
  if(npatchinfo>0){
    patchdata **patchlist;
    float sum = 0.0, compressed_sum=0;

    printf("boundary files sizes: \n");
    NewMemory((void **)&patchlist, npatchinfo*sizeof(patchdata *));
    for(i = 0; i<npatchinfo; i++){
      patchlist[i] = patchinfo+i;
    }
    qsort((patchdata **)patchlist, (size_t)npatchinfo, sizeof(patchdata *), ComparePatchLabels);
    for(i = 0; i<npatchinfo; i++){
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
      if(i==npatchinfo-1){
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
  for(i = 0; i < nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->loaded == 1)smoke3di->display = 0;
  }
  for(i = 0; i < nisoinfo; i++){
    isodata *isoi;

    isoi = isoinfo + i;
    if(isoi->loaded == 1)isoi->display = 0;
  }
}

/* ------------------ HideAllSlices ------------------------ */

void HideAllSlices(void){
  int i;

  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  for(i = 0; i < nsliceinfo; i++){
    sliceinfo[i].display = 0;
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ ShowAllSmoke ------------------------ */

void ShowAllSmoke(void){
  int i;
  for(i = 0; i < nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->loaded == 1)smoke3di->display = 1;
  }
  for(i = 0; i < nisoinfo; i++){
    isodata *isoi;

    isoi = isoinfo + i;
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
    UpdateShowSliceInObst(ONLY_IN_GAS);
    break;
  case MENU_SHOWSLICE_IN_GASANDSOLID:
    UpdateShowSliceInObst(GAS_AND_SOLID);
    break;
  case MENU_SHOWSLICE_IN_SOLID:
    UpdateShowSliceInObst(ONLY_IN_SOLID);
    break;
  case -12:
    offset_slice = 1 - offset_slice;
    break;
  case -14:
    show_fed_area = 1 - show_fed_area;
    break;
  default:
    if(value<=-20){
      value = -20 - value;
      ShowBoundaryMenu(value+1000);
    }
    else{
      multislicedata *mslicei;
      slicedata *sd;

      mslicei = multisliceinfo+value;
      mdisplay = 0;

      sd = sliceinfo+mslicei->islices[0];
      if(slicefile_labelindex==sd->slicefile_labelindex){
        if(plotstate!=DYNAMIC_PLOTS){
          plotstate = DYNAMIC_PLOTS;
          mdisplay = 1;
        }
        else{
          if(mslicei->display == -1){
            mdisplay = 0;
          }
          else {
            mdisplay = 1 - mslicei->display;
          }
        }
      }
      else{
        plotstate = DYNAMIC_PLOTS;
        sd = sliceinfo+mslicei->islices[0];
        slicefile_labelindex = sd->slicefile_labelindex;
        mdisplay = 1;
      }
      for(i = 0; i<mslicei->nslices; i++){
        sd = sliceinfo+mslicei->islices[i];
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
    for(i = 0; i < nsliceinfo; i++){
      slicedata *slicei;

      slicei = sliceinfo + i;
      slicei->display = 0;
      if(slicei->loaded == 0)continue;
      if(
        (type1 != NULL&&STRCMP(slicei->label.longlabel, type1) == 0) ||
        (type2 != NULL&&STRCMP(slicei->label.longlabel, type2) == 0)
        ){
        sliceinfo[i].display = 1;
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
    ASSERT(FFALSE);
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
  UpdatePlot3dListIndex();
#define BOUND_PERCENTILE_DRAW          120
  Plot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
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
    UpdatePlot3dListIndex();
  }
}

/* ------------------ LabelMenu ------------------------ */

void LabelMenu(int value){
  updatemenu=1;
  if(value == MENU_DUMMY)return;
  GLUTPOSTREDISPLAY;
  switch(value){
  case MENU_LABEL_SETTINGS:
    ShowGluiDisplay(DIALOG_DISPLAY);
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
    else {
      visColorbarHorizontal = 1 - visColorbarHorizontal;
      if (visColorbarHorizontal == 1)visColorbarVertical = 0;
    }
    UpdateColorbarControls();
    UpdateColorbarControls2();
    vis_colorbar = GetColorbarState();
    break;
  case MENU_LABEL_timebar:
    visTimebar=1-visTimebar;
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
    if(ntickinfo>0)visFDSticks=1;
    visgridloc=1;
    visHRRlabel=1;
    show_firecutoff=1;
    visFramelabel=1;
    if(hrrinfo != NULL&&hrrinfo->display != 1)UpdateHRRInfo(1);
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
    visHRRlabel=0;
    show_firecutoff=0;
    if(hrrinfo != NULL&&hrrinfo->display != 0)UpdateHRRInfo(0);
    if(ntickinfo>0)visFDSticks=0;
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
    UpdateVisAxisLabels();
    break;
   case MENU_LABEL_textlabels:
     visLabels = 1 - visLabels;
     break;
   case MENU_LABEL_timelabel:
     visTimelabel=1-visTimelabel;
   case MENU_LABEL_frametimelabel:
     visFrameTimelabel = 1-visFrameTimelabel;
     UpdateFrameTimelabel();
     break;
   case MENU_LABEL_framelabel:
     visFramelabel=1-visFramelabel;
     if(visFramelabel==1){
       visHRRlabel=0;
       UpdateHRRInfo(visHRRlabel);
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
     visHRRlabel=1-visHRRlabel;
     UpdateHRRInfo(visHRRlabel);
     break;
   case MENU_LABEL_firecutoff:
     show_firecutoff=1-show_firecutoff;
     break;
   case MENU_LABEL_userticks:
     visUSERticks = 1 - visUSERticks;
     break;
   default:
     ASSERT(FFALSE);
     break;
  }
  SetLabelControls();
}

/* ------------------ SmokeColorbarMenu ------------------------ */

void SmokeColorbarMenu(int value){
  if(value==MENU_DUMMY)return;
  updatemenu=1;

  value = CLAMP(value, 0, ncolorbars - 1);
  fire_colorbar_index=value;
  fire_colorbar = colorbarinfo + value;
  UpdateRGBColors(COLORBAR_INDEX_NONE);
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
      UpdateColorbarFlip();
      break;
    case COLORBAR_FLIP:
      colorbar_flip=1-colorbar_flip;
      UpdateColorbarFlip();
      break;
    case COLORBAR_RESET:
      show_extreme_mindata=0;
      show_extreme_maxdata=0;
      colorbar_flip=0;
      contour_type=SHADED_CONTOURS;
      setbw=0;
      UpdateExtreme();
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      break;
    case COLORBAR_HIGHLIGHT_BELOW:
      show_extreme_mindata=1-show_extreme_mindata;
      UpdateExtreme();
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      break;
    case COLORBAR_HIGHLIGHT_ABOVE:
      show_extreme_maxdata=1-show_extreme_maxdata;
      UpdateExtreme();
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      break;
    case COLORBAR_TOGGLE_BW_DATA:
      setbwdata = 1 - setbwdata;
      if(setbwdata==1){
        colorbartype_save=colorbartype;
        ColorbarMenu(bw_colorbar_index);
      }
      else{
        ColorbarMenu(colorbartype_save);
      }
      IsoBoundCB(ISO_COLORS);
      break;
    case COLORBAR_TOGGLE_BW:
      setbw=1-setbw;
      InitRGB();
      SetLabelControls();
      break;
    case USE_LIGHTING:
      use_lighting = 1 - use_lighting;
      UpdateUseLighting();
      break;
   case COLORBAR_TRANSPARENT:
     use_transparency_data=1-use_transparency_data;
     UpdateRGBColors(COLORBAR_INDEX_NONE);
     SetLabelControls();
     UpdateTransparency();
     break;
   case COLORBAR_CONTINUOUS:
     contour_type=SHADED_CONTOURS;
     UpdateRGBColors(COLORBAR_INDEX_NONE);
     break;
   case COLORBAR_STEPPED:
     contour_type=STEPPED_CONTOURS;
     UpdateRGBColors(COLORBAR_INDEX_NONE);
     break;
   case COLORBAR_LINES:
     contour_type=LINE_CONTOURS;
     UpdateRGBColors(COLORBAR_INDEX_NONE);
     break;
   case COLORBAR_HORIZONTAL:
     LabelMenu(MENU_LABEL_colorbar_horizontal);
     break;
   case COLORBAR_VERTICAL:
     LabelMenu(MENU_LABEL_colorbar_vertical);
     break;
   default:
     ASSERT(FFALSE);
     break;
   }
  }
  if(value>=0){
    colorbartype=value;
    iso_colorbar_index=value;
    iso_colorbar = colorbarinfo + iso_colorbar_index;
    update_texturebar=1;
    UpdateListIsoColorobar();
    selectedcolorbar_index2=colorbartype;
    UpdateCurrentColorbar(colorbarinfo+colorbartype);
    UpdateColorbarType();
    UpdateColorbarList2();
    if(colorbartype == bw_colorbar_index){
      setbwdata = 1;
    }
    else{
      setbwdata = 0;
    }
    IsoBoundCB(ISO_COLORS);
    SetLabelControls();
  }
  if(value>-10){
    UpdateRGBColors(COLORBAR_INDEX_NONE);
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
      show_3dsmokefiles=1;
      Smoke3DShowMenu(SET_SMOKE3D);
      break;
    case HIDE_ALL:
      show_3dsmokefiles=0;
      Smoke3DShowMenu(SET_SMOKE3D);
      break;
    case TOGGLE_SMOKE3D:
      show_3dsmokefiles=1-show_3dsmokefiles;
      Smoke3DShowMenu(SET_SMOKE3D);
    break;
    case SET_SMOKE3D:
      for(i=0;i<nsmoke3dinfo;i++){
        smoke3di = smoke3dinfo + i;
        if(smoke3di->loaded==1)smoke3di->display=show_3dsmokefiles;
      }
    break;
    default:
      ASSERT(FFALSE);
    }
  }
  else{
    smoke3di = smoke3dinfo + value;
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
    UpdateGluiIsotype();
    break;
   case MENU_ISOSHOW_ALLSOLID:
    transparent_state=ALL_SOLID;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = surfinfo + nsurfinfo + 1 + i;
      surfi->transparent_level=1.0;
    }
    use_transparency_data=0;
    break;
   case MENU_ISOSHOW_ALLTRANSPARENT:
    transparent_state=ALL_TRANSPARENT;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = surfinfo + nsurfinfo + 1 + i;
      surfi->transparent_level=transparent_level;
    }
    use_transparency_data=1;
    break;
   case MENU_ISOSHOW_MINSOLID:
    transparent_state=MIN_SOLID;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = surfinfo + nsurfinfo + 1 + i;
      surfi->transparent_level=transparent_level;
    }
    surfinfo[nsurfinfo+1].transparent_level=1.0;
    use_transparency_data=1;
    break;
   case MENU_ISOSHOW_MAXSOLID:
    transparent_state=MAX_SOLID;
    if(loaded_isomesh==NULL)break;
    for(i=0;i<loaded_isomesh->nisolevels;i++){
      surfdata *surfi;

      surfi = surfinfo + nsurfinfo + 1 + i;
      surfi->transparent_level=transparent_level;
    }
    use_transparency_data=1;
    surfinfo[nsurfinfo+1+loaded_isomesh->nisolevels-1].transparent_level=1.0;
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
     isoi = isoinfo + value - 1000;          // hope that is enough!!
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
      for(i=0;i<nisoinfo;i++){
        isoinfo[i].display=show_isofiles;
      }
      UpdateShow();
    }
  }
  UpdateIsoShowLevels();
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
    for(i=0;i<nvsliceinfo;i++){
      vd = vsliceinfo+i;
      if(vd->loaded==0)continue;
      vd->display= showall_slices;
    }
    UpdateTimes();
    return;
  }
  if(value==MENU_SHOWSLICE_IN_GAS){
    UpdateShowSliceInObst(ONLY_IN_GAS);
    return;
  }
  else if(value==MENU_SHOWSLICE_IN_GASANDSOLID){
    UpdateShowSliceInObst(GAS_AND_SOLID);
    return;
  }
  else if(value==MENU_SHOWSLICE_IN_SOLID){
    UpdateShowSliceInObst(ONLY_IN_SOLID);
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
  vd = vsliceinfo + value;
  if(slicefile_labelindex==sliceinfo[vd->ival].slicefile_labelindex){
    if(plotstate!=DYNAMIC_PLOTS){
      plotstate=DYNAMIC_PLOTS;
      vd->display=1;
    }
    else{
      vd->display = 1 - vd->display;
    }
    if(vd->iu!=-1){
      slicedata *sd;

      sd = sliceinfo+vd->iu;
      sd->vloaded=vd->display;
    }
    if(vd->iv!=-1){
      slicedata *sd;

      sd = sliceinfo+vd->iv;
      sd->vloaded=vd->display;
    }
    if(vd->iw!=-1){
      slicedata *sd;

      sd = sliceinfo+vd->iw;
      sd->vloaded=vd->display;
    }
    if(vd->ival!=-1){
      slicedata *sd;

      sd = sliceinfo+vd->ival;
      sd->vloaded=vd->display;
    }
  }
  else{
    slicefile_labelindex = sliceinfo[vd->ival].slicefile_labelindex;
    vd->display=1;
  }
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  UpdateShow();
}

/* ------------------ SliceSKipMenu ------------------------ */

void SliceSkipMenu(int value){
  slice_skip = value;
  UpdateSliceSkip();
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
      for(i=0;i<nsliceinfo;i++){
        sliceinfo[i].display=showall_slices;
      }
      break;
    case MENU_SHOWSLICE_IN_GAS:
      UpdateShowSliceInObst(ONLY_IN_GAS);
      break;
    case MENU_SHOWSLICE_IN_GASANDSOLID:
      UpdateShowSliceInObst(GAS_AND_SOLID);
      break;
    case MENU_SHOWSLICE_IN_SOLID:
      UpdateShowSliceInObst(ONLY_IN_SOLID);
      break;
    case MENU_SHOWSLICE_OFFSET:
      offset_slice=1-offset_slice;
      break;
    case MENU_SHOWSLICE_TERRAIN:
      planar_terrain_slice=1-planar_terrain_slice;
      break;
    case MENU_SHOWSLICE_FEDAREA:
      show_fed_area=1-show_fed_area;
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

    sd = sliceinfo + value;
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
    if(value < nsliceinfo - nfedinfo){
      colorbardata *fed_colorbar;
      int reset_colorbar = 0;

      fed_colorbar = GetColorbar(default_fed_colorbar);
      if(fed_colorbar != NULL&&fed_colorbar - colorbarinfo == colorbartype)reset_colorbar = 1;
      if(reset_colorbar == 1)ColorbarMenu(colorbartype_save);
    }
    else{
      colorbardata *fed_colorbar;

      fed_colorbar = GetColorbar(default_fed_colorbar);
      if(fed_colorbar!=NULL){
        if(current_colorbar!=fed_colorbar)colorbartype_save = current_colorbar - colorbarinfo;
        ColorbarMenu(fed_colorbar - colorbarinfo);
      }
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
   UpdateRGBColors(COLORBAR_INDEX_NONE);
   SetLabelControls();
   SetColorControls();
   UpdateBackgroundFlip(background_flip);
   UpdateBackgroundFlip2(background_flip);
   break;
  case MENU_SHOWHIDE_EVAC:
    if(plotstate==DYNAMIC_PLOTS){
      visEvac=1-visEvac;
    }
    else{
      plotstate=DYNAMIC_PLOTS;
      visEvac=1;
    }
    UpdateTimes();
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
    ASSERT(FFALSE);
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
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ DialogMenu ------------------------ */

void DialogMenu(int value){
  GLUTPOSTREDISPLAY;
  switch(value){
  case DIALOG_SHOOTER:
    ShowGluiShooter();
    break;
  case DIALOG_TRAINER:
    ShowGluiTrainer();
    break;
  case DIALOG_DEVICE:
    ShowGluiDevice();
    break;
  case DIALOG_3DSMOKE:
  case DIALOG_AUTOLOAD:
  case DIALOG_BOUNDS:
  case DIALOG_CONFIG:
  case DIALOG_SCRIPT:
  case DIALOG_SHOWFILES:
  case DIALOG_SMOKEZIP:
  case DIALOG_TIME:
    ShowGluiBounds(value);
    break;
  case DIALOG_MOTION:
  case DIALOG_RENDER:
  case DIALOG_MOVIE:
  case DIALOG_MOVIE_BATCH:
  case DIALOG_SCALING:
  case DIALOG_VIEW:
  case DIALOG_WINDOW:
    ShowGluiMotion(value);
    break;
  case DIALOG_TICKS:
  case DIALOG_FONTS:
  case DIALOG_LABELS:
  case DIALOG_DISPLAY:
    ShowGluiDisplay(value);
    break;
  case DIALOG_TOUR_SHOW:
    ShowGluiTour();
    break;
  case DIALOG_TOUR_HIDE:
    HideGluiTour();
    break;
  case DIALOG_CLIP:
    ShowGluiClip();
    break;
  case DIALOG_STEREO:
    ShowGluiStereo();
    break;
  case DIALOG_COLORBAR:
    showcolorbar_dialog=1-showcolorbar_dialog;
    if(showcolorbar_dialog==1){
      ShowGluiColorbar();
    }
    if(showcolorbar_dialog==0){
      HideGluiColorbar();
    }
    break;
  case DIALOG_GEOMETRY:
    showedit_dialog=1-showedit_dialog;
    if(showedit_dialog==1){
      if(fds_filein!=NULL&&updategetobstlabels==1){
        CheckMemoryOff;
        GetObstLabels(fds_filein);
        CheckMemoryOn;
        updategetobstlabels=0;
      }
      visBlocksSave=visBlocks;
      ShowGluiGeometry();
      visBlocks=visBLOCKNormal;
    }
    if(showedit_dialog==0){
      HideGluiGeometry();
      visBlocks=visBlocksSave;
    }
    UpdateTrainerOutline();
    break;
  case DIALOG_SHRINKALL:
    ShrinkDialogs();
    break;
  case DIALOG_HIDEALL:
    showcolorbar_dialog = 0;
    HideGluiShooter();
    HideGluiDisplay();
    HideGluiBounds();
    HideGluiMotion();
    HideGluiTour();
    HideGluiClip();
    HideGluiStereo();
    HideGluiColorbar();
    if(showedit_dialog==1)DialogMenu(DIALOG_GEOMETRY);
    HideGluiTrainer();
    HideGluiDevice();
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  updatemenu=1;
#ifdef pp_REFRESH
  refresh_glui_dialogs = 1;
  SetMainWindow();
  RefreshGluiDialogs();
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
      UpdateProjectionType();
    }
  }
  else if(zoomindex==UPDATE_PROJECTION){
    camera_current->projection_type=projection_type;
    UpdateProjectionType();
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
      UpdateProjectionType();
    }
  }
  camera_current->zoom=zoom;
  UpdateGluiZoom();
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
    ShowGluiDisplay(DIALOG_FONTS);
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
    ASSERT(FFALSE);
  }
  GluiUpdateFontIndex();
  SetLabelControls();
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
    SetLabelControls();

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
      ShowGluiTrainer();
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
    ShowGluiMotion(DIALOG_VIEW);
    break;
  case MENU_SIZEPRESERVING:
    projection_type = 1 - projection_type;
    SceneMotionCB(PROJECTION);
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
      AddListView(view_label);
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
    SetStartupView();
    break;
  default:
    ASSERT(value>=-5);
    if(value<100000){
      ResetGluiView(value);
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
  if(var==2){
    int i;

    use_geom_factors = 1 - use_geom_factors;
    updatemenu = 1;
    UpdateUseGeomFactors();
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
      ASSERT(FFALSE);
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
    Enable360Zoom();
    screenWidth  = saveW/scale;
    screenHeight = saveH/scale;
    SetScreenSize(&screenWidth, &screenHeight);
    ResizeWindow(saveW/scale, saveH/scale);
    ResetRenderResolution(&width_low, &height_low, &width_high, &height_high);
    UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
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
  int i,n;
  meshdata *meshi;

  if(value==MENU_DUMMY)return;
  updatemenu=1;
  if(value>=11000)return;
  if(opengldefined==1){
    GLUTPOSTREDISPLAY;
  }
  if(value>=10000&&value<=10005){
    resolution_multiplier=value-10000;
    UpdateResolutionMultiplier();
    return;
  }
  switch(value){
  case MENU_RENDER_SETTINGS:
    ShowGluiMotion(DIALOG_RENDER);
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
      int json_option;

      json_option = HTML_CURRENT_TIME;
      if(value==RenderJSONALL)json_option = HTML_ALL_TIMES;
      if(Obst2Data(htmlobst_filename)!=0){
        printf("blockage data output to %s\n", htmlobst_filename);
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
      break;
    }
    break;
  case RenderHTML:
    Smv2Html(html_filename,   HTML_CURRENT_TIME, FROM_SMOKEVIEW, VR_NO);
#ifdef pp_HTML_VR
    Smv2Html(htmlvr_filename, HTML_CURRENT_TIME, FROM_SMOKEVIEW, VR_YES);
#endif
    break;
  case RenderHTMLALL:
    Smv2Html(html_filename,   HTML_ALL_TIMES, FROM_SMOKEVIEW, VR_NO);
#ifdef pp_HTML_VR
    Smv2Html(htmlvr_filename, HTML_ALL_TIMES, FROM_SMOKEVIEW, VR_YES);
#endif
    break;
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
    if(touring==1){
      rendertourcount=0;
    }
    if(render_skip == RENDER_CURRENT_SINGLE){
      UpdateFrameNumber(0);
    }
    else{
      if(stept == 0)Keyboard('t', FROM_SMOKEVIEW);
      ResetItimes0();
      for(i=0;i<nsliceinfo;i++){
        sd=sliceinfo+i;
        sd->itime=0;
      }
      frame_index=first_frame_index;
      for(i=0;i<nmeshes;i++){
        meshi=meshinfo+i;
        meshi->patch_itime=0;
      }
    }
    RenderState(RENDER_ON);
    UpdateTimeLabels();
    FlowDir=1;
    for(n=0;n<nglobal_times;n++){
      render_frame[n]=0;
    }
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"RENDERALL\n");
      fprintf(scriptoutstream," %i\n",render_skip);
      fprintf(scriptoutstream,"\n");
    }
    render_times = RENDER_ALLTIMES;
    break;
  case RenderLABELframenumber:
    render_label_type=RENDER_LABEL_FRAMENUM;
    UpdateGluiFileLabel(render_label_type);
    break;
  case RenderLABELtime:
    render_label_type=RENDER_LABEL_TIME;
    UpdateGluiFileLabel(render_label_type);
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
     ASSERT(FFALSE);
     break;
  }
  UpdateResolutionMultiplier();
}

/* ------------------ EvacShowMenu ------------------------ */

void EvacShowMenu(int value){
  partdata *parti;
  int i;

  if(nevac==0)return;
  if(value==MENU_DUMMY)return;
  if(value<0){
    value = -value;
    value--;
    parti = partinfo + value;
    parti->display = 1 - parti->display;
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    return;
  }
  if(plotstate==DYNAMIC_PLOTS){
    switch(value){
    case 3:
      visEvac=1;
      for(i=0;i<npartinfo;i++){
        parti = partinfo + i;
        if(parti->loaded==0||parti->evac==0)continue;
        parti->display=1;
      }
      break;
    case 4:
      visEvac=0;
      for(i=0;i<npartinfo;i++){
        parti = partinfo + i;
        if(parti->loaded==0||parti->evac==0)continue;
        parti->display=0;
      }
      break;
      default:
        ASSERT(FFALSE);
        break;
    }
  }
  else{
    switch(value){
    case 3:
      visEvac=1;
      for(i=0;i<npartinfo;i++){
        parti = partinfo + i;
        if(parti->loaded==0||parti->evac==0)continue;
        parti->display=1;
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }
  updatemenu=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  GLUTPOSTREDISPLAY;
}

/* ------------------ ParticleShowMenu ------------------------ */

void ParticleShowMenu(int value){
  partdata *parti;
  int i;

  if(npartinfo==0)return;
  if(value==MENU_DUMMY)return;
  if(value<0){
    value = -value;
    value--;
    parti = partinfo + value;
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
        for(i=0;i<npartinfo;i++){
          parti = partinfo + i;
          if(parti->loaded==0||parti->evac==1)continue;
          parti->display=1;
        }
        break;
      case MENU_PARTSHOW_STATIC:
        break;
      case MENU_PARTSHOW_HIDEALL:
        visSprinkPart=0;
        visSmokePart=0;
        for(i=0;i<npartinfo;i++){
          parti = partinfo + i;
          if(parti->loaded==0||parti->evac==1)continue;
          parti->display=0;
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    /*
    for(i=0;i<npartinfo;i++){
      parti = partinfo + i;
      if(parti->loaded==0||parti->evac==1)continue;
      parti->display_droplet=0;
      parti->display_smoke=0;
      if(visSmokePart!=0)parti->display_smoke=1;
      if(visSprinkPart==1)parti->display_droplet=1;
    }
    */
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
        for(i=0;i<npartinfo;i++){
          parti = partinfo + i;
          if(parti->loaded==0)continue;
          parti->display=1;
        }
        break;
      case 5:
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    /*
    for(i=0;i<npartinfo;i++){
      parti = partinfo + i;
      if(parti->loaded==0||parti->evac==1)continue;
      parti->display_droplet=0;
      parti->display_smoke=0;
      if(visSmokePart!=0)parti->display_smoke=1;
      if(visSprinkPart==1)parti->display_droplet=1;
    }
    */
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
      ASSERT(FFALSE);
      break;
    }
    UpdateGluiPlot3Dtype();
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
      OPENURL("https://groups.google.com/forum/?fromgroups#!forum/fds-smv");
      break;
    case MENU_HELP_ISSUES:
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
      ASSERT(FFALSE);
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
  GLUTPOSTREDISPLAY;
}

/* ------------------ TextureShowMenu ------------------------ */

void TextureShowMenu(int value){
  texturedata *texti;
  int i;
  int texturedisplay=0;
  int texture_flag=0;

  updatefacelists=1;
  if(value>=0){
    texti = textureinfo + value;
    texti->display = 1-texti->display;
    if(texti->display==1)texturedisplay=1;
    for(i=0;i<ntextureinfo;i++){
      texti = textureinfo + i;
      if(texti->loaded==0||texti->used==0)continue;
      if(texti->display==0){
        showall_textures=0;
        texture_flag=1;
        break;
      }
    }
    if(texture_flag==0)showall_textures=1;
  }
  else{
    switch(value){
    case MENU_TEXTURE_SHOWALL:
      for(i=0;i<ntextureinfo;i++){
        texti = textureinfo + i;
        if(texti->loaded==0||texti->used==0)continue;
        texti->display=1;
        texturedisplay=1;
      }
      showall_textures=1;
      break;
    case MENU_TEXTURE_HIDEALL:
      for(i=0;i<ntextureinfo;i++){
        texti = textureinfo + i;
        if(texti->loaded==0||texti->used==0)continue;
        texti->display=0;
      }
      showall_textures=0;
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }
  visGeomTextures=0;
  if(texturedisplay==1){
    for(i=0;i<ngeominfo;i++){
      geomdata *geomi;
      surfdata *surf;
      texturedata *textii=NULL;

      geomi = geominfo + i;
      surf = geomi->surfgeom;
      if(terrain_textures!=NULL){
        textii = terrain_textures+iterrain_textures;
      }
      else{
        if(surf!=NULL)textii = surf->textureinfo;
      }
      if(textii!=NULL&&textii->display==1){
        visGeomTextures=1;
        break;
      }
    }
    if(value!=visBLOCKOutline&&value!=visBLOCKSolidOutline&&value!=visBLOCKHide){
      BlockageMenu(visBLOCKAsInput);
    }
  }
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
          ASSERT(FFALSE);
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
     for(i=0;i<nplot3dinfo;i++){
       if(plot3dinfo[i].loaded==1)plot3dinfo[i].display=show_plot3dfiles;
     }
     break;
   default:
     if(value>=1000){
       if(plotstate==STATIC_PLOTS){
         plot3dinfo[value-1000].display=1-plot3dinfo[value-1000].display;
       }
       else{
         plot3dinfo[value-1000].display=1;
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
  UpdateGLuiGridLocation();
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
        ASSERT(FFALSE);
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
        ASSERT(FFALSE);
        break;
    }
    break;
  default:
    ASSERT(FFALSE);
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
    ShowGluiBounds(DIALOG_SMOKEZIP);
    break;
  case MENU_ERASECOMPRESS:
    erase_all=1;
    overwrite_all=0;
    UpdateOverwrite();
    CompressSVZip();
    break;
  case MENU_OVERWRITECOMPRESS:
    erase_all=0;
    overwrite_all=1-overwrite_all;
    UpdateOverwrite();
    break;
  case MENU_COMPRESSNOW:
    erase_all=0;
    CompressSVZip();
    break;
  case MENU_COMPRESSAUTOLOAD:
    compress_autoloaded=1-compress_autoloaded;
    UpdateOverwrite();
    break;
  default:
    ASSERT(FFALSE);
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
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    break;
  case MENU_WRITEINI:
    WriteIni(GLOBAL_INI,NULL);
    break;
  case MENU_WRITECASEINI:
    WriteIni(LOCAL_INI,NULL);
    break;
  case MENU_READSVO:
    InitObjectDefs();
    break;
  case MENU_DUMMY:
    break;
  case MENU_CONFIG_SETTINGS:
    ShowGluiBounds(DIALOG_CONFIG);
    break;
  default:
    ASSERT(FFALSE);
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
  UpdateScriptStep();
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
      ShowGluiBounds(DIALOG_SCRIPT);
      break;
    case SCRIPT_STEP:
      script_step=1-script_step;
      break;
    case SCRIPT_CANCEL:
      script_defer_loading = 0;
      iso_multithread = iso_multithread_save;
      current_script_command=NULL;
      runscript=0;
      first_frame_index=0;
      script_startframe=-1;
      script_skipframe=-1;
      script_step=0;
      GluiScriptEnable();
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
      UpdateScriptStart();
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
        AddScriptList(script_recording->file,script_recording->id);
        script_recording=NULL;
      }
      if(scriptoutstream!=NULL){
        fclose(scriptoutstream);
        scriptoutstream=NULL;
        PRINTF("Script recorder off\n");
      }
      UpdateScriptStop();
      break;
    default:
      for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
        char *file;

        file=scriptfile->file;
        if(file==NULL)continue;
        if(scriptfile->id!=value)continue;
        error_code= CompileScript(file);
        if(error_code==0){
      //    ReadIni(NULL);
          StartScript();
        }
        else{
          fprintf(stderr,"*** Error (fatal): unable to open script file");
          if(file!=NULL)fprintf(stderr,": %s",file);
          fprintf(stderr,"\n");
          if(from_commandline==1)SMV_EXIT(1);
        }
        break;
      }
      break;
  }
}

/* ------------------ ScriptMenu ------------------------ */
#ifdef pp_LUA
// prototype here rather than create a header file for lua_api
int load_script(char *filename);
void LuaScriptMenu(int value){
  luascriptfiledata *luascriptfile;

  if(value==MENU_DUMMY)return;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  switch(value){
    default:
      for(luascriptfile=first_luascriptfile.next;luascriptfile->next!=NULL;luascriptfile=luascriptfile->next){
        char *file;

        file=luascriptfile->file;
        if(file==NULL)continue;
        if(luascriptfile->id!=value)continue;
        // set the runluascript variable to true, this must be done before
        // calling loadscript, as it both uses and modifies this variable
        runluascript = 1;
        // load the script
        load_script(luascriptfile->file);
        // let the display callback do its work, i.e. just return to the main
        // loop, DisplayCB will run through the script.
        break;
      }
      break;
  }
}
#endif

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
    ReadSMVDynamic(smv_filename);
    break;
  case RELOAD_MODE_INCREMENTAL:
    load_incremental = 1;
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

/* ------------------ AboutMenu ------------------------ */

void AboutMenu(int value){
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
    meshi = meshinfo + value;
    vr = &(meshi->volrenderinfo);
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
    ShowBoundsDialog(DLG_3DSMOKE);
  }
  updatemenu = 1;
  ForceIdle();
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ UnloadAllSliceFiles ------------------------ */

void UnloadAllSliceFiles(char *longlabel){
  int i, errorcode;

  for(i=0; i<nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = vsliceinfo+i;
    if(vslicei->loaded==0)continue;
    if(vslicei->val==NULL)continue;
    if(longlabel==NULL||strcmp(vslicei->val->label.longlabel,longlabel)!=0){
      ReadVSlice(i,ALL_FRAMES, NULL, UNLOAD,&errorcode);
    }
  }
  for(i=0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(slicei->loaded==0||slicei->vloaded==1)continue;
    if(longlabel==NULL||strcmp(slicei->label.longlabel,longlabel)!=0){
      ReadSlice("", i, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR, &errorcode);
    }
  }
}

/* ------------------ ReloadAllVectorSliceFiles ------------------------ */

void ReloadAllVectorSliceFiles(void){
  int i, errorcode;

  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    slicei->uvw = 0;
  }
  for(i = 0; i<nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = vsliceinfo+i;
    vslicei->reload = 0;
    if(vslicei->loaded==1&&vslicei->display==1)vslicei->reload = 1;
    if(vslicei->iu>=0)sliceinfo[vslicei->iu].uvw = 1;
    if(vslicei->iv>=0)sliceinfo[vslicei->iv].uvw = 1;
    if(vslicei->iw>=0)sliceinfo[vslicei->iw].uvw = 1;
  }

    //*** reload vector slice files

  for(i = 0; i<nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = vsliceinfo+i;
    if(vslicei->reload==1){
      ReadVSlice(i, ALL_FRAMES, NULL, UNLOAD, &errorcode);
      ReadVSlice(i, ALL_FRAMES, NULL, LOAD, &errorcode);
    }
  }
}

/* ------------------ ReloadAllSliceFiles ------------------------ */

void ReloadAllSliceFiles(void){
  int ii;
  int file_count = 0;
  float load_size = 0.0, load_time;
  slicedata **reload_slicelist;

  NewMemory((void **)&reload_slicelist, nsliceinfo*sizeof(slicedata *));
  slicefile_labelindex_save = slicefile_labelindex;
  START_TIMER(load_time);

  for(ii=0; ii<nsliceinfo; ii++){
    slicedata *slicei;

    slicei = sliceinfo+ii;
    reload_slicelist[ii] = NULL;
    if(slicei->loaded==1&&slicei->display==1){ // don't reload a slice file that is part of a vector slice
      if(slicei->vloaded==0){
        reload_slicelist[ii] = slicei;
      }
    }
  }
  for(ii = 0; ii < nsliceinfo; ii++){
    slicedata *slicei;
    int i;
    int errorcode;

    slicei = reload_slicelist[ii];
    if(slicei==NULL)continue;
    i = slicei-sliceinfo;

    if(slicei->slice_filetype == SLICE_GEOM){
      load_size+=ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, &errorcode);
    }
    else{
      load_size+=ReadSlice(slicei->file, i, ALL_FRAMES, NULL, LOAD, DEFER_SLICECOLOR, &errorcode);
    }
    file_count++;
  }
  STOP_TIMER(load_time);
  FREEMEMORY(reload_slicelist);
  PRINT_LOADTIMES(file_count,load_size,load_time);
  slicefile_labelindex = slicefile_labelindex_save;
}

/* ------------------ LoadUnloadMenu ------------------------ */

void LoadUnloadMenu(int value){
  int errorcode;
  int i;
  int ii;
  int file_count = 0;
  float load_size = 0.0, load_time;

  if(value==MENU_DUMMY)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value==UNLOADALL){
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"UNLOADALL\n");
    }
    if(hrr_csv_filename!=NULL){
      ReadHRR(UNLOAD, &errorcode);
    }
    if(nvolrenderinfo>0){
      LoadVolsmoke3DMenu(UNLOAD_ALL);
    }

    LoadVSliceMenu2(UNLOAD_ALL);

    for(i = 0; i < nsliceinfo; i++){
      slicedata *slicei;

      slicei = sliceinfo + i;
      if(slicei->loaded == 1){
        if(slicei->slice_filetype == SLICE_GEOM){
          ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, &errorcode);
        }
        else{
          ReadSlice(slicei->file, i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
        }
      }
    }
    for(i = 0; i<nplot3dinfo; i++){
      ReadPlot3D("",i,UNLOAD,&errorcode);
    }
    for(i=0;i<npatchinfo;i++){
      ReadBoundary(i,UNLOAD,&errorcode);
    }
    for(i=0;i<npartinfo;i++){
      ReadPart("",i,UNLOAD,&errorcode);
    }
    for(i=0;i<nisoinfo;i++){
      ReadIso("",i,UNLOAD,NULL,&errorcode);
    }
    for(i=0;i<nzoneinfo;i++){
      ReadZone(i,UNLOAD,&errorcode);
    }
    for(i=0;i<nsmoke3dinfo;i++){
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
    }
    if(nvolrenderinfo>0){
      UnLoadVolsmoke3DMenu(UNLOAD_ALL);
    }
    if(showdevice_val==1||showdevice_plot!=DEVICE_PLOT_HIDDEN){
      showdevice_plot = DEVICE_PLOT_HIDDEN;
      showdevice_val = 0;
      UpdateDeviceShow();
    }
    updatemenu=1;
    GLUTPOSTREDISPLAY;
  }
  if(value==RELOADALL||value==RELOAD_INCREMENTAL_ALL){
    int load_mode;

    if(value==RELOADALL)load_mode = LOAD;
    if(value==RELOAD_INCREMENTAL_ALL)load_mode = RELOAD;

    LOCK_COMPRESS
    if(hrr_csv_filename!=NULL){
      ReadHRR(LOAD, &errorcode);
    }


    //*** reload vector slice and slice files

#define BOUND_UPDATE_COLORS  110
    slicefile_labelindex_save = slicefile_labelindex;
    START_TIMER(load_time);
    SetLoadedSliceBounds(NULL, 0);
    ReloadAllVectorSliceFiles();
    ReloadAllSliceFiles();
    SliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
    STOP_TIMER(load_time);
    PRINT_LOADTIMES(file_count,load_size,load_time);
    slicefile_labelindex=slicefile_labelindex_save;

    //*** reload plot3d files

    for(i = 0; i<nplot3dinfo; i++){
      plot3dinfo[i].finalize=0;
    }
    for(i = nplot3dinfo-1; i>=0; i--){
      if(plot3dinfo[i].loaded==1){
        plot3dinfo[i].finalize = 1;
        break;
      }
    }
    for(i=0;i<nplot3dinfo;i++){
      if(plot3dinfo[i].loaded==1){
        ReadPlot3D(plot3dinfo[i].file,i,LOAD,&errorcode);
      }
    }

    //*** reload boundary files

    for(ii=0;ii<npatch_loaded;ii++){
      patchdata *patchi;

      i = patch_loaded_list[ii];
      patchi = patchinfo+i;
      PRINTF("Loading %s(%s)", patchi->file, patchi->label.shortlabel);
      ReadBoundary(i, load_mode,&errorcode);
    }

    //*** reload 3d smoke files

    for(i=0;i<nsmoke3dinfo;i++){
      if(smoke3dinfo[i].loaded==1||smoke3dinfo[i].request_load==1){
        ReadSmoke3D(ALL_SMOKE_FRAMES, i, load_mode, FIRST_TIME, &errorcode);
      }
    }

    //*** reload particle files

    int npartloaded_local = 0;
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo+i;
      if(parti->loaded==1){
        parti->reload=1;
        npartloaded_local++;
        ReadPart(parti->file,i,UNLOAD,&errorcode);
      }
      else{
        parti->reload=0;
      }
      parti->loadstatus = FILE_UNLOADED;
    }
    if(npartloaded_local>0){
      npartframes_max = GetMinPartFrames(PARTFILE_RELOADALL);
      LoadAllPartFilesMT(RELOAD_LOADED_PART_FILES);
    }


    //*** reload isosurface files

    update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
    CancelUpdateTriangles();
    for(i = 0; i<nisoinfo; i++){
      isodata *isoi;

      isoi = isoinfo + i;
      if(isoi->loaded==0)continue;
      ReadIso(isoi->file,i,LOAD,NULL,&errorcode);
      printf("\n");
    }
    if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)ReadIsoGeomWrapup(BACKGROUND);
    update_readiso_geom_wrapup = UPDATE_ISO_OFF;
    ReadSMVDynamic(smv_filename);
    UNLOCK_COMPRESS
  //  plotstate=DYNAMIC_PLOTS;
  //  visParticles=1;
    updatemenu=1;
    GLUTPOSTREDISPLAY;
  }
  if(value==SHOWFILES){
    GLUTPOSTREDISPLAY;
    showfiles=1-showfiles;
    updatemenu=1;
    UpdateSliceMenuLabels();
    UpdateVsliceMenuLabels();
    UpdateSmoke3dMenuLabels();
    UpdateBoundaryMenuLabels();
    UpdateIsoMenuLabels();
    UpdatePartMenuLabels();
    UpdateTourMenuLabels();
    UpdatePlot3dMenuLabels();
  }
  if(value==COMPUTE_SMV_BOUNDS){
    bounds_each_mesh = 1-bounds_each_mesh;
    updatemenu = 1;
  }
  if(value==SHOW_BOUND_DIFFS){
    show_bound_diffs = 1-show_bound_diffs;
    updatemenu = 1;
  }
  if(value==CACHE_FILE_DATA){
    cache_file_data = 1-cache_file_data;
    cache_plot3d_data = cache_file_data;
    cache_boundary_data = cache_file_data;
    cache_part_data = cache_file_data;
    SetCacheFlag(BOUND_PATCH, cache_file_data);
    SetCacheFlag(BOUND_PLOT3D, cache_file_data);
    SetCacheFlag(BOUND_PART, cache_file_data);
#define BOUND_CACHE_DATA               112
    PatchBoundsCPP_CB(BOUND_CACHE_DATA);
    SliceBoundsCPP_CB(BOUND_CACHE_DATA);
    PartBoundsCPP_CB(BOUND_CACHE_DATA);
    updatemenu = 1;
  }
  if(value==REDIRECT){
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    redirect=1-redirect;
    if(LOG_FILENAME!=NULL){
      fclose(LOG_FILENAME);
      LOG_FILENAME=NULL;
    }
    if(redirect==1){
      LOG_FILENAME=fopen(log_filename,"w");
      if(LOG_FILENAME==NULL)redirect=0;
    }
    if(redirect==1){
      SetStdOut(LOG_FILENAME);
    }
    else{
      SetStdOut(stdout);
    }
  }
  GLUTSETCURSOR(GLUT_CURSOR_RIGHT_ARROW);
}

void AvatarEvacMenu(int value){
  if(value==MENU_DUMMY)return;
  iavatar_evac=value;
  updatemenu=1;
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
    AddNewTour();
    DialogMenu(DIALOG_TOUR_SHOW);
    break;
  case MENU_TOUR_CLEARALL:
    for(i=0;i<ntourinfo;i++){  // clear all tours
      touri = tourinfo + i;
      touri->display=touri->display2;
    }
    if(viewtourfrompath==1){
      SetViewPoint(RESTORE_EXTERIOR_VIEW);
    }
    from_glui_trainer=0;
    for(i=0;i<ntourinfo;i++){
      touri = tourinfo + i;
      if(touri->display==1){
        selected_tour=touri;
        break;
      }
    }
    selected_tour=NULL;
    break;
  case MENU_TOUR_MANUAL:
    for(i=0;i<ntourinfo;i++){  // clear all tours
      touri = tourinfo + i;
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
      ShowGluiTour();
    }
    break;
  case MENU_TOUR_SHOWALL:               // show all tours
    for(i=0;i<ntourinfo;i++){
      touri = tourinfo + i;
      touri->display=1;
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    break;
  case MENU_TOUR_VIEWFROMROUTE:               // view from route
    viewtourfrompath = 1 - viewtourfrompath;
    if(viewtourfrompath==0)SetViewPoint(RESTORE_EXTERIOR_VIEW);
    break;
  case MENU_TOUR_DEFAULT:
    for(i=0;i<ntourinfo;i++){
      touri = tourinfo + i;
      touri->display=0;
    }
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
    DefaultTour();
    break;
  default:
    if(value<-22){
      tourlocus_type=2;
      iavatar_types=(-value-23);
      if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
        tourinfo[selectedtour_index].glui_avatar_index=iavatar_types;
      }
    }

    //  show one tour

    if(value>=0&&value<ntourinfo){
      int j;

      touri = tourinfo + value;
      touri->display = 1 - touri->display;
      if(touri->display==1){
        selectedtour_index=value;
        selected_frame=touri->first_frame.next;
        selected_tour=touri;
      }
      else{
        for(j=0;j<ntourinfo;j++){
          tourdata *tourj;

          tourj = tourinfo + j;
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
  DeleteTourList();
  CreateTourList();
  UpdateTourControls();
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
  tournumber = thetour - tourinfo;
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
  for(i=0;i<npartinfo;i++){
    parti = partinfo + i;
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
    streak5show=0;
    streak5step=0;
  }
  else if(value==MENU_STREAK_HEAD){
    showstreakhead=1-showstreakhead;
  }
  else{
    streak5show=1;
    streak5step=0;
    rvalue=streak_rvalue[value];
    UpdateStreakValue(rvalue-0.001);
    UpdateGluiStreakValue(rvalue);

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

  iprop = value%npropinfo;
  iobject = value / npropinfo;
  if(iprop >= 0 && iprop < npropinfo){
    propdata *propi;

    propi = propinfo + iprop;
    if(iobject >= 0 && iobject < propi->nsmokeview_ids){
      int i;

      propi->smokeview_id = propi->smokeview_ids[iobject];
      propi->smv_object = propi->smv_objects[iobject];
      updatemenu = 1;
      GetIndepVarIndices(propi->smv_object,
        propi->vars_indep, propi->nvars_indep,
        propi->vars_indep_index);

      for(i = 0;i < npartclassinfo;i++){
        partclassdata *partclassi;

        partclassi = partclassinfo + i;
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
    SetValTypeIndex(BOUND_PART, ipart5prop);

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
    PartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }
  else if(value==MENU_PROP_SHOWALL){
    if(current_property!=NULL){
      unsigned char *vis;
      int i;

      vis = current_property->class_vis;
      for(i=0;i< npartclassinfo;i++){
        vis[i]=1;
      }
    }
  }
  else if(value==MENU_PROP_HIDEALL){
    if(current_property!=NULL){
      unsigned char *vis;
      int i;

      vis = current_property->class_vis;
      for(i=0;i< npartclassinfo;i++){
        vis[i]=0;
      }
    }

  }
  else if(value==MENU_PROP_HIDEPART){
    int i;
    int unhide=1;

    for(i=0;i<npart5prop;i++){
      propi = part5propinfo + i;
      if(propi->particle_property==1){
        if(propi->display==1)unhide=0;
        propi->display=0;
      }
    }
    part5show=0;
    parttype=0;
    if(unhide==1&&last_prop_display>=0){
      ParticlePropShowMenu(last_prop_display);
    }
  }
  else if(value==MENU_PROP_HIDEAVATAR){
    int i;

    for(i=0;i<npart5prop;i++){
      propi = part5propinfo + i;
      if(propi->human_property==1){
        propi->display=0;
      }
    }
    part5show=0;
    parttype=0;
  }
  else if(value==MENU_PROP_TRACERS){
    show_tracers_always=1-show_tracers_always;
    UpdateTracers();
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

      partclassj = partclassinfo + iclass;
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

  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int errorcode;

    parti = partinfo+i;
    if(parti->evac==1||parti->loaded==0)continue;
    ReadPart(parti->file, i, UNLOAD, &errorcode);
  }
}

/* ------------------ LoadAllPartFiles ------------------------ */

void LoadAllPartFiles(int partnum){
  int i;

  for(i = 0;i<npartinfo;i++){
    partdata *parti;
    int errorcode;
    FILE_SIZE file_size;

    parti = partinfo+i;
    if(parti->skipload==1)continue;
    if(partnum>=0&&i!=partnum)continue;  //  load only particle file with file index partnum
    LOCK_PART_LOAD;                      //  or load all particle files
    if(parti->loadstatus==FILE_UNLOADED){
      if(partnum==LOAD_ALL_PART_FILES||(partnum==RELOAD_LOADED_PART_FILES&&parti->reload==1)||partnum==i){
        parti->loadstatus = FILE_LOADING;
        UNLOCK_PART_LOAD;
        file_size = ReadPart(parti->file, i, LOAD, &errorcode);
        LOCK_PART_LOAD;
        parti->loadstatus = FILE_LOADED;
        part_load_size += file_size;
        part_file_count++;
      }
    }
    UNLOCK_PART_LOAD;
  }
}

#define PART 0
#define EVAC 1

/* ------------------ SetupPart ------------------------ */

void SetupPart(int value, int option){
  int i;

#define SETVALMIN 1
#define SETVALMAX 2
  int *list = NULL, nlist = 0;

  NewMemory((void **)&list, npartinfo*sizeof(int));
  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(option==PART&&parti->evac==1)continue;                 // don't load an evac file if part files are loaded
    if(option==EVAC&&parti->evac==0)continue;                 // don't load a part file if evac files are loaded
    if(parti->loaded==0&&value==PARTFILE_RELOADALL)continue;  // don't reload a file that is not currently loaded
    if(parti->loaded==0&&value>=0&&value!=i)continue;         // if value>0 only load file with index  value
    list[nlist++] = i;
  }
  SetLoadedPartBounds(list, nlist);
  FREEMEMORY(list);
  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    parti->finalize = 0;
    parti->skipload = 1;
    parti->loadstatus = FILE_UNLOADED;
    parti->boundstatus = PART_BOUND_UNDEFINED;
    if(option==PART&&parti->evac==1)continue;                 // don't load an evac file if part files are loaded
    if(option==EVAC&&parti->evac==0)continue;                 // don't load a part file if evac files are loaded
    if(parti->loaded==0&&value==PARTFILE_RELOADALL)continue;  // don't reload a file that is not currently loaded
    if(parti->loaded==0&&value>=0&&value!=i)continue;         // if value>0 only load file with index  value
    parti->skipload = 0;
  }

  if(value>=0){
    partdata *parti;

    parti = partinfo+value;
    ASSERT(value>=0&&value<npartinfo);
    value = CLAMP(value, 0, npartinfo-1);
    parti->finalize = 1;
  }
  else{
    for(i = npartinfo-1; i>=0; i--){
      partdata *parti;

      parti = partinfo+i;
      if(parti->skipload==1)continue;
      parti->finalize = 1;
      break;
    }
  }
}

/* ------------------ LoadParticleEvacMenu ------------------------ */

void LoadParticleEvacMenu(int value, int option){
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

    parti = partinfo + value;
    partfile = parti->file;
    parti->finalize = 1;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOADFILE\n");
      fprintf(scriptoutstream," %s\n",partfile);
    }
    npartframes_max=GetMinPartFrames(PARTFILE_RELOADALL);
    npartframes_max=MAX(GetMinPartFrames(value),npartframes_max);
    if(scriptoutstream==NULL||script_defer_loading==0){
      SetupPart(value,option);                                                // load only particle file with index value
      GetAllPartBoundsMT();
      LoadAllPartFilesMT(value);
    }
  }
  else{
    if(value==MENU_PARTICLE_UNLOAD_ALL){
      for(i=0;i<npartinfo;i++){
        partdata *parti;

        parti = partinfo + i;
        if(option==PART&&parti->evac==1)continue;
        if(option==EVAC&&parti->evac==0)continue;
        ReadPart("", i, UNLOAD, &errorcode);
      }
    }
    else if(value==MENU_PART_SETTINGS){
      ShowBoundsDialog(DLG_PART);
    }
    else if(value==MENU_PART_PARTFAST){
      updatemenu = 1;
      partfast = 1-partfast;
      if(partfast==0)printf("fast loading: off\n");
      if(partfast==1)printf("fast loading: on\n");
      UpdateGluiPartFast();
    }
    else{
      if(scriptoutstream!=NULL){
        fprintf(scriptoutstream,"LOADPARTICLES\n");
      }
      if(value==PARTFILE_LOADALL){
        SetupPart(value,option);
        GetAllPartBoundsMT();
        npartframes_max=GetMinPartFrames(PARTFILE_LOADALL);
      }
      else{
        npartframes_max=GetMinPartFrames(PARTFILE_RELOADALL);
      }

      if(scriptoutstream==NULL||script_defer_loading==0){

        SetupPart(value,option);

        // unload particle files


        if(value!=PARTFILE_RELOADALL&&value!=EVACFILE_RELOADALL){
          UnloadAllPartFiles();
        }

        // load particle files unless we are reloading and the were not loaded before

        START_TIMER(part_load_time);
        GetAllPartBoundsMT();
        LoadAllPartFilesMT(LOAD_ALL_PART_FILES);
        STOP_TIMER(part_load_time);
        PRINT_LOADTIMES(part_file_count,part_load_size,part_load_time);

        force_redisplay = 1;
        UpdateFrameNumber(0);
      }
    }
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadParticleMenu ------------------------ */

void LoadParticleMenu(int value){
  LoadParticleEvacMenu(value,PART);
}

/* ------------------ LoadEvacMenu ------------------------ */

void LoadEvacMenu(int value){
  LoadParticleEvacMenu(value,EVAC);
}

/* ------------------ ZoneMenu ------------------------ */

void ZoneMenu(int value){
  int i,errorcode;
  if(value>=0){
    if(scriptoutstream!=NULL){
      zonedata *zonei;

      zonei = zoneinfo + value;
      fprintf(scriptoutstream,"LOADFILE\n");
      fprintf(scriptoutstream," %s\n",zonei->file);
    }
    ReadZone(value,LOAD,&errorcode);
  }
  else{
    for(i=0;i<nzoneinfo;i++){
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
    ReadVSlice(value,ALL_FRAMES, NULL, UNLOAD,&errorcode);
  }
  else if(value==UNLOAD_ALL){
    for(i=0;i<nvsliceinfo;i++){
      ReadVSlice(i,ALL_FRAMES, NULL, UNLOAD,&errorcode);
    }
  }
  else if(value==-2){
    int unload_index;

    unload_index=LastVSliceLoadstack();
    if(unload_index>=0&&unload_index<nvsliceinfo){
      ReadVSlice(unload_index,ALL_FRAMES, NULL, UNLOAD,&errorcode);
    }
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
    for(i=0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo+i;
      if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label, "INCLUDE_GEOM")!=0){
        ReadBoundary(i,UNLOAD,&errorcode);
      }
    }
  }
}

/* ------------------ UnloadIsoMenu ------------------------ */

void UnloadIsoMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    ReadIso("",value,UNLOAD,NULL,&errorcode);
  }
  else{
    for(i=0;i<nisoinfo;i++){
      ReadIso("",i,UNLOAD,NULL,&errorcode);
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
    for(i=0;i<nplot3dinfo;i++){
      ReadPlot3D("",i,UNLOAD,&errorcode);
    }
  }
}

/* ------------------ UnloadEvacMenu ------------------------ */

void UnloadEvacMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    ReadPart("", value, UNLOAD, &errorcode);
  }
  else{
    for(i=0;i<npartinfo;i++){
      if(partinfo[i].evac==0)continue;
      ReadPart("", i, UNLOAD, &errorcode);
    }
  }
}

/* ------------------ UnloadPartMenu ------------------------ */

void UnloadPartMenu(int value){
  int errorcode,i;

  updatemenu=1;
  GLUTPOSTREDISPLAY;
  if(value>=0){
    ReadPart("", value, UNLOAD, &errorcode);
  }
  else{
    for(i=0;i<npartinfo;i++){
      if(partinfo[i].evac==1)continue;
      ReadPart("", i, UNLOAD, &errorcode);
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
    for(i=0;i<nvsliceinfo;i++){
      ReadVSlice(i,ALL_FRAMES, NULL,  UNLOAD,&errorcode);
    }
    return 0;
  }
  else if(value==MENU_LOADVSLICE_SETTINGS){
    ShowBoundsDialog(DLG_SLICE);
  }
  else if(value>=0){
    vslicedata *vslicei;
    slicedata *slicei;

    return_filesize = ReadVSlice(value, ALL_FRAMES, NULL, LOAD, &errorcode);
    vslicei = vsliceinfo + value;
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
    int file_count = 0;
    float load_size = 0.0, load_time;

    value = -(1000 + value);
    submenutype=value/4;
    dir=value%4;
    submenutype=subvslice_menuindex[submenutype];
    vslicei = vsliceinfo + submenutype;
    slicei = sliceinfo + vslicei->ival;
    submenulabel = slicei->label.longlabel;
    START_TIMER(load_time);
    for(i=0;i<nvsliceinfo;i++){
      char *longlabel;

      vslicei = vsliceinfo + i;
      slicei=sliceinfo + vslicei->ival;
      longlabel = slicei->label.longlabel;
      if(strcmp(longlabel,submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicei->idir)continue;
      file_count++;
      load_size+=ReadVSlice(i,ALL_FRAMES, NULL, LOAD,&errorcode);
    }
    STOP_TIMER(load_time);
    PRINT_LOADTIMES(file_count,load_size,load_time);
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

    slicei = sliceinfo+value;

    if(slicei->slice_filetype==SLICE_GEOM){
      ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, &errorcode);
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
      for(i=0;i<nsliceinfo;i++){
        slicedata *slicei;

        slicei = sliceinfo+i;
        if(slicei->slice_filetype == SLICE_GEOM){
          ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, &errorcode);
        }
        else{
          ReadSlice("",i, ALL_FRAMES, NULL, UNLOAD,DEFER_SLICECOLOR,&errorcode);
        }
      }
      for(i=0;i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo + i;
        if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
          UnloadBoundaryMenu(i);
        }
      }
    }
    else if(value==UNLOAD_LAST){
      int unload_index;

      unload_index=LastSliceLoadstack();
      if(unload_index>=0&&unload_index<nsliceinfo){
        slicedata *slicei;

        slicei = sliceinfo+unload_index;
        if(slicei->slice_filetype==SLICE_GEOM){
          ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, &errorcode);
        }
        else{
          ReadSlice("", unload_index, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR, &errorcode);
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
    mvslicei = multivsliceinfo + value;
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
    mslicei = multisliceinfo + value;
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

    meshi = meshinfo + value;
    vr = &(meshi->volrenderinfo);
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
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = meshinfo + i;
      vr = &(meshi->volrenderinfo);
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
      for(i=0;i<nmeshes;i++){
        meshdata *meshi;
        volrenderdata *vr;

        meshi = meshinfo + i;
        vr = &(meshi->volrenderinfo);
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

    meshi = meshinfo + value;
    vr = &(meshi->volrenderinfo);
    if(vr->fireslice!=NULL||vr->smokeslice!=NULL||vr->lightslice!=NULL){
      UnloadVolsmokeAllFrames(vr);
    }
  }
  updatemenu=1;
  read_vol_mesh=VOL_READNONE;
  GLUTPOSTREDISPLAY;
}

/* ------------------ UnLoadSmoke3DMenu ------------------------ */

void UnLoadSmoke3DMenu(int value){
  int errorcode;
  int i;
  smoke3ddata *smoke3di;

  if(value==MENU_DUMMY)return;
  updatemenu=1;
  if(value<0){
    value= -value-1;
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3di = smoke3dinfo + i;
      if(smoke3di->loaded==1&&smoke3di->type==value){
        ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
      }
    }
  }
  else{
    ReadSmoke3D(ALL_SMOKE_FRAMES, value, UNLOAD, FIRST_TIME, &errorcode);
  }
}

/* ------------------ LoadSmoke3D ------------------------ */

FILE_SIZE LoadSmoke3D(int type, int *count){
  int last_smoke = 0, i, file_count=0,errorcode;
  FILE_SIZE load_size=0;
  int need_soot, need_hrrpuv, need_temp, need_co2;
  FILE_SIZE total_size;

  need_soot   = type&SOOT_2;
  need_hrrpuv = type&HRRPUV_2;
  need_temp   = type&TEMP_2;
  need_co2    = type&CO2_2;
  for(i = nsmoke3dinfo-1; i>=0; i--){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo+i;
    if(
      (need_soot   == SOOT_2    && smoke3di->type == SOOT)   ||
      (need_hrrpuv == HRRPUV_2  && smoke3di->type == HRRPUV) ||
      (need_temp   == TEMP_2    && smoke3di->type == TEMP)   ||
      (need_co2    == CO2_2     && smoke3di->type == CO2)){
    last_smoke = i;
    break;
    }
  }
  total_size = 0;
  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(
      (need_soot   == SOOT_2    && smoke3di->type == SOOT)   ||
      (need_hrrpuv == HRRPUV_2  && smoke3di->type == HRRPUV) ||
      (need_temp   == TEMP_2    && smoke3di->type == TEMP)   ||
      (need_co2    == CO2_2     && smoke3di->type == CO2)){
      file_count++;
      smoke3di->finalize = 0;
      if(i == last_smoke)smoke3di->finalize = 1;
      if(compute_smoke3d_file_sizes==1){
        smoke3di->file_size = GetFileSizeSMV(smoke3di->reg_file);
        total_size += smoke3di->file_size;
      }
      else{
        load_size += ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      }
    }
  }
  if(compute_smoke3d_file_sizes==1){
    PRINTF(" file size: ");
    if(total_size>1000000000){
      PRINTF("%.1f GB\n", (float)total_size/1000000000.);
    }
    else if(total_size>1000000){
      PRINTF("%.1f MB\n", (float)total_size/1000000.);
    }
    else{
      PRINTF("%.0f kB\n", (float)total_size/1000.);
    }
    PRINTF(" load time: %f s\n",(float)total_size*8.0/1000000000.0);
    PRINTF("   (assuming a gigabit network connection)\n");
  }
  *count = file_count;
  return load_size;
}

/* ------------------ LoadSmoke3DMenu ------------------------ */

void LoadSmoke3DMenu(int value){
  int i,errorcode;
  int file_count;
  float load_time, load_size;

#ifdef pp_SMOKE_FAST
#define MENU_SMOKE_SOOT_HRRPUV     -5
#define MENU_SMOKE3D_LOAD_TEST     -3
#define MENU_SMOKE_SOOT_HRRPUV_CO2 -6
#define MENU_SMOKE_SOOT_TEMP       -7
#define MENU_SMOKE_SOOT_TEMP_CO2   -8
#endif

#define MENU_DUMMY_SMOKE           -9
#define MENU_SMOKE_SETTINGS        -4
#define MENU_SMOKE_FILE_SIZES     -10

  if(value == MENU_DUMMY_SMOKE)return;
  START_TIMER(load_time);
  load_size = 0.0;
  file_count=0;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    if(scriptoutstream!=NULL){
      char *file;

      file = smoke3dinfo[value].file;
      fprintf(scriptoutstream,"LOADFILE\n");
      fprintf(scriptoutstream," %s\n",file);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + value;
      smoke3di->finalize = 1;
      ReadSmoke3D(ALL_SMOKE_FRAMES, value, LOAD, FIRST_TIME, &errorcode);
    }
  }
  else if(value==UNLOAD_ALL){
    for(i=0;i<nsmoke3dinfo;i++){
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
    }
  }
  else if(value == MENU_SMOKE_SETTINGS){
    ShowBoundsDialog(DLG_3DSMOKE);
  }
#ifdef pp_SMOKE_FAST
  else if(value == MENU_SMOKE_SOOT_HRRPUV){
    if(smoke3d_load_test == 1){
      ReadSmoke3DAllMeshesAllTimes(SOOT_2|HRRPUV_2, &errorcode);
    }
    else{
      load_size=LoadSmoke3D(SOOT_2|HRRPUV_2, &file_count);
    }
  }
  else if(value == MENU_SMOKE_SOOT_HRRPUV_CO2){
    if(smoke3d_load_test==1){
      ReadSmoke3DAllMeshesAllTimes(SOOT|HRRPUV|CO2, &errorcode);
    }
    else{
      load_size=LoadSmoke3D(SOOT_2|HRRPUV_2|CO2_2, &file_count);
     }
  }
  else if(value == MENU_SMOKE_SOOT_TEMP){
    if(smoke3d_load_test==1){
      ReadSmoke3DAllMeshesAllTimes(SOOT|TEMP, &errorcode);
    }
    else{
     load_size=LoadSmoke3D(SOOT_2|TEMP_2,&file_count);
    }
  }
  else if(value == MENU_SMOKE_SOOT_TEMP_CO2){
    if(smoke3d_load_test==1){
      ReadSmoke3DAllMeshesAllTimes(SOOT|TEMP|CO2, &errorcode);
    }
    else{
      load_size=LoadSmoke3D(SOOT_2|TEMP_2|CO2_2, &file_count);
    }
  }
#endif
    else if(value ==MENU_SMOKE_FILE_SIZES){
      compute_smoke3d_file_sizes = 1-compute_smoke3d_file_sizes;
      updatemenu = 1;
    }

#ifdef pp_SMOKE_FAST
  else if(value == MENU_SMOKE3D_LOAD_TEST){
    smoke3d_load_test = 1 - smoke3d_load_test;
  }
#endif
  else if(value<=-100){
    smoke3ddata *smoke3dj;

    value = -(value + 100);
    smoke3dj = smoke3dinfo + value;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOAD3DSMOKE\n");
      fprintf(scriptoutstream," %s\n",smoke3dj->label.longlabel);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
#ifdef pp_SMOKE_FAST
      if(smoke3d_load_test==1){
        ReadSmoke3DAllMeshesAllTimes(smoke3dj->type2, &errorcode);
      }
      else{
        load_size=LoadSmoke3D(smoke3dj->type2, &file_count);
      }
#endif
    }
  }
  STOP_TIMER(load_time);
  if(compute_smoke3d_file_sizes==0){
    PRINT_LOADTIMES(file_count, load_size, load_time);
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ AnySmoke ------------------------ */

int AnySmoke(void){

  if(nsmoke3dinfo>0)return 1;
  return 0;
}

/* ------------------ AnySlices ------------------------ */

int AnySlices(const char *type){
  int i;

  for(i=0;i<nsliceinfo;i++){
    if(STRCMP(sliceinfo[i].label.longlabel,type)==0)return 1;
  }
  return 0;
}

/* ------------------ DefineAllFEDs ------------------------ */

void DefineAllFEDs(void){
  int i;

  compute_fed=0;
  for(i=nsliceinfo-nfedinfo;i<nsliceinfo;i++){
    LoadSliceMenu(i);
    UnloadSliceMenu(i);
  }
  SMV_EXIT(0);
}

/* ------------------ LoadSlicei ------------------------ */

FILE_SIZE LoadSlicei(int set_slicecolor, int value, int time_frame, float *time_value){
  slicedata *slicei;
  int errorcode;
  FILE_SIZE return_filesize=0;

  slicei = sliceinfo + value;
  slicei->loading=1;
  if(script_multislice == 0 && scriptoutstream != NULL){
    fprintf(scriptoutstream, "LOADSLICEM\n");
    fprintf(scriptoutstream, " %s\n", slicei->label.longlabel);
    fprintf(scriptoutstream, " %i %f\n", slicei->idir, slicei->position_orig);
    fprintf(scriptoutstream, " %i\n", slicei->blocknumber + 1);
  }
  if(scriptoutstream==NULL||script_defer_loading==0){
    if(value < nsliceinfo - nfedinfo){
      colorbardata *fed_colorbar;
      int reset_colorbar = 0;

      fed_colorbar = GetColorbar(default_fed_colorbar);
      if(fed_colorbar != NULL&&fed_colorbar - colorbarinfo == colorbartype)reset_colorbar = 1;

      if(slicei->slice_filetype == SLICE_GEOM){
        return_filesize = ReadGeomData(slicei->patchgeom, slicei, LOAD, time_frame, time_value, &errorcode);
      }
      else {
        return_filesize = ReadSlice(slicei->file, value, time_frame, time_value, LOAD, set_slicecolor, &errorcode);
      }
      if(reset_colorbar == 1)ColorbarMenu(colorbartype_save);
    }
    else{
      colorbardata *fed_colorbar;

      fed_colorbar = GetColorbar(default_fed_colorbar);
      if(fed_colorbar != NULL && current_colorbar != fed_colorbar)colorbartype_save = current_colorbar - colorbarinfo;
      ReadFed(value, time_frame, time_value, LOAD, FED_SLICE, &errorcode);
      if(fed_colorbar != NULL)ColorbarMenu(fed_colorbar - colorbarinfo);
      return_filesize = 0;
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

  for(i = 0; i<nsliceinfo; i++){
    char *longlabel;
    int set_slicecolor;
    slicedata *slicei;

    slicei = sliceinfo+i;
    longlabel = slicei->label.longlabel;
    if(strcmp(longlabel, submenulabel)!=0)continue;
    if(dir!=0&&dir!=slicei->idir)continue;
    set_slicecolor = DEFER_SLICECOLOR;
    if(i==last_slice)set_slicecolor = SET_SLICECOLOR;
    if(slicei->slice_filetype==SLICE_GEOM){
      load_size += ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, &errorcode);
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
  float load_time, load_size = 0.0;
  int file_count=0;

  if(value==MENU_DUMMY)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    SetLoadedSliceBounds(&value, 1);
    LoadSlicei(SET_SLICECOLOR,value, ALL_FRAMES, NULL);
  }
  else{
    switch (value){
      int submenutype;
      char *submenulabel;
      slicedata *slicei;
      int dir;
      int last_slice;

      case UNLOAD_ALL:
        for(i=0;i<nsliceinfo;i++){
          slicei = sliceinfo + i;
          if(slicei->loaded == 1){
            if(slicei->slice_filetype == SLICE_GEOM){
              ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, &errorcode);
            }
            else{
              ReadSlice("",i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR, &errorcode);
            }
          }
        }
        break;
      case MENU_SHOWSLICE_IN_GAS:
        UpdateShowSliceInObst(ONLY_IN_GAS);
        break;
      case  MENU_SHOWSLICE_IN_GASANDSOLID:
        UpdateShowSliceInObst(GAS_AND_SOLID);
        break;
      case MENU_SHOWSLICE_IN_SOLID:
        UpdateShowSliceInObst(ONLY_IN_SOLID);
        break;
      case MENU_SLICE_SETTINGS:
        ShowBoundsDialog(DLG_SLICE);
        break;
      default:
        value = -(1000 + value);
        submenutype=value/4;
        dir=value%4;
        submenutype=subslice_menuindex[submenutype];
        slicei = sliceinfo + submenutype;
        submenulabel = slicei->label.longlabel;
        last_slice = nsliceinfo - 1;
        for(i = nsliceinfo-1; i>=0; i--){
          char *longlabel;

          slicei = sliceinfo + i;
          longlabel = slicei->label.longlabel;
          if(strcmp(longlabel, submenulabel) != 0)continue;
          if(dir != 0 && dir != slicei->idir)continue;
          last_slice = i;
          break;
        }
        START_TIMER(load_time);
        load_size = LoadAllSliceFiles(last_slice, submenulabel, dir, &file_count);
        STOP_TIMER(load_time);
        PRINT_LOADTIMES(file_count,load_size,load_time);
      }
  }
  updatemenu=1;
  CheckMemory;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadVMultiSliceMenu ------------------------ */

void LoadMultiVSliceMenu(int value){
  int i;
  int file_count = 0;
  float load_size = 0.0, load_time;

  if(value==MENU_DUMMY)return;
  if(value>=0){
    multivslicedata *mvslicei;

    mvslicei = multivsliceinfo + value;
    if(scriptoutstream!=NULL){
      if(mvslicei->nvslices>0){
        slicedata *slicei;

        slicei = sliceinfo + mvslicei->ivslices[0];
        fprintf(scriptoutstream,"LOADVSLICE\n");
        fprintf(scriptoutstream," %s\n",slicei->label.longlabel);
        fprintf(scriptoutstream," %i %f\n",slicei->idir,slicei->position_orig);
        script_multivslice=1;
      }
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      char *longlabel=NULL;
      vslicedata *vslice1;

      vslice1 = vsliceinfo+mvslicei->ivslices[0];
      if(vslice1->ival>=0)longlabel = sliceinfo[vslice1->ival].label.longlabel;
      UnloadAllSliceFiles(longlabel); // unload all vector slices except for the type being loaded now

      START_TIMER(load_time);
      for(i = 0; i<mvslicei->nvslices; i++){
        vslicedata *vslicei;

        vslicei = vsliceinfo+mvslicei->ivslices[i];
        vslicei->finalize = 0;
      }
      for(i = mvslicei->nvslices-1; i>=0; i--){
        vslicedata *vslicei;

        vslicei = vsliceinfo+mvslicei->ivslices[i];
        if(vslicei->skip==0&&vslicei->loaded==0){
          vslicei->finalize = 1;
          break;
        }
      }
      for(i = 0; i<mvslicei->nvslices; i++){
        vslicedata *vslicei;

        vslicei = vsliceinfo + mvslicei->ivslices[i];
        if(vslicei->skip==0&&vslicei->loaded==0){
          load_size+=LoadVSliceMenu2(mvslicei->ivslices[i]);
          file_count++;
        }
        if(vslicei->skip==1&&vslicei->loaded==1)UnloadVSliceMenu(mvslicei->ivslices[i]);
      }
      STOP_TIMER(load_time);
      PRINT_LOADTIMES(file_count,load_size,load_time);
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
    submenutype=msubvslice_menuindex[submenutype];

    slicei = sliceinfo + submenutype;
    submenulabel = slicei->label.longlabel;
    START_TIMER(load_time);
    for(i = 0; i<nmultivsliceinfo; i++){
      char *longlabel;
      multivslicedata *mvslicei;
      slicedata *slicej;
      vslicedata *vslicej;

      mvslicei = multivsliceinfo + i;
      vslicej = vsliceinfo + mvslicei->ivslices[0];
      slicej = sliceinfo+vslicej->ival;
      longlabel = slicej->label.longlabel;
      if(strcmp(longlabel,submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicej->idir)continue;
      LoadMultiVSliceMenu(i);
      file_count++;
    }
    STOP_TIMER(load_time);
    PRINT_LOADTIMES(file_count,load_size,load_time);
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
        UpdateSliceDupDialog();
      }
      break;

      case  MENU_KEEP_COARSE:
      if(vectorslicedup_option!=SLICEDUP_KEEPCOARSE){
        vectorslicedup_option = SLICEDUP_KEEPCOARSE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateVSliceDups();
        UpdateSliceDupDialog();
      }
      break;

      case MENU_KEEP_FINE:
      if(vectorslicedup_option!=SLICEDUP_KEEPFINE){
        vectorslicedup_option = SLICEDUP_KEEPFINE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateVSliceDups();
        UpdateSliceDupDialog();
      }
      break;
      case MENU_LOADVSLICE_SETTINGS:
        ShowBoundsDialog(DLG_SLICE);
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
  }
}

/* ------------------ LoadAllMSlices ------------------------ */

FILE_SIZE LoadAllMSlices(int last_slice, multislicedata *mslicei){
  float load_time;
#ifdef pp_SLICE_BUFFER
  float process_time;
#endif
  FILE_SIZE file_size = 0;
  int file_count=0;

  START_TIMER(load_time);
#ifdef pp_SLICE_BUFFER
  for(i = 0; i<mslicei->nslices; i++){
    slicedata *slicei;
    int set_slicecolor;

    slicei = sliceinfo+mslicei->islices[i];
    printf("reading %s\n",slicei->file);
    slicei->stream_slice = fopen_buffer(slicei->file,"rb");
    file_size += slicei->stream_slice->filesize;
    file_count++;
  }
  STOP_TIMER(load_time);
  PRINT_LOADTIMES(file_count, (float)file_size, load_time);
  START_TIMER(process_time);
#endif

  SetLoadedSliceBounds(mslicei->islices, mslicei->nslices);
  file_size = LoadAllMSlicesMT(last_slice, mslicei, &file_count);

#ifdef pp_SLICE_BUFFER
  STOP_TIMER(process_time);
  PRINT_PROCESSTIMES(file_count, (float)file_size, process_time);
#else
  STOP_TIMER(load_time);
  PRINT_LOADTIMES(file_count,(float)file_size,load_time);
#endif
  return file_size;
}

/* ------------------ LoadMultiSliceMenu ------------------------ */

void LoadMultiSliceMenu(int value){
  int i;

  if(value==MENU_DUMMY)return;
  if(value>=0){
    multislicedata *mslicei;

    mslicei = multisliceinfo + value;
    if(scriptoutstream!=NULL){
      if(mslicei->nslices>0){
        slicedata *slicei;

        slicei = sliceinfo + mslicei->islices[0];
        fprintf(scriptoutstream,"LOADSLICE\n");
        fprintf(scriptoutstream," %s\n",slicei->label.longlabel);
        fprintf(scriptoutstream," %i %f\n",slicei->idir,slicei->position_orig);
        script_multislice=1;
      }
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      int last_slice;
      FILE_SIZE total_size=0;
      char *longlabel;

      last_slice = mslicei->nslices - 1;
      for(i = mslicei->nslices-1; i >=0; i--){
        slicedata *slicei;

        slicei = sliceinfo + mslicei->islices[i];
        if(slicei->skipdup== 0){
          last_slice = mslicei->islices[i];
          break;
        }
      }
      for(i = 0; i < mslicei->nslices; i++){
        slicedata *slicei;

        slicei = sliceinfo + mslicei->islices[i];
        if(slicei->skipdup== 1 && slicei->loaded == 1){
          UnloadSliceMenu(mslicei->islices[i]);
        }
      }
      longlabel = sliceinfo[last_slice].label.longlabel;
      UnloadAllSliceFiles(longlabel);  // unload all slices except for the type being loaded now
      total_size = LoadAllMSlices(last_slice, mslicei);
      if(compute_slice_file_sizes==1){
        PRINTF(" file size: ");
        if(total_size>1000000000){
          PRINTF("%.1f GB\n", (float)total_size/1000000000.);
        }
        else if(total_size>1000000){
          PRINTF("%.1f MB\n", (float)total_size/1000000.);
        }
        else{
          PRINTF("%.0f kB\n", (float)total_size/1000.);
        }
        PRINTF(" load time: %f s\n",(float)total_size*8.0/1000000000.0);
        PRINTF("   (assuming a gigabit network connection)\n");
      }
    }
    script_multislice=0;
  }
  else if(value<=-1000){
    int submenutype, last_slice, dir, errorcode;
    char *submenulabel;
    slicedata *slicei;
    float load_time, load_size = 0.0;
    int file_count = 0;

    value = -(1000 + value);
    submenutype=value/4;
    dir=value%4;
    submenutype=msubslice_menuindex[submenutype];
    slicei = sliceinfo + submenutype;
    submenulabel = slicei->label.longlabel;
    last_slice = nsliceinfo - 1;
    for(i = nsliceinfo-1; i>=0; i--){
      char *longlabel;

      slicei = sliceinfo + i;
      if(slicei->skipdup == 1)continue;
      longlabel = slicei->label.longlabel;
      if(strcmp(longlabel, submenulabel) != 0)continue;
      if(dir != 0 && dir != slicei->idir)continue;
      if(dir !=0 && slicei->volslice == 1)continue;
      last_slice = i;
      break;
    }
    START_TIMER(load_time);
    for(i = 0; i<nsliceinfo; i++){
      int set_slicecolor;
      char *longlabel;

      slicei = sliceinfo + i;
      if(slicei->skipdup== 1)continue;
      longlabel = slicei->label.longlabel;
      if(strcmp(longlabel,submenulabel)!=0)continue;
      if(dir!=0&&dir!=slicei->idir)continue;
      if(dir!=0&&slicei->volslice==1)continue;
      set_slicecolor = DEFER_SLICECOLOR;
      if(i == last_slice)set_slicecolor = SET_SLICECOLOR;
      if(slicei->slice_filetype == SLICE_GEOM){
        load_size+=ReadGeomData(slicei->patchgeom, slicei, LOAD, ALL_FRAMES, NULL, &errorcode);
      }
      else{
        load_size+=             ReadSlice(slicei->file,i, ALL_FRAMES, NULL, LOAD,set_slicecolor,&errorcode);
      }
      file_count++;
    }
    STOP_TIMER(load_time);
    PRINT_LOADTIMES(file_count,load_size,load_time);
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
        UpdateSliceDups();
        UpdateSliceDupDialog();
      }
      break;

      case  MENU_KEEP_COARSE:
      if(slicedup_option!=SLICEDUP_KEEPCOARSE){
        slicedup_option = SLICEDUP_KEEPCOARSE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateSliceDups();
        UpdateSliceDupDialog();
      }
      break;

      case MENU_KEEP_FINE:
      if(slicedup_option!=SLICEDUP_KEEPFINE){
        slicedup_option = SLICEDUP_KEEPFINE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateSliceDups();
        UpdateSliceDupDialog();
      }
      break;
      case MENU_SLICECOLORDEFER:
        use_set_slicecolor = 1 - use_set_slicecolor;
        updatemenu = 1;
        break;
      case MENU_SLICE_FILE_SIZES:
        compute_slice_file_sizes = 1-compute_slice_file_sizes;
        updatemenu = 1;
        break;
      case MENU_SLICE_SETTINGS:
        ShowBoundsDialog(DLG_SLICE);
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
  }
}

/* ------------------ Plot3DListMenu ------------------------ */

void Plot3DListMenu(int value){
  int i;
  float delta_time=0.5;

  value = CLAMP(value, 0, nplot3dtimelist-1);
  iplot3dtimelist = value;
  LoadPlot3dMenu(UNLOAD_ALL);
  if(scriptoutstream!=NULL){
    fprintf(scriptoutstream,"LOADPLOT3D\n");
    fprintf(scriptoutstream," %f\n",plot3dtimelist[value]);
  }
  if(nplot3dtimelist>1)delta_time = (plot3dtimelist[1]-plot3dtimelist[0])/2.0;
  int *list=NULL, nlist = 0;

  NewMemory((void **)&list, nplot3dinfo*sizeof(int));
  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    if(ABS(plot3di->time-plot3dtimelist[value])<delta_time){
      list[nlist++] = i;
    }
  }
  SetLoadedPlot3DBounds(list, nlist);
  for(i = 0; i<nlist; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+list[i];
    plot3di->finalize = 0;
  }
  for(i = nlist-1; i>=0; i--){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+list[i];
    plot3di->finalize = 1;
    break;;
  }
  for(i=0;i<nlist;i++){
    int errorcode;
    plot3ddata *plot3di;

    plot3di = plot3dinfo + list[i];
    ReadPlot3D(plot3di->file, list[i], LOAD, &errorcode);
  }
  printf("\n");
  FREEMEMORY(list);
}

/* ------------------ UpdateMenu ------------------------ */

void UpdateMenu(void){
  updatemenu=1;
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
}

/* ------------------ LoadPlot3DMenu ------------------------ */

void LoadPlot3dMenu(int value){
  int errorcode;
  int i;

  if(value==MENU_PLOT3D_DUMMY)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    char *plot3dfile;

    plot3dfile = plot3dinfo[value].file;
    if(scriptoutstream!=NULL&&loadplot3dall==0){
      fprintf(scriptoutstream,"LOADPLOT3D\n");
      fprintf(scriptoutstream," %i %f\n",
        plot3dinfo[value].blocknumber+1,plot3dinfo[value].time);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      SetLoadedPlot3DBounds(&value, 1);
      plot3dinfo[value].finalize = 1;
      for(i = 0; i<nplot3dinfo; i++){
        if(plot3dinfo[i].loaded==1){
          ReadPlot3D("", i, UNLOAD, &errorcode);
        }
      }
      ReadPlot3D(plot3dfile,value,LOAD,&errorcode);
    }
  }
  else if(value==RELOAD_ALL){
    plot3ddata **plot3d_list;
    int nlist=0;

    NewMemory((void **)&plot3d_list, nplot3dinfo*sizeof(plot3ddata *));
    for(i = 0; i<nplot3dinfo; i++){
      plot3ddata *plot3di;

      plot3di = plot3dinfo+i;
      if(plot3di->loaded==0)continue;
      plot3di->finalize = 0;
      plot3d_list[nlist++] = plot3di;
    }
    if(nlist>0)SetLoadedPlot3DBounds(&value, 1);
    if(nlist>0)plot3d_list[nlist-1]->finalize = 1;
    for(i = 0; i<nlist; i++){
      ReadPlot3D("", plot3d_list[i]-plot3dinfo, UNLOAD, &errorcode);
    }
    for(i = 0; i<nlist; i++){
      plot3ddata *plot3di;

      plot3di = plot3d_list[i];
      ReadPlot3D(plot3di->file, plot3di-plot3dinfo, LOAD, &errorcode);
    }
    FREEMEMORY(plot3d_list);
  }
  else if(value==UNLOAD_ALL){
    for(i=0;i<nplot3dinfo;i++){
      ReadPlot3D("",i,UNLOAD,&errorcode);
    }
  }
  else if(value==MENU_PLOT3D_SETTINGS){
    ShowBoundsDialog(DLG_PLOT3D);
  }
  else{
    value+=100000;
    for(i = 0; i<nplot3dinfo; i++){
      if(plot3dinfo[i].loaded==1){
        ReadPlot3D("", i, UNLOAD, &errorcode);
      }
    }
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
  ReadIsoFile=1;
  isoi = isoinfo + value;
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
  int file_count=0;
  float load_time=0.0, load_size=0.0;

  START_TIMER(load_time);
  CancelUpdateTriangles();
  for(i = 0; i < nisoinfo; i++){
    isodata *isoi;

    isoi = isoinfo + i;
    if(iso_type==isoi->type){
      load_size+=LoadIsoI(i);
      file_count++;
    }
  }
  STOP_TIMER(load_time);
  PRINT_LOADTIMES(file_count,load_size,load_time);
}

/* ------------------ LoadIsoMenu ------------------------ */

void LoadIsoMenu(int value){
  int errorcode;
  int i;
  int ii;

  if(value==MENU_DUMMY3)return;
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    LoadIsoI(value);
  }
  if(value==-1){
    for(i=0;i<nisoinfo;i++){
      isodata *isoi;

      isoi = isoinfo + i;
      if(isoi->loaded==1)ReadIso("",i,UNLOAD,NULL,&errorcode);
    }
  }
  if(value==MENU_ISO_SETTINGS){
    ShowBoundsDialog(DLG_ISO);
    return;
  }
  if(value<=-10){
    isodata *isoi;

    ii = -(value + 10);
    isoi = isoinfo + ii;
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

void LoadBoundaryMenu(int value){
  int errorcode;
  int i,ii;
  int boundarytypenew;

  GLUTSETCURSOR(GLUT_CURSOR_WAIT);
  if(value>=0){
    boundarytypenew=GetBoundaryType(patchinfo+value);
    if(boundarytypenew!=-1){
      for(ii=0;ii<npatch_loaded;ii++){
        patchdata *patchi;

        i = patch_loaded_list[ii];
        patchi = patchinfo + i;
        if(patchi->shortlabel_index !=boundarytypenew)ReadBoundary(i,UNLOAD,&errorcode);
      }
    }
    if(scriptoutstream!=NULL){
      patchdata *patchi;

      patchi = patchinfo + value;
      fprintf(scriptoutstream,"// LOADFILE\n");
      fprintf(scriptoutstream,"//  %s\n",patchi->file);
      fprintf(scriptoutstream, "LOADBOUNDARYM\n");
      fprintf(scriptoutstream, " %s\n", patchi->label.longlabel);
      fprintf(scriptoutstream, " %i\n", patchi->blocknumber+1);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      LOCK_COMPRESS;
      SetLoadedPatchBounds(&value, 1);
      ReadBoundary(value,LOAD,&errorcode);
      UNLOCK_COMPRESS;
    }
  }
  else if(value<=-10){
    patchdata *patchj;

    value = -(value + 10);
    patchj = patchinfo + value;
    if(scriptoutstream!=NULL){
      fprintf(scriptoutstream,"LOADBOUNDARY\n");
      fprintf(scriptoutstream," %s\n",patchj->label.longlabel);
    }
    if(scriptoutstream==NULL||script_defer_loading==0){
      int file_count=0;
      float load_time=0.0, load_size=0.0;

      START_TIMER(load_time);

      // only perform wrapup operations when loading last boundary file
      for(i = 0; i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo+i;
        patchi->finalize = 0;
      }
      int *list=NULL, nlist=0;

      NewMemory((void **)&list,npatchinfo*sizeof(int));
      nlist=0;
      for(i = 0; i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo+i;
        if(strcmp(patchi->label.longlabel, patchj->label.longlabel)==0&&patchi->patch_filetype==patchj->patch_filetype){
          list[nlist++]=i;
        }
      }
      SetLoadedPatchBounds(list, nlist);
      FREEMEMORY(list);
      for(i = npatchinfo-1; i>=0; i--){
        patchdata *patchi;

        patchi = patchinfo+i;
#ifdef pp_MERGE_GEOMS
        if(strcmp(patchi->label.longlabel, patchj->label.longlabel)==0){
#else
        if(strcmp(patchi->label.longlabel, patchj->label.longlabel)==0&&patchi->patch_filetype==patchj->patch_filetype){
#endif
          LOCK_COMPRESS;
          patchi->finalize = 1;
          UNLOCK_COMPRESS;
          break;
        }
      }
      for(i=0;i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo + i;
#ifdef pp_MERGE_GEOMS
        if(strcmp(patchi->label.longlabel,patchj->label.longlabel)==0){
#else
        if(strcmp(patchi->label.longlabel,patchj->label.longlabel)==0&&patchi->patch_filetype==patchj->patch_filetype){
#endif
          LOCK_COMPRESS;
          if(patchi->structured == YES){
            PRINTF("Loading %s(%s)", patchi->file, patchi->label.shortlabel);
          }
          load_size+=ReadBoundary(i, LOAD, &errorcode);
          if(patchi->structured!=NO&&patchi->finalize==1){
            UpdateTriangles(GEOM_STATIC, GEOM_UPDATE_ALL);
          }
          file_count++;
          UNLOCK_COMPRESS;
        }
      }
      STOP_TIMER(load_time);
      PRINT_LOADTIMES(file_count,load_size,load_time);
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
    case MENU_BNDF_SHOW_MESH_INTERFACE:
      show_bndf_mesh_interface = 1-show_bndf_mesh_interface;
      updatemenu = 1;
      break;
    case MENU_UPDATEBOUNDS:
      UpdateAllBoundaryBounds();
      break;
    case MENU_BOUNDARY_SETTINGS:
      ShowBoundsDialog(DLG_BOUNDARY);
      break;
      case MENU_KEEP_ALL:
      if(boundaryslicedup_option!=SLICEDUP_KEEPALL){
        boundaryslicedup_option = SLICEDUP_KEEPALL;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateBoundarySliceDups();
        UpdateSliceDupDialog();
      }
      break;

      case  MENU_KEEP_COARSE:
      if(boundaryslicedup_option!=SLICEDUP_KEEPCOARSE){
        boundaryslicedup_option = SLICEDUP_KEEPCOARSE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateBoundarySliceDups();
        UpdateSliceDupDialog();
      }
      break;

      case MENU_KEEP_FINE:
      if(boundaryslicedup_option!=SLICEDUP_KEEPFINE){
        boundaryslicedup_option = SLICEDUP_KEEPFINE;
        updatemenu = 1;
        GLUTPOSTREDISPLAY;
        UpdateBoundarySliceDups();
        UpdateSliceDupDialog();
      }
      break;
    default:
      for(i=0;i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo+i;
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

/* ------------------ ShowBoundaryMenu ------------------------ */

void ShowBoundaryMenu(int value){
  updatemenu=1;
  updatefacelists=1;
  GLUTPOSTREDISPLAY;
  if(value>=1000){
    int ii;
    patchdata *patchj;

    patchj = patchinfo + value-1000;
    patchj->display = 1 - patchj->display;
    for(ii=0;ii<npatch_loaded;ii++){
      patchdata *patchi;
      int i;

      i = patch_loaded_list[ii];
      patchi = patchinfo + i;
      if(strcmp(patchi->label.longlabel,patchj->label.longlabel)==0)patchi->display=patchj->display;
    }
    UpdateBoundaryType();
  }
  if(value==SHOW_CHAR){
    vis_threshold = 1 - vis_threshold;
    UpdateChar();
  }
  if(value==GLUI_SHOWALL_BOUNDARY||value==GLUI_HIDEALL_BOUNDARY){
    int ii;

    if(value == GLUI_SHOWALL_BOUNDARY){
      show_boundaryfiles = 1;
    }
    if(value == GLUI_HIDEALL_BOUNDARY){
      show_boundaryfiles = 0;
    }
    for(ii=0;ii<npatch_loaded;ii++){
      patchdata *patchi;
      int i;

      i = patch_loaded_list[ii];
      patchi = patchinfo + i;
      if(patchi->structured == YES)patchi->display=show_boundaryfiles;
    }
  }
  if(value<0){
    if(value==ShowEXTERIORwallmenu||value==HideEXTERIORwallmenu){
      int i,ii,val;

      if(value==ShowEXTERIORwallmenu){
        val = 1;
      }
      else{
        val = 0;
      }
      for(ii = 0;ii < npatch_loaded;ii++){
        int n;

        patchdata *patchi;
        meshdata *meshi;

        patchi = patchinfo + patch_loaded_list[ii];
        meshi = meshinfo + patchi->blocknumber;
        for(n = 0;n < meshi->npatches;n++){
          if(meshi->boundarytype[n] != INTERIORwall){
            meshi->vis_boundaries[n] = val;
          }
        }
      }
      for(i=1;i<7;i++){
        vis_boundary_type[i]=val;
      }
    }
    else if(value==INTERIORwallmenu){
      int ii,val;

      allinterior = 1 - allinterior;
      val = allinterior;
      vis_boundary_type[INTERIORwall]=val;
      for(ii = 0;ii < npatch_loaded;ii++){
        patchdata *patchi;
        meshdata *meshi;
        int n;

        patchi = patchinfo + patch_loaded_list[ii];
        meshi = meshinfo + patchi->blocknumber;
        for(n = 0;n < meshi->npatches;n++){
          if(meshi->boundarytype[n] == INTERIORwall){
            meshi->vis_boundaries[n] = val;
          }
        }
      }
    }
    if(value==INI_EXTERIORwallmenu){
      int ii;

      for(ii = 0;ii < npatch_loaded;ii++){
        int n;

        patchdata *patchi;
        meshdata *meshi;

        patchi = patchinfo + patch_loaded_list[ii];
        meshi = meshinfo + patchi->blocknumber;
        for(n = 0;n < meshi->npatches;n++){
          if(meshi->boundarytype[n] != INTERIORwall){
            meshi->vis_boundaries[n] = vis_boundary_type[meshi->boundarytype[n]];
          }
        }
      }
    }
    else if(value != DUMMYwallmenu){
      int ii;

      value = -(value + 2); /* map xxxwallmenu to xxxwall */
      for(ii = 0;ii < npatch_loaded;ii++){
        patchdata *patchi;
        meshdata *meshi;
        int n;

        patchi = patchinfo + patch_loaded_list[ii];
        meshi = meshinfo + patchi->blocknumber;
        for(n = 0;n < meshi->npatches;n++){
          if(meshi->boundarytype[n] == value){
            meshi->vis_boundaries[n] = 1 - meshi->vis_boundaries[n];
            vis_boundary_type[value] = meshi->vis_boundaries[n];
          }
        }
      }
    }
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
    visOtherVents=1;
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
     updatefaces=1;
     break;
  case MENU_VENT_TWOEXTERIOR:
     show_bothsides_ext = 1 - show_bothsides_ext;
     updatefaces=1;
     break;
  case MENU_VENT_TRANSPARENT:
     show_transparent_vents=1-show_transparent_vents;
     updatefaces=1;
     break;
  case MENU_VENT_OTHER:
     visOtherVents=1-visOtherVents;
     break;
   case HIDE_ALL_VENTS: // Hide all vents
     visVents=0;
     visOpenVents=0;
     visDummyVents=0;
     visOtherVents=0;
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
    ASSERT(FFALSE);
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
#define GEOMETRY_TETRA_HIDE             11
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
    case GEOM_BOUNDING_BOX_ALWAYS:
      if(show_geom_boundingbox==SHOW_BOUNDING_BOX_ALWAYS){
        show_geom_boundingbox = SHOW_BOUNDING_BOX_NEVER;
      }
      else{
        show_geom_boundingbox = SHOW_BOUNDING_BOX_ALWAYS;
      }
      UpdateGeomBoundingBox();
      break;
    case GEOM_BOUNDING_BOX_MOUSE_DOWN:
      if(show_geom_boundingbox==SHOW_BOUNDING_BOX_MOUSE_DOWN){
        show_geom_boundingbox = SHOW_BOUNDING_BOX_NEVER;
      }
      else{
        show_geom_boundingbox = SHOW_BOUNDING_BOX_MOUSE_DOWN;
      }
      UpdateGeomBoundingBox();
      break;
    case GEOMETRY_TERRAIN_SHOW_TOP:
      terrain_showonly_top = 1 - terrain_showonly_top;
      UpdateShowOnlyTop();
      break;
    case GEOMETRY_INTERIOR_SOLID:
      show_volumes_solid=1-show_volumes_solid;
      break;
    case GEOMETRY_INTERIOR_OUTLINE:
      show_volumes_outline=1-show_volumes_outline;
      break;
    case GEOMETRY_TETRA_HIDE:
      show_volumes_solid=0;
      show_volumes_outline=0;
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
      ImmersedMenu(GEOMETRY_TETRA_HIDE);
      show_geom_normal = 0;
      break;
    case MENU_DUMMY:
      break;
    case GEOMETRY_INSIDE_DOMAIN:
      showgeom_inside_domain = 1 - showgeom_inside_domain;
      UpdateWhereFaceVolumes();
      break;
    case GEOMETRY_OUTSIDE_DOMAIN:
      showgeom_outside_domain = 1 - showgeom_outside_domain;
      UpdateWhereFaceVolumes();
      break;
    case GEOMETRY_VOLUMES_INTERIOR:
      show_volumes_interior = 1 - show_volumes_interior;
      UpdateWhereFaceVolumes();
      break;
    case GEOMETRY_VOLUMES_EXTERIOR:
      show_volumes_exterior = 1 - show_volumes_exterior;
      UpdateWhereFaceVolumes();
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  UpdateGeometryControls();

  GLUTPOSTREDISPLAY;
}

/* ------------------ BlockageMenu ------------------------ */

void BlockageMenu(int value){
  int change_state=0;

  if(value==visLightFaces){
    light_faces = 1 - light_faces;
    updatemenu=1;
    GLUTPOSTREDISPLAY;
    return;
  }
  if(solid_state<0)solid_state=visBlocks;
  if(outline_state<0)outline_state=OUTLINE_NONE;
  switch(value){
    case visBLOCKOutlineColor:
      outline_color_flag = 1 - outline_color_flag;
      updatefaces=1;
      break;
    case visBLOCKOnlyOutline:
      if(outline_state!=OUTLINE_ONLY){
        outline_state=OUTLINE_ONLY;
      }
      else{
        outline_state=OUTLINE_NONE;
      }
      change_state=1;
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
      change_state=1;
      break;
    case visBLOCKAsInput:
      solid_state=visBLOCKAsInput;
      if(outline_state==OUTLINE_ONLY)outline_state=OUTLINE_ADDED;
      change_state=1;
      break;
    case visBLOCKNormal:
      solid_state=visBLOCKNormal;
      if(outline_state==OUTLINE_ONLY)outline_state=OUTLINE_ADDED;
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
      ASSERT(FFALSE);
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
            ASSERT(FFALSE);
            break;
        }
        break;
      default:
        ASSERT(FFALSE);
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
     UpdateTrainerOutline();
     break;
   case visBLOCKNormal:
   case visBLOCKOutline:
   case visBLOCKHide:
   case visBLOCKSolidOutline:
     visBlocks=value;
     if(value==visBLOCKSolidOutline||visBLOCKold==visBLOCKSolidOutline)updatefaces=1;
     UpdateTrainerOutline();
     break;
   case BLOCKlocation_grid:
   case BLOCKlocation_exact:
   case BLOCKlocation_cad:
     blocklocation=value;
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
       if(value>=0&&value<=npropinfo-1){
         propdata *propi;

         propi = propinfo + value;
         propi->blockvis=1-propi->blockvis;
       }
     }
     else{
       ASSERT(FFALSE);
     }
     break;
  }
  visBLOCKold=value;
  updatemenu=1;
 // updatefaces=1;
  updatefacelists=1;
  updatehiddenfaces=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ RotateTypeMenu ------------------------ */

void RotateTypeMenu(int value){
  if(value==MENU_DUMMY)return;
  if(value==MENU_MOTION_SETTINGS){
    ShowGluiMotion(DIALOG_MOTION);
    return;
  }
  else if(value == MENU_MOTION_SHOW_VECTORS){
	showgravity_vector = 1-showgravity_vector;
	UpdateShowGravityVector();
  }
  else if(value == MENU_MOTION_GRAVITY_VECTOR){
    gvec_down = 1;
#define USE_GVEC 28
    SceneMotionCB(USE_GVEC);
  }
  else if(value==MENU_MOTION_Z_VECTOR){
#define ZAXIS_UP 41
    SceneMotionCB(ZAXIS_UP);
  }
  else{
    rotation_type = value;
    UpdateRotationType(rotation_type);
    RotationTypeCB(rotation_type);
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
    ASSERT(FFALSE);
    break;
  }
  SetLabelControls();
}

/* ------------------ ShowADeviceType ------------------------ */

void ShowADeviceType(void){
  int i;

  for(i=0;i<nobject_defs;i++){
    sv_object *obj_typei;

    obj_typei = object_defs[i];
    if(obj_typei->used_by_device==1&&obj_typei->visible==1)return;
  }
  for(i=0;i<nobject_defs;i++){
    sv_object *obj_typei;

    obj_typei = object_defs[i];
    if(obj_typei->used_by_device==1){
      obj_typei->visible=1;
      return;
    }
  }
}

/* ------------------ DeviceTypeMenu ------------------------ */

void DeviceTypeMenu(int val){
  UpdateDeviceTypes(val);
  DeviceCB(DEVICE_devicetypes);
}

/* ------------------ ShowObjectsMenu ------------------------ */

void ShowObjectsMenu(int value){
  sv_object *objecti;
  int i;

  if(value>=0&&value<nobject_defs){
    objecti = object_defs[value];
    objecti->visible = 1 - objecti->visible;
    if(showdevice_val==1||showdevice_plot!=DEVICE_PLOT_HIDDEN){
      update_times = 1;
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      UpdateDeviceShow();
    }
  }
  else if(value == OBJECT_MISSING){
    updatemenu = 1;
    show_missing_objects = 1 - show_missing_objects;
  }
  else if(value==OBJECT_SHOWALL){
    for(i=0;i<nobject_defs;i++){
      objecti = object_defs[i];
      objecti->visible=1;
    }
  }
  else if(value==OBJECT_HIDEALL){
    for(i=0;i<nobject_defs;i++){
      objecti = object_defs[i];
      objecti->visible=0;
    }
  }
  else if(value==OBJECT_SELECT){
    select_device=1-select_device;
  }
  else if(value==OBJECT_PLOT_SHOW_TREE_ALL){
    update_times=1;
    if(showdevice_plot==DEVICE_PLOT_SHOW_TREE_ALL){
      showdevice_plot = DEVICE_PLOT_HIDDEN;
    }
    else{
      showdevice_plot = DEVICE_PLOT_SHOW_TREE_ALL;
      select_device = 1;
      ShowADeviceType();
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateDeviceShow();
  }
  else if(value==PLOT_HRRPUV){
    show_hrrpuv_plot = 1-show_hrrpuv_plot;
    UpdateShowHRRPUVPlot(show_hrrpuv_plot);
    if(show_hrrpuv_plot==1){
      visHRRlabel = 0;
      LabelMenu(MENU_LABEL_hrr);
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateShow();
    update_times = 1;
  }
  else if(value==OBJECT_PLOT_SHOW_ALL){
    update_times=1;
    if(showdevice_plot==DEVICE_PLOT_SHOW_ALL){
      showdevice_plot = DEVICE_PLOT_HIDDEN;
    }
    else{
      showdevice_plot = DEVICE_PLOT_SHOW_ALL;
      select_device = 1;
      ShowADeviceType();
    }
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateDeviceShow();
  }
  else if(value==OBJECT_PLOT_SHOW_SELECTED){
    update_times = 1;
    if(showdevice_plot==DEVICE_PLOT_SHOW_SELECTED){
      showdevice_plot = DEVICE_PLOT_HIDDEN;
    }
    else{
      showdevice_plot = DEVICE_PLOT_SHOW_SELECTED;
      select_device = 1;
      ShowADeviceType();
    }
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    UpdateDeviceShow();
  }
  else if(value==MENU_DEVICE_SETTINGS){
    ShowGluiPlotDevice();
  }
  else if(value==OBJECT_VALUES){
    update_times=1;
    showdevice_val = 1 - showdevice_val;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateDeviceShow();
  }
  else if(value==OBJECT_OUTLINE){
    object_outlines=1-object_outlines;
  }
  else if(value==OBJECT_BOX){
    object_box = 1-object_box;
  }
  else if(value==OBJECT_ORIENTATION){
    show_device_orientation=1-show_device_orientation;
    UpdateDeviceOrientation();
  }
  else if(value == OBJECT_SHOWBEAM){
    showbeam_as_line = 1 - showbeam_as_line;
    UpdateShowbeamAsLine();
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

    if(value>=0&&value<nterrain_textures){
      texti = terrain_textures+value;
      texti->display = 1-texti->display;
      UpdateTerrainTexture(value);
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
    case MENU_TERRAIN_BOUNDING_BOX:
      if(show_geom_boundingbox==SHOW_BOUNDING_BOX_ALWAYS){
        show_geom_boundingbox = SHOW_BOUNDING_BOX_NEVER;
      }
      else{
        show_geom_boundingbox = SHOW_BOUNDING_BOX_ALWAYS;
      }
      UpdateGeomBoundingBox();
      break;
    case MENU_TERRAIN_BOUNDING_BOX_AUTO:
      if(show_geom_boundingbox==SHOW_BOUNDING_BOX_MOUSE_DOWN){
        show_geom_boundingbox = SHOW_BOUNDING_BOX_NEVER;
      }
      else{
        show_geom_boundingbox = SHOW_BOUNDING_BOX_MOUSE_DOWN;
      }
      UpdateGeomBoundingBox();
      break;
    default:
      ASSERT(0);
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
  case MENU_ZONE_HORIZONTAL:
    if(zonecolortype==ZONESMOKE_COLOR)zonecolortype = ZONETEMP_COLOR;
    visVZone=0;
    visHZone=1;
    visZone=1;
    break;
  case MENU_ZONE_VERTICAL:
    if(zonecolortype==ZONESMOKE_COLOR)zonecolortype = ZONETEMP_COLOR;
    visVZone = 1;
    visHZone=0;
    visZone=1;
    break;
  case MENU_ZONE_LAYERHIDE:
    visVZone=0;
    visHZone=0;
    visSZone=0;
    break;
  case MENU_ZONE_2DHAZARD:
    zonecolortype=ZONEHAZARD_COLOR;
    visSZone=0;
    if(visVZone==0&&visHZone==0)visVZone=1;
    visZone=1;
    break;
  case MENU_ZONE_2DTEMP:
  case MENU_ZONE_2DTEMP2:
    show_zonelower = 1;
    if(value==MENU_ZONE_2DTEMP)show_zonelower = 0;
    zonecolortype=ZONETEMP_COLOR;
    visSZone=0;
    if(visVZone==0&&visHZone==0)visVZone=1;
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
#define VISVENTFLOW     if((nzhvents>0&&visVentHFlow==1)||(nzvvents>0&&visVentVFlow==1)||(nzmvents>0&&visVentMFlow==1)){\
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
    ASSERT(FFALSE);
  }
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ GeometryMenu ------------------------ */

void GeometryMenu(int value){

  switch(value){
  case GEOM_Outline:
    if(isZoneFireModel==0)visFrame=1-visFrame;
    break;
  case 5:
    visFloor=1-visFloor;
    break;
  case 6:
    visWalls=1-visWalls;
    break;
  case 7:
    visCeiling=1-visCeiling;
    break;
  case 17+TERRAIN_SKIP:
    terrain_skip = 1-terrain_skip;
    updatemenu = 1;
    break;
  case 17+TERRAIN_DEBUG:
    terrain_debug = 1-terrain_debug;
    updatemenu = 1;
    break;
  case 17+TERRAIN_TOP:
    terrain_showonly_top = 1 - terrain_showonly_top;
    updatemenu = 1;
    break;
  case 17+TERRAIN_3D:
  case 17+TERRAIN_3D_MAP:
  case 17+TERRAIN_HIDDEN:
    if(value==17+TERRAIN_HIDDEN){
      BlockageMenu(visBlocksSave);
      if(visOtherVents!=visOtherVentsSAVE){
        visOtherVents=visOtherVentsSAVE;
      }
    }
    else{
      if(visOtherVents!=0){
        visOtherVentsSAVE=visOtherVents;
        visOtherVents=0;
      }
    }
    visTerrainType=value-17;
    if(visTerrainType==TERRAIN_3D){
      planar_terrain_slice=0;
    }
    else{
      planar_terrain_slice=1;
    }
    break;
  case GEOM_ShowAll:
    if(isZoneFireModel)visFrame=1;
    /*
    visFloor=1;
    visWalls=1;
    visCeiling=1;
    */
    show_faces_shaded=1;
    visVents=1;
    BlockageMenu(visBLOCKAsInput);
    break;
  case GEOM_HideAll:
    visFrame=0;
    visFloor=0;
    visWalls=0;
    visCeiling=0;
    visVents=0;
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
  default:
    ASSERT(FFALSE);
    break;
  }
  updatefacelists=1;
  updatemenu=1;
  GLUTPOSTREDISPLAY;
}

/* ------------------ GetNumActiveDevices ------------------------ */

int GetNumActiveDevices(void){
  int num_activedevices = 0;

  if(nobject_defs > 0){
    int i;

    for(i = 0; i < nobject_defs; i++){
      sv_object *obj_typei;

      obj_typei = object_defs[i];
      if(obj_typei->used_by_device == 1)num_activedevices++;
    }
  }
  return num_activedevices;
}

/* ------------------ GetNTotalVents ------------------------ */

int GetNTotalVents(void){
  int ntotal_vents = 0;
  int i;

  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    ntotal_vents += meshi->nvents;
  }
  return ntotal_vents;
}

/* ------------------ IsBoundaryType ------------------------ */

int IsBoundaryType(int type){
  int i;

  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;
    int n;

    meshi = meshinfo + i;
    for(n = 0; n < meshi->npatches; n++){
      if(meshi->boundarytype[n] == type)return 1;
    }
  }
  return 0;
}

/* ------------------ IsoLoadState ------------------------ */

void IsoLoadState(isodata *isoi, int  *load_state){
  int i, total=0, loaded=0;

  for(i=0; i<nisoinfo; i++){
    isodata *isoii;

    isoii = isoinfo + i;
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

  for(i=0; i<npatchinfo; i++){
    patchdata *patchii;

    patchii = patchinfo + i;
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

  for(i=0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo + i;
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

  for(i=0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo + i;
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

/* ------------------ InitMenus ------------------------ */

void InitMenus(int unload){
  int i;
  int nmultisliceloaded;
  int showhide_data = 0;
  int patchgeom_slice_showhide;


static int filesdialogmenu = 0, viewdialogmenu = 0, datadialogmenu = 0, windowdialogmenu=0;
static int labelmenu=0, titlemenu=0, colorbarmenu=0, colorbarsmenu=0, colorbarshademenu, smokecolorbarmenu=0, showhidemenu=0,colorbardigitmenu=0;
static int optionmenu=0, rotatetypemenu=0;
static int resetmenu=0, defaultviewmenu=0, frameratemenu=0, rendermenu=0, smokeviewinimenu=0, inisubmenu=0, resolutionmultipliermenu=0;
static int terrain_geom_showmenu = 0;
static int render_resolutionmenu=0, render_filetypemenu=0, render_filesuffixmenu=0, render_skipmenu=0;
static int render_startmenu = 0;
#ifdef pp_COMPRESS
static int compressmenu=0;
#endif
static int showhideslicemenu=0, sliceskipmenu=0, showvslicemenu=0;
static int plot3dshowmenu=0, staticvariablemenu=0, helpmenu=0, webhelpmenu=0, keyboardhelpmenu=0, mousehelpmenu=0;
static int vectorskipmenu=0,unitsmenu=0;
static int isosurfacemenu=0, isovariablemenu=0, levelmenu=0;
static int fontmenu=0, aperturemenu=0,dialogmenu=0,zoommenu=0;
static int gridslicemenu=0, griddigitsmenu=0, blockagemenu=0, immersedmenu=0, immersedinteriormenu=0, immersedsurfacemenu=0, loadpatchmenu=0, ventmenu=0, circularventmenu=0;
static int loadpatchsinglemenu=0,loadsmoke3dsinglemenu=0,loadvolsmokesinglemenu=0,unloadsmoke3dsinglemenu=0, showvolsmokesinglemenu=0, includepatchmenu=0;
static int plot3dshowsinglemeshmenu=0;
static int showsingleslicemenu=0,plot3dsinglemeshmenu=0;
static int loadisomenu=0, isosinglemeshmenu=0, isosurfacetypemenu=0,showpatchsinglemenu=0,showpatchextmenu=0;
static int geometrymenu=0, loadunloadmenu=0, reloadmenu=0, fileinfomenu=0, aboutmenu=0, disclaimermenu=0, terrain_obst_showmenu=0;
static int scriptmenu=0;
static int scriptlistmenu=0,scriptsteplistmenu=0,scriptrecordmenu=0;
#ifdef pp_LUA
static int luascriptmenu=0;
static int luascriptlistmenu=0;
#endif
static int loadplot3dmenu=0, unloadvslicemenu=0, unloadslicemenu=0;
static int loadsmoke3dmenu=0,loadsmoke3dsootmenu=0,loadsmoke3dhrrmenu=0;
static int loadsmoke3dtempmenu = 0, loadsmoke3dco2menu = 0;
static int loadvolsmoke3dmenu=0,showvolsmoke3dmenu=0;
static int unloadsmoke3dmenu=0,unloadvolsmoke3dmenu=0;
static int unloadevacmenu=0, unloadpartmenu=0, loadslicemenu=0, loadmultislicemenu=0;
static int *loadsubvslicemenu=NULL, nloadsubvslicemenu=0;
static int *loadsubslicemenu=NULL, nloadsubslicemenu=0, iloadsubslicemenu=0;
static int *loadsubpatchmenu_b = NULL, *nsubpatchmenus_b=NULL, iloadsubpatchmenu_b=0, nloadsubpatchmenu_b = 0;
static int *loadsubpatchmenu_s = NULL, *nsubpatchmenus_s=NULL, iloadsubpatchmenu_s=0, nloadsubpatchmenu_s = 0;
static int *loadsubmslicemenu=NULL, nloadsubmslicemenu=0;
static int *loadsubmvslicemenu=NULL, nloadsubmvslicemenu=0;
static int *loadsubplot3dmenu=NULL, nloadsubplot3dmenu=0;
static int loadmultivslicemenu=0, unloadmultivslicemenu=0;
static int duplicatevectorslicemenu=0, duplicateslicemenu=0, duplicateboundaryslicemenu=0;
static int unloadmultislicemenu=0, vsliceloadmenu=0, staticslicemenu=0;
static int evacmenu=0, particlemenu=0, particlesubmenu=0, showpatchmenu=0, zonemenu=0, isoshowmenu=0, isoshowsubmenu=0, isolevelmenu=0, smoke3dshowmenu=0;
static int smoke3dshowsinglemenu = 0;
static int particlepropshowmenu=0,humanpropshowmenu=0;
static int *particlepropshowsubmenu=NULL;
static int particlestreakshowmenu=0;
static int tourmenu=0,tourcopymenu=0;
static int avatartourmenu=0,avatarevacmenu=0;
static int trainerviewmenu=0,mainmenu=0,zoneshowmenu=0,particleshowmenu=0,evacshowmenu=0;
static int showobjectsmenu=0,showobjectsplotmenu=0,devicetypemenu=0,spheresegmentmenu=0,propmenu=0;
static int unloadplot3dmenu=0, unloadpatchmenu=0, unloadisomenu=0;
static int showmultislicemenu=0;
static int textureshowmenu=0;
#ifdef _DEBUG
static int menu_count=0;
static int in_menu=0;
#endif

updatemenu=0;
#ifdef _DEBUG
  PRINTF("Updating Menus %i In menu %i\n",menu_count++,in_menu);
  in_menu=1;
#endif
  UpdateShowHideButtons();
  GLUTPOSTREDISPLAY;

  for(i=0;i<nmultisliceinfo;i++){
    multislicedata *mslicei;
    int j;

    mslicei = multisliceinfo + i;
    mslicei->loaded=0;
    mslicei->display=0;
    for(j=0;j<mslicei->nslices;j++){
      slicedata *sd;

      sd = sliceinfo + mslicei->islices[j];
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
  for(i=0;i<nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    int j;

    mvslicei = multivsliceinfo + i;
    mvslicei->loaded=0;
    mvslicei->display=0;
    for(j=0;j<mvslicei->nvslices;j++){
      vslicedata *vd;

      vd = vsliceinfo + mvslicei->ivslices[j];
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

#ifdef pp_DEBUG_SUBMENU

#define CREATEMENU(menu,Menu) menu=glutCreateMenu(Menu);\
  if(nmenus<10000){\
    strcpy(menuinfo[nmenus].label,#Menu);\
    menuinfo[nmenus].menuvar_ptr=&menu;\
    menuinfo[nmenus++].menuvar = menu;\
  }

#ifdef _DEBUG
#define GLUTADDSUBMENU(menu_label,menu_value){ASSERT(menu_value!=0);glutAddSubMenu(menu_label,menu_value);}
#else
#define GLUTADDSUBMENU(menu_label,menu_value){if(menu_value==0){printf("*** warning: sub-menu entry %s added to non-existant menu at line %i in file %s\n",menu_label,__LINE__,__FILE__);};glutAddSubMenu(menu_label,menu_value);}
#endif

#else

#define CREATEMENU(menu,Menu) menu=glutCreateMenu(Menu);\
  if(nmenus<10000){\
    strcpy(menuinfo[nmenus].label,#Menu);\
    menuinfo[nmenus++].menuvar=menu;\
  }

#define GLUTADDSUBMENU(menu_label,menu_value) glutAddSubMenu(menu_label,menu_value)

#endif

  {
    for(i=0;i<nmenus;i++){
      menudata *menui;

      menui = menuinfo + i;

      if(menui->menuvar>0){
        glutDestroyMenu(menui->menuvar);
#ifdef pp_DEBUG_SUBMENU
        *(menui->menuvar_ptr) = 0;
#endif
      }
    }
    nmenus=0;
  }
  if(nloadsubpatchmenu_b > 0){
    FREEMEMORY(loadsubpatchmenu_b);
    FREEMEMORY(nsubpatchmenus_b);
  }
  if(nloadsubpatchmenu_s>0){
    FREEMEMORY(loadsubpatchmenu_s);
    FREEMEMORY(nsubpatchmenus_s);
  }
  if(nloadsubslicemenu>0){
    FREEMEMORY(loadsubslicemenu);
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
  if(unload==UNLOAD)return;

  patchgeom_slice_showhide = 0;
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo+i;
    if(patchi->loaded==1){
      if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
        patchgeom_slice_showhide = 1;
      }
    }
  }

/* --------------------------------patch menu -------------------------- */
  if(npatchinfo>0){
    int ii;
    char menulabel[1024];
    int next_total=0;

    CREATEMENU(showpatchsinglemenu,ShowBoundaryMenu);
    for(ii=0;ii<npatchinfo;ii++){
      patchdata *patchi;

      i = patchorderindex[ii];
      patchi = patchinfo+i;
      if(patchi->loaded==0)continue;
      STRCPY(menulabel, "");
      if(patchi->display==1&&patchi->shortlabel_index ==iboundarytype){
        STRCAT(menulabel,"*");
      }
      STRCAT(menulabel,patchi->menulabel);
      glutAddMenuEntry(menulabel,1000+i);
    }

    CREATEMENU(showpatchextmenu, ShowBoundaryMenu);
      for(i=1;i<7;i++){
        next_total+=vis_boundary_type[i];
      }
    if(next_total == 6){
      glutAddMenuEntry(_("*Show all"),  ShowEXTERIORwallmenu);
      glutAddMenuEntry(_("Hide all"),   HideEXTERIORwallmenu);
    }
    else if(next_total == 0){
      glutAddMenuEntry(_("Show all"),  ShowEXTERIORwallmenu);
      glutAddMenuEntry(_("*Hide all"), HideEXTERIORwallmenu);
    }
    else{
      glutAddMenuEntry(_("#Show all"),  ShowEXTERIORwallmenu);
      glutAddMenuEntry(_("#Hide all"),  HideEXTERIORwallmenu);
    }
    if(IsBoundaryType(FRONTwall) == 1 && vis_boundary_type[FRONTwall] == 1)glutAddMenuEntry(_("*Front"), FRONTwallmenu);
    if(IsBoundaryType(FRONTwall) == 1 && vis_boundary_type[FRONTwall] == 0)glutAddMenuEntry(_("Front"), FRONTwallmenu);
    if(IsBoundaryType(BACKwall) == 1 && vis_boundary_type[BACKwall] == 1)glutAddMenuEntry(_("*Back"), BACKwallmenu);
    if(IsBoundaryType(BACKwall) == 1 && vis_boundary_type[BACKwall] == 0)glutAddMenuEntry(_("Back"), BACKwallmenu);
    if(IsBoundaryType(LEFTwall) == 1 && vis_boundary_type[LEFTwall] == 1)glutAddMenuEntry(_("*Left"), LEFTwallmenu);
    if(IsBoundaryType(LEFTwall) == 1 && vis_boundary_type[LEFTwall] == 0)glutAddMenuEntry(_("Left"), LEFTwallmenu);
    if(IsBoundaryType(RIGHTwall) == 1 && vis_boundary_type[RIGHTwall] == 1)glutAddMenuEntry(_("*Right"), RIGHTwallmenu);
    if(IsBoundaryType(RIGHTwall) == 1 && vis_boundary_type[RIGHTwall] == 0)glutAddMenuEntry(_("Right"), RIGHTwallmenu);
    if(IsBoundaryType(UPwall) == 1 && vis_boundary_type[UPwall] == 1)glutAddMenuEntry(_("*Up"), UPwallmenu);
    if(IsBoundaryType(UPwall) == 1 && vis_boundary_type[UPwall] == 0)glutAddMenuEntry(_("Up"), UPwallmenu);
    if(IsBoundaryType(DOWNwall) == 1 && vis_boundary_type[DOWNwall] == 1)glutAddMenuEntry(_("*Down"), DOWNwallmenu);
    if(IsBoundaryType(DOWNwall) == 1 && vis_boundary_type[DOWNwall] == 0)glutAddMenuEntry(_("Down"), DOWNwallmenu);

    CREATEMENU(showpatchmenu,ShowBoundaryMenu);
    if(npatchloaded>0){
      patchdata *patchi=NULL, *patchim1=NULL;

      for(ii = 0;ii<npatchinfo;ii++){

        i = patchorderindex[ii];
        patchi = patchinfo+i;
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
            glutAddMenuEntry("Hide all",  GLUI_SHOWALL_BOUNDARY);
          }
          else{
            glutAddMenuEntry("Show all",  GLUI_SHOWALL_BOUNDARY);
            glutAddMenuEntry("*Hide all", GLUI_SHOWALL_BOUNDARY);
          }
        }
      }
      if(npatchloaded>1)GLUTADDSUBMENU(_("Mesh"), showpatchsinglemenu);
    }
    npatchloaded=0;
    {
      int local_do_threshold=0;

      for(i = 0;i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo+i;
        if(patchi->loaded==0)continue;
        if(patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0)continue;
        npatchloaded++;
      }
      for(ii=0;ii<npatchinfo;ii++){
        patchdata *patchi;

        i = patchorderindex[ii];
        patchi = patchinfo+i;
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
    if(vis_boundary_type[INTERIORwall]==1)glutAddMenuEntry(_("*Interior"),INTERIORwallmenu);
    if(vis_boundary_type[INTERIORwall]==0)glutAddMenuEntry(_("Interior"),INTERIORwallmenu);
  }

  /* --------------------------------terrain menu -------------------------- */


  if(nterrain_textures>0){
    CREATEMENU(terrain_geom_showmenu, TerrainGeomShowMenu);
    for(i = 0; i<nterrain_textures; i++){
      texturedata *texti;

      texti = terrain_textures+i;
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

  if(have_volumes==1){
    CREATEMENU(immersedsurfacemenu,ImmersedMenu);
  }
  else{
    CREATEMENU(immersedmenu, ImmersedMenu);
  }
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
  else {
    glutAddMenuEntry(_("   Inside Domain domain"), GEOMETRY_INSIDE_DOMAIN);
  }
  if(showgeom_outside_domain == 1){
    glutAddMenuEntry(_("   *Outside FDS domain"), GEOMETRY_OUTSIDE_DOMAIN);
  }
  else {
    glutAddMenuEntry(_("   Outside FDS domain"), GEOMETRY_OUTSIDE_DOMAIN);
  }
  glutAddMenuEntry("-", GEOMETRY_DUMMY);
  if(nterrain_textures>0){
    GLUTADDSUBMENU(_("Terrain images"), terrain_geom_showmenu);
  }
  if(terrain_nindices>0){
    if(terrain_showonly_top==1)glutAddMenuEntry(_("*Show only top surface"), GEOMETRY_TERRAIN_SHOW_TOP);
    if(terrain_showonly_top==0)glutAddMenuEntry(_("Show only top surface"), GEOMETRY_TERRAIN_SHOW_TOP);
  }
  if(ngeominfoptrs>0){
    if(show_geom_boundingbox==SHOW_BOUNDING_BOX_ALWAYS)glutAddMenuEntry(_("*bounding box(always)"),         GEOM_BOUNDING_BOX_ALWAYS);
    if(show_geom_boundingbox!=SHOW_BOUNDING_BOX_ALWAYS)glutAddMenuEntry(_("bounding box(always)"),          GEOM_BOUNDING_BOX_ALWAYS);
    if(show_geom_boundingbox==SHOW_BOUNDING_BOX_MOUSE_DOWN)glutAddMenuEntry(_("*bounding box(mouse down)"), GEOM_BOUNDING_BOX_MOUSE_DOWN);
    if(show_geom_boundingbox!=SHOW_BOUNDING_BOX_MOUSE_DOWN)glutAddMenuEntry(_("bounding box(mouse down)"),  GEOM_BOUNDING_BOX_MOUSE_DOWN);
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

/* --------------------------------interior geometry menu -------------------------- */

  CREATEMENU(immersedinteriormenu,ImmersedMenu);
  glutAddMenuEntry(_("How"),GEOMETRY_DUMMY);
  if(have_volumes==1){
    if(show_volumes_solid==1)glutAddMenuEntry(_("  *Solid"),GEOMETRY_INTERIOR_SOLID);
    if(show_volumes_solid==0)glutAddMenuEntry(_("  Solid"),GEOMETRY_INTERIOR_SOLID);
    if(show_volumes_outline==1)glutAddMenuEntry(_("  *Outline"),GEOMETRY_INTERIOR_OUTLINE);
    if(show_volumes_outline==0)glutAddMenuEntry(_("  Outline"),GEOMETRY_INTERIOR_OUTLINE);
    if(show_volumes_outline == 0 && show_volumes_solid == 0){
      glutAddMenuEntry(_("  *Hide"),GEOMETRY_TETRA_HIDE);
    }
    else{
      glutAddMenuEntry(_("  Hide"),GEOMETRY_TETRA_HIDE);
    }
  }
  glutAddMenuEntry(_("Where"), GEOMETRY_DUMMY);
  if(show_volumes_interior == 1){
    glutAddMenuEntry(_("   *Interior"), GEOMETRY_VOLUMES_INTERIOR);
  }
  else {
    glutAddMenuEntry(_("   Interior"), GEOMETRY_VOLUMES_INTERIOR);
  }
  if(show_volumes_exterior == 1){
    glutAddMenuEntry(_("   *Exterior"), GEOMETRY_VOLUMES_EXTERIOR);
  }
  else {
    glutAddMenuEntry(_("   Exterior"), GEOMETRY_VOLUMES_EXTERIOR);
  }

  /* --------------------------------surface geometry menu -------------------------- */

  if(have_volumes==1){
    CREATEMENU(immersedmenu,ImmersedMenu);
    GLUTADDSUBMENU(_("Faces"),  immersedsurfacemenu);
    GLUTADDSUBMENU(_("Volumes"),immersedinteriormenu);
    if(show_faces_shaded == 0 && show_faces_outline == 0 && show_volumes_solid == 0){
      glutAddMenuEntry(_("*Hide all"), GEOMETRY_HIDEALL);
    }
    else{
      glutAddMenuEntry(_("Hide all"), GEOMETRY_HIDEALL);
    }
  }

/* --------------------------------blockage menu -------------------------- */

  CREATEMENU(blockagemenu,BlockageMenu);
  glutAddMenuEntry(_("View Method:"),MENU_DUMMY);
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
  if(ncadgeom>0){
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

    for(i=0;i<npropinfo;i++){
      propdata *propi;

      propi = propinfo + i;
      if(propi->inblockage==1)nblockprop++;
    }
    if(nblockprop>0){
      char propmenulabel[255];

      glutAddMenuEntry("-",MENU_DUMMY);
      glutAddMenuEntry(_("Show/Hide blockage types:"),MENU_DUMMY);
      for(i=0;i<npropinfo;i++){
        propdata *propi;

        propi = propinfo + i;
        if(propi->inblockage==1){
          strcpy(propmenulabel,"    ");
          if(propi->blockvis==1)strcat(propmenulabel,"*");
          strcat(propmenulabel,propi->label);
          glutAddMenuEntry(propmenulabel,-i-1);
        }
      }
    }
  }
  glutAddMenuEntry("-",MENU_DUMMY);
  glutAddMenuEntry(_("Locations:"),MENU_DUMMY);
  if(blocklocation==BLOCKlocation_grid){
    glutAddMenuEntry(_("   *Actual"),BLOCKlocation_grid);
  }
  else{
    glutAddMenuEntry(_("   Actual"),BLOCKlocation_grid);
  }
  if(blocklocation==BLOCKlocation_exact){
    glutAddMenuEntry(_("   *Requested"),BLOCKlocation_exact);
  }
  else{
    glutAddMenuEntry(_("   Requested"),BLOCKlocation_exact);
  }
  if(ncadgeom>0){
    if(blocklocation==BLOCKlocation_cad){
      glutAddMenuEntry(_("   *Cad"),BLOCKlocation_cad);
    }
    else{
      glutAddMenuEntry(_("   Cad"),BLOCKlocation_cad);
    }
    {
      cadgeomdata *cd;
      cadlookdata *cdi;
      int showtexturemenu;

      showtexturemenu=0;
      for(i=0;i<ncadgeom;i++){
        int j;

        cd = cadgeominfo + i;
        for(j=0;j<cd->ncadlookinfo;j++){
          cdi = cd->cadlookinfo+j;
          if(cdi->textureinfo.loaded==1){
            showtexturemenu=1;
            break;
          }
        }
        if(showtexturemenu==1)break;
      }
      if(showtexturemenu==1){
        if(visCadTextures==1){
          glutAddMenuEntry(_(" *Show CAD textures"),BLOCKtexture_cad);
        }
        else{
          glutAddMenuEntry(_(" Show CAD textures"),BLOCKtexture_cad);
        }
      }
    }
  }


/* --------------------------------level menu -------------------------- */

  if(nplot3dinfo>0){
    CREATEMENU(levelmenu,LevelMenu);
    for(i=1;i<nrgb-1;i++){
      if(colorlabeliso!=NULL){
        char *colorlabel;
        char levellabel2[256];

        colorlabel=&colorlabeliso[plotn-1][nrgb-2-i][0];
        strcpy(levellabel2,"");
        if(plotiso[plotn-1]==nrgb-2-i&&visiso==1){
          strcat(levellabel2,"*");
        }
        strcat(levellabel2,colorlabel);
        glutAddMenuEntry(levellabel2,nrgb-2-i);
      }
      else{
        char chari[4];

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

  if(nplot3dinfo>0){
    int n;

    CREATEMENU(staticvariablemenu,StaticVariableMenu);
    for(n=0;n<numplot3dvars;n++){
      char *p3label;

      p3label = plot3dinfo[0].label[n].shortlabel;
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

  if(nplot3dinfo>0){
    int n;

    CREATEMENU(isovariablemenu,IsoVariableMenu);
    for(n=0;n<numplot3dvars;n++){
      char *p3label;

      p3label = plot3dinfo[0].label[n].shortlabel;
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
  if(nplot3dinfo>0){
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

  if(nplot3dinfo>0){
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
    for(i=0;i<ntextureinfo;i++){
      texturedata *texti;
      char menulabel[1024];

      texti = textureinfo + i;
      if(texti->loaded==0||texti->used==0)continue;
      if(texti>=terrain_textures&&texti<terrain_textures+nterrain_textures)continue;
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
  if(nplot3dinfo>0){
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

    CREATEMENU(plot3dshowsinglemeshmenu,Plot3DShowMenu);
    if(nplot3dloaded>0){
      int ii;

      for(ii=0;ii<nplot3dinfo;ii++){
        plot3ddata *plot3di;
        char menulabel[1024];

        i=plot3dorderindex[ii];
        plot3di = plot3dinfo + i;
        if(plot3di->loaded==0)continue;
        if(plotstate==STATIC_PLOTS&&plot3di->display==1){
          STRCPY(menulabel,"*");
          STRCAT(menulabel,plot3di->menulabel);
        }
        else{
          STRCPY(menulabel,plot3di->menulabel);
        }
        glutAddMenuEntry(menulabel,1000+i);
      }
    }

    CREATEMENU(plot3dshowmenu,Plot3DShowMenu);
    if(nplot3dloaded>0){
      int ii;
      plot3ddata *plot3di;
      char menulabel[1024];

      for(ii = 0;ii<nplot3dinfo;ii++){
        i = plot3dorderindex[ii];
        plot3di = plot3dinfo+i;
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
    if(nplot3dloaded>1)GLUTADDSUBMENU(_("Mesh"),plot3dshowsinglemeshmenu);
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
    if(ndummyvents>0){
      if(visDummyVents == 1)glutAddMenuEntry(_("*Exterior"), MENU_VENT_EXTERIOR);
      if(visDummyVents == 0)glutAddMenuEntry(_("Exterior"), MENU_VENT_EXTERIOR);
    }
    if(ncvents>0){
      if(visCircularVents!=VENT_HIDE)GLUTADDSUBMENU(_("*Circular"),circularventmenu);
      if(visCircularVents==VENT_HIDE)GLUTADDSUBMENU(_("Circular"),circularventmenu);
    }
    if(GetNTotalVents()>nopenvents+ndummyvents){
      if(visOtherVents == 1)glutAddMenuEntry(_("*Other"), MENU_VENT_OTHER);
      if(visOtherVents == 0)glutAddMenuEntry(_("Other"), MENU_VENT_OTHER);
    }
    if(visOpenVents==1&&visDummyVents==1&&visOtherVents==1){
      glutAddMenuEntry(_("*Show all"),SHOW_ALL_VENTS);
    }
    else{
      glutAddMenuEntry(_("Show all"),SHOW_ALL_VENTS);
    }
    if(visOpenVents==0&&visDummyVents==0&&visOtherVents==0){
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
    if(nvent_transparent>0){
      if(show_transparent_vents == 1)glutAddMenuEntry(_("*Transparent"), MENU_VENT_TRANSPARENT);
      if(show_transparent_vents == 0)glutAddMenuEntry(_("Transparent"), MENU_VENT_TRANSPARENT);
    }
  }

/* --------------------------------terrain_obst_showmenu -------------------------- */

  CREATEMENU(terrain_obst_showmenu, GeometryMenu);
  if(terrain_showonly_top==1)glutAddMenuEntry(_("*Show only top surface"), 17 + TERRAIN_TOP);
  if(terrain_showonly_top==0)glutAddMenuEntry(_("Show only top surface"),  17 + TERRAIN_TOP);
  if(visTerrainType==TERRAIN_3D)glutAddMenuEntry(_("*3D surface"),17+TERRAIN_3D);
  if(visTerrainType!=TERRAIN_3D)glutAddMenuEntry(_("3D surface"),17+TERRAIN_3D);
  if(terrain_textures!=NULL){ // &&terrain_texture->loaded==1
    if(visTerrainType==TERRAIN_3D_MAP)glutAddMenuEntry(_("*Image"),17+TERRAIN_3D_MAP);
    if(visTerrainType!=TERRAIN_3D_MAP)glutAddMenuEntry(_("Image"),17+TERRAIN_3D_MAP);
  }
  if(visTerrainType==TERRAIN_HIDDEN)glutAddMenuEntry(_("*Hidden"),17+TERRAIN_HIDDEN);
  if(visTerrainType!=TERRAIN_HIDDEN)glutAddMenuEntry(_("Hidden"),17+TERRAIN_HIDDEN);
  if(terrain_skip==1)glutAddMenuEntry(_("*skip"), 17+TERRAIN_SKIP);
  if(terrain_skip==0)glutAddMenuEntry(_("skip"), 17+TERRAIN_SKIP);
  if(terrain_debug==1)glutAddMenuEntry(_("*debug"), 17+TERRAIN_DEBUG);
  if(terrain_debug==0)glutAddMenuEntry(_("debug"), 17+TERRAIN_DEBUG);


  if(nobject_defs>0){
    int multiprop;

    multiprop=0;
    for(i=0;i<npropinfo;i++){
      propdata *propi;

      propi = propinfo + i;
      if(propi->nsmokeview_ids>1)multiprop=1;
    }
    if(multiprop==1){
      for(i=0;i<npropinfo;i++){
        propdata *propi;

        propi = propinfo + i;
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
            glutAddMenuEntry(menulabel,jj*npropinfo+i);
          }
        }
      }
      CREATEMENU(propmenu,PropMenu);
      for(i=0;i<npropinfo;i++){
        propdata *propi;

        propi = propinfo + i;
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

  if(nobject_defs>0||hrrinfo!=NULL){
    CREATEMENU(showobjectsplotmenu,ShowObjectsMenu);
    if(ndevicetypes>0){
      GLUTADDSUBMENU(_("quantity"),devicetypemenu);
    }
    if(nobject_defs>0){
      if(showdevice_plot==DEVICE_PLOT_SHOW_ALL)glutAddMenuEntry(      "*All devices",           OBJECT_PLOT_SHOW_ALL);
      if(showdevice_plot!=DEVICE_PLOT_SHOW_ALL)glutAddMenuEntry(      "All devices",            OBJECT_PLOT_SHOW_ALL);
      if(showdevice_plot==DEVICE_PLOT_SHOW_SELECTED)glutAddMenuEntry( "*Selected devices",      OBJECT_PLOT_SHOW_SELECTED);
      if(showdevice_plot!=DEVICE_PLOT_SHOW_SELECTED)glutAddMenuEntry( "Selected devices",       OBJECT_PLOT_SHOW_SELECTED);
#ifdef pp_ZTREE
      if(showdevice_plot==DEVICE_PLOT_SHOW_TREE_ALL)glutAddMenuEntry( "*All device trees",      OBJECT_PLOT_SHOW_TREE_ALL);
      if(showdevice_plot!=DEVICE_PLOT_SHOW_TREE_ALL)glutAddMenuEntry( "All device trees",       OBJECT_PLOT_SHOW_TREE_ALL);
#endif
    }
    if(hrrinfo!=NULL){
      if(show_hrrpuv_plot==1)glutAddMenuEntry("*HRRPUV", PLOT_HRRPUV);
      if(show_hrrpuv_plot==0)glutAddMenuEntry("HRRPUV", PLOT_HRRPUV);
    }

    if(showdevice_val==1)glutAddMenuEntry(_("*Show values"), OBJECT_VALUES);
    if(showdevice_val==0)glutAddMenuEntry(_("Show values"),  OBJECT_VALUES);
    glutAddMenuEntry(_("Settings..."), MENU_DEVICE_SETTINGS);
  }
  if(nobject_defs>0){
    CREATEMENU(showobjectsmenu,ShowObjectsMenu);
    for(i=0;i<nobject_defs;i++){
      sv_object *obj_typei;

      obj_typei = object_defs[i];
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
    if(have_missing_objects == 1&&isZoneFireModel==0){
      if(show_missing_objects==1)glutAddMenuEntry(_("*undefined"),OBJECT_MISSING);
      if(show_missing_objects == 0)glutAddMenuEntry(_("undefined"),OBJECT_MISSING);
    }
    if(ndeviceinfo>0){
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
    if(have_object_box==1){
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
    if(have_beam == 1){
      if(showbeam_as_line == 1){
        glutAddMenuEntry(_("*Show beam as line"), OBJECT_SHOWBEAM);
      }
      else{
        glutAddMenuEntry(_("Show beam as line"), OBJECT_SHOWBEAM);
      }
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    GLUTADDSUBMENU(_("Segments"),spheresegmentmenu);
    glutAddMenuEntry(_("Settings..."), MENU_DEVICE_SETTINGS);

  }

  /* --------------------------------geometry menu -------------------------- */

  CREATEMENU(geometrymenu,GeometryMenu);
  if(ntotal_blockages>0)GLUTADDSUBMENU(_("Obstacles"),blockagemenu);
  if(ngeominfo>0){
    GLUTADDSUBMENU(_("Immersed"), immersedmenu);
  }
  if(GetNTotalVents()>0)GLUTADDSUBMENU(_("Surfaces"), ventmenu);
  if(nrooms > 0){
    if(visCompartments == 1){
      glutAddMenuEntry(_("*Compartments"), GEOM_Compartments);
    }
    else{
      glutAddMenuEntry(_("Compartments"), GEOM_Compartments);
    }
  }
  if(nzvents > 0){
    if(visVents == 1){
      glutAddMenuEntry(_("*Vents"), GEOM_Vents);
    }
    else{
      glutAddMenuEntry(_("Vents"), GEOM_Vents);
    }
  }
  GLUTADDSUBMENU(_("Grid"),gridslicemenu);
  if(isZoneFireModel==0){
    if(visFrame==1)glutAddMenuEntry(_("*Outline"), GEOM_Outline);
    if(visFrame==0)glutAddMenuEntry(_("Outline"), GEOM_Outline);
  }
  else{
    visFrame=0;
  }
  glutAddMenuEntry(_("Show all"), GEOM_ShowAll);
  glutAddMenuEntry(_("Hide all"), GEOM_HideAll);

  CREATEMENU(titlemenu, TitleMenu);
  if(vis_title_smv_version == 1)glutAddMenuEntry(_("*Smokeview version, build data"), MENU_TITLE_title_smv_version);
  if(vis_title_smv_version == 0)glutAddMenuEntry(_("Smokeview version, build date"), MENU_TITLE_title_smv_version);
  if(vis_title_gversion== 1)glutAddMenuEntry(_("*FDS, Smokeview version"), MENU_TITLE_gversion);
  if(vis_title_gversion== 0)glutAddMenuEntry(_("FDS, Smokeview version"), MENU_TITLE_gversion);
  if(fds_title!=NULL){
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
  if(have_northangle==1){
    if(vis_northangle==1)glutAddMenuEntry(_("*North"), MENU_LABEL_northangle);
    if(vis_northangle==0)glutAddMenuEntry(_("North"), MENU_LABEL_northangle);
  }
  if(ntickinfo>0){
    if(visFDSticks == 0)glutAddMenuEntry(_("FDS generated ticks"), MENU_LABEL_fdsticks);
    if(visFDSticks == 1)glutAddMenuEntry(_("*FDS generated ticks"), MENU_LABEL_fdsticks);
  }

  if(visFramerate == 1)glutAddMenuEntry(_("*Frame rate"), MENU_LABEL_framerate);
  if(visFramerate == 0)glutAddMenuEntry(_("Frame rate"), MENU_LABEL_framerate);
  if(visgridloc == 1)glutAddMenuEntry(_("*Grid locations"), MENU_LABEL_grid);
  if(visgridloc == 0)glutAddMenuEntry(_("Grid locations"), MENU_LABEL_grid);

  if(show_hrrcutoff_active == 1&&show_tempcutoff_active==1){
    if(show_firecutoff == 1)glutAddMenuEntry(_("*Fire cutoff"), MENU_LABEL_firecutoff);
    if(show_firecutoff == 0)glutAddMenuEntry(_("Fire cutoff"), MENU_LABEL_firecutoff);
  }
  else if(show_hrrcutoff_active == 1&&show_tempcutoff_active==0){
    if(show_firecutoff == 1)glutAddMenuEntry(_("*HRRPUV cutoff"), MENU_LABEL_firecutoff);
    if(show_firecutoff == 0)glutAddMenuEntry(_("HRRPUV cutoff"), MENU_LABEL_firecutoff);
  }
  else if(show_hrrcutoff_active == 0&&show_tempcutoff_active==1){
    if(show_firecutoff == 1)glutAddMenuEntry(_("*Temperature cutoff"), MENU_LABEL_firecutoff);
    if(show_firecutoff == 0)glutAddMenuEntry(_("Temperature cutoff"), MENU_LABEL_firecutoff);
  }

  if(hrrinfo != NULL){
    if(visHRRlabel == 1)glutAddMenuEntry(_("*HRR"), MENU_LABEL_hrr);
    if(visHRRlabel == 0)glutAddMenuEntry(_("HRR"), MENU_LABEL_hrr);
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
  if(LabelGetNUserLabels() > 0){
    if(visLabels == 1)glutAddMenuEntry(_("*Text labels"), MENU_LABEL_textlabels);
    if(visLabels == 0)glutAddMenuEntry(_("Text labels"), MENU_LABEL_textlabels);
  }

  if(visUSERticks == 1)glutAddMenuEntry(_("*User settable ticks"), MENU_LABEL_userticks);
  if(visUSERticks == 0)glutAddMenuEntry(_("User settable ticks"), MENU_LABEL_userticks);

  glutAddMenuEntry("-", MENU_DUMMY);
  glutAddMenuEntry(_("Show all"), MENU_LABEL_ShowAll);
  glutAddMenuEntry(_("Hide all"), MENU_LABEL_HideAll);
  glutAddMenuEntry(_("Settings..."), MENU_LABEL_SETTINGS);

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
    ASSERT(FFALSE);
    break;
  }
  glutAddMenuEntry(_("Direction vectors:"), MENU_DUMMY);
  if(showgravity_vector==1)glutAddMenuEntry(_("  *show gravity, axis vectors"), MENU_MOTION_SHOW_VECTORS);
  if(showgravity_vector==0)glutAddMenuEntry(_("  show gravity, axis vectors"), MENU_MOTION_SHOW_VECTORS);
  glutAddMenuEntry(_("  gravity vector down"), MENU_MOTION_GRAVITY_VECTOR);
  glutAddMenuEntry(_("  z vector up"), MENU_MOTION_Z_VECTOR);
  glutAddMenuEntry(_("Settings..."), MENU_MOTION_SETTINGS);

/* --------------------------------zone show menu -------------------------- */

  if(nzoneinfo>0&&(ReadZoneFile==1||nzvents>0)){
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
        glutAddMenuEntry(_("      Horizontal"), MENU_ZONE_HORIZONTAL);
        glutAddMenuEntry(_("      Vertical"), MENU_ZONE_VERTICAL);
      }
      else{
        if(visHZone==1)glutAddMenuEntry(_("      *Horizontal"), MENU_ZONE_HORIZONTAL);
        if(visHZone==0)glutAddMenuEntry(_("      Horizontal"), MENU_ZONE_HORIZONTAL);
        if(visVZone==1)glutAddMenuEntry(_("      *Vertical"), MENU_ZONE_VERTICAL);
        if(visVZone==0)glutAddMenuEntry(_("      Vertical"), MENU_ZONE_VERTICAL);
      }
    }
    else{
      glutAddMenuEntry(_("      Horizontal"), MENU_ZONE_HORIZONTAL);
      glutAddMenuEntry(_("      Vertical"), MENU_ZONE_VERTICAL);
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
    if(nzvents>0){
      if(visVentFlow==1){
        glutAddMenuEntry(_("*Vent flow"), MENU_ZONE_VENTS);
      }
      else{
        glutAddMenuEntry(_("Vent flow"), MENU_ZONE_VENTS);
      }
      if(nzhvents>0){
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
      if(nzvvents>0){
        if(visVentVFlow==1){
          glutAddMenuEntry(_("   *Vertical"), MENU_ZONE_VVENTS);
        }
        else{
          glutAddMenuEntry(_("   Vertical"), MENU_ZONE_VVENTS);
        }
      }
      if(nzmvents>0){
        if(visVentMFlow==1){
          glutAddMenuEntry(_("   *Mechanical"), MENU_ZONE_MVENTS);
        }
        else{
          glutAddMenuEntry(_("   Mechanical"), MENU_ZONE_MVENTS);
        }
      }
    }
    if(nfires>0){
      if(viszonefire==1){
        glutAddMenuEntry(_("*Fires"), MENU_ZONE_FIRES);
      }
      else{
        glutAddMenuEntry(_("Fires"), MENU_ZONE_FIRES);
      }
    }
  }

  /* --------------------------------particle class show menu -------------------------- */

  if(npartclassinfo>0){
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

    if(npart5prop*npartclassinfo>0){
      NewMemory((void **)&particlepropshowsubmenu,npart5prop*npartclassinfo*sizeof(int));
    }

      ntypes=0;
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        int j;

        propi = part5propinfo + i;
        if(propi->display==0)continue;
        for(j=0;j<npartclassinfo;j++){
          partclassdata *partclassj;
          char menulabel[1024];

          if(propi->class_present[j]==0)continue;
          partclassj = partclassinfo + j;
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
            if(partclassj->kind!=HUMANS){
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

                  propvalue = iii*npropinfo + (propclass-propinfo);
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
        if(propi->particle_property==0)continue;
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
        for(j=0;j<npartclassinfo;j++){
          partclassdata *partclassj;
          char menulabel[1024];

          if(propi->class_present[j]==0)continue;
          partclassj = partclassinfo + j;
          if(partclassj->kind==HUMANS)continue;
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

    CREATEMENU(humanpropshowmenu,ParticlePropShowMenu);
    if(npart5prop>=0){
      glutAddMenuEntry(_("Color with:"),MENU_PROP_DUMMY);
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        char menulabel[1024];

        propi = part5propinfo + i;
        if(propi->human_property==0)continue;
        if(propi->display==1){
          strcpy(menulabel,"  *");
        }
        else{
          strcpy(menulabel,"  ");
        }
        strcat(menulabel,propi->label->longlabel);
        glutAddMenuEntry(menulabel,i);
      }

      if(part5show==0)glutAddMenuEntry(_("  *Hide"),MENU_PROP_HIDEAVATAR);
      if(part5show==1)glutAddMenuEntry(_("  Hide"), MENU_PROP_HIDEAVATAR);
      glutAddMenuEntry("-",MENU_PROP_DUMMY);
      glutAddMenuEntry(_("Draw"),MENU_PROP_DUMMY);
      ntypes=0;
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        int j;

        propi = part5propinfo + i;
        if(propi->display==0)continue;
        for(j=0;j<npartclassinfo;j++){
          partclassdata *partclassj;
          char menulabel[1024];

          if(propi->class_present[j]==0)continue;
          partclassj = partclassinfo + j;
          if(partclassj->kind!=HUMANS)continue;
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
        //break;
      }
      if(ntypes>1){
        glutAddMenuEntry(_("  Show all"),MENU_PROP_SHOWALL);
        glutAddMenuEntry(_("  Hide all"),MENU_PROP_HIDEALL);
      }
      glutAddMenuEntry("-",MENU_PROP_DUMMY);
      if(streak5show==1){
        GLUTADDSUBMENU(_("  *Streaks"),particlestreakshowmenu);
      }
      else{
        GLUTADDSUBMENU(_("  Streaks"),particlestreakshowmenu);
      }
    }
  }

/* --------------------------------particle show menu -------------------------- */

  if(npartinfo>0&&nevac!=npartinfo){
    int ii;
    int showall;

    CREATEMENU(particleshowmenu,ParticleShowMenu);
    for(ii=0;ii<npartinfo;ii++){
      partdata *parti;
      char menulabel[1024];

      i = partorderindex[ii];
      parti = partinfo + i;
      if(parti->loaded==0)continue;
      if(parti->evac==1)continue;
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

/* --------------------------------Evac show menu -------------------------- */

  if(nevac>0){
    int ii;

    CREATEMENU(evacshowmenu,EvacShowMenu);
    for(ii=0;ii<npartinfo;ii++){
      partdata *parti;
      char menulabel[1024];

      i = partorderindex[ii];
      parti = partinfo + i;
      if(parti->loaded==0)continue;
      if(parti->evac==0)continue;
      STRCPY(menulabel,"");
      if(parti->display==1)STRCAT(menulabel,"*");
      STRCAT(menulabel,parti->menulabel);
      glutAddMenuEntry(menulabel,-1-i);
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    glutAddMenuEntry(_("Show all"), MENU_PARTSHOW_SHOWALL);
    if(plotstate==DYNAMIC_PLOTS){
      glutAddMenuEntry(_("Hide all"), MENU_PARTSHOW_HIDEALL);
    }
  }

/* -------------------------------- colorbarmenu -------------------------- */

  if(nsmoke3dloaded>0||nvolrenderinfo>0){
    colorbardata *cbi;
    char ccolorbarmenu[256];

    CREATEMENU(smokecolorbarmenu,SmokeColorbarMenu);

    for(i=0;i<ncolorbars;i++){
      cbi = colorbarinfo + i;

      strcpy(ccolorbarmenu,"  ");
      if(fire_colorbar_index==i){
        strcat(ccolorbarmenu,"*");
        strcat(ccolorbarmenu,cbi->label);
      }
      else{
        strcat(ccolorbarmenu,cbi->label);
      }
      glutAddMenuEntry(ccolorbarmenu,i);
    }
  }


  /* --------------------------------smoke3d showmenu -------------------------- */

  if(nsmoke3dloaded>0){
    {
      if(nsmoke3dloaded>0){
        CREATEMENU(smoke3dshowsinglemenu, Smoke3DShowMenu);
        for(i=0;i<nsmoke3dinfo;i++){
          smoke3ddata *smoke3di;
          char menulabel[1024];

          smoke3di = smoke3dinfo + i;
          if(smoke3di->loaded==0)continue;
          strcpy(menulabel,"");
          if(smoke3di->display==1)strcpy(menulabel,"*");
          strcat(menulabel,smoke3di->menulabel);
          glutAddMenuEntry(menulabel,i);
        }
        CREATEMENU(smoke3dshowmenu, Smoke3DShowMenu);
        if(show_3dsmokefiles==1)glutAddMenuEntry(_("*Show"), TOGGLE_SMOKE3D);
        if(show_3dsmokefiles==0)glutAddMenuEntry(_("Show"), TOGGLE_SMOKE3D);
        GLUTADDSUBMENU(_("Smoke colorbar"),smokecolorbarmenu);
        GLUTADDSUBMENU(_("Mesh"), smoke3dshowsinglemenu);
      }
    }
  }

/* --------------------------------iso level menu -------------------------- */

  if(loaded_isomesh!=NULL&&nisoinfo>0&&ReadIsoFile==1){
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
          STRCAT(levellabel,isoinfo[loaded_isomesh->isofilenum].surface_label.unit);
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

    if(nisoinfo>0&&ReadIsoFile==1){
      meshdata *hmesh;
      isodata *iso2;
      int ii;

      CREATEMENU(isoshowsubmenu,IsoShowMenu);
      iso2=NULL;
      for(ii=0;ii<nisoinfo;ii++){
        isodata *isoi;
        char menulabel[1024];

        i = isoorderindex[ii];
        isoi = isoinfo + i;
        if(isoi->loaded==0)continue;
        if(iso2==NULL&&isoi->type==iisotype)iso2=isoi;
        if(plotstate==DYNAMIC_PLOTS&&isoi->display==1&&isoi->type==iisotype){
          iso2=isoi;
          STRCPY(menulabel,"*");
          STRCAT(menulabel,isoi->menulabel);
        }
        else{
          STRCPY(menulabel,isoi->menulabel);
        }
        glutAddMenuEntry(menulabel,1000+i);
      }
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
      hmesh=meshinfo+highlight_mesh;
      if(hmesh->isofilenum!=-1){
        char levellabel[1024];

        STRCPY(levellabel,isoinfo[hmesh->isofilenum].surface_label.shortlabel);
        STRCAT(levellabel," ");
        STRCAT(levellabel,_("Levels"));
        GLUTADDSUBMENU(levellabel,isolevelmenu);
      }
      if(niso_compressed==0){
        if(smooth_iso_normal == 1)glutAddMenuEntry(_("*Smooth"), MENU_ISOSHOW_SMOOTH);
        if(smooth_iso_normal == 0)glutAddMenuEntry(_("Smooth"), MENU_ISOSHOW_SMOOTH);
      }
      if(show_iso_normal == 1)glutAddMenuEntry(_("*Show normals"), MENU_ISOSHOW_NORMALS);
      if(show_iso_normal == 0)glutAddMenuEntry(_("Show normals"), MENU_ISOSHOW_NORMALS);
      GLUTADDSUBMENU(_("Mesh"), isoshowsubmenu);
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

  CREATEMENU(colorbarsmenu,ColorbarMenu);
  {
    colorbardata *cbi;
    char ccolorbarmenu[256];

    for(i=0;i<ncolorbars;i++){
      cbi = colorbarinfo + i;

      strcpy(ccolorbarmenu,"  ");
      if(colorbartype==i){
        strcat(ccolorbarmenu,"*");
        strcat(ccolorbarmenu,cbi->label);
      }
      else{
        strcat(ccolorbarmenu,cbi->label);
      }
      glutAddMenuEntry(ccolorbarmenu,i);
    }
  }

/* -------------------------------- colorbarmenu -------------------------- */

  CREATEMENU(colorbarmenu,ColorbarMenu);
  GLUTADDSUBMENU(_("Colorbar"),colorbarsmenu);
  GLUTADDSUBMENU(_("Colorbar type"), colorbarshademenu);
  GLUTADDSUBMENU(_("Colorbar digits"), colorbardigitmenu);
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
  if(use_lighting==1)glutAddMenuEntry(_("*Lighting"), USE_LIGHTING);
  if(use_lighting==0)glutAddMenuEntry(_("Lighting"), USE_LIGHTING);
  glutAddMenuEntry(_("Settings..."), MENU_COLORBAR_SETTINGS);

/* --------------------------------showVslice menu -------------------------- */
  if(nvsliceloaded==0){
    vd_shown=NULL;
  }
  if(nvsliceinfo>0&&nvsliceloaded>0){
    CREATEMENU(showsingleslicemenu,ShowVSliceMenu);
    for(i=0;i<nvsliceinfo;i++){
      vslicedata *vd;
      slicedata *sd;
      char menulabel[1024];

      vd = vsliceinfo + i;
      if(vd->loaded==0)continue;
      sd = sliceinfo + vd->ival;
      STRCPY(menulabel,"");
      if(plotstate==DYNAMIC_PLOTS&&sd->slicefile_labelindex==slicefile_labelindex&&vd->display==1){
        vd_shown=vd;
        STRCAT(menulabel,"*");
      }
      STRCAT(menulabel,sd->menulabel2);
      if(sd->slicelabel!=NULL){
        STRCAT(menulabel," - ");
        STRCAT(menulabel,sd->slicelabel);
      }
      glutAddMenuEntry(menulabel,i);
    }
    CREATEMENU(showvslicemenu,ShowVSliceMenu);
    if(vd_shown!=NULL&&nvsliceloaded!=0){
      char menulabel[1024];
      slicedata *slice_shown;

      STRCPY(menulabel, "");
      if(showall_slices==1)STRCAT(menulabel, "*");
      slice_shown = sliceinfo+vd_shown->ival;
      STRCAT(menulabel, slice_shown->label.longlabel);
      STRCAT(menulabel, ", ");
      STRCAT(menulabel, slice_shown->slicedir);
      if(slice_shown->slicelabel!=NULL){
        STRCAT(menulabel, " - ");
        STRCAT(menulabel, slice_shown->slicelabel);
      }
      glutAddMenuEntry(menulabel,SHOW_ALL);
    }
    glutAddMenuEntry(_("  Show in:"), MENU_DUMMY);
    if(show_slice_in_obst==ONLY_IN_GAS){
      glutAddMenuEntry(_("    *gas"),  MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(show_slice_in_obst==GAS_AND_SOLID){
      glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("    *gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(show_slice_in_obst==ONLY_IN_SOLID){
      glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
      glutAddMenuEntry(_("    *solid"), MENU_SHOWSLICE_IN_SOLID);
      glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
    }
    if(show_node_slices_and_vectors == 1)glutAddMenuEntry(_("*Show node centered slices and vectors"), MENU_SHOWSLICE_NODESLICEANDVECTORS);
    if(show_node_slices_and_vectors == 0)glutAddMenuEntry(_("Show node centered slices and vectors"), MENU_SHOWSLICE_NODESLICEANDVECTORS);
    if(show_cell_slices_and_vectors == 1)glutAddMenuEntry(_("*Show cell/face centered slices and vectors"), MENU_SHOWSLICE_CELLSLICEANDVECTORS);
    if(show_cell_slices_and_vectors == 0)glutAddMenuEntry(_("Show cell/face centered slices and vectors"), MENU_SHOWSLICE_CELLSLICEANDVECTORS);
    if(offset_slice == 1)glutAddMenuEntry(_("*Offset vector slice"), MENU_SHOWSLICE_OFFSET);
    if(offset_slice == 0)glutAddMenuEntry(_("Offset vector slice"), MENU_SHOWSLICE_OFFSET);
    GLUTADDSUBMENU(_("Mesh"), showsingleslicemenu);
  }

/* --------------------------------showslice menu -------------------------- */
  if(nsliceloaded==0){
    sd_shown=NULL;
  }
  if(nsliceloaded>0||patchgeom_slice_showhide==1){
    int ii;

    CREATEMENU(showhideslicemenu,ShowHideSliceMenu);
    // loaded slice entries
    for(ii=0;ii<nslice_loaded;ii++){
      slicedata *sd;
      char menulabel[1024];
      int doit;

      i = slice_loaded_list[ii];
      sd = sliceinfo + i;
      if(sd_shown==NULL&&sd->slicefile_labelindex==slicefile_labelindex){
        sd_shown = sd;
      }
      STRCPY(menulabel,"");
      if(plotstate==DYNAMIC_PLOTS&&sd->display==1&&sd->slicefile_labelindex==slicefile_labelindex){
        sd_shown=sd;
        STRCAT(menulabel,"*");
      }
      STRCAT(menulabel,sd->menulabel2);
      if(sd->slicelabel!=NULL){
        STRCAT(menulabel," - ");
        STRCAT(menulabel,sd->slicelabel);
      }
      doit=1;
      if(sd->slice_filetype==SLICE_TERRAIN){
        meshdata *meshslice;
        terraindata *terri;

        meshslice = meshinfo + sd->blocknumber;
        terri = meshslice->terrain;
        if(terri==NULL||terri->nvalues==0)doit = 0;
      }
      if(doit==1)glutAddMenuEntry(menulabel,i);
    }
    // loaded geometry slice entries
    for(ii = 0;ii<npatchinfo;ii++){
      patchdata *patchi;

      i = patchorderindex[ii];
      patchi = patchinfo+i;
      if(patchi->loaded==1&&patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
        char mlabel[250];

        strcpy(mlabel, "");
        if(patchi->display == 1)strcat(mlabel, "*");
        strcat(mlabel, patchi->label.longlabel);
        glutAddMenuEntry(mlabel,-20-i);
      }
    }
    if(nsliceinfo>0&&nmultisliceinfo+nfedinfo<nsliceinfo){
    }
    else{
      glutAddMenuEntry("-", MENU_DUMMY);
      if(HaveTerrainSlice()==1){
        if(planar_terrain_slice==1)glutAddMenuEntry(_("*Planar terrain slice"), MENU_SHOWSLICE_TERRAIN);
        if(planar_terrain_slice==0)glutAddMenuEntry(_("Planar terrain slice"), MENU_SHOWSLICE_TERRAIN);
      }

      if(nsliceloaded>0){
        glutAddMenuEntry(_("Show in:"), MENU_DUMMY);
        if(show_slice_in_obst==ONLY_IN_GAS){
          glutAddMenuEntry(_("  *gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
          glutAddMenuEntry(_("  gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
        }
        if(show_slice_in_obst==GAS_AND_SOLID){
          glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  solid"), MENU_SHOWSLICE_IN_SOLID);
          glutAddMenuEntry(_("  *gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
        }
        if(show_slice_in_obst==ONLY_IN_SOLID){
          glutAddMenuEntry(_("  gas"), MENU_SHOWSLICE_IN_GAS);
          glutAddMenuEntry(_("  *solid"), MENU_SHOWSLICE_IN_SOLID);
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
      if(nfedinfo>0){
        int showfedmenu = 0;

        for(i = nsliceinfo-nfedinfo;i<nsliceinfo;i++){
          slicedata *slicei;

          slicei = sliceinfo+i;
          if(slicei->loaded==1){
            showfedmenu = 1;
            break;
          }
        }
        if(showfedmenu==1){
          if(show_fed_area==1)glutAddMenuEntry(_("*Show FED areas"), MENU_SHOWSLICE_FEDAREA);
          if(show_fed_area==0)glutAddMenuEntry(_("Show FED areas"), MENU_SHOWSLICE_FEDAREA);
        }
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
  if((nsliceinfo>0&&nmultisliceinfo+nfedinfo<nsliceinfo)||patchgeom_slice_showhide==1){
    int ii;
    patchdata *patchim1=NULL;

    CREATEMENU(showmultislicemenu, ShowMultiSliceMenu);
    for(i = 0;i<nmultisliceinfo;i++){
      slicedata *sd;
      char menulabel[1024];
      multislicedata *mslicei;

      mslicei = multisliceinfo+i;
      if(mslicei->loaded==0)continue;
      sd = sliceinfo+mslicei->islices[0];
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
    for(ii = 0;ii<npatchinfo;ii++){
      patchdata *patchi;

      i = patchorderindex[ii];
      patchi = patchinfo+i;
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
      if(show_slice_in_obst==ONLY_IN_GAS){
        glutAddMenuEntry(_("    *gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
      if(show_slice_in_obst==GAS_AND_SOLID){
        glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("    solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("    *gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
      if(show_slice_in_obst==ONLY_IN_SOLID){
        glutAddMenuEntry(_("    gas"), MENU_SHOWSLICE_IN_GAS);
        glutAddMenuEntry(_("    *solid"), MENU_SHOWSLICE_IN_SOLID);
        glutAddMenuEntry(_("    gas and solid"), MENU_SHOWSLICE_IN_GASANDSOLID);
      }
    }
    if(nsliceloaded>0){
      if(offset_slice==1)glutAddMenuEntry(_("*Offset slice"), MENU_SHOWSLICE_OFFSET);
      if(offset_slice==0)glutAddMenuEntry(_("Offset slice"), MENU_SHOWSLICE_OFFSET);
    }
    if(nfedinfo>0){
      int showfedmenu = 0;

      for(i = nsliceinfo-nfedinfo;i<nsliceinfo;i++){
        slicedata *slicei;

        slicei = sliceinfo+i;
        if(slicei->loaded==1){
          showfedmenu = 1;
          break;
        }
      }
      if(showfedmenu==1){
        if(show_fed_area==1)glutAddMenuEntry(_("*Show FED areas"), MENU_SHOWSLICE_FEDAREA);
        if(show_fed_area==0)glutAddMenuEntry(_("Show FED areas"), MENU_SHOWSLICE_FEDAREA);
      }
    }
    if(nslice_loaded>0)GLUTADDSUBMENU(_("Mesh"), showhideslicemenu);
  }

/* -------------------------------- avatar tour menu -------------------------- */

  CREATEMENU(avatarevacmenu,AvatarEvacMenu);
  if(navatar_types>0){
    if(iavatar_evac==-1){
      glutAddMenuEntry(_("*Defined in evac file"),MENU_AVATAR_DEFINED);
    }
    else{
      glutAddMenuEntry(_("Defined in evac file"),MENU_AVATAR_DEFINED);
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    for(i=0;i<navatar_types;i++){
      char menulabel[1024];

      strcpy(menulabel,"");
      if(iavatar_evac==i){
        strcat(menulabel,"*");
      }
      strcat(menulabel,avatar_types[i]->label);
      glutAddMenuEntry(menulabel,i);
    }
  }
  CREATEMENU(avatartourmenu,TourMenu);
  if(navatar_types>0){
    if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
      tourdata *touri;
      char menulabel[1024];

      touri = tourinfo + selectedtour_index;
      strcpy(menulabel,"For ");
      strcat(menulabel,touri->label);
      glutAddMenuEntry(menulabel,MENU_DUMMY);
      glutAddMenuEntry("-",MENU_DUMMY);
    }

    for(i=0;i<navatar_types;i++){
      char menulabel[1024];

      strcpy(menulabel,"");
      if(tourlocus_type==2&&iavatar_types==i){
        strcat(menulabel,"*");
      }
      strcat(menulabel,avatar_types[i]->label);
      glutAddMenuEntry(menulabel,-23-i);
    }
  }

  CREATEMENU(tourcopymenu, TourCopyMenu);
  glutAddMenuEntry("Path through domain", -1);
  for(i = 0; i < ntourinfo; i++){
    tourdata *touri;

    touri = tourinfo + i;
    glutAddMenuEntry(touri->menulabel, i);
  }

    /* --------------------------------tour menu -------------------------- */

  CREATEMENU(tourmenu,TourMenu);

  GLUTADDSUBMENU(_("New"),tourcopymenu);
  if(ntourinfo>0){
    glutAddMenuEntry("-",MENU_DUMMY);
    for(i=0;i<ntourinfo;i++){
      tourdata *touri;
      int glui_avatar_index_local;
      char menulabel[1024];

      touri = tourinfo + i;
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
      if(glui_avatar_index_local>=0&&glui_avatar_index_local<navatar_types){
        sv_object *avatari;

        avatari=avatar_types[glui_avatar_index_local];
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
      strcat(menulabel,tourinfo[selectedtour_index].label);
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

    CREATEMENU(showvolsmokesinglemenu,ShowVolsmoke3DMenu);
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;
      char menulabel[1024];

      meshi = meshinfo + i;
      vr = &(meshi->volrenderinfo);
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0)continue;
      strcpy(menulabel,"");
      if(vr->display==1)strcat(menulabel,"*");
      strcat(menulabel,meshi->label);
      glutAddMenuEntry(menulabel,i);
    }
    CREATEMENU(showvolsmoke3dmenu,ShowVolsmoke3DMenu);
    strcpy(vlabel, "");
    if(show_volsmokefiles==1)strcat(vlabel, "*");
    strcat(vlabel,_("Show"));
    glutAddMenuEntry(vlabel,TOGGLE_VOLSMOKE);
    GLUTADDSUBMENU(_("Smoke colorbar"),smokecolorbarmenu);
    GLUTADDSUBMENU(_("Mesh"), showvolsmokesinglemenu);
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
  if(nterraininfo>0&&ngeominfo==0){
    GLUTADDSUBMENU(_("Terrain"), terrain_obst_showmenu);
  }
  if(GetNumActiveDevices()>0||ncvents>0){
    GLUTADDSUBMENU(_("Devices"), showobjectsmenu);
  }
  if(nobject_defs>0&&ndeviceinfo>0){
    GLUTADDSUBMENU(_("Plot data"),showobjectsplotmenu);
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
  {
    int human_present=0;
    int particle_present=0;

    if(npart5loaded>0){
      int ii;

      showhide_data = 1;
      for(ii = 0; ii<npartinfo; ii++){
        partdata *parti;

        parti = partinfo + ii;
        if(parti->loaded==0)continue;
        if(parti->evac==1)human_present=1;
        if(parti->evac==0)particle_present=1;
      }
      if(particle_present==1){
        GLUTADDSUBMENU(_("Particles"),particlepropshowmenu);
      }
      if(human_present==1){
        GLUTADDSUBMENU(_("Humans"),humanpropshowmenu);
      }
    }
  }

  if(ReadIsoFile==1){
    int niso_loaded=0;

    for(i=0;i<nisoinfo;i++){
      isodata *isoi;

      isoi = isoinfo + i;
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
    if(nmultisliceinfo+nfedinfo<nsliceinfo){
      GLUTADDSUBMENU(_("Slice"),showmultislicemenu);
    }
    else{
      GLUTADDSUBMENU(_("Slice"), showhideslicemenu);
    }
  }

  if(nzoneinfo>0&&(ReadZoneFile==1||nzvents>0)){
    showhide_data = 1;
    GLUTADDSUBMENU(_("Zone"), zoneshowmenu);
  }
  if(nobject_defs>0){
    int num_activedevices=0;

    for(i = 0; i<nobject_defs; i++){
      sv_object *obj_typei;

      obj_typei = object_defs[i];
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
      for(i=0;i<nobject_defs;i++){
        sv_object *obj_typei;

        obj_typei = object_defs[i];
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
  if(ntc_total>0){
    showhide_data = 1;
    if(isZoneFireModel==1){
      if(visSensor==1)glutAddMenuEntry(_("*Targets"), MENU_SHOWHIDE_SENSOR);
      if(visSensor==0)glutAddMenuEntry(_("Targets"), MENU_SHOWHIDE_SENSOR);
      if(hasSensorNorm==1){
        if(visSensorNorm==1)glutAddMenuEntry(_("*Target orientation"), MENU_SHOWHIDE_SENSOR_NORM);
        if(visSensorNorm==0)glutAddMenuEntry(_("Target orientation"), MENU_SHOWHIDE_SENSOR_NORM);
      }
    }
    else{
      if(visSensor==1)glutAddMenuEntry(_("*Thermocouples"), MENU_SHOWHIDE_SENSOR);
      if(visSensor==0)glutAddMenuEntry(_("Thermocouples"), MENU_SHOWHIDE_SENSOR);
      if(hasSensorNorm==1){
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
  if(ntextures_loaded_used>nterrain_textures){
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
    UpdateGluiRender();
  }

  /* --------------------------------filesdialog menu -------------------------- */

  CREATEMENU(filesdialogmenu, DialogMenu);
  glutAddMenuEntry(_("Auto load data files..."), DIALOG_AUTOLOAD);
#ifdef pp_COMPRESS
  if(smokezippath!=NULL&&(npatchinfo>0||nsmoke3dinfo>0||nsliceinfo>0)){
#ifdef pp_DIALOG_SHORTCUTS
    glutAddMenuEntry(_("Compress data files...  ALT z"), DIALOG_SMOKEZIP);
#else
    glutAddMenuEntry(_("Compress data files..."), DIALOG_SMOKEZIP);
#endif
  }
#endif
  glutAddMenuEntry(_("Save/load configuration files..."), DIALOG_CONFIG);
  glutAddMenuEntry(_("Render images..."), DIALOG_RENDER);
  if(have_slurm==1){
    if(have_ffmpeg==1){
      glutAddMenuEntry(_("Make movies(local)..."), DIALOG_MOVIE);
    }
    glutAddMenuEntry(_("Make movies(cluster)..."), DIALOG_MOVIE_BATCH);
  }
  else{
    if(have_ffmpeg==1){
      glutAddMenuEntry(_("Make movies..."), DIALOG_MOVIE);
    }
    }
  glutAddMenuEntry(_("Record/run scripts..."), DIALOG_SCRIPT);

  /* --------------------------------viewdialog menu -------------------------- */

  CREATEMENU(viewdialogmenu, DialogMenu);
#ifdef pp_DIALOG_SHORTCUTS
  glutAddMenuEntry(_("Clip scene...  ALT c"), DIALOG_CLIP);
  if(showtour_dialog==1)glutAddMenuEntry(_("*Create/modify tours...  ALT t"), DIALOG_TOUR_HIDE);
  if(showtour_dialog==0)glutAddMenuEntry(_("Create/modify tours...  ALT t"), DIALOG_TOUR_SHOW);
  glutAddMenuEntry(_("Edit colorbar...  ALT C"), DIALOG_COLORBAR);
  if(isZoneFireModel==0){
    glutAddMenuEntry(_("Examine geometry...  ALT e"), DIALOG_GEOMETRY);
  }
#else
  glutAddMenuEntry(_("Clip scene..."), DIALOG_CLIP);
  if(showtour_dialog==1)glutAddMenuEntry(_("*Create/edit tours..."), DIALOG_TOUR_HIDE);
  if(showtour_dialog==0)glutAddMenuEntry(_("Create/edit tours..."), DIALOG_TOUR_SHOW);
  glutAddMenuEntry(_("Edit colorbar...  "), DIALOG_COLORBAR);
  if(isZoneFireModel==0){
    glutAddMenuEntry(_("Examine geometry...  "), DIALOG_GEOMETRY);
  }
#endif
  if(have_vr==1){
    glutAddMenuEntry(_("Stereo/VR settings..."), DIALOG_STEREO);
  }
  else{
    glutAddMenuEntry(_("Stereo settings..."), DIALOG_STEREO);
  }
  if(trainer_active==1){
    glutAddMenuEntry(_("Trainer..."), DIALOG_TRAINER);
  }

  /* --------------------------------datadialog menu -------------------------- */

  CREATEMENU(datadialogmenu, DialogMenu);
  if(ndeviceinfo>0&&GetNumActiveDevices()>0){
    glutAddMenuEntry(_("Devices/Objects..."), DIALOG_DEVICE);
  }
  glutAddMenuEntry(_("Show/Hide..."), DIALOG_SHOWFILES);
  glutAddMenuEntry(_("Particle tracking..."), DIALOG_SHOOTER);

  /* --------------------------------window menu -------------------------- */

  CREATEMENU(windowdialogmenu, DialogMenu);
  glutAddMenuEntry(_("Fonts..."), DIALOG_FONTS);
  glutAddMenuEntry(_("User ticks..."), DIALOG_TICKS);
  glutAddMenuEntry(_("Labels..."), DIALOG_LABELS);
  glutAddMenuEntry(_("Properties..."), DIALOG_WINDOW);
  glutAddMenuEntry(_("Scaling..."), DIALOG_SCALING);

  /* --------------------------------dialog menu -------------------------- */

  CREATEMENU(dialogmenu,DialogMenu);

  glutAddMenuEntry(_("Display...  ALT D"), DIALOG_DISPLAY);
  glutAddMenuEntry(_("Files/Data/Coloring... ALT b"), DIALOG_BOUNDS);
  glutAddMenuEntry(_("Motion/View/Render...  ALT m"),DIALOG_MOTION);
  glutAddMenuEntry(_("Viewpoints... ALT g"),DIALOG_VIEW);

  glutAddMenuEntry("-",MENU_DUMMY2);

  GLUTADDSUBMENU(_("Data"), datadialogmenu);
  GLUTADDSUBMENU(_("Files"), filesdialogmenu);
  GLUTADDSUBMENU(_("View"), viewdialogmenu);
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
    ASSERT(FFALSE);
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
        if(smokediff==1&&uci->diff_index==j&&uci->units[j].rel_defined==1){
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
#ifdef pp_BETA
    if(show_all_units==1)glutAddMenuEntry(_("*show all units"), MENU_UNITS_SHOWALL);
    if(show_all_units==0)glutAddMenuEntry(_("show all units"), MENU_UNITS_SHOWALL);
#endif
    glutAddMenuEntry(_("Reset"), MENU_UNITS_RESET);
  }

/* --------------------------------option menu -------------------------- */

  CREATEMENU(optionmenu,OptionMenu);
  if(nunitclasses>0)GLUTADDSUBMENU(_("Display Units"),unitsmenu);
  GLUTADDSUBMENU(_("Rotation parameters"),rotatetypemenu);
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
    char compiler_version_label[256];

    sprintf(menulabel,"  Smokeview build: %s",smv_githash);
    glutAddMenuEntry(menulabel,1);
#ifndef pp_COMPVER
#define pp_COMPVER "unknown"
#endif
    strcpy(compiler_version_label, _("    Compiler version:"));
    strcat(compiler_version_label, " ");
    strcat(compiler_version_label, pp_COMPVER);
    glutAddMenuEntry(compiler_version_label, 1);
    if(fds_version!=NULL){
      sprintf(menulabel, "  FDS version: %s", fds_version);
      glutAddMenuEntry(menulabel, 1);
    }
    if(fds_githash!=NULL){
      sprintf(menulabel,"  FDS build: %s",fds_githash);
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
#ifdef pp_QUARTZ
    glutAddMenuEntry("  Platform: OSX64/QUARTZ", 1);
#else
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
#endif
#ifdef pp_LINUX
    glutAddMenuEntry("  Platform: LINUX64", 1);
#endif
    GLUTADDSUBMENU(_("Disclaimer"),disclaimermenu);
  }

  /* --------------------------------web help menu -------------------------- */

  CREATEMENU(webhelpmenu,HelpMenu);

#ifdef WIN32
  glutAddMenuEntry(_("Downloads"), MENU_HELP_DOWNLOADS);
  glutAddMenuEntry(_("Documentation"), MENU_HELP_DOCUMENTATION);
  glutAddMenuEntry(_("Discussion forum"), MENU_HELP_FORUM);
  glutAddMenuEntry(_("Issue tracker"), MENU_HELP_ISSUES);
  glutAddMenuEntry(_("Release notes"), MENU_HELP_RELEASENOTES);
  glutAddMenuEntry(_("Home page"), MENU_HELP_FDSWEB);
#endif
#ifdef pp_OSX
  glutAddMenuEntry(_("Downloads"), MENU_HELP_DOWNLOADS);
  glutAddMenuEntry(_("Documentation"), MENU_HELP_DOCUMENTATION);
  glutAddMenuEntry(_("Discussion forum"), MENU_HELP_FORUM);
  glutAddMenuEntry(_("Issue tracker"), MENU_HELP_ISSUES);
  glutAddMenuEntry(_("Release notes"), MENU_HELP_RELEASENOTES);
  glutAddMenuEntry(_("Home page"), MENU_HELP_FDSWEB);
#endif
#ifdef pp_LINUX
  glutAddMenuEntry(_("Downloads: https://pages.nist.gov/fds-smv/"),MENU_HELP_DOWNLOADS);
  glutAddMenuEntry(_("Documentation:  https://pages.nist.gov/fds-smv/manuals.html"),MENU_HELP_DOCUMENTATION);
  glutAddMenuEntry(_("Discussion forum: https://groups.google.com/forum/?fromgroups#!forum/fds-smv"), MENU_HELP_FORUM);
  glutAddMenuEntry(_("Issue tracker: https://pages.nist.gov/fds-smv/"),MENU_HELP_ISSUES);
  glutAddMenuEntry(_("Release notes: https://pages.nist.gov/fds-smv/smv_readme.html"), MENU_HELP_RELEASENOTES);
  glutAddMenuEntry(_("Home page: https://pages.nist.gov/fds-smv/"),MENU_HELP_FDSWEB);
#endif

  /* --------------------------------keyboard help menu -------------------------- */

  CREATEMENU(keyboardhelpmenu,HelpMenu);
  if(plotstate==DYNAMIC_PLOTS){
    glutAddMenuEntry(_("Animation"),MENU_DUMMY);
    glutAddMenuEntry(_("  t: set/unset single time step mode"), MENU_DUMMY);
    glutAddMenuEntry(_("  0: reset animation to the initial time"), MENU_DUMMY);
    glutAddMenuEntry(_("  p,P: increment particle variable displayed"), MENU_DUMMY);
    glutAddMenuEntry(_("  T: time display between 'Time s' and 'h:m:s'"), MENU_DUMMY);
    if(cellcenter_slice_active==1){
      glutAddMenuEntry(_("     (also, toggles cell center display on/off)"), MENU_DUMMY);
      glutAddMenuEntry(_("  @: display FDS values in cell centered slices"), MENU_DUMMY);
    }
    glutAddMenuEntry(_("  ,: toggle colorbar display (vertical, horizontal, hidden)"), MENU_DUMMY);
    glutAddMenuEntry(_("  :: toggle timebar region overlap (always, never, only if timebar/horizontal colorbar hidden)"), MENU_DUMMY);
    glutAddMenuEntry(_("  u: reload files"), MENU_DUMMY);
    glutAddMenuEntry(_("  H: toggle  visibility of slice and vector slice files"), MENU_DUMMY);
    glutAddMenuEntry(_("  I: toggle  visibility of slices in blockages"), MENU_DUMMY);
    glutAddMenuEntry(_("  L: unload last slice file loaded"), MENU_DUMMY);
    glutAddMenuEntry(_("  1-9: number of frames to skip"), MENU_DUMMY);
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
    glutAddMenuEntry(_("  x,y,z: toggle contour plot visibility along x, y and z axis"), MENU_DUMMY);
    glutAddMenuEntry(_("  p,P: increment plot3d variable displayed"), MENU_DUMMY);
    glutAddMenuEntry(_("  v: toggle flow vector visiblity"), MENU_DUMMY);
    glutAddMenuEntry(_("  a/ALT a: increase/decrease flow vector length by 1.5"), MENU_DUMMY);
    glutAddMenuEntry(_("  s: change interval between adjacent vectors"), MENU_DUMMY);
    glutAddMenuEntry(_("  c: toggle between continuous and 2D stepped contours"), MENU_DUMMY);
    glutAddMenuEntry(_("  i: toggle iso-surface visibility"), MENU_DUMMY);
    glutAddMenuEntry(_("  {: load previous time Plot3D files"), MENU_DUMMY);
    glutAddMenuEntry(_("  }: load next time Plot3D files"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("Misc"), MENU_DUMMY);
  glutAddMenuEntry(_("  A: toggle between plot types (device and HRRPUV)"), MENU_DUMMY);
  glutAddMenuEntry(_("  r/R: render the current scene to an image file"), MENU_DUMMY);
  glutAddMenuEntry("             r: image has the same resolution as the scene", MENU_DUMMY);
  {
    char render_label[1024];
    unsigned char deg360[] = {'3','6','0',0};

    sprintf(render_label, "            R: image has %i times the resolution of of scene", MAX(2,resolution_multiplier));
    glutAddMenuEntry(render_label, MENU_DUMMY);
    sprintf(render_label, "    ALT R: %s view - all view directions are shown in a 1024x512 image", deg360);
    glutAddMenuEntry(render_label, MENU_DUMMY);
  }
  if(ntotal_blockages>0||isZoneFireModel==1){
    glutAddMenuEntry(_("  g: toggle grid visibility"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("  e: toggle between view rotation types: scene centered 2 axis, 1 axis, 3 axis and eye centered"), MENU_DUMMY);
  glutAddMenuEntry(_("  q: display blockage locations as specified by user or by FDS"), MENU_DUMMY);
  if(ntotal_blockages>0){
    glutAddMenuEntry(_("  O: toggle blockage view (normal <--> outline)"), MENU_DUMMY);
    glutAddMenuEntry(_("  ALT o: cycle between all blockage view types"), MENU_DUMMY);
  }
  if(ndeviceinfo>0&&GetNumActiveDevices()>0){
    glutAddMenuEntry("  j/ALT j: increase/decrease object size", MENU_DUMMY);
  }
  if(have_cface_normals==CFACE_NORMALS_YES){
    glutAddMenuEntry(_("  n: display cface normal vectors"), MENU_DUMMY);
  }
  glutAddMenuEntry("  ALT r: toggle research mode (global min/max for coloring data, turn off axis label smoothing)", MENU_DUMMY);
  glutAddMenuEntry(_("  W: toggle clipping - use Options/Clip menu to specify clipping planes"), MENU_DUMMY);
  glutAddMenuEntry(_("  -/space bar: decrement/increment time step, 2D contour planes, 3D contour levels"), MENU_DUMMY);
  glutAddMenuEntry("", MENU_DUMMY);
  glutAddMenuEntry(_("  ALT v: toggle projection  method (between perspective and size preserving)"), MENU_DUMMY);
  if(n_embedded_meshes>0){
    glutAddMenuEntry(_("  ALT u: toggle coarse slice display in embedded mesh"), MENU_DUMMY);
  }
  if(cellcenter_slice_active==1){
    glutAddMenuEntry(_("  ALT y: if current slice is cell centered, toggle interpolation on/off"), MENU_DUMMY);
  }
  if(caseini_filename!=NULL&&strlen(caseini_filename)>0){
    char inilabel[512];

    sprintf(inilabel,"  #: save settings to %s",caseini_filename);
    glutAddMenuEntry(inilabel,MENU_DUMMY);
  }
  else{
    glutAddMenuEntry(_("  #: save settings (create casename.ini file)"), MENU_DUMMY);
  }
  if(ngeominfo){
    glutAddMenuEntry(_("  =: toggle vertex selected in examine geometry dialog"), MENU_DUMMY);
    glutAddMenuEntry(_("  Z: toggle rotation center between FDS and FDS+GEOM center"), MENU_DUMMY);
  }
  glutAddMenuEntry(_("  !: snap scene to closest 45 degree orientation"), MENU_DUMMY);
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
      ASSERT(FFALSE);
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
      ASSERT(FFALSE);
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

  /* --------------------------------particle menu -------------------------- */

  if(npartinfo>0&&nevac!=npartinfo){
    int ii;

    CREATEMENU(unloadpartmenu,UnloadPartMenu);
    for(ii=0;ii<npartinfo;ii++){
      partdata *parti;
      char menulabel[1024];

      i = partorderindex[ii];
      parti = partinfo + i;
      if(parti->loaded==1&&parti->evac==0){
        STRCPY(menulabel,parti->menulabel);
        glutAddMenuEntry(menulabel,i);
      }
    }
    glutAddMenuEntry(_("Unload all"), MENU_PARTICLE_UNLOAD_ALL);

    if(nmeshes==1){
      CREATEMENU(particlemenu,LoadParticleMenu);
    }
    else{
      CREATEMENU(particlesubmenu,LoadParticleMenu);
    }
    for(ii=0;ii<npartinfo;ii++){
      char menulabel[1024];

      i = partorderindex[ii];
      if(partinfo[i].evac==1)continue;
      if(partinfo[i].loaded==1){
        STRCPY(menulabel,"*");
        STRCAT(menulabel,partinfo[i].menulabel);
      }
      else{
        STRCPY(menulabel,partinfo[i].menulabel);
      }
      glutAddMenuEntry(menulabel,i);
    }
    if(nmeshes>1){
      char menulabel[1024];

      CREATEMENU(particlemenu,LoadParticleMenu);
      if(npartinfo > 0){
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
      if(nmeshes>1){
        strcpy(menulabel, "Mesh");
        GLUTADDSUBMENU(menulabel, particlesubmenu);
      }
    }
    glutAddMenuEntry(_("Settings..."), MENU_PART_SETTINGS);
    if(npartloaded>1){
      GLUTADDSUBMENU(_("Unload"),unloadpartmenu);
    }
    else{
      glutAddMenuEntry(_("Unload"), MENU_PARTICLE_UNLOAD_ALL);
    }
  }

  if(nevac>0){
    int ii;

    CREATEMENU(unloadevacmenu,UnloadEvacMenu);
    for(ii=0;ii<npartinfo;ii++){
      partdata *parti;
      char menulabel[1024];

      i = partorderindex[ii];
      parti = partinfo + i;
      if(parti->loaded==1&&parti->evac==0){
        STRCPY(menulabel,parti->menulabel);
        glutAddMenuEntry(menulabel,i);
      }
    }
    glutAddMenuEntry(_("Unload all"),MENU_UNLOADEVAC_UNLOADALL);

    CREATEMENU(evacmenu,LoadEvacMenu);
    {
      int nevacs=0,nevacloaded2=0;
      char menulabel[1024];

      for(ii=0;ii<npartinfo;ii++){
        partdata *parti;

        parti = partinfo + ii;
        if(parti->evac==1){
          if(parti->loaded==1)nevacloaded2++;
          nevacs++;
        }
      }

      if(nevacs>1){
        strcpy(menulabel,_("Humans - all meshes"));
        glutAddMenuEntry(menulabel,EVACFILE_LOADALL);
        glutAddMenuEntry("-",MENU_EVAC_DUMMY);
      }
      for(ii=0;ii<npartinfo;ii++){
        i = partorderindex[ii];
        if(partinfo[i].evac==0)continue;
        if(partinfo[i].loaded==1){
          STRCPY(menulabel,"*");
          STRCAT(menulabel,partinfo[i].menulabel);
        }
        else{
          STRCPY(menulabel,partinfo[i].menulabel);
        }
        glutAddMenuEntry(menulabel,i);
      }
      if(nevacloaded2<=1){
        glutAddMenuEntry(_("Unload"),MENU_EVAC_UNLOADALL);
      }
       else{
         GLUTADDSUBMENU(_("Unload"),unloadevacmenu);
       }
  }
    }

/* --------------------------------unload and load vslice menus -------------------------- */

  if(nvsliceinfo>0){
    int ii;

    CREATEMENU(unloadvslicemenu,UnloadVSliceMenu);
    for(ii=0;ii<nvsliceinfo;ii++){
      vslicedata *vd;

      i = vsliceorderindex[ii];
      vd = vsliceinfo + i;
      if(vd->loaded==0)continue;
      glutAddMenuEntry(vd->menulabel2,i);
    }
    glutAddMenuEntry("-",MENU_DUMMY);
    glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);

    if(nvsliceinfo>0){
      vslicedata *vd, *vdim1,*vdip1;

      if(nvsliceinfo>0){
        nloadsubvslicemenu=1;
        for(ii=1;ii<nvsliceinfo;ii++){
          slicedata *sd, *sdm1;

          i=vsliceorderindex[ii];
          vd = vsliceinfo + i;
          sd = sliceinfo + vd->ival;
          if(ii>0){
            vdim1 = vsliceinfo + vsliceorderindex[ii-1];
            sdm1 = sliceinfo + vdim1->ival;
          }
          if(ii==0||strcmp(sd->label.longlabel,sdm1->label.longlabel)!=0){
            nloadsubvslicemenu++;
          }
        }
        NewMemory((void **)&loadsubvslicemenu,nloadsubvslicemenu*sizeof(int));
        for(i=0;i<nloadsubvslicemenu;i++){
          loadsubvslicemenu[i]=0;
        }
        nloadsubvslicemenu=0;
        for(ii=0;ii<nvsliceinfo;ii++){
          slicedata *sd, *sdm1, *sdp1;
          char menulabel[1024];

          i=vsliceorderindex[ii];
          vd = vsliceinfo + i;
          sd = sliceinfo + vd->ival;

          if(ii!=0){
            vdim1 = vsliceinfo + vsliceorderindex[ii-1];
            sdm1 = sliceinfo + vdim1->ival;
          }
          if(ii!=nvsliceinfo-1){
            vdip1 = vsliceinfo + vsliceorderindex[ii+1];
            sdp1 = sliceinfo + vdip1->ival;
          }

          if(ii==0||strcmp(sd->label.longlabel,sdm1->label.longlabel)!=0){
            CREATEMENU(loadsubvslicemenu[nloadsubvslicemenu],LoadVSliceMenu);
          }
          if(vd->loaded==1){
            STRCPY(menulabel,"*");
            STRCAT(menulabel,sd->menulabel);
          }
          else{
            STRCPY(menulabel,sd->menulabel);
          }
          if(sd->slicelabel!=NULL){
            STRCAT(menulabel, " - ");
            STRCAT(menulabel, sd->slicelabel);
          }
          glutAddMenuEntry(menulabel,i);
          if(ii==nvsliceinfo-1||strcmp(sd->label.longlabel,sdp1->label.longlabel)!=0){
            subvslice_menuindex[nloadsubvslicemenu]=vsliceorderindex[ii];
            if(sd->ndirxyz[1]+sd->ndirxyz[2]+sd->ndirxyz[3]>1){
              glutAddMenuEntry("-",MENU_DUMMY);
            }
            if(sd->ndirxyz[1]>1){
              glutAddMenuEntry(_A(_("Load all")," x"),-1000-4*nloadsubvslicemenu-1);
            }
            if(sd->ndirxyz[2]>1){
              glutAddMenuEntry(_A(_("Load all"), "y"),-1000-4*nloadsubvslicemenu-2);
            }
            if(sd->ndirxyz[3]>1){
              glutAddMenuEntry(_A(_("Load all"), "z"),-1000-4*nloadsubvslicemenu-3);
            }
            if(sd->ndirxyz[1]+sd->ndirxyz[2]+sd->ndirxyz[3]>1){
              glutAddMenuEntry(_("Load all"),-1000-4*nloadsubvslicemenu);
            }
          }
          if(ii==0||strcmp(sd->label.longlabel,sdm1->label.longlabel)!=0){
            nloadsubvslicemenu++;
          }
        }

        CREATEMENU(vsliceloadmenu,LoadVSliceMenu);
        nloadsubvslicemenu=0;
        for(ii=0;ii<nvsliceinfo;ii++){
          slicedata *sd, *sdm1;

          i=vsliceorderindex[ii];
          vd = vsliceinfo + i;
          sd = sliceinfo + vd->ival;
          if(ii>0){
            vdim1 = vsliceinfo + vsliceorderindex[ii-1];
            sdm1 = sliceinfo + vdim1->ival;
          }
          if(ii==0||strcmp(sd->label.longlabel,sdm1->label.longlabel)!=0){
            char mlabel[1024], mlabel2[1024];

            STRCPY(mlabel,sd->label.longlabel);
            if((ii==0&&sd->mesh_type>0)||(ii>0&&sd->mesh_type!=sdm1->mesh_type)){
              sprintf(mlabel2,"*** Evac type %i meshdata ***",sd->mesh_type);
              glutAddMenuEntry(mlabel2,MENU_DUMMY);
            }
            GLUTADDSUBMENU(mlabel,loadsubvslicemenu[nloadsubvslicemenu]);
            nloadsubvslicemenu++;
          }
        }
        glutAddMenuEntry("-",MENU_DUMMY);
        GLUTADDSUBMENU(_("Unload"),unloadvslicemenu);

        if(nslicedups > 0){
          CREATEMENU(duplicatevectorslicemenu,LoadMultiVSliceMenu);
          if(vectorslicedup_option == SLICEDUP_KEEPALL){
            glutAddMenuEntry(_("  *keep all"), MENU_KEEP_ALL);
          }
          else{
            glutAddMenuEntry(_("  keep all"), MENU_KEEP_ALL);
          }
          if(vectorslicedup_option == SLICEDUP_KEEPFINE){
            glutAddMenuEntry(_("  *keep fine"), MENU_KEEP_FINE);
          }
          else{
            glutAddMenuEntry(_("  keep fine"), MENU_KEEP_FINE);
          }
          if(vectorslicedup_option == SLICEDUP_KEEPCOARSE){
            glutAddMenuEntry(_("  *keep coarse"), MENU_KEEP_COARSE);
          }
          else{
            glutAddMenuEntry(_("  keep coarse"), MENU_KEEP_COARSE);
          }
        }
        if(nmultivsliceinfo<nvsliceinfo){
          CREATEMENU(unloadmultivslicemenu,UnloadMultiVSliceMenu);
          for(i=0;i<nmultivsliceinfo;i++){
            multivslicedata *mvslicei;

            mvslicei = multivsliceinfo + i;
            if(mvslicei->loaded!=0){
              glutAddMenuEntry(mvslicei->menulabel2,i);
            }
          }
          glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);

          nloadsubmvslicemenu=1;
          for(i=1;i<nmultivsliceinfo;i++){
            vslicedata *vi, *vim1;
            slicedata *si, *sim1;

            vi = vsliceinfo + (multivsliceinfo+i)->ivslices[0];
            vim1 = vsliceinfo + (multivsliceinfo+i-1)->ivslices[0];
            si = sliceinfo + vi->ival;
            sim1 = sliceinfo + vim1->ival;
            if(strcmp(si->label.longlabel,sim1->label.longlabel)!=0){
              nloadsubmvslicemenu++;
            }
          }
          NewMemory((void **)&loadsubmvslicemenu,nloadsubmvslicemenu*sizeof(int));
          for(i=0;i<nloadsubmvslicemenu;i++){
            loadsubmvslicemenu[i]=0;
          }

          nmultisliceloaded=0;
          nloadsubmvslicemenu=0;
          for(i=0;i<nmultivsliceinfo;i++){
            vslicedata *vi, *vim1, *vip1;
            slicedata *si, *sim1, *sip1;
            char menulabel[1024];
            multivslicedata *mvslicei;

            mvslicei = multivsliceinfo + i;

            if(i>0){
              vim1 = vsliceinfo + (multivsliceinfo+i-1)->ivslices[0];
              sim1 = sliceinfo + vim1->ival;
            }
            vi = vsliceinfo + mvslicei->ivslices[0];
            si = sliceinfo + vi->ival;
            if(i<nmultivsliceinfo-1){
              vip1 = vsliceinfo + (multivsliceinfo+i+1)->ivslices[0];
              sip1 = sliceinfo + vip1->ival;
            }
            if(i==0||strcmp(si->label.longlabel,sim1->label.longlabel)!=0){
              CREATEMENU(loadsubmvslicemenu[nloadsubmvslicemenu],LoadMultiVSliceMenu);
              msubvslice_menuindex[nloadsubmvslicemenu] = vi->ival;
            }

            STRCPY(menulabel,"");
            if(mvslicei->loaded==1){
              STRCAT(menulabel,"*");
              nmultisliceloaded++;
            }
            else if(mvslicei->loaded==-1){
              STRCAT(menulabel,"#");
            }
            else{
            }
            STRCAT(menulabel,mvslicei->menulabel);
            if(si->slicelabel!=NULL){
              STRCAT(menulabel," - ");
              STRCAT(menulabel,si->slicelabel);
            }
            glutAddMenuEntry(menulabel,i);
            if(i==nmultivsliceinfo-1||strcmp(si->label.longlabel,sip1->label.longlabel)!=0){
              if(mvslicei->ndirxyz[1]+ mvslicei->ndirxyz[2] + mvslicei->ndirxyz[3]>1){
                glutAddMenuEntry("-",MENU_DUMMY);
              }
              if(mvslicei->ndirxyz[1]>1){
                glutAddMenuEntry(_A(_("Load all"), " x"),-1000-4*nloadsubmvslicemenu-1);
              }
              if(mvslicei->ndirxyz[2]>1){
                glutAddMenuEntry(_A(_("Load all"), " y"),-1000-4*nloadsubmvslicemenu-2);
              }
              if(mvslicei->ndirxyz[3]>1){
                glutAddMenuEntry(_A(_("Load all"), " z"),-1000-4*nloadsubmvslicemenu-3);
              }
              if(mvslicei->ndirxyz[1]+mvslicei->ndirxyz[2]+mvslicei->ndirxyz[3]>1){
                glutAddMenuEntry(_("Load all"),-1000-4*nloadsubmvslicemenu);
              }
            }
            if(i==0||strcmp(si->label.longlabel,sim1->label.longlabel)!=0){
              nloadsubmvslicemenu++;
            }
          }

          nloadsubmvslicemenu=0;
          CREATEMENU(loadmultivslicemenu,LoadMultiVSliceMenu);
          for(i=0;i<nmultivsliceinfo;i++){
            vslicedata *vi, *vim1;
            slicedata *si, *sim1;

            vi = vsliceinfo + (multivsliceinfo+i)->ivslices[0];
            si = sliceinfo + vi->ival;
            if(i>0){
              vim1 = vsliceinfo + (multivsliceinfo+i-1)->ivslices[0];
              sim1 = sliceinfo + vim1->ival;
            }
            if(i==0||(i>0&&strcmp(si->label.longlabel,sim1->label.longlabel)!=0)){
              char mlabel[1024], mlabel2[1024];

              STRCPY(mlabel,si->label.longlabel);
              if((i==0&&si->mesh_type>0)||(i>0&&si->mesh_type!=sim1->mesh_type)){
                sprintf(mlabel2,"*** Evac type %i meshes ***",si->mesh_type);
                glutAddMenuEntry(mlabel2,MENU_DUMMY);
              }
              GLUTADDSUBMENU(mlabel,loadsubmvslicemenu[nloadsubmvslicemenu]);
              nloadsubmvslicemenu++;
            }
          }
          if(nmultivsliceinfo>0)glutAddMenuEntry("-",MENU_DUMMY);
        }
      }
    }
    if(nslicedups > 0){
      GLUTADDSUBMENU(_("Duplicate vector slices"), duplicatevectorslicemenu);
    }
    if(nmultivsliceinfo>0&&nmultivsliceinfo<nvsliceinfo){
      char loadmenulabel[100];
      char steplabel[100];

      strcpy(loadmenulabel, "Mesh");
      if(sliceframestep>1){
        sprintf(steplabel, "/Skip %i", sliceframeskip);
        strcat(loadmenulabel, steplabel);
      }
      GLUTADDSUBMENU(loadmenulabel, vsliceloadmenu);
    }
    glutAddMenuEntry(_("Settings..."), MENU_LOADVSLICE_SETTINGS);
    if(nvsliceloaded>1){
      GLUTADDSUBMENU(_("Unload"),unloadmultivslicemenu);
    }
    else{
     glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
    }
  }

// setup boundary slices for slice files

  {
    int ii;

// count patch submenus

    have_geom_slice_menus=0;
    nloadsubpatchmenu_s = 0;
    for(ii = 0;ii<npatchinfo;ii++){
      int im1;
      patchdata *patchi, *patchim1;

      i = patchorderindex[ii];
      if(ii>0){
        im1 = patchorderindex[ii-1];
        patchim1 = patchinfo+im1;
      }
      patchi = patchinfo+i;
      if(ii==0||strcmp(patchi->menulabel_base, patchim1->menulabel_base)!=0){
        nloadsubpatchmenu_s++;
      }
    }

// create patch submenus

    if(nloadsubpatchmenu_s > 0){
      NewMemory((void **)&loadsubpatchmenu_s, nloadsubpatchmenu_s * sizeof(int));
      NewMemory((void **)&nsubpatchmenus_s, nloadsubpatchmenu_s * sizeof(int));
    }
    for(i = 0;i<nloadsubpatchmenu_s;i++){
      loadsubpatchmenu_s[i] = 0;
      nsubpatchmenus_s[i] = 0;
    }

    iloadsubpatchmenu_s = 0;
    for(ii = 0;ii<npatchinfo;ii++){
      int im1;
      patchdata *patchi, *patchim1;

      i = patchorderindex[ii];
      if(ii>0){
        im1 = patchorderindex[ii-1];
        patchim1 = patchinfo+im1;
      }
      patchi = patchinfo+i;
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

  /* --------------------------------unload and load slice menus -------------------------- */

  if(nsliceinfo>0||have_geom_slice_menus==1){
    CREATEMENU(unloadslicemenu,UnloadSliceMenu);
    for(i=0;i<nsliceinfo;i++){
      slicedata *sd;
      char menulabel[1024];

      sd = sliceinfo + sliceorderindex[i];
      if(sd->loaded==1){
        STRCPY(menulabel,sd->menulabel2);
        glutAddMenuEntry(menulabel,sliceorderindex[i]);
      }
    }
    for(i = 0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo+i;
      if(patchi->loaded==1&&patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
        glutAddMenuEntry(patchi->label.longlabel, -3-i);
        geom_slice_loaded++;
      }
    }

    glutAddMenuEntry("-",MENU_DUMMY);
    glutAddMenuEntry(_("Unload last"),UNLOAD_LAST);
    glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);

//  count slice submenus

    nloadsubslicemenu=1;
    for(i=1;i<nsliceinfo;i++){
      slicedata *sd,*sdim1;

      sd = sliceinfo + sliceorderindex[i];
      sdim1 = sliceinfo + sliceorderindex[i-1];
      if(strcmp(sd->label.longlabel,sdim1->label.longlabel)!=0)nloadsubslicemenu++;
    }

// create slice submenus

    if(nloadsubslicemenu>0)NewMemory((void **)&loadsubslicemenu,nloadsubslicemenu*sizeof(int));
    for(i=0;i<nloadsubslicemenu;i++){
      loadsubslicemenu[i]=0;
    }
    iloadsubslicemenu=0;
    for(i=0;i<nsliceinfo;i++){
      slicedata *sd,*sdim1,*sdip1;
      char menulabel[1024];

      if(i!=0){
        sdim1 = sliceinfo + sliceorderindex[i-1];
      }
      sd = sliceinfo + sliceorderindex[i];
      if(i!=nsliceinfo-1){
        sdip1 = sliceinfo + sliceorderindex[i+1];
      }
      if(i==0||strcmp(sd->label.longlabel,sdim1->label.longlabel)!=0){
        CREATEMENU(loadsubslicemenu[iloadsubslicemenu],LoadSliceMenu);
      }
      STRCPY(menulabel,"");
      if(sd->loaded==1){
        STRCAT(menulabel,"*");
      }
      STRCAT(menulabel,sd->menulabel);
      if(sd->slicelabel!=NULL){
        STRCAT(menulabel," - ");
        STRCAT(menulabel,sd->slicelabel);
      }
      if(sd->menu_show==1){
        int doit;

        doit=1;
        if(sd->slice_filetype==SLICE_TERRAIN){
          meshdata *meshslice;
          terraindata *terri;

          meshslice = meshinfo + sd->blocknumber;
          terri = meshslice->terrain;
          if(terri==NULL||terri->nvalues==0)doit = 0;
        }
        if(doit==1)glutAddMenuEntry(menulabel,sliceorderindex[i]);
      }
      if(i==nsliceinfo-1||strcmp(sd->label.longlabel,sdip1->label.longlabel)!=0){
        subslice_menuindex[iloadsubslicemenu]=sliceorderindex[i];
        if(sd->ndirxyz[1]+sd->ndirxyz[2]+sd->ndirxyz[3]>1){
          glutAddMenuEntry("-",MENU_DUMMY);
        }
        if(sd->ndirxyz[1]>1){
          glutAddMenuEntry(_A(_("Load all"), " x"),-1000-4*iloadsubslicemenu-1);
        }
        if(sd->ndirxyz[2]>1){
          glutAddMenuEntry(_A(_("Load all"), " y"),-1000-4*iloadsubslicemenu-2);
        }
        if(sd->ndirxyz[3]>1){
          glutAddMenuEntry(_A(_("Load all"), " z"),-1000-4*iloadsubslicemenu-3);
        }
        if(sd->ndirxyz[1]+sd->ndirxyz[2]+sd->ndirxyz[3]>1){
          glutAddMenuEntry(_("Load all"),-1000-4*iloadsubslicemenu);
        }
      }
      if(i!=nsliceinfo-1&&strcmp(sd->label.longlabel,sdip1->label.longlabel)!=0){
        iloadsubslicemenu++;
      }
    }

    CREATEMENU(sliceskipmenu, SliceSkipMenu);
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

// call slice submenus from main slice menu

    CREATEMENU(loadslicemenu,LoadSliceMenu);
    iloadsubslicemenu=0;
    for(i=0;i<nsliceinfo;i++){
      slicedata *sd,*sdim1;

      sd = sliceinfo + sliceorderindex[i];
      if(i>0)sdim1 = sliceinfo + sliceorderindex[i-1];
      if(i==0||strcmp(sd->label.longlabel,sdim1->label.longlabel)!=0){
        char mlabel[1024],mlabel2[1024];;

        STRCPY(mlabel,sd->label.longlabel);
        if((i==0&&sd->mesh_type>0)||(i>0&&sd->mesh_type!=sdim1->mesh_type)){
        if(sd->menu_show==1){
            sprintf(mlabel2,"*** Evac type %i meshdata ***",sd->mesh_type);
            glutAddMenuEntry(mlabel2,MENU_DUMMY);
          }
        }
        if(sd->menu_show==1)GLUTADDSUBMENU(mlabel,loadsubslicemenu[iloadsubslicemenu]);
        iloadsubslicemenu++;
      }
    }
// menus for boundary/slice geometry files
    {
      int ii;

      iloadsubpatchmenu_s = 0;
      for(ii = 0;ii<npatchinfo;ii++){
        int im1;
        patchdata *patchi, *patchim1;

        i = patchorderindex[ii];
        if(ii>0){
          im1 = patchorderindex[ii-1];
          patchim1 = patchinfo+im1;
        }
        patchi = patchinfo+i;
        if(ii==0||strcmp(patchi->menulabel_base, patchim1->menulabel_base)!=0){
          if(nsubpatchmenus_s[iloadsubpatchmenu_s]>0){
            GLUTADDSUBMENU(patchi->menulabel_base, loadsubpatchmenu_s[iloadsubpatchmenu_s]);
          }
          iloadsubpatchmenu_s++;
        }
      }
    }
    glutAddMenuEntry("-", MENU_DUMMY);
    if(nsliceloaded+geom_slice_loaded>1){
      GLUTADDSUBMENU(_("Unload"),unloadslicemenu);
    }
    else{
      glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
    }
  }

  /* --------------------------------unload and load multislice menus -------------------------- */

  if(nmultisliceinfo+nfedinfo<nsliceinfo||have_geom_slice_menus==1){
    CREATEMENU(unloadmultislicemenu, UnloadMultiSliceMenu);
    nmultisliceloaded = 0;
    for(i = 0;i<nmultisliceinfo;i++){
      multislicedata *mslicei;

      mslicei = multisliceinfo+i;
      if(mslicei->loaded!=0){
        glutAddMenuEntry(mslicei->menulabel2, i);
      }
    }
    for(i = 0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo+i;
      if(patchi->loaded==1&&patchi->filetype_label!=NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
        glutAddMenuEntry(patchi->label.longlabel, -3-i);
      }
    }
    glutAddMenuEntry(_("Unload all"), UNLOAD_ALL);

    nloadsubmslicemenu = 1;
    for(i = 1;i<nmultisliceinfo;i++){
      slicedata *sd, *sdim1;

      sd = sliceinfo+(multisliceinfo+i)->islices[0];
      sdim1 = sliceinfo+(multisliceinfo+i-1)->islices[0];
      if(strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0)nloadsubmslicemenu++;
    }
    NewMemory((void **)&loadsubmslicemenu, nloadsubmslicemenu*sizeof(int));
    for(i = 0;i<nloadsubmslicemenu;i++){
      loadsubmslicemenu[i] = 0;
    }
    nloadsubmslicemenu = 0;
    for(i = 0;i<nmultisliceinfo;i++){
      slicedata *sd, *sdim1, *sdip1;
      char menulabel[1024];
      multislicedata *mslicei,*msliceim1,*msliceip1;

      if(i>0){
        msliceim1 = multisliceinfo+i-1;
        sdim1 = sliceinfo+msliceim1->islices[0];
      }
      mslicei = multisliceinfo+i;
      sd = sliceinfo+mslicei->islices[0];
      if(i<nmultisliceinfo-1){
        msliceip1 = multisliceinfo+i+1;
        sdip1 = sliceinfo+msliceip1->islices[0];
      }

      if(i==0||strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0){
        msubslice_menuindex[nloadsubmslicemenu]=mslicei->islices[0];
        CREATEMENU(loadsubmslicemenu[nloadsubmslicemenu], LoadMultiSliceMenu);
        nloadsubmslicemenu++;
      }
      STRCPY(menulabel, "");
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
      glutAddMenuEntry(menulabel, i);
      if(i==nmultisliceinfo-1||strcmp(sd->label.longlabel, sdip1->label.longlabel)!=0){
        if(mslicei->ndirxyz[1] + mslicei->ndirxyz[2] + mslicei->ndirxyz[3] > 1){
          glutAddMenuEntry("-", MENU_DUMMY);
        }
        if(mslicei->ndirxyz[1]>1){
          glutAddMenuEntry(_A(_("Load all"), " x"),-1000-4*(nloadsubmslicemenu-1)-1);
        }
        if(mslicei->ndirxyz[2]>1){
          glutAddMenuEntry(_A(_("Load all"), " y"),-1000-4*(nloadsubmslicemenu-1)-2);
        }
        if(mslicei->ndirxyz[3]>1){
          glutAddMenuEntry(_A(_("Load all"), " z"),-1000-4*(nloadsubmslicemenu-1)-3);
        }
        if(mslicei->ndirxyz[1]+mslicei->ndirxyz[2]+mslicei->ndirxyz[3]>1){
          glutAddMenuEntry(_("Load all"),  -1000-4*(nloadsubmslicemenu-1));
        }
      }
    }
    if(nslicedups>0){
      CREATEMENU(duplicateslicemenu,LoadMultiSliceMenu);
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
    CREATEMENU(loadmultislicemenu, LoadMultiSliceMenu);
    nloadsubmslicemenu = 0;
    for(i = 0;i<nmultisliceinfo;i++){
      slicedata *sd, *sdim1;

      sd = sliceinfo+(multisliceinfo+i)->islices[0];
      if(i>0)sdim1 = sliceinfo+(multisliceinfo+i-1)->islices[0];

      if(i==0||strcmp(sd->label.longlabel, sdim1->label.longlabel)!=0){
        char mlabel[1024], mlabel2[1024];

        STRCPY(mlabel, sd->label.longlabel);
        if((i==0&&sd->mesh_type>0)||(i>0&&sd->mesh_type!=sdim1->mesh_type)){
          sprintf(mlabel2, "*** Evac type %i meshes ***", sd->mesh_type);
          glutAddMenuEntry(mlabel2, MENU_DUMMY);
        }
        GLUTADDSUBMENU(mlabel, loadsubmslicemenu[nloadsubmslicemenu]);
        nloadsubmslicemenu++;
      }
    }
    if(nmeshes>0){
      int ii;

      iloadsubpatchmenu_s = 0;
      for(ii = 0;ii<npatchinfo;ii++){
        int im1;
        patchdata *patchi, *patchim1;

        i = patchorderindex[ii];
        if(ii>0){
          im1 = patchorderindex[ii-1];
          patchim1 = patchinfo+im1;
        }
        patchi = patchinfo+i;
        if(ii==0||strcmp(patchi->menulabel_base, patchim1->menulabel_base)!=0){
          if(nsubpatchmenus_s[iloadsubpatchmenu_s]>0){
            GLUTADDSUBMENU(patchi->menulabel_base, loadsubpatchmenu_s[iloadsubpatchmenu_s]);
          }
          iloadsubpatchmenu_s++;
        }
      }
    }

    if(nmultisliceinfo>0)glutAddMenuEntry("-", MENU_DUMMY);
    GLUTADDSUBMENU(_("Skip"), sliceskipmenu);
    if(use_set_slicecolor==1){
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
    if(nsliceinfo>0&&nmultisliceinfo+nfedinfo<nsliceinfo){
      char loadmenulabel[100];
      char steplabel[100];

      strcpy(loadmenulabel,"Mesh");
      if(sliceframeskip>0){
        sprintf(steplabel,"/Skip %i",sliceframeskip);
        strcat(loadmenulabel,steplabel);
      }
      GLUTADDSUBMENU(loadmenulabel,loadslicemenu);
    }
    glutAddMenuEntry(_("Settings..."), MENU_SLICE_SETTINGS);
    if(nmultisliceloaded+geom_slice_loaded>1){
      GLUTADDSUBMENU(_("Unload"), unloadmultislicemenu);
    }
    else{
      glutAddMenuEntry(_("Unload"), UNLOAD_ALL);
    }

  }

/* --------------------------------unload and load 3d vol smoke menus -------------------------- */

    if(nvolrenderinfo>0){
      CREATEMENU(loadvolsmokesinglemenu, LoadVolsmoke3DMenu);
      for(i=0;i<nmeshes;i++){
        meshdata *meshi;
        volrenderdata *vr;
        char menulabel[1024];

        meshi = meshinfo + i;
        vr = &(meshi->volrenderinfo);
        if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
        strcpy(menulabel,"");
        if(vr->loaded==1)strcat(menulabel,"*");
        strcat(menulabel,meshi->label);
        glutAddMenuEntry(menulabel,i);
      }
    }
    if(nvolsmoke3dloaded>0){
      CREATEMENU(unloadvolsmoke3dmenu,UnLoadVolsmoke3DMenu);
      if(nvolsmoke3dloaded>1){
        char vlabel[256];

        strcpy(vlabel,_("3D smoke (Volume rendered)"));
        glutAddMenuEntry(vlabel,UNLOAD_ALL);
      }
      for(i=0;i<nmeshes;i++){
        meshdata *meshi;
        volrenderdata *vr;

        meshi = meshinfo + i;
        vr = &(meshi->volrenderinfo);
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
      GLUTADDSUBMENU(_("Mesh"), loadvolsmokesinglemenu);
      glutAddMenuEntry(_("Settings..."), MENU_VOLSMOKE_SETTINGS);
      if(nvolsmoke3dloaded==1)glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
      if(nvolsmoke3dloaded>1)GLUTADDSUBMENU(_("Unload"),unloadvolsmoke3dmenu);
    }

    /* --------------------------------unload and load 3d smoke menus -------------------------- */

      if(nsmoke3dloaded>0){
        CREATEMENU(unloadsmoke3dsinglemenu,UnLoadSmoke3DMenu);
        for(i=0;i<nsmoke3dinfo;i++){
          smoke3ddata *smoke3di;
          char smokemenulabel[256];

          smoke3di=smoke3dinfo+i;
          if(smoke3di->loaded==0)continue;
          strcpy(smokemenulabel, smoke3di->menulabel);
          strcat(smokemenulabel, "- ");
          strcat(smokemenulabel, smoke3di->label.longlabel);
          glutAddMenuEntry(smokemenulabel,i);
        }
        CREATEMENU(unloadsmoke3dmenu,UnLoadSmoke3DMenu);
        if(nsootloaded>0)glutAddMenuEntry(_("SOOT DENSITY"), MENU_UNLOADSMOKE3D_UNLOADALLSOOT);
        if(nhrrpuvloaded>0)glutAddMenuEntry(_("HRRPUV"), MENU_UNLOADSMOKE3D_UNLOADALLFIRE);
        if(ntemploaded>0)glutAddMenuEntry(_("TEMPERATURE"), MENU_UNLOADSMOKE3D_UNLOADALLTEMP);
        if(nco2loaded>0)glutAddMenuEntry(_("CARBON DIOXIDE DENSITY"), MENU_UNLOADSMOKE3D_UNLOADALLCO2);
        if(nsootloaded+nhrrpuvloaded+ntemploaded+nco2loaded>0)glutAddMenuEntry("-", MENU_DUMMY);
        GLUTADDSUBMENU(_("Mesh"), unloadsmoke3dsinglemenu);
      }
    {
      smoke3ddata *smoke3di;
      int n_soot_menu=0, n_hrr_menu=0;
      int n_temp_menu = 0, n_co2_menu = 0;

      if(nsmoke3dinfo>0){
        char menulabel[1024];

        if(nmeshes==1){
          CREATEMENU(loadsmoke3dmenu,LoadSmoke3DMenu);
        }

        // 3d smoke soot menu

        if(nmeshes>1){
          CREATEMENU(loadsmoke3dsootmenu,LoadSmoke3DMenu);
        }
        for(i=0;i<nsmoke3dinfo;i++){
          smoke3di = smoke3dinfo + i;
          if(smoke3di->type!=SOOT)continue;
          n_soot_menu++;
          strcpy(menulabel,"");
          if(smoke3di->loaded==1){
            strcat(menulabel,"*");
          }
          strcat(menulabel,smoke3di->menulabel);
          glutAddMenuEntry(menulabel,i);
        }

        // 3d smoke hrrpuv menu

        if(nmeshes>1){
          CREATEMENU(loadsmoke3dhrrmenu,LoadSmoke3DMenu);
        }
        for(i=0;i<nsmoke3dinfo;i++){
          smoke3di = smoke3dinfo + i;
          if(smoke3di->type!=HRRPUV)continue;
          n_hrr_menu++;
          strcpy(menulabel,"");
          if(smoke3di->loaded==1){
            strcat(menulabel,"*");
          }
          strcat(menulabel,smoke3di->menulabel);
          glutAddMenuEntry(menulabel,i);
        }

        // 3d smoke temperature menu

        if(nmeshes > 1){
          CREATEMENU(loadsmoke3dtempmenu, LoadSmoke3DMenu);
        }
        for(i = 0;i < nsmoke3dinfo;i++){
          smoke3di = smoke3dinfo + i;
          if(smoke3di->type != TEMP)continue;
          n_temp_menu++;
          strcpy(menulabel, "");
          if(smoke3di->loaded == 1){
            strcat(menulabel, "*");
          }
          strcat(menulabel, smoke3di->menulabel);
          glutAddMenuEntry(menulabel, i);
        }

        // 3d smoke co2 menu

        if(nmeshes > 1){
          CREATEMENU(loadsmoke3dco2menu, LoadSmoke3DMenu);
        }
        for(i = 0;i < nsmoke3dinfo;i++){
          smoke3di = smoke3dinfo + i;
          if(smoke3di->type != CO2)continue;
          n_co2_menu++;
          strcpy(menulabel, "");
          if(smoke3di->loaded == 1){
            strcat(menulabel, "*");
          }
          strcat(menulabel, smoke3di->menulabel);
          glutAddMenuEntry(menulabel, i);
        }
        if(nmeshes>1){
          int useitem;
          smoke3ddata *smoke3dj;

          if(n_soot_menu>0||n_hrr_menu>0){
            CREATEMENU(loadsmoke3dsinglemenu,LoadSmoke3DMenu);
            if(n_soot_menu>0)GLUTADDSUBMENU(_("SOOT DENSITY"),loadsmoke3dsootmenu);
            if(n_hrr_menu>0)GLUTADDSUBMENU(_("HRRPUV"),loadsmoke3dhrrmenu);
            if(n_temp_menu > 0)GLUTADDSUBMENU(_("TEMPERATURE"), loadsmoke3dtempmenu);
            if(n_co2_menu > 0)GLUTADDSUBMENU(_("CO2"), loadsmoke3dco2menu);
          }

          CREATEMENU(loadsmoke3dmenu,LoadSmoke3DMenu);
          {
#ifdef pp_SMOKE_FAST
            // comment out for now
           // if(smoke3d_load_test==1)glutAddMenuEntry("*smoke3d load test", MENU_SMOKE3D_LOAD_TEST);
           // if(smoke3d_load_test == 0)glutAddMenuEntry("smoke3d load test", MENU_SMOKE3D_LOAD_TEST);
#endif
          }
          // multi mesh smoke menus items
          for(i=0;i<nsmoke3dinfo;i++){
            int j;

            useitem=i;
            smoke3di=smoke3dinfo + i;
            for(j=0;j<i;j++){
              smoke3dj = smoke3dinfo + j;
              if(strcmp(smoke3di->label.longlabel,smoke3dj->label.longlabel)==0){
                useitem=-1;
                break;
              }
            }
            if(useitem!=-1){
              strcpy(menulabel, "");
              if(nsootloaded>0 && (
                strcmp(smoke3di->label.longlabel, "SOOT MASS FRACTION") == 0 ||
                strcmp(smoke3di->label.longlabel, "SOOT DENSITY") == 0)
                ){
                strcat(menulabel, "*");
              }
              if(nhrrpuvloaded>0&&strcmp(smoke3di->label.longlabel, "HRRPUV")==0)strcat(menulabel, "*");
              if(ntemploaded>0 && strcmp(smoke3di->label.longlabel, "TEMPERATURE") == 0)strcat(menulabel, "*");
              if(nco2loaded>0 && strcmp(smoke3di->label.longlabel, "CARBON DIOXIDE DENSITY") == 0)strcat(menulabel, "*");
              strcat(menulabel,smoke3di->label.longlabel);
              glutAddMenuEntry(menulabel,-useitem-100);
            }
          }
#ifdef pp_SMOKE_FAST
          if(n_soot_menu>0&&n_hrr_menu>0){
            glutAddMenuEntry("SOOT/HRRPUV(experimental loading)",MENU_SMOKE_SOOT_HRRPUV);
          }
#endif
        }

        glutAddMenuEntry("-", MENU_DUMMY3);
        if(compute_smoke3d_file_sizes==1){
          glutAddMenuEntry(_("*Show 3D smoke file size"), MENU_SMOKE_FILE_SIZES);
        }
        else{
          glutAddMenuEntry(_("Show 3D smoke file size"), MENU_SMOKE_FILE_SIZES);
        }
        if(nmeshes>1&&(n_soot_menu>0||n_hrr_menu>0)){
          GLUTADDSUBMENU(_("Mesh"), loadsmoke3dsinglemenu);
        }
        glutAddMenuEntry(_("Settings..."), MENU_SMOKE_SETTINGS);
        if(nsmoke3dloaded==1)glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
        if(nsmoke3dloaded>1)GLUTADDSUBMENU(_("Unload"),unloadsmoke3dmenu);
      }
    }

/* --------------------------------plot3d menu -------------------------- */

    if(nplot3dinfo>0){
      plot3ddata *plot3dim1, *plot3di;
      char menulabel[1024];
      int ii;

      CREATEMENU(unloadplot3dmenu,UnloadPlot3dMenu);
#ifndef pp_PLOT3D_REDUCEMENUS
      for(ii=0;ii<nplot3dinfo;ii++){
        i=plot3dorderindex[ii];
        plot3di = plot3dinfo + i;
        if(ii==0){
          strcpy(menulabel,plot3di->longlabel);
          glutAddMenuEntry(menulabel,MENU_PLOT3D_DUMMY);
        }
        if(ii!=0&&strcmp(plot3di->longlabel,plot3dinfo[plot3dorderindex[ii-1]].longlabel)!=0){
          glutAddMenuEntry(plot3di->longlabel,MENU_PLOT3D_DUMMY);
        }
        if(plot3di->loaded==0)continue;
        STRCPY(menulabel,plot3dinfo[i].menulabel);
        glutAddMenuEntry(menulabel,i);
      }
#endif
      glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);

#ifndef pp_PLOT3D_REDUCEMENUS
      nloadsubplot3dmenu=1;
      for(ii=1;ii<nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        im1 = plot3dorderindex[ii-1];
        plot3di = plot3dinfo + i;
        plot3dim1 = plot3dinfo + im1;
        if(ABS(plot3di->time-plot3dim1->time)>0.1)nloadsubplot3dmenu++;
      }
      NewMemory((void **)&loadsubplot3dmenu,nloadsubplot3dmenu*sizeof(int));
      for(i=0;i<nloadsubplot3dmenu;i++){
        loadsubplot3dmenu[i]=0;
      }

      nloadsubplot3dmenu=0;
      i = plot3dorderindex[0];
      plot3di = plot3dinfo + i;
      CREATEMENU(loadsubplot3dmenu[nloadsubplot3dmenu],LoadPlot3dMenu);
      strcpy(menulabel,"");
      if(plot3di->loaded==1){
        strcat(menulabel,"*");
      }
      strcat(menulabel,plot3di->menulabel);
      glutAddMenuEntry(menulabel,i);
      nloadsubplot3dmenu++;

      for(ii=1;ii<nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        im1 = plot3dorderindex[ii-1];
        plot3di = plot3dinfo + i;
        plot3dim1 = plot3dinfo + im1;
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
#endif

      nloadsubplot3dmenu=0;
#ifndef pp_PLOT3D_REDUCEMENUS
      CREATEMENU(plot3dsinglemeshmenu,LoadPlot3dMenu);
      for(ii=0;ii<nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        plot3di = plot3dinfo + i;
        if(ii==0){
          sprintf(menulabel,"  %f",plot3di->time);
          TrimZeros(menulabel);
          strcat(menulabel," s");
          if(nmeshes>1){
            GLUTADDSUBMENU(menulabel,loadsubplot3dmenu[nloadsubplot3dmenu]);
          }
          nloadsubplot3dmenu++;
        }
        if(ii!=0){
          i = plot3dorderindex[ii];
          im1 = plot3dorderindex[ii-1];
          plot3di = plot3dinfo + i;
          plot3dim1 = plot3dinfo + im1;
          if(strcmp(plot3di->longlabel,plot3dim1->longlabel)!=0){
            glutAddMenuEntry(plot3di->longlabel,MENU_PLOT3D_DUMMY);
          }
          if(ABS(plot3di->time-plot3dim1->time)>0.1){
            sprintf(menulabel,"  %f",plot3di->time);
            TrimZeros(menulabel);
            strcat(menulabel," s");
            if(nmeshes>1){
              GLUTADDSUBMENU(menulabel,loadsubplot3dmenu[nloadsubplot3dmenu]);
            }
            nloadsubplot3dmenu++;
          }
        }
      }
#endif

      nloadsubplot3dmenu=0;
      CREATEMENU(loadplot3dmenu,LoadPlot3dMenu);
      for(ii=0;ii<nplot3dinfo;ii++){
        int im1;

        i = plot3dorderindex[ii];
        plot3di = plot3dinfo + i;
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
          if(nmeshes>1){
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
          plot3di = plot3dinfo + i;
          plot3dim1 = plot3dinfo + im1;
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
            if(nmeshes>1){
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
        if(ii==nplot3dinfo-1){
          glutAddMenuEntry("-", MENU_PLOT3D_DUMMY);
#ifndef pp_PLOT3D_REDUCEMENUS
          if(nmeshes>1){
            GLUTADDSUBMENU(_("Mesh"), plot3dsinglemeshmenu);
          }
#endif
          glutAddMenuEntry(_("Settings..."), MENU_PLOT3D_SETTINGS);
          if(nplot3dloaded>1){
            GLUTADDSUBMENU(_("Unload"), unloadplot3dmenu);
          }
          else{
            glutAddMenuEntry(_("Unload"), UNLOAD_ALL);
          }
        }
      }
    }

/* --------------------------------load patch menu -------------------------- */

    if(npatchinfo>0){
      int ii;
      int npatchloaded2=0;

      if(nmeshes>1||n_mirrorvents>0||n_openvents>0){
        CREATEMENU(includepatchmenu, LoadBoundaryMenu);
        if(nmeshes>1){
          if(show_bndf_mesh_interface==1)glutAddMenuEntry("*Mesh interface", MENU_BNDF_SHOW_MESH_INTERFACE);
          if(show_bndf_mesh_interface==0)glutAddMenuEntry("Mesh interface", MENU_BNDF_SHOW_MESH_INTERFACE);
        }
        if(n_mirrorvents>0){
          if(show_mirror_boundary==1)glutAddMenuEntry("*Mirror surface", MENU_BNDF_MIRROR);
          if(show_mirror_boundary==0)glutAddMenuEntry("Mirror surface", MENU_BNDF_MIRROR);
        }
        if(n_openvents>0){
          if(show_open_boundary==1)glutAddMenuEntry("*Open vent", MENU_BNDF_OPEN);
          if(show_open_boundary==0)glutAddMenuEntry("Open vent", MENU_BNDF_OPEN);
        }
      }

      nloadpatchsubmenus=0;

      CREATEMENU(unloadpatchmenu,UnloadBoundaryMenu);
      for(ii=0;ii<npatchinfo;ii++){
        patchdata *patchi;
        char menulabel[1024];

        i = patchorderindex[ii];
        patchi = patchinfo + i;
        if(patchi->loaded==1){
          if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label, "INCLUDE_GEOM")!=0){
            STRCPY(menulabel, patchi->menulabel);
            glutAddMenuEntry(menulabel, i);
            npatchloaded2++;
          }
        }
      }
      glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);

      if(nmeshes>1&&loadpatchsubmenus==NULL){
        NewMemory((void **)&loadpatchsubmenus,npatchinfo*sizeof(int));
      }

      if(nmeshes>1){
        CREATEMENU(loadpatchsubmenus[nloadpatchsubmenus],LoadBoundaryMenu);
        nloadpatchsubmenus++;
      }
      else{
        CREATEMENU(loadpatchmenu,LoadBoundaryMenu);
      }

      for(ii=0;ii<npatchinfo;ii++){
        patchdata *patchim1, *patchi;
        char menulabel[1024];

        i = patchorderindex[ii];
        patchi = patchinfo + i;
        if(ii>0){
          patchim1 = patchinfo + patchorderindex[ii-1];
          if(nmeshes>1&&strcmp(patchim1->label.longlabel,patchi->label.longlabel)!=0){
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

//*** these same lines also appear below
      glutAddMenuEntry("-",MENU_DUMMY3);

#ifdef pp_SHOW_BOUND_MIRROR
      if(nmeshes>1||n_mirrorvents>0||n_openvents>0){
        GLUTADDSUBMENU(_("Include"),includepatchmenu);
      }
#endif
      glutAddMenuEntry(_("Update bounds"),MENU_UPDATEBOUNDS);
      if(nboundaryslicedups>0){
        GLUTADDSUBMENU(_("Duplicate boundary slices"),duplicateboundaryslicemenu);
      }
      glutAddMenuEntry(_("Settings..."), MENU_BOUNDARY_SETTINGS);
      if(npatchloaded2>1){
        GLUTADDSUBMENU(_("Unload"),unloadpatchmenu);
      }
      else{
       glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
      }
      if(nmeshes>1){
        char menulabel[1024];

        CREATEMENU(loadpatchsinglemenu, LoadBoundaryMenu);
        for(ii=0;ii<npatchinfo;ii++){
          patchdata *patch1, *patch2;

          i = patchorderindex[ii];
          patch2 = patchinfo + i;
          if(ii==0){
            nloadpatchsubmenus=0;
            strcpy(menulabel,patch2->label.longlabel);
            GLUTADDSUBMENU(menulabel,loadpatchsubmenus[nloadpatchsubmenus]);
            nloadpatchsubmenus++;
          }
          else{
            patch1 = patchinfo + patchorderindex[ii-1];
            if(strcmp(patch1->label.longlabel,patch2->label.longlabel)!=0){
              strcpy(menulabel,patch2->label.longlabel);
              GLUTADDSUBMENU(menulabel,loadpatchsubmenus[nloadpatchsubmenus]);
              nloadpatchsubmenus++;
            }
          }
        }
      }

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
      if(nmeshes>1){

// count patch submenus

        nloadsubpatchmenu_b=0;
        for(ii=0;ii<npatchinfo;ii++){
          int im1;
          patchdata *patchi, *patchim1;

          i = patchorderindex[ii];
          if(ii>0){
            im1 = patchorderindex[ii-1];
            patchim1=patchinfo + im1;
          }
          patchi = patchinfo + i;
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
        for(ii=0;ii<npatchinfo;ii++){
          int im1;
          patchdata *patchi, *patchim1;

          i = patchorderindex[ii];
          if(ii>0){
            im1 = patchorderindex[ii-1];
            patchim1=patchinfo + im1;
          }
          patchi = patchinfo + i;
          if(ii==0||strcmp(patchi->menulabel_base,patchim1->menulabel_base)!=0){
            CREATEMENU(loadsubpatchmenu_b[iloadsubpatchmenu_b],LoadBoundaryMenu);
            iloadsubpatchmenu_b++;
          }
          if(ii==0||strcmp(patchi->menulabel_suffix,patchim1->menulabel_suffix)!=0){
            if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label,"INCLUDE_GEOM")!=0){
              nsubpatchmenus_b[iloadsubpatchmenu_b-1]++;
              glutAddMenuEntry(patchi->menulabel_suffix,-i-10);
            }
          }
        }

// call patch submenus from main patch menu

        CREATEMENU(loadpatchmenu,LoadBoundaryMenu);
        iloadsubpatchmenu_b=0;
        for(ii=0;ii<npatchinfo;ii++){
          int im1;
          patchdata *patchi, *patchim1;

          i = patchorderindex[ii];
          if(ii>0){
            im1 = patchorderindex[ii-1];
            patchim1=patchinfo + im1;
          }
          patchi = patchinfo + i;
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
              glutAddMenuEntry(menulabel,-i-10);
            }
            iloadsubpatchmenu_b++;
          }
        }
      }
//*** these same lines also appear above (except for nmeshes>1 line)
      glutAddMenuEntry("-",MENU_DUMMY3);
#ifdef pp_SHOW_BOUND_MIRROR
      if(nmeshes>1||n_mirrorvents>0||n_openvents>0){
        GLUTADDSUBMENU(_("Include"),includepatchmenu);
      }
#endif

      glutAddMenuEntry(_("Update bounds"),MENU_UPDATEBOUNDS);
      if(nboundaryslicedups>0){
        GLUTADDSUBMENU(_("Duplicate boundary slices"),duplicateboundaryslicemenu);
      }
      if(nmeshes>1)GLUTADDSUBMENU(_("Mesh"), loadpatchsinglemenu);
      glutAddMenuEntry(_("Settings..."), MENU_BOUNDARY_SETTINGS);
      if(npatchloaded2>1){
        GLUTADDSUBMENU(_("Unload"),unloadpatchmenu);
      }
      else{
       glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
      }
    }

/* --------------------------------load iso menu -------------------------- */

    if(nisoinfo>0){
      int ii;

      CREATEMENU(unloadisomenu,UnloadIsoMenu);
      for(ii=0;ii<nisoinfo;ii++){
        isodata *isoi;
        char menulabel[1024];

        i = isoorderindex[ii];
        isoi = isoinfo + i;
        if(isoi->loaded==0)continue;
        STRCPY(menulabel,isoi->menulabel);
        glutAddMenuEntry(menulabel,i);
      }
      glutAddMenuEntry(_("Unload all"),UNLOAD_ALL);

      if(nisoinfo>0){
        if(isosubmenus==NULL){
          NewMemory((void **)&isosubmenus,nisoinfo*sizeof(int));
        }
        nisosubmenus=0;

        CREATEMENU(isosubmenus[nisosubmenus],LoadIsoMenu);
        nisosubmenus++;
      }

      if(nmeshes==1){
        CREATEMENU(loadisomenu,LoadIsoMenu);
      }
      for(ii=0;ii<nisoinfo;ii++){
        isodata *iso1, *iso2;
        char menulabel[1024];

        i = isoorderindex[ii];
        if(ii>0){
          iso1 = isoinfo + isoorderindex[ii-1];
          iso2 = isoinfo + isoorderindex[ii];
          if(nmeshes>1&&strcmp(iso1->surface_label.longlabel,iso2->surface_label.longlabel)!=0){
            CREATEMENU(isosubmenus[nisosubmenus],LoadIsoMenu);
            nisosubmenus++;
          }
        }
        if(isoinfo[i].loaded==1){
          STRCPY(menulabel,"*");
          STRCAT(menulabel,isoinfo[i].menulabel);
        }
        else{
          STRCPY(menulabel,isoinfo[i].menulabel);
        }
        glutAddMenuEntry(menulabel,i);
      }

      {
        int useitem;
        isodata *isoi, *isoj;

        if(nmeshes>1){
          CREATEMENU(isosinglemeshmenu,LoadIsoMenu);
          for(ii=0;ii<nisoinfo;ii++){
            isodata *iso1, *iso2;
            char menulabel[1024];

            i = isoorderindex[ii];
            iso1 = isoinfo + i;
            if(ii==0){
              nisosubmenus=0;
              strcpy(menulabel,iso1->surface_label.longlabel);
              GLUTADDSUBMENU(menulabel,isosubmenus[nisosubmenus]);
              nisosubmenus++;
            }
            else{
              iso2 = isoinfo + isoorderindex[ii-1];
              if(strcmp(iso1->surface_label.longlabel,iso2->surface_label.longlabel)!=0){
                strcpy(menulabel,iso1->surface_label.longlabel);
                GLUTADDSUBMENU(menulabel,isosubmenus[nisosubmenus]);
                nisosubmenus++;
              }
            }
          }
          CREATEMENU(loadisomenu,LoadIsoMenu);
          for(i=0;i<nisoinfo;i++){
            int j;

            useitem=i;
            isoi = isoinfo + i;
            for(j=0;j<i;j++){
              isoj = isoinfo + j;
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
          GLUTADDSUBMENU(_("Mesh"), isosinglemeshmenu);
        }
      }
      glutAddMenuEntry(_("Settings..."), MENU_ISO_SETTINGS);
      if(nisoloaded>1){
        GLUTADDSUBMENU(_("Unload"),unloadisomenu);
      }
      else{
       glutAddMenuEntry(_("Unload"),UNLOAD_ALL);
      }
    }

/* --------------------------------zone menu -------------------------- */

    if(nzoneinfo>0){
      CREATEMENU(zonemenu,ZoneMenu);
      for(i=0;i<nzoneinfo;i++){
        zonedata *zonei;
        char menulabel[1024];
        int n;

        zonei = zoneinfo + i;
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
    if(smokezippath != NULL && (npatchinfo > 0 || nsmoke3dinfo > 0 || nsliceinfo > 0)){
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
      if(caseini_filename!=NULL&&FILE_EXISTS(caseini_filename)==YES){
        glutAddMenuEntry(caseini_filename,MENU_READCASEINI);
      }
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
    if( n_inifiles>0||FILE_EXISTS(smokeviewini_filename)==YES||FILE_EXISTS(caseini_filename)==YES||FILE_EXISTS(smokeviewini)==YES){
      if(n_inifiles==0){
        glutAddMenuEntry(_("Read ini files"),MENU_READINI);
      }
      else{
        GLUTADDSUBMENU(_("Read ini files"),inisubmenu);
      }
    }
  }

    glutAddMenuEntry("Write smokeview.ini",MENU_WRITEINI);

    {
      char caselabel[255];

      STRCPY(caselabel,_("Write"));
      STRCAT(caselabel," ");
      STRCAT(caselabel,caseini_filename);

      glutAddMenuEntry(caselabel,MENU_WRITECASEINI);
    }

    if(ndeviceinfo>0){
      glutAddMenuEntry("-",MENU_DUMMY);
      glutAddMenuEntry(_("Read .svo files"),MENU_READSVO);
    }
    glutAddMenuEntry(_("Settings..."), MENU_CONFIG_SETTINGS);

    CREATEMENU(reloadmenu,ReloadMenu);
    glutAddMenuEntry(_("smv"), RELOAD_SMV_FILE);
    if(load_incremental==1){
      glutAddMenuEntry(_("*New data"), RELOAD_MODE_INCREMENTAL);
      glutAddMenuEntry(_("All data"), RELOAD_MODE_ALL);
    }
    if(load_incremental==0){
      glutAddMenuEntry(_("New data"), RELOAD_MODE_INCREMENTAL);
      glutAddMenuEntry(_("*All data"), RELOAD_MODE_ALL);
    }
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
#ifdef pp_LUA
    {
      int nluascripts;

      nluascripts=0;
      // For Lua, the list of scripts is simply a list of filenames in the
      // directory with the right extension.
      luascriptfiledata *luascriptfile;
      STRUCTSTAT luastatbuffer;

      for(luascriptfile=first_luascriptfile.next;luascriptfile->next!=NULL;luascriptfile=luascriptfile->next){
        char *file;
        int len;

        file=luascriptfile->file;
        if(file==NULL)continue;
        len = strlen(file);
        if(len<=0)continue;
        if(STAT(file,&luastatbuffer)!=0)continue;

        nluascripts++;
      }
      if(nluascripts>0){
        CREATEMENU(luascriptlistmenu,LuaScriptMenu);
        for(luascriptfile=first_luascriptfile.next;luascriptfile->next!=NULL;luascriptfile=luascriptfile->next){
          char *file;
          int len;
          char menulabel[1024];

          file=luascriptfile->file;
          if(file==NULL)continue;
          len = strlen(file);
          if(len<=0)continue;
          if(STAT(file,&luastatbuffer)!=0)continue;

          strcpy(menulabel,"  ");
          strcat(menulabel,file);
          glutAddMenuEntry(menulabel,luascriptfile->id);
        }
      }
      CREATEMENU(luascriptmenu,LuaScriptMenu);
      if(nluascripts>0){
        GLUTADDSUBMENU(_("Run"),luascriptlistmenu);
      }
    }
#endif

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

      if(nsmoke3dinfo>0){
        strcpy(loadmenulabel,_("3D smoke"));
        if(smoke3dframeskip>0){
          sprintf(steplabel,"/Skip %i",smoke3dframeskip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadsmoke3dmenu);
      }

      // volume rendered smoke

      if(nvolrenderinfo>0&&smokediff==0){
        char vlabel[256];

        strcpy(vlabel,_("3D smoke (Volume rendered)"));
        GLUTADDSUBMENU(vlabel,loadvolsmoke3dmenu);
      }

      // terrain

      if(manual_terrain==1&&nterraininfo>0){
 //       GLUTADDSUBMENU(_("Terrain"),loadterrainmenu);
      }

      // slice

      if((nmultisliceinfo > 0 && nmultisliceinfo + nfedinfo < nsliceinfo)||have_geom_slice_menus==1){
        strcpy(loadmenulabel, _("Slice"));
        if(sliceframeskip > 0){
          sprintf(steplabel, "/Skip %i", sliceframeskip);
          strcat(loadmenulabel, steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel, loadmultislicemenu);
      }
      else if(nsliceinfo > 0){
        strcpy(loadmenulabel, "Slice");
        if(sliceframeskip > 0){
          sprintf(steplabel, "/Skip %i", sliceframeskip);
          strcat(loadmenulabel, steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel, loadslicemenu);
      }

      // vector slice

      if(nvsliceinfo > 0 && nmultivsliceinfo < nvsliceinfo){
        strcpy(loadmenulabel,_("Vector Slice"));
        if(sliceframeskip>0){
          sprintf(steplabel,"/Skip %i",sliceframeskip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadmultivslicemenu);
      }
      else if(nvsliceinfo>0){
        strcpy(loadmenulabel,_("Vector slice"));
        if(sliceframestep>1){
          sprintf(steplabel,"/Skip %i",sliceframeskip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,vsliceloadmenu);
      }

      // isosurface

      if(nisoinfo>0){
        strcpy(loadmenulabel,"Isosurface");
        if(isoframeskip_global>0){
          sprintf(steplabel,"/Skip %i",isoframeskip_global);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadisomenu);
      }

      // boundary

      if(npatchinfo>0){
        strcpy(loadmenulabel,"Boundary");
        if(boundframeskip>0){
          sprintf(steplabel,"/Skip %i",boundframeskip);
          strcat(loadmenulabel,steplabel);
        }
        GLUTADDSUBMENU(loadmenulabel,loadpatchmenu);
      }

      // particle

      if(npartinfo>0){
        if(nevac!=npartinfo){
          strcpy(loadmenulabel,"Particles");
          if(partframeskip>0){
            sprintf(steplabel,"/Skip Frame %i",partframeskip);
            strcat(loadmenulabel,steplabel);
          }
          GLUTADDSUBMENU(loadmenulabel,particlemenu);
        }
        if(nevac>0){
          strcpy(loadmenulabel,_("Evacuation"));
          if(partframeskip>0){
            sprintf(steplabel,"/Skip Frame %i",partframeskip);
            strcat(loadmenulabel,steplabel);
          }
          GLUTADDSUBMENU(loadmenulabel,evacmenu);
        }
      }

      // plot3d

      if(nplot3dinfo>0)GLUTADDSUBMENU(_("Plot3D"),loadplot3dmenu);

      // zone fire

      if(nzoneinfo>0){
        strcpy(loadmenulabel,"Zone fire");
        GLUTADDSUBMENU(loadmenulabel,zonemenu);
      }
      if(glui_active==1){
        glutAddMenuEntry("-",MENU_DUMMY);
      }
      GLUTADDSUBMENU(_("Configuration files"),smokeviewinimenu);
      GLUTADDSUBMENU(_("Scripts"),scriptmenu);
#ifdef pp_LUA
      GLUTADDSUBMENU("Lua Scripts",luascriptmenu);
#endif
#ifdef pp_COMPRESS
      if(smokezippath!=NULL&&(npatchinfo>0||nsmoke3dinfo>0||nsliceinfo>0)){
        GLUTADDSUBMENU(_("Compression"),compressmenu);
      }
#endif
      GLUTADDSUBMENU(_("Misc"),fileinfomenu);
      {
        char menulabel[1024];

        strcpy(menulabel,"");
        if(redirect==1)strcat(menulabel,"*");
        strcat(menulabel,"Redirect messages to ");
        strcat(menulabel,log_filename);
        glutAddMenuEntry(menulabel,REDIRECT);
      }

      GLUTADDSUBMENU(_("Reload"),reloadmenu);
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
    updatemenu=0;
#ifdef _DEBUG
  in_menu=0;
  PRINTF("nmenus=%i\n",nmenus);
#endif

}

/* ------------------ MenuStatusCB ------------------------ */

void MenuStatusCB(int status, int x, int y){
  float *eye_xyz;

  menustatus=status;
  if(menustatus==GLUT_MENU_IN_USE &&  show_geom_boundingbox==SHOW_BOUNDING_BOX_MOUSE_DOWN){
    geom_bounding_box_mousedown = 1;
  }
  else{
    geom_bounding_box_mousedown = 0;
  }

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
