#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "glew.h"

#include "string_util.h"
#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "IOobjects.h"
#include "getdata.h"

/* ------------------ GetZoneSizeCSV ------------------------ */

void GetZoneSizeCSV(int *nzone_times_local, int *nroom, int *nfires_local, int *nhvents, int *nvvents, int *nmvents, int *ntargets_arg, int *error){
   devicedata *dev;
   int nr,nf,nv,nt;
   int i;

   *error=0;

   nt = 0;
   for(i=0;;i++){
     char label[100];

     sprintf(label, "TARGET_%i", i+1);
     if(GetCSVDeviceFromLabel(label, -1)==NULL)break;
     nt++;
   }
   *ntargets_arg = nt;

   nr=0;
   for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
     char label[100];

     sprintf(label,"ULT_%i",i+1);
     dev=GetDeviceFromLabel(label,-1);
     if(dev==NULL)break;
     *nzone_times_local=dev->nvals;
     nr++;
   }
   *nroom=nr;

   nv=0;
   for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
     char label[100];

     sprintf(label,"HVENT_%i",i+1);
     dev=GetDeviceFromLabel(label,-1);
     if(dev==NULL)break;
     nv++;
   }
   *nhvents=nv;

   nv=0;
   for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
     char label[100];

     sprintf(label,"VVENT_%i",i+1);
     dev=GetDeviceFromLabel(label,-1);
     if(dev==NULL)break;
     nv++;
   }
   *nvvents=nv;

   nv=0;
   for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
     char label[100];

     sprintf(label,"MVENT_%i",i+1);
     dev=GetDeviceFromLabel(label,-1);
     if(dev==NULL)break;
     nv++;
   }
   *nmvents=nv;

   nf=0;
   for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
     char label[100];

     sprintf(label,"HRR_%i",i+1);
     dev=GetDeviceFromLabel(label,-1);
     if(dev==NULL)break;
     nf++;
   }
   *nfires_local=nf;
}

#define FREEZONEMEM \
  FREEMEMORY(zonevents_devs); \
  FREEMEMORY(zoneslab_n_devs); \
  FREEMEMORY(zoneslab_T_devs); \
  FREEMEMORY(zoneslab_F_devs); \
  FREEMEMORY(zoneslab_YB_devs); \
  FREEMEMORY(zoneslab_YT_devs); \
  FREEMEMORY(zoneqfire_devs); \
  FREEMEMORY(zonepr_devs); \
  FREEMEMORY(zoneylay_devs); \
  FREEMEMORY(zonetl_devs); \
  FREEMEMORY(zonetu_devs); \
  FREEMEMORY(zonerhol_devs); \
  FREEMEMORY(zonerhou_devs); \
  FREEMEMORY(zoneodl_devs); \
  FREEMEMORY(zoneodu_devs); \
  FREEMEMORY(zonefheight_devs); \
  FREEMEMORY(zonefarea_devs); \
  FREEMEMORY(zonefbase_devs); \
  FREEMEMORY(zonefl_devs); \
  FREEMEMORY(zonelw_devs); \
  FREEMEMORY(zoneuw_devs); \
  FREEMEMORY(zonecl_devs)

#define GETZONEDEV(zonedev)\
      zonedev = GetDeviceFromLabel(label, -1);\
      if(zonedev == NULL || zonedev->nvals != nzone_times_local){\
        *error=1;\
        FREEZONEMEM;\
        return;\
      }\
      zonedev->in_zone_csv = 1

/* ------------------ GetZoneDataCSV ------------------------ */

void GetZoneDataCSV(int nzone_times_local, int nrooms_local, int nfires_local, int ntargets_local,
                    float *zone_times_local, float *zoneqfire_local, float *zonefheight_local, float *zonefbase_local, float *zonefdiam_local,
                    float *zonepr_local, float *zoneylay_local,  float *zonetl_local, float *zonetu_local, float *zonerhol_local, float *zonerhou_local,
                    float **zoneodlptr, float **zoneoduptr, float *zonevents_local,
                    int *zoneslab_n_local, float *zoneslab_T_local, float *zoneslab_F_local, float *zoneslab_YB_local, float *zoneslab_YT_local,
                    float *zonefl_local, float *zonelw_local, float *zoneuw_local, float *zonecl_local,
                    float *zonetargets_local,
                    int *error){
  int i, iir, iit, iif, use_od = 1, iihv;
  devicedata **zoneqfire_devs=NULL;
  devicedata **zonepr_devs=NULL, **zoneylay_devs=NULL, **zonetl_devs=NULL, **zonetu_devs=NULL, **zoneodl_devs=NULL, **zoneodu_devs=NULL;
  devicedata **zonefl_devs = NULL, **zonelw_devs = NULL, **zoneuw_devs = NULL, **zonecl_devs = NULL;
  devicedata **zonerhol_devs = NULL, **zonerhou_devs = NULL;
  devicedata **zonefheight_devs=NULL, **zonefbase_devs=NULL, **zonefarea_devs=NULL;
  devicedata **zonevents_devs=NULL;
  devicedata **zoneslab_n_devs = NULL, **zoneslab_T_devs = NULL, **zoneslab_F_devs = NULL, **zoneslab_YB_devs = NULL, **zoneslab_YT_devs = NULL;
  devicedata **zonetarget_devs = NULL;
  float *zoneodl_local, *zoneodu_local;
  float *times_local;

  *error=0;
  if(nfires_local>0){
    NewMemory((void **)&zoneqfire_devs,   nfires_local*sizeof(devicedata *));
    NewMemory((void **)&zonefheight_devs, nfires_local*sizeof(devicedata *));
    NewMemory((void **)&zonefbase_devs,   nfires_local*sizeof(devicedata *));
    NewMemory((void **)&zonefarea_devs,   nfires_local*sizeof(devicedata *));
  }

  if(nrooms_local>0){
    NewMemory((void **)&zonepr_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zoneylay_devs, nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonetl_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonetu_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonefl_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonelw_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zoneuw_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonecl_devs,   nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonerhol_devs, nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zonerhou_devs, nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zoneodl_devs,  nrooms_local*sizeof(devicedata *));
    NewMemory((void **)&zoneodu_devs,  nrooms_local*sizeof(devicedata *));
  }

  if(global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents>0){
    int ntotalvents;

    ntotalvents = global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents;
    NewMemory((void **)&zonevents_devs,   ntotalvents*sizeof(devicedata *));
    NewMemory((void **)&zoneslab_n_devs,  ntotalvents*sizeof(devicedata *));
    NewMemory((void **)&zoneslab_T_devs,  MAX_HSLABS*ntotalvents*sizeof(devicedata *));
    NewMemory((void **)&zoneslab_F_devs,  MAX_HSLABS*ntotalvents*sizeof(devicedata *));
    NewMemory((void **)&zoneslab_YB_devs, MAX_HSLABS*ntotalvents*sizeof(devicedata *));
    NewMemory((void **)&zoneslab_YT_devs, MAX_HSLABS*ntotalvents*sizeof(devicedata *));
  }

  have_zonefl = 1;
  have_zonelw = 1;
  have_zoneuw = 1;
  have_zonecl = 1;
  have_wall_data = 1;

  if(ntargets_local>0){
    have_target_data = 1;
    NewMemory((void **)&zonetarget_devs, ntargets_local*sizeof(devicedata *));
    for(i = 0; i<ntargets_local; i++){
      char label[100];

      sprintf(label, "TARGET_%i", i+1);
      zonetarget_devs[i] = GetCSVDeviceFromLabel(label, -1);
      if(zonetarget_devs[i]==NULL||zonetarget_devs[i]->nvals!=nzone_times_local){
        zonetarget_devs[i] = NULL;
        have_target_data = 0;
      }
      else{
        zonetarget_devs[i]->in_zone_csv = 1;
        zonetarget_devs[i]->target_index = i;
      }
    }
  }
  for(i=0;i<nrooms_local;i++){
    char label[100];

    sprintf(label,"PRS_%i",i+1);
    zonepr_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonepr_devs[i]==NULL||zonepr_devs[i]->nvals!=nzone_times_local){
      *error = 1;
      FREEZONEMEM;
      return;
    }
    zonepr_devs[i]->in_zone_csv=1;

    sprintf(label,"HGT_%i",i+1);
    zoneylay_devs[i]=GetDeviceFromLabel(label,-1);
    if(zoneylay_devs[i]==NULL||zoneylay_devs[i]->nvals!=nzone_times_local){
      zoneylay_devs[i]=NULL;
    }
    else{
      zoneylay_devs[i]->in_zone_csv=1;
    }

    sprintf(label,"FLT_%i",i+1);
    zonefl_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonefl_devs[i]==NULL||zonefl_devs[i]->nvals!=nzone_times_local){
      zonefl_devs[i]=NULL;
      have_zonefl = 0;
    }
    else{
      zonefl_devs[i]->in_zone_csv=1;
    }

    sprintf(label, "LWT_%i", i+1);
    zonelw_devs[i] = GetDeviceFromLabel(label, -1);
    if(zonelw_devs[i]==NULL||zonelw_devs[i]->nvals!=nzone_times_local){
      zonelw_devs[i] = NULL;
      have_zonelw = 0;
    }
    else{
      zonelw_devs[i]->in_zone_csv = 1;
    }

    sprintf(label, "UWT_%i", i+1);
    zoneuw_devs[i] = GetDeviceFromLabel(label, -1);
    if(zoneuw_devs[i]==NULL||zoneuw_devs[i]->nvals!=nzone_times_local){
      zoneuw_devs[i] = NULL;
      have_zoneuw = 0;
    }
    else{
      zoneuw_devs[i]->in_zone_csv = 1;
    }

    sprintf(label, "CLT_%i", i+1);
    zonecl_devs[i] = GetDeviceFromLabel(label, -1);
    if(zonecl_devs[i]==NULL||zonecl_devs[i]->nvals!=nzone_times_local){
      zonecl_devs[i] = NULL;
      have_zonecl = 0;
    }
    else{
      zonecl_devs[i]->in_zone_csv = 1;
    }

    sprintf(label,"LLT_%i",i+1);
    zonetl_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonetl_devs[i]==NULL||zonetl_devs[i]->nvals!=nzone_times_local){
      zonetl_devs[i]=NULL;
    }
    else{
      zonetl_devs[i]->in_zone_csv=1;
    }

    sprintf(label,"ULT_%i",i+1);
    zonetu_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonetu_devs[i]==NULL||zonetu_devs[i]->nvals!=nzone_times_local){
      *error = 1;
      FREEZONEMEM;
      return;
    }
    else{
      zonetu_devs[i]->in_zone_csv=1;
    }

    sprintf(label, "RHOL_%i", i+1);
    zonerhol_devs[i] = GetDeviceFromLabel(label, -1);
    if(zonerhol_devs[i]==NULL||zonerhol_devs[i]->nvals!=nzone_times_local){
      zonerhol_devs[i] = NULL;
      zone_rho=0;
    }
    else{
      zonerhol_devs[i]->in_zone_csv = 1;
    }

    sprintf(label, "RHOU_%i", i+1);
    zonerhou_devs[i] = GetDeviceFromLabel(label, -1);
    if(zonerhou_devs[i]==NULL||zonerhou_devs[i]->nvals!=nzone_times_local){
      zonerhou_devs[i] = NULL;
      zone_rho=0;
    }
    else{
      zonerhou_devs[i]->in_zone_csv = 1;
    }

    sprintf(label,"ULOD_%i",i+1);
    zoneodu_devs[i]=GetDeviceFromLabel(label,-1);
    if(zoneodu_devs[i]==NULL){
      use_od=0;
    }
    else{
      zoneodu_devs[i]->in_zone_csv=1;
    }

    sprintf(label,"LLOD_%i",i+1);
    zoneodl_devs[i]=GetDeviceFromLabel(label,-1);
    if(zoneodl_devs[i]==NULL)zoneodl_devs[i]=zoneodu_devs[i];
    if(zoneodl_devs[i]==NULL){
      use_od=0;
    }
    else{
      zoneodl_devs[i]->in_zone_csv=1;
    }
  }
  if(have_zonefl==0&&have_zonelw==0&&have_zoneuw==0&&have_zonecl==0)have_wall_data = 0;
  if(use_od==1){
    NewMemory((void **)&zoneodl_local     ,nrooms_local*nzone_times_local*sizeof(float));
    NewMemory((void **)&zoneodu_local     ,nrooms_local*nzone_times_local*sizeof(float));
  }
  else{
    zoneodl_local=NULL;
    zoneodu_local=NULL;
  }
  *zoneodlptr=zoneodl_local;
  *zoneoduptr=zoneodu_local;

  for(i=0;i<nfires_local;i++){
    char label[100];

    sprintf(label,"HRR_%i",i+1);
    zoneqfire_devs[i]=GetDeviceFromLabel(label,-1);
    if(zoneqfire_devs[i]==NULL||zoneqfire_devs[i]->nvals!=nzone_times_local){
      *error=1;
      FREEZONEMEM;
      return;
    }
    zoneqfire_devs[i]->in_zone_csv=1;

    sprintf(label,"FLHGT_%i",i+1);
    zonefheight_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonefheight_devs[i]==NULL||zonefheight_devs[i]->nvals!=nzone_times_local){
      *error=1;
      FREEZONEMEM;
      return;
    }
    zonefheight_devs[i]->in_zone_csv=1;

    sprintf(label,"FBASE_%i",i+1);
    zonefbase_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonefbase_devs[i]==NULL||zonefbase_devs[i]->nvals!=nzone_times_local){
      *error=1;
      FREEZONEMEM;
      return;
    }
    zonefbase_devs[i]->in_zone_csv=1;

    sprintf(label,"FAREA_%i",i+1);
    zonefarea_devs[i]=GetDeviceFromLabel(label,-1);
    if(zonefarea_devs[i]==NULL||zonefarea_devs[i]->nvals!=nzone_times_local){
      *error=1;
      FREEZONEMEM;
      return;
    }
    zonefarea_devs[i]->in_zone_csv=1;
  }

  for(i = 0; i < global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents; i++){
    char label[100],vent_type[10];
    int vent_index;

    if(i<global_scase.nzhvents){
      strcpy(vent_type,"HVENT");
      vent_index=i+1;
    }
    else if(i>=global_scase.nzhvents&&i<global_scase.nzhvents+global_scase.nzvvents){
      strcpy(vent_type,"VVENT");
      vent_index=i+1-global_scase.nzhvents;
    }
    else{
      strcpy(vent_type,"MVENT");
      vent_index=i+1-global_scase.nzhvents-global_scase.nzvvents;
    }
    sprintf(label, "%s_%i", vent_type,vent_index);
    zonevents_devs[i] = GetDeviceFromLabel(label, -1);
    if(zonevents_devs[i] == NULL || zonevents_devs[i]->nvals != nzone_times_local){
      *error=1;
      FREEZONEMEM;
      return;
    }
    zonevents_devs[i]->in_zone_csv = 1;
  }

