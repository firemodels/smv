#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "glew.h"
#include GLUT_H

#include "smokeviewvars.h"
#include "infoheader.h"
#include "update.h"
#ifdef pp_LUA
#include "lua_api.h"
#endif

/* ------------------ Init ------------------------ */

void Init(void){
  int i;

  FREEMEMORY(plotiso);
  NewMemory((void **)&plotiso,mxplot3dvars*sizeof(int));

  for(i=0;i<16;i++){
    if(i%5==0){
      modelview_identity[i]=1.0;
    }
    else{
      modelview_identity[i]=0.0;
    }
  }
  for(i=0;i<mxplot3dvars;i++){
    plotiso[i]=nrgb/2;
  }

  for(i=0;i<16;i++){
    modelview_setup[i]=0.0;
  }
  for(i=0;i<4;i++){
    modelview_setup[i+4*i]=1.0;
  }

  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi=meshinfo+i;
    InitContour(&meshi->plot3dcontour1,rgb_plot3d_contour,nrgb);
    InitContour(&meshi->plot3dcontour2,rgb_plot3d_contour,nrgb);
    InitContour(&meshi->plot3dcontour3,rgb_plot3d_contour,nrgb);
  }

  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi=meshinfo+i;
    meshi->currentsurf.defined=0;
    meshi->currentsurf2.defined=0;
  }

  /* initialize box sizes, lighting parameters */

  xyzbox = MAX(MAX(xbar,ybar),zbar);

  {
    char name_external[32];

    strcpy(name_external,"external");
    InitCamera(camera_external,name_external);
    camera_external->view_id=EXTERNAL_LIST_ID;
  }
  if(camera_ini!=NULL&&camera_ini->defined==1){
    CopyCamera(camera_current,camera_ini);
  }
  else{
    camera_external->zoom=zoom;
    CopyCamera(camera_current,camera_external);
  }
  strcpy(camera_label,camera_current->name);
  UpdateCameraLabel();
  {
    char name_internal[32];
    strcpy(name_internal,"internal");
    InitCamera(camera_internal,name_internal);
  }
  camera_internal->eye[0]=0.5*xbar;
  camera_internal->eye[1]=0.5*ybar;
  camera_internal->eye[2]=0.5*zbar;
  camera_internal->view_id=0;
  CopyCamera(camera_save,camera_current);
  CopyCamera(camera_last,camera_current);

  InitCameraList();
  AddDefaultViews();
  CopyCamera(camera_external_save,camera_external);
  UpdateGluiCameraViewList();

  //ResetGluiView(i_view_list);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  glClearColor(backgroundcolor[0],backgroundcolor[1],backgroundcolor[2], 0.0f);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_DITHER);

  thistime=0;
  lasttime=0;

  /* define colorbar */

  UpdateRGBColors(COLORBAR_INDEX_NONE);

  block_ambient2[3] = 1.0;
  block_specular2[3] = 1.0;
  mat_ambient2[3] = 1.0;
  mat_specular2[3] = 1.0;

  ResetGluiView(startup_view_ini);
  UpdateShow();
}

/* ------------------ ReadBoundINI ------------------------ */

void ReadBoundINI(void){
  FILE *stream = NULL;
  char *fullfilename = NULL;

  if(boundinfo_filename == NULL)return;
  fullfilename = GetFileName(smokeviewtempdir, boundinfo_filename, tempdir_flag);
  if(fullfilename != NULL)stream = fopen(fullfilename, "r");
  if(stream == NULL || IsFileNewer(smv_filename, fullfilename) == 1){
    if(stream != NULL)fclose(stream);
    FREEMEMORY(fullfilename);
    return;
  }
  PRINTF("%s", _("reading: "));
  PRINTF("%s\n", fullfilename);

  while(!feof(stream)){
    char buffer[255], buffer2[255];

    CheckMemory;
    if(fgets(buffer, 255, stream) == NULL)break;

    if(Match(buffer, "B_BOUNDARY") == 1){
      float gmin, gmax;
      float pmin, pmax;
      int filetype;
      char *buffer2ptr;
      int lenbuffer2;
      int i;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%f %f %f %f %i %s", &gmin, &pmin, &pmax, &gmax, &filetype, buffer2);
      TrimBack(buffer2);
      buffer2ptr = TrimFront(buffer2);
      lenbuffer2 = strlen(buffer2ptr);
      for(i = 0; i < npatchinfo; i++){
        patchdata *patchi;

        patchi = patchinfo + i;
        if(lenbuffer2 != 0 &&
          strcmp(patchi->label.shortlabel, buffer2ptr) == 0 &&
          patchi->patch_filetype == filetype&&
          IfFirstLineBlank(boundinfo_filename) == 1){
          bounddata *boundi;

          boundi = &patchi->bounds;
          boundi->defined = 1;
          boundi->global_min = gmin;
          boundi->global_max = gmax;
          boundi->percentile_min = pmin;
          boundi->percentile_max = pmax;
        }
      }
      continue;
    }
  }
  FREEMEMORY(fullfilename);
  return;
}

/* ------------------ SetupCase ------------------------ */

int SetupCase(int argc, char **argv){
  int return_code;
  char *input_file;

  return_code=-1;
  if(strcmp(input_filename_ext,".svd")==0||demo_option==1){
    trainer_mode=1;
    trainer_active=1;
    if(strcmp(input_filename_ext,".svd")==0){
      input_file=trainer_filename;
    }
    else if(strcmp(input_filename_ext,".smt")==0){
      input_file=test_filename;
    }
    else{
      input_file=smv_filename;
    }
    return_code=ReadSMV(input_file,iso_filename);
    if(return_code==0){
      ShowGluiTrainer();
      ShowGluiAlert();
    }
  }
  else{
    input_file=smv_filename;
    return_code=ReadSMV(input_file,iso_filename);
  }
  switch(return_code){
    case 1:
      fprintf(stderr,"*** Error: Smokeview file, %s, not found\n",input_file);
      return 1;
    case 2:
      fprintf(stderr,"*** Error: problem reading Smokeview file, %s\n",input_file);
      return 2;
    case 0:
      ReadSMVDynamic(input_file);
      break;
    case 3:
      return 3;
    default:
      ASSERT(FFALSE);
  }

  /* initialize units */

  InitUnits();
  InitUnitDefs();
  SetUnitVis();

  CheckMemory;
  ReadIni(NULL);
  ReadBoundINI();
  if(use_graphics==0)return 0;
#ifdef pp_LANG
  InitTranslate(smokeview_bindir, tr_name);
#endif

  if(ntourinfo==0)SetupTour();
  GluiColorbarSetup(mainwindow_id);
  GluiMotionSetup(mainwindow_id);
  GluiBoundsSetup(mainwindow_id);
  GluiShooterSetup(mainwindow_id);
  GluiGeometrySetup(mainwindow_id);
  GluiClipSetup(mainwindow_id);
  GluiWuiSetup(mainwindow_id);
  GluiLabelsSetup(mainwindow_id);
  GluiDeviceSetup(mainwindow_id);
  GluiTourSetup(mainwindow_id);
  GluiAlertSetup(mainwindow_id);
  GluiStereoSetup(mainwindow_id);
  Glui3dSmokeSetup(mainwindow_id);

  UpdateLights(light_position0, light_position1);
  NORMALIZE_XYZ(light_position0, light_position0);
  NORMALIZE_XYZ(light_position1, light_position1);

  glutReshapeWindow(screenWidth,screenHeight);

  glutSetWindow(mainwindow_id);
  glutShowWindow();
  glutSetWindowTitle(fdsprefix);
  Init();
  GluiTrainerSetup(mainwindow_id);
  glutDetachMenu(GLUT_RIGHT_BUTTON);
  InitMenus(LOAD);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  if(trainer_mode==1){
    ShowGluiTrainer();
    ShowGluiAlert();
  }
  // intialise info header
  initialiseInfoHeader(&titleinfo, release_title, smv_githash, fds_githash,
                       chidfilebase);
  return 0;
}

