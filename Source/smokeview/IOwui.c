#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include GLUT_H

#include "update.h"
#include "smokeviewvars.h"
#include "IOobjects.h"

#define ijnode2(i,j) ((nxcell+1)*(j) + (i))
#define ijnode3(i,j) ((nycell+1)*(i) + (j))
#define FORTWUIREAD(var,size) FSEEK(WUIFILE,4,SEEK_CUR);\
                           returncode=fread(var,4,size,WUIFILE);\
                           FSEEK(WUIFILE,4,SEEK_CUR)

/* ------------------ DrawNorth ------------------------ */

void DrawNorth(void){
  glPushMatrix();
  glTranslatef(northangle_position[0], northangle_position[1], northangle_position[2]);
  glRotatef(-northangle, 0.0, 0.0, 1.0);
  glBegin(GL_LINES);
  glColor3fv(foregroundcolor);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.1, 0.0);
  glEnd();
  glPopMatrix();
}

  /*
typedef struct {
  float xyz[3];
  float trunk_diam;
  float tree_height;
  float base_diam;
  float base_height;
*/

/* ------------------ DrawTrees ------------------------ */

void DrawTrees(void){
  int i;

 ENABLE_LIGHTING;

 glEnable(GL_COLOR_MATERIAL);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);
  for(i=0;i<ntreeinfo;i++){
    treedata *treei;
    float crown_height;
    int state;

    treei = treeinfo + i;

    state=0;
    if(showtime==1&&global_times!=NULL){
      ASSERT(itimes>=0)
      if(treei->time_char>0.0&&global_times[itimes]>treei->time_char)state=1;
      if(treei->time_complete>0.0&&global_times[itimes]>treei->time_complete)state=2;
    }

    glPushMatrix();
    glTranslatef(treei->xyz[0],treei->xyz[1],treei->xyz[2]);

    switch(state){
      case 0:
        glColor4fv(trunccolor);
        DrawDisk(treei->trunk_diam,treei->base_height,trunccolor_uc);

        crown_height=treei->tree_height-treei->base_height;
        glTranslatef(0.0,0.0,treei->base_height);
        glColor4fv(treecolor);
        DrawCone(treei->base_diam,crown_height,treecolor_uc);
        break;
      case 1:
        glColor4fv(treecharcolor);
        DrawDisk(treei->trunk_diam,treei->base_height,trunccolor_uc);

        crown_height=treei->tree_height-treei->base_height;
        glTranslatef(0.0,0.0,treei->base_height);
        DrawCone(treei->base_diam,crown_height,treecolor_uc);
        break;
      case 2:
        glColor4fv(treecharcolor);
        DrawDisk(treei->trunk_diam,treei->base_height,trunccolor_uc);
        crown_height=treei->tree_height-treei->base_height;
        glTranslatef(0.0,0.0,treei->base_height);
        DrawCone(treei->trunk_diam,crown_height,trunccolor_uc);
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    glPopMatrix();


  }
  glPopMatrix();


  glDisable(GL_COLOR_MATERIAL);

}

/* ------------------ GetZCellVal ------------------------ */

float GetZCellVal(meshdata *meshi,float xval, float yval, float *zval_offset, int *loc){
  int imesh;
  int meshstart=-1;

  if(meshi==NULL)meshstart=0;
  if(zval_offset!=NULL)*zval_offset=0.0;
  for(imesh=meshstart;imesh<nmeshes;imesh++){
    meshdata *meshj;
    float *xplt, *yplt;
    int ibar, jbar;

    if(imesh==-1){
      meshj=meshi;
    }
    else{
      meshj=meshinfo+imesh;
      if(meshi==meshj)continue;
    }
    xplt = meshj->xplt_orig;
    yplt = meshj->yplt_orig;
    ibar = meshj->ibar;
    jbar = meshj->jbar;
    if(xplt[0]<=xval&&xval<=xplt[ibar]&&yplt[0]<=yval&&yval<=yplt[jbar]){
      float dx, dy;
      terraindata *terri;
      int ival, jval;
      int nxcell;
      float zval;
      float *zcell;
      float zval_return;

      dx = xplt[1]-xplt[0];
      dy = yplt[1]-yplt[0];
      ival = (xval-xplt[0])/dx;
      if(ival>=ibar)ival=ibar-1;
      jval = (yval-yplt[0])/dy;
      if(jval>=jbar)jval=jbar-1;
      terri=meshj->terrain;
      nxcell = terri->ibar;
      zcell = terri->zcell;
      zval = zcell[IJCELL2(ival,jval)];
      *loc=1;
      zval_return = zterrain_min+vertical_factor*(zval-zterrain_min);
      if(zval_offset!=NULL){
        *zval_offset=(vertical_factor-1.0)*(zval-zterrain_min);
      }
      return zval_return;
    }
  }
  *loc=0;
  return 0.0;
}

