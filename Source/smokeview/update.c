#define IN_UPDATE
#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "compress.h"
#include "IOscript.h"
#include "glui_smoke.h"
#include "glui_motion.h"
#include "glui_wui.h"

/* ------------------ CompareFloat ------------------------ */

int CompareFloat( const void *arg1, const void *arg2 ){
  float x, y;

  x=*(float *)arg1;
  y=*(float *)arg2;
  if( x< y)return -1;
  if( x> y)return 1;
  return 0;
}

/* ------------------ UpdateHRRInfo ------------------------ */

void UpdateHRRInfo(int vis){
  if(hrrinfo!=NULL&&hrrinfo->loaded==1){
    hrrinfo->display = vis;
    if(hrrinfo->display == 0)show_hrrpuv_plot = 0;
  }
  if(visHRRlabel == 0)show_hrrpuv_plot=0;

  UpdateShowHRRPUVPlot(show_hrrpuv_plot);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  UpdateShow();
  update_times = 1;
}

/* ------------------ UpdateFrameNumber ------------------------ */

void UpdateFrameNumber(int changetime){
  if(force_redisplay==1||(itimeold!=itimes&&changetime==1)){
    int i;

    force_redisplay=0;
    itimeold=itimes;
    if(showsmoke==1||showevac==1){
      for(i=0;i<npartinfo;i++){
        partdata *parti;

        parti = partinfo+i;
        if(parti->loaded==0||parti->timeslist==NULL)continue;
        parti->itime=parti->timeslist[itimes];
      }
    }
    if(hrrinfo!=NULL&&hrrinfo->loaded==1&&hrrinfo->display==1&&hrrinfo->timeslist!=NULL){
      hrrinfo->itime=hrrinfo->timeslist[itimes];
    }
    if(showvolrender==1){
      int imesh;

      for(imesh=0;imesh<nmeshes;imesh++){
        meshdata *meshi;
        volrenderdata *vr;
        slicedata *fireslice, *smokeslice, *lightslice;
        int j;

        meshi = meshinfo + imesh;
        vr = &(meshi->volrenderinfo);
        fireslice=vr->fireslice;
        smokeslice=vr->smokeslice;
        lightslice=vr->lightslice;
        if(fireslice==NULL||smokeslice==NULL)continue;
        if(vr->loaded==0||vr->display==0)continue;
        vr->itime = vr->timeslist[itimes];
        for(j=vr->itime;j>=0;j--){
          if(vr->dataready[j]==1)break;
        }
        vr->itime=j;

        if(smokeslice!=NULL&&vr->itime>=0){
          if(vr->is_compressed==1||load_volcompressed==1){
            unsigned char *c_smokedata_compressed;
            uLongf framesize;
            float timeval;

            c_smokedata_compressed = vr->smokedataptrs[vr->itime];
            framesize = smokeslice->nslicei*smokeslice->nslicej*smokeslice->nslicek;
            UnCompressVolSliceFrame(c_smokedata_compressed,
                           vr->smokedata_view, framesize, &timeval,
                           vr->c_smokedata_view);

            vr->smokedataptr = vr->smokedata_view;
          }
          else{
            if(runscript==0)vr->smokedataptr = vr->smokedataptrs[vr->itime];
          }
          CheckMemory;
        }

        if(fireslice!=NULL&&vr->itime>=0){
          if(vr->is_compressed==1||load_volcompressed==1){
            unsigned char *c_firedata_compressed;
            uLongf framesize;
            float timeval;

            c_firedata_compressed = vr->firedataptrs[vr->itime];
            framesize = fireslice->nslicei*fireslice->nslicej*fireslice->nslicek;
            UnCompressVolSliceFrame(c_firedata_compressed,
                           vr->firedata_view, framesize, &timeval,
                           vr->c_firedata_view);

            vr->firedataptr = vr->firedata_view;
            CheckMemory;
          }
          else{
            if(runscript==0)vr->firedataptr = vr->firedataptrs[vr->itime];
          }
          CheckMemory;
        }

        if(lightslice!=NULL&&vr->itime>=0){
          if(vr->is_compressed==1||load_volcompressed==1){
            unsigned char *c_lightdata_compressed;
            uLongf framesize;
            float timeval;

            c_lightdata_compressed = vr->lightdataptrs[vr->itime];
            framesize = lightslice->nslicei*lightslice->nslicej*lightslice->nslicek;
            UnCompressVolSliceFrame(c_lightdata_compressed,
                           vr->lightdata_view, framesize, &timeval,
                           vr->c_lightdata_view);

            vr->lightdataptr = vr->lightdata_view;
            CheckMemory;
          }
          else{
            if(runscript==0)vr->lightdataptr = vr->lightdataptrs[vr->itime];
          }
          CheckMemory;
        }
      }
    }
    for(i=0;i<ngeominfoptrs;i++){
      geomdata *geomi;

      geomi = geominfoptrs[i];
      if(geomi->loaded==0||geomi->timeslist==NULL)continue;
      geomi->itime=geomi->timeslist[itimes];
    }
    if(showslice==1||showvslice==1){
      int ii;

      for(ii=0;ii<nslice_loaded;ii++){
        slicedata *sd;

        i = slice_loaded_list[ii];
        sd = sliceinfo+i;
        if(sd->slice_filetype == SLICE_GEOM){
          patchdata *patchi;

          patchi = sd->patchgeom;
          if(patchi->geom_timeslist == NULL)continue;
          if(patchi->structured == YES || patchi->boundary == 1 || patchi->geom_times == NULL || patchi->geom_timeslist == NULL)continue;
          if(current_script_command!=NULL && current_script_command->command == SCRIPT_LOADSLICERENDER){
            patchi->geom_itime = 0; // only one frame loaded at a time when using LOADSLICERNDER
          }
          else{
            patchi->geom_itime = patchi->geom_timeslist[itimes];
          }
          patchi->geom_val_static   = patchi->geom_vals_static[patchi->geom_itime];
          patchi->geom_ival_static  = patchi->geom_ivals_static[patchi->geom_itime];
          patchi->geom_ival_dynamic = patchi->geom_ivals_dynamic[patchi->geom_itime];
          patchi->geom_val_static   = patchi->geom_vals_static[patchi->geom_itime];
          patchi->geom_val_dynamic  = patchi->geom_vals_dynamic[patchi->geom_itime];
          patchi->geom_nval_static  = patchi->geom_nstatics[patchi->geom_itime];
          patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
          sd->itime                 = patchi->geom_timeslist[itimes];
          slice_time                = sd->itime;
        }
        else{
          if(sd->timeslist == NULL)continue;
          sd->itime = sd->timeslist[itimes];
          slice_time = sd->itime;
        }
      }
      for (i = 0; i < npatchinfo; i++) {
        patchdata *patchi;

        patchi = patchinfo + i;
        if(patchi->structured == YES || patchi->boundary == 1 || patchi->geom_times == NULL || patchi->geom_timeslist == NULL)continue;
        patchi->geom_itime = patchi->geom_timeslist[itimes];
        patchi->geom_ival_static = patchi->geom_ivals_static[patchi->geom_itime];
        patchi->geom_ival_dynamic = patchi->geom_ivals_dynamic[patchi->geom_itime];
        patchi->geom_nval_static = patchi->geom_nstatics[patchi->geom_itime];
        patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
      }
    }
    if(show3dsmoke==1){
      if(nsmoke3dinfo > 0){
        for(i = 0;i < nsmoke3dinfo;i++){
          smoke3ddata *smoke3di;

          smoke3di = smoke3dinfo + i;
          if(smoke3di->loaded == 0 || smoke3di->display == 0)continue;
          smoke3di->ismoke3d_time = smoke3di->timeslist[itimes];
          if(IsSmokeComponentPresent(smoke3di) == 0)continue;
          if(smoke3di->ismoke3d_time != smoke3di->lastiframe){
            smoke3di->lastiframe = smoke3di->ismoke3d_time;
            UpdateSmoke3D(smoke3di);
          }
        }
        if(use_newsmoke==SMOKE3D_ORIG||use_newsmoke==SMOKE3D_NEW){
          MergeSmoke3D(NULL);
          PrintMemoryInfo;
        }
      }
    }
    if(showpatch==1){
      for(i=0;i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo + i;
        if(patchi->structured == YES||patchi->boundary==0||patchi->geom_times==NULL||patchi->geom_timeslist==NULL)continue;
        patchi->geom_itime=patchi->geom_timeslist[itimes];
        patchi->geom_ival_static = patchi->geom_ivals_static[patchi->geom_itime];
        patchi->geom_ival_dynamic = patchi->geom_ivals_dynamic[patchi->geom_itime];
        patchi->geom_nval_static = patchi->geom_nstatics[patchi->geom_itime];
        patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
      }
      for(i=0;i<nmeshes;i++){
        patchdata *patchi;
        meshdata *meshi;

        meshi = meshinfo+i;
        if(meshi->patchfilenum < 0||meshi->patchfilenum>npatchinfo-1)continue;
        patchi=patchinfo + meshi->patchfilenum;
        if(patchi->structured == NO||meshi->patch_times==NULL||meshi->patch_timeslist==NULL)continue;
        meshi->patch_itime=meshi->patch_timeslist[itimes];
        if(patchi->compression_type==UNCOMPRESSED){
          meshi->cpatchval_iframe = meshi->cpatchval + meshi->patch_itime*meshi->npatchsize;
        }
        else{
          UncompressBoundaryDataFrame(meshi,meshi->patch_itime);
        }
      }
    }
    if(showiso==1){
      isodata *isoi;
      meshdata *meshi;

      CheckMemory;
      for(i=0;i<nisoinfo;i++){
        isoi = isoinfo + i;
        meshi = meshinfo + isoi->blocknumber;
        if(isoi->loaded==0||meshi->iso_times==NULL||meshi->iso_timeslist==NULL)continue;
        meshi->iso_itime=meshi->iso_timeslist[itimes];
      }
    }
    if(showzone==1){
      izone=zone_timeslist[itimes];
    }
  }
}

/* ------------------ UpdateFileLoad  ------------------------ */

