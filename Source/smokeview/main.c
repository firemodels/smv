#define INMAIN
#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "file_util.h"
#include "string_util.h"
#include "smokeviewvars.h"
#include "command_args.h"
#include "paths.h"
#include "readsmvfile.h"
#include "IOscript.h"
#include "IOvolsmoke.h"

#ifdef WIN32
#include <direct.h>
#endif

#include <assert.h>



/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[100];
  char gitdate[100];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\nsmokeview [options] casename\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);
  PRINTF("%s\n\n", _("Visualize fire/smoke flow simulations."));
  PRINTF("options:\n");
  PRINTF("%s\n", _(" casename       - project id (file names without the extension)"));
  PRINTF("%s\n", _(" -bindir dir    - specify location of smokeview bin directory"));
  PRINTF("%s\n", _(" -ini           - output smokeview parameter values to smokeview.ini"));
  PRINTF("%s\n", _(" -runscript     - run the script file casename.ssf"));
  UsageCommon(HELP_SUMMARY);
  if(option==HELP_ALL){
    PRINTF("\n%s\n", _("Other options:"));
#ifdef pp_OSX_HIGHRES
    PRINTF("%s\n", _(" -1x            - turn off 2x scene scaling (do not scale scene)."));
    PRINTF("%s\n", _(" -2x            - turn on 2x scene scaling."));
#endif
    PRINTF("%s\n", _(" -big           - hide scene and data when moving scene or selecting menus"));
    PRINTF("%s\n", _(" -casedir dir   - specify location of case (if different than current directory)"));
    PRINTF("%s\n", _(" -checkscript file.ssf  - check the script file file.ssf for errors"));
    PRINTF("%s\n", _(" -convert_ini case1.ini case2.ini - update case1.ini to the current format"));
    PRINTF("%s\n", _("                  and save the results into case2.ini"));
    PRINTF("%s\n", _(" -demo          - use demonstrator mode of Smokeview"));
    PRINTF("%s\n", _(" -fast          - assume slice files exist in order to reduce startup time,"));
    PRINTF("%s\n", _("                  don't compute blanking arrays"));
    PRINTF("%s\n", _(" -full          - full startup - check if files exist"));
    PRINTF("%s\n", _(" -geominfo      - output information about geometry triangles"));
    PRINTF("%s\n", _(" -info            generate casename.slcf and casename.viewpoint files containing slice file and viewpiont info"));
    PRINTF("%s\n", _(" -lang xx       - where xx is de, es, fr, it for German, Spanish, French or Italian"));
    PRINTF("%s\n", _(" -large         - take some shortcuts when reading in large geometry cases"));
    PRINTF("%s\n", _(" -load_co2      - load 3D smoke of type co2 at startup "));
    PRINTF("%s\n", _(" -load_hrrpuv   - load 3D smoke of type hrrpuv at startup "));
    PRINTF("%s\n", _(" -load_soot     - load 3D smoke of type soot at startup "));
    PRINTF("%s\n", _(" -load_temp     - load 3D smoke of type temperature at startup "));
    PRINTF("%s\n", _(" -make_movie    - open the movie generating dialog box"));
    PRINTF("%s\n", _(" -max_mem mem   - specify maximum memory used in GB"));
    PRINTF("%s\n", _(" -outline       - show geometry bound boxes instead of geometry"));
    PRINTF("%s\n", _(" -ng_ini        - non-graphics version of -ini."));
    PRINTF("%s\n", _(" -scriptrenderdir dir - directory containing script rendered images"));
    PRINTF("%s\n", _("                  (override directory specified by RENDERDIR script keyword)"));
    PRINTF("%s\n", _(" -setup         - only show geometry"));
    PRINTF("%s\n", _(" -script scriptfile - run the script file scriptfile"));
    PRINTF("%s\n", _(" -htmlscript scriptfile - run the script file scriptfile without using the video card"));
    PRINTF("%s\n", _(" -runhtmlscript - run the script file casename.ssf without using the video card"));
    PRINTF("%s\n", _("     the -htmlscript and -runhtmlscript keywords are used to generate JSON files"));
    PRINTF("%s\n", _(" -sizes         - output files sizes then exit"));
    PRINTF("%s\n", _(" -skipframe n   - render every n frames"));
    PRINTF("%s\n", _(" -smoke3d       - only show 3D smoke"));
    PRINTF("%s\n", _(" -startframe n  - start rendering at frame n"));
    PRINTF("%s\n", _(" -stereo        - activate stereo mode"));
    PRINTF("%s\n", _(" -timings       - show timings"));
    PRINTF("%s\n", _(" -trirates      - show triangle display rates"));
    PRINTF("%s\n", _(" -update_slice  - calculate slice file parameters"));
    PRINTF("%s\n", _(" -update        - equivalent to -update_bounds and -update_slice"));
    PRINTF("%s\n", _(" -update_ini case.ini - update case.ini to the current format"));
    PRINTF("%s\n", _(" -x0 val - horizontal screen coordinate in pixels where smokeview window is place at startup"));
    PRINTF("%s\n", _(" -y0 val - vertical screen coordinate in pixels where smokeview window is place at startup"));
    PRINTF("%s\n", _(" -X0 val - horizontal screen coordinate in pixels where dialog windows are placed when opened"));
    PRINTF("%s\n", _(" -Y0 val - vertical screen coordinate in pixels where dialog windows are placed when opened"));
    PRINTF("%s\n", _(" -volrender     - generate images of volume rendered smoke and fire"));
    UsageCommon(HELP_ALL);
  }
}

