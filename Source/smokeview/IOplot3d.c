#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "IOobjects.h"
#include "getdata.h"

#define PLOT3DCONVERT(val, valmin, valmax) ( (val-valmin)/(valmax-valmin) )

#ifndef GET_QDATA
#define GET_QDATA(i,j,k,n) qdata[IJKN(i,j,k,n)]
#endif

#ifndef GET_QVAL
#define GET_QVAL(i,j,k,n) \
  if(qdata!=NULL){\
    qval=qdata[IJKN(i,j,k,n)];\
  }\
  else{\
    float *qvals;\
    qvals=p3levels256[n];\
    qval=qvals[iqdata[IJKN(i,j,k,n)]];\
  }
#endif


/* ------------------ GetPlot3DHists ------------------------ */

void GetPlot3DHists(plot3ddata *p){
  int i;

  for(i = 0; i<p->nplot3dvars; i++){
    histogramdata *histi;
    float *vals;
    int nvals;
    meshdata *plot3d_mesh;

    // histi, if already allocated, is freed in ReadPlot3d
    NewMemoryMemID((void **)&histi, sizeof(histogramdata), p->memory_id);
    p->histograms[i] = histi;
    InitHistogramMemID(histi, NHIST_BUCKETS, NULL, NULL, p->memory_id);

    plot3d_mesh = global_scase.meshescoll.meshinfo+p->blocknumber;
    nvals = (plot3d_mesh->ibar+1)*(plot3d_mesh->jbar+1)*(plot3d_mesh->kbar+1);
    vals = plot3d_mesh->qdata+i*nvals;
    int use_bounds = 0;
    float valmin_dummy = 0.0, valmax_dummy = 1.0;
    CopyVals2Histogram(vals, NULL, NULL, nvals, histi, use_bounds, valmin_dummy, valmax_dummy);
  }
}

/* ------------------ MergePlot3DHistograms ------------------------ */

void MergePlot3DHistograms(void){
  int i;

  if(full_plot3D_histograms==NULL){
    NewMemory((void **)&full_plot3D_histograms, MAXPLOT3DVARS*sizeof(histogramdata));
    for(i = 0; i<MAXPLOT3DVARS; i++){
      InitHistogram(full_plot3D_histograms+i, NHIST_BUCKETS, NULL, NULL);
    }
  }
  else{
    for(i = 0; i<MAXPLOT3DVARS; i++){
      FreeHistogram(full_plot3D_histograms+i);
      InitHistogram(full_plot3D_histograms+i, NHIST_BUCKETS, NULL, NULL);
    }
  }
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;
    int k;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    for(k = 0; k<plot3di->nplot3dvars; k++){
      MergeHistogram(full_plot3D_histograms+k, plot3di->histograms[k], MERGE_BOUNDS);
    }
  }
}

/* ------------------ Plot3dCompare  ------------------------ */

int Plot3dCompare(const void *arg1, const void *arg2){
  plot3ddata *plot3di, *plot3dj;

  plot3di = global_scase.plot3dinfo + *(int *)arg1;
  plot3dj = global_scase.plot3dinfo + *(int *)arg2;

  if(strcmp(plot3di->longlabel,plot3dj->longlabel)<0)return -1;
  if(strcmp(plot3di->longlabel,plot3dj->longlabel)>0)return 1;
  if(plot3di->time<plot3dj->time)return -1;
  if(plot3di->time>plot3dj->time)return 1;
  if(plot3di->blocknumber<plot3dj->blocknumber)return -1;
  if(plot3di->blocknumber>plot3dj->blocknumber)return 1;
  return 0;
}

/* ------------------ AllocatePlot3DColorLabels  ------------------------ */

int AllocatePlot3DColorLabels(plot3ddata *plot3di){
  int nn, error;
  int ifile;

  ifile = plot3di-global_scase.plot3dinfo;
  for(nn = 0; nn<numplot3dvars; nn++){
    int n;

    for(n = 0; n<MAXRGB; n++){
      (*(colorlabelp3+nn))[n] = NULL;
      (*(colorlabeliso+nn))[n] = NULL;
    }

    if(p3levels[nn]==NULL){
      if(NewMemoryMemID((void **)&p3levels[nn], (global_scase.nrgb+1)*sizeof(float), plot3di->memory_id)==0){
        ReadPlot3D("", ifile, UNLOAD, &error);
        if(error==1)return 1;
      }
    }
    if(p3levels256[nn]==NULL){
      if(NewMemoryMemID((void **)&p3levels256[nn], 256*sizeof(float), plot3di->memory_id)==0){
        ReadPlot3D("", ifile, UNLOAD, &error);
        if(error==1)return 1;
      }
    }
    for(n = 0; n<global_scase.nrgb; n++){
      if(colorlabelp3[nn][n]==NULL){
        if(NewMemoryMemID((void **)&(*(colorlabelp3+nn))[n], 11, plot3di->memory_id)==0){
          ReadPlot3D("", ifile, UNLOAD, &error);
          if(error==1)return 1;
        }
      }
      if(colorlabeliso[nn][n]==NULL){
        if(NewMemoryMemID((void **)&(*(colorlabeliso+nn))[n], 11, plot3di->memory_id)==0){
          ReadPlot3D("", ifile, UNLOAD, &error);
          if(error==1)return 1;
        }
      }
    }
  }
  return 0;
}

/* ------------------ UpdatePlot3DColors  ------------------------ */

void  UpdatePlot3DColors(plot3ddata *plot3di, int flag, int *errorcode){
  int nn;

  int num;

  GLUIGetMinMaxAll(BOUND_PLOT3D, setp3min_all, p3min_all, setp3max_all, p3max_all, &num);
  *errorcode=AllocatePlot3DColorLabels(plot3di);
  if(*errorcode==1)return;
  for(nn = 0; nn < numplot3dvars; nn++){
    if(global_scase.nplot3dinfo > 0){
      shortp3label[nn] = plot3di->label[nn].shortlabel;
      unitp3label[nn] = plot3di->label[nn].unit;
    }
    else{
      char numstring[4];

      sprintf(numstring, "%i", nn);
      strcpy(shortp3label[nn], numstring);
      unitp3label[nn] = blank_global;
    }
    GetPlot3DColors(nn, p3min_all + nn, p3max_all + nn,
                    nrgb_full, global_scase.nrgb - 1, colorlabelp3[nn], colorlabeliso[nn], p3levels[nn], p3levels256[nn],
                    plot3di->extreme_min + nn, plot3di->extreme_max + nn, flag);
  }
}

/* ------------------ GetPlot3DBounds  ------------------------ */

int GetPlot3DBounds(plot3ddata *plot3di){
  float valmin, valmax, *vals;
  char *iblank;
  meshdata *meshi;
  int i, ntotal;

  meshi = global_scase.meshescoll.meshinfo+plot3di->blocknumber;
  if(meshi->qdata==NULL)return 0;
  ntotal = (meshi->ibar+1)*(meshi->jbar+1)*(meshi->kbar+1);

  for(i = 0; i<plot3di->nplot3dvars; i++){
    int n;

    valmin = 1000000000.;
    valmax = -valmin;
    vals = meshi->qdata+i*ntotal;
    iblank = meshi->c_iblank_node;
    for(n = 0; n<ntotal; n++){
      float val;

      val = *vals++;
      if(iblank==NULL||*iblank++==GAS){
        valmin = MIN(val, valmin);
        valmax = MAX(val, valmax);
      }
    }
    plot3di->valmin_plot3d[i] = valmin;
    plot3di->valmax_plot3d[i] = valmax;
  }
  return 1;
}

/* ------------------ ComputeLoadedPlot3DBounds  ------------------------ */

void ComputeLoadedPlot3DBounds(float *valmin_loaded, float *valmax_loaded){
  int i, first;

  plot3d_uvw_max = 1.0;
  for(first = 1, i = 0; i < global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;
    meshdata *meshi;

    plot3di = global_scase.plot3dinfo + i;
    if(plot3di->loaded == 0)continue;
    meshi = global_scase.meshescoll.meshinfo + plot3di->blocknumber;
    plot3d_uvw_max = MAX(plot3d_uvw_max, meshi->plot3d_uvw_max);
    if(first == 1){
      first = 0;
      memcpy(valmin_loaded, plot3di->valmin_plot3d, plot3di->nplot3dvars * sizeof(float));
      memcpy(valmax_loaded, plot3di->valmax_plot3d, plot3di->nplot3dvars * sizeof(float));
      valmax_loaded[plot3di->nplot3dvars - 1] = meshi->plot3d_speedmax;
    }
    else{
      int j;

      for(j = 0; j < plot3di->nplot3dvars; j++){
        valmin_loaded[j] = MIN(valmin_loaded[j], plot3di->valmin_plot3d[j]);
        valmax_loaded[j] = MAX(valmax_loaded[j], plot3di->valmax_plot3d[j]);
      }
      valmax_loaded[plot3di->nplot3dvars - 1] = MAX(valmax_loaded[plot3di->nplot3dvars - 1], meshi->plot3d_speedmax);
    }
  }
}


