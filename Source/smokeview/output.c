#include "options.h"
#ifdef WIN32
#ifdef __MINGW32__
#include <stddef.h>
#endif
#endif

#include GLUT_H
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"

#define DENORMAL(x,i, n, min,max) ((min) + (i)*((max)-(min))/(n))
#define NORMALH(x,min,max) (((x)-(min))/((max)-(min))   )

/* ------------------ PrintTime ------------------------ */

void PrintTime(char *filepath, int line, float *timer, char *label){
  char *file;

  if(show_timings==0)return;
  file = strrchr(filepath, '\\');
  if(file==NULL)file = strrchr(filepath, '/');
  if(file==NULL){
    file = filepath;
  }
  else{
    file++;
  }
  if(*timer>0.0){
    if(strcmp(label, "null") != 0){
      STOP_TIMER(*timer);
      if(*timer>0.1){
        printf("%s/%i/%s %.1f s\n", file, line, label, *timer);
      }
    }
  }
  START_TIMER(*timer);
}

  /* ------------------ DrawHistogram ------------------------ */

#define MAXN 201
void DrawHistogram(histogramdata *histogram, float valmin, float valmax, float gmin, float gmax, int ndigits){
  float x[MAXN], y[MAXN], ymax, *buckets, valmin_normalized, valmax_normalized;
  int index[MAXN+1], i, n = MAXN;
  float blue[] = {0.0,0.0,1.0}, *color, *color_old = NULL, median_normalized;
  char cmin[20], cmedian[20], cmax[20], cvalmin[20], cvalmax[20];
  float cmin_width, cmax_width, median_width, cvalmin_width, cvalmax_width;
  float nmin, nmax, hmin, hmax;

  if(gmin<gmax){
    nmin = gmin;
    nmax = gmax;
  }
  else{
    nmin = histogram->val_min;
    nmax = histogram->val_max;
  }
  hmin = histogram->val_min;
  hmax = histogram->val_max;

  if(histogram==NULL||histogram->buckets==NULL||histogram->defined==0)return;
  if(histogram->val_max>histogram->val_min){
    valmin_normalized = NORMALH(valmin, nmin, nmax);
    valmax_normalized = NORMALH(valmax, nmin, nmax);
    median_normalized = NORMALH(histogram->median, nmin, nmax);
  }
  Float2String(cmin, gmin, ndigits, force_fixedpoint);
  Float2String(cmedian, histogram->median, ndigits, force_fixedpoint);
  Float2String(cmax, gmax, ndigits, force_fixedpoint);
  Float2String(cvalmin, valmin, ndigits, force_fixedpoint);
  Float2String(cvalmax, valmax, ndigits, force_fixedpoint);
  cmin_width = (float)GetStringWidth(cmin)/screenWidth;
  cmax_width = (float)GetStringWidth(cmax)/screenWidth;
  median_width = (float)GetStringWidth(cmedian)/screenWidth;
  cvalmin_width = (float)GetStringWidth(cvalmin)/screenWidth;
  cvalmax_width = (float)GetStringWidth(cvalmax)/screenWidth;

  for(i = 0; i<n; i++){
    x[i] = DENORMAL((float)i*(float)(n-1), i, n-1, hmin, hmax);
    x[i] = NORMALH(x[i], nmin, nmax);
    index[i] = (float)i*(float)histogram->nbuckets/(float)n;
  }
  index[n] = histogram->nbuckets;

  buckets = histogram->buckets;
  ymax = 0.0;
  for(i = 0; i<n; i++){
    int j;

    y[i] = 0.0;
    for(j = index[i]; j<index[i+1]; j++){
      y[i] += buckets[j];
    }
    ymax = MAX(ymax, y[i]);
  }
  for(i = 0; i<n; i++){
    y[i] /= ymax;
  }

  glPushMatrix();
  glBegin(GL_TRIANGLES);
  for(i = 0; i<n-1; i++){
    float x1, x2;

    x1 = x[i];
    x2 = x[i+1];
    if(x1<0.0||x2>1.0)continue;

    if(valmin_normalized<=valmax_normalized&&(x1<valmin_normalized||x2>valmax_normalized)){
      color = blue;
    }
    else{
      color = foregroundcolor;
    }
    if(color_old!=color){
      glColor3fv(color);
      color_old = color;
    }

    glVertex2f(x1, 0.0);
    glVertex2f(x2, 0.0);
    glVertex2f(x2, y[i+1]);

    glVertex2f(x1, 0.0);
    glVertex2f(x2, y[i+1]);
    glVertex2f(x2, 0.0);

    glVertex2f(x1, 0.0);
    glVertex2f(x2, y[i+1]);
    glVertex2f(x1, y[i]);

    glVertex2f(x1, 0.0);
    glVertex2f(x1, y[i]);
    glVertex2f(x2, y[i+1]);
  }
  glEnd();

#define DZHIST1 0.025

  glColor3fv(foregroundcolor);
  glBegin(GL_LINES);
  glVertex2f(0.0, 0.0);
  glVertex2f(0.0, -0.02);
  glVertex2f(median_normalized, 0.0);
  glVertex2f(median_normalized, -DZHIST1);
  glVertex2f(1.0, 0.0);
  glVertex2f(1.0, -DZHIST1);

  glVertex2f(0.0, 0.0);
  glVertex2f(1.0, 0.0);

  glColor3fv(blue);
  glVertex2f(valmin_normalized, 0.0);
  glVertex2f(valmin_normalized, -DZHIST1);
  glVertex2f(valmax_normalized, 0.0);
  glVertex2f(valmax_normalized, -DZHIST1);
 glEnd();

  if(hist_show_labels==1){
    float text_height;

    text_height = (float)GetFontHeight();
    text_height += 3.0;
    text_height /= pixel_dens;

    float offset = -2.0*text_height;

    OutputTextColor(foregroundcolor, -cmin_width/2.0,                                               offset, cmin);
    OutputTextColor(foregroundcolor, 0.001+MAX(median_normalized-median_width/2.0, cmin_width/2.0), offset, cmedian);
    OutputTextColor(foregroundcolor, 1.0-cmax_width/2.0,                                            offset, cmax);

    offset -= text_height;
    OutputTextColor(blue, valmin_normalized-cvalmin_width/2.0, offset, cvalmin);
    OutputTextColor(blue, valmax_normalized-cvalmax_width/2.0, offset, cvalmax);

    if(histogram_label1!=NULL){
      offset -= text_height;
      OutputTextColor(foregroundcolor, -cmin_width/2.0, offset, histogram_label1);
    }
    if(histogram_label2!=NULL){
      offset -= text_height;
      OutputTextColor(foregroundcolor, -cmin_width/2.0, offset, histogram_label2);
    }
  }
  glPopMatrix();
}

