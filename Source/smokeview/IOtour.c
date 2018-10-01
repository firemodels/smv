#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "update.h"
#include "smokeviewvars.h"
#include "IOobjects.h"

/* ------------------ FreeTours ------------------------ */

void FreeTours(void){
  int i;

  if(ntourinfo>0){
    for(i=0;i<ntourinfo;i++){
      tourdata *touri;

      touri = tourinfo + i;
      FreeTour(touri);
    }
    FREEMEMORY(tourinfo);
  }
  ntourinfo=0;
}

  /* ------------------ FreeTour ------------------------ */

void FreeTour(tourdata *touri){
  int i;
  keyframe *framei;

  for(i=0;i<touri->nkeyframes;i++){
    framei = touri->keyframe_list[i];
    FREEMEMORY(framei);
  }
  FREEMEMORY(touri->pathnodes);
  FREEMEMORY(touri->keyframe_list);
  FREEMEMORY(touri->keyframe_times);
  FREEMEMORY(touri->timeslist);
  FREEMEMORY(touri->path_times);
}

/* ------------------ InitTour ------------------------ */

void InitTour(tourdata *touri){
  touri->glui_avatar_index=0;
  touri->display2=0;
  touri->display=0;
  touri->periodic=0;
  touri->first_frame.prev=NULL;
  touri->first_frame.next=&touri->last_frame;
  touri->first_frame.noncon_time=-1000000000.0;
  touri->first_frame.disp_time=touri->first_frame.noncon_time;

  touri->last_frame.prev=&touri->first_frame;
  touri->last_frame.next=NULL;
  touri->last_frame.noncon_time=1000000000.0;
  touri->last_frame.disp_time=touri->last_frame.noncon_time;

  touri->nkeyframes=0;
  touri->ntimes=view_ntimes;
  touri->pathnodes=NULL;
  touri->keyframe_times=NULL;
  touri->keyframe_list=NULL;
  touri->timeslist=NULL;
  touri->path_times=NULL;

  touri->global_dist=0.0;
  touri->local_dist=0.0;
  touri->startup=0;
  touri->isDefault=0;
}

/* ------------------ UpdateViewTour ------------------------ */

void UpdateViewTour(void){
  int i;
  tourdata *touri;

  viewalltours=1;
  viewanytours=0;
  for(i=0;i<ntourinfo;i++){
    touri = tourinfo + i;
    if(touri->display==0){
      viewalltours=0;
    }
    else{
      viewanytours++;
    }
  }
}

/* ------------------ UpdateTourMenulabels ------------------------ */

void UpdateTourMenuLabels(void){
  int i;
  tourdata *touri;

  if(ntourinfo>0){
    for(i=0;i<ntourinfo;i++){
      touri = tourinfo + i;
      STRCPY(touri->menulabel,touri->label);
    }
  }
  updatemenu=1;
}

/* ------------------ drawcir ------------------------ */

void DrawCir(float *center, float rad, float *color){
  glColor3fv(color);
  glBegin(GL_QUADS);
  glVertex3f(center[0] - rad / 2.0, center[1] - rad / 2.0, center[2]);
  glVertex3f(center[0] + rad / 2.0, center[1] - rad / 2.0, center[2]);
  glVertex3f(center[0] + rad / 2.0, center[1] + rad / 2.0, center[2]);
  glVertex3f(center[0] - rad / 2.0, center[1] + rad / 2.0, center[2]);
  glEnd();
}

/* ------------------ DrawTours ------------------------ */

