#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOobjects.h"
#include "readtour.h"

/* ------------------ UpdateViewTour ------------------------ */

void UpdateViewTour(void){
  int i;
  tourdata *touri;

  viewalltours=1;
  viewanytours=0;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    touri = global_scase.tourcoll.tourinfo + i;
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

  if(global_scase.tourcoll.ntourinfo>0){
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      touri = global_scase.tourcoll.tourinfo + i;
      STRCPY(touri->menulabel,touri->label);
    }
  }
  updatemenu=1;
}

/* ------------------ DrawCir ------------------------ */

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

  if(selected_frame!=NULL)selectedtour_index = selected_tour-global_scase.tourcoll.tourinfo;

  if(edittour==1){
    tourdata *tour_sel;

    /* path line (non-selected)*/

    if(showtours_whenediting==1){
      AntiAliasLine(ON);
      glColor3fv(tmp_tourcol_pathline);
      glBegin(GL_LINES);
      for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
        tourdata *touri;
        int j;

        touri =global_scase.tourcoll.tourinfo + i;
        if(touri->display==0||touri->nkeyframes<=1||selectedtour_index==i)continue;

        for(j=0;j<global_scase.tourcoll.tour_ntimes-1;j++){
          float xyz[3], xyz2[3], *times;

          times = touri->path_times;
          GetTourXYZ(times[j],   touri, xyz);
          GetTourXYZ(times[j+1], touri, xyz2);
          glVertex3fv(xyz);
          glVertex3fv(xyz2);
        }
      }
     glEnd();
     AntiAliasLine(OFF);
    }

    /* path line (selected)*/

    tour_sel = global_scase.tourcoll.tourinfo + selectedtour_index;
    if(selectedtour_index!=TOURINDEX_MANUAL&&tour_sel->display==1&&tour_sel->nkeyframes>1){
      int j;
      tourdata *touri;

      glColor3fv(tourcol_selectedpathline);
      glBegin(GL_LINES);
      touri = global_scase.tourcoll.tourinfo + selectedtour_index;

      for(j=0;j<global_scase.tourcoll.tour_ntimes-1;j++){
        float xyz[3], xyz2[3], *times;

        times = touri->path_times;
        GetTourXYZ(times[j], touri, xyz);
        GetTourXYZ(times[j+1], touri, xyz2);
        glVertex3fv(xyz);
        glVertex3fv(xyz2);
      }
      glEnd();
    }

    /* path knots */

    if(show_path_knots==1&&tour_sel->nkeyframes>1){
      glColor3f(0.0f,0.0f,1.0f);
      glPointSize(5.0f);
      glBegin(GL_POINTS);
      for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
        int j;
        tourdata *touri;

        touri = global_scase.tourcoll.tourinfo + i;
        if(touri->display==0||selectedtour_index!=i)continue;

        for(j=0;j<global_scase.tourcoll.tour_ntimes-1;j++){
        float xyz[3], *times;

        times = touri->path_times;
        GetTourXYZ(times[j], touri, xyz);
        glVertex3fv(xyz);
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

        glVertex3fv(framej->xyz_smv);
      }
      glEnd();
    }

    /* non-selected path, keyframe knots */

    if(showtours_whenediting==1){
      glColor3fv(tmp_tourcol_pathknots);
      glPointSize(10.0f);
      glBegin(GL_POINTS);
      for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
        int j;
        tourdata *touri;

        touri = global_scase.tourcoll.tourinfo + i;
        if(touri->display==0||i==selectedtour_index)continue;

        for(j=0;j<touri->nkeyframes;j++){
          keyframe *framej;

          framej = touri->keyframe_list[j];
          if(framej->selected==1)continue;
          glVertex3fv(framej->xyz_smv);
        }
      }
      glEnd();
    }

    /* selected path, selected keyframe */

    if(selected_frame!=NULL&&selected_tour->display==1){
      glBegin(GL_POINTS);
      glColor3fv(tourcol_selectedknot);
      glVertex3fv(selected_frame->xyz_smv);
      glColor3fv(tourcol_selectedview);
      glVertex3fv(selected_frame->view_smv);
      glEnd();
      if(show_tour_hint==1){
        float *xyz;

        xyz = selected_frame->xyz_smv;
        Output3Text(foregroundcolor,xyz[0]+0.1,xyz[1]+0.1,xyz[2]+0.1,"click and drag to change tour node");
      }
    }

    /* keyframe times */
    SNIFF_ERRORS("after select path, selected keyframe");
    CheckMemory;

    if(fontindex==SCALED_FONT)ScaleFont3D();
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      int j;
      tourdata *touri;

      touri = global_scase.tourcoll.tourinfo + i;
      if(touri->display==0)continue;
      if(showtours_whenediting==0&&selectedtour_index!=i)continue;

      for(j=0;j<touri->nkeyframes;j++){
        keyframe *framej;
        float *xyz;

        framej = touri->keyframe_list[j];
        xyz = framej->xyz_smv;
        {
          char label[128];
#ifdef _DEBUG
          if(showdebugtour == 1){
            sprintf(label, "t=%8.2f/%i n=%i n/ad=%8.2f ad=%8.2f ld=%8.2f",
              framej->time, framej->set_tour_time, framej->npoints, (float)framej->npoints / framej->arc_dist, framej->arc_dist, framej->line_dist);
            if(j==0)strcat(label, "(ad==arc distance, ld==line distance)");
          }
          else{
            sprintf(label, "%8.2f", framej->time);
          }
#else
          sprintf(label, "%8.2f", framej->time);
#endif
          TrimZeros(label);
          Output3Text(tmp_tourcol_text, xyz[0]+0.02f, xyz[1]+0.015f, xyz[2]+0.015f,label);
        }
      }
    }
  }

    /* keyframe avatar */

  //show_avatar=1;
  //tourlocus_type=2;
  if(show_avatar==1){
    switch(tourlocus_type){
      case 0:
        AntiAliasLine(ON);
        glBegin(GL_LINES);
        glColor3fv(tourcol_avatar);
        for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
          tourdata *touri;
          touri = global_scase.tourcoll.tourinfo + i;
          if(touri->display==0||touri->nkeyframes<=1)continue;
          if(touri->timeslist==NULL)continue;

          float xyz[3], view[3];
          float tour_time=0.0;

          if(global_times!=NULL)tour_time = global_times[itimes];
          GetTourXYZ(tour_time,  touri, xyz);
          GetTourView(tour_time, touri, view);

          glVertex3fv(xyz);
          glVertex3f(xyz[0], xyz[1], xyz[2]+0.1);
          glVertex3fv(xyz);
          glVertex3fv(view);
        }
        glEnd();
        AntiAliasLine(OFF);
        break;
      case 1:
        for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
          tourdata *touri;

          touri = global_scase.tourcoll.tourinfo + i;
          if(touri->display==0||touri->nkeyframes<=1)continue;
          if(touri->timeslist==NULL)continue;

          float xyz[3];
          float tour_time=0.0;

          if(global_times!=NULL)tour_time = global_times[itimes];
          GetTourXYZ(tour_time, touri, xyz);
          DrawCir(xyz,tourrad_avatar,tourcol_avatar);
        }
        break;
      case 2:
        for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
          tourdata *touri;
          float az_angle, dxy[2];

          touri = global_scase.tourcoll.tourinfo + i;
          if(touri->display==0||touri->nkeyframes<=1)continue;
          if(touri->timeslist==NULL)continue;

          float xyz[3], tour_view[3];
          float tour_time=0.0;

          if(global_times!=NULL)tour_time = global_times[itimes];
          GetTourXYZ(tour_time,  touri, xyz);
          GetTourView(tour_time, touri, tour_view);
          dxy[0]=tour_view[0]-xyz[0];
          dxy[1]=tour_view[1]-xyz[1];
          if(dxy[0]!=0.0||dxy[1]!=0.0){
            az_angle=atan2(dxy[1],dxy[0])*RAD2DEG;
          }
          else{
            az_angle=0.0;
          }

          glPushMatrix();
          glTranslatef(xyz[0], xyz[1], xyz[2]);
          glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));

          glRotatef(az_angle,0.0,0.0,1.0);

          DrawSmvObject(global_scase.objectscoll.avatar_types[touri->glui_avatar_index],0,NULL,0,NULL,0);
          glPopMatrix();
        }
        break;
      default:
        assert(FFALSE);
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
  float *xyz;

  glPointSize(20.0f);
  glBegin(GL_POINTS);
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    touri = global_scase.tourcoll.tourinfo + i;
    for(j=0;j<touri->nkeyframes;j++){

      if(showtours_whenediting==1||selectedtour_index==i){

        if(touri->display==1){
          unsigned char r, g, b;

          framej = touri->keyframe_list[j];
          xyz = framej->xyz_smv;

          GetRGB(color_index+1,&r,&g,&b);
          glColor3ub(r,g,b);
          glVertex3fv(xyz);
        }
      }
      color_index++;
    }
  }
  glEnd();
}