/* ------------------ GetZCellValOffset ------------------------ */

float GetZCellValOffset(meshdata *meshi,float xval, float yval, int *loc){
  int imesh;
  int meshstart=-1;

  if(meshi==NULL)meshstart=0;

  for(imesh=meshstart;imesh<nmeshes;imesh++){
    meshdata *meshj;
    float *xplt, *yplt;
    int ibar, jbar;

    if(imesh==-1){
      meshj=meshi;
    }
    else{
      meshj=meshinfo+imesh;
      if(meshi==meshj)continue;
    }

// convert xval and yval to "user" units
    xval = DENORMALIZE_X(xval);
    yval = DENORMALIZE_Y(yval);

    xplt = meshj->xplt_orig;
    yplt = meshj->yplt_orig;
    ibar = meshj->ibar;
    jbar = meshj->jbar;
    if(xplt[0]<=xval&&xval<=xplt[ibar]&&yplt[0]<=yval&&yval<=yplt[jbar]){
      terraindata *terri;
      int ival, jval;
      float dx, dy;
      int nxcell;
      float *zcell,zval;
      float zvaloffset;

      dx = xplt[1]-xplt[0];
      dy = yplt[1]-yplt[0];
      ival = (xval-xplt[0])/dx;
      if(ival>=ibar)ival=ibar-1;
      jval = (yval-yplt[0])/dy;
      if(jval>=jbar)jval=jbar-1;
      terri=meshj->terrain;
      nxcell = terri->ibar;
      zcell = terri->zcell;
      zval = zcell[IJCELL2(ival,jval)];
      *loc=1;
      zvaloffset = zterrain_min+vertical_factor*(zval-zterrain_min)-zval;

    // convert zoffset back to smokeview/scaled units

      zvaloffset = SCALE2SMV(zvaloffset);
      return zvaloffset;
    }
  }
  *loc=0;
  return 0.0;
}

/* ------------------ UpdateTerrainColors ------------------------ */

void UpdateTerrainColors(void){
  int i;

  for(i=0;i<MAXRGB;i++){
    float f1;

    f1 = (float)i/(float)(MAXRGB-1);
    rgbterrain[4*i  ]=((1.0-f1)*terrain_rgba_zmin[0] + f1*terrain_rgba_zmax[0])/255.0;
    rgbterrain[4*i+1]=((1.0-f1)*terrain_rgba_zmin[1] + f1*terrain_rgba_zmax[1])/255.0;
    rgbterrain[4*i+2]=((1.0-f1)*terrain_rgba_zmin[2] + f1*terrain_rgba_zmax[2])/255.0;
    rgbterrain[4*i+3]=1.0;
  }
}

/* ------------------ GetZTerrain ------------------------ */

float GetZTerrain(float x, float y){
  int loc;
  float zterrain;

  zterrain = GetZCellVal(NULL, x, y, NULL, &loc);
  return zterrain;
}

/* ------------------ ComputeTerrainNormalsManual ------------------------ */

void ComputeTerrainNormalsManual(void){
  int imesh;

  for(imesh=0;imesh<nmeshes;imesh++){
    meshdata *meshi;
    terraindata *terri;
    float *znode;
    int j;
    int nycell;

    meshi = meshinfo + imesh;
    terri = meshi->terrain;
    if(terri==NULL)continue;
    znode = terri->znode;
    nycell = meshi->jbar;

    for(j=0;j<=terri->jbar;j++){
      int i;
      float *yplt;

      yplt = terri->yplt+j;
      for(i=0;i<=terri->ibar;i++){
        float dzdx, dzdy, sum, znormal3[3];
        unsigned char *uc_znormal;
        float *xplt;

     //     i      j    k
     //     1      0    dzdx
     //     0      1    dzdy

     //     -dzdx -dzdy 1

        xplt = terri->xplt+i;
        if(i==0){
          dzdx = (znode[IJ2(i+1,j)]  -znode[IJ2(i,j)])/(xplt[1]-xplt[0]);
        }
        else if(i==terri->ibar){
          dzdx = (znode[IJ2(i, j)]-znode[IJ2(i-1, j)])/(xplt[0]-terri->xplt[-1]);
        }
        else{
          float dx1, dx3;
          float dz1, dz3;

          dx1 = (xplt[0]-xplt[-1]);
          dx3 = (xplt[1]-xplt[0]);
          dz1 = (znode[IJ2(i,j)]  -znode[IJ2(i-1,j)]);
          dz3 = (znode[IJ2(i+1, j)]-znode[IJ2(i, j)]);
          dzdx = ((dz1/dx1)*dx3+(dz3/dx3)*dx1)/(dx1+dx3);
        }

        if(j==0){
          dzdy = (znode[IJ2(i, j+1)]-znode[IJ2(i, j)])/(yplt[1]-yplt[0]);
        }
        else if(j==terri->jbar){
          dzdy = (znode[IJ2(i, j)]-znode[IJ2(i, j-1)])/(yplt[0]-yplt[-1]);
        }
        else{
          float dy1, dy3;
          float dz1, dz3;

          dy1 = (yplt[0]-yplt[-1]);
          dy3 = (yplt[1]-yplt[0]);
          dz1 = (znode[IJ2(i, j)]-znode[IJ2(i, j-1)]);
          dz3 = (znode[IJ2(i, j+1)]-znode[IJ2(i, j)]);
          dzdy = ((dz1/dy1)*dy3+(dz3/dy3)*dy1)/(dy1+dy3);
        }

        uc_znormal = terri->uc_znormal + IJ2(i,j);
        znormal3[0] = -dzdx;
        znormal3[1] = -dzdy;
        znormal3[2] = 1.0;

        sum  = znormal3[0]*znormal3[0];
        sum += znormal3[1]*znormal3[1];
        sum += 1.0;
        sum = sqrt(sum);
        znormal3[0]/=sum;
        znormal3[1]/=sum;
        znormal3[2]/=sum;
        *uc_znormal = GetNormalIndex(wui_sphereinfo, znormal3);
      }
    }
  }
}