/* ------------------------ GetFontHeight ------------------------- */

int GetFontHeight(void){
  int height = 0;

  switch(fontindex){
    case SMALL_FONT:
      height = 10;
      break;
    case LARGE_FONT:
      height = 18;
      break;
    case SCALED_FONT:
        height = glutStrokeWidth(GLUT_STROKE_ROMAN, 'A');
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return height;
}

/* ------------------ OutputAxisLabels ------------------------ */

void OutputAxisLabels(){
  float x, y, z;
  float x0, y0, z0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  x = (xbar0+xbarORIG)/2.0;
  y = (ybar0+ybarORIG)/2.0;
  z = (zbar0+zbarORIG)/2.0;
  x0 = xbar0 - SCALE2FDS(0.02);
  y0 = ybar0 - SCALE2FDS(0.02);
  z0 = zbar0 - SCALE2FDS(0.02);

  Output3Text(foregroundcolor,   x,y0, z0, "X");
  Output3Text(foregroundcolor, x0,  y, z0, "Y");
  Output3Text(foregroundcolor, x0,y0,   z, "Z");

  glPopMatrix();
}

/* ------------------ OutputSText3 ------------------------ */

void OutputSText3(float x, float y, float z, char *string){
  char *c;
  float u[3]={0.0,0.0,1.0},v[3];
  float axis[3],angle,theta;
  float quateye[4],quatz[4],rot[16];
  float scale_x, scale_y;


  if(string==NULL)return;
  scale_x = SCALE2FDS(scaled_font3d_height2width*(float)scaled_font3d_height/(float)104.76)/(float)port_pixel_width;
  scale_y = SCALE2FDS((float)scaled_font3d_height/(float)152.38)/(float)port_pixel_height;
  glPushMatrix();
  glTranslatef(x,y,z);
  v[0]=fds_eyepos[0]-x;
  v[1]=fds_eyepos[1]-y;
  v[2]=fds_eyepos[2]-z;
  RotateU2V(u,v,axis,&angle);
  theta=atan2(v[0],-v[1])*RAD2DEG;
  AngleAxis2Quat(theta*DEG2RAD,u,quatz);
  AngleAxis2Quat(angle,axis,quateye);
  MultQuat(quateye,quatz,quateye);
  Quat2Rot(quateye,rot);

  glRotatef(90.0,cos(theta*DEG2RAD),sin(theta*DEG2RAD),0.0);
  glRotatef(theta,u[0],u[1],u[2]);

  glScalef(scale_x,scale_y,1.0);
  for(c=string; *c != '\0'; c++){
    glutStrokeCharacter(GLUT_STROKE_ROMAN,*c);
  }
  glPopMatrix();
}


/* ------------------ OutputSText2r ------------------------ */

void OutputSText2r(float x, float y, float z, char *string){
  char *c;
  int total_width=0;
  float scale_x, scale_y;

  if(string==NULL)return;
  total_width=0;
  for(c=string; *c != '\0'; c++){
    total_width+=glutStrokeWidth(GLUT_STROKE_ROMAN,*c);
  }
  glPushMatrix();
  scale_x = port_unit_width*(scaled_font2d_height2width*(float)scaled_font2d_height/(float)104.76)/(float)port_pixel_width;
  scale_y = port_unit_height*((float)scaled_font2d_height/(float)152.38)/(float)port_pixel_height;
  if(render_mode==RENDER_NORMAL&&resolution_multiplier>1&&render_status==RENDER_ON){
    scale_x *= (float)resolution_multiplier;
    scale_y *= (float)resolution_multiplier;
  }
  glTranslatef(x-scale_x*total_width,y,z);
  glScalef(scale_x,scale_y,1.0);
  for(c=string; *c != '\0'; c++){
    glutStrokeCharacter(GLUT_STROKE_ROMAN,*c);
  }
  glPopMatrix();
}

/* ------------------ OutputSText2 ------------------------ */

void OutputSText2(float x, float y, float z, char *string){
  char *c;
  int total_width=0;
  float scale_x, scale_y;

  if(string==NULL)return;
  total_width=0;
  for(c=string; *c != '\0'; c++){
    total_width+=glutStrokeWidth(GLUT_STROKE_ROMAN,*c);
  }
  glPushMatrix();
  scale_x = (25.0/36.0)*port_unit_width*(scaled_font2d_height2width*(float)scaled_font2d_height/(float)104.76)/(float)port_pixel_width;
  scale_y = (12.0/18.0)*(25.0/18.0)*port_unit_height*((float)scaled_font2d_height/(float)152.38)/(float)port_pixel_height;
  if(render_mode == RENDER_NORMAL&&resolution_multiplier > 1&&render_status==RENDER_ON){
    scale_x *= (float)resolution_multiplier;
    scale_y *= (float)resolution_multiplier;
  }
  glTranslatef(x,y,z);
  glScalef(scale_x,scale_y,1.0);
  glTranslatef(0.0,25.0,0.0);
  for(c=string; *c != '\0'; c++){
    glutStrokeCharacter(GLUT_STROKE_ROMAN,*c);
  }
  glPopMatrix();
}

/* ------------------ Output3Val ------------------------ */

void Output3Val(float x, float y, float z, float val){
  char string[256];

  sprintf(string,"%f",val);
  TrimZeros(string);
  Output3Text(foregroundcolor,x,y,z,string);
}

/* ------------------ Output3Text ------------------------ */

void Output3Text(float *color, float x, float y, float z, char *string){
  char *c;

  if(string==NULL)return;
  glColor3fv(color);

  if(fontindex==SCALED_FONT){
    ScaleFont3D();
    OutputSText3(x,y,z,string);
  }
  else{
    glRasterPos3f(x, y, z);
    for(c=string; *c!='\0'; c++){
      glutBitmapCharacter(font_ptr,(unsigned char)*c);
    }
  }
}

/* ------------------ OutputLargeText ------------------------ */

void OutputLargeText(float x, float y, char *string){
  char *c;

  if(string==NULL)return;
  glColor3fv(foregroundcolor);
  glRasterPos2f(x, y);
  for(c=string; *c!='\0'; c++){
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,(unsigned char)*c);
  }
}

