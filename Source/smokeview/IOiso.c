#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "getdata.h"

/* ------------------ GetIsoLevels ------------------------ */

void GetIsoLevels(const char *isofile, int dataflag, float **levelsptr, float ***colorlevelsptr, int *nisolevels){
  int one;
  int version;
  int len[3],labellengths=0;
  int nlevels;
  FILE *isostreamptr;
  int i;
  float **colorlevels=NULL;

  isostreamptr=fopen(isofile,"rb");

  fread(&one,4,1,isostreamptr);
  if(dataflag!=0){
    fread(&version,4,1,isostreamptr);
  }
  else{
    version=1;
  }
  fread(len,4,3,isostreamptr);
  labellengths=len[0]+len[1]+len[2];
  FSEEK(isostreamptr,labellengths+4,SEEK_CUR);
  fread(&nlevels,4,1,isostreamptr);
  *nisolevels=nlevels;
  FREEMEMORY(*levelsptr);
  NewMemory((void **)levelsptr,nlevels*sizeof(float));
  fread(*levelsptr,4,(unsigned int)(nlevels),isostreamptr);
  fclose(isostreamptr);
  NewMemory((void **)&colorlevels,nlevels*sizeof(float *));
  for(i=0;i<nlevels;i++){
    colorlevels[i]=NULL;
  }
  *colorlevelsptr=colorlevels;
}

/* ------------------ GetIsoSizes ------------------------ */

void GetIsoSizes(const char *isofile, int dataflag, FILE **isostreamptr, int *nvertices, int *ntriangles,
                 float **levelsptr, int *nisolevels, int *niso_times,
                 float *tmin_local, float *tmax_local){
  int len[3],labellengths=0;
  int nlevels, n;
  int nvertices_i, ntriangles_i;
  int i;
  float time_local, time_max;
  int beg;
  int version;
  int one;
  int skip_local;
  float ttmin, ttmax;

  *isostreamptr=fopen(isofile,"rb");

  *tmin_local=1000000000.;
  *tmax_local=-1000000000.;
  fread(&one,4,1,*isostreamptr);
  if(dataflag!=0){
    fread(&version,4,1,*isostreamptr);
  }
  else{
    version=1;
  }
  fread(len,4,3,*isostreamptr);
  labellengths=len[0]+len[1]+len[2];
  FSEEK(*isostreamptr,labellengths+4,SEEK_CUR);
  fread(&nlevels,4,1,*isostreamptr);
  *nisolevels=nlevels;
  if(*levelsptr==NULL){
    if(NewMemory((void **)levelsptr,*nisolevels*sizeof(float))==0)return;
  }
  else{
    if(ResizeMemory((void **)levelsptr,*nisolevels*sizeof(float))==0)return;
  }
  fread(*levelsptr,4,(unsigned int)(nlevels),*isostreamptr);
  *niso_times=0; *nvertices=0; *ntriangles=0;
  beg=FTELL(*isostreamptr);
  i=0;
  time_max=-1000000.0;
  for(;;){
    int skip_frame;

    {fread(&time_local,4,1,*isostreamptr);}
    skip_frame=1;
    if(time_local>time_max){
      skip_frame=0;
      time_max=time_local;
    }
    nvertices_i=0;
    ntriangles_i=0;
    if(feof(*isostreamptr)!=0)break;
    for(n=0;n<nlevels;n++){
      {fread(&nvertices_i,4,1,*isostreamptr);}
      if(feof(*isostreamptr)!=0)break;
      {fread(&ntriangles_i,4,1,*isostreamptr);}
      if(feof(*isostreamptr)!=0)break;
      skip_local=0;
      if(nvertices_i>0){
        skip_local += 6*nvertices_i;
        FSEEK(*isostreamptr,skip_local,SEEK_CUR);
        skip_local=0;
        if(dataflag==1){
          fread(&ttmin,4,1,*isostreamptr);
          if(ttmin<*tmin_local)*tmin_local=ttmin;
          fread(&ttmax,4,1,*isostreamptr);
          if(ttmax>*tmax_local)*tmax_local=ttmax;
          skip_local += 2*nvertices_i;
        }
      }
      if(nvertices_i<256){                 /* number of triangles */
        skip_local+=ntriangles_i;
      }
      else if(nvertices_i>=256&&nvertices_i<65536){
        skip_local+=ntriangles_i*2;
      }
      else{
        skip_local+=ntriangles_i*4;
      }
      {FSEEK(*isostreamptr,skip_local,SEEK_CUR);}
    }
    if(skip_frame==1)continue;
    i++;
    if(i%tload_step!=0)continue;
    if((use_tload_begin==1&&time_local<global_scase.tload_begin))continue;
    if((use_tload_end==1&&time_local>global_scase.tload_end))continue;

    *nvertices += nvertices_i;
    *ntriangles += ntriangles_i;
    *niso_times += 1;
  }
  FSEEK(*isostreamptr,beg,SEEK_SET);
}

/* ------------------ UpdateTrianglesAll ------------------------ */

void *UpdateTrianglesAll(void *arg){
  UpdateTriangles(GEOM_DYNAMIC, GEOM_UPDATE_ALL);
  THREAD_EXIT(triangles_threads);
}

/* ------------------ ReadIsoGeomWrapup ------------------------ */

void ReadIsoGeomWrapup(int flag){
  update_readiso_geom_wrapup = UPDATE_ISO_OFF;

  if(triangles_threads == NULL){
    triangles_threads = THREADinit(&n_triangles_threads, &use_triangles_threads, UpdateTrianglesAll);
  }
  THREADrun(triangles_threads);
  if(flag == FOREGROUND)THREADcontrol(triangles_threads, THREAD_JOIN);
  UpdateTimes();
  GetFaceInfo();
  ForceIdle();
}

/* ------------------ UnloadIsoTrans ------------------------ */

void UnloadIsoTrans(void){
  if(iso_trans_list != NULL){
    int i;

    for(i = 0;i < niso_timesteps;i++){
      FREEMEMORY(iso_trans_list[i]);
    }
    FREEMEMORY(niso_trans_list);
    FREEMEMORY(iso_trans_list);
  }
  if(iso_opaques_list != NULL){
    int i;

    for(i = 0;i < niso_timesteps;i++){
      FREEMEMORY(iso_opaques_list[i]);
    }
    FREEMEMORY(niso_opaques_list);
    FREEMEMORY(iso_opaques_list);
  }

  niso_trans = 0;
  niso_opaques = 0;
}

/* ------------------ UnloadIso ------------------------ */

void UnloadIso(meshdata *meshi){
  isodata *ib;
  int nloaded = 0;
  int i;
  meshdata *meshi2;

  if(meshi->isofilenum == -1)return;
  ib = global_scase.isoinfo + meshi->isofilenum;

  FreeAllMemory(ib->memory_id);
  meshi->iso_times = NULL;
  meshi->iso_times_map = NULL;

  UnloadIsoTrans();

  ib->loaded = 0;
  ib->display = 0;
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  meshi->isofilenum = -1;
  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    meshi2 = global_scase.meshescoll.meshinfo + i;
    if(meshi2->isofilenum != -1)nloaded++;
  }
  if(nloaded == 0){
    ReadIsoFile = 0;
  }

  UpdateTimes();
  PrintMemoryInfo;
  updatemenu = 1;
  ForceIdle();
}

/* ------------------ GetIsoType ------------------------ */

int GetIsoType(const isodata *isoi){
  isodata *isoi2;
  int j;

  for(j = 0;j < global_scase.nisotypes;j++){
    isoi2 = global_scase.isoinfo + global_scase.isotypes[j];

    if(strcmp(isoi->surface_label.longlabel, isoi2->surface_label.longlabel) == 0)return j;
  }
  return -1;
}

/* ------------------ GetIsoDataBounds ------------------------ */

void GetIsoDataBounds(isodata *isod, float *pmin, float *pmax){
  float *pdata;
  int ndata;
  int i;

  pdata = isod->geom_vals;
  ndata = isod->geom_nvals;
  if(ndata > 0 && pdata != NULL){
    *pmin = pdata[0];
    *pmax = pdata[0];
    for(i = 1; i < ndata; i++){
      *pmin = MIN(*pmin, pdata[i]);
      *pmax = MAX(*pmax, pdata[i]);
    }
  }
  else{
    *pmin = 0.0;
    *pmax = 1.0;
  }
}

/* ------------------ OutputIsoBounds ------------------------ */