/* ------------------ GetTourVal ------------------------ */

void GetTourVal(float t, keyframe *kf1, keyframe *kf2, float *xyz){
  int i;
  float t3, t2;

  t2 = t*t;
  t3 = t2*t;

  for(i = 0; i<3; i++){
    float p0, p1, m0, m1;

    if(i==0){
      p0 = FDS2SMV_X(kf1->xyz_fds[i]);
      p1 = FDS2SMV_X(kf2->xyz_fds[i]);
    }
    else if(i==1){
      p0 = FDS2SMV_Y(kf1->xyz_fds[i]);
      p1 = FDS2SMV_Y(kf2->xyz_fds[i]);
    }
    else{
      p0 = FDS2SMV_Z(kf1->xyz_fds[i]);
      p1 = FDS2SMV_Z(kf2->xyz_fds[i]);
    }
    m0 = kf1->xyz_tangent_right[i];
    m1 = kf2->xyz_tangent_left[i];

    xyz[i] = HERMVAL(p0,p1,m0,m1);
  }
}

/* ------------------ HermiteXYZ ------------------------ */

void HermiteXYZ(float t, keyframe *kf1, keyframe *kf2, float *xyz, float *slope){
  float t3, t2;
  float p0[3], p1[3], *m0, *m1;
  t2 = t*t;
  t3 = t2*t;

  FDS2SMV_XYZ(p0, kf1->xyz_fds);
  FDS2SMV_XYZ(p1, kf2->xyz_fds);
  m0 = kf1->xyz_tangent_right;
  m1 = kf2->xyz_tangent_left;
  xyz[0] = HERMVAL(p0[0], p1[0], m0[0], m1[0]);
  xyz[1] = HERMVAL(p0[1], p1[1], m0[1], m1[1]);
  xyz[2] = HERMVAL(p0[2], p1[2], m0[2], m1[2]);
  if(slope != NULL){
    slope[0] = HERMDERIV(p0[0], p1[0], m0[0], m1[0]);
    slope[1] = HERMDERIV(p0[1], p1[1], m0[1], m1[1]);
  }
}