/* ------------------ SetupGlut ------------------------ */

void SetupGlut(int argc, char **argv){
  int i;
  char *smoketempdir;
  size_t lensmoketempdir;
#ifdef pp_OSX
  char workingdir[1000];
#endif

// get smokeview bin directory from argv[0] which contains the full path of the smokeview binary

  NewMemory((void **)&smokeviewini,    (unsigned int)(strlen(smokeview_bindir)+14));
  STRCPY(smokeviewini,smokeview_bindir);
  STRCAT(smokeviewini,"smokeview.ini");

  startup_pass=2;

  smoketempdir=getenv("SVTEMPDIR");
  if(smoketempdir==NULL)smoketempdir=getenv("svtempdir");
  if(smoketempdir == NULL){
    char *homedir;

    homedir = getenv("HOME");
    if(homedir != NULL){
      NewMemory((void **)&smoketempdir, strlen(homedir) + strlen(dirseparator) + strlen(".smokeview") + 1);
      strcpy(smoketempdir, homedir);
      strcat(smoketempdir, dirseparator);
      strcat(smoketempdir, ".smokeview");
      if(FileExistsOrig(smoketempdir)==NO){
        if(MKDIR(smoketempdir)!=0){
          FREEMEMORY(smoketempdir);
        }
      }
    }
  }

  if(smoketempdir == NULL){
    NewMemory((void **)&smoketempdir,8);
#ifdef WIN32
    strcpy(smoketempdir,"c:\\temp");
#else
    strcpy(smoketempdir, "/tmp");
#endif
  }

  if(smoketempdir != NULL){
    lensmoketempdir = strlen(smoketempdir);
    if(NewMemory((void **)&smokeviewtempdir,(unsigned int)(lensmoketempdir+2))!=0){
      STRCPY(smokeviewtempdir,smoketempdir);
      if(smokeviewtempdir[lensmoketempdir-1]!=dirseparator[0]){
        STRCAT(smokeviewtempdir,dirseparator);
      }
      PRINTF("%s",_("Scratch directory:"));
      PRINTF(" %s\n",smokeviewtempdir);
    }
  }
#ifdef pp_BETA
  fprintf(stderr,"%s\n","\n*** This version of Smokeview is intended for review and testing ONLY. ***");
#endif

#ifdef pp_OSX
  getcwd(workingdir,1000);
#endif
  if(use_graphics==1){
    PRINTF("\n");
    PRINTF("%s\n",_("initializing Glut"));
    glutInit(&argc, argv);
    PRINTF("%s\n",_("complete"));
  }
#ifdef pp_OSX
  chdir(workingdir);
#endif

  if(use_graphics==1){
#ifdef _DEBUG
    PRINTF("%s",_("initializing Smokeview graphics window - "));
#endif
    glutInitWindowSize(screenWidth, screenHeight);
#ifdef _DEBUG
    PRINTF("%s\n",_("initialized"));
#endif

    max_screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    max_screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    if(trainer_mode==1){
      int TRAINER_WIDTH;
      int scrW, scrH;

      TRAINER_WIDTH=300;
      scrW = glutGet(GLUT_SCREEN_WIDTH)-TRAINER_WIDTH;
      scrH = glutGet(GLUT_SCREEN_HEIGHT)-50;
      SetScreenSize(&scrW,&scrH);
      max_screenWidth = screenWidth;
      max_screenHeight = screenHeight;
    }
    InitOpenGL();
  }

  NewMemory((void **)&rgbptr,MAXRGB*sizeof(float *));
  for(i=0;i<MAXRGB;i++){
    rgbptr[i]=&rgb[i][0];
  }
  NewMemory((void **)&rgb_plot3d_contour,MAXRGB*sizeof(float *));
  for(i=0;i<nrgb-2;i++){
    int ii;
    float factor;

    factor=256.0/(float)(nrgb-2);

    ii = factor*((float)i+0.5);
    if(ii>255)ii=255;
    rgb_plot3d_contour[i]=&rgb_full[ii][0];
  }
  rgb_plot3d_contour[nrgb-2]=&rgb_full[0][0];
  rgb_plot3d_contour[nrgb-1]=&rgb_full[255][0];
}

/* ------------------ GetOpenGLVersion ------------------------ */

int GetOpenGLVersion(char *version_label){
  const GLubyte *version_string;
  char version_label2[256];
  int i;
  int major=0, minor=0, subminor=0;

  version_string=glGetString(GL_VERSION);
  if(version_string==NULL){
    PRINTF("*** Warning: GL_VERSION string is NULL\n");
    return -1;
  }
  strcpy(version_label2,(char *)version_string);
  strcpy(version_label,version_label2);
  for(i=0;i<strlen(version_label2);i++){
    if(version_label2[i]=='.')version_label2[i]=' ';
  }
  sscanf(version_label2,"%i %i %i",&major,&minor,&subminor);
  if(major == 1)use_data_extremes = 0;
  if(use_data_extremes == 0){
    extreme_data_offset = 0;
    colorbar_offset = 2;
  }
  return 100*major + 10*minor + subminor;
}

/* ------------------ InitOpenGL ------------------------ */

void InitOpenGL(void){
  int type;
  int err;

  PRINTF("%s\n",_("initializing OpenGL"));

  type = GLUT_RGB|GLUT_DEPTH;
  if(buffertype==GLUT_DOUBLE){
    type |= GLUT_DOUBLE;
  }
  else{
    type |= GLUT_SINGLE;
  }

//  glutInitDisplayMode(GLUT_STEREO);
  if(stereoactive==1){
    if(glutGet(GLUT_DISPLAY_MODE_POSSIBLE)==1){
      videoSTEREO=1;
      type |= GLUT_STEREO;
    }
    else{
      videoSTEREO=0;
      fprintf(stderr,"*** Error: video hardware does not support stereo\n");
    }
  }

#ifdef _DEBUG
  PRINTF("%s",_("   Initializing Glut display mode - "));
#endif
#ifdef pp_OSXGLUT32
  type|=GLUT_3_2_CORE_PROFILE;
#endif
  glutInitDisplayMode(type);
#ifdef _DEBUG
  PRINTF("%s\n",_("initialized"));
#endif

  CheckMemory;
#ifdef _DEBUG
  PRINTF("%s\n",_("   creating window"));
#endif
  mainwindow_id = glutCreateWindow("");
#ifdef _DEBUG
  PRINTF("%s\n",_("   window created"));
#endif

#ifdef _DEBUG
  PRINTF("%s",_("   Initializing callbacks - "));
#endif
  glutSpecialUpFunc(SpecialKeyboardUpCB);
  glutKeyboardUpFunc(KeyboardUpCB);
  glutKeyboardFunc(KeyboardCB);
  glutMouseFunc(MouseCB);
  glutSpecialFunc(SpecialKeyboardCB);
  glutMotionFunc(MouseDragCB);
  glutReshapeFunc(ReshapeCB);
  glutDisplayFunc(DisplayCB);
  glutVisibilityFunc(NULL);
  glutMenuStatusFunc(MenuStatus_CB);
#ifdef _DEBUG
  PRINTF("%s\n",_("initialized"));
#endif

  opengl_version = GetOpenGLVersion(opengl_version_label);

  err=0;
 #ifdef pp_GPU
  err=glewInit();
  if(err==GLEW_OK){
    err=0;
  }
  else{
    PRINTF("   GLEW initialization failed\n");
    err=1;
  }
  if(err==0){
    if(disable_gpu==1){
      err=1;
    }
    else{
      err= InitShaders();
    }
#ifdef _DEBUG
    if(err==0){
      PRINTF("%s\n",_("  GPU shader initialization succeeded"));
    }
#endif
    if(err!=0){
      PRINTF("%s\n",_("  GPU shader initialization failed"));
    }
  }
#endif

  light_position0[0]=1.0f;
  light_position0[1]=1.0f;
  light_position0[2]=4.0f;
  light_position0[3]=0.f;

  light_position1[0]=-1.0f;
  light_position1[1]=1.0f;
  light_position1[2]=4.0f;
  light_position1[3]=0.f;

  {
    glGetIntegerv(GL_RED_BITS,&nredbits);
    glGetIntegerv(GL_GREEN_BITS,&ngreenbits);
    glGetIntegerv(GL_BLUE_BITS,&nbluebits);

    nredshift = 8 - nredbits;
    if(nredshift<0)nredshift=0;
    ngreenshift = 8 - ngreenbits;
    if(ngreenshift<0)ngreenshift=0;
    nblueshift=8-nbluebits;
    if(nblueshift<0)nblueshift=0;
  }
  opengldefined=1;
  PRINTF("%s\n\n",_("complete"));
}