//  setup devices that describe VENTS
  have_ventslab_flow = 0;
  for(i = 0; i < global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents; i++){
    char label[100], vent_type[10];
    int islab, vent_index, max_slabs;

    if(i<global_scase.nzhvents){
      vent_index = 1+i;
      strcpy(vent_type, "HSLAB");
      max_slabs = MAX_HSLABS;
    }
    else if(i>=global_scase.nzhvents&&i<global_scase.nzhvents+global_scase.nzvvents){
      vent_index = 1+i-global_scase.nzhvents;
      strcpy(vent_type, "VSLAB");
      max_slabs = MAX_VSLABS;
    }
    else{
      vent_index = 1+i-global_scase.nzhvents-global_scase.nzvvents;
      strcpy(vent_type, "MSLAB");
      max_slabs = MAX_MSLABS;
    }
    sprintf(label, "%s_%i", vent_type, vent_index);
    zoneslab_n_devs[i] = GetDeviceFromLabel(label, -1);
    if(zoneslab_n_devs[i] != NULL){
      have_ventslab_flow = 1;
      break;
    }
    for(islab = 0; islab < max_slabs; islab++){
      int idev;

      idev = MAX_HSLABS * i + islab;
      sprintf(label, "%s_%i_%i", vent_type, vent_index, islab+1);
      zoneslab_T_devs[idev] = GetDeviceFromLabel(label, -1);
      if(zoneslab_T_devs[idev] != NULL)have_ventslab_flow = 1;

      sprintf(label, "%sF_%i_%i", vent_type, vent_index, islab+1);
      zoneslab_F_devs[idev] = GetDeviceFromLabel(label, -1);
      if(zoneslab_T_devs[idev] != NULL)have_ventslab_flow = 1;

      sprintf(label, "%sYB_%i_%i", vent_type, vent_index, islab+1);
      zoneslab_YB_devs[idev] = GetDeviceFromLabel(label, -1);
      if(zoneslab_T_devs[idev] != NULL)have_ventslab_flow = 1;

      sprintf(label, "%sYT_%i_%i", vent_type, vent_index, islab+1);
      zoneslab_YT_devs[idev] = GetDeviceFromLabel(label, -1);
      if(zoneslab_T_devs[idev] != NULL)have_ventslab_flow = 1;
    }
    if(have_ventslab_flow == 1)break;
  }
  if(have_ventslab_flow == 1){
    for(i = 0; i < global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents; i++){
      char label[100], vent_type[10];
      int islab, vent_index, max_slabs;

      if(i<global_scase.nzhvents){
        vent_index = 1+i;
        strcpy(vent_type, "HSLAB");
        max_slabs = MAX_HSLABS;
      }
      else if(i>=global_scase.nzhvents&&i<global_scase.nzhvents+global_scase.nzvvents){
        vent_index = 1+i-global_scase.nzhvents;
        strcpy(vent_type, "VSLAB");
        max_slabs = MAX_VSLABS;
      }
      else{
        vent_index = 1+i-global_scase.nzhvents-global_scase.nzvvents;
        strcpy(vent_type, "MSLAB");
        max_slabs = MAX_MSLABS;
      }

      sprintf(label, "%s_%i", vent_type, vent_index);
      GETZONEDEV(zoneslab_n_devs[i]);
      for(islab = 0; islab < max_slabs; islab++){
        int idev;

        idev = MAX_HSLABS * i + islab;
        sprintf(label, "%sT_%i_%i", vent_type, vent_index, islab + 1);
        GETZONEDEV(zoneslab_T_devs[idev]);

        sprintf(label, "%sF_%i_%i", vent_type, vent_index, islab + 1);
        GETZONEDEV(zoneslab_F_devs[idev]);

        sprintf(label, "%sYB_%i_%i", vent_type, vent_index, islab + 1);
        GETZONEDEV(zoneslab_YB_devs[idev]);

        sprintf(label, "%sYT_%i_%i", vent_type, vent_index, islab + 1);
        GETZONEDEV(zoneslab_YT_devs[idev]);
      }
    }
  }

  iir=0;
  iif=0;
  iihv=0;
  iit = 0;
  times_local = zonepr_devs[0]->times;

  maxslabflow = 0.0;
  for(i=0;i<nzone_times_local;i++){
    int j, ivent;

    zone_times_local[i]=times_local[i];
    for(j = 0; j<ntargets_local; j++){
      if(zonetarget_devs[j]!=NULL){
        zonetargets_local[iit] = zonetarget_devs[j]->vals[i];
      }
      iit++;
    }
    for(j=0;j<nrooms_local;j++){
      zonepr_local[iir]=zonepr_devs[j]->vals[i];
      if(zoneylay_devs[j]==NULL){
        zoneylay_local[iir]=0.0;
      }
      else{
        zoneylay_local[iir]=zoneylay_devs[j]->vals[i];
      }
      zonetu_local[iir]=zonetu_devs[j]->vals[i];
      if(zonetl_devs[j]!=NULL){
        zonetl_local[iir]=zonetl_devs[j]->vals[i];
      }
      else{
        zonetl_local[iir]=zonetu_devs[j]->vals[i];
      }

      if(zonefl_devs[j]!=NULL){
        zonefl_local[iir] = zonefl_devs[j]->vals[i];
      }
      if(zonelw_devs[j]!=NULL){
        zonelw_local[iir] = zonelw_devs[j]->vals[i];
      }
      if(zoneuw_devs[j]!=NULL){
        zoneuw_local[iir] = zoneuw_devs[j]->vals[i];
      }
      if(zonecl_devs[j]!=NULL){
        zonecl_local[iir] = zonecl_devs[j]->vals[i];
      }

      if(zonerhou_devs[j]!=NULL){
        zonerhou_local[iir] = zonerhou_devs[j]->vals[i];
      }
      if(zonerhol_devs[j]!=NULL){
        zonerhol_local[iir] = zonerhol_devs[j]->vals[i];
      }

      if(use_od==1){
        zoneodu_local[iir]=zoneodu_devs[j]->vals[i];
        if(zoneodl_devs[j]!=NULL){
          zoneodl_local[iir]=zoneodl_devs[j]->vals[i];
        }
        else{
          zoneodl_local[iir]=zoneodu_devs[j]->vals[i];
        }
      }
      iir++;
    }
    for(j=0;j<nfires_local;j++){
      float area, diam;

      zoneqfire_local[iif]=1000.0*zoneqfire_devs[j]->vals[i];
      zonefheight_local[iif]=zonefheight_devs[j]->vals[i];
      area=zonefarea_devs[j]->vals[i];
      diam=2.0*sqrt(area/PI);
      if(diam<0.0001){
        diam=SCALE2SMV(0.1);
      }
      zonefdiam_local[iif]=diam;
      zonefbase_local[iif]=zonefbase_devs[j]->vals[i];
      iif++;
    }
    for(ivent=0;ivent<global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents;ivent++){
      int islab, max_slabs;

      zonevents_local[iihv] = zonevents_devs[ivent]->vals[i];
      if(ivent<global_scase.nzhvents){
        max_slabs = MAX_HSLABS;
      }
      else if(ivent>=global_scase.nzhvents&&i<global_scase.nzhvents+global_scase.nzvvents){
        max_slabs = MAX_VSLABS;
      }
      else{
        max_slabs = MAX_MSLABS;
      }
      if(zoneslab_n_devs[ivent]!=NULL)zoneslab_n_local[iihv] = (int)(zoneslab_n_devs[ivent]->vals[i]+0.1);
      for(islab = 0; islab<max_slabs; islab++){
        int idev, ival;

        idev = MAX_HSLABS * ivent + islab;
        ival = MAX_HSLABS * iihv + islab;
        if(zoneslab_T_devs[idev]!=NULL)zoneslab_T_local[ival] =  zoneslab_T_devs[idev]->vals[i];
        if(zoneslab_F_devs[idev] != NULL){
          float slabflow;

          slabflow = zoneslab_F_devs[idev]->vals[i];
          if(zoneslab_YB_devs[idev] != NULL&&zoneslab_YT_devs[idev] != NULL){
            float factor;

            factor = ABS(zoneslab_YT_devs[idev]->vals[i] - zoneslab_YB_devs[idev]->vals[i]);
            if(factor == 0.0)factor = 1.0;
            slabflow /= factor;
          }
          maxslabflow = MAX(ABS(slabflow), maxslabflow);
          zoneslab_F_local[ival] = slabflow;
        }
        if(zoneslab_YB_devs[idev]!=NULL)zoneslab_YB_local[ival] = zoneslab_YB_devs[idev]->vals[i];
        if(zoneslab_YT_devs[idev]!=NULL)zoneslab_YT_local[ival] = zoneslab_YT_devs[idev]->vals[i];
      }
      iihv++;
    }
  }
}