/* ------------------ UpdatePlot3DFileLoad  ------------------------ */

void UpdatePlot3DFileLoad(void){
  int i;

  nplot3dloaded = 0;
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loaded==1)nplot3dloaded++;
  }
}

/* ------------------ ReadPlot3d  ------------------------ */

FILE_SIZE ReadPlot3D(char *file, int ifile, int flag, int *errorcode){
  int ntotal, i;
  float *udata, *vdata, *wdata, *sdata;
  int error;
  int ibar,jbar,kbar;
  meshdata *meshi,*gbb;
  plot3ddata *p;
  int nloaded=0;
  int nx, ny, nz;
  int pn;
  FILE_SIZE file_size=0;
  float read_time, total_time;

  CheckMemory;
  START_TIMER(total_time);
  *errorcode=0;
  updatefacelists = 1;

  assert(ifile>=0&&ifile<global_scase.nplot3dinfo);
  p=global_scase.plot3dinfo+ifile;
  if(flag==UNLOAD&&p->loaded==0)return 0;

  highlight_mesh=p->blocknumber;
  meshi=global_scase.meshescoll.meshinfo+highlight_mesh;
  UpdateCurrentMesh(meshi);

  if(meshi->plot3dfilenum!=-1){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+meshi->plot3dfilenum;
    FreeAllMemory(plot3di->memory_id);
    colorlabeliso = NULL;
    colorlabelp3  = NULL;
  }
  if(flag==UNLOAD){
    meshi->plot3dfilenum=-1;
  }
  else{
    pn = meshi->plot3dfilenum;
    if(pn!=-1){
      global_scase.plot3dinfo[pn].loaded=0;
      global_scase.plot3dinfo[pn].display=0;
    }
    meshi->plot3dfilenum=ifile;
  }

  FreeSurface(meshi->currentsurf);
  FreeSurface(meshi->currentsurf2);
  FreeContour(meshi->plot3dcontour1);
  FreeContour(meshi->plot3dcontour2);
  FreeContour(meshi->plot3dcontour3);
  InitContour(meshi->plot3dcontour1,rgb_plot3d_contour,global_scase.nrgb);
  InitContour(meshi->plot3dcontour2,rgb_plot3d_contour,global_scase.nrgb);
  InitContour(meshi->plot3dcontour3,rgb_plot3d_contour,global_scase.nrgb);


  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    gbb=global_scase.meshescoll.meshinfo+i;
    if(gbb->plot3dfilenum!=-1)nloaded++;
  }

  if(flag==UNLOAD){
    p->loaded=0;
    p->display=0;
    UpdatePlot3DFileLoad();
    plotstate = GetPlotState(STATIC_PLOTS);
    meshi=global_scase.meshescoll.meshinfo+p->blocknumber;
    meshi->plot3dfilenum=-1;
    if(nloaded==0){
      numplot3dvars=0;
    }
    updatemenu=1;
    PrintMemoryInfo;
    update_times = 1;
    UpdateUnitDefs();
    return 0;
  }

  ibar=meshi->ibar;
  jbar=meshi->jbar;
  kbar=meshi->kbar;
  nx = ibar+1;
  ny = jbar+1;
  nz = kbar+1;
  ntotal = nx*ny*nz;
  numplot3dvars=5;

  uindex = global_scase.plot3dinfo[ifile].u;
  vindex = global_scase.plot3dinfo[ifile].v;
  windex = global_scase.plot3dinfo[ifile].w;
  if(uindex!=-1||vindex!=-1||windex!=-1)numplot3dvars=global_scase.plot3dinfo[ifile].nplot3dvars;

  if(NewMemoryMemID((void **)&meshi->qdata,numplot3dvars*ntotal*sizeof(float), p->memory_id)==0){
    *errorcode=1;
    ReadPlot3D("",ifile,UNLOAD,&error);
    return 0;
  }

  if(NewMemoryMemID((void **)&meshi->yzcolorbase ,ny*nz*sizeof(unsigned char), p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->xzcolorbase ,nx*nz*sizeof(unsigned char), p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->xycolorbase ,nx*ny*sizeof(unsigned char), p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->yzcolorfbase,ny*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->xzcolorfbase,nx*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->xycolorfbase,nx*ny*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->yzcolortbase,ny*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->xzcolortbase,nx*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->xycolortbase,nx*ny*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dx_xy       ,nx*ny*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dy_xy       ,nx*ny*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dz_xy       ,nx*ny*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dx_xz       ,nx*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dy_xz       ,nx*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dz_xz       ,nx*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dx_yz       ,ny*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dy_yz       ,ny*nz*sizeof(float),         p->memory_id)==0||
     NewMemoryMemID((void **)&meshi->dz_yz       ,ny*nz*sizeof(float),         p->memory_id)==0){
     *errorcode=1;
     ReadPlot3D("",ifile,UNLOAD,&error);
     return 0;
  }

  file_size= GetFileSizeSMV(file);
  PRINTF("\nLoading %s\n", file);
  START_TIMER(read_time);
  float qmin[6], qmax[6], *qminptr=NULL, *qmaxptr=NULL;

  if(p->have_bound_file==0){
    qminptr = qmin;
    qmaxptr = qmax;
  }
  getplot3dq(file, nx, ny, nz, meshi->qdata, qminptr, qmaxptr, &error, isotest);
  update_plot3d_bnd = 1;
  if(NewMemoryMemID((void **)&meshi->iqdata,numplot3dvars*ntotal*sizeof(unsigned char), p->memory_id)==0){
    *errorcode=1;
    ReadPlot3D("",ifile,UNLOAD,&error);
    return 0;
  }
  STOP_TIMER(read_time);
  p->loaded=1;
  p->display=1;
  p->hist_update = 1;
  if(nplot3dloaded==0)UpdatePlot3DFileLoad();
  meshi->udata=NULL;
  meshi->vdata=NULL;
  meshi->wdata=NULL;
  if(uindex!=-1||vindex!=-1||windex!=-1){
    vectorspresent=1;
    p->nplot3dvars= MAXPLOT3DVARS;
    if(uindex!=-1)udata = meshi->qdata + ntotal*uindex;
    if(vindex!=-1)vdata = meshi->qdata + ntotal*vindex;
    if(windex!=-1)wdata = meshi->qdata + ntotal*windex;
    sdata = meshi->qdata + ntotal*5;
    meshi->plot3d_speedmax = 0.0;
    meshi->plot3d_uvw_max = 0.0;
    for(i=0;i<ntotal;i++){
      float uval=0.0, vval=0.0, wval=0.0;
      float uvwmax1, uvwmax2, uvwmax;

      if(uindex!=-1)uval = udata[i];
      if(vindex!=-1)vval = vdata[i];
      if(windex!=-1)wval = wdata[i];
      uvwmax1 = MAX(ABS(uval), ABS(vval));
      uvwmax2 = MAX(ABS(wval), 1.0);
      uvwmax = MAX(uvwmax1, uvwmax2);
      uval /= uvwmax;
      vval /= uvwmax;
      wval /= uvwmax;
      sdata[i] = uvwmax*sqrt(uval*uval + vval*vval + wval*wval);
      meshi->plot3d_speedmax = MAX(meshi->plot3d_speedmax, sdata[i]);
      meshi->plot3d_uvw_max  = MAX(meshi->plot3d_uvw_max, uvwmax);
    }
    if(uindex!=-1)meshi->udata=meshi->qdata + ntotal*uindex;
    if(vindex!=-1)meshi->vdata=meshi->qdata + ntotal*vindex;
    if(windex!=-1)meshi->wdata=meshi->qdata + ntotal*windex;
  }

  if(NewMemoryMemID((void **)&colorlabelp3, MAXPLOT3DVARS*sizeof(char **),  p->memory_id)==0||
     NewMemoryMemID((void **)&colorlabeliso, MAXPLOT3DVARS*sizeof(char **), p->memory_id)==0){
    *errorcode=1;
    ReadPlot3D("",ifile,UNLOAD,&error);
    return 0;
  }
  {
    int nn;

    for(nn=0;nn<MAXPLOT3DVARS;nn++){
      colorlabelp3[nn]=NULL;
      colorlabeliso[nn]=NULL;
    }
    for(nn=0;nn<MAXPLOT3DVARS;nn++){
      if(NewMemoryMemID((void **)&colorlabelp3[nn],MAXRGB*sizeof(char *),  p->memory_id)==0||
         NewMemoryMemID((void **)&colorlabeliso[nn],MAXRGB*sizeof(char *), p->memory_id)==0){
        *errorcode=1;
        ReadPlot3D("",ifile,UNLOAD,&error);
        return 0;
      }
    }
  }

  if(NewMemoryMemID((void **)&p3levels, MAXPLOT3DVARS*sizeof(float *),    p->memory_id)==0||
     NewMemoryMemID((void **)&p3levels256, MAXPLOT3DVARS*sizeof(float *), p->memory_id)==0){
    *errorcode=1;
    ReadPlot3D("",ifile,UNLOAD,&error);
    return 0;
  }
  {
    int nn;

    for(nn=0;nn<MAXPLOT3DVARS;nn++){
      p3levels[nn]=NULL;
      p3levels256[nn]=NULL;
    }
  }
  if(nloaded>1){
    int nn;

    for(nn=0;nn<numplot3dvars;nn++){
      if(setp3min_all[nn]!=SET_MIN&&setp3min_all[nn]!=CHOP_MIN)setp3min_all[nn]=SET_MIN;
      if(setp3max_all[nn]!=SET_MAX&&setp3max_all[nn]!=CHOP_MAX)setp3max_all[nn]=SET_MAX;
    }
  }
  AllocatePlot3DColorLabels(p);
  if(cache_plot3d_data==1){
    if(p->finalize==1){
      if(update_plot3d_bnd==1){
        update_plot3d_bnd = 0;
        float valmin_loaded[6], valmax_loaded[6];

        void BoundsUpdate(int file_type);
        if(no_bounds==0 || force_bounds==1)BoundsUpdate(BOUND_PLOT3D);
        ComputeLoadedPlot3DBounds(valmin_loaded, valmax_loaded);
        GLUISetLoadedMinMaxAll(BOUND_PLOT3D, valmin_loaded, valmax_loaded, global_scase.plot3dinfo->nplot3dvars);
      }
      UpdateAllPlot3DColors(0);
      UpdatePlotSlice(XDIR);
      UpdatePlotSlice(YDIR);
      UpdatePlotSlice(ZDIR);
    }
  }
  else{
    UpdatePlot3DColors(p, 0, errorcode);
  }

  if(meshi->plotx==-1)meshi->plotx=ibar/2;
  if(meshi->ploty==-1)meshi->ploty=jbar/2;
  if(meshi->plotz==-1)meshi->plotz=kbar/2;
  visGrid=0;
  meshi->visInteriorBoundaries=0;
  if(visx_all==1){
    UpdateShowStep(1,XDIR);
  }
  if(visy_all==1){
    UpdateShowStep(1,YDIR);
  }
  if(visz_all==1){
    UpdateShowStep(1,ZDIR);
  }
  if(visiso==1){
    UpdateSurface();
  }

  GLUIUpdatePlot3dListIndex();
  PrintMemoryInfo;
  update_times = 1;
  UpdateUnitDefs();
  ForceIdle();
  STOP_TIMER(total_time);

  update_plot3d_bounds = ifile;
  GetPlot3DBounds(p);
  if(cache_plot3d_data==0){
    cache_plot3d_data=0;
    FREEMEMORY(meshi->qdata);
  }
  UpdatePlot3dTitle();
  if(p->time>=0.0){
    char label[256];

    sprintf(label, "%f", p->time);
    TrimZeros(label);
    STRCAT(label, " s");
    STRCPY(p->timelabel, label);
  }
  else{
    STRCPY(p->timelabel, "");
  }
  show_plot3dfiles = 1;
  GLUTPOSTREDISPLAY;
  return file_size;
}

