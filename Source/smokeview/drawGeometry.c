#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOobjects.h"
#include "readimage.h"
#include "readcad.h"
#include "readobject.h"
#include "readsmvfile.h"

#define DRAW_OBSTS_AND_VENTS 0
#define DRAW_OBSTS           1
#define DRAW_VENTS           2

cadgeomdata *current_cadgeom;

/* ------------------ DrawCircVentsApproxSolid ------------------------ */

void DrawCircVentsApproxSolid(int option){
  int i;

  if(option==VENT_HIDE)return;
  assert(option==VENT_CIRCLE||option==VENT_RECTANGLE);

  glBegin(GL_TRIANGLES);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;
    float *xplt, *yplt, *zplt;
    float dx, dy, dz, dxyz;

    meshi = global_scase.meshescoll.meshinfo + i;
    xplt = meshi->xplt_smv;
    yplt = meshi->yplt_smv;
    zplt = meshi->zplt_smv;
    dx = xplt[1] - xplt[0];
    dy = yplt[1] - yplt[0];
    dz = zplt[1] - zplt[0];
    dxyz = MIN(MIN(dx, dy), dz)/10.0;

    for(j=0;j<meshi->ncvents;j++){
      cventdata *cvi;
      int ii, jj, kk;
      float xx, yy, zz;
      float xx2, yy2, zz2;

      cvi = meshi->cventinfo + j;
      if(showpatch==1 && cvi->have_boundary_file == 1)continue;

      // check for visibility

      if(cvi->showtimelist!=NULL&&cvi->showtimelist[itimes]==0)continue;

      glColor3fv(cvi->color);
      if(cvi->dir==UP_X||cvi->dir==UP_Y||cvi->dir==UP_Z){
        dx = dxyz;
      }
      else{
        dx= -dxyz;
      }
      switch(cvi->dir){
        case UP_X:
        case DOWN_X:
          xx=xplt[cvi->imin]+dx;
          for(kk=cvi->kmin;kk<cvi->kmax;kk++){
            zz = zplt[kk];
            zz2 = zplt[kk+1];
            for(jj=cvi->jmin;jj<cvi->jmax;jj++){
              int in_circle;

              yy = yplt[jj];
              yy2 = yplt[jj+1];

              INCIRCLE(xx,(yy+yy2)/2.0,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO)continue;

              glVertex3f(xx, yy,zz);
              glVertex3f(xx,yy2,zz);
              glVertex3f(xx,yy2,zz2);

              glVertex3f(xx, yy,zz);
              glVertex3f(xx,yy2,zz2);
              glVertex3f(xx,yy2,zz);

              glVertex3f(xx, yy,zz);
              glVertex3f(xx,yy2,zz2);
              glVertex3f(xx, yy,zz2);

              glVertex3f(xx, yy,zz);
              glVertex3f(xx, yy,zz2);
              glVertex3f(xx,yy2,zz2);
            }
          }
          break;
        case UP_Y:
        case DOWN_Y:
          yy=yplt[cvi->jmin]+dx;
          for(kk=cvi->kmin;kk<cvi->kmax;kk++){
            zz = zplt[kk];
            zz2 = zplt[kk+1];
            for(ii=cvi->imin;ii<cvi->imax;ii++){
              int in_circle;

              xx = xplt[ii];
              xx2 = xplt[ii+1];

              INCIRCLE((xx+xx2)/2.0,yy,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO)continue;

              glVertex3f( xx,yy,zz);
              glVertex3f(xx2,yy,zz);
              glVertex3f(xx2,yy,zz2);

              glVertex3f( xx,yy,zz);
              glVertex3f(xx2,yy,zz2);
              glVertex3f(xx2,yy,zz);

              glVertex3f( xx,yy,zz);
              glVertex3f(xx2,yy,zz2);
              glVertex3f( xx,yy,zz2);

              glVertex3f( xx,yy,zz);
              glVertex3f( xx,yy,zz2);
              glVertex3f(xx2,yy,zz2);
            }
          }
          break;
        case UP_Z:
        case DOWN_Z:
          zz=zplt[cvi->kmin]+dx;
          for(jj=cvi->jmin;jj<cvi->jmax;jj++){
            yy = yplt[jj];
            yy2 = yplt[jj+1];
            for(ii=cvi->imin;ii<cvi->imax;ii++){
              int in_circle;

              xx = xplt[ii];
              xx2 = xplt[ii+1];

              INCIRCLE((xx+xx2)/2.0,(yy+yy2)/2.0,zz,in_circle);
              if(in_circle==NO)continue;

              glVertex3f( xx, yy,zz);
              glVertex3f(xx2, yy,zz);
              glVertex3f(xx2,yy2,zz);

              glVertex3f( xx, yy,zz);
              glVertex3f(xx2,yy2,zz);
              glVertex3f(xx2, yy,zz);

              glVertex3f( xx, yy,zz);
              glVertex3f(xx2,yy2,zz);
              glVertex3f( xx,yy2,zz);

              glVertex3f( xx, yy,zz);
              glVertex3f( xx,yy2,zz);
              glVertex3f(xx2,yy2,zz);
            }
          }
          break;
        default:
          assert(FFALSE);
          break;
      }
    }
  }
  glEnd();
}

/* ------------------ DrawCircVentsApproxOutline ------------------------ */

void DrawCircVentsApproxOutline(int option){
  int i;

  if(option==VENT_HIDE)return;
  assert(option==VENT_CIRCLE||option==VENT_RECTANGLE);

  glBegin(GL_LINES);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;
    float *xplt, *yplt, *zplt;
    float dx, dy, dz, dxyz;

    meshi = global_scase.meshescoll.meshinfo + i;
    xplt = meshi->xplt_smv;
    yplt = meshi->yplt_smv;
    zplt = meshi->zplt_smv;

    dx = xplt[1] - xplt[0];
    dy = yplt[1] - yplt[0];
    dz = zplt[1] - zplt[0];
    dxyz = MIN(MIN(dx, dy), dz)/10.0;

    for(j=0;j<meshi->ncvents;j++){
      cventdata *cvi;
      int ii, jj, kk;
      float xx0, yy0, zz0;
      float xx, yy, zz;
      float xx2, yy2, zz2;
      float xx3, yy3, zz3;
      int in_circle;

      cvi = meshi->cventinfo + j;

      // check for visibility

      if(cvi->showtimelist!=NULL&&cvi->showtimelist[itimes]==0)continue;
      if(showpatch==1 && cvi->have_boundary_file == 1)continue;

      glColor3fv(cvi->color);
      if(cvi->dir==UP_X||cvi->dir==UP_Y||cvi->dir==UP_Z){
        dx = dxyz;
      }
      else{
        dx = -dxyz;
      }
      switch(cvi->dir){
        case UP_X:
        case DOWN_X:
          xx=xplt[cvi->imin]+dx;
          for(kk=cvi->kmin;kk<cvi->kmax;kk++){
            zz0 = zplt[MAX(kk-1,cvi->kmin)];
            zz = zplt[kk];
            zz2 = zplt[kk+1];
            zz3 = zplt[MIN(kk+2,cvi->kmax)];
            for(jj=cvi->jmin;jj<cvi->jmax;jj++){
              yy0 = yplt[MAX(jj-1,cvi->jmin)];
              yy = yplt[jj];
              yy2 = yplt[jj+1];
              yy3 = yplt[MIN(jj+2,cvi->jmax)];

              INCIRCLE(xx,(yy+yy2)/2.0,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO)continue;

              INCIRCLE(xx,(yy2+yy3)/2.0,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy2,zz);
                glVertex3f(xx,yy2,zz2);
              }

              INCIRCLE(xx,(yy0+yy)/2.0,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz);
                glVertex3f(xx,yy,zz2);
              }

              INCIRCLE(xx,(yy+yy2)/2.0,(zz2+zz3)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz2);
                glVertex3f(xx,yy2,zz2);
              }
              INCIRCLE(xx,(yy+yy2)/2.0,(zz0+zz)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz);
                glVertex3f(xx,yy2,zz);
              }
            }
          }
          break;
        case UP_Y:
        case DOWN_Y:
          yy=yplt[cvi->jmin]+dx;
          for(kk=cvi->kmin;kk<cvi->kmax;kk++){
            zz0 = zplt[MAX(kk-1,cvi->kmin)];
            zz = zplt[kk];
            zz2 = zplt[kk+1];
            zz3 = zplt[MIN(kk+2,cvi->kmax)];
            for(ii=cvi->imin;ii<cvi->imax;ii++){
              xx0 = xplt[MAX(ii-1,cvi->imin)];
              xx = xplt[ii];
              xx2 = xplt[ii+1];
              xx3 = xplt[MIN(ii+2,cvi->imax)];

              INCIRCLE((xx+xx2)/2.0,yy,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO)continue;

              INCIRCLE((xx2+xx3)/2.0,yy,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx2,yy,zz);
                glVertex3f(xx2,yy,zz2);
              }

              INCIRCLE((xx0+xx)/2.0,yy,(zz+zz2)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz);
                glVertex3f(xx,yy,zz2);
              }

              INCIRCLE((xx+xx2)/2.0,yy,(zz2+zz3)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz2);
                glVertex3f(xx2,yy,zz2);
              }

              INCIRCLE((xx+xx2)/2.0,yy,(zz0+zz)/2.0,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz);
                glVertex3f(xx2,yy,zz);
              }
            }
          }
          break;
        case UP_Z:
        case DOWN_Z:
          zz=zplt[cvi->kmin]+dx;
          for(jj=cvi->jmin;jj<cvi->jmax;jj++){
            yy0 = yplt[MAX(jj-1,cvi->jmin)];
            yy = yplt[jj];
            yy2 = yplt[jj+1];
            yy3 = yplt[MIN(jj+2,cvi->jmax)];
            for(ii=cvi->imin;ii<cvi->imax;ii++){
              xx0 = xplt[MAX(ii-1,cvi->imin)];
              xx = xplt[ii];
              xx2 = xplt[ii+1];
              xx3 = xplt[MIN(ii+2,cvi->imax)];

              INCIRCLE((xx+xx2)/2.0,(yy+yy2)/2.0,zz,in_circle);
              if(in_circle==NO)continue;

              INCIRCLE((xx2+xx3)/2.0,(yy+yy2)/2.0,zz,in_circle);
              if(in_circle==NO){
                glVertex3f(xx2,yy,zz);
                glVertex3f(xx2,yy2,zz);
              }

              INCIRCLE((xx0+xx)/2.0,(yy+yy2)/2.0,zz,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz);
                glVertex3f(xx,yy2,zz);
              }

              INCIRCLE((xx+xx2)/2.0,(yy2+yy3)/2.0,zz,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy2,zz);
                glVertex3f(xx2,yy2,zz);
              }

              INCIRCLE((xx+xx2)/2.0,(yy0+yy)/2.0,zz,in_circle);
              if(in_circle==NO){
                glVertex3f(xx,yy,zz);
                glVertex3f(xx2,yy,zz);
              }
            }
          }
          break;
        default:
          assert(FFALSE);
          break;
      }
    }
  }
  glEnd();
}

/* ------------------ DrawCircVentsExactSolid ------------------------ */

void DrawCircVentsExactSolid(int option){
  int i;

  if(option==VENT_HIDE)return;
  assert(option==VENT_CIRCLE||option==VENT_RECTANGLE);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->ncvents;j++){
      cventdata *cvi;
      float x0, yy0, z0;
      unsigned char vcolor[3];
      float delta;
      float *color;
      float width, height;

      cvi = meshi->cventinfo + j;

      // check for visibility

      if(cvi->showtimelist!=NULL&&cvi->showtimelist[itimes]==0)continue;
      if(showpatch==1 && cvi->have_boundary_file == 1)continue;

      if(option==VENT_CIRCLE){
        x0 = cvi->origin[0];
        yy0 = cvi->origin[1];
        z0 = cvi->origin[2];
      }
      else{
        x0 = cvi->xmin;
        yy0 = cvi->ymin;
        z0 = cvi->zmin;
      }

      delta=0.001;
      color=cvi->color;
      vcolor[0]=color[0]*255;
      vcolor[1]=color[1]*255;
      vcolor[2]=color[2]*255;
      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
      if(option==VENT_CIRCLE){
        clipdata circleclip;
        float *ventmin, *ventmax;

        ventmin=cvi->boxmin;
        ventmax=cvi->boxmax;

        InitBoxClipInfo(&circleclip,ventmin[0],ventmax[0],ventmin[1],ventmax[1],ventmin[2],ventmax[2]);
        MergeClipPlanes(&circleclip,&clipinfo);
        SetClipPlanes(&circleclip,CLIP_ON_DENORMAL);
      }
      glTranslatef(x0,yy0,z0);
      width  = 0.0;
      height = 0.0;
      switch(cvi->dir){
        case DOWN_X:
          glTranslatef(-delta,0.0,0.0);
          glRotatef(-90.0,0.0,1.0,0.0);
          width  += cvi->ymax-cvi->ymin;
          height += cvi->zmax-cvi->zmin;
          break;
        case UP_X:
          glTranslatef(delta,0.0,0.0);
          glRotatef(-90.0,0.0,1.0,0.0);
          width  += cvi->ymax-cvi->ymin;
          height += cvi->zmax-cvi->zmin;
          break;
        case DOWN_Y:
          glTranslatef(0.0,-delta,0.0);
          glRotatef(90.0,1.0,0.0,0.0);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->zmax-cvi->zmin;
          break;
        case UP_Y:
          glTranslatef(0.0,delta,0.0);
          glRotatef(90.0,1.0,0.0,0.0);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->zmax-cvi->zmin;
          break;
        case DOWN_Z:
          glTranslatef(0.0,0.0,-delta);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->ymax-cvi->ymin;
          break;
        case UP_Z:
          glTranslatef(0.0,0.0,delta);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->ymax-cvi->ymin;
          break;
        default:
          assert(FFALSE);
          break;
      }
      if(option==VENT_CIRCLE){
        if(cvi->type==VENT_SOLID)DrawFilledCircle(2.0*cvi->radius,vcolor,&cvent_circ);
        if(cvi->type==VENT_OUTLINE)DrawCircle(2.0*cvi->radius,vcolor,&cvent_circ);
      }
      if(option==VENT_RECTANGLE){
        if(cvi->type==VENT_SOLID)DrawFilledRectangle(width,height,vcolor);
        if(cvi->type==VENT_OUTLINE)DrawRectangle(width,height,vcolor);
      }
      glPopMatrix();
      if(option==VENT_CIRCLE)SetClipPlanes(&clipinfo,CLIP_ON);
    }
  }
}

/* ------------------ DrawCircVentsExactOutline ------------------------ */

void DrawCircVentsExactOutline(int option){
  int i;

  if(option==VENT_HIDE)return;
  assert(option==VENT_CIRCLE||option==VENT_RECTANGLE);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->ncvents;j++){
      cventdata *cvi;
      float x0, yy0, z0;
      unsigned char vcolor[3];
      float delta;
      float *color;
      float width, height;

      cvi = meshi->cventinfo + j;

      // check for visibility

      if(cvi->showtimelist!=NULL&&cvi->showtimelist[itimes]==0)continue;
      if(showpatch==1 && cvi->have_boundary_file == 1)continue;

      if(option==VENT_CIRCLE){
        x0 = cvi->origin[0];
        yy0 = cvi->origin[1];
        z0 = cvi->origin[2];
      }
      else{
        x0 = cvi->xmin;
        yy0 = cvi->ymin;
        z0 = cvi->zmin;
      }

      delta=0.001;
      color=cvi->color;
      vcolor[0]=color[0]*255;
      vcolor[1]=color[1]*255;
      vcolor[2]=color[2]*255;
      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
      if(option==VENT_CIRCLE){
        clipdata circleclip;
        float *ventmin, *ventmax;

        ventmin=cvi->boxmin;
        ventmax=cvi->boxmax;

        InitBoxClipInfo(&circleclip,ventmin[0],ventmax[0],ventmin[1],ventmax[1],ventmin[2],ventmax[2]);
        MergeClipPlanes(&circleclip,&clipinfo);
        SetClipPlanes(&circleclip,CLIP_ON_DENORMAL);
      }
      glTranslatef(x0,yy0,z0);
      width  = 0.0;
      height = 0.0;
      switch(cvi->dir){
        case DOWN_X:
          glTranslatef(-delta,0.0,0.0);
          glRotatef(-90.0,0.0,1.0,0.0);
          width  += cvi->ymax-cvi->ymin;
          height += cvi->zmax-cvi->zmin;
          break;
        case UP_X:
          glTranslatef(delta,0.0,0.0);
          glRotatef(-90.0,0.0,1.0,0.0);
          width  += cvi->ymax-cvi->ymin;
          height += cvi->zmax-cvi->zmin;
          break;
        case DOWN_Y:
          glTranslatef(0.0,-delta,0.0);
          glRotatef(90.0,1.0,0.0,0.0);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->zmax-cvi->zmin;
          break;
        case UP_Y:
          glTranslatef(0.0,delta,0.0);
          glRotatef(90.0,1.0,0.0,0.0);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->zmax-cvi->zmin;
          break;
        case DOWN_Z:
          glTranslatef(0.0,0.0,-delta);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->ymax-cvi->ymin;
          break;
        case UP_Z:
          glTranslatef(0.0,0.0,delta);
          width  += cvi->xmax-cvi->xmin;
          height += cvi->ymax-cvi->ymin;
          break;
        default:
          assert(FFALSE);
          break;
      }
      if(option==VENT_CIRCLE){
        DrawCircle(2.0*cvi->radius,vcolor,&cvent_circ);
      }
      if(option==VENT_RECTANGLE){
        DrawRectangle(width,height,vcolor);
      }
      glPopMatrix();
      if(option==VENT_CIRCLE)SetClipPlanes(&clipinfo,CLIP_ON);
    }
  }
}

/* ------------------ DrawCircVents ------------------------ */

void DrawCircVents(int option){
  if(option==VENT_HIDE||cvents_defined==0)return;
  if(blocklocation==BLOCKlocation_grid&&visCircularVents!=VENT_RECTANGLE){
    if(circle_outline==0)DrawCircVentsApproxSolid(option);
    if(circle_outline==1)DrawCircVentsApproxOutline(option);
  }
  else{
    if(circle_outline==0)DrawCircVentsExactSolid(option);
    if(circle_outline==1)DrawCircVentsExactOutline(option);
  }
}

/* ------------------ UpdateIndexolors ------------------------ */

void UpdateIndexColors(void){
  int i;
  int colorindex;
  int j;
  float s_color[4];

  global_scase.updateindexcolors=0;

  if(strcmp(global_scase.surfacedefault->surfacelabel,"INERT")==0){
    global_scase.surfacedefault->color=global_scase.color_defs.block_ambient2;
  }
  for(i=0;i<global_scase.surfcoll.nsurfinfo;i++){
    surfdata *surfi;

    surfi = global_scase.surfcoll.surfinfo + i;
    if(strcmp(surfi->surfacelabel,"INERT")==0){
      surfi->color=global_scase.color_defs.block_ambient2;
    }
    if(strcmp(surfi->surfacelabel,"OPEN")==0){
      surfi->color=global_scase.color_defs.ventcolor;
    }
  }

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[j];
      if(bc->usecolorindex==1){
        colorindex=bc->colorindex;
        if(colorindex>=0){
          bc->color = GetColorPtr(&global_scase, global_scase.rgb[global_scase.nrgb+colorindex]);
        }
      }
    }
    for(j=0;j<meshi->nvents;j++){
      ventdata *vi;

      vi = meshi->ventinfo + j;
      if(vi->usecolorindex==1){
        colorindex=vi->colorindex;

        s_color[0]=global_scase.rgb[global_scase.nrgb+colorindex][0];
        s_color[1]=global_scase.rgb[global_scase.nrgb+colorindex][1];
        s_color[2]=global_scase.rgb[global_scase.nrgb+colorindex][2];
        s_color[3]=1.0;
        vi->color = GetColorPtr(&global_scase, s_color);
      }
    }
  }
  global_scase.updatefaces=1;
}


/* ------------------ DrawObstOutlines ------------------------ */

void DrawObstOutlines(void){
  int n;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
  AntiAliasLine(ON);
  glLineWidth(global_scase.linewidth);
  glBegin(GL_LINES);
  for(n = 0; n < global_scase.meshescoll.nmeshes; n++){
    int i;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    meshdata *meshi;
    float *color, *oldcolor=NULL;
    float *xplt, *yplt, *zplt;

    meshi = global_scase.meshescoll.meshinfo + n;
    xplt = meshi->xplt_fds;
    yplt = meshi->yplt_fds;
    zplt = meshi->zplt_fds;
    for(i = 0;i < meshi->nbptrs;i++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[i];
      if(bc == NULL)continue;
      if(bc->showtimelist != NULL && bc->showtimelist[itimes] == 0)continue;
      color = bc->color;
      if(color != oldcolor){
        glColor3fv(color);
        oldcolor = color;
      }
      xmin = xplt[bc->ijk[0]];
      xmax = xplt[bc->ijk[1]];
      ymin = yplt[bc->ijk[2]];
      ymax = yplt[bc->ijk[3]];
      zmin = zplt[bc->ijk[4]];
      zmax = zplt[bc->ijk[5]];
      glVertex3f(xmin, ymin, zmin);
      glVertex3f(xmin, ymin, zmax);
      glVertex3f(xmax, ymin, zmin);
      glVertex3f(xmax, ymin, zmax);
      glVertex3f(xmin, ymax, zmin);
      glVertex3f(xmin, ymax, zmax);
      glVertex3f(xmax, ymax, zmin);
      glVertex3f(xmax, ymax, zmax);

      glVertex3f(xmin, ymin, zmin);
      glVertex3f(xmin, ymax, zmin);
      glVertex3f(xmax, ymin, zmin);
      glVertex3f(xmax, ymax, zmin);
      glVertex3f(xmin, ymin, zmax);
      glVertex3f(xmin, ymax, zmax);
      glVertex3f(xmax, ymin, zmax);
      glVertex3f(xmax, ymax, zmax);

      glVertex3f(xmin, ymin, zmin);
      glVertex3f(xmax, ymin, zmin);
      glVertex3f(xmin, ymax, zmin);
      glVertex3f(xmax, ymax, zmin);
      glVertex3f(xmin, ymin, zmax);
      glVertex3f(xmax, ymin, zmax);
      glVertex3f(xmin, ymax, zmax);
      glVertex3f(xmax, ymax, zmax);
    }
  }
  glEnd();
  AntiAliasLine(OFF);
  glPopMatrix();
}