/* ------------------ FillZoneData ------------------------ */

void FillZoneData(int izone_index){
  float *pr0, *tl0, *tu0, *ylay0, *odl0, *odu0, *hvent0, *rhol0, *rhou0;
  int iroom, ivent;
  roomdata *roomi;
  int *zoneslab_n0;
  float *zoneslab_T0, *zoneslab_F0, *zoneslab_YB0, *zoneslab_YT0;
  int ntotal_vents;

  float gamma = 1.4;
  float CP = 1012.0;
  float R = (gamma - 1.0)*CP / gamma;

  if(ReadZoneFile == 0)return;
  pr0 = zonepr + izone_index*global_scase.nrooms;
  ylay0 = zoneylay + izone_index*global_scase.nrooms;
  tl0 = zonetl + izone_index*global_scase.nrooms;
  tu0 = zonetu + izone_index*global_scase.nrooms;
  if(zone_rho == 1){
    rhol0 = zonerhol + izone_index*global_scase.nrooms;
    rhou0 = zonerhou + izone_index*global_scase.nrooms;
  }
  ntotal_vents = global_scase.nzhvents + global_scase.nzvvents + global_scase.nzmvents;
  hvent0 = zonevents + izone_index*ntotal_vents;
  zoneslab_n0 = zoneslab_n + izone_index*ntotal_vents;
  zoneslab_T0 = zoneslab_T + izone_index*MAX_HSLABS*ntotal_vents;
  zoneslab_F0 = zoneslab_F + izone_index*MAX_HSLABS*ntotal_vents;
  zoneslab_YB0 = zoneslab_YB + izone_index*MAX_HSLABS*ntotal_vents;
  zoneslab_YT0 = zoneslab_YT + izone_index*MAX_HSLABS*ntotal_vents;
  if(zoneodl != NULL)odl0 = zoneodl + izone_index*global_scase.nrooms;
  if(zoneodu != NULL)odu0 = zoneodu + izone_index*global_scase.nrooms;
  for(ivent = 0;ivent < global_scase.nzhvents + global_scase.nzvvents + global_scase.nzmvents;ivent++){
    zventdata *zventi;
    int islab;

    zventi = global_scase.zventinfo + ivent;
    zventi->area_fraction = CLAMP(hvent0[ivent] / zventi->area, 0.0, 1.0);

    zventi->nslab = zoneslab_n0[ivent];
    for(islab = 0; islab < zventi->nslab; islab++){
      int iislab;

      iislab = MAX_HSLABS*ivent + islab;
      zventi->slab_bot[islab] = zoneslab_YB0[iislab];
      zventi->slab_top[islab] = zoneslab_YT0[iislab];
      zventi->slab_vel[islab] = zoneslab_F0[iislab];
      zventi->slab_temp[islab] = zoneslab_T0[iislab];
    }
  }
  for(iroom = 0;iroom < global_scase.nrooms;iroom++){
    roomi = global_scase.roominfo + iroom;
    roomi->pfloor = pr0[iroom];
    roomi->ylay = ylay0[iroom];
    roomi->tl = C2K(tl0[iroom]);
    roomi->tu = C2K(tu0[iroom]);
    roomi->itl = GetZoneColor(tl0[iroom], zonemin, zonemax, nrgb_full);
    roomi->itu = GetZoneColor(tu0[iroom], zonemin, zonemax, nrgb_full);
    if(zone_rho == 1){
      roomi->rho_L = rhol0[iroom];
      roomi->rho_U = rhou0[iroom];
    }
    else{
      roomi->rho_L = (global_scase.pref + pr0[iroom]) / R / roomi->tl;
      roomi->rho_U = (global_scase.pref + pr0[iroom]) / R / roomi->tu;
    }
    if(zoneodl != NULL)roomi->od_L = 1.0 / MAX(odl0[iroom], 0.0001);
    if(zoneodu != NULL)roomi->od_U = 1.0 / MAX(odu0[iroom], 0.0001);
  }
  roomi = global_scase.roominfo + global_scase.nrooms;
  roomi->pfloor = 0.0;
  roomi->ylay = 99999.0;
  roomi->tl = global_scase.tamb;
  roomi->tu = global_scase.tamb;
  roomi->itl = GetZoneColor(K2C(global_scase.tamb), zonemin, zonemax, nrgb_full);
  roomi->itu = GetZoneColor(K2C(global_scase.tamb), zonemin, zonemax, nrgb_full);
  roomi->rho_L = (global_scase.pref + global_scase.pamb) / R / roomi->tl;
  roomi->rho_U = (global_scase.pref + global_scase.pamb) / R / roomi->tu;
  roomi->z0 = 0.0;
  roomi->z1 = 100000.0;
}

/* ------------------ GetPressure ------------------------ */

float GetPressure(float y, float pfloor, float ylay, float rho_L, float rho_U){
  // y and ylay are relative coordinates
  float g = 9.80;
  float p;

  p = pfloor - rho_L*g*SCALE2FDS(MIN(y, ylay)) - rho_U*g*SCALE2FDS(MAX(y - ylay, 0.0));
  return p;
}

/* ------------------ GetZoneVentVel ------------------------ */

void GetZoneVentVel(float *yy, int n, roomdata *r1, roomdata *r2, float *vdata, float *vmin, float *vmax, int *iT){
  float p1, p2;
  int itslab;
  float fsign;
  int i;
  float y;
  float rho_slab;

  // yy - absolute coordinates
  // r1->ylay, r2->ylay - relative coordinates

  for(i = 0;i < n;i++){

    y = yy[i];

    if(y<r1->z0 || y<r2->z0 || y>r1->z1 || y>r2->z1){
      vdata[i] = 0.0;
      iT[i] = r1->itl;
      continue;
    }

    p1 = GetPressure(y - r1->z0, r1->pfloor, r1->ylay, r1->rho_L, r1->rho_U);
    p2 = GetPressure(y - r2->z0, r2->pfloor, r2->ylay, r2->rho_L, r2->rho_U);

    if(p1 > p2){
      fsign = 1.0;
      if(y - r1->z0 > r1->ylay){
        itslab = r1->itu;
        rho_slab = r1->rho_U;
      }
      else{
        itslab = r1->itl;
        rho_slab = r1->rho_L;
      }
    }
    else{
      fsign = -1.0;
      if(y - r2->z0 > r2->ylay){
        itslab = r2->itu;
        rho_slab = r2->rho_U;
      }
      else{
        itslab = r2->itl;
        rho_slab = r2->rho_L;
      }
    }
    vdata[i] = fsign*sqrt(2.0*ABS(p1 - p2) / rho_slab);
    iT[i] = itslab;
  }
  *vmin = vdata[0];
  *vmax = vdata[0];
  for(i = 1;i < n;i++){
    *vmin = MIN(*vmin, vdata[i]);
    *vmax = MAX(*vmax, vdata[i]);
  }
  return;
}

/* ------------------ GetZoneVentBounds ------------------------ */

void GetZoneVentBounds(void){
  int i;
#define VEL_MAX  100000000.0
#define VEL_MIN -100000000.0
  for(i = 0;i < global_scase.nzvents;i++){
    zventdata *zvi;

    zvi = global_scase.zventinfo + i;
    zvi->g_vmax = VEL_MIN;
    zvi->g_vmin = VEL_MAX;
  }
  for(izone = 0;izone < nzone_times;izone++){
    FillZoneData(izone);
    for(i = 0;i < global_scase.nzvents;i++){
      int j;
      zventdata *zvi;
      float zelev[NELEV_ZONE];

      zvi = global_scase.zventinfo + i;
      if(zvi->area_fraction < 0.0001)continue;
      if(zvi->vent_type == VFLOW_VENT || zvi->vent_type == MFLOW_VENT)continue;
      for(j = 0;j < NELEV_ZONE;j++){
        zelev[j] = (zvi->z0*(NELEV_ZONE - 1 - j) + zvi->z1*j) / (float)(NELEV_ZONE - 1);
      }
      GetZoneVentVel(zelev, NELEV_ZONE, zvi->room1, zvi->room2, zvi->vdata, &zvi->vmin, &zvi->vmax, zvi->itempdata);
      zvi->g_vmin = MIN(zvi->vmin, zvi->g_vmin);
      zvi->g_vmax = MAX(zvi->vmax, zvi->g_vmax);
    }
  }
  zone_maxventflow = 0.0;
  for(i = 0;i < global_scase.nzvents;i++){
    zventdata *zvi;

    zvi = global_scase.zventinfo + i;
    if(zvi->vent_type == VFLOW_VENT || zvi->vent_type == MFLOW_VENT)continue;
    if(ABS(zvi->g_vmin)<VEL_MAX-1.0)zone_maxventflow = MAX(ABS(zvi->g_vmin), zone_maxventflow);
    if(ABS(zvi->g_vmax)<VEL_MAX-1.0)zone_maxventflow = MAX(ABS(zvi->g_vmax), zone_maxventflow);
  }
}

/* ------------------ GetZoneGlobalBounds ------------------------ */

void GetZoneGlobalBounds(const float *pdata, int ndata, float *pglobalmin, float *pglobalmax, int flag){
  int n, nstart;
  float pmin2, pmax2, val;

  if(flag==FIRST_TIME){
    pmin2 = pdata[0];
    pmax2 = pmin2;
    nstart = 1;
  }
  else{
    pmin2 = *pglobalmin;
    pmax2 = *pglobalmax;
    nstart = 0;
  }
  for(n = nstart; n<ndata; n++){
    val = pdata[n];
    pmin2 = MIN(val, pmin2);
    pmax2 = MAX(val, pmax2);
  }
  *pglobalmin = pmin2;
  *pglobalmax = pmax2;
}

 /* ------------------ GetSliceTempBounds ------------------------ */

void GetSliceTempBounds(void){
  int i;

  for(i=0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;
    int framesize, headersize, return_val, error;
    int ntimes_slice_old=0;
    float qmin, qmax;

    slicei = global_scase.slicecoll.sliceinfo + i;
    if(strcmp(slicei->label.shortlabel, "TEMP")!=0)continue;
    GetSliceSizes(slicei->file, ALL_FRAMES, &slicei->nslicei, &slicei->nslicej, &slicei->nslicek, &slicei->ntimes, tload_step, &error,
                  use_tload_begin, use_tload_end, global_scase.tload_begin, global_scase.tload_end, &headersize, &framesize);
    return_val = NewResizeMemory(slicei->qslicedata, sizeof(float)*(slicei->nslicei+1)*(slicei->nslicej+1)*(slicei->nslicek+1)*slicei->ntimes);
    if(return_val!=0)return_val = NewResizeMemory(slicei->times, sizeof(float)*slicei->ntimes);
    qmin = 1.0e30;
    qmax = -1.0e30;
    GetSliceData(slicei, slicei->file, ALL_FRAMES, &slicei->is1, &slicei->is2, &slicei->js1, &slicei->js2, &slicei->ks1, &slicei->ks2, &slicei->idir,
      &qmin, &qmax, slicei->qslicedata, slicei->times, ntimes_slice_old, &slicei->ntimes,
      tload_step, use_tload_begin, use_tload_end, global_scase.tload_begin, global_scase.tload_end
    );
    slicei->globalmin_slice = qmin;
    slicei->globalmax_slice = qmax;
    slicei->valmin_slice    = qmin;
    slicei->valmax_slice    = qmax;
    FREEMEMORY(slicei->qslicedata);
    FREEMEMORY(slicei->times);
  }
  slice_temp_bounds_defined = 1;
}