void DrawTours(void){
  int i;
  float *tmp_tourcol_text;
  float *tmp_tourcol_pathline;
  float *tmp_tourcol_pathknots;

  tmp_tourcol_text=tourcol_text;
  if(tourcol_text[0]<0.0)tmp_tourcol_text=foregroundcolor;

  tmp_tourcol_pathline=tourcol_pathline;
  if(tourcol_pathline[0]<0.0)tmp_tourcol_pathline=foregroundcolor;

  tmp_tourcol_pathknots=tourcol_pathknots;
  if(tourcol_pathknots[0]<0.0)tmp_tourcol_pathknots=foregroundcolor;

  if(selected_frame!=NULL)selectedtour_index = selected_tour-tourinfo;

  if(edittour==1){
    tourdata *tour_sel;

    /* path line (non-selected)*/

    if(showtours_whenediting==1){
      AntiAliasLine(ON);
      glColor3fv(tmp_tourcol_pathline);
      glBegin(GL_LINES);
      for(i=0;i<ntourinfo;i++){
        tourdata *touri;
        int j;

        touri = tourinfo + i;
        if(touri->display==0||touri->nkeyframes<=1||selectedtour_index==i)continue;

        for(j=0;j<view_ntimes-1;j++){
          pathdata *pj;

          pj = touri->pathnodes + j;
          glVertex3fv(pj->eye);

          pj++;
          glVertex3fv(pj->eye);
        }
      }
     glEnd();
     AntiAliasLine(OFF);
    }

    /* path line (selected)*/

    tour_sel = tourinfo + selectedtour_index;
    if(selectedtour_index!=TOURINDEX_MANUAL&&tour_sel->display==1&&tour_sel->nkeyframes>1){
      int j;
      tourdata *touri;

      glColor3fv(tourcol_selectedpathline);
      if(tour_antialias==1)AntiAliasLine(ON);
      glBegin(GL_LINES);
      touri = tourinfo + selectedtour_index;

      for(j=0;j<view_ntimes-1;j++){
        pathdata *pj;

        pj = touri->pathnodes + j;
        glVertex3fv(pj->eye);

        pj++;
        glVertex3fv(pj->eye);
      }
      glEnd();
      if(tour_antialias==1)AntiAliasLine(OFF);

    }

    /* path knots */

    if(show_path_knots==1&&tour_sel->nkeyframes>1){
      glColor3f(0.0f,0.0f,1.0f);
      glPointSize(5.0f);
      glBegin(GL_POINTS);
      for(i=0;i<ntourinfo;i++){
        int j;
        tourdata *touri;

        touri = tourinfo + i;
        if(touri->display==0||selectedtour_index!=i)continue;

        for(j=0;j<view_ntimes;j++){
          pathdata *pj;

          pj = touri->pathnodes + j;
          glVertex3fv(pj->eye);
        }
      }
      glEnd();

    }

    /* selected path - non-selected keyframe knots */

    if(selectedtour_index!=TOURINDEX_MANUAL&&selected_tour->display==1){
      int j;

      glColor3fv(tourcol_selectedpathlineknots);
      glPointSize(10.0f);
      glBegin(GL_POINTS);
      for(j=0;j<tour_sel->nkeyframes;j++){
        keyframe *framej;

        framej = tour_sel->keyframe_list[j];
        if(framej->selected==1)continue;
        glVertex3fv(framej->nodeval.eye);
      }
      glEnd();
    }

    /* non-selected path, keyframe knots */

    if(showtours_whenediting==1){
      glColor3fv(tmp_tourcol_pathknots);
      glPointSize(10.0f);
      glBegin(GL_POINTS);
      for(i=0;i<ntourinfo;i++){
        int j;
        tourdata *touri;

        touri = tourinfo + i;
        if(touri->display==0||i==selectedtour_index)continue;

        for(j=0;j<touri->nkeyframes;j++){
          keyframe *framej;

          framej = touri->keyframe_list[j];
          if(framej->selected==1)continue;
          glVertex3fv(framej->nodeval.eye);
        }
      }
      glEnd();
    }

    /* selected path, selected keyframe */

    if(selected_frame!=NULL&&selected_tour->display==1){
      glBegin(GL_POINTS);
      glColor3fv(tourcol_selectedknot);
      glVertex3fv(selected_frame->nodeval.eye);
      if(selected_frame->viewtype==ABS_VIEW){
        glColor3fv(tourcol_selectedview);
        glVertex3fv(selected_frame->nodeval.xyz_view_abs);
      }
      glEnd();
      if(show_tour_hint==1){
        float *xyz;

        xyz = selected_frame->nodeval.eye;
        Output3Text(foregroundcolor,xyz[0]+0.1,xyz[1]+0.1,xyz[2]+0.1,"click and drag to change tour node");
      }
    }

    /* keyframe times */
    SNIFF_ERRORS("after select path, selected keyframe");
    CheckMemory;

    if(fontindex==SCALED_FONT)ScaleFont3D();
    for(i=0;i<ntourinfo;i++){
      int j;
      tourdata *touri;

      touri = tourinfo + i;
      if(touri->display==0)continue;
      if(showtours_whenediting==0&&selectedtour_index!=i)continue;

      for(j=0;j<touri->nkeyframes;j++){
        keyframe *framej;
        float *eye;

        framej = touri->keyframe_list[j];
        eye = framej->nodeval.eye;
        {
          char label[128];
          sprintf(label,"%8.2f",framej->disp_time+0.005);
          TrimZeros(label);
          Output3Text(tmp_tourcol_text,eye[0]+0.02f,eye[1]+0.015f,eye[2]+0.015f,label);
        }
      }
    }
  }

    /* keyframe avatar */

  //show_tourlocus=1;
  //tourlocus_type=2;
  if(show_tourlocus==1){
    switch(tourlocus_type){
      case 0:
        AntiAliasLine(ON);
        glBegin(GL_LINES);
        glColor3fv(tourcol_avatar);
        for(i=0;i<ntourinfo;i++){
          tourdata *touri;
          pathdata *pj;
          int iframe_local;

          touri = tourinfo + i;
          if(touri->display==0||touri->nkeyframes<=1)continue;
          if(touri->timeslist==NULL)continue;

          iframe_local = touri->timeslist[itimes];
          pj = touri->pathnodes + iframe_local;
          if(keyframe_snap==1)pj = pj->keysnap;

          glVertex3fv(pj->eye);
          glVertex3f(pj->eye[0],pj->eye[1],pj->eye[2]+0.1);

          glVertex3fv(pj->eye);
          glVertex3fv(pj->tour_view);
        }
        glEnd();
        AntiAliasLine(OFF);
        break;
      case 1:
        for(i=0;i<ntourinfo;i++){
          tourdata *touri;
          pathdata *pj;
          int iframe_local;

          touri = tourinfo + i;
          if(touri->display==0||touri->nkeyframes<=1)continue;
          if(touri->timeslist==NULL)continue;

          iframe_local = touri->timeslist[itimes];
          pj = touri->pathnodes + iframe_local;
          if(keyframe_snap==1)pj = pj->keysnap;


          DrawCir(pj->eye,tourrad_avatar,tourcol_avatar);

        }
        break;
      case 2:
        for(i=0;i<ntourinfo;i++){
          tourdata *touri;
          float *tour_view, az_angle, dxy[2];
          pathdata *pj;
          float *eye;
          int iframe_local;

          touri = tourinfo + i;
          if(touri->display==0||touri->nkeyframes<=1)continue;
          if(touri->timeslist==NULL)continue;

          iframe_local = touri->timeslist[itimes];
          pj = touri->pathnodes + iframe_local;
          if(keyframe_snap==1)pj = pj->keysnap;
          eye = pj->eye;
          tour_view = pj->tour_view;
          dxy[0]=tour_view[0]-eye[0];
          dxy[1]=tour_view[1]-eye[1];
          if(dxy[0]!=0.0||dxy[1]!=0.0){
            az_angle=atan2(dxy[1],dxy[0])*RAD2DEG;
          }
          else{
            az_angle=0.0;
          }

          glPushMatrix();
          glTranslatef(eye[0],eye[1],eye[2]);
          glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));

          glRotatef(az_angle,0.0,0.0,1.0);

          DrawSmvObject(avatar_types[touri->glui_avatar_index],0,NULL,0,NULL,0);
          glPopMatrix();
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
  }
}

/* ------------------ DrawSelectTours ------------------------ */

void DrawSelectTours(void){
  int i,j;
  tourdata *touri;
  keyframe *framej;
  int color_index=0;
  float *eye;

  glPointSize(20.0f);
  glBegin(GL_POINTS);
  for(i=0;i<ntourinfo;i++){
    touri = tourinfo + i;
    for(j=0;j<touri->nkeyframes;j++){

      if(showtours_whenediting==1||selectedtour_index==i){

        if(touri->display==1){
          unsigned char r, g, b;

          framej = touri->keyframe_list[j];
          eye = framej->nodeval.eye;

          GetRGB(color_index+1,&r,&g,&b);
          glColor3ub(r,g,b);
          glVertex3fv(eye);
        }
      }
      color_index++;
    }
  }
  glEnd();
}

#define HERMVAL() ((2.0*t3-3.0*t2+1.0)*p0 + (t3-2.0*t2+t)*m0 + (t3-t2)*m1 + (-2.0*t3+3.0*t2)*p1)
#define HERMDERIV() ((6.0*t2-6.0*t)*p0 + (3.0*t2-4.0*t+1.0)*m0 + (3.0*t2-2.0*t)*m1 + (-6.0*t2+6.0*t)*p1)

/* ------------------ HermiteEye ------------------------ */

void HermiteEye(float t, keyframe *kf1, keyframe *kf2, float *eye, float *slope){
  int i;
  float t3, t2;

  t2 = t*t;
  t3 = t2*t;

  for(i = 0;i < 3;i++){
    float p0, p1, m0, m1;

    p0 = kf1->nodeval.eye[i];
    p1 = kf2->nodeval.eye[i];
    m0 = kf1->d_eye[i];
    m1 = kf2->s_eye[i];

    eye[i] = HERMVAL();
    if(i != 2)slope[i] = HERMDERIV();
  }
}

