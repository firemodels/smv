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

#ifdef pp_HTML

/* ------------------ GetSliceFileNodes ------------------------ */

void GetSliceFileNodes(int option, int option2, int *offset, float *verts, unsigned char *textures, int *nverts, int *tris, int *ntris, int *frame_size, int *nframes){
  int islice, nv = 0, nt = 0, count = 0;
  int ibeg, iend, itime, first=1, minsteps;
  slicedata *slicetime=NULL;

  for(islice = 0; islice<nsliceinfo; islice++){
    slicedata *slicei;

    slicei = sliceinfo+islice;
    if(slicei->loaded==0||slicei->display==0||slicei->slicefile_type!=SLICE_NODE_CENTER||slicei->volslice==1)continue;
    if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;
    slicetime = slicei;
    if(first==1){
      minsteps=slicei->ntimes;
      first = 0;
    }
    else{
      minsteps = MIN(minsteps, slicei->ntimes);
    }
    if(option2==CURRENT_TIME)break;
  }
  if(option2==ALL_TIMES){
    ibeg = 0;
    iend = minsteps;
    *nframes = iend;
  }
  else{
    ibeg = slicetime->itime;
    iend = slicetime->itime+1;
    *nframes = 1;
  }
  *frame_size = 0;
  for(itime = ibeg; itime<iend; itime++){

    for(islice = 0; islice<nsliceinfo; islice++){
      slicedata *slicei;
      int nrows, ncols;
      unsigned char *iq;

      slicei = sliceinfo+islice;

      if(slicei->loaded==0||slicei->display==0||slicei->slicefile_type!=SLICE_NODE_CENTER||slicei->volslice==1)continue;
      if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;

      // preliminary code for obtaining geometry vertices and triangles
      if(1==0){
        geomdata *geomi;
        geomlistdata *geomlisti;

        geomi = slicei->patchgeom->geominfo;
        geomlisti = geomi->geomlistinfo - 1;
        if(geomlisti->norms_defined==0){
        //  UpdatePatchGeomTriangles(slicei->patchgeom, geom_type);
        }
      }

      iq = slicei->slicelevel+itime*slicei->nsliceijk;
      switch(slicei->idir){
      case XDIR:
        ncols = slicei->nslicej;
        nrows = slicei->nslicek;
        break;
      case YDIR:
        ncols = slicei->nslicei;
        nrows = slicei->nslicek;
        break;
      case ZDIR:
        ncols = slicei->nslicei;
        nrows = slicei->nslicej;
        break;
      }
      if(nrows>1&&ncols>1){
        if(itime==ibeg){
          *frame_size += nrows*ncols;
          nv += nrows*ncols;
          nt += 2*(nrows-1)*(ncols-1);
        }
        if(option==1){
          meshdata *meshi;
          float *xplt, *yplt, *zplt;
          int plotx, ploty, plotz;
          float  constval;
          int n, i, j, k, nj, nk;
          int ii, jj, kk;

          meshi = meshinfo+slicei->blocknumber;

          xplt = meshi->xplt;
          yplt = meshi->yplt;
          zplt = meshi->zplt;
          plotx = slicei->is1;
          ploty = slicei->js1;
          plotz = slicei->ks1;

          switch(slicei->idir){
          case XDIR:
            if(itime==ibeg){
              // vertices
              constval = xplt[plotx];
              for(j = slicei->js1; j<=slicei->js2; j++){
                for(k = slicei->ks1; k<=slicei->ks2; k++){
                  *verts++ = constval;
                  *verts++ = yplt[j];
                  *verts++ = zplt[k];
                }
              }
              // triangle indices
              nk = slicei->ks2+1-slicei->ks1;
              for(j = slicei->js1; j<slicei->js2; j++){
                jj = j-slicei->js1;
                for(k = slicei->ks1; k<slicei->ks2; k++){
                  int i00, i01, i11, i10;

                  kk = k-slicei->ks1;
                  i00 = nk*(jj+0)+kk+0;
                  i01 = nk*(jj+0)+kk+1;
                  i10 = nk*(jj+1)+kk+0;
                  i11 = nk*(jj+1)+kk+1;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i10;
                  *tris++ = *offset+i11;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i11;
                  *tris++ = *offset+i01;
                }
              }
              *offset += nrows*ncols;
            }
            // textures
            for(j = slicei->js1; j<=slicei->js2; j++){
              n = (j-slicei->js1)*slicei->nslicei*slicei->nslicek-1;
              n += (plotx-slicei->is1)*slicei->nslicek;

              for(k = slicei->ks1; k<=slicei->ks2; k++){
                *textures++ = iq[++n];
              }
            }
            break;
          case YDIR:
            // vertices
            if(itime==ibeg){
              constval = yplt[ploty];
              for(i = slicei->is1; i<=slicei->is2; i++){
                for(k = slicei->ks1; k<=slicei->ks2; k++){
                  *verts++ = xplt[i];
                  *verts++ = constval;
                  *verts++ = zplt[k];
                }
              }
              // triangle indices
              nk = slicei->ks2+1-slicei->ks1;
              for(i = slicei->is1; i<slicei->is2; i++){
                ii = i-slicei->is1;
                for(k = slicei->ks1; k<slicei->ks2; k++){
                  int i00, i01, i11, i10;

                  kk = k-slicei->ks1;
                  i00 = nk*(ii+0)+kk+0;
                  i01 = nk*(ii+0)+kk+1;
                  i10 = nk*(ii+1)+kk+0;
                  i11 = nk*(ii+1)+kk+1;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i10;
                  *tris++ = *offset+i11;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i11;
                  *tris++ = *offset+i01;
                }
              }
              *offset += nrows*ncols;
            }
            // textures
            for(i = slicei->is1; i<=slicei->is2; i++){
              n = (i-slicei->is1)*slicei->nslicej*slicei->nslicek-1;
              n += (ploty-slicei->js1)*slicei->nslicek;

              for(k = slicei->ks1; k<=slicei->ks2; k++){
                *textures++ = iq[++n];
              }
            }
            break;
          case ZDIR:
            if(itime==ibeg){
              // vertices
              constval = zplt[plotz];
              for(i = slicei->is1; i<=slicei->is2; i++){
                for(j = slicei->js1; j<=slicei->js2; j++){
                  *verts++ = xplt[i];
                  *verts++ = yplt[j];
                  *verts++ = constval;
                }
              }
              // triangle indices
              nj = slicei->js2+1-slicei->js1;
              for(i = slicei->is1; i<slicei->is2; i++){
                ii = i-slicei->is1;
                for(j = slicei->js1; j<slicei->js2; j++){
                  int i00, i01, i11, i10;

                  jj = j-slicei->js1;
                  i00 = nj*(ii+0)+jj+0;
                  i01 = nj*(ii+0)+jj+1;
                  i10 = nj*(ii+1)+jj+0;
                  i11 = nj*(ii+1)+jj+1;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i10;
                  *tris++ = *offset+i11;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i11;
                  *tris++ = *offset+i01;
                }
              }
              *offset += nrows*ncols;
            }
            // textures
            for(i = slicei->is1; i<=slicei->is2; i++){
              n = (i-slicei->is1)*slicei->nslicej*slicei->nslicek-1;
              n += (plotz-slicei->ks1)*slicei->nslicey;

              for(j = slicei->js1; j<=slicei->js2; j++){
                *textures++ = iq[++n];
              }
            }
            break;
          }
        }
      }
    }
  }
  *nverts = nv;
  *ntris = nt;
}

