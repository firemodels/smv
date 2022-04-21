#include "options.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "glui_motion.h"

#ifdef pp_LUA
#include "lua_api.h"
#endif
#include "IOscript.h"

int nrenderonce=0;

/* ------------------ GetGridIndex ------------------------ */

int GetGridIndex(float val, int dir, float *plotxyz, int nplotxyz){
  int i;
  int return_index=0;
  float min_val,vali;

  switch(dir){
    case XDIR:
      val=NORMALIZE_X(val);
      break;
    case YDIR:
      val=NORMALIZE_Y(val);
      break;
    case ZDIR:
      val=NORMALIZE_Z(val);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }

  if(plotxyz!=NULL){
    min_val=ABS(val-plotxyz[0]);
    return_index=0;
    for(i=1;i<nplotxyz;i++){
      vali = ABS(val-plotxyz[i]);
      if(vali<min_val){
        return_index=i;
        min_val = vali;
      }
    }
  }
  return return_index;
}

/* ------------------ NexteXIndex  ------------------------ */

void NextXIndex(int inc,int flag){
  int i,j,first;

  first=1;
  if(flag==1)inc=-1;
  if(flag==-1)inc=1;
  for(j=0;j<nplotx_all;j++){
    if(first==1){
      first=0;
      if(flag==1)iplotx_all=nplotx_all-1;
      if(flag==-1)iplotx_all=0;
      if(flag==0)iplotx_all+=inc;
    }
    else{
      iplotx_all+=inc;
    }
    if(iplotx_all<0)iplotx_all=nplotx_all-1;
    if(iplotx_all>nplotx_all-1)iplotx_all=0;
    if(visGrid!=NOGRID_NOPROBE)return;
    if(plotstate==DYNAMIC_PLOTS){
      for(i=0;i<nsliceinfo;i++){
        slicedata *slicei;
        meshdata *meshi;

        slicei = sliceinfo + i;
        if(slicei->loaded==0||slicei->display==0)continue;
        meshi = meshinfo + slicei->blocknumber;
        if(meshi->iplotx_all[iplotx_all]!=-1)return;
      }
      for(i=0;i<nvsliceinfo;i++){
        vslicedata *vslicei;
        meshdata *meshi;

        vslicei = vsliceinfo + i;
        if(vslicei->loaded==0||vslicei->display==0)continue;
        meshi = meshinfo + vslicei->val->blocknumber;
        if(meshi->iploty_all[iploty_all]!=-1)return;
      }
    }
    else{
      for(i=0;i<nplot3dinfo;i++){
        plot3ddata *plot3di;
        meshdata *meshi;

        plot3di = plot3dinfo + i;
        if(plot3di->loaded==0||plot3di->display==0)continue;
        meshi = meshinfo + plot3di->blocknumber;
        if(meshi->iplotx_all[iplotx_all]!=-1)return;
      }
    }
  }
}

/* ------------------ next_yindex ------------------------ */

void NextYIndex(int inc,int flag){
  int i,j,first;

  first=1;
  if(flag==1)inc=-1;
  if(flag==-1)inc=1;
  for(j=0;j<nploty_all;j++){
    if(first==1){
      first=0;
      if(flag==1)iploty_all=nploty_all-1;
      if(flag==-1)iploty_all=0;
      if(flag==0)iploty_all+=inc;
    }
    else{
      iploty_all+=inc;
    }
    if(iploty_all<0)iploty_all=nploty_all-1;
    if(iploty_all>nploty_all-1)iploty_all=0;
    if(visGrid!=NOGRID_NOPROBE)return;
    if(plotstate==DYNAMIC_PLOTS){
      for(i=0;i<nsliceinfo;i++){
        slicedata *slicei;
        meshdata *meshi;

        slicei = sliceinfo + i;
        if(slicei->loaded==0||slicei->display==0)continue;
        meshi = meshinfo + slicei->blocknumber;
        if(meshi->iploty_all[iploty_all]!=-1)return;
      }
      for(i=0;i<nvsliceinfo;i++){
        vslicedata *vslicei;
        meshdata *meshi;

        vslicei = vsliceinfo + i;
        if(vslicei->loaded==0||vslicei->display==0)continue;
        meshi = meshinfo + vslicei->val->blocknumber;
        if(meshi->iploty_all[iploty_all]!=-1)return;
      }
    }
    else{
      for(i=0;i<nplot3dinfo;i++){
        plot3ddata *plot3di;
        meshdata *meshi;

        plot3di = plot3dinfo + i;
        if(plot3di->loaded==0||plot3di->display==0)continue;
        meshi = meshinfo + plot3di->blocknumber;
        if(meshi->iploty_all[iploty_all]!=-1)return;
      }
    }
  }
}

/* ------------------ next_zindex ------------------------ */

void NextZIndex(int inc,int flag){
  int i,j,first;

  first=1;
  if(flag==1)inc=-1;
  if(flag==-1)inc=1;
  for(j=0;j<nplotz_all;j++){
    if(first==1){
      first=0;
      if(flag==1)iplotz_all=nplotz_all-1;
      if(flag==-1)iplotz_all=0;
      if(flag==0)iplotz_all+=inc;
    }
    else{
      iplotz_all+=inc;
    }
    if(iplotz_all<0)iplotz_all=nplotz_all-1;
    if(iplotz_all>nplotz_all-1)iplotz_all=0;
    if(visGrid!=NOGRID_NOPROBE)return;
    if(plotstate==DYNAMIC_PLOTS){
      for(i=0;i<nsliceinfo;i++){
        slicedata *slicei;
        meshdata *meshi;

        slicei = sliceinfo + i;
        if(slicei->loaded==0||slicei->display==0)continue;
        meshi = meshinfo + slicei->blocknumber;
        if(meshi->iplotz_all[iplotz_all]!=-1)return;
      }
      for(i=0;i<nvsliceinfo;i++){
        vslicedata *vslicei;
        meshdata *meshi;

        vslicei = vsliceinfo + i;
        if(vslicei->loaded==0||vslicei->display==0)continue;
        meshi = meshinfo + vslicei->val->blocknumber;
        if(meshi->iploty_all[iploty_all]!=-1)return;
      }
    }
    else{
      for(i=0;i<nplot3dinfo;i++){
        plot3ddata *plot3di;
        meshdata *meshi;

        plot3di = plot3dinfo + i;
        if(plot3di->loaded==0||plot3di->display==0)continue;
        meshi = meshinfo + plot3di->blocknumber;
        if(meshi->iplotz_all[iplotz_all]!=-1)return;
      }
    }
  }
}

/* ------------------ WindowStatus ------------------------ */

void WindowStatus(int state){
  PRINTF("state=%i\n",state);
  switch(state){
  case GLUT_HIDDEN:
  case GLUT_FULLY_COVERED:
    break;
  case GLUT_FULLY_RETAINED:
  case GLUT_PARTIALLY_RETAINED:
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}


/* ------------------ MouseEditColorbar ------------------------ */

void MouseEditColorbar(int x, int y){
  int val;
  int mouse_x, mouse_y;
  GLubyte r, g, b;
  colorbardata *cbi;

  if(show_firecolormap==0){
    cbi = colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbarinfo+fire_colorbar_index;
  }

  mouse_x = x;
  mouse_y = screenHeight-y;

  glDisable(GL_BLEND);
  glShadeModel(GL_FLAT);
  DISABLE_LIGHTING;
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);

  ShowScene(SELECTOBJECT, VIEW_CENTER, 0, 0, 0, NULL);
  glReadBuffer(GL_BACK);
  glReadPixels(mouse_x, mouse_y, 1, 1, GL_RED,   GL_UNSIGNED_BYTE, &r);
  glReadPixels(mouse_x, mouse_y, 1, 1, GL_GREEN, GL_UNSIGNED_BYTE, &g);
  glReadPixels(mouse_x, mouse_y, 1, 1, GL_BLUE,  GL_UNSIGNED_BYTE, &b);

  r = r>>nredshift;
  g = g>>ngreenshift;
  b = b>>nblueshift;

  val = (r<<(nbluebits+ngreenbits))|(g<<nbluebits)|b;
  colorbaredit_drag = 0;
  if(val>0&&val<=cbi->nnodes){

    /* need to start colors at 1 so that black (color 0,0,0) is not interpreted as a blockage */

    val--;
    colorbaredit_drag = 1;
    colorbarpoint = val;
    ColorbarCB(COLORBAR_SET);
  }
  glEnable(GL_BLEND);
  ENABLE_LIGHTING;
  glShadeModel(GL_SMOOTH);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &block_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, block_specular2);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DITHER);
  glEnable(GL_TEXTURE_1D);
  glEnable(GL_TEXTURE_2D);
}

/* ------------------ MouseEditTour ------------------------ */

void MouseEditTour(int x, int y){
  int val, val1;
  int mouse_x, mouse_y;
  GLubyte r, g, b;

  mouse_x=x; mouse_y=screenHeight-y;
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  DISABLE_LIGHTING;
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);

  ShowScene(SELECTOBJECT,VIEW_CENTER,0,0,0,NULL);
  glReadBuffer(GL_BACK);
  glReadPixels(mouse_x,mouse_y,1,1,GL_RED,   GL_UNSIGNED_BYTE, &r);
  glReadPixels(mouse_x,mouse_y,1,1,GL_GREEN, GL_UNSIGNED_BYTE, &g);
  glReadPixels(mouse_x,mouse_y,1,1,GL_BLUE,  GL_UNSIGNED_BYTE, &b);

  r = r>>nredshift;
  g = g>>ngreenshift;
  b = b>>nblueshift;

  val1 = (r << (nbluebits+ngreenbits)) | (g << nbluebits) | b;
  val = val1;
  if(val!=0&&itourknots>=0&&itourknots<ntourknots&&tourknotskeylist!=NULL){
    tourknotskeylist[itourknots]->selected=0;
  }
  tour_drag=0;
  if(val>0&&val<=ntourknots){

  /* need to start colors at 1 so that black (color 0,0,0) is not interpreted as a blockage */

    val--;
    itourknots=val;
    if(tourknotskeylist!=NULL){
      NewSelect(tourknotskeylist[itourknots]);
      selected_tour=tourknotstourlist[itourknots];
    }
    else{
      selected_tour=NULL;
      itourknots=-1;
    }
    SetGluiTourKeyframe();
    UpdateTourControls();
    tour_drag=1;
  }
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  ENABLE_LIGHTING;
}

/* ------------------ MouseEditBlockage ------------------------ */

void MouseEditBlockage(int x, int y){
  int val, val1;
  int mouse_x, mouse_y;
  GLubyte r, g, b;

  mouse_x=x; mouse_y=screenHeight-y;
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  DISABLE_LIGHTING;
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);

  ShowScene(SELECTOBJECT,VIEW_CENTER,0,0,0,NULL);
  glReadBuffer(GL_BACK);
  glReadPixels(mouse_x,mouse_y,1,1,GL_RED,   GL_UNSIGNED_BYTE, &r);
  glReadPixels(mouse_x,mouse_y,1,1,GL_GREEN, GL_UNSIGNED_BYTE, &g);
  glReadPixels(mouse_x,mouse_y,1,1,GL_BLUE,  GL_UNSIGNED_BYTE, &b);

  r = r>>nredshift;
  g = g>>ngreenshift;
  b = b>>nblueshift;

  val1 = (r << (nbluebits+ngreenbits)) | (g << nbluebits) | b;
  val = val1;

  if(val>0&&val<=ntotalfaces){
    meshdata *meshi;
    selectdata *sd;
    int i;

    /* need to start colors at 1 so that black (color 0,0,0) is not
                interpreted as a blockage */
    val--;
    sd = selectfaceinfo + val;
    highlight_block=sd->blockage;
    highlight_mesh=sd->mesh;
    meshi = meshinfo + highlight_mesh;
    UpdateCurrentMesh(meshi);
    bchighlight_old=bchighlight;
    bchighlight = meshi->blockageinfoptrs[highlight_block];
    for(i=0;i<6;i++){
      surface_indices[i]=inv_sorted_surfidlist[bchighlight->surf_index[i]];
      surface_indices_bak[i]=inv_sorted_surfidlist[bchighlight->surf_index[i]];
    }

    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    ENABLE_LIGHTING;

    switch(sd->dir){
      case DOWN_X:
      case UP_X:
        xyz_dir=XDIR;
        break;
      case DOWN_Y:
      case UP_Y:
        xyz_dir=YDIR;
        break;
      case DOWN_Z:
      case UP_Z:
        xyz_dir=ZDIR;
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    switch(sd->dir){
      case DOWN_X:
      case DOWN_Y:
      case DOWN_Z:
        which_face=0;
        break;
      case UP_X:
      case UP_Y:
      case UP_Z:
        which_face=1;
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    UpdateBlockVals(SELECT_BLOCKS);
  }
}

/* ------------------ MouseSelectDevice ------------------------ */

void MouseSelectDevice(int x, int y){
  int val;
  int mouse_x, mouse_y;
  GLubyte r, g, b;

  mouse_x=x; mouse_y=screenHeight-y;
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  DISABLE_LIGHTING;
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);

  ShowScene(SELECTOBJECT,VIEW_CENTER,0,0,0,NULL);
  glReadBuffer(GL_BACK);
  glReadPixels(mouse_x,mouse_y,1,1,GL_RED,   GL_UNSIGNED_BYTE, &r);
  glReadPixels(mouse_x,mouse_y,1,1,GL_GREEN, GL_UNSIGNED_BYTE, &g);
  glReadPixels(mouse_x,mouse_y,1,1,GL_BLUE,  GL_UNSIGNED_BYTE, &b);

  r = r>>nredshift;
  g = g>>ngreenshift;
  b = b>>nblueshift;

  val = (r << (nbluebits+ngreenbits)) | (g << nbluebits) | b;

  if(val>0&&val<ndeviceinfo){
    devicedata *devicei;
    float *xyz;

    devicei = deviceinfo+val-1;
    devicei->selected = 1-devicei->selected;
    xyz = devicei->xyz;

    if(devicei->labelptr!=NULL&&strcmp(devicei->labelptr, "null")!=0){
      PRINTF("Selected Device: index=%i location:(%f,%f,%f) label:%s\n", val, xyz[0], xyz[1], xyz[2], devicei->labelptr);
    }
    else{
      PRINTF("Selected Device: index=%i location:(%f,%f,%f)\n", val, xyz[0], xyz[1], xyz[2]);
    }
  }
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  ENABLE_LIGHTING;
}

/* ------------------ MouseSelectAvatar ------------------------ */

void MouseSelectAvatar(int x, int y){
  int val;
  int mouse_x, mouse_y;
  GLubyte r, g, b;

  mouse_x=x; mouse_y=screenHeight-y;
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  DISABLE_LIGHTING;
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);

  ShowScene(SELECTOBJECT,VIEW_CENTER,0,0,0,NULL);
  glReadBuffer(GL_BACK);
  glReadPixels(mouse_x,mouse_y,1,1,GL_RED,   GL_UNSIGNED_BYTE, &r);
  glReadPixels(mouse_x,mouse_y,1,1,GL_GREEN, GL_UNSIGNED_BYTE, &g);
  glReadPixels(mouse_x,mouse_y,1,1,GL_BLUE,  GL_UNSIGNED_BYTE, &b);

  r = r>>nredshift;
  g = g>>ngreenshift;
  b = b>>nblueshift;

  val = (r << (nbluebits+ngreenbits)) | (g << nbluebits) | b;

  if(val>0){
    selected_avatar_tag=val;
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    ENABLE_LIGHTING;
  }
}