void UpdateFileLoad(void){
  int i;

// remove following directive when update_fileload has been set everywhere that a file has been loaded or unloaded
#ifdef pp_UPDATE_FILELOAD
  update_fileload = 0;
#endif
  npartloaded = 0;
  nevacloaded = 0;
  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(parti->loaded==1&&parti->evac==0)npartloaded++;
    if(parti->loaded==1&&parti->evac==1)nevacloaded++;
  }

  nsliceloaded = 0;
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(slicei->loaded==1)nsliceloaded++;
  }

  nvsliceloaded = 0;
  for(i = 0; i<nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = vsliceinfo+i;
    if(vslicei->loaded==1)nvsliceloaded++;
  }

  nisoloaded = 0;
  for(i = 0; i<nisoinfo; i++){
    isodata *isoi;

    isoi = isoinfo+i;
    if(isoi->loaded==1)nisoloaded++;
  }

  npatchloaded = 0;
  for(i = 0; i<npatchinfo; i++){
    patchdata *patchi;

    patchi = patchinfo+i;
    if(patchi->loaded==1)npatchloaded++;
  }

  nsmoke3dloaded = 0;
  for(i = 0; i<nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo+i;
    if(smoke3di->loaded==1)nsmoke3dloaded++;
  }

  nplot3dloaded = 0;
  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    if(plot3di->loaded==1)nplot3dloaded++;
  }

  nvolsmoke3dloaded = 0;
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = meshinfo+i;
    vr = &(meshi->volrenderinfo);
    if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
    if(vr->loaded==1)nvolsmoke3dloaded++;
  }

  npart5loaded = 0;
  npartloaded = 0;
  nevacloaded = 0;
  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(parti->loaded==1&&parti->evac==0)npartloaded++;
    if(parti->loaded==1&&parti->evac==1)nevacloaded++;
    if(parti->loaded==1)npart5loaded++;
  }
}

/* ------------------ UpdateShow ------------------------ */

void UpdateShow(void){
  int i,evacflag,sliceflag,vsliceflag,partflag,patchflag,isoflag,smoke3dflag,tisoflag,showdeviceflag;
  int slicecolorbarflag;
  int shooter_flag;

  if(update_fileload==1)UpdateFileLoad();
  have_fire = HaveFire();
  showtime=0;
  showtime2=0;
  showplot3d=0;
  showpatch=0;
  showslice=0;
  showvslice=0;
  showsmoke=0;
  showzone=0;
  showiso=0;
  showvolrender=0;
  have_extreme_mindata=0;
  have_extreme_maxdata=0;
  showshooter=0;
  showevac=0;
  showevac_colorbar=0;
  show3dsmoke=0;
  smoke3dflag=0;
  showtours=0;
  showdeviceflag = 0;
  visTimeParticles=1; visTimeSlice=1; visTimeBoundary=1; visTimeZone=1; visTimeIso=1;

  drawing_boundary_files = 0;

  RenderTime=0;
  if(global_times!=NULL){
    if(settmin_p==1&&global_times[itimes]<tmin_p)visTimeParticles=0;
    if(settmax_p==1&&global_times[itimes]>tmax_p)visTimeParticles=0;

    if(settmin_s==1&&global_times[itimes]<tmin_s)visTimeSlice=0;
    if(settmax_s==1&&global_times[itimes]>tmax_s)visTimeSlice=0;

    if(settmin_i==1&&global_times[itimes]<tmin_i)visTimeIso=0;
    if(settmax_i==1&&global_times[itimes]>tmax_i)visTimeIso=0;

    if(settmin_b==1&&global_times[itimes]<tmin_b)visTimeBoundary=0;
    if(settmax_b==1&&global_times[itimes]>tmax_b)visTimeBoundary=0;

    if(settmin_z==1&&global_times[itimes]<tmin_z)visTimeZone=0;
    if(settmax_z==1&&global_times[itimes]>tmax_z)visTimeZone=0;
  }
  if(visHRRlabel==1&&show_hrrpuv_plot==1&&hrrinfo!=NULL){
    showdeviceflag = 1;
  }
  if(showdevice_val==1||showdevice_plot!=DEVICE_PLOT_HIDDEN){
    for(i = 0; i<ndeviceinfo; i++){
      devicedata *devicei;

      devicei = deviceinfo+i;
      if(devicei->type2==devicetypes_index&&devicei->object->visible==1){
        showdeviceflag = 1;
        break;
      }
    }
  }

  {
    tourdata *touri;

    if(ntourinfo>0){
      for(i=0;i<ntourinfo;i++){
        touri = tourinfo + i;
        if(touri->display==1){
          showtours=1;
          break;
        }
      }
    }
  }
  {
    int ii;

    for(ii=0;ii<nsmoke3dinfo;ii++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + ii;
      if(smoke3di->loaded==1&&smoke3di->display==1){
        smoke3dflag = 1;
        break;
      }
    }
  }
  if(nvolrenderinfo>0&&usevolrender==1){
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = meshinfo + i;
      vr = &(meshi->volrenderinfo);
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      showvolrender=1;
      break;
    }
  }

  sliceflag=0;
  slicecolorbarflag=0;
  SHOW_gslice_data=0;
  if(visTimeSlice==1){
    int ii;

    for(ii=0;ii<nslice_loaded;ii++){
      slicedata *sd;

      i=slice_loaded_list[ii];
      sd = sliceinfo+i;
      if(sd->display==0||sd->slicefile_labelindex!=slicefile_labelindex)continue;
      if(sd->volslice==1&&sd->slice_filetype==SLICE_NODE_CENTER&&vis_gslice_data==1)SHOW_gslice_data=1;
      if(sd->ntimes>0){
        sliceflag=1;
        break;
      }
    }
    if(SHOW_gslice_data!=SHOW_gslice_data_old){
      SHOW_gslice_data_old=SHOW_gslice_data;
      updatemenu=1;
    }
    for(ii=0;ii<nslice_loaded;ii++){
      meshdata *slicemesh;
      slicedata *sd;

      i=slice_loaded_list[ii];
      sd = sliceinfo+i;
      slicemesh = meshinfo + sd->blocknumber;
      if(sd->display==0||sd->slicefile_labelindex!=slicefile_labelindex)continue;
      if(sd->constant_color==NULL&&show_evac_colorbar==0&&slicemesh->mesh_type!=0)continue;
      if(sd->constant_color!=NULL)continue;
      if(sd->ntimes>0){
        slicecolorbarflag=1;
        break;
      }
    }
    if(have_extreme_maxdata==0){
      for(ii=0;ii<nslice_loaded;ii++){
        slicedata *sd;

        i=slice_loaded_list[ii];
        sd = sliceinfo+i;
        if(sd->display==0||sd->slicefile_labelindex!=slicefile_labelindex)continue;
        if(sd->extreme_max==1){
          have_extreme_maxdata=1;
          break;
        }
      }
    }
    if(have_extreme_mindata==0){
      for(ii=0;ii<nslice_loaded;ii++){
        slicedata *sd;

        i=slice_loaded_list[ii];
        sd = sliceinfo+i;
        if(sd->display==0||sd->slicefile_labelindex!=slicefile_labelindex)continue;
        if(sd->extreme_min==1){
          have_extreme_mindata=1;
          break;
        }
      }
    }
    for(ii=0;ii<npatch_loaded;ii++){
      patchdata *patchi;

      i = patch_loaded_list[ii];
      patchi=patchinfo+i;
      if(patchi->boundary == 0 && patchi->display == 1 && patchi->shortlabel_index == slicefile_labelindex){
        sliceflag = 1;
        slicecolorbarflag = 1;
        break;
       // if(patchi->extreme_max == 1)have_extreme_maxdata = 1;
       // if(patchi->extreme_min == 1)have_extreme_mindata = 1;
       // if(patchi->geominfo != NULL)patchi->geominfo->patchactive = 1;
      }
    }
  }

  isoflag=0;
  tisoflag=0;
  if(visTimeIso==1){
    for(i=0;i<nisoinfo;i++){
      isodata *isoi;

      isoi = isoinfo+i;
      if(isoi->loaded==0)continue;
      if(isoi->display==1&&isoi->type==iisotype){
        isoflag=1;
        if(isoi->dataflag==1){
          tisoflag=1;
          break;
        }
      }
    }
  }

  vsliceflag=0;
  vslicecolorbarflag=0;
  if(visTimeSlice==1){
    for(i=0;i<nvsliceinfo;i++){
      vslicedata *vd;
      slicedata *sd;

      vd = vsliceinfo+i;
      if(vd->loaded==0||vd->display==0)continue;
      sd = sliceinfo + vd->ival;

      if(sd->slicefile_labelindex!=slicefile_labelindex)continue;
      if(sd->volslice==1&&sd->slice_filetype==SLICE_NODE_CENTER&&vis_gslice_data==1)SHOW_gslice_data=1;
      vsliceflag=1;
      break;
    }
    for(i=0;i<nvsliceinfo;i++){
      meshdata *slicemesh;
      slicedata *sd;
      vslicedata *vd;

      vd = vsliceinfo+i;
      sd = sliceinfo + vd->ival;
      slicemesh = meshinfo + sd->blocknumber;
      if(vd->loaded==0||vd->display==0)continue;
      if(sliceinfo[vd->ival].slicefile_labelindex!=slicefile_labelindex)continue;
      if(sd->constant_color==NULL&&show_evac_colorbar==0&&slicemesh->mesh_type!=0)continue;
      if(sd->constant_color!=NULL)continue;
      vslicecolorbarflag=1;
      break;
    }
  }

  patchflag=0;
  if(visTimeBoundary==1){
    int ii;

    for (i = 0; i < ngeominfo; i++) {
      geomdata *geomi;

      geomi = geominfo + i;
      geomi->patchactive = 0;
    }
    wall_cell_color_flag=0;
    for(ii=0;ii<npatch_loaded;ii++){
      patchdata *patchi;

      i = patch_loaded_list[ii];
      patchi=patchinfo+i;
      if(patchi->boundary == 1 && patchi->display == 1 && patchi->shortlabel_index == iboundarytype){
        if (strcmp(patchi->label.shortlabel, "wc") == 0)wall_cell_color_flag = 1;
        patchflag = 1;
        if(patchi->extreme_max == 1)have_extreme_maxdata = 1;
        if(patchi->extreme_min == 1)have_extreme_mindata = 1;
        if(patchi->geominfo != NULL)patchi->geominfo->patchactive = 1;
      }
    }
  }

  partflag=0;
  if(visParticles==1&&visTimeParticles==1){
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;
      if(parti->evac==0&&parti->loaded==1&&parti->display==1){
        partflag=1;
        break;
      }
    }
    if(current_property!=NULL){
      if(current_property->extreme_max==1)have_extreme_maxdata=1;
      if(current_property->extreme_min==1)have_extreme_mindata=1;
    }
  }
  ReadPartFile = partflag;

  evacflag=0;
  if(visEvac==1&&visTimeEvac==1){
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;
      if(parti->evac==1&&parti->loaded==1&&parti->display==1){
        evacflag=1;
        break;
      }
    }
  }

  shooter_flag=0;
  if(visShooter!=0&&shooter_active==1){
    shooter_flag=1;
  }

  if( plotstate==DYNAMIC_PLOTS &&
    ( showdeviceflag==1 || sliceflag==1 || vsliceflag==1 || partflag==1 || patchflag==1 ||
    shooter_flag==1|| smoke3dflag==1 || showtours==1 || evacflag==1 ||
    (ReadZoneFile==1&&visZone==1&&visTimeZone==1)||showvolrender==1
    )
    )showtime=1;
  if(plotstate==DYNAMIC_PLOTS&&ReadIsoFile==1&&visAIso!=0&&isoflag==1)showtime2=1;
  if(plotstate==DYNAMIC_PLOTS){
    if(smoke3dflag==1)show3dsmoke=1;
    if(partflag==1)showsmoke=1;
    if(evacflag==1)showevac=1;
    if(showevac==1&&parttype>0){
      showevac_colorbar=1;
      if(current_property!=NULL&&strcmp(current_property->label->longlabel,"HUMAN_COLOR")==0){
        showevac_colorbar=0;
      }
    }
    if(patchflag==1)showpatch=1;
    drawing_boundary_files = showpatch;

    for(i=0;i<nmeshes;i++){
      meshdata *meshi;

      meshi=meshinfo+i;
      meshi->visInteriorBoundaries=0;
    }
    if(showpatch==1&&vis_boundary_type[0]==1){
      for(i=0;i<nmeshes;i++){
        patchdata *patchi;
        meshdata *meshi;

        meshi=meshinfo+i;
        if(meshi->patch_times==NULL)continue;
        patchi = patchinfo+meshi->patchfilenum;
        if(patchi->loaded==1&&patchi->display==1&&patchi->shortlabel_index ==iboundarytype){
          meshi->visInteriorBoundaries=1;
        }
      }
    }
    if(sliceflag==1)showslice=1;
    if(vsliceflag==1)showvslice=1;
    if(ReadZoneFile==1&&visZone==1&&visTimeZone==1)showzone=1;
    if(ReadIsoFile==1&&visAIso!=0){
      showiso=1;
    }
    if(shooter_flag==1)showshooter=1;
  }
  if(showsmoke==1||showevac==1||showpatch==1||showslice==1||showvslice==1||showzone==1||showiso==1||showevac==1)RenderTime=1;
  if(showtours==1||show3dsmoke==1||touring==1||showvolrender==1)RenderTime=1;
  if(showshooter==1)RenderTime=1;
  if(plotstate==STATIC_PLOTS&&nplot3dloaded>0&&plotn>0&&plotn<=numplot3dvars)showplot3d=1;
  if(showplot3d==1){
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      int ii;

      meshi=meshinfo+i;
      ii=meshi->plot3dfilenum;
      if(ii==-1)continue;
      if(plot3dinfo[ii].loaded==0)continue;
      if(plot3dinfo[ii].display==0)continue;
      if(plot3dinfo[ii].extreme_min[plotn-1]==1)have_extreme_mindata=1;
    }
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      int ii;

      meshi=meshinfo+i;
      ii=meshi->plot3dfilenum;
      if(ii==-1)continue;
      if(plot3dinfo[ii].loaded==0)continue;
      if(plot3dinfo[ii].display==0)continue;
      if(plot3dinfo[ii].extreme_max[plotn-1]==1)have_extreme_maxdata=1;
    }
  }

  num_colorbars=0;
  if(plotstate==DYNAMIC_PLOTS){
    if(evacflag==1||(partflag==1&&parttype!=0))num_colorbars++;
    if(slicecolorbarflag==1||vslicecolorbarflag==1)num_colorbars++;
    if(patchflag==1&&wall_cell_color_flag==0)num_colorbars++;
    if(ReadZoneFile==1)num_colorbars++;
    if(tisoflag==1&&1==0){ // disable isosurface colorbar label for now
      showiso_colorbar = 1;
      num_colorbars++;
    }
  }
  if(nplot3dloaded>0&&num_colorbars==0)num_colorbars=1;

  // note: animated iso-contours do not need a colorbar, so we don't test for isosurface files

  if ((showtime == 1 || showplot3d == 1) && (visColorbarVertical == 1|| visColorbarHorizontal == 1)) {
    if(old_draw_colorlabel == 0)updatemenu = 1;
    old_draw_colorlabel = 1;
  }
  else {
    if(old_draw_colorlabel == 1)updatemenu = 1;
    old_draw_colorlabel = 0;
  }

  if(showtime2==1)showtime=1;
  if(use_graphics==1){
    if(plotstate==DYNAMIC_PLOTS&&stept==1){
      glutIdleFunc(IdleCB);
    }
    else{
      glutIdleFunc(NULL);
    }
  }
}

