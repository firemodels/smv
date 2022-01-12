#include "options.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOscript.h"

/* ------------------ UpdateTimeLabels ------------------------ */

void UpdateTimeLabels(void){
  float time0;

  time0 = timeoffset;
  if(global_times!=NULL)time0 = timeoffset+global_times[itimes];
  if(current_script_command!=NULL&&current_script_command->command==SCRIPT_LOADSLICERENDER){
    time0 = current_script_command->fval4;
  }

  if(vishmsTimelabel==1){
    int hour, min, sec, sec10;
    char sign[2];

    if(time0<0){
      strcpy(sign, "-");
      time0 = ABS(time0);
    }
    else{
      strcpy(sign, " ");
    }
    hour = time0/3600;
    min = (int)(time0/60.0-60.0*hour);
    sec10 = (int)(10*(time0-60.0*min-3600.0*hour));
    sec = sec10/10;
    sec10 = sec10-10*sec;
    sprintf(timelabel, "  %s%i:%.2i:%.2i.%i", sign, hour, min, sec, sec10);
  }
  else{
    float dt;
    char timeval[30], *timevalptr;

    if(current_script_command!=NULL&&current_script_command->command==SCRIPT_LOADSLICERENDER){
      dt = current_script_command->fval5;
    }
    else{
      if(nglobal_times>1){
        dt = global_times[1]-global_times[0];
      }
      else{
        dt = 0.0;
      }
    }
    if(dt<0.0)dt = -dt;
    timevalptr = Time2TimeLabel(time0, dt, timeval, force_fixedpoint);
    strcpy(timelabel, "");
    if(visFrameTimelabel==1)strcat(timelabel, "Time: ");
    strcat(timelabel, timevalptr);
  }

  {
    int itime_val;

    if(current_script_command!=NULL&&current_script_command->command==SCRIPT_LOADSLICERENDER){
      itime_val = script_itime;
    }
    else{
      itime_val = itimes;
    }

    if(visFrameTimelabel==1){
      sprintf(framelabel, "Frame: %i", itime_val);
    }
    else{
      sprintf(framelabel, "%i", itime_val);
    }
  }

  if(hrrinfo!=NULL&&hrrinfo->hrrval!=NULL&&hrrinfo->display==1&&hrrinfo->loaded==1){
    float hrr;

    hrr = hrrinfo->hrrval[hrrinfo->itime];
    if(hrr<1.0){
      sprintf(hrrinfo->hrrlabel,"HRR: %4.1f W",hrr*1000.0);
    }
    else if(hrr>1000.0){
      sprintf(hrrinfo->hrrlabel,"HRR: %4.1f MW",hrr/1000.0);
    }
    else{
      sprintf(hrrinfo->hrrlabel,"HRR: %4.1f kW",hrr);
    }
  }
}

/* ------------------ DrawTimebar ------------------------ */

void DrawTimebar(float xleft, float xright, float ybot, float ytop){
  float xxright;

  if(xright<=xleft)return;
  DISABLE_LIGHTING;

  glLineWidth(linewidth);
  glBegin(GL_LINE_LOOP);
  glColor4fv(timebarcolor);
  glVertex2f(xleft,ybot);
  glVertex2f(xright,ybot);
  glVertex2f(xright,ytop);
  glVertex2f(xleft,ytop);
  glEnd();

  xxright = xright;
  if(current_script_command!=NULL&&current_script_command->command==SCRIPT_LOADSLICERENDER){
    float factor, time_min, time_max, time_now;

    time_min = current_script_command->fval2;
    time_max = current_script_command->fval3;
    if(time_max>time_min){
      time_now = current_script_command->fval4;
      factor = CLAMP((time_now-time_min)/(time_max-time_min), 0.0, 1.0);
      xxright = (1.0-factor)*xleft+factor*xright;
    }
  }
  else{
// draw time bar proportional to time
    float factor=0.0, dtime;

    dtime = global_times[nglobal_times-1] - global_times[0];
    if(dtime!=0.0&&nglobal_times>1){
      factor = CLAMP((global_times[itimes] - global_times[0])/dtime, 0.0, 1.0);
    }
    xxright = xleft*(1.0-factor) + xright*factor;
  }
  glBegin(GL_POLYGON);
  glColor4fv(timebarcolor);
  glVertex2f(xleft,ybot);
  glVertex2f(xxright,ybot);
  glVertex2f(xxright,ytop);
  glVertex2f(xleft,ytop);
  glEnd();
}

/* ------------------ AddColorbar ------------------------ */

void AddColorbar(int icolorbar){
  colorbardata *cb_to, *cb_from;

  ncolorbars++;
  CheckMemory;
  ResizeMemory((void **)&colorbarinfo,ncolorbars*sizeof(colorbardata));
  UpdateCurrentColorbar(colorbarinfo + colorbartype);

  cb_from = colorbarinfo + icolorbar;
  CheckMemory;

      // new colorbar

  cb_to=colorbarinfo+ncolorbars-1;

  memcpy(cb_to,cb_from,sizeof(colorbardata));
  strcpy(cb_to->label,"Copy of ");
  strcat(cb_to->label,cb_from->label);
  cb_to->label_ptr=cb_to->label;

  RemapColorbar(cb_to);

}

/* ------------------ DrawSelectColorbar ------------------------ */

void DrawSelectColorbar(void){
  int i;
  colorbardata *cbi;

  if(show_firecolormap==0){
    cbi = colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbarinfo+fire_colorbar_index;
  }

  glPointSize(20.0f);
  glBegin(GL_POINTS);
  for(i=0;i<cbi->nnodes;i++){
    unsigned char *rrgb, r, g, b;

    GetRGB(i+1, &r, &g, &b);
    glColor3ub(r, g, b);

    rrgb=cbi->rgb_node+3*i;
    glVertex3f(rrgb[0]/255.0,rrgb[1]/255.0,rrgb[2]/255.0);
  }
  glEnd();
}


/* ------------------ DrawColorbarPath ------------------------ */

void DrawColorbarPath(void){
  int i;
  colorbardata *cbi;
  int ncolors;

  if(show_firecolormap==0){
    cbi = colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbarinfo+fire_colorbar_index;
  }
  glPointSize(5.0);
  glBegin(GL_POINTS);
  for(i=0;i<255;i++){
    float *rgbi;

    rgbi=cbi->colorbar+3*i;
    glColor3fv(rgbi);
    glVertex3fv(rgbi);
  }
  glEnd();

  glPointSize(10.0);
  glBegin(GL_POINTS);
  for(i=0;i<cbi->nnodes;i++){
    unsigned char *rrgb;

    rrgb=cbi->rgb_node+3*i;
    glColor3ubv(rrgb);
    glVertex3f(rrgb[0]/255.0,rrgb[1]/255.0,rrgb[2]/255.0);
  }
#define PLEFT2 -0.1
#define PRIGHT2 1.1

  glEnd();

  // draw rgb color axese

  glLineWidth(5.0);
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex3f( PLEFT2,PLEFT2,PLEFT2);
  glVertex3f(PRIGHT2,PLEFT2,PLEFT2);

  glColor3f(0.0,1.0,0.0);
  glVertex3f(PLEFT2, PLEFT2,PLEFT2);
  glVertex3f(PLEFT2,PRIGHT2,PLEFT2);

  glColor3f(0.0,0.0,1.0);
  glVertex3f(PLEFT2,PLEFT2, PLEFT2);
  glVertex3f(PLEFT2,PLEFT2,PRIGHT2);

  glEnd();

  if(colorbarpoint>=0&&colorbarpoint<cbi->nnodes){
    unsigned char *rgbleft;

    rgbleft = cbi->rgb_node+3*colorbarpoint;

    glPointSize(20.0);
    glBegin(GL_POINTS);
    glColor3ubv(rgbleft);
    glVertex3f(rgbleft[0]/255.0,rgbleft[1]/255.0,rgbleft[2]/255.0);
    glEnd();
    if(show_colorbar_hint==1){
      float xyz[3];

      xyz[0] = rgbleft[0] / 255.0 + 0.1;
      xyz[1] = rgbleft[1] / 255.0 + 0.1;
      xyz[2] = rgbleft[2] / 255.0 + 0.1;
      Output3Text(foregroundcolor, xyz[0], xyz[1], xyz[2], "click and drag to change colorbar node");
    }
  }

  {
    float xdenorm, ydenorm, zdenorm;

    glPointSize(10.0);
    glBegin(GL_POINTS);
    for(i=0;i<cbi->nnodes;i++){
      float *rgbi;
      float dzpoint;

      rgbi = cbi->colorbar+3*cbi->index_node[i];
      dzpoint = (float)cbi->index_node[i]/255.0;
      glColor3fv(rgbi);
      glVertex3f(1.5,0.0,dzpoint);
    }
    glEnd();

    xdenorm = DENORMALIZE_X(1.55);
    ydenorm = DENORMALIZE_Y(0.0);
    if(fontindex==SCALED_FONT)ScaleFont3D();
    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
    glTranslatef(-xbar0,-ybar0,-zbar0);
    for(i=0;i<cbi->nnodes;i++){
      char cbuff[1024];
      float dzpoint;

      dzpoint = (float)cbi->index_node[i]/255.0;
      zdenorm = DENORMALIZE_Z(dzpoint);
      sprintf(cbuff,"%i",(int)cbi->index_node[i]);
      Output3Text(foregroundcolor, xdenorm,ydenorm,zdenorm, cbuff);
    }
    glPopMatrix();
    glLineWidth(5.0);
    if(colorbarpoint>=0&&colorbarpoint<cbi->nnodes){
      float *rgbi;
      float dzpoint;

      glPointSize(20.0);
      glBegin(GL_POINTS);
      rgbi = cbi->colorbar+3*cbi->index_node[colorbarpoint];
      dzpoint = (float)cbi->index_node[colorbarpoint]/255.0;
      glColor3fv(rgbi);
      glVertex3f(1.5,0.0,dzpoint);
      glEnd();
    }
    if(show_firecolormap!=0){
      char vvlabel[255];
      float vval_min, vval_cutoff, vval_max;

      if(have_fire==HRRPUV&&smoke_render_option==RENDER_SLICE){
        vval_min=global_hrrpuv_min;
        vval_cutoff=global_hrrpuv_cutoff;
        vval_max=global_hrrpuv_max;
      }
      else{
        vval_min=global_temp_min;
        vval_cutoff= global_temp_cutoff;
        vval_max= global_temp_max;
      }
      sprintf(vvlabel,"%4.0f",vval_min);
      Output3Text(foregroundcolor, 1.0,0.0,0.0,vvlabel);

      sprintf(vvlabel,"%4.0f",vval_cutoff);
      Output3Text(foregroundcolor, 1.0,0.0,(vval_cutoff-vval_min)/(vval_max-vval_min),vvlabel);

      sprintf(vvlabel,"%4.0f",vval_max);
      Output3Text(foregroundcolor, 1.0,0.0,1.0,vvlabel);
    }

    if(show_firecolormap!=0){
      ncolors=MAXSMOKERGB-1;
    }
    else{
      ncolors=MAXRGB-1;
    }
    glBegin(GL_TRIANGLES);
    for(i=1;i<ncolors;i++){
      float *rgbi;
      float zbot, ztop;

      if(show_firecolormap!=0){
        rgbi=rgb_volsmokecolormap+4*i;
      }
      else{
        rgbi=cbi->colorbar+3*i;
      }
      glColor3fv(rgbi);
      zbot=(float)i/(float)ncolors;
      ztop=(float)(i+1)/(float)ncolors;

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.3,0.0,zbot);
      glVertex3f(1.3,0.0,ztop);

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.3,0.0,ztop);
      glVertex3f(1.3,0.0,zbot);

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.3,0.0,ztop);
      glVertex3f(1.1,0.0,ztop);

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.1,0.0,ztop);
      glVertex3f(1.3,0.0,ztop);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2, 0.1,zbot);
      glVertex3f(1.2, 0.1,ztop);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2, 0.1,ztop);
      glVertex3f(1.2, 0.1,zbot);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2, 0.1,ztop);
      glVertex3f(1.2,-0.1,ztop);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2,-0.1,ztop);
      glVertex3f(1.2, 0.1,ztop);
    }
    glEnd();
  }
}

