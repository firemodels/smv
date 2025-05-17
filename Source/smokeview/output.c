#include "options.h"
#ifdef WIN32
#ifdef __MINGW32__
#include <stddef.h>
#endif
#endif

#include GLUT_H
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"
#include "glutbitmap.h"
#include "paths.h"


#define DENORMAL(x,i, n, min,max) ((min) + (i)*((max)-(min))/(n))
#define NORMALH(x,min,max) (((x)-(min))/((max)-(min))   )

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
      assert(FFALSE);
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
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);

  x = (global_scase.xbar0+xbarORIG)/2.0;
  y = (global_scase.ybar0+ybarORIG)/2.0;
  z = (global_scase.zbar0+zbarORIG)/2.0;
  x0 = global_scase.xbar0 - SCALE2FDS(0.02);
  y0 = global_scase.ybar0 - SCALE2FDS(0.02);
  z0 = global_scase.zbar0 - SCALE2FDS(0.02);

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
  float scale_x, scale_y;

  if(string==NULL)return;
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

  if(sliceval_ndigits>0){
    Float2String(string, val, sliceval_ndigits, 0);
  }
  else{
    sprintf(string, "%f", val);
  }
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

/* ------------------ GetStringLength ------------------------ */

float GetStringLength(char *string){
  float length = 0.0;
  int i;

  if(string == NULL || strlen(string) == 0)return length;

  for(i = 0; i < strlen(string); i++){
    char *c;

    c = string + i;
    length += glutBitmapWidth(font_ptr, *c);
  }
  return length;
}

/* ------------------------ GetStringWidth ------------------------- */

int GetStringWidth(char *string){
  char *c;
  int length = 0;

  if(string == NULL)return 0;
  switch(fontindex){
  case SMALL_FONT:
    for(c = string; *c != '\0'; c++){
      length += glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, *c);
    }
    length *= (288.0 / 235.0);
#ifdef pp_OSX_HIGHRES
    if(double_scale == 1){
      length *= 2;
    }
#endif
    break;
  case LARGE_FONT:
    for(c = string; *c != '\0'; c++){
      length += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    }
    length *= (416.0 / 423.0);
#ifdef pp_OSX_HIGHRES
    if(double_scale == 1){
      length *= 2;
    }
#endif
    break;
  case SCALED_FONT:
    for(c = string; *c != '\0'; c++){
      length += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
    }
    length *= (283.0 / 402.0) * scale_2d_x;
    break;
  default:
    assert(FFALSE);
    break;
  }
  return length;
}

/* ------------------ Output3TextRight ------------------------ */

void Output3TextRight(float *color, float x, float y, float z, char *string, float pad_length){
  char *c;

  if(string == NULL)return;
  glColor3fv(color);

  if(fontindex == SCALED_FONT){
    ScaleFont3D();
    OutputSText3(x, y, z, string);
  }
  else{
    unsigned char blank;
    float blank_advance;
    int i;

    blank = ' ';
    glRasterPos3f(x, y, z);
    blank_advance = glutBitmapWidth(font_ptr, blank);
    int count;

    count = pad_length / blank_advance + 1;
    if(pad_length > 0.0){
      for(i = 0; i < count; i++){
        glBitmap(0,0,0.0,0.0,-blank_advance,0.0,NULL);
      }
    }
    for(i = 0; i<strlen(string); i++){
      c = string + i;
      glutBitmapCharacter(font_ptr, (unsigned char)*c);
    }
  }
}

void ScaleFont2D(void){
  if(render_mode == RENDER_360){
    glLineWidth(( float )resolution_multiplier * ( float )scaled_font2d_thickness);
  }
  else{
    glLineWidth(( float )scaled_font2d_thickness);
  }
}

/* ------------------ OutputTextColor ------------------------ */

void OutputTextColor(float *fontcolor, float x, float y, char *string){
  char *c;
  float *fcolor;

  if(string==NULL)return;
  if(fontcolor==NULL){
    fcolor = foregroundcolor;
  }
  else{
    fcolor = fontcolor;
  }
  glColor3fv(fcolor);
  if(fontindex==SCALED_FONT){
    ScaleFont2D();
    OutputSText2(x, y, 0.0, string);
    return;
  }
  glRasterPos2f(x, y);
  for(c = string; *c!='\0'; c++){
    glutBitmapCharacter(font_ptr, (unsigned char)*c);
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
  glRasterPos2f(x, y);
  for(c=string; *c!='\0'; c++){
    glutBitmapCharacter(font_ptr,(unsigned char)*c);
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

void WriteLabels(labels_collection *labelscoll_arg){
  labeldata *first_label, *thislabel;
  FILE *stream = NULL;
  char quote[2];

  if(event_file_exists==0)return;
  char *event_filename = CasePathEvent(&global_scase);
  stream = fopen(event_filename, "w");
  FREEMEMORY(event_filename);
  if(stream==NULL)return;

  first_label = labelscoll_arg->label_first_ptr;
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

void DrawLabels(labels_collection *labelscoll_arg){
  labeldata *first_label, *thislabel;

  first_label = labelscoll_arg->label_first_ptr;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
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

/* ----------------------- ScaleFont2D ----------------------------- */

/* ----------------------- ScaleFont3D ----------------------------- */

void ScaleFont3D(void){
  if(render_mode == RENDER_360){
    glLineWidth((float)resolution_multiplier*(float)scaled_font3d_thickness);
  }
  else{
    glLineWidth((float)scaled_font3d_thickness);
  }
}