/* ------------------ GetItime ------------------------ */

int GetItime(int n, int *timeslist, float *times, int ntimes){
  int istart=0;

  if(n>0)istart=timeslist[n-1];
  while(istart<ntimes-1&&times[istart+1]<=global_times[n]){
    istart++;
  }
  istart=CLAMP(istart,0,ntimes-1);
  return istart;
}

/* ------------------ SynchTimes ------------------------ */

void SynchTimes(void){
  int n,i,istart,igrid;

  /* synchronize smooth blockage times */

  for(n=0;n<nglobal_times;n++){
    int j,jj;

  /* synchronize tour times */

    for(j=0;j<ntourinfo;j++){
      tourdata *tourj;

      tourj = tourinfo + j;
      if(tourj->display==0)continue;
      tourj->timeslist[n]=GetItime(n,tourj->timeslist,tourj->path_times,tourj->ntimes);
    }

    /* synchronize hrrpuv times */

    if(hrrinfo!=NULL&&hrrinfo->loaded==1&&hrrinfo->display==1){
      hrrinfo->timeslist[n]=GetItime(n,hrrinfo->timeslist,hrrinfo->times,hrrinfo->ntimes);
    }

  /* synchronize geometry times */

    for(j=0;j<ngeominfoptrs;j++){
      geomdata *geomi;

      geomi = geominfoptrs[j];
      if(geomi->loaded==0||geomi->display==0)continue;
      geomi->timeslist[n]=GetItime(n,geomi->timeslist,geomi->times,geomi->ntimes);
    }

  /* synchronize particle times */

    for(j=0;j<npartinfo;j++){
      partdata *parti;

      parti=partinfo+j;
      if(parti->loaded==0)continue;
      parti->timeslist[n]=GetItime(n,parti->timeslist,parti->times,parti->ntimes);
    }

  /* synchronize shooter times */
    if(visShooter!=0&&shooter_active==1){
      if(n==0){
        istart=0;
      }
      else{
        istart=shooter_timeslist[n-1];
      }
      i=istart;
      while(shoottimeinfo[i].time<global_times[n]&&i<nshooter_frames){
        i++;
      }
      if(i>=nshooter_frames){
        i=nshooter_frames-1;
      }
      shooter_timeslist[n]=i;
    }

  /* synchronize slice times */

    for(jj=0;jj<nslice_loaded;jj++){
      slicedata *sd;

      sd = sliceinfo + slice_loaded_list[jj];
      if(sd->slice_filetype == SLICE_GEOM){
        sd->patchgeom->geom_timeslist[n] = GetItime(n, sd->patchgeom->geom_timeslist, sd->patchgeom->geom_times, sd->ntimes);
      }
      else{
        sd->timeslist[n] = GetItime(n, sd->timeslist, sd->times, sd->ntimes);
      }
    }

  /* synchronize smoke times */
    {
      smoke3ddata *smoke3di;

      for(jj=0;jj<nsmoke3dinfo;jj++){
        smoke3di = smoke3dinfo + jj;
        if(smoke3di->loaded==0)continue;
        smoke3di->timeslist[n]=GetItime(n,smoke3di->timeslist,smoke3di->times,smoke3di->ntimes);
      }
    }

  /* synchronize patch times */

    for(j=0;j<npatchinfo;j++){
      patchdata *patchi;

      patchi = patchinfo + j;
      if(patchi->loaded==0)continue;
      if(patchi->structured == YES)continue;
      patchi->geom_timeslist[n]=GetItime(n,patchi->geom_timeslist,patchi->geom_times,patchi->ngeom_times);
    }
    for(j=0;j<nmeshes;j++){
      patchdata *patchi;
      meshdata *meshi;

      meshi=meshinfo+j;
      if(meshi->patchfilenum<0||meshi->patch_times==NULL)continue;
      patchi=patchinfo+meshi->patchfilenum;
      if(patchi->structured == NO)continue;
      meshi->patch_timeslist[n]=GetItime(n,meshi->patch_timeslist,meshi->patch_times,meshi->npatch_times);
    }

  /* synchronize isosurface times */

    for(igrid=0;igrid<nmeshes;igrid++){
      meshdata *meshi;

      meshi=meshinfo+igrid;
      if(meshi->iso_times==NULL)continue;
      meshi->iso_timeslist[n]=GetItime(n,meshi->iso_timeslist,meshi->iso_times,meshi->niso_times);
    }

  /* synchronize volume render times */

    if(nvolrenderinfo>0){
      for(igrid=0;igrid<nmeshes;igrid++){
        volrenderdata *vr;
        meshdata *meshi;

        meshi=meshinfo+igrid;
        vr = &meshi->volrenderinfo;
        if(vr->smokeslice==NULL)continue;
        if(vr->loaded==0||vr->display==0)continue;
        if(vr->times==NULL)continue;
        vr->timeslist[n]=GetItime(n,vr->timeslist,vr->times,vr->ntimes);
      }
    }
    /* synchronize zone times */

    if(showzone==1){
      zone_timeslist[n]=GetItime(n,zone_timeslist,zone_times,nzone_times);
    }

  }
  ResetGLTime();
}