/* ------------------ GetColorbar ------------------------ */

colorbardata *GetColorbar(char *label){
  int i;

  for(i=0;i<ncolorbars;i++){
    colorbardata *cb;

    cb = colorbarinfo + i;
    if(strcmp(cb->label,label)==0)return cb;
  }
  return NULL;
}

/* ------------------ UpdateCurrentColorbar ------------------------ */
#define FILE_UPDATE 6
void UpdateCurrentColorbar(colorbardata *cb){
  int jj=0,fed_loaded=0;

  current_colorbar = cb;
  if(current_colorbar != NULL&&strcmp(current_colorbar->label, "FED") == 0){
    is_fed_colorbar = 1;
  }
  else{
    is_fed_colorbar = 0;
  }
  for(jj=0;jj<nslice_loaded;jj++){
    slicedata *slicej;
    int j;

    j = slice_loaded_list[jj];
    slicej = sliceinfo + j;
    if(slicej->display==0)continue;
    if(slicej->is_fed==1){
      fed_loaded=1;
      break;
    }
  }
  if(is_fed_colorbar==1&&fed_loaded==1)SliceBoundCB(FILE_UPDATE);
}

/* ------------------ RemapColorbar ------------------------ */

void RemapColorbar(colorbardata *cbi){
  int i;
  float *colorbar;
  unsigned char *rgb_node;
  unsigned char *alpha;

  CheckMemory;
  colorbar=cbi->colorbar;
  rgb_node=cbi->rgb_node;
  alpha=cbi->alpha;

  for(i=0;i<cbi->index_node[0];i++){
    colorbar[3*i]=rgb_node[0]/255.0;
    colorbar[1+3*i]=rgb_node[1]/255.0;
    colorbar[2+3*i]=rgb_node[2]/255.0;
    if(
      (rgb_node[0]==0&&rgb_node[1]==1&&rgb_node[2]==2)||
      (rgb_node[0]==253&&rgb_node[1]==254&&rgb_node[2]==255)
      ){
      alpha[i]=0;
    }
    else{
      alpha[i]=255;
    }
  }
  for(i=0;i<cbi->nnodes-1;i++){
    int i1,i2,j;

    i1 = cbi->index_node[i];
    i2 = cbi->index_node[i+1];
    if(i2==i1)continue;
    rgb_node = cbi->rgb_node+3*i;
    for(j=i1;j<i2;j++){
      float factor;

      factor = (float)(j-i1)/(float)(i2-i1);
      colorbar[3*j]=MIX(factor,rgb_node[3],rgb_node[0])/255.0;
      colorbar[1+3*j]=MIX(factor,rgb_node[4],rgb_node[1])/255.0;
      colorbar[2+3*j]=MIX(factor,rgb_node[5],rgb_node[2])/255.0;
      if(
        (rgb_node[0]==0&&rgb_node[1]==1&&rgb_node[2]==2&&
        rgb_node[3]==0&&rgb_node[4]==1&&rgb_node[5]==2)||
        (rgb_node[0]==253&&rgb_node[1]==254&&rgb_node[2]==255&&
         rgb_node[3]==253&&rgb_node[4]==254&&rgb_node[5]==255)
        ){
        alpha[j]=0;
      }
      else{
        alpha[j]=255;
      }
    }
  }
  rgb_node = cbi->rgb_node+3*(cbi->nnodes-1);
  for(i=cbi->index_node[cbi->nnodes-1];i<256;i++){
    colorbar[3*i]=rgb_node[0]/255.0;
    colorbar[1+3*i]=rgb_node[1]/255.0;
    colorbar[2+3*i]=rgb_node[2]/255.0;
    if(
      (rgb_node[0]==0&&rgb_node[1]==1&&rgb_node[2]==2)||
      (rgb_node[0]==253&&rgb_node[1]==254&&rgb_node[2]==255)
      )
    {
      alpha[i]=0;
    }
    else{
      alpha[i]=255;
    }
  }
  if(show_extreme_mindata==1){
    colorbar[0]=rgb_below_min[0];
    colorbar[1]=rgb_below_min[1];
    colorbar[2]=rgb_below_min[2];
  }
  if(show_extreme_maxdata==1){
    colorbar[0+3*255]=rgb_above_max[0];
    colorbar[1+3*255]=rgb_above_max[1];
    colorbar[2+3*255]=rgb_above_max[2];
  }
  CheckMemory;
}

/* ------------------ UpdateColorbarNodes ------------------------ */

void UpdateColorbarNodes(colorbardata *cbi){
  float total_dist = 0.0;
  int i;

  for(i = 0;i<cbi->nnodes-1;i++){
    unsigned char *node1, *node2;
    float dist,dx,dy,dz;

    node1 = cbi->rgb_node+3*i;
    node2 = node1+3;
    dx = node1[0]-node2[0];
    dy = node1[1]-node2[1];
    dz = node1[2]-node2[2];
    dist = sqrt(dx*dx+dy*dy+dz*dz);
    total_dist += dist;
  }
  cbi->index_node[0] = 0;
  for(i = 1;i<cbi->nnodes-1;i++){
    int index;
    unsigned char *node1, *node2;
    float dist, dx, dy, dz;

    node1 = cbi->rgb_node+3*(i-1);
    node2 = node1+3;
    dx = node1[0]-node2[0];
    dy = node1[1]-node2[1];
    dz = node1[2]-node2[2];
    dist = sqrt(dx*dx+dy*dy+dz*dz);
    index = 255*dist/total_dist;
    cbi->index_node[i] = CLAMP(cbi->index_node[i-1]+index,0,255);
  }
  cbi->index_node[cbi->nnodes-1] = 255;
  RemapColorbar(cbi);
}

/* ------------------ RemapColorbarType ------------------------ */

void RemapColorbarType(int cb_oldtype, char *cb_newname){
  switch(cb_oldtype){
    case 0:
      strcpy(cb_newname,"Rainbow");
      break;
    case 1:
      strcpy(cb_newname,"Rainbow 2");
      break;
    case 2:
      strcpy(cb_newname,"yellow->red");
      break;
    case 3:
      strcpy(cb_newname,"blue->green->red");
      break;
    case 4:
      strcpy(cb_newname,"blue->red split");
      break;
    case 5:
      strcpy(cb_newname,"FED");
      break;
    case 6:
      //strcpy(cb_newname,"fire (original)");
      strcpy(cb_newname,"fire 2");
      break;
    case 7:
     // strcpy(cb_newname,"fire (black->orange)");
      strcpy(cb_newname,"fire 2");
      break;
    case 8:
      //strcpy(cb_newname,"fire (new)");
      strcpy(cb_newname,"fire 2");
      break;
    case 9:
      //strcpy(cb_newname,"fire (new2)");
      strcpy(cb_newname,"fire 2");
      break;
    case 10:
      //strcpy(cb_newname,"fire (custom)");
      strcpy(cb_newname,"fire 2");
      break;
    case 11:
      strcpy(cb_newname,"fire line (level set)");
      break;
    case 12:
      strcpy(cb_newname,"fire line (wall thickness)");
      break;
    case 13:
      strcpy(cb_newname,"black->white");
      break;
    case 14:
      strcpy(cb_newname, "Methanol");
      break;
    case 15:
      strcpy(cb_newname, "Propane");
      break;
    case 16:
      strcpy(cb_newname, "CO2");
      break;
    default:
#define NCOLORBARS_PREV 17
      if(cb_oldtype>=NCOLORBARS_PREV){
        cb_oldtype -= (NCOLORBARS_PREV-ndefaultcolorbars);
      }
      if(cb_oldtype>=0&&cb_oldtype<ncolorbars){
        colorbardata *cb;

        cb = colorbarinfo + cb_oldtype;
        strcpy(cb_newname,cb->label);
      }
      else{
        strcpy(cb_newname,"Rainbow");
      }
      break;
  }
}

/* ------------------ InitDefaultColorbars ------------------------ */