/* ------------------ ReadZone ------------------------ */

void ReadZone(int ifile, int flag, int *errorcode){
  int error,ntotal_rooms,ntotal_targets,i,j,ii;
  int nrooms2,nfires2,nzhvents2,nzvvents2,nzmvents2=0,ntargets_local=0;
  zonedata *zonei;
  char *file;

  *errorcode=0;

  assert(ifile>=0&&ifile<global_scase.nzoneinfo);
  zonei = global_scase.zoneinfo + ifile;
  file = zonei->file;
  if(zonei->loaded==0&&flag==UNLOAD)return;
  FREEMEMORY(zonevents);
  FREEMEMORY(zone_times);
  FREEMEMORY(zoneylay);
  FREEMEMORY(zonetl);
  FREEMEMORY(zonetu);
  FREEMEMORY(zonerhol);
  FREEMEMORY(zonerhou);
  FREEMEMORY(zonepr);
  FREEMEMORY(zonetargets);
  FREEMEMORY(hazardcolor);
  FREEMEMORY(zoneqfire);
  FREEMEMORY(zonefheight);
  FREEMEMORY(zonefdiam);
  FREEMEMORY(zonefbase);
  FREEMEMORY(izonetu);
  FREEMEMORY(izonetl);
  FREEMEMORY(izonefl);
  FREEMEMORY(izonelw);
  FREEMEMORY(izoneuw);
  FREEMEMORY(izonecl);
  FREEMEMORY(izonetargets);
  FREEMEMORY(zoneodl);
  FREEMEMORY(zoneodu);
  FREEMEMORY(zoneslab_n);
  FREEMEMORY(zoneslab_T);
  FREEMEMORY(zoneslab_F);
  FREEMEMORY(zoneslab_YB);
  FREEMEMORY(zoneslab_YT);

  have_zonefl = 0;
  have_zonelw = 0;
  have_zoneuw = 0;
  have_zonecl = 0;
  have_wall_data = 0;
  have_target_data = 0;

  activezone=NULL;
  if(flag==UNLOAD){
    zonei->loaded=0;
    zonei->display=0;
    nzone_times=0;
    ReadZoneFile=0;
    showzone=0;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    updatemenu=1;
    return;
  }
  if(zonei->csv==1){
    ReadDeviceData(zonei->file,CSV_CFAST,UNLOAD);
    ReadDeviceData(zonei->file,CSV_CFAST,LOAD);
    GetZoneSizeCSV(&nzone_times,&nrooms2,&nfires2,&nzhvents2,&nzvvents2,&nzmvents2,&ntargets_local, &error);
  }
  else{
    getzonesize(file,&nzone_times,&nrooms2,&nfires2,&error);
    nzhvents2=global_scase.nzhvents;
    nzvvents2=global_scase.nzvvents;
  }
  CheckMemory;
  if(error!=0||global_scase.nrooms!=nrooms2||nzone_times==0||global_scase.nzhvents!=nzhvents2||global_scase.nzvvents!=nzvvents2||global_scase.nzmvents!=nzmvents2){
    showzone=0;
    UpdateTimes();
    ReadZoneFile=0;
    if(global_scase.nrooms!=nrooms2){
      fprintf(stderr,"*** Error: number of rooms specified in the smv file (%i)\n",global_scase.nrooms);
      fprintf(stderr,"    not consistent with the number specified in the zone file (%i)\n",nrooms2);
    }
    if(global_scase.nzhvents!=nzhvents2){
      fprintf(stderr,"*** Error: number of horizontal flow vents specified in the smv file (%i)\n",global_scase.nzhvents);
      fprintf(stderr,"    not consistent with the number specified in the data file (%i)\n",nzhvents2);
    }
    if(global_scase.nzvvents!=nzvvents2){
      fprintf(stderr,"*** Error: number of vertical flow vents specified in the smv file (%i)\n",global_scase.nzvvents);
      fprintf(stderr,"    not consistent with the number specified in the data file (%i)\n",nzvvents2);
    }
    if(global_scase.nzmvents != nzmvents2){
      fprintf(stderr, "*** Error: number of mechanical vents specified in the smv file (%i)\n", global_scase.nzmvents);
      fprintf(stderr, "    not consistent with the number specified in the data file (%i)\n", nzmvents2);
    }
    if(nzone_times <= 0)fprintf(stderr, "*** Error: The file, %s, contains no data\n", file);
    return;
  }
  FREEMEMORY(zonelonglabels);
  FREEMEMORY(zoneshortlabels);
  FREEMEMORY(zoneunits);
  FREEMEMORY(zonelevels);
  if(NewMemory((void **)&zonelonglabels  ,LABELLEN)==0||
     NewMemory((void **)&zoneshortlabels ,LABELLEN)==0||
     NewMemory((void **)&zoneunits       ,LABELLEN)==0||
     NewMemory((void **)&zonelevels      ,global_scase.nrgb*sizeof(float))==0){
    *errorcode=1;
    return;
  }
  CheckMemory;

  PRINTF("Loading zone data: %s\n",file);

  ntotal_rooms = global_scase.nrooms*nzone_times;
  nzonetotal=ntotal_rooms;

  if(ntotal_rooms>0){
    FREEMEMORY(zone_times);
    FREEMEMORY(zoneylay);
    FREEMEMORY(zonetl);
    FREEMEMORY(zonetu);
    FREEMEMORY(zonepr);
    FREEMEMORY(zonefl);
    FREEMEMORY(zonelw);
    FREEMEMORY(zoneuw);
    FREEMEMORY(zonecl);
    FREEMEMORY(zoneodl);
    FREEMEMORY(zoneodu);
    if(NewMemory((void **)&zone_times,  ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zoneylay,    ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zonetu,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zonetl,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zonepr,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zonefl,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zonelw,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zoneuw,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&zonecl,      ntotal_rooms*sizeof(float))==0||
       NewMemory((void **)&hazardcolor, ntotal_rooms*sizeof(unsigned char))==0){
      *errorcode=1;
      return;
    }
    if(zone_rho==1){
      if(NewMemory((void **)&zonerhou, ntotal_rooms*sizeof(float))==0||
         NewMemory((void **)&zonerhol, ntotal_rooms*sizeof(float))==0){
        *errorcode = 1;
        return;
      }
    }
    FREEMEMORY(zonevents);
    FREEMEMORY(zoneslab_n);
    FREEMEMORY(zoneslab_T);
    FREEMEMORY(zoneslab_F);
    FREEMEMORY(zoneslab_YB);
    FREEMEMORY(zoneslab_YT);
    if(global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents>0){
      int ntotalvents;

      ntotalvents = global_scase.nzhvents+global_scase.nzvvents+global_scase.nzmvents;
      NewMemory((void **)&zonevents,   nzone_times*ntotalvents*sizeof(float));
      NewMemory((void **)&zoneslab_n,  nzone_times*ntotalvents*sizeof(int));
      NewMemory((void **)&zoneslab_T,  nzone_times*ntotalvents*MAX_HSLABS*sizeof(float));
      NewMemory((void **)&zoneslab_YT, nzone_times*ntotalvents*MAX_HSLABS*sizeof(float));
      NewMemory((void **)&zoneslab_F,  nzone_times*ntotalvents*MAX_HSLABS*sizeof(float));
      NewMemory((void **)&zoneslab_YB, nzone_times*ntotalvents*MAX_HSLABS*sizeof(float));
    }

    FREEMEMORY(zoneqfire);
    FREEMEMORY(zonefheight);
    FREEMEMORY(zonefdiam);
    FREEMEMORY(zonefbase);
    if(global_scase.nfires!=0){
      if(
        NewMemory((void **)&zoneqfire,global_scase.nfires*nzone_times*sizeof(float))==0||
        NewMemory((void **)&zonefheight,global_scase.nfires*nzone_times*sizeof(float))==0||
        NewMemory((void **)&zonefdiam,global_scase.nfires*nzone_times*sizeof(float))==0||
        NewMemory((void **)&zonefbase,global_scase.nfires*nzone_times*sizeof(float))==0
        ){
        *errorcode=1;
        return;
      }
    }
    else{
      zoneqfire=NULL;
    }
    FREEMEMORY(izonetu);
    if(NewMemory((void **)&izonetu, ntotal_rooms*sizeof(unsigned char))==0){
      *errorcode=1;
      return;
    }
    FREEMEMORY(izonetl);
    if(NewMemory((void **)&izonetl, ntotal_rooms* sizeof(unsigned char)) == 0){
      *errorcode = 1;
      return;
    }
    FREEMEMORY(izonefl);
    if(NewMemory((void **)&izonefl, ntotal_rooms*sizeof(unsigned char))==0){
      *errorcode = 1;
      return;
    }
    FREEMEMORY(izonelw);
    if(NewMemory((void **)&izonelw, ntotal_rooms*sizeof(unsigned char))==0){
      *errorcode = 1;
      return;
    }
    FREEMEMORY(izoneuw);
    if(NewMemory((void **)&izoneuw, ntotal_rooms*sizeof(unsigned char))==0){
      *errorcode = 1;
      return;
    }
    FREEMEMORY(izonecl);
    if(NewMemory((void **)&izonecl, ntotal_rooms*sizeof(unsigned char))==0){
      *errorcode = 1;
      return;
    }
  }
  else{
    return;
  }
  ntotal_targets = ntargets_local*nzone_times;
  nzonetotal_targets = ntotal_targets;
  nzone_targets = ntargets_local;

  if(ntotal_targets>0){
    FREEMEMORY(zonetargets);
    if(NewMemory((void **)&zonetargets, ntotal_targets*sizeof(float))==0){
      *errorcode = 1;
      return;
    }
    FREEMEMORY(izonetargets);
    if(NewMemory((void **)&izonetargets, ntotal_targets*sizeof(unsigned char))==0){
      *errorcode = 1;
      return;
    }
  }
  CheckMemory;
  if(zonei->csv==1){
    GetZoneDataCSV(nzone_times,global_scase.nrooms,  global_scase.nfires, ntargets_local,
                   zone_times,zoneqfire, zonefheight, zonefbase, zonefdiam,
                   zonepr,zoneylay,zonetl,zonetu,zonerhol,zonerhou,&zoneodl,&zoneodu, zonevents,
                   zoneslab_n, zoneslab_T, zoneslab_F, zoneslab_YB, zoneslab_YT,
                   zonefl, zonelw, zoneuw, zonecl,
                   zonetargets,
                   &error);
  }
  else{
    getzonedata(file,&nzone_times,&global_scase.nrooms, &global_scase.nfires, zone_times,zoneqfire,zonepr,zoneylay,zonetl,zonetu,&error);
  }
  CheckMemory;

  if(zonei->csv==0){
    ii=0;
    for(i=0;i<nzone_times;i++){
      for(j=0;j<global_scase.nrooms;j++){
        zonetu[ii] = K2C(zonetu[ii]);
        zonetl[ii] = K2C(zonetl[ii]);
        ii++;
      }
    }
  }
  CheckMemory;
  ii = 0;
  for(i=0;i<nzone_times;i++){
    for(j=0;j<global_scase.nrooms;j++){
      if(zonetu[ii]>=500.0){
        hazardcolor[ii]=RED;
      }
      else{
        if(zonetu[ii]>=50.0){
          if(zoneylay[ii]>1.5){
            hazardcolor[ii]=YELLOW;
          }
          else{
            hazardcolor[ii]=PINK;
          }
        }
        else{
          if(zoneylay[ii]>2.0){
            hazardcolor[ii]=BLUE;
          }
          else{
            hazardcolor[ii]=GREEN;
          }
        }
      }
      zoneylay[ii]=SCALE2SMV(zoneylay[ii]);
      ii++;
    }
  }

  PRINTF("computing zone color levels \n");
  GetZoneGlobalBounds(zonetu, ntotal_rooms,&zoneglobalmin,&zoneglobalmax,FIRST_TIME);
  GetZoneGlobalBounds(zonetl, ntotal_rooms,&zoneglobalmin,&zoneglobalmax,NOT_FIRST_TIME);
  if(have_zonefl==1)GetZoneGlobalBounds(zonefl, ntotal_rooms,&zoneglobalmin,&zoneglobalmax,NOT_FIRST_TIME);
  if(have_zonelw==1)GetZoneGlobalBounds(zonelw, ntotal_rooms,&zoneglobalmin,&zoneglobalmax,NOT_FIRST_TIME);
  if(have_zoneuw==1)GetZoneGlobalBounds(zoneuw, ntotal_rooms,&zoneglobalmin,&zoneglobalmax,NOT_FIRST_TIME);
  if(have_zonecl==1)GetZoneGlobalBounds(zonecl, ntotal_rooms,&zoneglobalmin,&zoneglobalmax,NOT_FIRST_TIME);
  if(have_target_data==1)GetZoneGlobalBounds(zonetargets, ntotal_targets, &zoneglobalmin, &zoneglobalmax, NOT_FIRST_TIME);
  if(slice_temp_bounds_defined==0){
    GetSliceTempBounds();
  }
  if(flag==BOUNDS_ONLY)return;
  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(strcmp(slicei->label.shortlabel, "TEMP")==0){
      zoneglobalmin = MIN(slicei->valmin_slice, zoneglobalmin);
      zoneglobalmax = MAX(slicei->valmax_slice, zoneglobalmax);
    }
  }
  zone_temp_bounds_defined = 1;
  if(setzonemin==GLOBAL_MIN)zonemin = zoneglobalmin;
  if(setzonemax==GLOBAL_MAX)zonemax = zoneglobalmax;
  if(setzonemin==SET_MIN)zonemin = zoneusermin;
  if(setzonemax==SET_MAX)zonemax = zoneusermax;
  GLUIUpdateZoneBounds();
  GetZoneColors(zonetu, ntotal_rooms, izonetu, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
  GetZoneColors(zonetl, ntotal_rooms, izonetl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
  if(have_zonefl==1)GetZoneColors(zonefl, ntotal_rooms, izonefl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
  if(have_zonelw==1)GetZoneColors(zonelw, ntotal_rooms, izonelw, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
  if(have_zoneuw==1)GetZoneColors(zoneuw, ntotal_rooms, izoneuw, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
  if(have_zonecl==1)GetZoneColors(zonecl, ntotal_rooms, izonecl, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);
  if(have_target_data==1)GetZoneColors(zonetargets, ntotal_targets, izonetargets, zonemin, zonemax, global_scase.nrgb, nrgb_full, global_scase.colorlabelzone, colorvalueszone, zonelevels256);

  ReadZoneFile=1;
  visZone=1;
  showzone=1;
  zonei->loaded=1;
  zonei->display=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  updatemenu=1;
  activezone = global_scase.zoneinfo + ifile;
  if(global_scase.nzhvents>0||global_scase.nzvvents>0||global_scase.nzmvents>0){
    PRINTF("computing vent bounds\n");
    GetZoneVentBounds();
  }
  ForceIdle();
  CheckMemory;

}

/* ------------------ DrawZoneVent ------------------------ */

void DrawZoneVent(float area_fraction, float x1, float x2, float y1, float y2, float z1, float z2){
// draw an X through the vent if it is closed
  if(area_fraction < 0.0001){
    if(ABS(x1-x2)>0.0 && ABS(y1-y2)>0.0){
      glVertex3f(x1,y1,z1);
      glVertex3f(x2,y2,z1);
      glVertex3f(x1,y2,z1);
      glVertex3f(x2,y1,z1);
    }
    if(ABS(x1-x2)>0.0 && ABS(z1-z2)>0.0){
      glVertex3f(x1,y1,z1);
      glVertex3f(x2,y1,z2);
      glVertex3f(x1,y1,z2);
      glVertex3f(x2,y1,z1);
    }
    if(ABS(y1-y2)>0.0 && ABS(z1-z2)>0.0){
      glVertex3f(x1,y1,z1);
      glVertex3f(x1,y2,z2);
      glVertex3f(x1,y1,z2);
      glVertex3f(x1,y2,z1);
    }
  }
  // on Z plane
  if(ABS(x1 - x2)>0.0 && ABS(y1 - y2)>0.0){
    glVertex3f(x1,y1,z1);
    glVertex3f(x2,y1,z1);

    glVertex3f(x2, y1, z1);
    glVertex3f(x2,y2,z1);

    glVertex3f(x2, y2, z1);
    glVertex3f(x1,y2,z1);

    glVertex3f(x1, y2, z1);
    glVertex3f(x1,y1,z1);
  }

  // on y plane
  if(ABS(x1 - x2)>0.0 && ABS(z1 - z2)>0.0){
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y1, z1);

    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y1, z2);

    glVertex3f(x2, y1, z2);
    glVertex3f(x1, y1, z2);

    glVertex3f(x1, y1, z2);
    glVertex3f(x1, y1, z1);
  }

  // on x plane
  if(ABS(y1 - y2)>0.0 && ABS(z1 - z2)>0.0){
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z2);

    glVertex3f(x1, y1, z2);
    glVertex3f(x1, y2, z2);

    glVertex3f(x1, y2, z2);
    glVertex3f(x1, y2, z1);

    glVertex3f(x1, y2, z1);
    glVertex3f(x1, y1, z1);
  }
}

/* ------------------ DrawZoneRoomGeom ------------------------ */

void DrawZoneRoomGeom(void){
  int i;

  FillZoneData(izone);

/* draw the frame */

  if(visCompartments == 1){
    AntiAliasLine(ON);
    glBegin(GL_LINES);

    for(i = 0; i < global_scase.nrooms; i++){
      roomdata *roomi;
      float xroom0, yroom0, zroom0, xroom, yroom, zroom;

      if(zone_highlight == 1 && zone_highlight_room == i){
        glEnd();
        glLineWidth(5.0*global_scase.linewidth);
        glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
      }
      else{
        glEnd();
        glLineWidth(global_scase.linewidth);
        glBegin(GL_LINES);
        glColor4fv(foregroundcolor);
      }

      roomi = global_scase.roominfo + i;
      xroom0 = roomi->x0;
      yroom0 = roomi->y0;
      zroom0 = roomi->z0;
      xroom = roomi->x1;
      yroom = roomi->y1;
      zroom = roomi->z1;


      glVertex3f(xroom0, yroom0, zroom);
      glVertex3f(xroom, yroom0, zroom);

      glVertex3f(xroom, yroom0, zroom);
      glVertex3f(xroom, yroom, zroom);

      glVertex3f(xroom, yroom, zroom);
      glVertex3f(xroom0, yroom, zroom);

      glVertex3f(xroom0, yroom, zroom);
      glVertex3f(xroom0, yroom0, zroom);

      glVertex3f(xroom0, yroom0, zroom0);
      glVertex3f(xroom, yroom0, zroom0);

      glVertex3f(xroom, yroom0, zroom0);
      glVertex3f(xroom, yroom, zroom0);

      glVertex3f(xroom, yroom, zroom0);
      glVertex3f(xroom0, yroom, zroom0);

      glVertex3f(xroom0, yroom, zroom0);
      glVertex3f(xroom0, yroom0, zroom0);

      glVertex3f(xroom0, yroom0, zroom0);
      glVertex3f(xroom0, yroom0, zroom);

      glVertex3f(xroom, yroom0, zroom0);
      glVertex3f(xroom, yroom0, zroom);

      glVertex3f(xroom, yroom, zroom0);
      glVertex3f(xroom, yroom, zroom);

      glVertex3f(xroom0, yroom, zroom0);
      glVertex3f(xroom0, yroom, zroom);
    }
    glEnd();
    AntiAliasLine(OFF);
  }

  if(visVents==1){
    for(i=0;i<global_scase.nzvents;i++){
      zventdata *zvi;
      float x1, x2, y1, y2, z1, z2;

      zvi = global_scase.zventinfo + i;
      x1 = zvi->x0;
      x2 = zvi->x1;
      y1 = zvi->y0;
      y2 = zvi->y1;
      z1 = zvi->z0;
      z2 = zvi->z1;
      if(zvi->vent_type == MFLOW_VENT){
        glPushMatrix();
        glTranslatef(x2, y2, z2);
        {
          unsigned char hvac_sphere_color[3];

          hvac_sphere_color[0] = 255 * foregroundcolor[0];
          hvac_sphere_color[1] = 255 * foregroundcolor[1];
          hvac_sphere_color[2] = 255 * foregroundcolor[2];
          DrawSphere(SCALE2SMV(zone_hvac_diam), hvac_sphere_color);
        }
        glPopMatrix();
        glLineWidth(2.0*global_scase.ventlinewidth);
      }
      else{
        glLineWidth(global_scase.ventlinewidth);
      }
      glColor4fv(zvi->color);
      if(zvi->vent_type==VFLOW_VENT&&zvi->vertical_vent_type==ZONEVENT_CIRCLE){
        unsigned char uc_color[4];

        glPushMatrix();
        if(zvi->wall==TOP_WALL){
          glTranslatef(FDS2SMV_X(zvi->xcen), FDS2SMV_Y(zvi->ycen), z2);
	}
	else{
          glTranslatef(FDS2SMV_X(zvi->xcen), FDS2SMV_Y(zvi->ycen), z1);
	}
        uc_color[0] = zvi->color[0]*255;
        uc_color[1] = zvi->color[1]*255;
        uc_color[2] = zvi->color[2]*255;
        uc_color[3] = zvi->color[3]*255;
        DrawCircle(2.0*SCALE2SMV(zvi->radius), uc_color, &cvent_circ);
        if(zvi->area_fraction<0.00001){
          float x45;

          x45 = sqrt(2.0)/2.0;
          glLineWidth(global_scase.ventlinewidth);
          glBegin(GL_LINES);
          glColor3ubv(uc_color);
          glVertex3f(-x45*SCALE2SMV(zvi->radius), -x45*SCALE2SMV(zvi->radius), 0.0);
          glVertex3f( x45*SCALE2SMV(zvi->radius),  x45*SCALE2SMV(zvi->radius), 0.0);
          glVertex3f(-x45*SCALE2SMV(zvi->radius),  x45*SCALE2SMV(zvi->radius), 0.0);
          glVertex3f( x45*SCALE2SMV(zvi->radius), -x45*SCALE2SMV(zvi->radius), 0.0);
          glEnd();
        }
        glPopMatrix();
      }
      else{
        float delta;
        float XB[6];

        delta = ventoffset_factor*0.05/xyzmaxdiff;
        XB[0] = x1;
        XB[1] = x2;
        XB[2] = y1;
        XB[3] = y2;
        XB[4] = z1;
        XB[5] = z2;
        glBegin(GL_LINES);
        switch(zvi->wall){
        case LEFT_WALL:
          XB[0] += delta;
          XB[1]  = XB[0];
          break;

        case RIGHT_WALL:
          XB[0] -= delta;
          XB[1]  = XB[0];
          break;

        case FRONT_WALL:
          XB[2] += delta;
          XB[3]  = XB[2];
          break;

        case BACK_WALL:
          XB[2] -= delta;
          XB[3]  = XB[2];
          break;

        case BOTTOM_WALL:
          XB[4] += delta;
          XB[5]  = XB[4];
          break;

        case TOP_WALL:
          XB[4] -= delta;
          XB[5]  = XB[4];
          break;

        default:
          assert(FFALSE);
          break;
        }
        DrawZoneVent(zvi->area_fraction, XB[0], XB[1], XB[2], XB[3], XB[4], XB[5]);
        glEnd();
      }
    }
  }
}

/* ------------------ DrawZoneVentDataProfile ------------------------ */

void DrawZoneVentDataProfile(void){
  float factor;
  int i;

  if(visVentHFlow==0||visventprofile==0)return;

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  for(i=0;i<global_scase.nzvents;i++){
    int j;
    zventdata *zvi;
    float zelev[NELEV_ZONE];

    zvi = global_scase.zventinfo + i;
    assert(zvi->z0 <= zvi->z1);
    if(zvi->vent_type==VFLOW_VENT||zvi->vent_type==MFLOW_VENT)continue;
    for(j=0;j<NELEV_ZONE;j++){
      zelev[j]=(zvi->z0*(NELEV_ZONE-1-j)+zvi->z1*j)/(float)(NELEV_ZONE-1);
    }
    GetZoneVentVel(zelev, NELEV_ZONE, zvi->room1, zvi->room2, zvi->vdata, &zvi->vmin, &zvi->vmax, zvi->itempdata);
  }
  factor = 0.1*zone_ventfactor/zone_maxventflow;
  for(i=0;i<global_scase.nzvents;i++){
    zventdata *zvi;
    int j;
    float zelev[NELEV_ZONE];
    float *vcolor1,*vcolor2;
    float xmid, ymid;

    zvi = global_scase.zventinfo + i;

    if(zvi->vent_type==VFLOW_VENT||zvi->vent_type==MFLOW_VENT)continue;
    for(j=0;j<NELEV_ZONE;j++){
      zelev[j]=(zvi->z0*(NELEV_ZONE-1-j)+zvi->z1*j)/(float)(NELEV_ZONE-1);
    }
    xmid = (zvi->x0 + zvi->x1)/2.0;
    ymid = (zvi->y0 + zvi->y1)/2.0;
    glBegin(GL_QUADS);
    for(j=0;j<NELEV_ZONE-1;j++){
      float dvent1,dvent2;
      float xwall, ywall;

      switch(zvi->wall){
      case LEFT_WALL:
        xwall = zvi->x0;
        break;
      case RIGHT_WALL:
        xwall = zvi->x1;
        break;
      case FRONT_WALL:
        ywall = zvi->y0;
        break;
      case BACK_WALL:
        ywall = zvi->y1;
        break;
      default:
	    assert(FFALSE);
	    break;
      }
      dvent1 = factor*zvi->area_fraction*zvi->vdata[j];
      dvent2 = factor*zvi->area_fraction*zvi->vdata[j+1];

      if(zvi->wall==FRONT_WALL||zvi->wall==LEFT_WALL){
        dvent1=-dvent1;
        dvent2=-dvent2;
      }
      vcolor1=rgb_full[zvi->itempdata[j]];
      vcolor2=rgb_full[zvi->itempdata[j+1]];
      vcolor2=vcolor1;
      switch(zvi->wall){
      case LEFT_WALL:
      case RIGHT_WALL:
        if(dvent1*dvent2>=0.0){
          glColor3fv(vcolor1);
          glVertex3f(xwall,       ymid,zelev[j]);
          glVertex3f(xwall+dvent1,ymid,zelev[j]);

          glColor3fv(vcolor2);
          glVertex3f(xwall+dvent2,ymid,zelev[j+1]);
          glVertex3f(xwall,       ymid,zelev[j+1]);
        }
        else{
          float dvent;

          dvent =  dvent1*(zelev[j+1]-zelev[j])/(dvent2-dvent1);
          glColor3fv(vcolor1);
          glVertex3f(xwall,          ymid, zelev[j]);
          glVertex3f(xwall + dvent1, ymid, zelev[j]);
          glVertex3f(xwall,          ymid, zelev[j] - dvent);
          glVertex3f(xwall,          ymid, zelev[j] - dvent);

          glColor3fv(vcolor2);
          glVertex3f(xwall,          ymid, zelev[j] - dvent);
          glVertex3f(xwall,          ymid, zelev[j] - dvent);
          glVertex3f(xwall + dvent2, ymid, zelev[j + 1]);
          glVertex3f(xwall,          ymid, zelev[j + 1]);
        }
        break;
      case BACK_WALL:
      case FRONT_WALL:
        if(dvent1*dvent2>=0.0){
          glColor3fv(vcolor1);
          glVertex3f(xmid, ywall,          zelev[j]);
          glVertex3f(xmid, ywall + dvent1, zelev[j]);

          glColor3fv(vcolor2);
          glVertex3f(xmid, ywall + dvent2, zelev[j + 1]);
          glVertex3f(xmid, ywall,          zelev[j + 1]);
        }
        else{
          float dvent;

          dvent =  dvent1*(zelev[j+1]-zelev[j])/(dvent2-dvent1);
          glColor3fv(vcolor1);
          glVertex3f(xmid, ywall,          zelev[j]);
          glVertex3f(xmid, ywall + dvent1, zelev[j]);
          glVertex3f(xmid, ywall,          zelev[j] - dvent);
          glVertex3f(xmid, ywall,          zelev[j] - dvent);

          glColor3fv(vcolor2);
          glVertex3f(xmid, ywall,          zelev[j] - dvent);
          glVertex3f(xmid, ywall,          zelev[j] - dvent);
          glVertex3f(xmid, ywall + dvent2, zelev[j + 1]);
          glVertex3f(xmid, ywall,          zelev[j + 1]);
        }
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
    glEnd();
  }
  if(cullfaces==1)glEnable(GL_CULL_FACE);
}

/* ------------------ DrawZoneVentDataSlab ------------------------ */

void DrawZoneVentDataSlab(void){
  int i;

  if(visVentHFlow==0&&visVentVFlow&&visVentMFlow==0)return;

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  for(i = 0; i<global_scase.nzvents; i++){
    zventdata *zvi;
    int islab;
    float xmid, ymid;

    zvi = global_scase.zventinfo+i;

    if((visVentHFlow==0||visventslab!=1)&&zvi->vent_type==HFLOW_VENT)continue;
    if(visVentVFlow==0&&zvi->vent_type==VFLOW_VENT)continue;
    if(visVentMFlow==0&&zvi->vent_type==MFLOW_VENT)continue;
    xmid = (zvi->x0+zvi->x1)/2.0;
    ymid = (zvi->y0+zvi->y1)/2.0;

    glBegin(GL_QUADS);
    for(islab = 0; islab<zvi->nslab;islab++){
      float slab_bot, slab_top, tslab, *tcolor;
      int itslab;
      float dvent;

      slab_bot = FDS2SMV_Z(zvi->slab_bot[islab]);
      slab_top = FDS2SMV_Z(zvi->slab_top[islab]);
      tslab = zvi->slab_temp[islab];
      itslab = GetZoneColor(K2C(tslab), zonemin, zonemax, nrgb_full);
      tcolor = rgb_full[itslab];
      glColor3fv(tcolor);

      dvent = 0.1*zone_ventfactor*zvi->slab_vel[islab] / maxslabflow;
      switch(zvi->wall){
      case LEFT_WALL:
        glVertex3f(zvi->x0,         ymid, slab_bot);
        glVertex3f(zvi->x0 - dvent, ymid, slab_bot);

        glVertex3f(zvi->x0 - dvent, ymid, slab_top);
        glVertex3f(zvi->x0,         ymid, slab_top);
        break;
      case RIGHT_WALL:
        glVertex3f(zvi->x1,         ymid, slab_bot);
        glVertex3f(zvi->x1 + dvent, ymid, slab_bot);

        glVertex3f(zvi->x1 + dvent, ymid, slab_top);
        glVertex3f(zvi->x1,         ymid, slab_top);
        break;
      case FRONT_WALL:
        glVertex3f(xmid, zvi->y0,         slab_bot);
        glVertex3f(xmid, zvi->y0 - dvent, slab_bot);

        glVertex3f(xmid, zvi->y0 - dvent, slab_top);
        glVertex3f(xmid, zvi->y0,         slab_top);
        break;
      case BACK_WALL:
        glVertex3f(xmid, zvi->y1,         slab_bot);
        glVertex3f(xmid, zvi->y1 + dvent, slab_bot);

        glVertex3f(xmid, zvi->y1 + dvent, slab_top);
        glVertex3f(xmid, zvi->y1, slab_top);
        break;
      case BOTTOM_WALL:
        glVertex3f(zvi->x0, ymid, zvi->z0);
        glVertex3f(zvi->x1, ymid, zvi->z0);

        glVertex3f(zvi->x1, ymid, zvi->z0 - dvent);
        glVertex3f(zvi->x0, ymid, zvi->z0 - dvent);
        break;
      case TOP_WALL:
        glVertex3f(zvi->x0, ymid, zvi->z1);
        glVertex3f(zvi->x1, ymid, zvi->z1);

        glVertex3f(zvi->x1, ymid, zvi->z1 + dvent);
        glVertex3f(zvi->x0, ymid, zvi->z1 + dvent);
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
    glEnd();
  }
  if(cullfaces==1)glEnable(GL_CULL_FACE);
}

  /* ------------------ DrawZoneWallData ------------------------ */

void DrawZoneWallData(void){
  int i;
  unsigned char *cl, *uw, *lw, *fl;

  FillZoneData(izone);

  cl = izonecl + izone*global_scase.nrooms;
  uw = izoneuw + izone*global_scase.nrooms;
  lw = izonelw + izone*global_scase.nrooms;
  fl = izonefl + izone*global_scase.nrooms;


/* draw the frame */

  glBegin(GL_TRIANGLES);

  for(i = 0; i < global_scase.nrooms; i++){
    roomdata *roomi;
    float x0, y0, z0, x1, y1, z1, z;

    roomi = global_scase.roominfo + i;
    x0 = roomi->x0;
    y0 = roomi->y0;
    z0 = roomi->z0;
    x1 = roomi->x1;
    y1 = roomi->y1;
    z1 = roomi->z1;
    z  = roomi->ylay;


    // ceiling
    glColor4fv(rgb_full[cl[i]]);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);

    // floor
    glColor4fv(rgb_full[fl[i]]);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0);

    // upper wall
    glColor4fv(rgb_full[uw[i]]);
    glVertex3f(x0, y0, z);
    glVertex3f(x0, y1, z);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y0, z);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y0, z1);

    glVertex3f(x1, y0, z);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z);
    glVertex3f(x1, y0, z);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);

    glVertex3f(x0, y0, z);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y0, z);
    glVertex3f(x0, y0, z);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);

    glVertex3f(x0, y1, z);
    glVertex3f(x1, y1, z);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    // lower wall
    glColor4fv(rgb_full[lw[i]]);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x0, y1, z);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z);
    glVertex3f(x0, y0, z);

    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z);
    glVertex3f(x1, y1, z);

    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z);
    glVertex3f(x1, y0, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z);
    glVertex3f(x1, y0, z);

    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z);
    glVertex3f(x0, y1, z);

  }
  glEnd();
}