/* ------------------ GetLoadvfileinfo ------------------------ */

int GetLoadvfileinfo(FILE *stream, char *filename){
  int i;
  char *fileptr;

  TrimBack(filename);
  fileptr = TrimFront(filename);
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(strcmp(fileptr, slicei->file)==0){
      fprintf(stream, "// LOADVFILE\n");
      fprintf(stream, "//  %s\n", slicei->file);
      fprintf(stream, "LOADVSLICEM\n");
      fprintf(stream, " %s\n", slicei->label.longlabel);
      if(slicei->volslice==1){
        fprintf(stream, " %i %f\n", 0, slicei->position_orig);
      }
      else{
        fprintf(stream, " %i %f\n", slicei->idir, slicei->position_orig);
      }
      fprintf(stream, " %i\n", slicei->blocknumber+1);
      return 1;
    }
  }
  return 0;
}

/* ------------------ GetLoadfileinfo ------------------------ */

int GetLoadfileinfo(FILE *stream, char *filename){
  int i;
  char *fileptr;

  TrimBack(filename);
  fileptr = TrimFront(filename);
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(strcmp(fileptr, slicei->file)==0){
      fprintf(stream, "// LOADFILE\n");
      fprintf(stream, "//  %s\n", slicei->file);
      fprintf(stream, "LOADSLICEM\n");
      fprintf(stream, " %s\n", slicei->label.longlabel);
      if(slicei->volslice==1){
        fprintf(stream, " %i %f\n", 0, slicei->position_orig);
      }
      else{
        fprintf(stream, " %i %f\n", slicei->idir, slicei->position_orig);
      }
      fprintf(stream, " %i\n", slicei->blocknumber+1);
      return 1;
    }
  }
  for(i = 0; i < nisoinfo; i++){
    isodata *isoi;

    isoi = isoinfo + i;
    if(strcmp(fileptr, isoi->file) == 0){
      fprintf(stream, "// LOADFILE\n");
      fprintf(stream, "//  %s\n", isoi->file);
      fprintf(stream, "LOADISOM\n");
      fprintf(stream, " %s\n", isoi->surface_label.longlabel);
      fprintf(stream, " %i\n", isoi->blocknumber+1);
      return 1;
    }

  }
  for(i = 0; i < npatchinfo; i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(strcmp(fileptr, patchi->file) == 0){
      fprintf(stream, "// LOADFILE\n");
      fprintf(stream, "//  %s\n", patchi->file);
      fprintf(stream, "LOADBOUNDARYM\n");
      fprintf(stream, " %s\n", patchi->label.longlabel);
      fprintf(stream, " %i\n", patchi->blocknumber+1);
      return 1;
    }

  }
  return 0;
}

  /* ------------------ ConvertSsf ------------------------ */

void ConvertSsf(void){
  FILE *stream_from, *stream_to;
  int outeqin = 0;
#define LENTEMP 20
  char tempfile[LENTEMP+1];
  char *template = "tempssf";

  if(ssf_from==NULL||ssf_to==NULL)return;
  stream_from = fopen(ssf_from, "r");
  if(stream_from==NULL)return;

  if(strcmp(ssf_from, ssf_to)==0){
    strcpy(tempfile, template);
    if(RandStr(tempfile+strlen(template), LENTEMP-strlen(template))==NULL||strlen(tempfile)==0){
      fclose(stream_from);
      return;
    }
    stream_to = fopen(tempfile, "w");
    outeqin = 1;
  }
  else{
    stream_to = fopen(ssf_to, "w");
  }
  if(stream_to==NULL){
    fclose(stream_from);
    return;
  }

  while(!feof(stream_from)){
    char buffer[255], filename[255];

    CheckMemory;
    if(fgets(buffer, 255, stream_from)==NULL)break;
    TrimBack(buffer);
    if(strlen(buffer)>=8 && strncmp(buffer, "LOADFILE", 8)==0){
      if(fgets(filename, 255, stream_from)==NULL)break;
      if(GetLoadfileinfo(stream_to,filename)==0){
        fprintf(stream_to, "%s\n", buffer);
        fprintf(stream_to, "%s\n", filename);
      }
    }
    else if(strlen(buffer)>=9&&strncmp(buffer, "LOADVFILE", 9)==0){
      if(fgets(filename, 255, stream_from)==NULL)break;
      if(GetLoadvfileinfo(stream_to, filename)==0){
        fprintf(stream_to, "%s\n", buffer);
        fprintf(stream_to, "%s\n", filename);
      }
    }
    else{
      fprintf(stream_to, "%s\n", buffer);
    }
  }
  fclose(stream_from);
  fclose(stream_to);
  if(outeqin == 1){
    char *tofile;

    tofile = ssf_from;
    CopyFILE(".",tempfile,tofile,REPLACE_FILE);
    UNLINK(tempfile);
  }
}

  /* ------------------ MergeGlobalTimes ------------------------ */

void MergeGlobalTimes(float *time_in, int ntimes_in){
  int left, right, nbuffer, i;
  float dt_eps;

  if(ntimes_in<=0){
    nglobal_times = 0;
    return;
  }

  // when adding a time to current list assume it is already there if it closer than dt_es
  dt_eps = 0.001;
  for(i = 1; i<ntimes_in; i++){
    float dt;

    dt = time_in[i]-time_in[i-1];
    ASSERT(dt>=0.0);
    dt_eps = MIN(dt_eps, ABS(dt)/2.0);
  }
  if(nglobal_times>1){
    for(i = 1; i<nglobal_times; i++){
      float dt;

      dt = global_times[i]-global_times[i-1];
      ASSERT(dt>=0.0);
      dt_eps = MIN(dt_eps, ABS(dt)/2.0);
    }
  }

  // add time_in values to global_times the first time
  if(global_times==NULL || nglobal_times<=0){
    if(global_times==NULL){
      NewMemory((void **)&global_times, ntimes_in*sizeof(float));
    }
    memcpy(global_times, time_in, ntimes_in*sizeof(float));
    nglobal_times = ntimes_in;
    return;
  }

// allocate buffer for merged times
  if(nglobal_times+ntimes_in>ntimes_buffer){
    ntimes_buffer = nglobal_times+ntimes_in+1000;
    FREEMEMORY(times_buffer);
    NewMemory((void **)&times_buffer, ntimes_buffer*sizeof(float));
  }

  // merge global_times and times_in into times_buffer
  for(left=0,right=0,nbuffer=0;left<nglobal_times||right<ntimes_in;){
    float minval;

    if(left>=nglobal_times){
      minval = time_in[right++];
    }
    else if(right>=ntimes_in){
      minval = global_times[left++];
    }
    else{
      float lval, rval;

      lval = global_times[left];
      rval = time_in[right];
      if(lval<rval){
        minval = lval;
        left++;
      }
      else{
        minval = rval;
        right++;
      }
    }
    if(nbuffer==0||minval>times_buffer[nbuffer-1]+dt_eps){
      times_buffer[nbuffer++] = minval;
    }
  }

  // copy merged times array back into original global_times array
  if(nbuffer>nglobal_times){
    FREEMEMORY(global_times);
    NewMemory((void **)&global_times, nbuffer*sizeof(float));
  }
  memcpy(global_times, times_buffer, nbuffer*sizeof(float));

  nglobal_times = nbuffer;
}

  /* ------------------ UpdateTimes ------------------------ */