char *ProcessCommandLine(CommandlineArgs *args);

char *ParseCommandline(int argc, char **argv){
  enum CommandLineError error;
  char message[256];

  CommandlineArgs args = ParseCommandlineNew(argc, argv, message, &error);
  if(error != CLE_OK){
    const char *msg = CLE_Message(error, message);
    if(msg != NULL){
      fprintf(stderr, "%s\n", msg);
    }
    SMV_EXIT(0);
  }
  return ProcessCommandLine(&args);
}

/// @brief Once the commandline arguments ahve been parsed, they can be passed
/// to this function to update global settings and undertake various actions.
/// @param args The args which were previously parsed. All commandline arguments
/// are parsed into @ref CommandlineArgs.
/// @return The iput file name (the SMV file).
char *ProcessCommandLine(CommandlineArgs *args){
  int len_casename;
  size_t len_memory;
  char *argi, *smv_ext;
  char SMVFILENAME[MAX_SMV_FILENAME_BUFFER];
  char *filename_local = NULL;

  CheckMemory;
  if(args->checkscript){
    int error_code;

    int CheckScript(char *file);
    error_code = CheckScript(args->script);
    switch(error_code){
    case 0:
      fprintf(stderr, "***no errors detected in %s\n", args->script);
      break;
    case 1:
      fprintf(stderr, "***error: unable to open script file %s\n", args->script);
      break;
    case 2:
      fprintf(stderr, "***errors detected in script file %s\n", args->script);
      break;
    default:
      assert(FFALSE);
      break;
    }
    void SMV_EXIT(int error);
    SMV_EXIT(0);
  }
  if(args->have_x0){
    use_commandline_origin = 1;
    screenX0 = args->x0;
  }
  if(args->have_y0){
    use_commandline_origin = 1;
    screenY0 = args->y0;
  }
  if(args->have_X0){
    dialogX0 = args->X0;
    have_dialogX0 = 1;
  }
  if(args->have_Y0){
    dialogY0 = args->Y0;
    have_dialogY0 = 1;
  }
  if(args->csv){
    update_csv_load = 1;
  }
  if(args->max_mem){
    max_mem_GB = args->max_mem_GB;
  }
  if(args->ini){
    InitCameraList();
    InitOpenGL(NO_PRINT);
    UpdateRGBColors(colorbar_select_index);
    InitStartupDirs();
    WriteIni(GLOBAL_INI, NULL);
    SMV_EXIT(0);
  }
  if(args->ng_ini){
    InitCameraList();
    use_graphics = 0;
    UpdateRGBColors(colorbar_select_index);
    InitStartupDirs();
    WriteIni(GLOBAL_INI, NULL);
    SMV_EXIT(0);
  }
  if(args->print_version){
    if(args->bindir != NULL){
      SetSmvRootOverride(args->bindir);
    }
    show_version = 1;
  }
  strcpy(SMVFILENAME, "");
  if(args->input_file != NULL){
    if(strlen(args->input_file) > MAX_SMV_FILENAME_BUFFER-1){
      fprintf(stderr, "*** Error: input filename exceeds maximum length of %d\n", MAX_SMV_FILENAME_BUFFER-1);
      SMV_EXIT(1);
    }
    strcat(SMVFILENAME, args->input_file);
  }
// strip .smv extension if present
  char *smvext;
  smvext = strstr(SMVFILENAME, ".smv");
  if(smvext!=NULL)*smvext = 0;

  argi = SMVFILENAME;
#ifndef pp_OSX
  argi = LastName(argi);
#endif
  len_casename = (int)strlen(argi);
  CheckMemory;
  FREEMEMORY(global_scase.fdsprefix);
  len_memory = len_casename + strlen(".part") + 100;
  NewMemory((void **)&global_scase.fdsprefix, (unsigned int)len_memory);
  STRCPY(global_scase.fdsprefix, argi);

  if(global_scase.fdsprefix!=NULL&&strlen(global_scase.fdsprefix)>0){
    NewMemory((void **)&filename_local, (unsigned int)len_memory+4);
    strcpy(filename_local, global_scase.fdsprefix);
    strcat(filename_local, ".smv");
  }
  else{
#ifdef WIN32
    if(show_version == 0){
      int openfile, filelength;

      openfile = 0;
      filelength = 1024;
      NewMemory((void **)&filename_local, (unsigned int)filelength);
      OpenSMVFile(filename_local, filelength, &openfile);
      if(openfile == 1 && ResizeMemory((void **)&filename_local, strlen(filename_local) + 1) != 0){
        char *dirlast = NULL, *caselast = NULL;

        FREEMEMORY(smokeview_casedir);
        FREEMEMORY(global_scase.fdsprefix);
        NewMemory((void **)&smokeview_casedir, strlen(filename_local) + 1);
        NewMemory((void **)&global_scase.fdsprefix, strlen(filename_local) + 1);
        strcpy(smokeview_casedir, filename_local);
        dirlast = strrchr(smokeview_casedir, '\\');
        if(dirlast != NULL){
          strcpy(filename_local, dirlast + 1);
          strcpy(global_scase.fdsprefix, filename_local);
          caselast = strrchr(global_scase.fdsprefix, '.');
          if(caselast != NULL)caselast[0] = 0;
          dirlast[1] = 0;
          len_casename = strlen(filename_local);
        }
        CHDIR(smokeview_casedir);
      }
      else{
        FREEMEMORY(filename_local);
      }
    }
#endif
    if(filename_local==NULL)return NULL;
  }
  strcpy(movie_name, global_scase.fdsprefix);
  strcpy(render_file_base, global_scase.fdsprefix);
  strcpy(html_file_base, global_scase.fdsprefix);
  smv_ext = strstr(html_file_base, ".smv");
  if(smv_ext!=NULL)*smv_ext = 0;

  strcpy(input_filename_ext, "");

  if(len_casename > 4){
    char *c_ext;

    c_ext = strrchr(argi, '.');
    if(c_ext != NULL){
      STRCPY(input_filename_ext, c_ext);
      ToLower(input_filename_ext);

      if(c_ext != NULL &&
        (strcmp(input_filename_ext, ".smv") == 0 ||
        strcmp(input_filename_ext, ".svd") == 0 ||
        strcmp(input_filename_ext, ".smt") == 0)
        ){
        c_ext[0] = 0;
        STRCPY(global_scase.fdsprefix, argi);
        strcpy(movie_name, global_scase.fdsprefix);
        strcpy(render_file_base, global_scase.fdsprefix);
      }
    }
  }

  {
    char scriptbuffer[1024];

    STRCPY(scriptbuffer, global_scase.fdsprefix);
    STRCAT(scriptbuffer, ".ssf");
    if(default_script == NULL&&FILE_EXISTS(scriptbuffer) == YES){
      default_script = InsertScriptFile(scriptbuffer);
    }
  }

#ifdef pp_OSX_HIGHRES
    if(args->x1){
      double_scale = 0;
      force_scale  = 1;
    }
    else if(args->x2){
      double_scale = 1;
      force_scale = 1;
    }
#endif
    if(args->update_bounds){
      use_graphics = 0;
      update_bounds = 1;
    }
    if(args->no_graphics){
      use_graphics = 0;
    }
    if(args->large_case){
      large_case = 1;
    }
    if(args->update_slice){
      use_graphics = 0;
      update_slice = 1;
    }
    if(args->update){
        use_graphics = 0;
        update_slice = 1;
        update_bounds = 1;
    }
    if(args->nogpu){
      disable_gpu = 1;
    }
    if(args->check_colorbar){
      check_colorbar = 1;
    }
    if(args->demo){
      demo_option = 1;
    }
    if(args->info){
      generate_info_from_commandline = 1;
      use_graphics = 0;
    }
    if(args->sizes){
      update_filesizes = 1;
      use_graphics = 0;
    }
    if(args->stereo){
      stereoactive = 1;
      stereotype = STEREO_TIME;
      PRINTF("stereo option activated\n");
    }
    if(args->big){
      hide_scene = 1;
    }
    if(args->timings){
      show_timings = 1;
    }
    if(args->trirates){
      show_trirates = 1;
    }
    if(args->lang != NULL){
        FREEMEMORY(tr_name);
        NewMemory((void **)&tr_name, strlen(args->lang)+1);
        strcpy(tr_name, args->lang);
    }
    if(args->convert_ini){
      if(args->ini_from != NULL&&args->ini_to != NULL){
        NewMemory((void **)&ini_from, strlen(args->ini_from) + 1);
        strcpy(ini_from, args->ini_from);

        NewMemory((void **)&ini_to, strlen(args->ini_to) + 1);
        strcpy(ini_to, args->ini_to);
        convert_ini = 1;
      }
    }
    if(args->convert_ssf){
      if(args->ssf_from != NULL&&args->ssf_to != NULL){
        NewMemory((void **)&ssf_from, strlen(args->ssf_from) + 1);
        strcpy(ssf_from, args->ssf_from);

        NewMemory((void **)&ssf_to, strlen(args->ssf_to) + 1);
        strcpy(ssf_to, args->ssf_to);
        convert_ssf = 1;
      }
    }
    if(args->update_ssf){
      update_ssf = 1;
    }
    if(args->update_ini){
      if(args->ini_from != NULL){
        NewMemory((void **)&ini_from, strlen(args->ini_from) + 1);
        strcpy(ini_from, args->ini_from);

        NewMemory((void **)&ini_to, strlen(args->ini_from) + 1);
        strcpy(ini_to, args->ini_from);
        convert_ini = 1;
      }
    }
    if(args->isotest){
      isotest = 1;
    }
    if(args->smoke3d){
      parse_opts.smoke3d_only = 1;
    }
    if(args->no_slcf){
    parse_opts.handle_slice_files = 0;
    }
    if(args->show_help_summary){
      Usage(HELP_SUMMARY);
      SMV_EXIT(0);
    }
    if(args->show_help_all){
      Usage(HELP_ALL);
      SMV_EXIT(0);
    }
    if(args->noblank){
      global_scase.iblank_set_on_commandline = 1;
      global_scase.use_iblank = 0;
    }
    if(args->nobounds){
      no_bounds = 1;
    }
    if(args->verbose){
      verbose_output = 1;
    }
    if(args->outline){
      hide_scene = 1;
    }
    if(args->make_movie){
      open_movie_dialog = 1;
    }
    if(args->geominfo){
      print_geominfo = 1;
    }
    if(args->load_co2){
      loadfiles_commandline[0]          = 1;
      loadfiles_commandline[LOAD_3DCO2] = 1;
    }
    if(args->load_hrrpuv){
      loadfiles_commandline[0]             = 1;
      loadfiles_commandline[LOAD_3DHRRPUV] = 1;
      loadfiles_commandline[LOAD_3DTEMP]   = 0;
    }
    if(args->load_soot){
      loadfiles_commandline[0]           = 1;
      loadfiles_commandline[LOAD_3DSOOT] = 1;
    }
    if(args->load_temp){
      loadfiles_commandline[0]             = 1;
      loadfiles_commandline[LOAD_3DTEMP]   = 1;
      loadfiles_commandline[LOAD_3DHRRPUV] = 0;
    }
    if(args->fast){
      parse_opts.fast_startup = 1;
      parse_opts.lookfor_compressed_files = 0;
    }
    if(args->full){
      parse_opts.fast_startup = 0;
      parse_opts.lookfor_compressed_files = 1;
    }
    if(args->blank){
      global_scase.iblank_set_on_commandline = 1;
      global_scase.use_iblank = 1;
    }
    if(args->gversion){
      vis_title_gversion = 1;
    }
    if(args->redirect){
      char *log_filename = CasePathLogFile(&global_scase);
      LOG_FILENAME = fopen(log_filename, "w");
      FREEMEMORY(log_filename);
      if(LOG_FILENAME != NULL){
        redirect = 1;
        SetStdOut(LOG_FILENAME);
      }
    }
    if(args->runscript){
      from_commandline = 1;
      use_iso_threads=0;
      runscript = 1;
    }
    if(args->runhtmlscript){
      from_commandline = 1;
      use_graphics = 0;
      use_iso_threads = 0;
      runhtmlscript = 1;
    }
    if(args->scriptrenderdir != NULL){
      int nrenderdir = strlen(args->scriptrenderdir);
      if(nrenderdir>0){
        NewMemory((void **)&script_renderdir_cmd, nrenderdir+1);
        strcpy(script_renderdir_cmd, args->scriptrenderdir);
      }
    }
    if(args->skipframe_defined){
      from_commandline = 1;
      render_skipframe0 = args->skipframe;
    }
    if(args->startframe_defined){
      from_commandline = 1;
      render_startframe0 = args->startframe;
    }
    if(args->volrender){
      from_commandline = 1;
      make_volrender_script = 1;
    }
    if(args->script!=NULL||args->htmlscript!=NULL){
      char scriptbuffer[MAX_SCRIPT_FILENAME_BUFFER];
      scriptfiledata *sfd;

      bool is_htmlscript = args->htmlscript!=NULL;
      if(is_htmlscript){
        use_graphics = 0;
        runhtmlscript = 1;
      }
      from_commandline = 1;
      use_iso_threads=0;
      if(args->script != NULL){
        if(strlen(args->script) < MAX_SCRIPT_FILENAME_BUFFER){
          strcpy(scriptbuffer, args->script);
        }
        else{
          fprintf(stderr, "*** Error: script filename exceeds maximum length of %d\n", MAX_SCRIPT_FILENAME_BUFFER-1);
          SMV_EXIT(1);
        }
      } else{
        if(strlen(args->htmlscript) < MAX_SCRIPT_FILENAME_BUFFER){
          strcpy(scriptbuffer, args->htmlscript);
        }
        else{
          fprintf(stderr, "*** Error: htmlscript filename exceeds maximum length of %d\n", MAX_SCRIPT_FILENAME_BUFFER-1);
          SMV_EXIT(1);
        }
      }
      sfd = InsertScriptFile(scriptbuffer);
      if(sfd != NULL)default_script = sfd;
      if(!is_htmlscript){
        runscript = 1;
      }
    }
    if(args->noexit){
      noexit = 1;
    }
    if(args->setup){
      parse_opts.setup_only = 1;
    }
    if(args->bindir != NULL){
      SetSmvRootOverride(args->bindir);
    }
    if(args->casedir){
      NewMemory((void **)&smokeview_casedir, strlen(args->casedir) +2);
      strcpy(smokeview_casedir, args->casedir);
    }
    if(args->threads_defined){
        n_readallgeom_threads = CLAMP(args->threads, 1, 16);
    }
  if(update_ssf == 1){
    int len_prefix = 0;

    len_prefix = strlen(global_scase.fdsprefix);

    FREEMEMORY(ssf_from);
    NewMemory((void **)&ssf_from, len_prefix + 4 + 1);
    strcpy(ssf_from, global_scase.fdsprefix);
    strcat(ssf_from, ".ssf");

    FREEMEMORY(ssf_to);
    NewMemory((void **)&ssf_to, len_prefix + 4 + 1);
    strcpy(ssf_to, global_scase.fdsprefix);
    strcat(ssf_to, ".ssf");
  }
  if(make_volrender_script == 1){

    NewMemory((void **)&volrender_scriptname, (unsigned int)(len_casename + 14 + 1));
    STRCPY(volrender_scriptname, global_scase.fdsprefix);
    STRCAT(volrender_scriptname, "_volrender.ssf");

    InitVolrenderScript(global_scase.fdsprefix, NULL, vol_startframe0, vol_skipframe0);
  }
  return filename_local;
}