/* ------------------ ComputeTerrainNormalsAuto ------------------------ */

void ComputeTerrainNormalsAuto(void){
  int imesh;
  float zmin, zmax;

  for(imesh=0;imesh<nmeshes;imesh++){
    meshdata *meshi;
    terraindata *terri;
    int j;
    float dx, dy;
    float *znode, *znode_offset;
    int nycell;
    unsigned char *uc_znormal;

    meshi = meshinfo + imesh;

    terri = meshi->terrain;

    dx = terri->xplt[1]-terri->xplt[0];
    dy = terri->yplt[1]-terri->yplt[0];

    znode = terri->znode;
    znode_offset = terri->znode_offset;
    nycell = terri->jbar;

    uc_znormal = terri->uc_znormal;
    for(j=0;j<=terri->jbar;j++){
      int i;
      float ynode;

      ynode = terri->yplt[j];

      for(i=0;i<=terri->ibar;i++){
        float xnode;
        int count, loc1, loc2, loc3, loc4;
        float val1, val2, val3, val4;
        float val1_offset, val2_offset, val3_offset, val4_offset;
        float valx1a, valx2a, valx3a, valx4a;
        float valx1b, valx2b, valx3b, valx4b;
        float valy1a, valy2a, valy3a, valy4a;
        float valy1b, valy2b, valy3b, valy4b;
        float zval, zval_offset;
        float zvalxa, zvalxb;
        float zvalya, zvalyb;
        float dxa, dxb, dya, dyb;
        float dzdx, dzdy;
        float sum;
        float znormal3[3];
        float denom;

        xnode = terri->xplt[i];

        val1 =  GetZCellVal(meshi,xnode-dx/2.0,ynode-dy/2.0,&val1_offset,&loc1);
        val2 =  GetZCellVal(meshi,xnode+dx/2.0,ynode-dy/2.0,&val2_offset,&loc2);
        val3 =  GetZCellVal(meshi,xnode+dx/2.0,ynode+dy/2.0,&val3_offset,&loc3);
        val4 =  GetZCellVal(meshi,xnode-dx/2.0,ynode+dy/2.0,&val4_offset,&loc4);
        count = loc1 + loc2 + loc3 + loc4;

        zval = (val1*loc1 + val2*loc2 + val3*loc3 + val4*loc4)/(float)MAX(1,count);

        znode[ijnode3(i,j)]=zval;
        zval_offset = SCALE2SMV((val1_offset*loc1 + val2_offset*loc2 + val3_offset*loc3 + val4_offset*loc4))/(float)MAX(1,count);

        *znode_offset++=zval_offset;

 // compute (f(x+dx,y) - f(x-dx,y))/(2*dx)

        valx1a =  GetZCellVal(meshi,xnode-dx-dx/2.0,ynode-dy/2.0,NULL,&loc1);
        valx2a =  GetZCellVal(meshi,xnode-dx+dx/2.0,ynode-dy/2.0,NULL,&loc2);
        valx3a =  GetZCellVal(meshi,xnode-dx+dx/2.0,ynode+dy/2.0,NULL,&loc3);
        valx4a =  GetZCellVal(meshi,xnode-dx-dx/2.0,ynode+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalxa = valx1a*loc1 + valx2a*loc2 + valx3a*loc3 + valx4a*loc4;
        if(count==0){
          zvalxa = zval;
          dxa = 0.0;
        }
        else{
          zvalxa /= (float)count;
          dxa = dx;
        }
        valx1b =  GetZCellVal(meshi,xnode+dx-dx/2.0,ynode-dy/2.0,NULL,&loc1);
        valx2b =  GetZCellVal(meshi,xnode+dx+dx/2.0,ynode-dy/2.0,NULL,&loc2);
        valx3b =  GetZCellVal(meshi,xnode+dx+dx/2.0,ynode+dy/2.0,NULL,&loc3);
        valx4b =  GetZCellVal(meshi,xnode+dx-dx/2.0,ynode+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalxb = valx1b*loc1 + valx2b*loc2 + valx3b*loc3 + valx4b*loc4;
        if(count==0){
          zvalxb = zval;
          dxb = 0.0;
        }
        else{
          zvalxb /= (float)count;
          dxb = dx;
        }
        denom = dxa+dxb;
        if(denom==0.0){
          dzdx=1.0;
        }
        else{
          dzdx = (zvalxb - zvalxa)/denom;
        }

 // compute (f(x,y+dy) - f(x,y-dy))/(2*dy)

        valy1a =  GetZCellVal(meshi,xnode-dx/2.0,ynode-dy-dy/2.0,NULL,&loc1);
        valy2a =  GetZCellVal(meshi,xnode+dx/2.0,ynode-dy-dy/2.0,NULL,&loc2);
        valy3a =  GetZCellVal(meshi,xnode+dx/2.0,ynode-dy+dy/2.0,NULL,&loc3);
        valy4a =  GetZCellVal(meshi,xnode-dx/2.0,ynode-dy+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalya = valy1a*loc1 + valy2a*loc2 + valy3a*loc3 + valy4a*loc4;
        if(count==0){
          zvalya = zval;
          dya = 0.0;
        }
        else{
          zvalya /= (float)count;
          dya = dy;
        }
        valy1b =  GetZCellVal(meshi,xnode-dx/2.0,ynode+dy-dy/2.0,NULL,&loc1);
        valy2b =  GetZCellVal(meshi,xnode+dx/2.0,ynode+dy-dy/2.0,NULL,&loc2);
        valy3b =  GetZCellVal(meshi,xnode+dx/2.0,ynode+dy+dy/2.0,NULL,&loc3);
        valy4b =  GetZCellVal(meshi,xnode-dx/2.0,ynode+dy+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalyb = valy1b*loc1 + valy2b*loc2 + valy3b*loc3 + valy4b*loc4;
        if(count==0){
          zvalyb = zval;
          dyb = 0.0;
        }
        else{
          zvalyb /= (float)count;
          dyb = dy;
        }
        denom = dya + dyb;
        if(denom==0.0){
          dzdy=1.0;
        }
        else{
          dzdy = (zvalyb - zvalya)/denom;
        }

     //     i  j  k
     //     1  0 dzdx
     //     0  1 dzdy

     //     -dzdx -dzdy 1

        //znormal = terri->znormal + 3*ijnode2(i,j);
        uc_znormal = terri->uc_znormal + ijnode3(i,j);
        znormal3[0] = -dzdx;
        znormal3[1] = -dzdy;
        znormal3[2] = 1.0;

        sum  = znormal3[0]*znormal3[0];
        sum += znormal3[1]*znormal3[1];
        sum += znormal3[2]*znormal3[2];
        sum = sqrt(sum);
        znormal3[0]/=sum;
        znormal3[1]/=sum;
        znormal3[2]/=sum;
        *uc_znormal = GetNormalIndex(wui_sphereinfo, znormal3);
      }
    }
  }

  zmin = meshinfo->terrain->znode[0];
  zmax = zmin;
  for(imesh=0;imesh<nmeshes;imesh++){
    meshdata *meshi;
    terraindata *terri;
    int i;

    meshi = meshinfo + imesh;
    terri = meshi->terrain;

    for(i=0;i<(terri->ibar+1)*(terri->jbar+1);i++){
      float *znode;

      znode = terri->znode+i;
      zmin = MIN(zmin,*znode);
      zmax = MAX(zmax,*znode);
    }
  }
}

/* ------------------ InitTerrainZNode ------------------------ */

void InitTerrainZNode(meshdata *meshi, terraindata *terri, float xmin, float xmax, int nx, float ymin, float ymax, int ny,
                       int allocate_memory){
  float dx, dy;
  float *x, *y, *z;
  int nxcell;
  int i,j;
  int ij;

  if(meshi!=NULL){
    meshi->terrain=terri;
  }

  if(terri==NULL)return;
  if(allocate_memory==1){
    terri->xplt=NULL;
    terri->yplt=NULL;
    terri->zcell=NULL;
    terri->znode=NULL;
    terri->uc_znormal=NULL;
    terri->ter_texture=NULL;
  }

  terri->xmin=xmin;
  terri->xmax=xmax;
  terri->ymin=ymin;
  terri->ymax=ymax;
  terri->jbar=ny;
  nx=ABS(nx);
  terri->ibar=nx;

  if(allocate_memory==1){
    NewMemory((void **)&terri->xplt,(nx+1)*sizeof(float));
    NewMemory((void **)&terri->yplt,(ny+1)*sizeof(float));
    NewMemory((void **)&terri->zcell,nx*ny*sizeof(float));
    NewMemory((void **)&terri->znode,(nx+1)*(ny+1)*sizeof(float));
    NewMemory((void **)&terri->znode_offset,(nx+1)*(ny+1)*sizeof(float));
    NewMemory((void **)&terri->znode_scaled,(nx+1)*(ny+1)*sizeof(float));
    NewMemory((void **)&terri->uc_znormal,(nx+1)*(ny+1)*sizeof(unsigned char));
  }

  x = terri->xplt;
  y = terri->yplt;
  dx = (xmax-xmin)/nx;
  dy = (ymax-ymin)/ny;
  for(i=0;i<nx;i++){
    x[i] = xmin + dx*i;
  }
  x[nx] = xmax;

  for(i=0;i<ny;i++){
    y[i] = ymin + dy*i;
  }
  y[ny] = ymax;

  z=terri->zcell;

  nxcell = nx;
  if(meshi->zcell!=NULL){
    for(j=0;j<ny;j++){
      for(i=0;i<nx;i++){
        float zval;

        ij = IJCELL2(i,j);
        zval=meshi->zcell[ij];
        zterrain_min = MIN(zval,zterrain_min);
        zterrain_max = MAX(zval,zterrain_max);
        z[ij]=zval;
      }
    }
  }
  GetTerrainData(terri->file, terri);
}

/* ------------------ DrawTerrain ------------------------ */

void DrawTerrain(terraindata *terri){
  float *znode, *zn;
  unsigned char *uc_znormal;
  int nycell;
  int i, j;
  float *x, *y;
  float terrain_color[4];
  float terrain_shininess=100.0;
  float terrain_specular[4]={0.8,0.8,0.8,1.0};

#define ZOFFSET 0.001

  if(terri->terrain_mesh->is_bottom==0)return;

  terrain_color[0]=0.47843;
  terrain_color[1]=0.45882;
  terrain_color[2]=0.18824;
  terrain_color[3]=1.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&terrain_shininess);
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,rgbterrain);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,terrain_specular);
  glEnable(GL_COLOR_MATERIAL);

  glBegin(GL_QUADS);
  uc_znormal = terri->uc_znormal;
  znode = terri->znode;
  nycell = terri->jbar;
  x = terri->xplt;
  y = terri->yplt;
  glColor4fv(terrain_color);
  for(j=0;j<terri->jbar;j++){
    int jp1;

    jp1 = j + 1;

    for(i=0;i<terri->ibar;i++){
      unsigned char *uc_zn;
      int ip1;
      float zval;

      ip1 = i + 1;
      uc_zn = uc_znormal+ijnode3(i,j);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));

      glNormal3fv(zn);
      zval = znode[ijnode3(i,j)]+ZOFFSET;
      glVertex3f(x[i],y[j],zval);

      uc_zn = uc_znormal+ijnode3(ip1,j);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      zval = znode[ijnode3(ip1,j)]+ZOFFSET;
      glVertex3f(x[i+1],y[j],zval);

      uc_zn = uc_znormal+ijnode3(ip1,jp1);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      zval = znode[ijnode3(ip1,jp1)]+ZOFFSET;
      glVertex3f(x[i+1],y[j+1],zval);

      uc_zn = uc_znormal+ijnode3(i,jp1);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      zval = znode[ijnode3(i,jp1)]+ZOFFSET;
      glVertex3f(x[i],y[j+1],zval);
    }
  }
  glEnd();

  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;

  if(show_terrain_normals==1||show_terrain_grid==1){
    glBegin(GL_LINES);
    uc_znormal = terri->uc_znormal;
    znode = terri->znode;
    nycell = terri->jbar;
    x = terri->xplt;
    y = terri->yplt;
    glColor3f(0.0, 0.0, 0.0);
    if(show_terrain_grid==1){
      for(j = 0; j<terri->jbar; j++){
        for(i = 0; i<terri->ibar; i++){
          float zval11, zval13, zval33, zval31;

          zval11 = znode[ijnode3(i,     j)]+ZOFFSET;
          zval31 = znode[ijnode3(i+1,   j)]+ZOFFSET;
          zval33 = znode[ijnode3(i+1, j+1)]+ZOFFSET;
          zval13 = znode[ijnode3(i,   j+1)]+ZOFFSET;

          glVertex3f(x[i],   y[j],   zval11);
          glVertex3f(x[i+1], y[j],   zval31);

          glVertex3f(x[i+1], y[j],   zval31);
          glVertex3f(x[i+1], y[j+1], zval33);

          glVertex3f(x[i+1], y[j+1], zval33);
          glVertex3f(x[i],   y[j+1], zval13);

          glVertex3f(x[i],   y[j+1], zval13);
          glVertex3f(x[i],   y[j],   zval11);
        }
      }
    }
    if(show_terrain_normals==1){
      for(j = 0; j<terri->jbar; j+=MAX(ABS(terrain_normal_skip),1)){
        for(i = 0; i<terri->ibar; i+= MAX(ABS(terrain_normal_skip), 1)){
          unsigned char *uc_zn;
          float zval11;

          zval11 = znode[ijnode3(i,     j)]+ZOFFSET;

          uc_zn = uc_znormal+ijnode3(i, j);
          zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));

          glVertex3f(x[i], y[j], zval11);
          glVertex3f(x[i]  +terrain_normal_length*zn[0],
                     y[j]  +terrain_normal_length*zn[1],
                     zval11+terrain_normal_length*zn[2]);
        }
      }
    }
    glEnd();
  }
  glPopMatrix();

}

