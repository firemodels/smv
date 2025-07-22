#define IN_UPDATE
#include "options.h"
#include <assert.h>
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
#include "glui_bounds.h"
#include "IOobjects.h"
#include "readsmvfile.h"

/* ------------------ CompareFloat ------------------------ */

int CompareFloat(const void *arg1, const void *arg2){
  float x, y;

  x=*(float *)arg1;
  y=*(float *)arg2;
  if( x< y)return -1;
  if( x> y)return 1;
  return 0;
}

/* ------------------ UpdateFrameNumber ------------------------ */

void UpdateFrameNumber(int changetime){
  if(force_redisplay==1||(itimeold!=itimes&&changetime==1)){
    int i;

    force_redisplay=0;
    itimeold=itimes;
    if(showsmoke==1){
      for(i=0;i<global_scase.npartinfo;i++){
        partdata *parti;

        parti = global_scase.partinfo+i;
        if(parti->loaded==0||parti->timeslist==NULL)continue;
        parti->itime=parti->timeslist[itimes];
      }
    }
    if(showvolrender==1){
      int imesh;

      for(imesh=0;imesh<global_scase.meshescoll.nmeshes;imesh++){
        meshdata *meshi;
        volrenderdata *vr;
        slicedata *fireslice, *smokeslice;
        int j;

        meshi = global_scase.meshescoll.meshinfo + imesh;
        vr = meshi->volrenderinfo;
        fireslice=vr->fireslice;
        smokeslice=vr->smokeslice;
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
        sd = global_scase.slicecoll.sliceinfo+i;
        if(sd->slice_filetype == SLICE_GEOM){
          patchdata *patchi;

          patchi = sd->patchgeom;
          if(patchi->geom_timeslist == NULL)continue;
          if(patchi->structured == YES || patchi->boundary == 1 || patchi->geom_times == NULL || patchi->geom_timeslist == NULL)continue;
          if(current_script_command!=NULL && IS_LOADRENDER){
            patchi->geom_itime = 0; // only one frame loaded at a time when using LOADSLICERNDER
          }
          else{
            patchi->geom_itime = patchi->geom_timeslist[itimes];
          }
          patchi->geom_val_static   = patchi->geom_vals  + patchi->geom_vals_static_offset[patchi->geom_itime];
          patchi->geom_ival_static  = patchi->geom_ivals + patchi->geom_ivals_static_offset[patchi->geom_itime];
          patchi->geom_ival_dynamic = patchi->geom_ivals + patchi->geom_ivals_dynamic_offset[patchi->geom_itime];
          patchi->geom_val_static   = patchi->geom_vals  + patchi->geom_vals_static_offset[patchi->geom_itime];
          patchi->geom_val_dynamic  = patchi->geom_vals  + patchi->geom_vals_dynamic_offset[patchi->geom_itime];
          patchi->geom_nval_static  = patchi->geom_nstatics[patchi->geom_itime];
          patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
          sd->itime                 = patchi->geom_timeslist[itimes];
          slice_time                = sd->itime;
        }
        else{
          if(sd->timeslist == NULL)continue;
          sd->itime = sd->timeslist[itimes];

          assert(sd->times_map == NULL || sd->times_map[sd->itime] == 1);
          slice_time = sd->itime;
        }
      }
      for(i = 0; i < global_scase.npatchinfo; i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->structured == YES || patchi->boundary == 1 || patchi->geom_times == NULL || patchi->geom_timeslist == NULL)continue;
        patchi->geom_itime = patchi->geom_timeslist[itimes];
        patchi->geom_ival_static  = patchi->geom_ivals + patchi->geom_ivals_static_offset[patchi->geom_itime];
        patchi->geom_ival_dynamic = patchi->geom_ivals + patchi->geom_ivals_dynamic_offset[patchi->geom_itime];
        patchi->geom_val_static   = patchi->geom_vals  + patchi->geom_vals_static_offset[patchi->geom_itime];
        patchi->geom_val_dynamic  = patchi->geom_vals  + patchi->geom_vals_dynamic_offset[patchi->geom_itime];
        patchi->geom_nval_static  = patchi->geom_nstatics[patchi->geom_itime];
        patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
      }
    }
    if(show3dsmoke==1 && global_scase.smoke3dcoll.nsmoke3dinfo > 0){
      INIT_PRINT_TIMER(merge_smoke_time);
      MergeSmoke3DAll();
      PrintMemoryInfo;
      PRINT_TIMER(merge_smoke_time, "UpdateSmoke3D + MergeSmoke3D");
    }
    if(showpatch==1){
      for(i=0;i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->structured == YES||patchi->boundary==0||patchi->geom_times==NULL||patchi->geom_timeslist==NULL)continue;
        patchi->geom_itime=patchi->geom_timeslist[itimes];
        if(patchi->geom_ivals != NULL){
          patchi->geom_ival_static = patchi->geom_ivals + patchi->geom_ivals_static_offset[patchi->geom_itime];
          patchi->geom_ival_dynamic = patchi->geom_ivals + patchi->geom_ivals_dynamic_offset[patchi->geom_itime];
        }
        patchi->geom_nval_static  = patchi->geom_nstatics[patchi->geom_itime];
        patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
        if(patchi->is_compressed==1)UncompressBoundaryDataGEOM(patchi, patchi->geom_itime);
      }
      for(i=0;i<global_scase.meshescoll.nmeshes;i++){
        patchdata *patchi;
        meshdata *meshi;

        meshi = global_scase.meshescoll.meshinfo+i;
        if(meshi->patchfilenum < 0||meshi->patchfilenum>global_scase.npatchinfo-1)continue;
        patchi=global_scase.patchinfo + meshi->patchfilenum;
        if(patchi->structured == NO||meshi->patch_times==NULL||meshi->patch_timeslist==NULL)continue;
        meshi->patch_itime=meshi->patch_timeslist[itimes];
        if(patchi->compression_type==UNCOMPRESSED){

          meshi->patchval_iframe  = meshi->patchval  + meshi->patch_itime*meshi->npatchsize;
          meshi->cpatchval_iframe = meshi->cpatchval + meshi->patch_itime * meshi->npatchsize;
        }
        else{
          UncompressBoundaryDataBNDF(meshi, meshi->patch_itime);
        }
      }
    }
    if(showiso==1){
      isodata *isoi;
      meshdata *meshi;

      CheckMemory;
      for(i=0;i<global_scase.nisoinfo;i++){
        isoi = global_scase.isoinfo + i;
        meshi = global_scase.meshescoll.meshinfo + isoi->blocknumber;
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

  npartloaded = 0;
  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo+i;
    if(parti->loaded==1)npartloaded++;
  }

  nsliceloaded = 0;
  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(slicei->loaded==1)nsliceloaded++;
  }

  nvsliceloaded = 0;
  for(i = 0; i<global_scase.slicecoll.nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo+i;
    if(vslicei->loaded==1)nvsliceloaded++;
  }

  nisoloaded = 0;
  for(i = 0; i<global_scase.nisoinfo; i++){
    isodata *isoi;

    isoi = global_scase.isoinfo+i;
    if(isoi->loaded==1)nisoloaded++;
  }

  npatchloaded = 0;
  for(i = 0; i<global_scase.npatchinfo; i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==1)npatchloaded++;
  }

  nsmoke3dloaded = 0;
  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    if(smoke3di->loaded==1)nsmoke3dloaded++;
  }

  nplot3dloaded = 0;
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loaded==1)nplot3dloaded++;
  }

  nvolsmoke3dloaded = 0;
  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo+i;
    vr = meshi->volrenderinfo;
    if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
    if(vr->loaded==1)nvolsmoke3dloaded++;
  }

  npart5loaded = 0;
  npartloaded = 0;
  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo+i;
    if(parti->loaded==1)npartloaded++;
    if(parti->loaded==1)npart5loaded++;
  }
}

/* ------------------ UpdateShow ------------------------ */