/* ------------------ DrawOrigObstOutlines ------------------------ */

void DrawOrigObstOutlines(void){
  int i;
  float *color, *oldcolor=NULL;


  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
  AntiAliasLine(ON);
  glLineWidth(global_scase.linewidth);
  glBegin(GL_LINES);
  for(i=0; i<global_scase.obstcoll.nobstinfo; i++){
    xbdata *obi;
    float *xyz;
    float xmin, xmax, ymin, ymax, zmin, zmax;

    obi = global_scase.obstcoll.obstinfo + i;
    color = foregroundcolor;
    if(obi->bc!=NULL&&obi->bc->showtimelist!=NULL&&obi->bc->showtimelist[itimes]==0)continue;
    if(obi->color!=NULL)color = obi->color;
    if(obi->color==NULL&&obi->surfs[0]->color!=NULL)color = obi->surfs[0]->color;
    if(color!=oldcolor){
      glColor3fv(color);
      oldcolor = color;
    }
    xyz = obi->xyz;
    xmin = xyz[0];
    xmax = xyz[1];
    ymin = xyz[2];
    ymax = xyz[3];
    zmin = xyz[4];
    zmax = xyz[5];
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmax);

    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmax);

    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmax, ymax, zmax);
  }
  glEnd();
  AntiAliasLine(OFF);
  glPopMatrix();
}

/* ------------------ DrawOutlines ------------------------ */

void DrawOutlines(void){
  int i;

  if(global_scase.noutlineinfo<=0)return;
  AntiAliasLine(ON);
  glLineWidth(global_scase.linewidth);
  glBegin(GL_LINES);
  glColor3fv(foregroundcolor);
  for(i=0;i<global_scase.noutlineinfo;i++){
    outlinedata *outlinei;
    float *xx1, *yy1, *zz1;
    float *xx2, *yy2, *zz2;
    int j;

    outlinei = global_scase.outlineinfo + i;
    xx1 = outlinei->x1;
    xx2 = outlinei->x2;
    yy1 = outlinei->y1;
    yy2 = outlinei->y2;
    zz1 = outlinei->z1;
    zz2 = outlinei->z2;
    for(j=0;j<outlinei->nlines;j++){
      glVertex3f(*xx1++,*yy1++,*zz1++);
      glVertex3f(*xx2++,*yy2++,*zz2++);
    }
  }
  glEnd();
  AntiAliasLine(OFF);
}

/* ------------------ DrawCBox ------------------------ */

void DrawCBox(float x, float y, float z, float size){
  float xx[8], yy[8], zz[8];
  float xbound[2],ybound[2],zbound[2];
  int i;
  int ix[8]={0,1,1,0,0,1,1,0};
  int iy[8]={0,0,0,0,1,1,1,1};
  int iz[8]={0,0,1,1,0,0,1,1};
  float dsize=SCALE2SMV(size);

  xbound[0]=x-dsize/2.0;
  ybound[0]=y-dsize/2.0;
  zbound[0]=z-dsize/2.0;
  xbound[1]=x+dsize/2.0;
  ybound[1]=y+dsize/2.0;
  zbound[1]=z+dsize/2.0;
  for(i=0;i<8;i++){
    xx[i]=xbound[ix[i]];
    yy[i]=ybound[iy[i]];
    zz[i]=zbound[iz[i]];
  }

  glVertex3f(xx[0],yy[0],zz[0]);
  glVertex3f(xx[1],yy[1],zz[1]);
  glVertex3f(xx[5],yy[5],zz[5]);
  glVertex3f(xx[4],yy[4],zz[4]);

  glVertex3f(xx[1],yy[1],zz[1]);
  glVertex3f(xx[2],yy[2],zz[2]);
  glVertex3f(xx[6],yy[6],zz[6]);
  glVertex3f(xx[5],yy[5],zz[5]);

  glVertex3f(xx[2],yy[2],zz[2]);
  glVertex3f(xx[3],yy[3],zz[3]);
  glVertex3f(xx[7],yy[7],zz[7]);
  glVertex3f(xx[6],yy[6],zz[6]);

  glVertex3f(xx[3],yy[3],zz[3]);
  glVertex3f(xx[0],yy[0],zz[0]);
  glVertex3f(xx[4],yy[4],zz[4]);
  glVertex3f(xx[7],yy[7],zz[7]);

  glVertex3f(xx[0],yy[0],zz[0]);
  glVertex3f(xx[3],yy[3],zz[3]);
  glVertex3f(xx[2],yy[2],zz[2]);
  glVertex3f(xx[1],yy[1],zz[1]);

  glVertex3f(xx[4],yy[4],zz[4]);
  glVertex3f(xx[5],yy[5],zz[5]);
  glVertex3f(xx[6],yy[6],zz[6]);
  glVertex3f(xx[7],yy[7],zz[7]);
}

/* ------------------ GetBlockVals ------------------------ */

void GetBlockVals(  float *xmin, float *xmax,
                     float *ymin, float *ymax,
                     float *zmin, float *zmax,
                     int *imin, int *jmin, int *kmin){
  blockagedata *bc;

  bc=bchighlight;
  if(bc==NULL){
    *xmin = 0.0;
    *xmax = 0.0;
    *ymin = 0.0;
    *ymax = 0.0;
    *zmin = 0.0;
    *zmax = 0.0;
    *imin = 0;
    *jmin = 0;
    *kmin = 0;
    return;
  }
  if(blockage_as_input==1){
    float *xyz;

    xyz = bc->xyzEXACT;
    *xmin = xyz[0];
    *xmax = xyz[1];
    *ymin = xyz[2];
    *ymax = xyz[3];
    *zmin = xyz[4];
    *zmax = xyz[5];
  }
  else{
    *xmin = current_mesh->xplt_fds[bc->ijk[IMIN]]-current_mesh->offset[XXX];
    *xmax = current_mesh->xplt_fds[bc->ijk[IMAX]]-current_mesh->offset[XXX];
    *ymin = current_mesh->yplt_fds[bc->ijk[JMIN]]-current_mesh->offset[YYY];
    *ymax = current_mesh->yplt_fds[bc->ijk[JMAX]]-current_mesh->offset[YYY];
    *zmin = current_mesh->zplt_fds[bc->ijk[KMIN]]-current_mesh->offset[ZZZ];
    *zmax = current_mesh->zplt_fds[bc->ijk[KMAX]]-current_mesh->offset[ZZZ];
  }
  *imin = bc->ijk[IMIN];
  *jmin = bc->ijk[JMIN];
  *kmin = bc->ijk[KMIN];

}

/* ------------------ HaveCircularVents ------------------------ */

int HaveCircularVents(void){
  int i;

  for(i = 0; i < global_scase.meshescoll.nmeshes; i++){
    meshdata* meshi;

    meshi = global_scase.meshescoll.meshinfo + i;

    if(meshi->ncvents > 0)return 1;
  }
  return 0;
}

      /* ------------------ SetCVentDirs ------------------------ */

void SetCVentDirs(void){
  int ii;

  InitCircle(90,&cvent_circ);
  for(ii=0;ii<global_scase.meshescoll.nmeshes;ii++){
    meshdata *meshi;
    int ibar, jbar;
    char *c_iblank;
    int iv;

    meshi=global_scase.meshescoll.meshinfo+ii;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    c_iblank = meshi->c_iblank_cell;

    for(iv=0;iv<meshi->ncvents;iv++){
      cventdata *cvi;
      int dir;
      int orien;
      float *boxmin, *boxmax;

      cvi=meshi->cventinfo+iv;
      boxmin = cvi->boxmin;
      boxmax = cvi->boxmax;

      dir=0;
      if(cvi->imin==cvi->imax)dir=XDIR;
      if(cvi->jmin==cvi->jmax)dir=YDIR;
      if(cvi->kmin==cvi->kmax)dir=ZDIR;

      boxmin[0]=cvi->xmin;
      boxmin[1]=cvi->ymin;
      boxmin[2]=cvi->zmin;
      boxmax[0]=cvi->xmax;
      boxmax[1]=cvi->ymax;
      boxmax[2]=cvi->zmax;

      if(dir!=0){
        boxmin[dir-1]-=0.1;
        boxmax[dir-1]+=0.1;
      }

      int ventdir;
      switch(dir){
      case XDIR:
        if(cvi->imin==0){
          orien=1;
        }
        else if(cvi->imin==meshi->ibar){
          orien=-1;
        }
        else{
          int breakloop;
          int i,j;

          orien=1;
          i=cvi->imin;
          breakloop=0;
          for(j=cvi->jmin;j<=cvi->jmax;j++){
            int k;

            for(k=cvi->kmin;k<=cvi->kmax;k++){
              int state1, state2;

              if(global_scase.use_iblank==1&&c_iblank!=NULL){
                state1=c_iblank[IJKCELL(i-1,j,k)];
                state2=c_iblank[IJKCELL(i,j,k)];
              }
              else{
                state1=GAS;
                state2=GAS;
              }
              if(state1==GAS  &&state2==GAS)continue; // air on both sides
              if(state1==SOLID&&state2==SOLID)continue; // solid on both sides
              if(state1==GAS  &&state2==SOLID){
                orien=-1;
              }
              breakloop=1;
              break;
            }
            if(breakloop==1)break;
          }
        }
        if(orien==1){
          ventdir=UP_X;
        }
        else{
          ventdir=DOWN_X;
        }
        cvi->dir=ventdir;
        break;
      case YDIR:
        if(cvi->jmin==0){
          orien=1;
        }
        else if(cvi->jmin==meshi->jbar){
          orien=-1;
        }
        else{
          int breakloop;
          int i, j;

          orien=1;
          j=cvi->jmin;
          breakloop=0;
          for(i=cvi->imin;i<=cvi->imax;i++){
            int k;

            for(k=cvi->kmin;k<=cvi->kmax;k++){
              int state1, state2;

              if(global_scase.use_iblank==1&&c_iblank!=NULL){
                state1=c_iblank[IJKCELL(i,j-1,k)];
                state2=c_iblank[IJKCELL(i,j,k)];
              }
              else{
                state1=GAS;
                state2=GAS;
              }
              if(state1==GAS  &&state2==GAS)continue; // air on both sides
              if(state1==SOLID&&state2==SOLID)continue; // solid on both sides
              if(state1==GAS  &&state2==SOLID){
                orien=-1;
              }
              breakloop=1;
              break;
            }
            if(breakloop==1)break;
          }
        }
        if(orien==1){
          ventdir=UP_Y;
        }
        else{
          ventdir=DOWN_Y;
        }
        cvi->dir=ventdir;
        break;
      case ZDIR:
        if(cvi->kmin==0){
          orien=1;
        }
        else if(cvi->kmin==meshi->kbar){
          orien=-1;
        }
        else{
          int breakloop;
          int i, k;

          orien=1;
          k=cvi->kmin;
          breakloop=0;
          for(i=cvi->imin;i<=cvi->imax;i++){
            int j;

            for(j=cvi->jmin;j<=cvi->jmax;j++){
              int state1, state2;

              if(global_scase.use_iblank==1&&c_iblank!=NULL){
                state1=c_iblank[IJKCELL(i,j,k-1)];
                state2=c_iblank[IJKCELL(i,j,k)];
              }
              else{
                state1=GAS;
                state2=GAS;
              }
              if(state1==GAS  &&state2==GAS)continue; // air on both sides
              if(state1==SOLID&&state2==SOLID)continue; // solid on both sides
              if(state1==GAS  &&state2==SOLID){
                orien=-1;
              }
              breakloop=1;
              break;
            }
            if(breakloop==1)break;
          }
        }
        if(orien==1){
          ventdir=UP_Z;
        }
        else{
          ventdir=DOWN_Z;
        }
        cvi->dir=ventdir;
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
  }

  // set up blanking arrays for circular vents

  for(ii=0;ii<global_scase.meshescoll.nmeshes;ii++){
    meshdata *meshi;
    int iv,i,j,k;
    unsigned char *blank;
    float *xplt, *yplt, *zplt;

    meshi=global_scase.meshescoll.meshinfo+ii;

    xplt = meshi->xplt_cen_smv;
    yplt = meshi->yplt_cen_smv;
    zplt = meshi->zplt_cen_smv;
    for(iv = 0;iv < meshi->ncvents;iv++){
      cventdata *cvi;
      int nx=0, ny=0;

      cvi = meshi->cventinfo + iv;

      switch(cvi->dir){
      case UP_X:
      case DOWN_X:
        nx = cvi->jmax - cvi->jmin;
        ny = cvi->kmax - cvi->kmin;
        break;
      case UP_Y:
      case DOWN_Y:
        nx = cvi->imax - cvi->imin;
        ny = cvi->kmax - cvi->kmin;
        break;
      case UP_Z:
      case DOWN_Z:
        nx = cvi->imax - cvi->imin;
        ny = cvi->jmax - cvi->jmin;
        break;
      default:
        assert(FFALSE);
        break;
      }
      nx += 2;
      ny += 2;
      NewMemory((void **)&cvi->blank, nx * ny * sizeof(unsigned char));
      blank = cvi->blank;
      for(j = 0;j < ny;j++){
        for(i = 0;i < nx;i++){
          if(i == 0 || j == 0 || i == nx - 1 || j == ny - 1){
            blank[IJCIRC(i - 1, j - 1)] = 0;
          }
          else{
            blank[IJCIRC(i - 1, j - 1)] = 1;
          }
        }
      }

      blank = cvi->blank;
      if(blank != NULL){
        switch(cvi->dir){
        case DOWN_X:
        case UP_X:
          for(k = cvi->kmin;k < cvi->kmax;k++){
            float dz;

            dz = zplt[k] - FDS2SMV_Z(cvi->origin[2]);
            for(j = cvi->jmin;j < cvi->jmax;j++){
              float dy;
              float drad;

              dy = yplt[j] - FDS2SMV_Y(cvi->origin[1]);
              drad = sqrt(dy * dy + dz * dz);
              if(SCALE2SMV(drad) > cvi->radius){
                blank[IJCIRC(j - cvi->jmin, k - cvi->kmin)] = 0;
              }
            }
          }
          break;
        case DOWN_Y:
        case UP_Y:
          for(k = cvi->kmin;k < cvi->kmax;k++){
            float dz;

            dz = zplt[k] - FDS2SMV_Z(cvi->origin[2]);
            for(i = cvi->imin;i < cvi->imax;i++){
              float dx;
              float drad;

              dx = xplt[i] - FDS2SMV_X(cvi->origin[0]);
              drad = sqrt(dx * dx + dz * dz);
              if(SCALE2SMV(drad) > cvi->radius){
                blank[IJCIRC(i - cvi->imin, k - cvi->kmin)] = 0;
              }
            }
          }
          break;
        case DOWN_Z:
        case UP_Z:
          for(j = cvi->jmin;j < cvi->jmax;j++){
            float dy;

            dy = yplt[j] - FDS2SMV_Y(cvi->origin[1]);
            for(i = cvi->imin;i < cvi->imax;i++){
              float dx;
              float drad;

              dx = xplt[i] - FDS2SMV_X(cvi->origin[0]);
              drad = sqrt(dx * dx + dy * dy);
              if(SCALE2SMV(drad) > cvi->radius){
                blank[IJCIRC(i - cvi->imin, j - cvi->jmin)] = 0;
              }
              else{
                blank[IJCIRC(i - cvi->imin, j - cvi->jmin)] = 1;
              }
            }
          }
          break;
        default:
          assert(FFALSE);
          break;
        }
      }
    }
  }

  for(ii = 0; ii < global_scase.meshescoll.nmeshes; ii++){
    meshdata *meshi;
    int iv;

    meshi = global_scase.meshescoll.meshinfo + ii;
    for(iv = 0; iv < meshi->ncvents; iv++){
      cventdata *cvi;

      cvi = meshi->cventinfo + iv;
      cvi->blank = cvi->blank0;
    }
  }
  cvents_defined = 1;
}

/* ------------------ CheckVentDup ------------------------ */

int CheckVentDup(ventdata* vi, meshdata* meshi){
  int i;
  int nreal_vents;

  nreal_vents = meshi->nvents - meshi->ndummyvents;
  if(nreal_vents == 0)return 0;

  for(i = 0; i < nreal_vents; i++){
    ventdata *vi2;

    vi2 = meshi->ventinfo + i;
    if(vi2->imin != vi->imin || vi2->imax != vi->imax)return 0;
    if(vi2->jmin != vi->jmin || vi2->jmax != vi->jmax)return 0;
    if(vi2->kmin != vi->kmin || vi2->kmax != vi->kmax)return 0;
  }
  return 1;
}

/* ------------------ SetVentDirs ------------------------ */

void SetVentDirs(void){
  int ii;

  INIT_PRINT_TIMER(vent_setup_timer);
  n_mirrorvents = 0;
  n_openvents = 0;
  for(ii=0;ii<global_scase.meshescoll.nmeshes;ii++){
    meshdata *meshi;
    float *xplttemp;
    float *yplttemp;
    float *zplttemp;
    int ibar, jbar, kbar;
    char *c_iblank;
    int orien;
    int iv;
    int dir;
    int i, j, k;
    int breakloop;
    int ventdir;
    float voffset, offset;

    meshi=global_scase.meshescoll.meshinfo+ii;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;
    c_iblank = meshi->c_iblank_cell;
    xplttemp=meshi->xplt_smv;
    yplttemp=meshi->yplt_smv;
    zplttemp=meshi->zplt_smv;

    for(iv=0;iv<meshi->nvents+12;iv++){
      ventdata *vi;

      vi=meshi->ventinfo+iv;
      if(vi->dir!=DIR_UNDEFINED)continue;

      if(vi->isMirrorvent==1)n_mirrorvents++; // count number of mirror and open vents
      if(vi->isOpenvent==1)n_openvents++;
      dir=0;
      if(vi->imin==vi->imax)dir=XDIR;
      if(vi->jmin==vi->jmax)dir=YDIR;
      if(vi->kmin==vi->kmax)dir=ZDIR;

      switch(dir){
      case XDIR:
        vi->dir2=XDIR;
        offset=ventoffset_factor*(xplttemp[1]-xplttemp[0]);
        if(vi->imin==0){
          orien=1;
        }
        else if(vi->imin==meshi->ibar){
          orien=-1;
        }
        else{
          orien=1;
          i=vi->imin;
          breakloop=0;
          for(j=vi->jmin;j<=MIN(vi->jmax,jbar-1);j++){
            for(k=vi->kmin;k<=MIN(vi->kmax,kbar-1);k++){
              int state1, state2;

              if(global_scase.use_iblank==1&&c_iblank!=NULL){
                state1=c_iblank[IJKCELL(i-1,j,k)];
                state2=c_iblank[IJKCELL(i,j,k)];
              }
              else{
                state1=GAS;
                state2=GAS;
              }
              if(state1==GAS  &&state2==GAS)continue; // air on both sides
              if(state1==SOLID&&state2==SOLID)continue; // solid on both sides
              if(state1==GAS  &&state2==SOLID){
                orien=-1;
              }
              breakloop=1;
              break;
            }
            if(breakloop==1)break;
          }
        }
        if(orien==1){
          ventdir=UP_X;
          voffset=offset;
        }
        else{
          ventdir=DOWN_X;
          voffset=-offset;
        }
        if(iv<meshi->nvents)vi->dir=ventdir;
        if(vi->dummy != 0){
          if(CheckVentDup(vi, meshi)==1)vi->dummy = 0;
        }
        if(vi->dummy==0){
          vi->xvent1 = vi->xvent1_orig+voffset;
          vi->xvent2 = vi->xvent2_orig+voffset;
        }
        break;
      case YDIR:
        vi->dir2=YDIR;
        offset=ventoffset_factor*(yplttemp[1]-yplttemp[0]);
        if(vi->jmin==0){
          orien=1;
        }
        else if(vi->jmin==meshi->jbar){
          orien=-1;
        }
        else{
          orien=1;
          j=vi->jmin;
          breakloop=0;
          for(i=vi->imin;i<=MIN(vi->imax,ibar-1);i++){
            for(k=vi->kmin;k<=MIN(vi->kmax,kbar-1);k++){
              int state1, state2;

              if(global_scase.use_iblank==1&&c_iblank!=NULL){
                state1=c_iblank[IJKCELL(i,j-1,k)];
                state2=c_iblank[IJKCELL(i,j,k)];
              }
              else{
                state1=GAS;
                state2=GAS;
              }
              if(state1==GAS  &&state2==GAS)continue; // air on both sides
              if(state1==SOLID&&state2==SOLID)continue; // solid on both sides
              if(state1==GAS  &&state2==SOLID){
                orien=-1;
              }
              breakloop=1;
              break;
            }
            if(breakloop==1)break;
          }
        }
        if(orien==1){
          ventdir=UP_Y;
          voffset=offset;
        }
        else{
          ventdir=DOWN_Y;
          voffset=-offset;
        }
        if(iv<meshi->nvents)vi->dir=ventdir;
        if(vi->dummy != 0){
          if(CheckVentDup(vi, meshi)==1)vi->dummy = 0;
        }
        if(vi->dummy==0){
          vi->yvent1 = vi->yvent1_orig+voffset;
          vi->yvent2 = vi->yvent2_orig+voffset;
        }
        break;
      case ZDIR:
        vi->dir2=ZDIR;
        offset=ventoffset_factor*(zplttemp[1]-zplttemp[0]);
        if(vi->kmin==0){
          orien=1;
        }
        else if(vi->kmin==meshi->kbar){
          orien=-1;
        }
        else{
          orien=1;
          k=vi->kmin;
          breakloop=0;
          for(i=vi->imin;i<=MIN(vi->imax,ibar-1);i++){
            for(j=vi->jmin;j<=MIN(vi->jmax,jbar-1);j++){
              int state1, state2;

              if(global_scase.use_iblank==1&&c_iblank!=NULL){
                state1=c_iblank[IJKCELL(i,j,k-1)];
                state2=c_iblank[IJKCELL(i,j,k)];
              }
              else{
                state1=GAS;
                state2=GAS;
              }
              if(state1==GAS  &&state2==GAS)continue; // air on both sides
              if(state1==SOLID&&state2==SOLID)continue; // solid on both sides
              if(state1==GAS  &&state2==SOLID){
                orien=-1;
              }
              breakloop=1;
              break;
            }
            if(breakloop==1)break;
          }
        }
        if(orien==1){
          ventdir=UP_Z;
          voffset=offset;
        }
        else{
          ventdir=DOWN_Z;
          voffset=-offset;
        }
        if(iv<meshi->nvents)vi->dir=ventdir;
        if(vi->dummy != 0){
          if(CheckVentDup(vi, meshi)==1)vi->dummy = 0;
        }
        if(vi->dummy==0){
          vi->zvent1 = vi->zvent1_orig+voffset;
          vi->zvent2 = vi->zvent2_orig+voffset;
        }
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
  }
  PRINT_TIMER(vent_setup_timer, "set vent directions");
}

/* ------------------ InBlockage ------------------------ */

int InBlockage(const meshdata *meshi,float x, float y, float z){
  int i;
  float *xplt, *yplt, *zplt;

  xplt=meshi->xplt_smv;
  yplt=meshi->yplt_smv;
  zplt=meshi->zplt_smv;

  for(i=0;i<meshi->nbptrs;i++){
    blockagedata *bc;
    float xmin, xmax, ymin, ymax, zmin, zmax;

    bc=meshi->blockageinfoptrs[i];
    xmin = xplt[bc->ijk[IMIN]]; xmax = xplt[bc->ijk[IMAX]];
    ymin = yplt[bc->ijk[JMIN]]; ymax = yplt[bc->ijk[JMAX]];
    zmin = zplt[bc->ijk[KMIN]]; zmax = zplt[bc->ijk[KMAX]];
    if(xmin<x && x<xmax && ymin<y && y<ymax && zmin<z && z<zmax){
      return 1;
    }
  }
  return 0;
}

/* ------------------ InAnyBlockage ------------------------ */

int InAnyBlockage(float *xyz){
  int i;

  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo+i;
    if(InBlockage(meshi, xyz[0], xyz[1], xyz[2])==1)return 1;
  }
  return 0;
}

/* ------------------ SetInteriorBlockages ------------------------ */

void SetInteriorBlockages(void){
  int i;

  for(i=0; i<global_scase.meshescoll.nmeshes; i++){
    int j;
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j=0; j<meshi->nbptrs; j++){
      blockagedata *bc;
      int k;
      float *xyzDELTA;

      bc = meshi->blockageinfoptrs[j];
      for(k=0; k<6; k++){
        bc->interior[k] = 0;
      }
      xyzDELTA = bc->xyzDELTA;

      xyzDELTA[0] = bc->xmin-meshi->boxeps_smv[0];
      xyzDELTA[1] = (bc->ymin+bc->ymax)/2.0;
      xyzDELTA[2] = (bc->zmin+bc->zmax)/2.0;

      xyzDELTA += 3;
      xyzDELTA[0] = bc->xmax+meshi->boxeps_smv[0];
      xyzDELTA[1] = (bc->ymin+bc->ymax)/2.0;
      xyzDELTA[2] = (bc->zmin+bc->zmax)/2.0;

      xyzDELTA += 3;
      xyzDELTA[0] = (bc->xmin+bc->xmax)/2.0;
      xyzDELTA[1] = bc->ymin-meshi->boxeps_smv[1];
      xyzDELTA[2] = (bc->zmin+bc->zmax)/2.0;

      xyzDELTA += 3;
      xyzDELTA[0] = (bc->xmin+bc->xmax)/2.0;
      xyzDELTA[1] = bc->ymax+meshi->boxeps_smv[1];
      xyzDELTA[2] = (bc->zmin+bc->zmax)/2.0;

      xyzDELTA += 3;
      xyzDELTA[0] = (bc->xmin+bc->xmax)/2.0;
      xyzDELTA[1] = (bc->ymin+bc->ymax)/2.0;
      xyzDELTA[2] = bc->zmin-meshi->boxeps_smv[2];

      xyzDELTA += 3;
      xyzDELTA[0] = (bc->xmin+bc->xmax)/2.0;
      xyzDELTA[1] = (bc->ymin+bc->ymax)/2.0;
      xyzDELTA[2] = bc->zmax+meshi->boxeps_smv[2];
    }
  }
  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    int j;
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo+i;
    for(j = 0; j<meshi->nbptrs; j++){
      blockagedata *bc;
      int k;

      bc = meshi->blockageinfoptrs[j];
      if(bc->transparent==0)continue;
      for(k=0;k<6;k++){
        bc->interior[k] = InAnyBlockage(bc->xyzDELTA+3*k);
      }
    }
  }
}

/* ------------------ UpdateCADTextCoords ------------------------ */

void UpdateCADTextCoords(cadquad *quadi){
  float twidth, theight;
  float nx, ny, nz;
  float l1, l2;
  int i;
  float *xyz;
  float *txy;
  float *t_origin;

  xyz = quadi->xyzpoints;
  txy = quadi->txypoints;

  twidth = quadi->cadlookq->texture_width;
  t_origin = quadi->cadlookq->texture_origin;
  if(twidth==0.0)twidth=1.0;
  theight = quadi->cadlookq->texture_height;
  if(theight==0.0)theight=1.0;
  nx=quadi->normals[0];
  ny=quadi->normals[1];
  nz=quadi->normals[2];
  l1 = sqrt(nx*nx+ny*ny);
  l2 = l1*sqrt(nx*nx+ny*ny+nz*nz);

  for(i=0;i<4;i++){
    float qx, qy, qz;

    qx=SMV2FDS_X(xyz[3*i+0]) - t_origin[0];
    qy=SMV2FDS_Y(xyz[3*i+1]) - t_origin[1];
    qz=SMV2FDS_Z(xyz[3*i+2]) - t_origin[2];

    if(l1!=0.0){
      txy[2*i]=(ny*qx-nx*qy)/l1;
      txy[2*i+1]=(nx*nz*qx+ny*nz*qy-(nx*nx+ny*ny)*qz)/l2;
    }
    else{
      txy[2*i]=-qx;
      txy[2*i+1]=-qy;
    }
    txy[2*i]/=twidth;
    txy[2*i+1]/=theight;
    txy[2*i]=1.0-txy[2*i];
    txy[2*i+1]=1.0-txy[2*i+1];
  }
}

/* ------------------ DrawCADGeom ------------------------ */

void DrawCADGeom(const cadgeomdata *cd){
  int i;
  int last_colorindex=-999;
  float *lastcolor;
  float rgbtemp[4]={(float)-1.0,(float)-1.0,(float)-1.0,(float)-1.0};

  lastcolor=rgbtemp;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,global_scase.color_defs.block_specular2);
  glEnable(GL_COLOR_MATERIAL);
  glBegin(GL_QUADS);
  for(i=0;i<cd->nquads;i++){
    float *xyzpoint, *normal;
    int colorindex;
    int colorindex2;
    float *thiscolor;
    cadquad *quadi;

    quadi = cd->quad+i;
    colorindex=quadi->colorindex;
    thiscolor=quadi->colors;
    if(colorindex!=last_colorindex||
      thiscolor[0]!=lastcolor[0]||
      thiscolor[1]!=lastcolor[1]||
      thiscolor[2]!=lastcolor[2]
      ){
      if(colorindex==-1){
        thiscolor=quadi->colors;
      }
      else{
        colorindex2 = 15 + (15*colorindex % 230);
        thiscolor=rgb_cad[colorindex2];
      }
      glColor4fv(thiscolor);
    }
    last_colorindex=colorindex;
    lastcolor=thiscolor;
    xyzpoint = quadi->xyzpoints;
    normal = quadi->normals;

    glNormal3fv(normal);
    glVertex3fv(xyzpoint);
    glVertex3fv(xyzpoint+3);
    glVertex3fv(xyzpoint+6);
    glVertex3fv(xyzpoint+9);
  }
  glEnd();
  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;
  SNIFF_ERRORS("DrawCADGeom");
  if(cullfaces==1)glEnable(GL_CULL_FACE);

}