void UpdateTimes(void){
  int i;

  GetGeomInfoPtrs(0);

  UpdateShow();
  CheckMemory;
  nglobal_times = 0;

  FREEMEMORY(global_times);
  nglobal_times = 0;
  FREEMEMORY(times_buffer);
  ntimes_buffer = 0;

  // determine min time, max time and number of times

  if(current_script_command!=NULL&&current_script_command->command==SCRIPT_LOADSLICERENDER){
    scriptdata *ss;

    ss = current_script_command;
    float ss_tmin = ss->fval2;
    float ss_tmax = ss->fval3;
    if(ss_tmin<=ss_tmax){
      float stimes[2];

      stimes[0] = ss_tmin;
      stimes[1] = ss_tmax;
      MergeGlobalTimes(stimes, 2);
    }
  }

  if(global_tbegin<global_tend){
    float stimes[2];

    stimes[0] = global_tbegin;
    stimes[1] = global_tend;
    MergeGlobalTimes(stimes, 2);
  }

  if(visHRRlabel==1&&show_hrrpuv_plot==1&&hrrinfo!=NULL){
    MergeGlobalTimes(hrrinfo->times_csv, hrrinfo->ntimes_csv);
  }
  if(showdevice_val==1||showdevice_plot!=DEVICE_PLOT_HIDDEN){
    for(i = 0; i<ndeviceinfo; i++){
      devicedata *devicei;

      devicei = deviceinfo+i;
      if(devicei->object->visible==0||devicei->nvals==0)continue;
      if(devicei->type2==devicetypes_index){
        MergeGlobalTimes(devicei->times, devicei->nvals);
      }
    }
  }

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0||geomi->ntimes<=1)continue;
    MergeGlobalTimes(geomi->times, geomi->ntimes);
  }
  if(visShooter!=0&&shooter_active==1){
    nglobal_times = MAX(nglobal_times,nshooter_frames);
  }
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->loaded==0)continue;
    MergeGlobalTimes(parti->times, parti->ntimes);
  }
  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd=sliceinfo+i;
    if(sd->loaded==1||sd->vloaded==1){
      MergeGlobalTimes(sd->times, sd->ntimes);
    }
  }
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->loaded==1&&patchi->structured == NO){
      MergeGlobalTimes(patchi->geom_times, patchi->ngeom_times);
    }
  }
  for(i=0;i<nmeshes;i++){
    patchdata *patchi;
    meshdata *meshi;
    int filenum;

    meshi=meshinfo+i;
    filenum =meshi->patchfilenum;
    if(filenum!=-1){
      patchi=patchinfo+filenum;
      if(patchi->loaded==1&&patchi->structured == YES){
        MergeGlobalTimes(meshi->patch_times, meshi->npatch_times);
      }
    }
  }
  if(ReadZoneFile==1&&visZone==1){
    MergeGlobalTimes(zone_times, nzone_times);
  }
  if(ReadIsoFile==1&&visAIso!=0){
    for(i=0;i<nisoinfo;i++){
      meshdata *meshi;
      isodata *ib;

      ib = isoinfo+i;
      if(ib->geomflag==1||ib->loaded==0)continue;
      meshi=meshinfo + ib->blocknumber;
      MergeGlobalTimes(meshi->iso_times, meshi->niso_times);
    }
  }
  if(nvolrenderinfo>0){
    for(i=0;i<nmeshes;i++){
      volrenderdata *vr;
      meshdata *meshi;

      meshi=meshinfo+i;
      vr = &meshi->volrenderinfo;
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      MergeGlobalTimes(vr->times, vr->ntimes);
    }
  }
  {
    smoke3ddata *smoke3di;

    if(nsmoke3dloaded>0&&vis3DSmoke3D==1){
      for(i=0;i<nsmoke3dinfo;i++){
        smoke3di = smoke3dinfo + i;
        if(smoke3di->loaded==0)continue;
        MergeGlobalTimes(smoke3di->times, smoke3di->ntimes);
      }
    }
  }

  for(i=0;i<ntourinfo;i++){
    tourdata *touri;

    touri = tourinfo + i;
    if(touri->display==0)continue;
    MergeGlobalTimes(touri->path_times, touri->ntimes);
  }

  CheckMemory;

  // allocate memory for individual timelist arrays

  FREEMEMORY(render_frame);
  if(nglobal_times>0)NewMemory((void **)&render_frame,nglobal_times*sizeof(int));

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    FREEMEMORY(geomi->timeslist);
    if(nglobal_times>0)NewMemory((void **)&geomi->timeslist,nglobal_times*sizeof(int));
  }
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti=partinfo+i;
    FREEMEMORY(parti->timeslist);
    if(nglobal_times>0)NewMemory((void **)&parti->timeslist,nglobal_times*sizeof(int));
  }
  for(i=0;i<ntourinfo;i++){
    tourdata *touri;

    touri=tourinfo + i;
    if(touri->display==0)continue;
    FREEMEMORY(touri->timeslist);
    if(nglobal_times>0)NewMemory((void **)&touri->timeslist,nglobal_times*sizeof(int));
  }
  if(hrrinfo!=NULL){
    FREEMEMORY(hrrinfo->timeslist);
    FREEMEMORY(hrrinfo->times);
    FREEMEMORY(hrrinfo->hrrval);
    FREEMEMORY(hrrinfo->hrrval_orig);
    if(hrrinfo->loaded==1&&hrrinfo->display==1&&nglobal_times>0){
      int jstart=0;

      NewMemory((void **)&hrrinfo->timeslist,nglobal_times*sizeof(int));
      NewMemory((void **)&hrrinfo->times,nglobal_times*sizeof(float));
      NewMemory((void **)&hrrinfo->hrrval,nglobal_times*sizeof(float));
      NewMemory((void **)&hrrinfo->hrrval_orig, nglobal_times*sizeof(float));
      hrrinfo->ntimes=nglobal_times;
      for(i=0;i<nglobal_times;i++){
        int j, foundit;

        foundit=0;
        hrrinfo->times[i]=global_times[i];
        for(j=jstart;j<hrrinfo->ntimes_csv-1;j++){
          if(hrrinfo->times_csv[j]<=global_times[i]&&global_times[i]<hrrinfo->times_csv[j+1]){
            float f1, tbot;

            foundit=1;
            tbot = hrrinfo->times_csv[j+1]-hrrinfo->times_csv[j];
            if(tbot>0.0){
              f1 = (global_times[i]-hrrinfo->times_csv[j])/tbot;
            }
            else{
              f1=0.0;
            }
            hrrinfo->hrrval[i]=(1.0-f1)*hrrinfo->hrrval_csv[j]+f1*hrrinfo->hrrval_csv[j+1];
            hrrinfo->hrrval_orig[i]=hrrinfo->hrrval[i];
            jstart=j;
            break;
          }
        }
        if(foundit==0){
          hrrinfo->hrrval[i]=hrrinfo->hrrval_csv[hrrinfo->ntimes_csv-1];
          hrrinfo->hrrval_orig[i]=hrrinfo->hrrval[i];
        }
      }
    }
  }
  FREEMEMORY(shooter_timeslist);
  if(visShooter!=0&&shooter_active==1){
    NewMemory((void **)&shooter_timeslist,nshooter_frames*sizeof(int));
  }

  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd = sliceinfo + i;
    if(sd->slice_filetype == SLICE_GEOM){
      FREEMEMORY(sd->patchgeom->geom_timeslist);
      if(nglobal_times > 0)NewMemory((void **)&(sd->patchgeom->geom_timeslist), nglobal_times * sizeof(int));
    }
    else {
      FREEMEMORY(sd->timeslist);
      if(nglobal_times > 0)NewMemory((void **)&sd->timeslist, nglobal_times * sizeof(int));
    }
  }
  if(nvolrenderinfo>0){
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = meshinfo + i;
      vr = &(meshi->volrenderinfo);
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      FREEMEMORY(vr->timeslist);
      if(nglobal_times>0)NewMemory((void **)&vr->timeslist,nglobal_times*sizeof(int));
    }
  }
  {
    smoke3ddata *smoke3di;

    for(i=0;i<nsmoke3dinfo;i++){
      smoke3di = smoke3dinfo + i;
      FREEMEMORY(smoke3di->timeslist);
      if(nglobal_times>0)NewMemory((void **)&smoke3di->timeslist,nglobal_times*sizeof(int));
    }
  }
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi=meshinfo+i;
    if(meshi->iso_times==NULL)continue;
    FREEMEMORY(meshi->iso_timeslist);
    if(nglobal_times>0)NewMemory((void **)&meshi->iso_timeslist,  nglobal_times*sizeof(int));
  }

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    FREEMEMORY(patchi->geom_timeslist);
    if(patchi->structured == YES)continue;
    if(patchi->geom_times==NULL)continue;
    if(nglobal_times>0)NewMemory((void **)&patchi->geom_timeslist,nglobal_times*sizeof(int));
  }
  for(i=0;i<nmeshes;i++){
    FREEMEMORY(meshinfo[i].patch_timeslist);
  }
  for(i=0;i<nmeshes;i++){
    if(meshinfo[i].patch_times==NULL)continue;
    if(nglobal_times>0)NewMemory((void **)&meshinfo[i].patch_timeslist,nglobal_times*sizeof(int));
  }

  FREEMEMORY(zone_timeslist);
  if(nglobal_times>0)NewMemory((void **)&zone_timeslist,     nglobal_times*sizeof(int));

  FREEMEMORY(targtimeslist);
  if(nglobal_times>0)NewMemory((void **)&targtimeslist,  nglobal_times*sizeof(int));
  CheckMemory;

  // reset render_frame array

  if(current_script_command!=NULL&&
    (current_script_command->command==SCRIPT_VOLSMOKERENDERALL||current_script_command->command==SCRIPT_ISORENDERALL)
    ){
    if(current_script_command->first==1){
      int n;

      for(n=0;n<nglobal_times;n++){
        render_frame[n]=0;
      }
      current_script_command->first=0;
    }
  }
  else{
    int n;

    for(n=0;n<nglobal_times;n++){
      render_frame[n]=0;
    }
  }

  // initialize individual time pointers

  izone=0;
  ResetItimes0();
  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    geomi->itime=0;
  }
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi=meshinfo+i;
    meshi->patch_itime=0;
  }
  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd = sliceinfo + i;
    sd->itime=0;
  }
  frame_index=first_frame_index;
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi=meshinfo+i;
    if(meshi->iso_times==NULL)continue;
    meshi->iso_itime=0;
  }
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    parti->itime=0;
  }

  /* determine visibility of each blockage at each time step */

  for(i=0;i<nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi=meshinfo+i;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[j];
      if(bc->showtime==NULL)continue;
      FREEMEMORY(bc->showtimelist);
      if(nglobal_times>0){
        int k;

        NewMemory((void **)&bc->showtimelist,nglobal_times*sizeof(int));
        for(k=0;k<nglobal_times;k++){
          int listindex;

          bc->showtimelist[k]=1;
          listindex=GetIndex(global_times[k],bc->showtime,bc->nshowtime);
          bc->showtimelist[k]=bc->showhide[listindex];
        }
      }
    }
  }

  /* determine state of each device at each time step */

  for(i=0;i<ndeviceinfo;i++){
    devicedata *devicei;

    devicei = deviceinfo + i;
    if(devicei->object->visible==0)continue;
    if(devicei->nstate_changes==0)continue;
    FREEMEMORY(devicei->showstatelist);
    if(nglobal_times>0){
      int k;

      NewMemory((void **)&devicei->showstatelist,nglobal_times*sizeof(int));
      for(k=0;k<nglobal_times;k++){
        int listindex;

        listindex=GetIndex(global_times[k],devicei->act_times,devicei->nstate_changes);
        devicei->showstatelist[k]=devicei->state_values[listindex];
      }
    }
  }

  /* determine visibility of each vent at each time step */

  for(i=0;i<nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi=meshinfo+i;
    if(meshi->ventinfo==NULL)continue;
    for(j=0;j<meshi->nvents;j++){
      ventdata *vi;

      vi = meshi->ventinfo+j;
      if(vi->showtime==NULL)continue;
      FREEMEMORY(vi->showtimelist);
      if(nglobal_times>0){
        int k;

        NewMemory((void **)&vi->showtimelist,nglobal_times*sizeof(int));
        for(k=0;k<nglobal_times;k++){
          int listindex;

          vi->showtimelist[k]=1;
          listindex=GetIndex(global_times[k],vi->showtime,vi->nshowtime);
          vi->showtimelist[k]=vi->showhide[listindex];
        }
      }
    }
  }

  /* determine visibility of each circular vent at each time step */

  for(i = 0; i<nmeshes; i++){
    int j;
    meshdata *meshi;

    meshi = meshinfo + i;
    if(meshi->cventinfo == NULL)continue;
    for(j = 0; j<meshi->ncvents; j++){
      cventdata *cvi;

      cvi = meshi->cventinfo + j;
      if(cvi->showtime == NULL)continue;
      FREEMEMORY(cvi->showtimelist);
      if(nglobal_times>0){
        int k;

        NewMemory((void **)&cvi->showtimelist, nglobal_times * sizeof(int));
        for(k = 0; k<nglobal_times; k++){
          int listindex;

          cvi->showtimelist[k] = 1;
          listindex = GetIndex(global_times[k], cvi->showtime, cvi->nshowtime);
          cvi->showtimelist[k] = cvi->showhide[listindex];
        }
      }
    }
  }

  if(nglobal_times>0)SynchTimes();
  updatefaces=1;
  if(nglobal_times>0){
    UpdateTimeLabels();
    UpdateGluiTimeBounds(global_times[0],global_times[nglobal_times-1]);
  }
  CheckMemory;
}