/* ------------------ MouseSelectGeom ------------------------ */

void MouseSelectGeom(int x, int y){
  int val;
  int mouse_x, mouse_y;
  GLubyte r, g, b;

  mouse_x = x;
  mouse_y = screenHeight-y;

  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  DISABLE_LIGHTING;
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glShadeModel(GL_FLAT);

  ShowScene(SELECTOBJECT, VIEW_CENTER, 0, 0, 0, NULL);
  glReadBuffer(GL_BACK);
  glReadPixels(mouse_x, mouse_y, 1, 1, GL_RED,   GL_UNSIGNED_BYTE, &r);
  glReadPixels(mouse_x, mouse_y, 1, 1, GL_GREEN, GL_UNSIGNED_BYTE, &g);
  glReadPixels(mouse_x, mouse_y, 1, 1, GL_BLUE,  GL_UNSIGNED_BYTE, &b);

  r = r>>nredshift;
  g = g>>ngreenshift;
  b = b>>nblueshift;

  val = (r<<(nbluebits+ngreenbits))|(g<<nbluebits)|b;

  if(val>0){
    geomdata *geomi;
    geomlistdata *geomlisti;

    geomi = geominfoptrs[0];
    geomlisti = geomi->geomlistinfo-1;

    switch (select_geom){
    case GEOM_PROP_VERTEX1:
      selected_geom_vertex1 = val-1;
      break;
    case GEOM_PROP_VERTEX2:
      selected_geom_vertex2 = val-1;
      break;
    case GEOM_PROP_TRIANGLE:
    case GEOM_PROP_SURF:
      selected_geom_triangle = val-1;
      break;
    default:
      ASSERT(FFALSE);
      break;
    }

    switch(select_geom){
    case GEOM_PROP_VERTEX1:
    case GEOM_PROP_VERTEX2:
    {
      float *xyz1, *xyz2;

      xyz1 = NULL;
      if(selected_geom_vertex1>=0){
        vertdata *verti;

        verti = geomlisti->verts+selected_geom_vertex1;
        xyz1 = verti->xyz;
      }
      xyz2 = NULL;
      if(selected_geom_vertex2>=0){
        vertdata *verti;

        verti = geomlisti->verts+selected_geom_vertex2;
        xyz2 = verti->xyz;
      }
      UpdateVertexInfo(xyz1, xyz2);
    }
      break;
    case GEOM_PROP_TRIANGLE:
    case GEOM_PROP_SURF:
      if(geomlisti->ntriangles>0){
        surfdata *tri_surf;
        tridata *trii;

        trii = geomlisti->triangles+selected_geom_triangle;
        tri_surf = trii->geomsurf;
        UpdateTriangleInfo(tri_surf, trii->area);
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    ENABLE_LIGHTING;
  }
}

/* ------------------ CheckTimeBound ------------------------ */

void CheckTimeBound(void){
  int i;

  if((timebar_drag==0&&itimes>nglobal_times-1)||(timebar_drag==1&&itimes<0)){
    if(timebar_drag==0){
      if(itimes>nglobal_times-1)itime_cycle++;
      if(itimes<0)itime_cycle--;
    }
    izone = 0;
    itimes=first_frame_index;
    if(render_status==RENDER_ON){
      RenderMenu(RenderCancel);
      if(current_script_command!=NULL&&current_script_command->command!=SCRIPT_LOADSLICERENDER){
        current_script_command->exit=1;
      }
    }
    frame_index=first_frame_index;
    for(i=0;i<nsliceinfo;i++){
      slicedata *sd;

      sd=sliceinfo+i;
      sd->itime=0;
    }
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;

      meshi=meshinfo+i;

      meshi->patch_itime=0;
    }
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;

      meshi=meshinfo+i;
      if(meshi->iso_times==NULL)continue;
      meshi->iso_itime=0;
    }
  }
  if((timebar_drag==0&&itimes<0)||(timebar_drag==1&&itimes>nglobal_times-1)){
    izone=nzone_times-1;
    itimes=nglobal_times-1;
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti=partinfo+i;
      parti->itime=parti->ntimes-1;
    }
    for(i=0;i<nsliceinfo;i++){
      slicedata *sd;

      sd=sliceinfo+i;
      sd->itime=sd->ntimes-1;
      if(sd->volslice==1)sd->itime--;
    }
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;

      meshi=meshinfo+i;
      meshi->patch_itime=meshi->npatch_times-1;
    }
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;

      meshi=meshinfo+i;
      if(meshi->iso_times==NULL)continue;
      meshi->iso_itime=meshi->niso_times-1;
    }
  }
  /* set blockage visibility */

  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi=meshinfo+i;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;

      bc=meshi->blockageinfoptrs[j];
      if(bc->showtimelist==NULL)continue;
      bc->show=bc->showtimelist[itimes];
    }
  }
}

#define CB_SELECT_CONTINUE -2
#define CB_SELECT_STOP -1
/* ------------------ GetColorbarIndex ------------------------ */

int GetColorbarIndex(int flag, int x, int y){

  if(flag==0||(vcolorbar_left_pos<=x&&x<=vcolorbar_right_pos)){
      y = screenHeight - y;
      if(vcolorbar_down_pos<=y&&y<=vcolorbar_top_pos){
        int index;
        index = CLAMP(255*(float)(y-vcolorbar_down_pos)/(float)(vcolorbar_top_pos - vcolorbar_down_pos),0,255);
        return index;
      }
      else{
        return CB_SELECT_STOP;
      }
  }
  return CB_SELECT_CONTINUE;
}

#define GLUTGETMODIFIERS GlutGetModifiersNew
int GlutGetModifiersNew(void){
  int modifier;

  switch(alt_ctrl_key_state){
  case KEY_NONE:
    modifier = glutGetModifiers();
    break;
  case KEY_CTRL:
    modifier = GLUT_ACTIVE_CTRL;
    break;
  case KEY_ALT:
    modifier = GLUT_ACTIVE_ALT;
    break;
  default:
    modifier = glutGetModifiers();
    ASSERT(FFALSE);
    break;
  }
#ifdef _DEBUG
  printf("modifier=%i\n", modifier);
#endif
  return modifier;
}

/* ------------------ ColorbarClick ------------------------ */

int ColorbarClick(int x, int y){
  int colorbar_index;

  colorbar_index = GetColorbarIndex(1,x,y);
  if(colorbar_index>=0){
    int state;

    colorbar_select_index=colorbar_index;
    state=GLUTGETMODIFIERS();
    if(state==GLUT_ACTIVE_CTRL&&current_colorbar!=NULL&&current_colorbar->nsplits==1){
      colorbar_splitdrag=1;
    }
    else{
      colorbar_drag=1;
      UpdateRGBColors(colorbar_index);
    }
    return 1;
  }
  else if(colorbar_index==CB_SELECT_CONTINUE){
    return 0;
  }
  else if(colorbar_index==CB_SELECT_STOP){
    colorbar_drag=0;
    UpdateRGBColors(COLORBAR_INDEX_NONE);
  }
  else{
    ASSERT(FFALSE);
  }
  return 0;
}

/* ------------------ GetTimeFrame ------------------------ */

int GetTimeFrame(int xm){
  int timebar_right_pos;
  int timebar_left_pos;
  int iframe;

  timebar_left_pos = VP_timebar.left+timebar_left_width;
  timebar_right_pos = VP_timebar.right-timebar_right_width;

  iframe = 0;
  if(global_times!=NULL&&timebar_right_pos>timebar_left_pos){
    float time, factor;

    factor = (float)(xm-timebar_left_pos)/(float)(timebar_right_pos-timebar_left_pos);
    factor = CLAMP(factor, 0.0, 1.0);
    time = global_times[0]*(1.0-factor)+global_times[nglobal_times-1]*factor;
    if(time<=global_times[0]){
      iframe = 0;
    }
    else if(time>=global_times[nglobal_times-1]){
      iframe = nglobal_times-1;
    }
    else{
      iframe = GetInterval(time, global_times, nglobal_times);
      iframe = CLAMP(iframe, 0, nglobal_times-1);
    }
  }
  return iframe;
}

/* ------------------ TimebarClick ------------------------ */

int TimebarClick(int xm, int ym){
  if(screenHeight-ym<titlesafe_offset+VP_timebar.height&&nglobal_times>0){
//    PRINTF("ngt=%i xl=%i x=%i xr=%i\n",nglobal_times,timebar_left_pos,x,timebar_right_pos);
    itimes = GetTimeFrame(xm);
    CheckTimeBound();
    timebar_drag=1;
    stept=0;
    last_time_paused = 1;
    IdleCB();
    return 1;
  }
  return 0;
}

/* ------------------ TimebarDrag ------------------------ */

void TimebarDrag(int xm){
  if(nglobal_times>0){
    itimes = GetTimeFrame(xm);
    CheckTimeBound();
    timebar_drag = 1;
  }
  IdleCB();
}

/* ------------------ UpdateMouseInfo ------------------------ */

#define MOUSE_DOWN 0
#define MOUSE_UP 1
#define MOUSE_MOTION 2

void UpdateMouseInfo(int flag, int xm, int ym){
  float thistime_local;
  float dx, dy;
  int maxWH;
  float quat_temp[4];
  float delta_angle,delta_angle2,delta_distance;
  float xymax;
  mousedata *mi;

  mi = &mouseinfo;

  ym = screenHeight - ym; // flip y

  maxWH = MAX(screenWidth,screenHeight);
  xymax = 0.3*(float)MIN(screenWidth,screenHeight)/(float)maxWH;
  switch(flag){
    case MOUSE_DOWN:
      mi->current[0]=xm;
      mi->current[1]=ym;
      mi->last[0]=xm;
      mi->last[1]=ym;
      mi->direction[0]=0;
      mi->direction[1]=0;
      mi->angle = 0.0;
      mi->lastangle=0.0;
      START_TIMER(mi->lasttime);
      mi->xcurrent[0] = (float)(mi->current[0]-screenWidth/2)/(float)maxWH;
      mi->xcurrent[1] = (float)(mi->current[1]-screenHeight/2)/(float)maxWH;
      mi->angle=atan2(mi->xcurrent[1],mi->xcurrent[0]);
      delta_angle2 = 0.0;
      mi->lastangle=mi->angle;
      delta_distance=0.0;
      if((ABS(mi->xcurrent[0])<xymax&&ABS(mi->xcurrent[1])<xymax)||key_state==KEY_SHIFT){
        mi->region=0;
      }
      else{
        mi->region=1;
      }
      break;
    case MOUSE_UP:
      delta_angle2=0.0;
      delta_distance=0.0;
      break;
    case MOUSE_MOTION:
      START_TIMER(thistime_local);
      mi->current[0]=xm;
      mi->current[1]=ym;
      if(thistime_local-mi->lasttime>0.2){
        mi->direction[0]=mi->current[0]-mi->last[0];
        mi->direction[1]=mi->current[1]-mi->last[1];
        mi->last[0]=mi->current[0];
        mi->last[1]=mi->current[1];
        mi->lasttime=thistime_local;
      }
      mi->xcurrent[0] = (float)(mi->current[0]-screenWidth/2)/(float)maxWH;
      mi->xcurrent[1] = (float)(mi->current[1]-screenHeight/2)/(float)maxWH;
      dx = (float)(mi->current[0]-mi->last[0])/(float)maxWH;
      dy = (float)(mi->current[1]-mi->last[1])/(float)maxWH;
      delta_distance=sqrt(dx*dx+dy*dy);
      mi->angle=atan2(mi->xcurrent[1],mi->xcurrent[0]);
      delta_angle2 = mi->angle - mi->lastangle;
      mi->lastangle = mi->angle;
      break;
    default:
      ASSERT(FFALSE);
      break;
  }

  if(mi->direction[0]==0&&mi->direction[1]==0){
    mi->xdirection[0]=0.0;
    mi->xdirection[1]=0.0;
  }
  else{
    float sum;

    sum = mi->direction[0]*mi->direction[0] + mi->direction[1]*mi->direction[1];
    sum = sqrt(sum);
    mi->xdirection[0]=mi->direction[0]/sum;
    mi->xdirection[1]=mi->direction[1]/sum;
  }
  if(mi->region==0){
    delta_angle = (delta_distance);
    if(delta_angle!=0.0&&ABS(mi->xdirection[0])+ABS(mi->xdirection[1])>0.0){
      float axis[3];

      if(key_state==KEY_SHIFT){
        float w, x, y, z;
        int inside=0;

        w = quat_general[0];
        x = quat_general[1];
        y = quat_general[2];
        z = quat_general[3];
        if(z*z<x*x+y*y)inside=1;
        if((inside==1&&dy<0.0)||(inside==0&&dx<0.0))delta_angle=-delta_angle;
        axis[0]=2.0*x*z + 2.0*w*y;
        axis[1]=2.0*y*z-2.0*w*x;
        axis[2]=1.0-2*x*x-2*y*y;
      }
      else{
        axis[0]=-mi->xdirection[1]; // direction orthogonal to mouse motion
        axis[1]=0.0;
        axis[2]=mi->xdirection[0];
      }
      AngleAxis2Quat(delta_angle,axis,quat_temp);
      MultQuat(quat_temp,quat_general,quat_general);
     // LevelScene(0,1,quat_general);
      Quat2Rot(quat_general,quat_rotation);
    }
  }
  else{
    float axis[3];

    axis[0]=0.0;
    axis[1]=1.0;
    axis[2]=0.0;
    AngleAxis2Quat(-delta_angle2,axis,quat_temp);
    MultQuat(quat_temp,quat_general,quat_general);
   // LevelScene(0,1,quat_general);
    Quat2Rot(quat_general,quat_rotation);
  }
#ifdef _DEBUG
  {
    float denom;

    denom = sqrt(1.0-quat_general[0]*quat_general[0]);
    if(denom>0.0){
      PRINTF("angle= %f axis=(%f,%f,%f)\n",2.0*RAD2DEG*acos(quat_general[0]),quat_general[1]/denom,quat_general[2]/denom,quat_general[3]/denom);
    }
    else{
      PRINTF("angle= %f axis=(*,*,*)\n",2.0*RAD2DEG*acos(quat_general[0]));
    }
  }
#endif
  camera_current->quaternion[0]=quat_general[0];
  camera_current->quaternion[1]=quat_general[1];
  camera_current->quaternion[2]=quat_general[2];
  camera_current->quaternion[3]=quat_general[3];
  camera_current->quat_defined=1;
}

/* ------------------ MouseCB ------------------------ */

#define DELTA_TIME 0.25