/* ------------------ GetGeometryNodes ------------------------ */

void GetGeometryNodes(int option, int *offset, float *verts, float *norms, float *colors, int *nverts, int *tris, int *ntris){
  int i, nv = 0, nt = 0;

  for(i = 0; i<ngeominfoptrs; i++){
    geomdata *geomi;
    geomlistdata *geomlisti;

    geomi = geominfoptrs[i];

    // reject unwanted geometry

    if((geomi->fdsblock==NOT_FDSBLOCK && geomi->geomtype!=GEOM_ISO)||geomi->patchactive==1)continue;
    geomlisti = geomi->geomlistinfo-1;

    nv += geomlisti->nverts;
    nt += geomlisti->ntriangles;

    if(option==1){
      int j;
      float *xyz_in, xyz_out[3];
      float *norm_in;

      for(j = 0; j<geomlisti->nverts; j++){
        float col2[3] = {0.0, 0.0, 1.0};
        float *col;

        if(geomlisti->verts[j].ntriangles>0){
          col = geomlisti->verts[j].triangles[0]->geomsurf->color;
        }
        else{
          col = col2;
        }
        xyz_in = geomlisti->verts[j].xyz;
        norm_in = geomlisti->verts[j].vert_norm;
        NORMALIZE_XYZ(xyz_out, xyz_in);
        *verts++ = xyz_out[0];
        *verts++ = xyz_out[1];
        *verts++ = xyz_out[2];
        *norms++ = norm_in[0];
        *norms++ = norm_in[1];
        *norms++ = norm_in[2];
        *colors++ = col[0];
        *colors++ = col[1];
        *colors++ = col[2];
      }
      for(j = 0; j<geomlisti->ntriangles; j++){
        *tris++ = *offset+geomlisti->triangles[j].verts[0]-geomlisti->verts;
        *tris++ = *offset+geomlisti->triangles[j].verts[1]-geomlisti->verts;
        *tris++ = *offset+geomlisti->triangles[j].verts[2]-geomlisti->verts;
      }
      *offset += geomlisti->nverts;
    }
  }
  *nverts = nv;
  *ntris = nt;
}