void OutputIsoBounds(isodata *isoi){
  FILE *stream;
  char file[256];
  int i;
  geomdata *geomi;

  geomi = isoi->geominfo;
  strcpy(file, geomi->file);
  strcat(file, ".csv");
  stream = fopen(file, "w");
  if(stream == NULL)return;
  if(geomi->ntimes <= 0){
    fclose(stream);
    return;
  }
  fprintf(stream,"%s\n", isoi->surface_label.longlabel);
  fprintf(stream, "t,xmin,xmax,ymin,ymax,zmin,zmaz\n");
  for(i = 0;i < geomi->ntimes;i++){
    int j;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    geomlistdata *geomlisti;
    float *xyz;

    geomlisti = geomi->geomlistinfo + i;
    if(geomlisti->nverts>0){
      xyz = geomlisti->verts->xyz;

      xmin = xyz[0];
      xmax = xmin;
      ymin = xyz[1];
      ymax = ymin;
      zmin = xyz[2];
      zmax = zmin;
      for(j = 1;j < geomlisti->nverts;j++){
        vertdata *vertj;

        vertj = geomlisti->verts+j;
        xyz = vertj->xyz;
        xmin = MIN(xyz[0], xmin);
        xmax = MAX(xyz[0], xmax);
        ymin = MIN(xyz[1], ymin);
        ymax = MAX(xyz[1], ymax);
        zmin = MIN(xyz[2], zmin);
        zmax = MAX(xyz[2], zmax);
      }
      fprintf(stream, "%f,%f,%f,%f,%f,%f,%f\n", geomi->times[i], xmin, xmax, ymin, ymax, zmin, zmax);
    }
  }
  fclose(stream);
}

/* ------------------ OutputAllIsoBounds ------------------------ */

void OutputAllIsoBounds(void){
  int i;

  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded == 0||isoi->geominfo==NULL)continue;
    OutputIsoBounds(isoi);
  }
}

/* ------------------ GetIsoTType ------------------------ */

int GetIsoTType(const isodata *isoi){
  isodata *isoi2;
  int j;
  int jj;

  if(isoi->dataflag == 0)return -1;
  jj = 0;
  for(j = 0;j < global_scase.nisoinfo;j++){
    isoi2 = global_scase.isoinfo + j;

    if(isoi2->dataflag == 0)continue;
    if(isoi2->firstshort_iso == 0)continue;
    if(strcmp(isoi->color_label.longlabel, isoi2->color_label.longlabel) == 0)return jj;
    jj++;
  }
  return -1;
}

/* ------------------ SyncIsoBounds ------------------------ */

void SyncIsoBounds(){
  int i, ncount;
  int firsttime = 1;
  float tmin_local=1.0, tmax_local=0.0;

  // find number of iso-surfaces with values

  ncount = 0;
  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->type != iisotype || isoi->dataflag == 0 || iisottype != GetIsoTType(isoi))continue;
    ncount++;
  }
  if(ncount <= 1)return;

  // find min and max bounds for valued iso-surfaces

  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->type != iisotype || isoi->dataflag == 0 || iisottype != GetIsoTType(isoi))continue;
    if(firsttime == 1){
      firsttime = 0;
      tmin_local = isoi->tmin;
      tmax_local = isoi->tmax;
    }
    else{
      if(tmin_local < isoi->tmin)isoi->tmin = tmin_local;
      if(tmax_local > isoi->tmax)isoi->tmax = tmax_local;
    }
  }

  // set min and max bounds for valued iso-surfaces

  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->type != iisotype || isoi->dataflag == 0 || iisottype != GetIsoTType(isoi))continue;
    isoi->tmin = tmin_local;
    isoi->tmax = tmax_local;
  }

  // rescale all data

  for(i = 0;i < global_scase.nisoinfo;i++){
    isodata *isoi;
    meshdata *meshi;
    int ii;
    isosurface *asurface;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded == 0 || isoi->type != iisotype || isoi->dataflag == 0)continue;
    if(iisottype != GetIsoTType(isoi))continue;

    meshi = global_scase.meshescoll.meshinfo + isoi->blocknumber;
    asurface = meshi->animatedsurfaces;

    for(ii = 0;ii < meshi->niso_times;ii++){
      int j;

      for(j = 0;j < meshi->nisolevels;j++){
        float tcolor, tcolor0, tcolorfactor;
        int kk;

        if(isoi->tmax > isoi->tmin){
          tcolor0 = (asurface->tmin - isoi->tmin) / (isoi->tmax - isoi->tmin);
          tcolorfactor = (asurface->tmax - asurface->tmin) / 65535.;
          tcolorfactor /= (isoi->tmax - isoi->tmin);
        }
        else{
          tcolor0 = 0.5;
          tcolorfactor = 0.0;
        }
        for(kk = 0;kk < asurface->nvertices;kk++){
          tcolor = tcolor0 + asurface->tvertices[kk] * tcolorfactor;
          asurface->color8[kk] = (unsigned char)(CLAMP(tcolor, 0.0, 1.0) * 255);
        }
        asurface++;
      }
    }
  }
}

/* ------------------ ReadIsoGeom ------------------------ */

FILE_SIZE ReadIsoGeom(int ifile, int load_flag, int *geom_frame_index, int *errorcode){
  isodata *isoi;
  geomdata *geomi;
  int ilevel,error;
  meshdata *meshi;
  int i;
  surfdata *surfi;
  FILE_SIZE return_filesize=0;

  isoi = global_scase.isoinfo + ifile;
  if(load_flag==LOAD||load_flag==RELOAD){
    THREADcontrol(isosurface_threads, THREAD_JOIN);
  }
  if(load_flag==UNLOAD){
    CancelUpdateTriangles();
  }
  meshi = global_scase.meshescoll.meshinfo + isoi->blocknumber;
  geomi = isoi->geominfo;
  UnloadIso(meshi);
  FreeAllMemory(isoi->memory_id);
  meshi->showlevels = NULL;
  meshi->isolevels = NULL;
  return_filesize = ReadGeom(geomi, NULL, 0, load_flag, GEOM_ISO, geom_frame_index);

  if(load_flag==UNLOAD){
    meshi->isofilenum = -1;
    return 0;
  }

  if(isoi->dataflag==1){
    int filesize;
    int ntimes_local;
    float *valptr;

    ntimes_local = GetGeomDataSize(isoi->tfile, &isoi->geom_nvals, ALL_FRAMES, NULL, NULL, NULL, NULL, NULL, &error);
    if(isoi->geom_nvals>0&&ntimes_local>0){
      NewMemoryMemID((void **)&isoi->geom_nstatics,  ntimes_local*sizeof(int),       isoi->memory_id);
      NewMemoryMemID((void **)&isoi->geom_ndynamics, ntimes_local*sizeof(int),       isoi->memory_id);
      NewMemoryMemID((void **)&isoi->geom_times,     ntimes_local*sizeof(float),     isoi->memory_id);
      NewMemoryMemID((void **)&isoi->geom_times_map, ntimes_local*sizeof(unsigned char), isoi->memory_id);
      NewMemoryMemID((void **)&isoi->geom_vals,      isoi->geom_nvals*sizeof(float), isoi->memory_id);
    }

    filesize = GetGeomData(NULL, isoi->tfile, load_flag, ntimes_local, isoi->geom_nvals, isoi->geom_times,
                           isoi->geom_nstatics, isoi->geom_ndynamics, isoi->geom_vals,
                           ALL_FRAMES, NULL, NULL, &error);
    return_filesize += filesize;
    FREEMEMORY(isoi->geom_nstatics);
    FREEMEMORY(isoi->geom_times);
    FREEMEMORY(isoi->geom_times_map);
    valptr = isoi->geom_vals;
    for(i = 0; i<ntimes_local; i++){
      geomlistdata *geomlisti;

      geomlisti = geomi->geomlistinfo+i;
      geomlisti->vertvals = valptr;
      valptr += isoi->geom_ndynamics[i];
    }
    FREEMEMORY(isoi->geom_ndynamics);
  }

  surfi = global_scase.surfcoll.surfinfo + global_scase.surfcoll.nsurfinfo+1;
  UpdateIsoColors();
  if(strcmp(isoi->surface_label.shortlabel,"hrrpuv")==0){
    surfi->color=GetColorPtr(&global_scase, hrrpuv_iso_color);
  }

  meshi->isofilenum=ifile;
  meshi->niso_times=geomi->ntimes;
  if(NewMemoryMemID((void **)&meshi->iso_times, sizeof(float)*meshi->niso_times, isoi->memory_id) == 0){
    ReadIso("",ifile,UNLOAD,geom_frame_index,&error);
    *errorcode=1;
    return 0;
  }
  if(NewMemoryMemID((void **)&meshi->iso_times_map, meshi->niso_times, isoi->memory_id) == 0){
    ReadIso("", ifile, UNLOAD, geom_frame_index, &error);
    *errorcode = 1;
    return 0;
  }
  for(i=0;i<geomi->ntimes;i++){
    meshi->iso_times[i]=geomi->times[i];
    meshi->iso_times_map[i] = 1;
  }
  MakeTimesMap(meshi->iso_times, &meshi->iso_times_map, geomi->ntimes);
  meshi->nisolevels=geomi->nfloat_vals;
  if(
    NewMemoryMemID((void **)&meshi->showlevels, sizeof(int)*meshi->nisolevels, isoi->memory_id) == 0 ||
    NewMemoryMemID((void **)&meshi->isolevels, sizeof(int)*meshi->nisolevels, isoi->memory_id) == 0
    ){
    *errorcode=1;
    ReadIso("",ifile,UNLOAD,geom_frame_index,&error);
    return 0;
  }
  for(ilevel=0;ilevel<meshi->nisolevels;ilevel++){
    meshi->showlevels[ilevel]=1;
    meshi->isolevels[ilevel]=geomi->float_vals[ilevel];
  }
  isoi->loaded=1;
  isoi->display=1;
  loaded_isomesh= GetLoadedIsoMesh();
  UpdateIsoShowLevels(&global_scase, loaded_isomesh);
  ReadIsoFile=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  updatemenu=1;
  iisotype=GetIsoType(isoi);

  if(update_readiso_geom_wrapup==UPDATE_ISO_OFF)update_readiso_geom_wrapup=UPDATE_ISO_ONE_NOW;
  if(update_readiso_geom_wrapup==UPDATE_ISO_START_ALL)update_readiso_geom_wrapup=UPDATE_ISO_ALL_NOW;

  if(isoi->dataflag==1){
    GetIsoDataBounds(isoi, &iso_valmin, &iso_valmax);
    isoi->globalmin_iso = iso_valmin;
    isoi->globalmax_iso = iso_valmax;
    if(isoi->finalize == 1){
      iso_global_min = 1.0;
      iso_global_max = 0.0;
      for(i = 0;i < global_scase.nisoinfo;i++){
        isodata *isoj;

        isoj = global_scase.isoinfo + i;
        if(isoj->loaded == 0)continue;
        if(iso_global_min > iso_valmax){
          iso_global_min = isoj->globalmin_iso;
          iso_global_max = isoj->globalmax_iso;
        }
        else{
          iso_global_min = MIN(iso_global_min, isoj->globalmin_iso);
          iso_global_max = MAX(iso_global_max, isoj->globalmax_iso);
        }
      }
      for(i = 0;i < global_scase.nisoinfo;i++){
        isodata *isoj;

        isoj = global_scase.isoinfo + i;
        if(isoj->loaded == 0)continue;
        isoj->globalmin_iso = iso_global_min;
        isoj->globalmax_iso = iso_global_max;
      }
      iisottype = GetIsoTType(isoi);
   //   SyncIsoBounds();
      SetIsoLabels(isoi->tmin, isoi->tmax, isoi, errorcode);
      GLUIUpdateIsoBounds();
      GLUIIsoBoundCB(ISO_VALMIN);
      GLUIIsoBoundCB(ISO_VALMAX);
    }
  }
  PrintMemoryInfo;
  show_isofiles = 1;

  GLUTPOSTREDISPLAY;
  CheckMemory;
  return return_filesize;
}