/* ------------------ DrawPlot3dTexture ------------------------ */

void DrawPlot3dTexture(meshdata *meshi){
  int i,j,k;
  int colorindex;
  float dx, dy, dz;
  int plotx, ploty, plotz;
  int visx, visy, visz;
  float *xplt, *yplt, *zplt;
  int ibar, jbar, kbar;
  isosurface *currentsurfptr,*currentsurf2ptr;

  float *dx_xy, *dy_xy, *dz_xy;
  float *dx_xz, *dy_xz, *dz_xz;
  float *dx_yz, *dy_yz, *dz_yz;
  float *dx_yzcopy, *dy_yzcopy, *dz_yzcopy;
  float *dx_xzcopy, *dy_xzcopy, *dz_xzcopy;
  float *dx_xycopy, *dy_xycopy, *dz_xycopy;
  int nx, ny, nz,nxy;
  char *c_iblank_x, *c_iblank_y, *c_iblank_z, *iblank;
  float *vector_color;
  float *qdata;
  int nxyz;
  int num;

  plotx = meshi->iplotx_all[iplotx_all];
  ploty = meshi->iploty_all[iploty_all];
  plotz = meshi->iplotz_all[iplotz_all];
  qdata = meshi->qdata;

  GLUIGetMinMaxAll(BOUND_PLOT3D, setp3min_all, p3min_all, setp3max_all, p3max_all, &num);
  float ttmin, ttmax;

  ttmin = p3min_all[plotn-1];
  ttmax = p3max_all[plotn-1];

  visx = visx_all;
  visy = visy_all;
  visz = visz_all;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;
  xplt = meshi->xplt_smv;
  yplt = meshi->yplt_smv;
  zplt = meshi->zplt_smv;
  c_iblank_x = meshi->c_iblank_x;
  c_iblank_y = meshi->c_iblank_y;
  c_iblank_z = meshi->c_iblank_z;
  iblank = meshi->c_iblank_node;


  nx = ibar+1;
  ny = jbar+1;
  nz = kbar+1;
  nxy = nx*ny;
  nxyz = nx*ny*nz;

  currentsurfptr=meshi->currentsurf;
  currentsurf2ptr=meshi->currentsurf2;
  dx_xy=meshi->dx_xy;
  dx_xz=meshi->dx_xz;
  dx_yz=meshi->dx_yz;

  dy_xy=meshi->dy_xy;
  dy_xz=meshi->dy_xz;
  dy_yz=meshi->dy_yz;

  dz_xy=meshi->dz_xy;
  dz_xz=meshi->dz_xz;
  dz_yz=meshi->dz_yz;

  if(cullfaces==1)glDisable(GL_CULL_FACE);
  if(visiso==1){
    DrawStaticIso(currentsurfptr,p3dsurfacetype,p3dsurfacesmooth,2,0,plot3dlinewidth);
    if(surfincrement!=0)DrawStaticIso(currentsurf2ptr,p3dsurfacetype,p3dsurfacesmooth,2,0,plot3dlinewidth);
    if(visGrid!=NOGRID_NOPROBE){
      if(use_transparency_data==1)TransparentOff();
      if(cullfaces==1)glEnable(GL_CULL_FACE);
      return;
    }
  }

  if(use_transparency_data==1){
    TransparentOn();
  }
  if(visVector==0){
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D,texture_plot3d_colorbar_id);
  }

  /* +++++++++++++++++++++++++++   draw yz contours +++++++++++++++++++++++++++++++++++++ */

  if(plotx>=0&&visx!=0){
    if(visVector==0){
      if(plotx<0){
        plotx=ibar;
        UpdatePlotSlice(XDIR);
      }
      if(plotx>ibar){
        plotx=0;
        UpdatePlotSlice(XDIR);
      }
      glBegin(GL_TRIANGLES);
      for(j=0; j<jbar; j++){
        for(k=0; k<kbar; k++){
          if(c_iblank_x==NULL||c_iblank_x[IJKNODE(plotx,j,k)]==GASGAS){
            float val[4];

            val[0] = GET_QDATA(plotx, j,   k,   plotn-1);
            val[1] = GET_QDATA(plotx, j,   k+1, plotn-1);
            val[2] = GET_QDATA(plotx, j+1, k,   plotn-1);
            val[3] = GET_QDATA(plotx, j+1, k+1, plotn-1);
            val[0] = CLAMP(PLOT3DCONVERT(val[0], ttmin, ttmax), 0.0, 1.0);
            val[1] = CLAMP(PLOT3DCONVERT(val[1], ttmin, ttmax), 0.0, 1.0);
            val[2] = CLAMP(PLOT3DCONVERT(val[2], ttmin, ttmax), 0.0, 1.0);
            val[3] = CLAMP(PLOT3DCONVERT(val[3], ttmin, ttmax), 0.0, 1.0);
            if(ABS(val[0]-val[3])<ABS(val[1]-val[2])){
              glTexCoord1f(val[0]); glVertex3f(xplt[plotx],yplt[j],zplt[k]);
              glTexCoord1f(val[2]); glVertex3f(xplt[plotx],yplt[j+1],zplt[k]);
              glTexCoord1f(val[3]); glVertex3f(xplt[plotx],yplt[j+1],zplt[k+1]);

              glTexCoord1f(val[0]); glVertex3f(xplt[plotx],yplt[j],zplt[k]);
              glTexCoord1f(val[3]); glVertex3f(xplt[plotx],yplt[j+1],zplt[k+1]);
              glTexCoord1f(val[1]); glVertex3f(xplt[plotx],yplt[j],zplt[k+1]);
            }
            else{
              glTexCoord1f(val[0]); glVertex3f(xplt[plotx],yplt[j],zplt[k]);
              glTexCoord1f(val[2]); glVertex3f(xplt[plotx],yplt[j+1],zplt[k]);
              glTexCoord1f(val[1]); glVertex3f(xplt[plotx],yplt[j],zplt[k+1]);

              glTexCoord1f(val[2]); glVertex3f(xplt[plotx],yplt[j+1],zplt[k]);
              glTexCoord1f(val[3]); glVertex3f(xplt[plotx],yplt[j+1],zplt[k+1]);
              glTexCoord1f(val[1]); glVertex3f(xplt[plotx],yplt[j],zplt[k+1]);
            }
          }
        }
      }
      glEnd();
    }
    /* draw yz vectors */

    if(visVector==1){
      AntiAliasLine(ON);
      glLineWidth(vectorlinewidth);
      glBegin(GL_LINES);
      for(j=0; j<=jbar; j+=vectorskip){
        dx_yzcopy = dx_yz+j*(kbar+1);
        dy_yzcopy = dy_yz+j*(kbar+1);
        dz_yzcopy = dz_yz+j*(kbar+1);
        for(k=0; k<=kbar; k+=vectorskip){
          float val;

          val = GET_QDATA(plotx, j,   k,   plotn-1);
          colorindex = 255*CLAMP(PLOT3DCONVERT(val, ttmin, ttmax), 0.0, 1.0);
          vector_color = rgb_plot3d + 4*colorindex;
          if((iblank==NULL||iblank[IJKNODE(plotx,j,k)]==GAS)&&vector_color[3]>0.5){
            glColor4fv(vector_color);
            dx=*dx_yzcopy/2.0;
            dy=*dy_yzcopy/2.0;
            dz=*dz_yzcopy/2.0;
            glVertex3f(xplt[plotx]-dx,yplt[j]-dy,zplt[k]-dz);
            glVertex3f(xplt[plotx]+dx,yplt[j]+dy,zplt[k]+dz);
          }
          dx_yzcopy+=vectorskip;
          dy_yzcopy+=vectorskip;
          dz_yzcopy+=vectorskip;
        }
      }
      glEnd();
      AntiAliasLine(OFF);

      /* draw points for yz vectors */

      glPointSize(vectorpointsize);
      glBegin(GL_POINTS);
      for(j=0; j<=jbar; j+=vectorskip){
        dx_yzcopy = dx_yz+j*(kbar+1);
        dy_yzcopy = dy_yz+j*(kbar+1);
        dz_yzcopy = dz_yz+j*(kbar+1);
        for(k=0; k<=kbar; k+=vectorskip){
          float val;

          val = GET_QDATA(plotx, j,   k,   plotn-1);
          colorindex = 255*CLAMP(PLOT3DCONVERT(val, ttmin, ttmax), 0.0, 1.0);
          vector_color = rgb_plot3d + 4*colorindex;
          if((iblank==NULL||iblank[IJKNODE(plotx,j,k)]==GAS)&&vector_color[3]>0.5){
            glColor4fv(vector_color);
            glVertex3f(
              xplt[plotx]+*dx_yzcopy/(float)2.0,
              yplt[j]+*dy_yzcopy/(float)2.0,
              zplt[k]+*dz_yzcopy/(float)2.0
              );
          }
          dx_yzcopy+=vectorskip;
          dy_yzcopy+=vectorskip;
          dz_yzcopy+=vectorskip;
        }
      }
      glEnd();
    }
  }

  /* +++++++++++++++++++++++++++++++++  draw xz contours  ++++++++++++++++++++++++++++++++++++++++ */

  if(ploty>=0&&visy!=0){
    if(visVector==0){
      glBegin(GL_TRIANGLES);
      for(i=0; i<ibar; i++){
        for(k=0; k<kbar; k++){
          float val[4];

          val[0] = GET_QDATA(i,   ploty, k,   plotn-1);
          val[1] = GET_QDATA(i,   ploty, k+1, plotn-1);
          val[2] = GET_QDATA(i+1, ploty, k,   plotn-1);
          val[3] = GET_QDATA(i+1, ploty, k+1, plotn-1);
          val[0] = CLAMP(PLOT3DCONVERT(val[0], ttmin, ttmax), 0.0, 1.0);
          val[1] = CLAMP(PLOT3DCONVERT(val[1], ttmin, ttmax), 0.0, 1.0);
          val[2] = CLAMP(PLOT3DCONVERT(val[2], ttmin, ttmax), 0.0, 1.0);
          val[3] = CLAMP(PLOT3DCONVERT(val[3], ttmin, ttmax), 0.0, 1.0);
          if(c_iblank_y==NULL||c_iblank_y[IJKNODE(i, ploty, k)]==GASGAS){
            if(ABS(val[0]-val[3])<ABS(val[1]-val[2])){
              glTexCoord1f(val[0]);  glVertex3f(xplt[i],   yplt[ploty], zplt[k]);
              glTexCoord1f(val[2]);  glVertex3f(xplt[i+1], yplt[ploty], zplt[k]);
              glTexCoord1f(val[3]);  glVertex3f(xplt[i+1], yplt[ploty], zplt[k+1]);

              glTexCoord1f(val[0]);  glVertex3f(xplt[i],  yplt[ploty], zplt[k]);
              glTexCoord1f(val[3]); glVertex3f(xplt[i+1], yplt[ploty], zplt[k+1]);
              glTexCoord1f(val[1]); glVertex3f(xplt[i],   yplt[ploty], zplt[k+1]);
            }
            else{
              glTexCoord1f(val[0]);  glVertex3f(xplt[i],   yplt[ploty], zplt[k]);
              glTexCoord1f(val[2]);  glVertex3f(xplt[i+1], yplt[ploty], zplt[k]);
              glTexCoord1f(val[1]);  glVertex3f(xplt[i],   yplt[ploty], zplt[k+1]);

              glTexCoord1f(val[2]); glVertex3f(xplt[i+1], yplt[ploty], zplt[k]);
              glTexCoord1f(val[3]); glVertex3f(xplt[i+1], yplt[ploty], zplt[k+1]);
              glTexCoord1f(val[1]); glVertex3f(xplt[i],   yplt[ploty], zplt[k+1]);
            }
          }
        }
      }
      glEnd();
    }

    /* draw xz vectors */

    if(visVector==1){

      AntiAliasLine(ON);
      glLineWidth(vectorlinewidth);
      glBegin(GL_LINES);
      for(i=0; i<=ibar; i+=vectorskip){
        dx_xzcopy=dx_xz+i*(kbar+1);
        dy_xzcopy=dy_xz+i*(kbar+1);
        dz_xzcopy=dz_xz+i*(kbar+1);
        for(k=0; k<=kbar; k+=vectorskip){
          float val;

          val = GET_QDATA(i, ploty,   k,   plotn-1);
          colorindex = 255*CLAMP(PLOT3DCONVERT(val, ttmin, ttmax), 0.0, 1.0);
          vector_color = rgb_plot3d + 4*colorindex;
          if((iblank==NULL||iblank[IJKNODE(i,ploty,k)]==GAS)&&vector_color[3]>0.5){
            glColor4fv(vector_color);
            dx=*dx_xzcopy/2.0;
            dy=*dy_xzcopy/2.0;
            dz=*dz_xzcopy/2.0;
            glVertex3f(xplt[i]-dx,yplt[ploty]-dy,zplt[k]-dz);
            glVertex3f(xplt[i]+dx,yplt[ploty]+dy,zplt[k]+dz);
          }
          dx_xzcopy+=vectorskip;
          dy_xzcopy+=vectorskip;
          dz_xzcopy+=vectorskip;
        }
      }
      glEnd();
      AntiAliasLine(OFF);

      /* draw points for xz vectors */

      glPointSize(vectorpointsize);
      glBegin(GL_POINTS);
      for(i=0; i<=ibar; i+=vectorskip){
        dx_xzcopy=dx_xz+i*(kbar+1);
        dy_xzcopy=dy_xz+i*(kbar+1);
        dz_xzcopy=dz_xz+i*(kbar+1);
        for(k=0; k<=kbar; k+=vectorskip){
          float val;

          val = GET_QDATA(i, ploty,   k,   plotn-1);
          colorindex = 255*CLAMP(PLOT3DCONVERT(val, ttmin, ttmax), 0.0, 1.0);
          vector_color = rgb_plot3d + 4*colorindex;
          if((iblank==NULL||iblank[IJKNODE(i,ploty,k)]==GAS)&&vector_color[3]>0.5){
            glColor4fv(vector_color);
            dx=*dx_xzcopy/2.0;
            dy=*dy_xzcopy/2.0;
            dz=*dz_xzcopy/2.0;
            glVertex3f(xplt[i]+dx,yplt[ploty]+dy,zplt[k]+dz);
          }
          dx_xzcopy+=vectorskip;
          dy_xzcopy+=vectorskip;
          dz_xzcopy+=vectorskip;
        }
      }
      glEnd();
    }
  }

  /* ++++++++++++++++++++++++++++ draw xy contours ++++++++++++++++++++++++++++++++ */

  if(plotz>=0&&visz!=0){
    if(visVector==0){
      if(plotz<0){
        plotz=kbar;
        UpdatePlotSlice(ZDIR);
      }
      if(plotz>kbar){
        plotz=0;
        UpdatePlotSlice(ZDIR);
      }
      glBegin(GL_TRIANGLES);
      for(i=0; i<ibar; i++){
        for(j=0; j<jbar; j++){
          float val[4];

          val[0] = GET_QDATA(i,   j,   plotz, plotn-1);
          val[1] = GET_QDATA(i,   j+1, plotz, plotn-1);
          val[2] = GET_QDATA(i+1, j,   plotz, plotn-1);
          val[3] = GET_QDATA(i+1, j+1, plotz, plotn-1);
          val[0] = CLAMP(PLOT3DCONVERT(val[0], ttmin, ttmax), 0.0, 1.0);
          val[1] = CLAMP(PLOT3DCONVERT(val[1], ttmin, ttmax), 0.0, 1.0);
          val[2] = CLAMP(PLOT3DCONVERT(val[2], ttmin, ttmax), 0.0, 1.0);
          val[3] = CLAMP(PLOT3DCONVERT(val[3], ttmin, ttmax), 0.0, 1.0);
          if(c_iblank_z==NULL||c_iblank_z[IJKNODE(i, j, plotz)]==GASGAS){
            if(ABS(val[0]-val[3])<ABS(val[1]-val[2])){
               glTexCoord1f(val[0]); glVertex3f(xplt[i],   yplt[j],   zplt[plotz]);
               glTexCoord1f(val[2]); glVertex3f(xplt[i+1], yplt[j],   zplt[plotz]);
               glTexCoord1f(val[3]); glVertex3f(xplt[i+1], yplt[j+1], zplt[plotz]);

               glTexCoord1f(val[0]); glVertex3f(xplt[i],   yplt[j],   zplt[plotz]);
               glTexCoord1f(val[3]); glVertex3f(xplt[i+1], yplt[j+1], zplt[plotz]);
               glTexCoord1f(val[1]); glVertex3f(xplt[i],   yplt[j+1], zplt[plotz]);
            }
            else{
               glTexCoord1f(val[0]); glVertex3f(xplt[i],   yplt[j],   zplt[plotz]);
               glTexCoord1f(val[2]); glVertex3f(xplt[i+1], yplt[j],   zplt[plotz]);
               glTexCoord1f(val[1]); glVertex3f(xplt[i],   yplt[j+1], zplt[plotz]);

               glTexCoord1f(val[2]); glVertex3f(xplt[i+1], yplt[j],   zplt[plotz]);
               glTexCoord1f(val[3]); glVertex3f(xplt[i+1], yplt[j+1], zplt[plotz]);
               glTexCoord1f(val[1]); glVertex3f(xplt[i],   yplt[j+1], zplt[plotz]);
            }
          }
        }
      }
      glEnd();
    }

    /* draw xy vectors */

    if(visVector==1){

      AntiAliasLine(ON);
      glLineWidth(vectorlinewidth);
      glBegin(GL_LINES);
      for(i=0; i<=ibar; i+=vectorskip){
        dx_xycopy=dx_xy+i*(jbar+1);
        dy_xycopy=dy_xy+i*(jbar+1);
        dz_xycopy=dz_xy+i*(jbar+1);
        for(j=0; j<=jbar; j+=vectorskip){
          float val;

          val = GET_QDATA(i, j, plotz,   plotn-1);
          colorindex = 255*CLAMP(PLOT3DCONVERT(val, ttmin, ttmax), 0.0, 1.0);
          vector_color = rgb_plot3d + 4*colorindex;
          if((iblank==NULL||iblank[IJKNODE(i,j,plotz)]==GAS)&&vector_color[3]>0.5){
            glColor4fv(vector_color);
            dx=*dx_xycopy/2.0;
            dy=*dy_xycopy/2.0;
            dz=*dz_xycopy/2.0;
            glVertex3f(xplt[i]-dx,yplt[j]-dy,zplt[plotz]-dz);
            glVertex3f(xplt[i]+dx,yplt[j]+dy,zplt[plotz]+dz);
          }
          dx_xycopy+=vectorskip;
          dy_xycopy+=vectorskip;
          dz_xycopy+=vectorskip;
        }
      }
      glEnd();
      AntiAliasLine(OFF);

      /* draw points for xy vectors */

      glPointSize(vectorpointsize);
      glBegin(GL_POINTS);
      for(i=0; i<=ibar; i+=vectorskip){
        dx_xycopy=dx_xy+i*(jbar+1);
        dy_xycopy=dy_xy+i*(jbar+1);
        dz_xycopy=dz_xy+i*(jbar+1);
        for(j=0; j<=jbar; j+=vectorskip){
          float val;

          val = GET_QDATA(i, j, plotz,   plotn-1);
          colorindex = 255*CLAMP(PLOT3DCONVERT(val, ttmin, ttmax), 0.0, 1.0);
          vector_color = rgb_plot3d + 4*colorindex;
          if((iblank==NULL||iblank[IJKNODE(i,j,plotz)]==GAS)&&vector_color[3]>0.5){
            glColor4fv(vector_color);
            glVertex3f(
              xplt[i]+*dx_xycopy/(float)2.0,
              yplt[j]+*dy_xycopy/(float)2.0,
              zplt[plotz]+*dz_xycopy/(float)2.0
              );
          }
          dx_xycopy+=vectorskip;
          dy_xycopy+=vectorskip;
          dz_xycopy+=vectorskip;
        }
      }
      glEnd();
    }
  }
  if(visVector==0){
    glDisable(GL_TEXTURE_1D);
  }
  if(use_transparency_data==1){
    TransparentOff();
  }
  if(cullfaces==1)glEnable(GL_CULL_FACE);

}