/* ------------------ GetBlockNodes ------------------------ */

void GetBlockNodes(const meshdata *meshi, blockagedata *bc, float *xyz, float *norms, int *tris){
  /*

  23---------22
  /         /
  /         /
  20--------21
  z direction
  19 ------  18
  /         /
  /         /
  16 ------ 17


  15--------14
  /         /
  /         /
  12--------13
  x direction
  11------  10
  /         /
  /         /
  8 ------ 9

  7---------6
  /         /
  /         /
  4--------5
  y direction
  3 ------  2
  /         /
  /         /
  0 ------ 1
  */
  int n;
  float xminmax[2], yminmax[2], zminmax[2];
  float *xplt, *yplt, *zplt;
  int ii[8] = {0, 1, 1, 0, 0, 1, 1, 0};
  int jj[8] = {0, 0, 1, 1, 0, 0, 1, 1};
  int kk[8] = {0, 0, 0, 0, 1, 1, 1, 1};

  int inds[36] = {
    0, 1, 5, 0, 5, 4,
    2, 3, 7, 2, 7, 6,
    1, 2, 6, 1, 6, 5,
    3, 0, 4, 3, 4, 7,
    4, 5, 6, 4, 6, 7,
    0, 2, 1, 0, 3, 2
  };

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;

  xminmax[0] = xplt[bc->ijk[IMIN]];
  xminmax[1] = xplt[bc->ijk[IMAX]];
  yminmax[0] = yplt[bc->ijk[JMIN]];
  yminmax[1] = yplt[bc->ijk[JMAX]];
  zminmax[0] = zplt[bc->ijk[KMIN]];
  zminmax[1] = zplt[bc->ijk[KMAX]];

  for(n = 0; n<8; n++){
    *xyz++ = xminmax[ii[n]];
    *xyz++ = yminmax[jj[n]];
    *xyz++ = zminmax[kk[n]];
  }
  for(n = 8; n<16; n++){
    *xyz++ = xminmax[ii[n-8]];
    *xyz++ = yminmax[jj[n-8]];
    *xyz++ = zminmax[kk[n-8]];
  }
  for(n = 16; n<24; n++){
    *xyz++ = xminmax[ii[n-16]];
    *xyz++ = yminmax[jj[n-16]];
    *xyz++ = zminmax[kk[n-16]];
  }
  for(n = 0; n<36; n++){
    int offset;

    offset = 0;
    if(n>=12)offset = 8;
    if(n>=24)offset = 16;
    *tris++ = offset+inds[n];
  }
  for(n = 0; n<72; n++){
    norms[n] = 0.0;
  }

  norms[1] = -1.0;
  norms[4] = -1.0;
  norms[7] = 1.0;
  norms[10] = 1.0;
  norms[13] = -1.0;
  norms[16] = -1.0;
  norms[19] = 1.0;
  norms[22] = 1.0;

  norms[24] = -1.0;
  norms[27] = 1.0;
  norms[30] = 1.0;
  norms[33] = -1.0;
  norms[36] = -1.0;
  norms[39] = 1.0;
  norms[42] = 1.0;
  norms[45] = -1.0;

  norms[50] = -1.0;
  norms[53] = -1.0;
  norms[56] = -1.0;
  norms[59] = -1.0;
  norms[62] = 1.0;
  norms[65] = 1.0;
  norms[68] = 1.0;
  norms[71] = 1.0;
}