/* ------------------ ReadIsoOrig ------------------------ */

void ReadIsoOrig(const char *file, int ifile, int flag, int *errorcode){
  int itime,ilevel,itri,ivert,iitime;
  isosurface *asurface;
  int nisopoints, nisotriangles;
#ifdef _DEBUG
  int ntotal_isotris=0, ntotal_isoverts=0;
#endif
  float time_local, time_max;
  FILE *isostream;
  int break_frame;
  int skip_local;
  float *fed_colors[3];
  float read_time, total_time;
  FILE_SIZE read_size;

  int blocknumber;
  int error;
  float factor, offset[3];

  meshdata *meshi;
  isodata *ib;

  START_TIMER(total_time);
  if(flag==LOAD){
    THREADcontrol(isosurface_threads, THREAD_JOIN);
  }
  assert(ifile>=0&&ifile<global_scase.nisoinfo);
  ib = global_scase.isoinfo+ifile;
  if(ib->loaded==0&&flag==UNLOAD)return;
  blocknumber=ib->blocknumber;
  ib->isoupdate_timestep=-1;
  meshi = global_scase.meshescoll.meshinfo+blocknumber;
  UnloadIso(meshi);
  UnloadIsoTrans();
  ib->loaded=0;
  ib->display=0;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  *errorcode = 0;

  UpdateIsoType();
  if(flag==UNLOAD){
    updatemenu=1;
    loaded_isomesh= GetLoadedIsoMesh();
    UpdateIsoShowLevels(&global_scase, loaded_isomesh);
    return;
  }
  meshi->isofilenum=ifile;
  highlight_mesh = blocknumber;

  factor = (SCALE2SMV(meshi->xyzmaxdiff))/65535.0;
  FDS2SMV_XYZ(offset,meshi->xyz_bar0);

  GetIsoSizes(file, ib->dataflag, &isostream, &nisopoints, &nisotriangles,
    &meshi->isolevels, &meshi->nisolevels, &meshi->niso_times,
    &ib->tmin, &ib->tmax);

  if(meshi->isolevels==NULL){
    ReadIso("",ifile,UNLOAD,NULL,&error);
    *errorcode=1;
    fclose(isostream);
    return;
  }
  if(NewMemoryMemID((void **)&meshi->iso_times,sizeof(float)*meshi->niso_times, ib->memory_id)==0){
    ReadIso("",ifile,UNLOAD,NULL,&error);
    *errorcode=1;
    fclose(isostream);
    return;
  }
  if(NewMemoryMemID((void **)&meshi->showlevels,sizeof(int)*meshi->nisolevels,ib->memory_id)==0){
    *errorcode=1;
    ReadIso("",ifile,UNLOAD,NULL,&error);
    fclose(isostream);
    return;
  }
  for(ilevel=0;ilevel<meshi->nisolevels;ilevel++){
    meshi->showlevels[ilevel]=1;
  }
  isomin=meshi->isolevels[0];
  isomax=meshi->isolevels[0];
  meshi->isomin_index=0;
  meshi->isomax_index=0;
  for(ilevel=1;ilevel<meshi->nisolevels;ilevel++){
    if(meshi->isolevels[ilevel]<isomin){
      isomin=meshi->isolevels[ilevel];
      meshi->isomin_index=ilevel;
    }
    if(meshi->isolevels[ilevel]>isomax){
      isomax=meshi->isolevels[ilevel];
      meshi->isomax_index=ilevel;
    }
  }
  //assert(meshi->animatedsurfaces==NULL);
  if(NewMemoryMemID((void **)&meshi->animatedsurfaces,meshi->nisolevels*meshi->niso_times*sizeof(isosurface),ib->memory_id)==0){
    *errorcode=1;
    ReadIso("",ifile,UNLOAD,NULL,&error);
    fclose(isostream);
    return;
  }
  if(ResizeMemory((void **)&meshi->iso_times,sizeof(float)*meshi->niso_times)==0){
    *errorcode=1;
    ReadIso("",ifile,UNLOAD,NULL,&error);
    fclose(isostream);
    return;
  }
  {
    float fed_blue[]={0.0,0.0,1.0,1.0};
    float fed_yellow[]={1.0,1.0,0.0,1.0};
    float fed_red[]={1.0,0.0,0.0,1.0};

    fed_colors[0]=GetColorPtr(&global_scase, fed_blue);
    fed_colors[1]=GetColorPtr(&global_scase, fed_yellow);
    fed_colors[2]=GetColorPtr(&global_scase, fed_red);
  }
  asurface=meshi->animatedsurfaces;
  break_frame=0;
  iitime=0;
  itime=0;
  time_max = -1000000.0;

  START_TIMER(read_time);
  read_size=0;
  for(;;){
    int skip_frame;

    iitime++;

    fread(&time_local,4,1,isostream);
    read_size+=4+4+4;
    if(feof(isostream)!=0)break;
    skip_frame=1;
    if(time_local>time_max){
      skip_frame=0;
      time_max=time_local;
    }
    meshi->iso_times[itime]=time_local;
    if(iitime%tload_step!=0||(use_tload_begin==1&&time_local<global_scase.tload_begin)||(use_tload_end==1&&time_local>global_scase.tload_end)||skip_frame==1){
    }
    for(ilevel=0;ilevel<meshi->nisolevels;ilevel++){
      int nvertices_i, ntriangles_i;

      asurface->dataflag=ib->dataflag;

      fread(&nvertices_i,4,1,isostream);
      read_size+=4+4+4;
#ifdef _DEBUG
      ntotal_isoverts+=nvertices_i;
#endif
      if(feof(isostream)!=0)break;
      fread(&ntriangles_i,4,1,isostream);
      read_size+=4+4+4;
#ifdef _DEBUG
      ntotal_isotris+=ntriangles_i;
#endif
      if(feof(isostream)!=0)break;
      asurface->niso_triangles=ntriangles_i/3;
      asurface->niso_vertices=nvertices_i;

      if(iitime%tload_step!=0||(use_tload_begin==1&&time_local<global_scase.tload_begin)||(use_tload_end==1&&time_local>global_scase.tload_end)||skip_frame==1){
        skip_local=0;
        if(nvertices_i<=0||ntriangles_i<=0)continue;
        skip_local += (6*nvertices_i);
        if(ib->dataflag==1)skip_local += (8 + 2*nvertices_i);
        if(nvertices_i<256){
          skip_local += (ntriangles_i);
        }
        else if(nvertices_i>=256&&nvertices_i<65536){
          skip_local += (ntriangles_i*2);
        }
        else{
          skip_local += (ntriangles_i*4);
        }
        FSEEK(isostream,skip_local,SEEK_CUR);
        continue;
      }

      asurface->iso_triangles=NULL;
      asurface->iso_vertices=NULL;
      if(nvertices_i>0){
        unsigned short *verti;
        unsigned short *vertices_i;

        if(NewMemoryMemID((void **)&asurface->iso_vertices,nvertices_i*sizeof(isovert),ib->memory_id)==0){
          break_frame=1;
          break;
        }
        if(NewMemoryMemID((void **)&vertices_i,3*nvertices_i*sizeof(unsigned short),ib->memory_id)==0){
          break_frame=1;
          break;
        }
        verti = vertices_i;
        fread(vertices_i,2,(unsigned int)(3*nvertices_i),isostream);
        read_size+=4+3*nvertices_i*2+4;
        for(ivert=0;ivert<nvertices_i;ivert++){
          isovert *isoverti;
          float *xyz;

          isoverti = asurface->iso_vertices+ivert;
          xyz = isoverti->xyz;
          xyz[0]=offset[XXX]+factor*(*verti++);
          xyz[1]=offset[YYY]+factor*(*verti++);
          xyz[2]=offset[ZZZ]+factor*(*verti++);
          isoverti->flag=0;

          if(ilevel==0&&strcmp(ib->surface_label.shortlabel,"hrrpuv")==0){
            isoverti->color=hrrpuv_iso_color;
          }
          else if(strcmp(ib->surface_label.shortlabel,"FED")==0){
            isoverti->color=fed_colors[ilevel%3];
          }
          else{
            isoverti->color=iso_colors+4*ilevel;
          }
        }
        FREEMEMORY(vertices_i);

        if(ib->dataflag==1){
          unsigned short *tvertices_i;
          float tcolorfactor, tcolorfactor2;

          fread(&asurface->tmin,4,1,isostream);
          fread(&asurface->tmax,4,1,isostream);
          read_size+=2*(4+4+4);
          //printf("amin=%f amax=%f imin=%f imax=%f\n",asurface->tmin,asurface->tmax,ib->tmin,ib->tmax);
          if(NewMemoryMemID((void **)&tvertices_i,nvertices_i*sizeof(unsigned short),ib->memory_id)==0){
            break_frame=1;
            break;
          }
          fread(tvertices_i,2,(unsigned int)nvertices_i,isostream);
          read_size+=4+2*nvertices_i+4;
          tcolorfactor = (asurface->tmax-asurface->tmin)/65535.;
          if(ib->tmax>ib->tmin){
            tcolorfactor2 = 255.0/(ib->tmax-ib->tmin);
          }
          else{
            tcolorfactor2 = 1.0;
          }
          for(ivert=0;ivert<nvertices_i;ivert++){
            isovert *isoverti;
            unsigned char colorindex;
            float tcolor;

            isoverti = asurface->iso_vertices+ivert;
            tcolor = asurface->tmin + tvertices_i[ivert]*tcolorfactor;
            colorindex = (unsigned char)CLAMP((tcolor-ib->tmin)*tcolorfactor2,0,255);
           // PRINTF("color= %f %i %i\n",tcolor,(int)colorindex,(int)tvertices_i[ivert]);
            isoverti->color = rgb_iso+4*colorindex;
            isoverti->ctexturecolor=colorindex;
          }
          FREEMEMORY(tvertices_i);
        }
      }
      if(feof(isostream)!=0)break;
      if(ntriangles_i>0){
        unsigned char *triangles1_i;
        unsigned short *triangles2_i;
        int *triangles_i;

        if(NewMemoryMemID((void **)&triangles_i,ntriangles_i*sizeof(int),ib->memory_id)==0){
          break_frame=1;
          break;
        }
        if(nvertices_i<256&&nvertices_i>0){
          if(NewMemoryMemID((void **)&triangles1_i,ntriangles_i*sizeof(unsigned char),ib->memory_id)==0){
            break_frame=1;
            break;
          }
          fread(triangles1_i,1,(unsigned int)ntriangles_i,isostream);
          read_size+=4+ntriangles_i+4;
          for(itri=0;itri<ntriangles_i;itri++){
            triangles_i[itri]=triangles1_i[itri];
          }
          FREEMEMORY(triangles1_i);
        }
        else if(nvertices_i>=256&&nvertices_i<65536){
          if(NewMemoryMemID((void **)&triangles2_i,ntriangles_i*sizeof(unsigned short),ib->memory_id)==0){
            break_frame=1;
            break;
          }
          fread(triangles2_i,2,(unsigned int)ntriangles_i,isostream);
          read_size+=4+2*ntriangles_i+4;
          for(itri=0;itri<ntriangles_i;itri++){
            triangles_i[itri]=triangles2_i[itri];
          }
          FREEMEMORY(triangles2_i);
        }
        else{
          fread(triangles_i,4,(unsigned int)ntriangles_i,isostream);
          read_size+=4+4*ntriangles_i+4;
        }
        if(NewMemoryMemID((void **)&asurface->iso_triangles,(ntriangles_i/3)*sizeof(isotri),ib->memory_id)==0){
          break_frame=1;
          break;
        }
        for(itri=0;itri<ntriangles_i/3;itri++){
          isotri *isotrii;
          float **color;

          isotrii=asurface->iso_triangles+itri;
          isotrii->v1=asurface->iso_vertices+triangles_i[3*itri];
          isotrii->v2=asurface->iso_vertices+triangles_i[3*itri+1];
          isotrii->v3=asurface->iso_vertices+triangles_i[3*itri+2];
          if(ilevel==0&&strcmp(ib->surface_label.shortlabel,"hrrpuv")==0){
            ib->colorlevels[ilevel]=hrrpuv_iso_color;
          }
          else if(strcmp(ib->surface_label.shortlabel,"FED")==0){
            ib->colorlevels[ilevel]=fed_colors[ilevel%3];
          }
          else{
            ib->colorlevels[ilevel]=iso_colors+4*ilevel;
          }
          color=ib->colorlevels+ilevel;
          if(ib->dataflag==0){
            isotrii->v1->color=*color;
            isotrii->v2->color=*color;
            isotrii->v3->color=*color;
          }
        }
        FREEMEMORY(triangles_i);
      }

      if(feof(isostream)!=0)break;

      if(nvertices_i>0){
        float *vertnorms=NULL;
        if(NewMemoryMemID((void **)&vertnorms,3*nvertices_i*sizeof(float),ib->memory_id)==0){
          break_frame=1;
          break;
        }
        for(ivert=0;ivert<nvertices_i;ivert++){
          vertnorms[3*ivert]=0.0;
          vertnorms[3*ivert+1]=0.0;
          vertnorms[3*ivert+2]=0.0;
        }
        for(itri=0;itri<ntriangles_i/3;itri++){
          isotri *isotrii;
          float *v1, *v2, *v3;
          float *vertnorm;
          float area;
          float out[3];

          isotrii = asurface->iso_triangles+itri;
          v1=isotrii->v1->xyz;
          v2=isotrii->v2->xyz;
          v3=isotrii->v3->xyz;
          CalcNormal2f(v1,v2,v3,out,&area);

          vertnorm = vertnorms + 3*(isotrii->v1-asurface->iso_vertices);
          vertnorm[0] += out[0]*area;
          vertnorm[1] += out[1]*area;
          vertnorm[2] += out[2]*area;

          vertnorm = vertnorms + 3*(isotrii->v2-asurface->iso_vertices);
          vertnorm[0] += out[0]*area;
          vertnorm[1] += out[1]*area;
          vertnorm[2] += out[2]*area;

          vertnorm = vertnorms + 3*(isotrii->v3-asurface->iso_vertices);
          vertnorm[0] += out[0]*area;
          vertnorm[1] += out[1]*area;
          vertnorm[2] += out[2]*area;
        }
        for(ivert=0;ivert<nvertices_i;ivert++){
          isovert *v1;

          v1 = asurface->iso_vertices + ivert;
          ReduceToUnit(vertnorms+3*ivert);
          v1->cnorm=(unsigned char)GetNormalIndex(global_scase.sphereinfo,vertnorms+3*ivert);
        }
        FREEMEMORY(vertnorms);
      }
      asurface++;
    }

    if(break_frame==1){
      fprintf(stderr,"*** Error: memory allocation attempt failed at time step: %i while reading isosurface file\n",itime);
      meshi->niso_times=itime;
      break;
    }
    if(skip_frame==1||iitime%tload_step!=0||(use_tload_begin==1&&time_local<global_scase.tload_begin)||(use_tload_end==1&&time_local>global_scase.tload_end)){
    }
    else{
      itime++;
      if(itime>=meshi->niso_times)break;
    }
  }
#ifdef _DEBUG
  PRINTF("nverts=%i ntris=%i\n",ntotal_isoverts,ntotal_isotris);
  PRINTF("size verts=%i tris=%i\n",(int)(ntotal_isoverts*sizeof(isovert)),(int)(ntotal_isotris*sizeof(isotri)/3));
#endif
  STOP_TIMER(read_time);
  PRINTF(" - %.1f MB/%.1f s\n",(float)read_size/1000000.,read_time);

  fclose(isostream);
  if(*errorcode!=0){
    UnloadIso(meshi);
    ReadIso("",ifile,UNLOAD,NULL,&error);
    return;
  }

  ib->loaded=1;
  ib->display=1;
  loaded_isomesh= GetLoadedIsoMesh();
  UpdateIsoShowLevels(&global_scase, loaded_isomesh);
  ReadIsoFile=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  updatemenu=1;
  iisotype= GetIsoType(ib);

  CheckMemory;
  if(ib->dataflag==1){
    iisottype = GetIsoTType(ib);
   // SyncIsoBounds();
    SetIsoLabels(ib->tmin, ib->tmax, ib, errorcode);
    CheckMemory;
  }

  UpdateTimes();
  PrintMemoryInfo;
  ForceIdle();

  STOP_TIMER(total_time);

  show_isofiles = 1;
  GLUTPOSTREDISPLAY;
  CheckMemory;
}