void MouseCB(int button, int state, int xm, int ym){
  float *eye_xyz;

#ifdef pp_OSX_HIGHRES
  if(double_scale==1){
    xm *= 2;
    ym *= 2;
  }
#endif

  {
    float delta_time;

    delta_time = glutGet(GLUT_ELAPSED_TIME)/1000.0 - timer_reshape;
    if(delta_time<DELTA_TIME)return;
  }

  if(autofreeze_volsmoke==ON&&nvolsmoke_loaded>0){
    if(state==GLUT_DOWN)UpdateFreeze(ON);
    if(state==GLUT_UP)UpdateFreeze(OFF);
  }
  if(state == GLUT_UP){
    alt_ctrl_key_state = KEY_NONE;
  }
  if(rotation_type==ROTATION_3AXIS){
    if(state==GLUT_DOWN){
      UpdateMouseInfo(MOUSE_DOWN,xm,ym);
    }
    else if(state==GLUT_UP){
      UpdateMouseInfo(MOUSE_UP,xm,ym);
    }
  }

  if(trainer_mode==1){
    SetGLuiViewListManual();
  }
  eye_xyz = camera_current->eye;
  if(selected_view!=-999){
    selected_view=-999;
    updatemenu=1;
  }
  glui_move_mode=-1;
  move_gslice=0;
  glutPostRedisplay();

  if(state==GLUT_UP){
    tour_drag=0;
    mouse_down=0;
    show_gslice_normal_keyboard=0;
    eye_xyz0[0]=eye_xyz[0];
    eye_xyz0[1]=eye_xyz[1];
    eye_xyz0[2]=eye_xyz[2];
    UpdateTranslate();
    timebar_drag=0;
    colorbar_drag=0;
    colorbar_splitdrag=0;
    GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
    UpdateTrainerMoves();
    geom_bounding_box_mousedown = 0;
    return;
  }

  if(show_geom_boundingbox==SHOW_BOUNDING_BOX_MOUSE_DOWN)geom_bounding_box_mousedown = 1;
  mouse_down=1;

  // check for double click for translating/rotating 3D slice plane

  if(vis_gslice_data==1||show_gslice_triangles==1||show_gslice_triangulation==1){
    START_TIMER(this_mouse_time);
    if(this_mouse_time-last_mouse_time<0.5){
      gslice_xyz0[0]=gslice_xyz[0];
      gslice_xyz0[1]=gslice_xyz[1];
      gslice_xyz0[2]=gslice_xyz[2];
      gslice_normal_azelev0[0]=gslice_normal_azelev[0];
      gslice_normal_azelev0[1]=gslice_normal_azelev[1];
      move_gslice=1;
      show_gslice_normal_keyboard=1;
    }
    last_mouse_time=this_mouse_time;
  }
  if(button==GLUT_LEFT_BUTTON||button==GLUT_MIDDLE_BUTTON||button==GLUT_RIGHT_BUTTON){
    GLUTSETCURSOR(GLUT_CURSOR_INFO);

    /* edit blockages */

    if(button==GLUT_LEFT_BUTTON){
      if(blockageSelect == 1){
        GetGeomDialogState();
        if(structured_isopen == 1 && unstructured_isopen == 0)MouseEditBlockage(xm, ym);
      }
      if(edittour==1&&blockageSelect==0)MouseEditTour(xm,ym);
      if(viscolorbarpath==1)MouseEditColorbar(xm, ym);
      if(select_avatar==1)MouseSelectAvatar(xm,ym);
      if(select_device==1)MouseSelectDevice(xm,ym);
      if(select_geom!=GEOM_PROP_NONE)MouseSelectGeom(xm, ym);
    }
    glutPostRedisplay();
    if( showtime==1 || showplot3d==1){
      if(ColorbarClick(xm,ym)==1)return;
    }
    if(visTimebar==1&&showtime==1){
      if(TimebarClick(xm,ym)==1)return;
    }
    CopyCamera(camera_last,camera_current);
    if(canrestorelastview==0){
      updatemenu=1;
      canrestorelastview=1;
      EnableResetSavedView();
    }
    switch(button){
      case GLUT_MIDDLE_BUTTON:
        state=GLUT_ACTIVE_CTRL;
        break;
      case GLUT_RIGHT_BUTTON:
        state=GLUT_ACTIVE_ALT;
        break;
      default:
        state=GLUTGETMODIFIERS();
        break;
    }
    switch(state){
      case GLUT_ACTIVE_CTRL:
        key_state = KEY_CTRL;
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[1]=eye_xyz[1];
        touring=0;
        break;
      case GLUT_ACTIVE_ALT:
        key_state = KEY_ALT;
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[2]=eye_xyz[2];
        touring=0;
        break;
      case GLUT_ACTIVE_SHIFT:
        key_state = KEY_SHIFT;
        eye_xyz0[0] = eye_xyz[0];
        eye_xyz0[1] = eye_xyz[1];
        aperture_glui0 = aperture_glui;
        touring=0;
        break;
      default:
        key_state = KEY_NONE;
        start_xyz0[0]=xm;
        start_xyz0[1]=ym;
        touring=0;
        break;
    }
    mouse_down_xy0[0]=xm;
    mouse_down_xy0[1]=ym;
  }
  glutPostRedisplay();
  if(blockageSelect == 1){
    GetGeomDialogState();
    if(structured_isopen == 1 && unstructured_isopen == 0)DisplayCB();
  }
}

/* ------------------ ColorbarDrag ------------------------ */

void ColorbarDrag(int xm, int ym){
  int colorbar_index;

  colorbar_index = GetColorbarIndex(0,xm,ym);
  if(colorbar_index>=0){
    colorbar_select_index=colorbar_index;
    UpdateRGBColors(colorbar_index);
  }
}

/* ------------------ ColorbarSplitDrag ------------------------ */

void ColorbarSplitDrag(int xm, int ym){
  int colorbar_index;

  colorbar_index = GetColorbarIndex(0,xm,ym);
  if(colorbar_index>=0){
    int ii;

    if(colorbar_index>250)colorbar_index=250;
    if(colorbar_index<5)colorbar_index=5;
    ii=current_colorbar->splits[0];
    current_colorbar->index_node[ii]=colorbar_index;
    current_colorbar->index_node[ii-1]=colorbar_index;
    RemapColorbar(current_colorbar);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    UpdateColorbarSplits(current_colorbar);
  }
}

/* ------------------ DragColorbarEditNode ------------------------ */