void InitDefaultColorbars(int nini){
  int i;
  colorbardata *cbi;

  ndefaultcolorbars=18;

  FREEMEMORY(colorbarinfo);
  ncolorbars=ndefaultcolorbars;
  NewMemory((void **)&colorbarinfo,(ncolorbars+nini)*sizeof(colorbardata));
  UpdateCurrentColorbar(colorbarinfo + colorbartype);

  // rainbow colorbar

  cbi=colorbarinfo;

  strcpy(cbi->label,"Rainbow");
  cbi->label_ptr=cbi->label;
  cbi->nnodes=5;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=255;

  cbi->index_node[1]=64;
  cbi->rgb_node[3]=0;
  cbi->rgb_node[4]=255;
  cbi->rgb_node[5]=255;

  cbi->index_node[2]=128;
  cbi->rgb_node[6]=0;
  cbi->rgb_node[7]=255;
  cbi->rgb_node[8]=0;

  cbi->index_node[3]=192;
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=255;
  cbi->rgb_node[11]=0;

  cbi->index_node[4]=255;
  cbi->rgb_node[12]=255;
  cbi->rgb_node[13]=0;
  cbi->rgb_node[14]=0;
  cbi++;

  // Rainbow 2 colorbar

  strcpy(cbi->label,"Rainbow 2");
  cbi->label_ptr=cbi->label;
  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=255;

  cbi->index_node[1]=85;
  cbi->rgb_node[3]=0;
  cbi->rgb_node[4]=255;
  cbi->rgb_node[5]=255;

  cbi->index_node[2]=170;
  cbi->rgb_node[6]=255;
  cbi->rgb_node[7]=255;
  cbi->rgb_node[8]=0;

  cbi->index_node[3]=255;
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=0;
  cbi->rgb_node[11]=0;
  cbi++;

  // yellow/red

  strcpy(cbi->label,"yellow->red");
  cbi->label_ptr=cbi->label;
  cbi->nnodes=2;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=255;
  cbi->rgb_node[1]=255;
  cbi->rgb_node[2]=0;

  cbi->index_node[1]=255;
  cbi->rgb_node[3]=255;
  cbi->rgb_node[4]=0;
  cbi->rgb_node[5]=0;
  cbi++;

  // blue/green/red

  strcpy(cbi->label,"blue->green->red");
  cbi->label_ptr=cbi->label;
  cbi->nnodes=3;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=255;

  cbi->index_node[1]=128;
  cbi->rgb_node[3]=0;
  cbi->rgb_node[4]=255;
  cbi->rgb_node[5]=0;

  cbi->index_node[2]=255;
  cbi->rgb_node[6]=255;
  cbi->rgb_node[7]=0;
  cbi->rgb_node[8]=0;
  cbi++;

  // blue/yellow/white

  strcpy(cbi->label, "blue->yellow->white");
  cbi->label_ptr = cbi->label;
  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->index_node[0]  =   0;

  cbi->rgb_node[0]    =   0;
  cbi->rgb_node[1]    = 151;
  cbi->rgb_node[2]    = 255;

  cbi->index_node[1]  = 113;

  cbi->rgb_node[3]    = 255;
  cbi->rgb_node[4]    =   0;
  cbi->rgb_node[5]    =   0;

  cbi->index_node[2]  = 212;

  cbi->rgb_node[6]    = 255;
  cbi->rgb_node[7]    = 255;
  cbi->rgb_node[8]    =   0;

  cbi->index_node[3]  = 255;

  cbi->rgb_node[9]    = 255;
  cbi->rgb_node[10]   = 255;
  cbi->rgb_node[11]   = 255;

  cbi++;

  // blue->red split

  strcpy(cbi->label,"blue->red split");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=255;

  cbi->index_node[1]=128;
  cbi->rgb_node[3]=0;
  cbi->rgb_node[4]=255;
  cbi->rgb_node[5]=255;

  cbi->index_node[2]=128;
  cbi->rgb_node[6]=255;
  cbi->rgb_node[7]=255;
  cbi->rgb_node[8]=0;

  cbi->index_node[3]=255;
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=0;
  cbi->rgb_node[11]=0;
  cbi++;

  // black->white

  bw_colorbar_index = cbi - colorbarinfo;
  strcpy(cbi->label,"black->white");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=2;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=0;

  cbi->index_node[1]=255;
  cbi->rgb_node[3] =255;
  cbi->rgb_node[4]=255;
  cbi->rgb_node[5]=255;
  cbi++;

  // FED

  strcpy(cbi->label,"FED");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=6;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=96;
  cbi->rgb_node[1]=96;
  cbi->rgb_node[2]=255;

  cbi->index_node[1]=26; // 0.295276,0.307087
  cbi->rgb_node[3]=96;
  cbi->rgb_node[4]=96;
  cbi->rgb_node[5]=255;

  cbi->index_node[2]=26;
  cbi->rgb_node[6]=255;
  cbi->rgb_node[7]=255;
  cbi->rgb_node[8]=0;

  cbi->index_node[3]=85; // 0.992126,1.003937
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=255;
  cbi->rgb_node[11]=0;

  cbi->index_node[4]=85;
  cbi->rgb_node[12]=255;
  cbi->rgb_node[13]=155;
  cbi->rgb_node[14]=0;

  cbi->index_node[5]=255;
  cbi->rgb_node[15]=255;
  cbi->rgb_node[16]=155;
  cbi->rgb_node[17]=0;
  cbi++;

  // fire (original)

  fire_colorbar_index=cbi-colorbarinfo;
  fire_colorbar=cbi;
  strcpy(cbi->label,"fire");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=0;

  cbi->index_node[1]=127;
  cbi->rgb_node[3]=0;
  cbi->rgb_node[4]=0;
  cbi->rgb_node[5]=0;

  cbi->index_node[2]=128;
  cbi->rgb_node[6]=255;
  cbi->rgb_node[7]=128;
  cbi->rgb_node[8]=0;

  cbi->index_node[3]=255;
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=128;
  cbi->rgb_node[11]=0;
  cbi++;

  // fire 2

  fire_colorbar_index=cbi-colorbarinfo;
  fire_colorbar=cbi;
  strcpy(cbi->label,"fire 2");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=10;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=0;

  cbi->index_node[1]=127;
  cbi->rgb_node[3]=38;
  cbi->rgb_node[4]=0;
  cbi->rgb_node[5]=0;

  cbi->index_node[2]=128;
  cbi->rgb_node[6]=219;
  cbi->rgb_node[7]=68;
  cbi->rgb_node[8]=21;

  cbi->index_node[3]=160;
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=125;
  cbi->rgb_node[11]=36;

  cbi->index_node[4]=183;
  cbi->rgb_node[12]=255;
  cbi->rgb_node[13]=157;
  cbi->rgb_node[14]=52;

  cbi->index_node[5]=198;
  cbi->rgb_node[15]=255;
  cbi->rgb_node[16]=170;
  cbi->rgb_node[17]=63;

  cbi->index_node[6]=214;
  cbi->rgb_node[18]=255;
  cbi->rgb_node[19]=198;
  cbi->rgb_node[20]=93;

  cbi->index_node[7]=229;
  cbi->rgb_node[21]=255;
  cbi->rgb_node[22]=208;
  cbi->rgb_node[23]=109;

  cbi->index_node[8]=244;
  cbi->rgb_node[24]=255;
  cbi->rgb_node[25]=234;
  cbi->rgb_node[26]=161;

  cbi->index_node[9]=255;
  cbi->rgb_node[27]=255;
  cbi->rgb_node[28]=255;
  cbi->rgb_node[29]=238;
  cbi++;

  // fire 3

  fire_colorbar_index = cbi - colorbarinfo;
  fire_colorbar = cbi;
  strcpy(cbi->label, "fire 3");
  cbi->label_ptr = cbi->label;

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->index_node[0] = 0;
  cbi->rgb_node[0] = 0;
  cbi->rgb_node[1] = 0;
  cbi->rgb_node[2] = 0;

  cbi->index_node[1] = 108;
  cbi->rgb_node[3] = 255;
  cbi->rgb_node[4] = 127;
  cbi->rgb_node[5] = 0;

  cbi->index_node[2] = 156;
  cbi->rgb_node[6] = 255;
  cbi->rgb_node[7] = 255;
  cbi->rgb_node[8] = 0;

  cbi->index_node[3] = 255;
  cbi->rgb_node[9] = 255;
  cbi->rgb_node[10] = 255;
  cbi->rgb_node[11] = 255;

  cbi++;

  // cool

  fire_colorbar_index = cbi - colorbarinfo;
  fire_colorbar = cbi;
  strcpy(cbi->label, "cool");
  cbi->label_ptr = cbi->label;

  cbi->nnodes = 7;
  cbi->nodehilight = 0;

  cbi->index_node[0] = 0;
  cbi->rgb_node[0] = 0;
  cbi->rgb_node[1] = 0;
  cbi->rgb_node[2] = 0;

  cbi->index_node[1] = 90;
  cbi->rgb_node[3] = 64;
  cbi->rgb_node[4] = 64;
  cbi->rgb_node[5] = 255;

  cbi->index_node[2] = 110;
  cbi->rgb_node[6] = 155;
  cbi->rgb_node[7] = 35;
  cbi->rgb_node[8] = 33;

  cbi->index_node[3] = 120;
  cbi->rgb_node[9] = 108;
  cbi->rgb_node[10] = 19;
  cbi->rgb_node[11] = 43;

  cbi->index_node[4] = 130;
  cbi->rgb_node[12] = 208;
  cbi->rgb_node[13] = 93;
  cbi->rgb_node[14] = 40;

  cbi->index_node[5] = 160;
  cbi->rgb_node[15] = 255;
  cbi->rgb_node[16] = 178;
  cbi->rgb_node[17] = 0;

  cbi->index_node[6] = 255;
  cbi->rgb_node[18] = 255;
  cbi->rgb_node[19] = 255;
  cbi->rgb_node[20] = 255;
  cbi++;

  // fire line (level set)

  levelset_colorbar=cbi-colorbarinfo;
  strcpy(cbi->label,"fire line (level set)");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=6;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=64;
  cbi->rgb_node[1]=64;
  cbi->rgb_node[2]=64;

  cbi->index_node[1]=120;
  cbi->rgb_node[3]=64;
  cbi->rgb_node[4]=64;
  cbi->rgb_node[5]=64;

  cbi->index_node[2]=120;
  cbi->rgb_node[6]=255;
  cbi->rgb_node[7]=0;
  cbi->rgb_node[8]=0;

  cbi->index_node[3]=136;
  cbi->rgb_node[9]=255;
  cbi->rgb_node[10]=0;
  cbi->rgb_node[11]=0;

  cbi->index_node[4]=136;
  cbi->rgb_node[12]=0;
  cbi->rgb_node[13]=1;
  cbi->rgb_node[14]=2;

  cbi->index_node[5]=255;
  cbi->rgb_node[15]=0;
  cbi->rgb_node[16]=1;
  cbi->rgb_node[17]=2;
  cbi++;


  // fire line (wall thickness)

  wallthickness_colorbar=cbi-colorbarinfo;
  strcpy(cbi->label,"fire line (wall thickness)");
  cbi->label_ptr=cbi->label;

  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->index_node[0]=0;
  cbi->rgb_node[0]=0;
  cbi->rgb_node[1]=0;
  cbi->rgb_node[2]=0;

  cbi->index_node[1]=32;
  cbi->rgb_node[3]=0;
  cbi->rgb_node[4]=0;
  cbi->rgb_node[5]=0;

  cbi->index_node[2]=32;
  cbi->rgb_node[6]=253;
  cbi->rgb_node[7]=254;
  cbi->rgb_node[8]=255;

  cbi->index_node[3]=255;
  cbi->rgb_node[9]=253;
  cbi->rgb_node[10]=254;
  cbi->rgb_node[11]=255;
  cbi++;

  // split

  split_colorbar_index = cbi - colorbarinfo;
  split_colorbar = cbi;
  strcpy(cbi->label, "split");
  cbi->label_ptr = cbi->label;

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->index_node[0] = 0;
  cbi->index_node[1] = 127;
  cbi->index_node[2] = 127;
  cbi->index_node[3] = 255;
  for(i = 0; i < 12; i++){
    cbi->rgb_node[i] = colorsplit[i];
  }
  cbi++;


  // Methanol

  strcpy(cbi->label, "Methanol");
  cbi->label_ptr = cbi->label;

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->index_node[0] = 0;
  cbi->rgb_node[0] = 9;
  cbi->rgb_node[1] = 160;
  cbi->rgb_node[2] = 255;

  cbi->index_node[1] = 192;
  cbi->rgb_node[3] = 9;
  cbi->rgb_node[4] = 160;
  cbi->rgb_node[5] = 255;

  cbi->index_node[2] = 200;
  cbi->rgb_node[6] = 255;
  cbi->rgb_node[7] = 255;
  cbi->rgb_node[8] = 255;

  cbi->index_node[3] = 255;
  cbi->rgb_node[9] = 255;
  cbi->rgb_node[10] = 255;
  cbi->rgb_node[11] = 255;

  cbi++;

  // Propane

  strcpy(cbi->label, "Propane");
  cbi->label_ptr = cbi->label;

  cbi->nnodes = 5;
  cbi->nodehilight = 0;

  cbi->index_node[0] = 0;
  cbi->rgb_node[0] = 0;
  cbi->rgb_node[1] = 0;
  cbi->rgb_node[2] = 0;

  cbi->index_node[1] = 140;
  cbi->rgb_node[3] = 235;
  cbi->rgb_node[4] = 120;
  cbi->rgb_node[5] = 0;

  cbi->index_node[2] = 160;
  cbi->rgb_node[6] = 250;
  cbi->rgb_node[7] = 180;
  cbi->rgb_node[8] = 0;

  cbi->index_node[3] = 190;
  cbi->rgb_node[9] = 252;
  cbi->rgb_node[10] = 248;
  cbi->rgb_node[11] = 70;

  cbi->index_node[4] = 255;
  cbi->rgb_node[12] = 255;
  cbi->rgb_node[13] = 255;
  cbi->rgb_node[14] = 255;

  cbi++;

  // CO2

  co2_colorbar_index = cbi - colorbarinfo;
  strcpy(cbi->label, "CO2");
  cbi->label_ptr = cbi->label;

  cbi->nnodes = 3;
  cbi->nodehilight = 0;

  cbi->index_node[0] = 0;
  cbi->rgb_node[0] = 0;
  cbi->rgb_node[1] = 0;
  cbi->rgb_node[2] = 255;

  cbi->index_node[1] = 192;
  cbi->rgb_node[3] = 0;
  cbi->rgb_node[4] = 0;
  cbi->rgb_node[5] = 255;

  cbi->index_node[2] = 255;
  cbi->rgb_node[6] = 255;
  cbi->rgb_node[7] = 255;
  cbi->rgb_node[8] = 255;

  cbi++;

  // construct colormaps from color node info

  for(i=0;i<ndefaultcolorbars;i++){
    cbi = colorbarinfo + i;

    RemapColorbar(cbi);
    UpdateColorbarSplits(cbi);
  }
}