/* ------------------ OutputTextColor ------------------------ */

void OutputTextColor(float *fontcolor, float x, float y, char *string){
  char *c;
  float *fcolor;

  fcolor = foregroundcolor;
  if(fontcolor==NULL){
    fcolor = foregroundcolor;
  }
  else{
    fcolor = fontcolor;
  }

  if(string==NULL)return;
  glColor3fv(fcolor);
  if(fontindex==SCALED_FONT){
    ScaleFont2D();
    OutputSText2(x, y, 0.0, string);
    return;
  }
  else{
    glRasterPos2f(x, y);
    for(c = string; *c!='\0'; c++){
      glutBitmapCharacter(font_ptr, (unsigned char)*c);
    }
  }
}

/* ------------------ OutputText ------------------------ */

void OutputText(float x, float y, char *string){
  char *c;

  if(string==NULL)return;
  glColor3fv(foregroundcolor);
  if(fontindex==SCALED_FONT){
    ScaleFont2D();
    OutputSText2(x,y,0.0,string);
    return;
  }
  else{
    glRasterPos2f(x, y);
    for(c=string; *c!='\0'; c++){
      glutBitmapCharacter(font_ptr,(unsigned char)*c);
    }
  }
}

/* ------------------ OutputBarText ------------------------ */

void OutputBarText(float x, float y, const GLfloat *color, char *string){
  char *c;

  if(string==NULL)return;
  glColor3fv(color);

  if(fontindex==SCALED_FONT){
    ScaleFont2D();
    OutputSText2(x,y,0.0,string);
  }
  else{
    glRasterPos2f(x, y);
    for(c=string; *c!='\0'; c++){
      glutBitmapCharacter(colorbar_font_ptr,(unsigned char)(*c));
    }
  }
}