/* ------------------ DrawPlot3dFrame ------------------------ */

void DrawPlot3dFrame(void){
  int i;

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    if(meshi->use == 0)continue;
    if(meshi->plot3dfilenum==-1)continue;
    if(global_scase.plot3dinfo[meshi->plot3dfilenum].display==0)continue;
    DrawPlot3dTexture(meshi);
  }
}

/* ------------------ UpdateSurface ------------------------ */

void UpdateSurface(void){
  int colorindex,colorindex2;
  float level,level2;
  int ibar, jbar, kbar;
  float *xplt, *yplt, *zplt;
  isosurface *currentsurfptr,*currentsurf2ptr;
  float *qdata;
  char *iblank_cell;
  int plot3dsize;
  int i;

  if(nplot3dloaded==0)return;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo+i;
    if(meshi->plot3dfilenum==-1)continue;

    ibar=meshi->ibar;
    jbar=meshi->jbar;
    kbar=meshi->kbar;
    plot3dsize=(ibar+1)*(jbar+1)*(kbar+1);
    xplt=meshi->xplt_smv;
    yplt=meshi->yplt_smv;
    zplt=meshi->zplt_smv;
    iblank_cell=meshi->c_iblank_cell;

    currentsurfptr = meshi->currentsurf;
    currentsurf2ptr = meshi->currentsurf2;
    qdata=meshi->qdata;
    if(plotiso[plotn-1]<0){
      plotiso[plotn-1]=global_scase.nrgb-3;
    }
    if(plotiso[plotn-1]>global_scase.nrgb-3){
      plotiso[plotn-1]=0;
    }
    colorindex=plotiso[plotn-1];
    level = p3min_all[plotn-1] + (colorindex+0.5)*(p3max_all[plotn-1]-p3min_all[plotn-1])/((float)global_scase.nrgb-2.0f);
    isolevelindex=colorindex;
    isolevelindex2=colorindex;
    FreeSurface(currentsurfptr);
    InitIsoSurface(currentsurfptr, level, rgb_plot3d_contour[colorindex],-999);
    GetIsoSurface(currentsurfptr,qdata+(plotn-1)*plot3dsize,NULL,iblank_cell,level,
      xplt,ibar+1,yplt,jbar+1,zplt,kbar+1);
    GetNormalSurface(currentsurfptr);
    CompressIsoSurface(currentsurfptr,1,
        xplt[0],xplt[ibar],
        yplt[0],yplt[jbar],
        zplt[0],zplt[kbar]);
    SmoothIsoSurface(currentsurfptr);

    if(surfincrement!=0){
      colorindex2=colorindex+surfincrement;
      if(colorindex2<0)colorindex2=global_scase.nrgb-2;
      if(colorindex2>global_scase.nrgb-2)colorindex2=0;
      level2 = p3min_all[plotn-1] + colorindex2*(p3max_all[plotn-1]-p3min_all[plotn-1])/((float)global_scase.nrgb-2.0f);
      FreeSurface(currentsurf2ptr);
      InitIsoSurface(currentsurf2ptr, level2, rgb_plot3d_contour[colorindex2],-999);
      GetIsoSurface(currentsurf2ptr,qdata+(plotn-1)*plot3dsize,NULL,iblank_cell,level2,
        xplt,ibar+1,yplt,jbar+1,zplt,kbar+1);
      GetNormalSurface(currentsurf2ptr);
      CompressIsoSurface(currentsurf2ptr,1,
        xplt[0],xplt[ibar],
        yplt[0],yplt[jbar],
        zplt[0],zplt[kbar]);
      SmoothIsoSurface(currentsurf2ptr);

      isolevelindex2=colorindex2;
    }
  }
}