/* ------------------ GetPlotStateSub ------------------------ */

int GetPlotStateSub(int choice){
  int i;

  UpdateLoadedLists();
  switch(choice){
    case STATIC_PLOTS:
    case STATIC_PLOTS_NORECURSE:
      stept = 0;
      for(i=0;i<nmeshes;i++){
        plot3ddata *ploti;
        meshdata *meshi;

        meshi=meshinfo + i;
        if(meshi->plot3dfilenum==-1)continue;
        ploti = plot3dinfo + meshi->plot3dfilenum;
        if(ploti->loaded==0||ploti->display==0)continue;
        if(visx_all==0&&visy_all==0&&visz_all==0&&visiso==0)continue;
        return STATIC_PLOTS;
      }
      if(choice!=STATIC_PLOTS_NORECURSE){
        return GetPlotState(DYNAMIC_PLOTS_NORECURSE);
      }
      break;
    case DYNAMIC_PLOTS:
    case DYNAMIC_PLOTS_NORECURSE:
      if(visHRRlabel==1&&show_hrrpuv_plot==1&&hrrinfo!=NULL){
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(showdevice_val==1||showdevice_plot!=DEVICE_PLOT_HIDDEN){
        for(i = 0; i<ndeviceinfo; i++){
          devicedata *devicei;

          devicei = deviceinfo+i;
          if(devicei->object->visible==0)continue;
          if(devicei->type2==devicetypes_index){
            stept = 1;
            return DYNAMIC_PLOTS;
          }
        }
      }
      for(i=0;i<nslice_loaded;i++){
        slicedata *slicei;

        slicei = sliceinfo + slice_loaded_list[i];
        if(slicei->display==0||slicei->slicefile_labelindex!=slicefile_labelindex)continue;
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(visGrid==0)stept = 1;
      for(i=0;i<nvsliceinfo;i++){
        vslicedata *vslicei;

        vslicei = vsliceinfo + i;
        if(vslicei->display==0||vslicei->vslicefile_labelindex!=slicefile_labelindex)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<npatch_loaded;i++){
        patchdata *patchi;

        patchi = patchinfo + patch_loaded_list[i];
        if (patchi->display == 1) {
          if(patchi->boundary == 1 && patchi->shortlabel_index == iboundarytype)return DYNAMIC_PLOTS;
          if(patchi->boundary == 0 && patchi->shortlabel_index == slicefile_labelindex)return DYNAMIC_PLOTS;
        }
      }
      for(i=0;i<npartinfo;i++){
        partdata *parti;

        parti = partinfo + i;
        if(parti->loaded==0||parti->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<nisoinfo;i++){
        isodata *isoi;

        isoi = isoinfo + i;
        if(isoi->loaded==0)continue;
        if(isoi->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<nzoneinfo;i++){
        zonedata *zonei;

        zonei = zoneinfo + i;
        if(zonei->loaded==0||zonei->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<ntourinfo;i++){
        tourdata *touri;

        touri = tourinfo + i;
        if(touri->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<nsmoke3dinfo;i++){
        smoke3ddata *smoke3di;

        smoke3di = smoke3dinfo + i;
        if(smoke3di->loaded==0||smoke3di->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      if(nvolrenderinfo>0){
        for(i=0;i<nmeshes;i++){
          meshdata *meshi;
          volrenderdata *vr;

          meshi = meshinfo + i;
          vr = &(meshi->volrenderinfo);
          if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
          if(vr->loaded==0||vr->display==0)continue;
          return DYNAMIC_PLOTS;
        }
      }
      if(visShooter!=0&&shooter_active==1){
        return DYNAMIC_PLOTS;
      }
      if(choice!=DYNAMIC_PLOTS_NORECURSE)return GetPlotState(STATIC_PLOTS_NORECURSE);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  stept = 0;
  return NO_PLOTS;
}

/* ------------------ GetPlotState ------------------------ */

int GetPlotState(int choice){
  int plot_state;

  plot_state = GetPlotStateSub(choice);
#ifdef pp_REFRESH
  if(plot_state==DYNAMIC_PLOTS){
    periodic_refresh = 0;
  }
  else{
    if(periodic_refresh==0&&glui_refresh_rate>0){
      periodic_refresh = 1;
      PeriodicRefresh(refresh_interval);
    }
  }
#endif
  if(plot_state!=DYNAMIC_PLOTS&&last_time_paused==1){
    last_time_paused = 0;
  }
  return plot_state;
}

/* ------------------ GetIndex ------------------------ */

int GetIndex(float key, const float *list, int nlist){
  int i;

  if(nlist==1)return 0;
  if(key<list[1])return 0;
  if(key>=list[nlist-1])return nlist-1;
  for(i=1;i<nlist-1;i++){
    if(list[i]<=key&&key<list[i+1])return i;
  }
  return 0;
}

/* ------------------ ISearch ------------------------ */

int ISearch(float *list, int nlist, float key, int guess){
  /*
     find val such that list[val]<=key<list[val+1]
     start with val=guess
  */

  int low, mid, high;

  if(nlist<=2||key<list[0])return 0;
  if(key>=list[nlist-2])return nlist-2;
  if(guess<0)guess=0;
  if(guess>nlist-2)guess=nlist-2;
  if(list[guess]<=key&&key<list[guess+1])return guess;

  low = 0;
  high = nlist - 1;
  while(high-low>1){
    mid = (low+high)/2;
    if(list[mid]>key){
      high=mid;
    }
    else{
      low=mid;
    }
  }
  if(list[high]==key)return high;
  return low;
}

/* ------------------ ResetItimes0 ------------------------ */

void ResetItimes0(void){
  if(current_script_command==NULL||current_script_command->command!=SCRIPT_VOLSMOKERENDERALL||current_script_command->command!=SCRIPT_ISORENDERALL){
    itimes=first_frame_index;
  }
}

/* ------------------ UpdateClipbounds ------------------------ */

void UpdateClipbounds(int set_i0, int *i0, int set_i1, int *i1, int imax){

  if(set_i0==0&&set_i1==0)return;
  if(set_i0==1&&set_i1==1){
    if(*i0>imax-1){*i0=imax-1; *i1=imax;}
    if(*i1>imax)*i1=imax;
    if(*i1<1){*i1=1;*i0=0;}
    if(*i0<0)*i0=0;
    if(*i0>=*i1){*i0=*i1-1;}
  }
  if(set_i0==1&&set_i1==0){
    if(*i0<0)*i0=0;
    if(*i0>imax)*i0=imax;
  }
  if(set_i0==0&&set_i1==1){
    if(*i1<0)*i1=0;
    if(*i1>imax)*i1=imax;
  }
}

/* ------------------ UpdateColorTable ------------------------ */

void UpdateColorTable(colortabledata *ctableinfo, int nctableinfo){
  int i, ncolortableinfo_old;

  if(nctableinfo<=0)return;

  ncolortableinfo_old=ncolortableinfo;
  ResizeMemory((void **)&colortableinfo, (ncolortableinfo+nctableinfo)*sizeof(colortabledata));
  for(i = 0; i<nctableinfo; i++){
    colortabledata *newentryi, *fromi;

    fromi = ctableinfo+i;
    newentryi= GetColorTable(fromi->label);
    if(newentryi==NULL){
      newentryi = colortableinfo + ncolortableinfo;
      ncolortableinfo++;
    }
    newentryi->color[0] = fromi->color[0];
    newentryi->color[1] = fromi->color[1];
    newentryi->color[2] = fromi->color[2];
    newentryi->color[3] = fromi->color[3];
    strcpy(newentryi->label, fromi->label);
  }
  ResizeMemory((void **)&colortableinfo, ncolortableinfo*sizeof(colortabledata));
  UpdateColorTableList(ncolortableinfo_old);
}

/* ------------------ UpdateShowScene ------------------------ */

void UpdateShowScene(void){
  if(open_movie_dialog==1){
    open_movie_dialog = 0;
    if(have_slurm==1&&nmovie_queues>0){
      ShowGluiMotion(DIALOG_MOVIE_BATCH);
    }
    else{
      ShowGluiMotion(DIALOG_MOVIE);
    }
  }
  if(terrain_update_normals==1&&ngeominfo>0){
    int sizeof_vertices, sizeof_indices;

    terrain_update_normals = 0;
    UpdateAllGeomTriangles();
    if(auto_terrain==1){
      GenerateTerrainGeom(&terrain_vertices, &sizeof_vertices, &terrain_indices, &sizeof_indices, &terrain_nindices);
    }
  }
  if(update_splitcolorbar==1){
    SplitCB(SPLIT_COLORBAR);
    update_splitcolorbar = 0;
  }
  if(update_generate_part_histograms==1){
    update_generate_part_histograms = 0;
    GeneratePartHistogramsMT();
    update_generate_part_histograms = -1;
  }
  if(update_stept==1){
    update_stept = 0;
    SetTimeVal(time_paused);
  }
  if(update_movie_parms==1){
    update_movie_parms = 0;
    UpdateMovieParms();
  }
#ifdef pp_REFRESH
  if(update_refresh==1){
    update_refresh = 0;
    PeriodicRefresh(refresh_interval);
  }
#endif
  if(update_glui_devices==1){
    update_glui_devices = 0;
    UpdateGluiDevices();
  }
  if(update_times==1){
    update_times = 0;
    UpdateTimes();
  }
  if(update_smoketype_vals==1){
    update_smoketype_vals = 0;
    Smoke3dCB(SMOKE_NEW);
    Smoke3dCB(SMOKE_DELTA_MULTIPLE);
  }
  if(update_use_lighting==1){
    use_lighting = 1-use_lighting_ini;
    ColorbarMenu(USE_LIGHTING);
    update_use_lighting = 0;
  }
  if(update_opacity_map==1){
    UpdateOpacityMap();
  }
  if(update_playmovie==1){
    EnableDisablePlayMovie();
    update_playmovie = 0;
  }
  UpdateRenderStartButton();
  if(update_makemovie == 1||output_ffmpeg_command==1)MakeMovie();
  if(compute_fed == 1)DefineAllFEDs();
  if(restart_time == 1){
    restart_time = 0;
    ResetItimes0();
  }
  if(loadfiles_at_startup==1&&update_load_files == 1){
    LoadFiles();
  }
  if(update_startup_view>0){
    SetCurrentViewPoint(viewpoint_label_startup);
    update_rotation_center = 0;
    update_rotation_center_ini = 0;
    update_startup_view--;
  }
  if(update_saving_viewpoint>0){
    SetCurrentViewPoint(viewpoint_label_saved);
    update_saving_viewpoint--;
  }
  if(update_viewpoint_script>0){
    SetCurrentViewPoint(viewpoint_script);
    update_viewpoint_script--;
  }
  if(update_tour_list == 1){
    UpdateTourList();
  }
  if(update_gslice == 1){
    UpdateGsliceParms();
  }
  if(update_rotation_center == 1){
    camera_current->rotation_index = glui_rotation_index;
    SceneMotionCB(ROTATE_ABOUT);
    update_rotation_center = 0;
  }
  if(update_rotation_center_ini == 1){
    camera_current->rotation_index = glui_rotation_index_ini;
    SceneMotionCB(ROTATE_ABOUT);
    update_rotation_center_ini = 0;
  }
  if(camera_current->dirty == 1){
    UpdateCamera(camera_current);
  }
  if(updateclipvals == 1){
    Clip2Cam(camera_current);
    UpdateClipAll();
    updateclipvals = 0;
  }
  if(update_selectedtour_index == 1){
    UpdateTourIndex();
  }
  if(trainer_mode == 1 && fontindex != LARGE_FONT)FontMenu(LARGE_FONT);
  if(updateindexcolors == 1){
    UpdateIndexColors();
  }
  if(force_isometric == 1){
    force_isometric = 0;
    projection_type = PROJECTION_ORTHOGRAPHIC;
    camera_current->projection_type = projection_type;
    ZoomMenu(UPDATE_PROJECTION);
  }
  if(convert_ini == 1){
    WriteIni(SCRIPT_INI, ini_to);
    SMV_EXIT(0);
  }
  if(convert_ssf==1||update_ssf==1){
    ConvertSsf();
    SMV_EXIT(0);
  }
  UpdateShow();
  if(global_times!=NULL&&updateUpdateFrameRateMenu==1)FrameRateMenu(frameratevalue);
  if(updatefaces==1)UpdateFaces();
  if(updatefacelists==1)UpdateFaceLists();
  if(update_draw_hist==1){
    update_draw_hist = 0;
    SetPercentileDrawOff();
  }
}

/* ------------------ UpdateFlippedColorbar ------------------------ */

void UpdateFlippedColorbar(void){
  int i, flip = 0;

  for(i = 0;i < nslice_loaded;i++){
    slicedata *slicei;

    slicei = sliceinfo + slice_loaded_list[i];
    if(slicei->slicefile_labelindex!=slicefile_labelindex)continue;
    if(slicei->display == 0)continue;
    if(slicei->colorbar_autoflip == 1&&colorbar_autoflip == 1){
      flip = 1;
      break;
    }
  }
  if(flip != colorbar_flip){
    colorbar_flip = 1 - flip;
    ColorbarMenu(COLORBAR_FLIP);
  }
}

/* ------------------ GetColorbarState ------------------------ */

int GetColorbarState(void){
  if(visColorbarVertical==1&&visColorbarHorizontal==0){
    return COLORBAR_SHOW_VERTICAL;
  }
  else if(visColorbarVertical==0&&visColorbarHorizontal==1){
    return COLORBAR_SHOW_HORIZONTAL;
  }
  // if colorbars are hidden then research mode needs to be off
  visColorbarVertical = 0;
  visColorbarHorizontal = 0;
  visColorbarVertical_save = 0;
  visColorbarHorizontal_save = 0;
  return COLORBAR_HIDDEN;
}


/* ------------------ OutputMinMax  ------------------------ */

void OutputMinMax(char *meshlabel, char *label, char *unit, float valmin_fds, float valmax_fds, float valmin_smv, float valmax_smv){
  char cvalmin_fds[20], cvalmax_fds[20];
  char cdiff_min[20], cdiff_max[20];
  char cmin[100], cmax[100];
  char labelunit[50];

  Float2String(cvalmin_fds, valmin_fds, 6, force_fixedpoint);
  Float2String(cvalmax_fds, valmax_fds, 6, force_fixedpoint);

  Float2String(cdiff_min, valmin_fds-valmin_smv, 3, force_fixedpoint);
  Float2String(cdiff_max, valmax_fds-valmax_smv, 3, force_fixedpoint);

  strcpy(cmin,cvalmin_fds);
  strcat(cmin,"(");
  strcat(cmin,cdiff_min);
  strcat(cmin,")");

  strcpy(cmax,cvalmax_fds);
  strcat(cmax,"(");
  strcat(cmax,cdiff_max);
  strcat(cmax,")");

  strcpy(labelunit, label);
  strcat(labelunit," ");
  strcat(labelunit,unit);

  if(show_bound_diffs==1){
    printf("%s: %23.23s, min(delta)=%22.22s, max(delta)=%22.22s\n", meshlabel, labelunit, cmin, cmax);
  }
  else{
    printf("%s: %s, min=%12.12s, max=%12.12s\n", meshlabel, labelunit, cvalmin_fds, cvalmax_fds);
  }
}

/* ------------------ UpdateBounds ------------------------ */

void OutputBounds(void){
// slice bounds
  if(update_slice_bounds != -1){
    float valmin_fds=1.0, valmax_fds=0.0, valmin_smv=1.0, valmax_smv=0.0;
    char *label, *unit;
    int i;

    label = sliceinfo[update_slice_bounds].label.longlabel;
    unit = sliceinfo[update_slice_bounds].label.unit;
    for(i=0;i<nsliceinfo;i++){
      slicedata *slicei;
      char *labeli;
      meshdata *meshi;

      slicei = sliceinfo + i;
      if(slicei->loaded==0)continue;
      meshi = meshinfo+slicei->blocknumber;
      labeli = slicei->label.longlabel;
      if(strcmp(label,labeli)!=0)continue;
      if(nmeshes>1&&bounds_each_mesh==1){
        OutputMinMax(meshi->label, label, unit, slicei->valmin_fds, slicei->valmax_fds, slicei->valmin_smv, slicei->valmax_smv);
      }
      if(valmin_fds>valmax_fds){
        valmin_fds = slicei->valmin_fds;
        valmax_fds = slicei->valmax_fds;
      }
      else{
        valmin_fds = MIN(slicei->valmin_fds, valmin_fds);
        valmax_fds = MAX(slicei->valmax_fds, valmax_fds);
      }
      if(valmin_smv>valmax_smv){
        valmin_smv = slicei->valmin_smv;
        valmax_smv = slicei->valmax_smv;
      }
      else{
        valmin_smv = MIN(slicei->valmin_smv, valmin_smv);
        valmax_smv = MAX(slicei->valmax_smv, valmax_smv);
      }
    }
    OutputMinMax("global", label, unit, valmin_fds, valmax_fds, valmin_smv, valmax_smv);
  }

// boundary file bounds
  if(update_patch_bounds != -1){
    float valmin_fds=1.0, valmax_fds=0.0, valmin_smv=1.0, valmax_smv=0.0;
    char *label, *unit;
    int i;

    label = patchinfo[update_patch_bounds].label.longlabel;
    unit = patchinfo[update_patch_bounds].label.unit;
    for(i=0;i<npatchinfo;i++){
      patchdata *patchi;
      char *labeli;
      meshdata *meshi;

      patchi = patchinfo + i;
      if(patchi->loaded==0)continue;
      meshi = meshinfo+patchi->blocknumber;
      labeli = patchi->label.longlabel;
      if(strcmp(label,labeli)!=0)continue;
      if(nmeshes>1&&bounds_each_mesh==1){
        OutputMinMax(meshi->label, label, unit, patchi->valmin_fds, patchi->valmax_fds, patchi->valmin_smv, patchi->valmax_smv);
      }
      if(valmin_fds>valmax_fds){
        valmin_fds = patchi->valmin_fds;
        valmax_fds = patchi->valmax_fds;
      }
      else{
        valmin_fds = MIN(patchi->valmin_fds, valmin_fds);
        valmax_fds = MAX(patchi->valmax_fds, valmax_fds);
      }
      if(valmin_smv>valmax_smv){
        valmin_smv = patchi->valmin_smv;
        valmax_smv = patchi->valmax_smv;
      }
      else{
        valmin_smv = MIN(patchi->valmin_smv, valmin_smv);
        valmax_smv = MAX(patchi->valmax_smv, valmax_smv);
      }
    }
    OutputMinMax("global", label, unit, valmin_fds, valmax_fds, valmin_smv, valmax_smv);
  }

// particle file bounds
  if(update_part_bounds!=-1){
    float valmin_fds = 1.0, valmax_fds = 0.0, valmin_smv = 1.0, valmax_smv = 0.0;
    char *label, *unit;
    int i, j;

    if(nmeshes>1&&nmeshes>1&&bounds_each_mesh==1){
      for(i = 0; i<npartinfo; i++){
        partdata *parti;
        meshdata *meshi;

        parti = partinfo+i;
        if(parti->loaded==0)continue;
        meshi = meshinfo + parti->blocknumber;
        for(j = 0; j<npart5prop; j++){
          partpropdata *propj;

          if(j==0)continue;
          propj = part5propinfo+j;

          label = propj->label->longlabel;
          unit = propj->label->unit;
          OutputMinMax(meshi->label, label, unit, parti->valmin_fds[j], parti->valmax_fds[j], parti->valmin_smv[j], parti->valmax_smv[j]);
        }
        printf("\n");
      }
    }

    for(j = 0; j<npart5prop; j++){
      partpropdata *propj;

      if(j==0)continue;
      propj = part5propinfo+j;

      label = propj->label->longlabel;
      unit = propj->label->unit;
      valmin_fds = 1.0;
      valmax_fds = 0.0;
      valmin_smv = 1.0;
      valmax_smv = 0.0;
      for(i = 0; i<npartinfo; i++){
        partdata *parti;

        parti = partinfo+i;
        if(parti->loaded==0)continue;
        if(valmin_fds>valmax_fds){
          valmin_fds = parti->valmin_fds[j];
          valmax_fds = parti->valmax_fds[j];
        }
        else{
          valmin_fds = MIN(parti->valmin_fds[j], valmin_fds);
          valmax_fds = MAX(parti->valmax_fds[j], valmax_fds);
        }
        if(valmin_smv>valmax_smv){
          valmin_smv = parti->valmin_smv[j];
          valmax_smv = parti->valmax_smv[j];
        }
        else{
          valmin_smv = MIN(parti->valmin_smv[j], valmin_smv);
          valmax_smv = MAX(parti->valmax_smv[j], valmax_smv);
        }
      }
      OutputMinMax("global", label, unit, valmin_fds, valmax_fds, valmin_smv, valmax_smv);
    }
  }

// plot3d file bounds
  if(update_plot3d_bounds!=-1){
    float valmin_fds = 1.0, valmax_fds = 0.0, valmin_smv = 1.0, valmax_smv = 0.0;
    char *label, *unit;
    int i, j;
    plot3ddata *p;

    p = plot3dinfo+update_plot3d_bounds;

    if(nmeshes>1&&bounds_each_mesh==1){
      printf("\n");
      for(i = 0; i<nplot3dinfo; i++){
        plot3ddata *plot3di;
        meshdata *meshi;

        plot3di = plot3dinfo+i;
        if(plot3di->loaded==0)continue;
        meshi = meshinfo+plot3di->blocknumber;
        for(j = 0; j<MAXPLOT3DVARS; j++){

          label = p->label[j].longlabel;
          unit = p->label[j].unit;

          OutputMinMax(meshi->label, label, unit, plot3di->valmin_fds[j], plot3di->valmax_fds[j], plot3di->valmin_smv[j], plot3di->valmax_smv[j]);
        }
        printf("\n");
      }
    }

    p = plot3dinfo+update_plot3d_bounds;
    for(j=0;j<MAXPLOT3DVARS;j++){

      label = plot3dinfo[update_plot3d_bounds].label[j].longlabel;
      unit = plot3dinfo[update_plot3d_bounds].label[j].unit;
      valmin_fds = 1.0;
      valmax_fds = 0.0;
      valmin_smv = 1.0;
      valmax_smv = 0.0;
      for(i = 0; i<nplot3dinfo; i++){
        plot3ddata *plot3di;

        plot3di = plot3dinfo+i;
        if(plot3di->loaded==0)continue;
        if(valmin_fds>valmax_fds){
          valmin_fds = plot3di->valmin_fds[j];
          valmax_fds = plot3di->valmax_fds[j];
        }
        else{
          valmin_fds = MIN(plot3di->valmin_fds[j], valmin_fds);
          valmax_fds = MAX(plot3di->valmax_fds[j], valmax_fds);
        }
        if(valmin_smv>valmax_smv){
          valmin_smv = plot3di->valmin_smv[j];
          valmax_smv = plot3di->valmax_smv[j];
        }
        else{
          valmin_smv = MIN(plot3di->valmin_smv[j], valmin_smv);
          valmax_smv = MAX(plot3di->valmax_smv[j], valmax_smv);
        }
      }
      OutputMinMax("global", label, unit, valmin_fds, valmax_fds, valmin_smv, valmax_smv);
    }
  }
  printf("\n");
}

/* ------------------ UpdateDisplay ------------------------ */

void UpdateDisplay(void){

  LOCK_IBLANK;
  if(update_adjust_y>0){
    AdjustY(camera_current);
    update_adjust_y--;
  }
  if(update_ini_boundary_type==1){
    update_ini_boundary_type = 0;
    ShowBoundaryMenu(INI_EXTERIORwallmenu);
  }
  if(update_fire_alpha==1){
    update_fire_alpha=0;
    UpdateFireAlpha();
  }
  if(update_texturebar==1){
    update_texturebar = 0;
    UpdateTexturebar();
  }
  if(update_setvents==1){
    SetVentDirs();
    update_setvents=0;
  }
  UNLOCK_IBLANK;
  if(update_ini==1){
    update_ini = 0;
    ReadIni(NULL);

    update_glui_bounds = 0;
    UpdateGluiBounds();
  }
  if(update_glui_bounds==1){
    update_glui_bounds = 0;
    UpdateGluiBounds();
  }
  if(update_cache_data==1){
    update_cache_data = 0;
    SetCacheFlag(BOUND_PLOT3D, cache_plot3d_data);
    SetCacheFlag(BOUND_PART, cache_part_data);
    SetCacheFlag(BOUND_PATCH, cache_boundary_data);
    SetCacheFlag(BOUND_SLICE, cache_slice_data);
  }
  if(update_chop_colors==1){
    update_chop_colors = 0;
    UpdateChopColors();
  }
  if(update_zaxis_custom == 1){
    update_zaxis_custom = 0;
    UpdateZAxisCustom();
  }
  if(update_flipped_colorbar == 1){
    update_flipped_colorbar = 0;
    UpdateFlippedColorbar();
  }
  if(update_smokecolorbar == 1){
    update_smokecolorbar = 0;
    SmokeColorbarMenu(fire_colorbar_index);
  }
  if(update_colorbartype == 1){
    colorbardata *cb;

    cb = GetColorbar(colorbarname);
    if(cb != NULL){
      colorbartype = cb - colorbarinfo;
      UpdateCurrentColorbar(cb);
      if(colorbartype != colorbartype_default){
        colorbartype_ini = colorbartype;
      }
    }
    update_colorbartype = 0;
  }
  if(updatezoommenu == 1 || first_display > 0){
    if(first_display > 0)first_display--;
    updatezoommenu = 0;
    ZoomMenu(zoomindex);
  }
  if(update_streaks == 1 && ReadPartFile == 1){
    ParticleStreakShowMenu(streak_index);
    update_streaks = 0;
  }
  if(update_screensize == 1){
    update_screensize = 0;
    UpdateWindowSizeList();
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      screenWidthINI  /= 2;
      screenHeightINI /= 2;
    }
#endif
    ResizeWindow(screenWidthINI, screenHeightINI);
  }
  if(updatemenu == 1 && usemenu == 1 && menustatus == GLUT_MENU_NOT_IN_USE){
    glutDetachMenu(GLUT_RIGHT_BUTTON);
    InitMenus(LOAD);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    updatemenu = 0;
  }
  if(update_patch_bounds!=-1||update_slice_bounds!=-1||update_part_bounds!=-1||update_plot3d_bounds!=-1){

    if(current_script_command==NULL||current_script_command->command!=SCRIPT_LOADSLICERENDER){
      OutputBounds();
    }
    update_patch_bounds = -1;
    update_slice_bounds = -1;
    update_part_bounds = -1;
    update_plot3d_bounds = -1;
  }
  if(update_fire_colorbar_index == 1){
    SmokeColorbarMenu(fire_colorbar_index_ini);
    update_fire_colorbar_index = 0;
  }
  if(update_co2_colorbar_index==1){
    UpdateCO2ColorbarList(co2_colorbar_index_ini);
    update_co2_colorbar_index = 0;
  }
  if(update_colorbar_select_index == 1 && colorbar_select_index >= 0 && colorbar_select_index <= 255){
    update_colorbar_select_index = 0;
    UpdateRGBColors(colorbar_select_index);
  }
  if(histograms_defined==0&&update_slice_hists == 1){
    update_slice_hists = 0;
    UpdateSliceHist();
  }
  if(update_vol_lights==1){
    update_vol_lights = 0;
    InitAllLightFractions(xyz_light_global, light_type_global);
  }
  if(update_windrose_showhide==1){
    UpdateWindRoseDevices(UPDATE_WINDROSE_DEVICE);
  }
  if(update_research_mode == 1){
    update_research_mode = 0;
    UpdateResearchMode();
  }
  if(update_percentile_mode==1){
    update_percentile_mode = 0;
    SetPercentileMode(percentile_mode);
  }
  if(update_colorbar_digits==1){
    update_colorbar_digits = 0;
    SetColorbarDigitsCPP(ncolorlabel_digits);
    SetColorbarDigits();
  }
  if(update_visColorbars==1){
    update_visColorbars = 0;
    visColorbarVertical = visColorbarVertical_val;
    visColorbarHorizontal = visColorbarHorizontal_val;
    vis_colorbar = GetColorbarState();
    UpdateColorbarControls();
    UpdateColorbarControls2();
    updatemenu = 1;
  }
  if(update_windrose==1){
    update_windrose = 0;
    DeviceData2WindRose(nr_windrose, ntheta_windrose);
  }
#ifdef pp_REFRESH
  if(refresh_glui_dialogs>=-1){
    refresh_glui_dialogs--;
    RefreshGluiDialogs();
  }
#endif
}

/* ------------------ ShiftColorbars ------------------------ */

void ShiftColorbars(void){
  int i;

#define BASE_EPS 0.0001

  if(ABS(colorbar_shift-1.0)<BASE_EPS)return;
  for(i=0;i<MAXRGB;i++){
    float factor;
    float *color1, *color2, color_index, *color_new;
    int color1_index;

    color_index = SHIFT_VAL(i, 0, MAXRGB-1, colorbar_shift);
    color1_index = CLAMP((int)color_index,0,MAXRGB-2);
    factor = color_index - color1_index;

    color1 = rgb_full[color1_index];
    color2 = rgb_full[color1_index+1];
    color_new = rgb_slice + 4*i;

    color_new[0] = (1.0-factor)*color1[0] + factor*color2[0];
    color_new[1] = (1.0-factor)*color1[1] + factor*color2[1];
    color_new[2] = (1.0-factor)*color1[2] + factor*color2[2];
    color_new[3] = (1.0-factor)*color1[3] + factor*color2[3];
  }
  CheckMemory;
}
