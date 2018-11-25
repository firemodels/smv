#define IN_UPDATE
#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "update.h"
#include "smokeviewvars.h"
#include "compress.h"

/* ------------------ CompareFloat ------------------------ */

int CompareFloat( const void *arg1, const void *arg2 ){
  float x, y;

  x=*(float *)arg1;
  y=*(float *)arg2;
  if( x< y)return -1;
  if( x> y)return 1;
  return 0;
}

/* ------------------ UpdateHrrinfo ------------------------ */

void UpdateHrrinfo(int vis){
  if(hrrinfo != NULL && hrrinfo->display!=vis&&hrrinfo->loaded==1){
    hrrinfo->display = vis;
    UpdateTimes();
  }
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
        if(sd->slicefile_type == SLICE_GEOM){
          patchdata *patchi;

          patchi = sd->patchgeom;
          if(patchi->geom_timeslist == NULL)continue;
          if(patchi->structured == YES || patchi->boundary == 1 || patchi->geom_times == NULL || patchi->geom_timeslist == NULL)continue;
          patchi->geom_itime = patchi->geom_timeslist[itimes];
          patchi->geom_ival_static = patchi->geom_ivals_static[patchi->geom_itime];
          patchi->geom_ival_dynamic = patchi->geom_ivals_dynamic[patchi->geom_itime];
          patchi->geom_nval_static = patchi->geom_nstatics[patchi->geom_itime];
          patchi->geom_nval_dynamic = patchi->geom_ndynamics[patchi->geom_itime];
          sd->itime = patchi->geom_timeslist[itimes];
          slice_time = sd->itime;
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

/* ------------------ UpdateShow ------------------------ */

void UpdateShow(void){
  int i,evacflag,sliceflag,vsliceflag,partflag,patchflag,isoflag,smoke3dflag,tisoflag;
  int slicecolorbarflag;
  int shooter_flag;

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
  showterrain=0;
  visTimeParticles=1; visTimeSlice=1; visTimeBoundary=1; visTimeZone=1; visTimeIso=1;

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
  if(visTerrainType!=TERRAIN_HIDDEN){
    for(i=0;i<nterraininfo;i++){
      terraindata *terri;

      terri = terraininfo + i;
      if(terri->loaded==1){
        showterrain=1;
        break;
      }
    }
  }
  {
    smoke3ddata *smoke3di;
    int ii;

    for(ii=0;ii<nsmoke3dinfo;ii++){
      smoke3di = smoke3dinfo + ii;
      if(smoke3di->loaded==1&&smoke3di->display==1){
        smoke3dflag=1;
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
      if(sd->volslice==1&&sd->slicefile_type==SLICE_NODE_CENTER&&vis_gslice_data==1)SHOW_gslice_data=1;
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
      if(sd->volslice==1&&sd->slicefile_type==SLICE_NODE_CENTER&&vis_gslice_data==1)SHOW_gslice_data=1;
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
    ( sliceflag==1 || vsliceflag==1 || partflag==1 || patchflag==1 ||
    shooter_flag==1||
    smoke3dflag==1|| showtours==1 || evacflag==1||
    (ReadZoneFile==1&&visZone==1&&visTimeZone==1)||
    showterrain==1||showvolrender==1
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
  if(plotstate==STATIC_PLOTS&&ReadPlot3dFile==1&&plotn>0&&plotn<=numplot3dvars)showplot3d=1;
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
  if(ReadEvacFile==1)num_colorbars++;
  if(ReadPartFile==1)num_colorbars++;
  if(plotstate==DYNAMIC_PLOTS&&(slicecolorbarflag==1||vslicecolorbarflag==1))num_colorbars++;
  if(plotstate==DYNAMIC_PLOTS&&patchflag==1&&wall_cell_color_flag==0)num_colorbars++;
  if(plotstate==DYNAMIC_PLOTS&&ReadZoneFile==1)num_colorbars++;
  if(plotstate==DYNAMIC_PLOTS&&tisoflag==1&&1==0){ // disable isosurface colorbar label for now
    showiso_colorbar=1;
    num_colorbars++;
  }
  if(ReadPlot3dFile==1&&num_colorbars==0)num_colorbars=1;
  
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
  if(plotstate==DYNAMIC_PLOTS&&stept==1){
    glutIdleFunc(IdleCB);
  }
  else{
    glutIdleFunc(NULL);
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

    /* synchronize terrain times */

    for(j=0;j<nterraininfo;j++){
      terraindata *terri;

      terri = terraininfo + j;
      if(terri->loaded==0)continue;
      terri->timeslist[n]=GetItime(n,terri->timeslist,terri->times,terri->ntimes);
    }
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

      j = slice_loaded_list[jj];
      sd = sliceinfo + j;
      if(sd->slicefile_type == SLICE_GEOM){
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

  /* ------------------ UpdateTimes ------------------------ */

void UpdateTimes(void){
  int i;
  int nglobal_times_copy;
  float *global_times_copy = NULL;

  FREEMEMORY(geominfoptrs);
  ngeominfoptrs=0;
  GetGeomInfoPtrs(&geominfoptrs,&ngeominfoptrs);

  // pass 1 - determine ntimes

  UpdateShow();
  CheckMemory;
  nglobal_times = 0;

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    nglobal_times+=geomi->ntimes;
  }
  if(visTerrainType!=TERRAIN_HIDDEN){
    for(i=0;i<nterraininfo;i++){
      terraindata *terri;

      terri = terraininfo + i;
      if(terri->loaded==1)nglobal_times+=terri->ntimes;
    }
  }
  if(visShooter!=0&&shooter_active==1){
    nglobal_times+=nshooter_frames;
  }
  for(i=0;i<ntourinfo;i++){
    tourdata *touri;

    touri = tourinfo + i;
    if(touri->display==0)continue;
    nglobal_times+= touri->ntimes;
  }
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->loaded==0)continue;
    nglobal_times+= parti->ntimes;
  }
  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd=sliceinfo+i;
    if(sd->loaded==1||sd->vloaded==1){
      nglobal_times+=sd->ntimes;
    }
  }
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->loaded==1&&patchi->structured == NO){
      nglobal_times+=patchi->ngeom_times;
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
        nglobal_times+=meshi->npatch_times;
      }
    }
  }
  if(ReadZoneFile==1&&visZone==1){
    nglobal_times+=nzone_times;
  }
  if(ReadIsoFile==1&&visAIso!=0){
    for(i=0;i<nisoinfo;i++){
      meshdata *meshi;
      isodata *ib;

      ib = isoinfo+i;
      if(ib->geomflag==1||ib->loaded==0)continue;
      meshi=meshinfo + ib->blocknumber;
      nglobal_times+=meshi->niso_times;
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
      nglobal_times+=vr->ntimes;
    }
  }
  {
    smoke3ddata *smoke3di;

    if(Read3DSmoke3DFile==1&&vis3DSmoke3D==1){
      for(i=0;i<nsmoke3dinfo;i++){
        smoke3di = smoke3dinfo + i;
        if(smoke3di->loaded==0)continue;
        nglobal_times+= smoke3di->ntimes;
      }
    }
  }
  CheckMemory;

  // end pass 1

  FREEMEMORY(global_times);
  if(nglobal_times > 0){
    NewMemory((void **)&global_times, nglobal_times * sizeof(float));
    NewMemory((void **)&global_times_copy, nglobal_times * sizeof(float));
  }

  // pass 2 - merge times arrays

  nglobal_times_copy = 0;
  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    memcpy(global_times + nglobal_times_copy, geomi->times, geomi->ntimes * sizeof(float));
    nglobal_times_copy += geomi->ntimes;
  }
  if(visTerrainType!=TERRAIN_HIDDEN){
    for(i=0;i<nterraininfo;i++){
      terraindata *terri;

      terri = terraininfo + i;
      if(terri->loaded==0)continue;
      memcpy(global_times + nglobal_times_copy, terri->times, terri->ntimes * sizeof(float));
      nglobal_times_copy += terri->ntimes;
    }
  }
  if(visShooter!=0&&shooter_active==1){
    for(i=0;i<nshooter_frames;i++){
      global_times[nglobal_times_copy++]=shoottimeinfo[i].time;
    }
  }

  for(i=0;i<ntourinfo;i++){
    tourdata *touri;

    touri = tourinfo + i;
    if(touri->display==0)continue;
    memcpy(global_times + nglobal_times_copy, touri->path_times, touri->ntimes * sizeof(float));
    nglobal_times_copy += touri->ntimes;
  }

  tmax_part=0.0;
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->loaded==0)continue;
    memcpy(global_times + nglobal_times_copy, parti->times, parti->ntimes * sizeof(float));
    nglobal_times_copy += parti->ntimes;
    tmax_part=MAX(parti->times[parti->ntimes-1],tmax_part);
  }

  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd = sliceinfo + i;
    if(sd->loaded==1||sd->vloaded==1){
      memcpy(global_times + nglobal_times_copy, sd->times, sd->ntimes * sizeof(float));
      nglobal_times_copy += sd->ntimes;
    }
  }

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->loaded==1&&patchi->structured == NO){
      memcpy(global_times + nglobal_times_copy, patchi->geom_times, patchi->ngeom_times * sizeof(float));
      nglobal_times_copy += patchi->ngeom_times;
    }
  }
  for(i=0;i<nmeshes;i++){
    patchdata *patchi;
    meshdata *meshi;
    int filenum;

    meshi=meshinfo + i;
    filenum=meshi->patchfilenum;
    if(filenum!=-1){
      patchi = patchinfo + filenum;
      if(patchi->loaded==1&&patchi->structured == YES){
        memcpy(global_times + nglobal_times_copy, meshi->patch_times, meshi->npatch_times * sizeof(float));
        nglobal_times_copy += meshi->npatch_times;
      }
    }
  }
  if(nvolrenderinfo>0){
    for(i=0;i<nmeshes;i++){
      volrenderdata *vr;
      meshdata *meshi;

      meshi=meshinfo + i;
      vr = &meshi->volrenderinfo;
      if(vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      memcpy(global_times + nglobal_times_copy, vr->times, vr->ntimes * sizeof(float));
      nglobal_times_copy += vr->ntimes;
    }
  }
  if(ReadZoneFile==1&&visZone==1){
    memcpy(global_times + nglobal_times_copy, zone_times, nzone_times * sizeof(float));
    nglobal_times_copy += nzone_times;
  }
  if(ReadIsoFile==1&&visAIso!=0){
    for(i=0;i<nisoinfo;i++){
      meshdata *meshi;
      isodata *ib;

      ib = isoinfo+i;
      if(ib->geomflag==1||ib->loaded==0)continue;
      meshi=meshinfo + ib->blocknumber;
      memcpy(global_times + nglobal_times_copy, meshi->iso_times, meshi->niso_times * sizeof(float));
      nglobal_times_copy += meshi->niso_times;
    }
  }
  if(Read3DSmoke3DFile==1&&vis3DSmoke3D==1){
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + i;
      if(smoke3di->loaded==0)continue;
      memcpy(global_times + nglobal_times_copy, smoke3di->times, smoke3di->ntimes * sizeof(float));
      nglobal_times_copy += smoke3di->ntimes;
    }
  }
  CheckMemory;

  ASSERT(nglobal_times==nglobal_times_copy);

  // end pass 2

  // sort and remove duplicates

  if(nglobal_times>0){
    int n,to,from;

    memcpy(global_times_copy, global_times, nglobal_times * sizeof(float));

    qsort((float *)global_times_copy, (size_t)nglobal_times, sizeof(float), CompareFloat);
    CheckMemory;

#define DT_EPS 0.00001

    to = 1;
    global_times[0]=global_times_copy[0];
    for(from=1;from<nglobal_times;from++){
      if(ABS(global_times_copy[from]-global_times_copy[from-1])>DT_EPS){
        global_times[to]=global_times_copy[from];
        to++;
      }
    }
    nglobal_times = to;
    FREEMEMORY(global_times_copy);

    for(n = 0; n < nglobal_times-1; n++){
      int it;

      if(global_times[n] < global_times[n+1])continue;
      timearray_test++;
      fprintf(stderr, "*** Error: time array out of order at position %i, nglobal_times=%i times=", n+1,nglobal_times);
      if(timearray_test == 1){
        for(it = 0; it < nglobal_times; it++){
          fprintf(stderr," %f", global_times[it]);
        }
        fprintf(stderr, "\n");
      }
      else if(timearray_test > 1){
        for(it = 0; it < MIN(nglobal_times, 10); it++){
          fprintf(stderr, " %f", global_times[it]);
        }
        fprintf(stderr, "......\n");
      }
      break;
    }
  }
  CheckMemory;


  // pass 3 - allocate memory for individual times array

  if(nglobal_times>ntimes_old){
    ntimes_old=nglobal_times;
    FREEMEMORY(render_frame);
    if(nglobal_times>0)NewMemory((void **)&render_frame,nglobal_times*sizeof(int));
  }

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
  if(visTerrainType!=TERRAIN_HIDDEN){
    for(i=0;i<nterraininfo;i++){
      terraindata *terri;

      terri = terraininfo + i;
      if(terri->loaded==0)continue;
      FREEMEMORY(terri->timeslist);
      if(nglobal_times>0)NewMemory((void **)&terri->timeslist,nglobal_times*sizeof(int));
    }
  }
  if(hrrinfo!=NULL){
    FREEMEMORY(hrrinfo->timeslist);
    FREEMEMORY(hrrinfo->times);
    FREEMEMORY(hrrinfo->hrrval);
    if(hrrinfo->loaded==1&&hrrinfo->display==1&&nglobal_times>0){
      int jstart=0;

      NewMemory((void **)&hrrinfo->timeslist,nglobal_times*sizeof(int));
      NewMemory((void **)&hrrinfo->times,nglobal_times*sizeof(float));
      NewMemory((void **)&hrrinfo->hrrval,nglobal_times*sizeof(float));
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
            jstart=j;
            break;
          }
        }
        if(foundit==0){
          hrrinfo->hrrval[i]=hrrinfo->hrrval_csv[hrrinfo->ntimes_csv-1];
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
    if(sd->slicefile_type == SLICE_GEOM){
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

  // end pass 3

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

  // reallocate times array

  if(nglobal_times==0){
    FREEMEMORY(global_times);
  }
  if(nglobal_times>0)ResizeMemory((void **)&global_times,nglobal_times*sizeof(float));

  // pass 4 - initialize individual time pointers

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
  show_slice_terrain=0;
  if(visTerrainType==TERRAIN_3D_MAP){
    for(i=0;i<nsliceinfo;i++){
      slicedata *sd;

      sd = sliceinfo + i;
      if(sd->loaded==0||sd->display==0||sd->slicefile_type!=SLICE_TERRAIN)continue;
      show_slice_terrain=1;
      break;
    }
  }
  CheckMemory;
}

/* ------------------ GetPlotState ------------------------ */

int GetPlotState(int choice){
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
      for(i=0;i<nslice_loaded;i++){
        slicedata *slicei;

        slicei = sliceinfo + slice_loaded_list[i];
        if(slicei->display==0||slicei->slicefile_labelindex!=slicefile_labelindex)continue;
        stept = 1;
        return DYNAMIC_PLOTS;
      }
      if(visGrid==0)stept = 1;
      if(visTerrainType!=TERRAIN_HIDDEN){
        for(i=0;i<nterraininfo;i++){
          terraindata *terri;

          terri = terraininfo + i;
          if(terri->loaded==1){
            return DYNAMIC_PLOTS;
          }
        }
      }
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
  if(update_smoketype_vals==1){
    update_smoketype_vals = 0;
#define SMOKE_NEW 77
#define SMOKE_DELTA_MULTIPLE 78
    Smoke3dCB(SMOKE_NEW);
    Smoke3dCB(SMOKE_DELTA_MULTIPLE);
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
  if(update_startup_view == 1){
    cameradata *ca;

    ca = GetCamera(startup_view_label);
    if(ca != NULL){
      ResetMenu(ca->view_id);
      startup_view_ini = ca->view_id;
    }
    update_rotation_center = 0;
    update_rotation_center_ini = 0;
    update_startup_view = 0;
  }
  if(update_tour_list == 1){
    UpdateTourList();
  }
  if(update_gslice == 1){
    UpdateGsliceParms();
  }
#define MESH_LIST 4
  if(update_rotation_center == 1){
    camera_current->rotation_index = glui_rotation_index;
    SceneMotionCB(MESH_LIST);
    update_rotation_center = 0;
  }
  if(update_rotation_center_ini == 1){
    camera_current->rotation_index = glui_rotation_index_ini;
    SceneMotionCB(MESH_LIST);
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
    exit(0);
  }
  if(convert_ssf==1||update_ssf==1){
    ConvertSsf();
    exit(0);
  }
  UpdateShow();
  if(global_times!=NULL&&updateUpdateFrameRateMenu==1)FrameRateMenu(frameratevalue);
  if(updatefaces==1)UpdateFaces();
  if(updatefacelists==1)UpdateFaceLists();
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

/* ------------------ UpdateDisplay ------------------------ */
#define TERRAIN_FIRE_LINE_UPDATE 39

void UpdateDisplay(void){

  LOCK_IBLANK;
 #ifdef pp_TISO
  if(update_texturebar==1){
    update_texturebar = 0;
    UpdateTexturebar();
  }
#endif
  if(update_setvents==1){
    SetVentDirs();
    update_setvents=0;
  }
  UNLOCK_IBLANK
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
  if(update_fire_line == 1){
    WuiCB(TERRAIN_FIRE_LINE_UPDATE);
    update_fire_line = 0;
  }
  if(updatezoommenu == 1 || first_display > 0){
    if(first_display > 0)first_display--;
    updatezoommenu = 0;
    ZoomMenu(zoomindex);
  }
#ifdef pp_MAKE_SMOKEIBLANK
  if(update_makeiblank_smoke3d == 1){
    MakeIBlankSmoke3D();
  }
#endif
  if(update_streaks == 1 && ReadPartFile == 1){
    ParticleStreakShowMenu(streak_index);
    update_streaks = 0;
  }
  if(update_screensize == 1){
    update_screensize = 0;
    UpdateWindowSizeList();
    ResizeWindow(screenWidthINI, screenHeightINI);
  }
  if(updatemenu == 1 && usemenu == 1 && menustatus == GLUT_MENU_NOT_IN_USE){
    glutDetachMenu(GLUT_RIGHT_BUTTON);
    InitMenus(LOAD);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    updatemenu = 0;
  }
  if(update_fire_colorbar_index == 1){
    SmokeColorbarMenu(fire_colorbar_index_ini);
    update_fire_colorbar_index = 0;
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
  if(update_visColorbarVertical==1){
    update_visColorbarVertical = 0;
    visColorbarVertical = visColorbarVertical_val;
  }
}