/* ------------------ UpdateAllPlotSlices ------------------------ */

void UpdateAllPlotSlices(void){
  if(visx_all==1)UpdatePlotSlice(XDIR);
  if(visy_all==1)UpdatePlotSlice(YDIR);
  if(visz_all==1)UpdatePlotSlice(ZDIR);
}

/* ------------------ GetPlot3dIndex ------------------------ */

int GetPlot3dIndex(meshdata *meshi, int dir, float val){
  float valmin;
  int i, ivalmin, nvals;
  float *xyz=NULL;

  switch(dir){
    case XDIR:
      xyz = meshi->xplt_fds;
      nvals = meshi->ibar;
      break;
    case YDIR:
      xyz = meshi->yplt_fds;
      nvals = meshi->jbar;
      break;
    case ZDIR:
      xyz = meshi->zplt_fds;
      nvals = meshi->kbar;
      break;
    default:
      xyz = NULL;
      assert(FFALSE);
      break;
  }

  ivalmin=0;
  assert(xyz != NULL);
  if(xyz != NULL){
    valmin = ABS(xyz[0] - val);
    for(i = 1;i <= nvals;i++){
      if(ABS(xyz[i] - val) < valmin){
        valmin = ABS(xyz[i] - val);
        ivalmin = i;
      }
    }
  }
  return ivalmin;
}