/* ------------------ DrawZoneVentData ------------------------ */

void DrawZoneVentData(void){
  if(have_ventslab_flow==1)DrawZoneVentDataSlab();
  if(have_ventslab_flow!=1||visventslab!=1){
    DrawZoneVentDataProfile();
  }
}

/* ------------------ GetZoneThick ------------------------ */

float GetZoneThick(int dir, roomdata *roomi, float xyz[3]){
  float dx, dy, dz, L;
  float alpha, alpha_min, alpha_ylay;
  float x0, x1, yy0, yy1, z0, z1;
  float factor_L=0.0, factor_U=0.0, factor;
  float ylay;
  float thick;
  float odl, odu;

  x0 = roomi->x0;
  x1 = roomi->x1;
  yy0 = roomi->y0;
  yy1 = roomi->y1;
  z0 = roomi->z0;
  z1 = roomi->z1;
  odl = roomi->od_L;
  odu = roomi->od_U;
  alpha_min = 100000.0;
  ylay = roomi->z0 + roomi->ylay;

  dx = xyz[0] - eye_position_smv[0];
  dy = xyz[1] - eye_position_smv[1];
  dz = xyz[2] - eye_position_smv[2];
  L = sqrt(dx*dx+dy*dy+dz*dz);

  alpha_ylay = (ylay - xyz[2])/dz;
  if(roomi->zoneinside==0){
    if(dx!=0.0){
      if(dir!=-1){
        alpha = (x0-xyz[0])/dx;
        if(alpha>0.0&&alpha<alpha_min){
          alpha_min = alpha;
        }
      }
      if(dir!=1){
        alpha = (x1-xyz[0])/dx;
        if(alpha>0.0&&alpha<alpha_min){
          alpha_min = alpha;
        }
      }
    }
    if(dy!=0.0){
      if(dir!=-2){
        alpha = (yy0-xyz[1])/dy;
        if(alpha>0.0&&alpha<alpha_min){
          alpha_min = alpha;
        }
      }
      if(dir!=2){
        alpha = (yy1-xyz[1])/dy;
        if(alpha>0.0&&alpha<alpha_min){
          alpha_min = alpha;
        }
      }
    }
    if(dz!=0.0){
      if(dir!=-3){
        alpha = (z0-xyz[2])/dz;
        if(alpha>0.0&&alpha<alpha_min){
          alpha_min = alpha;
        }
      }
      if(dir!=3){
        alpha = (z1-xyz[2])/dz;
        if(alpha>0.0&&alpha<alpha_min){
          alpha_min = alpha;
        }
      }
    }
    if(eye_position_smv[2]>ylay&&xyz[2]>ylay){
      if(alpha_ylay>0.0&&alpha_ylay<alpha_min){
        factor_U=alpha_ylay/odu;
        factor_L=(alpha_min-alpha_ylay)/odl;
      }
      else{
        factor_U=alpha_min/odu;
        factor_L=0.0;
      }
    }
    if(eye_position_smv[2]>ylay&&xyz[2]<=ylay){
      factor_U=0.0;
      factor_L=alpha_min/odl;
    }
    if(eye_position_smv[2]<=ylay&&xyz[2]>ylay){
      factor_U=alpha_min/odu;
      factor_L=0.0;
    }
    if(eye_position_smv[2]<=ylay&&xyz[2]<=ylay){
      if(alpha_ylay>0.0&&alpha_ylay<alpha_min){
        factor_U=(alpha_min-alpha_ylay)/odu;
        factor_L=alpha_ylay/odl;
      }
      else{
        factor_U=0.0;
        factor_L=alpha_min/odl;
      }
    }
  }
  else{
    if(eye_position_smv[2]>ylay&&xyz[2]>ylay){
      factor_U=1.0/odu;
      factor_L=0.0;
    }
    if(eye_position_smv[2]>ylay&&xyz[2]<=ylay){
      factor_U=(1.0+alpha_ylay)/odu;
      factor_L=-alpha_ylay/odl;
    }
    if(eye_position_smv[2]<=ylay&&xyz[2]>ylay){
      factor_U=-alpha_ylay/odu;
      factor_L=(1.0+alpha_ylay)/odl;
    }
    if(eye_position_smv[2]<=ylay&&xyz[2]<=ylay){
      factor_U=0.0;
      factor_L=1.0/odl;
    }
  }

  factor = SCALE2FDS((factor_U+factor_L)*L);
  thick = 1.0-exp(-factor);
  return thick;
}