/* ------------------ DrawCAD2Geom ------------------------ */

void DrawCAD2Geom(const cadgeomdata *cd, int trans_flag){
  int ii;
  float *thiscolor,*lastcolor;
  int thisonesided, lastonesided;
  int colorindex;
  texturedata *lasttexture;
  float last_block_shininess;

  lastonesided=-1;
  lastcolor=NULL;
  last_block_shininess=-1.0;

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,global_scase.color_defs.block_specular2);
  ENABLE_LIGHTING;
  if(trans_flag==DRAW_TRANSPARENT)TransparentOn();
  glBegin(GL_QUADS);
  colorindex=0;
  for(ii=0;ii<cd->nquads;ii++){
    float *xyzpoint;
    texturedata *texti;
    float this_block_shininess;
    float *normal;
    cadquad *quadi;
    int i;

    i=cd->order[ii];
    assert(i>=0&&i<cd->nquads);
    quadi = cd->quad+i;
    xyzpoint = quadi->xyzpoints;
    texti = &quadi->cadlookq->textureinfo;

    if(global_times!=NULL){
      float timeval;

      timeval=global_times[itimes];
      if(quadi->time_show>=0.0&&timeval<quadi->time_show)continue;
      if(quadi->time_show <0.0&&timeval>-quadi->time_show)continue;
    }
    if(visCadTextures==1&&texti->loaded==1)continue;


    thiscolor=quadi->cadlookq->rgb;
    if(thiscolor!=lastcolor){
      if(thiscolor[0]<0.0||thiscolor[1]<0.0||thiscolor[2]<0.0){
        GLfloat *colorptr;
        int colorindex2;

        colorindex2 = 15 + (15*colorindex % 230);
        colorptr = &rgb_cad[colorindex2][0];
        glColor4fv(colorptr);
        colorindex++;
      }
      else{
        if((viscadopaque==0&&thiscolor[3]<1.0)&&trans_flag!=DRAW_TRANSPARENT)continue;
        if((viscadopaque==1||thiscolor[3]>=1.0)&&trans_flag==DRAW_TRANSPARENT)continue;
        glColor4fv(thiscolor);
      }
      lastcolor=thiscolor;
    }

    if(RectangleInFrustum(xyzpoint,xyzpoint+3,xyzpoint+6,xyzpoint+9)==0)continue;

    thisonesided = quadi->cadlookq->onesided;
    if(lastonesided!=thisonesided){
      glEnd();
      if(thisonesided){
        glEnable(GL_CULL_FACE);
      }
      else{
        glDisable(GL_CULL_FACE);
      }
      lastonesided=thisonesided;
      glBegin(GL_QUADS);
    }

    this_block_shininess = quadi->cadlookq->shininess;
    if(last_block_shininess!=this_block_shininess){
      last_block_shininess=this_block_shininess;
      glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&this_block_shininess);
    }

    normal = quadi->normals;

    glNormal3fv(normal);
    glVertex3fv(xyzpoint);
    glVertex3fv(xyzpoint+3);
    glVertex3fv(xyzpoint+6);
    glVertex3fv(xyzpoint+9);
  }
  glEnd();

  if(visCadTextures==1){
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,enable_texture_lighting? GL_MODULATE : GL_REPLACE);
    glEnable(GL_TEXTURE_2D);
    glColor4ub(255, 255, 255, 255);

    last_block_shininess=-1.0;
    lasttexture=NULL;
    glBegin(GL_QUADS);
    for(ii=0;ii<cd->nquads;ii++){
      float this_block_shininess;
      float *xyzpoint;
      texturedata *texti;
      float *txypoint;
      float *normal;
      cadquad *quadi;
      int i;

      i=cd->order[ii];
      assert(i>=0&&i<cd->nquads);
      quadi = cd->quad+i;
      xyzpoint = quadi->xyzpoints;

      texti=&quadi->cadlookq->textureinfo;
      if(texti->loaded==0)continue;

      if(RectangleInFrustum(xyzpoint,xyzpoint+3,xyzpoint+6,xyzpoint+9)==0)continue;

      txypoint = quadi->txypoints;
      normal = quadi->normals;
      thisonesided = quadi->cadlookq->onesided;

      if(lasttexture!=texti || lastonesided!=thisonesided){
        glEnd();
        if(lasttexture!=texti){
          glBindTexture(GL_TEXTURE_2D,texti->name);
          lasttexture=texti;
        }
        if(lastonesided!=thisonesided){
          if(thisonesided){
            glEnable(GL_CULL_FACE);
          }
          else{
            glDisable(GL_CULL_FACE);
          }
          lastonesided=thisonesided;
        }
        glBegin(GL_QUADS);
      }

      this_block_shininess = quadi->cadlookq->shininess;
      if(last_block_shininess!=this_block_shininess){
        last_block_shininess=this_block_shininess;
        glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&this_block_shininess);
      }

      glNormal3fv(normal);
      glTexCoord2fv(txypoint);
      glVertex3fv(xyzpoint);

      glTexCoord2fv(txypoint+2);
      glVertex3fv(xyzpoint+3);

      glTexCoord2fv(txypoint+4);
      glVertex3fv(xyzpoint+6);

      glTexCoord2fv(txypoint+6);
      glVertex3fv(xyzpoint+9);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
  }

  DISABLE_LIGHTING;
  glDisable(GL_COLOR_MATERIAL);
  if(trans_flag==DRAW_TRANSPARENT)TransparentOff();
  if(cullfaces==1){
    glEnable(GL_CULL_FACE);
  }
  else{
    glDisable(GL_CULL_FACE);
  }
}

/* ------------------ ObstOrVent2Faces ------------------------ */