/* ------------------ WriteLabels ------------------------ */

void WriteLabels(void){
  labeldata *first_label, *thislabel;
  FILE *stream = NULL;
  char quote[2];

  if(event_file_exists==0)return;
  stream = fopen(event_filename, "w");
  if(stream==NULL)return;

  first_label = label_first_ptr;
  strcpy(quote,"\"");

  for(thislabel = first_label->next; thislabel->next!=NULL; thislabel = thislabel->next){
    float *tstart_stop, *xyz;
    int *rgblabel;

    tstart_stop = thislabel->tstart_stop;
    xyz = thislabel->xyz;
    rgblabel = thislabel->rgb;
    fprintf(stream, "%f, %f, %f, %f, %f, %i, %i, %i, %s%s%s\n",
            tstart_stop[0], tstart_stop[1],
            xyz[0], xyz[1], xyz[2],
            rgblabel[0], rgblabel[1], rgblabel[2],
            quote,TrimFrontBack(thislabel->name),quote
    );
  }
  fclose(stream);
}

/* ------------------ DrawLabels ------------------------ */

void DrawLabels(void){
  labeldata *first_label, *thislabel;

  first_label = label_first_ptr;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);
  for(thislabel=first_label->next;thislabel->next!=NULL;thislabel=thislabel->next){
    float *labelcolor,*tstart_stop,*xyz;
    int drawlabel;

    drawlabel=0;
    tstart_stop=thislabel->tstart_stop;
    xyz=thislabel->xyz;
    if(thislabel->useforegroundcolor==1){
      labelcolor=foregroundcolor;
    }
    else{
      labelcolor=thislabel->frgb;
    }
    if(plotstate!=DYNAMIC_PLOTS||thislabel->show_always==1||showtime==0)drawlabel=1;
    if(drawlabel==0&&plotstate==DYNAMIC_PLOTS&&showtime==1){
      if(tstart_stop[0]<0.0||tstart_stop[1]<0.0)drawlabel=1;
      if(drawlabel==0&&global_times[itimes]>=tstart_stop[0]-0.05&&global_times[itimes]<=tstart_stop[1]+0.05)drawlabel=1;
    }
    if(drawlabel==1){
      Output3Text(labelcolor,xyz[0],xyz[1],xyz[2],thislabel->name);
      if(thislabel->show_tick==1){
        float *xyztick, *xyztickdir;
        float xb[3], xe[3];
        int i;

        xyztick = thislabel->tick_begin;
        xyztickdir = thislabel->tick_direction;
        for(i = 0; i<3; i++){
          xb[i] = xyz[i]+xyztick[i];
          xe[i] = xb[i]+xyztickdir[i];
        }
        AntiAliasLine(ON);
        glLineWidth(ticklinewidth);
        glBegin(GL_LINES);
        glVertex3fv(xb);
        glVertex3fv(xe);
        glEnd();
        AntiAliasLine(OFF);
      }
    }
  }
  glPopMatrix();
}