#ifdef pp_GPU
/* ------------------ DrawZoneSmokeGpu ------------------------ */

void DrawZoneSmokeGpu(roomdata *roomi){
#define NROWS_GPU 2
#define NCOLS_GPU 2
  int iwall;
  float dx, dy, dz;

  glUniform3f(GPUzone_eyepos,eye_position_smv[0],eye_position_smv[1],eye_position_smv[2]);
  glUniform1i(GPUzone_zoneinside,roomi->zoneinside);
  glUniform1f(GPUzone_xyzmaxdiff,xyzmaxdiff);
  glUniform3f(GPUzone_boxmin,roomi->x0,roomi->y0,roomi->z0);
  glUniform3f(GPUzone_boxmax,roomi->x1,roomi->y1,roomi->z1);
  glUniform1f(GPUzone_zlay,roomi->z0+roomi->ylay);
  glUniform1f(GPUzone_odl,roomi->od_L);
  glUniform1f(GPUzone_odu,roomi->od_U);

  for(iwall=-3;iwall<=3;iwall++){
    int i,j;
    float x1, x2, yy1, yy2, z1, z2;

    if(iwall==0||roomi->drawsides[iwall+3]==0)continue;

    glUniform1i(GPUzone_zonedir,iwall);
    glBegin(GL_TRIANGLES);

    switch(iwall){
      case XLEFT:
      case XRIGHT:
        dy = roomi->dy/(NCOLS_GPU-1);
        dz = roomi->dz/(NROWS_GPU-1);
        if(iwall<0){
          x1 = roomi->x0;
        }
        else{
          x1=roomi->x1;
        }
        for(i=0;i<NCOLS_GPU-1;i++){
          yy1 = roomi->y0 + i*dy;
          yy2 = yy1 + dy;
          for(j=0;j<NROWS_GPU-1;j++){
            z1 = roomi->z0 + j*dz;
            z2 = z1 + dz;

            glVertex3f(x1,yy1,z1);
            glVertex3f(x1,yy2,z1);
            glVertex3f(x1,yy2,z2);

            glVertex3f(x1,yy1,z1);
            glVertex3f(x1,yy2,z2);
            glVertex3f(x1,yy1,z2);
          }
        }
        break;
      case YBACK:
      case YFRONT:
        dx = roomi->dx/(NCOLS_GPU-1);
        dz = roomi->dz/(NROWS_GPU-1);
        if(iwall<0){
          yy1=roomi->y0;
        }
        else{
          yy1=roomi->y1;
        }
        for(i=0;i<NCOLS_GPU-1;i++){
          x1 = roomi->x0 + i*dx;
          x2 = x1 + dx;
          for(j=0;j<NROWS_GPU-1;j++){
            z1 = roomi->z0 + j*dz;
            z2 = z1 + dz;

            if(roomi->zoneinside==0){
              glVertex3f(x1,yy1,z1);
              glVertex3f(x2,yy1,z1);
              glVertex3f(x2,yy1,z2);

              glVertex3f(x1,yy1,z1);
              glVertex3f(x2,yy1,z2);
              glVertex3f(x1,yy1,z2);
            }
            else{
              glVertex3f(x1,yy1,z1);
              glVertex3f(x2,yy1,z2);
              glVertex3f(x2,yy1,z1);

              glVertex3f(x1,yy1,z1);
              glVertex3f(x1,yy1,z2);
              glVertex3f(x2,yy1,z2);
            }
          }
        }
        break;
      case ZBOTTOM:
      case ZTOP:
        dx = roomi->dx/(NCOLS_GPU-1);
        dy = roomi->dy/(NROWS_GPU-1);
        if(iwall<0){
          z1=roomi->z0;
        }
        else{
          z1=roomi->z1;
        }
        for(i=0;i<NCOLS_GPU-1;i++){
          x1 = roomi->x0 + i*dx;
          x2 = x1 + dx;
          for(j=0;j<NROWS_GPU-1;j++){
            yy1 = roomi->y0 + j*dy;
            yy2 = yy1 + dy;

            if(roomi->zoneinside==0){
              glVertex3f(x1,yy1,z1);
              glVertex3f(x2,yy1,z1);
              glVertex3f(x2,yy2,z1);

              glVertex3f(x1,yy1,z1);
              glVertex3f(x2,yy2,z1);
              glVertex3f(x1,yy2,z1);
            }
            else{
              glVertex3f(x1,yy1,z1);
              glVertex3f(x2,yy2,z1);
              glVertex3f(x2,yy1,z1);

              glVertex3f(x1,yy1,z1);
              glVertex3f(x1,yy2,z1);
              glVertex3f(x2,yy2,z1);
            }
          }
        }
        break;
      default:
        assert(FFALSE);
        break;
    }
    glEnd();
  }

}