/* ------------------ HermiteOther ------------------------ */

void HermiteOther(float t, keyframe *kf1, keyframe *kf2, pathdata *pj){
  float p0, p1, m0, m1;
  float t3, t2;

  t2 = t*t;
  t3 = t2*t;

  p0 = kf1->az_path;
  p1 = kf2->az_path;
  m0 = kf1->d_az;
  m1 = kf2->s_az;
  pj->az_path = HERMVAL();

  p0 = kf1->nodeval.zoom;
  p1 = kf2->nodeval.zoom;
  m0 = kf1->d_zoom;
  m1 = kf2->s_zoom;
  pj->zoom = HERMVAL();

  p0 = kf1->nodeval.elev_path;
  p1 = kf2->nodeval.elev_path;
  m0 = kf1->d_elev;
  m1 = kf2->s_elev;
  pj->elev_path = HERMVAL();
}

/* ------------------ HermiteView ------------------------ */

void HermiteView(float t, keyframe *kf1, keyframe *kf2, float *view){
  int i;

  for(i = 0;i < 3;i++){
    float p0, p1, m0, m1;
    float t3, t2;

    p0 = kf1->nodeval.xyz_view_abs[i];
    p1 = kf2->nodeval.xyz_view_abs[i];
    m0 = kf1->d_xyz_view[i];
    m1 = kf2->s_xyz_view[i];
    t2 = t*t;
    t3 = t2*t;
    view[i] = HERMVAL();
  }
}

/* ------------------ CreateTourPaths ------------------------ */