void DragColorbarEditNode(int xm, int ym){
  float screen_perm[9];

  if(viscolorbarpath==0)return;

  switch(key_state){
  case KEY_NONE:
  case KEY_ALT:
  case KEY_SHIFT:
  case KEY_CTRL:
  {
    float xy[2] = {0.0,0.0}, xyz[3];

    xyz[0] = (float)cb_rgb[0]/255.0;
    xyz[1] = (float)cb_rgb[1]/255.0;
    xyz[2] = (float)cb_rgb[2]/255.0;
    GetScreenMapping(xyz, screen_perm);

    // scale mouse coordinates

    if(key_state!=KEY_ALT) xy[0] =  (float)(xm-mouse_down_xy0[0])/(float)screenWidth;
    if(key_state!=KEY_CTRL)xy[1] = -(float)(ym-mouse_down_xy0[1])/(float)screenHeight;

    // permute coordinates according to how screen is rotated

    xyz[0] = DOT2(xy, screen_perm);
    xyz[1] = DOT2(xy, screen_perm+3);
    xyz[2] = DOT2(xy, screen_perm+6);

    // compute mouse movement in rgb coordinates

    cb_rgb[0] = CLAMP(cb_rgb[0]+255*xyz[0],0,255);
    cb_rgb[1] = CLAMP(cb_rgb[1]+255*xyz[1],0,255);
    cb_rgb[2] = CLAMP(cb_rgb[2]+255*xyz[2],0,255);
    ColorbarCB(COLORBAR_RGB);

    mouse_down_xy0[0] = xm;
    mouse_down_xy0[1] = ym;

  }
  break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ DragTourNode ------------------------ */

void DragTourNode(int xm, int ym){
  float screen_perm[9];

  if(showtour_dialog==1&&edittour==1&&selected_frame!=NULL){
    GetScreenMapping(selected_frame->xyz_smv, screen_perm);
  }
  else{
    return;
  }
  switch(key_state){
    case KEY_NONE:
    case KEY_ALT:
    case KEY_SHIFT:
    case KEY_CTRL:
      {
        float xy[2]={0.0,0.0},xyz[3];
        float dx, dy, dz;

// scale mouse coordinates

        if(key_state!=KEY_ALT)xy[0] = (float)(xm-mouse_down_xy0[0])/(float)screenWidth;
        if(key_state!=KEY_CTRL)xy[1] = -(float)(ym-mouse_down_xy0[1])/(float)screenHeight;

// permute coordinates according to how screen is rotated

        xyz[0]=DOT2(xy,screen_perm);
        xyz[1]=DOT2(xy,screen_perm+3);
        xyz[2]=DOT2(xy,screen_perm+6);

// compute mouse movement in physical coordinates

        dx =  (xbarORIG-xbar0ORIG)*xyz[0];
        dy =  (ybarORIG-ybar0ORIG)*xyz[1];
        dz =  (zbarORIG-zbar0ORIG)*xyz[2];

// offset tour node location

        glui_tour_xyz[0] += dx;
        glui_tour_xyz[1] += dy;
        glui_tour_xyz[2] += dz;

        mouse_down_xy0[0]=xm;
        mouse_down_xy0[1]=ym;

// update tour data structures with new tour node location

        UpdateTourParms();
        UpdateGluiKeyframe();
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ MoveGenSlice ------------------------ */

void MoveGenSlice(int xm, int ym){
  int dxm, dym;

  dxm = xm - start_xyz0[0];
  dym = ym - start_xyz0[1];
  switch(key_state){
    case KEY_NONE:
      {
        float daz, delev;

        daz = 360.0*dxm/(float)screenWidth;
        delev = 360.0*dym/(float)screenHeight;
        gslice_normal_azelev[0] += daz;
        gslice_normal_azelev[1] += delev;
        UpdateGsliceParms();
        start_xyz0[0]=xm;
        start_xyz0[1]=ym;
      }
      break;
    case KEY_CTRL:
      {
        float xx, yy;
        float dx, dy;

        xx = xm-mouse_down_xy0[0];
        xx = xx/(float)screenWidth;
        yy = ym-mouse_down_xy0[1];
        yy = yy/(float)screenHeight;
        dx = (xyzbox+gslice_xyz0[0])*xx;
        dy = -(xyzbox-gslice_xyz0[1])*yy;
        gslice_xyz[0] += dx;
        gslice_xyz[1] += dy;
        gslice_xyz0[0] = gslice_xyz[0];
        gslice_xyz0[1] = gslice_xyz[1];
        mouse_down_xy0[0]=xm;
        mouse_down_xy0[1]=ym;
        UpdateGsliceParms();
      }
      break;
    case KEY_ALT:
      {
        float yy;

        yy = ym-mouse_down_xy0[1];
        yy = yy/(float)screenHeight;

        gslice_xyz[2] = gslice_xyz0[2] - DENORMALIZE_Z(4*(xyzbox-NORMALIZE_Z(gslice_xyz0[2]))*yy);
        UpdateGsliceParms();
      }
      break;
    case KEY_SHIFT:
    break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ MoveScene ------------------------ */

void MoveScene(int xm, int ym){
  float *eye_xyz, *az_elev;
  int dxm, dym;
  float elevation;
  float xx, yy;

  eye_xyz = camera_current->eye;
  az_elev = camera_current->az_elev;

  dxm = xm - start_xyz0[0];
  dym = ym - start_xyz0[1];
  switch(key_state){
    case KEY_NONE:
      switch(rotation_type){
        case ROTATION_3AXIS:
        break;
        case ROTATION_2AXIS:
        case ROTATION_1AXIS:
          az_elev[0] += dxm;
          if(rotation_type==ROTATION_2AXIS){
            az_elev[1] += dym;
          }
          else{
            az_elev[1]=0.0;
          }
          start_xyz0[0]=xm;
          start_xyz0[1]=ym;
          break;
        case EYE_CENTERED:
#define ANGLE_FACTOR 0.25
          camera_current->azimuth += dxm*ANGLE_FACTOR;
          start_xyz0[0]=xm;

          camera_current->elevation -= dym*ANGLE_FACTOR;
          elevation=camera_current->elevation;
          if(elevation>80.0)elevation=80.0;
          if(elevation<-80.0)elevation=-80.0;
          camera_current->elevation=elevation;
          start_xyz0[1]=ym;
          break;
        default:
          ASSERT(FFALSE);
          break;
      }
      break;

    case KEY_CTRL:
      {
        float dx, dy;

        xx = xm-mouse_down_xy0[0];
        xx = xx/(float)screenWidth;
        yy = ym-mouse_down_xy0[1];
        yy = yy/(float)screenHeight;
        if(rotation_type==EYE_CENTERED){
          float xx2, yy2;
          float cs_az, sn_az;

          cs_az = cos(DEG2RAD*camera_current->azimuth);
          sn_az = sin(DEG2RAD*camera_current->azimuth);

          xx2 = cs_az*xx - sn_az*yy;
          yy2 = sn_az*xx + cs_az*yy;
          xx = xx2;
          yy = yy2;
        }
        else{
          dx = (xyzbox+eye_xyz0[0])*xx;
          dy = -(xyzbox-eye_xyz0[1])*yy;
          eye_xyz[0] = eye_xyz0[0] + dx;
          eye_xyz[1] = eye_xyz0[1] + dy;
          eye_xyz0[0]=eye_xyz[0];
          eye_xyz0[1]=eye_xyz[1];
          mouse_down_xy0[0]=xm;
          mouse_down_xy0[1]=ym;
        }
      }
      break;

    case KEY_ALT:
      xx = xm-mouse_down_xy0[0];
      xx = xx/(float)screenWidth;
      yy = ym-mouse_down_xy0[1];
      yy = yy/(float)screenHeight;

      eye_xyz[0] = eye_xyz0[0]; /* disable horizontal motion */
      eye_xyz[2] = eye_xyz0[2] - 4*(xyzbox-eye_xyz0[2])*yy;
      viewx = eye_xyz[0] - delx;
      viewz = eye_xyz[2] - delz;
      break;
    case KEY_SHIFT:
      if(rotation_type!=EYE_CENTERED&&lock_mouse_aperture==0){
        float dx;

        xx = xm - mouse_down_xy0[0];
        xx = xx / (float)screenWidth;
        dx = (xyzbox+eye_xyz0[0])*xx;
        eye_xyz[0] = eye_xyz0[0] + dx;
        eye_xyz0[0]=eye_xyz[0];
        mouse_down_xy0[0]=xm;
      }
      yy = ym - mouse_down_xy0[1];
      yy = yy / (float)screenHeight;
      aperture_glui = CLAMP(aperture_glui0 + aperture_max*yy,aperture_min,aperture_max);
#define APERTURE 15
#define ZOOM 12
      SceneMotionCB(APERTURE);
      SceneMotionCB(ZOOM);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ ThrottleGpu ------------------------ */

int ThrottleGpu(void){
  float fps;

  START_TIMER(thisMOTIONtime);
  fps = MOTIONnframes/(thisMOTIONtime-lastMOTIONtime);
  if(fps>GPU_VOLframemax)return 1;
  MOTIONnframes++;
  if(thisMOTIONtime>lastMOTIONtime+0.25){
    PRINTF("MOTION: %4.1f fps\n",fps);
    lastMOTIONtime=thisMOTIONtime;
    MOTIONnframes=0;
  }
  return 0;
}

/* ------------------ MouseDragCB ------------------------ */

void MouseDragCB(int xm, int ym){
#ifdef pp_OSX_HIGHRES
  if(double_scale==1){
    xm *= 2;
    ym *= 2;
  }
#endif
  {
    float delta_time;

    delta_time = glutGet(GLUT_ELAPSED_TIME)/1000.0 - timer_reshape;
    if(delta_time<DELTA_TIME)return;
  }

  in_external=0;
#ifdef pp_GPUTHROTTLE
  if(usegpu==1&&showvolrender==1&&show_volsmoke_moving==1){
    if(ThrottleGpu()==1)return;
  }
#endif

  glutPostRedisplay();

  if( colorbar_drag==1&&(showtime==1 || showplot3d==1)){
    ColorbarDrag(xm,ym);
    return;
  }
  if(
    colorbar_splitdrag==1&&
    (showtime==1 || showplot3d==1)&&current_colorbar!=NULL&&current_colorbar->nsplits==1){
    ColorbarSplitDrag(xm,ym);
    return;
  }
  if(timebar_drag==1){
    TimebarDrag(xm);
    return;
  }
  if(move_gslice==1){
    MoveGenSlice(xm,ym);
    return;
  }
  if(tour_drag==1){
    DragTourNode(xm,ym);
    return;
  }
  if(colorbaredit_drag==1){
    DragColorbarEditNode(xm, ym);
    return;
  }
  if(rotation_type==ROTATION_3AXIS&&(key_state == KEY_NONE||key_state == KEY_SHIFT)){
    UpdateMouseInfo(MOUSE_MOTION,xm,ym);
  }
  MoveScene(xm,ym);
}

/* ------------------ KeyboardUpCB ------------------------ */

void KeyboardUpCB(unsigned char key, int x, int y){
  resetclock=1;
  alt_ctrl_key_state = KEY_NONE;
}

#ifdef pp_GPU
/* ------------------ PrintGPUState ------------------------ */

void PrintGPUState(void){
  char gpu_label[128];
  if(gpuactive==1){
    if(usegpu==1){
      strcpy(gpu_label,"GPU in use.");
    }
    else{
      strcpy(gpu_label,"GPU not in use.");
    }
  }
  else{
    strcpy(gpu_label,"GPU not available.");
  }
  PRINTF("%s ",gpu_label);
  PRINTF("\n");
}
#endif

/* ------------------ IsPartLoaded ------------------------ */

int IsPartLoaded(void){
  int i;

  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(parti->loaded==0||parti->display==0)continue;
    return 1;
  }
  return 0;
}

/* ------------------ IsPlot3DLoaded ------------------------ */

int IsPlot3DLoaded(void){
  int i;

  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    if(plot3di->loaded==0||plot3di->display==0)continue;
    return 1;
  }
  return 0;
}

/* ------------------ Plot3DListMenu ------------------------ */

int GetPlot3DTimeList(int inc){
  float time;
  int have_plot3d = 0, i;
  int return_val=-1;
  float delta_time;

  if(nplot3dtimelist<=1)return 0;
  delta_time = (plot3dtimelist[1]-plot3dtimelist[0])/2.0;

  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    if(plot3di->loaded==1){
      time = plot3di->time;
      have_plot3d = 1;
      break;
    }
  }
  if(have_plot3d==0)return 0;

  for(i = 0; i<nplot3dtimelist; i++){
    if(ABS(time-plot3dtimelist[i])<delta_time){
      return_val = i+inc;
      break;
    }
  }
  if(return_val<0)return_val = nplot3dtimelist-1;
  if(return_val>nplot3dtimelist-1)return_val = 0;
  return return_val;
}

/* ------------------ Keyboard ------------------------ */

void Keyboard(unsigned char key, int flag){
  char key2;
  int skip2;
  meshdata *gbsave,*gbi;
  int i;
  int keystate=0;

  if(flag==FROM_CALLBACK){
#ifdef pp_OSX
    keystate = GLUTGETMODIFIERS();
#else
    keystate = (GLUT_ACTIVE_ALT|GLUT_ACTIVE_CTRL)&GLUTGETMODIFIERS();
#endif
    if(scriptoutstream!=NULL&&key!='t'&&key!='r'&&key!='R'&&key!=' '&&key!='-'){
      fprintf(scriptoutstream,"KEYBOARD\n");
      switch(keystate){
        case GLUT_ACTIVE_ALT:
          fprintf(scriptoutstream," ALT %c\n",key);
          break;
        case GLUT_ACTIVE_CTRL:
          fprintf(scriptoutstream," CTRL %c\n",key);
          break;
        default:
          fprintf(scriptoutstream," %c\n",key);
          break;
      }
    }
  }
  else if(flag==FROM_SCRIPT){
    keystate=script_keystate;
  }
  else if(flag==FROM_SMOKEVIEW_ALT){
    keystate=GLUT_ACTIVE_ALT;
  }
  glutPostRedisplay();
  key2 = (char)key;

  switch(key2){
#define DEVNO_HRRNO   0
#define DEVYES_HRRYES 1
#define DEVYES_HRRNO  2
#define DEVNO_HRRYES  3
    case 'A':
      if(hrrptr==NULL&&ndeviceinfo==0)break;
      if(hrrptr!=NULL&&ndeviceinfo>0){
        plot_option++;
        if(plot_option>3)plot_option = 0;
      }
      else{
        int plot_option_temp = DEVNO_HRRNO;

        if(ndeviceinfo==0&&hrrptr!=NULL&&plot_option==DEVNO_HRRNO)plot_option_temp = DEVNO_HRRYES;
        if(ndeviceinfo>0&&hrrptr==NULL&&plot_option==DEVNO_HRRNO)plot_option_temp = DEVYES_HRRNO;
        plot_option = plot_option_temp;
      }
      // 0 - device no, hrr no
      // 1   device yes hrr yes
      // 2   device yes hrr no
      // 3   device no  hrr yes
// device plots
      switch (plot_option){
        case DEVNO_HRRNO: // device plots off
        case DEVNO_HRRYES:
          vis_device_plot = DEVICE_PLOT_SHOW_ALL;
          ShowObjectsMenu(OBJECT_PLOT_SHOW_ALL);
          break;
        case DEVYES_HRRYES: // device plots on
        case DEVYES_HRRNO:
          vis_device_plot = 0;
          ShowObjectsMenu(OBJECT_PLOT_SHOW_ALL);
          break;
	default:
	  ASSERT(FFALSE);
	  break;
      }
// hrr plot
      switch(plot_option){
        case DEVNO_HRRNO: // hrr plots off
        case DEVYES_HRRNO:
          vis_hrr_plot = 1;
          ShowObjectsMenu(PLOT_HRRPUV);
          break;
        case DEVYES_HRRYES: // hrr plots on
        case DEVNO_HRRYES:
          vis_hrr_plot = 0;
          ShowObjectsMenu(PLOT_HRRPUV);
          break;
	default:
	  ASSERT(FFALSE);
	  break;
      }
      break;
    case 'a':
      if(showtour_dialog==1&&edittour==1){
        AddDeleteKeyframe(ADD_KEYFRAME);
        break;
      }
      if(rotation_type==EYE_CENTERED){
        HandleMoveKeys(256+key2);
        break;
      }
      if((visVector==1&&nplot3dloaded>0)||showvslice==1||isZoneFireModel==1){
      }
      else{
        break;
      }
      if(isZoneFireModel==1){
        if(keystate==GLUT_ACTIVE_ALT){
          zone_ventfactor /= 1.5;
        }
        else{
          zone_ventfactor *= 1.5;
        }
        PRINTF("zone vent factor: %f\n",zone_ventfactor);
      }
      else{
        if(keystate==GLUT_ACTIVE_ALT){
          vecfactor/=1.5;
        }
        else{
          vecfactor*=1.5;
        }
        PRINTF("vector length factor: %f\n",vecfactor);
        UpdateGluiVecFactor();
      }
      if(visVector==1&&nplot3dloaded>0){
        gbsave=current_mesh;
        for(i=0;i<nmeshes;i++){
          gbi = meshinfo + i;
          if(gbi->plot3dfilenum==-1)continue;
          UpdateCurrentMesh(gbi);
          UpdatePlotSlice(XDIR);
          UpdatePlotSlice(YDIR);
          UpdatePlotSlice(ZDIR);
        }
        UpdateCurrentMesh(gbsave);
      }
      break;
    case 'B':
      if(show_geom_boundingbox==SHOW_BOUNDING_BOX_MOUSE_DOWN){
        show_geom_boundingbox = SHOW_BOUNDING_BOX_NEVER;
        printf("show bounding box when mouse is down: off\n");
      }
      else{
        show_geom_boundingbox = SHOW_BOUNDING_BOX_MOUSE_DOWN;
        printf("show bounding box when mouse is down: on\n");
      }
      UpdateGeomBoundingBox();

      break;
    case 'b':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_BOUNDS); // clip dialog
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        show_boundaryfiles = 1-show_boundaryfiles;
        if(show_boundaryfiles==1){
          ShowBoundaryMenu(GLUI_SHOWALL_BOUNDARY);
        }
        else{
          ShowBoundaryMenu(GLUI_HIDEALL_BOUNDARY);
        }
        break;
      }
      break;
    case 'c':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_CLIP); // clip dialog
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        if(nrooms>0){
          zone_highlight_room++;
          if(zone_highlight_room>=nrooms)zone_highlight_room=0;
          PRINTF("room %i\n",zone_highlight_room+1);
        }
        else{
          contour_type++;
          if(contour_type>2)contour_type=0;
          if(contour_type==LINE_CONTOURS)printf("line coloring\n");
          if(contour_type==STEPPED_CONTOURS)printf("stepped coloring\n");
          if(contour_type==SHADED_CONTOURS)printf("continuous coloring\n");
          UpdatePlot3dDisplay();
          UpdateRGBColors(COLORBAR_INDEX_NONE);
        }
      }
      break;
    case 'C':
      switch(keystate){
        case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
          DialogMenu(DIALOG_COLORBAR); // colorbar dialog
          break;
#endif
        case GLUT_ACTIVE_CTRL:
        default:
          if(nrooms>0){
            zone_highlight = 1 - zone_highlight;
            if(zone_highlight==1){
              PRINTF("room %i\n",zone_highlight_room+1);
            }
          }
      }
      break;
    case 'd':
    case 'D':
      if(key2=='d'&&keystate!=GLUT_ACTIVE_ALT){
        alt_ctrl_key_state = KEY_CTRL;
        break;
      }
      if(key2=='d'&&showtour_dialog==1&&edittour==1){
        AddDeleteKeyframe(DELETE_KEYFRAME);
        break;
      }
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_DISPLAY); // display dialog
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        if(rotation_type==EYE_CENTERED){
          HandleMoveKeys(256+key2);
        }
        else{
          demo_mode++;
          if(demo_mode>5)demo_mode=0;
        }
        break;
      }
      break;
    case 'e':
    case 'E':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_GEOMETRY); // edit geometry
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        rotation_type++;
        if(rotation_type>3)rotation_type=0;
        RotationTypeCB(rotation_type);
        UpdateRotationType(rotation_type);
        HandleRotationType(ROTATION_2AXIS);
      }
      break;
    case 'f':
      alt_ctrl_key_state = KEY_ALT;
      break;
    case 'F':
      hide_overlaps=1-hide_overlaps;
      updatehiddenfaces=1;
      UpdateHiddenFaces();
      UpdateShowHideButtons();
      glutPostRedisplay();
      break;
    case 'g':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_VIEW);
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        if(ntotal_blockages>0||isZoneFireModel==0||(isZoneFireModel==1&&ntrnx>0)){
          switch(visGrid){
            case NOGRID_NOPROBE:
              visGrid=GRID_NOPROBE;
              break;
            case GRID_NOPROBE:
              visGrid=GRID_PROBE;
              break;
            case GRID_PROBE:
              visGrid= NOGRID_PROBE;
              break;
            case NOGRID_PROBE:
              visGrid= NOGRID_PROBE2;
              break;
            case NOGRID_PROBE2:
              visGrid = NOGRID_NOPROBE;
              break;
            default:
              visGrid= NOGRID_NOPROBE;
              break;
          }
          if(visGrid==GRID_PROBE||visGrid==NOGRID_PROBE)visgridloc=1;
        }
        break;
      }
      break;
#ifdef pp_GPU
    case 'G':
      if(gpuactive==1){
        usegpu=1-usegpu;
      }
      else{
        usegpu=0;
      }
      if(nsmoke3dinfo>0){
        UpdateSmoke3dFlags();
      }
      PrintGPUState();
      return;
