#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "infoheader.h"

#define CONV(p,pl,pr,pxl,pxr) ( (pxl) + ((pxr)-(pxl))*((p)-(pl))/((pr)-(pl)) )

/* ------------------------ GetStringWidth ------------------------- */

int GetStringWidth(char *string){
  char *c;
  int length=0;

  if(string==NULL)return 0;
  switch(fontindex){
    case SMALL_FONT:
      length = strlen(string);
      length *= (288.0/235.0)*glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, 'a');
      break;
    case LARGE_FONT:
      length = strlen(string);
      length *= (416.0/423.0)*glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, 'a');
      break;
    case SCALED_FONT:
      for(c=string;*c!='\0';c++){
        length += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
      }
      length *= (283.0/402.0)*scale_2d_x;
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
  return length;
}

/* ------------------------ GetViewportInfo ------------------------- */

void GetViewportInfo(void){
  int doit;
  float text_height;
  float text_width;
  int ninfo_lines=0;
  int info_width;
  int dohist=0;
  int hbar_height;

  info_width = GetStringWidth("y: 115, 11.5 m");
  colorbar_label_width = GetStringWidth("*10^-02");

  v_space = 2;
  text_height=18;
  text_width=18;
  if(fontindex==SCALED_FONT){
    scale_2d_x = (scaled_font2d_height2width*(float)scaled_font2d_height/(float)104.76);
    scale_2d_y = ((float)scaled_font2d_height/(float)152.38);

    text_height = MAX(18,(int)( (12.0/18.0)*(25.0/18.0)*(float)scaled_font2d_height));
    text_width =  MAX(18, (25.0/36.0)*(scaled_font2d_height2width*(float)scaled_font2d_height));
  }

  // full screen viewport dimensions

  VP_fullscreen.left = 0;
  VP_fullscreen.down = 0;
  VP_fullscreen.width = screenWidth;
  VP_fullscreen.height = screenHeight;
  VP_fullscreen.right = VP_fullscreen.left + VP_fullscreen.width;
  VP_fullscreen.top = VP_fullscreen.down + VP_fullscreen.height;
  VP_info.doit = 1;

  // INFO viewport dimensions

  doit=0;
  if(visMeshlabel==1){
    ninfo_lines++;
    doit=1;
  }
  if(((showplot3d==1||visGrid!=noGridnoProbe)&&visx_all==1)||visGrid==noGridProbe||visGrid==GridProbe){
    if(visgridloc==1){
      ninfo_lines++;
      doit=1;
    }
  }
  if(((showplot3d==1||visGrid!=noGridnoProbe)&&visy_all==1)||visGrid==GridProbe||visGrid==noGridProbe){
    if(visgridloc==1){
      ninfo_lines++;
      doit=1;
    }
  }
  if(((showplot3d==1||visGrid!=noGridnoProbe)&&visz_all==1)||visGrid==GridProbe||visGrid==noGridProbe){
    if(visgridloc==1){
      ninfo_lines++;
      doit=1;
    }
  }

  VP_info.left = screenWidth-info_width-titlesafe_offset;
  VP_info.down = titlesafe_offset;
  VP_info.doit = doit;
  VP_info.text_height = text_height;
  VP_info.text_width = text_width;
  if(doit==1){
    VP_info.width = info_width;
    VP_info.height = ninfo_lines*(text_height+v_space);
  }
  else{
    VP_info.width = 0;
    VP_info.height = 0;
  }
  VP_info.right = VP_info.left + VP_fullscreen.width;
  VP_fullscreen.top = VP_fullscreen.down + VP_info.height;

  // timebar viewport dimensions

  doit=0;
  if(showtime==1){
    if(visTimelabel == 1 || visFramelabel == 1 || visHRRlabel == 1 || visTimebar == 1)doit=1;
    if(doit==0&&show_firecutoff==1&&current_mesh!=NULL){
      if(hrrpuv_loaded==1||temp_loaded==1)doit=1;
    }
    if(doit==0&&visFramerate==1)doit=1;
    if(doit==0&&vis_slice_average==1&&show_slice_average&&slice_average_flag==1)doit=1;
  }
  if(visColorbarHorizontal == 1){
    doit = 1;
  }
#ifdef pp_memstatus
  if(doit==0&&visAvailmemory==1)doit=1;
#endif

  VP_timebar.left = titlesafe_offset;
  VP_timebar.down = titlesafe_offset;
  VP_timebar.doit=doit;
  VP_timebar.text_height = text_height;
  VP_timebar.text_width  = text_width;
  hbar_height = text_height + v_space+MAX(hcolorbar_delta, 3 * (text_height + v_space));
  if(doit==1){
    VP_timebar.width = screenWidth-VP_info.width-2*titlesafe_offset;
    VP_timebar.height=2*(text_height+v_space);
    if(show_firecutoff==1 && current_mesh != NULL){
      if(hrrpuv_loaded == 1||temp_loaded == 1)VP_timebar.height += (text_height + v_space);
    }
    if(visColorbarHorizontal==1){
      VP_timebar.height += hbar_height;
    }
  }
  else{
    VP_timebar.width = 0;
    VP_timebar.height = 0;
  }
  VP_timebar.right = VP_timebar.left + VP_timebar.width;
  VP_timebar.top   = VP_timebar.down + VP_timebar.height;

  // vertical colorbar viewport dimensions

  doit=1;
  if(showslice==1||(showvslice==1&&vslicecolorbarflag==1)){
    if(histogram_show_graph == 1 || histogram_show_numbers == 1){
      if(hists12_slice!=NULL)dohist=1;
    }
  }

  if(visColorbarVertical==0||num_colorbars==0||(showtime==0&&showplot3d==0))doit=0;
  VP_vcolorbar.left = screenWidth-vcolorbar_delta - num_colorbars*(colorbar_label_width+2*h_space)-titlesafe_offset;
  if(dohist==1){
    VP_vcolorbar.left -= colorbar_label_width;
  }
  VP_vcolorbar.down = MAX(VP_timebar.height,VP_info.height)+titlesafe_offset;
  VP_vcolorbar.doit = doit;
  VP_vcolorbar.text_height = text_height;
  VP_vcolorbar.text_width  = text_width;
  if(doit==1){
    VP_vcolorbar.width = vcolorbar_delta + h_space+num_colorbars*(colorbar_label_width+h_space);
    if(dohist==1){
      VP_vcolorbar.width += colorbar_label_width;
    }
    VP_vcolorbar.height = screenHeight-MAX(VP_timebar.height,VP_info.height)-2*titlesafe_offset;

  }
  else{
    VP_vcolorbar.width = 0;
    VP_vcolorbar.height = 0;
  }
  VP_vcolorbar.right = VP_vcolorbar.left+VP_vcolorbar.width;
  VP_vcolorbar.top = VP_vcolorbar.down+VP_vcolorbar.height;

  // title viewport dimensions
  titleinfo.left_margin = 0;
  titleinfo.top_margin = 0;
  titleinfo.bottom_margin = 5;
  titleinfo.line_space = 0;
  titleinfo.text_height = text_height;

  // set the correct dimensions for the view point based on the list of strings
  // we want to print and the spacing information
  // only do this if title is set
  if(visTitle==1){
    // add the margins
    VP_title.height=titleinfo.top_margin+titleinfo.bottom_margin;
    // count the lines first, then add space after
    int nlinestotal = 0;
    // first add the space for the hard coded lines if necessary
    if(visTitle==1){
      nlinestotal++;
    }
    if(gversion==1){
      nlinestotal++;
    }
    if(gversion==1&&(strlen(titleinfo.fdsbuildline)>0)){
      nlinestotal++;
    }
    if(visCHID==1){
      nlinestotal++;
    }
    nlinestotal += titleinfo.nlines;
    if(nlinestotal==0){
      // if there is no information to be displayed, set everything to zero
      VP_title.width = 0;
      VP_title.height = 0;
      VP_title.doit = 0;
    } else{
      // add the space for each line
      // one fewer spacings are needed as they only go between each line
      VP_title.height += nlinestotal*titleinfo.text_height +
                         (nlinestotal-1)*titleinfo.line_space;
      VP_title.doit = 1;
      VP_title.width = screenWidth-VP_vcolorbar.width-2*titlesafe_offset;
    }

  } else{
    VP_title.width = 0;
    VP_title.height = 0;
    VP_title.doit = 0;
  }

  VP_title.text_height = text_height;
  VP_title.text_width  = text_width;
  VP_title.left = titlesafe_offset;
  VP_title.down = (int)screenHeight-VP_title.height-titlesafe_offset;
  VP_title.right = VP_title.left + VP_title.width;
  VP_title.top = VP_title.down + VP_title.height;

  // scene viewport dimensions

  {
    int timebar_height;

    timebar_height = MAX(VP_timebar.height, VP_info.height);
    if(timebar_overlap == TIMEBAR_OVERLAP_ALWAYS)timebar_height = 0;
    if(timebar_overlap==TIMEBAR_OVERLAP_AUTO&&visTimebar==0&&visColorbarHorizontal==0)timebar_height = 0;
    VP_scene.text_height = text_height;
    VP_scene.text_width = text_width;
    VP_scene.left = titlesafe_offset;
    VP_scene.down = titlesafe_offset + timebar_height;
    VP_scene.width = MAX(1, screenWidth - 2 * titlesafe_offset - VP_vcolorbar.width);
    if(dohist == 1)VP_scene.width += colorbar_label_width / 2;
    VP_scene.height = MAX(1, screenHeight - timebar_height - VP_title.height - 2 * titlesafe_offset);
    VP_scene.right = VP_scene.left + VP_scene.width;
    VP_scene.top = VP_scene.down + VP_scene.height;
  }

  scene_aspect_ratio = (float)VP_scene.height/(float)VP_scene.width;

  // vertical colorbar boundaries

  vcolorbar_right_pos = VP_vcolorbar.right  - h_space;
  vcolorbar_left_pos  = vcolorbar_right_pos - vcolorbar_delta;
  vcolorbar_top_pos   = VP_vcolorbar.top - 4*(v_space + VP_vcolorbar.text_height) - vcolorbar_delta;
  vcolorbar_down_pos  = VP_vcolorbar.down + vcolorbar_delta;

  // horizontal colorbar boundaries

  hcolorbar_right_pos = VP_timebar.right - colorbar_label_width-hcolorbar_delta;
  hcolorbar_left_pos  = VP_timebar.left  + colorbar_label_width;
  hcolorbar_down_pos  = VP_timebar.top - hbar_height + (text_height + v_space);
  hcolorbar_top_pos   = hcolorbar_down_pos + hcolorbar_delta;
}

 /* ------------------------ SubPortOrtho ------------------------- */