#endif
/* ------------------ DrawZoneSmoke ------------------------ */

void DrawZoneSmoke(roomdata *roomi){
#define NROWS 100
#define NCOLS 100
  float vxyz[4][NROWS][NCOLS];
  int iwall;
  float xyz[3];
  float dx, dy, dz;

  for(iwall=-3;iwall<=3;iwall++){
    int i,j;

    if(iwall==0)continue;
    if(roomi->drawsides[iwall+3]==0)continue;

    switch(iwall){
      case XLEFT:
      case XRIGHT:
        dy = roomi->dy/(NCOLS-1);
        dz = roomi->dz/(NROWS-1);
        if(iwall<0){
          xyz[0]=roomi->x0;
        }
        else{
          xyz[0]=roomi->x1;
        }
        for(i=0;i<NCOLS;i++){
          xyz[1] = roomi->y0 + i*dy;
          for(j=0;j<NROWS;j++){
            xyz[2] = roomi->z0 + j*dz;
            vxyz[0][i][j]=xyz[0];
            vxyz[1][i][j]=xyz[1];
            vxyz[2][i][j]=xyz[2];
            vxyz[3][i][j]= GetZoneThick(iwall,roomi,xyz);
          }
        }
        break;
      case YBACK:
      case YFRONT:
        dx = roomi->dx/(NCOLS-1);
        dz = roomi->dz/(NROWS-1);
        if(iwall<0){
          xyz[1]=roomi->y0;
        }
        else{
          xyz[1]=roomi->y1;
        }
        for(i=0;i<NCOLS;i++){
          xyz[0] = roomi->x0 + i*dx;
          for(j=0;j<NROWS;j++){
            xyz[2] = roomi->z0 + j*dz;
            vxyz[0][i][j]=xyz[0];
            vxyz[1][i][j]=xyz[1];
            vxyz[2][i][j]=xyz[2];
            vxyz[3][i][j]= GetZoneThick(iwall,roomi,xyz);
          }
        }
        break;
      case ZBOTTOM:
      case ZTOP:
        dx = roomi->dx/(NCOLS-1);
        dy = roomi->dy/(NROWS-1);
        if(iwall<0){
          xyz[2]=roomi->z0;
        }
        else{
          xyz[2]=roomi->z1;
        }
        for(i=0;i<NCOLS;i++){
          xyz[0] = roomi->x0 + i*dx;
          for(j=0;j<NROWS;j++){
            xyz[1] = roomi->y0 + j*dy;
            vxyz[0][i][j]=xyz[0];
            vxyz[1][i][j]=xyz[1];
            vxyz[2][i][j]=xyz[2];
            vxyz[3][i][j]= GetZoneThick(iwall,roomi,xyz);
          }
        }
        break;
      default:
        assert(FFALSE);
        break;
    }

    glBegin(GL_TRIANGLES);
    for(i=0;i<NCOLS-1;i++){
      for(j=0;j<NROWS-1;j++){
        float x11, x12, x22, x21;
        float y11, y12, y22, y21;
        float z11, z12, z22, z21;
        float a11, a12, a22, a21;
        float grey=0.0;

        x11 = vxyz[0][i][j];
        x12 = vxyz[0][i][j+1];
        x21 = vxyz[0][i+1][j];
        x22 = vxyz[0][i+1][j+1];
        y11 = vxyz[1][i][j];
        y12 = vxyz[1][i][j+1];
        y21 = vxyz[1][i+1][j];
        y22 = vxyz[1][i+1][j+1];
        z11 = vxyz[2][i][j];
        z12 = vxyz[2][i][j+1];
        z21 = vxyz[2][i+1][j];
        z22 = vxyz[2][i+1][j+1];
        a11 = vxyz[3][i][j];
        a12 = vxyz[3][i][j+1];
        a21 = vxyz[3][i+1][j];
        a22 = vxyz[3][i+1][j+1];

        glColor4f(grey,grey,grey,a11);
        glVertex3f(x11,y11,z11);
        glColor4f(grey,grey,grey,a12);
        glVertex3f(x12,y12,z12);
        glColor4f(grey,grey,grey,a22);
        glVertex3f(x22,y22,z22);

        glColor4f(grey,grey,grey,a11);
        glVertex3f(x11,y11,z11);
        glColor4f(grey,grey,grey,a22);
        glVertex3f(x22,y22,z22);
        glColor4f(grey,grey,grey,a21);
        glVertex3f(x21,y21,z21);
      }
    }
    glEnd();
  }
}