/* ------------------ Set3DSmokeStartup ------------------------ */

 void Set3DSmokeStartup(void){
   int i;

    for(i=0;i<nvsliceinfo;i++){
      vslicedata *vslicei;

      vslicei = vsliceinfo + i;

      if(vslicei->loaded==1){
        vslicei->autoload=1;
      }
      else{
        vslicei->autoload=0;
      }
    }
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;

      if(parti->loaded==1){
        parti->autoload=1;
      }
      else{
        parti->autoload=0;
      }
    }
    for(i=0;i<nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = plot3dinfo + i;

      if(plot3di->loaded==1){
        plot3di->autoload=1;
      }
      else{
        plot3di->autoload=0;
      }
    }
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + i;

      if(smoke3di->loaded==1){
        smoke3di->autoload=1;
      }
      else{
        smoke3di->autoload=0;
      }
    }
    for(i=0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo + i;

      if(patchi->loaded==1){
        patchi->autoload=1;
      }
      else{
        patchi->autoload=0;
      }
    }
    for(i=0;i<nisoinfo;i++){
      isodata *isoi;

      isoi = isoinfo + i;

      if(isoi->loaded==1){
        isoi->autoload=1;
      }
      else{
        isoi->autoload=0;
      }
    }
    for(i=0;i<nsliceinfo;i++){
      slicedata *slicei;

      slicei = sliceinfo + i;

      if(slicei->loaded==1){
        slicei->autoload=1;
      }
      else{
        slicei->autoload=0;
      }
    }
 }

 /* ------------------ PutStartupSmoke3d ------------------------ */

  void PutStartupSmoke3D(FILE *fileout){
   int i;
   int nstartup;

   if(fileout==NULL)return;


   // startup particle

   nstartup=0;
   for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;

      if(parti->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"PARTAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<npartinfo;i++){
        partdata *parti;

        parti = partinfo + i;

        if(parti->loaded==1)fprintf(fileout," %i\n",parti->seq_id);
     }
   }

   // startup plot3d

   nstartup=0;
   for(i=0;i<nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = plot3dinfo + i;

      if(plot3di->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"PLOT3DAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<nplot3dinfo;i++){
        plot3ddata *plot3di;

        plot3di = plot3dinfo + i;

        if(plot3di->loaded==1)fprintf(fileout," %i\n",plot3di->seq_id);
     }
   }

   // startup iso

   nstartup=0;
   for(i=0;i<nisoinfo;i++){
      isodata *isoi;

      isoi = isoinfo + i;

      if(isoi->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"ISOAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<nisoinfo;i++){
        isodata *isoi;

        isoi = isoinfo + i;

        if(isoi->loaded==1)fprintf(fileout," %i\n",isoi->seq_id);
     }
   }

   // startup vslice

   nstartup=0;
   for(i=0;i<nvsliceinfo;i++){
      vslicedata *vslicei;

      vslicei = vsliceinfo + i;

      if(vslicei->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"VSLICEAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<nvsliceinfo;i++){
        vslicedata *vslicei;

        vslicei = vsliceinfo + i;

        if(vslicei->loaded==1)fprintf(fileout," %i\n",vslicei->seq_id);
     }
   }

   // startup slice

   nstartup=0;
   for(i=0;i<nsliceinfo;i++){
      slicedata *slicei;

      slicei = sliceinfo + i;

      if(slicei->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"SLICEAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<nsliceinfo;i++){
        slicedata *slicei;

        slicei = sliceinfo + i;
        if(slicei->loaded==1)fprintf(fileout," %i\n",slicei->seq_id);
     }
   }

   // startup mslice

   nstartup=0;
   for(i=0;i<nmultisliceinfo;i++){
      multislicedata *mslicei;

      mslicei = multisliceinfo + i;

      if(mslicei->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"MSLICEAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<nmultisliceinfo;i++){
        multislicedata *mslicei;

        mslicei = multisliceinfo + i;
        if(mslicei->loaded==1)fprintf(fileout," %i\n",i);
     }
   }

   // startup smoke

   nstartup=0;
   for(i=0;i<nsmoke3dinfo;i++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + i;

      if(smoke3di->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"S3DAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<nsmoke3dinfo;i++){
        smoke3ddata *smoke3di;

        smoke3di = smoke3dinfo + i;

        if(smoke3di->loaded==1)fprintf(fileout," %i\n",smoke3di->seq_id);
     }
   }

   // startup patch

   nstartup=0;
   for(i=0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo + i;

      if(patchi->loaded==1)nstartup++;
   }
   if(nstartup!=0){
     fprintf(fileout,"PATCHAUTO\n");
     fprintf(fileout," %i \n",nstartup);
     for(i=0;i<npatchinfo;i++){
        patchdata *patchi;

        patchi = patchinfo + i;

        if(patchi->loaded==1)fprintf(fileout," %i\n",patchi->seq_id);
     }
   }
   fprintf(fileout,"COMPRESSAUTO\n");
   fprintf(fileout," %i \n",compress_autoloaded);
 }

 /* ------------------ GetStartupPart ------------------------ */

  void GetStartupPart(int seq_id){
    int i;
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;
      if(parti->seq_id==seq_id){
        parti->autoload=1;
        return;
      }
    }
  }

 /* ------------------ GetStartupPlot3d ------------------------ */

  void GetStartupPlot3D(int seq_id){
    int i;
    for(i=0;i<nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = plot3dinfo + i;
      if(plot3di->seq_id==seq_id){
        plot3di->autoload=1;
        return;
      }
    }
  }

 /* ------------------ GetStartupBoundary ------------------------ */

  void GetStartupBoundary(int seq_id){
    int i;
    for(i=0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo + i;
      if(patchi->seq_id==seq_id){
        patchi->autoload=1;
        return;
      }
    }
  }

 /* ------------------ GetStartupSmoke ------------------------ */

  void GetStartupSmoke(int seq_id){
    int i;
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + i;

      if(smoke3di->seq_id==seq_id){
        smoke3di->autoload=1;
        return;
      }
    }
  }


 /* ------------------ GetStartupISO ------------------------ */

  void GetStartupISO(int seq_id){
    int i;
    for(i=0;i<nisoinfo;i++){
      isodata *isoi;

      isoi = isoinfo + i;

      if(isoi->seq_id==seq_id){
        isoi->autoload=1;
        return;
      }
    }
  }

 /* ------------------ GetStartupSlice ------------------------ */

  void GetStartupSlice(int seq_id){
    int i;
    for(i=0;i<nsliceinfo;i++){
      slicedata *slicei;

      slicei = sliceinfo + i;

      if(slicei->seq_id==seq_id){
        slicei->autoload=1;
        return;
      }
    }
  }

 /* ------------------ GetStartupVSlice ------------------------ */

  void GetStartupVSlice(int seq_id){
    int i;
    for(i=0;i<nvsliceinfo;i++){
      vslicedata *vslicei;

      vslicei = vsliceinfo + i;

      if(vslicei->seq_id==seq_id){
        vslicei->autoload=1;
        return;
      }
    }
  }

 /* ------------------ LoadFiles ------------------------ */

  void LoadFiles(void){
    int i;
    int errorcode;

//    ShowGluiAlert();
    for(i=0;i<nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = plot3dinfo + i;
      if(plot3di->autoload==0&&plot3di->loaded==1){
        ReadPlot3D(plot3di->file,i,UNLOAD,&errorcode);
      }
      if(plot3di->autoload==1){
        ReadPlot3dFile=1;
        ReadPlot3D(plot3di->file,i,LOAD,&errorcode);
      }
    }
    npartframes_max=GetMinPartFrames(PARTFILE_RELOADALL);
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;
      if(parti->autoload==0&&parti->loaded==1)ReadPart(parti->file, i, UNLOAD, &errorcode);
      if(parti->autoload==1)ReadPart(parti->file, i, UNLOAD, &errorcode);
    }
    for(i=0;i<npartinfo;i++){
      partdata *parti;

      parti = partinfo + i;
      if(parti->autoload==0&&parti->loaded==1)ReadPart(parti->file, i, UNLOAD, &errorcode);
      if(parti->autoload==1)ReadPart(parti->file, i, LOAD, &errorcode);
    }
    update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
    CancelUpdateTriangles();
    for(i = 0; i<nisoinfo; i++){
      isodata *isoi;

      isoi = isoinfo + i;
      if(isoi->autoload==0&&isoi->loaded==1)ReadIso(isoi->file,i,UNLOAD,NULL,&errorcode);
      if(isoi->autoload == 1){
        ReadIso(isoi->file, i, LOAD,NULL, &errorcode);
      }
    }
    if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)ReadIsoGeomWrapup(BACKGROUND);
    update_readiso_geom_wrapup = UPDATE_ISO_OFF;
    for(i = 0; i<nvsliceinfo; i++){
      vslicedata *vslicei;

      vslicei = vsliceinfo + i;
      if(vslicei->autoload==0&&vslicei->loaded==1)ReadVSlice(i,UNLOAD,&errorcode);
      if(vslicei->autoload==1){
        ReadVSlice(i,LOAD,&errorcode);
      }
    }
    // note:  only slices that are NOT a part of a vector slice will be loaded here
    {
      int last_slice;

      last_slice = nsliceinfo - 1;
      for(i = nsliceinfo-1; i >=0; i--){
        slicedata *slicei;

        slicei = sliceinfo + i;
        if((slicei->autoload == 0 && slicei->loaded == 1)||(slicei->autoload == 1 && slicei->loaded == 0)){
          last_slice = i;
          break;
        }
      }
      for(i = 0; i < nsliceinfo; i++){
        slicedata *slicei;
        int set_slicecolor;

        slicei = sliceinfo + i;
        set_slicecolor = DEFER_SLICECOLOR;
        if(i == last_slice)set_slicecolor = SET_SLICECOLOR;
        if(slicei->autoload == 0 && slicei->loaded == 1)ReadSlice(slicei->file, i, UNLOAD, set_slicecolor,&errorcode);
        if(slicei->autoload == 1 && slicei->loaded == 0)ReadSlice(slicei->file, i, LOAD, set_slicecolor, &errorcode);
      }
    }
    for(i=0;i<nterraininfo;i++){
      terraindata *terri;

      terri = terraininfo + i;
      if(terri->autoload==0&&terri->loaded==1)ReadTerrain(terri->file,i,UNLOAD,&errorcode);
      if(terri->autoload==1&&terri->loaded==0)ReadTerrain(terri->file,i,LOAD,&errorcode);
    }
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + i;
      if(smoke3di->autoload==0&&smoke3di->loaded==1)ReadSmoke3D(ALL_FRAMES,i,UNLOAD,&errorcode);
      if(smoke3di->autoload==1)ReadSmoke3D(ALL_FRAMES,i,LOAD,&errorcode);
    }
    for(i=0;i<npatchinfo;i++){
      patchdata *patchi;

      patchi = patchinfo + i;
      if(patchi->autoload==0&&patchi->loaded==1)ReadBoundary(i,UNLOAD,&errorcode);
      if(patchi->autoload==1)ReadBoundary(i,LOAD,&errorcode);
    }
    force_redisplay=1;
    UpdateFrameNumber(0);
    updatemenu=1;
    update_load_files=0;
    HideGluiAlert();
    TrainerViewMenu(trainerview);
  }