/* ------------------ UpdateColorbarSplits ------------------------ */

void UpdateColorbarSplits(colorbardata *cbi){
  int i;

  cbi->nsplits=0;
  for(i=1;i<cbi->nnodes;i++){
    if(cbi->index_node[i]==cbi->index_node[i-1]){
      cbi->splits[cbi->nsplits]=i;
      cbi->nsplits++;
    }
  }
}

/* ------------------ DrawColorbarHist ------------------------ */

void DrawColorbarHist(void){
  int ibucket;
  int cbt, cbb, cbdiff;

  cbdiff = vcolorbar_top_pos - vcolorbar_down_pos;
  cbt =  vcolorbar_top_pos + cbdiff / (float)(histogram_nbuckets - 2);
  cbb = vcolorbar_down_pos - cbdiff / (float)(histogram_nbuckets - 2);

  if(histogram_show_graph == 1){
    if(histogram_show_outline == 1){
      glBegin(GL_LINES);
    }
    else{
      glBegin(GL_TRIANGLES);
    }
    for(ibucket = 0; ibucket < histogram_nbuckets; ibucket++){
      float *rgb_cb, *rgb_cb2;
      float yy, yy2;
      int cbl, cbr;
      int icolor, icolor2;
      float dcolor, val;
      histogramdata *histi;

      icolor = ibucket*(float)(nrgb_full - 1) / (float)histogram_nbuckets;
      rgb_cb = rgb_full[icolor];

      icolor2 = (ibucket + 1)*(float)(nrgb_full - 1) / (float)histogram_nbuckets;
      rgb_cb2 = rgb_full[icolor2];

      if(histogram_static == 0){
        histi = hists256_slice + CLAMP(slice_time + 1, 1, nhists256_slice);
      }
      else{
        histi = hists256_slice;
      }

      dcolor = 3.0*(vcolorbar_right_pos - vcolorbar_left_pos)/(histogram_width_factor/100.0);
      if(histi->bucket_maxval!=0.0){
        val = (float)histi->buckets[ibucket] / (float)histi->ntotal;
        cbl = vcolorbar_right_pos - dcolor*val / histi->bucket_maxval;
      }
      else{
        cbl = vcolorbar_right_pos;
      }

      cbr = vcolorbar_right_pos;

      yy = MIX2(icolor, 255, cbt, cbb);
      yy2 = MIX2(icolor2, 255, cbt, cbb);

      //   (cbl,yy)-------(cbr,yy)
      //      |         /    |
      //      |     /        |
      //      |  /           |
      //   (cbl,yy2)------(cbr,yy2)

      if(histogram_show_outline == 1){
        glColor4fv(rgb_cb);
        glVertex2f(cbl, yy);
        glVertex2f(cbr, yy);

        glVertex2f(cbr, yy);
        glColor4fv(rgb_cb2);
        glVertex2f(cbr, yy2);

        glVertex2f(cbr, yy2);
        glVertex2f(cbl, yy2);

        glVertex2f(cbl, yy2);
        glColor4fv(rgb_cb);
        glVertex2f(cbl, yy);
      }
      else{
        glColor4fv(rgb_cb);
        glVertex2f(cbl, yy);
        glVertex2f(cbr, yy);
        glColor4fv(rgb_cb2);
        glVertex2f(cbl, yy2);

        glVertex2f(cbr, yy2);
        glVertex2f(cbl, yy2);
        glColor4fv(rgb_cb);
        glVertex2f(cbr, yy);
      }
    }
    glEnd();
  }
}

/* ------------------ DrawColorbarHistLabels ------------------------ */

void DrawColorbarHistLabels(int lefthist){
  if(histogram_show_numbers == 1 && (showslice == 1 || (showvslice == 1 && vslicecolorbarflag == 1))){
    char *percen = "%";
    int i;

    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-lefthist*(colorbar_label_width + h_space), 0.0, 0.0);
    glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);

    for(i = 0; i < nrgb; i++){
      float vert_position;
      char string[100], *stringptr;
      GLfloat *foreground_color;
      histogramdata *histi;
      float val;

      foreground_color = &(foregroundcolor[0]);

      if(histogram_static == 0){
        histi = hists12_slice + CLAMP(slice_time + 1, 1, nhists256_slice);
      }
      else{
        histi = hists12_slice;
      }

      val = 100.0*(float)histi->buckets[i] / (float)histi->ntotal;

      sprintf(string, "%i%s", (int)(val + 0.5), percen);

      stringptr = string;
      vert_position = MIX2(MAX(i - 0.5, -0.15), nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      OutputBarText(0.0, vert_position, foreground_color, stringptr);
    }
    glPopMatrix();
  }
}

/* ------------------ DrawHorizontalColorbarReg ------------------------ */

void DrawHorizontalColorbarReg(void){
  int i;

  glBegin(GL_QUADS);
  for(i = 0; i < nrgb_full - 1; i++){
    float *rgb_cb, *rgb_cb2;
    float xx, xx2;
    int cbd;
    int i3;

    rgb_cb = rgb_full[i];

    cbd = hcolorbar_down_pos;

    xx  = MIX2(i,     255, hcolorbar_right_pos, hcolorbar_left_pos);
    xx2 = MIX2(i + 1, 255, hcolorbar_right_pos, hcolorbar_left_pos);
    i3 = i + 1;
    if(i == nrgb_full - 2)i3 = i;
    rgb_cb2 = rgb_full[i3];

    glColor4fv(rgb_cb2);
    glVertex2f(xx2, cbd);
    glVertex2f(xx2, hcolorbar_top_pos);

    glColor4fv(rgb_cb);
    glVertex2f(xx, hcolorbar_top_pos);
    glVertex2f(xx, cbd);
  }
  glEnd();
}

/* ------------------ DrawVerticalColorbarReg ------------------------ */

void DrawVerticalColorbarReg(void){
  int i;

  glBegin(GL_QUADS);
  for(i = 0; i < nrgb_full - 1; i++){
    float *rgb_cb, *rgb_cb2;
    float yy, yy2;
    int i3;

    rgb_cb = rgb_full[i];

    yy = MIX2(i, 255, vcolorbar_top_pos, vcolorbar_down_pos);
    yy2 = MIX2(i + 1, 255, vcolorbar_top_pos, vcolorbar_down_pos);
    i3 = i + 1;
    if(i == nrgb_full - 2)i3 = i;
    rgb_cb2 = rgb_full[i3];

    glColor4fv(rgb_cb);
    glVertex2f(vcolorbar_left_pos,  yy);
    glVertex2f(vcolorbar_right_pos, yy);

    glColor4fv(rgb_cb2);
    glVertex2f(vcolorbar_right_pos, yy2);
    glVertex2f(vcolorbar_left_pos,  yy2);
  }
  glEnd();
}

#define COLORBAR_PART     0
#define COLORBAR_SLICE    1
#define COLORBAR_ISO      2
#define COLORBAR_BOUNDARY 3
#define COLORBAR_PLOT3D   4
#define COLORBAR_ZONE     5

/* ------------------ UpdateShowSliceColorbar ------------------------ */

void UpdateShowSliceColorbar(int *showcfast_arg, int *show_slice_colorbar_arg){
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;

  if(showzone==1&&zonecolortype==ZONETEMP_COLOR)showcfast_local = 1;
  if(showslice==1||(showcfast_local==0&&showvslice==1&&vslicecolorbarflag==1))show_slice_colorbar_local = 1;
  if(show_slice_colorbar_local==1&&showcfast_local==1&&strcmp(slicebounds[slicefile_labelindex].label->shortlabel, "TEMP")==0)show_slice_colorbar_local=0;
  *showcfast_arg = showcfast_local;
  *show_slice_colorbar_arg = show_slice_colorbar_local;
}

/* ------------------ CountColorbars ------------------------ */

int CountColorbars(void){
  int count = 0;
  int i;
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;

  UpdateShowSliceColorbar(&showcfast_local, &show_slice_colorbar_local);

  for(i=0;i<6;i++){
    hcolorbar_vis[i]=-1;
  }
  if(showevac_colorbar == 1 || showsmoke == 1){
    hcolorbar_vis[COLORBAR_PART]=count+2;
    count++;
  }
  if(show_slice_colorbar_local==1){
    hcolorbar_vis[COLORBAR_SLICE]=count+2;
    count++;
  }
  if(showiso_colorbar == 1){
    hcolorbar_vis[COLORBAR_ISO]=count+2;
    count++;
  }
  if(showpatch == 1 && wall_cell_color_flag == 0){
    hcolorbar_vis[COLORBAR_BOUNDARY]=count+2;
    count++;
  }
  if(showplot3d == 1){
    hcolorbar_vis[COLORBAR_PLOT3D]=count+2;
    count++;
  }
  if(showcfast_local==1){
    hcolorbar_vis[COLORBAR_ZONE]=count+2;
    count++;
  }
  return count;
}

/* ------------------ DrawHorizontalColorbars ------------------------ */

void DrawHorizontalColorbars(void){
  int doit=0;
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;

  UpdateShowSliceColorbar(&showcfast_local, &show_slice_colorbar_local);

  CountColorbars();

  if(vis_colorbar==hcolorbar_vis[COLORBAR_SLICE]){
    if(show_slice_colorbar_local == 1)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_BOUNDARY]){
    if(showpatch == 1 && wall_cell_color_flag == 0)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_PLOT3D]){
    if(showplot3d==1)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_ZONE]){
    if(showcfast_local==1)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_PART]){
    if(showevac_colorbar==1)doit=1;
    if(showsmoke==1&&parttype!=0)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_ISO]){
    if(showiso_colorbar)doit=1;
  }
  else{
    doit=1;
  }
  if(doit==0)return;

  SNIFF_ERRORS("before horizontal colorbar");
  CheckMemory;

    // -------------- draw plot3d colorbars ------------

  if(showplot3d == 1 && contour_type == STEPPED_CONTOURS){
    int i;
    float right_hat;

    right_hat = hcolorbar_right_pos*(float)(nrgb - 3) / (float)(nrgb - 2)+hcolorbar_left_pos/(float)(nrgb-2);

    glBegin(GL_QUADS);
    for(i = 0; i < nrgb - 2; i++){
      float *rgb_plot3d_local;
      float xleft, xright;

      rgb_plot3d_local = rgb_plot3d_contour[i];

      xleft = MIX2(i, nrgb - 3, right_hat, hcolorbar_left_pos);
      xright = MIX2(i + 1, nrgb - 3, right_hat, hcolorbar_left_pos);

      if(rgb_plot3d_local[3] != 0.0){
        glColor4fv(rgb_plot3d_local);
        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xright, hcolorbar_down_pos);
        glVertex2f(xright, hcolorbar_top_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
      }
    }
    glEnd();
    if(show_extreme_mindata == 1 || show_extreme_maxdata == 1){
      float barmid;
      float *rgb_plot3d_local;
      float xleft, xright;

      rgb_plot3d_local = rgb_plot3d_contour[nrgb - 2];
      barmid = (hcolorbar_down_pos + hcolorbar_top_pos) / 2.0;
      i = -1;
      xright = MIX2(i + 0.5, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);
      xleft  = MIX2(i + 1, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);

      if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glEnable(GL_POLYGON_SMOOTH);

      if(show_extreme_mindata == 1 && have_extreme_mindata == 1 && rgb_plot3d_local[3] != 0.0){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_plot3d_local);

        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
        glVertex2f(xright, barmid);
        glEnd();
      }

      i = nrgb - 2;
      xleft = MIX2(i, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);
      xright = MIX2(i + 0.5, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);

      rgb_plot3d_local = rgb_plot3d_contour[nrgb - 1];
      if(show_extreme_maxdata == 1 && have_extreme_maxdata == 1 && rgb_plot3d_local[3] != 0.0){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_plot3d_local);
        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
        glVertex2f(xright, barmid);
        glEnd();
      }
      if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glDisable(GL_POLYGON_SMOOTH);
    }
  }
  else{

    // -------------- draw all other colorbars ------------

    if(hists12_slice == NULL || histogram_show_graph == 0){
      DrawHorizontalColorbarReg();
    }
  }
  if(show_extreme_mindata == 1 || show_extreme_maxdata == 1){
    float barmid;

    barmid = (hcolorbar_top_pos + hcolorbar_down_pos) / 2.0;

    if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glEnable(GL_POLYGON_SMOOTH);

    if(show_extreme_mindata == 1 && have_extreme_mindata == 1){
      glBegin(GL_TRIANGLES);
      glColor4fv(rgb_full[0]);

      glVertex2f(hcolorbar_left_pos, hcolorbar_down_pos);
      glVertex2f(hcolorbar_left_pos, hcolorbar_top_pos);
      glVertex2f(hcolorbar_left_pos - 0.866*hcolorbar_delta,barmid);
      glEnd();
    }

    if(show_extreme_maxdata == 1 && have_extreme_maxdata == 1){
      glBegin(GL_TRIANGLES);
      glColor4fv(rgb_full[nrgb_full - 1]);
      glVertex2f(hcolorbar_right_pos, hcolorbar_top_pos);
      glVertex2f(hcolorbar_right_pos, hcolorbar_down_pos);
      glVertex2f(hcolorbar_right_pos + 0.866*hcolorbar_delta, barmid);
      glEnd();
    }
    if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glDisable(GL_POLYGON_SMOOTH);
  }
}

