#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

/* ------------------ GetShooterVel ------------------------ */

void GetShooterVel(float *uvw, float *xyz){
  float factor;

  if(shooter_vel_type==0&&plot3dtimelist!=NULL){
    // plot3d velocities
    GetPlot3dUVW(xyz, uvw);
  }
  else{
    // power law velocities
    factor = pow(xyz[2]/shooter_z0,shooter_p);
    uvw[0] = factor*shooter_velx;
    uvw[1] = factor*shooter_vely;
    uvw[2] = shooter_velz;
  }
}

/* ------------------ IncrementShooterData ------------------------ */

void IncrementShooterData(shootpointdata *pold, shootpointdata *pnew, float dtstep){
  int i;
  float *xyzold, *uvwold, uvw_air[3];
  float *xyznew, *uvwnew;
  float g=9.8;
  float dt;

  // dv/dt = g - |g|(v-v_a)/v_inf
  // dx/dt = v

  shooter_time+=dtstep;
  for(i=0;i<shooter_nparts;i++){
    float dvel[3];
    meshdata *meshpoint;
    float grid_vel,tstep;

    xyzold = pold[i].xyz;
    uvwold = pold[i].uvw;
    xyznew = pnew[i].xyz;
    uvwnew = pnew[i].uvw;

    xyznew[0] = xyzold[0];
    xyznew[1] = xyzold[1];
    xyznew[2] = xyzold[2];
    uvwnew[0] = uvwold[0];
    uvwnew[1] = uvwold[1];
    uvwnew[2] = uvwold[2];

    for(tstep=0.0;tstep<dtstep;tstep+=dt){
      float dvelmin;

      GetShooterVel(uvw_air,xyznew);
      meshpoint = GetMeshNoFail(xyznew);
      if(meshpoint==NULL)meshpoint=global_scase.meshescoll.meshinfo;
      grid_vel =  sqrt(uvwnew[0]*uvwnew[0]+uvwnew[1]*uvwnew[1]+uvwnew[2]*uvwnew[2]);
      if(grid_vel<0.01)grid_vel=0.01;
      dt = MIN(meshpoint->cellsize/grid_vel,dtstep-tstep);

      dvel[0] = uvwnew[0]-uvw_air[0];
      dvel[1] = uvwnew[1]-uvw_air[1];
      dvel[2] = uvwnew[2]-uvw_air[2];

      dvelmin = MIN(ABS(dvel[0]),ABS(dvel[1]));
      dvelmin = MIN(ABS(1.0+dvel[2]),dvelmin);
      if(shooter_v_inf>0.01){
        dt = MIN(dt,0.1*shooter_v_inf/dvelmin);
        uvwnew[0] -= g*dt*(    dvel[0]/shooter_v_inf);
        uvwnew[1] -= g*dt*(    dvel[1]/shooter_v_inf);
        uvwnew[2] -= g*dt*(1.0+dvel[2]/shooter_v_inf);
      }
      else{
        uvwnew[0] = uvw_air[0];
        uvwnew[1] = uvw_air[1];
        uvwnew[2] = uvw_air[2];
      }
      dt=MAX(0.01,0.0);

      xyznew[0] += dt*uvwnew[0];
      xyznew[1] += dt*uvwnew[1];
      xyznew[2] += dt*uvwnew[2];
    }
    pnew[i].visible=1;
    if(GetMeshNoFail(xyznew)==NULL)pnew[i].visible=0;
  }
  shooter_active=1;
}

/* ------------------ InitShooterData ------------------------ */

void InitShooterData(void){
  int i;
  float *xyz, *uvw;
  float xmin, ymin, zmin;

  xmin = shooter_xyz[0]-shooter_dxyz[0]/2.0;
  ymin = shooter_xyz[1]-shooter_dxyz[1]/2.0;
  zmin = shooter_xyz[2]-shooter_dxyz[2]/2.0;

  for(i=0;i<shooter_nparts;i++){
    xyz = shootpointinfo[i].xyz;
    uvw = shootpointinfo[i].uvw;
    shootpointinfo[i].visible=1;
    xyz[0] = xmin + shooter_dxyz[0]*(float)rand()/(float)RAND_MAX;
    xyz[1] = ymin + shooter_dxyz[1]*(float)rand()/(float)RAND_MAX;
    xyz[2] = zmin + shooter_dxyz[2]*(float)rand()/(float)RAND_MAX;
    uvw[0]=shooter_uvw[0];
    uvw[1]=shooter_uvw[1];
    uvw[2]=shooter_uvw[2];
    shootpointinfo[i].prev=NULL;
    shootpointinfo[i].val=0.0;
  }
  shoottimeinfo[0].beg=shootpointinfo;
  shoottimeinfo[0].end=shootpointinfo+shooter_nparts-1;
  shoottimeinfo[0].frame=0;
  shoottimeinfo[0].time=0.0;
  shooter_active=1;
  shooter_time=0.0;
}

/* ------------------ SolveShooterData ------------------------ */

void SolveShooterData(void){
  int i;
  float shooter_dt;

  if(shooter_fps<1){
    shooter_dt=1.0;
  }
  else{
    shooter_dt=1.0/(float)shooter_fps;
  }

  InitShooterData();
  for(i=1;i<nshooter_frames;i++){
    shootpointdata *pold, *pnew;

    pold = shootpointinfo + (i-1)*shooter_nparts;
    pnew = pold + shooter_nparts;
    pnew->prev=pold;
    IncrementShooterData(pold,pnew,shooter_dt);
    shoottimeinfo[i].beg=pold;
    shoottimeinfo[i].end=pnew-1;
    shoottimeinfo[i].frame=i;
    shoottimeinfo[i].time=i*shooter_dt;
  }
}

/* ------------------ DrawShooter ------------------------ */

void DrawShooter(void){
  int i;
  int iframe_local;
  shootpointdata *pb, *pe;
  int nframes;

  iframe_local = shooter_timeslist[itimes];
  pb = shoottimeinfo[iframe_local].beg;
  pe = shoottimeinfo[iframe_local].end;
  nframes = pe + 1 - pb;


  glPointSize(shooterpointsize);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);

  //glColor4fv(static_color);
  glBegin(GL_POINTS);
  for(i=0;i<nframes;i++){
    float *xyz;
    shootpointdata *pbi;

    pbi = pb + i;

    xyz = pbi->xyz;
    if(pbi->visible==1)glVertex3fv(xyz);
  }
  glEnd();
  glPopMatrix();
}