/* ------------------ UpdatePlotXYZ ------------------------ */

void UpdatePlotXYZ(meshdata *current_mesh_local){
  int i;
  float xval, yval, zval;

  xval = current_mesh_local->xplt_smv[current_mesh_local->plotx];
  yval = current_mesh_local->yplt_smv[current_mesh_local->ploty];
  zval = current_mesh_local->zplt_smv[current_mesh_local->plotz];

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    float xmin, xmax;
    float ymin, ymax;
    float zmin, zmax;

    meshi=global_scase.meshescoll.meshinfo+i;
    if(meshi==current_mesh_local)continue;

    xmin = meshi->xplt_smv[0];
    xmax = meshi->xplt_smv[meshi->ibar];
    if(xmin<=xval&&xval<=xmax){
      int j;
      int iimin;
      float xxmin,xxdif;

      iimin=0;
      xxdif = xval - meshi->xplt_smv[0];
      if(xxdif<0)xxdif=-xxdif;
      xxmin = xxdif;

      for(j=1;j<=meshi->ibar;j++){
        xxdif = xval - meshi->xplt_smv[j];
        if(xxdif<0)xxdif=-xxdif;
        if(xxdif<xxmin){
          xxmin=xxdif;
          iimin=j;
        }
      }
      meshi->plotx=iimin;
    }

    ymin = meshi->yplt_smv[0];
    ymax = meshi->yplt_smv[meshi->jbar];
    if(ymin<=yval&&yval<=ymax){
      int j;
      int iimin;
      float yymin,yydif;

      iimin=0;
      yydif = yval - meshi->yplt_smv[0];
      if(yydif<0)yydif=-yydif;
      yymin = yydif;

      for(j=1;j<=meshi->jbar;j++){
        yydif = yval - meshi->yplt_smv[j];
        if(yydif<0)yydif=-yydif;
        if(yydif<yymin){
          yymin=yydif;
          iimin=j;
        }
      }
      meshi->ploty=iimin;
    }

    zmin = meshi->zplt_smv[0];
    zmax = meshi->zplt_smv[meshi->kbar];
    if(zmin<=zval&&zval<=zmax){
      int j;
      int iimin;
      float zzmin,zzdif;

      iimin=0;
      zzdif = zval - meshi->zplt_smv[0];
      if(zzdif<0)zzdif=-zzdif;
      zzmin = zzdif;

      for(j=1;j<=meshi->kbar;j++){
        zzdif = zval - meshi->zplt_smv[j];
        if(zzdif<0)zzdif=-zzdif;
        if(zzdif<zzmin){
          zzmin=zzdif;
          iimin=j;
        }
      }
      meshi->plotz=iimin;
    }



  }
}

/* ------------------ UpdatePlotSliceMesh ------------------------ */