/* ------------------ CheckSMVFile ------------------------ */

int CheckSMVFile(char *file, char *subdir){
  char casedir[256], *casedirptr, casename[256];
  FILE *stream;

  if(file==NULL)return 1;

  strcpy(casename, file);
  if(subdir==NULL){
    casedirptr = casedir;
    strcpy(casedir, casename);
  }
  else{
    casedirptr = subdir;
  }
  stream = fopen(casename, "r");
  if(stream==NULL){
    stream = fopen_indir(casedirptr, casename, "r");
    if(stream==NULL){
      printf("***error: unable to open %s\n", casename);
      return 0;
    }
    CHDIR(casedirptr);
  }
  fclose(stream);
  return 1;
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int return_code;

#ifdef TEST
  int nnn;

  nnn = global_scase.smoke3dcoll.nsmoke3dinfo;
  printf("nsmoke3dinfo=%i\n", nnn);
#endif

  START_TIMER(timer_startup);
  // uncomment following block of code to test crash detection
/*
  float *x = NULL, xx, yy, zz;

  printf("before using undefined variable\n");
  zz = xx+yy;
  printf("after using undefined variable: z=%f\n", zz);

  xx = 0.0;
  printf("before divide by zero\n");
  zz = 1.0/xx;
  printf("after divide by zero: zz=%f\n", zz);

  printf("before accessing null variable\n");
  x[0] = 1.0;
  printf("after accessing null variable: %f\n", x[0]);
*/
  SetStdOut(stdout);
  initMALLOC();
  InitRandAB(1000000);
  InitVars();

  ParseCommonOptions(argc, argv);
  if(show_help==1){
    Usage(HELP_SUMMARY);
    return 1;
  }
  if(show_help==2){
    Usage(HELP_ALL);
    return 1;
  }

  smv_filename = ParseCommandline(argc, argv);

  if(smv_filename == NULL || show_version == 1){
    InitStartupDirs();
    DisplayVersionInfo("Smokeview ");
    SMV_EXIT(0);
  }
  if(CheckSMVFile(smv_filename, smokeview_casedir)==0){
    SMV_EXIT(1);
  }
  MakeFireColors(fire_temp_min, fire_temp_max, nfire_colors);

  InitTextureDir();
  InitScriptErrorFiles();
  char *smv_bindir = GetSmvRootDir();
  smokezippath= GetSmokeZipPath(smv_bindir);
  FREEMEMORY(smv_bindir);
  InitStartupDirs();
  DisplayVersionInfo("Smokeview ");
  SetupGlut(argc,argv);
  START_TIMER(startup_time);

  return_code= SetupCase(smv_filename);
  if(return_code!=0)return 1;
  if(convert_ini==1){
    INIT_PRINT_TIMER(timer_read_ini);
    ReadIni(ini_from);
    PRINT_TIMER(timer_read_ini, "ReadIni");
  }
  if(runhtmlscript==1){
    DoScriptHtml();
  }

  STOP_TIMER(startup_time);
  PRINTF("\n");
  if(runhtmlscript==1){
    PRINTF("Time: %.1f s\n", startup_time);
    return 0;
  }
  PRINTF("Startup time: %.1f s\n", startup_time);

  glutMainLoop();
  FreeVars();
  return 0;
}