/* ------------------ DrawTerrainTexture ------------------------ */

void DrawTerrainTexture(terraindata *terri){
  float *znode;
  unsigned char *uc_znormal, *uc_zn;
  int nxcell,nycell;
  int i, j;
  float *x, *y;
  float terrain_color[4];

  if(terri->terrain_mesh->is_bottom==0)return;

  terrain_color[0]=1.0;
  terrain_color[1]=1.0;
  terrain_color[2]=1.0;
  terrain_color[3]=1.0;

  glPushMatrix();
  glScalef(SCALE2SMV(mscale[0]),SCALE2SMV(mscale[1]),vertical_factor*SCALE2SMV(mscale[2]));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&block_shininess);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,terrain_texture->name);

  glEnable(GL_COLOR_MATERIAL);
  glColor4fv(terrain_color);
  glBegin(GL_QUADS);
  uc_znormal = terri->uc_znormal;
  znode = terri->znode;
  nxcell = terri->ibar;
  nycell = terri->jbar;
  x = terri->xplt;
  y = terri->yplt;
  for(j=0;j<terri->jbar;j++){
    int jp1;
    float ty,typ1;

    jp1 = j + 1;
    ty = (y[j]-ybar0ORIG)/(ybarORIG-ybar0ORIG);
    typ1 = (y[j+1]-ybar0ORIG)/(ybarORIG-ybar0ORIG);

    for(i=0;i<terri->ibar;i++){
      float *zn;
      int ip1;
      float tx,txp1;

      ip1 = i + 1;
      tx = (x[i]-xbar0ORIG)/(xbarORIG-xbar0ORIG);
      txp1 = (x[i+1]-xbar0ORIG)/(xbarORIG-xbar0ORIG);

      uc_zn = uc_znormal+ijnode2(i,j);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      glTexCoord2f(tx,ty);
      glVertex3f(x[i],y[j],znode[ijnode3(i,j)]);

      uc_zn = uc_znormal+ijnode2(ip1,j);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      glTexCoord2f(txp1,ty);
      glVertex3f(x[i+1],y[j],znode[ijnode3(ip1,j)]);

      uc_zn = uc_znormal+ijnode2(ip1,jp1);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      glTexCoord2f(txp1,typ1);
      glVertex3f(x[i+1],y[j+1],znode[ijnode3(ip1,jp1)]);

      uc_zn = uc_znormal+ijnode2(i,jp1);
      zn = GetNormalVectorPtr(wui_sphereinfo, (unsigned int)(*uc_zn));
      glNormal3fv(zn);
      glTexCoord2f(tx,typ1);
      glVertex3f(x[i],y[j+1],znode[ijnode3(i,jp1)]);
    }
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;

  glPopMatrix();

}