#endif
    case 'h':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
        if(titlesafe_offset==0){
          titlesafe_offset=titlesafe_offsetBASE;
        }
        else{
          titlesafe_offset=0;
        }
        break;
      case GLUT_ACTIVE_CTRL:
      default:
        if(histogram_show_graph == 1 || histogram_show_numbers == 1){
          histogram_show_graph = 0;
          histogram_show_numbers = 0;
        }
        else{
          histogram_show_graph = 1;
          histogram_show_numbers = 1;
          visColorbarVertical = 1;
          update_slice_hists = 1;
        }
        UpdateHistogramType();
        break;
      }
      break;
    case 'H':
      {
        int nslice_loaded_local=0, nvslice_loaded_local=0;

        for(i=0;i<nsliceinfo;i++){
          slicedata *sd;

          sd = sliceinfo + i;
          if(sd->loaded==1)nslice_loaded_local++;
        }
        for(i=0;i<nvsliceinfo;i++){
          vslicedata *vd;

          vd = vsliceinfo + i;
          if(vd->loaded==1)nvslice_loaded_local++;
        }
        stept=1;
        if(nvslice_loaded_local>0){
          if(showall_slices==0){
            ShowVSliceMenu(GLUI_SHOWALL_VSLICE);
            force_redisplay=1;
          }
          else{
            itime_save=itimes;
            ShowVSliceMenu(GLUI_HIDEALL_VSLICE);
          }
        }
        if(nvslice_loaded_local==0&&nslice_loaded_local>0){
          if(showall_slices==0){
            ShowHideSliceMenu(GLUI_SHOWALL);
            force_redisplay=1;
          }
          else{
            itime_save=itimes;
            ShowHideSliceMenu(GLUI_HIDEALL);
          }
        }
      }
      break;
    case 'i':
      if(keystate==GLUT_ACTIVE_ALT){ // toggle device visibility
        if(nobject_defs>0){
          int vis;

          vis = 1-object_defs[0]->visible;
          for(i = 0; i<nobject_defs; i++){
            sv_object *objecti;

            objecti = object_defs[i];
            objecti->visible = vis;
          }
          updatemenu = 1;
        }
      }
      else if(cache_plot3d_data==1){
        HandleIso();
        return;
      }
      break;
    case 'I':
      show_slice_in_obst++;
      if(show_slice_in_obst>2)show_slice_in_obst = 0;
      UpdateShowSliceInObst(show_slice_in_obst);
      updatemenu = 1;
      break;
    case 'j':
    case 'J':
      if(keystate==GLUT_ACTIVE_ALT||key2=='J'){
        sensorrelsize /= 1.5;
      }
      else{
        sensorrelsize *= 1.5;
      }
      UpdateDeviceSize();
      break;
    case '`':
      if(ndeviceinfo>0){
        int selected;

        selected = 1-deviceinfo[0].selected;
        if(selected==1&&select_device==0)select_device = 1;
        for(i = 0; i<ndeviceinfo; i++){
          devicedata *devicei;

          devicei = deviceinfo+i;
          devicei->selected = selected;
        }
      }
      if(nobject_defs>0){
        int makevis=1;

        for(i = 0; i<nobject_defs; i++){
          sv_object *objecti;

          objecti = object_defs[i];
          if(objecti->visible==1){
            makevis = 0;
            break;
          }
        }
        if(makevis==1){
          for(i = 0; i<nobject_defs; i++){
            sv_object *objecti;

            objecti = object_defs[i];
            objecti->visible = 1;
          }
        }
        updatemenu = 1;
      }
      break;
    case 'k':
    case 'K':
      if(keystate==GLUT_ACTIVE_ALT){ // toggle device selection
        select_device = 1-select_device;
        updatemenu = 1;
        if(select_device==1){
          printf("device selection on\n");
        }
        else{
          printf("device selection off\n");
        }
      }
      else{
        visTimebar = 1 - visTimebar;
        if(visTimebar==0)PRINTF("Time bar hidden\n");
        if(visTimebar==1)PRINTF("Time bar visible\n");
      }
      break;
    case 'l':
      LoadUnloadMenu(RELOADALL);
      break;
    case 'L':
      UnloadSliceMenu(UNLOAD_LAST);
      break;
    case 'm':
    case 'M':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_MOTION); // motion dialog
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        if(nmeshes>1){
          highlight_mesh++;
          if(highlight_mesh>nmeshes-1)highlight_mesh=0;
          UpdateCurrentMesh(meshinfo+highlight_mesh);
        }
      }
      break;
    case 'n':
    case 'N':
      show_cface_normals = 1-show_cface_normals;
      if(show_cface_normals==1){
        printf("show cface normals\n");
      }
      else{
        printf("hide cface normals\n");
      }
      UpdateGluiCfaces();
      break;
    case 'O':
    if(ncgeominfo>0){
      if(show_faces_outline==0&&show_faces_shaded==1){
        show_faces_outline = 1;
        show_faces_shaded = 1;
        terrain_show_geometry_outline = 1;
      }
      else if(show_faces_outline==1&&show_faces_shaded==1){
        show_faces_outline = 1;
        show_faces_shaded = 0;
        terrain_show_geometry_outline = 1;
      }
      else if(show_faces_outline==1&&show_faces_shaded==0){
        show_faces_outline = 0;
        show_faces_shaded = 0;
        terrain_show_geometry_outline = 0;
      }
      else if(show_faces_outline==0&&show_faces_shaded==0){
        show_faces_outline = 0;
        show_faces_shaded = 1;
        terrain_show_geometry_outline = 0;
      }
      if(use_cfaces==1){
        printf("cfaces: ");
      }
      else{
        printf("geometry: ");
      }
      if(show_faces_shaded==1) printf("shaded triangles ");
      if(show_faces_outline==1)printf("outlines");
      if(show_faces_shaded==0&&show_faces_outline==0)printf("hidden");
      printf("\n");
    }
    UpdateGeometryControls();
      switch(visBlocks){
        case visBLOCKAsInput:
        case visBLOCKAsInputOutline:
          BlockageMenu(visBLOCKHide);
          BlockageMenu(visBLOCKOnlyOutline);
          break;
        default:
          BlockageMenu(visBLOCKHide);
          BlockageMenu(visBLOCKAsInput);
          break;
      }
      break;
    case 'o':
      if(keystate==GLUT_ACTIVE_ALT){
        switch(visBlocks){
          case visBLOCKAsInput:
            BlockageMenu(visBLOCKHide);
            BlockageMenu(visBLOCKAsInput);
            BlockageMenu(visBLOCKAddOutline);
            break;
          case visBLOCKAsInputOutline:
            BlockageMenu(visBLOCKHide);
            BlockageMenu(visBLOCKNormal);
            break;
          case visBLOCKNormal:
            BlockageMenu(visBLOCKHide);
            BlockageMenu(visBLOCKOnlyOutline);
            break;
          case visBLOCKOutline:
          case visBLOCKOnlyOutline:
            BlockageMenu(visBLOCKHide);
            break;
          case visBLOCKHide:
            BlockageMenu(visBLOCKHide);
            BlockageMenu(visBLOCKAsInput);
            break;
          default:
            BlockageMenu(visBLOCKHide);
            BlockageMenu(visBLOCKAsInput);
            break;
        }
      }
      else{
        highlight_flag++;
        if(highlight_flag!=0&&visFrame==0){
          visFrame = 1;
          updatefacelists = 1;
          updatemenu = 1;
          glutPostRedisplay();
        }
        if(highlight_flag>2&&noutlineinfo>0)highlight_flag=0;
        if(highlight_flag>1&&noutlineinfo==0)highlight_flag=0;
        PRINTF("outline mode=%i\n",highlight_flag);
      }
      break;
    case 'p':
    case 'P':
      {
        int is_part_loaded, is_plot3d_loaded;

        is_part_loaded = IsPartLoaded();
        is_plot3d_loaded = IsPlot3DLoaded();

        if(is_part_loaded==1||is_plot3d_loaded==1){
          if(is_part_loaded==1){
            IncrementPartPropIndex();
#define BOUND_PERCENTILE_DRAW          120
            PartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
          }
          if(is_plot3d_loaded==1){
            plotn += FlowDir;
            if(plotn<1){
              plotn = numplot3dvars;
            }
            if(plotn>numplot3dvars){
              plotn = 1;
            }
            UpdateAllPlotSlices();
            if(visiso==1&&cache_plot3d_data==1)UpdateSurface();
            UpdatePlot3dListIndex();
            Plot3DBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
          }
        }
        else{
          if(ndevicetypes>0){
            devicetypes_index++;
            if(devicetypes_index>=ndevicetypes)devicetypes_index = 0;
            updatemenu = 1;
            UpdateDeviceTypes(devicetypes_index);
          }
        }
      }
      update_chop_colors = 1;
      break;
    case 'q':
      if(ncgeominfo>0){
        use_cfaces = 1-use_cfaces;
        if(use_cfaces==1){
          printf("cfaces: ");
        }
        else{
          printf("geometry: ");
        }
        if(show_faces_shaded==1) printf("shaded triangles ");
        if(show_faces_outline==1)printf("outlines");
        if(show_faces_shaded==0&&show_faces_outline==0)printf("hidden");
        printf("\n");
        UpdateGluiCfaces();
      }
      blocklocation++;
      if((ncadgeom==0&&blocklocation>BLOCKlocation_exact)||
                       blocklocation>BLOCKlocation_cad){
        blocklocation=BLOCKlocation_grid;
      }
      if(blocklocation==BLOCKlocation_grid)printf("blocklocation: snapped to grid\n");
      if(blocklocation==BLOCKlocation_exact)printf("blocklocation: as input\n");
      if(blocklocation==BLOCKlocation_cad)printf("blocklocation: cad\n");
      if(showedit_dialog==1){
        if(blocklocation==BLOCKlocation_exact){
          blockage_as_input=1;
        }
        else{
          blockage_as_input=0;
        }
        ObjectCB(BLOCKAGE_AS_INPUT2);
      }
      break;
    case 'Q':
      showhide_textures = 1-showhide_textures;
      for(i = 0; i<ntextureinfo; i++){
        texturedata *texti;

        texti = textureinfo+i;
        if(texti->loaded==0||texti->used==0)continue;
        if(texti->display==0){ // if any textures are hidden then show them all
          showhide_textures = 1;
          break;
        }
      }
      if(showhide_textures==1){
        TextureShowMenu(MENU_TEXTURE_SHOWALL);
      }
      else{
        TextureShowMenu(MENU_TEXTURE_HIDEALL);
      }
      break;
    case 'r':
    case 'R':
      {
        int rflag=0;

        if(keystate==GLUT_ACTIVE_ALT&&strncmp((const char *)&key2, "r", 1) == 0){
          research_mode = 1-research_mode;
          UpdatdateResearchModeCPP();
          update_research_mode = 1;
          return;
        }

        if(strncmp((const char *)&key2, "R", 1)==0&&keystate!=GLUT_ACTIVE_ALT){
          resolution_multiplier = glui_resolution_multiplier;
        }
        else{
          resolution_multiplier = 1;
        }

        if(keystate == GLUT_ACTIVE_ALT){
          render_mode = RENDER_360;
        }
        else{
          render_mode = RENDER_NORMAL;
        }
        render_times = RENDER_SINGLETIME;

        if(strncmp((const char *)&key2, "R", 1)==0||render_mode==RENDER_360){
          rflag=1;
        }
        else if(render_size_index==2){
          renderW=0;
          renderH=0;
        }
        if(scriptoutstream!=NULL){
          if(nglobal_times>0){
            float timeval;

            timeval=global_times[itimes];
            fprintf(scriptoutstream,"SETTIMEVAL\n");
            fprintf(scriptoutstream," %f\n",timeval);
            if(nvolrenderinfo>0&&load_at_rendertimes==1){
              for(i=0;i<nmeshes;i++){
                meshdata *meshi;
                volrenderdata *vr;
                int j;
                int framenum;
                float timediffmin;

                meshi = meshinfo + i;
                vr = &meshi->volrenderinfo;
                if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
                if(vr->loaded==0||vr->display==0)continue;
                timediffmin = ABS(timeval-vr->times[0]);
                framenum=0;
                for(j=1;j<vr->ntimes;j++){
                  float timediff;

                  timediff = ABS(vr->times[j]-timeval);
                  if(timediff<timediffmin){
                    timediffmin=timediff;
                    framenum=j;
                  }
                }
                fprintf(scriptoutstream,"LOADVOLSMOKEFRAME\n");
                fprintf(scriptoutstream," %i %i\n",i,framenum);
              }
            }
          }
          else{
            int show_plot3dkeywords=0;

            for(i=0;i<nmeshes;i++){
              meshdata *meshi;
              plot3ddata *plot3di;
              float *xp, *yp, *zp;

              meshi = meshinfo  + i;
              if(meshi->plot3dfilenum==-1)continue;

              plot3di = plot3dinfo + meshi->plot3dfilenum;
              if(plot3di->display==0)continue;
              show_plot3dkeywords=1;
              xp = meshi->xplt_orig;
              yp = meshi->yplt_orig;
              zp = meshi->zplt_orig;
              fprintf(scriptoutstream,"SHOWPLOT3DDATA\n");
              fprintf(scriptoutstream," %i %i %i %i %f\n",i+1,1, plotn,visx_all,xp[meshi->plotx]);
              fprintf(scriptoutstream,"SHOWPLOT3DDATA\n");
              fprintf(scriptoutstream," %i %i %i %i %f\n",i+1,2, plotn,visy_all,yp[meshi->ploty]);
              fprintf(scriptoutstream,"SHOWPLOT3DDATA\n");
              fprintf(scriptoutstream," %i %i %i %i %f\n",i+1,3, plotn,visz_all,zp[meshi->plotz]);
              fprintf(scriptoutstream,"SHOWPLOT3DDATA\n");
              fprintf(scriptoutstream," %i %i %i %i %i\n",i+1,4, plotn,visiso,plotiso[plotn-1]);
            }
            if(show_plot3dkeywords==1){
              fprintf(scriptoutstream,"PLOT3DPROPS\n");
              fprintf(scriptoutstream," %i %i 4 %i\n",plotn,visVector,contour_type);
            }
          }
          if(clip_rendered_scene!=0){
            fprintf(scriptoutstream,"RENDERCLIP\n");
            fprintf(scriptoutstream," %i %i %i %i %i\n",
              clip_rendered_scene,render_clip_left,render_clip_right,render_clip_bottom,render_clip_top);
          }
          if(vis_gslice_data==1||show_gslice_triangles==1||show_gslice_triangulation==1||show_gslice_normal){
            fprintf(scriptoutstream,"GSLICEVIEW\n");
            fprintf(scriptoutstream," %i %i %i %i\n",vis_gslice_data,show_gslice_triangles,show_gslice_triangulation,show_gslice_normal);
            fprintf(scriptoutstream,"GSLICEPOS\n");
            fprintf(scriptoutstream," %f %f %f\n",gslice_xyz[0],gslice_xyz[1],gslice_xyz[2]);
            fprintf(scriptoutstream,"GSLICEORIEN\n");
            fprintf(scriptoutstream," %f %f\n",gslice_normal_azelev[0],gslice_normal_azelev[1]);
          }
          if(rflag==0){
            fprintf(scriptoutstream,"RENDERONCE\n");
          }
          else{
            fprintf(scriptoutstream,"RENDERDOUBLEONCE\n");
          }
          fprintf(scriptoutstream," %s\n",script_renderfile);
        }
        RenderState(RENDER_ON);
      }
      break;
    case 's':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        DialogMenu(DIALOG_3DSMOKE); // 3d smoke dialog
        break;
#endif
      case GLUT_ACTIVE_CTRL:
        SnapScene();
        break;
      default:
        if(rotation_type==EYE_CENTERED){
          HandleMoveKeys(GLUT_KEY_DOWN);
        }
        else{
          vectorskip++;
          if(vectorskip>4)vectorskip=1;
        }
      }
      break;
    case 'S':
      stereotypeOLD=stereotype;
      stereotype++;
      if(stereotype>5)stereotype=0;
      if(stereotype==STEREO_TIME&&videoSTEREO!=1)stereotype=STEREO_LR;
      UpdateGluiStereo();
      break;
    case 't':
      switch(keystate){
      case GLUT_ACTIVE_ALT:
#ifdef pp_DIALOG_SHORTCUTS
        if(showtour_dialog==1){
          DialogMenu(DIALOG_TOUR_HIDE);
        }
        else{
          DialogMenu(DIALOG_TOUR_SHOW);
        }
        break;
#endif
      case GLUT_ACTIVE_CTRL:
      default:
        stept=(stept+1)%2;
        if(stept==1){
          plotstate=GetPlotState(DYNAMIC_PLOTS);
          if(plotstate==DYNAMIC_PLOTS){
            ResetGLTime();
          }
          else{
            stept=0;
          }
        }
        if(stept == 1){
          //if(render_skip!=RENDER_CURRENT_SINGLE)render_skip = 1;
        }
        else{
          if(flag==FROM_CALLBACK){
            last_time_paused = 0;
          }
          itime_save = -1;
          render_skip = RENDER_CURRENT_SINGLE;
        }
        updatemenu = 1;
        UpdateRenderListSkip();
      }
      break;
    case 'T':
      vishmsTimelabel = 1-vishmsTimelabel;
      SetLabelControls();
      break;
    case 'u':
    case 'U':
      switch(keystate){
        case GLUT_ACTIVE_ALT:
          skip_slice_in_embedded_mesh = 1 - skip_slice_in_embedded_mesh;
          break;
        default:
          if(key2=='U'){
            ReloadMenu(RELOAD_INCREMENTAL_NOW);
          }
          else{
            ReloadMenu(RELOAD_ALL_NOW);
          }
          break;
      }
      break;
    case '|':
      projection_type = 1-projection_type;
      SceneMotionCB(PROJECTION);
      break;
    case 'v':
      switch(keystate){
        case GLUT_ACTIVE_ALT:
          projection_type = 1 - projection_type;
          SceneMotionCB(PROJECTION);
          break;
        default:
          visVector=1-visVector;
          if(vectorspresent==0)visVector=0;
          break;
      }
      break;
    case 'V':
      if(nvolrenderinfo>0){
        usevolrender=1-usevolrender;
        UpdateSmoke3dFlags();
#ifdef pp_GPU
        PrintGPUState();
#endif
        return;
      }
      break;
    case 'w':
      switch(keystate){
        case GLUT_ACTIVE_ALT:
        case GLUT_ACTIVE_CTRL:
        default:
          if(rotation_type==EYE_CENTERED){
            HandleMoveKeys(GLUT_KEY_UP);
          }
          else{
            vis_gslice_data = 1 - vis_gslice_data;
            UpdateGsliceParms();
          }
          break;
      }
      break;
    case 'W':
      clip_mode++;
      if(clip_mode>CLIP_MAX)clip_mode=0;
      UpdateClipAll();
      break;
    case 'x':
    case 'X':