void GetTourXYZ(float t, tourdata *this_tour, float *xyz){
  keyframe *this_key;

  this_key = GetKeyFrame(this_tour, t);
  GetKeyXYZ(t, this_key, xyz);
}

/* ------------------ GetKeyXYZ ------------------------ */

void GetKeyXYZ(float t, keyframe *this_key, float *xyz){
  keyframe *next_key;
  float dt, t_scaled;

  next_key = this_key->next;
  t_scaled = 0.0;
  if(t < this_key->time + this_key->pause_time){
    HermiteXYZ(t_scaled, this_key, next_key, xyz, NULL);
  }
  else{
    dt = next_key->time - this_key->time - this_key->pause_time;
    if(dt > 0.0)t_scaled = CLAMP((t - this_key->time -this_key->pause_time) / dt, 0.0, 1.0);
    HermiteXYZ(t_scaled, this_key, next_key, xyz, NULL);
  }
}

/* ------------------ GetTourTimeBounds ------------------------ */

void GetTourTimeBounds(tourdata *touri, float *tour_tstart, float *tour_tstop){
  *tour_tstart = touri->first_frame.next->time;
  *tour_tstop  = touri->last_frame.prev->time;
}

/* ------------------ GetTourProperties ------------------------ */

void GetTourProperties(tourdata *touri){
  keyframe *thiskey;
  int j;

  // count key frames
  for(thiskey = (touri->first_frame).next, j = 0; thiskey->next!=NULL; thiskey = thiskey->next, j++){
  }
  touri->nkeyframes = j;

  touri->global_dist = 0.0;
  for(thiskey = (touri->first_frame).next; thiskey->next!=NULL; thiskey = thiskey->next){
    float xyz0[3], dist;
    float dx, dy, dz, dt;
    int i, n;
    keyframe *nextkey;

    nextkey = thiskey->next;
    if(nextkey->next==NULL)break;

    DDIST3(thiskey->xyz_smv, nextkey->xyz_smv, dist);
    n = MIN(MAX(dist/0.1, 11), 1001);
    dt = 1.0/(float)(n-1);
    GetTourVal(0.0, thiskey, nextkey, xyz0);
    dist = 0.0;
    for(i = 1; i<n; i++){
      float t, xyz1[3], delta;

      t = (float)i*dt;
      GetTourVal(t, thiskey, nextkey, xyz1);
      DDIST3(xyz0, xyz1, delta);
      dist += delta;
      memcpy(xyz0, xyz1, 3*sizeof(float));
    }
    thiskey->arc_dist   = dist;
    touri->global_dist += dist;
  }

  float tour_tstart, tour_tstop;
  GetTourTimeBounds(touri, &tour_tstart, &tour_tstop);

  thiskey = (touri->first_frame).next;
  thiskey->time = tour_tstart;
  float cum_dist = 0.0;
  int cum_npoints = 0;
  float *tour_times, *xyzs, *views, total_distance;

  float total_pause_time;
  keyframe *first_key;

  first_key = (touri->first_frame).next;
  first_key->cum_pause_time = 0.0;
  for(total_pause_time=0.0, thiskey = first_key; thiskey->next!=NULL; thiskey = thiskey->next){
    total_pause_time += thiskey->pause_time;
    thiskey->cum_pause_time = total_pause_time;
  }
  for(thiskey = (touri->first_frame).next; thiskey->next!=NULL; thiskey = thiskey->next){
    int npoints_i;
    keyframe *nextkey;

    nextkey = thiskey->next;
    if(nextkey->next==NULL){
      thiskey->npoints = 0;
      break;
    }

    npoints_i = global_scase.tourcoll.tour_ntimes*(thiskey->arc_dist/touri->global_dist);
    cum_npoints += npoints_i;
    if(nextkey->next==NULL){
      npoints_i   += (global_scase.tourcoll.tour_ntimes-cum_npoints);
      cum_npoints += (global_scase.tourcoll.tour_ntimes-cum_npoints);
    }
    thiskey->npoints = npoints_i;
    cum_dist       += thiskey->arc_dist;
    if(nextkey->set_tour_time == 0){
      nextkey->time = tour_tstart + thiskey->cum_pause_time + (tour_tstop - total_pause_time - tour_tstart) * (cum_dist / touri->global_dist);
    }
  }

  (touri->last_frame).prev->time = tour_tstop;

  for(thiskey = (touri->first_frame).next, j = 0; thiskey->next!=NULL; thiskey = thiskey->next, j++){
    touri->keyframe_list[j]  = thiskey;
    touri->keyframe_times[j] = thiskey->time;
    FDS2SMV_XYZ(thiskey->xyz_smv, thiskey->xyz_fds);
  }
  touri->nkeyframes = j;


  NewMemory((void **)&(tour_times), global_scase.tourcoll.tour_ntimes*sizeof(float));
  NewMemory((void **)&(xyzs),       3*global_scase.tourcoll.tour_ntimes*sizeof(float));
  NewMemory((void **)&(views),      3*global_scase.tourcoll.tour_ntimes*sizeof(float));

  for(j=0;j<global_scase.tourcoll.tour_ntimes;j++){
    float f1, vtime;

    f1 = 0.0;
    if(global_scase.tourcoll.tour_ntimes>1)f1 = (float)j/(float)(global_scase.tourcoll.tour_ntimes - 1);
    vtime                = tour_tstart*(1.0-f1) + tour_tstop*f1;
    tour_times[j]        = vtime;
    touri->path_times[j] = vtime;
    {
      if(j==0){
        tour_times[j] = 0.0;
      }
      else{
        float dt, *xyz1, *xyz2;
        float dx, dy, dz;

        xyz1 = xyzs + 3*(j-1);
        xyz2 = xyzs + 3*j;
        DDIST3(xyz1, xyz2, dt);
        tour_times[j] = tour_times[j-1] + dt;
      }
    }
  }
  total_distance = tour_times[global_scase.tourcoll.tour_ntimes-1];
  for(j=0;j<global_scase.tourcoll.tour_ntimes;j++){
    tour_times[j] = tour_tstart + tour_times[j]*(tour_tstop - tour_tstart)/total_distance;
  }
  FREEMEMORY(tour_times);
  FREEMEMORY(xyzs);
  FREEMEMORY(views);
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
  if(global_scase.tourcoll.ntourinfo==0)return;

  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *touri;
    keyframe *keyj;
    int nframes;

    touri = global_scase.tourcoll.tourinfo + i;
    nframes=0;
    for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
      ntourknots++;
      nframes++;
      keyframe *this_key, *next_key;
      float *this_xyz, *next_xyz, *xyz_diff, *view_diff;
      float dx, dy, dz;

      this_key = keyj;
      next_key = this_key->next;
      xyz_diff = this_key->xyz_diff;
      view_diff = this_key->view_diff;
      if(next_key->next==NULL){
        int ii;

        this_key->line_dist = 0.0;
        for(ii=0;ii<3;ii++){
          xyz_diff[ii]  = 0.0;
          view_diff[ii] = 0.0;
        }
      }
      else{
        int ii;

        this_xyz = this_key->xyz_smv;
        next_xyz = next_key->xyz_smv;
        DDIST3(this_xyz, next_xyz, this_key->line_dist);
        for(ii=0;ii<3;ii++){
          xyz_diff[ii]  = (next_xyz[ii]           - this_xyz[ii]);
          view_diff[ii] = (next_key->view_smv[ii] - this_key->view_smv[ii]);
        }
      }
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
  for(i = 0; i<global_scase.tourcoll.ntourinfo; i++){
    tourdata *touri;
    keyframe *keyj;
    int j;

    touri = global_scase.tourcoll.tourinfo+i;
    for(keyj = (touri->first_frame).next, j = 0; keyj->next!=NULL; keyj = keyj->next, j++){
      FDS2SMV_XYZ(keyj->xyz_smv, keyj->xyz_fds);
    }
    if(viewalltours==1)touri->display = 1;
    FREEMEMORY(touri->keyframe_list);
    NewMemory((void **)&(touri->keyframe_list), touri->nkeyframes*sizeof(keyframe *));
    FREEMEMORY(touri->keyframe_times);
    NewMemory((void **)&(touri->keyframe_times), touri->nkeyframes*sizeof(float));
    for(keyj = (touri->first_frame).next, j = 0; keyj->next!=NULL; keyj = keyj->next, j++){
      touri->keyframe_list[j] = keyj;
    }
    if(touri->nkeyframes<=1)continue;
    for(keyj = (touri->first_frame).next, j = 0; keyj->next!=NULL; keyj = keyj->next, j++){
      keyframe *lastkey, *thiskey, *nextkey;
      float *last_xyz, *this_xyz, *next_xyz;
      float *last_view, *this_view, *next_view;

      *tourknotskeylist_copy++ = keyj;
      *tourknotstourlist_copy++ = touri;
      keyj->selected = 0;

      lastkey = keyj->prev;
      thiskey = keyj;
      nextkey = keyj->next;
      if(touri->periodic==1){
        if(j==0){
          lastkey = touri->keyframe_list[touri->nkeyframes-2];
        }
        if(j==touri->nkeyframes-1){
          nextkey = touri->keyframe_list[1];
        }
      }

      last_xyz = lastkey->xyz_smv;
      this_xyz = thiskey->xyz_smv;
      next_xyz = nextkey->xyz_smv;
      last_view = lastkey->view_smv;
      this_view = thiskey->view_smv;
      next_view = nextkey->view_smv;

      if(touri->periodic==0&&j==0){
        VEC3EQCONS(keyj->xyz_tangent_left, 0.0);
        VEC3DIFF(keyj->xyz_tangent_right, next_xyz, this_xyz);
        VEC3EQCONS(keyj->view_tangent_left, 0.0);
        VEC3DIFF(keyj->view_tangent_right, next_view, this_view);
      }
      else if(touri->periodic==0&&j==touri->nkeyframes-1){
        VEC3DIFF(keyj->xyz_tangent_left, this_xyz, last_xyz);
        VEC3EQCONS(keyj->xyz_tangent_right, 0.0);
        VEC3DIFF(keyj->view_tangent_left, this_view, last_view);
        VEC3EQCONS(keyj->view_tangent_right, 0.0);
      }
      else{
        //assume non-uniform spacing but not working
#define AVERAGE_SLOPE(last_dist, last_diff, next_dist, next_diff, val)\
        val[0]=(next_dist*last_diff[0] + last_dist*next_diff[0])/(last_dist+next_dist);\
        val[1]=(next_dist*last_diff[1] + last_dist*next_diff[1])/(last_dist+next_dist);\
        val[2]=(next_dist*last_diff[2] + last_dist*next_diff[2])/(last_dist+next_dist)

        AVERAGE_SLOPE(lastkey->line_dist, lastkey->xyz_diff, thiskey->line_dist, thiskey->xyz_diff, thiskey->xyz_tangent_left);
        AVERAGE_SLOPE(lastkey->line_dist, lastkey->xyz_diff, thiskey->line_dist, thiskey->xyz_diff, thiskey->xyz_tangent_right);

        AVERAGE_SLOPE(lastkey->line_dist, lastkey->view_diff, thiskey->line_dist, thiskey->view_diff, thiskey->view_tangent_left);
        AVERAGE_SLOPE(lastkey->line_dist, lastkey->view_diff, thiskey->line_dist, thiskey->view_diff, thiskey->view_tangent_right);
      }
    }
  }

  // get tour properties
  for(i = 0; i<global_scase.tourcoll.ntourinfo; i++){
    tourdata *touri;

    touri = global_scase.tourcoll.tourinfo+i;
    GetTourProperties(touri);
  }

  for(i = 0; i<global_scase.tourcoll.ntourinfo; i++){
    tourdata *touri;
    keyframe *keyj;

    touri = global_scase.tourcoll.tourinfo+i;

    for(keyj = (touri->first_frame).next; keyj->next!=NULL; keyj = keyj->next){
      float denom, view_smv[3];

      memcpy(view_smv, keyj->view_tangent_right, 2*sizeof(float));
      view_smv[2] = 0.0;

      ROTATE(keyj->view2_smv, view_smv, 0.0);
      keyj->view2_smv[2] = 0.0;
      denom = NORM2(keyj->view2_smv);
      if(denom==0.0)continue;
      VEC2MA(keyj->view2_smv, 10000.0/denom);
    }
  }

  if(selected_frame!=NULL)selected_frame->selected = 1;
  UpdateTimes();
}