/* ------------------ GetTerrainData ------------------------ */

int GetTerrainData(char *file, terraindata *terri){
  FILE *WUIFILE;
  float zmin_cutoff;
  int ibp1, jbp1, ijbar[2];
  float *xplt, *yplt, *z_terrain;
  int returncode=1;
  int nvalues,i;
#ifdef pp_DEBUG_TERRAIN
  FILE *stream = NULL;
  char fileout[255];
#endif

#ifdef _DEBUG
  printf("reading terrain data mesh: %i\n",(int)(terri-terraininfo));
#endif
  WUIFILE = fopen(file, "rb");
  if(WUIFILE == NULL)return 1;

//    WRITE(LU_TERRAIN(NM)) REAL(M%ZS-1._EB, FB)
//    WRITE(LU_TERRAIN(NM)) IBP1, JBP1
//    WRITE(LU_TERRAIN(NM)) (M%XPLT(I), I = 0, IBAR)
//    WRITE(LU_TERRAIN(NM)) (M%YPLT(J), J = 0, JBAR)
//    WRITE(LU_TERRAIN(NM)) Z_TERRAIN

  FORTWUIREAD(&zmin_cutoff, 1);
  zmin_cutoff -= 0.1;
  terri->zmin_cutoff = zmin_cutoff;
  FORTWUIREAD(ijbar, 2);
  ibp1 = ijbar[0];
  jbp1 = ijbar[1];

  NewMemory((void **)&xplt, ibp1*sizeof(float));
  NewMemory((void **)&yplt, jbp1*sizeof(float));
  terri->xplt = xplt;
  terri->yplt = yplt;
  FORTWUIREAD(xplt, ibp1);
  FORTWUIREAD(yplt, jbp1);

  if(terri->znode==NULL){
    NewMemory((void **)&z_terrain, ibp1*jbp1*sizeof(float));
    terri->znode = z_terrain;
  }
  else{
    z_terrain = terri->znode;
  }
  FORTWUIREAD(z_terrain, ibp1*jbp1);

  for(i = 0, nvalues=0; i<ibp1*jbp1; i++){
    if(z_terrain[i]>zmin_cutoff)nvalues++;
  }
#ifdef pp_DEBUG_TERRAIN
  strcpy(fileout, file);
  strcat(fileout, ".csv");
  stream = fopen(fileout, "w");
  if(stream!=NULL){
    int j;

    fprintf(stream, "%s\n", file);
    fprintf(stream, " ibar+1,jbar+1,xmin,xmax,ymin,ymax,z cutoff\n");
    fprintf(stream, " %i,%i,%f,%f,%f,%f,%f\n", ibp1, jbp1,xplt[0], xplt[ibp1-1], yplt[0], yplt[jbp1-1],zmin_cutoff);
    fprintf(stream, "\nelevations\n");
    for(j = jbp1-1; j>=0; j--){
      for(i = 0; i<ibp1-1; i++){
        fprintf(stream, " %f,", z_terrain[i*jbp1+j]);
      }
      i = ibp1-1;
      fprintf(stream, " %f\n", z_terrain[i*jbp1+j]);
    }
    fclose(stream);
  }
#endif
  terri->nvalues = nvalues;
  if(returncode!=0)returncode=0;
  fclose(WUIFILE);
  return returncode;
}