/* ------------------ Lines2Geom ------------------------ */

void Lines2Geom(float **vertsptr, float **colorsptr, int *n_verts, int **linesptr, int *n_lines){
  int nverts = 0, nlines = 0, offset = 0;
  float *verts, *verts_save, *colors, *colors_save;
  int *lines, *lines_save;
  int i;

  nverts = 8*3;
  nlines = 12*2;

  if(nverts==0||nlines==0){
    *n_verts = 0;
    *n_lines = 0;
    *vertsptr = NULL;
    *colorsptr = NULL;
    *linesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&lines_save, nlines*sizeof(int));
  verts = verts_save;
  colors = colors_save;
  lines = lines_save;

  *verts++ = 0.0;
  *verts++ = 0.0;
  *verts++ = 0.0;

  *verts++ = xbar;
  *verts++ = 0.0;
  *verts++ = 0.0;

  *verts++ = xbar;
  *verts++ = ybar;
  *verts++ = 0.0;

  *verts++ = 0.0;
  *verts++ = ybar;
  *verts++ = 0.0;

  *verts++ = 0.0;
  *verts++ = 0.0;
  *verts++ = zbar;

  *verts++ = xbar;
  *verts++ = 0.0;
  *verts++ = zbar;

  *verts++ = xbar;
  *verts++ = ybar;
  *verts++ = zbar;

  *verts++ = 0.0;
  *verts++ = ybar;
  *verts++ = zbar;

  for(i = 0; i<24; i++){
    *colors++ = 0.0;
    verts_save[i] = verts_save[i];
  }

  *lines++ = 0;
  *lines++ = 4;
  *lines++ = 1;
  *lines++ = 5;
  *lines++ = 2;
  *lines++ = 6;
  *lines++ = 3;
  *lines++ = 7;

  *lines++ = 0;
  *lines++ = 1;
  *lines++ = 3;
  *lines++ = 2;
  *lines++ = 4;
  *lines++ = 5;
  *lines++ = 7;
  *lines++ = 6;

  *lines++ = 0;
  *lines++ = 3;
  *lines++ = 1;
  *lines++ = 2;
  *lines++ = 5;
  *lines++ = 6;
  *lines++ = 4;
  *lines++ = 7;

  *n_verts = nverts;
  *n_lines = nlines;
  *vertsptr = verts_save;
  *colorsptr = colors_save;
  *linesptr = lines_save;
}

/* ------------------ UnlitTriangles2Geom ------------------------ */

void UnlitTriangles2Geom(float **vertsptr, unsigned char **texturesptr, int *n_verts, int **trianglesptr, int *n_triangles, int option, int *frame_size, int *nframes){
  int j;
  int nverts = 0, ntriangles = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *triangles, *triangles_save;

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceFileNodes(0, option, NULL, NULL, NULL, &nslice_verts, NULL, &nslice_tris, frame_size, nframes);

    nverts += 3*nslice_verts;     // 3 coordinates per vertex
    ntriangles += 3*nslice_tris;  // 3 indices per triangles
  }

  if(nverts==0||ntriangles==0){
    *n_verts = 0;
    *n_triangles = 0;
    *vertsptr = NULL;
    *texturesptr = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (*frame_size*(*nframes))*sizeof(float));
  NewMemory((void **)&triangles_save, ntriangles*sizeof(int));
  verts = verts_save;
  textures = textures_save;
  triangles = triangles_save;

  // load slice file data into data structures

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceFileNodes(1, option, &offset, verts, textures, &nslice_verts, triangles, &nslice_tris, frame_size, nframes);
    verts += 3*nslice_verts;
    triangles += 3*nslice_tris;
  }

  *n_verts = nverts;
  *n_triangles = ntriangles;
  *vertsptr = verts_save;
  *texturesptr = textures_save;
  *trianglesptr = triangles_save;
}