int SubPortOrtho(int quad,
                  portdata *p,
                   GLdouble portx_left, GLdouble portx_right, GLdouble portx_down, GLdouble portx_top,
                   GLint screen_left, GLint screen_down
                   ){

  GLint subport_left, subport_right, subport_down, subport_top;
  GLdouble subportx_left, subportx_right, subportx_down, subportx_top;
  GLsizei subport_width, subport_height;
  GLint subwindow_left, subwindow_right, subwindow_down, subwindow_top;
  GLint port_right, port_top;

  int irow, icol;

  switch(quad){
  case 0:
    port_pixel_width = p->width;
    port_pixel_height = p->height;
    port_unit_width = portx_right - portx_left;
    port_unit_height = portx_top - portx_down;
    glViewport(p->left,p->down,p->width,p->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(portx_left,portx_right,portx_down,portx_top);
    return 1;
  case 1:
    icol = screen_left/screenWidth;
    irow = screen_down/screenHeight;

    subwindow_left = icol*screenWidth;
    subwindow_right = subwindow_left + screenWidth;
    subwindow_down = irow*screenHeight;
    subwindow_top = subwindow_down + screenHeight;

    port_right = p->left + p->width;
    port_top = p->down + p->height;

    subport_left =  MAX( resolution_multiplier*p->left,subwindow_left);
    subport_right = MIN(resolution_multiplier*port_right,subwindow_right);
    subport_down =  MAX( resolution_multiplier*p->down,subwindow_down);
    subport_top =   MIN(  resolution_multiplier*port_top,subwindow_top);
    if(subport_left>=subport_right||subport_down>=subport_top)return 0;

    subportx_left = CONV(subport_left,resolution_multiplier*p->left,resolution_multiplier*port_right,portx_left,portx_right);
    subportx_right = CONV(subport_right,resolution_multiplier*p->left,resolution_multiplier*port_right,portx_left,portx_right);
    subportx_down = CONV(subport_down,resolution_multiplier*p->down,resolution_multiplier*port_top,portx_down,portx_top);
    subportx_top = CONV(subport_top,resolution_multiplier*p->down,resolution_multiplier*port_top,portx_down,portx_top);

    subport_left -= icol*screenWidth;
    subport_right -= icol*screenWidth;
    subport_down -= irow*screenHeight;
    subport_top -= irow*screenHeight;
    subport_width = subport_right - subport_left;
    subport_height = subport_top - subport_down;

    port_pixel_width = subport_width;
    port_pixel_height = subport_height;
    port_unit_width = subportx_right - subportx_left;
    port_unit_height = subportx_top - subportx_down;

    glViewport(subport_left,subport_down,subport_width,subport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(subportx_left,subportx_right,subportx_down,subportx_top);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  return 1;
}


/* ------------------------ SubPortOrtho2 ------------------------- */

int SubPortOrtho2(int quad,
                  portdata *p,
                  GLint screen_left, GLint screen_down
                  ){

  GLint subport_left, subport_right, subport_down, subport_top;
  GLdouble subportx_left, subportx_right, subportx_down, subportx_top;
  GLsizei subport_width, subport_height;
  GLint subwindow_left, subwindow_right, subwindow_down, subwindow_top;
  GLint port_right, port_top;

  int irow, icol;
  GLdouble portx_left, portx_right, portx_down, portx_top;

  portx_left = p->left;
  portx_right = p->left + p->width;
  portx_down = p->down;
  portx_top = p->down + p->height;
  switch(quad){
  case 0:
    port_pixel_width = p->width;
    port_pixel_height = p->height;
    port_unit_width = portx_right - portx_left;
    port_unit_height = portx_top - portx_down;
    glViewport(p->left,p->down,p->width,p->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(portx_left,portx_right,portx_down,portx_top);
    return 1;
  case 1:
    icol = screen_left/screenWidth;
    irow = screen_down/screenHeight;

    subwindow_left = icol*screenWidth;
    subwindow_right = subwindow_left + screenWidth;
    subwindow_down = irow*screenHeight;
    subwindow_top = subwindow_down + screenHeight;

    port_right = p->left + p->width;
    port_top = p->down + p->height;

    subport_left =  MAX( resolution_multiplier*p->left,subwindow_left);
    subport_right = MIN(resolution_multiplier*port_right,subwindow_right);
    subport_down =  MAX( resolution_multiplier*p->down,subwindow_down);
    subport_top =   MIN(  resolution_multiplier*port_top,subwindow_top);
    if(subport_left>=subport_right||subport_down>=subport_top)return 0;

    subportx_left = CONV(subport_left,resolution_multiplier*p->left,resolution_multiplier*port_right,portx_left,portx_right);
    subportx_right = CONV(subport_right,resolution_multiplier*p->left,resolution_multiplier*port_right,portx_left,portx_right);
    subportx_down = CONV(subport_down,resolution_multiplier*p->down,resolution_multiplier*port_top,portx_down,portx_top);
    subportx_top = CONV(subport_top,resolution_multiplier*p->down,resolution_multiplier*port_top,portx_down,portx_top);

    subport_left -= icol*screenWidth;
    subport_right -= icol*screenWidth;
    subport_down -= irow*screenHeight;
    subport_top -= irow*screenHeight;
    subport_width = subport_right - subport_left;
    subport_height = subport_top - subport_down;

    port_pixel_width = subport_width;
    port_pixel_height = subport_height;
    port_unit_width = subportx_right - subportx_left;
    port_unit_height = subportx_top - subportx_down;

    glViewport(subport_left,subport_down,subport_width,subport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(subportx_left,subportx_right,subportx_down,subportx_top);
    break;
  default:
    ASSERT(FFALSE);
    break;
                    }
  return 1;
}

/* ------------------------ SubPortFrustum ------------------------- */

int SubPortFrustum(int quad,
                   portdata *p,
                   GLdouble portx_left, GLdouble portx_right,
                   GLdouble portx_down, GLdouble portx_top,
                   GLdouble portx_near, GLdouble portx_far,
                   GLint screen_left, GLint screen_down
                   ){
  GLint subport_left, subport_right, subport_down, subport_top;
  GLdouble subportx_left, subportx_right, subportx_down, subportx_top;
  GLsizei subport_width, subport_height;
  GLint subwindow_left, subwindow_right, subwindow_down, subwindow_top;
  GLint port_right, port_top;

  int irow, icol;

  switch(quad){
  case 0:
    port_pixel_width = p->width;
    port_pixel_height = p->height;
    port_unit_width = portx_right - portx_left;
    port_unit_height = portx_top - portx_down;
    glViewport(p->left,p->down,p->width,p->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(camera_current->projection_type==PROJECTION_PERSPECTIVE){
      glFrustum(
        (double)portx_left,(double)portx_right,
        (double)portx_down,(double)portx_top,
        (double)portx_near,(double)portx_far);
    }
    else{
      glOrtho(
        (double)portx_left,(double)portx_right,
        (double)portx_down,(double)portx_top,
        (double)portx_near,(double)portx_far);
    }
    return 1;
  case 1:
    icol = screen_left/screenWidth;
    irow = screen_down/screenHeight;

    subwindow_left = icol*screenWidth;
    subwindow_right = subwindow_left + screenWidth;
    subwindow_down = irow*screenHeight;
    subwindow_top = subwindow_down + screenHeight;

    port_right = p->left + p->width;
    port_top = p->down + p->height;

    subport_left =  MAX( resolution_multiplier*p->left,subwindow_left);
    subport_right = MIN(resolution_multiplier*port_right,subwindow_right);
    subport_down =  MAX( resolution_multiplier*p->down,subwindow_down);
    subport_top =   MIN(  resolution_multiplier*port_top,subwindow_top);
    if(subport_left>=subport_right||subport_down>=subport_top)return 0;

    subportx_left = CONV(subport_left,resolution_multiplier*p->left,resolution_multiplier*port_right,portx_left,portx_right);
    subportx_right = CONV(subport_right,resolution_multiplier*p->left,resolution_multiplier*port_right,portx_left,portx_right);
    subportx_down = CONV(subport_down,resolution_multiplier*p->down,resolution_multiplier*port_top,portx_down,portx_top);
    subportx_top = CONV(subport_top,resolution_multiplier*p->down,resolution_multiplier*port_top,portx_down,portx_top);

    subport_left -= icol*screenWidth;
    subport_right -= icol*screenWidth;
    subport_down -= irow*screenHeight;
    subport_top -= irow*screenHeight;
    subport_width = subport_right - subport_left;
    subport_height = subport_top - subport_down;

    port_pixel_width = subport_width;
    port_pixel_height = subport_height;
    port_unit_width = subportx_right - subportx_left;
    port_unit_height = subportx_top - subportx_down;

    glViewport(subport_left,subport_down,subport_width,subport_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(camera_current->projection_type==PROJECTION_PERSPECTIVE){
      glFrustum(
        (double)subportx_left,(double)subportx_right,
        (double)subportx_down,(double)subportx_top,
        (double)portx_near,(double)portx_far);
    }
    else{
      glOrtho(
        (double)subportx_left,(double)subportx_right,
        (double)subportx_down,(double)subportx_top,
        (double)portx_near,(double)portx_far);
    }
    return 1;
  default:
    ASSERT(FFALSE);
    break;
  }
  return 1;
}

 /* ------------------------ ViewportClip ------------------------- */

void ViewportClip(int quad, GLint screen_left, GLint screen_down){
  GLdouble x_left, x_right, x_down, x_top;
  float c_left, c_right, c_top, c_bottom;

  x_left=0.0;
  x_right=screenWidth;
  x_down=0.0;
  x_top=screenHeight;

  if(SubPortOrtho(quad,&VP_fullscreen,x_left, x_right, x_down, x_top,screen_left, screen_down)==0)return;

   c_left = render_clip_left-3;
   c_right = screenWidth + 3 - render_clip_right;
   c_bottom = render_clip_bottom -3;
   c_top = screenHeight + 3 - render_clip_top;

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glLineWidth(3.0);
   glColor3fv(foregroundcolor);
   glBegin(GL_LINES);

   if(c_left>0){
     glVertex2f(c_left,c_bottom);
     glVertex2f(c_left,c_top);
   }

   if(c_right<screenWidth){
     glVertex2f(c_right,c_bottom);
     glVertex2f(c_right,c_top);
   }

   if(c_top<screenHeight){
     glVertex2f(c_left,c_top);
     glVertex2f(c_right,c_top);
   }

   if(c_bottom>0){
     glVertex2f(c_left,c_bottom);
     glVertex2f(c_right,c_bottom);
   }
   glEnd();
}

 /* ------------------------ ViewportInfo ------------------------- */

void ViewportInfo(int quad, GLint screen_left, GLint screen_down){
  char slicelabel[255];
  meshdata *mesh_xyz=NULL;
  float xyz[3];
  int info_lines=0;

  if(SubPortOrtho2(quad,&VP_info,screen_left, screen_down)==0)return;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if((showplot3d==1||visGrid!=noGridnoProbe)&&(visx_all==1||visy_all||visz_all||visGrid==GridProbe||visGrid==noGridProbe)){
    xyz[0]=DENORMALIZE_X(plotx_all[iplotx_all]);
    xyz[1]=DENORMALIZE_Y(ploty_all[iploty_all]);
    xyz[2]=DENORMALIZE_Z(plotz_all[iplotz_all]);
    mesh_xyz= GetMeshNoFail(xyz);
  }
  if(((showplot3d==1||visGrid!=noGridnoProbe)&&visx_all==1)||visGrid==noGridProbe||visGrid==GridProbe){
    float plotval;
    int iplotval;
    char buff_label[128];


    iplotval=mesh_xyz->iplotx_all[iplotx_all];
    plotval=xyz[0];
    if(plotval>0.0){
      plotval=(int)(plotval*100+0.5);
    }
    else{
      plotval=(int)(plotval*100-0.5);
    }
    plotval/=100;

    sprintf(buff_label,"%f",plotval);
    TrimZeros(buff_label);
    strcat(buff_label," m");
    if(cursorPlot3D==1){
      sprintf(slicelabel,"*x: %i, ",iplotval);
    }
    else{
      sprintf(slicelabel,"x: %i, ",iplotval);
    }
    strcat(slicelabel,buff_label);
    if(visgridloc==1){
      OutputText(VP_info.left+h_space,VP_info.down+v_space, slicelabel);
      info_lines++;
    }
  }
  if(((showplot3d==1||visGrid!=noGridnoProbe)&&visy_all==1)||visGrid==GridProbe||visGrid==noGridProbe){
    float plotval;
    int iplotval;
    char buff_label[128];

    iplotval=mesh_xyz->iploty_all[iploty_all];
    plotval=xyz[1];
    if(plotval>0.0){
      plotval=(int)(plotval*100+0.5);
    }
    else{
      plotval=(int)(plotval*100-0.5);
    }
    plotval/=100;

    sprintf(buff_label,"%f",plotval);
    TrimZeros(buff_label);
    strcat(buff_label," m");
    if(cursorPlot3D==1){
      sprintf(slicelabel,"*y: %i, ",iplotval);
    }
    else{
      sprintf(slicelabel,"y: %i, ",iplotval);
    }
    strcat(slicelabel,buff_label);
    if(visgridloc==1){
      OutputText(VP_info.left+h_space,VP_info.down+v_space+info_lines*(v_space+VP_info.text_height), slicelabel);
      info_lines++;
    }
  }
  if(((showplot3d==1||visGrid!=noGridnoProbe)&&visz_all==1)||visGrid==GridProbe||visGrid==noGridProbe){
    float plotval;
    int iplotval;
    char buff_label[128];

    iplotval=mesh_xyz->iplotz_all[iplotz_all];
    plotval=xyz[2];
    if(plotval>0.0){
      plotval=(int)(plotval*100+0.5);
    }
    else{
      plotval=(int)(plotval*100-0.5);
    }
    plotval/=100;

    sprintf(buff_label,"%f",plotval);
    TrimZeros(buff_label);
    strcat(buff_label," m");
    if(cursorPlot3D==1){
      sprintf(slicelabel,"*z: %i, ",iplotval);
    }
    else{
      sprintf(slicelabel,"z: %i, ",iplotval);
    }
    strcat(slicelabel,buff_label);
    if(visgridloc==1){
      OutputText(VP_info.left+h_space,VP_info.down+v_space+info_lines*(v_space+VP_info.text_height), slicelabel);
      info_lines++;
    }
  }
  if(visMeshlabel==1){
    char meshlabel[255];

    if(mesh_xyz==NULL){
      sprintf(meshlabel,"mesh: %i",highlight_mesh+1);
      mesh_xyz = meshinfo + highlight_mesh;
    }
    else{
      int imesh;

      imesh = mesh_xyz-meshinfo+1;
      sprintf(meshlabel,"mesh: %i",imesh);
    }
    OutputText(VP_info.left+h_space,VP_info.down+v_space+info_lines*(v_space+VP_info.text_height), meshlabel);
  }
}

/* ------------------------ ViewportTimebar ------------------------- */

void ViewportTimebar(int quad, GLint screen_left, GLint screen_down) {
#ifdef pp_memstatus
  unsigned int availmemory;
  char percen[] = "%";
#endif
  int right_label_pos, timebar_right_pos;
  int timebar_left_pos;

  if (SubPortOrtho2(quad, &VP_timebar, screen_left, screen_down) == 0)return;

  timebar_left_width = GetStringWidth("Time: 1234.11");
  timebar_right_width = GetStringWidth("Frame rate: 99.99");

  timebar_left_pos = VP_timebar.left + timebar_left_width;
  timebar_right_pos = VP_timebar.right - timebar_right_width - h_space;
  right_label_pos = timebar_right_pos + h_space;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (visColorbarHorizontal == 1 && num_colorbars > 0 && (showtime == 1 || showplot3d == 1)){
    DrawHorizontalColorbarRegLabels();
    DrawHorizontalColorbars();
  }

  if((visTimelabel == 1 || visFramelabel == 1 || visHRRlabel == 1 || visTimebar == 1) &&showtime==1){
    if(visTimelabel==1){
      OutputText(VP_timebar.left,v_space, timelabel);
    }
    if(visFramelabel==1&&(visHRRlabel==0||hrrinfo==NULL)){
      OutputText(VP_timebar.left,v_space+VP_timebar.text_height+v_space, framelabel);
    }
    if(visHRRlabel==1&&hrrinfo!=NULL){
      OutputText(VP_timebar.left,v_space+VP_timebar.text_height+v_space, hrrinfo->hrrlabel);
    }
    if(visTimebar==1)DrawTimebar(timebar_left_pos,timebar_right_pos,v_space+VP_timebar.down,v_space+(VP_timebar.down+20));
  }

  if(visFramerate==1&&showtime==1){
    sprintf(frameratelabel," Frame rate:%4.1f",framerate);
    OutputText(right_label_pos,v_space,frameratelabel);
  }
  if(show_slice_average==1&&vis_slice_average==1&&slice_average_flag==1){
    sprintf(frameratelabel," AVG: %4.1f",slice_average_interval);
    OutputText(right_label_pos,3*v_space+2*VP_timebar.text_height, frameratelabel); // test print
  }

  if((hrrpuv_loaded == 1 || temp_loaded == 1) && show_firecutoff == 1 && current_mesh != NULL){
    char cutoff_label[256];
    int i_cutoff;
    float x1, x2, y1, y2;
    float f_red, f_green, f_blue;

    if(hrrpuv_loaded == 1 && show_firecutoff == 1){
      i_cutoff = (int)(global_hrrpuv_cutoff + 0.5);
      sprintf(cutoff_label, ">%i kW/m3", i_cutoff);
    }
    else {
      i_cutoff = (int)(global_temp_cutoff + 0.5);
      sprintf(cutoff_label, ">%i %s", i_cutoff,degC);
    }
    OutputText(right_label_pos+5+h_space,3*v_space+2*VP_timebar.text_height,cutoff_label);

    if(firecolormap_type == 0){
      f_red = (float)fire_red / 255.0;
      f_green = (float)fire_green / 255.0;
      f_blue = (float)fire_blue / 255.0;
      glColor3f(f_red, f_green, f_blue);
    }
    else{
      float *colors;
      int icolor;

      if(strcmp(fire_colorbar->label, "fire") == 0){
        icolor = 192;
      }
      else if(strcmp(fire_colorbar->label, "fire 2") == 0){
        icolor = 128 + 127*(global_hrrpuv_cutoff - global_hrrpuv_min) / (global_hrrpuv_max - global_hrrpuv_min);
        icolor = CLAMP((icolor + 1), 0, 255);
      }
      else{
        icolor = 255*(global_hrrpuv_cutoff-global_hrrpuv_min)/(global_hrrpuv_max-global_hrrpuv_min);
        icolor = CLAMP((icolor + 1), 0, 255);
      }
      colors = fire_colorbar->colorbar;
      f_red = colors[3*icolor + 0];
      f_green = colors[3*icolor + 1];
      f_blue = colors[3*icolor + 2];
      glColor3f(f_red, f_green, f_blue);
    }

    x1 = (float)(right_label_pos + h_space - 20);
    x2 = x1 + (float)20;
    y1 = (float)(5 + 2*VP_timebar.text_height);
    y2 = y1 + (float)20;

    glBegin(GL_TRIANGLES);
    glVertex3f(x1,y1,0.0);
    glVertex3f(x2,y1,0.0);
    glVertex3f(x2,y2,0.0);
    glVertex3f(x1, y1, 0.0);
    glVertex3f(x2, y2, 0.0);
    glVertex3f(x1, y2, 0.0);
    glEnd();
  }
#ifdef pp_memstatus
  if(visAvailmemory==1){
    MEMSTATUS(0,&availmemory,NULL,NULL);
    sprintf(frameratelabel," Mem Load:%u%s",availmemory,percen);
    if(visFramerate==1&&showtime==1){
      OutputText(right_label_pos,2*v_space+VP_timebar.text_height,frameratelabel);
    }
    else{
      OutputText(right_label_pos,v_space,frameratelabel);
    }
  }
#endif
#ifdef pp_MEMDEBUG
  if(visUsagememory==1
#ifdef pp_memstatus
     &&visAvailmemory==0
#endif
    ){
      char MEMlabel[128];

      getMemusage(MMtotalmemory,MEMlabel);
      if(visFramerate==1&&showtime==1){
        OutputText(right_label_pos,2*v_space+VP_timebar.text_height,MEMlabel);
      }
      else{
        OutputText(right_label_pos,v_space,MEMlabel);
      }
  }
#endif
}

/* --------------------- ViewportVerticalColorbar ------------------------- */

void ViewportVerticalColorbar(int quad, GLint screen_left, GLint screen_down){
  if(SubPortOrtho2(quad,&VP_vcolorbar,screen_left, screen_down)==0)return;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  DrawVerticalColorbarRegLabels();
  DrawVerticalColorbars();
}

    /* -------------------------- ViewportTitle -------------------------- */

void ViewportTitle(int quad, GLint screen_left, GLint screen_down){

  if(SubPortOrtho2(quad,&VP_title,screen_left,screen_down)==0)return;



  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  renderInfoHeader(&titleinfo);

}

/* ----------------------- CompareMeshes ----------------------------- */

int CompareMeshes(const void *arg1, const void *arg2){
  smoke3ddata *smoke3di, *smoke3dj;
  meshdata *meshi, *meshj;
  float *xyzmini, *xyzmaxi;
  float *xyzminj, *xyzmaxj;
  int dir = 0;
  int returnval = 0;

  smoke3di = *(smoke3ddata **)arg1;
  smoke3dj = *(smoke3ddata **)arg2;
  meshi = meshinfo + smoke3di->blocknumber;
  meshj = meshinfo + smoke3dj->blocknumber;
  if(meshi == meshj)return 0;
  xyzmini = meshi->boxmin;
  xyzmaxi = meshi->boxmax;
  xyzminj = meshj->boxmin;
  xyzmaxj = meshj->boxmax;
  if(dir == 0){
    if(xyzmaxi[0] <= xyzminj[0])dir = 1;
    if(xyzmaxj[0] <= xyzmini[0])dir = -1;
  }
  if(dir == 0){
    if(xyzmaxi[1] <= xyzminj[1])dir = 2;
    if(xyzmaxj[1] <= xyzmini[1])dir = -2;
  }
  if(dir == 0){
    if(xyzmaxi[2] <= xyzminj[2])dir = 3;
    if(xyzmaxj[2] <= xyzmini[2])dir = -3;
  }
  switch(dir){
  case 0:
    returnval = 0;
    break;
  case XDIR:
    if(fds_eyepos[0] < xyzmaxi[0]){
      returnval = 1;
    }
    else{
      returnval = -1;
    }
    break;
  case XDIRNEG:
    if(fds_eyepos[0] < xyzmaxj[0]){
      returnval = -1;
    }
    else{
      returnval = 1;
    }
    break;
  case YDIR:
    if(fds_eyepos[1] < xyzmaxi[1]){
      returnval = 1;
    }
    else{
      returnval = -1;
    }
    break;
  case YDIRNEG:
    if(fds_eyepos[1] < xyzmaxj[1]){
      returnval = -1;
    }
    else{
      returnval = 1;
    }
    break;
  case ZDIR:
    if(fds_eyepos[2] < xyzmaxi[2]){
      returnval = 1;
    }
    else{
      returnval = -1;
    }
    break;
  case ZDIRNEG:
    if(fds_eyepos[2] < xyzmaxj[2]){
      returnval = -1;
    }
    else{
      returnval = 1;
    }
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  return returnval;
}

/* ------------------ SortSmoke3dinfo ------------------------ */

void SortSmoke3dinfo(void){
  if(nsmoke3dinfo > 1){
    qsort((meshdata **)smoke3dinfo_sorted, (size_t)nsmoke3dinfo, sizeof(smoke3ddata *), CompareMeshes);
  }
}

/* ------------------ GetEyePos ------------------------ */

void GetEyePos(float *mm){
  int i;
  float scene_center[3] = {0.0, 0.0, 0.0};

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

  smv_eyepos[0] = -(mm[0]*mm[12] + mm[1]*mm[13] +  mm[2]*mm[14])/mscale[0];
  smv_eyepos[1] = -(mm[4]*mm[12] + mm[5]*mm[13] +  mm[6]*mm[14])/mscale[1];
  smv_eyepos[2] = -(mm[8]*mm[12] + mm[9]*mm[13] + mm[10]*mm[14])/mscale[2];
  DENORMALIZE_XYZ(fds_eyepos, smv_eyepos);

  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo+i;
    scene_center[0] += meshi->boxmiddle[0];
    scene_center[1] += meshi->boxmiddle[1];
    scene_center[2] += meshi->boxmiddle[2];
  }
  scene_center[0] /= nmeshes;
  scene_center[1] /= nmeshes;
  scene_center[2] /= nmeshes;
  fds_viewdir[0] = scene_center[0] - fds_eyepos[0];
  fds_viewdir[1] = scene_center[1] - fds_eyepos[1];
  fds_viewdir[2] = scene_center[2] - fds_eyepos[2];
  NORMALIZE3(fds_viewdir);
}

/* ------------------ CompareVolFaceListData ------------------------ */

int CompareVolFaceListData(const void *arg1, const void *arg2){
  volfacelistdata *vi, *vj;

  vi = *(volfacelistdata **)arg1;
  vj = *(volfacelistdata **)arg2;

  if(vi->dist2 < vj->dist2)return 1;
  if(vi->dist2 > vj->dist2)return -1;
  return 0;
}

/* ------------------ GetVolSmokeDir ------------------------ */

void GetVolSmokeDir(float *mm){
  /*
  ( m0 m4 m8  m12 ) (x)    (0)
  ( m1 m5 m9  m13 ) (y)    (0)
  ( m2 m6 m10 m14 ) (z)  = (0)
  ( m3 m7 m11 m15 ) (1)    (1)

      ( m0 m4  m8 )      (m12)
  Q=  ( m1 m5  m9 )  u = (m13)
      ( m2 m6 m10 )      (m14)

        ( m0 m1  m2 )
  Q^T=  ( m4 m5  m6 )
        ( m8 m9 m10 )

      ( M_x  0    0  )
  M = ( 0   M_y   0  )
      ( 0    0   M_z )

  (Q   u) (M) (x)     (0)
  (v^T 1) (1) (y)   = (1)

  m3=m7=m11=0, v^T=0, y=1   QMx+u=0 => x=-inv(M)Q^Tu

       ( m0 m1  m2 ) (m12)   ( m0*m12 + m1*m13 +  m2*m14 )/M_x
  x = -( m4 m5  m6 ) (m13) = ( m4*m12 + m5*m13 +  m6*m14 )/M_y
       ( m8 m9 m10 ) (m14)   ( m8*m12 + m9*m13 + m10*m14 )/M_z

  */
  int i, ii, j;
  float norm[3];
  float eyedir[3];
  float cosdir;
  float angles[7];

  volfacelistdata *vi;

  if(freeze_volsmoke == 1)return;

  eye_position_fds[0] = -DOT3(mm + 0, mm + 12) / mscale[0];
  eye_position_fds[1] = -DOT3(mm + 4, mm + 12) / mscale[1];
  eye_position_fds[2] = -DOT3(mm + 8, mm + 12) / mscale[2];

  for(j = 0;j<nmeshes;j++){
    meshdata *meshj;
    int *inside;
    int *drawsides;
    float x0, x1, yy0, yy1, z0, z1;
    float xcen, ycen, zcen;

    meshj = meshinfo + j;

    inside = &meshj->inside;
    drawsides = meshj->drawsides;

      x0 = meshj->x0;
      x1 = meshj->x1;
     yy0 = meshj->y0;
     yy1 = meshj->y1;
      z0 = meshj->z0;
      z1 = meshj->z1;
    xcen = meshj->xcen;
    ycen = meshj->ycen;
    zcen = meshj->zcen;

    *inside = 0;
    if(
      eye_position_fds[0]> x0&&eye_position_fds[0]<x1&&
      eye_position_fds[1]>yy0&&eye_position_fds[1]<yy1&&
      eye_position_fds[2]> z0&&eye_position_fds[2]<z1
      ){
      for(i = -3;i <= 3;i++){
        if(i == 0)continue;
        drawsides[i + 3] = 1;
      }
      *inside = 1;
      continue;
    }

    for(i = -3;i <= 3;i++){
      if(i == 0)continue;
      ii = ABS(i);
      norm[0] = 0.0;
      norm[1] = 0.0;
      norm[2] = 0.0;
      switch(ii){
      case XDIR:
        if(i<0){
          norm[0] = -1.0;
          eyedir[0] = x0;
        }
        else{
          norm[0] = 1.0;
          eyedir[0] = x1;
        }
        eyedir[1] = ycen;
        eyedir[2] = zcen;
        break;
      case YDIR:
        eyedir[0] = xcen;
        if(i<0){
          norm[1] = -1.0;
          eyedir[1] = yy0;
        }
        else{
          norm[1] = 1.0;
          eyedir[1] = yy1;
        }
        eyedir[2] = zcen;
        break;
      case ZDIR:
        eyedir[0] = xcen;
        eyedir[1] = ycen;
        if(i<0){
          norm[2] = -1.0;
          eyedir[2] = z0;
        }
        else{
          norm[2] = 1.0;
          eyedir[2] = z1;
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      VEC3DIFF(eyedir, eye_position_fds, eyedir);
      Normalize(eyedir, 3);
      cosdir = CLAMP(DOT3(eyedir, norm), -1.0, 1.0);
      cosdir = acos(cosdir)*RAD2DEG;
      if(cosdir<0.0)cosdir = -cosdir;
      angles[3 + i] = cosdir;
    }
    for(i = -3;i <= 3;i++){
      if(i == 0)continue;
      if(angles[i + 3]<90.0){
        drawsides[i + 3] = 1;
      }
      else{
        drawsides[i + 3] = 0;
      }
    }
  }

  // turn off drawing for mesh sides that are on the inside of a supermesh
  if(combine_meshes == 1){
    for(i = 0;i<nmeshes;i++){
      meshdata *meshi;
      int *drawsides, *extsides;
      int jj;

      meshi = meshinfo + i;
      drawsides = meshi->drawsides;
      extsides = meshi->extsides;
      for(jj = 0;jj<7;jj++){
        if(extsides[jj] == 0){
          drawsides[jj] = 0;
        }
      }
    }
    for(i = 0;i<nsupermeshinfo;i++){
      supermeshdata *smesh;

      smesh = supermeshinfo + i;
      for(j = 0;j<7;j++){
        smesh->drawsides[j] = 0;
      }
      for(j = 0;j<smesh->nmeshes;j++){
        meshdata *meshj;
        int k;

        meshj = smesh->meshes[j];
        for(k = 0;k<7;k++){
          if(meshj->extsides[k] == 1 && meshj->drawsides[k] == 1)smesh->drawsides[k] = 1;
        }
      }
    }
  }

  vi = volfacelistinfo;
  nvolfacelistinfo = 0;
  for(i = 0;i<nmeshes;i++){
    meshdata *meshi;
    int facemap[7] = {12,6,0,0,3,9,15};
    volrenderdata *vr;
    int *drawsides;

    meshi = meshinfo + i;

    drawsides = meshi->drawsides;

    vr = &(meshi->volrenderinfo);
    if(vr->firedataptr == NULL&&vr->smokedataptr == NULL)continue;
    if(vr->loaded == 0 || vr->display == 0)continue;
    for(j = -3;j <= 3;j++){
      float dx, dy, dz;
      float *xyz;

      if(j == 0)continue;
      if(drawsides[j + 3] == 0)continue;
      vi->facemesh = meshi;
      vi->iwall = j;
      xyz = meshi->face_centers + facemap[j + 3];

      dx = xyz[0] - eye_position_fds[0];
      dy = xyz[1] - eye_position_fds[1];
      dz = xyz[2] - eye_position_fds[2];
      vi->dist2 = dx*dx + dy*dy + dz*dz;
      vi->xyz = xyz;
      vi++;
      nvolfacelistinfo++;
    }
  }
  if(nvolfacelistinfo>0){
    for(i = 0;i<nvolfacelistinfo;i++){
      volfacelistinfoptrs[i] = volfacelistinfo + i;
    }
    qsort((volfacelistdata *)volfacelistinfoptrs, nvolfacelistinfo, sizeof(volfacelistdata *), CompareVolFaceListData);
  }
}

/* ------------------ GetSmokeDir ------------------------ */

void GetSmokeDir(float *mm){
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
  int i, ii, j;
  meshdata *meshj;
  float norm[3], scalednorm[3];
  float normdir[3];
  float absangle, cosangle, minangle;
  int iminangle;
  float dx, dy, dz;
  float factor;

  eye_position_fds[0] = -DOT3(mm + 0, mm + 12) / mscale[0];
  eye_position_fds[1] = -DOT3(mm + 4, mm + 12) / mscale[1];
  eye_position_fds[2] = -DOT3(mm + 8, mm + 12) / mscale[2];

  for(j = 0;j<nmeshes;j++){
    meshdata  *meshi;

    meshi = meshinfo + j;
    dx = meshi->boxmiddle_scaled[0] - eye_position_fds[0];
    dy = meshi->boxmiddle_scaled[1] - eye_position_fds[1];
    dz = meshi->boxmiddle_scaled[2] - eye_position_fds[2];
    meshi->eyedist = sqrt(dx*dx + dy*dy + dz*dz);
  }

  for(j = 0;j<nmeshes;j++){
    meshj = meshinfo + j;

    minangle = 1000.0;
    iminangle = -10;
    meshj->dx = meshj->xplt_orig[1] - meshj->xplt_orig[0];
    meshj->dy = meshj->yplt_orig[1] - meshj->yplt_orig[0];
    meshj->dz = meshj->zplt_orig[1] - meshj->zplt_orig[0];
    meshj->dxyz[0] = meshj->dx;
    meshj->dxyz[1] = meshj->dy;
    meshj->dxyz[2] = meshj->dz;
    meshj->dxy = meshj->dx*meshj->dx + meshj->dy*meshj->dy;
    meshj->dxy = sqrt(meshj->dxy) / 2.0;
    meshj->dxz = meshj->dx*meshj->dx + meshj->dz*meshj->dz;
    meshj->dxz = sqrt(meshj->dxz) / 2.0;
    meshj->dyz = meshj->dy*meshj->dy + meshj->dz*meshj->dz;
    meshj->dyz = sqrt(meshj->dyz) / 2.0;

    meshj->dy /= meshj->dx;
    meshj->dz /= meshj->dx;
    meshj->dxy /= meshj->dx;
    meshj->dxz /= meshj->dx;
    meshj->dyz /= meshj->dx;
    meshj->dx = 1.0;

    for(i = -9;i <= 9;i++){
      if(i == 0)continue;
      ii = ABS(i);
      norm[0] = 0.0;
      norm[1] = 0.0;
      norm[2] = 0.0;
      switch(ii){
      case XDIR:
        if(i<0)norm[0] = -1.0;
        if(i>0)norm[0] = 1.0;
        break;
      case YDIR:
        if(i<0)norm[1] = -1.0;
        if(i>0)norm[1] = 1.0;
        break;
      case ZDIR:
        if(i<0)norm[2] = -1.0;
        if(i>0)norm[2] = 1.0;
        break;
      case 4:
        dx = meshj->xplt_orig[1] - meshj->xplt_orig[0];
        dy = meshj->yplt_orig[1] - meshj->yplt_orig[0];
        factor = dx*dx + dy*dy;
        if(factor == 0.0){
          factor = 1.0;
        }
        else{
          factor = 1.0 / sqrt(factor);
        }
        if(i<0){
          norm[0] = -dy*factor;
          norm[1] = -dx*factor;
        }
        else{
          norm[0] = dy*factor;
          norm[1] = dx*factor;
        }
        break;
      case 5:
        dx = meshj->xplt_orig[1] - meshj->xplt_orig[0];
        dy = meshj->yplt_orig[1] - meshj->yplt_orig[0];
        factor = dx*dx + dy*dy;
        if(factor == 0.0){
          factor = 1.0;
        }
        else{
          factor = 1.0 / sqrt(factor);
        }
        if(i<0){
          norm[0] = dy*factor;
          norm[1] = -dx*factor;
        }
        else{
          norm[0] = -dy*factor;
          norm[1] = dx*factor;
        }
        break;
      case 6:
        dy = meshj->yplt_orig[1] - meshj->yplt_orig[0];
        dz = meshj->zplt_orig[1] - meshj->zplt_orig[0];
        factor = dz*dz + dy*dy;
        if(factor == 0.0){
          factor = 1.0;
        }
        else{
          factor = 1.0 / sqrt(factor);
        }
        if(i<0){
          norm[1] = -dz*factor;
          norm[2] = -dy*factor;
        }
        else{
          norm[1] = dz*factor;
          norm[2] = dy*factor;
        }
        break;
      case 7:
        dy = meshj->yplt_orig[1] - meshj->yplt_orig[0];
        dz = meshj->zplt_orig[1] - meshj->zplt_orig[0];
        factor = dz*dz + dy*dy;
        if(factor == 0.0){
          factor = 1.0;
        }
        else{
          factor = 1.0 / sqrt(factor);
        }
        if(i<0){
          norm[1] = dz*factor;
          norm[2] = -dy*factor;
        }
        else{
          norm[1] = -dz*factor;
          norm[2] = dy*factor;
        }
        break;
      case 8:
        dx = meshj->xplt_orig[1] - meshj->xplt_orig[0];
        dz = meshj->zplt_orig[1] - meshj->zplt_orig[0];
        factor = dz*dz + dx*dx;
        if(factor == 0.0){
          factor = 1.0;
        }
        else{
          factor = 1.0 / sqrt(factor);
        }
        if(i<0){
          norm[0] = -dz*factor;
          norm[2] = -dx*factor;
        }
        else{
          norm[0] = dz*factor;
          norm[2] = dx*factor;
        }
        break;
      case 9:
        dx = meshj->xplt_orig[1] - meshj->xplt_orig[0];
        dz = meshj->zplt_orig[1] - meshj->zplt_orig[0];
        factor = dx*dx + dz*dz;
        if(factor == 0.0){
          factor = 1.0;
        }
        else{
          factor = 1.0 / sqrt(factor);
        }
        if(i<0){
          norm[0] = dz*factor;
          norm[2] = -dx*factor;
        }
        else{
          norm[0] = -dz*factor;
          norm[2] = dx*factor;
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      scalednorm[0] = norm[0] * mscale[0];
      scalednorm[1] = norm[1] * mscale[1];
      scalednorm[2] = norm[2] * mscale[2];

      normdir[0] = DOT3SKIP(mm, 4, scalednorm, 1);
      normdir[1] = DOT3SKIP(mm + 1, 4, scalednorm, 1);
      normdir[2] = DOT3SKIP(mm + 2, 4, scalednorm, 1);

      cosangle = normdir[2] / NORM3(normdir);
      cosangle = CLAMP(cosangle, -1.0, 1.0);
      absangle = acos(cosangle)*RAD2DEG;
      if(absangle<0.0)absangle = -absangle;
      if(absangle<minangle){
        iminangle = i;
        minangle = absangle;
        meshj->norm[0] = norm[0];
        meshj->norm[1] = norm[1];
        meshj->norm[2] = norm[2];
      }
    }
    meshj->smokedir = iminangle;
    if(demo_mode != 0){
      meshj->smokedir = 1;
    }
  }
}

/* ------------------ GetZoneSmokeDir ------------------------ */

void GetZoneSmokeDir(float *mm){
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
  int i, ii, j;
  float norm[3];
  float eyedir[3];
  float cosdir;
  float angles[7];

  eye_position_fds[0] = -(mm[0] * mm[12] + mm[1] * mm[13] + mm[2] * mm[14]) / mscale[0];
  eye_position_fds[1] = -(mm[4] * mm[12] + mm[5] * mm[13] + mm[6] * mm[14]) / mscale[1];
  eye_position_fds[2] = -(mm[8] * mm[12] + mm[9] * mm[13] + mm[10] * mm[14]) / mscale[2];

  for(j = 0;j<nrooms;j++){
    roomdata *roomj;

    roomj = roominfo + j;

    roomj->zoneinside = 0;
    if(
      eye_position_fds[0]>roomj->x0&&eye_position_fds[0]<roomj->x1&&
      eye_position_fds[1]>roomj->y0&&eye_position_fds[1]<roomj->y1&&
      eye_position_fds[2]>roomj->z0&&eye_position_fds[2]<roomj->z1
      ){
      for(i = -3;i <= 3;i++){
        if(i == 0)continue;
        roomj->drawsides[i + 3] = 1;
      }
      roomj->zoneinside = 1;
      continue;
    }

    for(i = -3;i <= 3;i++){
      if(i == 0)continue;
      ii = ABS(i);
      norm[0] = 0.0;
      norm[1] = 0.0;
      norm[2] = 0.0;
      switch(ii){
      case XRIGHT:
        if(i<0){
          norm[0] = -1.0;
          eyedir[0] = roomj->x0;
        }
        else{
          norm[0] = 1.0;
          eyedir[0] = roomj->x0 + roomj->dx;
        }
        eyedir[1] = roomj->y0 + roomj->dy / 2.0;
        eyedir[2] = roomj->z0 + roomj->dz / 2.0;
        break;
      case YBACK:
        eyedir[0] = roomj->x0 + roomj->dx / 2.0;
        if(i<0){
          norm[1] = -1.0;
          eyedir[1] = roomj->y0;
        }
        else{
          norm[1] = 1.0;
          eyedir[1] = roomj->y0 + roomj->dy;
        }
        eyedir[2] = roomj->z0 + roomj->dz / 2.0;
        break;
      case ZTOP:
        eyedir[0] = roomj->x0 + roomj->dx / 2.0;
        eyedir[1] = roomj->y0 + roomj->dy / 2.0;
        if(i<0){
          norm[2] = -1.0;
          eyedir[2] = roomj->z0;
        }
        else{
          norm[2] = 1.0;
          eyedir[2] = roomj->z0 + roomj->dz;
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      eyedir[0] = eye_position_fds[0] - eyedir[0];
      eyedir[1] = eye_position_fds[1] - eyedir[1];
      eyedir[2] = eye_position_fds[2] - eyedir[2];
      Normalize(eyedir, 3);
      cosdir = (eyedir[0] * norm[0] + eyedir[1] * norm[1] + eyedir[2] * norm[2]);
      if(cosdir>1.0)cosdir = 1.0;
      if(cosdir<-1.0)cosdir = -1.0;
      cosdir = acos(cosdir)*RAD2DEG;
      if(cosdir<0.0)cosdir = -cosdir;
      angles[3 + i] = cosdir;
    }
    for(i = -3;i <= 3;i++){
      if(i == 0)continue;
      if(angles[i + 3]<90.0){
        roomj->drawsides[i + 3] = 1;
      }
      else{
        roomj->drawsides[i + 3] = 0;
      }
    }
  }
}

/* ----------------------- ViewportScene ----------------------------- */

void ViewportScene(int quad, int view_mode, GLint screen_left, GLint screen_down, screendata *screen){

  float fleft, fright, fup, fdown;
  float EyeSeparation,FrustumAsymmetry,dEyeSeparation[3];
  float aperture_temp;
  float widthdiv2;
  float eyexINI, eyeyINI, eyezINI;

  if(stereotype==STEREO_LR){
    VP_scene.left=screen_left;
    VP_scene.width=screenWidth;
  }
  if(plotstate==DYNAMIC_PLOTS&&selected_tour!=NULL&&selected_tour->timeslist!=NULL){
    if((viewtourfrompath==1&&selectedtour_index>=0)||keyframe_snap==1){
      tourdata *touri;
      pathdata *pj;

      touri = tourinfo + selectedtour_index;
      frame_index = touri->timeslist[itimes];
      if(keyframe_snap==1&&selected_frame!=NULL){
        pj=&selected_frame->nodeval;
      }
      else{
        pj = touri->pathnodes + frame_index;
      }

      camera_current->eye[0]=pj->eye[0];
      camera_current->eye[1]=pj->eye[1];
      camera_current->eye[2]=pj->eye[2];
      camera_current->az_elev[1]=0.0;
      camera_current->az_elev[0]=0.0;
     }
  }

  if(plotstate==DYNAMIC_PLOTS&&select_avatar==1&&selected_avatar_tag>0&&view_from_selected_avatar==1){
    camera_current->eye[0]=selected_avatar_pos[0];
    camera_current->eye[1]=selected_avatar_pos[1];
    camera_current->eye[2]=selected_avatar_pos[2];
    camera_current->azimuth=selected_avatar_angle;
    camera_current->view_angle=0.0;
    UpdateCamera(camera_current);
  }

  eyexINI = camera_current->eye[0];
  eyeyINI = camera_current->eye[1];
  eyezINI = camera_current->eye[2];

#ifdef pp_CLIP
  if(projection_type==PROJECTION_ORTHOGRAPHIC){
    fnear = -eyeyINI - 1.0;
    if(fnear < nearclip)fnear = nearclip;
    ffar = fnear + farclip;
  }
  else{
    float min_depth, max_depth, *eye;

    eye = camera_current->eye;
    GetMinMaxDepth(eye, &min_depth, &max_depth);
    fnear = MAX(min_depth-1.0, 0.001);
    ffar  = MAX(    max_depth+1.0, farclip);
  }
#else
  fnear = -eyeyINI - 1.0;
  if(fnear < nearclip)fnear = nearclip;
  ffar = fnear + farclip;
#endif

  aperture_temp = Zoom2Aperture(zoom);

  if(plotstate==DYNAMIC_PLOTS&&selected_tour!=NULL&&selected_tour->timeslist!=NULL){
    if((viewtourfrompath==1&&selectedtour_index>=0)||keyframe_snap==1){
      tourdata *touri;
      pathdata *pj;

      touri = tourinfo + selectedtour_index;
      frame_index = touri->timeslist[itimes];
      if(keyframe_snap==1&&selected_frame!=NULL){
        pj=&selected_frame->nodeval;
      }
      else{
        pj = touri->pathnodes + frame_index;
      }

      aperture_temp=Zoom2Aperture(pj->zoom);
    }
  }

  widthdiv2 = fnear*tan(0.5*aperture_temp*DEG2RAD);
  fleft = -widthdiv2;
  fright = widthdiv2;
  fup = scene_aspect_ratio*widthdiv2;
  fdown = -scene_aspect_ratio*widthdiv2;

  EyeSeparation=0.0;
  FrustumAsymmetry=0.0;
  if(stereotype!=STEREO_NONE&&(view_mode==VIEW_LEFT||view_mode==VIEW_RIGHT)){
    EyeSeparation = SCALE2SMV(fzero) / 30.0;
    if(view_mode == VIEW_LEFT)EyeSeparation = -EyeSeparation;
    FrustumAsymmetry = -0.5*EyeSeparation*fnear / SCALE2SMV(fzero);
  }

  if(SubPortFrustum(quad,&VP_scene,
    (double)(fleft+FrustumAsymmetry),(double)(fright+FrustumAsymmetry),(double)fdown,(double)fup,(double)fnear,(double)ffar,
    screen_left, screen_down)==0)return;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  UpdateLights(light_position0, light_position1);

  {
    float xcen, ycen, zcen;
    float pos[3];
    int use_tour=0;

    xcen = camera_current->xcen;
    ycen = camera_current->ycen;
    zcen = camera_current->zcen;

    if(rotation_type == EYE_CENTERED){
      float sin_dv_sum, cos_dv_sum;
      float sin_azimuth, cos_azimuth;
      float sn_view_angle, cs_view_angle;
      float cos_elevation, sin_elevation;

      sn_view_angle = sin(DEG2RAD*camera_current->view_angle);
      cs_view_angle = cos(DEG2RAD*camera_current->view_angle);

      sin_azimuth = sin(DEG2RAD*camera_current->azimuth);
      cos_azimuth = cos(DEG2RAD*camera_current->azimuth);

      cos_elevation = cos(DEG2RAD*camera_current->elevation);
      sin_elevation = sin(DEG2RAD*camera_current->elevation);

      sin_dv_sum = sin_azimuth*cs_view_angle + cos_azimuth*sn_view_angle;
      cos_dv_sum = cos_azimuth*cs_view_angle - sin_azimuth*sn_view_angle;

      dEyeSeparation[0] = cos_dv_sum*EyeSeparation / 2.0;
      dEyeSeparation[1] = -sin_dv_sum*EyeSeparation / 2.0;

      pos[0] = eyexINI + dEyeSeparation[0];
      pos[1] = eyeyINI + dEyeSeparation[1];
      pos[2] = eyezINI;

      viewx = pos[0] + sin_dv_sum*cos_elevation;
      viewy = pos[1] + cos_dv_sum*cos_elevation;
      viewz = pos[2] + sin_elevation;
    }
    else{
      dEyeSeparation[0] = EyeSeparation / 2.0;
      dEyeSeparation[1] = 0.0;

      pos[0] = eyexINI + dEyeSeparation[0];
      pos[1] = eyeyINI + dEyeSeparation[1];
      pos[2] = eyezINI;

      viewx = pos[0];
      viewy = pos[1] + 1.0;
      viewz = pos[2] + 0.0;
    }

    /* set view direction for virtual tour */
    {
      tourdata *touri;
      pathdata *pj;

      if(plotstate==DYNAMIC_PLOTS&&selected_tour!=NULL&&selected_tour->timeslist!=NULL){
        if((viewtourfrompath==1&&selectedtour_index>=0)||keyframe_snap==1){
          touri = tourinfo + selectedtour_index;
          frame_index = touri->timeslist[itimes];
          if(keyframe_snap==1&&selected_frame!=NULL){
            pj=&selected_frame->nodeval;
          }
          else{
            pj = touri->pathnodes + frame_index;
          }

          viewx = pj->tour_view[0]+dEyeSeparation[0];
          viewy = pj->tour_view[1]-dEyeSeparation[1];
          viewz = pj->tour_view[2];
          use_tour = 1;
        }
      }
    }

    if(screen == NULL&&screenglobal==NULL){
      float *uup;

      uup = camera_current->up;
      gluLookAt(
        (double)pos[0], (double)pos[1], (double)pos[2],
        (double)viewx,  (double)viewy,  (double)viewz,
        (double)uup[0], (double)uup[1], (double)uup[2]
      );
      smv_viewpos[0] = viewx;
      smv_viewpos[1] = viewy;
      smv_viewpos[2] = viewz;
    }
    else{
      float *view, *uup, *right;
      float ppos[3], vview[3];

      if(screenglobal!=NULL){
        view = screenglobal->view;
        uup = screenglobal->up;
        right = screenglobal->right;
      }
      else{
        view = screen->view;
        uup = screen->up;
        right = screen->right;
      }
      dEyeSeparation[0] = EyeSeparation*right[0] / 2.0;
      dEyeSeparation[1] = EyeSeparation*right[1] / 2.0;
      dEyeSeparation[2] = EyeSeparation*right[2] / 2.0;
      ppos[0] = eyexINI + dEyeSeparation[0];
      ppos[1] = eyeyINI + dEyeSeparation[1];
      ppos[2] = eyezINI + dEyeSeparation[2];
      vview[0] = ppos[0] + view[0];
      vview[1] = ppos[1] + view[1];
      vview[2] = ppos[2] + view[2];
      gluLookAt(
         (double)ppos[0], (double)ppos[1], (double)ppos[2],
        (double)vview[0],(double)vview[1],(double)vview[2],
          (double)uup[0],  (double)uup[1],  (double)uup[2]
      );
      smv_viewpos[0] = vview[0];
      smv_viewpos[1] = vview[1];
      smv_viewpos[2] = vview[2];
    }

    glGetFloatv(GL_MODELVIEW_MATRIX,modelview_setup);
    GetInverse(modelview_setup,inverse_modelview_setup);

    glMultMatrixf(modelview_identity);

    glTranslatef(xcen,ycen,zcen);

    // rotate scene
    if(rotation_type==ROTATION_3AXIS){
      glMultMatrixf(quat_rotation);
    }
    else{
      if(use_tour == 0){
        float azimuth, elevation;

        elevation = camera_current->az_elev[1];
        azimuth = camera_current->az_elev[0];
        if(rotation_type == ROTATION_2AXIS){
          glRotatef(elevation, 1.0, 0.0, 0.0);  /* rotate about x axis */
        }
        glRotatef(azimuth, 0.0, 0.0, 1.0);      /* rotate about z axis */
      }
    }
    {
      float u[3], axis[3], angle;

      u[0] = 0.0;
      u[1] = 0.0;
      u[2] = 1.0;
      RotateU2V(user_zaxis, u, axis, &angle);
      glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);
      glRotatef(zaxis_angles[2], u[0], u[1], u[2]);
    }

    glTranslatef(-xcen*mscale[0],-ycen*mscale[1],-zcen*mscale[1]);

    glGetFloatv(GL_MODELVIEW_MATRIX,modelview_scratch);
    MatMultMat(inverse_modelview_setup,modelview_scratch,modelview_current);

    GetEyePos(modelview_scratch);

    if(show_gslice_triangles==1||SHOW_gslice_data==1){
      UpdateGslicePlanes();
    }
    if(nrooms>0){
      GetZoneSmokeDir(modelview_scratch);
    }
    if(nvolrenderinfo>0&&showvolrender==1&&usevolrender==1){
      GetVolSmokeDir(modelview_scratch);
      SNIFF_ERRORS("after GetVolSmokeDir");
#ifdef pp_GPU
      if(usegpu==0)ComputeAllSmokecolors();
#else
      ComputeAllSmokecolors();
#endif
    }
#ifdef pp_SMOKEDIAG
    smoketime_merge = 0.0;
    smoketime_draw = 0.0;
#endif
    if(nsmoke3dinfo>0&&show3dsmoke==1){
      SortSmoke3dinfo();
#ifdef pp_GPUSMOKE
      if(use_newsmoke==SMOKE3D_ORIG||smoke_mesh_aligned==1)GetSmokeDir(modelview_scratch);
      if(update_smokeplanes==1||use_newsmoke==SMOKE3D_NEW){
        UpdateSmoke3DPlanes(smoke3d_delta_perp, smoke3d_delta_par);
      }
#else
      GetSmokeDir(modelview_scratch);
#endif
      SNIFF_ERRORS("after GetSmokeDir");
    }
    else if(showslice==1&&(showall_3dslices==1||nslice_loaded>1)){
      GetSmokeDir(modelview_scratch);
    }
    if(nface_transparent>0&&sort_transparent_faces==1)SortTransparentFaces(modelview_scratch);
    if(showiso==1)UpdateIsoTriangles(0);
    LOCK_TRIANGLES;
    GetGeomInfoPtrs(0);
    UNLOCK_TRIANGLES;
    if(ngeominfoptrs>0)ShowHideSortGeometry(modelview_scratch);
    if(showiso==1&&sort_iso_triangles==1&&niso_trans>0)SortIsoTriangles(modelview_scratch);

    glScalef(mscale[0],mscale[1],mscale[2]);
    ExtractFrustum();
    SetCullVis();
  }
}