void UpdateShow(void){
  int i,sliceflag,vsliceflag,partflag,patchflag,isoflag,smoke3dflag,tisoflag,showdeviceflag;
  int slicecolorbarflag;
  int shooter_flag;
  int showhrrflag;
  int plot2dflag;
  int showhvacflag;

  UpdateFileLoad();
  showtime             = 0;
  showtime2            = 0;
  showplot3d           = 0;
  showpatch            = 0;
  showslice            = 0;
  showvslice           = 0;
  showsmoke            = 0;
  showzone             = 0;
  showiso              = 0;
  showvolrender        = 0;
  have_extreme_mindata = 0;
  have_extreme_maxdata = 0;
  showshooter          = 0;
  show3dsmoke          = 0;
  smoke3dflag          = 0;
  showtours            = 0;
  showdeviceflag       = 0;
  showhrrflag          = 0;
  showhvacflag         = 0;
  visTimeParticles=1; visTimeSlice=1; visTimeBoundary=1; visTimeZone=1; visTimeIso=1;

  drawing_boundary_files = 0;

  RenderTime=0;

  if(vis_hrr_plot==1&&global_scase.hrrptr!=NULL)showhrrflag = 1;

  if(global_scase.hvaccoll.hvacductvar_index >= 0 || global_scase.hvaccoll.hvacnodevar_index >= 0){
    showhvacflag = 1;
  }

  if(showdevice_val==1||vis_device_plot!=DEVICE_PLOT_HIDDEN){
    for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo+i;
      if(devicei->type2==devicetypes_index&&devicei->object->visible==1&&devicei->show==1){
        showdeviceflag = 1;
        break;
      }
    }
  }

  {
    tourdata *touri;

    if(global_scase.tourcoll.ntourinfo>0){
      for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
        touri = global_scase.tourcoll.tourinfo + i;
        if(touri->display==1){
          showtours=1;
          break;
        }
      }
    }
  }
  {
    int ii;

    for(ii=0;ii<global_scase.smoke3dcoll.nsmoke3dinfo;ii++){
      smoke3ddata *smoke3di;

      smoke3di = global_scase.smoke3dcoll.smoke3dinfo + ii;
      if(smoke3di->loaded==1&&smoke3di->display==1){
        smoke3dflag = 1;
        break;
      }
    }
  }
  if(nvolrenderinfo>0&&usevolrender==1){
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = global_scase.meshescoll.meshinfo + i;
      vr = meshi->volrenderinfo;
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
      sd = global_scase.slicecoll.sliceinfo+i;
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
      slicedata *sd;

      i=slice_loaded_list[ii];
      sd = global_scase.slicecoll.sliceinfo+i;
      if(sd->display==0||sd->slicefile_labelindex!=slicefile_labelindex)continue;
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
        sd = global_scase.slicecoll.sliceinfo+i;
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
        sd = global_scase.slicecoll.sliceinfo+i;
        if(sd->display==0||sd->slicefile_labelindex!=slicefile_labelindex)continue;
        if(sd->extreme_min==1){
          have_extreme_mindata=1;
          break;
        }
      }
    }
    for(i=0;i<global_scase.npatchinfo;i++){
      patchdata *patchi;

      patchi=global_scase.patchinfo+i;
      if(patchi->loaded == 0)continue;
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
    for(i=0;i<global_scase.nisoinfo;i++){
      isodata *isoi;

      isoi = global_scase.isoinfo+i;
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
    for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
      vslicedata *vd;
      slicedata *sd;

      vd = global_scase.slicecoll.vsliceinfo+i;
      if(vd->loaded==0||vd->display==0)continue;
      sd = global_scase.slicecoll.sliceinfo + vd->ival;

      if(sd->slicefile_labelindex!=slicefile_labelindex)continue;
      if(sd->volslice==1&&sd->slice_filetype==SLICE_NODE_CENTER&&vis_gslice_data==1)SHOW_gslice_data=1;
      vsliceflag=1;
      break;
    }
    for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
      slicedata *sd;
      vslicedata *vd;

      vd = global_scase.slicecoll.vsliceinfo+i;
      sd = global_scase.slicecoll.sliceinfo + vd->ival;
      if(vd->loaded==0||vd->display==0)continue;
      if(global_scase.slicecoll.sliceinfo[vd->ival].slicefile_labelindex!=slicefile_labelindex)continue;
      if(sd->constant_color!=NULL)continue;
      vslicecolorbarflag=1;
      break;
    }
  }

  patchflag=0;
  if(visTimeBoundary==1){
    for(i = 0; i < global_scase.ngeominfo; i++){
      geomdata *geomi;

      geomi = global_scase.geominfo + i;
      geomi->patchactive = 0;
    }
    wall_cell_color_flag=0;
    for(i=0;i<global_scase.npatchinfo;i++){
      patchdata *patchi;

      patchi=global_scase.patchinfo+i;
      if(patchi->loaded == 0)continue;
      if(patchi->boundary == 1 && patchi->display == 1 && patchi->shortlabel_index == iboundarytype){
        if(strcmp(patchi->label.shortlabel, "wc") == 0)wall_cell_color_flag = 1;
        patchflag = 1;
        if(patchi->extreme_max == 1)have_extreme_maxdata = 1;
        if(patchi->extreme_min == 1)have_extreme_mindata = 1;
        if(patchi->geominfo != NULL)patchi->geominfo->patchactive = 1;
      }
    }
  }

  partflag=0;
  if(visParticles==1&&visTimeParticles==1){
    for(i=0;i<global_scase.npartinfo;i++){
      partdata *parti;

      parti = global_scase.partinfo + i;
      if(parti->loaded==1&&parti->display==1){
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

  plot2dflag = 0;
  if(HavePlot2D(NULL,NULL)==1)plot2dflag = 1;

  shooter_flag=0;
  if(visShooter!=0&&shooter_active==1){
    shooter_flag=1;
  }

  if( plotstate==DYNAMIC_PLOTS &&
    ( showdeviceflag==1 || showhrrflag==1 || sliceflag==1 || vsliceflag==1 || partflag==1 || patchflag==1 ||
    shooter_flag==1|| smoke3dflag==1 || showtours==1 || showhvacflag == 1 || plot2dflag == 1 ||
    (ReadZoneFile==1&&visZone==1&&visTimeZone==1)||showvolrender==1
    )
    )showtime=1;
  if(plotstate==DYNAMIC_PLOTS&&ReadIsoFile==1&&visAIso!=0&&isoflag==1)showtime2=1;
  if(plotstate==DYNAMIC_PLOTS){
    if(smoke3dflag==1)show3dsmoke=1;
    if(partflag==1)showsmoke=1;
    if(patchflag==1)showpatch=1;
    drawing_boundary_files = showpatch;

    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi=global_scase.meshescoll.meshinfo+i;
      meshi->visInteriorBoundaries=0;
    }
    if(showpatch==1&&vis_boundary_type[0]==1){
      for(i=0;i<global_scase.meshescoll.nmeshes;i++){
        patchdata *patchi;
        meshdata *meshi;

        meshi=global_scase.meshescoll.meshinfo+i;
        if(meshi->patch_times==NULL)continue;
        patchi = global_scase.patchinfo+meshi->patchfilenum;
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
  if(showsmoke==1||showpatch==1||showslice==1||showvslice==1||showzone==1||showiso==1)RenderTime=1;
  if(showtours==1||show3dsmoke==1||touring==1||showvolrender==1)RenderTime=1;
  if(showhvacflag == 1)RenderTime = 1;
  if(showshooter==1)RenderTime=1;
  if(plotstate==STATIC_PLOTS&&nplot3dloaded>0&&plotn>0&&plotn<=numplot3dvars)showplot3d=1;
  if(showplot3d==1){
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      int ii;

      meshi=global_scase.meshescoll.meshinfo+i;
      ii=meshi->plot3dfilenum;
      if(ii==-1)continue;
      if(global_scase.plot3dinfo[ii].loaded==0)continue;
      if(global_scase.plot3dinfo[ii].display==0)continue;
      if(global_scase.plot3dinfo[ii].extreme_min[plotn-1]==1)have_extreme_mindata=1;
    }
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      int ii;

      meshi=global_scase.meshescoll.meshinfo+i;
      ii=meshi->plot3dfilenum;
      if(ii==-1)continue;
      if(global_scase.plot3dinfo[ii].loaded==0)continue;
      if(global_scase.plot3dinfo[ii].display==0)continue;
      if(global_scase.plot3dinfo[ii].extreme_max[plotn-1]==1)have_extreme_maxdata=1;
    }
  }

  num_colorbars=0;
  if(plotstate==DYNAMIC_PLOTS){
    if(partflag==1&&parttype!=0)num_colorbars++;
    if(slicecolorbarflag==1||vslicecolorbarflag==1)num_colorbars++;
    if(patchflag==1&&wall_cell_color_flag==0)num_colorbars++;
    if(ReadZoneFile==1)num_colorbars++;
    if(global_scase.hvaccoll.hvacductvar_index >= 0)num_colorbars++;
    if(global_scase.hvaccoll.hvacnodevar_index >= 0)num_colorbars++;

    if(tisoflag==1){
      showiso_colorbar = 1;
      num_colorbars++;
    }
  }
  if(nplot3dloaded>0&&num_colorbars==0)num_colorbars=1;

  // note: animated iso-contours do not need a colorbar, so we don't test for isosurface files

  if((showtime == 1 || showplot3d == 1) && (visColorbarVertical == 1|| visColorbarHorizontal == 1)){
    if(old_draw_colorlabel == 0)updatemenu = 1;
    old_draw_colorlabel = 1;
  }
  else{
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

/* ------------------ GetDataTimeFrame ------------------------ */

int GetDataTimeFrame(float time, unsigned char *times_map, float *times, int ntimes){
  int i;

  if(times_map == NULL){
    return GetTimeInterval(time, times, ntimes);
  }
  else{
    for(i = 0; i < ntimes - 1; i++){
      if(times_map[i] == 0)continue;
      if(times[i]<=time&&time<times[i+1])return i;
    }
  }
  return ntimes-1;
}

/* ------------------ SynchTimes ------------------------ */

void SynchTimes(void){
  int n,i,istart,igrid;

  /* synchronize smooth blockage times */

  for(n=0;n<nglobal_times;n++){
    int j,jj;

  /* synchronize tour times */

    for(j=0;j<global_scase.tourcoll.ntourinfo;j++){
      tourdata *tourj;

      tourj = global_scase.tourcoll.tourinfo + j;
      if(tourj->display==0)continue;
      tourj->timeslist[n] = GetDataTimeFrame(global_times[n], NULL, tourj->path_times,tourj->ntimes);
    }

  /* synchronize geometry times */

    for(j=0;j<ngeominfoptrs;j++){
      geomdata *geomi;
      unsigned char *times_map = NULL;

      geomi = geominfoptrs[j];
      if(geomi->loaded==0||geomi->display==0)continue;
      if(geomi->geomtype == GEOM_ISO&& geomi->block_number >= 0){
        times_map = global_scase.meshescoll.meshinfo[geomi->block_number].iso_times_map;
      }
      geomi->timeslist[n] = GetDataTimeFrame(global_times[n], times_map, geomi->times, geomi->ntimes);
    }

  /* synchronize particle times */

    for(j=0;j<global_scase.npartinfo;j++){
      partdata *parti;

      parti=global_scase.partinfo+j;
      if(parti->loaded==0)continue;
      parti->timeslist[n] = GetDataTimeFrame(global_times[n], parti->times_map, parti->times,parti->ntimes);
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

      sd = global_scase.slicecoll.sliceinfo + slice_loaded_list[jj];
      if(sd->slice_filetype == SLICE_GEOM){
        sd->patchgeom->geom_timeslist[n] = GetDataTimeFrame(global_times[n], sd->patchgeom->geom_times_map , sd->patchgeom->geom_times, sd->ntimes);
      }
      else{
        sd->timeslist[n] = GetDataTimeFrame(global_times[n], sd->times_map, sd->times, sd->ntimes);
        assert(sd->times_map == NULL || sd->times_map[sd->timeslist[n]] != 0);
      }
    }

  /* synchronize smoke times */
    {
      smoke3ddata *smoke3di;

      for(jj=0;jj<global_scase.smoke3dcoll.nsmoke3dinfo;jj++){
        smoke3di = global_scase.smoke3dcoll.smoke3dinfo + jj;
        if(smoke3di->loaded==0)continue;
        smoke3di->timeslist[n] = GetDataTimeFrame(global_times[n], smoke3di->times_map, smoke3di->times,smoke3di->ntimes);
      }
    }

  /* synchronize patch times */

    for(j=0;j<global_scase.npatchinfo;j++){
      patchdata *patchi;

      patchi = global_scase.patchinfo + j;
      if(patchi->loaded==0)continue;
      if(patchi->structured == YES)continue;
      patchi->geom_timeslist[n] = GetDataTimeFrame(global_times[n], patchi->geom_times_map, patchi->geom_times,patchi->ngeom_times);
    }
    for(j=0;j<global_scase.meshescoll.nmeshes;j++){
      patchdata *patchi;
      meshdata *meshi;

      meshi=global_scase.meshescoll.meshinfo+j;
      if(meshi->patchfilenum<0||meshi->patch_times==NULL)continue;
      patchi=global_scase.patchinfo+meshi->patchfilenum;
      if(patchi->structured == NO||patchi->loaded==0)continue;
      meshi->patch_timeslist[n] = GetDataTimeFrame(global_times[n], meshi->patch_times_map, meshi->patch_times,patchi->ntimes);
    }

  /* synchronize isosurface times */

    for(igrid=0;igrid<global_scase.meshescoll.nmeshes;igrid++){
      meshdata *meshi;

      meshi=global_scase.meshescoll.meshinfo+igrid;
      if(meshi->iso_times==NULL)continue;
      meshi->iso_timeslist[n] = GetDataTimeFrame(global_times[n], meshi->iso_times_map, meshi->iso_times,meshi->niso_times);
    }

  /* synchronize volume render times */

    if(nvolrenderinfo>0){
      for(igrid=0;igrid<global_scase.meshescoll.nmeshes;igrid++){
        volrenderdata *vr;
        meshdata *meshi;

        meshi=global_scase.meshescoll.meshinfo+igrid;
        vr = meshi->volrenderinfo;
        if(vr->smokeslice==NULL)continue;
        if(vr->loaded==0||vr->display==0)continue;
        if(vr->times==NULL)continue;
        vr->timeslist[n] = GetDataTimeFrame(global_times[n], vr->smokeslice->times_map, vr->times,vr->ntimes);
      }
    }
    /* synchronize zone times */

    if(showzone==1){
      zone_timeslist[n] = GetDataTimeFrame(global_times[n], NULL, zone_times,nzone_times);
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
  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
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
  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
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
  for(i = 0; i < global_scase.nisoinfo; i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(strcmp(fileptr, isoi->file) == 0){
      fprintf(stream, "// LOADFILE\n");
      fprintf(stream, "//  %s\n", isoi->file);
      fprintf(stream, "LOADISOM\n");
      fprintf(stream, " %s\n", isoi->surface_label.longlabel);
      fprintf(stream, " %i\n", isoi->blocknumber+1);
      return 1;
    }

  }
  for(i = 0; i < global_scase.npatchinfo; i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
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

/* ------------------ GetTime ------------------------ */

float GetTime(void){
  if(global_times != NULL)return global_times[CLAMP(itimes,0,nglobal_times)];
  return 0.0;
}

  /* ------------------ MergeGlobalTimes ------------------------ */

void MergeGlobalTimes(float *t, int n);

void TruncateGlobalTimes(void){
  int i, ibeg, iend;

  if(use_tload_begin == 0 && use_tload_end == 0)return;
  if(nglobal_times==0 || global_times==NULL)return;
  ibeg = 0;
  iend = nglobal_times - 1;
  if(use_tload_begin==1){
    for(i=0;i<nglobal_times;i++){
      if(global_scase.tload_begin<global_times[i]){
        ibeg = i;
        break;
      }
    }
  }
  if(use_tload_end==1){
    for(i=nglobal_times-1;i>=0;i--){
      if(global_times[i]<global_scase.tload_end){
        iend = i;
        break;
      }
    }
  }
  for(i=ibeg;i<=iend;i++){
    global_times[i-ibeg] = global_times[i];
  }
  nglobal_times = iend + 1 - ibeg;
  if(use_tload_begin==1){
    MergeGlobalTimes(&global_scase.tload_begin, 1);
  }
  if(use_tload_end==1){
    MergeGlobalTimes(&global_scase.tload_end, 1);
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
 //   assert(dt>=0.0);
    dt_eps = MIN(dt_eps, ABS(dt)/2.0);
  }
  if(nglobal_times>1){
    for(i = 1; i<nglobal_times; i++){
      float dt;

      dt = global_times[i]-global_times[i-1];
  //    assert(dt>=0.0);
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

    unsigned char *times_map = NULL;
    MakeTimesMap(global_times, &times_map, nglobal_times);
    int n;

    n = 0;
    for(i = 0; i < nglobal_times; i++){
      if(times_map==NULL || times_map[i] == 1){
        if(i != n)global_times[n] = global_times[i];
        n++;
      }
    }
    nglobal_times = n;
    FREEMEMORY(times_map);
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

  unsigned char *times_map = NULL;
  NewMemory((void **)&times_map, nglobal_times*sizeof(unsigned char));
  MakeTimesMap(global_times, &times_map, nglobal_times);
  int n;

  n = 0;
  for(i = 0; i < nglobal_times; i++){
    if(times_map==NULL || times_map[i] == 1){
      if(i != n)global_times[n] = global_times[i];
      n++;
    }
  }
  nglobal_times = n;
  FREEMEMORY(times_map);
}

  /* ------------------ UpdateTimes ------------------------ */

void UpdateTimes(void){
  int i;

  INIT_PRINT_TIMER(setup_timer);
  GetGeomInfoPtrs(0);

  UpdateShow();

  UpdateRGBColors(colorbar_select_index);
  HandleColorbarIndex(colorbar_select_index);

  CheckMemory;
  nglobal_times = 0;

  FREEMEMORY(global_times);
  nglobal_times = 0;
  FREEMEMORY(times_buffer);
  ntimes_buffer = 0;

  PRINT_TIMER(setup_timer, "UpdateTimes: setup");

  // determine min time, max time and number of times

  if(current_script_command!=NULL&& IS_LOADRENDER){
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
  if(global_scase.hvaccoll.hvacductvar_index >= 0){
    MergeGlobalTimes(global_scase.hvaccoll.hvacductvalsinfo->times, global_scase.hvaccoll.hvacductvalsinfo->ntimes);
  }
  if(global_scase.hvaccoll.hvacnodevar_index >= 0){
    MergeGlobalTimes(global_scase.hvaccoll.hvacnodevalsinfo->times, global_scase.hvaccoll.hvacnodevalsinfo->ntimes);
  }
  if(use_tload_begin==1){
    MergeGlobalTimes(&global_scase.tload_begin, 1);
  }
  if(use_tload_end==1){
    MergeGlobalTimes(&global_scase.tload_end, 1);
  }

  if(vis_hrr_plot==1&&global_scase.hrrptr!=NULL){
    MergeGlobalTimes(global_scase.timeptr->vals, global_scase.timeptr->nvals);
  }
  {
    float *times = NULL;
    int ntimes = 0;

    if(HavePlot2D(&times, &ntimes)==1){
      MergeGlobalTimes(times, ntimes);
    }
  }
  if(showdevice_val==1||vis_device_plot!=DEVICE_PLOT_HIDDEN){
    for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo+i;
      if(devicei->object->visible==0||devicei->nvals==0)continue;
      if(devicei->show == 0)continue;
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
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *pd;

    pd = global_scase.plot3dinfo+i;
    if(pd->loaded==1){
      float ptime[1];

      ptime[0] = pd->time;
      MergeGlobalTimes(ptime, 1);
    }
  }
  INIT_PRINT_TIMER(slice_timer);
  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *sd;

    sd=global_scase.slicecoll.sliceinfo+i;
    if(sd->loaded==1||sd->vloaded==1){
      MergeGlobalTimes(sd->times, sd->ntimes);
    }
  }
  PRINT_TIMER(slice_timer, "UpdateTimes: slice");
  for(i=0;i<global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->loaded==1&&patchi->structured == NO){
      MergeGlobalTimes(patchi->geom_times, patchi->ngeom_times);
    }
  }
  INIT_PRINT_TIMER(boundary_timer);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    patchdata *patchi;
    meshdata *meshi;
    int filenum;

    meshi=global_scase.meshescoll.meshinfo+i;
    filenum =meshi->patchfilenum;
    if(filenum!=-1){
      patchi=global_scase.patchinfo+filenum;
      if(patchi->loaded==1&&patchi->structured == YES){
        MergeGlobalTimes(meshi->patch_times, patchi->ntimes);
      }
    }
  }
  PRINT_TIMER(boundary_timer, "UpdateTimes: boundary");
  if(ReadZoneFile==1&&visZone==1){
    MergeGlobalTimes(zone_times, nzone_times);
  }
  if(ReadIsoFile==1&&visAIso!=0){
    INIT_PRINT_TIMER(iso_timer);
    for(i=0;i<global_scase.nisoinfo;i++){
      meshdata *meshi;
      isodata *ib;

      ib = global_scase.isoinfo+i;
      if(ib->geomflag==1||ib->loaded==0)continue;
      meshi=global_scase.meshescoll.meshinfo + ib->blocknumber;
      MergeGlobalTimes(meshi->iso_times, meshi->niso_times);
    }
    PRINT_TIMER(iso_timer, "UpdateTimes: iso");
  }
  if(nvolrenderinfo>0){
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      volrenderdata *vr;
      meshdata *meshi;

      meshi=global_scase.meshescoll.meshinfo+i;
      vr = meshi->volrenderinfo;
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      MergeGlobalTimes(vr->times, vr->ntimes);
    }
  }
  {
    smoke3ddata *smoke3di;

    if(nsmoke3dloaded>0&&vis3DSmoke3D==1){
      INIT_PRINT_TIMER(smoke3d_timer);
      for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
        smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
        if(smoke3di->loaded==0)continue;
        MergeGlobalTimes(smoke3di->times, smoke3di->ntimes);
      }
      PRINT_TIMER(smoke3d_timer, "UpdateTimes: smoke3d");
    }
  }
  for(i = 0; i < global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(parti->loaded == 0)continue;
    MergeGlobalTimes(parti->times, parti->ntimes);
  }

  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *touri;

    touri = global_scase.tourcoll.tourinfo + i;
    if(touri->display==0)continue;
    MergeGlobalTimes(touri->path_times, touri->ntimes);
  }

  TruncateGlobalTimes();

  //--------------------------------------------------------------

  INIT_PRINT_TIMER(timer_allocate);
  CheckMemory;

  // allocate memory for individual timelist arrays

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    FREEMEMORY(geomi->timeslist);
    if(nglobal_times>0)NewMemory((void **)&geomi->timeslist,nglobal_times*sizeof(int));
  }
  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;

    parti=global_scase.partinfo+i;
    FREEMEMORY(parti->timeslist);
    if(nglobal_times>0)NewMemory((void **)&parti->timeslist,nglobal_times*sizeof(int));
  }
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *touri;

    touri=global_scase.tourcoll.tourinfo + i;
    if(touri->display==0)continue;
    FREEMEMORY(touri->timeslist);
    if(nglobal_times>0)NewMemory((void **)&touri->timeslist,nglobal_times*sizeof(int));
  }
  FREEMEMORY(shooter_timeslist);
  if(visShooter!=0&&shooter_active==1){
    NewMemory((void **)&shooter_timeslist,nshooter_frames*sizeof(int));
  }

  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *sd;

    sd = global_scase.slicecoll.sliceinfo + i;
    if(sd->loaded==0)continue;
    if(sd->slice_filetype == SLICE_GEOM){
      FREEMEMORY(sd->patchgeom->geom_timeslist);
      if(nglobal_times > 0)NewMemory((void **)&(sd->patchgeom->geom_timeslist), nglobal_times * sizeof(int));
    }
    else{
      FREEMEMORY(sd->timeslist);
      if(nglobal_times > 0)NewMemory((void **)&sd->timeslist, nglobal_times * sizeof(int));
    }
  }
  if(nvolrenderinfo>0){
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = global_scase.meshescoll.meshinfo + i;
      vr = meshi->volrenderinfo;
      if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      FREEMEMORY(vr->timeslist);
      if(nglobal_times>0)NewMemory((void **)&vr->timeslist,nglobal_times*sizeof(int));
    }
  }
  {
    smoke3ddata *smoke3di;

    for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
      smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
      FREEMEMORY(smoke3di->timeslist);
      if(nglobal_times>0)NewMemory((void **)&smoke3di->timeslist,nglobal_times*sizeof(int));
    }
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    if(meshi->iso_times==NULL)continue;
    FREEMEMORY(meshi->iso_timeslist);
    if(nglobal_times>0)NewMemory((void **)&meshi->iso_timeslist,  nglobal_times*sizeof(int));
  }

  for(i=0;i<global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    FREEMEMORY(patchi->geom_timeslist);
    if(patchi->structured == YES)continue;
    if(patchi->geom_times==NULL)continue;
    if(nglobal_times>0)NewMemory((void **)&patchi->geom_timeslist,nglobal_times*sizeof(int));
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    FREEMEMORY(global_scase.meshescoll.meshinfo[i].patch_timeslist);
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    if(global_scase.meshescoll.meshinfo[i].patch_times==NULL)continue;
    if(nglobal_times>0)NewMemory((void **)&global_scase.meshescoll.meshinfo[i].patch_timeslist,nglobal_times*sizeof(int));
  }

  FREEMEMORY(zone_timeslist);
  if(nglobal_times>0)NewMemory((void **)&zone_timeslist,     nglobal_times*sizeof(int));

  FREEMEMORY(targtimeslist);
  if(nglobal_times>0)NewMemory((void **)&targtimeslist,  nglobal_times*sizeof(int));
  CheckMemory;

  if(current_script_command!=NULL&&
    (current_script_command->command==SCRIPT_VOLSMOKERENDERALL||current_script_command->command==SCRIPT_ISORENDERALL)
    ){
    if(current_script_command->first==1){
      current_script_command->first=0;
    }
  }
  PRINT_TIMER(timer_allocate, "UpdateTimes: allocate memory");

  // initialize individual time pointers

  INIT_PRINT_TIMER(timer_setpointers);
  izone=0;
  ResetItimes0();
  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    geomi->itime=0;
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    meshi->patch_itime=0;
  }
  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *sd;

    sd = global_scase.slicecoll.sliceinfo + i;
    sd->itime=0;
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    if(meshi->iso_times==NULL)continue;
    meshi->iso_itime=0;
  }
  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    parti->itime=0;
  }
  PRINT_TIMER(timer_setpointers, "UpdateTimes: set pointer");

  /* determine visibility of each blockage at each time step */

  INIT_PRINT_TIMER(timer_visblocks);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
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
  PRINT_TIMER(timer_visblocks, "UpdateTimes: block vis");

  /* determine state of each device at each time step */

  INIT_PRINT_TIMER(timer_device);
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo + i;
    if(devicei->object->visible == 0 || devicei->show == 0)continue;
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
  PRINT_TIMER(timer_device, "UpdateTimes: device state");

  /* determine visibility of each vent at each time step */

  INIT_PRINT_TIMER(timer_vent);

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
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

  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    int j;
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
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
  PRINT_TIMER(timer_vent, "UpdateTimes: vent state");

  if(nglobal_times>0){
    INIT_PRINT_TIMER(timer_synch_times);
    SynchTimes();
    PRINT_TIMER(timer_synch_times, "timer: SynchTimes");
  }
  if(nglobal_times>0){
    INIT_PRINT_TIMER(timer_labels);
    UpdateTimeLabels();
    PRINT_TIMER(timer_labels, "UpdateTimes: time labels");
    INIT_PRINT_TIMER(timer_bounds);
    GLUIUpdateTimeBounds(global_times[0],global_times[nglobal_times-1]);
    PRINT_TIMER(timer_bounds, "UpdateTimes: time bound");
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
      for(i=0;i<global_scase.meshescoll.nmeshes;i++){
        plot3ddata *ploti;
        meshdata *meshi;

        meshi=global_scase.meshescoll.meshinfo + i;
        if(meshi->plot3dfilenum==-1)continue;
        ploti = global_scase.plot3dinfo + meshi->plot3dfilenum;
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
      if(global_scase.hvaccoll.hvacductvar_index>=0||global_scase.hvaccoll.hvacnodevar_index>=0){
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(vis_hrr_plot==1&&global_scase.hrrptr!=NULL){
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(HavePlot2D(NULL,NULL)==1){
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(showdevice_val==1||vis_device_plot!=DEVICE_PLOT_HIDDEN){
        for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
          devicedata *devicei;

          devicei = global_scase.devicecoll.deviceinfo+i;
          if(devicei->object->visible == 0 || devicei->show == 0)continue;
          if(devicei->type2==devicetypes_index){
            stept = 1;
            return DYNAMIC_PLOTS;
          }
        }
      }
      for(i=0;i<nslice_loaded;i++){
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + slice_loaded_list[i];
        if(slicei->display==0||slicei->slicefile_labelindex!=slicefile_labelindex)continue;
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(visGrid==0)stept = 1;
      for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
        vslicedata *vslicei;

        vslicei = global_scase.slicecoll.vsliceinfo + i;
        if(vslicei->display==0||vslicei->vslicefile_labelindex!=slicefile_labelindex)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<global_scase.npatchinfo;i++){
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->loaded == 0)continue;
        if(patchi->display == 1){
          if(patchi->boundary == 1 && patchi->shortlabel_index == iboundarytype)return DYNAMIC_PLOTS;
          if(patchi->boundary == 0 && patchi->shortlabel_index == slicefile_labelindex)return DYNAMIC_PLOTS;
        }
      }
      for(i=0;i<global_scase.npartinfo;i++){
        partdata *parti;

        parti = global_scase.partinfo + i;
        if(parti->loaded==0||parti->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<global_scase.nisoinfo;i++){
        isodata *isoi;

        isoi = global_scase.isoinfo + i;
        if(isoi->loaded==0)continue;
        if(isoi->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<global_scase.nzoneinfo;i++){
        zonedata *zonei;

        zonei = global_scase.zoneinfo + i;
        if(zonei->loaded==0||zonei->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
        tourdata *touri;

        touri = global_scase.tourcoll.tourinfo + i;
        if(touri->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
        smoke3ddata *smoke3di;

        smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
        if(smoke3di->loaded==0||smoke3di->display==0)continue;
        return DYNAMIC_PLOTS;
      }
      if(nvolrenderinfo>0){
        for(i=0;i<global_scase.meshescoll.nmeshes;i++){
          meshdata *meshi;
          volrenderdata *vr;

          meshi = global_scase.meshescoll.meshinfo + i;
          vr = meshi->volrenderinfo;
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
      assert(FFALSE);
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
  if(current_script_command==NULL||(current_script_command->command!=SCRIPT_VOLSMOKERENDERALL&&current_script_command->command!=SCRIPT_ISORENDERALL)){
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
  GLUIUpdateColorTableList(ncolortableinfo_old);
}

/* ------------------ HaveFire ------------------------ */

int HaveFireLoaded(void){
  int i;

  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    if(smoke3di->loaded==1){
      if(smoke3di->type==HRRPUV_index)return HRRPUV_index;
      if(smoke3di->type==TEMP_index)return TEMP_index;
    }
  }
  return NO_FIRE;
}

/* ------------------ HaveSoot ------------------------ */

int HaveSootLoaded(void){
  int i;

  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    if(smoke3di->loaded==1&&smoke3di->extinct>0.0)return GetSmoke3DType(&global_scase, smoke3di->label.shortlabel);
  }
  return NO_SMOKE;
}

/* ------------------ UpdateIsoIni ------------------------ */

void UpdateIsoIni(void){
  int i;

  for(i = 0; i < niso_bounds; i++){
    if(isobounds[i].ini_defined == 1){
      isobounds[i].dlg_setvalmin = isobounds[i].ini_setvalmin;
      isobounds[i].dlg_setvalmax = isobounds[i].ini_setvalmax;
      isobounds[i].dlg_valmin    = isobounds[i].ini_valmin;
      isobounds[i].dlg_valmax    = isobounds[i].ini_valmax;
    }
  }
}

/* ------------------ Bytes2Label ------------------------ */

char *Bytes2Label(char *label, FILE_SIZE bytes){
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

/* ------------------ OutputFrameSteps ------------------------ */

#ifdef pp_FRAME
void OutputFrameSteps(void){
  int i, count, frames_read, show;
  float total_time, total_wrapup_time;
  FILE_SIZE bytes_read;
  char size_label[256], geom_slice_label[256], slice_label[256], part_label[256];
  char iso_label[256],  smoke_label[256],      bound_label[256], geom_bound_label[256];
  char time_label[256], time_label2[256],      time_label3[256];
  char file_count_label[256];

  show = 0;
  strcpy(geom_slice_label, "");
  strcpy(slice_label, "");
  strcpy(part_label, "");
  strcpy(iso_label, "");
  strcpy(smoke_label, "");
  strcpy(bound_label, "");
  strcpy(geom_bound_label, "");

  //*** slice files

  count             = 0;
  bytes_read        = 0;
  frames_read       = 0;
  total_time        = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0;i < global_scase.slicecoll.nsliceinfo;i++){
    slicedata *slicei;
    framedata *frameinfo=NULL;

    slicei = global_scase.slicecoll.sliceinfo + i;
    if(slicei->loaded == 0)continue;
    if(slicei->slice_filetype == SLICE_GEOM)continue;
    frameinfo = slicei->frameinfo;
    if(frameinfo == NULL || frameinfo->update == 0)continue;
    count++;
    frameinfo->update = 0;
    frames_read = MAX(frames_read, frameinfo->frames_read);
    bytes_read += frameinfo->bytes_read;
    total_time += frameinfo->load_time;
    total_wrapup_time += frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(slice_label, "   slice(structured): loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(slice_label, time_label);
    show = 1;
  }

  //*** geometry slice files

  count = 0;
  bytes_read = 0;
  frames_read = 0;
  total_time = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0; i < global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;
    framedata *frameinfo = NULL;

    slicei = global_scase.slicecoll.sliceinfo + i;
    if(slicei->loaded == 0 || slicei->slice_filetype != SLICE_GEOM)continue;
    frameinfo = slicei->frameinfo;
    if(slicei->patchgeom != NULL)frameinfo = slicei->patchgeom->frameinfo;
    if(frameinfo == NULL || frameinfo->update == 0)continue;
    frameinfo->update = 0;
    count++;
    frames_read = MAX(frames_read, frameinfo->frames_read);
    bytes_read += frameinfo->bytes_read;
    total_time += frameinfo->load_time;
    total_wrapup_time += frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(geom_slice_label, "         slice(geom): loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(geom_slice_label, time_label);
    show = 1;
  }

  //*** 3d smoke files

  count             = 0;
  bytes_read        = 0;
  frames_read       = 0;
  total_time        = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->loaded == 0 || smoke3di->frameinfo == NULL || smoke3di->frameinfo->update == 0)continue;
    smoke3di->frameinfo->update = 0;
    count++;
    frames_read = MAX(frames_read, smoke3di->frameinfo->frames_read);
    bytes_read += smoke3di->frameinfo->bytes_read;
    total_time += smoke3di->frameinfo->load_time;
    total_wrapup_time += smoke3di->frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(smoke_label, "            3D smoke: loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(smoke_label, time_label);
    show = 1;
  }

  //*** boundary files

  count = 0;
  bytes_read = 0;
  frames_read = 0;
  total_time = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0;i < global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->loaded == 0)continue;
    if(patchi->patch_filetype !=PATCH_STRUCTURED_NODE_CENTER &&  patchi->patch_filetype != PATCH_STRUCTURED_CELL_CENTER)continue;
    if(patchi->frameinfo == NULL || patchi->frameinfo->update == 0)continue;

    patchi->frameinfo->update = 0;
    count++;
    frames_read = MAX(frames_read, patchi->frameinfo->frames_read);
    bytes_read += patchi->frameinfo->bytes_read;
    total_time += patchi->frameinfo->load_time;
    total_wrapup_time += patchi->frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(bound_label, "boundary(structured): loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(bound_label, time_label);
    show = 1;
  }

  //*** geometry boundary files

  count = 0;
  bytes_read = 0;
  frames_read = 0;
  total_time = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0;i < global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->loaded == 0 || patchi->patch_filetype != PATCH_GEOMETRY_BOUNDARY)continue;
    if(patchi->frameinfo == NULL || patchi->frameinfo->update == 0)continue;
    patchi->frameinfo->update = 0;
    count++;
    frames_read = MAX(frames_read, patchi->frameinfo->frames_read);
    bytes_read += patchi->frameinfo->bytes_read;
    total_time += patchi->frameinfo->load_time;
    total_wrapup_time += patchi->frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(geom_bound_label, "      boundary(geom): loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(geom_bound_label, time_label);
    show = 1;
  }

  //*** isosurface files

  count = 0;
  bytes_read = 0;
  frames_read = 0;
  total_time = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded == 0 || isoi->frameinfo == NULL || isoi->frameinfo->update == 0)continue;
    isoi->frameinfo->update = 0;
    count++;
    frames_read = MAX(frames_read, isoi->frameinfo->frames_read);
    bytes_read += isoi->frameinfo->bytes_read;
    total_time += isoi->frameinfo->load_time;
    total_wrapup_time += isoi->frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(iso_label, "          isosurface: loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(iso_label, time_label);
    show = 1;
  }

  //*** particle files

  count = 0;
  bytes_read = 0;
  frames_read = 0;
  total_time = 0.0;
  total_wrapup_time = 0.0;
  for(i = 0;i < global_scase.npartinfo;i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(parti->loaded == 0 || parti->frameinfo == NULL || parti->frameinfo->update == 0)continue;
    parti->frameinfo->update = 0;
    count++;
    frames_read  = MAX(frames_read, parti->frameinfo->frames_read);
    bytes_read  += parti->frameinfo->bytes_read;
    total_time += parti->frameinfo->load_time;
    total_wrapup_time += parti->frameinfo->total_time;
  }
  if(count > 0){
    strcpy(file_count_label, "file");
    if(count > 1)strcpy(file_count_label, "s");
    sprintf(part_label, "            particle: loaded %i frames, %i %s, %s", frames_read, count, file_count_label, Bytes2Label(size_label, bytes_read));
    strcpy(time_label, "");
    Float2String(time_label2, total_time, ncolorlabel_digits, force_fixedpoint);
    Float2String(time_label3, total_wrapup_time, ncolorlabel_digits, force_fixedpoint);
    sprintf(time_label, " in %ss/%ss", time_label2, time_label3);
    strcat(part_label, time_label);
    show = 1;
  }
  if(show == 1){
    printf("\n");
    if(strlen(geom_bound_label) > 0)printf("%s\n", geom_bound_label);
    if(strlen(bound_label) > 0)printf("%s\n", bound_label);
    if(strlen(iso_label) > 0)printf("%s\n",   iso_label);
    if(strlen(part_label) > 0)printf("%s\n",  part_label);
    if(strlen(geom_slice_label) > 0)printf("%s\n", geom_slice_label);
    if(strlen(slice_label) > 0)printf("%s\n", slice_label);
    if(strlen(smoke_label) > 0)printf("%s\n", smoke_label);
  }
}
#endif
#ifdef pp_SHOW_UPDATE
#define SHOW_UPDATE(var) printf("updating: %s\n", #var);INIT_PRINT_TIMER(update_timer);updating=1
#define END_SHOW_UPDATE(var) PRINT_TIMER(update_timer,#var)
#else
#define SHOW_UPDATE(var)
#define END_SHOW_UPDATE(var)
#endif

/* ------------------ SetupAutoSmoke ------------------------ */

void SetupAutoSmoke(void){
  int i;

  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    smoke3di->autoload = 0;
    if(smoke3di->type == CO2_index && loadfiles_commandline[LOAD_3DCO2] == 1){
      smoke3di->autoload = 1;
      continue;
    }
    if(smoke3di->type == HRRPUV_index && loadfiles_commandline[LOAD_3DHRRPUV] == 1){
      smoke3di->autoload = 1;
      continue;
    }
    if(smoke3di->type == SOOT_index && loadfiles_commandline[LOAD_3DSOOT] == 1){
      smoke3di->autoload = 1;
      continue;
    }
    if(smoke3di->type == TEMP_index && loadfiles_commandline[LOAD_3DTEMP] == 1){
      smoke3di->autoload = 1;
      continue;
    }
  }
  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->autoload == 1)printf("loaded: %i\n", i);
  }
}

/* ------------------ UpdateShowScene ------------------------ */

void UpdateShowScene(void){
#ifdef pp_SHOW_UPDATE
  int updating = 0;
#endif

  have_fire  = HaveFireLoaded();
  have_smoke = HaveSootLoaded();
#ifdef pp_FRAME
  if(update_frame == 1){
    SHOW_UPDATE(update_frame);
    update_frame = 0;
    OutputFrameSteps();
    UpdateSliceNtimes();
    update_times = 1;
    END_SHOW_UPDATE(update_frame);
  }
#endif
  if(update_fire_histogram_now == 1){
    update_fire_histogram_now = 0;
    if(update_fire_histogram==1)GLUISmoke3dCB(UPDATE_FIRE_HISTOGRAM);
  }
#define SHOW_EXTERIOR_PATCH_DATA     32
void BoundBoundCB(int var);
  if(update_patch_vis == 1){
    BoundBoundCB(SHOW_EXTERIOR_PATCH_DATA);
    update_patch_vis = 0;
  }
  if(update_smoke3dmenulabels == 1){
    SHOW_UPDATE(update_smoke3dmenulabels);
    update_smoke3dmenulabels = 0;
    UpdateSmoke3dMenuLabels();
    END_SHOW_UPDATE(update_smoke3dmenulabels);
  }
  if(glui_meshclip_defined==1&&update_meshclip == 1){
    SHOW_UPDATE(update_meshclip);
    update_meshclip = 0;
    GLUIUpdateMeshBounds();
    END_SHOW_UPDATE(update_meshclip);
  }
  if(update_csv_load == 1){
    SHOW_UPDATE(update_csv_load);
    InitializeDeviceCsvData(LOAD);
    update_csv_load = 0;
    END_SHOW_UPDATE(update_csv_load);
  }
  if(global_scase.update_terrain_type == 1){
    SHOW_UPDATE(update_terrain_type);
    global_scase.update_terrain_type = 0;
    GLUIUpdateTerrain();
    END_SHOW_UPDATE(update_terrain_type);
  }
  if(update_iso_ini == 1){
    SHOW_UPDATE(update_iso_ini);
    UpdateIsoIni();
    update_iso_ini = 0;
    END_SHOW_UPDATE(update_iso_ini);
  }
  if(check_colorbar == 1){
    SHOW_UPDATE(check_colorbar);
    CheckLab();
    check_colorbar++;
    END_SHOW_UPDATE(check_colorbar);
  }
  if(update_colorbar_orig == 1){
    SHOW_UPDATE(update_colorbar_orig);
    UpdateColorbarOrig();
    update_colorbar_orig = 0;
    END_SHOW_UPDATE(update_colorbar_orig);
  }
  if(update_loadall_textures == 1){
    SHOW_UPDATE(update_loadall_textures);
    update_loadall_textures = 0;
    TextureShowMenu(MENU_TEXTURE_SHOWALL2);
    END_SHOW_UPDATE(update_loadall_textures);
  }
  if(update_plot2dini == 1){
    SHOW_UPDATE(update_plot2dini);
    update_plot2dini = 0;
    GLUIUpdatePlot2DINI();
    END_SHOW_UPDATE(update_plot2dini);
  }
  if(update_device_timeaverage == 1){
    SHOW_UPDATE(update_device_timeaverage);
    update_device_timeaverage = 0;
    GLUIDeviceCB(DEVICE_TIMEAVERAGE);
    END_SHOW_UPDATE(update_device_timeaverage);
  }
  if(global_scase.update_smoke_alphas==1){
    SHOW_UPDATE(update_smoke_alphas);
    global_scase.update_smoke_alphas = 0;
    UpdateSmokeAlphas();
    END_SHOW_UPDATE(update_smoke_alphas);
  }
  if(update_slice2device==1){
    SHOW_UPDATE(update_slice2device);
    update_slice2device = 0;
    Slice2Device();
    update_slicexyz = 1;
    END_SHOW_UPDATE(update_slice2device);
  }
  if(update_slicexyz==1){
    SHOW_UPDATE(update_slicexyz);
    update_slicexyz = 0;
    GLUIUpdateSliceXYZ();
    END_SHOW_UPDATE(update_slicexyz);
  }
  if(update_vectorskip == 1){
    SHOW_UPDATE(update_vectorskip);
    update_vectorskip = 0;
    UpdateVectorSkip(vectorskip);
    END_SHOW_UPDATE(update_vectorskip);
  }
  if(update_plot_label == 1){
    SHOW_UPDATE(update_plot_label);
    update_plot_label = 0;
    GLUIUpdatePlotLabel();
    END_SHOW_UPDATE(update_plot_label);
  }
  if(open_movie_dialog==1){
    SHOW_UPDATE(open_movie_dialog);
    open_movie_dialog = 0;
    if(have_slurm==1&&nmovie_queues>0){
      GLUIShowMotion(DIALOG_MOVIE_BATCH);
    }
    else{
      GLUIShowMotion(DIALOG_MOVIE);
    }
    END_SHOW_UPDATE(open_movie_dialog);
  }
  if(terrain_update_normals==1&&global_scase.ngeominfo>0){
    SHOW_UPDATE(terrain_update_normals);
    terrain_update_normals = 0;
    UpdateAllGeomTriangles();
    if(global_scase.auto_terrain==1){
      GenerateTerrainGeom(&terrain_vertices, &terrain_indices, &terrain_nindices);
    }
    END_SHOW_UPDATE(terrain_update_normals);
  }
  if(update_smokefire_colors==1){
    SHOW_UPDATE(update_smokefire_colors);
    update_smokefire_colors = 0;
    GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS);
    GLUISmoke3dCB(UPDATE_CO2_COLORS);
    GLUISmoke3dCB(USE_OPACITY_DEPTH);
    END_SHOW_UPDATE(update_smokefire_colors);
  }
  if(update_smoke3d_frame_inc == 1){
    update_smoke3d_frame_inc = 0;
    GLUIUpdateSmoke3dFlags();
  }
  if(update_splitcolorbar==1){
    SHOW_UPDATE(update_splitcolorbar);
    GLUISplitCB(SPLIT_COLORBAR);
    update_splitcolorbar = 0;
    END_SHOW_UPDATE(update_splitcolorbar);
  }
  if(update_stept==1){
    SHOW_UPDATE(update_stept);
    update_stept = 0;
    SetTimeVal(time_paused);
    END_SHOW_UPDATE(update_stept);
  }
  if(update_movie_parms==1){
    SHOW_UPDATE(update_movie_parms);
    update_movie_parms = 0;
    GLUIUpdateMovieParms();
    END_SHOW_UPDATE(update_movie_parms);
  }
#ifdef pp_REFRESH
  if(update_refresh==1){
    SHOW_UPDATE(update_refresh);
    update_refresh = 0;
    PeriodicRefresh(refresh_interval);
    END_SHOW_UPDATE(update_refresh);
  }
#endif
  if(update_glui_devices==1){
    SHOW_UPDATE(update_glui_devices);
    update_glui_devices = 0;
    GLUIUpdateDevices();
    END_SHOW_UPDATE(update_glui_devices);
  }
  if(update_times==1){
    SHOW_UPDATE(update_times);
    update_times = 0;
    UpdateTimes();
    END_SHOW_UPDATE(update_times);
  }
  if(global_scase.update_device==1){
    SHOW_UPDATE(global_scase.update_device);
    global_scase.update_device = 0;
    if(HaveSmokeSensor()==1){
      use_lighting = 0;
      update_use_lighting = 1;
    }
    END_SHOW_UPDATE(global_scase.update_device);
  }
  if(update_use_lighting==1){
    SHOW_UPDATE(update_use_lighting);
    ColorbarMenu(USE_LIGHTING);
    update_use_lighting = 0;
    END_SHOW_UPDATE(update_use_lighting);
  }
  if(update_playmovie==1){
    SHOW_UPDATE(update_playmovie);
    EnableDisablePlayMovie();
    update_playmovie = 0;
    END_SHOW_UPDATE(update_playmovie);
  }
  UpdateRenderStartButton();
  if(update_makemovie == 1||output_ffmpeg_command==1){
    SHOW_UPDATE(update_makemovie);
    MakeMovie();
    update_makemovie = 0;
    END_SHOW_UPDATE(update_makemovie);
  }
  if(restart_time == 1){
    SHOW_UPDATE(restart_time);
    restart_time = 0;
    ResetItimes0();
    END_SHOW_UPDATE(restart_time);
  }
  if(loadfiles_commandline[0] == 1){
    loadfiles_commandline[0] = 0;
    loadfiles_at_startup     = 0;
    update_load_files        = 0;
    SetupAutoSmoke();
    LoadFiles();
  }
  if(loadfiles_at_startup==1&&update_load_files == 1){
    SHOW_UPDATE(update_load_files);
    LoadFiles();
    END_SHOW_UPDATE(update_load_files);
  }
  if(update_startup_view>0){
    SHOW_UPDATE(update_startup_view);
    GLUISetCurrentViewPoint(viewpoint_label_startup);
    update_rotation_center = 0;
    update_rotation_center_ini = 0;
    update_startup_view--;
    END_SHOW_UPDATE(update_startup_view);
  }
  if(update_saving_viewpoint>0){
    SHOW_UPDATE(update_saving_viewpoint);
    GLUISetCurrentViewPoint(viewpoint_label_saved);
    update_saving_viewpoint--;
    END_SHOW_UPDATE(update_saving_viewpoint);
  }
  if(update_viewpoint_script>0){
    SHOW_UPDATE(update_viewpoint_script);
    GLUISetCurrentViewPoint(viewpoint_script);
    update_viewpoint_script--;
    END_SHOW_UPDATE(update_viewpoint_script);
  }
  if(update_tour_list == 1){
    SHOW_UPDATE(update_tour_list);
    GLUIUpdateTourList();
    END_SHOW_UPDATE(update_tour_list);
  }
  if(update_gslice == 1){
    SHOW_UPDATE(update_gslice);
    GLUIUpdateGsliceParms();
    END_SHOW_UPDATE(update_gslice);
  }
  if(update_rotation_center == 1){
    SHOW_UPDATE(update_rotation_center);
    camera_current->rotation_index = glui_rotation_index;
    GLUISceneMotionCB(ROTATE_ABOUT);
    update_rotation_center = 0;
    END_SHOW_UPDATE(update_rotation_center);
  }
  if(update_rotation_center_ini == 1){
    SHOW_UPDATE(update_rotation_center_ini);
    camera_current->rotation_index = glui_rotation_index_ini;
    GLUISceneMotionCB(ROTATE_ABOUT);
    update_rotation_center_ini = 0;
    END_SHOW_UPDATE(update_rotation_center_ini);
  }
  if(camera_current->dirty == 1){
    SHOW_UPDATE(camera_current->dirty);
    UpdateCamera(camera_current);
    END_SHOW_UPDATE(camera_current->dirty);
  }
  if(updateclipvals == 1){
    SHOW_UPDATE(updateclipvals);
    Clip2Cam(camera_current);
    GLUIUpdateClipAll();
    updateclipvals = 0;
    END_SHOW_UPDATE(updateclipvals);
  }
  if(update_selectedtour_index == 1){
    SHOW_UPDATE(update_selectedtour_index);
    GLUIUpdateTourIndex();
    END_SHOW_UPDATE(update_selectedtour_index);
  }
  if(trainer_mode == 1 && fontindex != LARGE_FONT){
    SHOW_UPDATE(trainer_mode);
    FontMenu(LARGE_FONT);
    END_SHOW_UPDATE(trainer_mode);
  }
  if(global_scase.updateindexcolors == 1){
    SHOW_UPDATE(updateindexcolors);
    UpdateIndexColors();
    END_SHOW_UPDATE(updateindexcolors);
  }
  if(force_isometric == 1){
    SHOW_UPDATE(force_isometric);
    force_isometric = 0;
    projection_type = PROJECTION_ORTHOGRAPHIC;
    camera_current->projection_type = projection_type;
    ZoomMenu(UPDATE_PROJECTION);
    END_SHOW_UPDATE(force_isometric);
  }
  if(convert_ini == 1){
    SHOW_UPDATE(convert_ini);
    WriteIni(SCRIPT_INI, ini_to);
    SMV_EXIT(0);
    END_SHOW_UPDATE(convert_ini);
  }
  if(convert_ssf==1||update_ssf==1){
    SHOW_UPDATE(update_ssf);
    ConvertSsf();
    SMV_EXIT(0);
    END_SHOW_UPDATE(update_ssf);
  }
  INIT_PRINT_TIMER(timer_updateshow);
  UpdateShow();
  PRINT_TIMER(timer_updateshow, "UpdateShow");
  if(global_times!=NULL&&updateUpdateFrameRateMenu==1){
    SHOW_UPDATE(updateUpdateFrameRateMenu);
    FrameRateMenu(frameratevalue);
    END_SHOW_UPDATE(updateUpdateFrameRateMenu);
  }
  if(global_scase.updatefaces == 1){
    global_scase.updatefaces = 0;
    SHOW_UPDATE(updatefaces);
    INIT_PRINT_TIMER(timer_update_faces);
    UpdateFaces();
    PRINT_TIMER(timer_update_faces, "UpdateFaces");
    END_SHOW_UPDATE(updatefaces);
  }
  if(updatefacelists == 1){
    SHOW_UPDATE(updatefacelists);
    INIT_PRINT_TIMER(timer_update_facelists);
    UpdateFaceLists();
    PRINT_TIMER(timer_update_facelists, "UpdateFaceLists");
    END_SHOW_UPDATE(updatefacelists);
  }
#ifdef pp_SHOW_UPDATE
  if(updating==1){
    printf("update complete\n\n");
  }
#endif

}