#ifdef pp_DIALOG_SHORTCUTS
      if(keystate==GLUT_ACTIVE_ALT){
        if(key2=='x')DialogMenu(DIALOG_HIDEALL);
        if(key2=='X')DialogMenu(DIALOG_SHRINKALL);
      }
      else{
        visx_all=1-visx_all;
        plotstate = GetPlotState(STATIC_PLOTS);
        updatemenu = 1;
      }
#else
      visx_all = 1-visx_all;
      plotstate = GetPlotState(STATIC_PLOTS);
      updatemenu = 1;
#endif
      if(visx_all==1||visy_all==1||visz_all==1)update_slice2device = 1;
      break;
    case 'y':
    case 'Y':
      visy_all = 1-visy_all;
      if(visx_all==1||visy_all==1||visz_all==1)update_slice2device = 1;
      plotstate = GetPlotState(STATIC_PLOTS);
      updatemenu = 1;
      break;
    case 'Z':
      rotate_center = 1-rotate_center;
      if(rotate_center==1&&have_geom_bb==1){
        printf("rotate about FDS+GEOM center\n");
        UpdateGluiRotateAbout(ROTATE_ABOUT_WORLD_CENTER);
      }
      else{
        printf("rotate about FDS domain center\n");
        UpdateGluiRotateAbout(ROTATE_ABOUT_FDS_CENTER);
      }
      break;
    case 'z':
#ifdef pp_DIALOG_SHORTCUTS
      if(keystate==GLUT_ACTIVE_ALT){
        DialogMenu(DIALOG_SMOKEZIP); // compress dialog
      }
      else{
        visz_all = 1 - visz_all;
        plotstate = GetPlotState(STATIC_PLOTS);
        updatemenu = 1;
      }
#else
      visz_all = 1-visz_all;
      plotstate = GetPlotState(STATIC_PLOTS);
      updatemenu = 1;
#endif
      if(visx_all==1||visy_all==1||visz_all==1)update_slice2device = 1;
      break;
    case '0':
      if(plotstate==DYNAMIC_PLOTS){
        itime_cycle = 0;
        UpdateTimes();
        return;
      }
      break;
    case '~':
      LevelScene(1,1,quat_general);
      Quat2Rot(quat_general,quat_rotation);
      break;
    case '=':
      if(ngeominfo>0){
        select_geom++;
        if(select_geom==5)select_geom=0;
        if(select_geom==GEOM_PROP_NONE)printf("geometry selection off\n");
        if(select_geom==GEOM_PROP_VERTEX1)printf("select vertex 1\n");
        if(select_geom==GEOM_PROP_VERTEX2)printf("select vertex 2\n");
        if(select_geom==GEOM_PROP_TRIANGLE)printf("select triangle\n");
        if(select_geom==GEOM_PROP_SURF)printf("select surf\n");
        UpdateSelectGeom();
      }
      break;
    case '!':
      SnapScene();
      break;
    case '@':
      show_slice_values_all_regions = 1 - show_slice_values_all_regions;
      if(show_slice_values_all_regions==1){
        show_slice_values[0]=1;
        show_slice_values[1]=1;
        show_slice_values[2]=1;
      }
      else{
        show_slice_values[0]=0;
        show_slice_values[1]=0;
        show_slice_values[2]=0;
      }
#define IMMERSED_SWITCH_CELLTYPE 0
      ImmersedBoundCB(IMMERSED_SWITCH_CELLTYPE);
      break;
    case '.':
      lock_mouse_aperture = 1 - lock_mouse_aperture;
      break;
    case '?':
      vector_debug = 1 - vector_debug;
      break;
    case ':':
      timebar_overlap++;
      if (timebar_overlap > 2)timebar_overlap = 0;
      UpdateTimebarOverlap();
      printf("overlap time/colorbar region: ");
      switch(timebar_overlap){
      case 0:
        printf("always\n");
        break;
      case 1:
        printf("never\n");
        break;
      case 2:
        printf("only if time/colorbar hidden\n");
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      break;
 //    vis_colorbar                       state
 //    0/COLORBAR_HIDDEN                  hidden
 //    1/COLORBAR_SHOW_VERTICAL           vertical
 //    2->max/COLORBAR_SHOW_HORIZONTAL    horizontal
    case ',':
      {
        int maxtoggle;

        maxtoggle = MAX(3, 2 + CountColorbars());
        vis_colorbar++;
        if(vis_colorbar>= maxtoggle)vis_colorbar = 0;
        if(vis_colorbar== COLORBAR_HIDDEN) {
          visColorbarVertical = 0;
          visColorbarHorizontal = 0;
        }
        else if(vis_colorbar== COLORBAR_SHOW_VERTICAL) {
          visColorbarVertical = 1;
          visColorbarHorizontal = 0;
        }
        else {
          visColorbarVertical = 0;
          visColorbarHorizontal = 1;
        }
      }
      updatemenu = 1;
      break;
    case '<':
      vectorpointsize+=2;
      if(vectorpointsize>20.0)vectorpointsize = 1.0;
      UpdateVectorpointsize();
      updatemenu = 1;
      break;
    case '>':
      vectorpointsize-=2;
      if(vectorpointsize<1.0)vectorpointsize = 20.0;
      UpdateVectorpointsize();
      updatemenu = 1;
      break;
    case '#':
      WriteIni(LOCAL_INI,NULL);
      break;
    case '/':
      updatemenu=1;
      partfast = 1 - partfast;
      if(nevac>0){
        partfast = 0;
        part_multithread = 0;
      }
      if(current_script_command==NULL){
        if(npartinfo>1&&nevac==0){
          part_multithread = partfast;
        }
        else{
          part_multithread = 0;
        }
      }
      if(part_multithread==1){
        if(npartthread_ids>1)printf("parallel particle loading: on(%i threads)\n",npartthread_ids);
        if(npartthread_ids==1)printf("parallel particle loading: on(1 thread)\n");
      }
      if(part_multithread==0)printf("parallel particle loading: off\n");
      UpdateGluiPartFast();
      break;
    case '$':
      trainer_active=1-trainer_active;
      if(trainer_active==1){
        PRINTF("Trainer mode active\n");
        trainer_mode=1;
        ShowGluiTrainer();
      }
      if(trainer_active==0){
        PRINTF("Trainer mode inactive\n");
        trainer_mode=0;
        HideGluiTrainer();
      }
      break;
    case '%':
      script_step=1-script_step;
      break;
    case '^':
      script_step_now=1;
      break;
    case '&':
      antialiasflag=1-antialiasflag;
      PRINTF("antialiasflag=%i\n",antialiasflag);
      break;
    case '*':
      visx_all=0;
      visy_all=0;
      visz_all=0;
      vis_gslice_data=0;
      show_gslice_triangles=0;
      show_gslice_triangulation=0;
      show_gslice_normal=0;
      break;
    case '(':
      clip_rendered_scene=1-clip_rendered_scene;
      break;
    case '[':
      edittour=1;
      UpdateEditTour();
      break;
    case ']':
      edittour=0;
      UpdateEditTour();
      break;
    case ';':
      ColorbarMenu(COLORBAR_FLIP);
      break;
#ifdef pp_REFRESH
    case '_':
      RefreshGluiDialogs();
      break;
#endif
    case '{':
    case '}':
      if(key2=='{')iplot3dtimelist = GetPlot3DTimeList(-1);
      if(key2=='}')iplot3dtimelist = GetPlot3DTimeList(1);
      Plot3DListMenu(iplot3dtimelist);
      updatemenu = 1;
      break;
    case '-':
      break;
    case ' ':
      break;
    default:
      ASSERT(FFALSE);
      break;
  }

  skip2=key2-'1'+1;
  if(skip2>0&&skip2<10)skip_global=skip2;

  /* if not a directional key then return */

  if(strncmp((const char *)&key2,"-",1)!=0&&strncmp((const char *)&key2," ",1)!=0&&
     strncmp((const char *)&key2,"+",1)!=0&&strncmp((const char *)&key2,"=",1)!=0&&
     strncmp((const char *)&key2,"<",1)!=0&&strncmp((const char *)&key2,">",1)!=0&&
     strncmp((const char *)&key2,",",1)!=0&&strncmp((const char *)&key2,".",1)!=0&&
     strncmp((const char *)&key2,"_",1)!=0&&(skip2<=0||skip2>=10))return;

  if(clip_mode!=CLIP_OFF&&(
    strncmp((const char *)&key2,"<",1)==0||strncmp((const char *)&key2,",",1)==0||
    strncmp((const char *)&key2,">",1)==0||strncmp((const char *)&key2,".",1)==0)){

    if(strncmp((const char *)&key2,"<",1)==0||strncmp((const char *)&key2,",",1)==0){ClipDir=-1;}
     else if(strncmp((const char *)&key2,">",1)==0||strncmp((const char *)&key2,".",1)==0){ClipDir=1;}

    if(stepclip_xmin==1  )clip_i += skip_global*ClipDir;
    if(stepclip_ymin==1  )clip_j += skip_global*ClipDir;
    if(stepclip_zmin==1  )clip_k += skip_global*ClipDir;
    if(stepclip_xmax==1  )clip_I += skip_global*ClipDir;
    if(stepclip_ymax==1  )clip_J += skip_global*ClipDir;
    if(stepclip_zmax==1  )clip_K += skip_global*ClipDir;

    UpdateClipbounds(clipinfo.clip_xmin,&clip_i,clipinfo.clip_xmax,&clip_I,current_mesh->ibar);
    UpdateClipbounds(clipinfo.clip_ymin,&clip_j,clipinfo.clip_ymax,&clip_J,current_mesh->jbar);
    UpdateClipbounds(clipinfo.clip_zmin,&clip_k,clipinfo.clip_zmax,&clip_K,current_mesh->kbar);
    return;
  }

  if(strncmp((const char *)&key2,"-",1)==0||strncmp((const char *)&key2,"_",1)==0){
    FlowDir=-1;
  }
  else if(strncmp((const char *)&key2," ",1)==0||
     strncmp((const char *)&key2,"=",1)==0||
     strncmp((const char *)&key2,"+",1)==0
     ){
     FlowDir=1;
  }

  if(plotstate==DYNAMIC_PLOTS){
    if(timebar_drag==0)itimes += skip_global*FlowDir;
    CheckTimeBound();
    IdleCB();

    return;
  }
  switch(iplot_state){
    case XDIR:
      NextXIndex(skip_global*FlowDir,0);
      break;
    case 0:
    case YDIR:
      NextYIndex(skip_global*FlowDir,0);
      break;
    case ZDIR:
      NextZIndex(skip_global*FlowDir,0);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  if(nplot3dloaded>0){
    plotstate = GetPlotState(STATIC_PLOTS);
    if(visiso!=0&&current_mesh->slicedir==ISO){
      plotiso[plotn-1] += FlowDir;
      UpdateSurface();
    }
    glutPostRedisplay();
  }
  if(iplot_state!=0)UpdatePlotSlice(iplot_state);
}

/* ------------------ KeyboardCB ------------------------ */

void KeyboardCB(unsigned char key, int x, int y){
  Keyboard(key,FROM_CALLBACK);
  glutPostRedisplay();
  updatemenu=1;
}

/* ------------------ HandleRotationType ------------------------ */