void ObstOrVent2Faces(const meshdata *meshi, blockagedata *bc,
                        ventdata *vi, facedata *faceptr, int facetype){
  /*

       7---------6
     /         /
   /         /
  4--------5

       3 ------  2
      /         /
    /         /
  0 ------ 1

  */
  int n,j,k;
  int jend=1,jjj;
  float xminmax[2]={0.0,1.0}, xminmax2[2]={0.0,1.0};
  float yminmax[2]={0.0,1.0}, yminmax2[2]={0.0,1.0};
  float zminmax[2]={0.0,1.0}, zminmax2[2]={0.0,1.0};
  float xx[8], yy[8], zz[8];
  float xx2[8], yy2[8], zz2[8];
  float *xplt, *yplt, *zplt;
  int ii[8] = {0,1,1,0,0,1,1,0};
  int jj[8] = {0,0,1,1,0,0,1,1};
  int kk[8] = {0,0,0,0,1,1,1,1};
  float offset[3];

  int blockfaceindex[]={
               0,1,5,4,  /* down y */
               1,2,6,5,  /*   up x */
               2,3,7,6,  /*   up y */
               3,0,4,7,  /* down x */
               3,2,1,0,  /* down z */
               4,5,6,7}; /*   up z */
  int *bfi;
  float *xtex, *ytex;
  float *xtex2, *ytex2;
  float t_width, t_height;
  float *xstart, *ystart;

  surfdata *face_surf;

  assert((bc==NULL&&vi!=NULL)||(bc!=NULL&&vi==NULL));

  xplt = meshi->xplt_smv;
  yplt = meshi->yplt_smv;
  zplt = meshi->zplt_smv;
  assert((bc!=NULL&&vi==NULL)||(bc==NULL&&vi!=NULL));
  if(bc!=NULL){
    jend=6;
    xminmax[0] = xplt[bc->ijk[IMIN]];
    xminmax[1] = xplt[bc->ijk[IMAX]];
    yminmax[0] = yplt[bc->ijk[JMIN]];
    yminmax[1] = yplt[bc->ijk[JMAX]];
    zminmax[0] = zplt[bc->ijk[KMIN]];
    zminmax[1] = zplt[bc->ijk[KMAX]];

    xminmax2[0] = bc->xmin;
    xminmax2[1] = bc->xmax;
    yminmax2[0] = bc->ymin;
    yminmax2[1] = bc->ymax;
    zminmax2[0] = bc->zmin;
    zminmax2[1] = bc->zmax;
  }
  if(vi!=NULL){
    jend=1;
    xminmax[0] = xplt[vi->imin];
    xminmax[1] = xplt[vi->imax];
    yminmax[0] = yplt[vi->jmin];
    yminmax[1] = yplt[vi->jmax];
    zminmax[0] = zplt[vi->kmin];
    zminmax[1] = zplt[vi->kmax];

    xminmax2[0] = vi->xmin;
    xminmax2[1] = vi->xmax;
    yminmax2[0] = vi->ymin;
    yminmax2[1] = vi->ymax;
    zminmax2[0] = vi->zmin;
    zminmax2[1] = vi->zmax;
  }


  for(n=0;n<8;n++){
    xx[n]=xminmax[ii[n]];
    yy[n]=yminmax[jj[n]];
    zz[n]=zminmax[kk[n]];
    xx2[n]=xminmax2[ii[n]];
    yy2[n]=yminmax2[jj[n]];
    zz2[n]=zminmax2[kk[n]];
  }

  for(j=0;j<jend;j++){
    faceptr->meshindex=meshi-global_scase.meshescoll.meshinfo;
    faceptr->type2=facetype;
    faceptr->show_bothsides=0;
    faceptr->bc = bc;
    faceptr->vi = vi;
    faceptr->interior = 0;

    if(bc!=NULL){
      faceptr->hidden = bc->hidden6[j];
      faceptr->blockageindex=-2;
      if(visBlocks==visBLOCKSolidOutline){
        faceptr->linewidth=&solidlinewidth;
      }
      else{
        faceptr->linewidth=&global_scase.linewidth;
      }
      faceptr->showtimelist_handle=&bc->showtimelist;
      faceptr->surfinfo=bc->surf[j];
      faceptr->texture_origin=bc->texture_origin;
      faceptr->transparent=bc->transparent;
    }
    if(vi!=NULL){
      faceptr->blockageindex=-2;
      faceptr->hidden=0;
      faceptr->texture_origin=vi->texture_origin;
      faceptr->transparent=vi->transparent;
      if(faceptr->type2==OUTLINE_FRAME_face){
        faceptr->linewidth=&global_scase.linewidth;
      }
      else{
        faceptr->linewidth=&global_scase.ventlinewidth;
      }
      faceptr->showtimelist_handle=&vi->showtimelist;
      faceptr->surfinfo=vi->surf[j];
    }
    face_surf = faceptr->surfinfo;
    if(face_surf!=NULL){
      t_width =face_surf->t_width;
      t_height=face_surf->t_height;
      if(bc!=NULL){
        if(bc->type==-1){
          faceptr->type=face_surf->type;
        }
        else{
          faceptr->type=bc->type;
        }
        if(face_surf->textureinfo!=NULL){
          faceptr->type=face_surf->type;
        }
      }
      if(vi!=NULL){
        if(ABS(vi->type)==99){
          faceptr->type=face_surf->type;
        }
        else{
          faceptr->type=vi->type;
        }
      }
    }
    else{
      if(bc!=NULL)faceptr->type=bc->type;
      if(vi!=NULL)faceptr->type=vi->type;
      t_width=1.0;
      t_height=1.0;
    }
    if(t_width==0.0)t_width=1.0;
    if(t_height==0.0)t_height=1.0;
    if(bc!=NULL){
      switch(bc->useblockcolor){
      case TTRUE:
        faceptr->color=bc->color;
        faceptr->transparent=bc->transparent;
        break;
      case FFALSE:
        if(bc->surf[j]==global_scase.surfacedefault){
         // faceptr->color=global_scase.color_defs.block_ambient2;
          faceptr->color=global_scase.surfacedefault->color;  /* fix ?? */
          faceptr->transparent=global_scase.surfacedefault->transparent;
        }
        else{
          faceptr->color=bc->surf[j]->color;
          faceptr->transparent=bc->surf[j]->transparent;
        }
        break;
      default:
        assert(FFALSE);
        break;
      }
      if(outline_color_flag==1){
        faceptr->linecolor=foregroundcolor;
      }
      else{
        faceptr->linecolor=faceptr->color;
      }
    }
    if(vi!=NULL){
      if(vi->useventcolor==1){
        faceptr->color=vi->color;
        if(facetype!=OUTLINE_FRAME_face)faceptr->transparent=vi->transparent;
      }
      else if(facetype==OUTLINE_FRAME_face){
        if(meshi->meshrgb_ptr!=NULL){
          faceptr->color=meshi->meshrgb_ptr;
        }
        else{
          faceptr->color=vi->color;
        }
      }
      else{
        faceptr->color=vi->surf[j]->color;
        faceptr->transparent=vi->surf[j]->transparent;
      }
      faceptr->linecolor=faceptr->color;
    }


    if(bc!=NULL){
      faceptr->textureinfo=bc->surf[j]->textureinfo;
    }
    if(vi!=NULL){
      faceptr->textureinfo=vi->surf[j]->textureinfo;
    }
    if(bc!=NULL){
      faceptr->dir=j;
    }
    if(vi!=NULL){
      faceptr->dir=vi->dir;
    }
    faceptr->normal[0]=0.0;
    faceptr->normal[1]=0.0;
    faceptr->normal[2]=0.0;
    if(bc!=NULL){
      faceptr->imin=bc->ijk[IMIN];
      faceptr->imax=bc->ijk[IMAX];
      faceptr->jmin=bc->ijk[JMIN];
      faceptr->jmax=bc->ijk[JMAX];
      faceptr->kmin=bc->ijk[KMIN];
      faceptr->kmax=bc->ijk[KMAX];
      faceptr->show_bothsides = show_bothsides_blockages;
    }
    if(vi!=NULL){
      faceptr->imin=vi->imin;
      faceptr->imax=vi->imax;
      faceptr->jmin=vi->jmin;
      faceptr->jmax=vi->jmax;
      faceptr->kmin=vi->kmin;
      faceptr->kmax=vi->kmax;
    }
    offset[XXX]=(float)0.0;
    offset[YYY]=(float)0.0;
    offset[ZZZ]=(float)0.0;
    switch(faceptr->dir){
     case DOWN_Y:
       faceptr->normal[1]=(float)-1.0;
       if(facetype==VENT_face&&vi!=NULL&&vi->dummy==0)offset[YYY] = -meshi->vent_offset[YYY];
       faceptr->jmax=faceptr->jmin;
       xtex = xx;
       ytex = zz;
       xtex2 = xx2;
       ytex2 = zz2;
       xstart = &global_scase.xbar0;
       ystart = &global_scase.zbar0;
       if(bc!=NULL)faceptr->interior = bc->interior[2];
       break;
     case UP_X:
       faceptr->normal[0]=(float)1.0;
       if(facetype==VENT_face&&vi!=NULL&&vi->dummy==0)offset[XXX] = meshi->vent_offset[XXX];
       faceptr->imin=faceptr->imax;
       xtex = yy;
       ytex = zz;
       xtex2 = yy2;
       ytex2 = zz2;
       xstart = &global_scase.ybar0;
       ystart = &global_scase.zbar0;
       if(bc!=NULL)faceptr->interior = bc->interior[1];
       break;
     case UP_Y:
       faceptr->normal[1]=(float)1.0;
       if(facetype==VENT_face&&vi!=NULL&&vi->dummy==0)offset[YYY] = meshi->vent_offset[YYY];
       faceptr->jmin=faceptr->jmax;
       xtex = xx;
       ytex = zz;
       xtex2 = xx2;
       ytex2 = zz2;
       xstart = &global_scase.xbar0;
       ystart = &global_scase.zbar0;
       if(bc!=NULL)faceptr->interior = bc->interior[3];
       break;
     case DOWN_X:
       if(facetype==VENT_face&&vi!=NULL&&vi->dummy==0)offset[XXX] = -meshi->vent_offset[XXX];
       xtex = yy;
       ytex = zz;
       xtex2 = yy2;
       ytex2 = zz2;
       faceptr->normal[0]=(float)-1.0;
       faceptr->imax=faceptr->imin;
       xstart = &global_scase.ybar0;
       ystart = &global_scase.zbar0;
       if(bc!=NULL)faceptr->interior = bc->interior[0];
       break;
     case DOWN_Z:
       if(facetype==VENT_face&&vi!=NULL&&vi->dummy==0)offset[ZZZ] = -meshi->vent_offset[ZZZ];
       xtex = xx;
       ytex = yy;
       xtex2 = xx2;
       ytex2 = yy2;
       faceptr->normal[2]=(float)-1.0;
       faceptr->kmax=faceptr->kmin;
       xstart = &global_scase.xbar0;
       ystart = &global_scase.ybar0;
       if(bc!=NULL)faceptr->interior = bc->interior[4];
       break;
     case UP_Z:
       if(facetype==VENT_face&&vi!=NULL&&vi->dummy==0)offset[ZZZ] = meshi->vent_offset[ZZZ];
       xtex = xx;
       ytex = yy;
       xtex2 = xx2;
       ytex2 = yy2;
       faceptr->normal[2]=(float)1.0;
       faceptr->kmin=faceptr->kmax;
       xstart = &global_scase.xbar0;
       ystart = &global_scase.ybar0;
       if(bc!=NULL)faceptr->interior = bc->interior[5];
       break;
     default:
       assert(FFALSE);
       break;
    }
    if(facetype==SHADED_FRAME_face){
      faceptr->normal[0]*=-1;
      faceptr->normal[1]*=-1;
      faceptr->normal[2]*=-1;
    }
    bfi=blockfaceindex + 4*faceptr->dir;
    faceptr->approx_center_coord[0]=0.0;
    faceptr->approx_center_coord[1]=0.0;
    faceptr->approx_center_coord[2]=0.0;
    faceptr->dist2eye=0.0;
    faceptr->xmin=xx2[bfi[0]]+offset[XXX];
    faceptr->ymin=yy2[bfi[0]]+offset[YYY];
    faceptr->zmin=zz2[bfi[0]]+offset[ZZZ];
    faceptr->xmax=xx2[bfi[0]]+offset[XXX];
    faceptr->ymax=yy2[bfi[0]]+offset[YYY];
    faceptr->zmax=zz2[bfi[0]]+offset[ZZZ];
    for(k=0;k<4;k++){
      float xvert, yvert, zvert;

      jjj = bfi[k];

      xvert=xx[jjj]+offset[XXX];
      yvert=yy[jjj]+offset[YYY];
      zvert=zz[jjj]+offset[ZZZ];
      faceptr->approx_vertex_coords[3*k]=xvert;
      faceptr->approx_vertex_coords[3*k+1]=yvert;
      faceptr->approx_vertex_coords[3*k+2]=zvert;

      faceptr->approx_center_coord[0]+=xvert;
      faceptr->approx_center_coord[1]+=yvert;
      faceptr->approx_center_coord[2]+=zvert;


      faceptr->exact_vertex_coords[3*k]  =xx2[jjj]+offset[XXX];
      faceptr->exact_vertex_coords[3*k+1]=yy2[jjj]+offset[YYY];
      faceptr->exact_vertex_coords[3*k+2]=zz2[jjj]+offset[ZZZ];
      if(faceptr->exact_vertex_coords[3*k]  <faceptr->xmin)faceptr->xmin=faceptr->exact_vertex_coords[3*k];
      if(faceptr->exact_vertex_coords[3*k]  >faceptr->xmax)faceptr->xmax=faceptr->exact_vertex_coords[3*k];
      if(faceptr->exact_vertex_coords[3*k+1]<faceptr->ymin)faceptr->ymin=faceptr->exact_vertex_coords[3*k+1];
      if(faceptr->exact_vertex_coords[3*k+1]>faceptr->ymax)faceptr->ymax=faceptr->exact_vertex_coords[3*k+1];
      if(faceptr->exact_vertex_coords[3*k+2]<faceptr->zmin)faceptr->zmin=faceptr->exact_vertex_coords[3*k+2];
      if(faceptr->exact_vertex_coords[3*k+2]>faceptr->zmax)faceptr->zmax=faceptr->exact_vertex_coords[3*k+2];
    }
    faceptr->approx_center_coord[0]/=4.0;
    faceptr->approx_center_coord[1]/=4.0;
    faceptr->approx_center_coord[2]/=4.0;


    {
      float xa_texture[4], ya_texture[4];
      float xe_texture[4], ye_texture[4];
      float dx_e, dy_e, dx_a, dy_a;

      xa_texture[0]=(*xstart+SCALE2FDS(xtex[bfi[0]]));
      ya_texture[0]=(*ystart+SCALE2FDS(ytex[bfi[0]]));

      xe_texture[0]=(*xstart+SCALE2FDS(xtex2[bfi[0]]));
      ye_texture[0]=(*ystart+SCALE2FDS(ytex2[bfi[0]]));

      switch(faceptr->dir){
        case DOWN_X:
        case UP_X:
          xe_texture[0] -= faceptr->texture_origin[1];
          ye_texture[0] -= faceptr->texture_origin[2];
          xa_texture[0] -= faceptr->texture_origin[1];
          ya_texture[0] -= faceptr->texture_origin[2];
          break;
        case DOWN_Y:
        case UP_Y:
          xe_texture[0] -= faceptr->texture_origin[0];
          ye_texture[0] -= faceptr->texture_origin[2];
          xa_texture[0] -= faceptr->texture_origin[0];
          ya_texture[0] -= faceptr->texture_origin[2];
          break;
        case DOWN_Z:
        case UP_Z:
          xe_texture[0] -= faceptr->texture_origin[0];
          ye_texture[0] -= faceptr->texture_origin[1];
          xa_texture[0] -= faceptr->texture_origin[0];
          ya_texture[0] -= faceptr->texture_origin[1];
          break;
        default:
          assert(FFALSE);
          break;
      }

      dx_a = SCALE2FDS(xtex[bfi[0]]-xtex[bfi[1]]);
      dy_a = SCALE2FDS(ytex[bfi[0]]-ytex[bfi[2]]);
      dx_e = SCALE2FDS(xtex2[bfi[0]]-xtex2[bfi[1]]);
      dy_e = SCALE2FDS(ytex2[bfi[0]]-ytex2[bfi[2]]);
      if(dx_a<0.0)dx_a=-dx_a;
      if(dy_a<0.0)dy_a=-dy_a;
      if(dx_e<0.0)dx_e=-dx_e;
      if(dy_e<0.0)dy_e=-dy_e;

      xe_texture[1] = xe_texture[0] + dx_e;
      xe_texture[2] = xe_texture[1];
      xe_texture[3] = xe_texture[0];
      ye_texture[1] = ye_texture[0];
      ye_texture[2] = ye_texture[1] + dy_e;
      ye_texture[3] = ye_texture[2];

      xa_texture[1] = xa_texture[0] + dx_a;
      xa_texture[2] = xa_texture[1];
      xa_texture[3] = xa_texture[0];
      ya_texture[1] = ya_texture[0];
      ya_texture[2] = ya_texture[1] + dy_a;
      ya_texture[3] = ya_texture[2];

      for(k=0;k<4;k++){
        faceptr->approx_texture_coords[2*k]=xa_texture[k]/t_width;
        faceptr->approx_texture_coords[2*k+1]=ya_texture[k]/t_height;
        faceptr->exact_texture_coords[2*k]=xe_texture[k]/t_width;
        faceptr->exact_texture_coords[2*k+1]=ye_texture[k]/t_height;
      }
    }
    faceptr++;
  }
}

/* ------------------ UpdateFacesWorker ------------------------ */

void UpdateFacesWorker(void){
  int i;

  INIT_PRINT_TIMER(timer_allocate_faces);
  AllocateFaces();
  PRINT_TIMER(timer_allocate_faces, "AllocateFaces");
  global_scase.updatefaces=0;
  have_vents_int=0;

  INIT_PRINT_TIMER(timer_update_faces_1);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    facedata *faceptr;
    int j;

    meshi = global_scase.meshescoll.meshinfo + i;
    faceptr = meshi->faceinfo;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[j];
      if(global_scase.visTerrainType!=TERRAIN_HIDDEN&&bc->is_wuiblock==1)continue;
      ObstOrVent2Faces(meshi,bc,NULL,faceptr,BLOCK_face);
      faceptr += 6;
    }
    for(j=0;j<meshi->nvents;j++){
      ventdata *vi;

      vi = meshi->ventinfo+j;
      ObstOrVent2Faces(meshi,NULL,vi,faceptr,VENT_face);
      faceptr++;
    }
    for(j=meshi->nvents;j<meshi->nvents+6;j++){
      ventdata *vi;

      vi = meshi->ventinfo+j;
      ObstOrVent2Faces(meshi,NULL,vi,faceptr,OUTLINE_FRAME_face);
      assert(faceptr->color!=NULL);
      faceptr++;
    }
    for(j=meshi->nvents+6;j<meshi->nvents+12;j++){
      ventdata *vi;

      vi = meshi->ventinfo+j;
      ObstOrVent2Faces(meshi,NULL,vi,faceptr,SHADED_FRAME_face);
      assert(faceptr->color!=NULL);
      faceptr++;
    }
    meshi->nfaces=faceptr-meshi->faceinfo;
  }
  PRINT_TIMER(timer_update_faces_1,"UpdateFaces(loop over meshes)");

  INIT_PRINT_TIMER(timer_update_face_lists);
  UpdateFaceLists();
  PRINT_TIMER(timer_update_face_lists,"UpdateFaceLists(in UpdateFaces)");

  INIT_PRINT_TIMER(timer_update_select_faces);
  UpdateSelectFaces();
  PRINT_TIMER(timer_update_select_faces, "UpdateSelectFaces");

  INIT_PRINT_TIMER(timer_update_select_blocks);
  UpdateSelectBlocks();
  PRINT_TIMER(timer_update_select_blocks, "UpdateSelectBlocks");
}

/* ------------------ UpdateFaces ------------------------ */

void UpdateFaces(void){
  INIT_PRINT_TIMER(timer_update_faces);
  UpdateFacesWorker();
  PRINT_TIMER(timer_update_faces, "UpdateFaces");
}

/* ------------------ ClipFace ------------------------ */

int ClipFace(clipdata *ci, facedata *facei){
  if(clip_mode==CLIP_OFF||clip_mode==CLIP_DATA)return 0;
  if(ci->clip_xmin==1&&SMV2FDS_X(facei->xmax)<ci->xmin)return 1;
  if(ci->clip_xmax==1&&SMV2FDS_X(facei->xmin)>ci->xmax)return 1;
  if(ci->clip_ymin==1&&SMV2FDS_Y(facei->ymax)<ci->ymin)return 1;
  if(ci->clip_ymax==1&&SMV2FDS_Y(facei->ymin)>ci->ymax)return 1;
  if(ci->clip_zmin==1&&SMV2FDS_Z(facei->zmax)<ci->zmin)return 1;
  if(ci->clip_zmax==1&&SMV2FDS_Z(facei->zmin)>ci->zmax)return 1;
  return 0;
}

/* ------------------ CompareSingleFaces0 ------------------------ */

int CompareSingleFaces0(const void *arg1, const void *arg2){
  facedata *facei, *facej;
  int dirs[6];

  facei = *(facedata **)arg1;
  facej = *(facedata **)arg2;

  dirs[DOWN_X]=1;
  dirs[UP_X]=1;
  dirs[DOWN_Y]=2;
  dirs[UP_Y]=2;
  dirs[DOWN_Z]=3;
  dirs[UP_Z]=3;
  if(dirs[facei->dir]<dirs[facej->dir])return -1;
  if(dirs[facei->dir]>dirs[facej->dir])return 1;
  switch(facei->dir){
    case DOWN_X:
    case UP_X:
      if(facei->imin<facej->imin)return -1;
      if(facei->imin>facej->imin)return 1;
      if(facei->jmin<facej->jmin)return -1;
      if(facei->jmin>facej->jmin)return 1;
      if(facei->kmin<facej->kmin)return -1;
      if(facei->kmin>facej->kmin)return 1;
      if(facei->imax<facej->imax)return -1;
      if(facei->imax>facej->imax)return 1;
      if(facei->jmax<facej->jmax)return -1;
      if(facei->jmax>facej->jmax)return 1;
      if(facei->kmax<facej->kmax)return -1;
      if(facei->kmax>facej->kmax)return 1;
      break;
    case DOWN_Y:
    case UP_Y:
      if(facei->jmin<facej->jmin)return -1;
      if(facei->jmin>facej->jmin)return 1;
      if(facei->imin<facej->imin)return -1;
      if(facei->imin>facej->imin)return 1;
      if(facei->kmin<facej->kmin)return -1;
      if(facei->kmin>facej->kmin)return 1;
      if(facei->jmax<facej->jmax)return -1;
      if(facei->jmax>facej->jmax)return 1;
      if(facei->imax<facej->imax)return -1;
      if(facei->imax>facej->imax)return 1;
      if(facei->kmax<facej->kmax)return -1;
      if(facei->kmax>facej->kmax)return 1;
      break;
    case DOWN_Z:
    case UP_Z:
      if(facei->kmin<facej->kmin)return -1;
      if(facei->kmin>facej->kmin)return 1;
      if(facei->imin<facej->imin)return -1;
      if(facei->imin>facej->imin)return 1;
      if(facei->jmin<facej->jmin)return -1;
      if(facei->jmin>facej->jmin)return 1;
      if(facei->kmax<facej->kmax)return -1;
      if(facei->kmax>facej->kmax)return 1;
      if(facei->imax<facej->imax)return -1;
      if(facei->imax>facej->imax)return 1;
      if(facei->jmax<facej->jmax)return -1;
      if(facei->jmax>facej->jmax)return 1;
      break;
    default:
      assert(FFALSE);
      break;
  }
  if(facei->color<facej->color)return  1;
  if(facei->color>facej->color)return -1;
  return 0;
}

/* ------------------ CompareSingleFaces ------------------------ */

int CompareSingleFaces(const void *arg1, const void *arg2){
  facedata *facei, *facej;

  facei = *(facedata **)arg1;
  facej = *(facedata **)arg2;

  if(facei->dir<facej->dir)return -1;
  if(facei->dir>facej->dir)return 1;
  switch(facei->dir){
    case DOWN_X:   // sort DOWN data from big to small
      if(facei->imin<facej->imin)return 1;
      if(facei->imin>facej->imin)return -1;
      break;
    case UP_X:     // sort UP data from small to big
      if(facei->imin<facej->imin)return -1;
      if(facei->imin>facej->imin)return 1;
      break;
    case DOWN_Y:
      if(facei->jmin<facej->jmin)return 1;
      if(facei->jmin>facej->jmin)return -1;
      break;
    case UP_Y:
      if(facei->jmin<facej->jmin)return -1;
      if(facei->jmin>facej->jmin)return 1;
      break;
    case DOWN_Z:
      if(facei->kmin<facej->kmin)return 1;
      if(facei->kmin>facej->kmin)return -1;
      break;
    case UP_Z:
      if(facei->kmin<facej->kmin)return -1;
      if(facei->kmin>facej->kmin)return 1;
      break;
    default:
      assert(FFALSE);
      break;
  }
  if(facei->color<facej->color)return  1;
  if(facei->color>facej->color)return -1;
  return 0;
}

/* ------------------ CompareColorFaces ------------------------ */

int CompareColorFaces(const void *arg1, const void *arg2){
  facedata *facei, *facej;

  facei = *(facedata **)arg1;
  facej = *(facedata **)arg2;

  if(facei->color<facej->color)return  1;
  if(facei->color>facej->color)return -1;
  return 0;
}

/* ------------------ ShowHideInternalFaces ------------------------ */

void ShowHideInternalFaces(meshdata *meshi, int show){
  int j;

  for(j = 0;j < meshi->nbptrs;j++){
    facedata *facej;

    facej = meshi->faceinfo + 6 * j;
    facej->hidden = 0; facej++;
    facej->hidden = 0; facej++;
    facej->hidden = 0; facej++;
    facej->hidden = 0; facej++;
    facej->hidden = 0; facej++;
    facej->hidden = 0; facej++;
  }
  if(show == 1)return;

  for(j = 0; j < meshi->nbptrs; j++){
    facedata *facej;
    blockagedata *bc;
    int *show_bndf, *is_extface;

    bc = meshi->blockageinfoptrs[j];
    show_bndf  = bc->show_bndf;
    is_extface = bc->is_extface;

    facej = meshi->faceinfo + 6 * j;

//down y
    if(show_bndf[2]==1 && is_extface[2] == 0)facej->hidden = 1;
    facej++;

// up x
    if(show_bndf[1]==1 && is_extface[1] == 0)facej->hidden = 1;
    facej++;

//up y
    if(show_bndf[3]==1 && is_extface[3] == 0)facej->hidden = 1;
    facej++;

// down x
    if(show_bndf[0]==1 && is_extface[0] == 0)facej->hidden = 1;
    facej++;

// down z
    if(show_bndf[4]==1 && is_extface[4] == 0)facej->hidden = 1;
    facej++;

// up z
    if(show_bndf[5]==1 && is_extface[5] == 0)facej->hidden = 1;
    facej++;
  }
}

/* ------------------ IsVentVisible ------------------------ */

int IsVentVisible(ventdata *vi){
  if(boundary_loaded == 0)return 1; // bouindary file not loaded
  if(vi->patch_index < 0)return 1;  // patch not associated with this vent

  // show vent if boundary file is not visible on corresponding vent
  if(vi->wall_type == LEFTwall)return  1 - vis_boundary_type[LEFTwall];
  if(vi->wall_type == RIGHTwall)return 1 - vis_boundary_type[RIGHTwall];
  if(vi->wall_type == FRONTwall)return 1 - vis_boundary_type[FRONTwall];
  if(vi->wall_type == BACKwall)return  1 - vis_boundary_type[BACKwall];
  if(vi->wall_type == DOWNwall)return  1 - vis_boundary_type[DOWNwall];
  if(vi->wall_type == UPwall  )return  1 - vis_boundary_type[UPwall];

  return 0; //boundary file is visible so hide vent
}

/* ------------------ UpdateFaceListsWorker ------------------------ */