/* ------------------ UpdateFlippedColorbar ------------------------ */

void UpdateFlippedColorbar(void){
  int i, flip = 0;

  for(i = 0;i < nslice_loaded;i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo + slice_loaded_list[i];
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

#define BSIZE 256
void OutputMinMax(char *meshlabel, char *label, char *unit, float valmin_fds, float valmax_fds, float valmin_smv, float valmax_smv){
  char cvalmin_fds[BSIZE], cvalmax_fds[BSIZE];
  char cdiff_min[BSIZE], cdiff_max[BSIZE];
  char cmin[BSIZE], cmax[BSIZE];
  char labelunit[BSIZE];
  int abortprint = 0, buffer_size;

  Float2String(cvalmin_fds, valmin_fds, 6, force_fixedpoint);
  Float2String(cvalmax_fds, valmax_fds, 6, force_fixedpoint);

  Float2String(cdiff_min, valmin_fds-valmin_smv, 3, force_fixedpoint);
  Float2String(cdiff_max, valmax_fds-valmax_smv, 3, force_fixedpoint);

  buffer_size = strlen(cvalmin_fds)+strlen(cdiff_min)+3;
  if(buffer_size <  BSIZE){
    strcpy(cmin, cvalmin_fds);
    strcat(cmin, "(");
    strcat(cmin, cdiff_min);
    strcat(cmin, ")");
  }
  else{
    abortprint = 1;
    printf("***error: cmin buffer in OutputMinMax is %i > %i", buffer_size, BSIZE);
  }

  buffer_size = strlen(cvalmax_fds)+strlen(cdiff_max)+3;
  if(buffer_size <  BSIZE){
    strcpy(cmax, cvalmax_fds);
    strcat(cmax, "(");
    strcat(cmax, cdiff_max);
    strcat(cmax, ")");
  }
  else{
    abortprint = 1;
    printf("***error: cmax buffer in OutputMinMax is %i > %i", buffer_size, BSIZE);
  }

  buffer_size = strlen(label)+strlen(unit)+2;
  if(buffer_size < BSIZE){
    strcpy(labelunit, label);
    strcat(labelunit, " ");
    strcat(labelunit, unit);
  }
  else{
    printf("***error: labelunit buffer in OutputMinMax is %i > %i", buffer_size, BSIZE);
    abortprint = 1;
  }

  if(abortprint == 0){
    if(show_bound_diffs==1){
      printf("%s: %23.23s, min(delta)=%22.22s, max(delta)=%22.22s\n", meshlabel, labelunit, cmin, cmax);
    }
    else{
      printf("%s: %s, min=%12.12s, max=%12.12s\n", meshlabel, labelunit, cvalmin_fds, cvalmax_fds);
    }
  }
}

/* ------------------ UpdateBounds ------------------------ */

void OutputBounds(void){
// slice bounds
  if(update_slice_bounds != -1){
    float valmin_fds=1.0, valmax_fds=0.0, valmin_smv=1.0, valmax_smv=0.0;
    char *label, *unit;
    int i;

    label = global_scase.slicecoll.sliceinfo[update_slice_bounds].label.longlabel;
    unit = global_scase.slicecoll.sliceinfo[update_slice_bounds].label.unit;
    for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
      slicedata *slicei;
      char *labeli;
      meshdata *meshi;

      slicei = global_scase.slicecoll.sliceinfo + i;
      if(slicei->loaded==0)continue;
      meshi = global_scase.meshescoll.meshinfo+slicei->blocknumber;
      labeli = slicei->label.longlabel;
      if(strcmp(label,labeli)!=0)continue;
      if(global_scase.meshescoll.nmeshes>1&&bounds_each_mesh==1){
        OutputMinMax(meshi->label, label, unit, slicei->valmin_slice, slicei->valmax_slice, slicei->valmin_slice, slicei->valmax_slice);
      }
      if(valmin_fds>valmax_fds){
        valmin_fds = slicei->valmin_slice;
        valmax_fds = slicei->valmax_slice;
      }
      else{
        valmin_fds = MIN(slicei->valmin_slice, valmin_fds);
        valmax_fds = MAX(slicei->valmax_slice, valmax_fds);
      }
      if(valmin_smv>valmax_smv){
        valmin_smv = slicei->valmin_slice;
        valmax_smv = slicei->valmax_slice;
      }
      else{
        valmin_smv = MIN(slicei->valmin_slice, valmin_smv);
        valmax_smv = MAX(slicei->valmax_slice, valmax_smv);
      }
    }
    OutputMinMax("slice min/max", label, unit, valmin_fds, valmax_fds, valmin_smv, valmax_smv);
  }

// boundary file bounds
  if(update_patch_bounds != -1){
    float valmin_patch=1.0, valmax_patch=0.0;
    char *label, *unit;
    int i;

    label = global_scase.patchinfo[update_patch_bounds].label.longlabel;
    unit = global_scase.patchinfo[update_patch_bounds].label.unit;
    for(i=0;i<global_scase.npatchinfo;i++){
      patchdata *patchi;
      char *labeli;
      meshdata *meshi;

      patchi = global_scase.patchinfo + i;
      if(patchi->loaded==0)continue;
      meshi = global_scase.meshescoll.meshinfo+patchi->blocknumber;
      labeli = patchi->label.longlabel;
      if(strcmp(label,labeli)!=0)continue;
      if(global_scase.meshescoll.nmeshes>1&&bounds_each_mesh==1){
        OutputMinMax(meshi->label, label, unit, patchi->valmin_patch, patchi->valmax_patch, patchi->valmin_patch, patchi->valmax_patch);
      }
      if(valmin_patch>valmax_patch){
        valmin_patch = patchi->valmin_patch;
        valmax_patch = patchi->valmax_patch;
      }
      else{
        valmin_patch = MIN(patchi->valmin_patch, valmin_patch);
        valmax_patch = MAX(patchi->valmax_patch, valmax_patch);
      }
    }
    OutputMinMax("boundary min/max", label, unit, valmin_patch, valmax_patch, valmin_patch, valmax_patch);
  }

// particle file bounds
  if(update_part_bounds!=-1){
    float valmin_part = 1.0, valmax_part = 0.0;
    char *label, *unit;
    int i, j;

    if(global_scase.meshescoll.nmeshes>1&&global_scase.meshescoll.nmeshes>1&&bounds_each_mesh==1){
      for(i = 0; i<global_scase.npartinfo; i++){
        partdata *parti;
        meshdata *meshi;

        parti = global_scase.partinfo+i;
        if(parti->loaded==0)continue;
        meshi = global_scase.meshescoll.meshinfo + parti->blocknumber;
        for(j = 0; j<npart5prop; j++){
          partpropdata *propj;

          if(j==0)continue;
          propj = part5propinfo+j;

          label = propj->label->longlabel;
          unit = propj->label->unit;
          OutputMinMax(meshi->label, label, unit, parti->valmin_part[j], parti->valmax_part[j], parti->valmin_part[j], parti->valmax_part[j]);
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
      valmin_part = 1.0;
      valmax_part = 0.0;
      for(i = 0; i<global_scase.npartinfo; i++){
        partdata *parti;

        parti = global_scase.partinfo+i;
        if(parti->loaded==0)continue;
        if(valmin_part>valmax_part){
          valmin_part = parti->valmin_part[j];
          valmax_part = parti->valmax_part[j];
        }
        else{
          valmin_part = MIN(parti->valmin_part[j], valmin_part);
          valmax_part = MAX(parti->valmax_part[j], valmax_part);
        }
      }
      OutputMinMax("particle min/max:", label, unit, valmin_part, valmax_part, valmin_part, valmax_part);
    }
  }

// plot3d file bounds
  if(update_plot3d_bounds!=-1){
    float valmin_fds = 1.0, valmax_fds = 0.0, valmin_smv = 1.0, valmax_smv = 0.0;
    char *label, *unit;
    int i, j;
    plot3ddata *p;

    p = global_scase.plot3dinfo+update_plot3d_bounds;

    if(global_scase.meshescoll.nmeshes>1&&bounds_each_mesh==1){
      printf("\n");
      for(i = 0; i<global_scase.nplot3dinfo; i++){
        plot3ddata *plot3di;
        meshdata *meshi;

        plot3di = global_scase.plot3dinfo+i;
        if(plot3di->loaded==0)continue;
        meshi = global_scase.meshescoll.meshinfo+plot3di->blocknumber;
        for(j = 0; j<MAXPLOT3DVARS; j++){

          label = p->label[j].longlabel;
          unit = p->label[j].unit;

          OutputMinMax(meshi->label, label, unit, plot3di->valmin_plot3d[j], plot3di->valmax_plot3d[j], plot3di->valmin_plot3d[j], plot3di->valmax_plot3d[j]);
        }
        printf("\n");
      }
    }

    for(j=0;j<MAXPLOT3DVARS;j++){

      label = global_scase.plot3dinfo[update_plot3d_bounds].label[j].longlabel;
      unit = global_scase.plot3dinfo[update_plot3d_bounds].label[j].unit;
      valmin_fds = 1.0;
      valmax_fds = 0.0;
      valmin_smv = 1.0;
      valmax_smv = 0.0;
      for(i = 0; i<global_scase.nplot3dinfo; i++){
        plot3ddata *plot3di;

        plot3di = global_scase.plot3dinfo+i;
        if(plot3di->loaded==0)continue;
        if(valmin_fds>valmax_fds){
          valmin_fds = plot3di->valmin_plot3d[j];
          valmax_fds = plot3di->valmax_plot3d[j];
        }
        else{
          valmin_fds = MIN(plot3di->valmin_plot3d[j], valmin_fds);
          valmax_fds = MAX(plot3di->valmax_plot3d[j], valmax_fds);
        }
        if(valmin_smv>valmax_smv){
          valmin_smv = plot3di->valmin_plot3d[j];
          valmax_smv = plot3di->valmax_plot3d[j];
        }
        else{
          valmin_smv = MIN(plot3di->valmin_plot3d[j], valmin_smv);
          valmax_smv = MAX(plot3di->valmax_plot3d[j], valmax_smv);
        }
      }
      OutputMinMax("PLOT3D min/max", label, unit, valmin_fds, valmax_fds, valmin_smv, valmax_smv);
    }
  }
  printf("\n");
}

/* ------------------ HandleMakeMovie ------------------------ */

void HandleMakeMovie(void){
  THREADcontrol(ffmpeg_threads, THREAD_LOCK);
  if(have_ffmpeg == 0){
    PRINTF("*** Error: The movie generating program ffmpeg is not available\n");
    THREADcontrol(ffmpeg_threads, THREAD_UNLOCK);
    return;
  }
  GLUIEnableDisableMakeMovieCPP(OFF);
  update_makemovie = 1;
  THREADcontrol(ffmpeg_threads, THREAD_UNLOCK);
}

/* ------------------ EnableDisableMakeMovie ------------------------ */

void EnableDisableMakeMovie(int onoff){
  THREADcontrol(ffmpeg_threads, THREAD_LOCK);
  GLUIEnableDisableMakeMovieCPP(onoff);
  THREADcontrol(ffmpeg_threads, THREAD_UNLOCK);
}

/* ------------------ EnableDisablePlayMovie ------------------------ */

void EnableDisablePlayMovie(void){
  THREADcontrol(ffmpeg_threads, THREAD_LOCK);
  GLUIEnableDisablePlayMovieCPP();
  THREADcontrol(ffmpeg_threads, THREAD_UNLOCK);
}

/* ------------------ UpdateDisplay ------------------------ */

void UpdateDisplay(void){
  SNIFF_ERRORS("UpdateDisplay: start");

  if(sortslices == 1&&nsliceloaded>0){
    SortSlices();
  }
  if(csv_loaded == 0){
    if(vis_hrr_plot == 1 || viswindrose == 1 || vis_device_plot != DEVICE_PLOT_HIDDEN || showdevice_val == 1){
      InitializeDeviceCsvData(LOAD);
    }
  }
  if(update_ini_boundary_type==1){
    update_ini_boundary_type = 0;
    ShowBoundaryMenu(INTERIOR_WALL_MENU);
    ShowBoundaryMenu(INI_EXTERIORwallmenu);
  }
  if(update_boundary_loaded == 1){ // a hack, shouldn't be necessary
    update_boundary_loaded = 0;
    ShowBoundaryMenu(INTERIOR_WALL_MENU);
    ShowBoundaryMenu(INTERIOR_WALL_MENU);
  }
  if(update_fire_alpha==1){
    update_fire_alpha=0;
    GLUIUpdateFireAlpha();
  }
  if(update_texturebar==1){
    update_texturebar = 0;
    UpdateTexturebar();
  }
  if(update_make_iblank == 1){
    int ig;

    for(ig = 0; ig < global_scase.meshescoll.nmeshes; ig++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + ig;
      meshi->c_iblank_node = meshi->c_iblank_node_temp;
      meshi->c_iblank_cell = meshi->c_iblank_cell_temp;
      meshi->f_iblank_cell = meshi->f_iblank_cell_temp;
      meshi->c_iblank_x    = meshi->c_iblank_x_temp;
      meshi->c_iblank_y    = meshi->c_iblank_y_temp;
      meshi->c_iblank_z    = meshi->c_iblank_z_temp;

      meshi->c_iblank_node_html = meshi->c_iblank_node_html_temp;
      meshi->c_iblank_node0     = meshi->c_iblank_node0_temp;
      meshi->c_iblank_cell0     = meshi->c_iblank_cell0_temp;
      meshi->f_iblank_cell0     = meshi->f_iblank_cell0_temp;
      meshi->c_iblank_x0        = meshi->c_iblank_x0_temp;
      meshi->c_iblank_y0        = meshi->c_iblank_y0_temp;
      meshi->c_iblank_z0        = meshi->c_iblank_z0_temp;
    }
    INIT_PRINT_TIMER(timer_hidden_blockages);
    int nhidden_faces = 0, ntotal_obsts = 0;
    for(ig = 0; ig < global_scase.meshescoll.nmeshes; ig++){
      meshdata *meshi;
      int j;

      meshi = global_scase.meshescoll.meshinfo + ig;
      void SetHiddenBlockages(meshdata *meshi);
      if(global_scase.have_hidden6 == 0){
        if(ig == 0)printf("setting hidden blockages\n");
        SetHiddenBlockages(meshi);
      }
      for(j = 0; j < meshi->nbptrs; j++){
        blockagedata *bc;

        bc = meshi->blockageinfoptrs[j];
        if(bc->hidden6[0] == 1)nhidden_faces++;
        if(bc->hidden6[1] == 1)nhidden_faces++;
        if(bc->hidden6[2] == 1)nhidden_faces++;
        if(bc->hidden6[3] == 1)nhidden_faces++;
        if(bc->hidden6[4] == 1)nhidden_faces++;
        if(bc->hidden6[5] == 1)nhidden_faces++;
      }
      ntotal_obsts += meshi->nbptrs;
    }
    if(nhidden_faces > 0)printf("%i blockage faces out of %i hidden\n", nhidden_faces, 6*ntotal_obsts);
    PRINT_TIMER(timer_hidden_blockages, "SetHiddenBlockages");
    update_make_iblank = 0;
    update_setvents    = 1;
    update_setcvents   = 1;
  }
  if(update_setvents==1){
    SetVentDirs();
    update_setvents=0;
  }
  if(update_setcvents == 1){
    SetCVentDirs();
    update_setcvents = 0;
  }
  THREADcontrol(ffmpeg_threads, THREAD_LOCK);
  if(update_ff == 1){
    update_ff = 0;
    if(have_ffmpeg == 1){
      GLUIEnableDisableMakeMovieCPP(ON);
    }
    else{
      GLUIEnableDisableMakeMovieCPP(OFF);
    }
  }
  THREADcontrol(ffmpeg_threads, THREAD_UNLOCK);
  if(update_ini==1){
    update_ini = 0;
    ReadIni(NULL);

    update_glui_bounds = 0;
    GLUIUpdateBounds();
  }
  if(update_colorbar_list == 1){
    GLUIUpdateColorbarList();
    update_colorbar_list = 0;
  }
  if(update_glui_bounds==1){
    update_glui_bounds = 0;
    GLUIUpdateBounds();
  }
  if(update_cache_data==1){
    update_cache_data = 0;
    GLUISetCacheFlag(BOUND_PLOT3D, cache_plot3d_data);
    GLUISetCacheFlag(BOUND_PART, cache_part_data);
    GLUISetCacheFlag(BOUND_PATCH, cache_boundary_data);
    GLUISetCacheFlag(BOUND_SLICE, cache_slice_data);
  }
  if(update_chop_colors==1){
    update_chop_colors = 0;
    UpdateChopColors();
  }
  if(update_zaxis_custom == 1){
    update_zaxis_custom = 0;
    GLUIUpdateZAxisCustom();
  }
  if(update_flipped_colorbar == 1){
    update_flipped_colorbar = 0;
    UpdateFlippedColorbar();
  }
  if(update_smokecolorbar == 1){
    update_smokecolorbar = 0;
    SmokeColorbarMenu(colorbars.fire_colorbar_index);
  }
  if(update_colorbar_dialog == 1){
    GLUIUpdateNodeLabel(colorbars.colorbarinfo + colorbartype);
    update_colorbar_dialog = 0;
  }
  if(update_colorbartype == 1){
    colorbardata *cb;

    cb = GetColorbar(&colorbars, colorbarname);
    if(cb != NULL){
      colorbartype = cb - colorbars.colorbarinfo;
      colorbartype_default = colorbartype;
      if(cb->can_adjust == 1){
        cb->interp = INTERP_LAB;
      }
      else{
        cb->interp = INTERP_RGB;
      }
      RemapColorbar(cb, show_extreme_mindata, rgb_below_min,
                    show_extreme_maxdata, rgb_above_max);
      memcpy(cb->node_rgb_orig, cb->node_rgb, 3*cb->nnodes*sizeof(unsigned char));
      UpdateCurrentColorbar(cb);
      if(colorbartype != colorbartype_default){
        colorbartype_ini = colorbartype;
      }
      if(colorbars.colorbarinfo != NULL){
        colorbartype = colorbartype_default;
        UpdateColorbarDialogs();
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
    GLUIUpdateWindowSizeList();
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      screenWidthINI  /= 2;
      screenHeightINI /= 2;
    }
#endif
    ResizeWindow(screenWidthINI, screenHeightINI);
  }
  if(updatemenu == 1 && usemenu == 1 && menustatus == GLUT_MENU_NOT_IN_USE){
    INIT_PRINT_TIMER(timer_update_menus);
    glutDetachMenu(GLUT_RIGHT_BUTTON);
    attachmenu_status = 0;
    THREADcontrol(checkfiles_threads, THREAD_LOCK);
    InitMenus();
    THREADcontrol(checkfiles_threads, THREAD_UNLOCK);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    attachmenu_status = 1;
    updatemenu = 0;
    PRINT_TIMER(timer_update_menus, "update menus");
  }
  if(attachmenu_print == 1){
    if(attachmenu_status == 1)printf("menus attached(%i)\n",attachmenu_counter++);
    if(attachmenu_status == 0)printf("menus detached(%i)\n",attachmenu_counter++);
  }
  if(update_patch_bounds!=-1||update_slice_bounds!=-1||update_part_bounds!=-1||update_plot3d_bounds!=-1){

    if(current_script_command==NULL|| NOT_LOADRENDER){
      OutputBounds();
    }
    update_patch_bounds = -1;
    update_slice_bounds = -1;
    update_part_bounds = -1;
    update_plot3d_bounds = -1;
  }
  if(update_fire_colorbar_index == 1){
    SmokeColorbarMenu(fire_colorbar_index_ini);
    GLUIUpdateFireColorbarList();
    update_fire_colorbar_index = 0;
  }
  if(update_co2_colorbar_index==1){
    GLUIUpdateCO2ColorbarList(co2_colorbar_index_ini);
    update_co2_colorbar_index = 0;
  }
  if(update_colorbar_select_index == 1 && colorbar_select_index >= 0 && colorbar_select_index <= 255){
    update_colorbar_select_index = 0;
    UpdateRGBColors(colorbar_select_index);
  }
  if(update_windrose_showhide==1){
    GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_DEVICE);
  }
  if(update_research_mode == 1){
    update_research_mode = 0;
    GLUIUpdateResearchMode();
  }
  if(update_colorbar_digits==1){
    update_colorbar_digits = 0;
    GLUISetColorbarDigitsCPP(ncolorlabel_digits);
    GLUISetColorbarDigits();
  }
  if(update_visColorbars==1){
    update_visColorbars = 0;
    visColorbarVertical = visColorbarVertical_val;
    visColorbarHorizontal = visColorbarHorizontal_val;
    vis_colorbar = GetColorbarState();
    GLUIUpdateColorbarControls();
    GLUIUpdateColorbarControls2();
    updatemenu = 1;
  }
  if(update_windrose==1){
    update_windrose = 0;
    DeviceData2WindRose(nr_windrose, ntheta_windrose);
  }
#ifdef pp_REFRESH
  if(refresh_glui_dialogs>=-1){
    refresh_glui_dialogs--;
    GLUIRefreshDialogs();
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

/* ------------------ PauseTime ------------------------ */

void PauseTime(float pause_time){
  float start_time;

  // pause no more than 60 s
  start_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
  for(;;){
    float delta_time;

    delta_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0 - start_time;
    if(delta_time > pause_time || delta_time > 60.0)return;
    }
  }
