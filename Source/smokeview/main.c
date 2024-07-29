#define INMAIN
#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "string_util.h"
#include "smokeviewvars.h"
#include "command_args.h"
#include "fopen.h"

#ifdef WIN32
#include <direct.h>
#endif

#ifdef pp_LUA
#include "c_api.h"
#include "lua_api.h"
#endif

#include <assert.h>

/* ------------------ IsInstallBinDir ------------------------ */

int IsInstallBinDir(char *bindir){
  char smvfile[1024];

  if(bindir == NULL)return 0;
  strcpy(smvfile, bindir);
  strcat(smvfile, dirseparator);
  strcat(smvfile, ".smokeview_bin");
  return FileExistsOrig(smvfile);
}

/* ------------------ SetBinDirAlways ------------------------ */

void SetBinDirAlways(char *new_bindir){
  char savedir[1024], new_bindir_local[1024];

  GETCWD(savedir, 1024);
  CHDIR(new_bindir);
  GETCWD(new_bindir_local, 1024);
  CHDIR(savedir);
  FREEMEMORY(smokeview_bindir);
  NewMemory((void **)&smokeview_bindir, strlen(new_bindir_local) + 2);
  strcpy(smokeview_bindir, new_bindir_local);
  if(smokeview_bindir[strlen(smokeview_bindir) - 1] != dirseparator[0])strcat(smokeview_bindir, dirseparator);
}

/* ------------------ SetBinDir ------------------------ */

int SetBinDir(char *new_bindir){
  if(IsInstallBinDir(new_bindir) == 1){
    SetBinDirAlways(new_bindir);
    return 1;
  }
  return 0;
}

/* ------------------ Usage ------------------------ */