/* ------------------ ReadIso ------------------------ */

FILE_SIZE ReadIso(const char *file, int ifile, int flag, int *geom_frame_index, int *errorcode){
  isodata *isoi;
  FILE_SIZE return_filesize=0;

  SetTimeState();
  if(ifile>=0&&ifile<global_scase.nisoinfo){

    isoi = global_scase.isoinfo+ifile;
    if(flag == UNLOAD && isoi->loaded == 0)return 0;
    if(flag==LOAD)PRINTF("Loading %s(%s)", file,isoi->surface_label.shortlabel);
      if(isoi->geomflag==1){
        return_filesize=ReadIsoGeom(ifile,flag,geom_frame_index,errorcode);
      }
      else{
        ReadIsoOrig(file,ifile,flag,errorcode);
      }
    }
  return return_filesize;
}

/* ------------------ DrawIsoOrig ------------------------ */

void DrawIsoOrig(int tranflag){
  int i;
  isodata *isoi=NULL;
  int iso_lighting;
  meshdata *meshi;

  meshi = loaded_isomesh;
  if(meshi->datavis == 0)return;

  CheckMemory;
  if(tranflag==DRAW_TRANSPARENT&&((visAIso&1)==0))return;
  if(meshi->isofilenum>=0&& global_scase.isoinfo!=NULL){
    isoi = global_scase.isoinfo + meshi->isofilenum;
  }
  if(isoi==NULL)return;

  iso_lighting=1;

  if((visAIso&1)==1){
    isotri **iso_list_start;
    int niso_list_start;

    if(cullfaces==1)glDisable(GL_CULL_FACE);

    iso_specular[3] = 1.0;
    if(tranflag==DRAW_TRANSPARENT)TransparentOn();

    if(isoi->dataflag==1){
      glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
      glEnable(GL_TEXTURE_1D);
      glBindTexture(GL_TEXTURE_1D,texture_iso_colorbar_id);
    }

    glPushAttrib(GL_LIGHTING_BIT);
    if(iso_lighting==1){
      ENABLE_LIGHTING;
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,iso_specular);
      glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,iso_shininess);
      glEnable(GL_COLOR_MATERIAL);
    }
    glBegin(GL_TRIANGLES);

    if(tranflag==DRAW_TRANSPARENT){
      iso_list_start=iso_trans;
      niso_list_start=niso_trans;
    }
    else{
      iso_list_start=iso_opaques;
      niso_list_start=niso_opaques;
    }
    CheckMemory;
    if(isoi->dataflag==1){
      for(i=0;i<niso_list_start;i++){
        isotri *tri;
        isovert *v1, *v2, *v3;

        tri=iso_list_start[i];

        v1 = tri->v1;
        v2 = tri->v2;
        v3 = tri->v3;

        glTexCoord1f(v1->ctexturecolor/255.0);
        glNormal3fv(GetNormalVectorPtr(global_scase.sphereinfo,v1->cnorm));
        glVertex3fv(v1->xyz);

        glTexCoord1f(v2->ctexturecolor/255.0);
        glNormal3fv(GetNormalVectorPtr(global_scase.sphereinfo,v2->cnorm));
        glVertex3fv(v2->xyz);

        glTexCoord1f(v3->ctexturecolor/255.0);
        glNormal3fv(GetNormalVectorPtr(global_scase.sphereinfo,v3->cnorm));
        glVertex3fv(v3->xyz);
      }
    }
    else{
      for(i=0;i<niso_list_start;i++){
        isotri *tri;
        isovert *v1, *v2, *v3;

        tri=iso_list_start[i];

        v1 = tri->v1;
        v2 = tri->v2;
        v3 = tri->v3;

        glColor4fv(v1->color);
        glNormal3fv(GetNormalVectorPtr(global_scase.sphereinfo,v1->cnorm));
        glVertex3fv(v1->xyz);

        glColor4fv(v2->color);
        glNormal3fv(GetNormalVectorPtr(global_scase.sphereinfo,v2->cnorm));
        glVertex3fv(v2->xyz);

        glColor4fv(v3->color);
        glNormal3fv(GetNormalVectorPtr(global_scase.sphereinfo,v3->cnorm));
        glVertex3fv(v3->xyz);
      }
    }
    glEnd();

    glPopAttrib();
    if(isoi->dataflag==1)glDisable(GL_TEXTURE_1D);


    if(tranflag==DRAW_TRANSPARENT)TransparentOff();
    if(cullfaces==1)glEnable(GL_CULL_FACE);
    CheckMemory;
  }

  if((visAIso&2)==2){
    glPushAttrib(GL_LIGHTING_BIT);
    AntiAliasLine(ON);
    glLineWidth(isolinewidth);
    glBegin(GL_LINES);
    for(i=0;i<niso_trans;i++){
      isotri *tri;
      float *xyz1, *xyz2, *xyz3;
      float *color1, *color2, *color3;

      tri=iso_trans[i];

      xyz1 = tri->v1->xyz;
      xyz2 = tri->v2->xyz;
      xyz3 = tri->v3->xyz;
      color1 = tri->v1->color;
      color2 = tri->v2->color;
      color3 = tri->v3->color;

      glColor3fv(color1);
      glVertex3fv(xyz1);
      glColor3fv(color2);
      glVertex3fv(xyz2);

      glVertex3fv(xyz2);
      glColor3fv(color3);
      glVertex3fv(xyz3);

      glVertex3fv(xyz3);
      glColor3fv(color1);
      glVertex3fv(xyz1);
    }
    for(i=0;i<niso_opaques;i++){
      isotri *tri;
      float *xyz1, *xyz2, *xyz3;
      float *color1, *color2, *color3;

      tri=iso_opaques[i];

      xyz1 = tri->v1->xyz;
      xyz2 = tri->v2->xyz;
      xyz3 = tri->v3->xyz;
      color1 = tri->v1->color;
      color2 = tri->v2->color;
      color3 = tri->v3->color;

      glColor3fv(color1);
      glVertex3fv(xyz1);
      glColor3fv(color2);
      glVertex3fv(xyz2);

      glVertex3fv(xyz2);
      glColor3fv(color3);
      glVertex3fv(xyz3);

      glVertex3fv(xyz3);
      glColor3fv(color1);
      glVertex3fv(xyz1);
    }
    glEnd();
    AntiAliasLine(OFF);
    glPopAttrib();
  }

  if((visAIso&4)==4){
    AntiAliasLine(ON);
    glPointSize(isopointsize);
    glBegin(GL_POINTS);
    for(i=0;i<niso_trans;i++){
      isotri *tri;
      float *xyz1, *xyz2, *xyz3;
      float *color1, *color2, *color3;

      tri=iso_trans[i];

      xyz1 = tri->v1->xyz;
      xyz2 = tri->v2->xyz;
      xyz3 = tri->v3->xyz;
      color1 = tri->v1->color;
      color2 = tri->v2->color;
      color3 = tri->v3->color;

      glColor3fv(color1);
      glVertex3fv(xyz1);
      glColor3fv(color2);
      glVertex3fv(xyz2);
      glColor3fv(color3);
      glVertex3fv(xyz3);
    }
    for(i=0;i<niso_opaques;i++){
      isotri *tri;
      float *xyz1, *xyz2, *xyz3;
      float *color1, *color2, *color3;

      tri=iso_opaques[i];

      xyz1 = tri->v1->xyz;
      xyz2 = tri->v2->xyz;
      xyz3 = tri->v3->xyz;
      color1 = tri->v1->color;
      color2 = tri->v2->color;
      color3 = tri->v3->color;

      glColor3fv(color1);
      glVertex3fv(xyz1);
      glColor3fv(color2);
      glVertex3fv(xyz2);
      glColor3fv(color3);
      glVertex3fv(xyz3);
    }
    glEnd();
    AntiAliasLine(OFF);
  }
}