/* ------------------ DrawVerticalColorbars ------------------------ */

void DrawVerticalColorbars(void){
  int i;
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;

  UpdateShowSliceColorbar(&showcfast_local, &show_slice_colorbar_local);

  // -------------- compute columns where left labels will occur ------------

  if(showiso_colorbar==1||showevac_colorbar==1||
    (showsmoke==1&&parttype!=0)|| show_slice_colorbar_local==1||
    (showpatch==1&&wall_cell_color_flag==0)||
    showcfast_local==1||
    showplot3d==1){

    SNIFF_ERRORS("before vertical colorbar");
    CheckMemory;

    // -------------- draw plot3d colorbars ------------

    if(showplot3d==1&&contour_type==STEPPED_CONTOURS){
      float top_hat;

      top_hat = vcolorbar_top_pos*(float)(nrgb - 3) / (float)(nrgb - 2)+vcolorbar_down_pos/(float)(nrgb-2);

      glBegin(GL_QUADS);
      for(i = 0; i < nrgb-2; i++){
        float *rgb_plot3d_local;
        float ybot, ytop;

        rgb_plot3d_local = rgb_plot3d_contour[i];
        ybot = MIX2(  i,nrgb-3,top_hat,vcolorbar_down_pos);
        ytop = MIX2(i+1,nrgb-3,top_hat,vcolorbar_down_pos);

        if(rgb_plot3d_local[3]!=0.0){
          glColor4fv(rgb_plot3d_local);
          glVertex2f((float)vcolorbar_left_pos, ybot);
          glVertex2f(vcolorbar_right_pos,ybot);

          glVertex2f(vcolorbar_right_pos,ytop);
          glVertex2f(vcolorbar_left_pos, ytop);
        }
      }
      glEnd();
      if(show_extreme_mindata==1||show_extreme_maxdata==1){
        float barmid;
        float *rgb_plot3d_local;
        float ybot, ytop;

        rgb_plot3d_local = rgb_plot3d_contour[nrgb-2];
        barmid = (vcolorbar_left_pos+vcolorbar_right_pos)/2.0;
        i=-1;
        ytop = MIX2(i+0.5,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);
        ybot = MIX2(i+1,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);

        if(have_extreme_mindata==1||have_extreme_maxdata==1)glEnable(GL_POLYGON_SMOOTH);

        if(show_extreme_mindata==1&&have_extreme_mindata==1&&rgb_plot3d_local[3]!=0.0){
          glBegin(GL_TRIANGLES);
          glColor4fv(rgb_plot3d_local);

          glVertex2f(vcolorbar_left_pos,ybot);
          glVertex2f(barmid,ytop);
          glVertex2f(vcolorbar_right_pos,ybot);
          glEnd();
        }

        i=nrgb-2;
        ybot = MIX2(i,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);
        ytop = MIX2(i+0.5,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);

        rgb_plot3d_local = rgb_plot3d_contour[nrgb-1];
        if(show_extreme_maxdata==1&&have_extreme_maxdata==1&&rgb_plot3d_local[3]!=0.0){
          glBegin(GL_TRIANGLES);
          glColor4fv(rgb_plot3d_local);
          glVertex2f(vcolorbar_left_pos, ybot);
          glVertex2f(vcolorbar_right_pos,ybot);
          glVertex2f(barmid, ytop);
          glEnd();
        }
        if(have_extreme_mindata==1||have_extreme_maxdata==1)glDisable(GL_POLYGON_SMOOTH);
      }
    }
    else{

      // -------------- draw all other colorbars ------------

      if(hists12_slice == NULL || histogram_show_graph == 0){
        DrawVerticalColorbarReg();
      }
      if(histogram_show_graph == 1 || histogram_show_numbers == 1){
        if(hists12_slice != NULL)DrawColorbarHist();
      }
    }
    if(show_extreme_mindata==1||show_extreme_maxdata==1){
      float barmid;

      barmid=(vcolorbar_right_pos+vcolorbar_left_pos)/2.0;

      if(have_extreme_mindata==1||have_extreme_maxdata==1)glEnable(GL_POLYGON_SMOOTH);

      if(show_extreme_mindata==1&&have_extreme_mindata==1){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_full[0]);

        glVertex2f( vcolorbar_left_pos, vcolorbar_down_pos);
        glVertex2f(            barmid, vcolorbar_down_pos-0.866*vcolorbar_delta);
        glVertex2f(vcolorbar_right_pos, vcolorbar_down_pos);
        glEnd();
      }

      if(show_extreme_maxdata==1&&have_extreme_maxdata==1){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_full[nrgb_full-1]);
        glVertex2f(vcolorbar_right_pos, vcolorbar_top_pos);
        glVertex2f(            barmid, vcolorbar_top_pos+0.866*vcolorbar_delta);
        glVertex2f( vcolorbar_left_pos, vcolorbar_top_pos);
        glEnd();
      }
      if(have_extreme_mindata==1||have_extreme_maxdata==1)glDisable(GL_POLYGON_SMOOTH);
    }
  }
}

/* ------------------ DrawHorizontalColorbarRegLabels ------------------------ */