void UpdateFaceListsWorker(void){
  int n_textures, n_outlines;
  int n_normals_single, n_normals_double, n_transparent_double;
  int i;
  int drawing_transparent, drawing_blockage_transparent, drawing_vent_transparent;

  GetDrawingParms(&drawing_transparent, &drawing_blockage_transparent, &drawing_vent_transparent);

  updatefacelists=0;
  nface_normals_single=0;
  nface_normals_double=0;
  nface_transparent_double=0;
  nface_textures=0;
  nface_outlines=0;
  nface_transparent=0;
  if(opengldefined==1){
    glutPostRedisplay();
  }

  int show;
  show = GetInternalFaceShow();

  // if we are not showing boundary files then don't try to hide blockages
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int patchfilenum;
    int j;
    patchdata *patchi;
    int vent_offset, outline_offset, exteriorsurface_offset;

    meshi = global_scase.meshescoll.meshinfo + i;

    patchfilenum=meshi->patchfilenum;
    patchi=NULL;
    if(showplot3d == 0 && patchfilenum>=0 && patchfilenum<global_scase.npatchinfo)patchi = global_scase.patchinfo + patchfilenum;
    ShowHideInternalFaces(meshi, show);

    n_normals_single=0;
    n_normals_double=0;
    n_transparent_double=0;
    n_textures=0;
    n_outlines=0;

    vent_offset = 6*meshi->nbptrs;
    outline_offset = vent_offset + meshi->nvents;
    exteriorsurface_offset = outline_offset + 6;

    for(j=0;j<meshi->nfaces;j++){
      facedata *facej;
      ventdata *vi;

      vi = meshi->ventinfo+j-vent_offset;
      facej = meshi->faceinfo + j;

      if(j >= outline_offset && j < outline_offset + 6){
        facej->linecolor = foregroundcolor;
      }

      if(showonly_hiddenfaces==0&&facej->hidden==1)continue;
      if(showonly_hiddenfaces==1&&facej->hidden==0)continue;
      if(facej->bc!=NULL&&facej->bc->prop!=NULL&&facej->bc->prop->blockvis==0)continue;
      if(ClipFace(&clipinfo,facej)==1)continue;

      if(showedit_dialog == 1 && j<vent_offset){
        if(facej->show_bothsides==0)meshi->face_normals_single[n_normals_single++]=facej;
        if(facej->show_bothsides==1)meshi->face_normals_double[n_normals_double++]=facej;
        continue;
      }
      if(j<vent_offset){
        if(visBlocks==visBLOCKHide)continue;
      }
      if(j>=outline_offset&&j<outline_offset+6&&outline_mode==SCENE_OUTLINE_HIDDEN){
        continue;
      }
      if(j>=vent_offset&&j<vent_offset+meshi->nvents){
        if(visOpenVents==0&&vi->isOpenvent==1)continue;
        if(visDummyVents==0&&vi->dummy==1)continue;
        if(global_scase.visOtherVents==0&&vi->isOpenvent==0&&vi->dummy==0)continue;
        if(
           patchi!=NULL
           &&patchi->loaded==1
           &&patchi->display==1
           &&IsVentVisible(vi)==0
           &&(vis_threshold==0||vis_onlythreshold==0||do_threshold==0)
           &&(vi->dummy==1||vi->hideboundary==0)
           ){
          continue;
        }
        if(facej->transparent==1&&drawing_vent_transparent==1){
          if(facej->show_bothsides==1){
            meshi->face_transparent_double[n_transparent_double++]=facej;
          }
          else{
            face_transparent[nface_transparent++]=facej;
          }
          continue;
        }
        if(facej->textureinfo!=NULL&&facej->textureinfo->display==1){
          meshi->face_textures[n_textures++]=facej;
          continue;
        }
      }
      if(j>=exteriorsurface_offset){
        switch(j-exteriorsurface_offset){
         case DOWN_Z:
           if(global_scase.visFloor==0){
             continue;
           }
          break;
         case UP_Z:
           if(global_scase.visCeiling==0)continue;
          break;
         case UP_X:
         case DOWN_X:
         case UP_Y:
         case DOWN_Y:
           if(global_scase.visWalls==0)continue;
          break;
         default:
           assert(FFALSE);
           break;
        }
      }
      if((
         (visBlocks==visBLOCKAsInputOutline||visBlocks==visBLOCKOutline||visBlocks==visBLOCKSolidOutline)&&j<vent_offset)||
         (facej->type==BLOCK_outline&&visBlocks==visBLOCKAsInput)||
         ((j>=vent_offset&&j<vent_offset+meshi->nvents)&&vi->isOpenvent==1&&visOpenVentsAsOutline==1)
        ){
        if(global_scase.obstcoll.nobstinfo==0||(global_scase.obstcoll.nobstinfo>0&&blocklocation==BLOCKlocation_grid))meshi->face_outlines[n_outlines++]=facej;
        if(visBlocks!=visBLOCKSolidOutline&&visBlocks!=visBLOCKAsInputOutline)continue;
      }
      if(j<vent_offset){
        int drawing_texture=0;

        if(facej->type==BLOCK_texture&&facej->textureinfo!=NULL&&facej->textureinfo->display==1){
          drawing_texture=1;
        }

        if(facej->transparent==0||drawing_blockage_transparent==0){
          if(drawing_texture==0){
            if(facej->show_bothsides==0){
              meshi->face_normals_single[n_normals_single++]=facej;
            }
            if(facej->show_bothsides==1){
              meshi->face_normals_double[n_normals_double++]=facej;
            }
            continue;
          }
        }
        if(facej->transparent==1&&drawing_blockage_transparent==1){
          if(blocklocation!=BLOCKlocation_grid&&facej->interior==1)continue;
          face_transparent[nface_transparent++]=facej;
          continue;
        }
      }

      if(facej->hidden == 0){
        switch(facej->type){
          case BLOCK_regular:
            if(facej->show_bothsides == 0)meshi->face_normals_single[n_normals_single++] = facej;
            if(facej->show_bothsides == 1)meshi->face_normals_double[n_normals_double++] = facej;
            break;
          case BLOCK_texture:
            if(facej->textureinfo != NULL){
              if(facej->textureinfo->display == 1){
                meshi->face_textures[n_textures++] = facej;
              }
              else{
                if(facej->type2 == BLOCK_face){
                  if(facej->show_bothsides == 0)meshi->face_normals_single[n_normals_single++] = facej;
                  if(facej->show_bothsides == 1)meshi->face_normals_double[n_normals_double++] = facej;
                }
                if(facej->type2 == VENT_face)meshi->face_outlines[n_outlines++] = facej;
              }
              continue;
            }
            break;
          case BLOCK_outline:
            meshi->face_outlines[n_outlines++] = facej;
            break;
          case BLOCK_hidden:
            break;
          default:
            PRINTF("facej->type=%i\n", facej->type);
            assert(FFALSE);
            break;
        }
      }
    }

    meshi->nface_textures = n_textures;
    meshi->nface_normals_single  = n_normals_single;
    meshi->nface_normals_double  = n_normals_double;
    meshi->nface_transparent_double  = n_transparent_double;
    meshi->nface_outlines = n_outlines;
    nface_textures += n_textures;
    nface_normals_single += n_normals_single;
    nface_normals_double += n_normals_double;
    nface_transparent_double += n_transparent_double;
    nface_outlines += n_outlines;

    meshi->nface_normals_single_DOWN_X = 0;
    meshi->nface_normals_single_UP_X   = 0;
    meshi->nface_normals_single_DOWN_Y = 0;
    meshi->nface_normals_single_UP_Y   = 0;
    meshi->nface_normals_single_DOWN_Z = 0;
    meshi->nface_normals_single_UP_Z   = 0;
    meshi->face_normals_single_DOWN_X  = NULL;
    meshi->face_normals_single_UP_X    = NULL;
    meshi->face_normals_single_DOWN_Y  = NULL;
    meshi->face_normals_single_UP_Y    = NULL;
    meshi->face_normals_single_DOWN_Z  = NULL;
    meshi->face_normals_single_UP_Z    = NULL;

    if(blockage_draw_option != 1)continue;

    if(n_normals_single>0){
      int iface;
      int istartD=-1,istartU=-1;
      int jstartD=-1,jstartU=-1;
      int kstartD=-1,kstartU=-1;
      int nhidden;

      nhidden=0;
      qsort((facedata **)meshi->face_normals_single,(size_t)n_normals_single,sizeof(facedata *),CompareSingleFaces0);
      meshi->face_normals_single[0]->dup=0;
      for(iface=1;iface<meshi->nface_normals_single;iface++){
        facedata *facei;
        facedata *faceim1;

        facei=meshi->face_normals_single[iface];
        facei->dup=0;
        faceim1 = meshi->face_normals_single[iface-1];
        if(
          facei->imax-facei->imin<=1&&
          facei->jmax-facei->jmin<=1&&
          facei->kmax-facei->kmin<=1&& // only hide duplicate one cell faces
          faceim1->imin==facei->imin&&
          faceim1->imax==facei->imax&&
          faceim1->jmin==facei->jmin&&
          faceim1->jmax==facei->jmax&&
          faceim1->kmin==facei->kmin&&
          faceim1->kmax==facei->kmax&&
          faceim1->dir!=facei->dir
          ){
          if(*(facei->showtimelist_handle)==NULL){
            facei->dup=1;
            nhidden++;
          }
        }
      }
      if(nhidden>0){
        n_normals_single=0;
        for(iface=0;iface<meshi->nface_normals_single;iface++){
          facedata *facei;

          facei=meshi->face_normals_single[iface];
          if(facei->dup==0)meshi->face_normals_single[n_normals_single++]=facei;
        }
        meshi->nface_normals_single=n_normals_single;
      }

      qsort((facedata **)meshi->face_normals_single,(size_t)n_normals_single,sizeof(facedata *),CompareSingleFaces);
      for(iface=0;iface<meshi->nface_normals_single;iface++){
        facedata *facei;

        facei=meshi->face_normals_single[iface];
        switch(facei->dir){
          case DOWN_X:
            if(istartD==-1){
              meshi->face_normals_single_DOWN_X=meshi->face_normals_single+iface;
              istartD=0;
            }
            meshi->nface_normals_single_DOWN_X++;
            break;
          case UP_X:
            if(istartU==-1){
              meshi->face_normals_single_UP_X=meshi->face_normals_single+iface;
              istartU=0;
            }
            meshi->nface_normals_single_UP_X++;
            break;
          case DOWN_Y:
            if(jstartD==-1){
              meshi->face_normals_single_DOWN_Y=meshi->face_normals_single+iface;
              jstartD=0;
            }
            meshi->nface_normals_single_DOWN_Y++;
            break;
          case UP_Y:
            if(jstartU==-1){
              meshi->face_normals_single_UP_Y=meshi->face_normals_single+iface;
              jstartU=0;
            }
            meshi->nface_normals_single_UP_Y++;
            break;
          case DOWN_Z:
            if(kstartD==-1){
              meshi->face_normals_single_DOWN_Z=meshi->face_normals_single+iface;
              kstartD=0;
            }
            meshi->nface_normals_single_DOWN_Z++;
            break;
          case UP_Z:
            if(kstartU==-1){
              meshi->face_normals_single_UP_Z=meshi->face_normals_single+iface;
              kstartU=0;
            }
            meshi->nface_normals_single_UP_Z++;
            break;
          default:
            assert(FFALSE);
            break;
        }
      }
    }
    if(n_normals_double>1){
      qsort((facedata **)meshi->face_normals_double,(size_t)n_normals_double,sizeof(facedata *),CompareColorFaces);
    }
    if(n_outlines>1){
      qsort((facedata **)meshi->face_outlines,(size_t)n_outlines,sizeof(facedata *),CompareColorFaces);
    }
  }
  n_geom_triangles=0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo  + i;
    n_geom_triangles += meshi->nface_textures+meshi->nface_normals_single+meshi->nface_normals_double;
  }
}

/* ------------------ UpdateFaceLists ------------------------ */

void UpdateFaceLists(void){
  INIT_PRINT_TIMER(timer_update_face_lists);
  UpdateFaceListsWorker();
  PRINT_TIMER(timer_update_face_lists, "UpdateFacesLists");
}

/* ------------------ DrawSelectFaces ------------------------ */

void DrawSelectFaces(){
  int i;
  int color_index=0;

  DISABLE_LIGHTING;
  glBegin(GL_QUADS);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->nbptrs;j++){
      int k;

      for(k=0;k<6;k++){
        unsigned char r, g, b;
        facedata *facek;
        float *vertices;
        int sides[]={DOWN_Y,UP_X,UP_Y,DOWN_X,DOWN_Z,UP_Z};

        facek = meshi->faceinfo + 6*j + sides[k];

        vertices = facek->approx_vertex_coords;
        GetRGB(color_index+1,&r,&g,&b);
        glColor3ub(r,g,b);
        color_index++;
        glVertex3fv(vertices);
        glVertex3fv(vertices+3);
        glVertex3fv(vertices+6);
        glVertex3fv(vertices+9);
      }
    }
  }
  glEnd();
}

#define DRAWFACE(facei,DEFfacetest,DEFeditcolor)    \
        float *facepos;\
        float *vertices;\
        if(blocklocation==BLOCKlocation_grid){\
          vertices = facei->approx_vertex_coords;\
        }\
        else{\
          vertices = facei->exact_vertex_coords;\
        }\
        facepos=vertices;\
        if(DEFfacetest)break;\
 \
        showtimelist_handle = facei->showtimelist_handle;\
        showtimelist = *showtimelist_handle;\
        if(showtimelist!=NULL&&showtimelist[itimes]==0)continue;\
        if(showedit_dialog==0){\
          new_color=facei->color;\
        }\
        else{\
          if(visNormalEditColors==0)new_color=global_scase.color_defs.block_ambient2;\
          if(visNormalEditColors==1)new_color=facei->color;\
          if(highlight_block==facei->blockageindex&&highlight_mesh==facei->meshindex){\
            new_color=DEFeditcolor;\
          }\
        }\
        if(new_color!=old_color){\
          old_color=new_color;\
          glColor4fv(old_color);\
        }\
        glVertex3fv(vertices);\
        glVertex3fv(vertices+3);\
        glVertex3fv(vertices+6);\
        glVertex3fv(vertices);\
        glVertex3fv(vertices+6);\
        glVertex3fv(vertices+9);

/* ------------------ DrawObstsDebug ------------------------ */

void DrawObstsDebug(void){
  int i;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
  if(light_faces == 1){
    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, global_scase.color_defs.block_specular2);
    glEnable(GL_COLOR_MATERIAL);
  }
  for(i = 1; i <= global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    int j;
    int jmin, jmax;

    meshi = global_scase.meshescoll.meshinfo + i - 1;
    jmin = 0;
    jmax = meshi->nbptrs-1;
    if(mesh_index_debug >= 1 && mesh_index_debug <= global_scase.meshescoll.nmeshes){
      int max_blockage_index_debug;
      if(mesh_index_debug!=i)continue;
      max_blockage_index_debug = min_blockage_index_debug + n_blockages_debug - 1;
      if(min_blockage_index_debug >= 1 && min_blockage_index_debug <= meshi->nbptrs){
        if(max_blockage_index_debug >= 1 && max_blockage_index_debug <= meshi->nbptrs){
          if(min_blockage_index_debug <= max_blockage_index_debug){
            jmin = min_blockage_index_debug-1;
            jmax = max_blockage_index_debug-1;
          }
        }
      }
    }
    for(j = jmin; j<=jmax; j++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[j];
      void DrawBoxShaded(float *bb, int flag, int *hidden6, float *box_color);
      DrawBoxShaded(bc->xyz, blockage_draw_option, bc->hidden6, bc->color);
    }
  }
  if(light_faces == 1){
    glDisable(GL_COLOR_MATERIAL);
    DISABLE_LIGHTING;
  }
  glPopMatrix();
}

/* ------------------ DrawFacesOLD ------------------------ */

void DrawFacesOLD(int option){
  float *new_color=NULL, *old_color = NULL;
  int **showtimelist_handle, *showtimelist;
  float up_color[4] = {0.9,0.9,0.9,1.0};
  float down_color[4] = {0.1,0.1,0.1,1.0};
  float highlight_color[4] = {1.0,0.0,0.0,1.0};

  if(nface_normals_single > 0){
    int j;

    glEnable(GL_CULL_FACE);
    if(light_faces == 1){
      ENABLE_LIGHTING;
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, global_scase.color_defs.block_specular2);
      glEnable(GL_COLOR_MATERIAL);
    }
    glBegin(GL_TRIANGLES);
    for(j = 0; j < global_scase.meshescoll.nmeshes; j++){
      meshdata *meshi;
      int i;

      meshi = global_scase.meshescoll.meshinfo + j;
      if(meshi->blockvis == 0)continue;
      for(i = 0; i < meshi->nface_normals_single; i++){
        facedata *facei;
        float *vertices;

        facei = meshi->face_normals_single[i];
        if(option == DRAW_OBSTS && facei->bc == NULL)continue;
        if(option == DRAW_VENTS){
          if(facei->vi == NULL || facei->vi->dummy == 0)continue;
        }
        if(facei->hidden == 1)continue;
        if(blocklocation == BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
        }
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist != NULL && showtimelist[itimes] == 0)continue;
        if(showedit_dialog == 0){
          new_color = facei->color;
        }
        else{
          if(visNormalEditColors == 0)new_color = global_scase.color_defs.block_ambient2;
          if(visNormalEditColors == 1)new_color = facei->color;
          if(highlight_block == facei->blockageindex && highlight_mesh == facei->meshindex){
            new_color = highlight_color;
            switch(xyz_dir){
            case XDIR:
              if(facei->dir == UP_X)new_color = up_color;
              if(facei->dir == DOWN_X)new_color = down_color;
              break;
            case YDIR:
              if(facei->dir == UP_Y)new_color = up_color;
              if(facei->dir == DOWN_Y)new_color = down_color;
              break;
            case ZDIR:
              if(facei->dir == UP_Z)new_color = up_color;
              if(facei->dir == DOWN_Z)new_color = down_color;
              break;
            default:
              assert(FFALSE);
              break;
            }
          }
        }
        if(new_color != old_color){
          old_color = new_color;
          glColor4fv(old_color);
        }
        glNormal3fv(facei->normal);
        glVertex3fv(vertices);
        glVertex3fv(vertices + 3);
        glVertex3fv(vertices + 6);
        glVertex3fv(vertices);
        glVertex3fv(vertices + 6);
        glVertex3fv(vertices + 9);
      }
    }
    glEnd();
    if(light_faces == 1){
      glDisable(GL_COLOR_MATERIAL);
      DISABLE_LIGHTING;
    }
  }
  if(nface_normals_double > 0){
    int j;

    if(light_faces == 1){
      ENABLE_LIGHTING;
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, global_scase.color_defs.block_specular2);
      glEnable(GL_COLOR_MATERIAL);
    }
    if(cullfaces == 1)glDisable(GL_CULL_FACE);
    AntiAliasLine(ON);
    glBegin(GL_QUADS);
    for(j = 0; j < global_scase.meshescoll.nmeshes; j++){
      meshdata *meshi;
      int i;

      meshi = global_scase.meshescoll.meshinfo + j;
      for(i = 0; i < meshi->nface_normals_double; i++){
        facedata *facei;
        float *vertices;

        facei = meshi->face_normals_double[i];
        if(option == DRAW_OBSTS && facei->bc == NULL)continue;
        if(option == DRAW_VENTS){
          if(facei->vi == NULL || facei->vi->dummy == 0)continue;
        }
        if(facei->hidden == 1)continue;
        if(blocklocation == BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
        }
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist != NULL && showtimelist[itimes] == 0)continue;
        if(showedit_dialog == 0){
          new_color = facei->color;
        }
        else{
          if(visNormalEditColors == 0)new_color = global_scase.color_defs.block_ambient2;
          if(visNormalEditColors == 1)new_color = facei->color;
          if(highlight_block == facei->blockageindex && highlight_mesh == facei->meshindex){
            new_color = highlight_color;
            switch(xyz_dir){
            case XDIR:
              if(facei->dir == UP_X)new_color = up_color;
              if(facei->dir == DOWN_X)new_color = down_color;
              break;
            case YDIR:
              if(facei->dir == UP_Y)new_color = up_color;
              if(facei->dir == DOWN_Y)new_color = down_color;
              break;
            case ZDIR:
              if(facei->dir == UP_Z)new_color = up_color;
              if(facei->dir == DOWN_Z)new_color = down_color;
              break;
            default:
              assert(FFALSE);
              break;
            }
          }
        }
        if(new_color != old_color){
          old_color = new_color;
          glColor4fv(old_color);
        }
        glNormal3fv(facei->normal);
        glVertex3fv(vertices);
        glVertex3fv(vertices + 3);
        glVertex3fv(vertices + 6);
        glVertex3fv(vertices + 9);
      }
    }
    glEnd();
    AntiAliasLine(OFF);
    if(cullfaces == 1)glEnable(GL_CULL_FACE);
    if(light_faces == 1){
      glDisable(GL_COLOR_MATERIAL);
      DISABLE_LIGHTING;
    }
  }
  if(nface_outlines > 0){
    int j;

    DISABLE_LIGHTING;
    AntiAliasLine(ON);
    glLineWidth(global_scase.linewidth);
    glBegin(GL_LINES);
    for(j = 0; j < global_scase.meshescoll.nmeshes; j++){
      meshdata *meshi;
      int i;

      meshi = global_scase.meshescoll.meshinfo + j;
      if(meshi->blockvis == 0)continue;
      for(i = 0; i < meshi->nface_outlines; i++){
        facedata *facei;
        float *vertices;

        facei = meshi->face_outlines[i];
        if(option == DRAW_OBSTS && facei->bc == NULL)continue;
        if(option == DRAW_VENTS){
          if(facei->vi == NULL || facei->vi->dummy == 0)continue;
        }
        if(facei->hidden == 1)continue;
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist != NULL && showtimelist[itimes] == 0 && facei->type2 == BLOCK_face)continue;
        if(blocklocation == BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
        }
        if(facei->type2 != OUTLINE_FRAME_face || outline_mode == SCENE_OUTLINE_MESH){
          glEnd();
          if(global_scase.meshescoll.nmeshes > 1 && facei->type2 == OUTLINE_FRAME_face &&
            highlight_mesh == facei->meshindex && outline_mode == SCENE_OUTLINE_MESH){
            glLineWidth(highlight_linewidth);
          }
          else{
            glLineWidth(*facei->linewidth);
          }
          glBegin(GL_LINES);
          //xxx facei->linecolor not defined properly when reading a geometry file
          glColor3fv(facei->linecolor);
          glVertex3fv(vertices);
          glVertex3fv(vertices + 3);
          glVertex3fv(vertices + 3);
          glVertex3fv(vertices + 6);
          glVertex3fv(vertices + 6);
          glVertex3fv(vertices + 9);
          glVertex3fv(vertices + 9);
          glVertex3fv(vertices);
          if(showtimelist != NULL && showtimelist[itimes] == 0){
            glVertex3fv(vertices);
            glVertex3fv(vertices + 6);
            glVertex3fv(vertices + 3);
            glVertex3fv(vertices + 9);
          }
        }
      }
    }
    glEnd();
    AntiAliasLine(OFF);
  }
  if(nface_textures > 0){
    int j;

    if(light_faces == 1){
      ENABLE_LIGHTING;
      glEnable(GL_COLOR_MATERIAL);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, enable_texture_lighting?GL_MODULATE:GL_REPLACE);
    if(light_faces == 1){
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, global_scase.color_defs.block_specular2);
    }
    glEnable(GL_TEXTURE_2D);
    glColor4ub(255, 255, 255, 255);
    for(j = 0; j < global_scase.meshescoll.nmeshes; j++){
      meshdata *meshi;
      int i;

      meshi = global_scase.meshescoll.meshinfo + j;
      if(meshi->blockvis == 0)continue;
      for(i = 0; i < meshi->nface_textures; i++){
        facedata *facei;
        float *tvertices;
        float *vertices;
        texturedata *texti;

        facei = meshi->face_textures[i];
        if(option == DRAW_OBSTS && facei->bc == NULL)continue;
        if(option == DRAW_VENTS){
          if(facei->vi == NULL || facei->vi->dummy == 0)continue;
        }
        if(facei->hidden == 1)continue;
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist != NULL && showtimelist[itimes] == 0)continue;
        texti = facei->textureinfo;
        if(blocklocation == BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
          tvertices = facei->approx_texture_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
          tvertices = facei->exact_texture_coords;
        }

        if(facei->type2 == BLOCK_face && cullfaces == 0)glDisable(GL_CULL_FACE);


        glBindTexture(GL_TEXTURE_2D, texti->name);
        glBegin(GL_QUADS);

        glNormal3fv(facei->normal);
        glTexCoord2fv(tvertices);
        glVertex3fv(vertices);

        glTexCoord2fv(tvertices + 2);
        glVertex3fv(vertices + 3);

        glTexCoord2fv(tvertices + 4);
        glVertex3fv(vertices + 6);

        glTexCoord2fv(tvertices + 6);
        glVertex3fv(vertices + 9);
        glEnd();
      }
      if(cullfaces == 1)glEnable(GL_CULL_FACE);


    }
    glDisable(GL_TEXTURE_2D);
    if(light_faces == 1){
      DISABLE_LIGHTING;
      glDisable(GL_COLOR_MATERIAL);
    }
  }
  if(show_triangle_count == 1)printf("obst/vent triangles: %i\n", n_geom_triangles);
}