/* ------------------ DrawZoneFirePlume ------------------------ */

void DrawZoneFirePlume(float diameter, float height, float maxheight){
  unsigned char firecolor[3] = {255,128,0};

  if(height <= maxheight){
    float dlower, dupper;

    dlower = diameter;
    dupper = 0.0;
    DrawTruncCone(dlower, dupper, height, firecolor);
  }
  else{
    float dh;
    float dlower1, dupper1;
    float dlower2, dupper2;

    dh = height - maxheight;
    dlower1 = diameter;
    dupper1 = diameter * dh / height;
    DrawTruncCone(dlower1, dupper1, maxheight, firecolor);

    glPushMatrix();
    glTranslatef(0.0, 0.0, maxheight - dupper1 / 2.0);
    dlower2 = 0.0;
    dupper2 = 2.0 * dh;
    DrawTruncCone(dlower2, dupper2, dupper1 / 2.0, firecolor);
    glPopMatrix();
  }
}

/* ------------------ DrawZoneFireData ------------------------ */

void DrawZoneFireData(void){
  int i;
  float *zoneqfirebase, *zonefheightbase, *zonefdiambase, *zonefbasebase;

  if(zone_times[0]>global_times[itimes])return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  zoneqfirebase = zoneqfire + izone*global_scase.nfires;
  zonefheightbase = zonefheight + izone*global_scase.nfires;
  zonefdiambase = zonefdiam + izone*global_scase.nfires;
  zonefbasebase = zonefbase + izone*global_scase.nfires;

  if(viszonefire==1){
    for(i=0;i<global_scase.nfires;i++){
      float qdot;
      float diameter, flameheight, maxheight;

      qdot = zoneqfirebase[i]/1000.0f;
      if(global_scase.zonecsv==1){
        if(qdot>0.0f){
          firedata *firei;
          roomdata *roomi;
          float deltaz;
          meshdata *meshi;

          // radius/plumeheight = .268 = atan(15 degrees)
          firei = global_scase.fireinfo + i;
          roomi = global_scase.roominfo + firei->roomnumber-1;
          meshi = global_scase.meshescoll.meshinfo + firei->roomnumber-1;
          diameter = SCALE2SMV(zonefdiambase[i]);
          deltaz = SCALE2SMV(zonefbasebase[i]);
          maxheight=roomi->z1-roomi->z0-deltaz;
          flameheight = SCALE2SMV(zonefheightbase[i]);
          SetClipPlanes(meshi->box_clipinfo,CLIP_ON);
          glPushMatrix();
          glTranslatef(firei->absx,firei->absy,roomi->z0+deltaz);
          DrawZoneFirePlume(diameter,flameheight,maxheight);
          glPopMatrix();
          SetClipPlanes(meshi->box_clipinfo,CLIP_OFF);
        }
      }
      else{
        if(qdot>0.0f){
          firedata *firei;
          roomdata *roomi;
          meshdata *meshi;

          // radius/plumeheight = .268 = atan(15 degrees)
          firei = global_scase.fireinfo + i;
          roomi = global_scase.roominfo + firei->roomnumber-1;
          meshi = global_scase.meshescoll.meshinfo + firei->roomnumber-1;
          maxheight=roomi->z1-firei->absz;
          flameheight = SCALE2SMV((0.23f*pow((double)qdot,(double)0.4)/(1.0f+2.0f*0.268f)));
          diameter = 2.0*flameheight*0.268f;
          SetClipPlanes(meshi->box_clipinfo,CLIP_ON);
          glPushMatrix();
          glTranslatef(firei->absx,firei->absy,firei->absz);
          DrawZoneFirePlume(diameter,flameheight,maxheight);
          glPopMatrix();
          SetClipPlanes(meshi->box_clipinfo,CLIP_OFF);
        }
      }
    }
  }
  if(use_transparency_data==1)TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
}

/* ------------------ DrawZoneRoomData ------------------------ */

void DrawZoneRoomData(void){
  float xroom0, yroom0, zroom0, xroom, yroom, zroom;
  float *zoneylaybase,dx,dy;
  unsigned char *hazardcolorbase, *zonecolorbaseU=NULL;
  unsigned char *zonecolorbaseL=NULL;
  float ylay;
  float *colorvU;
  unsigned char *izonetubase;
  unsigned char *izonetlbase;
  float *colorvL;
  int i;

  if(zone_times[0]>global_times[itimes])return;

  if(cullfaces==1)glDisable(GL_CULL_FACE);
  if(use_transparency_data==1)TransparentOn();

  izonetubase = izonetu + izone*global_scase.nrooms;
  izonetlbase = izonetl + izone*global_scase.nrooms;
  hazardcolorbase = hazardcolor + izone*global_scase.nrooms;
  zoneylaybase = zoneylay + izone*global_scase.nrooms;

  if(zonecolortype==ZONEHAZARD_COLOR){
    zonecolorbaseU = hazardcolorbase;
    zonecolorbaseL = hazardcolorbase;
  }
  else{
    zonecolorbaseU=izonetubase;
    zonecolorbaseL = izonetlbase;
  }

#ifdef pp_GPU
   if(usegpu==1){
     LoadZoneSmokeShaders();
   }
#endif
  for(i=0;i<global_scase.nrooms;i++){
    roomdata *roomi;
    unsigned char colorU;
    unsigned char colorL;

    roomi = global_scase.roominfo + i;

    ylay = *(zoneylaybase+i);
    colorU = *(zonecolorbaseU+i);
    if(zonecolortype==ZONEHAZARD_COLOR){
      colorvU = rgbhazard[colorU];
      colorvL = rgbhazard[colorU];
    }
    else{
      colorvU = rgb_full[colorU];
      colorL = *(zonecolorbaseL+i);
      colorvL = rgb_full[colorL];
    }
    xroom0 = roomi->x0;
    yroom0 = roomi->y0;
    xroom = roomi->x1;
    yroom = roomi->y1;
    zroom0 = roomi->z0;
    zroom = roomi->z1;
    dx = roomi->dx/2.;
    dy = roomi->dy/2.;

    if(zonecolortype==ZONESMOKE_COLOR&&visSZone==1){
#ifdef pp_GPU
      if(usegpu==1){
        DrawZoneSmokeGpu(roomi);
      }
      else{
        DrawZoneSmoke(roomi);
      }
#else
      DrawZoneSmoke(roomi);
#endif
    }
    else{
      if(visZonePlane==ZONE_ZPLANE){
        glColor4fv(colorvU);
        glBegin(GL_QUADS);
        glVertex3f(xroom0,yroom0,ylay+zroom0);
        glVertex3f(xroom,yroom0,ylay+zroom0);
        glVertex3f(xroom,yroom,ylay+zroom0);
        glVertex3f(xroom0,yroom,ylay+zroom0);
        glEnd();
      }
      if(visZonePlane == ZONE_YPLANE){
        glBegin(GL_QUADS);
        glColor4fv(colorvU);
        glVertex3f(xroom0, yroom0+dy,ylay+zroom0);
        glVertex3f(xroom,  yroom0+dy,ylay+zroom0);
        glVertex3f(xroom,  yroom0+dy,zroom);
        glVertex3f(xroom0, yroom0+dy,zroom);

        if(show_zonelower == 1&&zonecolortype!=ZONEHAZARD_COLOR){
          glColor4fv(colorvL);
          glVertex3f(xroom0, yroom0 + dy, zroom0);
          glVertex3f(xroom,  yroom0 + dy, zroom0);
          glVertex3f(xroom,  yroom0 + dy, zroom0 + ylay);
          glVertex3f(xroom0, yroom0 + dy, zroom0 + ylay);
        }
        glEnd();
      }
      if(visZonePlane == ZONE_XPLANE){
        glBegin(GL_QUADS);
        glColor4fv(colorvU);
        glVertex3f(xroom0+dx, yroom0, ylay + zroom0);
        glVertex3f(xroom0+dx, yroom,  ylay + zroom0);
        glVertex3f(xroom0+dx, yroom,  zroom);
        glVertex3f(xroom0+dx, yroom0, zroom);

        if(show_zonelower == 1 && zonecolortype != ZONEHAZARD_COLOR){
          glColor4fv(colorvL);
          glVertex3f(xroom0+dx, yroom0, zroom0);
          glVertex3f(xroom0+dx, yroom,  zroom0);
          glVertex3f(xroom0+dx, yroom,  zroom0+ylay);
          glVertex3f(xroom0+dx, yroom0, zroom0+ylay);
        }
        glEnd();
      }
    }
  }
#ifdef pp_GPU
  if(usegpu==1){
    UnLoadShaders();
  }
#endif

  if(use_transparency_data==1)TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
}