/* ------------------ DrawIso ------------------------ */

void DrawIso(int tranflag){
  if(niso_opaques>0||niso_trans>0){
    if(use_tload_begin==1&&global_times[itimes]<global_scase.tload_begin)return;
    if(use_tload_end==1&&global_times[itimes]>global_scase.tload_end)return;
    DrawIsoOrig(tranflag);
  }
}

/* ------------------ DrawStaticIso ------------------------ */

void DrawStaticIso(const isosurface *asurface,int surfacetype,
                   int smoothnorm, int trans_flag, int data_type,
                   float line_width){
  int j,k;
  float vv1[3],vv2[3],vv3[3];
  float vv1n[3],vv2n[3],vv3n[3];
  short *norm=NULL;
  unsigned short *v1, *v2, *v3;
  unsigned short *vertices_i=NULL;
  int *triangles_i=NULL;
  int i1, i2, i3;
  short *norm1,*norm2,*norm3,*vertexnorm=NULL;
  int ntriangles;
  float xyzmin[3], xyzmaxdiff_local;
  int drawing_transparent, drawing_blockage_transparent, drawing_vent_transparent;
  int TransparentOn_flag=0;

  GetDrawingParms(&drawing_transparent, &drawing_blockage_transparent, &drawing_vent_transparent);

  xyzmin[0] = asurface->xmin;
  xyzmin[1] = asurface->ymin;
  xyzmin[2] = asurface->zmin;
  xyzmaxdiff_local = asurface->xyzmaxdiff;

  ntriangles=asurface->ntriangles/3;
  if(ntriangles==0)return;
  if(surfacetype==SURFACE_SOLID||surfacetype==-1){
    float rgbtemp[4];
    float *col;

    col = asurface->color;
    if(setbw==1){
      rgbtemp[0] = TOBW(col);
      rgbtemp[1]=rgbtemp[0];
      rgbtemp[2]=rgbtemp[0];
    }
    else{
      rgbtemp[0]=col[0];
      rgbtemp[1]=col[1];
      rgbtemp[2]=col[2];
    }

    rgbtemp[3]=asurface->color[3];
    if(data_type!=0){
      if(rgbtemp[3]<1.0&&trans_flag!=DRAW_TRANSPARENT)return;
      if(rgbtemp[3]>=1.0&&trans_flag==DRAW_TRANSPARENT)return;
    }
    if(
      trans_flag==DRAW_TRANSPARENT&&
      (
      (data_type==0&&use_transparency_data==1)||
      (data_type==1&&drawing_blockage_transparent==1)
      )
      ){
        if(rgbtemp[3]<0.99){
          drawing_transparent=1;
          drawing_blockage_transparent=1;
          TransparentOn_flag=1;
          TransparentOn();
        }
    }
    iso_specular[3] = 1.0;
    if(asurface->cullfaces==1)glDisable(GL_CULL_FACE);
    glPushAttrib(GL_LIGHTING_BIT);
    if(surfacetype==SURFACE_SOLID){
      ENABLE_LIGHTING;
      glEnable(GL_COLOR_MATERIAL);
    }
    glBegin(GL_TRIANGLES);
    if(surfacetype==SURFACE_SOLID){
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,asurface->color);
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,iso_specular);
    }

    if(TransparentOn_flag==1){
      glColor4fv(rgbtemp);
    }
    else{
      glColor3fv(rgbtemp);
    }
    vertices_i=asurface->vertices;
    triangles_i=asurface->triangles;
    norm=asurface->norm;
    vertexnorm=asurface->vertexnorm;
    for(j=0;j<ntriangles;j++){
      i1=3*triangles_i[3*j];
      i2=3*triangles_i[3*j+1];
      i3=3*triangles_i[3*j+2];
      v1=vertices_i+i1;
      v2=vertices_i+i2;
      v3=vertices_i+i3;
      for(k=0;k<3;k++){
        vv1[k]=xyzmin[k]+SCALE2FDSL(v1[k]/65535.);
        vv2[k]=xyzmin[k]+SCALE2FDSL(v2[k]/65535.);
        vv3[k]=xyzmin[k]+SCALE2FDSL(v3[k]/65535.);
      }
      if(smoothnorm==1){
        norm1 = vertexnorm+i1;
        norm2 = vertexnorm+i2;
        norm3 = vertexnorm+i3;
        glNormal3sv(norm1);
        glVertex3fv(vv1);
        glNormal3sv(norm2);
        glVertex3fv(vv2);
        glNormal3sv(norm3);
        glVertex3fv(vv3);
      }
      else{
        glNormal3sv(norm);
        glVertex3fv(vv1);
        glVertex3fv(vv2);
        glVertex3fv(vv3);
        norm += 3;
      }
    }
    glEnd();
    if(asurface->cullfaces==1)glEnable(GL_CULL_FACE);
    if(surfacetype==SURFACE_SOLID){
      glDisable(GL_COLOR_MATERIAL);
      DISABLE_LIGHTING;
    }

    glPopAttrib();
    if(TransparentOn_flag==1)TransparentOff();
  }

  if(surfacetype==SURFACE_OUTLINE){
    glPushMatrix();
    AntiAliasLine(ON);
    glLineWidth(line_width);
    glBegin(GL_LINES);
    glColor3fv(asurface->color);
    vertices_i=asurface->vertices;
    triangles_i=asurface->triangles;
    for(j=0;j<ntriangles;j++){
      i1=3*triangles_i[3*j];
      i2=3*triangles_i[3*j+1];
      i3=3*triangles_i[3*j+2];
      v1=vertices_i+i1;
      v2=vertices_i+i2;
      v3=vertices_i+i3;
      for(k=0;k<3;k++){
        vv1[k]=xyzmin[k]+SCALE2FDSL(v1[k]/65535.);
        vv2[k]=xyzmin[k]+SCALE2FDSL(v2[k]/65535.);
        vv3[k]=xyzmin[k]+SCALE2FDSL(v3[k]/65535.);
      }
      glVertex3fv(vv1);
      glVertex3fv(vv2);
      glVertex3fv(vv2);
      glVertex3fv(vv3);
      glVertex3fv(vv3);
      glVertex3fv(vv1);
    }
    glEnd();
    AntiAliasLine(OFF);
    glPopMatrix();
  }

  if(surfacetype==SURFACE_POINTS){
    int nvertices;

    glPushMatrix();
    AntiAliasLine(ON);
    glPointSize(plot3dpointsize);
    glBegin(GL_POINTS);
    glColor3fv(asurface->color);
    nvertices=asurface->nvertices;
    ntriangles=asurface->ntriangles/3;
    vertices_i=asurface->vertices;
    triangles_i=asurface->triangles;
    for(j=0;j<nvertices;j++){
      v1=vertices_i+3*j;
      for(k=0;k<3;k++){
        vv1[k]=xyzmin[k]+SCALE2FDSL(v1[k]/65535.);
      }

      glVertex3fv(vv1);
    }
    glEnd();
    AntiAliasLine(OFF);
    glPopMatrix();
  }

  if(show_iso_normal==1&&triangles_i!=NULL){
    glPushMatrix();
    AntiAliasLine(ON);
    glLineWidth(line_width);
    glBegin(GL_LINES);
    glColor3f((float)1.,(float)1.,(float)1.);
    for(j=0;j<ntriangles;j++){
      i1=3*triangles_i[3*j];
      i2=3*triangles_i[3*j+1];
      i3=3*triangles_i[3*j+2];
      v1=vertices_i+i1;
      v2=vertices_i+i2;
      v3=vertices_i+i3;
      for(k=0;k<3;k++){
        vv1[k]=xyzmin[k]+SCALE2FDSL(v1[k]/65535.);
        vv2[k]=xyzmin[k]+SCALE2FDSL(v2[k]/65535.);
        vv3[k]=xyzmin[k]+SCALE2FDSL(v3[k]/65535.);
      }

      if(smooth_iso_normal==1){
        if(vertexnorm==NULL){
          memcpy(vv1n, vv1, 3*sizeof(float));
          memcpy(vv2n, vv2, 3*sizeof(float));
          memcpy(vv3n, vv3, 3*sizeof(float));
        }
        else{
          norm1 = vertexnorm+i1;
          norm2 = vertexnorm+i2;
          norm3 = vertexnorm+i3;
          for(k=0;k<3;k++){
            vv1n[k]=vv1[k]+norm1[k]/(8.*32768.)/4.0;
            vv2n[k]=vv2[k]+norm2[k]/(8.*32768.)/4.0;
            vv3n[k]=vv3[k]+norm3[k]/(8.*32768.)/4.0;
          }
        }

        glVertex3fv(vv1);
        glVertex3fv(vv1n);
        glVertex3fv(vv2);
        glVertex3fv(vv2n);
        glVertex3fv(vv3);
        glVertex3fv(vv3n);
      }
      else{
        if(norm==NULL){
          memcpy(vv1n, vv1, 3*sizeof(float));
          memcpy(vv2n, vv2, 3*sizeof(float));
          memcpy(vv3n, vv3, 3*sizeof(float));
        }
        else{
          for(k=0;k<3;k++){
            vv1n[k]=vv1[k]+norm[k]/(8.*32768.)/4.0;
            vv2n[k]=vv2[k]+norm[k]/(8.*32768.)/4.0;
            vv3n[k]=vv3[k]+norm[k]/(8.*32768.)/4.0;
          }
        }

        glVertex3fv(vv1);
        glVertex3fv(vv1n);
        glVertex3fv(vv2);
        glVertex3fv(vv2n);
        glVertex3fv(vv3);
        glVertex3fv(vv3n);
        norm += 3;
      }
    }
    glEnd();
    AntiAliasLine(OFF);
    glPopMatrix();
  }
}