/* ------------------ DrawFaces ------------------------ */

void DrawFaces(){
  float *new_color=NULL,*old_color=NULL;
  int **showtimelist_handle, *showtimelist;
  float up_color[4]={0.9,0.9,0.9,1.0};
  float down_color[4]={0.1,0.1,0.1,1.0};
  float highlight_color[4]={1.0,0.0,0.0,1.0};

  if(projection_type==PROJECTION_ORTHOGRAPHIC){
    DrawFacesOLD(DRAW_OBSTS_AND_VENTS);
    return;
  }

  if(nface_normals_single>0){
    int j;

    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,global_scase.color_defs.block_ambient2);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,global_scase.color_defs.block_specular2);
    glEnable(GL_COLOR_MATERIAL);
    glBegin(GL_TRIANGLES);
    for(j=0;j<global_scase.meshescoll.nmeshes;j++){
      meshdata *meshi;
      int i;

      meshi=global_scase.meshescoll.meshinfo + j;
      if(meshi->blockvis==0)continue;

      // DOWN_X faces

      glNormal3f(-1.0,0.0,0.0);
      for(i=0;i<meshi->nface_normals_single_DOWN_X;i++){
        facedata *facei;

        facei = meshi->face_normals_single_DOWN_X[i];
        DRAWFACE(facei,smv_eyepos[0] > facepos[0], down_color)
      }

      // UP_X faces

      glNormal3f(1.0,0.0,0.0);
      for(i=0;i<meshi->nface_normals_single_UP_X;i++){
        facedata *facei;

        facei = meshi->face_normals_single_UP_X[i];
        DRAWFACE(facei,smv_eyepos[0]<facepos[0],up_color)
      }

      // DOWN_Y faces

      glNormal3f(0.0,-1.0,0.0);
      for(i=0;i<meshi->nface_normals_single_DOWN_Y;i++){
        facedata *facei;

        facei = meshi->face_normals_single_DOWN_Y[i];
        DRAWFACE(facei,smv_eyepos[1]>facepos[1],down_color)
      }

      // UP_Y faces

      glNormal3f(0.0,1.0,0.0);
      for(i=0;i<meshi->nface_normals_single_UP_Y;i++){
        facedata *facei;

        facei = meshi->face_normals_single_UP_Y[i];
        DRAWFACE(facei,smv_eyepos[1]<facepos[1],up_color)
      }

      // DOWN_Z faces

      glNormal3f(0.0,0.0,-1.0);
      for(i=0;i<meshi->nface_normals_single_DOWN_Z;i++){
        facedata *facei;

        facei = meshi->face_normals_single_DOWN_Z[i];
        DRAWFACE(facei,smv_eyepos[2]>facepos[2],down_color)
      }

      // UP_Z faces

      glNormal3f(0.0,0.0,1.0);
      for(i=0;i<meshi->nface_normals_single_UP_Z;i++){
        facedata *facei;

        facei = meshi->face_normals_single_UP_Z[i];
        DRAWFACE(facei,smv_eyepos[2]<facepos[2],up_color)
      }
    }
    glEnd();
    glDisable(GL_COLOR_MATERIAL);
    DISABLE_LIGHTING;
  }
  if(nface_normals_double>0){
    int j;

    new_color = NULL;
    old_color = NULL;
    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,global_scase.color_defs.block_ambient2);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,global_scase.color_defs.block_specular2);
    glEnable(GL_COLOR_MATERIAL);
    if(cullfaces==1)glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
    for(j=0;j<global_scase.meshescoll.nmeshes;j++){
      meshdata *meshi;
      int i;

      meshi=global_scase.meshescoll.meshinfo + j;
      for(i=0;i<meshi->nface_normals_double;i++){
        facedata *facei;
        float *vertices;

        facei = meshi->face_normals_double[i];
        if(blocklocation==BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
        }
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist!=NULL&&showtimelist[itimes]==0)continue;
        if(showedit_dialog == 0){
          new_color=facei->color;
        }
        else{
          if(visNormalEditColors==0)new_color=global_scase.color_defs.block_ambient2;
          if(visNormalEditColors==1)new_color=facei->color;
          if(highlight_block==facei->blockageindex&&highlight_mesh==facei->meshindex){
            new_color=highlight_color;
            switch(xyz_dir){
             case XDIR:
              if(facei->dir==UP_X)new_color=up_color;
              if(facei->dir==DOWN_X)new_color=down_color;
              break;
             case YDIR:
              if(facei->dir==UP_Y)new_color=up_color;
              if(facei->dir==DOWN_Y)new_color=down_color;
              break;
             case ZDIR:
              if(facei->dir==UP_Z)new_color=up_color;
              if(facei->dir==DOWN_Z)new_color=down_color;
              break;
             default:
              assert(FFALSE);
              break;
            }
          }
        }
        if(new_color!=old_color){
          old_color=new_color;
          glColor4fv(old_color);
        }
        glNormal3fv(facei->normal);
        glVertex3fv(vertices);
        glVertex3fv(vertices+3);
        glVertex3fv(vertices+6);
        glVertex3fv(vertices+9);
      }
    }
    glEnd();
    if(cullfaces==1)glEnable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);
    DISABLE_LIGHTING;
  }

  if(nface_outlines>0){
    int j;

    DISABLE_LIGHTING;
    AntiAliasLine(ON);
    glLineWidth(global_scase.linewidth);
    glBegin(GL_LINES);
    for(j=0;j<global_scase.meshescoll.nmeshes;j++){
      meshdata *meshi;
      int i;

      meshi = global_scase.meshescoll.meshinfo + j;
      if(meshi->blockvis==0)continue;
      for(i=0;i<meshi->nface_outlines;i++){
        facedata *facei;
        float *vertices;

        facei = meshi->face_outlines[i];
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist!=NULL&&showtimelist[itimes]==0&&facei->type2==BLOCK_face)continue;
        if(blocklocation==BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
        }
        if(facei->type2!=OUTLINE_FRAME_face || outline_mode==SCENE_OUTLINE_MESH){
          glEnd();
          if(global_scase.meshescoll.nmeshes>1&&facei->type2==OUTLINE_FRAME_face&&
            highlight_mesh==facei->meshindex&&outline_mode==1){
            glLineWidth(highlight_linewidth);
          }
          else{
            glLineWidth(*facei->linewidth);
          }
          glBegin(GL_LINES);
          glColor3fv(facei->linecolor);
          glVertex3fv(vertices);
          glVertex3fv(vertices+3);
          glVertex3fv(vertices+3);
          glVertex3fv(vertices+6);
          glVertex3fv(vertices+6);
          glVertex3fv(vertices+9);
          glVertex3fv(vertices+9);
          glVertex3fv(vertices);
          if(showtimelist!=NULL&&showtimelist[itimes]==0){
            glVertex3fv(vertices);
            glVertex3fv(vertices+6);
            glVertex3fv(vertices+3);
            glVertex3fv(vertices+9);
          }
        }
      }
    }
    glEnd();
    AntiAliasLine(OFF);
  }
  if(nface_textures>0){
    int j;
    ENABLE_LIGHTING;
    glEnable(GL_COLOR_MATERIAL);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,enable_texture_lighting? GL_MODULATE : GL_REPLACE);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,global_scase.color_defs.block_specular2);
    glEnable(GL_TEXTURE_2D);
    glColor4ub(255, 255, 255, 255);
    for(j=0;j<global_scase.meshescoll.nmeshes;j++){
      meshdata *meshi;
      int i;

      meshi = global_scase.meshescoll.meshinfo + j;
      if(meshi->blockvis==0)continue;
      for(i=0;i<meshi->nface_textures;i++){
        facedata *facei;
        float *tvertices;
        float *vertices;
        texturedata *texti;

        facei=meshi->face_textures[i];
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist!=NULL&&showtimelist[itimes]==0)continue;
        texti=facei->textureinfo;
        if(blocklocation==BLOCKlocation_grid){
           vertices = facei->approx_vertex_coords;
          tvertices = facei->approx_texture_coords;
        }
        else{
           vertices = facei->exact_vertex_coords;
          tvertices = facei->exact_texture_coords;
        }

        if(facei->type2==BLOCK_face&&cullfaces==0)glDisable(GL_CULL_FACE);


        glBindTexture(GL_TEXTURE_2D,texti->name);
        glBegin(GL_QUADS);

        glNormal3fv(facei->normal);
        glTexCoord2fv(tvertices);
        glVertex3fv(vertices);

        glTexCoord2fv(tvertices+2);
        glVertex3fv(vertices+3);

        glTexCoord2fv(tvertices+4);
        glVertex3fv(vertices+6);

        glTexCoord2fv(tvertices+6);
        glVertex3fv(vertices+9);
        glEnd();
      }
      if(cullfaces==1)glEnable(GL_CULL_FACE);


    }
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    DISABLE_LIGHTING;
  }
}

/* ------------------ CompareTransparentFaces ------------------------ */

int CompareTransparentFaces(const void *arg1, const void *arg2){
  facedata *facei, *facej;

  facei = *(facedata **)arg1;
  facej = *(facedata **)arg2;

  if(facei->dist2eye<facej->dist2eye)return  1;
  if(facei->dist2eye>facej->dist2eye)return -1;
  return 0;
}

/* ------------------ SortTransparentFaces ------------------------ */

void SortTransparentFaces(float *mm){
  int i;

  for(i=0;i<nface_transparent;i++){
    facedata *facei;
    float *xyzface;
    float xyzeye[3];

    facei = face_transparent[i];
    xyzface = facei->approx_center_coord;
    xyzeye[0] = mm[0]*xyzface[0] + mm[4]*xyzface[1] +  mm[8]*xyzface[2] + mm[12];
    xyzeye[1] = mm[1]*xyzface[0] + mm[5]*xyzface[1] +  mm[9]*xyzface[2] + mm[13];
    xyzeye[2] = mm[2]*xyzface[0] + mm[6]*xyzface[1] + mm[10]*xyzface[2] + mm[14];
    xyzeye[0]/=mscale[0];
    xyzeye[1]/=mscale[1];
    xyzeye[2]/=mscale[2];
    facei->dist2eye=xyzeye[0]*xyzeye[0]+xyzeye[1]*xyzeye[1]+xyzeye[2]*xyzeye[2];
  }
  qsort((facedata **)face_transparent,(size_t)nface_transparent,sizeof(facedata *),CompareTransparentFaces);
}

/* ------------------ DrawTransparentFaces ------------------------ */

void DrawTransparentFaces(){
  float old_color[4]={(float)-1.0,(float)-1.0,(float)-1.0,(float)-1.0};
  float *new_color;
  int **showtimelist_handle, *showtimelist;
  float up_color[4]={0.9,0.9,0.9,1.0};
  float down_color[4]={0.1,0.1,0.1,1.0};
  float highlight_color[4]={1.0,0.0,0.0,1.0};
  int drawing_transparent, drawing_blockage_transparent, drawing_vent_transparent;

  if(blocklocation==BLOCKlocation_cad||(NCADGeom(&global_scase.cadgeomcoll)!=0&&show_cad_and_grid==1))return;

  GetDrawingParms(&drawing_transparent, &drawing_blockage_transparent, &drawing_vent_transparent);

  if(nface_transparent<=0&&nface_transparent_double<=0)return;

  if(drawing_transparent==1)TransparentOn();

  if(nface_transparent>0){
    int i;

    new_color=global_scase.color_defs.block_ambient2;
    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,global_scase.color_defs.block_ambient2);
    glEnable(GL_COLOR_MATERIAL);
    glBegin(GL_QUADS);
    for(i=0;i<nface_transparent;i++){
      facedata *facei;
      float *vertices;

      facei = face_transparent[i];
      if(blocklocation==BLOCKlocation_grid){
        vertices = facei->approx_vertex_coords;
      }
      else{
        vertices = facei->exact_vertex_coords;
      }
      showtimelist_handle = facei->showtimelist_handle;
      showtimelist = *showtimelist_handle;
      if(showtimelist!=NULL&&showtimelist[itimes]==0)continue;
      if(showedit_dialog == 0){
        new_color=facei->color;
      }
      else{
        if(visNormalEditColors==0)new_color=global_scase.color_defs.block_ambient2;
        if(visNormalEditColors==1)new_color=facei->color;
        if(highlight_block==facei->blockageindex&&highlight_mesh==facei->meshindex){
          new_color=highlight_color;
          switch(xyz_dir){
           case XDIR:
            if(facei->dir==UP_X)new_color=up_color;
            if(facei->dir==DOWN_X)new_color=down_color;
            break;
           case YDIR:
            if(facei->dir==UP_Y)new_color=up_color;
            if(facei->dir==DOWN_Y)new_color=down_color;
            break;
           case ZDIR:
            if(facei->dir==UP_Z)new_color=up_color;
            if(facei->dir==DOWN_Z)new_color=down_color;
            break;
           default:
            assert(FFALSE);
            break;
          }
        }
      }
      if(
         ABS(new_color[0]-old_color[0])>0.0001||
         ABS(new_color[1]-old_color[1])>0.0001||
         ABS(new_color[2]-old_color[2])>0.0001||
         ABS(new_color[3]-old_color[3])>0.0001||
         use_transparency_geom==1
         ){
        old_color[0]=new_color[0];
        old_color[1]=new_color[1];
        old_color[2]=new_color[2];
        old_color[3]=new_color[3];
        if(use_transparency_geom==1)old_color[3]=transparency_geom;
        glColor4fv(old_color);
      }
      glNormal3fv(facei->normal);
      glVertex3fv(vertices);
      glVertex3fv(vertices+3);
      glVertex3fv(vertices+6);
      glVertex3fv(vertices+9);
    }
    glEnd();
    DISABLE_LIGHTING;
    glDisable(GL_COLOR_MATERIAL);
  }

  if(nface_transparent_double>0){
    int j;

    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,global_scase.color_defs.block_ambient2);
    glEnable(GL_COLOR_MATERIAL);
    if(cullfaces==1)glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
    for(j=0;j<global_scase.meshescoll.nmeshes;j++){
      meshdata *meshi;
      int i;

      meshi=global_scase.meshescoll.meshinfo + j;
      for(i=0;i<meshi->nface_transparent_double;i++){
        facedata *facei;
        float *vertices;

        facei = meshi->face_transparent_double[i];
        if(blocklocation==BLOCKlocation_grid){
          vertices = facei->approx_vertex_coords;
        }
        else{
          vertices = facei->exact_vertex_coords;
        }
        showtimelist_handle = facei->showtimelist_handle;
        showtimelist = *showtimelist_handle;
        if(showtimelist!=NULL&&showtimelist[itimes]==0)continue;
        new_color=facei->color;
        if(
         ABS(new_color[0]-old_color[0])>0.0001||
         ABS(new_color[1]-old_color[1])>0.0001||
         ABS(new_color[2]-old_color[2])>0.0001||
         ABS(new_color[3]-old_color[3])>0.0001
         ){
          old_color[0]=new_color[0];
          old_color[1]=new_color[1];
          old_color[2]=new_color[2];
          old_color[3]=new_color[3];
          glColor4fv(old_color);
        }
        glNormal3fv(facei->normal);
        glVertex3fv(vertices);
        glVertex3fv(vertices+3);
        glVertex3fv(vertices+6);
        glVertex3fv(vertices+9);
      }
    }
    glEnd();
    if(cullfaces==1)glEnable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);
    DISABLE_LIGHTING;
  }

  if(drawing_transparent==1)TransparentOff();
}

/* ------------------ GetFaceNabor ------------------------ */

facedata *GetFaceNabor(meshdata *meshi, facedata *facei, int dir){
  meshdata *meshj;
  int j;

  meshj = meshi->nabors[dir];
  if(meshj == NULL)return NULL;
  switch(dir){
  case MDOWN:
    if(facei->kmin != 0 || facei->kmin != facei->kmax)return NULL;
    for(j = 5; j < 6 * meshj->nbptrs; j += 6){
      facedata *facej;

      facej = meshj->faceinfo + j;
      if(facej->kmin != meshj->kbar || facej->kmax != facej->kmin)continue;
      if(facei->imin != facej->imin || facei->jmin != facej->jmin)continue;
      if(facei->imax != facej->imax || facei->jmax != facej->jmax)continue;
      return facej;
    }
    break;
  case MUP:
    if(facei->kmin != meshi->kbar || facei->kmax != facei->kmin)return NULL;
    for(j = 4; j < 6 * meshj->nbptrs; j += 6){
      facedata *facej;

      facej = meshj->faceinfo + j;
      if(facej->kmin != meshj->kbar || facej->kmax != facej->kmin)continue;
      if(facei->imin != facej->imin || facei->jmin != facej->jmin)continue;
      if(facei->imax != facej->imax || facei->jmax != facej->jmax)continue;
      return facej;
    }
    break;
  case MFRONT:
    if(facei->jmin != 0 || facei->jmin != facei->jmax)return NULL;
    for(j = 2; j < 6 * meshj->nbptrs; j += 6){
      facedata *facej;

      facej = meshj->faceinfo + j;
      if(facej->jmin != meshj->jbar || facej->jmax != facej->jmin)continue;
      if(facei->imin != facej->imin || facei->kmin != facej->kmin)continue;
      if(facei->imax != facej->imax || facei->kmax != facej->kmax)continue;
      return facej;
    }
    break;
  case MBACK:
    if(facei->jmin != meshi->jbar || facei->jmax != facei->jmin)return NULL;
    for(j = 0; j < 6 * meshj->nbptrs; j += 6){
      facedata *facej;

      facej = meshj->faceinfo + j;
      if(facej->jmin != meshj->jbar || facej->jmax != facej->jmin)continue;
      if(facei->imin != facej->imin || facei->kmin != facej->kmin)continue;
      if(facei->imax != facej->imax || facei->kmax != facej->kmax)continue;
      return facej;
    }
    break;
  case MLEFT:
    if(facei->imin != 0 || facei->imin != facei->imax)return NULL;
    for(j = 1; j < 6 * meshj->nbptrs; j += 6){
      facedata *facej;

      facej = meshj->faceinfo + j;
      if(facej->imin != meshj->ibar || facej->imax != facej->imin)continue;
      if(facei->kmin != facej->kmin || facei->jmin != facej->jmin)continue;
      if(facei->kmax != facej->kmax || facei->jmax != facej->jmax)continue;
      return facej;
    }
    break;
  case MRIGHT:
    if(facei->imin != meshi->ibar || facei->imax != facei->imin)return NULL;
    for(j = 3; j < 6 * meshj->nbptrs; j += 6){
      facedata *facej;

      facej = meshj->faceinfo + j;
      if(facej->imin != meshj->ibar || facej->imax != facej->imin)continue;
      if(facei->kmin != facej->kmin || facei->jmin != facej->kmin)continue;
      if(facei->kmax != facej->kmax || facei->jmax != facej->kmax)continue;
      return facej;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
  return NULL;
}

/* ------------------ AllocateFaces ------------------------ */

void AllocateFaces(){
  int i;
  int ntotal2=0;
  int abortflag=0;

  FREEMEMORY(face_transparent);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int ntotal;

    meshi = global_scase.meshescoll.meshinfo + i;
    ntotal = 6*meshi->nbptrs + meshi->nvents+12;
    ntotal2 += ntotal;

    if(ntotal>0){
      if(abortflag==0 && NEWMEM(meshi->faceinfo,sizeof(facedata)*ntotal)==0){
        abortflag=1;
      }
      if(abortflag==0 && NEWMEM(meshi->face_normals_single,sizeof(facedata *)*ntotal)==0){
        abortflag = 1;
      }
      if(abortflag==0 && NEWMEM(meshi->face_normals_double,sizeof(facedata *)*ntotal)==0){
        abortflag=1;
      }
      if(abortflag==0 && NEWMEM(meshi->face_transparent_double,sizeof(facedata *)*ntotal)==0){
        abortflag=1;
      }
      if(abortflag==0 && NEWMEM(meshi->face_textures,sizeof(facedata *)*ntotal)==0){
        abortflag=1;
      }
      if(abortflag==0 && NEWMEM(meshi->face_outlines,sizeof(facedata *)*ntotal)==0){
        abortflag=1;
      }
    }
  }
  if(ntotal2>0){
    if(abortflag==0&&NEWMEM(face_transparent,sizeof(facedata *)*ntotal2)==0){
      abortflag=1;
    }
  }
  if(abortflag==1){
    int mem_sum;
    float rmem;
    int nfaces_temp;

    mem_sum=0;
    nfaces_temp=0;
    ntotal2=0;
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      int ntotal;
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;

      ntotal = 6*meshi->nbptrs + meshi->nvents+12;
      nfaces_temp+=(6*meshi->nbptrs);
      mem_sum+= ntotal*(sizeof(facedata)+5*sizeof(facedata *));
      ntotal2 += ntotal;
    }
    mem_sum+= ntotal2*sizeof(facedata *);
    fprintf(stderr,"*** Error (fatal):  Unable to allocate ");
    if(mem_sum>=1000000000){
      rmem=(float)mem_sum/1000000000.0;
      fprintf(stderr,"%4.2f GB of memory\n",rmem);
      fprintf(stderr,"                  for %i blockage faces.\n",nfaces_temp);
    }
    else if(mem_sum>1000000&&mem_sum<1000000000){
      rmem=(float)mem_sum/1000000.0;
      fprintf(stderr,"%4.2f MB of memory\n",rmem);
      fprintf(stderr,"                  for %i blockage faces.\n",nfaces_temp);
    }
    else{
      fprintf(stderr,"%i bytes of memory\n",mem_sum);
      fprintf(stderr,"                  for %i blockage faces.\n",nfaces_temp);
    }
    fprintf(stderr,"*** Error: memory allocation error\n");
  }
}