/* ------------------ InitTextureDir ------------------------ */

void InitTextureDir(void){
  char *texture_buffer;
  size_t texture_len;

  if(texturedir!=NULL)return;

  texture_buffer=getenv("texturedir");
  if(texture_buffer!=NULL){
    texture_len=strlen(texture_buffer);
    NewMemory((void **)&texturedir,texture_len+1);
    strcpy(texturedir,texture_buffer);
  }
  if(texturedir==NULL&&smokeview_bindir!=NULL){
    texture_len=strlen(smokeview_bindir)+strlen("textures");
    NewMemory((void **)&texturedir,texture_len+2);
    strcpy(texturedir,smokeview_bindir);
    strcat(texturedir,"textures");
  }
}

/* ------------------ InitScriptError ------------------------ */

void InitScriptErrorFiles(void){
  if(smokeview_bindir != NULL){
    NewMemory((void **)&script_error1_filename, strlen(smokeview_bindir)+strlen("script_error1.png") + 1);
    strcpy(script_error1_filename, smokeview_bindir);
    strcat(script_error1_filename, "script_error1.png");
  }
}

/* ------------------ InitVars ------------------------ */

void InitVars(void){
  int i;

  curdir_writable = Writable(".");
  windrose_circ.ncirc=0;
  InitCircle(180, &windrose_circ);

  object_circ.ncirc=0;
  cvent_circ.ncirc=0;

#ifdef pp_RENDER360_DEBUG
  NewMemory((void **)&screenvis, nscreeninfo * sizeof(int));
  for(i = 0; i < nscreeninfo; i++){
    screenvis[i] = 1;
  }
#endif

#ifdef pp_SPECTRAL
  GetBlackBodyColors(300.0,1200.0, blackbody_colors, 256);
#endif

  beam_color[0] = 255 * foregroundcolor[0];
  beam_color[1] = 255 * foregroundcolor[1];
  beam_color[2] = 255 * foregroundcolor[2];

  cos_geom_max_angle = cos(DEG2RAD*geom_max_angle);
  if(movie_filetype==WMV){
    strcpy(movie_ext, ".wmv");
  }
  else if(movie_filetype==MP4){
    strcpy(movie_ext, ".mp4");
  }
  else{
    strcpy(movie_ext, ".avi");
  }
  for(i=0;i<200;i++){
    face_id[i]=1;
  }
  for(i=0;i<10;i++){
    face_vis[i]=1;
    face_vis_old[i]=1;
  }
  for(i=0;i<7;i++){
    b_state[i]=-1;
  }
#ifdef pp_DEG
  degC[0]=DEG_SYMBOL;
  degC[1]='C';
  degC[2]='\0';
#else
  strcpy(degC,"C");
#endif

#ifdef pp_DEG
  degF[0]=DEG_SYMBOL;
  degF[1]='F';
  degF[2]='\0';
#else
  strcpy(degF,"F");
#endif

  strcpy(default_fed_colorbar,"FED");

  label_first_ptr = &label_first;
  label_last_ptr = &label_last;

  label_first_ptr->prev = NULL;
  label_first_ptr->next = label_last_ptr;
  strcpy(label_first_ptr->name,"first");

  label_last_ptr->prev = label_first_ptr;
  label_last_ptr->next = NULL;
  strcpy(label_last_ptr->name,"last");

  {
    labeldata *gl;

    gl=&LABEL_default;
    gl->rgb[0]=0;
    gl->rgb[1]=0;
    gl->rgb[2]=0;
    gl->rgb[3]=255;
    gl->frgb[0]=0.0;
    gl->frgb[1]=0.0;
    gl->frgb[2]=0.0;
    gl->frgb[3]=1.0;
    gl->tstart_stop[0]=0.0;
    gl->tstart_stop[1]=1.0;
    gl->useforegroundcolor=1;
    gl->show_always=1;
    strcpy(gl->name,"new");
    gl->xyz[0]=0.0;
    gl->xyz[1]=0.0;
    gl->xyz[2]=0.0;
    gl->tick_begin[0] = 0.0;
    gl->tick_begin[1] = 0.0;
    gl->tick_begin[2] = 0.0;
    gl->tick_direction[0] = 1.0;
    gl->tick_direction[1] = 0.0;
    gl->tick_direction[2] = 0.0;
    gl->show_tick = 0;
    gl->labeltype = TYPE_INI;
    memcpy(&LABEL_local,&LABEL_default,sizeof(labeldata));
  }

#ifdef pp_LANG
  strcpy(startup_lang_code,"en");
#endif
  mat_specular_orig[0]=0.5f;
  mat_specular_orig[1]=0.5f;
  mat_specular_orig[2]=0.2f;
  mat_specular_orig[3]=1.0f;
  mat_specular2=GetColorPtr(mat_specular_orig);

  mat_ambient_orig[0] = 0.5f;
  mat_ambient_orig[1] = 0.5f;
  mat_ambient_orig[2] = 0.2f;
  mat_ambient_orig[3] = 1.0f;
  mat_ambient2=GetColorPtr(mat_ambient_orig);

  ventcolor_orig[0]=1.0;
  ventcolor_orig[1]=0.0;
  ventcolor_orig[2]=1.0;
  ventcolor_orig[3]=1.0;
  ventcolor=GetColorPtr(ventcolor_orig);

  block_ambient_orig[0] = 1.0;
  block_ambient_orig[1] = 0.8;
  block_ambient_orig[2] = 0.4;
  block_ambient_orig[3] = 1.0;
  block_ambient2=GetColorPtr(block_ambient_orig);

  block_specular_orig[0] = 0.0;
  block_specular_orig[1] = 0.0;
  block_specular_orig[2] = 0.0;
  block_specular_orig[3] = 1.0;
  block_specular2=GetColorPtr(block_specular_orig);

  for(i=0;i<256;i++){
    boundarylevels256[i]=(float)i/255.0;
  }

  first_scriptfile.id=-1;
  first_scriptfile.prev=NULL;
  first_scriptfile.next=&last_scriptfile;

  last_scriptfile.id=-1;
  last_scriptfile.prev=&first_scriptfile;
  last_scriptfile.next=NULL;

#ifdef pp_LUA
  first_luascriptfile.id=-1;
  first_luascriptfile.prev=NULL;
  first_luascriptfile.next=&last_luascriptfile;

  last_luascriptfile.id=-1;
  last_luascriptfile.prev=&first_luascriptfile;
  last_luascriptfile.next=NULL;
#endif

  first_inifile.id=-1;
  first_inifile.prev=NULL;
  first_inifile.next=&last_inifile;

  last_inifile.id=-1;
  last_inifile.prev=&first_inifile;
  last_inifile.next=NULL;

  FontMenu(fontindex);

  treecharcolor[0]=0.3;
  treecharcolor[1]=0.3;
  treecharcolor[2]=0.3;
  treecharcolor[3]=1.0;
  trunccolor[0]=0.6;
  trunccolor[1]=0.2;
  trunccolor[2]=0.0;
  trunccolor[3]=1.0;

  direction_color[0]=39.0/255.0;
  direction_color[1]=64.0/255.0;
  direction_color[2]=139.0/255.0;
  direction_color[3]=1.0;

  direction_color_ptr=GetColorPtr(direction_color);
  show_slice_terrain=0;

  shooter_uvw[0]=0.0;
  shooter_uvw[1]=0.0;
  shooter_uvw[2]=0.0;
  vis_slice_contours=0;
  update_slicecontours=0;

  partfacedir[0]=0.0;
  partfacedir[1]=0.0;
  partfacedir[2]=1.0;

  select_device=0;
  selected_device_tag=-1;
  navatar_types=0;
  select_avatar=0;
  selected_avatar_tag=-1;
  select_device_color_ptr=NULL;
  avatar_types=NULL;
  navatar_colors=0;
  avatar_colors=NULL;
  view_from_selected_avatar=0;
  GetGitInfo(smv_githash,smv_gitdate);
  force_isometric=0;
  cb_valmin=0.0;
  cb_valmax=100.0;
  cb_val=50.0;
  cb_colorindex=128;

  rgb_terrain[0][0]=1.0;
  rgb_terrain[0][1]=0.0;
  rgb_terrain[0][2]=0.0;
  rgb_terrain[0][3]=1.0;

  rgb_terrain[1][0]=0.5;
  rgb_terrain[1][1]=0.5;
  rgb_terrain[1][2]=0.0;
  rgb_terrain[1][3]=1.0;

  rgb_terrain[2][0]=0.0;
  rgb_terrain[2][1]=1.0;
  rgb_terrain[2][2]=0.0;
  rgb_terrain[2][3]=1.0;

  rgb_terrain[3][0]=0.0;
  rgb_terrain[3][1]=0.5;
  rgb_terrain[3][2]=0.0;
  rgb_terrain[3][3]=1.0;

  rgb_terrain[4][0]=0.0;
  rgb_terrain[4][1]=0.5;
  rgb_terrain[4][2]=0.5;
  rgb_terrain[4][3]=1.0;

  rgb_terrain[5][0]=0.0;
  rgb_terrain[5][1]=0.0;
  rgb_terrain[5][2]=1.0;
  rgb_terrain[5][3]=1.0;

  rgb_terrain[6][0]=0.5;
  rgb_terrain[6][1]=0.0;
  rgb_terrain[6][2]=0.5;
  rgb_terrain[6][3]=1.0;

  rgb_terrain[7][0]=1.0;
  rgb_terrain[7][1]=0.5;
  rgb_terrain[7][2]=0.0;
  rgb_terrain[7][3]=1.0;

  rgb_terrain[8][0]=1.0;
  rgb_terrain[8][1]=0.5;
  rgb_terrain[8][2]=0.5;
  rgb_terrain[8][3]=1.0;

  rgb_terrain[9][0]=1.0;
  rgb_terrain[9][1]=0.25;
  rgb_terrain[9][2]=0.5;
  rgb_terrain[9][3]=1.0;

  percentile_level=0.01;

  strcpy(script_inifile_suffix,"");
  strcpy(script_renderdir,"");
  strcpy(script_renderfilesuffix,"");
  strcpy(script_renderfile,"");
  trainerview=1;
  show_bothsides_int=1;
  show_bothsides_ext=0;
  skip_slice_in_embedded_mesh=0;
  offset_slice=0;
  trainer_pause=0;
  trainee_location=0;
  trainer_inside=0;
  from_glui_trainer=0;
  trainer_path_old=-3;
  trainer_outline=1;
  trainer_viewpoints=-1;
  ntrainer_viewpoints=0;
  trainer_realtime=1;
  trainer_path=0;
  trainer_xzy[0]=0.0;
  trainer_xzy[1]=0.0;
  trainer_xzy[2]=0.0;
  trainer_ab[0]=0.0;
  trainer_ab[1]=0.0;
  motion_ab[0]=0.0;
  motion_ab[1]=0.0;
  motion_dir[0]=0.0;
  motion_dir[1]=0.0;
  fontsize_save=0;
  trainer_mode=0;
  trainer_active=0;
  show_slice_average=0;
  vis_slice_average=1;
  slice_average_interval=10.0;

  show_transparent_vents=1;
  maxtourframes=500;
  blockageSelect=0;
  stretch_var_black=0;
  stretch_var_white=0;
  move_var=0;

  xyz_dir=0;
  which_face=2;
  showfontmenu=1;

  glui_active=0;

  vis3DSmoke3D=1;
  smokeskip=1;
  smokeskipm1=0;
  nrooms=0;
  nzoneinfo=0;
  nfires=0;

  windowsize_pointer=0;
  fontindex=0;

  xbar=1.0, ybar=1.0, zbar=1.0;
  xbar0=0.0, ybar0=0.0, zbar0=0.0;
  xbarORIG=1.0, ybarORIG=1.0, zbarORIG=1.0;
  xbar0ORIG=0.0, ybar0ORIG=0.0, zbar0ORIG=0.0;
  ReadPlot3dFile=0, ReadIsoFile=0;

  ReadVolSlice=0;
  Read3DSmoke3DFile=0;
  ReadZoneFile=0, ReadPartFile=0, ReadEvacFile=0;;

  editwindow_status=-1;
  startup_pass=1;

  slicefilenumber=0;
  exportdata=0;
  nspr=0;
  smoke3dframestep=1;
  smoke3dframeskip=0;
  vectorskip=1;
  rotation_type=ROTATION_2AXIS;
  eyeview_level=1;
  rotation_type_old=ROTATION_2AXIS,eyeview_SAVE=0,eyeview_last=0;
  frameratevalue=1000;
  setpartmin=PERCENTILE_MIN, setpartmax=PERCENTILE_MAX;
  setpartmin_old=setpartmin;
  setpartmax_old=setpartmax;
  setpatchmin=GLOBAL_MIN, setpatchmax=GLOBAL_MAX;
  settargetmin=0, settargetmax=0;
  setpartchopmin=0, setpartchopmax=0;
  partchopmin=1.0,  partchopmax=0.;
  slicechopmin=0, slicechopmax=0;

  temp_threshold=400.0;
  vis_onlythreshold=0, vis_threshold=0;
  activate_threshold=1;
  canshow_threshold=1;
  settmin_p=0, settmin_b=0, settmin_s=0, settmin_z=0, settmin_i=0;
  settmax_p=0, settmax_b=0, settmax_s=0, settmax_z=0, settmax_i=0;
  tmin_p=1., tmin_b=1., tmin_s=1., tmin_z=1., tmin_i=1.;
  tmax_p=0., tmax_b=0., tmax_s=0., tmax_z=0., tmax_i=0.;
  speedmax=0.0;
  hrrpuv_max_smv=1200.0;
  FlowDir=1,ClipDir=1;
  plotn=1;
  stept=0;
  plotstate=NO_PLOTS;
  visVector=0;
  visSmokePart=2, visSprinkPart=1, havesprinkpart=0;
  visaxislabels=0;
  numplot3dvars=0;
  p3dsurfacesmooth=1;
  parttype=0;
  allinterior=1;
  hrrpuv_iso_color[0]=1.0;
  hrrpuv_iso_color[1]=0.5;
  hrrpuv_iso_color[2]=0.0;
  hrrpuv_iso_color[3]=1.0;
  showterrain=0;
  showgluitrainer=0;
  colorbartype=0;
  colorbartype_ini=-1;
  UpdateCurrentColorbar(colorbarinfo);
  colorbartype_save=colorbartype;
  colorbartype_default=colorbartype;
  colorbarpoint=0;
  vectorspresent=0;

  smokediff=0;
  smoke3d_cvis=1.0;
  test_smokesensors=0;
  active_smokesensors=0;
  loadplot3dall=0;
  visAIso=1;
  surfincrement=0,visiso=0;
  isotest=0;
  isolevelindex=0, isolevelindex2=0;
  pref=101325.,pamb=0.,tamb=293.15;
  ntc_total=0, nspr_total=0, nheat_total=0;
  n_devices=0;

  npartinfo=0, nsliceinfo=0, nvsliceinfo=0, npatch2=0, nplot3dinfo=0, npatchinfo=0;
  nevac=0;
  nsmoke3dinfo=0;
  nisoinfo=0, niso_bounds=0;
  ntrnx=0, ntrny=0, ntrnz=0,npdim=0,nmeshes=0,clip_mesh=0;
  noffset=0;
  visLabels=0;
  framerate=-1.0;
  itimes=0, itimeold=-999, seqnum=0,RenderTime=0; RenderTimeOld=0; itime_save=-1;
  nopart=1;
  uindex=-1, vindex=-1, windex=-1;

  cullfaces=1;
  showonly_hiddenfaces=0;

  windowresized=0;

  first_display=2;

  updatemenu=0;
  updatezoommenu=0;
  updatemenu_count=0;

  updatefaces=0,updatefacelists=0;
  updateOpenSMVFile=0;

  periodic_reloads=0;
  periodic_value=-2;

  slicefilenum=-1;
  partfilenum=-1,zonefilenum=-1;
  targfilenum=-1;

  setPDIM=0;
  menustatus=GLUT_MENU_NOT_IN_USE;

  vertical_factor=1.0;
  terrain_rgba_zmin[0]=90;
  terrain_rgba_zmin[1]=50;
  terrain_rgba_zmin[2]=50;

  terrain_rgba_zmax[0]=200;
  terrain_rgba_zmax[1]=200;
  terrain_rgba_zmax[2]=200;

#ifdef pp_memstatus
  visAvailmemory=0;
#endif
  visBlocks=visBLOCKAsInput;
  visTransparentBlockage=0;
  visBlocksSave=visBLOCKAsInput;
  blocklocation=BLOCKlocation_grid;
  ncadgeom=0;
  visFloor=0, visFrame=1;
  visNormalEditColors=1;
  visWalls=0, visGrid=0, visCeiling=0, cursorPlot3D=0;
  visSensor=1, visSensorNorm=1, hasSensorNorm=0;
  partframestep=1, sliceframestep=1, boundframestep=1;
  partframeskip=0, sliceframeskip=0, boundframeskip=0;
  boundzipstep=1, boundzipskip=0;
  smoke3dzipstep=1, smoke3dzipskip=0;
  isozipstep=1, isozipskip=0;
  slicezipstep=1, slicezipskip=0;
  evacframeskip=0, evacframestep=1;
  render_window_size=RenderWindow;
  RenderMenu(render_window_size);
  viewoption=0;

  partpointsize=4.0,vectorpointsize=3.0,streaklinewidth=1.0;
  isopointsize=4.0;
  isolinewidth=2.0;
  plot3dpointsize=4.0;
  plot3dlinewidth=2.0;
  sprinklerabssize=0.076f, sensorabssize=0.038f, heatabssize=0.076f;

  linewidth=2.0, ventlinewidth=2.0, highlight_linewidth=4.0;
  solidlinewidth=linewidth;
  visBLOCKold=-1;

  nrgb=NRGB;
  nrgb_ini=0;
  nrgb2_ini=0;
  rgb_white=NRGB, rgb_yellow=NRGB+1, rgb_blue=NRGB+2, rgb_red=NRGB+3;
  rgb_green=NRGB+4, rgb_magenta=NRGB+5, rgb_cyan=NRGB+6, rgb_black=NRGB+7;
  setbw=0;
  setbwSAVE=setbw;
  antialiasflag=1;
  nrgb_full=256;
  nrgb_cad=256;
  eyexfactor=0.5f, eyeyfactor=-0.9f, eyezfactor=0.5f;

  frameinterval=1.0;

  use_tload_begin=0;
  use_tload_end=0;
  use_tload_skip=0;
  tload_begin=0.0;
  tload_end=1.0;
  tload_skip=0;

  blockages_dirty=0;
  usetextures=0;
  canrestorelastview=0;
  ntargets=0;
  endian_data=0, endian_native=0, setendian=0;

  mainwindow_id=0;
  rendertourcount=0;

  static_color[0]=0.0;
  static_color[1]=1.0;
  static_color[2]=0.0;
  static_color[3]=1.0;

  sensorcolor[0]=1.0;
  sensorcolor[1]=1.0;
  sensorcolor[2]=0.0;
  sensorcolor[3]=1.0;


  sensornormcolor[0]=1.0;
  sensornormcolor[1]=1.0;
  sensornormcolor[2]=0.0;
  sensornormcolor[3]=1.0;


  sprinkoncolor[0]=0.0;
  sprinkoncolor[1]=1.0;
  sprinkoncolor[2]=0.0;
  sprinkoncolor[3]=1.0;

  sprinkoffcolor[0]=1.0; //xxxx check
  sprinkoffcolor[1]=0.0;
  sprinkoffcolor[2]=0.0;
  sprinkoffcolor[3]=1.0;


  heatoncolor[0]=1.0; //xxx check
  heatoncolor[1]=0.0;
  heatoncolor[2]=0.0;
  heatoncolor[3]=1.0;

  heatoffcolor[0]=1.0;
  heatoffcolor[1]=0.0;
  heatoffcolor[2]=0.0;
  heatoffcolor[3]=1.0;

  backgroundbasecolor[0]=0.0;
  backgroundbasecolor[1]=0.0;
  backgroundbasecolor[2]=0.0;
  backgroundbasecolor[3]=1.0;

  glui_backgroundbasecolor[0] = 255 * backgroundbasecolor[0];
  glui_backgroundbasecolor[1] = 255 * backgroundbasecolor[1];
  glui_backgroundbasecolor[2] = 255 * backgroundbasecolor[2];
  glui_backgroundbasecolor[3] = 255 * backgroundbasecolor[3];

  backgroundcolor[0]=0.0;
  backgroundcolor[1]=0.0;
  backgroundcolor[2]=0.0;
  backgroundcolor[3]=1.0;

  foregroundbasecolor[0]=1.0;
  foregroundbasecolor[1]=1.0;
  foregroundbasecolor[2]=1.0;
  foregroundbasecolor[3]=1.0;

  glui_foregroundbasecolor[0] = 255 * foregroundbasecolor[0];
  glui_foregroundbasecolor[1] = 255 * foregroundbasecolor[1];
  glui_foregroundbasecolor[2] = 255 * foregroundbasecolor[2];
  glui_foregroundbasecolor[3] = 255 * foregroundbasecolor[3];

  foregroundcolor[0]=1.0;
  foregroundcolor[1]=1.0;
  foregroundcolor[2]=1.0;
  foregroundcolor[3]=1.0;

  boundcolor[0]=0.5;
  boundcolor[1]=0.5;
  boundcolor[2]=0.5;
  boundcolor[3]=1.0;

  timebarcolor[0]=0.6;
  timebarcolor[1]=0.6;
  timebarcolor[2]=0.6;
  timebarcolor[3]=1.0;

  redcolor[0]=1.0;
  redcolor[1]=0.0;
  redcolor[2]=0.0;
  redcolor[3]=1.0;

  loadfiles_at_startup=0;

  nmenus=0;
  showbuild=0;

  strcpy(emptylabel,"");
  large_font=GLUT_BITMAP_HELVETICA_12;
  small_font=GLUT_BITMAP_HELVETICA_10;

  texture_origin[0]=0.0;
  texture_origin[1]=0.0;
  texture_origin[2]=0.0;

  isZoneFireModel=0;
  nunitclasses=0,nunitclasses_default=0,nunitclasses_ini=0;
  callfrom_tourglui=0;
  showtours_whenediting=0;

  right_green=0.0;
  right_blue=1.0;
  apertureindex=1;
  zoomindex=2;
  projection_type=PROJECTION_PERSPECTIVE;
  apertures[0]=30.;
  apertures[1]=45.;
  apertures[2]=60.;
  apertures[3]=75.;
  apertures[3]=90.;
  planar_terrain_slice=0;

  zooms[0]=0.25;
  zooms[1]=0.5;
  zooms[2]=1.0;
  zooms[3]=2.0;
  zooms[4]=4.0;
  zoom=1.0;
  aperture = Zoom2Aperture(zoom);
  aperture_glui = aperture;
  aperture_default = aperture;

  {
    int ii;
    rgbmask[0]=1;
    for(ii=1;ii<16;ii++){
      rgbmask[ii]=2*rgbmask[ii-1]+1;
    }
  }

  sv_age=0;
  titlesafe_offset=0;
  titlesafe_offsetBASE=45;
  reset_frame=0;
  reset_time=0.0,start_frametime=0.0,stop_frametime=0.0;
  reset_time_flag=0;

  nsorted_surfidlist=0;

  overwrite_all=0,erase_all=0;
  compress_autoloaded=0;
  strcpy(ext_png,".png");
  strcpy(ext_jpg,".jpg");
  render_filetype=PNG;
  strcpy(part_ext,".part");
  strcpy(ini_ext,".ini");

  start_xyz0[0]=0.0;
  start_xyz0[1]=0.0;
  start_xyz0[2]=0.0;
  glui_move_mode=-1;

  update_tour_list =0;
  desired_view_height=1.5;
  resetclock=1,initialtime=0;
  realtime_flag=0;
  slicefile_labelindex=-1,slicefile_labelindex_save=-1,iboundarytype=-1;
  iisotype=-1;


  cpuframe=0;

  adjustalphaflag=3;

  highlight_block=-1, highlight_mesh=0, highlight_flag=2;

  visUSERticks=0;
  user_tick_show_x=1;
  user_tick_show_y=1;
  user_tick_show_z=1;
  auto_user_tick_placement=1;

  smoke_extinct=7.600,smoke_dens=.50,smoke_pathlength=1.0;
  showall_textures=0;

  do_threshold=0;
  updateindexcolors=0;
  show_path_knots=0;
  keyframe_snap=0;
  tourrad_avatar=0.1;
  dirtycircletour=0;
  view_tstart=0.0, view_tstop=100.0;
  view_ntimes=1000;
  iavatar_evac=0;
  viewtourfrompath=0,viewalltours=0,viewanytours=0,edittour=0;
  tour_usecurrent=0;
  visFDSticks=0;
  visCadTextures=1;
  visTerrainTexture=1;
  nselectblocks=0;
  surface_indices[0]=0;
  surface_indices[1]=0;
  surface_indices[2]=0;
  surface_indices[3]=0;
  surface_indices[4]=0;
  surface_indices[5]=0;
  wall_case=0;
  strcpy(surfacedefaultlabel,"");
  mscale[0]=1.0;
  mscale[1]=1.0;
  mscale[2]=1.0;
  nearclip=0.001,farclip=3.0;
  updateclipvals=0;
  updateUpdateFrameRateMenu=0;
  ntextures=0;
  nskyboxinfo=0;
  part5show=1;
  streak5show=0;
  update_streaks=0;

  streak_rvalue[0]=0.25;
  streak_rvalue[1]=0.5;
  streak_rvalue[2]=1.0;
  streak_rvalue[3]=2.0;
  streak_rvalue[4]=4.0;
  streak_rvalue[5]=8.0;
  streak_rvalue[6]=16.0;
  streak_rvalue[7]=32.0;
  nstreak_rvalue=8;
  streak_index=-1;
  float_streak5value=0.0;
  if(streak_index>=0)float_streak5value=streak_rvalue[streak_index];

  streak5step=0;
  showstreakhead=1;
  npartclassinfo=0;
  noutlineinfo=0;
  nmultisliceinfo=0;
  nmultivsliceinfo=0;

  svofile_exists=0;
  devicenorm_length = 0.1;
  strcpy(object_def_first.label,"first");
  object_def_first.next=&object_def_last;
  object_def_first.prev=NULL;

  strcpy(object_def_last.label,"last");
  object_def_last.next=NULL;
  object_def_last.prev=&object_def_first;
  object_defs=NULL;

  showfiles=0;

  smokecullflag=1;
  visMAINmenus=0;
  smoke3d_thick=0;
#ifdef pp_GPU
  smoke3d_rthick=1.0;
  usegpu=0;
#endif
  ijkbarmax=5;
  blockage_as_input=0;
  blockage_snapped=1;
  show_cad_and_grid=0;
  nplot3dtimelist=0;

  buffertype=DOUBLE_BUFFER;
  opengldefined=0;

  GetTitle("Smokeview ", release_title);
  GetTitle("Smokeview ", plot3d_title);

  strcpy(INIfile,"smokeview.ini");
  strcpy(WRITEINIfile,"Write smokeview.ini");

  tourcol_selectedpathline[0]=1.0;
  tourcol_selectedpathline[1]=0.0;
  tourcol_selectedpathline[2]=0.0;


  tourcol_selectedpathlineknots[0]=1.0;
  tourcol_selectedpathlineknots[1]=0.0;
  tourcol_selectedpathlineknots[2]=0.0;


  tourcol_selectedknot[0]=0.0;
  tourcol_selectedknot[1]=1.0;
  tourcol_selectedknot[2]=0.0;


  tourcol_selectedview[0]=1.0;
  tourcol_selectedview[1]=1.0;
  tourcol_selectedview[2]=0.0;


  tourcol_pathline[0]=-1.0;
  tourcol_pathline[1]=-1.0;
  tourcol_pathline[2]=-1.0;

  tourcol_pathknots[0]=-1.0;
  tourcol_pathknots[1]=-1.0;
  tourcol_pathknots[2]=-1.0;

  tourcol_text[0]=-1.0;
  tourcol_text[1]=-1.0;
  tourcol_text[2]=-1.0;


  tourcol_avatar[0]=1.0;
  tourcol_avatar[1]=0.0;
  tourcol_avatar[2]=0.0;

  iso_specular[0] = 0.7;
  iso_specular[1] = 0.7;
  iso_specular[2] = 0.7;
  iso_specular[3] = 1.0;
  iso_shininess = 10.0f;

  light_position0[0] = 1.0f;
  light_position0[1] = 1.0f;
  light_position0[2] = 1.0f;
  light_position0[3] = 0.0f;

  light_position1[0] = -1.0f;
  light_position1[1] = -1.0f;
  light_position1[2] =  1.0f;
  light_position1[3] =  0.0f;

  ambientlight[0] = 0.6f;
  ambientlight[1] = 0.6f;
  ambientlight[2] = 0.6f;
  ambientlight[3] = 1.0f;

  diffuselight[0] = 0.50f;
  diffuselight[1] = 0.50f;
  diffuselight[2] = 0.50f;
  diffuselight[3] = 1.00f;


  list_p3_index_old=0, list_slice_index_old=0, list_patch_index_old=0;

  videoSTEREO=0;
  fzero=0.25;


  strcpy(blank_global,"");

  demo_mode=0;
  update_demo=1;
  mxplot3dvars=MAXPLOT3DVARS;

  valindex=0;

  NewMemory((void **)&iso_colors, 4 * MAX_ISO_COLORS*sizeof(float));
  NewMemory((void **)&iso_colorsbw, 4 * MAX_ISO_COLORS*sizeof(float));

#define N_ISO_COLORS 10
  iso_colors[0] = 0.96;
  iso_colors[1] = 0.00;
  iso_colors[2] = 0.96;

  iso_colors[4] = 0.75;
  iso_colors[5] = 0.80;
  iso_colors[6] = 0.80;

  iso_colors[8] = 0.00;
  iso_colors[9] = 0.96;
  iso_colors[10] = 0.28;

  iso_colors[12] = 0.00;
  iso_colors[13] = 0.00;
  iso_colors[14] = 1.00;

  iso_colors[16] = 0.00;
  iso_colors[17] = 0.718750;
  iso_colors[18] = 1.00;

  iso_colors[20] = 0.00;
  iso_colors[21] = 1.0;
  iso_colors[22] = 0.5625;

  iso_colors[24] = 0.17185;
  iso_colors[25] = 1.0;
  iso_colors[26] = 0.0;

  iso_colors[28] = 0.890625;
  iso_colors[29] = 1.0;
  iso_colors[30] = 0.0;

  iso_colors[32] = 1.0;
  iso_colors[33] = 0.380952;
  iso_colors[34] = 0.0;

  iso_colors[36] = 1.0;
  iso_colors[37] = 0.0;
  iso_colors[38] = 0.0;

  glui_iso_transparency = CLAMP(255 * iso_transparency+0.1, 1, 255);
  for(i = 0; i < N_ISO_COLORS; i++){
    iso_colors[4 * i + 3] = iso_transparency;
  }

  for(i = N_ISO_COLORS; i<MAX_ISO_COLORS; i++){
    int grey;

    grey=1.0-(float)(i-N_ISO_COLORS)/(float)(MAX_ISO_COLORS+1-N_ISO_COLORS);
    iso_colors[4*i+0]=grey;
    iso_colors[4*i+1]=grey;
    iso_colors[4*i+2]=grey;
    iso_colors[4 * i + 3] = iso_transparency;
  }

  for(i = 0; i < MAX_ISO_COLORS; i++){
    float graylevel;

    graylevel = TOBW(iso_colors+4*i);
    iso_colorsbw[4*i+0] = graylevel;
    iso_colorsbw[4*i+1] = graylevel;
    iso_colorsbw[4*i+2] = graylevel;
    iso_colorsbw[4*i+3] = 1.0;
  }
  CheckMemory;

  ncolortableinfo = 2;
  if(ncolortableinfo>0){
    colortabledata *cti;

    NewMemory((void **)&colortableinfo, ncolortableinfo*sizeof(colortabledata));

    cti = colortableinfo+0;
    cti->color[0] = 210;
    cti->color[1] = 180;
    cti->color[2] = 140;
    cti->color[3] = 255;
    strcpy(cti->label, "tan");

    cti = colortableinfo+1;
    cti->color[0] = 178;
    cti->color[1] = 34;
    cti->color[2] = 34;
    cti->color[3] = 255;
    strcpy(cti->label, "firebrick");
  }

  mouse_deltax=0.0, mouse_deltay=0.0;

  char_color[0]=0.0;
  char_color[1]=0.0;
  char_color[2]=0.0;
  char_color[3]=0.0;

  movedir[0]=0.0;
  movedir[1]=1.0;
  movedir[2]=0.0;

  memcpy(rgb_base,rgb_baseBASE,MAXRGB*4*sizeof(float));
  memcpy(bw_base,bw_baseBASE,MAXRGB*4*sizeof(float));
  memcpy(rgb2,rgb2BASE,MAXRGB*3*sizeof(float));
  memcpy(bw_base,bw_baseBASE,MAXRGB*4*sizeof(float));

  nrgb2=8;

  ncamera_list=0;
  i_view_list=1;
  camera_max_id=2;
  startup=0;
  startup_view_ini=1;
  strcpy(startup_view_label,"external");
  selected_view=-999;


  {
    int iii;

    for(iii=0;iii<7;iii++){
      vis_boundary_type[iii]=0;
    }
    vis_boundary_type[0]=1;
    for(iii=0;iii<MAXPLOT3DVARS;iii++){
      setp3min[iii]=PERCENTILE_MIN;
      p3min[iii]=1.0f;
      p3chopmin[iii]=1.0f;
      setp3max[iii]=PERCENTILE_MAX;
      p3max[iii]=1.0f;
      p3chopmax[iii]=0.0f;
    }
  }
}

/* ------------------ CopyArgs ------------------------ */

void CopyArgs(int *argc, char **aargv, char ***argv_sv){
#ifdef WIN32
  char *filename=NULL;
  char **argv=NULL;
  int filelength=1024,openfile;
  int i;

  if(NewMemory((void **)&argv,(*argc+1)*sizeof(char **))!=0){
    *argv_sv=argv;
    for(i=0;i<*argc;i++){
      argv[i]=aargv[i];
    }
    if(*argc==1){
      if(NewMemory((void **)&filename,(unsigned int)(filelength+1))!=0){
        openfile=0;
        OpenSMVFile(filename,filelength,&openfile);
        if(openfile==1&&ResizeMemory((void **)&filename,strlen(filename)+1)!=0){
          *argc=2;
          argv[1]=filename;
        }
        else{
          FREEMEMORY(filename);
        }
      }
    }
  }
  else{
    *argc=0;
  }
#else
  *argv_sv=aargv;
#endif
}