void UpdatePlotSliceMesh(meshdata *mesh_in, int slicedir){
  int i, j, k;
  int plotx, ploty, plotz;
  int ibar, jbar, kbar;
  float *xplt, *yplt, *zplt;
  meshdata *meshi;
  contour *plot3dcontour1ptr, *plot3dcontour2ptr, *plot3dcontour3ptr;
  unsigned char *yzcolorbase, *xzcolorbase, *xycolorbase;
  float *yzcolorfbase, *xzcolorfbase, *xycolorfbase;
  float *yzcolortbase, *xzcolortbase, *xycolortbase;
  float *dx_xy, *dy_xy, *dz_xy;
  float *dx_xz, *dy_xz, *dz_xz;
  float *dx_yz, *dy_yz, *dz_yz;
  float *dx_yzcopy, *dy_yzcopy, *dz_yzcopy;
  float *dx_xzcopy, *dy_xzcopy, *dz_xzcopy;
  float *dx_xycopy, *dy_xycopy, *dz_xycopy;
  float *qdata;
  unsigned char *iqdata;
  char *iblank_xy = NULL, *iblank_xz = NULL, *iblank_yz = NULL;
  int nx, ny, nz, nxy, nxyz;
  char *c_iblank_x, *c_iblank_y, *c_iblank_z;
  float qval=0.0;

  meshi = mesh_in;
  plotx = meshi->iplotx_all[iplotx_all];
  ploty = meshi->iploty_all[iploty_all];
  plotz = meshi->iplotz_all[iplotz_all];

  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;
  xplt = meshi->xplt_smv;
  yplt = meshi->yplt_smv;
  zplt = meshi->zplt_smv;
  c_iblank_x = meshi->c_iblank_x;
  c_iblank_y = meshi->c_iblank_y;
  c_iblank_z = meshi->c_iblank_z;

  yzcolorbase = meshi->yzcolorbase;
  xzcolorbase = meshi->xzcolorbase;
  xycolorbase = meshi->xycolorbase;

  yzcolorfbase = meshi->yzcolorfbase;
  xzcolorfbase = meshi->xzcolorfbase;
  xycolorfbase = meshi->xycolorfbase;

  yzcolortbase = meshi->yzcolortbase;
  xzcolortbase = meshi->xzcolortbase;
  xycolortbase = meshi->xycolortbase;

  dx_xy = meshi->dx_xy;
  dx_xz = meshi->dx_xz;
  dx_yz = meshi->dx_yz;

  dy_xy = meshi->dy_xy;
  dy_xz = meshi->dy_xz;
  dy_yz = meshi->dy_yz;

  dz_xy = meshi->dz_xy;
  dz_xz = meshi->dz_xz;
  dz_yz = meshi->dz_yz;

  plot3dcontour1ptr = meshi->plot3dcontour1;
  plot3dcontour2ptr = meshi->plot3dcontour2;
  plot3dcontour3ptr = meshi->plot3dcontour3;
  nx = ibar + 1;
  ny = jbar + 1;
  nz = kbar + 1;
  nxy = nx*ny;
  nxyz = nx*ny*nz;


  iqdata = meshi->iqdata;
  qdata = meshi->qdata;

  minfill = 1;
  maxfill = 1;
  if(setp3min_all[plotn - 1] == CHOP_MIN)minfill = 0;
  if(setp3max_all[plotn - 1] == CHOP_MAX)maxfill = 0;

  if(nplot3dloaded==0)return;
  if(plotx >= 0 && slicedir == XDIR){
    float *yzcolorf;
    float *yzcolort;
    unsigned char *yzcolor;

    yzcolor = yzcolorbase;
    yzcolorf = yzcolorfbase;
    yzcolort = yzcolortbase;
    dx_yzcopy = dx_yz;
    dy_yzcopy = dy_yz;
    dz_yzcopy = dz_yz;
    iblank_yz = NULL;
    if(global_scase.use_iblank == 1 && c_iblank_x != NULL){
      NewMemory((void **)&iblank_yz, (jbar + 1)*(kbar + 1) * sizeof(char));
      for(j = 0;j < jbar;j++){
        for(k = 0;k < kbar;k++){
          iblank_yz[k + j*kbar] = c_iblank_x[IJKNODE(plotx, j, k)];
        }
      }
    }
    for(j = 0;j <= jbar;j++){
      for(k = 0;k <= kbar;k++){
        *yzcolor++ = iqdata[IJKN(plotx, j, k, plotn - 1)];
        *yzcolort++ = (float)iqdata[IJKN(plotx, j, k, plotn - 1)] / 255.0;
        GET_QVAL(plotx, j, k, plotn - 1)
          *yzcolorf++ = qval;
        *dx_yzcopy = 0.0;
        *dy_yzcopy = 0.0;
        *dz_yzcopy = 0.0;
        if(uindex != -1){
          GET_QVAL(plotx, j, k, uindex)
            *dx_yzcopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
        if(vindex != -1){
          GET_QVAL(plotx, j, k, vindex)
            *dy_yzcopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
        if(windex != -1){
          GET_QVAL(plotx, j, k, windex)
            *dz_yzcopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
      }
    }
    FreeContour(plot3dcontour1ptr);
    InitContour(plot3dcontour1ptr, rgb_plot3d_contour, global_scase.nrgb);
    SetContourSlice(plot3dcontour1ptr, 1, xplt[plotx]);
    GetContours(yplt, zplt, jbar + 1, kbar + 1, yzcolorfbase, iblank_yz, p3levels[plotn - 1], DONT_GET_AREAS, DATA_FORTRAN, plot3dcontour1ptr);
    FREEMEMORY(iblank_yz);
  }
  else if(ploty >= 0 && slicedir == YDIR){
    float *xzcolorf;
    float *xzcolort;
    unsigned char *xzcolor;

    xzcolor = xzcolorbase;
    xzcolorf = xzcolorfbase;
    xzcolort = xzcolortbase;
    dx_xzcopy = dx_xz;
    dy_xzcopy = dy_xz;
    dz_xzcopy = dz_xz;
    iblank_xz = NULL;
    if(global_scase.use_iblank == 1 && c_iblank_y != NULL){
      NewMemory((void **)&iblank_xz, (ibar + 1)*(kbar + 1) * sizeof(char));
      for(i = 0;i < ibar;i++){
        for(k = 0;k < kbar;k++){
          iblank_xz[k + i*kbar] = c_iblank_y[IJKNODE(i, ploty, k)];
        }
      }
    }
    for(i = 0;i <= ibar;i++){
      for(k = 0;k <= kbar;k++){
        *xzcolor++ = iqdata[IJKN(i, ploty, k, plotn - 1)];
        *xzcolort++ = (float)iqdata[IJKN(i, ploty, k, plotn - 1)] / 255.0;
        GET_QVAL(i, ploty, k, plotn - 1)
          *xzcolorf++ = qval;
        *dx_xzcopy = 0.0;
        *dy_xzcopy = 0.0;
        *dz_xzcopy = 0.0;
        if(uindex != -1){
          GET_QVAL(i, ploty, k, uindex)
            *dx_xzcopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
        if(vindex != -1){
          GET_QVAL(i, ploty, k, vindex)
            *dy_xzcopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
        if(windex != -1){
          GET_QVAL(i, ploty, k, windex)
            *dz_xzcopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
      }
    }
    FreeContour(plot3dcontour2ptr);
    InitContour(plot3dcontour2ptr, rgb_plot3d_contour, global_scase.nrgb);
    SetContourSlice(plot3dcontour2ptr, 2, yplt[ploty]);
    GetContours(xplt, zplt, ibar + 1, kbar + 1, xzcolorfbase, iblank_xz, p3levels[plotn - 1], DONT_GET_AREAS, DATA_FORTRAN, plot3dcontour2ptr);
    FREEMEMORY(iblank_xz);
  }
  else if(plotz >= 0 && slicedir == ZDIR){
    float *xycolorf;
    float *xycolort;
    unsigned char *xycolor;

    xycolor = xycolorbase;
    xycolorf = xycolorfbase;
    xycolort = xycolortbase;
    dx_xycopy = dx_xy;
    dy_xycopy = dy_xy;
    dz_xycopy = dz_xy;
    iblank_xy = NULL;
    if(global_scase.use_iblank == 1 && c_iblank_z != NULL){
      NewMemory((void **)&iblank_xy, (ibar + 1)*(jbar + 1) * sizeof(char));
      for(i = 0;i < ibar;i++){
        for(j = 0;j < jbar;j++){
          iblank_xy[j + i*jbar] = c_iblank_z[IJKNODE(i, j, plotz)];
        }
      }
    }
    for(i = 0;i <= ibar;i++){
      for(j = 0;j <= jbar;j++){
        *xycolor++ = iqdata[IJKN(i, j, plotz, plotn - 1)];
        *xycolort++ = (float)iqdata[IJKN(i, j, plotz, plotn - 1)] / 255.0;
        GET_QVAL(i, j, plotz, plotn - 1)
          *xycolorf++ = qval;
        *dx_xycopy = 0.0;
        *dy_xycopy = 0.0;
        *dz_xycopy = 0.0;
        if(uindex != -1){
          GET_QVAL(i, j, plotz, uindex)
            *dx_xycopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
        if(vindex != -1){
          GET_QVAL(i, j, plotz, vindex)
            *dy_xycopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
        if(windex != -1){
          GET_QVAL(i, j, plotz, windex)
            *dz_xycopy++ = vecfactor*VECLENGTH*qval/plot3d_uvw_max;
        }
      }
    }
    FreeContour(plot3dcontour3ptr);
    InitContour(plot3dcontour3ptr, rgb_plot3d_contour, global_scase.nrgb);
    SetContourSlice(plot3dcontour3ptr, 3, zplt[plotz]);
    GetContours(xplt, yplt, ibar + 1, jbar + 1, xycolorfbase, iblank_xy, p3levels[plotn - 1], DONT_GET_AREAS, DATA_FORTRAN, plot3dcontour3ptr);
    FREEMEMORY(iblank_xy);
  }
}

/* ------------------ UpdatePlotSlice ------------------------ */

void UpdatePlotSlice(int slicedir){
  int i;

  UpdatePlotXYZ(current_mesh);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshjj;

    meshjj = global_scase.meshescoll.meshinfo + i;
    if(meshjj->plot3dfilenum==-1)continue;
    UpdatePlotSliceMesh(meshjj,slicedir);
  }
}

/* ------------------ UpdateShowStep ------------------------ */

void UpdateShowStep(int val, int slicedir){

  if(nplot3dloaded==0&&visGrid==0&&ReadVolSlice==0)return;
  current_mesh->slicedir=slicedir;
  switch(slicedir){
  case XDIR:
    if(visx_all!=val)updatemenu=1;
    break;
  case YDIR:
    if(visy_all!=val)updatemenu=1;
    break;
  case ZDIR:
    if(visz_all!=val)updatemenu=1;
    break;
  case ISO:
    if(nplot3dloaded>0){
      if(visiso!=val)updatemenu=1;
      visiso = val;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
  plotstate=GetPlotState(STATIC_PLOTS);
  stept=0;
  if(ReadVolSlice==0&&plotstate==DYNAMIC_PLOTS&&visGrid==0)update_times = 1;
  {
    int i;
    float xmin, xmax;
    float ymin, ymax;
    float zmin, zmax;

    xmin = current_mesh->xplt_smv[0];
    xmax = current_mesh->xplt_smv[current_mesh->ibar];
    ymin = current_mesh->yplt_smv[0];
    ymax = current_mesh->yplt_smv[current_mesh->jbar];
    zmin = current_mesh->zplt_smv[0];
    zmax = current_mesh->zplt_smv[current_mesh->kbar];
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      float xmin2, xmax2;
      float ymin2, ymax2;
      float zmin2, zmax2;

      meshi = global_scase.meshescoll.meshinfo+i;
      if(meshi==current_mesh)continue;

      xmin2 = meshi->xplt_smv[0];
      xmax2 = meshi->xplt_smv[meshi->ibar];
      ymin2 = meshi->yplt_smv[0];
      ymax2 = meshi->yplt_smv[meshi->jbar];
      zmin2 = meshi->zplt_smv[0];
      zmax2 = meshi->zplt_smv[meshi->kbar];


      if(slicedir==XDIR&&(xmax-MESHEPS<xmin2||xmax2-MESHEPS<xmin))continue;
      if(slicedir==YDIR&&(ymax-MESHEPS<ymin2||ymax2-MESHEPS<ymin))continue;
      if(slicedir==ZDIR&&(zmax-MESHEPS<zmin2||zmax2-MESHEPS<zmin))continue;
    }
  }
}

/* ------------------ DrawGrid ------------------------ */

void DrawGrid(const meshdata *meshi){
  int i, j, k;
  float *xplt, *yplt, *zplt;
  int ibar, jbar, kbar;
  int plotx, ploty, plotz;

  xplt = meshi->xplt_smv;
  yplt = meshi->yplt_smv;
  zplt = meshi->zplt_smv;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;
  plotx = meshi->iplotx_all[iplotx_all];
  ploty = meshi->iploty_all[iploty_all];
  plotz = meshi->iplotz_all[iplotz_all];

  // visGrid
  // 0 no grid, no pointer
  // 1 grid, no pointer
  // 2 grid, pointer
  // 3 no grid, pointer

  if(visGrid==NOGRID_PROBE||visGrid==GRID_PROBE){
    if(plotx>=0&&ploty>=0&&plotz>=0){
      unsigned char pcolor[4];

      glPushMatrix();
      glTranslatef(plotx_all[iplotx_all],ploty_all[iploty_all],plotz_all[iplotz_all]);
      pcolor[0]=255*foregroundcolor[0];
      pcolor[1]=255*foregroundcolor[1];
      pcolor[2]=255*foregroundcolor[2];
      DrawSphere(0.03,pcolor);
      glPopMatrix();
    }
  }
  if(visGrid==NOGRID_PROBE2){
    if(plotx>=0&&ploty>=0&&plotz>=0){
      int ii1, ii2, jj1, jj2, kk1, kk2;

      ii1 = iplotx_all;
      if(ii1+1>nplotx_all-1)ii1 = nplotx_all-2;
      ii2 = ii1+1;
      jj1 = iploty_all;
      if(jj1+1>nploty_all-1)jj1 = nploty_all-2;
      jj2 = jj1+1;
      kk1 = iplotz_all;
      if(kk1+1>nplotz_all-1)kk1 = nplotz_all-2;
      kk2 = kk1+1;

      float origin[3] = {plotx_all[ii1], ploty_all[jj1], plotz_all[kk1]};
      float dxyz[3] = {plotx_all[ii2]-plotx_all[ii1],
                       ploty_all[jj2]-ploty_all[jj1],
                       plotz_all[kk2]-plotz_all[kk1]};
      DrawBox2(origin, dxyz, foregroundcolor, 1);
    }
  }
  if(visx_all==0&&visy_all==0&&visz_all==0)return;
  if(visGrid==GRID_PROBE||visGrid==GRID_NOPROBE){
    AntiAliasLine(ON);
    glLineWidth(gridlinewidth);
    if(meshi->meshrgb_ptr!=NULL){
      glColor3fv(meshi->meshrgb);
    }
    else{
      glColor4fv(foregroundcolor);
    }

    int skipi, skipj, skipk;

    skipi = 1;
    skipj = 1;
    skipk = 1;
    if(clip_commandline==1){
      skipi = ibar;
      skipj = jbar;
      skipk = kbar;
    }

    glBegin(GL_LINES);
    if(visx_all==1&&plotx>=0){
      for(j=0;j<jbar+1;j+=skipj){
        glVertex3f(xplt[plotx],yplt[j],zplt[0]);
        glVertex3f(xplt[plotx],yplt[j],zplt[kbar]);
      }
      for(k=0;k<kbar+1;k+=skipk){
        glVertex3f(xplt[plotx],yplt[0],zplt[k]);
        glVertex3f(xplt[plotx],yplt[jbar],zplt[k]);
      }
    }

    if(visy_all==1&&ploty>=0){
      for(i=0;i<ibar+1;i+=skipi){
        glVertex3f(xplt[i],yplt[ploty],zplt[0]);
        glVertex3f(xplt[i],yplt[ploty],zplt[kbar]);
      }
      for(k=0;k<kbar+1;k+=skipk){
        glVertex3f(   xplt[0],yplt[ploty],zplt[k]);
        glVertex3f(xplt[ibar],yplt[ploty],zplt[k]);
      }
    }

    if(visz_all==1&&plotz>=0){
      for(i=0;i<ibar+1;i+=skipi){
        glVertex3f(xplt[i],   yplt[0],zplt[plotz]);
        glVertex3f(xplt[i],yplt[jbar],zplt[plotz]);
      }
      for(j=0;j<jbar+1;j+=skipj){
        glVertex3f(xplt[0],yplt[j],zplt[plotz]);
        glVertex3f(xplt[ibar],yplt[j],zplt[plotz]);
      }
    }

    glEnd();
    AntiAliasLine(OFF);
  }
}

/* ------------------ UpdatePlot3dMenuLabels ------------------------ */

void UpdatePlot3dMenuLabels(void){
  int i;
  plot3ddata *plot3di;
  char label[128];

  if(global_scase.nplot3dinfo>0){
    FREEMEMORY(plot3dorderindex);
    NewMemory((void **)&plot3dorderindex,sizeof(int)*global_scase.nplot3dinfo);
    for(i=0;i<global_scase.nplot3dinfo;i++){
      plot3dorderindex[i]=i;
    }
    qsort( (int *)plot3dorderindex, (size_t)global_scase.nplot3dinfo, sizeof(int), Plot3dCompare);

    for(i=0;i<global_scase.nplot3dinfo;i++){
      plot3di = global_scase.plot3dinfo + i;
      STRCPY(plot3di->menulabel,plot3di->longlabel);
      STRCPY(plot3di->menulabel,"");
      if(plot3di->time>=0.0){
        sprintf(label,"%f",plot3di->time);
        TrimZeros(label);
        STRCAT(label," s");
        STRCAT(plot3di->menulabel,label);
      }
      if(global_scase.meshescoll.nmeshes>1){
        meshdata *plot3dmesh;

        plot3dmesh = global_scase.meshescoll.meshinfo + plot3di->blocknumber;
        sprintf(label,"%s",plot3dmesh->label);
        if(plot3di->time>=0.0)STRCAT(plot3di->menulabel,", ");
        sprintf(label,"%s",plot3dmesh->label);
        STRCAT(plot3di->menulabel,label);
      }
      if(showfiles==1||plot3di->time<0.0){
        if(plot3di->time>=0.0||global_scase.meshescoll.nmeshes>1)STRCAT(plot3di->menulabel,", ");
        STRCAT(plot3di->menulabel,plot3di->file);
      }
    }
  }
}

/* ------------------ Plot3dListCompare  ------------------------ */

int Plot3dListCompare(const void *arg1, const void *arg2){
  float val1, val2;

  val1 = *(float *)arg1;
  val2 = *(float *)arg2;

  if(val1<val2)return -1;
  if(val1>val2)return 1;
  return 0;
}

/* ------------------ InitPlot3dTimeList  ------------------------ */

void InitPlot3dTimeList(void){
  int i;
  plot3ddata *plot3di;
  float lasttime_local,val;

  FREEMEMORY(plot3dtimelist);
  nplot3dtimelist=0;
  if(global_scase.nplot3dinfo>0){
    NewMemory((void **)&plot3dtimelist,global_scase.nplot3dinfo*sizeof(float));
  }
  if(plot3dtimelist==NULL)return;

  for(i=0;i<global_scase.nplot3dinfo;i++){
    plot3di = global_scase.plot3dinfo + i;
    plot3dtimelist[i]=plot3di->time;
  }
  qsort( (float *)plot3dtimelist, (size_t)global_scase.nplot3dinfo, sizeof(int), Plot3dListCompare);
  lasttime_local=-999999.0;
  for(i=0;i<global_scase.nplot3dinfo;i++){
    val=plot3dtimelist[i];
    if(ABS((double)(val-lasttime_local))>0.1){
      nplot3dtimelist++;
      plot3dtimelist[nplot3dtimelist-1]=val;
      lasttime_local=val;
    }
  }
}

/* ------------------ GetPlot3dUVW  ------------------------ */

void GetPlot3dUVW(float xyz[3], float uvw[3]){
  int i;
  float *xplt, *yplt, *zplt;

  uvw[0]=0.0;
  uvw[1]=0.0;
  uvw[2]=0.0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int ibar, jbar, kbar;
    float *udata,  *vdata, *wdata, *qdata;
    int nx, ny, nxy;
    int ix, iy, iz;
    int ijk;

    meshi = global_scase.meshescoll.meshinfo + i;

    xplt = meshi->xplt_fds;
    yplt = meshi->yplt_fds;
    zplt = meshi->zplt_fds;
    if(xyz[0]<xplt[0]||xyz[1]<yplt[0]||xyz[2]<zplt[0])continue;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;
    if(xyz[0]>xplt[ibar-1]||xyz[1]>yplt[jbar-1]||xyz[2]>zplt[kbar-1])continue;

    nx = ibar+1;
    ny = jbar+1;
    nxy = nx*ny;

    qdata = meshi->qdata;
    udata = meshi->udata;
    vdata = meshi->vdata;
    wdata = meshi->wdata;

    if(qdata==NULL)continue;
    if(udata==NULL&&vdata==NULL&&wdata==NULL)continue;

    ix = ibar*(xyz[0]-xplt[0])/(xplt[ibar]-xplt[0]);
    if(ix<0)ix=0;
    if(ix>ibar)ix=ibar;

    iy = jbar*(xyz[1]-yplt[0])/(yplt[jbar]-yplt[0]);
    if(iy<0)iy=0;
    if(iy>jbar)iy=jbar;

    iz = kbar*(xyz[2]-zplt[0])/(zplt[kbar]-zplt[0]);
    if(iz<0)iz=0;
    if(iz>kbar)iz=kbar;

    ijk=IJKNODE(ix,iy,iz);

    if(udata!=NULL){
      uvw[0]=udata[ijk];
    }

    if(vdata!=NULL){
      uvw[1]=vdata[ijk];
    }

    if(wdata!=NULL){
      uvw[2]=wdata[ijk];
    }
  }
}

  /* ------------------ GetPlot3dTime ------------------------ */

int GetPlot3dTime(float *time_local){
  int i;

  for(i=0;i<global_scase.nplot3dinfo;i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(plot3di->loaded==0||plot3di->display==0)continue;
    *time_local = plot3di->time;
    return 1;
  }
  return 0;
}