/* ------------------ GetTerrainSize ------------------------ */

int GetTerrainSize(char *file, float *xmin, float *xmax, int *nx, float *ymin, float *ymax, int *ny, int *times_local){
  FILE *WUIFILE;
  int one;
  float xyminmax[4];
  int nxy[2];
  size_t returncode;
  int version;
  float time_local;
  int nchanges;
  int nt = 0;

  WUIFILE = fopen(file, "rb");
  if(WUIFILE == NULL)return 1;

  FSEEK(WUIFILE, 4, SEEK_CUR);fread(&one, 4, 1, WUIFILE);FSEEK(WUIFILE, 4, SEEK_CUR);

  FORTWUIREAD(&version, 1);
  FORTWUIREAD(xyminmax, 4);
  *xmin = xyminmax[0];
  *xmax = xyminmax[1];
  *ymin = xyminmax[2];
  *ymax = xyminmax[3];

  FORTWUIREAD(nxy, 2);
  *nx = nxy[0];
  *ny = nxy[1];

  FSEEK(WUIFILE, 16 + 5 * (*nx)*(*ny), SEEK_CUR); // skip over zelev and state

  for(;;){

    FORTWUIREAD(&time_local, 1);
    if(returncode == 0)break;

    FORTWUIREAD(&nchanges, 1);
    if(returncode == 0)break;

    if(nchanges > 0)FSEEK(WUIFILE, 16 + 5 * nchanges, SEEK_CUR);

    nt++;

  }
  *times_local = nt;

  fclose(WUIFILE);

  return 0;
}