void Usage(char *prog,int option){
  PRINTF("%s\n", release_title);
  PRINTF("%s\n\n", _("Visualize fire/smoke flow simulations."));
  PRINTF("Usage: %s [options] casename", prog);
  PRINTF("\n\n");
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
    PRINTF("%s\n", _(" -make_movie    - open the movie generating dialog box"));
    PRINTF("%s\n", _(" -max_mem mem   - specify maximum memory used in GB"));
    PRINTF("%s\n", _(" -outline       - show geometry bound boxes instead of geometry"));
    PRINTF("%s\n", _(" -ng_ini        - non-graphics version of -ini."));
    PRINTF("%s\n", _(" -scriptrenderdir dir - directory containing script rendered images"));
    PRINTF("%s\n", _("                  (override directory specified by RENDERDIR script keyword)"));
    PRINTF("%s\n", _(" -setup         - only show geometry"));
    PRINTF("%s\n", _(" -script scriptfile - run the script file scriptfile"));
#ifdef pp_LUA
    PRINTF("%s\n", " -luascript scriptfile - run the Lua script file scriptfile");
    PRINTF("%s\n", " -killscript    - exit smokeview (with an error code) if the script fails");
#endif
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

char *ParseCommandline(int argc, char **argv) {
  enum CommandLineError error;
  char *return_val;
  char message[256];

  CommandlineArgs args = ParseCommandlineNew(argc, argv, message, &error);
  if (error != CLE_OK) {
    const char *msg = CLE_Message(error, message);
    if (msg != NULL) {
      fprintf(stderr, "%s\n", msg);
    }
    SMV_EXIT(0);
  }
  return_val = ProcessCommandLine(&args);
  if(args.bindir == NULL){
    have_bindir_arg = 0;
  }
  else{
    have_bindir_arg = 1;
    SetBinDirAlways(args.bindir);
    if(smokeview_bindir[strlen(smokeview_bindir) - 1] != dirseparator[0])strcat(smokeview_bindir, dirseparator);
  }
  return return_val;
}

/// @brief Once the commandline arguments ahve been parsed, they can be passed
/// to this function to update global settings and undertake various actions.
/// @param args The args which were previously parsed. All commandline arguments
/// are parsed into @ref CommandlineArgs.
/// @return The iput file name (the SMV file).
char *ProcessCommandLine(CommandlineArgs *args) {
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
  if (args->csv) {
    update_csv_load = 1;
  }
  if(args->max_mem){
    max_mem_GB = args->max_mem_GB;
  }
  if (args->ini) {
    InitCameraList();
    InitOpenGL(NO_PRINT);
    UpdateRGBColors(colorbar_select_index);
    InitStartupDirs();
    WriteIni(GLOBAL_INI, NULL);
    SMV_EXIT(0);
  }
  if (args->ng_ini) {
    InitCameraList();
    use_graphics = 0;
    UpdateRGBColors(colorbar_select_index);
    InitStartupDirs();
    WriteIni(GLOBAL_INI, NULL);
    SMV_EXIT(0);
  }
  if (args->print_version) {
    show_version = 1;
  }
  strcpy(SMVFILENAME, "");
  if (args->input_file != NULL) {
    if (strlen(args->input_file) > MAX_SMV_FILENAME_BUFFER-1) {
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
  FREEMEMORY(fdsprefix);
  len_memory = len_casename + strlen(".part") + 100;
  NewMemory((void **)&fdsprefix, (unsigned int)len_memory);
  STRCPY(fdsprefix, argi);

  if(fdsprefix!=NULL&&strlen(fdsprefix)>0){
    NewMemory((void **)&filename_local, (unsigned int)len_memory+4);
    strcpy(filename_local, fdsprefix);
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
        FREEMEMORY(fdsprefix);
        NewMemory((void **)&smokeview_casedir, strlen(filename_local) + 1);
        NewMemory((void **)&fdsprefix, strlen(filename_local) + 1);
        strcpy(smokeview_casedir, filename_local);
        dirlast = strrchr(smokeview_casedir, '\\');
        if(dirlast != NULL){
          strcpy(filename_local, dirlast + 1);
          strcpy(fdsprefix, filename_local);
          caselast = strrchr(fdsprefix, '.');
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
  strcpy(movie_name, fdsprefix);
  strcpy(render_file_base, fdsprefix);
  strcpy(html_file_base, fdsprefix);
  smv_ext = strstr(html_file_base, ".smv");
  if(smv_ext!=NULL)*smv_ext = 0;
  FREEMEMORY(trainer_filename);
  FREEMEMORY(test_filename);

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
        STRCPY(fdsprefix, argi);
        strcpy(movie_name, fdsprefix);
        strcpy(render_file_base, fdsprefix);
        FREEMEMORY(trainer_filename);
        NewMemory((void **)&trainer_filename, (unsigned int)(len_casename + 7));
        STRCPY(trainer_filename, argi);
        STRCAT(trainer_filename, ".svd");
        FREEMEMORY(test_filename);
        NewMemory((void **)&test_filename, (unsigned int)(len_casename + 7));
        STRCPY(test_filename, argi);
        STRCAT(test_filename, ".smt");
      }
    }
  }

  FREEMEMORY(log_filename);
  NewMemory((void **)&log_filename, len_casename + strlen(".smvlog") + 1);
  STRCPY(log_filename, fdsprefix);
  STRCAT(log_filename, ".smvlog");

  FREEMEMORY(caseini_filename);
  NewMemory((void **)&caseini_filename, len_casename + strlen(".ini") + 1);
  STRCPY(caseini_filename, fdsprefix);
  STRCAT(caseini_filename, ".ini");

#ifdef p_FRAME
  FREEMEMORY(frametest_filename);
  NewMemory((void **)&frametest_filename, len_casename + strlen(".tst") + 1);
  STRCPY(caseini_filename, fdsprefix);
  STRCAT(caseini_filename, ".tst");
#endif

  FREEMEMORY(fedsmv_filename);
  NewMemory((void **)&fedsmv_filename, len_casename + strlen(".fedsmv") + 1);
  STRCPY(fedsmv_filename, fdsprefix);
  STRCAT(fedsmv_filename, ".fedsmv");

  FREEMEMORY(expcsv_filename);
  NewMemory((void **)&expcsv_filename, len_casename + strlen("_exp.csv") + 1);
  STRCPY(expcsv_filename, fdsprefix);
  STRCAT(expcsv_filename, "_exp.csv");

  FREEMEMORY(stepcsv_filename);
  NewMemory(( void ** )&stepcsv_filename, len_casename + strlen("_steps.csv") + 1);
  STRCPY(stepcsv_filename, fdsprefix);
  STRCAT(stepcsv_filename, "_steps.csv");

  FREEMEMORY(dEcsv_filename);
  NewMemory(( void ** )&dEcsv_filename, len_casename + strlen("_dE.csv") + 1);
  STRCPY(dEcsv_filename, fdsprefix);
  STRCAT(dEcsv_filename, "_dE.csv");

  FREEMEMORY(html_filename);
  NewMemory((void **)&html_filename, len_casename+strlen(".html")+1);
  STRCPY(html_filename, fdsprefix);
  STRCAT(html_filename, ".html");

  FREEMEMORY(smv_orig_filename);
  NewMemory((void **)&smv_orig_filename, len_casename+strlen(".smo")+1);
  STRCPY(smv_orig_filename, fdsprefix);
  STRCAT(smv_orig_filename, ".smo");

  FREEMEMORY(hrr_filename);
  NewMemory((void **)&hrr_filename, len_casename+strlen("_hrr.csv")+1);
  STRCPY(hrr_filename, fdsprefix);
  STRCAT(hrr_filename, "_hrr.csv");

  FREEMEMORY(htmlvr_filename);
  NewMemory((void **)&htmlvr_filename, len_casename+strlen("_vr.html")+1);
  STRCPY(htmlvr_filename, fdsprefix);
  STRCAT(htmlvr_filename, "_vr.html");

  FREEMEMORY(htmlobst_filename);
  NewMemory((void **)&htmlobst_filename, len_casename+strlen("_obst.json")+1);
  STRCPY(htmlobst_filename, fdsprefix);
  STRCAT(htmlobst_filename, "_obst.json");

  FREEMEMORY(htmlslicenode_filename);
  NewMemory((void **)&htmlslicenode_filename, len_casename+strlen("_slicenode.json")+1);
  STRCPY(htmlslicenode_filename, fdsprefix);
  STRCAT(htmlslicenode_filename, "_slicenode.json");

  FREEMEMORY(htmlslicecell_filename);
  NewMemory((void **)&htmlslicecell_filename, len_casename+strlen("_slicecell.json")+1);
  STRCPY(htmlslicecell_filename, fdsprefix);
  STRCAT(htmlslicecell_filename, "_slicecell.json");

  FREEMEMORY(event_filename);
  NewMemory((void **)&event_filename, len_casename+strlen("_events.csv")+1);
  STRCPY(event_filename, fdsprefix);
  STRCAT(event_filename, "_events.csv");

  if(filename_local==NULL){
    NewMemory((void **)&filename_local, (unsigned int)(len_casename+6));
    STRCPY(filename_local, fdsprefix);
    STRCAT(filename_local, ".smv");
  }
  {
    char scriptbuffer[1024];

    STRCPY(scriptbuffer, fdsprefix);
    STRCAT(scriptbuffer, ".ssf");
    if(default_script == NULL&&FILE_EXISTS(scriptbuffer) == YES){
      default_script = InsertScriptFile(scriptbuffer);
    }
  }
  if(filename_local!= NULL){
    FREEMEMORY(fds_filein);
    NewMemory((void **)&fds_filein, strlen(fdsprefix) + 6);
    STRCPY(fds_filein, fdsprefix);
    STRCAT(fds_filein, ".fds");
    if(FILE_EXISTS(fds_filein) == NO){
      FREEMEMORY(fds_filein);
    }
  }
  if(ffmpeg_command_filename == NULL){
    NewMemory((void **)&ffmpeg_command_filename, (unsigned int)(len_casename + 12));
    STRCPY(ffmpeg_command_filename, fdsprefix);
    STRCAT(ffmpeg_command_filename, "_ffmpeg");
#ifdef WIN32
    STRCAT(ffmpeg_command_filename,".bat");
#else
    STRCAT(ffmpeg_command_filename,".sh");
#endif
  }
  if(stop_filename == NULL){
    NewMemory((void **)&stop_filename, (unsigned int)(len_casename + strlen(".stop") + 1));
    STRCPY(stop_filename, fdsprefix);
    STRCAT(stop_filename, ".stop");
  }
  if(smvzip_filename == NULL){
    NewMemory((void **)&smvzip_filename, (unsigned int)(len_casename + strlen(".smvzip") + 1));
    STRCPY(smvzip_filename, fdsprefix);
    STRCAT(smvzip_filename, ".smvzip");
  }
  if(sliceinfo_filename == NULL){
    NewMemory((void **)&sliceinfo_filename, strlen(fdsprefix) + strlen(".sinfo") + 1);
    STRCPY(sliceinfo_filename, fdsprefix);
    STRCAT(sliceinfo_filename, ".sinfo");
  }
  if(deviceinfo_filename==NULL){
    NewMemory((void **)&deviceinfo_filename, strlen(fdsprefix)+strlen("_device.info")+1);
    STRCPY(deviceinfo_filename, fdsprefix);
    STRCAT(deviceinfo_filename, "_device.info");
  }

  // if smokezip created part2iso files then concatenate .smv entries found in the .isosmv file
  // to the end of the .smv file creating a new .smv file.  Then read in that .smv file.

  {
    FILE *stream_iso = NULL;

    NewMemory((void **)&iso_filename, len_casename + strlen(".isosmv") + 1);
    STRCPY(iso_filename, fdsprefix);
    STRCAT(iso_filename, ".isosmv");
    stream_iso = fopen(iso_filename, "r");
    if(stream_iso != NULL){
      fclose(stream_iso);
    }
    else{
      FREEMEMORY(iso_filename);
    }
  }

  if(trainer_filename == NULL){
    NewMemory((void **)&trainer_filename, (unsigned int)(len_casename + 6));
    STRCPY(trainer_filename, fdsprefix);
    STRCAT(trainer_filename, ".svd");
  }
  if(test_filename == NULL){
    NewMemory((void **)&test_filename, (unsigned int)(len_casename + 6));
    STRCPY(test_filename, fdsprefix);
    STRCAT(test_filename, ".svd");
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
      show_geom_boundingbox = SHOW_BOUNDING_BOX_MOUSE_DOWN;
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
      smoke3d_only = 1;
    }
    if(args->no_slcf){
    handle_slice_files = 0;
    }
    if(args->show_help_summary){
      Usage(args->prog,HELP_SUMMARY);
      SMV_EXIT(0);
    }
    if(args->show_help_all){
      Usage(args->prog,HELP_ALL);
      SMV_EXIT(0);
    }
    if(args->noblank){
      iblank_set_on_commandline = 1;
      use_iblank = 0;
    }
    if(args->nobounds){
      no_bounds = 1;
    }
    if(args->verbose){
      verbose_output = 1;
    }
    if(args->outline){
      show_geom_boundingbox = SHOW_BOUNDING_BOX_ALWAYS;
    }
    if(args->make_movie){
      open_movie_dialog = 1;
    }
    if(args->geominfo){
      print_geominfo = 1;
    }
    if(args->fast){
      fast_startup = 1;
      lookfor_compressed_files = 0;
    }
    if(args->full){
      fast_startup = 0;
      lookfor_compressed_files = 1;
    }
    if(args->blank){
      iblank_set_on_commandline = 1;
      use_iblank = 1;
    }
    if(args->gversion){
      vis_title_gversion = 1;
    }
    if(args->redirect){
      LOG_FILENAME = fopen(log_filename, "w");
      if(LOG_FILENAME != NULL){
        redirect = 1;
        SetStdOut(LOG_FILENAME);
      }
    }
    if(args->runscript){
      from_commandline = 1;
      use_iso_threads=0;
#ifdef pp_LUA
      strcpy(script_filename, "");
#endif
      runscript = 1;
    }
    if(args->runhtmlscript){
      from_commandline = 1;
      use_graphics = 0;
      use_iso_threads = 0;
      runhtmlscript = 1;
    }
#ifdef pp_LUA
    if(args->runluascript){
      from_commandline = 1;
      use_iso_threads=0;
      strcpy(luascript_filename, "");
      strncpy(luascript_filename, fdsprefix, MAX_LUASCRIPT_FILENAME_BUFFER-5);
      strcat(luascript_filename, ".lua");
      runluascript = 1;
    }
    if(args->killscript){
      from_commandline = 1;
      exit_on_script_crash = 1;
    }
#endif
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
      bool is_htmlscript = args->htmlscript!=NULL;
      if(is_htmlscript){
        use_graphics = 0;
        runhtmlscript = 1;
      }
      from_commandline = 1;
      use_iso_threads=0;
        char scriptbuffer[MAX_SCRIPT_FILENAME_BUFFER];
        scriptfiledata *sfd;
        if (args->script != NULL) {
          if (strlen(args->script) > MAX_SCRIPT_FILENAME_BUFFER-1) {
            fprintf(stderr, "*** Error: script filename exceeds maximum length of %d\n", MAX_SCRIPT_FILENAME_BUFFER-1);
            SMV_EXIT(1);
          }
          strcpy(scriptbuffer, args->script);
        } else {
          if (strlen(args->htmlscript) > MAX_SCRIPT_FILENAME_BUFFER-1) {
            fprintf(stderr, "*** Error: luascript filename exceeds maximum length of %d\n", MAX_SCRIPT_FILENAME_BUFFER-1);
            SMV_EXIT(1);
          }
          strcpy(scriptbuffer, args->htmlscript);
        }
        sfd = InsertScriptFile(scriptbuffer);
        if(sfd != NULL)default_script = sfd;
        if(!is_htmlscript){
          runscript = 1;
        }
    }
#ifdef pp_LUA
    if(args->luascript != NULL){
      from_commandline = 1;
      use_iso_threads=0;
      if (strlen(args->luascript) > MAX_LUASCRIPT_FILENAME_BUFFER-1) {
        fprintf(stderr, "*** Error: luascript filename exceeds maximum length of %d\n", MAX_SMV_FILENAME_BUFFER-1);
        SMV_EXIT(1);
      }
      strncpy(luascript_filename, args->luascript, MAX_LUASCRIPT_FILENAME_BUFFER-1);
      runluascript = 1;
    }
#endif
    if(args->noexit){
      noexit = 1;
    }
    if(args->setup){
      setup_only = 1;
    }
    if(args->bindir != NULL){
      SetBinDirAlways(args->bindir);
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

    len_prefix = strlen(fdsprefix);

    FREEMEMORY(ssf_from);
    NewMemory((void **)&ssf_from, len_prefix + 4 + 1);
    strcpy(ssf_from, fdsprefix);
    strcat(ssf_from, ".ssf");

    FREEMEMORY(ssf_to);
    NewMemory((void **)&ssf_to, len_prefix + 4 + 1);
    strcpy(ssf_to, fdsprefix);
    strcat(ssf_to, ".ssf");
  }
  if(make_volrender_script == 1){

    NewMemory((void **)&volrender_scriptname, (unsigned int)(len_casename + 14 + 1));
    STRCPY(volrender_scriptname, fdsprefix);
    STRCAT(volrender_scriptname, "_volrender.ssf");

    InitVolrenderScript(fdsprefix, NULL, vol_startframe0, vol_skipframe0);
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
  char *progname;

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
#ifdef pp_LUA
  // If we are using lua, let lua take control here.
  // Initialise the lua interpreter, it does not take control at this point
  lua_State *L = InitLua();
  // This code branch gives more control to the interpreter during startup.
  return_code = RunLuaBranch(L, argc, argv);
  // All of the below code is run by the lua interpreter, therefore if we want
  // to use the lua interpreter we ignore the code below and exit once the lua
  // run is complete.
  return return_code;
#endif
#ifdef pp_OPEN_TEST
  InitOpenTest();
#endif
  SetStdOut(stdout);
  initMALLOC();
  InitRandAB(1000000);
  InitVars();

  ParseCommonOptions(argc, argv);
  if(show_help==1){
    Usage("smokeview", HELP_SUMMARY);
    return 1;
  }
  if(show_help==2){
    Usage("smokeview", HELP_ALL);
    return 1;
  }

  progname=argv[0];
  strcpy(smokeview_progname, progname);
  GetProgFullPath(smokeview_progname, 1024);
  smv_filename = ParseCommandline(argc, argv);

#ifdef WIN32
  if(Which("fds_local.bat", &fdsprog) != NULL)strcpy(fdsprog, "fds_local.bat");
#else
  Which("fds", &fdsprog);
#endif

  prog_fullpath = progname;
  if(smokeview_bindir==NULL){
    smokeview_bindir = GetProgDir(progname, &smokeviewpath);
  }
  int valid_bindir;

  valid_bindir = have_bindir_arg;
  if(valid_bindir == 0&&smokeview_bindir!=NULL&&IsInstallBinDir(smokeview_bindir)==0){
    char new_bindir[1024];
    char *bins[] = {"bot","Bundlebot","smv","for_bundle"};
    int i;

    strcpy(new_bindir, smokeview_bindir);
    for(i = 0; i < 4; i++){
      strcat(new_bindir, dirseparator);
      strcat(new_bindir, "..");
    }
    for(i = 0; i < 4; i++){
      strcat(new_bindir, dirseparator);
      strcat(new_bindir, bins[i]);
    }
    strcat(new_bindir, dirseparator);
    if(IsInstallBinDir(new_bindir) == 1){
      char savedir[1024];

      FreeMemory(smokeview_bindir);
      GETCWD(savedir, 1024);
      CHDIR(new_bindir);
      GETCWD(new_bindir, 1024);
      CHDIR(savedir);
      NewMemory((void **)&smokeview_bindir, strlen(new_bindir)+2);
      strcpy(smokeview_bindir, new_bindir);
      valid_bindir = 1;
    }
  }
#ifdef WIN32
  if(valid_bindir == 0){
    char new_bindir[1024];

    strcpy(new_bindir, "C:\\Program Files\\firemodels\\SMV6\\");
    valid_bindir = SetBinDir(new_bindir);
  }
  if(valid_bindir == 0){
    char new_bindir[1024];

    strcpy(new_bindir, "C:\\Program Files\\firemodels\\SMV7");
    valid_bindir = SetBinDir(new_bindir);
  }
#endif
  if(smv_filename == NULL){
    DisplayVersionInfo("Smokeview ");
    SMV_EXIT(0);
  }
  if(show_version==1 || smv_filename==NULL){
    PRINTVERSION("smokeview", argv[0]);
    return 1;
  }
  if(CheckSMVFile(smv_filename, smokeview_casedir)==0){
    SMV_EXIT(1);
  }
  MakeFireColors(fire_temp_min, fire_temp_max, nfire_colors);

  InitTextureDir();
  InitColorbarsDir();
  InitScriptErrorFiles();
  smokezippath= GetSmokeZipPath(smokeview_bindir);
  DisplayVersionInfo("Smokeview ");
  InitStartupDirs();
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