void CreateTourPaths(void){
  int i;
  keyframe **tourknotskeylist_copy;
  tourdata **tourknotstourlist_copy;

  // construct keyframe list for selecting keyframes

#ifdef _DEBUG
  printf("updating tour path\n");
#endif
  ntourknots=0;
  if(ntourinfo==0)return;

  for(i=0;i<ntourinfo;i++){
    tourdata *touri;

    touri = tourinfo + i;
    AdjustTourTimes(touri);
  }

  for(i=0;i<ntourinfo;i++){
    tourdata *touri;
    keyframe *keyj;
    int nframes;

    touri = tourinfo + i;
    nframes=0;
    for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
      keyj->nodeval.time=keyj->noncon_time;
      keyj->disp_time=keyj->noncon_time;
      ntourknots++;
      nframes++;
    }
    touri->nkeyframes=nframes;

  }
  FREEMEMORY(tourknotskeylist);
  FREEMEMORY(tourknotstourlist);
  NewMemory((void **)&(tourknotskeylist),ntourknots*sizeof(keyframe*));
  NewMemory((void **)&(tourknotstourlist),ntourknots*sizeof(keyframe*));

  // construct spline slopes for each keyframe interval and each quantity being interpolated

  tourknotskeylist_copy=tourknotskeylist;
  tourknotstourlist_copy=tourknotstourlist;
  for(i=0;i<ntourinfo;i++){
    tourdata *touri;
    keyframe *keyj;
    keyframe *kf1, *kf2;
    float *tour_dist3a;
    float factor,total_time;
    float total_distance;
    float vdist,vtime2,vdt;
    float tour_tstart, tour_tstop;
    int j,jj;
    int iframe_local;
    int ntotal,ntotal2;
    int iframe_old, iframe_new;

    touri = tourinfo + i;
    if(viewalltours==1)touri->display=1;
    FREEMEMORY(touri->keyframe_list);
    NewMemory((void **)&(touri->keyframe_list),touri->nkeyframes*sizeof(keyframe*));
    FREEMEMORY(touri->keyframe_times);
    NewMemory((void **)&(touri->keyframe_times),touri->nkeyframes*sizeof(float));
    for(keyj=(touri->first_frame).next,j=0;keyj->next!=NULL;keyj=keyj->next,j++){
      touri->keyframe_list[j]=keyj;
      touri->keyframe_times[j]=keyj->nodeval.time;
    }
    if(touri->nkeyframes<=1)continue;
    for(keyj=(touri->first_frame).next,j=0;keyj->next!=NULL;keyj=keyj->next,j++){
      keyframe *lastkey, *thiskey, *nextkey;
      float *lasteye, *thiseye, *nexteye;
      float *xyz_view0, *xyz_view1, *xyz_view2;

      *tourknotskeylist_copy++ = keyj;
      *tourknotstourlist_copy++ = touri;
      keyj->selected=0;
      keyj->distance=0.0;
      keyj->npoints=0;

      lastkey=keyj->prev;
      thiskey=keyj;
      nextkey=keyj->next;
      if(touri->periodic==1){
        if(j==0){
          lastkey=touri->keyframe_list[touri->nkeyframes-2];
        }
        if(j==touri->nkeyframes-1){
          nextkey=touri->keyframe_list[1];
        }
      }

      lasteye = lastkey->nodeval.eye;
      thiseye = thiskey->nodeval.eye;
      nexteye = nextkey->nodeval.eye;
      xyz_view0 = lastkey->nodeval.xyz_view_abs;
      xyz_view1 = thiskey->nodeval.xyz_view_abs;
      xyz_view2 = nextkey->nodeval.xyz_view_abs;

      if(touri->periodic==0&&j==0){
        VEC3EQCONS(keyj->s_eye,0.0);

        keyj->s_az=0.0;
        keyj->s_elev=0.0;
        keyj->s_zoom=0.0;

        VEC3DIFF(keyj->d_eye,nexteye,thiseye);

        keyj->d_az=nextkey->az_path - thiskey->az_path;
        keyj->d_zoom=nextkey->nodeval.zoom - thiskey->nodeval.zoom;
        keyj->d_elev=nextkey->nodeval.elev_path - thiskey->nodeval.elev_path;

        VEC3EQCONS(keyj->s_xyz_view,0.0);

        VEC3DIFF(keyj->d_xyz_view,xyz_view2,xyz_view1);
      }
      else if(touri->periodic==0&&j==touri->nkeyframes-1){
        VEC3DIFF(keyj->s_eye,thiseye,lasteye);

        keyj->s_az  =thiskey->az_path           - lastkey->az_path;
        keyj->s_zoom=thiskey->nodeval.zoom      - lastkey->nodeval.zoom;
        keyj->s_elev=thiskey->nodeval.elev_path - lastkey->nodeval.elev_path;

        VEC3EQCONS(keyj->d_eye,0.0);

        keyj->d_az=0.0;
        keyj->d_zoom=0.0;
        keyj->d_elev=0.0;

        VEC3DIFF(keyj->s_xyz_view,xyz_view1,xyz_view0);

        VEC3EQCONS(keyj->d_xyz_view,0.0);
      }
      else{
        float sfactor, dfactor;

        sfactor = 1.0;
        dfactor = 1.0;

#define HERM1(sfactor,lastval,nextval,val)\
        val[0]=sfactor*(nextval[0] - lastval[0])/2.0;\
        val[1]=sfactor*(nextval[1] - lastval[1])/2.0;\
        val[2]=sfactor*(nextval[2] - lastval[2])/2.0

        HERM1(sfactor,lasteye,nexteye,keyj->s_eye);
        keyj->s_az  =sfactor*(nextkey->az_path - lastkey->az_path)/2.0;
        keyj->s_zoom=sfactor*(nextkey->nodeval.zoom -      lastkey->nodeval.zoom)/2.0;
        keyj->s_elev=sfactor*(nextkey->nodeval.elev_path - lastkey->nodeval.elev_path)/2.0;
        HERM1(sfactor,xyz_view0,xyz_view2,keyj->s_xyz_view);

        HERM1(dfactor,lasteye,nexteye,keyj->d_eye);
        keyj->d_az  =dfactor*(nextkey->az_path - lastkey->az_path)/2.0;
        keyj->d_zoom=dfactor*(nextkey->nodeval.zoom - lastkey->nodeval.zoom)/2.0;
        keyj->d_elev=dfactor*(nextkey->nodeval.elev_path - lastkey->nodeval.elev_path)/2.0;
        HERM1(dfactor,xyz_view0,xyz_view2,keyj->d_xyz_view);
      }
    }

    for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
      float denom;

      keyj->keyview_xyz[0]=keyj->d_eye[0];
      keyj->keyview_xyz[1]=keyj->d_eye[1];
      keyj->keyview_xyz[2]=0.0;
      if(keyj->viewtype==ABS_VIEW)XYZView2AzElev(keyj);

      ROTATE(keyj->keyview_xyz2,keyj->keyview_xyz,keyj->az_path*DEG2RAD);
      keyj->keyview_xyz2[2]=0.0;
      denom=NORM2(keyj->keyview_xyz2);
      if(denom==0.0)continue;
      VEC2MA(keyj->keyview_xyz2,10000.0/denom);
    }

    // evaluate quantities along path - determine distances
    // define tour_t and tour_dist (tour_t is uniform )

    iframe_local=0;
    tour_dist[0]=0.0;
    for(j=0;j<view_ntimes;j++){
      pathdata *pj,*pjm1;
      float *eye, *xyz_view, *tour_view;
      float f1, f2, dt;
      float view_local[3];
      float vtime;

      pj = touri->pathnodes + j;
      if(view_ntimes == 1){
        f1 = 1.0;
      }
      else{
        f1 = (view_ntimes - 1 - j)/(float)(view_ntimes - 1);
      }
      f2 = 1-f1;
      vtime = view_tstart*f1 + view_tstop*f2;
      if(vtime != vtime)vtime = view_tstart; // remove NaN

      iframe_local = ISearch(touri->keyframe_times,touri->nkeyframes,vtime,iframe_local);
      kf1 = touri->keyframe_list[iframe_local];
      kf2 = touri->keyframe_list[iframe_local+1];
      pj->keysnap=&kf1->nodeval;
      dt = kf2->nodeval.time - kf1->nodeval.time;
      if(dt == 0.0){
        f1 = 1.0;
      }
      else{
        f1 = CLAMP((vtime - kf1->nodeval.time) / dt, 0.0, 1.0);
      }
      f2 = 1 - f1;
      pj->time=vtime;
      touri->path_times[j]=vtime;

      eye=pj->eye;
      xyz_view=pj->xyz_view_abs;
      tour_view=pj->tour_view;

      if(kf1->nodeval.eye[0]==kf2->nodeval.eye[0]&&
         kf1->nodeval.eye[1]==kf2->nodeval.eye[1]&&
         kf1->nodeval.eye[2]==kf2->nodeval.eye[2]){
        HermiteEye(1.0,kf1->prev,kf1,eye,view_local);
        Slerp(kf1->prev->keyview_xyz2,kf1->keyview_xyz2,1.0,view_local);
      }
      else{
        HermiteEye(f1,kf1,kf2,eye,view_local);
        Slerp(kf1->keyview_xyz2,kf2->keyview_xyz2,f1,view_local);
      }

      HermiteView(f1,kf1,kf2, xyz_view);
      HermiteOther(f1,kf1,kf2,pj);

      tour_view[0]=view_local[0];
      tour_view[1]=view_local[1];
      tour_view[2]=0.0;
      tour_t[j]=vtime;
      if(j!=0){
        float dx, dy;
        float dz, distance;

        pjm1 = pj - 1;
        dx = eye[0]-pjm1->eye[0];
        dy = eye[1]-pjm1->eye[1];
        dz = eye[2]-pjm1->eye[2];
        distance = sqrt(dx*dx+dy*dy+dz*dz);
        tour_dist[j]=tour_dist[j-1] + distance;
        kf1->distance += distance;
      }
    }

    // construct running "total_distance" info

    kf1 = touri->first_frame.next;
    kf1->total_distance=0.0;
    total_distance=0.0;
    for(keyj=kf1->next;keyj->next!=NULL;keyj=keyj->next){
      keyj->total_distance = keyj->prev->total_distance + keyj->prev->distance;
      total_distance += keyj->prev->distance;
    }

    // decide fraction of global path vs. local path

    touri->global_dist=0.0;
    touri->local_dist=0.0;
    factor=0.0;
    total_time=0.0;
    for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
      if(keyj->next->next!=NULL)total_time += keyj->next->noncon_time - keyj->noncon_time;
      if(tour_constant_vel==1){
        touri->global_dist+=keyj->distance;
      }
      else{
        if(keyj->next->next!=NULL)touri->local_dist+=keyj->next->noncon_time-keyj->noncon_time;
      }
    }
    if(total_time == 0.0)total_time = 1.0;
    if(tour_constant_vel == 1){
      if(touri->global_dist == 0.0)touri->global_dist = 1.0;
    }
    else{
      if(touri->local_dist == 0.0)touri->local_dist = 1.0;
    }
    factor = touri->local_dist/total_time;

    // find number of points for each interval

    ntotal=0;
    for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
      if(tour_constant_vel==1){
        keyj->npoints=view_ntimes*(1.0-factor)*keyj->distance/touri->global_dist;
      }
      else{
        keyj->npoints=0;
        if(keyj->next->next!=NULL)keyj->npoints=view_ntimes*factor*(float)(keyj->next->noncon_time-keyj->noncon_time)/(float)touri->local_dist;
      }
      ntotal += keyj->npoints;
    }
    touri->first_frame.next->npoints += view_ntimes - ntotal;

    if(tour_constant_vel==1){
      ntotal2=0;
      touri->first_frame.next->disp_time=view_tstart;
      {
        float vtime_temp;

        for(keyj=(touri->first_frame).next;keyj->next->next!=NULL;keyj=keyj->next){
          ntotal2+=keyj->npoints;
          if(view_ntimes == 0 || view_tstop==view_tstart){
            vtime_temp = view_tstart;
          }
          else{
            vtime_temp = view_tstart + (float)ntotal2 / (float)view_ntimes*(view_tstop - view_tstart);
          }
          keyj->next->disp_time=vtime_temp;
        }
      }
    }

   // construct distance array based on number of points in each interval

    jj = 0;
    tour_dist3a = tour_dist3 + 5;
    for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
      for(j=0;j<keyj->npoints;j++){
        tour_dist3a[jj] = keyj->total_distance + keyj->distance*(float)j/(float)keyj->npoints;
        jj++;
      }
    }

    // average tour_dist3 array and copy into tour_dist2

    for(j=0;j<5;j++){
      tour_dist3a[-1-j]=tour_dist3a[-j]-(tour_dist3a[1]-tour_dist3a[0]);
      tour_dist3a[view_ntimes+j]=tour_dist3a[view_ntimes-1+j]+(tour_dist3a[view_ntimes-1]-tour_dist3a[view_ntimes-2]);
    }
    for(j=0;j<view_ntimes;j++){
      float avgsum;

      avgsum=0.0;
      for(jj=-5;jj<6;jj++){
        avgsum += tour_dist3a[j+jj];
      }
      tour_dist2[j]=avgsum/11.0;
    }

    iframe_local = 0;
    tour_t2[0]=0.0;
    tour_dist2[0]=0.0;
    tour_tstart = touri->keyframe_list[0]->nodeval.time;
    tour_tstop = touri->keyframe_list[touri->nkeyframes-1]->nodeval.time;
    if(view_ntimes==1){
      vdt = 0.0;
    }
    else{
      vdt = (tour_tstop - tour_tstart)/(float)(view_ntimes-1);
    }
    for(j=1;j<view_ntimes;j++){
      float f1, f2, denom;

      vdist = tour_dist2[j];
      iframe_local = ISearch(tour_dist,view_ntimes,vdist,iframe_local);
      denom = tour_dist[iframe_local + 1] - tour_dist[iframe_local];
        if(denom==0.0){
        f1 = 0.0;
      }
      else{
        f1 = (vdist - tour_dist[iframe_local]) / denom;
      }
      f2 = 1 - f1;
      tour_t2[j] = f2*tour_t[iframe_local] + f1*tour_t[iframe_local+1] ;
      if(tour_t2[j] != tour_t2[j])tour_t2[j] = tour_t2[j - 1]; // remove NaNs
    }
    iframe_old=-1;
    for(j=0;j<view_ntimes;j++){
      pathdata *pj;
      float *eye, *xyz_view, *tour_view;
      float f1, dt;
      float view_local[3];
      float vtime;

      pj = touri->pathnodes + j;
      vtime = tour_t2[j];
      vtime2 = touri->keyframe_list[0]->nodeval.time + j*vdt;
      if(vtime2 != vtime2)vtime2 = vtime; // remove NaN
      iframe_new = ISearch(touri->keyframe_times,touri->nkeyframes,vtime,iframe_old);
      kf1 = touri->keyframe_list[iframe_new];
      kf2 = touri->keyframe_list[iframe_new+1];
      dt = kf2->nodeval.time - kf1->nodeval.time;
      if(dt==0.0){
        f1 = 1.0;
      }
      else{
        f1 = CLAMP((vtime - kf1->nodeval.time)/dt,0.0,1.0);
      }
      pj->time=vtime2;
      touri->path_times[j]=vtime2;

      eye=pj->eye;
      xyz_view=pj->xyz_view_abs;
      tour_view=pj->tour_view;

      HermiteEye(f1,kf1,kf2,eye,view_local);
      HermiteOther(f1,kf1,kf2,pj);
      HermiteView(f1,kf1,kf2,xyz_view);

      if(kf1->viewtype==REL_VIEW||kf2->viewtype==REL_VIEW){
        float az;
        float dxyz[3], denom, dxyz2[3];

        dxyz[0] = view_local[0];
        dxyz[1] = view_local[1];
        denom = 10.0*NORM2(dxyz);
        if(denom==0.0)denom=1.0;
        dxyz[0] /= denom;
        dxyz[1] /= denom;
        az = pj->az_path*DEG2RAD;
        ROTATE(dxyz2,dxyz,az);
        dxyz2[2] = tan(pj->elev_path*DEG2RAD)/10.0;
        VEC3ADD(tour_view,eye,dxyz2);
      }
      else{
        float dxyz[3], denom;

        VEC3DIFF(dxyz,xyz_view,eye);
        denom = 10.0*NORM3(dxyz);
        if(denom == 0.0)denom = 1.0;
        dxyz[0] /= denom;
        dxyz[1] /= denom;
        dxyz[2] /= denom;
        VEC3ADD(tour_view,eye,dxyz);
      }
      if(iframe_old!=iframe_new){
        iframe_old=iframe_new;
        VEC3EQ(pj->keysnap->tour_view,tour_view);
      }
    }
    for(keyj=kf1->next;keyj->next!=NULL;keyj=keyj->next){
      keyj->nodeval.time = tour_tstart + (tour_tstop-tour_tstart)*keyj->total_distance/total_distance;
    }
    if(selected_frame!=NULL)selected_frame->selected=1;
    UpdateTimes();
  }
}