/* ------------------ LabelNext ------------------------ */

labeldata *LabelNext(labeldata *label){
  labeldata *thislabel;

  if(label==NULL)return NULL;
  if(label_first_ptr->next->next==NULL)return NULL;
  for(thislabel=label->next;thislabel!=label;thislabel=thislabel->next){
    if(thislabel->next==NULL)thislabel=label_first_ptr->next;
    if(thislabel->labeltype==TYPE_SMV)continue;
    return thislabel;
  }
  return NULL;
}

/* ------------------ LabelPrevious ------------------------ */

labeldata *LabelPrevious(labeldata *label){
  labeldata *thislabel;

  if(label==NULL)return NULL;
  if(label_last_ptr->prev->prev==NULL)return NULL;
  for(thislabel=label->prev;thislabel!=label;thislabel=thislabel->prev){
    if(thislabel->prev==NULL)thislabel=label_last_ptr->prev;
    if(thislabel->labeltype==TYPE_SMV)continue;
    return thislabel;
  }
  return NULL;
}

/* ------------------ LabelInit ------------------------ */

int LabelInit(labeldata *gl){
  labeldata *thislabel;

  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    if(thislabel->labeltype==TYPE_SMV)continue;
    LabelCopy(gl,thislabel);
    return 1;
  }
  return 0;
}

/* ------------------ LabelGetNUserLabels ------------------------ */

int LabelGetNUserLabels(void){
  int count=0;
  labeldata *thislabel;

  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    if(thislabel->labeltype==TYPE_INI)count++;
  }
  return count;
}

/* ------------------ LabelGet ------------------------ */

labeldata *LabelGet(char *name){
  labeldata *thislabel;

  if(name==NULL)return NULL;
  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    if(strcmp(thislabel->name,name)==0)return thislabel;
  }
  return NULL;
}

/* ------------------ LabelInsertBefore ------------------------ */