/* ------------------ DefaultTour ------------------------ */

void DefaultTour(void){
  float *xyz,*az_elev;

  touring=1;
  xyz = camera_current->eye;
  az_elev = camera_current->az_elev;

  anglexy0 = az_elev[0];
  azimuth0 = camera_current->azimuth;
  eyex0 = xyz[0];
  eyey0 = xyz[1];
  eyez0 = xyz[2];
  selected_tour=NULL;
  selected_frame=NULL;
  selectedtour_index = TOURINDEX_MANUAL;
  selectedtour_index_old = TOURINDEX_MANUAL;

}


/* ------------------ AddFrame ------------------------ */

keyframe *AddFrame(keyframe *last_frame, float time_local, float pause_time_local, float *xyz, float view[3], int set_time){
  keyframe *this_frame,*next_frame;
  float *feye, *fxyz_view;

  NewMemory((void **)&this_frame, sizeof(keyframe));
  feye = this_frame->xyz_smv;
  fxyz_view = this_frame->view_smv;

  next_frame=last_frame->next;
  if(next_frame==NULL){
    return NULL;
  }

  last_frame->next=this_frame;
  this_frame->next=next_frame;

  next_frame->prev=this_frame;
  this_frame->prev=last_frame;

  FDS2SMV_XYZ(feye, xyz);
  FDS2SMV_XYZ(fxyz_view,view);
  this_frame->time =time_local;

  memcpy(this_frame->view_smv, fxyz_view, 3*sizeof(float));
  memcpy(this_frame->xyz_fds,  xyz,       3*sizeof(float));
  this_frame->pause_time = pause_time_local;
  this_frame->set_tour_time = set_time;

  CheckMemory;
  return this_frame;
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

  tour_circular_view[0]=(global_scase.xbar0+xbarORIG)/2.0;
  tour_circular_view[1]=(global_scase.ybar0+ybarORIG)/2.0;
  tour_circular_view[2]=(global_scase.zbar0+zbarORIG)/2.0;
  memcpy(tour_circular_center, tour_circular_view, 3*sizeof(float));

  dx = ABS(xbarORIG - global_scase.xbar0)/2.0;
  dy = ABS(ybarORIG - global_scase.ybar0)/2.0;
  dz = ABS(zbarORIG-global_scase.zbar0)/2.0;
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
  float key_view[3], key_xyz[3];
  float key_time;
  float angle_local;
  float f1;
  float rad, cosangle, sinangle;
  keyframe *thisframe,*addedframe;

  if(option == UPDATE){
    FREEMEMORY(touri->keyframe_times);
    FREEMEMORY(touri->path_times);
  }
  InitTour(&global_scase.tourcoll, touri);
  touri->isDefault=1;
  touri->startup=1;
  touri->periodic=1;
  tour_circular_index = touri - global_scase.tourcoll.tourinfo;
  strcpy(touri->label,"Circular");
  NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float));
  NewMemory((void **)&touri->path_times,global_scase.tourcoll.tour_ntimes*sizeof(float));
  memcpy(key_view, tour_circular_view, 3*sizeof(float));

  rad = tour_circular_radius;

  thisframe=&touri->first_frame;
  float tour_tstart, tour_tstop;
  tour_tstart = global_scase.tourcoll.tour_tstart;
  tour_tstop  = global_scase.tourcoll.tour_tstop;
  for(j=0;j<nkeyframes;j++){
    if(nkeyframes == 1){
      angle_local = 0.0;
    }
    else{
      float angle_deg;

      angle_deg = 360.0*(float)j/(float)(nkeyframes - 1);
      angle_local = DEG2RAD*angle_deg;
    }
    angle_local += DEG2RAD*tour_circular_angle0;
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
    key_time = tour_tstart*(1.0-f1) + tour_tstop*f1;

    addedframe = AddFrame(thisframe, key_time, 0.0, key_xyz, key_view, 0);
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
  for(i = 0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *tourj;

    tourj = global_scase.tourcoll.tourinfo+i;
    if(strcmp(tourj->label, label)==0){
      tourreverse = global_scase.tourcoll.tourinfo+i;
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
  }

  tourreverse->first_frame.next = keys[0];
  keys[0]->prev = &tourreverse->first_frame;
  for(i = 0;i<tourreverse->nkeyframes-1;i++){
    keys[i]->next=keys[i+1];
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

/* ------------------ AddTour  ------------------------ */

tourdata *AddTour(char *label){
  tourdata *tourtemp=NULL,*touri;
  int nkeyframes;
  float key_view[3], key_xyz[3];
  float key_time;
  int i;
  keyframe *thisframe, *addedframe;
  int itour=-1;

  GLUIDeleteTourList();
  global_scase.tourcoll.ntourinfo++;
  NewMemory( (void **)&tourtemp, global_scase.tourcoll.ntourinfo*sizeof(tourdata));
  if(global_scase.tourcoll.ntourinfo>1)memcpy(tourtemp,global_scase.tourcoll.tourinfo,(global_scase.tourcoll.ntourinfo-1)*sizeof(tourdata));
  FREEMEMORY(global_scase.tourcoll.tourinfo);
  global_scase.tourcoll.tourinfo=tourtemp;
  touri = global_scase.tourcoll.tourinfo + global_scase.tourcoll.ntourinfo - 1;

  InitTour(&global_scase.tourcoll, touri);
  if(label!=NULL){
    for(i = 0;i<global_scase.tourcoll.ntourinfo-1;i++){
      tourdata *tourj;

      tourj = global_scase.tourcoll.tourinfo+i;
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
    sprintf(touri->label,"Tour %i",global_scase.tourcoll.ntourinfo);
    nkeyframes=2;
  }

  NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float));
  NewMemory((void **)&touri->path_times,global_scase.tourcoll.tour_ntimes*sizeof(float));

  if(itour==-1){
    VEC3EQCONS(key_view,0.0);

    key_xyz[0] = global_scase.xbar0 - 1.0;
    key_xyz[1] = global_scase.ybar0 - 1.0;
    key_xyz[2] = (global_scase.zbar0 + zbarORIG)/2.0;
    key_time = global_scase.tourcoll.tour_tstart;
    thisframe=&touri->first_frame;
    addedframe = AddFrame(thisframe, key_time, 0.0, key_xyz, key_view, 0);
    touri->keyframe_times[0]=key_time;

    key_xyz[0] = xbarORIG + 1.0;
    key_xyz[1] = ybarORIG + 1.0;
    key_xyz[2] = (global_scase.zbar0 + zbarORIG)/2.0;
    key_time = global_scase.tourcoll.tour_tstop;
    thisframe=addedframe;
    AddFrame(thisframe, key_time, 0.0, key_xyz, key_view, 0);
    touri->keyframe_times[1]=key_time;
  }
  else{
    keyframe *keyfrom, *keylast;
    tourdata *tourfrom;
    int first=1;

    tourfrom = global_scase.tourcoll.tourinfo+itour;

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
    if(keylast!=NULL)keylast->next = &(touri->last_frame);
  }
  touri->display=1;

  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    touri=global_scase.tourcoll.tourinfo+i;
    touri->first_frame.next->prev=&touri->first_frame;
    touri->last_frame.prev->next=&touri->last_frame;
  }
  viewalltours=1;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    touri = global_scase.tourcoll.tourinfo + i;
    if(touri->display==0)viewalltours=0;
  }
  updatemenu=1;

  UpdateTourMenuLabels();
  CreateTourPaths();
  UpdateTimes();
  GLUICreateTourList();
  return global_scase.tourcoll.tourinfo + global_scase.tourcoll.ntourinfo-1;
}