/* ------------------ DefaultTour ------------------------ */

void DefaultTour(void){
  float *eye_xyz,*az_elev;

  touring=1;
  eye_xyz = camera_current->eye;
  az_elev = camera_current->az_elev;

  anglexy0 = az_elev[0];
  azimuth0 = camera_current->azimuth;
  eyex0 = eye_xyz[0];
  eyey0 = eye_xyz[1];
  eyez0 = eye_xyz[2];
  selected_tour=NULL;
  selected_frame=NULL;
  selectedtour_index = TOURINDEX_MANUAL;
  selectedtour_index_old = TOURINDEX_MANUAL;

}

/* ------------------ CopyFrame ------------------------ */

keyframe *CopyFrame(keyframe *framei){
  keyframe *frame;

  NewMemory((void **)&frame, sizeof(keyframe));
  memcpy(frame, framei, sizeof(keyframe));

  CheckMemory;
  return frame;
}

/* ------------------ AddFrame ------------------------ */


keyframe *AddFrame(keyframe *last_frame, float time_local, float *eye, float key_az_path, float elev_path, int viewtype,float zoom_local,float view[3]){
  keyframe *this_frame,*next_frame;
  float *feye, *fxyz_view;

  NewMemory((void **)&this_frame, sizeof(keyframe));
  feye      = this_frame->nodeval.eye;
  fxyz_view = this_frame->nodeval.xyz_view_abs;
  if(viewtype!=0)viewtype=1;

  next_frame=last_frame->next;
  if(next_frame==NULL){
    return NULL;
  }

  last_frame->next=this_frame;
  this_frame->next=next_frame;

  next_frame->prev=this_frame;
  this_frame->prev=last_frame;

  this_frame->az_path=key_az_path;
  this_frame->nodeval.elev_path=elev_path;
  NORMALIZE_XYZ(feye,eye);
  NORMALIZE_XYZ(fxyz_view,view);
  this_frame->noncon_time=time_local;
  this_frame->disp_time=time_local;

  this_frame->viewtype=viewtype;
  this_frame->nodeval.zoom=zoom_local;
  this_frame->keyview_xyz[0] = fxyz_view[0];
  this_frame->keyview_xyz[1] = fxyz_view[1];
  this_frame->keyview_xyz[2] = fxyz_view[2];

  this_frame->eye[0] = eye[0];
  this_frame->eye[1] = eye[1];
  this_frame->eye[2] = eye[2];

  CheckMemory;
  return this_frame;
}