/* ------------------ LitTriangles2Geom ------------------------ */

void LitTriangles2Geom(float **vertsptr, float **normalsptr, float **colorsptr, int *n_verts, int **trianglesptr, int *n_triangles){
  int j;
  int nverts = 0, ntriangles = 0, offset = 0;
  float *verts, *verts_save, *normals, *normals_save, *colors, *colors_save;
  int *triangles, *triangles_save;

  // count triangle vertices and indices for blockes

  for(j = 0; j<nmeshes; j++){
    meshdata *meshi;

    meshi = meshinfo+j;
    nverts += meshi->nbptrs*24*3;     // 24 vertices per blockages * 3 coordinates per vertex
    ntriangles += meshi->nbptrs*6*2*3;   // 6 faces per blockage * 2 triangles per face * 3 indicies per triangle
  }

  // count triangle vertices and indices for immersed geometry objects

  if(ngeominfoptrs>0){
    int ngeom_verts, ngeom_tris;

    LOCK_TRIANGLES;
    GetGeomInfoPtrs(0);
    UNLOCK_TRIANGLES;
    ShowHideSortGeometry(0, NULL);
    GetGeometryNodes(0, NULL, NULL, NULL, NULL, &ngeom_verts, NULL, &ngeom_tris);

    nverts += 3*ngeom_verts; // 3 coordinates per vertex
    ntriangles += 3*ngeom_tris;  // 3 indices per triangles
  }

  if(nverts==0||ntriangles==0){
    *n_verts = 0;
    *n_triangles = 0;
    *vertsptr = NULL;
    *normalsptr = NULL;
    *colorsptr = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&normals_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&triangles_save, ntriangles*sizeof(int));
  verts = verts_save;
  normals = normals_save;
  colors = colors_save;
  triangles = triangles_save;

  // load blockage info into data structures

  for(j = 0; j<nmeshes; j++){
    meshdata *meshi;
    int i;

    meshi = meshinfo+j;
    for(i = 0; i<meshi->nbptrs; i++){
      blockagedata *bc;
      float xyz[72];
      int tris[36];
      float norms[72];
      int k;

      bc = meshi->blockageinfoptrs[i];
      GetBlockNodes(meshi, bc, xyz, norms, tris);
      for(k = 0; k<24; k++){
        *verts++ = xyz[3*k+0];
        *verts++ = xyz[3*k+1];
        *verts++ = xyz[3*k+2];
        *normals++ = norms[3*k+0];
        *normals++ = norms[3*k+1];
        *normals++ = norms[3*k+2];
        *colors++ = bc->color[0];
        *colors++ = bc->color[1];
        *colors++ = bc->color[2];
      }
      for(k = 0; k<12; k++){
        *triangles++ = offset+tris[3*k+0];
        *triangles++ = offset+tris[3*k+1];
        *triangles++ = offset+tris[3*k+2];
      }
      offset += 24;
    }
  }

  // load immersed geometry info into data structures

  if(ngeominfoptrs>0){
    int ngeom_verts, ngeom_tris;

    GetGeometryNodes(1, &offset, verts, normals, colors, &ngeom_verts, triangles, &ngeom_tris);
    verts += 3*ngeom_verts;
    normals += 3*ngeom_verts;
    triangles += 3*ngeom_tris;
  }

  *n_verts = nverts;
  *n_triangles = ntriangles;
  *vertsptr = verts_save;
  *normalsptr = normals_save;
  *colorsptr = colors_save;
  *trianglesptr = triangles_save;
}

/* ------------------ GetHtmlFileName ------------------------ */