/* ------------------ DeleteTour  ------------------------ */

void DeleteTour(int tour_index){
  tourdata *touri,*tourtemp;
  int i;

  GLUIDeleteTourList();
  touri = global_scase.tourcoll.tourinfo + tour_index;
  FreeTour(touri);
  global_scase.tourcoll.ntourinfo--;
  if(global_scase.tourcoll.ntourinfo>0){
    NewMemory( (void **)&tourtemp, global_scase.tourcoll.ntourinfo*sizeof(tourdata));
    if(tour_index>0)memcpy(tourtemp,global_scase.tourcoll.tourinfo,tour_index*sizeof(tourdata));
    if(tour_index<global_scase.tourcoll.ntourinfo)memcpy(tourtemp+tour_index,global_scase.tourcoll.tourinfo+tour_index+1,(global_scase.tourcoll.ntourinfo-tour_index)*sizeof(tourdata));
    FREEMEMORY(global_scase.tourcoll.tourinfo);
    global_scase.tourcoll.tourinfo=tourtemp;
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      touri=global_scase.tourcoll.tourinfo+i;
      touri->first_frame.next->prev=&touri->first_frame;
      touri->last_frame.prev->next=&touri->last_frame;
    }
  }
  viewalltours=1;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    touri = global_scase.tourcoll.tourinfo + i;
    if(touri->display==0)viewalltours=0;
  }
  if(global_scase.tourcoll.ntourinfo==0){
    viewalltours=0;
  }
  updatemenu=1;
  CreateTourPaths();
  selectedtour_index=tour_index-1;
  selectedtour_index_old=selectedtour_index;
  if(global_scase.tourcoll.ntourinfo>0){
    if(selectedtour_index<0)selectedtour_index=0;
    selected_tour=global_scase.tourcoll.tourinfo+selectedtour_index;
    selected_frame=selected_tour->first_frame.next;
  }
  else{
    selected_tour=NULL;
    selected_frame=NULL;
  }
  GLUISetTourKeyframe();
  UpdateTourMenuLabels();
  UpdateTimes();
  GLUICreateTourList();

}

/* ------------------ SetupTour  ------------------------ */

void SetupTour(void){

  if(global_scase.tourcoll.ntourinfo==0){
    ReallocTourMemory(&global_scase.tourcoll);
    global_scase.tourcoll.ntourinfo=1;
    NewMemory( (void **)&global_scase.tourcoll.tourinfo, global_scase.tourcoll.ntourinfo*sizeof(tourdata));
    InitCircularTour(global_scase.tourcoll.tourinfo,ncircletournodes,INIT);
    UpdateTourMenuLabels();
    CreateTourPaths();
    UpdateTimes();
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    selectedtour_index = TOURINDEX_MANUAL;
    selected_frame=NULL;
    selected_tour=NULL;
    if(viewalltours==1)TourMenu(MENU_TOUR_SHOWALL);
  }
}