void DrawHorizontalColorbarRegLabels(void){
  int i;
  int iposition;

  int sliceflag = 0;
  int isoflag = 0;
  float *slicefactor = NULL;
  float slicefactor2[2];
  float *isofactor = NULL;

  int plot3dflag = 0;
  float *plot3dfactor = NULL;
  float plot3dfactor2[2];
  float plot3drange;

  int patchflag = 0;
  int zoneflag = 0;
  float *patchfactor = NULL;
  float *zonefactor = NULL;
  float patchrange = 0.0;
  float zonerange;

  int partflag = 0;
  float *partfactor = NULL;
  float partrange = 0.0;

  int type_label_left, type_label_down;
  int axis_label_left, axis_label_down;

  int fed_slice = 0;

  GLfloat *foreground_color, *red_color;

  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;

  UpdateShowSliceColorbar(&showcfast_local, &show_slice_colorbar_local);

  foreground_color = &(foregroundcolor[0]);
  red_color = &(redcolor[0]);

  type_label_left = hcolorbar_delta+hcolorbar_right_pos+h_space;
  type_label_down = 1.5*VP_vcolorbar.text_height;
  axis_label_left = -colorbar_label_width/4;
  axis_label_down = hcolorbar_down_pos-(VP_vcolorbar.text_height + v_space);

  if(showiso_colorbar == 1 || showevac_colorbar == 1 ||
    (showsmoke == 1 && parttype != 0) || show_slice_colorbar_local == 1 ||
    (showpatch == 1 && wall_cell_color_flag == 0) ||
    showcfast_local==1 || showplot3d == 1){

    SNIFF_ERRORS("before colorbar");
    CheckMemory;
    if(show_slice_colorbar_local==1){
      boundsdata *sb;

      sb = slicebounds + slicefile_labelindex;

      if(strcmp(sb->label->shortlabel, "FED") ==  0&& current_colorbar != NULL){
        strcpy(default_fed_colorbar, current_colorbar->label);
        if(strcmp(current_colorbar->label, "FED") == 0){
          fed_slice = 1;
          if(strcmp(sb->colorlabels[1], "0.00") != 0 || strcmp(sb->colorlabels[nrgb - 1], "3.00") != 0)fed_slice = 0;
        }
      }
    }
  }

  // -------------- particle file top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PART]&&(showevac_colorbar == 1 || showsmoke == 1)){
    char partunitlabel2[256], partshortlabel2[256];

    strcpy(partshortlabel2, "");
    strcpy(partunitlabel2, "");

    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);

    if(parttype != 0){
      if(showsmoke == 1 && showevac == 0)OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Part");
      if(showevac == 1)OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Human");
    }
    if(parttype == -1){
      strcpy(partshortlabel2, "temp");
      strcpy(partunitlabel2, (const char *)degC);
    }
    else if(parttype == -2){
      strcpy(partshortlabel2, "HRRPUV");
      strcpy(partunitlabel2, "kW/m3");
    }
    else{
      if(partshortlabel != NULL)strcpy(partshortlabel2, partshortlabel);
      if(partunitlabel != NULL)strcpy(partunitlabel2, partunitlabel);
    }
    if(parttype != 0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel2, &partunitclass, &partunittype);
      if(partunitclass >= 0 && partunitclass < nunitclasses && partunittype >= 0){
        partflag = 1;
        partfactor = unitclasses[partunitclass].units[partunittype].scale;
        strcpy(partunitlabel, unitclasses[partunitclass].units[partunittype].unit);
      }
      OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, partshortlabel);
      OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, partunitlabel);
    }
    glPopMatrix();
  }

  // -------------- slice file top labels ------------
 if(vis_colorbar==hcolorbar_vis[COLORBAR_SLICE]&&show_slice_colorbar_local==1){
    char unitlabel[256];
    int sliceunitclass, sliceunittype;
    boundsdata *sb;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
        sliceflag = 1;
        slicefactor = unitclasses[sliceunitclass].units[sliceunittype].scale;
        strcpy(unitlabel, unitclasses[sliceunitclass].units[sliceunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Slice");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    if(strcmp(unitlabel, "ppm") == 0 && slicefactor != NULL){
      slicefactor2[0] = *slicefactor;
      slicefactor2[1] = 0.0;
      slicefactor = slicefactor2;
    }
    glPopMatrix();
  }

  // -------------- isosurface top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ISO]&&showiso_colorbar == 1){
    char unitlabel[256];
    boundsdata *sb;

    sb = isobounds + iisottype;
    strcpy(unitlabel, sb->label->unit);
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Iso");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    glPopMatrix();
  }

  // -------------- boundary file top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_BOUNDARY]&&showpatch == 1 && wall_cell_color_flag == 0){
    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = patchinfo + boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
        patchflag = 1;
        patchfactor = unitclasses[patchunitclass].units[patchunittype].scale;
        strcpy(unitlabel, unitclasses[patchunitclass].units[patchunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);

    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Bndry");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, patchi->label.shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    glPopMatrix();
  }

  // -------------- plot3d top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PLOT3D]&&showplot3d == 1){
    char *p3label;
    char *up3label;
    char unitlabel[256];
    int plot3dunitclass, plot3dunittype;

    up3label = plot3dinfo[0].label[plotn - 1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass >= 0 && plot3dunitclass < nunitclasses){
      if(plot3dunittype > 0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }
    p3label = plot3dinfo[0].label[plotn - 1].shortlabel;
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Plot3D");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, p3label);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    if(strcmp(unitlabel, "ppm") == 0 && plot3dfactor != NULL){
      plot3dfactor2[0] = *plot3dfactor;
      plot3dfactor2[1] = 0.0;
      plot3dfactor = plot3dfactor2;
    }
    glPopMatrix();
  }

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ZONE]&&showcfast_local == 1){
    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
        zoneflag = 1;
        zonefactor = unitclasses[zoneunitclass].units[zoneunittype].scale;
        strcpy(unitlabel, unitclasses[zoneunitclass].units[zoneunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Zone");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, "Temp");
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    glPopMatrix();
    SNIFF_ERRORS("After ZONE labels");
  }

  // -------------- isosurface left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ISO]&&showiso_colorbar == 1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float isorange;

    sb = isobounds + iisottype;
    tttmin = sb->levels256[0];
    tttmax = sb->levels256[255];
    isorange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char isocolorlabel[256], isolabel[256];
      char *isocolorlabel_ptr = NULL;
      float horiz_position;

      tttval = sb->levels256[valindex];
      Num2String(isolabel, tttval);
      isocolorlabel_ptr = isolabel;
      if(isoflag == 1){
        ScaleFloat2String(tttval, isocolorlabel, isofactor);
        isocolorlabel_ptr = isocolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, isocolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float horiz_position;
      char isocolorlabel[256];
      char *isocolorlabel_ptr = NULL;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      isocolorlabel_ptr = &(sb->colorlabels[i + 1][0]);
      if(isoflag == 1){
        float val;

        val = tttmin + i*isorange / (nrgb - 2);
        ScaleFloat2String(val, isocolorlabel, isofactor);
        isocolorlabel_ptr = isocolorlabel;
      }
      OutputBarText(horiz_position, 0.0, foreground_color, isocolorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- particle left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PART]&&(showevac_colorbar == 1 || (showsmoke == 1 && parttype != 0))){
    float *partlevels256_ptr;
    float tttval, tttmin, tttmax;

    partlevels256_ptr = partlevels256;
    if(global_prop_index>= 0 &&global_prop_index < npart5prop){
      partlevels256_ptr = part5propinfo[global_prop_index].ppartlevels256;
    }

    iposition = -1;
    tttmin = partlevels256_ptr[0];
    tttmax = partlevels256_ptr[255];
    partrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char partcolorlabel[256], *partcolorlabel_ptr = NULL, partlabel[256];
      float horiz_position;

      tttval = partlevels256_ptr[valindex];
      Num2String(partlabel, tttval);
      partcolorlabel_ptr = partlabel;
      if(partflag == 1){
        ScaleFloat2String(tttval, partcolorlabel, partfactor);
        partcolorlabel_ptr = partcolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, partcolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float horiz_position;
      char partcolorlabel[256];
      char *partcolorlabel_ptr = NULL;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      if(global_prop_index>= 0 &&global_prop_index < npart5prop){
        float val;

        val = part5propinfo[global_prop_index].partlabelvals[i + 1];
        Float2String(partcolorlabel, val, ncolorlabel_digits, force_fixedpoint);
        partcolorlabel_ptr = partcolorlabel;
      }
      else{
        if(colorlabelpart != NULL){
          partcolorlabel_ptr = &colorlabelpart[i + 1][0];
        }
        else{
          partcolorlabel_ptr = NULL;
        }
      }
      if(partflag == 1){
        float val;

        partcolorlabel_ptr = partcolorlabel;
        val = tttmin + i*partrange / (nrgb - 2);
        val = ScaleFloat2Float(val, partfactor);
        Float2String(partcolorlabel_ptr, val, ncolorlabel_digits, force_fixedpoint);
      }
      OutputBarText(horiz_position, 0.0, foreground_color, partcolorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- slice left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_SLICE]&&show_slice_colorbar_local==1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float slicerange;

    sb = slicebounds + slicefile_labelindex;
    tttmin = sb->levels256[0];
    tttmax = sb->levels256[255];
    slicerange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(axis_label_left,axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char slicelabel[256], slicecolorlabel[256];
      char *slicecolorlabel_ptr = NULL;
      float horiz_position;

      tttval = sb->levels256[valindex];
      Num2String(slicelabel, tttval);
      slicecolorlabel_ptr = slicelabel;
      if(sliceflag == 1){
        ScaleFloat2String(tttval, slicecolorlabel, slicefactor);
        slicecolorlabel_ptr = slicecolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, slicecolorlabel_ptr);
    }
    if(fed_slice == 1){
      for(i = 0; i < nrgb - 1; i++){
        float horiz_position;

        horiz_position = MIX2(0.0, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "0.00");

        horiz_position = MIX2(0.3, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "0.30");

        horiz_position = MIX2(1.0, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "1.00");

        horiz_position = MIX2(3.0, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "3.00");
      }
    }
    else{
      for(i = 0; i < nrgb - 1; i++){
        float horiz_position;
        char slicecolorlabel[256];
        char *slicecolorlabel_ptr = NULL;

        horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
        if(iposition == i)continue;
        slicecolorlabel_ptr = &(sb->colorlabels[i + 1][0]);
        if(sliceflag == 1){
          float val;

          val = tttmin + i*slicerange / (nrgb - 2);
          ScaleFloat2String(val, slicecolorlabel, slicefactor);
          slicecolorlabel_ptr = slicecolorlabel;
          Float2String(slicecolorlabel_ptr, val, ncolorlabel_digits, force_fixedpoint);
        }
        else{
          slicecolorlabel_ptr = slicecolorlabel;
          Float2String(slicecolorlabel_ptr, sb->colorvalues[i+1], ncolorlabel_digits, force_fixedpoint);
        }
        OutputBarText(horiz_position, 0.0, foreground_color, slicecolorlabel_ptr);
      }
    }
    glPopMatrix();
  }

  // -------------- boundary left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_BOUNDARY]&&showpatch == 1 && wall_cell_color_flag == 0){
    float tttval, tttmin, tttmax;

    iposition = -1;
    tttmin = boundarylevels256[0];
    tttmax = boundarylevels256[255];
    patchrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char boundary_colorlabel[256], boundarylabel[256], *boundary_colorlabel_ptr = NULL;
      float horiz_position;

      // draw boundary file value selected with mouse
      tttval = boundarylevels256[valindex];
      Num2String(boundarylabel, tttval);
      boundary_colorlabel_ptr = &(boundarylabel[0]);
      if(patchflag == 1){
        ScaleFloat2String(tttval, boundary_colorlabel, patchfactor);
        boundary_colorlabel_ptr = boundary_colorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0,horiz_position, red_color, boundary_colorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      char boundary_colorlabel[256];
      char *boundary_colorlabel_ptr = NULL;
      float horiz_position;
      float val;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);

      if(iposition == i)continue;
      if(patchflag == 1){
        val = tttmin + i*patchrange / (nrgb - 2);
      }
      else{
        val = colorvaluespatch[i+1];
      }
      val = ScaleFloat2Float(val, patchfactor);
      Float2String(boundary_colorlabel, val, ncolorlabel_digits, force_fixedpoint);
      boundary_colorlabel_ptr = boundary_colorlabel;
      OutputBarText(horiz_position, 0.0, foreground_color, boundary_colorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- zone left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ZONE]&&showcfast_local == 1){
    float tttval, tttmin, tttmax;

    iposition = -1;
    tttmin = zonelevels256[0];
    tttmax = zonelevels256[255];
    zonerange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char zonecolorlabel[256], *zonecolorlabel_ptr = NULL, zonelabel[256];
      float horiz_position;

      tttval = zonelevels256[valindex];
      Num2String(zonelabel, tttval);
      zonecolorlabel_ptr = &(zonelabel[0]);
      if(zoneflag == 1){
        ScaleFloat2String(tttval, zonecolorlabel, zonefactor);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, zonecolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float horiz_position;
      char zonecolorlabel[256];
      char *zonecolorlabel_ptr = NULL;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      zonecolorlabel_ptr = &colorlabelzone[i + 1][0];
      if(zoneflag == 1){
        float val;

        val = tttmin + (i - 1)*zonerange / (nrgb - 2);
        ScaleFloat2String(val, zonecolorlabel, zonefactor);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      OutputBarText(horiz_position, 0.0, foreground_color, zonecolorlabel_ptr);
    }
    SNIFF_ERRORS("after zone left labels");
    glPopMatrix();
  }

  // -------------- plot3d left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PLOT3D]&&showplot3d == 1){
    float *p3lev;
    float tttval, tttmin, tttmax;

    iposition = -1;
    p3lev = p3levels256[plotn - 1];
    tttmin = p3lev[0];
    tttmax = p3lev[255];
    plot3drange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char plot3dcolorlabel[256], p3dlabel[256], *plot3dcolorlabel_ptr = NULL;
      float horiz_position;

      tttval = p3lev[valindex];
      Num2String(p3dlabel, tttval);
      plot3dcolorlabel_ptr = p3dlabel;
      if(plot3dflag == 1){
        ScaleFloat2String(tttval, plot3dcolorlabel, plot3dfactor);
        plot3dcolorlabel_ptr = plot3dcolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, plot3dcolorlabel_ptr);
    }
    if(visiso == 0){
      float horiz_position;

      for(i = 0; i < nrgb - 1; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;
        float val;

        horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
        if(iposition == i)continue;
        if(plot3dflag == 1){
          val = tttmin + i*plot3drange / (nrgb - 2);
        }
        else{
          val = colorvaluesp3[plotn - 1][i];
        }
        plot3dcolorlabel_ptr = plot3dcolorlabel;
        ScaleFloat2String(val, plot3dcolorlabel, plot3dfactor);
        Float2String(plot3dcolorlabel, val, ncolorlabel_digits, force_fixedpoint);
        OutputBarText(horiz_position, 0.0, foreground_color, plot3dcolorlabel_ptr);
      }
    }
    else{
      float horiz_position;
      float right_hat;

      right_hat = hcolorbar_right_pos*(float)(nrgb - 3) / (float)(nrgb - 2)+hcolorbar_left_pos/(float)(nrgb-2);
      for(i = 0; i < nrgb - 2; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;

        horiz_position = MIX2(i, nrgb - 2, right_hat, hcolorbar_left_pos);

        if(iposition == i)continue;
        plot3dcolorlabel_ptr = &colorlabeliso[plotn - 1][i][0];
        if(plot3dflag == 1){
          float val;

          val = tttmin + (i - 1)*plot3drange / (nrgb - 2);
          ScaleFloat2String(val, plot3dcolorlabel, plot3dfactor);
          plot3dcolorlabel_ptr = plot3dcolorlabel;
        }
        if(isolevelindex == i || isolevelindex2 == i){
          OutputBarText(horiz_position, 0.0, red_color, plot3dcolorlabel_ptr);
        }
        else{
          OutputBarText(horiz_position, 0.0, foreground_color, plot3dcolorlabel_ptr);
        }
      }
    }
    glPopMatrix();
  }
}