/* ------------------ DeleteTourFrames ------------------------ */

void DeleteTourFrames(tourdata *thistour){
  keyframe *frame;

  for(frame = thistour->first_frame.next;frame->next != NULL;){
    keyframe *next;

    next = frame->next;
    FREEMEMORY(frame);
    frame = next;
  }
  thistour->first_frame.next = &(thistour->last_frame);
  thistour->last_frame.prev = &(thistour->first_frame);
}

/* ------------------ DeleteFrame ------------------------ */

keyframe *DeleteFrame(keyframe *frame){
  keyframe *prev, *next;
  tourdata *thistour;

  //thistour=frame->key_tour;
  thistour=selected_tour;

  prev=frame->prev;
  next=frame->next;
  prev->next=next;
  next->prev=prev;
  FREEMEMORY(frame);
  if(thistour!=NULL){
    thistour->nkeyframes--;
    if(prev->prev!=NULL)return prev;
    if(next->next!=NULL)return next;
  }
  return NULL;
}

/* ------------------ NewSelect ------------------------ */

void NewSelect(keyframe *newselect){
  if(newselect!=selected_frame&&selected_frame!=NULL)selected_frame->selected=0;
  selected_frame=newselect;
  if(newselect!=NULL)selected_frame->selected=1;
}

/* ------------------ SetupCircularTourNodes ------------------------ */

void SetupCircularTourNodes(void){
  float dx, dy, dz, max_xyz;
  int i;

  tour_circular_view[0]=(xbar0+xbarORIG)/2.0;
  tour_circular_view[1]=(ybar0+ybarORIG)/2.0;
  tour_circular_view[2]=(zbar0+zbarORIG)/2.0;
  tour_circular_center[0]=tour_circular_view[0];
  tour_circular_center[1]=tour_circular_view[1];
  tour_circular_center[2]=tour_circular_view[2];

  dx = ABS(xbarORIG - xbar0)/2.0;
  dy = ABS(ybarORIG - ybar0)/2.0;
  dz = ABS(zbarORIG-zbar0)/2.0;
  max_xyz=MAX(dx,dy);
  max_xyz=MAX(max_xyz,dz);
  tour_circular_radius = max_xyz+max_xyz/tan(20.0*DEG2RAD);
  for(i=0;i<3;i++){
    tour_circular_view_default[i] = tour_circular_view[i];
    tour_circular_center_default[i] = tour_circular_center[i];
  }
  tour_circular_radius_default = tour_circular_radius;
}

/* ------------------ InitCircularTour ------------------------ */

void InitCircularTour(tourdata *touri, int nkeyframes, int option){
  int j;
  float key_az_path, elev_path, key_view[3], key_xyz[3], zoom_local;
  int viewtype=0;
  float key_time;
  float angle_local;
  float f1;
  float rad, cosangle, sinangle;
  keyframe *thisframe,*addedframe;

  if(option == UPDATE){
    FREEMEMORY(touri->keyframe_times);
    FREEMEMORY(touri->pathnodes);
    FREEMEMORY(touri->path_times);
  }
  InitTour(touri);
  touri->isDefault=1;
  touri->startup=1;
  touri->periodic=1;
  tour_circular_index = touri - tourinfo;
  strcpy(touri->label,"Circular");
  NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float));
  NewMemory((void **)&touri->pathnodes,view_ntimes*sizeof(pathdata));
  NewMemory((void **)&touri->path_times,view_ntimes*sizeof(float));
  key_view[0]=tour_circular_view[0];
  key_view[1]=tour_circular_view[1];
  key_view[2]=tour_circular_view[2];

  rad = tour_circular_radius;
  elev_path=0.0;

  thisframe=&touri->first_frame;
  for(j=0;j<nkeyframes;j++){
    key_az_path = 0.0;
    if(nkeyframes == 1){
      angle_local = 0.0;
    }
    else{
      angle_local = 2.0*PI*(float)j / (float)(nkeyframes - 1);
    }
    cosangle = cos(angle_local);
    sinangle = sin(angle_local);

    key_xyz[0] = tour_circular_center[0] + rad*cosangle;
    key_xyz[1] = tour_circular_center[1] + rad*sinangle;
    key_xyz[2] = tour_circular_center[2];
    if(nkeyframes == 1){
      f1 = 0.0;
    }
    else{
      f1 = (float)j / (float)(nkeyframes - 1);
    }
    key_time = view_tstart*(1.0-f1) + view_tstop*f1;

    viewtype=1;
    zoom_local=1.0;
    addedframe=AddFrame(thisframe, key_time, key_xyz, key_az_path, elev_path, viewtype, zoom_local, key_view);
    thisframe=addedframe;
    touri->keyframe_times[j]=key_time;
  }
  touri->nkeyframes = nkeyframes;
  touri->last_frame.prev = thisframe;
  thisframe->next = &(touri->last_frame);
  selected_frame = touri->first_frame.next;
}

/* ------------------ ReverseTour  ------------------------ */

