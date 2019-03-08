#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "smokeviewvars.h"

#include GLUT_H
#include "gd.h"

/* ------------------ PlayMovie ------------------------ */

void PlayMovie(void){
  char command_line[1024], moviefile_path[1024];

  if(play_movie_now==0)return;
  if(FILE_EXISTS(GetMovieFilePath(moviefile_path)) == YES){
    strcpy(command_line, "ffplay ");
    strcat(command_line,moviefile_path);
    PSystem(command_line);
  }
  else{
    PRINTF("*** Error: the movie file, %s, does not exist\n", moviefile_path);
  }
}

/* ------------------ GetMovieFilePath ------------------------ */

char *GetMovieFilePath(char *moviefile_path){
  char moviefile[1024], *movie;

  TrimBack(movie_name);
  movie = TrimFront(movie_name);
  strcpy(moviefile, movie);
  strcat(moviefile, movie_ext);
  strcpy(moviefile_path, "");

  // if a script is running  prepend path defined in script

  if(script_dir_path != NULL&&strlen(script_dir_path) > 0){
    strcat(moviefile_path, script_dir_path);
    strcat(moviefile_path, dirseparator);
  }
  strcat(moviefile_path, moviefile);
  return moviefile_path;
}

/* ------------------ MakeMovie ------------------------ */

void MakeMovie(void){
  char command_line[1024];
  char frame0[1024];
  char moviefile_path[1024],overwrite_flag[10],image_ext[10], movie_frames[1024];
  int make_movie_now=1;

  if(output_ffmpeg_command==1)make_movie_now = 0;
// wait to make movie until after images are rendered

  if(render_status == RENDER_ON)return;

  if(render_filetype==JPEG){
    strcpy(image_ext, ".jpg");
  }
  else{
    strcpy(image_ext, ".png");
  }

// if the first frame doesn't exist then generate images

  strcpy(frame0, render_file_base);
  strcat(frame0, "_0001");
  strcat(frame0, image_ext);
  if(runscript==0&& FILE_EXISTS(frame0)==NO){
    RenderCB(RENDER_START_NORMAL);
    return;
  }

// construct full pathname of movie

  GetMovieFilePath(moviefile_path);

// add -y option if overwriting movie file

  if(overwrite_movie == 1){
    strcpy(overwrite_flag, "-y ");
  }
  else{
    strcpy(overwrite_flag, "");
    if(FILE_EXISTS(moviefile_path) == YES&&script_dir_path==NULL){
       PRINTF("*** Warning: The movie file %s exists.  Set movie overwrite checkbox in movie dialog box.\n",moviefile_path);
       make_movie_now=0;
    }
  }

  if(make_movie_now==1||output_ffmpeg_command==1){
    char power_label[100];
// construct name of frames used to make movie

    strcpy(movie_frames, render_file_base);
    strcat(movie_frames,"_%04d");
    strcat(movie_frames, image_ext);

  // form command line for making movie

    sprintf(command_line, "ffmpeg %s -r %i -i ", overwrite_flag,movie_framerate);
    strcat(command_line, movie_frames);

    if(movie_filetype==MP4||movie_filetype==MOV){ // use -crf for MP4 and MOV, use -b for AVI and WMV
      strcat(command_line, " -vcodec libx264 ");
      sprintf(power_label, " -crf %i ", movie_crf);
    }
    else{
      sprintf(power_label, " -b:v %ik ", movie_bitrate);
    }
    strcat(command_line, power_label);

    if(movie_filetype==MP4||movie_filetype==MOV)strcat(command_line, " -pix_fmt yuv420p ");
    strcat(command_line, moviefile_path);

// make movie
    if(output_ffmpeg_command==1){
      if(ffmpeg_command_filename!=NULL){
        FILE *stream_ffmpeg=NULL;

        stream_ffmpeg = fopen(ffmpeg_command_filename,"w");
        if(stream_ffmpeg!=NULL){
#ifdef WIN32
          fprintf(stream_ffmpeg,"@echo off\n");
#else
          fprintf(stream_ffmpeg,"#!/bin/bash\n");
#endif
          fprintf(stream_ffmpeg,"%s\n",command_line);
          fclose(stream_ffmpeg);
        }
      }
      printf("%s\n", command_line);
      output_ffmpeg_command=0;
    }
    if(make_movie_now==1)system(command_line);
  }

// enable movie making button

  EnableDisableMakeMovie(ON);
  EnableDisablePlayMovie();

  update_makemovie = 0;
}

/* ------------------ ResetRenderResolution ------------------------ */

void ResetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high) {
  *width_low = screenWidth;
  *height_low = screenHeight;
  *width_high = *width_low*MAX(2, resolution_multiplier);
  *height_high = *height_low*MAX(2, resolution_multiplier);
}

/* ------------------ GetRenderResolution ------------------------ */

void GetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high) {
  if (render_status==RENDER_OFF||renderW == 0 || renderH == 0) {
    *width_low = screenWidth;
    *height_low = screenHeight;
  }
  else {
    *width_low = renderW;
    *height_low = renderH;
  }
  *width_high = *width_low*MAX(2, glui_resolution_multiplier);
  *height_high = *height_low*MAX(2, glui_resolution_multiplier);
}

/* ------------------ Render ------------------------ */