/* ------------------ DrawVerticalColorbarRegLabels ------------------------ */

void DrawVerticalColorbarRegLabels(void){
  int i;
  int ileft = 0;
  int leftzone, leftsmoke, leftslice, lefthist, leftpatch, leftiso;
  int iposition;

  int sliceflag = 0;
  int isoflag = 0;
  float *slicefactor = NULL;
  float slicefactor2[2];
  float *isofactor = NULL;

  int plot3dflag = 0;
  float *plot3dfactor = NULL;
  float plot3dfactor2[2];
  float plot3drange;

  int patchflag = 0;
  int zoneflag = 0;
  float *patchfactor = NULL;
  float *zonefactor = NULL;
  float patchrange = 0.0;
  float zonerange;

  int partflag = 0;
  float *partfactor = NULL;
  int dohist = 0;

  int fed_slice = 0;
  float colorbar_max, colorbar_eps;

  GLfloat *foreground_color, *red_color;

  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  char exp_factor_label[256];

  max_colorbar_label_width = 0.0;

  UpdateShowSliceColorbar(&showcfast_local, &show_slice_colorbar_local);

  colorbar_eps = pow(10.0, -ncolorlabel_digits);

  // -------------- compute columns where left labels will occur ------------

  leftsmoke = 0;
  leftslice = 0;
  lefthist = 0;
  leftpatch = 0;
  leftiso = 0;
  ileft = 0;
  if(showiso_colorbar == 1){
    leftiso = ileft;
    ileft++;
  }
  if(showevac_colorbar == 1 || showsmoke == 1){
    if(parttype != 0){
      leftsmoke = ileft;
      ileft++;
    }
  }
  if(show_slice_colorbar_local == 1){
    leftslice = ileft;
    ileft++;
    if(histogram_show_numbers == 1){
      lefthist = ileft;
      ileft++;
      dohist = 1;
    }
  }
  if(showpatch == 1 && wall_cell_color_flag == 0){
    leftpatch = ileft;
  }
  leftzone = ileft;

  foreground_color = &(foregroundcolor[0]);
  red_color = &(redcolor[0]);

  if(showiso_colorbar == 1 || showevac_colorbar == 1 ||
    (showsmoke == 1 && parttype != 0) || show_slice_colorbar_local == 1 ||
    (showpatch == 1 && wall_cell_color_flag == 0) ||
    showcfast_local==1 ||
    showplot3d == 1){

    SNIFF_ERRORS("before colorbar");
    CheckMemory;
    if(show_slice_colorbar_local==1){
      boundsdata *sb;

      sb = slicebounds + slicefile_labelindex;

      if(strcmp(sb->label->shortlabel, "FED") == 0){
        if(current_colorbar != NULL){
          strcpy(default_fed_colorbar, current_colorbar->label);
          if(strcmp(current_colorbar->label, "FED") == 0){
            fed_slice = 1;
            if(strcmp(sb->colorlabels[1], "0.00") != 0 || strcmp(sb->colorlabels[nrgb - 1], "3.00") != 0)fed_slice = 0;
          }
        }
      }
    }

    // -------------- draw plot3d colorbars ------------

    if(showplot3d == 1 && contour_type == STEPPED_CONTOURS){
    }
    else{

      // -------------- draw all other colorbars ------------

      if(show_fed_area == 1 && fed_slice == 1 && fed_areas != NULL){
        char area_label[256];
        char percen[] = "%";
        float yy;
        int *fed_areasi;

        fed_areasi = fed_areas + 4 * CLAMP(slice_time + 1, 1, nhists256_slice);

        glPushMatrix();
        glTranslatef(
          vcolorbar_left_pos,
          0.0,
          0.0);
        sprintf(area_label, "%i%s", fed_areasi[0], percen);
        yy = MIX2(0.15, 3.0, vcolorbar_top_pos, vcolorbar_down_pos) - VP_vcolorbar.text_height / 2;
        OutputBarText(0.0, yy, foreground_color, area_label);

        sprintf(area_label, "%i%s", fed_areasi[1], percen);
        yy = MIX2(0.65, 3.0, vcolorbar_top_pos, vcolorbar_down_pos) - VP_vcolorbar.text_height / 2;
        OutputBarText(0.0, yy, foreground_color, area_label);

        sprintf(area_label, "%i%s", fed_areasi[2], percen);
        yy = MIX2(2.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos) - VP_vcolorbar.text_height / 2;
        OutputBarText(0.0, yy, foreground_color, area_label);

        sprintf(area_label, "%i%s", fed_areasi[3], percen);
        yy = MIX2(3.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos) - VP_vcolorbar.text_height / 2;
        OutputBarText(0.0, yy + 10, foreground_color, area_label);
        glPopMatrix();
      }
      if(histogram_show_graph == 1 || histogram_show_numbers == 1){
        if(hists12_slice != NULL)DrawColorbarHistLabels(lefthist);
      }
    }
  }

  // -------------- isosurface left labels ------------

  if(showiso_colorbar == 1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float isorange;

    sb = isobounds + iisottype;
    tttmin = sb->levels256[0];
    tttmax = sb->levels256[255];
    isorange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftiso*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char isocolorlabel[256], isolabel[256];
      char *isocolorlabel_ptr = NULL;
      float vert_position;

      tttval = sb->levels256[valindex];
      Num2String(isolabel, tttval);
      isocolorlabel_ptr = isolabel;
      if(isoflag == 1){
        ScaleFloat2String(tttval, isocolorlabel, isofactor);
        isocolorlabel_ptr = isocolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, isocolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;
      char isocolorlabel[256];
      char *isocolorlabel_ptr = NULL;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      isocolorlabel_ptr = &(sb->colorlabels[i + 1][0]);
      if(isoflag == 1){
        float val;

        val = tttmin + i*isorange / (nrgb - 2);
        ScaleFloat2String(val, isocolorlabel, isofactor);
        isocolorlabel_ptr = isocolorlabel;
      }
      OutputBarText(0.0, vert_position, foreground_color, isocolorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- isosurface top labels ------------

  if(showiso_colorbar == 1){
    char unitlabel[256];
    boundsdata *sb;

    sb = isobounds + iisottype;
    strcpy(unitlabel, sb->label->unit);
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-leftiso*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Iso");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space),     foreground_color, unitlabel);
    glPopMatrix();
  }

  // -------------- particle left labels ------------

  if(showevac_colorbar == 1 || (showsmoke == 1 && parttype != 0)){
    float *partlevels256_ptr;
    float tttval;

    if(parttype!=0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel, &partunitclass, &partunittype);
      if(partunitclass>=0&&partunitclass<nunitclasses&&partunittype>=0){
        partflag = 1;
        partfactor = unitclasses[partunitclass].units[partunittype].scale;
      }
    }
    partlevels256_ptr = partlevels256;
    if(global_prop_index>= 0 &&global_prop_index < npart5prop){
      partlevels256_ptr = part5propinfo[global_prop_index].ppartlevels256;
    }

    iposition = -1;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftsmoke*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char partcolorlabel[256], *partcolorlabel_ptr = NULL, partlabel[256];
      float vert_position;

      tttval = partlevels256_ptr[valindex];
      Num2String(partlabel, tttval);
      partcolorlabel_ptr = partlabel;
      if(partflag == 1){
        ScaleFloat2String(tttval, partcolorlabel, partfactor);
        partcolorlabel_ptr = partcolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, partcolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = part5propinfo[global_prop_index].partlabelvals[i + 1];
      val = ScaleFloat2Float(val, partfactor);
      colorbar_vals[i] = val;
    }
    Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- particle file top labels ------------

  if(showevac_colorbar==1||(showsmoke==1&&parttype!=0)){
    char partunitlabel2[256], partshortlabel2[256];

    strcpy(partshortlabel2, "");
    strcpy(partunitlabel2, "");

    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    if(dohist == 1)glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);

    if(parttype != 0){
      if(showsmoke == 1 && showevac == 0)OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Part");
      if(showevac == 1)OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space),                   foreground_color, "Human");
    }
    if(parttype == -1){
      strcpy(partshortlabel2, "temp");
      strcpy(partunitlabel2, (const char *)degC);
    }
    else if(parttype == -2){
      strcpy(partshortlabel2, "HRRPUV");
      strcpy(partunitlabel2, "kW/m3");
    }
    else{
      if(partshortlabel != NULL)strcpy(partshortlabel2, partshortlabel);
      if(partunitlabel != NULL)strcpy(partunitlabel2,   partunitlabel);
    }
    if(parttype != 0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel, &partunitclass, &partunittype);
      if(partunitclass >= 0 && partunitclass < nunitclasses){
        if(partunittype >= 0){
          partflag = 1;
          partfactor = unitclasses[partunitclass].units[partunittype].scale;
          strcpy(partunitlabel2, unitclasses[partunitclass].units[partunittype].unit);
        }
      }
      OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space),     foreground_color, partshortlabel);
      OutputBarText(0.0, 1 * (VP_vcolorbar.text_height + v_space),     foreground_color, partunitlabel2);
      OutputBarText(0.0, 0 * (VP_vcolorbar.text_height + v_space),     foreground_color, exp_factor_label);
    }
    glPopMatrix();
  }

  // -------------- slice left labels ------------

  if(show_slice_colorbar_local==1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float slicerange;

    char unitlabel[256];
    int sliceunitclass, sliceunittype;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
        sliceflag = 1;
        slicefactor = unitclasses[sliceunitclass].units[sliceunittype].scale;
        strcpy(unitlabel, unitclasses[sliceunitclass].units[sliceunittype].unit);
      }
    }

    tttmin = sb->levels256[0];
    tttmax = sb->levels256[255];
    slicerange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftslice*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char slicelabel[256], slicecolorlabel[256];
      char *slicecolorlabel_ptr = NULL;
      float vert_position;
      int shifted_colorbar_index;

      tttval = sb->levels256[valindex];
      shifted_colorbar_index = global_colorbar_index;
      if(ABS(colorbar_shift-1.0)>0.0001){
        shifted_colorbar_index = SHIFT_VAL(global_colorbar_index, 0, 255, colorbar_shift);
      }
      Float2String(slicelabel, tttval, ncolorlabel_digits, force_fixedpoint);
      slicecolorlabel_ptr = slicelabel;
      if(sliceflag == 1){
        ScaleFloat2String(tttval, slicecolorlabel, slicefactor);
        slicecolorlabel_ptr = slicecolorlabel;
      }
      vert_position = MIX2(shifted_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(shifted_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, slicecolorlabel_ptr);
    }
    if(fed_slice == 1){
      for(i = 0; i < nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(0.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "0.00");

        vert_position = MIX2(0.3, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "0.30");

        vert_position = MIX2(1.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "1.00");

        vert_position = MIX2(3.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "3.00");
      }
    }
    else{
      float valmin, valmax;


      if(sliceflag==1){
        valmin = tttmin;
      }
      else{
        valmin = sb->colorvalues[1];
      }
      valmin = ScaleFloat2Float(valmin, slicefactor);

      if(sliceflag==1){
        valmax = tttmax;
      }
      else{
        valmax = sb->colorvalues[nrgb-1];
      }
      valmax = ScaleFloat2Float(valmax, slicefactor);
      colorbar_max = MAX(ABS(valmax), ABS(valmin));
      colorbar_max = MAX(colorbar_max, colorbar_eps);

      for(i = 0; i<nrgb-1; i++){
        float val;

        if(iposition==i)continue;
        if(sliceflag==1){
          val = tttmin+i*slicerange/(nrgb-2);
        }
        else{
          val = sb->colorvalues[i+1];
        }
        val = ScaleFloat2Float(val, slicefactor);
        if(ABS(colorbar_shift-1.0)>0.0001){
          val = SHIFT_VAL(val, valmin, valmax, 1.0/colorbar_shift);
        }
        colorbar_vals[i] = val;
      }
      Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
        if(iposition == i)continue;
        OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
        max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
      }
    }
    glPopMatrix();
  }

  // -------------- slice file top labels ------------

  if(show_slice_colorbar_local==1){
    char unitlabel[256];
    int sliceunitclass, sliceunittype;
    boundsdata *sb;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
        sliceflag = 1;
        slicefactor = unitclasses[sliceunitclass].units[sliceunittype].scale;
        strcpy(unitlabel, unitclasses[sliceunitclass].units[sliceunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-leftslice*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Slice");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    if(strcmp(unitlabel, "ppm") == 0 && slicefactor != NULL){
      slicefactor2[0] = *slicefactor;
      slicefactor2[1] = 0.0;
      slicefactor = slicefactor2;
    }
    glPopMatrix();
  }

  // -------------- boundary left labels ------------

  if(showpatch == 1 && wall_cell_color_flag == 0){
    float tttval, tttmin, tttmax;

    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = patchinfo + boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
        patchflag = 1;
        patchfactor = unitclasses[patchunitclass].units[patchunittype].scale;
        strcpy(unitlabel, unitclasses[patchunitclass].units[patchunittype].unit);
      }
    }

    iposition = -1;
    tttmin = boundarylevels256[0];
    tttmax = boundarylevels256[255];
    patchrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftpatch*(colorbar_label_width + h_space), 0.0, 0.0);
    if(dohist == 1)glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);
    if(global_colorbar_index != -1){
      char boundary_colorlabel[256], boundarylabel[256], *boundary_colorlabel_ptr = NULL;
      float vert_position;

      // draw boundary file value selected with mouse
      tttval = boundarylevels256[valindex];
      Num2String(boundarylabel, tttval);
      boundary_colorlabel_ptr = &(boundarylabel[0]);
      if(patchflag == 1){
        ScaleFloat2String(tttval, boundary_colorlabel, patchfactor);
        boundary_colorlabel_ptr = boundary_colorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, boundary_colorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      if(patchflag==1){
        val = tttmin+i*patchrange/(nrgb-2);
      }
      else{
        val = colorvaluespatch[i+1];
      }
      val = ScaleFloat2Float(val, patchfactor);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- boundary file top labels ------------

  if(showpatch == 1 && wall_cell_color_flag == 0){
    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = patchinfo + boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
        patchflag = 1;
        patchfactor = unitclasses[patchunitclass].units[patchunittype].scale;
        strcpy(unitlabel, unitclasses[patchunitclass].units[patchunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-leftpatch*(colorbar_label_width + h_space), 0.0, 0.0);
    if(dohist == 1)glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);

    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Bndry");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, patchi->label.shortlabel);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    glPopMatrix();
  }

  // -------------- zone left labels ------------

  if(showcfast_local==1){
    float tttval, tttmin, tttmax;

    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
        zoneflag = 1;
        zonefactor = unitclasses[zoneunitclass].units[zoneunittype].scale;
        strcpy(unitlabel, unitclasses[zoneunitclass].units[zoneunittype].unit);
      }
    }

    iposition = -1;
    tttmin = zonelevels256[0];
    tttmax = zonelevels256[255];
    zonerange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftzone*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char zonecolorlabel[256], *zonecolorlabel_ptr = NULL, zonelabel[256];
      float vert_position;

      tttval = zonelevels256[valindex];
      Num2String(zonelabel, tttval);
      zonecolorlabel_ptr = &(zonelabel[0]);
      if(zoneflag == 1){
        ScaleFloat2String(tttval, zonecolorlabel, zonefactor);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, zonecolorlabel_ptr);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(zonecolorlabel_ptr));
    }
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;
      char zonecolorlabel[256];
      char *zonecolorlabel_ptr = NULL;
      float val;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      if(zoneflag == 1){
        val = tttmin + (i - 1)*zonerange / (nrgb - 2);
      }
      else{
        val = colorvalueszone[i+1];
      }
      val = ScaleFloat2Float(val, zonefactor);
      Float2String(zonecolorlabel, val, ncolorlabel_digits, force_fixedpoint);
      zonecolorlabel_ptr = zonecolorlabel;
      OutputBarText(0.0, vert_position, foreground_color, zonecolorlabel_ptr);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(zonecolorlabel_ptr));
    }
    strcpy(exp_factor_label, "");
    SNIFF_ERRORS("after zone left labels");
    glPopMatrix();
  }

  // -------------- zone top labels ------------

  if(showcfast_local==1){
    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
        zoneflag = 1;
        zonefactor = unitclasses[zoneunitclass].units[zoneunittype].scale;
        strcpy(unitlabel, unitclasses[zoneunitclass].units[zoneunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, vcolorbar_top_pos + v_space + vcolorbar_delta, 0.0);
    glTranslatef(-leftzone*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Zone");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, "Temp");
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    glPopMatrix();
    SNIFF_ERRORS("After ZONE labels");
  }

  // -------------- plot3d left labels ------------

  if(showplot3d == 1){
    float *p3lev;
    float tttval, tttmin, tttmax;
    char *up3label;
    char unitlabel[256];
    int plot3dunitclass, plot3dunittype;

    up3label = plot3dinfo[0].label[plotn-1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass>=0&&plot3dunitclass<nunitclasses){
      if(plot3dunittype>0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }

    iposition = -1;
    p3lev = p3levels256[plotn - 1];
    tttmin = p3lev[0];
    tttmax = p3lev[255];
    plot3drange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    if(global_colorbar_index != -1){
      char plot3dcolorlabel[256], p3dlabel[256], *plot3dcolorlabel_ptr = NULL;
      float vert_position;

      tttval = p3lev[valindex];
      Num2String(p3dlabel, tttval);
      plot3dcolorlabel_ptr = p3dlabel;
      if(plot3dflag == 1){
        ScaleFloat2String(tttval, plot3dcolorlabel, plot3dfactor);
        plot3dcolorlabel_ptr = plot3dcolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, plot3dcolorlabel_ptr);
    }
    if(visiso == 0){
      for(i = 0; i < nrgb - 1; i++){
        float val;

        if(iposition == i)continue;
        if(plot3dflag == 1){
          val = tttmin + i*plot3drange / (nrgb - 2);
        }
        else{
          val = colorvaluesp3[plotn - 1][i];
        }
        val = ScaleFloat2Float(val, plot3dfactor);
        colorbar_vals[i] = val;
        GetMantissaExponent(ABS(val), colorbar_exponents + i);
      }
      Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

        if(iposition == i)continue;
        OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
        max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
      }
    }
    else{
      float vert_position;

      for(i = 0; i < nrgb - 2; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;

        vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

        if(iposition == i)continue;
        plot3dcolorlabel_ptr = &colorlabeliso[plotn - 1][i][0];
        if(plot3dflag == 1){
          float val;

          val = tttmin + (i - 1)*plot3drange / (nrgb - 2);
          ScaleFloat2String(val, plot3dcolorlabel, plot3dfactor);
          plot3dcolorlabel_ptr = plot3dcolorlabel;
        }
        if(isolevelindex == i || isolevelindex2 == i){
          OutputBarText(0.0, vert_position, red_color, plot3dcolorlabel_ptr);
        }
        else{
          OutputBarText(0.0, vert_position, foreground_color, plot3dcolorlabel_ptr);
        }
      }
    }
    glPopMatrix();
  }
  // -------------- plot3d top labels ------------

  if(showplot3d == 1){
    char *p3label;
    char *up3label;
    char unitlabel[256];
    int plot3dunitclass, plot3dunittype;

    up3label = plot3dinfo[0].label[plotn - 1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass >= 0 && plot3dunitclass < nunitclasses){
      if(plot3dunittype > 0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }
    p3label = plot3dinfo[0].label[plotn - 1].shortlabel;
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Plot3D");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, p3label);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    if(strcmp(unitlabel, "ppm") == 0 && plot3dfactor != NULL){
      plot3dfactor2[0] = *plot3dfactor;
      plot3dfactor2[1] = 0.0;
      plot3dfactor = plot3dfactor2;
    }
    glPopMatrix();
  }
}

/* ------------------ Rgb2Hsl ------------------------ */

void Rgb2Hsl(float *rgbvals, float *hslvals, int flag){
  // https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/
  float cmin, cmax, r, g, b;
  float luminance, saturation, hue;
  int maxmode;
  float cmaxmcmin;

  r = rgbvals[0];
  g = rgbvals[1];
  b = rgbvals[2];
  if(flag==1){
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;
  }

  cmin = MIN(r, MIN(g, b));
  cmax = MAX(r, MAX(g, b));
  cmaxmcmin = cmax-cmin;
  if(r>=MAX(g, b))maxmode = 0;
  if(g>=MAX(r, b))maxmode = 1;
  if(b>=MAX(r, g))maxmode = 2;

  luminance = (cmin+cmax)/2.0;
  saturation = 0.0;
  if(cmaxmcmin==0.0||luminance==0.0){
    saturation = 0.0;
    hue = 0.0;
  }
  else{
    if(luminance>0.0&&luminance<0.5){
      saturation = (cmax-cmin)/(2.0*luminance);
    }
    else if(luminance>=0.5){
      float denom;

      denom = 2.0-2.0*luminance;
      if(denom!=0.0){
        saturation = (cmax-cmin)/denom;
      }
      else{
        saturation = 1.0;
        hue = 0.0;
      }
    }
  }

  if(cmaxmcmin>0.0&&luminance!=0.0&&luminance!=1.0){
    if(maxmode==0){
      hue = (g-b)/cmaxmcmin;
    }
    else if(maxmode==1){
      hue = 2.0+(b-r)/cmaxmcmin;
    }
    else{
      hue = 4.0+(r-g)/cmaxmcmin;
    }
    hue *= 60.0;
    if(hue<0.0)hue += 360.0;
  }
  else{
    hue = 0.0;
  }
  hslvals[0] = hue;
  hslvals[1] = saturation;
  hslvals[2] = luminance;
}

/* ------------------ Hsl2Rgb ------------------------ */

void Hsl2Rgb(float *hslvals, float *rgbvals, int flag){
  // https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/

  float hue, saturation, luminance;
  float r, g, b;
//  float temp_1;
//  float  temp_2;
  float temp_r, temp_g, temp_b;

  hue = ABS(hslvals[0]);
  saturation = ABS(hslvals[1]);
  luminance = ABS(hslvals[2]);
  if(saturation==0.0){
    r = saturation;
    g = saturation;
    b = saturation;
    if(flag==1){
      r *= 255.0;
      g *= 255.0;
      b *= 255.0;
    }
    rgbvals[0] = r;
    rgbvals[1] = b;
    rgbvals[2] = g;
    return;
  }
  if(luminance<0.5){
//    temp_1 = luminance*(1.0+saturation);
  }
  else{
//    temp_1 = luminance+saturation-luminance*saturation;
  }
//  temp_2 = 2.0*luminance-temp_1;

  hue /= 360.0;

  temp_r = hue+1.0/3.0;
  if(temp_r<0.0)temp_r += 1.0;
  if(temp_r>1.0)temp_r -= 1.0;

  temp_g = hue;
  if(temp_g<0.0)temp_g += 1.0;
  if(temp_g>1.0)temp_g -= 1.0;

  temp_b = hue-1.0/3.0;
  if(temp_b<0.0)temp_b += 1.0;
  if(temp_b>1.0)temp_b -= 1.0;

}