int GetHtmlFileName(char *htmlfile_full, char *htmlslicefile_full, char *htmlslicefile_base, int option){
  char htmlfile_dir[1024], htmlfile_suffix[1024];
  int image_num;

  // construct html filename

  strcpy(htmlfile_dir, ".");
  strcpy(htmlfile_suffix, "");

  // directory - put files in '.' or smokevewtempdir

  if(Writable(htmlfile_dir)==NO){
    if(Writable(smokeviewtempdir)==YES){
      strcpy(htmlfile_dir, smokeviewtempdir);
    }
    else{
      if(smokeviewtempdir!=NULL&&strlen(smokeviewtempdir)>0){
        fprintf(stderr, "*** Error: unable to output html file to either directories %s or %s\n",
          htmlfile_dir, smokeviewtempdir);
      }
      else{
        fprintf(stderr, "*** Error: unable to output html file to directory %s \n", htmlfile_dir);
      }
      return 1;
    }
  }

  // filename suffix

  if(option==CURRENT_TIME){
    if(RenderTime==0){
      image_num = seqnum;
    }
    else{
      image_num = itimes;
    }
    sprintf(htmlfile_suffix, "_%04i", image_num);
  }
  else{
    strcpy(htmlfile_suffix, "_all");
  }

  // form full filename from parts

  strcpy(htmlfile_full, html_file_base);
  strcat(htmlfile_full, htmlfile_suffix);
  strcpy(htmlslicefile_full, htmlfile_full);
  strcat(htmlfile_full, ".html");
  strcat(htmlslicefile_full, ".htmld");

  strcpy(htmlslicefile_base, html_file_base);
  strcat(htmlslicefile_base, htmlfile_suffix);
  strcat(htmlslicefile_base, ".htmld");
  return 0;
}

/* ------------------ Smv2Html ------------------------ */