void LabelInsertBefore(labeldata *listlabel, labeldata *label){
  labeldata *prev, *next;

  prev        = listlabel->prev;
  next        = listlabel;
  prev->next  = label;
  next->prev  = label;
  label->prev = prev;
  label->next = next;
}

/* ------------------ LabelDelete ------------------------ */

void LabelDelete(labeldata *label){
  labeldata *prev, *next;

  prev = label->prev;
  next =label->next;
  CheckMemory;
  FREEMEMORY(label);
  prev->next=next;
  next->prev=prev;
}

/* ------------------ LabelCopy ------------------------ */

void LabelCopy(labeldata *label_to, labeldata *label_from){
  labeldata *prev, *next;

  prev=label_to->prev;
  next=label_to->next;
  memcpy(label_to,label_from,sizeof(labeldata));
  label_to->prev=prev;
  label_to->next=next;

}

/* ------------------ LabelResort ------------------------ */

void LabelResort(labeldata *label){
  labeldata labelcopy;

  CheckMemory;
  memcpy(&labelcopy,label,sizeof(labeldata));
  CheckMemory;
  LabelDelete(label);
  LabelInsert(&labelcopy);
}

/* ------------------ LabelInsertAfter ------------------------ */

void LabelInsertAfter(labeldata *listlabel, labeldata *label){
  labeldata *prev, *next;

  prev        = listlabel;
  next        = listlabel->next;
  prev->next  = label;
  next->prev  = label;
  label->prev = prev;
  label->next = next;
}

/* ------------------ LabelPrint ------------------------ */

void LabelPrint(void){
  labeldata *thislabel;
  float *xyz;

  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    xyz = thislabel->xyz;
    PRINTF("label: %s position: %f %f %f\n",thislabel->name,xyz[0],xyz[1],xyz[2]);
  }
}

/* ------------------ LabelInsert ------------------------ */

labeldata *LabelInsert(labeldata *labeltemp){
  labeldata *newlabel, *thislabel;
  labeldata *firstuserptr, *lastuserptr;

  NewMemory((void **)&newlabel,sizeof(labeldata));
  memcpy(newlabel,labeltemp,sizeof(labeldata));

  thislabel = LabelGet(newlabel->name);
  if(thislabel!=NULL){
    LabelInsertAfter(thislabel->prev,newlabel);
    return newlabel;
  }

  firstuserptr=label_first_ptr->next;
  if(firstuserptr==label_last_ptr)firstuserptr=NULL;

  lastuserptr=label_last_ptr->prev;
  if(lastuserptr==label_first_ptr)lastuserptr=NULL;

  if(firstuserptr!=NULL&&strcmp(newlabel->name,firstuserptr->name)<0){
    LabelInsertBefore(firstuserptr,newlabel);
    return newlabel;
  }
  if(lastuserptr!=NULL&&strcmp(newlabel->name,lastuserptr->name)>0){
    LabelInsertAfter(lastuserptr,newlabel);
    return newlabel;
  }
  if(firstuserptr==NULL&&lastuserptr==NULL){
    LabelInsertAfter(label_first_ptr,newlabel);
    return newlabel;
  }
  for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
    labeldata *nextlabel;

    nextlabel=thislabel->next;
    if(strcmp(thislabel->name,newlabel->name)<0&&strcmp(newlabel->name,nextlabel->name)<0){
      LabelInsertAfter(thislabel,newlabel);
      return newlabel;
    }
  }
  return NULL;
}

/* ----------------------- ScaleFont2D ----------------------------- */

void ScaleFont2D(void){
  if(render_mode == RENDER_360){
    glLineWidth((float)resolution_multiplier*(float)scaled_font2d_thickness);
  }
  else{
    glLineWidth((float)scaled_font2d_thickness);
  }
}

/* ----------------------- ScaleFont3D ----------------------------- */

void ScaleFont3D(void){
  if(render_mode == RENDER_360){
    glLineWidth((float)resolution_multiplier*(float)scaled_font3d_thickness);
  }
  else{
    glLineWidth((float)scaled_font3d_thickness);
  }
}