/* ------------------ GetIsoIndex ------------------------ */

int GetIsoIndex(const isodata *isoi){
  isodata *isoi2;
  int j;

  for(j = 0;j < global_scase.nisotypes;j++){
    isoi2 = global_scase.isoinfo + global_scase.isotypes[j];
    if(strcmp(isoi->surface_label.longlabel, isoi2->surface_label.longlabel) == 0)return global_scase.isotypes[j];
  }
  return -1;
}

/* ------------------ UpdateIsoTypes ------------------------ */

void UpdateIsoTypes(void){
  int i;
  isodata *isoi;

  global_scase.nisotypes = 0;
  for(i=0;i<global_scase.nisoinfo;i++){
    isoi = global_scase.isoinfo+i;
    if(GetIsoIndex(isoi)==-1)global_scase.isotypes[global_scase.nisotypes++]=i;
  }
  for(i=0;i<global_scase.nisoinfo;i++){
    isoi = global_scase.isoinfo+i;
    isoi->type= GetIsoType(isoi);
  }
}

/* ------------------ UpdateIsoType ------------------------ */

void UpdateIsoType(void){
  int i;
  isodata *isoi;


  for(i=0;i<global_scase.nisoinfo;i++){
    isoi = global_scase.isoinfo + i;
    if(isoi->loaded==0)continue;
    if(isoi->display==1&&isoi->type==iisotype)return;
  }

  for(i=0;i<global_scase.nisoinfo;i++){
    isoi = global_scase.isoinfo + i;
    if(isoi->loaded==0)continue;
    if(isoi->display==1){
      iisotype = GetIsoIndex(isoi);
      return;
    }
  }

  iisotype = -1;
}