int Smv2Html(char *html_file, int option){
  FILE *stream_in = NULL, *stream_out;
  float *vertsLitSolid, *normalsLitSolid, *colorsLitSolid;
  int nvertsLitSolid, *facesLitSolid, nfacesLitSolid;
  float *vertsUnlitSolid;
  unsigned char *texturesUnlitSolid;
  int nvertsUnlitSolid, *facesUnlitSolid, nfacesUnlitSolid;
  float *vertsLine, *colorsLine;
  int nvertsLine, *facesLine, nfacesLine;
  char html_fullfile[1024], html_slicefile[1024], html_slicefile_base[1024];
  int return_val;
  int copy_html;
  int frame_size, nframes;

  stream_in = fopen(smokeview_html, "r");
  if(stream_in==NULL){
    printf("***error: smokeview html template file %s failed to open\n", smokeview_html);
    return 1;
  }

  return_val = GetHtmlFileName(html_fullfile, html_slicefile, html_slicefile_base, option);
  if(return_val==1){
    fclose(stream_in);
    return 1;
  }
  stream_out = fopen(html_fullfile, "w");
  if(stream_out==NULL){
    printf("***error: html output file %s failed to open for output\n", html_fullfile);
    fclose(stream_in);
    return 1;
  }

  printf("outputting html to %s", html_fullfile);
  rewind(stream_in);

  // obtain vertices, triangles and lines

  UnlitTriangles2Geom(&vertsUnlitSolid, &texturesUnlitSolid, &nvertsUnlitSolid, &facesUnlitSolid, &nfacesUnlitSolid, option,
    &frame_size, &nframes);
  LitTriangles2Geom(&vertsLitSolid, &normalsLitSolid, &colorsLitSolid, &nvertsLitSolid, &facesLitSolid, &nfacesLitSolid);
  Lines2Geom(&vertsLine, &colorsLine, &nvertsLine, &facesLine, &nfacesLine);

#define PER_ROW 12
#define PERCOLOR_ROW 8
#define PERBIN_ROW 24
  copy_html = 1;
  for(;;){
    char buffer[255];

    if(feof(stream_in)!=0)break;

    if(fgets(buffer, 255, stream_in)==NULL)break;
    TrimBack(buffer);
    if(Match(buffer, "<!--***CANVAS")==1){
      fprintf(stream_out, "<canvas width = \"%i\" height = \"%i\" id = \"my_Canvas\"></canvas>", screenWidth, screenHeight);
      continue;
    }
    else if(Match(buffer, "//***VERTS")==1){
      int i;

      // center of scene
      fprintf(stream_out, "         var xcen=%f;\n", xbar/2.0);
      fprintf(stream_out, "         var ycen=%f;\n", ybar/2.0);
      fprintf(stream_out, "         var zcen=%f;\n", zbar/2.0);

      // add unlit triangles
      fprintf(stream_out, "         var vertices_unlit = [\n");

      for(i = 0; i<nvertsUnlitSolid; i++){
        char label[100];

        sprintf(label, "%f", vertsUnlitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsUnlitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var nframes = %i;\n", nframes);
      fprintf(stream_out, "         var frame_size = %i;\n", frame_size);
      fprintf(stream_out, "         var slice_file = \"%s\";\n", html_slicefile_base);

      if(frame_size*nframes>0){
        FILE *slicestream_out =NULL;

        slicestream_out = fopen(html_slicefile,"wb");
        if(slicestream_out!=NULL){
          fwrite(texturesUnlitSolid, sizeof(unsigned char), frame_size*nframes, slicestream_out);
          fclose(slicestream_out);
        }
      }
#ifdef HTML_FILE
      fprintf(stream_out, "         var slice_file_ready = 0;\n");
      fprintf(stream_out, "         var textures_unlit_data = new Uint8Array(nframes*frame_size);\n");
#else
      fprintf(stream_out, "         var slice_file_ready = 1;\n");
      fprintf(stream_out, "         var textures_unlit_data = [\n");
      for(i = 0; i<frame_size*nframes; i++){
        char label[100];

        sprintf(label, "%i", CLAMP((int)texturesUnlitSolid[i],0,255) );
        fprintf(stream_out, "%s,", label);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(frame_size*nframes-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");
#endif
      fprintf(stream_out, "         var textures_unlit = new Float32Array([\n");
      for(i = 0; i<frame_size; i++){
        char label[100];

        sprintf(label, "%f", CLAMP((float)texturesUnlitSolid[i]/255.0, 0.0, 1.0));
        fprintf(stream_out, "%s,", label);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(frame_size-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ]);\n");

      fprintf(stream_out, "         const texture_colorbar_data = new Uint8Array([\n");
      for(i = 0; i<256; i++){
        int ii[3];

        ii[0] = CLAMP(255*rgb_slice[4*i+0], 0, 255);
        ii[1] = CLAMP(255*rgb_slice[4*i+1], 0, 255);
        ii[2] = CLAMP(255*rgb_slice[4*i+2], 0, 255);
        fprintf(stream_out, "%i,%i,%i,255,", ii[0], ii[1], ii[2]);
        if(i%PERCOLOR_ROW==(PERCOLOR_ROW-1)||i==255)fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ]);\n");
      fprintf(stream_out, "         const texture_colorbar_data_height = 256;\n");

      fprintf(stream_out, "         var indices_unlit = [\n");
      for(i = 0; i<nfacesUnlitSolid; i++){
        fprintf(stream_out, "%i,", facesUnlitSolid[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(nfacesUnlitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      // add lit triangles
      fprintf(stream_out, "         var vertices_lit = [\n");
      for(i = 0; i<nvertsLitSolid; i++){
        char label[100];

        sprintf(label, "%f", vertsLitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var normals_lit = [\n");
      for(i = 0; i<nvertsLitSolid; i++){
        char label[100];

        sprintf(label, "%f", normalsLitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_lit = [\n");
      for(i = 0; i<nvertsLitSolid; i++){
        char label[100];

        sprintf(label, "%f", colorsLitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_lit = [\n");
      for(i = 0; i<nfacesLitSolid; i++){
        fprintf(stream_out, "%i,", facesLitSolid[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(nfacesLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      // add lines
      fprintf(stream_out, "         var vertices_line = [\n");
      for(i = 0; i<nvertsLine; i++){
        char label[100];

        sprintf(label, "%f", vertsLine[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLine-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_line = [\n");
      for(i = 0; i<nvertsLine; i++){
        char label[100];

        sprintf(label, "%f", colorsLine[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLine-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_line = [\n");
      for(i = 0; i<nfacesLine; i++){
        fprintf(stream_out, "%i,", facesLine[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(nfacesLine-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");
      continue;
    }
    else if(Match(buffer, "//HIDE_ON")==1){
      copy_html = 0;
      continue;
    }
    else if(Match(buffer, "//HIDE_OFF")==1){
      copy_html = 1;
      continue;
    }
    else if(copy_html==1)fprintf(stream_out, "%s\n", buffer);
  }

  fclose(stream_in);
  fclose(stream_out);
  printf(" - complete\n");
  return 0;
}
#endif