void Render(int view_mode){
  if(render_status == RENDER_OFF)return;
  if(current_script_command!=NULL&&(current_script_command->command==SCRIPT_VOLSMOKERENDERALL||current_script_command->command==SCRIPT_ISORENDERALL)){
    int command;

    command = current_script_command->command;
    if(command == SCRIPT_VOLSMOKERENDERALL || command == SCRIPT_ISORENDERALL){
      if((render_frame[itimes] > 0 && stereotype == STEREO_NONE) || (render_frame[itimes] > 1 && stereotype != STEREO_NONE)){
        if(itimes == 0){
          current_script_command->remove_frame = itimes;
          current_script_command->exit = 1;
          stept = 0;
          return;
        }
      }
      //  render_frame[itimes]++; //xxx check whether this is needed
      if((render_frame[itimes] > 0 && stereotype == STEREO_NONE) || (render_frame[itimes] > 1 && stereotype != STEREO_NONE)){
        current_script_command->remove_frame = itimes;
      }
    }
  }
  if(render_times == RENDER_ALLTIMES && render_status == RENDER_ON&&render_mode == RENDER_NORMAL && plotstate == DYNAMIC_PLOTS && nglobal_times > 0){
    if(itimes>=0&&itimes<nglobal_times&&
     ((render_frame[itimes] == 0&&stereotype==STEREO_NONE)||(render_frame[itimes]<2&&stereotype!=STEREO_NONE))
     ){
      render_frame[itimes]++;
      RenderFrame(view_mode);
    }
    else{
      ASSERT(render_skip>0);
      RenderState(RENDER_OFF);
    }
  }

  if(render_times == RENDER_SINGLETIME){
    RenderFrame(view_mode);
    if(render_mode == RENDER_NORMAL){
      RenderState(RENDER_OFF);
      SNIFF_ERRORS("after render");
    }
  }

  if(script_render==1){
    script_render=0;
    RenderState(RENDER_OFF);
  }
}

/* ------------------ GetRenderFileName ------------------------ */