void ReverseTour(char *label){
  tourdata *tourreverse=NULL;
  keyframe *keyi, **keys;
  float *times_con, *times_noncon;
  int i;

  if(label==NULL)return;
  for(i = 0;i<ntourinfo;i++){
    tourdata *tourj;

    tourj = tourinfo+i;
    if(strcmp(tourj->label, label)==0){
      tourreverse = tourinfo+i;
      break;
    }
  }
  if(tourreverse==NULL)return;

  NewMemory((void **)&keys, tourreverse->nkeyframes*sizeof(keyframe *));
  NewMemory((void **)&times_con, tourreverse->nkeyframes*sizeof(float));
  NewMemory((void **)&times_noncon, tourreverse->nkeyframes*sizeof(float));

  keyi = &(tourreverse->first_frame);
  for(i = 0;i<tourreverse->nkeyframes;i++){
    keyi = keyi->next;
    keys[tourreverse->nkeyframes-1-i] = keyi;
    times_con[i] = keyi->con_time;
    times_noncon[i] = keyi->noncon_time;
  }

  tourreverse->first_frame.next = keys[0];
  keys[0]->prev = &tourreverse->first_frame;
  for(i = 0;i<tourreverse->nkeyframes-1;i++){
    keys[i]->next=keys[i+1];
  }
  for(i = 0;i<tourreverse->nkeyframes;i++){
    keys[i]->con_time = times_con[i];
    keys[i]->noncon_time = times_noncon[i];
  }
  for(i = 1;i<tourreverse->nkeyframes;i++){
    keys[i]->prev = keys[i-1];
  }
  tourreverse->last_frame.prev = keys[tourreverse->nkeyframes-1];
  keys[tourreverse->nkeyframes-1]->next = &tourreverse->last_frame;

  FREEMEMORY(keys);
  FREEMEMORY(times_con);
  FREEMEMORY(times_noncon);

  updatemenu = 1;

  CreateTourPaths();
  UpdateTimes();
  return;
}

/* ------------------ GetMinMaxDepth  ------------------------ */

void GetMinMaxDepth(float *eye, float *min_depth, float *max_depth){
  int i,first=1;

  *min_depth = -1.0;
  *max_depth = -1.0;
  
  // get distance to each corner of the domain
  
  for(i=0;i<8;i++){
    float depth, dx, dy, dz;

    dx = box_corners[i][0] - eye[0];
    dy = box_corners[i][1] - eye[1];
    dz = box_corners[i][2] - eye[2];
    if(dy<0.0)continue;
    depth = sqrt(dx*dx + dy*dy + dz*dz);
    if(first == 1){
      first = 0;
      *max_depth = depth;
      *min_depth = depth;
    }
    else{
      *min_depth = MIN(*min_depth, depth);
      *max_depth = MAX(*max_depth, depth);
    }
  }

  // get distance to each tour node

  if(edittour==1){
    for(i = 0;i < ntourinfo;i++){
      tourdata *touri;
      keyframe *keyj;

      touri = tourinfo + i;
      for(keyj = (touri->first_frame).next;keyj->next != NULL;keyj = keyj->next){
        float depth;
        float dx, dy, dz;

        dx = NORMALIZE_X(keyj->eye[0]) - eye[0];
        dy = NORMALIZE_Y(keyj->eye[1]) - eye[1];
        dz = NORMALIZE_Z(keyj->eye[2]) - eye[2];
        depth = sqrt(dx*dx + dy*dy + dz*dz);
        if(dy<0.0)continue;
        if(first == 1){
          first = 0;
          *min_depth = depth;
          *max_depth = depth;
        }
        else{
          *min_depth = MIN(*min_depth, depth);
          *max_depth = MAX(*max_depth, depth);
        }
      }
    }
  }
}

/* ------------------ AddTour  ------------------------ */

tourdata *AddTour(char *label){
  tourdata *tourtemp=NULL,*touri;
  int nkeyframes;
  float key_az_path, elev_path, key_view[3], key_xyz[3], zoom_local;
  int viewtype=0;
  float key_time;
  int i;
  keyframe *thisframe, *addedframe;
  int itour=-1;

  DeleteTourList();
  ntourinfo++;
  NewMemory( (void **)&tourtemp, ntourinfo*sizeof(tourdata));
  if(ntourinfo>1)memcpy(tourtemp,tourinfo,(ntourinfo-1)*sizeof(tourdata));
  FREEMEMORY(tourinfo);
  tourinfo=tourtemp;
  touri = tourinfo + ntourinfo - 1;

  InitTour(touri);
  if(label!=NULL){
    for(i = 0;i<ntourinfo-1;i++){
      tourdata *tourj;

      tourj = tourinfo+i;
      if(strcmp(tourj->label, label)==0){
        itour = i;
        sprintf(touri->label, "Copy of %s", TrimFront(label));
        nkeyframes=tourj->nkeyframes;
        break;
      }
    }
    if(itour==-1)label=NULL;
  }
  if(label==NULL){
    sprintf(touri->label,"Tour %i",ntourinfo);
    nkeyframes=2;
  }

  NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float));
  NewMemory((void **)&touri->pathnodes,view_ntimes*sizeof(pathdata));
  NewMemory((void **)&touri->path_times,view_ntimes*sizeof(float));

  if(itour==-1){
    VEC3EQCONS(key_view,0.0);
    float relpos[3];

    key_az_path = 0.0;
    elev_path=0.0;
    viewtype=1;
    zoom_local=1.0;

    key_xyz[0] = xbar0 - 1.0;
    key_xyz[1] = ybar0 - 1.0;
    key_xyz[2] = (zbar0 + zbarORIG)/2.0;
    key_time = view_tstart;
    thisframe=&touri->first_frame;
    addedframe=AddFrame(thisframe,key_time, key_xyz, key_az_path, elev_path, viewtype,zoom_local,key_view);
    touri->keyframe_times[0]=key_time;
    relpos[0] =  -key_xyz[0];
    relpos[1] =  -key_xyz[1];
    relpos[2] =  -key_xyz[2];
    XYZ2AzElev(relpos,&addedframe->az_path,&addedframe->nodeval.az_path);

    key_xyz[0] = xbarORIG + 1.0;
    key_xyz[1] = ybarORIG + 1.0;
    key_xyz[2] = (zbar0 + zbarORIG)/2.0;
    key_time = view_tstop;
    thisframe=addedframe;
    addedframe=AddFrame(thisframe,key_time, key_xyz, key_az_path, elev_path, viewtype,zoom_local,key_view);
    touri->keyframe_times[1]=key_time;
    relpos[0] =  -key_xyz[0];
    relpos[1] =  -key_xyz[1];
    relpos[2] =  -key_xyz[2];
    XYZ2AzElev(relpos, &addedframe->az_path, &addedframe->nodeval.az_path);
  }
  else{
    keyframe *keyfrom, *keylast;;
    tourdata *tourfrom;
    int first=1;

    tourfrom = tourinfo+itour;

    keylast=NULL;
    keyfrom = &(tourfrom->first_frame);
    for(i = 0;i<tourfrom->nkeyframes;i++){
      keyframe *keyj;

      keyfrom = keyfrom->next;
      keyj = CopyFrame(keyfrom);
      if(first==1){
        first = 0;
        touri->first_frame.next=keyj;
        keyj->prev = &(touri->first_frame);
      }
      else{
        keylast->next = keyj;
        keyj->prev = keylast;
      }
      keylast = keyj;
    }
    touri->last_frame.prev = keylast;
    keylast->next = &(touri->last_frame);
  }
  touri->display=1;

  for(i=0;i<ntourinfo;i++){
    touri=tourinfo+i;
    touri->first_frame.next->prev=&touri->first_frame;
    touri->last_frame.prev->next=&touri->last_frame;
  }
  viewalltours=1;
  for(i=0;i<ntourinfo;i++){
    touri = tourinfo + i;
    if(touri->display==0)viewalltours=0;
  }
  updatemenu=1;

  UpdateTourMenuLabels();
  CreateTourPaths();
  UpdateTimes();
  CreateTourList();
  return tourinfo + ntourinfo-1;
}