/* ------------------ UpdateTerrain ------------------------ */

void UpdateTerrain(int allocate_memory, float vertical_factor_local){
  if(auto_terrain==1||manual_terrain==1){
    int i;

    if(manual_terrain==0){
      nterraininfo = nmeshes;
      if(allocate_memory==1&&manual_terrain==0){
        NewMemory((void **)&terraininfo, nterraininfo*sizeof(terraindata));
      }
    }

    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      terraindata *terri;
      float xmin, xmax, ymin, ymax;
      int nx, ny;

      meshi=meshinfo + i;
      if(manual_terrain==1){
        terri = meshi->terrain;
      }
      else{
        terri = terraininfo + i;
      }

      nx = meshi->ibar;
      ny = meshi->jbar;
      xmin = meshi->xplt_orig[0];
      xmax = meshi->xplt_orig[nx];
      ymin = meshi->yplt_orig[0];
      ymax = meshi->yplt_orig[ny];

      InitTerrainZNode(meshi, terri, xmin, xmax, nx, ymin, ymax, ny, allocate_memory);
    }
    if(manual_terrain==0){ // slow
      ComputeTerrainNormalsAuto();
    }
    if(manual_terrain==1){
      ComputeTerrainNormalsManual();
    }
  }
  if(nterraininfo>0){
    int imesh;

    for(imesh=0;imesh<nmeshes;imesh++){
      meshdata *meshi;
      terraindata *terri;
      float *znode, *znode_scaled;
      int i, j;

      meshi=meshinfo + imesh;
      terri = meshi->terrain;
      if(terri==NULL)continue;
      terri->terrain_mesh = meshi;
      znode = terri->znode;
      znode_scaled = terri->znode_scaled;

      for(j=0;j<=terri->jbar;j++){
        for(i=0;i<=terri->ibar;i++){
          *znode_scaled = NORMALIZE_Z(*znode);
          znode++;
          znode_scaled++;
        }
      }
    }
  }
}