/* ------------------ CompareBlock ------------------------ */

int CompareBlock(const void *arg1, const void *arg2){
  blockagedata *bc1,*bc2;
  int i1, i2;

  i1 = *(int *)arg1;
  i2 = *(int *)arg2;
  bc1 = selectblockinfo[i1];
  bc2 = selectblockinfo[i2];
  if(bc1->blockage_id!=-1&&bc2->blockage_id!=-1){
    if(bc1->blockage_id<bc2->blockage_id)return -1;
    if(bc1->blockage_id>bc2->blockage_id)return 1;
    return 0;
  }
  if(bc1->blockage_id==-1&&bc2->blockage_id==-1)return 0;
  if(bc1->blockage_id==-1)return 1;
  if(bc2->blockage_id==-1)return -1;
  return 0;
}

/* ------------------ UpdateSelectBlocks ------------------------ */

void UpdateSelectBlocks(void){
  int i;
  int ntotal=0;
  int local_count=0;

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    ntotal += meshi->nbptrs;
  }
  if(ntotal==0)return;
  FREEMEMORY(selectblockinfo);
  FREEMEMORY(sortedblocklist);

  NewMemory((void **)&selectblockinfo,sizeof(blockagedata *)*ntotal);
  NewMemory((void **)&sortedblocklist,sizeof(int)*ntotal);

  for(i=0;i<ntotal;i++){
    sortedblocklist[i]=i;
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[j];
      selectblockinfo[local_count++]=bc;
    }
  }
  qsort(sortedblocklist,(size_t)ntotal,sizeof(int),CompareBlock);
  nselectblocks=ntotal;
}

/* ------------------ UpdateSelectFaces ------------------------ */

void UpdateSelectFaces(void){

  /* store info about faces that could be selected */

  int i;
  selectdata *sd;

  FREEMEMORY(selectfaceinfo);

  ntotalfaces=0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo + i;
    ntotalfaces += 6*meshi->nbptrs;
  }
  if(ntotalfaces==0)return;

  NewMemory((void **)&selectfaceinfo,ntotalfaces*sizeof(selectdata));

/* down y
     up x
     up y
   down x
   down z
     up z */
  ntotalfaces=0;
  sd = selectfaceinfo;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi=global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->nbptrs;j++){
      int k;

      for(k=0;k<6;k++){
        facedata *facek;
        int sides[]={DOWN_Y,UP_X,UP_Y,DOWN_X,DOWN_Z,UP_Z};

        facek = meshi->faceinfo + 6*j + sides[k];
        facek->blockageindex=j;
        facek->meshindex=i;
        ntotalfaces++;
        sd->mesh=i;
        sd->dir=facek->dir;
        sd->side=sides[k];
        sd->blockage=j;
        sd->facei=facek;
        sd->type=BLOCK_face;
        sd++;
      }
    }
  }
}

/* ------------------ InitDemo ------------------------ */

void InitDemo(float rad, int nlat, int nlong){
  int i,j;
  float phi, psi;
  extern float *sphere_xyz;
  extern int update_demo;
  float *s_xyz;

  if(nlat<=0||nlong<=0)return;
  update_demo=0;
  FREEMEMORY(sphere_xyz);
  NewMemory((void **)&sphere_xyz,3*nlat*(nlong+1)*sizeof(float));
  s_xyz=sphere_xyz;
  for(j=0;j<nlong+1;j++){
    phi=-PI + 2.0*PI*j/nlong;
    for(i=0;i<nlat;i++){
      psi = -PI/2.0 + i*PI/(nlat-1);
      *s_xyz++ = 0.2143 + rad*cos(psi)*cos(phi);
      *s_xyz++ = 0.2143 + rad*cos(psi)*sin(phi);
      *s_xyz++ = 0.5 + rad*sin(psi);
    }
  }
}

/* ------------------ CalcNormal3 ------------------------ */

void CalcNormal3(const float *v1,
                 const float *v2,
                 const float *v3,
                 float *out){
  float u[3], v[3];
  int i;


  for(i=0;i<3;i++){
    u[i]=v2[i]-v1[i];
    v[i]=v3[i]-v1[i];
  }


  out[0] = u[1]*v[2] - u[2]*v[1];
  out[1] = u[2]*v[0] - u[0]*v[2];
  out[2] = u[0]*v[1] - u[1]*v[0];

  ReduceToUnit(out);

}

/* ------------------ CalcNormal4 ------------------------ */

void CalcNormal4(const float *v1,
                 float *out){
  out[0]=v1[0]-0.2143;
  out[1]=v1[1]-0.2143;
  out[2]=v1[2]-0.5;



  ReduceToUnit(out);

}

/* ------------------ DrawDemo2 ------------------------ */

void DrawDemo2(void){
      demo_mode++;
      glBegin(GL_QUADS);
      if(demo_mode%2==0){
        glColor3f(1.0,0.0,0.0);
      }
      else{
        glColor3f(0.0,0.0,1.0);
      }
      glVertex3f(0.0,0.3,0.0);
      glVertex3f(0.0,0.6,0.0);
      glVertex3f(1.0,0.6,0.0);
      glVertex3f(1.0,0.3,0.0);
      glEnd();
}

/* ------------------ DrawDemo ------------------------ */

void DrawDemo(int nlat, int nlong){
  int i, j;
  extern float *sphere_xyz;
  extern int update_demo,demo_mode;
  float red, green;
//  float blue;
  float *xyz;
  float *xyz00,*xyz01,*xyz10,*xyz11;
  float norm[3];
//  float norm1[3],norm2[3],norm3[3];
//  float denom;
  float specular[4]={0.8,0.8,0.8,1.0};
#define sphere_index(ilat,ilong) (3*((ilong)*nlat + (ilat)))

  if(nlat<=0||nlong<=0)return;
  if(update_demo==1)InitDemo(0.4,nlat,nlong);
  switch(demo_mode){
    case 0:
      glPointSize(6.0);
      glColor3f(0.0,0.0,1.0);
      glBegin(GL_POINTS);
      for(j=0;j<nlong;j++){
        for(i=0;i<nlat;i++){
          xyz = sphere_xyz + sphere_index(i,j);
          glVertex3fv(xyz);
        }
      }
      glEnd();
      break;
    case 1:
      glLineWidth(2.0);
      glBegin(GL_LINES);
      glColor3f(0.0,0.0,1.0);
      for(j=0;j<nlong;j++){
        for(i=0;i<nlat-1;i++){
          xyz00 = sphere_xyz + sphere_index(i,j);
          xyz10 = sphere_xyz + sphere_index(i,j+1);
          xyz01 = sphere_xyz + sphere_index(i+1,j);
          glVertex3fv(xyz00);
          glVertex3fv(xyz01);
          glVertex3fv(xyz00);
          glVertex3fv(xyz10);
        }
      }
      glEnd();
      break;
    case 2:
      glBegin(GL_TRIANGLES);
      glColor3f(0.0,0.0,1.0);
      for(j=0;j<nlong;j++){
        for(i=0;i<nlat-1;i++){
          xyz00 = sphere_xyz + sphere_index(i,j);
          xyz10 = sphere_xyz + sphere_index(i,j+1);
          xyz01 = sphere_xyz + sphere_index(i+1,j);
          xyz11 = sphere_xyz + sphere_index(i+1,j+1);
          glVertex3fv(xyz00);
          glVertex3fv(xyz01);
          glVertex3fv(xyz11);
          glVertex3fv(xyz00);
          glVertex3fv(xyz11);
          glVertex3fv(xyz10);
        }
      }
      glEnd();
      break;
    case 3:
    case 4:
    case 5:
//#define COLOR(x) (1.0+((x)-0.2143)/0.3)/2.0
#define COLOR(x) 0.0
      ENABLE_LIGHTING;
      glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,global_scase.color_defs.block_ambient2);
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specular);
      glEnable(GL_COLOR_MATERIAL);
      for(j=0;j<nlong;j++){
        for(i=0;i<nlat-1;i++){
          xyz00 = sphere_xyz + sphere_index(i,j);
          xyz10 = sphere_xyz + sphere_index(i,j+1);
          xyz01 = sphere_xyz + sphere_index(i+1,j);
          xyz11 = sphere_xyz + sphere_index(i+1,j+1);

          if(demo_mode==3)CalcNormal3(xyz00,xyz11,xyz01,norm);
          glBegin(GL_TRIANGLES);
          if(demo_mode!=3)CalcNormal4(xyz00,norm);
          glNormal3fv(norm);
          red = COLOR(xyz00[0]);
          green = COLOR(xyz00[1]);
          glColor3f(red,green,1.0);
          glVertex3fv(xyz00);

          red = COLOR(xyz11[0]);
          green = COLOR(xyz11[1]);
          if(demo_mode!=3)CalcNormal4(xyz11,norm);
          glNormal3fv(norm);
          glColor3f(red,green,1.0);
          glVertex3fv(xyz11);

          red = COLOR(xyz01[0]);
          green = COLOR(xyz01[1]);
          glColor3f(red,green,1.0);
          if(demo_mode!=3)CalcNormal4(xyz01,norm);
          glNormal3fv(norm);
          glVertex3fv(xyz01);

          glEnd();
          if(demo_mode==5){
            glLineWidth(2.0);
            glBegin(GL_LINES);
            glColor3f(0.0,0.0,0.0);
            glVertex3fv(xyz00);
            glVertex3f(xyz00[0]+norm[0]/10.0,xyz00[1]+norm[1]/10.0,xyz00[2]+norm[2]/10.0);
            glEnd();
          }

          if(demo_mode==3)CalcNormal3(xyz00,xyz11,xyz01,norm);
          glBegin(GL_TRIANGLES);
          if(demo_mode==3)CalcNormal3(xyz00,xyz11,xyz01,norm);
          red = COLOR(xyz00[0]);
          green = COLOR(xyz00[1]);
          glColor3f(red,green,1.0);
          if(demo_mode!=3)CalcNormal4(xyz00,norm);
          glNormal3fv(norm);
          glVertex3fv(xyz00);

          red = COLOR(xyz10[0]);
          green = COLOR(xyz10[1]);
          glColor3f(red,green,1.0);
          if(demo_mode!=3)CalcNormal4(xyz10,norm);
          glNormal3fv(norm);
          glVertex3fv(xyz10);

          red = COLOR(xyz11[0]);
          green = COLOR(xyz11[1]);
          glColor3f(red,green,1.0);
          if(demo_mode!=3)CalcNormal4(xyz11,norm);
          glNormal3fv(norm);
          glVertex3fv(xyz11);

          glEnd();
        }
      }
      DISABLE_LIGHTING;
      glDisable(GL_COLOR_MATERIAL);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ InitUserTicks ------------------------ */

void InitUserTicks(void){
  int i;

  user_tick_min[0]=1000000000.0;
  user_tick_min[1]=1000000000.0;
  user_tick_min[2]=1000000000.0;
  user_tick_max[0]=-1000000000.0;
  user_tick_max[1]=-1000000000.0;
  user_tick_max[2]=-1000000000.0;

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    user_tick_min[0]=MIN(meshi->boxmin_fds[0],user_tick_min[0]);
    user_tick_min[1]=MIN(meshi->boxmin_fds[1],user_tick_min[1]);
    user_tick_min[2]=MIN(meshi->boxmin_fds[2],user_tick_min[2]);

    user_tick_max[0]=MAX(meshi->boxmax_fds[0],user_tick_max[0]);
    user_tick_max[1]=MAX(meshi->boxmax_fds[1],user_tick_max[1]);
    user_tick_max[2]=MAX(meshi->boxmax_fds[2],user_tick_max[2]);
  }

  user_tick_origin[0]=user_tick_min[0];
  user_tick_origin[1]=user_tick_min[1];
  user_tick_origin[2]=user_tick_min[2];

  user_tick_step[0]=1.0;
  user_tick_step[1]=1.0;
  user_tick_step[2]=1.0;

  user_tick_sub=5;

  user_tick_length=0.1;
  user_tick_width=2.0;
  user_tick_direction = 1.0;
}

/* ------------------ GetTickDir ------------------------ */

int GetTickDir(float *mm){
  /*
  ( m0 m4 m8  m12 ) (x)    (0)
  ( m1 m5 m9  m13 ) (y)    (0)
  ( m2 m6 m10 m14 ) (z)  = (0)
  ( m3 m7 m11 m15 ) (1)    (1)

  ( m0 m4  m8 )      (m12)
  Q=  ( m1 m5  m9 )  u = (m13)
  ( m2 m6 m10 )      (m14)

  (Q   u) (x)     (0)
  (v^T 1) (y)   = (1)

  m3=m7=m11=0, v^T=0, y=1   Qx+u=0 => x=-Q^Tu
  */
  int i, ii;
  float norm[3], scalednorm[3];
  float normdir[3];
  float absangle, cosangle, minangle;
  int iminangle;

  eye_position_smv[0] = -(mm[0] * mm[12] + mm[1] * mm[13] + mm[2] * mm[14]) / mscale[0];
  eye_position_smv[1] = -(mm[4] * mm[12] + mm[5] * mm[13] + mm[6] * mm[14]) / mscale[1];
  eye_position_smv[2] = -(mm[8] * mm[12] + mm[9] * mm[13] + mm[10] * mm[14]) / mscale[2];

  minangle = 1000000.0;

  for(i = -3;i <= 3;i++){
    if(i == 0)continue;
    ii = ABS(i);
    norm[0] = 0.0;
    norm[1] = 0.0;
    norm[2] = 0.0;
    switch(ii){
    case XDIR:
      if(i<0)norm[1] = -1.0;
      if(i>0)norm[1] = 1.0;
      break;
    case YDIR:
      if(i<0)norm[0] = -1.0;
      if(i>0)norm[0] = 1.0;
      break;
    case ZDIR:
      if(i<0)norm[2] = -1.0;
      if(i>0)norm[2] = 1.0;
      break;
    default:
      assert(FFALSE);
      break;
    }
    scalednorm[0] = norm[0] * mscale[0];
    scalednorm[1] = norm[1] * mscale[1];
    scalednorm[2] = norm[2] * mscale[2];

    normdir[0] = mm[0] * scalednorm[0] + mm[4] * scalednorm[1] + mm[8] * scalednorm[2];
    normdir[1] = mm[1] * scalednorm[0] + mm[5] * scalednorm[1] + mm[9] * scalednorm[2];
    normdir[2] = mm[2] * scalednorm[0] + mm[6] * scalednorm[1] + mm[10] * scalednorm[2];

    cosangle = normdir[2] / sqrt(normdir[0] * normdir[0] + normdir[1] * normdir[1] + normdir[2] * normdir[2]);
    cosangle = CLAMP(cosangle, -1.0, 1.0);
    absangle = acos(cosangle)*RAD2DEG;
    absangle = ABS(absangle);
    if(absangle<minangle){
      iminangle = i;
      minangle = absangle;
    }
  }
  return iminangle;
}

/* ------------------ DrawUserTicks ------------------------ */

void DrawUserTicks(void){
  int i;
  float xyz[3],xyz2[3];
  float tick_origin[3]={0,0,0}, step[3]={0,0,0};
  int show_tick_x=0, show_tick_y=0, show_tick_z=0;
  float fds_tick_length;

#define MIN_DTICK 0.0
#define TEXT_FACTOR 1.5

  user_tick_option=GetTickDir(modelview_scratch);
  fds_tick_length = user_tick_direction*SCALE2FDS(user_tick_length);

  if(auto_user_tick_placement==0){
    tick_origin[0]=user_tick_origin[0];
    tick_origin[1]=user_tick_origin[1];
    tick_origin[2]=user_tick_origin[2];
    step[0]=user_tick_step[0];
    step[1]=user_tick_step[1];
    step[2]=user_tick_step[2];
    show_tick_x = user_tick_show_x;
    show_tick_y = user_tick_show_y;
    show_tick_z = user_tick_show_z;
  }
  if(auto_user_tick_placement==1){
    step[0]=ABS(user_tick_step[0]);
    step[1]=ABS(user_tick_step[1]);
    step[2]=ABS(user_tick_step[2]);
    show_tick_x=0;
    show_tick_y=0;
    show_tick_z=0;
    switch(user_tick_option){
      case XDIRNEG:
        tick_origin[0] = user_tick_origin[0];
        tick_origin[1] = user_tick_origin[1];
        tick_origin[2] = user_tick_origin[2];
        show_tick_x = 1;
        show_tick_z = 1;
        break;
      case XDIR:
        tick_origin[0] = user_tick_origin[0];
        tick_origin[1] = user_tick_max[1];
        step[1] = -step[1];
        tick_origin[2] = user_tick_origin[2];
        show_tick_x = 1;
        show_tick_z = 1;
        break;
      case YDIRNEG:
        tick_origin[0] = user_tick_origin[0];
        tick_origin[1] = user_tick_origin[1];
        tick_origin[2] = user_tick_origin[2];
        show_tick_y = 1;
        show_tick_z = 1;
        break;
      case YDIR:
        tick_origin[0] = user_tick_max[0];
        step[0] = -step[0];
        tick_origin[1] = user_tick_origin[1];
        tick_origin[2] = user_tick_origin[2];
        show_tick_y = 1;
        show_tick_z = 1;
        break;
      case ZDIRNEG:
        tick_origin[0] = user_tick_origin[0];
        tick_origin[1] = user_tick_origin[1];
        tick_origin[2] = user_tick_origin[2];
        show_tick_x = 1;
        show_tick_y = 1;
        break;
      case ZDIR:
        tick_origin[0] = user_tick_origin[0];
        tick_origin[1] = user_tick_origin[1];
        tick_origin[2] = user_tick_max[2];
        step[2] = -step[2];
        show_tick_x = 1;
        show_tick_y = 1;
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
  if(step[0]>MIN_DTICK){
    user_tick_nxyz[0]=ABS((user_tick_max[0]+1.0-tick_origin[0])/step[0]);
  }
  else if(step[0]<-MIN_DTICK){
    user_tick_nxyz[0]=ABS((tick_origin[0]+1.0-user_tick_min[0])/step[0]);
  }
  else{
    user_tick_nxyz[0]=0;
  }
  if(step[1]>MIN_DTICK){
    user_tick_nxyz[1]=ABS((user_tick_max[1]+1.0-tick_origin[1])/step[1]);
  }
  else if(step[1]<-MIN_DTICK){
    user_tick_nxyz[1]=ABS((tick_origin[1]+1.0-user_tick_min[1])/step[1]);
  }
  else{
    user_tick_nxyz[1]=0;
  }
  if(step[2]>MIN_DTICK){
    user_tick_nxyz[2]=ABS((user_tick_max[2]+1.0-tick_origin[2])/step[2]);
  }
  else if(step[2]<-MIN_DTICK){
    user_tick_nxyz[2]=ABS((tick_origin[2]+1.0-user_tick_min[2])/step[2]);
  }
  else{
    user_tick_nxyz[2]=0;
  }
  if(user_tick_option<0){
    user_tick_option=-user_tick_option;
  }
  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);

 //*** x axis tick/labels

 // major ticks
  if(show_tick_x==1){
    glLineWidth(user_tick_width);
    glBegin(GL_LINES);
    glColor3fv(foregroundcolor);
    for(i=0;i<user_tick_nxyz[0];i++){
      xyz[0]=tick_origin[0] + i*step[0];
      if(
        (step[0]>0.0&&xyz[0]>user_tick_max[0])||
        (step[0]<0.0&&xyz[0]<user_tick_min[0])
        )continue;
      xyz[1]=tick_origin[1];
      xyz[2]=tick_origin[2];
      if(user_tick_option==ZDIR){
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1]-fds_tick_length;
        xyz2[2]=xyz[2];
      }
      else{
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1];
        xyz2[2]=xyz[2]-fds_tick_length;
      }
      if(i==0){
        glVertex3fv(xyz);
        if(step[0]>0.0){
          glVertex3f(user_tick_max[0],xyz[1],xyz[2]);
        }
        else{
          glVertex3f(user_tick_min[0],xyz[1],xyz[2]);
        }
      }
      glVertex3fv(xyz);
      glVertex3fv(xyz2);
    }

// minor ticks

    if(user_tick_sub>1){
      for(i=1;i<user_tick_nxyz[0]*user_tick_sub;i++){
        if(i%user_tick_sub==0)continue;
        xyz[0]=tick_origin[0] + i*step[0]/(float)user_tick_sub;
        if(
          (step[0]>0.0&&xyz[0]>user_tick_max[0])||
          (step[0]<0.0&&xyz[0]<user_tick_min[0])
          )continue;
        xyz[1]=tick_origin[1];
        xyz[2]=tick_origin[2];
        if(user_tick_option==ZDIR){
          xyz2[0]=xyz[0];
          xyz2[1]=xyz[1]-fds_tick_length/2.0;
          xyz2[2]=xyz[2];
        }
        else{
          xyz2[0]=xyz[0];
          xyz2[1]=xyz[1];
          xyz2[2]=xyz[2]-fds_tick_length/2.0;
        }
        glVertex3fv(xyz);
        glVertex3fv(xyz2);
      }
    }
    glEnd();
    if(fontindex==SCALED_FONT)ScaleFont3D();
    for(i=0;i<user_tick_nxyz[0];i++){
      char label[128];

      xyz[0]=tick_origin[0] + i*step[0];
      if((step[0]>0.0&&xyz[0]>user_tick_max[0])||(step[0]<0.0&&xyz[0]<user_tick_min[0]))continue;
      xyz[1]=tick_origin[1];
      xyz[2]=tick_origin[2];
      if(user_tick_option==ZDIR){
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1]-TEXT_FACTOR*fds_tick_length;
        xyz2[2]=xyz[2];
      }
      else{
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1];
        xyz2[2]=xyz[2]-TEXT_FACTOR*fds_tick_length;
      }
      {
        char form[20], form2[20];

        strcpy(form,"%5.");
        sprintf(form2,"%i",ntick_decimals);
        strcat(form,form2);
        strcat(form,"f");
        sprintf(label,form,GetUnitVal("Distance",xyz[0], ntick_decimals));
      }
      Output3Text(foregroundcolor,xyz2[0],xyz2[1],xyz2[2], label);
    }
  }

 //*** y axis tick/labels

 // major ticks

  if(show_tick_y){
    glLineWidth(user_tick_width);
    glBegin(GL_LINES);
    glColor3fv(foregroundcolor);
    for(i=0;i<user_tick_nxyz[1];i++){
      xyz[0]=tick_origin[0];
      xyz[1]=tick_origin[1] + i*step[1];
      if(
        (step[1]>0.0&&xyz[1]>user_tick_max[1])||
        (step[1]<0.0&&xyz[1]<user_tick_min[1])
        )continue;
      xyz[2]=tick_origin[2];
      if(user_tick_option==ZDIR){
        xyz2[0]=xyz[0]-fds_tick_length;
        xyz2[1]=xyz[1];
        xyz2[2]=xyz[2];
      }
      else{
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1];
        xyz2[2]=xyz[2]-fds_tick_length;
      }
      if(i==0){
        glVertex3fv(xyz);
        if(step[1]>0.0){
          glVertex3f(xyz[0],user_tick_max[1],xyz[2]);
        }
        else{
          glVertex3f(xyz[0],user_tick_min[1],xyz[2]);
        }
      }
      glVertex3fv(xyz);
      glVertex3fv(xyz2);
    }