/* ------------------ IsoCompare ------------------------ */

int IsoCompare(const void *arg1, const void *arg2){
  isodata *isoi, *isoj;

  isoi = global_scase.isoinfo + *(int *)arg1;
  isoj = global_scase.isoinfo + *(int *)arg2;

  if(strcmp(isoi->surface_label.longlabel,isoj->surface_label.longlabel)<0)return -1;
  if(strcmp(isoi->surface_label.longlabel,isoj->surface_label.longlabel)>0)return 1;
  if(isoi->blocknumber<isoj->blocknumber)return -1;
  if(isoi->blocknumber>isoj->blocknumber)return 1;
  return 0;
}

/* ------------------ UpdateIsoMenuLabels ------------------------ */

void UpdateIsoMenuLabels(void){
  int i;
  isodata *isoi;
  char label[128];

  if(global_scase.nisoinfo>0){
    FREEMEMORY(isoorderindex);
    NewMemory((void **)&isoorderindex,sizeof(int)*global_scase.nisoinfo);
    for(i=0;i<global_scase.nisoinfo;i++){
      isoorderindex[i]=i;
    }
    qsort( (int *)isoorderindex, (size_t)global_scase.nisoinfo, sizeof(int), IsoCompare);

    for(i=0;i<global_scase.nisoinfo;i++){
      isoi = global_scase.isoinfo + i;

      if(global_scase.meshescoll.nmeshes>1){
        meshdata *isomesh;

        isomesh = global_scase.meshescoll.meshinfo + isoi->blocknumber;
        sprintf(label,"%s",isomesh->label);
        STRCPY(isoi->menulabel,label);
      }
      else{
        STRCPY(isoi->menulabel, isoi->surface_label.longlabel);
      }
      if(showfiles==1){
        STRCAT(isoi->menulabel,", ");
        STRCAT(isoi->menulabel,isoi->file);
      }
    }
  }
}

/* ------------------ UpdateIsoShowLevels ------------------------ */

void UpdateIsoShowLevels(smv_case *scase, meshdata *isomesh){
  int nisolevels;
  int *showlevels;
  int i, j;
  meshdata *meshi;

  if(isomesh==NULL)return;

  nisolevels=isomesh->nisolevels;
  showlevels=isomesh->showlevels;

  for(j=0;j<scase->meshescoll.nmeshes;j++){
    meshi = scase->meshescoll.meshinfo+j;
    if(meshi->isofilenum==-1)continue;
    for(i=0;i<nisolevels;i++){
      if(i<meshi->nisolevels)meshi->showlevels[i]=showlevels[i];
    }
  }
}

/* ------------------ SetIsoLabels ------------------------ */

void SetIsoLabels(float smin, float smax,
                    isodata *sd, int *errorcode){
  int isotype;
  boundsdata *sb;

  isotype= GetIsoTType(sd);
  sb = isobounds + isotype;
  sb->label=&(sd->color_label);

  *errorcode=0;
  PRINTF("setting up iso labels \n");
  GetColorbarLabels(smin,smax,global_scase.nrgb,sb->colorlabels,sb->levels256);
}

/* ------------------ CompareIsoTriangles ------------------------ */

int CompareIsoTriangles(const void *arg1, const void *arg2){
  isotri *trii, *trij;
  float disti, distj;


  trii = *(isotri **)arg1;
  trij = *(isotri **)arg2;

  disti = trii->v1->distance+trii->v2->distance+trii->v3->distance;
  distj = trij->v1->distance+trij->v2->distance+trij->v3->distance;

  if(disti<distj)return  1;
  if(disti>distj)return -1;
  return 0;
}

/* ------------------ SortIsoTriangles ------------------------ */

void SortIsoTriangles(float *mm){
  int itri;
  int newflag;
  int dosort=0;

  if(niso_trans==0)return;
  newflag=1-iso_trans[0]->v1->flag;
  for(itri=0;itri<niso_trans;itri++){
    isotri *tri;
    float xyzeye[3];
    float *xyz;
    isovert *v1, *v2, *v3;
    float dist1, dist2;
    isotri *trim1;

    tri = iso_trans[itri];
    v1 = tri->v1;
    v2 = tri->v2;
    v3 = tri->v3;
    if(v1->flag!=newflag){
      v1->flag=newflag;
      xyz = v1->xyz;
      xyzeye[0] = mm[0]*xyz[0] + mm[4]*xyz[1] +  mm[8]*xyz[2] + mm[12];
      xyzeye[1] = mm[1]*xyz[0] + mm[5]*xyz[1] +  mm[9]*xyz[2] + mm[13];
      xyzeye[2] = mm[2]*xyz[0] + mm[6]*xyz[1] + mm[10]*xyz[2] + mm[14];
      xyzeye[0]/=mscale[0];
      xyzeye[1]/=mscale[1];
      xyzeye[2]/=mscale[2];
      v1->distance=xyzeye[0]*xyzeye[0]+xyzeye[1]*xyzeye[1]+xyzeye[2]*xyzeye[2];
    }
    if(v2->flag!=newflag){
      v2->flag=newflag;
      xyz = v2->xyz;
      xyzeye[0] = mm[0]*xyz[0] + mm[4]*xyz[1] +  mm[8]*xyz[2] + mm[12];
      xyzeye[1] = mm[1]*xyz[0] + mm[5]*xyz[1] +  mm[9]*xyz[2] + mm[13];
      xyzeye[2] = mm[2]*xyz[0] + mm[6]*xyz[1] + mm[10]*xyz[2] + mm[14];
      xyzeye[0]/=mscale[0];
      xyzeye[1]/=mscale[1];
      xyzeye[2]/=mscale[2];
      v2->distance=xyzeye[0]*xyzeye[0]+xyzeye[1]*xyzeye[1]+xyzeye[2]*xyzeye[2];
    }
    if(v3->flag!=newflag){
      v3->flag=newflag;
      xyz = v3->xyz;
      xyzeye[0] = mm[0]*xyz[0] + mm[4]*xyz[1] +  mm[8]*xyz[2] + mm[12];
      xyzeye[1] = mm[1]*xyz[0] + mm[5]*xyz[1] +  mm[9]*xyz[2] + mm[13];
      xyzeye[2] = mm[2]*xyz[0] + mm[6]*xyz[1] + mm[10]*xyz[2] + mm[14];
      xyzeye[0]/=mscale[0];
      xyzeye[1]/=mscale[1];
      xyzeye[2]/=mscale[2];
      v3->distance=xyzeye[0]*xyzeye[0]+xyzeye[1]*xyzeye[1]+xyzeye[2]*xyzeye[2];
    }
    dist1=(v1->distance+v2->distance+v3->distance);
    trim1 = iso_trans[itri-1];
    dist2=trim1->v1->distance+trim1->v2->distance+trim1->v3->distance;

    if(itri>0&&dosort==0&&dist1>dist2)dosort=1;
  }
  if(dosort==1)qsort((isotri **)iso_trans,(size_t)niso_trans,sizeof(isotri **), CompareIsoTriangles);
}