/* ------------------ HaveTerrainSlice ------------------------ */

int HaveTerrainSlice(void){
  int i;

  for(i=0;i<nsliceinfo;i++){
    slicedata *slicei;

    slicei = sliceinfo + i;

    if(slicei->loaded==1&&slicei->slice_filetype==SLICE_TERRAIN)return 1;

  }
  return 0;
}

/* ------------------ UpdateTerrainOptions ------------------------ */

void UpdateTerrainOptions(void){
  if(nterraininfo>0||auto_terrain==1){
    visOpenVents=0;
    visDummyVents=0;
    visFrame=0;
    updatemenu=1;
  }
}

/* ------------------ GetMeshZCell ------------------------ */

float GetMeshZCell(meshdata *meshi, float xval, float yval, int *valid){
  float *xplt, *yplt, *zcell;
  float dx, dy;
  int ibar, jbar;
  int ival, jval;
  float zval;
  int nxcell;

  *valid = 0;
  if(meshi==NULL)return 0.0;
  xplt = meshi->xplt_orig;
  yplt = meshi->yplt_orig;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  nxcell = ibar;
  if(xval<xplt[0]||xval>xplt[ibar])return 0.0;
  if(yval<yplt[0]||yval>yplt[jbar])return 0.0;

  dx = xplt[1]-xplt[0];
  dy = yplt[1]-yplt[0];
  ival = (xval-xplt[0])/dx;
  if(ival>=ibar)ival = ibar-1;
  jval = (yval-yplt[0])/dy;
  if(jval>=jbar)jval = jbar-1;
  zcell = meshi->zcell;
  zval = zcell[IJCELL2(ival, jval)];
  *valid = 1;
  return zval;
}

/* ------------------ UpdateMeshTerrain ------------------------ */

void UpdateMeshTerrain(void){
  int i;

  if(nterraininfo<=0)return;
  for(i = 0; i<nmeshes; i++){
    int j;
    meshdata *meshi;

    meshi = meshinfo+i;

    for(j = 0; j<nmeshes; j++){
      meshdata *meshj;

      meshj = meshinfo+j;
      if(meshi==meshj||meshj->above!=NULL)continue;

#define MUP 5
      if(MeshConnect(meshi, MUP, meshj)==1){
        meshi->above = meshj;
        break;
      }
    }
  }
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    meshdata *meshj=NULL;
    int ii, jj;
    float *x, *y, *z;
    float xyz[3];

    meshi = meshinfo + i;

    meshi->is_bottom=1;

    x = meshi->xplt_orig;
    y = meshi->yplt_orig;
    z = meshi->zplt_orig;

    xyz[2] = z[0] - (z[1]-z[0])/2.0;

    for(ii=0;ii<meshi->ibar;ii++){
      xyz[0]=x[ii];
      for(jj=0;jj<meshi->jbar;jj++){
        xyz[1]=y[jj];
        meshj = GetMesh(xyz,meshj);
        if(meshj==NULL||meshj==meshi)continue;
        meshi->is_bottom=0;
        break;
      }
      if(meshi->is_bottom==0)break;
    }
  }

  // compute z level above bottom mesh

  for(i=0;i<nmeshes;i++){ // xxslow
    meshdata *meshi;
    int ii;
    float xyz[3], *x, *y;
    float *zcell;
    int nxcell;

    meshi = meshinfo + i;
    if(meshi->is_bottom==0)continue;
    x = meshi->xplt_orig;
    y = meshi->yplt_orig;
    nxcell = meshi->ibar;
    zcell = meshi->zcell;
    for(ii=0;ii<meshi->ibar;ii++){
      int jj;

      xyz[0]=x[ii];
      for(jj=0;jj<meshi->jbar;jj++){
        int ij, valid;
        float zz;

        ij = IJCELL2(ii, jj);
        xyz[1]=y[jj];
        zz= GetMeshZCell(meshi->above, xyz[0],xyz[1], &valid);
        if(valid==1&&zz>zcell[ij])zcell[ij]=zz;
      }
    }
  }
}