int GetRenderFileName(int view_mode, char *renderfile_dir, char *renderfile_full){
  char renderfile_name[1024], renderfile_suffix[1024], *renderfile_ext;
  int use_scriptfile;

  // construct filename for image to be rendered

  strcpy(renderfile_dir, ".");
  strcpy(renderfile_suffix, "");
  use_scriptfile = 0;

  // filename base

  if(current_script_command == NULL){
    strcpy(renderfile_name, render_file_base);
  }
  else{
    int command;

    command = current_script_command->command;

    if(
      (command == SCRIPT_RENDERONCE || command == SCRIPT_RENDERALL ||
        command == SCRIPT_RENDER360ALL || command == SCRIPT_VOLSMOKERENDERALL || command == SCRIPT_ISORENDERALL
        ) &&
      current_script_command->cval2 != NULL
      ){
      strcpy(renderfile_name, current_script_command->cval2);
      use_scriptfile = 1;
    }
    else{
      strcpy(renderfile_name, fdsprefix);
    }
    if(script_dir_path != NULL&&strlen(script_dir_path) > 0){
      if(strlen(script_dir_path) == 2 && script_dir_path[0] == '.'&&script_dir_path[1] == dirseparator[0]){
      }
      else{
        strcpy(renderfile_dir, script_dir_path);
      }
    }
  }

  // directory

  if(Writable(renderfile_dir) == NO){
    if(Writable(smokeviewtempdir) == YES){
      strcpy(renderfile_dir, smokeviewtempdir);
    }
    else{
      if(smokeviewtempdir!=NULL&&strlen(smokeviewtempdir)>0){
        fprintf(stderr, "*** Error: unable to render screen image to either directories %s or %s\n",
        renderfile_dir,smokeviewtempdir);
      }
      else{
        fprintf(stderr, "*** Error: unable to render screen image to directory %s \n",renderfile_dir);
      }
      return 1;
    }
  }

  // filename suffix

  if(use_scriptfile == 0 ||
    (current_script_command != NULL &&
    (current_script_command->command == SCRIPT_RENDERALL ||
      current_script_command->command == SCRIPT_RENDER360ALL ||
      current_script_command->command == SCRIPT_VOLSMOKERENDERALL ||
      current_script_command->command == SCRIPT_ISORENDERALL
      ))){
    int image_num;
    char suffix[20];

    strcpy(renderfile_suffix, "_");
    if(RenderTime == 0){
      image_num = seqnum;
    }
    else{
      image_num = itimes;
    }
    if(render_label_type == RENDER_LABEL_FRAMENUM || RenderTime == 0){
      float time_local;
      int code;

      if(RenderTime == 0){
        sprintf(suffix, "s%04i", image_num);
      }
      else{
        sprintf(suffix, "%04i", image_num);
      }
      code = GetPlot3dTime(&time_local);
      if(code == 1 && render_label_type == RENDER_LABEL_TIME){
        char timelabel_local[20], *timelabelptr, dt = 1.0;

        timelabelptr = Time2TimeLabel(time_local, dt, timelabel_local);
        strcat(suffix, "_");
        strcat(suffix, timelabelptr);
        strcat(suffix, "s");
      }
    }
    else{
      float time_local;
      char timelabel_local[20], *timelabelptr;
      float dt;

      time_local = global_times[itimes];
      dt = global_times[1] - global_times[0];
      if(dt < 0.0)dt = -dt;
      timelabelptr = Time2TimeLabel(time_local, dt, timelabel_local);
      strcpy(suffix, timelabelptr);
      strcat(suffix, "s");
    }
    switch(view_mode){
    case VIEW_CENTER:
    case VIEW_RIGHT:
      if(RenderTime == 0)seqnum++;
      break;
    case VIEW_LEFT:
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
    strcat(renderfile_suffix, suffix);
  }

  // filename extension

  switch(render_filetype){
  case PNG:
    renderfile_ext = ext_png;
    break;
  case JPEG:
    renderfile_ext = ext_jpg;
    break;
  default:
    render_filetype = PNG;
    renderfile_ext = ext_png;
    break;
  }

  // form full filename from parts

  strcpy(renderfile_full, renderfile_name);
  if(strlen(renderfile_suffix) > 0)strcat(renderfile_full, renderfile_suffix);
  strcat(renderfile_full, renderfile_ext);
  return 0;
}

/* ------------------ OutputSliceData ------------------------ */

void OutputSliceData(void){
  FILE *fileout;
  char datafile[1024];
  int i, ii, n;
  float *data;
  slicedata *sd;
  int row, col;
  char *ext;
  char flabel[256];

  InitSliceData();

  for(ii = 0; ii < nslice_loaded; ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->display == 0 || sd->slicefile_labelindex != slicefile_labelindex)continue;
    if(sd->times[0] > global_times[itimes])continue;

    if(sd->qslicedata == NULL){
      PRINTF("  Slice data unavailable for output\n");
      continue;
    }
    data = sd->qslicedata + sd->itime*sd->nsliceijk;
    strcpy(datafile, sd->file);
    ext = strstr(datafile, ".");
    if(ext != NULL){
      ext[0] = 0;
    }
    sprintf(flabel, "%i", itimes);
    TrimBack(flabel);
    strcat(datafile, "_sf_");
    strcat(datafile, flabel);
    strcat(datafile, ".csv");
    fileout = fopen(datafile, "a");
    if(fileout == NULL)continue;
    if(global_times != NULL)fprintf(fileout, "%f\n", global_times[itimes]);
    switch (sd->idir){
    case XDIR:
      fprintf(fileout, "%i,%i\n", sd->ks2 + 1 - sd->ks1, sd->js2 + 1 - sd->js1);
      for(row = sd->ks1; row <= sd->ks2; row++){
        for(col = sd->js1; col <= sd->js2; col++){
          n = (col - sd->js1)*sd->nslicek + row - sd->ks1;
          if(col != sd->js2)fprintf(fileout, "%f, ", data[n]);
          if(col == sd->js2)fprintf(fileout, "%f ", data[n]);
        }
        fprintf(fileout, "\n");
      }
      break;
    case YDIR:
      fprintf(fileout, "%i, %i \n", sd->ks2 + 1 - sd->ks1, sd->is2 + 1 - sd->is1);
      for(row = sd->ks1; row <= sd->ks2; row++){
        for(col = sd->is1; col <= sd->is2; col++){
          n = (col - sd->is1)*sd->nslicek + row - sd->ks1;
          if(col != sd->is2)fprintf(fileout, "%f, ", data[n]);
          if(col == sd->is2)fprintf(fileout, "%f ", data[n]);
        }
        fprintf(fileout, "\n");
      }
      break;
    case ZDIR:
      fprintf(fileout, "%i, %i \n", sd->js2 + 1 - sd->js1, sd->is2 + 1 - sd->is1);
      for(row = sd->js1; row <= sd->js2; row++){
        for(col = sd->is1; col <= sd->is2; col++){
          n = (col - sd->is1)*sd->nslicej + row - sd->js1;
          if(col != sd->is2)fprintf(fileout, "%f, ", data[n]);
          if(col == sd->is2)fprintf(fileout, "%f ", data[n]);
        }
        fprintf(fileout, "\n");
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
    fclose(fileout);
    fileout = NULL;

  }
}

/* ------------------ RenderFrame ------------------------ */

void RenderFrame(int view_mode){
  char renderfile_full[1024], renderfile_dir[1024];

  int woffset=0,hoffset=0;
  int screenH;

#ifdef WIN32
  SetThreadExecutionState(ES_DISPLAY_REQUIRED); // reset display idle timer to prevent screen saver from activating
#endif

  screenH = screenHeight;
  if(view_mode==VIEW_LEFT&&stereotype==STEREO_RB)return;


  if(stereotype == STEREO_LR && (view_mode == VIEW_LEFT || view_mode == VIEW_RIGHT)){
    hoffset = screenHeight / 4;
    screenH = screenHeight / 2;
    if(view_mode == VIEW_RIGHT)woffset = screenWidth;
  }

  if(GetRenderFileName(view_mode, renderfile_dir, renderfile_full)!=0)return;

  SmokeviewImage2File(renderfile_dir,renderfile_full,render_filetype,woffset,screenWidth,hoffset,screenH);
  if(RenderTime==1&&output_slicedata==1){
    OutputSliceData();
  }
}

/* ------------------ GetScreenBuffer --------- */

GLubyte *GetScreenBuffer(void){

  GLubyte *OpenGLimage=NULL;

  int x=0, y=0;

  NewMemory((void **)&OpenGLimage,screenWidth * screenHeight * sizeof(GLubyte) * 3);

  if(OpenGLimage==NULL)return NULL;

  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  /* get the image from the OpenGL frame buffer */

  glReadPixels(x, y, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, OpenGLimage);

  return OpenGLimage;

}

/* ------------------ MergeRenderScreenBuffers ------------------------ */

int MergeRenderScreenBuffers(int nfactor, GLubyte **screenbuffers){

  char renderfile[1024], renderfile_dir[1024], renderfullfile[1024];
  FILE *RENDERfile=NULL;
  gdImagePtr RENDERimage;
  unsigned int r, g, b;
  int i,j,rgb_local;
  int irow;
  int clip_left, clip_right, clip_bottom, clip_top;
  int clip_left_hat, clip_right_hat, clip_bottom_hat, clip_top_hat;
  int width_hat, height_hat;

  if(render_filetype!=PNG&&render_filetype!=JPEG)render_filetype=PNG;

  if(GetRenderFileName(VIEW_CENTER, renderfile_dir, renderfile)!=0)return 1;

  strcpy(renderfullfile,"");
  if(strcmp(renderfile_dir,".")!=0){
    int len;

    strcat(renderfullfile,renderfile_dir);
    len = strlen(renderfile_dir);
    if(len>0&&renderfile_dir[len-1]!=dirseparator[0])strcat(renderfullfile,dirseparator);
  }
  strcat(renderfullfile,renderfile);

  if(script_viewpoint_found==NO&&current_script_command!=NULL){
    FileCopy(script_error1_filename,renderfullfile);
    return 0;
  }

  RENDERfile = fopen(renderfullfile, "wb");
  if(RENDERfile == NULL){
    fprintf(stderr, "*** Error: unable to render screen image to %s", renderfullfile);
    return 1;
  }

  if(clip_rendered_scene==1){
    clip_left = render_clip_left;
    clip_right = screenWidth - render_clip_right-1;
    clip_left_hat = nfactor*clip_left;
    clip_right_hat = nfactor*(clip_right+1) - 1;

    clip_bottom = render_clip_bottom;
    clip_top = screenHeight - render_clip_top - 1;
    clip_bottom_hat = nfactor*clip_bottom;
    clip_top_hat = nfactor*(clip_top+1)-1;

    width_hat = clip_right_hat - clip_left_hat + 1;
    height_hat = clip_top_hat - clip_bottom_hat + 1;
  }
  else{
    clip_left = 0;
    clip_right = screenWidth - 1;
    clip_left_hat = 0;
    clip_right_hat = nfactor*screenWidth - 1;

    clip_bottom = 0;
    clip_top = screenHeight - 1;
    clip_bottom_hat = 0;
    clip_top_hat = nfactor*screenHeight - 1;

    width_hat = nfactor*screenWidth;
    height_hat = nfactor*screenHeight;
  }

  PRINTF("Rendering to: %s .", renderfullfile);
  RENDERimage = gdImageCreateTrueColor(width_hat,height_hat);

  for(irow=0;irow<nfactor;irow++){
    int icol, imin, imax;

    imin = irow*screenHeight;
    imax = (irow+1)*screenHeight;

    for(icol=0;icol<nfactor;icol++){
      GLubyte *p;
      int jmin, jmax;

      jmin = icol*screenWidth;
      jmax = (icol+1)*screenWidth;

      p = *screenbuffers++;
      if(clip_rendered_scene==1&&
            (jmax<clip_left_hat||jmin>clip_right_hat||imax<clip_bottom_hat||imin>clip_top_hat)){
            continue;
      }

      for(i=imin; i<imax; i++){
        for(j=jmin; j<jmax; j++){
          r=*p++; g=*p++; b=*p++;
          if(clip_rendered_scene==0||
            (clip_left_hat<=j&&j<=clip_right_hat&&clip_bottom_hat<=i&&i<=clip_top_hat)){
            rgb_local = (r<<16)|(g<<8)|b;
            gdImageSetPixel(RENDERimage,j-clip_left_hat,clip_top_hat - i,rgb_local);
          }
        }
      }
    }
  }

  /* output the image */

  switch(render_filetype){
  case PNG:
    gdImagePng(RENDERimage,RENDERfile);
    break;
  case JPEG:
    gdImageJpeg(RENDERimage,RENDERfile,-1);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  fclose(RENDERfile);

  /* free up memory used by both OpenGL and GIF images */

  gdImageDestroy(RENDERimage);
  if(render_frame != NULL&&itimes >= 0 && itimes < nglobal_times){
    render_frame[itimes]++;
  }
  if(RenderTime==1&&output_slicedata==1){
    OutputSliceData();
  }
  PRINTF(" Completed\n");
  return 0;
}

/* ------------------ GetScreenMap360 ------------------------ */

unsigned int GetScreenMap360(float *xyz, float *xx, float *yy){
  screendata *screeni;
  int ibuff;
  float xyznorm;
  int maxbuff;
  float maxcos, cosangle;
  float *view, *up, *right, t;
  float A, B;

  xyznorm = sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);

  for(ibuff = 0; ibuff < nscreeninfo; ibuff++){
    screeni = screeninfo+ibuff;
    view = screeni->view;
    cosangle = DOT3(view, xyz) / xyznorm;
    if(ibuff==0){
      maxbuff = 0;
      maxcos = cosangle;
    }
    else{
      if(cosangle>maxcos){
        maxcos = cosangle;
        maxbuff = ibuff;
      }
    }
  }

  ibuff = maxbuff;
  screeni = screeninfo + ibuff;
  view = screeni->view;
  up = screeni->up;
  right = screeni->right;

  t = DOT3(xyz,view);
  A = DOT3(xyz, right)/t;
  B = DOT3(xyz, up)/t;

  {
    int index;
    unsigned int return_val;

    *xx = screeni->nwidth*(screeni->width / 2.0 + A) / screeni->width;
    *xx = CLAMP(*xx,0,screeni->nwidth-1);

    *yy = screeni->nheight*(screeni->height / 2.0 + B) / screeni->height;
    *yy = CLAMP(*yy,0,screeni->nheight - 1);

    index = (int)(*yy)*screeni->nwidth + (int)(*xx);
    return_val = ((ibuff+1) << 24) |  index;
    return return_val;
  }
}


#define LEFT 0
#define RIGHT 1
/* ------------------ GetScreenMap360LR ------------------------ */

unsigned int GetScreenMap360LR(int side, float *xyz){
  int ibuff, imax, ix, iy, index;
  float xyznorm, maxcosangle;
  float *view, *up, *right, t;
  float A, B, cosangle;
  screendata *screeni;

  xyznorm = sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);

  maxcosangle = -2.0;
  imax = 0;
  for(ibuff = 0; ibuff < nscreeninfo; ibuff++){
    screeni = screeninfo + ibuff;
    view = screeni->view;
    cosangle = DOT3(view, xyz) / xyznorm;
    if(cosangle > maxcosangle){
      imax = ibuff;
      maxcosangle = cosangle;
    }
  }

  ibuff = imax;
  screeni = screeninfo + ibuff;
  view = screeni->view;
  up = screeni->up;
  right = screeni->right;

  t = DOT3(xyz, view);
  A = DOT3(xyz, right) / t;
  B = DOT3(xyz, up) / t;

  ix = CLAMP((screeni->nwidth/2)*(0.5 + A*t/screeni->width),0,screeni->nwidth/2-1);
  if(side == RIGHT)ix += screeni->nwidth / 2;
  iy = CLAMP( screeni->nheight*(  0.5 + B*t/screeni->height),0,screeni->nheight-1);

  index = iy*screeni->nwidth + ix;
  return (unsigned int)(((ibuff + 1) << 24) | index);
}

#ifdef pp_RENDER360_DEBUG
/* ------------------ DrawScreenInfo ------------------------ */

void DrawScreenInfo(void){
  int i;
  int j;

  if(screeninfo == NULL || update_screeninfo == 1)SetupScreeninfo();
  glPushMatrix();
  glScalef(0.5,0.5,0.5);
  glTranslatef(1.0,1.0,1.0);

  glLineWidth(10.0);
  glBegin(GL_LINES);
  for(i = 0; i < nscreeninfo; i++){
    screendata *screeni;
    float xyz[12];
    float *view, *right, *up;

    if(screenvis[i]==0)continue;
    screeni = screeninfo + i;
    view = screeni->view;
    right = screeni->right;
    up = screeni->up;

    for(j = 0; j < 3; j++){
      xyz[j+0] = view[j] - right[j]/2.0 - up[j] / 2.0;
      xyz[j+3] = view[j] + right[j] / 2.0 - up[j] / 2.0;
      xyz[j+6] = view[j] + right[j] / 2.0 + up[j] / 2.0;
      xyz[j+9] = view[j] - right[j] / 2.0 + up[j] / 2.0
        ;
    }
    glColor3f(0.0, 0.0, 0.0);
    glVertex3fv(xyz);
    glVertex3fv(xyz+3);
    glVertex3fv(xyz+3);
    glVertex3fv(xyz+6);
    glVertex3fv(xyz+6);
    glVertex3fv(xyz+9);
    glVertex3fv(xyz+9);
    glVertex3fv(xyz);
  }
  glEnd();
  glBegin(GL_POINT);
  glPointSize(10.0);
  glVertex3f(0.0,0.0,0.0);
  glEnd();
  glPopMatrix();
}
#endif

/* ------------------ SetupScreeninfo ------------------------ */

void SetupScreeninfo(void){
  int ibuf;

  update_screeninfo = 0;
  nscreeninfo = 26;
  FREEMEMORY(screeninfo);
  NewMemory((void **)&screeninfo, nscreeninfo * sizeof(screendata));

  for(ibuf = 0; ibuf < nscreeninfo; ibuf++){
    screendata *screeni;
    float azimuth, elevation;
    float *right, *view, *up;
    float sina, cosa;
    float cose, sine;
    float aspect_ratio;
    float aperture_width, aperture_height, aperture_diagonal;

    aperture_width = 45.0;
    screeni = screeninfo + ibuf;
    screeni->nwidth=VP_scene.width;
    screeni->nheight=VP_scene.height;
    aspect_ratio = (float)screeni->nwidth/(float)screeni->nheight;
    screeni->width=2.0*tan(DEG2RAD*aperture_width/2.0);
    screeni->height = screeni->width / aspect_ratio;
    aperture_height = 2.0*RAD2DEG*atan(screeni->height / 2.0);
    aperture_diagonal = 2.0*atan(sqrt(screeni->height*screeni->height+screeni->width*screeni->width)/2.0);
    screeni->cosmax = cos(aperture_diagonal/2.0);

    azimuth = 0.0;
    elevation = 0.0;
    if(ibuf == 0){
      azimuth = 0.0;
      elevation = -90;
    }
    else if(ibuf >= 1 && ibuf < 9){
      int ii;

      ii = ibuf - 1;
      azimuth = ii*aperture_width;
      elevation = -90.0+aperture_height;
    }
    else if(ibuf >= 9 && ibuf < 17){
      int ii;

      ii = ibuf - 9;
      azimuth = ii*aperture_width;
      elevation = -90.0 + 2.0*aperture_height;
    }
    else if(ibuf >= 17 && ibuf < 25){
      int ii;

      ii = ibuf - 17;
      azimuth = ii*aperture_width;
      elevation = -90.0 + 3.0*aperture_height;
    }
    else if(ibuf == 25){
      azimuth = 0.0;
      elevation = -90.0 + 4.0*aperture_height;
    }

    cosa = cos(DEG2RAD*azimuth);
    sina = sin(DEG2RAD*azimuth);
    cose = cos(DEG2RAD*elevation);
    sine = sin(DEG2RAD*elevation);

    view = screeni->view;
    view[0] = sina*cose;
    view[1] = cosa*cose;
    view[2] = sine;

    up = screeni->up;
    if(ABS(sine) < 0.001){
      up[0] = 0.0;
      up[1] = 0.0;
      up[2] = 1.0;
    }
    else{
      float denom;

      denom = sqrt(1.0 + cose*cose/(sine*sine));
      up[0] = -sina / denom;
      up[1] = -cosa / denom;
      up[2] = (cose / sine) / denom;
    }

    right = screeni->right;
    if(sine < 0.0){
      right[0] = -cosa;
      right[1] = sina;
      right[2] = 0.0;
    }
    else{
      right[0] = cosa;
      right[1] = -sina;
      right[2] = 0.0;
    }
  }

  FREEMEMORY(screenmap360);
  FREEMEMORY(screenmap360IX);
  FREEMEMORY(screenmap360IY);
  NewMemory((void **)&screenmap360, nwidth360*nheight360 * sizeof(unsigned int));
  NewMemory((void **)&screenmap360IX, nwidth360*nheight360*sizeof(float));
  NewMemory((void **)&screenmap360IY, nwidth360*nheight360*sizeof(float));
  {
    int i,j;
    float *cos_az, *sin_az, *cos_elev, *sin_elev;
    float dazimuth;
    int nazimuth;

    NewMemory((void **)&sin_az, nwidth360 * sizeof(float));
    NewMemory((void **)&cos_az, nwidth360 * sizeof(float));
    NewMemory((void **)&sin_elev, nheight360 * sizeof(float));
    NewMemory((void **)&cos_elev, nheight360 * sizeof(float));

    nazimuth = nwidth360;
    if(stereotype == STEREO_LR)nazimuth /= 2;
    dazimuth = 360.0/(float)nazimuth;
    for(i = 0; i < nazimuth; i++){
      float alpha;

      alpha = -180.0 + (float)i*dazimuth;
      sin_az[i] = sin(DEG2RAD*alpha);
      cos_az[i] = cos(DEG2RAD*alpha);
    }
    for(i = 0; i < nheight360; i++){
      float eps;

      eps = -90.0 + (float)i*180.0 / (float)nheight360;
      sin_elev[i] = sin(DEG2RAD*eps);
      cos_elev[i] = cos(DEG2RAD*eps);
    }
    for(j = 0; j < nheight360; j++){
      for(i = 0; i < nazimuth; i++){
        float xyz[3];

        xyz[0] = sin_az[i] * cos_elev[j];
        xyz[1] = cos_az[i] * cos_elev[j];
        xyz[2] = sin_elev[j];
        if(stereotype == STEREO_LR){
          screenmap360[j*nwidth360 + i] = GetScreenMap360LR(LEFT, xyz);
          screenmap360[j*nwidth360 + nazimuth + i] = GetScreenMap360LR(RIGHT, xyz);
        }
        else{
          int ij;

          ij = j*nwidth360+i;
          screenmap360[ij] = GetScreenMap360(xyz,screenmap360IX+ij, screenmap360IY+ij);
        }
      }
    }
    FREEMEMORY(sin_az);
    FREEMEMORY(cos_az);
    FREEMEMORY(sin_elev);
    FREEMEMORY(cos_elev);
  }
}

/* ------------------ MergeRenderScreenBuffers360 ------------------------ */

int MergeRenderScreenBuffers360(void){

  char renderfile[1024], renderfullfile[1024], renderfile_dir[1024];
  FILE *RENDERfile = NULL;
  gdImagePtr RENDERimage;
  int i, j, ijk360;
  int *screenbuffer360;

  if(render_filetype!=PNG&&render_filetype!=JPEG)render_filetype=PNG;

  if(GetRenderFileName(VIEW_CENTER, renderfile_dir, renderfile)!=0)return 1;

  strcpy(renderfullfile,"");
  if(strcmp(renderfile_dir,".")!=0){
    int len;

    strcat(renderfullfile,renderfile_dir);
    len = strlen(renderfile_dir);
    if(len>0&&renderfile_dir[len-1]!=dirseparator[0])strcat(renderfullfile,dirseparator);
  }
  strcat(renderfullfile,renderfile);

  RENDERfile = fopen(renderfullfile, "wb");
  if(RENDERfile == NULL){
    fprintf(stderr, "*** Error: unable to render screen image to %s", renderfullfile);
    return 1;
  }
  PRINTF("Rendering to: %s .", renderfullfile);
  RENDERimage = gdImageCreateTrueColor(nwidth360, nheight360);
  NewMemory((void **)&screenbuffer360,nwidth360*nheight360 * sizeof(int));

  for(i=0;i<nwidth360*nheight360;i++){
    screenbuffer360[i]=0;
  }

  ijk360 = 0;
  for(j=0;j<nheight360;j++){
    for(i=0;i<nwidth360;i++){
      GLubyte *p00, *p01, *p10, *p11;
      int ibuff, rgb_local;
      screendata *screeni;
      unsigned int r, g, b;
      float xx, yy;
      int ix, iy;
      int ix2, iy2;
      float fx, fy;

      ibuff = screenmap360[ijk360] >> 24;
      if(ibuff == 0)continue;
      ibuff--;
      screeni = screeninfo + ibuff;

      xx = screenmap360IX[ijk360];
      ix = xx;
      ix2 = CLAMP(ix+1,0,screeni->nwidth-1);
      fx = 0.0;
      if(ix2>ix)fx=xx-(float)ix;

      yy = screenmap360IY[ijk360];
      iy = yy;
      iy2 = CLAMP(iy+1,0,screeni->nheight-1);
      fy = 0.0;
      if(iy2>iy)fy=yy-(float)yy;

#define AVG2(f,p0,p1) ((1.0-f)*(float)(p0) + (f)*(float)(p1))
#define AVG4(fx,fy,p00,p01,p10,p11) ((1.0-fy)*AVG2(fx,p00,p10)+(fy)*AVG2(fx,p01,p11))

#ifdef pp_RENDER360_DEBUG
      if(debug_360==1&&(j%debug_360_skip_y==0||i%debug_360_skip_x==0)){
        rgb_local = 128<<8|128;
      }
      else{
        p00  = screeni->screenbuffer+3*(iy*screeni->nwidth + ix);
        p01  = screeni->screenbuffer+3*(iy*screeni->nwidth + ix2);
        p10  = screeni->screenbuffer+3*(iy2*screeni->nwidth + ix);
        p11  = screeni->screenbuffer+3*(iy2*screeni->nwidth + ix2);
        r = AVG4(fx,fy,p00[0],p01[0],p10[0],p11[0]);
        g = AVG4(fx,fy,p00[1],p01[1],p10[1],p11[1]);
        b = AVG4(fx,fy,p00[2],p01[2],p10[2],p11[2]);
        rgb_local = (r<<16)|(g<<8)|b;
      }
#else
      p00  = screeni->screenbuffer+3*(iy*screeni->nwidth + ix);
      p01  = screeni->screenbuffer+3*(iy*screeni->nwidth + ix2);
      p10  = screeni->screenbuffer+3*(iy2*screeni->nwidth + ix);
      p11  = screeni->screenbuffer+3*(iy2*screeni->nwidth + ix2);
      r = AVG4(fx,fy,p00[0],p01[0],p10[0],p11[0]);
      g = AVG4(fx,fy,p00[1],p01[1],p10[1],p11[1]);
      b = AVG4(fx,fy,p00[2],p01[2],p10[2],p11[2]);
      rgb_local = (r<<16)|(g<<8)|b;
#endif
      screenbuffer360[ijk360]=rgb_local;
      ijk360++;
    }
  }

  ijk360 = 0;
  for(j=nheight360-1;j>=0;j--){
    for(i=0;i<nwidth360;i++){
      gdImageSetPixel(RENDERimage, i, j, screenbuffer360[ijk360++]);
    }
  }

  /* output the image */

  switch (render_filetype){
  case PNG:
    gdImagePng(RENDERimage, RENDERfile);
    break;
  case JPEG:
    gdImageJpeg(RENDERimage, RENDERfile, -1);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  fclose(RENDERfile);

  /* free up memory used by both OpenGL and GIF images */

  gdImageDestroy(RENDERimage);
  FREEMEMORY(screenbuffer360);
  if(render_frame!=NULL&&itimes>=0&&itimes<nglobal_times){
    render_frame[itimes]++;
  }
  PRINTF(" Completed\n");
  return 0;
}

/* ------------------ SetSmokeSensor ------------------------ */

void SetSmokeSensor(gdImagePtr RENDERimage, int width, int height){
  if(test_smokesensors == 1 && active_smokesensors == 1 && show_smokesensors != SMOKESENSORS_HIDDEN){
    int idev;

    for(idev = 0; idev < ndeviceinfo; idev++){
      devicedata *devicei;
      int idev_col, idev_row;
      int col_offset, row_offset;
      unsigned int red = 255 << 16;

      devicei = deviceinfo + idev;

      if(devicei->object->visible == 0)continue;
      if(strcmp(devicei->object->label, "smokesensor") != 0)continue;
      idev_row = devicei->screenijk[0];
      idev_col = devicei->screenijk[1];
      for(col_offset = -3; col_offset < 4; col_offset++){
        for(row_offset = -3; row_offset < 4; row_offset++){
          int irow, icol;

          irow = idev_row + row_offset;
          if(irow < 0)irow = 0;
          if(irow > width - 1)irow = width - 1;

          icol = height - 1 - (idev_col + col_offset);
          if(icol < 0)icol = 0;
          if(icol > height - 1)icol = height - 1;

          gdImageSetPixel(RENDERimage, irow, icol, red);
        }
      }
    }
  }
}

/* ------------------ SmokeviewImage2File ------------------------ */

int SmokeviewImage2File(char *directory, char *RENDERfilename, int rendertype, int woffset, int width, int hoffset, int height){

  FILE *RENDERfile;
  char *renderfile=NULL;
  GLubyte *OpenGLimage, *p;
  gdImagePtr RENDERimage;
  int i,j;
  int width_beg, width_end, height_beg, height_end;
  int width2, height2;

  width_beg=woffset;
  width_end=width+woffset;
  height_beg=hoffset;
  height_end=hoffset+height;
  if(clip_rendered_scene==1){
    width_beg+=render_clip_left;
    width_end-=render_clip_right;
    height_beg+=render_clip_bottom;
    height_end-=render_clip_top;
  }
  width2 = width_end-width_beg;
  height2 = height_end-height_beg;

  if(directory==NULL){
    renderfile= GetFileName(smokeviewtempdir,RENDERfilename,NOT_FORCE_IN_DIR);
  }
  else{
    renderfile= GetFileName(directory,RENDERfilename,FORCE_IN_DIR); //force
  }
  if(renderfile == NULL){
    fprintf(stderr,"*** Error: unable to render screen image to %s", RENDERfilename);
    return 1;
  }
  RENDERfile = fopen(renderfile, "wb");
  if(RENDERfile == NULL){
    fprintf(stderr,"*** Error: unable to render screen image to %s", renderfile);
    return 1;
  }
  NewMemory((void **)&OpenGLimage,width2 * height2 * sizeof(GLubyte) * 3);

  PRINTF("Rendering to: %s .",renderfile);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  /* get the image from the OpenGL frame buffer */

  glReadPixels(width_beg, height_beg, width2, height2, GL_RGB, GL_UNSIGNED_BYTE, OpenGLimage);

  /* copy the image from OpenGL memory to GIF memory */

  p = OpenGLimage;

  RENDERimage = gdImageCreateTrueColor(width2,height2);

  for(i = height2-1 ; i>=0; i--){
    for(j=0;j<width2;j++){
      unsigned int r, g, b;
      int rgb_local;

      r=*p++; g=*p++; b=*p++;
      rgb_local = (r<<16)|(g<<8)|b;
      gdImageSetPixel(RENDERimage,j,i,rgb_local);
    }
  }

  SetSmokeSensor(RENDERimage,width,height);

  // output image

  switch(rendertype){
  case PNG:
    gdImagePng(RENDERimage,RENDERfile);
    break;
  case JPEG:
    gdImageJpeg(RENDERimage,RENDERfile,-1);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }

  fclose(RENDERfile);
  FREEMEMORY(renderfile);

  gdImageDestroy(RENDERimage);
  FREEMEMORY(OpenGLimage);
  PRINTF(" Completed.\n");
  return 0;
}

/* ------------------ SVimage2var ------------------------ */
#ifdef pp_LUA
int SVimage2var(int rendertype,
    int woffset, int width, int hoffset, int height, gdImagePtr *RENDERimage){

  GLubyte *OpenGLimage, *p;
  unsigned int r, g, b;
  int i,j,rgb_local;
  int width_beg, width_end, height_beg, height_end;
  int width2, height2;

  width_beg=woffset;
  width_end=width+woffset;
  height_beg=hoffset;
  height_end=hoffset+height;
  if(clip_rendered_scene==1){
    width_beg+=render_clip_left;
    width_end-=render_clip_right;
    height_beg+=render_clip_bottom;
    height_end-=render_clip_top;
  }
  width2 = width_end-width_beg;
  height2 = height_end-height_beg;

  NewMemory((void **)&OpenGLimage,width2 * height2 * sizeof(GLubyte) * 3);
  if(OpenGLimage == NULL){
    fprintf(stderr,"*** Error allocating memory buffer for render var\n");
    return 1;
  }
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  /* get the image from the OpenGL frame buffer */

  glReadPixels(width_beg, height_beg, width2, height2, GL_RGB, GL_UNSIGNED_BYTE, OpenGLimage);

  /* copy the image from OpenGL memory to GIF memory */

  p = OpenGLimage;

  *RENDERimage = gdImageCreateTrueColor(width2,height2);

  for(i = height2-1 ; i>=0; i--){
    for(j=0;j<width2;j++){
      r=*p++; g=*p++; b=*p++;
      rgb_local = (r<<16)|(g<<8)|b;
      gdImageSetPixel(*RENDERimage,j,i,rgb_local);
    }
  }
  if(test_smokesensors==1&&active_smokesensors==1&&show_smokesensors!=SMOKESENSORS_HIDDEN){
    int idev;

    for(idev=0;idev<ndeviceinfo;idev++){
      devicedata *devicei;
      int idev_col, idev_row;
      int col_offset, row_offset;
      unsigned int red=255<<16;

      devicei = deviceinfo + idev;

      if(devicei->object->visible==0)continue;
      if(strcmp(devicei->object->label,"smokesensor")!=0)continue;
      idev_row = devicei->screenijk[0];
      idev_col = devicei->screenijk[1];
      for(col_offset=-3;col_offset<4;col_offset++){
        for(row_offset=-3;row_offset<4;row_offset++){
          int irow, icol;

          irow = idev_row+row_offset;
          if(irow<0)irow=0;
          if(irow>width-1)irow=width-1;

          icol = height - 1 - (idev_col+col_offset);
          if(icol<0)icol=0;
          if(icol>height-1)icol=height-1;

          gdImageSetPixel(*RENDERimage,irow,icol,red);
        }
      }
    }
  }

  /* free up memory used by OpenGL image */
  FREEMEMORY(OpenGLimage);
  PRINTF(" Completed.\n");
  return 0;
}
#endif

/* ------------------ ReadPicture ------------------------ */

unsigned char *ReadPicture(char *filename, int *width, int *height, int printflag){
  char *ext;
  unsigned char *returncode;
  char *filebuffer=NULL;
  int allocated;

  if(filename==NULL)return NULL;
  if(FILE_EXISTS(filename)==YES){
    filebuffer=filename;
    allocated=0;
  }
  else{
    size_t lenbuffer;

    if(texturedir==NULL){
      if(printflag==1){
        fprintf(stderr,"*** Error: texture file: %s unavailable\n",filename);
      }
      return NULL;
    }
    else{
      FILE *stream;

      lenbuffer=strlen(filename)+strlen(texturedir)+1;
      NewMemory((void **)&filebuffer,(unsigned int)(lenbuffer+1));
      allocated=1;
      strcpy(filebuffer,texturedir);
      strcat(filebuffer,dirseparator);
      strcat(filebuffer,filename);
      stream=fopen(filebuffer,"rb");
      if(stream==NULL){
        if(printflag==1){
          fprintf(stderr,"*** Error: texture file: %s unavailable\n",filebuffer);
        }
        FREEMEMORY(filebuffer);
        return NULL;
      }
      else{
        fclose(stream);
      }
    }
  }


  if(printflag==1)PRINTF("Loading texture:%s ",filebuffer);
  ext = filebuffer + strlen(filebuffer) - 4;
  if(strncmp(ext,".jpg",4)==0||strncmp(ext,".JPG",4)==0){
    returncode = ReadJPEG(filebuffer,width,height);
  }
  else if(strncmp(ext,".png",4)==0||strncmp(ext,".PNG",4)==0){
    returncode = ReadPNG(filebuffer,width,height);
  }
  else{
    if(allocated==1){
      FREEMEMORY(filebuffer);
    }
    return NULL;
  }
  if(allocated==1){
    FREEMEMORY(filebuffer);
  }
  if(printflag==1){
    if(returncode!=NULL){
      PRINTF(" - completed\n");
    }
    else{
      PRINTF(" - failed\n");
      fprintf(stderr,"*** Error: attempt to input %s failed\n",filename);
    }
  }
  return returncode;

}

/* ------------------ ReadJPEG ------------------------ */

unsigned char *ReadJPEG(const char *filename,int *width, int *height){

  FILE *file;
  gdImagePtr image;
  unsigned char *dataptr,*dptr;
  int i,j;
  unsigned int intrgb;
  int WIDTH, HEIGHT;

  file = fopen(filename, "rb");
  if(file == NULL)return NULL;
  image = gdImageCreateFromJpeg(file);
  fclose(file);
  if(image==NULL)return NULL;
  WIDTH=gdImageSX(image);
  HEIGHT=gdImageSY(image);
  *width=WIDTH;
  *height=HEIGHT;
  if( NewMemory((void **)&dataptr,(unsigned int)(4*WIDTH*HEIGHT) )==0){
    gdImageDestroy(image);
    return NULL;
  }
  dptr=dataptr;
  for(i = 0; i<HEIGHT; i++){
    for(j=0;j<WIDTH;j++){
      intrgb=(unsigned int)gdImageGetPixel(image,j,(unsigned int)(HEIGHT-(1+i)));
      *dptr++ = (intrgb>>16)&255;
      *dptr++ = (intrgb>>8)&255;
      *dptr++ = intrgb&255;
      *dptr++=0xff;
    }
  }
  gdImageDestroy(image);
  return dataptr;

}

/* ------------------ ReadPNG ------------------------ */

unsigned char *ReadPNG(const char *filename,int *width, int *height){

  FILE *file;
  gdImagePtr image;
  unsigned char *dataptr,*dptr;
  int i,j;
  unsigned int intrgb;

  file = fopen(filename, "rb");
  if(file == NULL)return NULL;
  image = gdImageCreateFromPng(file);
  fclose(file);
  *width=gdImageSX(image);
  *height=gdImageSY(image);
  if( NewMemory((void **)&dataptr,(unsigned int)(4*(*width)*(*height)) )==0){
    gdImageDestroy(image);
    return NULL;
  }
  dptr=dataptr;
  for(i = 0; i<*height; i++){
    for(j=0;j<*width;j++){
      intrgb=(unsigned int)gdImageGetPixel(image,j,(unsigned int)(*height-(1+i)));
      *dptr++ = (intrgb>>16)&255;
      *dptr++ = (intrgb>>8)&255;
      *dptr++ = intrgb&255;
      *dptr++ = 0xff;
    }
  }
  gdImageDestroy(image);
  return dataptr;

}