/* ------------------ DeleteTour  ------------------------ */

void DeleteTour(int tour_index){
  tourdata *touri,*tourtemp;
  int i;

  DeleteTourList();
  touri = tourinfo + tour_index;
  FreeTour(touri);
  ntourinfo--;
  if(ntourinfo>0){
    NewMemory( (void **)&tourtemp, ntourinfo*sizeof(tourdata));
    if(tour_index>0)memcpy(tourtemp,tourinfo,tour_index*sizeof(tourdata));
    if(tour_index<ntourinfo)memcpy(tourtemp+tour_index,tourinfo+tour_index+1,(ntourinfo-tour_index)*sizeof(tourdata));
    FREEMEMORY(tourinfo);
    tourinfo=tourtemp;
    for(i=0;i<ntourinfo;i++){
      touri=tourinfo+i;
      touri->first_frame.next->prev=&touri->first_frame;
      touri->last_frame.prev->next=&touri->last_frame;
    }
  }
  viewalltours=1;
  for(i=0;i<ntourinfo;i++){
    touri = tourinfo + i;
    if(touri->display==0)viewalltours=0;
  }
  if(ntourinfo==0){
    viewalltours=0;
  }
  updatemenu=1;
  CreateTourPaths();
  selectedtour_index=tour_index-1;
  selectedtour_index_old=selectedtour_index;
  if(ntourinfo>0){
    if(selectedtour_index<0)selectedtour_index=0;
    selected_tour=tourinfo+selectedtour_index;
    selected_frame=selected_tour->first_frame.next;
  }
  else{
    selected_tour=NULL;
    selected_frame=NULL;
  }
  SetGluiTourKeyframe();
  UpdateTourMenuLabels();
  UpdateTimes();
  CreateTourList();

}

/* ------------------ ReallocTourMemory  ------------------------ */


void ReallocTourMemory(void){
  int i;
  tourdata *touri;

  if(view_ntimes>0){
    for(i=0;i<ntourinfo;i++){
      touri = tourinfo + i;
      FREEMEMORY(touri->pathnodes);
      FREEMEMORY(touri->path_times);
      NewMemory((void **)&touri->pathnodes,view_ntimes*sizeof(pathdata));
      NewMemory((void **)&touri->path_times,view_ntimes*sizeof(float));
      touri->ntimes=view_ntimes;
      }
    FREEMEMORY(tour_t);
    FREEMEMORY(tour_t2);
    FREEMEMORY(tour_dist);
    FREEMEMORY(tour_dist2);
    FREEMEMORY(tour_dist3);
    NewMemory((void **)&tour_t,view_ntimes*sizeof(float));
    NewMemory((void **)&tour_t2,view_ntimes*sizeof(float));
    NewMemory((void **)&tour_dist,view_ntimes*sizeof(float));
    NewMemory((void **)&tour_dist2,view_ntimes*sizeof(float));
    NewMemory((void **)&tour_dist3,(view_ntimes+10)*sizeof(float));
  }
}

/* ------------------ SetupTour  ------------------------ */

void SetupTour(void){

  if(ntourinfo==0){
    ReallocTourMemory();
    ntourinfo=1;
    NewMemory( (void **)&tourinfo, ntourinfo*sizeof(tourdata));
    InitCircularTour(tourinfo,ncircletournodes,INIT);
    UpdateTourMenuLabels();
    CreateTourPaths();
    UpdateTimes();
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    selectedtour_index = TOURINDEX_MANUAL;
    selectedtour_index = TOURINDEX_MANUAL;
    selected_frame=NULL;
    selected_tour=NULL;
    if(viewalltours==1)TourMenu(MENU_TOUR_SHOWALL);
  }
}

/* ------------------ XYZView2AzElev ------------------------ */

void XYZView2AzElev(keyframe *kf){
  float dxyz[3];
  float dxy2[2];
  float distxy, distxy2;
  float angle_temp, angle_temp2;
  float az, elev;

  float *eye, *xyz_view;

  dxy2[0] = kf->keyview_xyz[0];
  dxy2[1] = kf->keyview_xyz[1];
  distxy2 = NORM2(dxy2);
  if(distxy2<=0.0)return;

  eye = kf->nodeval.eye;
  xyz_view = kf->nodeval.xyz_view_abs;

  VEC3DIFF(dxyz,xyz_view,eye);

  distxy = NORM2(dxyz);
  if(distxy<=0.0)return;

  VEC2DA(dxy2,distxy2);
  VEC3DA(dxyz,distxy);

  angle_temp = RAD2DEG*atan2(dxyz[1],dxyz[0]);
  angle_temp2 = RAD2DEG*atan2(dxy2[1],dxy2[0]);
  az = angle_temp - angle_temp2;
  if(az>180.0)az = 360.0 - az;
  if(az<-180.0)az = az + 360.0;
  elev=atan(dxyz[2])*RAD2DEG;

  kf->az_path=az;
  kf->nodeval.elev_path=elev;
}

/* ------------------ AdjustTourTimes ------------------------ */

void AdjustTourTimes(tourdata *touri){
  float dt;
  keyframe *keyj;
  float tstart, tstop,dtmin;
  int small_flag;


  if(touri->nkeyframes>1){
    tstart = touri->first_frame.next->noncon_time;
    tstop = touri->last_frame.prev->noncon_time;
    if(view_ntimes==0){
      dtmin=0.0;
    }
    else{
      dtmin = (float)4*(tstop-tstart)/(float)view_ntimes;
    }
    small_flag=0;
    for(keyj=(touri->first_frame).next->next;keyj->next!=NULL;keyj=keyj->next){
      dt = keyj->noncon_time - keyj->prev->noncon_time;
      if(dt<dtmin){
        small_flag=1;
        keyj->noncon_time = keyj->prev->noncon_time + dtmin;
      }
    }
    if(small_flag==1&&tstop>view_tstop&&tstop>0.0){
      for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
        if(tstop == 0.0){
          keyj->noncon_time = keyj->noncon_time;
        }
        else{
          keyj->noncon_time = keyj->noncon_time*view_tstop / tstop;
        }
      }
    }
  }
}