void HandleRotationType(int flag){
  float *az_elev;

  if(rotation_type==rotation_type_old)return;
  camera_current->rotation_type=rotation_type;
  az_elev = camera_current->az_elev;
  updatemenu=1;
  switch(rotation_type){
  case ROTATION_3AXIS:
      if(trainer_mode==0)PRINTF("Scene centered (3 axis rotation)\n");
      if(showtrainer_dialog==0&&flag==ROTATION_2AXIS&&rotation_type_old==EYE_CENTERED){
        SetViewPoint(RESTORE_EXTERIOR_VIEW);
      }
      break;
  case ROTATION_2AXIS:
      if(trainer_mode==0)PRINTF("Scene centered (2 axis rotation)\n");
      if(showtrainer_dialog==0&&flag==ROTATION_2AXIS&&rotation_type_old==EYE_CENTERED){
        SetViewPoint(RESTORE_EXTERIOR_VIEW);
      }
      break;
  case EYE_CENTERED:
       az_elev[1]=0.0;
       if(showtrainer_dialog==0&&flag==ROTATION_2AXIS&&rotation_type_old!=EYE_CENTERED){
         SetViewPoint(RESTORE_EXTERIOR_VIEW);
       }
      if(trainer_mode==0)PRINTF("eye centered\n");
      break;
  case ROTATION_1AXIS:
    az_elev[1]=0.0;
    if(trainer_mode==0)PRINTF("Scene centered (level rotation)\n");
    if(showtrainer_dialog==0&&flag==ROTATION_2AXIS&&rotation_type_old==EYE_CENTERED){
      SetViewPoint(RESTORE_EXTERIOR_VIEW);
    }
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  ShowHideTranslate(rotation_type);
  rotation_type_old = rotation_type;
  return;
}

/* ------------------ UpdateClipPlanes ------------------------ */

void UpdateClipPlanes(void){
  if(trainer_mode==0){
    if(clip_mode!=clip_mode_last){
      if(startup==1){
        if(clip_mode==CLIP_OFF)PRINTF("clipping off\n");
        if(clip_mode==CLIP_BLOCKAGES_DATA)PRINTF("clipping blockages + data\n");
        if(clip_mode==CLIP_BLOCKAGES)PRINTF("clipping blockages\n");
        if(clip_mode==CLIP_DATA)PRINTF("clipping data\n");
      }
      else{
        startup = 0;
      }
      clip_mode_last=clip_mode;
    }
  }
  if(clip_mode==CLIP_OFF){
    SetClipPlanes(NULL,CLIP_OFF);
  }
}

/* ------------------ HandleIso ------------------------ */

void HandleIso(void){
    if(nplot3dloaded>0){
      UpdateShowStep(1-visiso,ISO);
      if(visiso==1){
        UpdateSurface();
        plotstate=STATIC_PLOTS;
      }
    }
    return;
}

/* ------------------ SpecialKeyboardUpCB ------------------------ */

void SpecialKeyboardUpCB(int key, int x, int y){
  resetclock=1;
}

/* ------------------ SpecialKeyboardCB ------------------------ */

void SpecialKeyboardCB(int key, int x, int y){

#define EYE_MODE 0
#define P3_MODE 1
  int keymode=EYE_MODE;

  glutPostRedisplay();

  if(rotation_type==EYE_CENTERED){
    keymode=EYE_MODE;
  }
  else{
    keymode=P3_MODE;
  }

  switch(keymode){
    case P3_MODE:
      HandlePLOT3DKeys(key);
      stept=0;
      break;
    case EYE_MODE:
      HandleMoveKeys(key);
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ HandlePLOT3DKeys ------------------------ */

void HandlePLOT3DKeys(int  key){
  switch(key){
  case GLUT_KEY_LEFT:
    visx_all=1;
    NextXIndex(-1,0);
    iplot_state=XDIR;
    break;
  case GLUT_KEY_RIGHT:
    visx_all=1;
    NextXIndex(1,0);
    iplot_state=XDIR;
    break;
  case GLUT_KEY_DOWN:
    visy_all=1;
    NextYIndex(-1,0);
    iplot_state=YDIR;
    break;
  case GLUT_KEY_UP:
    visy_all=1;
    NextYIndex(1,0);
    iplot_state=YDIR;
    break;
  case GLUT_KEY_PAGE_DOWN:
    visz_all=1;
    NextZIndex(-1,0);
    iplot_state=ZDIR;
    break;
  case GLUT_KEY_PAGE_UP:
    visz_all=1;
    NextZIndex(1,0);
    iplot_state=ZDIR;
    break;
  case GLUT_KEY_HOME:
    switch(iplot_state){
      case 0:
      case XDIR:
        NextXIndex(0,-1);
        break;
      case YDIR:
        NextYIndex(0,-1);
        break;
      case ZDIR:
        NextZIndex(0,-1);
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    break;
  case GLUT_KEY_END:
    switch(iplot_state){
      case 0:
      case XDIR:
        NextXIndex(0,1);
        break;
      case YDIR:
        NextYIndex(0,1);
        break;
      case ZDIR:
        NextZIndex(0,1);
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    break;
  default:
    break;
  }
  if(iplot_state!=0)UpdatePlotSlice(iplot_state);
  if(visx_all==1||visy_all==1||visz_all==1)update_slice2device = 1;
  return;

//  plotstate=GetPlotState(STATIC_PLOTS);

}

/* ------------------ HandleMoveKeys ------------------------ */

void HandleMoveKeys(int  key){
  int state;
  float dx, dy;
  float *azimuth, cos_azimuth, sin_azimuth;
  float *elevation;

  const float INC_ANGLE0=0.1;

  float INC_XY, INC_Z, INC_ANGLE;

  float *eye_xyz;
#define LOOKANGLE_CHANGE 11.25


  eye_xyz = camera_current->eye;

  azimuth=&camera_current->azimuth;
  cos_azimuth=cos(DEG2RAD*(*azimuth));
  sin_azimuth=sin(DEG2RAD*(*azimuth));

  elevation=&camera_current->elevation;

  glui_move_mode=-1;

  INC_XY=SCALE2SMV(meshinfo->cellsize);
  INC_Z=INC_XY;
  INC_ANGLE = 5*INC_ANGLE0;

  state=GLUTGETMODIFIERS();
  switch(state){
  case GLUT_ACTIVE_CTRL:
    key_state = KEY_CTRL;
    break;
  case GLUT_ACTIVE_ALT:
    key_state = KEY_ALT;
    break;
  case GLUT_ACTIVE_SHIFT:
    key_state = KEY_SHIFT;
    break;
  default:
    key_state = KEY_NONE;
    break;
  }
  switch(key){
    case GLUT_KEY_RIGHT:
      switch(key_state){
        case KEY_ALT:
          dx = INC_XY*(cos_azimuth);
          dy = INC_XY*(sin_azimuth);
          GetNewPos(eye_xyz,dx,-dy,0.0,1.0);
          break;
        case KEY_SHIFT:
        case KEY_CTRL:
        default:
          if(key_state==KEY_SHIFT){
            *azimuth += 4.0*INC_ANGLE;
          }
          else{
            *azimuth += INC_ANGLE;
          }
          break;
      }
      break;
    case 256+'d':
      dx = INC_XY*(cos_azimuth);
      dy = INC_XY*(sin_azimuth);
      {
        float local_speed_factor=1.0;

        if(key_state==KEY_SHIFT)local_speed_factor=4.0;
        GetNewPos(eye_xyz,dx,-dy,0.0,local_speed_factor);
      }
      break;
    case GLUT_KEY_LEFT:
      switch(key_state){
        case KEY_ALT:
          dx = INC_XY*(cos_azimuth);
          dy = INC_XY*(sin_azimuth);
          GetNewPos(eye_xyz,-dx,dy,0.0,1.0);
          break;
        case KEY_SHIFT:
        case KEY_CTRL:
        default:
          if(key_state==KEY_SHIFT){
            *azimuth -= 4.0*INC_ANGLE;
          }
          else{
            *azimuth -= INC_ANGLE;
          }
          break;
      }
      break;
    case 256+'a':
      dx = INC_XY*(cos_azimuth);
      dy = INC_XY*(sin_azimuth);
      if(key_state==KEY_SHIFT){
        GetNewPos(eye_xyz,-dx,dy,0.0,4.0);
      }
      else{
        GetNewPos(eye_xyz,-dx,dy,0.0,1.0);
      }
      break;
    case GLUT_KEY_DOWN:
      if(key_state==KEY_ALT){
        eye_xyz[2] -= INC_Z;
      }
      else{
        float local_speed_factor=1.0;

      if(key_state==KEY_SHIFT)local_speed_factor=4.0;
        dx = INC_XY*(sin_azimuth);
        dy = INC_XY*(cos_azimuth);
        GetNewPos(eye_xyz,-dx,-dy,0.0,local_speed_factor);
      }
      break;
    case GLUT_KEY_UP:
      if(key_state==KEY_ALT){
        eye_xyz[2] += INC_Z;
      }
      else{
        float local_speed_factor=1.0;

        if(key_state==KEY_SHIFT)local_speed_factor=4.0;
        dx = INC_XY*(sin_azimuth);
        dy = INC_XY*(cos_azimuth);
        GetNewPos(eye_xyz,dx,dy,0.0,local_speed_factor);
      }
      break;
    case GLUT_KEY_PAGE_UP:
      *elevation += LOOKANGLE_CHANGE;
      break;
    case GLUT_KEY_HOME:
      *elevation=0.0;
      break;
    case GLUT_KEY_INSERT:
    case GLUT_KEY_PAGE_DOWN:
      *elevation-=LOOKANGLE_CHANGE;
      break;
    case GLUT_KEY_END:
      SetViewPoint(RESTORE_EXTERIOR_VIEW);
      break;
    case GLUT_KEY_F4:
      camera_current->view_angle-=LOOKANGLE_CHANGE;
      if(camera_current->view_angle<0.0)camera_current->view_angle+=360.0;
      break;
    case GLUT_KEY_F5:
      camera_current->view_angle=0.0;
      break;
    case GLUT_KEY_F6:
      camera_current->view_angle+=LOOKANGLE_CHANGE;
      if(camera_current->view_angle>360.0)camera_current->view_angle-=360.0;
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  if(rotation_type==EYE_CENTERED){
    eye_xyz0[0]=eye_xyz[0];
    eye_xyz0[1]=eye_xyz[1];
    eye_xyz0[2]=eye_xyz[2];
    UpdateTranslate();
  }
}

/* ------------------ GMod ------------------------ */

float GMod(float x, float y){
  float returnval;

  if(y==0.0)return 0.0;
  returnval = x - (int)(x/y)*y;
  if(returnval<0.0)returnval+=y;
  return returnval;
}

/* ------------------ UpdateFrame ------------------------ */

void UpdateFrame(float thisinterval, int *changetime, int *redisplay){
  int oldcpuframe;
  float totalcpu;
  float elapsed_time;

  if(showtime==1&&((stept==1&&(float)thisinterval>frameinterval)||render_status==RENDER_ON||timebar_drag==1)){       /* ready for a new frame */
    cputimes[cpuframe]=thistime/1000.;

    oldcpuframe=cpuframe-10;
    if(oldcpuframe<0)oldcpuframe+=20;
    totalcpu=cputimes[cpuframe]-cputimes[oldcpuframe];
    if(totalcpu==0.0){
      framerate=0.0;
    }
    else{
     framerate=10.0/totalcpu;
    }
    cpuframe++;
    if(cpuframe>=20)cpuframe=0;

    last_frame_count=frame_count;
    frame_count=1;
    lasttime = thistime;
    if(nglobal_times>0){
      *changetime=1;
      if(stept ==1 && plotstate == DYNAMIC_PLOTS && timebar_drag==0 && render_status==RENDER_OFF){
        /*  skip frames here if displaying in real time and frame rate is too slow*/
        if(global_times!=NULL&&realtime_flag!=0&&FlowDir>0){
          elapsed_time = (float)thistime/1000.0 - reset_time;
          elapsed_time *= (float)realtime_flag;
          elapsed_time += global_times[reset_frame];
          if(nglobal_times>1&&
            global_times[nglobal_times-1]>global_times[0]&&
            (elapsed_time>global_times[nglobal_times-1]||elapsed_time<0.0)
            ){
            elapsed_time = GMod(elapsed_time,global_times[nglobal_times-1]-global_times[0])+global_times[0];
          }
          itimes = ISearch(global_times,nglobal_times,elapsed_time,itimes);
        }
        else{
          if(script_render_flag==0){
            itimes+=FlowDir;
          }
          else{
            itimes=script_itime;
          }
        }
      }
      if(stept==1&&timebar_drag==0&&render_status==RENDER_ON){
        if(render_firsttime==YES){
          render_firsttime = NO;
          itimes = first_frame_index;
        }
        else{
          itimes += render_skip*FlowDir;
        }
      }

// if toggling time display with H then show the frame that was visible

      if(stept==0){
        itime_save=-1;
      }
      else{
        if(itime_save>=0){
          itimes=itime_save;
        }
      }
      if(shooter_firstframe==1&&visShooter!=0&&shooter_active==1){
        ResetItimes0();
      }
      CheckTimeBound();
      UpdateTimeLabels();
    }
    *redisplay=1;
  }
}

/* ------------------ IdleCB ------------------------ */

void IdleCB(void){
  int changetime=0;
  float thisinterval;
  int redisplay=0;

  if(render_status == RENDER_ON && from_DisplayCB==0)return;
  CheckMemory;
  if(use_graphics==1)SetMainWindow();
  UpdateShow();
  START_TICKS(thistime);
  thisinterval = thistime - lasttime;
  frame_count++;

  /* increment frame counter if the proper amount of time has passed
     or if we are rendering images or stepping by hand */

  UpdateFrame(thisinterval,&changetime,&redisplay);
  if(showtime==1&&stept==0&&itimeold!=itimes){
    changetime=1;
    CheckTimeBound();
    UpdateTimeLabels();
  }
  UpdateFrameNumber(changetime);
  if(redisplay==1){
    GLUTPOSTREDISPLAY;
  }
}

/* ------------------ ForceIdle ------------------------ */

void ForceIdle(void){
  force_redisplay = 1;
  IdleCB();
}

/* ------------------ SetScreenSize ------------------------ */

void SetScreenSize(int *width, int *height){
  if(width!=NULL){
    screenWidth = MAX(*width,1);
    screenWidth = MAX(screenWidth, 1);
    if(screenWidth%2==1)screenWidth++;

#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      screenWidth *= 2;
    }
#endif
  }
  if(height!=NULL){
    screenHeight=MAX(*height,1);
#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      screenHeight *= 2;
    }
#endif
  }
  {
    int width_low, height_low, width_high, height_high;

    GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
    UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
  }
}

/* ------------------ AdjustY ------------------------ */

void AdjustY(cameradata *ca){

  if(projection_type!=PROJECTION_PERSPECTIVE||update_saving_viewpoint>0||update_viewpoint_script>0)return;
  switch(selected_view){
    case 0:
    case -1:
      UpdateCameraYpos(ca, 1);
      break;
    case 1:
    case -2:
    case -3:
      UpdateCameraYpos(ca, 2);
      break;
    case -4:
    case -5:
      UpdateCameraYpos(ca, 3);
      break;
    default:
      UpdateCameraYpos(ca, 2);
      break;
  }
  if(selected_view<=1&&selected_view>-5){
    void ResetDefaultMenu(int var);
    ResetDefaultMenu(selected_view);
  }
}

/* ------------------ ReshapeCB ------------------------ */

void ReshapeCB(int width, int height){
  START_TIMER(timer_reshape);
  if(disable_reshape==1)return;
  updatemenu=1;
  if(strcmp(camera_current->name,"external")!=0||in_external==0){
    CopyCamera(camera_save,camera_current);
  }
  SetScreenSize(&width,&height);
  windowresized=1;
  UpdateCameraYpos(camera_external, 2);
  if(strcmp(camera_current->name,"external")==0&&in_external==1){
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
  }
  else{
    CopyCamera(camera_current,camera_save);
  }
  if(current_script_command==NULL)AdjustY(camera_current);
  windowsize_pointer_old = -1;
  UpdateWindowSizeList();
  if(current_script_command==NULL)update_adjust_y = 2;
  update_reshape = 2;
 }

/* ------------------ ResetGLTime ------------------------ */

void ResetGLTime(void){
  if(showtime!=1)return;
  reset_frame=itimes;
  START_TIMER(reset_time);
  if(global_times!=NULL&&nglobal_times>0){
    start_frametime=global_times[0];
    stop_frametime=global_times[nglobal_times-1];
  }
}

/* ------------------ UpdatePlot3dTitle ------------------------ */

void UpdatePlot3dTitle(void){
  int filenum;
  plot3ddata *plot3di;
  meshdata *meshi;
  char title_base[1024];

  GetBaseTitle("Smokeview ", title_base);
  STRCPY(plot3d_title, title_base);
  meshi = current_mesh;
  if(meshi == NULL)meshi = meshinfo;
  filenum = meshi->plot3dfilenum;
  if(filenum != -1){
    plot3di = plot3dinfo + meshi->plot3dfilenum;
    STRCAT(plot3d_title, ", ");
    STRCAT(plot3d_title, plot3di->file);
  }
}

/* ------------------ UpdateCurrentMesh ------------------------ */

void UpdateCurrentMesh(meshdata *meshi){
  current_mesh=meshi;
  loaded_isomesh= GetLoadedIsoMesh();
  UpdateIsoShowLevels();
  UpdatePlot3dTitle();
}

/* ------------------ ClearBuffers ------------------------ */

void ClearBuffers(int mode){
  if(mode==DRAWSCENE){
    glClearColor(backgroundcolor[0],backgroundcolor[1],backgroundcolor[2], 0.0f);
  }
  else{
    glClearColor((float)0.0,(float)0.0,(float)0.0, (float)0.0);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/* ------------------ DoStereo ------------------------ */

void DoStereo(void){
  if(stereotype==STEREO_TIME&&videoSTEREO==1){  // temporal stereo (shuttered glasses)
    glDrawBuffer(GL_BACK_LEFT);
    if(stereotype_frame==LEFT_EYE||stereotype_frame==BOTH_EYES){
      ShowScene(DRAWSCENE,VIEW_LEFT,0,0,0,NULL);
    }
    Render(VIEW_LEFT);
    glDrawBuffer(GL_BACK_RIGHT);
    if(stereotype_frame==RIGHT_EYE||stereotype_frame==BOTH_EYES){
      ShowScene(DRAWSCENE,VIEW_RIGHT,0,0,0,NULL);
    }
    Render(VIEW_RIGHT);
    if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
  }
  else if(stereotype==STEREO_LR){             // left/right stereo
    int i;
    int nscreens;

    glDrawBuffer(GL_BACK);
    ClearBuffers(DRAWSCENE);

    nscreens = 1;
    if(render_mode == RENDER_360&&render_status==RENDER_ON){
      nscreens = nscreeninfo;
      if(screeninfo == NULL || update_screeninfo == 1)SetupScreeninfo();
    }

    for(i = 0; i < nscreens; i++){
      screendata *screeni;

      screeni = NULL;
      if(render_mode == RENDER_360 && render_status == RENDER_ON)screeni = screeninfo + i;
      if(stereotype_frame==LEFT_EYE||stereotype_frame==BOTH_EYES){
        int screenWidth_save;

        screenWidth_save=screenWidth;
        screenWidth/=2;
        screenWidth = MAX(screenWidth, 1);
        ShowScene(DRAWSCENE,VIEW_LEFT,0,0,0,screeni);
        screenWidth=screenWidth_save;
      }
      if(stereotype_frame==RIGHT_EYE||stereotype_frame==BOTH_EYES){
        int screenWidth_save;

        screenWidth_save=screenWidth;
        screenWidth/=2;
        screenWidth = MAX(screenWidth, 1);
        ShowScene(DRAWSCENE,VIEW_RIGHT,0,screenWidth,0,screeni);
        screenWidth=screenWidth_save;
        screenWidth = MAX(screenWidth, 1);
      }
      if(render_mode == RENDER_360 && render_status == RENDER_ON)screeni->screenbuffer = GetScreenBuffer();
      if(buffertype == DOUBLE_BUFFER)glutSwapBuffers();
    }
    if(render_status == RENDER_ON){
      if(render_mode == RENDER_360){
        MergeRenderScreenBuffers360();
        for(i = 0; i < nscreeninfo; i++){
          screendata *screeni;

          screeni = screeninfo + i;
          FREEMEMORY(screeni->screenbuffer);
        }
      }
      else{
        Render(VIEW_CENTER);
      }
    }
  }
  else if(stereotype==STEREO_RB){             // red/blue stereo
    glDrawBuffer(GL_BACK);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(stereotype_frame==LEFT_EYE||stereotype_frame==BOTH_EYES){
      glColorMask(GL_TRUE,GL_FALSE,GL_FALSE, GL_TRUE);
      ShowScene(DRAWSCENE,VIEW_LEFT,0,0,0,NULL);
      glFlush();
    }

    if(stereotype_frame==RIGHT_EYE||stereotype_frame==BOTH_EYES){
      glDrawBuffer(GL_BACK);
      glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_TRUE);
      glClearColor(0.0, 0.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

      ShowScene(DRAWSCENE,VIEW_RIGHT,0,0,0,NULL);
      glFlush();
    }
    Render(VIEW_CENTER);
    if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
  }
  else if(stereotype==STEREO_RC){             // red/cyan stereo
    glDrawBuffer(GL_BACK);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(stereotype_frame==LEFT_EYE||stereotype_frame==BOTH_EYES){
      glColorMask(GL_TRUE,GL_FALSE,GL_FALSE, GL_TRUE);
      ShowScene(DRAWSCENE,VIEW_LEFT,0,0,0,NULL);
      glFlush();
    }

    if(stereotype_frame==RIGHT_EYE||stereotype_frame==BOTH_EYES){
      glDrawBuffer(GL_BACK);
      glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE);
      glClearColor(0.0, 1.0, 1.0, 0.0);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

      ShowScene(DRAWSCENE,VIEW_RIGHT,0,0,0,NULL);
      glFlush();
    }
    Render(VIEW_CENTER);
    if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
  }
  else if(stereotype==STEREO_CUSTOM){             // custom red/blue stereo
    glDrawBuffer(GL_BACK);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(stereotype_frame==LEFT_EYE||stereotype_frame==BOTH_EYES){
      glColorMask(GL_TRUE,GL_FALSE,GL_FALSE, GL_TRUE);
      ShowScene(DRAWSCENE,VIEW_LEFT,0,0,0,NULL);
      glFlush();
    }
    if(stereotype_frame==RIGHT_EYE||stereotype_frame==BOTH_EYES){
      glDrawBuffer(GL_BACK);
      glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_TRUE);
      glClearColor(0.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

      ShowScene(DRAWSCENE,VIEW_RIGHT,0,0,0,NULL);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glEnable(GL_BLEND);
      DISABLE_LIGHTING;
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_DITHER);

      glBlendFunc(GL_DST_COLOR,GL_ZERO);
      glBegin(GL_QUADS);
      glColor4f(0.0,right_green,right_blue,1.0);
      glVertex3f(-1.0,-1.0,0.1);
      glVertex3f(1.0,-1.0,0.1);
      glVertex3f(1.0,1.0,0.1);
      glVertex3f(-1.0,1.0,0.1);
      glEnd();

      glFlush();
    }
    Render(VIEW_CENTER);
    if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
    ENABLE_LIGHTING;
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DITHER);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  }
}

#ifdef pp_LUA
/* ------------------ DoScriptLua ------------------------ */
void DoScriptLua(void){
  int script_return_code;
  if(runluascript == 1){
    if(!luascript_loaded && strlen(luascript_filename)>0)
      load_script(luascript_filename);
    runluascript = 0;
    PRINTF("running lua script section\n");
    fflush(stdout);
    script_return_code = runLuaScript();
    if(script_return_code != LUA_OK && script_return_code != LUA_YIELD && exit_on_script_crash){
        SMV_EXIT(1);
    }
  }
}
#endif

/* ------------------ DoScript ------------------------ */
#ifdef pp_LUA_SSF
void DoScript(void){
  int script_return_code;

  if(runscript == 1){
      runscript = 0;
      PRINTF("running ssf script instruction\n");
      fflush(stdout);
      script_return_code = runSSFScript();
      if(script_return_code != LUA_OK && script_return_code != LUA_YIELD && exit_on_script_crash){
          SM_EXIT(1);
      }
    }
}
#else
void DoScript(void){
  SNIFF_ERRORS("DoScript: start");
  if(runscript==1&&default_script!=NULL){
    ScriptMenu(default_script->id);
    runscript=2;
  }
  script_render_flag=0;
  if(nscriptinfo>0&&current_script_command!=NULL&&(script_step==0||(script_step==1&&script_step_now==1))){
    script_step_now=0;
#ifndef WIN32
    if(FILE_EXISTS(stop_filename)==YES){
      fprintf(stderr,"*** Warning: stop file found.  Remove before running smokeview script\n");
      SMV_EXIT(0);
    }
#endif
    if(current_script_command>=scriptinfo){
      if(current_script_command->command==SCRIPT_VOLSMOKERENDERALL){
        if(current_script_command->exit==0){
          RenderState(RENDER_ON);
        }
        else{
          RenderState(RENDER_OFF);
          current_script_command->first = 1;
          current_script_command->exit = 0;
        }
      }
      else if(current_script_command->command==SCRIPT_ISORENDERALL){
          if(current_script_command->exit==0){
            RenderState(RENDER_ON);
          }
          else{
            RenderState(RENDER_OFF);
            current_script_command->first = 1;
            current_script_command->exit = 0;
          }
      }
      else if(current_script_command->command==SCRIPT_LOADSLICERENDER){
        if(current_script_command->exit==0){
          if(render_resolution==RENDER_RESOLUTION_360){
            if(viewpoint_script_ptr!=NULL)SetCurrentViewPoint(viewpoint_script);
            render_size_index=RenderWindow;
            resolution_multiplier = 1;
            RenderCB(RENDER_RESOLUTION);
            RenderCB(RENDER_START_360);
          }
          RenderState(RENDER_ON);
          ScriptLoadSliceRender(current_script_command);
        }
        else{
          RenderState(RENDER_OFF);
          current_script_command->first = 1;
          current_script_command->exit = 0;
        }
      }
    }
    int advance_script = 0;
    if(current_script_command>=scriptinfo&&current_script_command->command==SCRIPT_SETVIEWPOINT&&update_viewpoint_script>0){
      advance_script=0;
    }
    else{
      advance_script = 1;
      if(render_status!=RENDER_OFF)advance_script = 0;
    }
#define NREPEATS 2
    if(nrenderonce>=NREPEATS){
      nrenderonce = 0;
    }

    if(advance_script==1){                        // don't advance command if Smokeview is executing a RENDERALL command
      if(nrenderonce==0){
        current_script_command++; // force RENDERONCE to be run twice
      }
      script_render_flag= RunScriptCommand(current_script_command);
      if(runscript==2&&noexit==0&&current_script_command==NULL){
        SMV_EXIT(0);
      }
      if(current_script_command==NULL){
        GluiScriptEnable();
      }
    }
    else{
      if(current_script_command->command==SCRIPT_VOLSMOKERENDERALL){
        int remove_frame;

        ScriptLoadVolSmokeFrame2();
        remove_frame=current_script_command->remove_frame;
        if(remove_frame>=0){
          UnloadVolsmokeFrameAllMeshes(remove_frame);
        }
      }
      else if(current_script_command->command==SCRIPT_ISORENDERALL){
        int remove_frame;

        ScriptLoadIsoFrame2(current_script_command);
        remove_frame = current_script_command->remove_frame;
        if(remove_frame>=0){
          //UnloadVolsmokeFrameAllMeshes(remove_frame);
        }
      }
    }
    if(current_script_command!=NULL&&current_script_command->command==SCRIPT_RENDERONCE){
      nrenderonce++;
    }
    glutPostRedisplay();
  }
  else{
    first_frame_index=0;
    script_startframe=-1;
    script_skipframe=-1;
  }
}
#endif

/* ------------------ DoScriptHtml ------------------------ */

void DoScriptHtml(void){
  int i;

  CompileScript(default_script->file);
  for(i=0;i<nscriptinfo;i++){
    scriptdata *scripti;

    scripti = scriptinfo + i;
    if(scripti->need_graphics==1){
      printf("***warning: script command, %s, requires graphics\n", scripti->command_label);
      continue;
    }
    RunScriptCommand(scripti);
  }
}

/* ------------------ IdleDisplay ------------------------ */

void IdleDisplay(void){
// when rendering files, onlyl call Idle routine from DisplayCB callback
  from_DisplayCB=1;
  IdleCB();
  from_DisplayCB=0;
}

/* ------------------ DoNonStereo ------------------------ */

void DoNonStereo(void){
  if(render_status==RENDER_OFF){
    glDrawBuffer(GL_BACK);
    ShowScene(DRAWSCENE, VIEW_CENTER, 0, 0, 0, NULL);
    if(update_rgb_test==1){
      update_rgb_test = 0;
      RGBTest();
    }
    if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
  }
  else{
    int stop_rendering;

    IdleDisplay();

    stop_rendering = 1;
    if(plotstate==DYNAMIC_PLOTS && nglobal_times>0){
      if(itimes>=0&&itimes<nglobal_times&&
        ((render_frame[itimes]==0&&stereotype==STEREO_NONE)||(render_frame[itimes]<2&&stereotype!=STEREO_NONE))
        ){
        render_frame[itimes]++;
        stop_rendering = 0;
      }
    }
    if(render_mode==RENDER_NORMAL){
      int i, ibuffer = 0;
      GLubyte **screenbuffers;

      NewMemory((void **)&screenbuffers, resolution_multiplier*resolution_multiplier*sizeof(GLubyte *));

      glDrawBuffer(GL_BACK);

      for(i = 0; i<resolution_multiplier; i++){
        int j;

        for(j = 0; j<resolution_multiplier; j++){
          ShowScene(DRAWSCENE, VIEW_CENTER, 1, j*screenWidth, i*screenHeight, NULL);
          screenbuffers[ibuffer++] = GetScreenBuffer();
          if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
        }
      }

      MergeRenderScreenBuffers(resolution_multiplier, screenbuffers);

      for(i = 0; i<resolution_multiplier*resolution_multiplier; i++){
        FREEMEMORY(screenbuffers[i]);
      }
      FREEMEMORY(screenbuffers);
    }
    if(render_mode==RENDER_360){
      int i;

      glDrawBuffer(GL_BACK);

      if(screeninfo==NULL||update_screeninfo==1)SetupScreeninfo();

      for(i = 0; i<nscreeninfo; i++){
        screendata *screeni;

        screeni = screeninfo+i;
        ShowScene(DRAWSCENE, VIEW_CENTER, 0, 0, 0, screeni);
        screeni->screenbuffer = GetScreenBuffer();
        if(buffertype==DOUBLE_BUFFER)glutSwapBuffers();
      }
      MergeRenderScreenBuffers360();

      for(i = 0; i<nscreeninfo; i++){
        screendata *screeni;

        screeni = screeninfo+i;
        FREEMEMORY(screeni->screenbuffer);
      }
    }
    if(stop_rendering==1){
      ASSERT(render_skip>0);
      RenderState(RENDER_OFF);
    }
  }
}

/* ------------------ DisplayCB ------------------------ */

void DisplayCB(void){
  SNIFF_ERRORS("DisplayDB: start");
  DoScript();
#ifdef pp_LUA
  DoScriptLua();
#endif
  UpdateDisplay();
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  if(stereotype==STEREO_NONE){
    if(use_vr==0){
      DoNonStereo();
    }
  }
  else{
    DoStereo();
  }
}

/* ------------------ SetMainWindow ------------------------ */

void SetMainWindow(void){
  glutSetWindow(mainwindow_id);
  GLUTPOSTREDISPLAY;
}

/* ------------------ ResizeWindow ------------------------ */

void ResizeWindow(int width, int height){
  float wscaled, hscaled;

  if(render_mode == RENDER_360&&render_status==RENDER_ON)return;
  SetMainWindow();
  wscaled = (float)width/(float)max_screenWidth;
  hscaled = (float)height/(float)max_screenHeight;
  if(wscaled>1.0||hscaled>1.0){
    if(wscaled>hscaled){
      width/=wscaled;
      height/=wscaled;
    }
    else{
      width/=hscaled;
      height/=hscaled;
    }
  }
  glutReshapeWindow(width,height);
  glutPostRedisplay();
}