/* ------------------ UpdateIsoTriangles ------------------------ */

void UpdateIsoTriangles(int flag){
  int itri;
  isosurface *asurfi;
  isotri **iso_trans_tmp,**iso_opaques_tmp;
  int *showlevels;
  meshdata *meshi;
  float *colorptr;
  isosurface *asurface;
  int ntris;

  if(loaded_isomesh==NULL||loaded_isomesh->isofilenum==-1)return;

  if(iso_trans_list==NULL||iso_opaques_list==NULL){
    int iitime;

    niso_timesteps=loaded_isomesh->niso_times;
    if(iso_trans_list==NULL){
      int i;

      NewMemory((void **)&niso_trans_list,niso_timesteps*sizeof(int));
      NewMemory((void **)&iso_trans_list,niso_timesteps*sizeof(isotri **));
      for(i=0;i<niso_timesteps;i++){
        iso_trans_list[i]=NULL;
      }
    }
    if(iso_opaques_list==NULL){
      int i;

      NewMemory((void **)&niso_opaques_list,niso_timesteps*sizeof(int));
      NewMemory((void **)&iso_opaques_list,niso_timesteps*sizeof(isotri **));
      for(i=0;i<niso_timesteps;i++){
        iso_opaques_list[i]=NULL;
      }
    }
    for(iitime=0;iitime<niso_timesteps;iitime++){
      int i;

      ntris=0;
      for(i=0;i<global_scase.nisoinfo;i++){
        isodata *isoi;
        int ilev;

        isoi = global_scase.isoinfo+i;
        if(isoi->geomflag==1||isoi->loaded==0||isoi->display==0)continue;

        meshi = global_scase.meshescoll.meshinfo + isoi->blocknumber;
        asurface = meshi->animatedsurfaces + iitime*meshi->nisolevels;
        for(ilev=0;ilev<meshi->nisolevels;ilev++){
          asurfi = asurface + ilev;
          ntris+=asurfi->niso_triangles;
        }
      }
      if(ntris>0){
        NewMemory((void **)&iso_trans,ntris*sizeof(isotri *));
        iso_trans_list[iitime]=iso_trans;
        NewMemory((void **)&iso_opaques,ntris*sizeof(isotri *));
        iso_opaques_list[iitime]=iso_opaques;
      }
    }
    flag=1;
  }
  if(flag==1){
    int iitime;

    for(iitime=0;iitime<niso_timesteps;iitime++){
      niso_trans_list[iitime]=-1;
      niso_opaques_list[iitime]=-1;
    }
  }

  iso_trans=iso_trans_list[loaded_isomesh->iso_itime];
  iso_opaques=iso_opaques_list[loaded_isomesh->iso_itime];
  niso_trans=niso_trans_list[loaded_isomesh->iso_itime];
  niso_opaques=niso_opaques_list[loaded_isomesh->iso_itime];

  if(niso_trans==-1||niso_opaques==-1){
    int i;

    iso_trans_tmp=iso_trans;
    iso_opaques_tmp=iso_opaques;
    niso_trans=0;
    niso_opaques=0;
    for(i=0;i<global_scase.nisoinfo;i++){
      isodata *isoi;

      isoi = global_scase.isoinfo+i;
      if(isoi->geomflag==1||isoi->loaded==0||isoi->display==0)continue;

      CheckMemory;
      meshi = global_scase.meshescoll.meshinfo + isoi->blocknumber;
      asurface = meshi->animatedsurfaces + meshi->iso_itime*meshi->nisolevels;
      showlevels=meshi->showlevels;

      if(transparent_state==ALL_TRANSPARENT){
        int ilev;

        for(ilev=0;ilev<meshi->nisolevels;ilev++){
          if(showlevels[ilev]==0)continue;
          asurfi = asurface + ilev;
          if(asurfi->niso_triangles>0){
            niso_trans += asurfi->niso_triangles;
            for(itri=0;itri<asurfi->niso_triangles;itri++){
              *iso_trans_tmp++=asurfi->iso_triangles+itri;
            }
            colorptr=isoi->colorlevels[ilev];
            colorptr[3]=transparent_level;
          }
        }
      }
      else if(transparent_state==MIN_SOLID){
        int ilev;

        for(ilev=0;ilev<1;ilev++){
          if(showlevels[ilev]==0)continue;
          asurfi = asurface + ilev;
          if(asurfi->niso_triangles>0){
            niso_opaques += asurfi->niso_triangles;
            for(itri=0;itri<asurfi->niso_triangles;itri++){
              *iso_opaques_tmp++=asurfi->iso_triangles+itri;
            }
            colorptr=isoi->colorlevels[ilev];
            colorptr[3]=1.0;
          }
        }
        for(ilev=1;ilev<meshi->nisolevels;ilev++){
          if(showlevels[ilev]==0)continue;
          asurfi = asurface + ilev;
          if(asurfi->niso_triangles>0){
            niso_trans += asurfi->niso_triangles;
            for(itri=0;itri<asurfi->niso_triangles;itri++){
              *iso_trans_tmp++=asurfi->iso_triangles+itri;
            }
            colorptr=isoi->colorlevels[ilev];
            colorptr[3]=transparent_level;
          }
        }
      }
      else if(transparent_state==MAX_SOLID){
        int ilev;

        for(ilev=0;ilev<meshi->nisolevels-1;ilev++){
          if(showlevels[ilev]==0)continue;
          asurfi = asurface + ilev;
          if(asurfi->niso_triangles>0){
            niso_trans += asurfi->niso_triangles;
            for(itri=0;itri<asurfi->niso_triangles;itri++){
              *iso_trans_tmp++=asurfi->iso_triangles+itri;
            }
            colorptr=isoi->colorlevels[ilev];
            colorptr[3]=transparent_level;
          }
        }
        for(ilev=meshi->nisolevels-1;ilev<meshi->nisolevels;ilev++){
          if(showlevels[ilev]==0)continue;
          asurfi = asurface + ilev;
          if(asurfi->niso_triangles>0){
            niso_opaques += asurfi->niso_triangles;
            for(itri=0;itri<asurfi->niso_triangles;itri++){
              *iso_opaques_tmp++=asurfi->iso_triangles+itri;
            }
            colorptr=isoi->colorlevels[ilev];
            colorptr[3]=1.0;
          }
        }
      }
      else if(transparent_state==ALL_SOLID){
        int ilev;

        for(ilev=0;ilev<meshi->nisolevels;ilev++){
          CheckMemory;
          if(showlevels[ilev]==0)continue;
          asurfi = asurface + ilev;
          if(asurfi->niso_triangles>0){
            niso_opaques += asurfi->niso_triangles;
            for(itri=0;itri<asurfi->niso_triangles;itri++){
              *iso_opaques_tmp++=asurfi->iso_triangles+itri;
            }
            colorptr=isoi->colorlevels[ilev];
            colorptr[3]=1.0;
          }
        }
      }
    }
  }

  if(sort_iso_triangles==1&&niso_trans>0){
    SortIsoTriangles(modelview_scratch);
  }
  niso_trans_list[loaded_isomesh->iso_itime]=niso_trans;
  niso_opaques_list[loaded_isomesh->iso_itime]=niso_opaques;

  CheckMemory;
}

/* ------------------ GetLoadedIsoMesh ------------------------ */

meshdata *GetLoadedIsoMesh(void){
  meshdata *return_mesh;
  int i,nsteps=-1;

  if(global_scase.isoinfo==NULL)return NULL;
  return_mesh=NULL;
  for(i=0;i<global_scase.nisoinfo;i++){
    meshdata *mesh2;
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded==0)continue;
    mesh2 = global_scase.meshescoll.meshinfo + isoi->blocknumber;
    if(nsteps==-1||mesh2->niso_times<nsteps){
      return_mesh = mesh2;
      nsteps=mesh2->niso_times;
    }
  }
  return return_mesh;
}

/* ------------------ UpdateIsoColors ------------------------ */

void UpdateIsoColors(void){
  int i;

  for(i = 0; i < MAX_ISO_COLORS; i++){
    surfdata *surfi;

    surfi = global_scase.surfcoll.surfinfo + i + global_scase.surfcoll.nsurfinfo + 1;
    surfi->transparent_level = iso_colors[4*i+3];
    if(setbwdata == 1){
      surfi->color = iso_colorsbw + 4*i;
    }
    else{
      surfi->color = iso_colors + 4*i;
    }
    surfi->iso_level= i + 1;
  }
  CheckMemory;
}