// minor ticks

    if(user_tick_sub>1){
      for(i=1;i<user_tick_nxyz[1]*user_tick_sub;i++){
        if(i%user_tick_sub==0)continue;
        xyz[0]=tick_origin[0];
        xyz[1]=tick_origin[1] + i*step[1]/(float)user_tick_sub;
        if(
          (step[1]>0.0&&xyz[1]>user_tick_max[1])||
          (step[1]<0.0&&xyz[1]<user_tick_min[1])
          )continue;
        xyz[2]=tick_origin[2];
        if(user_tick_option==ZDIR){
          xyz2[0]=xyz[0]-fds_tick_length/2.0;
          xyz2[1]=xyz[1];
          xyz2[2]=xyz[2];
        }
        else{
          xyz2[0]=xyz[0];
          xyz2[1]=xyz[1];
          xyz2[2]=xyz[2]-fds_tick_length/2.0;
        }
        glVertex3fv(xyz);
        glVertex3fv(xyz2);
      }
    }
    glEnd();
    if(fontindex==SCALED_FONT)ScaleFont3D();
    for(i=0;i<user_tick_nxyz[1];i++){
      char label[128];

      xyz[0]=tick_origin[0];
      xyz[1]=tick_origin[1] + i*step[1];
      if(
        (step[1]>0.0&&xyz[1]>user_tick_max[1])||
        (step[1]<0.0&&xyz[1]<user_tick_min[1])
        )continue;
      xyz[2]=tick_origin[2];
      xyz2[0]=xyz[0];
      if(user_tick_option==ZDIR){
        xyz2[0]=xyz[0]-TEXT_FACTOR*fds_tick_length;
        xyz2[1]=xyz[1];
        xyz2[2]=xyz[2];
      }
      else{
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1];
        xyz2[2]=xyz[2]-TEXT_FACTOR*fds_tick_length;
      }
      {
        char form[20], form2[20];

        strcpy(form,"%5.");
        sprintf(form2,"%i",ntick_decimals);
        strcat(form,form2);
        strcat(form,"f");
        sprintf(label,form,GetUnitVal("Distance",xyz[1], ntick_decimals));
      }
      Output3Text(foregroundcolor,xyz2[0],xyz2[1],xyz2[2], label);
    }
  }

 //*** z axis tick/labels

 // major ticks
  if(show_tick_z){
    glLineWidth(user_tick_width);
    glBegin(GL_LINES);
    glColor3fv(foregroundcolor);
    for(i=0;i<user_tick_nxyz[2];i++){
      xyz[0]=tick_origin[0];
      xyz[1]=tick_origin[1];
      xyz[2]=tick_origin[2] + i*step[2];
      if(
        (step[2]>0.0&&xyz[2]>user_tick_max[2])||
        (step[2]<0.0&&xyz[2]<user_tick_min[2])
        )continue;
      if(user_tick_option==YDIR){
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1]-fds_tick_length;
      }
      else{
        xyz2[0]=xyz[0]-fds_tick_length;
        xyz2[1]=xyz[1];
      }
      xyz2[2]=xyz[2];
      if(i==0){
        glVertex3fv(xyz);
        if(step[2]>0.0){
          glVertex3f(xyz[0],xyz[1],user_tick_max[2]);
        }
        else{
          glVertex3f(xyz[0],xyz[1],user_tick_min[2]);
        }
      }
      glVertex3fv(xyz);
      glVertex3fv(xyz2);
    }

// minor ticks

    if(user_tick_sub>1){
      for(i=1;i<user_tick_nxyz[2]*user_tick_sub;i++){
        if(i%user_tick_sub==0)continue;
        xyz[0]=tick_origin[0];
        xyz[1]=tick_origin[1];
        xyz[2]=tick_origin[2] + i*step[2]/(float)user_tick_sub;
        if(
          (step[2]>0.0&&xyz[2]>user_tick_max[2])||
          (step[2]<0.0&&xyz[2]<user_tick_min[2])
          )continue;
        if(user_tick_option==YDIR){
          xyz2[0]=xyz[0];
          xyz2[1]=xyz[1]-fds_tick_length/2.0;
        }
        else{
          xyz2[0]=xyz[0]-fds_tick_length/2.0;
          xyz2[1]=xyz[1];
        }
        xyz2[2]=xyz[2];
        glVertex3fv(xyz);
        glVertex3fv(xyz2);
      }
    }
    glEnd();
    if(fontindex==SCALED_FONT)ScaleFont3D();
    for(i=0;i<user_tick_nxyz[2];i++){
      char label[128];

      xyz[0]=tick_origin[0];
      xyz[1]=tick_origin[1];
      xyz[2]=tick_origin[2] + i*step[2];
      if(
        (step[2]>0.0&&xyz[2]>user_tick_max[2])||
        (step[2]<0.0&&xyz[2]<user_tick_min[2])
        )continue;
      if(user_tick_option==YDIR){
        xyz2[0]=xyz[0];
        xyz2[1]=xyz[1]-TEXT_FACTOR*fds_tick_length;
      }
      else{
        xyz2[0]=xyz[0]-TEXT_FACTOR*fds_tick_length;
        xyz2[1]=xyz[1];
      }
      xyz2[2]=xyz[2];
      {
        char form[20], form2[20];

        strcpy(form,"%5.");
        sprintf(form2,"%i",ntick_decimals);
        strcat(form,form2);
        strcat(form,"f");
        sprintf(label,form,GetUnitVal("Distance",xyz[2], ntick_decimals));
      }
      Output3Text(foregroundcolor,xyz2[0],xyz2[1],xyz2[2]+step[2]/20.0, label);
    }
  }

  glPopMatrix();
}

/* ------------------ DrawGravityAxis ------------------------ */

void DrawGravityAxis(void){
  glLineWidth(global_scase.linewidth);
  glBegin(GL_LINES);

  // x axis
  glColor3f(0.0,0.0,1.0);
  glVertex3f(global_scase.xbar/2.0,global_scase.ybar/2.0,global_scase.zbar/2.0);
  glVertex3f(global_scase.xbar/2.0,global_scase.ybar/2.0,global_scase.zbar/2.0+0.5);

  // y axis
  glColor3f(0.0,1.0,0.0);
  glVertex3f(global_scase.xbar/2.0,global_scase.ybar/2.0,global_scase.zbar/2.0);
  glVertex3f(global_scase.xbar/2.0,global_scase.ybar/2.0+0.5,global_scase.zbar/2.0);

  // z axis
  glColor3f(1.0,0.0,0.0);
  glVertex3f(global_scase.xbar/2.0,global_scase.ybar/2.0,global_scase.zbar/2.0);
  glVertex3f(global_scase.xbar/2.0+0.5,global_scase.ybar/2.0,global_scase.zbar/2.0);

  // gravity vector
  glColor3fv(foregroundcolor);
  glVertex3f(global_scase.xbar/2.0,global_scase.ybar/2.0,global_scase.zbar/2.0);
  glVertex3f(global_scase.xbar/2.0+global_scase.gvecunit[0],global_scase.ybar/2.0+global_scase.gvecunit[1],global_scase.zbar/2.0+global_scase.gvecunit[2]);

  glEnd();
  Output3Text(foregroundcolor, global_scase.xbar / 2.0, global_scase.ybar / 2.0, global_scase.zbar / 2.0 + 0.5, "z");
  Output3Text(foregroundcolor, global_scase.xbar / 2.0, global_scase.ybar / 2.0 + 0.5, global_scase.zbar / 2.0, "y");
  Output3Text(foregroundcolor, global_scase.xbar / 2.0 + 0.5, global_scase.ybar / 2.0, global_scase.zbar / 2.0, "x");
  Output3Text(foregroundcolor, global_scase.xbar / 2.0 + global_scase.gvecunit[0], global_scase.ybar / 2.0 + global_scase.gvecunit[1], global_scase.zbar / 2.0 + global_scase.gvecunit[2], "g");
}

/* ------------------ DrawTicks ------------------------ */

void DrawTicks(void){
  int i,j;
  tickdata *ticki;
  float *dxyz,xyz[3],xyz2[3],*begt,*endt,dbar[3];

  for(i=0;i<global_scase.ntickinfo;i++){
    ticki = global_scase.tickinfo + i;
    begt = ticki->begin;
    endt = ticki->end;

    glLineWidth(ticki->width);
    glBegin(GL_LINES);
    if(ticki->useforegroundcolor==1){
      glColor3fv(foregroundcolor);
    }
    else{
      glColor3fv(ticki->rgb);
    }

    dxyz=ticki->dxyz;
    if(ticki->nbars>1){
      dbar[0]=(endt[0]-begt[0])/(float)(ticki->nbars-1);
      dbar[1]=(endt[1]-begt[1])/(float)(ticki->nbars-1);
      dbar[2]=(endt[2]-begt[2])/(float)(ticki->nbars-1);
    }
    else{
      dbar[0] = 0.0;
      dbar[1] = 0.0;
      dbar[2] = 0.0;
    }

    for(j=0;j<ticki->nbars;j++){
      xyz[0]=begt[0] + j*dbar[0];
      xyz[1]=begt[1] + j*dbar[1];
      xyz[2]=begt[2] + j*dbar[2];
      xyz2[0]=xyz[0]+dxyz[0];
      xyz2[1]=xyz[1]+dxyz[1];
      xyz2[2]=xyz[2]+dxyz[2];
      FDS2SMV_XYZ(xyz,xyz);
      FDS2SMV_XYZ(xyz2,xyz2);
      glVertex3fv(xyz);
      glVertex3fv(xyz2);
    }
    glEnd();

  }
}

/* ------------------ DrawBlockages ------------------------ */

void DrawBlockages(int mode, int trans_flag){
  int i;
  cadgeomdata *cd;
  int drawing_transparent, drawing_blockage_transparent, drawing_vent_transparent;

  GetDrawingParms(&drawing_transparent, &drawing_blockage_transparent, &drawing_vent_transparent);

  if(trans_flag!=DRAW_TRANSPARENT&&blocklocation!=BLOCKlocation_cad){
    if(mode==SELECTOBJECT){
      if(blockageSelect==1){
        GLUIGetGeomDialogState();
        if(structured_isopen == 1 && unstructured_isopen == 0){
          DrawSelectFaces();
          return;
        }
      }
    }
    else{
      switch(blockage_draw_option){
      case 0:
        DrawFacesOLD(DRAW_OBSTS_AND_VENTS);
        break;
      case 1:
        DrawFaces();
        break;
      case 2:
      case 3:
        DrawObstsDebug();
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
  }

  if(blocklocation==BLOCKlocation_cad||(NCADGeom(&global_scase.cadgeomcoll)!=0&&show_cad_and_grid==1)){
    int ntriangles=0;

    DrawFacesOLD(DRAW_VENTS);
    for(i=0;i<NCADGeom(&global_scase.cadgeomcoll);i++){
      cd=global_scase.cadgeomcoll.cadgeominfo+i;
      if(cd->version==1){
        if(trans_flag==DRAW_TRANSPARENT)continue;
        if(clip_mode==CLIP_BLOCKAGES)SetClipPlanes(&clipinfo,CLIP_ON);
        DrawCADGeom(cd);
        if(clip_mode==CLIP_BLOCKAGES)SetClipPlanes(NULL,CLIP_OFF);
      }
      else if(cd->version==2){
        if(clip_mode==CLIP_BLOCKAGES)SetClipPlanes(&clipinfo,CLIP_ON);
        DrawCAD2Geom(cd,trans_flag);
        if(clip_mode==CLIP_BLOCKAGES)SetClipPlanes(NULL,CLIP_OFF);
      }
      ntriangles+=2*cd->nquads;
    }
    if(show_triangle_count==1)printf("cad triangles: %i\n",ntriangles);
  }
}
/* ------------------ LevelScene ------------------------ */

void LevelScene(int level_x, int level_y, float *quat){
  if(rotation_type==ROTATION_2AXIS&&key_state == KEY_NONE){
    float *elev;

    elev = camera_current->az_elev+1;
    *elev = 0.0;
    GLUIUpdateTrainerMoves();
    camera_current->dirty=1;
  }

  if(rotation_type==ROTATION_3AXIS&&key_state == KEY_NONE){
    float alpha,sum_axis;

    if(level_x==1)quat[1]=0.0;
    if(level_y==1)quat[2]=0.0;
    sum_axis = quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3];
    if(sum_axis==0.0){
      quat[0]=1.0;
    }
    else if(0.0<sum_axis&&sum_axis<1.0){
      alpha = sqrt((1.0-quat[0]*quat[0])/sum_axis);
      quat[1]*=alpha;
      quat[2]*=alpha;
      quat[3]*=alpha;
    }
    // do nothing if norm(quaternion axis)=1
  }
}

/* ------------------ SnapScene ------------------------ */

void SnapScene(int delta){
  float *az, *elev;
  int iaz, ielev;

  az   = camera_current->az_elev;
  elev = camera_current->az_elev+1;
  float DELTA;

  DELTA = (float)delta;
  if(delta == 0){
    *az   = 0.0;
    *elev = 0.0;
  }
  else{
    if(*az > 0.0) {
      iaz = (*az+DELTA/2.0)/DELTA;
    }
    else{
      iaz = (*az-DELTA/2.0)/DELTA;
    }
    *az = (int)(DELTA*iaz);

    if(*elev>0.0){
      ielev = (*elev+DELTA/2.0)/DELTA;
    }
    else{
      ielev = (*elev-DELTA/2.0)/DELTA;
    }
    *elev = (int)(DELTA*ielev);
  }
  GLUIUpdateTrainerMoves();
  camera_current->dirty=1;

  if(rotation_type==ROTATION_3AXIS&&key_state == KEY_NONE){
    float angle;

    if(delta == 0) {
      angle = 0.0;
    }
    else {
      angle = 2.0 * RAD2DEG * acos(quat_general[0]);
      if(angle > 0.0) {
        iaz = (angle + DELTA / 2.0) / DELTA;
      }
      else {
        iaz = (angle - DELTA / 2.0) / DELTA;
      }
      angle = (int)(DELTA * iaz);
    }

    quat_general[0]=cos(DEG2RAD*angle/2.0);
    quat_general[1]=0.0;
    quat_general[2]=0.0;
    quat_general[3]=sin(DEG2RAD*angle/2.0);
    Quat2Rot(quat_general,quat_rotation);
  }

}

/* ------------------ GetDrawingParms ------------------------ */

void GetDrawingParms(int *drawing_transparent, int *drawing_blockage_transparent, int *drawing_vent_transparent){
  *drawing_transparent=0;
  *drawing_blockage_transparent=0;
  *drawing_vent_transparent=0;
  if(ntransparentblocks>0){
    if(visTransparentBlockage==1||visBlocks==visBLOCKAsInput||visBlocks==visBLOCKAsInputOutline){
      if(visBlocks!=visBLOCKOutline&&visBlocks!=visBLOCKHide){
        *drawing_transparent=1;
        *drawing_blockage_transparent=1;
      }
    }
  }
  if(ntransparentvents>0&&show_transparent_vents==1){
    *drawing_transparent=1;
    *drawing_vent_transparent=1;
  }
}

/* ------------------ CompareBlockage ------------------------ */

int CompareBlockage(const void *arg1, const void *arg2){
  blockagedata *bc1, *bc2;
  int *ijk1, *ijk2;

  bc1 = *(blockagedata **)arg1;
  bc2 = *(blockagedata **)arg2;

  ijk1 = bc1->ijk;
  ijk2 = bc2->ijk;

  if(ijk1[0]<ijk2[0])return -1;
  if(ijk1[0]>ijk2[0])return 1;
  if(ijk1[1]<ijk2[1])return -1;
  if(ijk1[1]>ijk2[1])return 1;
  if(ijk1[2]<ijk2[2])return -1;
  if(ijk1[2]>ijk2[2])return 1;
  if(ijk1[3]<ijk2[3])return -1;
  if(ijk1[3]>ijk2[3])return 1;
  if(ijk1[4]<ijk2[4])return -1;
  if(ijk1[4]>ijk2[4])return 1;
  if(ijk1[5]<ijk2[5])return -1;
  if(ijk1[5]>ijk2[5])return 1;
  return 0;
}

/* ------------------ RemoveDupBlockages ------------------------ */

void RemoveDupBlockages(void){
  int i;

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;

    if(meshi->nbptrs>1){
      blockagedata **bclist;
      int jj,j;

      bclist=meshi->blockageinfoptrs;
      qsort(bclist,(size_t)meshi->nbptrs,sizeof(blockagedata *),CompareBlockage);
      for(j=1;j<meshi->nbptrs;j++){
        blockagedata *bc, *bcm1;
        int *ijk1, *ijk2;

        bc = bclist[j];
        bcm1 = bclist[j-1];
        if(bc->nshowtime>0)continue;
        if(bcm1->nshowtime>0)continue;
        ijk1=bcm1->ijk;
        ijk2=bc->ijk;
        if(ijk1[1]-ijk1[0]>1)continue; // only consider removing one cell blockages
        if(ijk1[3]-ijk1[2]>1)continue;
        if(ijk1[5]-ijk1[4]>1)continue;
        if(ijk1[0]!=ijk2[0]||ijk1[1]!=ijk2[1])continue;
        if(ijk1[2]!=ijk2[2]||ijk1[3]!=ijk2[3])continue;
        if(ijk1[4]!=ijk2[4]||ijk1[5]!=ijk2[5])continue;
        bcm1->dup=1;
        bc->dup=2;
      }
      jj=0;
      for(j=0;j<meshi->nbptrs;j++){
        blockagedata *bc;

        bc=bclist[j];
        if(bc->dup==1)continue;
        bclist[jj++]=bc;
      }
      meshi->nbptrs=jj;
    }
  }
  updatefacelists=1;
}

/* ------------------ GetObstLabels ------------------------ */

void GetObstLabels(const char *filein){

  FILE *stream_in;
  int fdsobstcount=0;
  char **obstlabels=NULL;
  int nobstlabels=0;
  int i;

  if(filein==NULL)return;
  stream_in = FOPEN(filein,"r");
  if(stream_in==NULL)return;

  while(!feof(stream_in)){
    char buffer[1000];

    if(fgets(buffer,1000,stream_in)==NULL)break;

    if(STRSTR(buffer,"&OBST")==NULL)continue;
    fdsobstcount++;
  }
  nobstlabels=fdsobstcount;
  if(nobstlabels>0){
    NewMemory((void **)&obstlabels,nobstlabels*sizeof(char *));
    for(i=0;i<nobstlabels;i++){
      obstlabels[i]=NULL;
    }
  }
  rewind(stream_in);
  fdsobstcount=0;
  while(!feof(stream_in)){
    char buffer[1000];
    char *obstlabel;
    size_t lenlabel;

    if(fgets(buffer,1000,stream_in)==NULL)break;

    if(STRSTR(buffer,"&OBST")==NULL)continue;
    fdsobstcount++;
    while((obstlabel=strstr(buffer,"/"))==NULL){
      fgets(buffer,1000,stream_in);
    }
    obstlabel++;
    obstlabel=TrimFront(obstlabel);
    TrimBack(obstlabel);
    lenlabel=strlen(obstlabel);
    if(lenlabel>0&&obstlabels!=NULL){
      NewMemory((void **)&obstlabels[fdsobstcount-1],(unsigned int)(lenlabel+1));
      strcpy(obstlabels[fdsobstcount-1],obstlabel);
    }
  }
  fclose(stream_in);

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;
      int id;

      bc = meshi->blockageinfoptrs[j];
      id = bc->blockage_id-1;
      if(id>=0&&id<nobstlabels){
        if(obstlabels[id]!=NULL){
          size_t lenlabel;

          lenlabel=strlen(obstlabels[id]);
          ResizeMemory((void **)&bc->label,(unsigned int)(lenlabel+1));
          strcpy(bc->label,obstlabels[id]);
        }
      }
    }
  }
  for(i=0;i<nobstlabels;i++){
    FREEMEMORY(obstlabels[i]);
  }
  FREEMEMORY(obstlabels);
}
