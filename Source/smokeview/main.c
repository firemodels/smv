#define INMAIN
#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

//   dummy

#include "string_util.h"
#include "smokeviewvars.h"

#ifdef pp_LUA
#include "c_api.h"
#include "lua_api.h"
#endif

/* ------------------ Usage ------------------------ */

void Usage(char *prog,int option){
  char buffer[1000];

  PRINTF("%s\n", release_title);
  PRINTF("%s\n\n", _("Visualize fire/smoke flow simulations."));
  PRINTF("Usage: %s [options] casename", GetBaseFileName(buffer, prog));
  PRINTF("%s\n\n", _("where "));
  PRINTF("%s\n", _(" casename       - project id (file names without the extension)"));
  PRINTF("%s\n", _(" -bindir dir    - specify location of smokeview bin directory"));
  PRINTF("%s\n", _(" -ini           - output smokeview parameter values to smokeview.ini"));
  PRINTF("%s\n", _(" -runscript     - run the script file casename.ssf"));
  UsageCommon(HELP_SUMMARY);
  if(option==HELP_ALL){
    PRINTF("\n%s\n", _("Other options:"));
#ifdef pp_READBUFFER
    PRINTF("%s\n", _(" -buffer        - scan .smv file using a memory buffer"));
#endif
    PRINTF("%s\n", _(" -build         - show directives used in this build of Smokeview"));
    PRINTF("%s\n", _(" -convert_ini case1.ini case2.ini - update case1.ini to the current format"));
    PRINTF("%s\n", _("                  and save results into case2.ini"));
    PRINTF("%s\n", _(" -demo          - use demonstrator mode of Smokeview"));
    PRINTF("%s\n", _(" -fast          - assume slice files exist in order to reduce startup time"));
    PRINTF("%s\n", _(" -fed           - pre-calculate all FED slice files"));
#ifdef pp_LANG
    PRINTF("%s\n", _(" -lang xx       - where xx is de, es, fr, it for German, Spanish, French or Italian"));
#endif
    PRINTF("%s\n", _(" -ng_ini        - non-graphics version of -ini."));
#ifdef pp_READBUFFER
    PRINTF("%s\n", _(" -no_buffer     - scan .smv file using file I/O rather from memory"));
#endif
    PRINTF("%s\n", _(" -setup         - only show geometry"));
    PRINTF("%s\n", _(" -script scriptfile - run the script file scriptfile"));
#ifdef pp_LUA
    PRINTF("%s\n", " -runluascript  - run the lua script file casename.lua");
    PRINTF("%s\n", " -luascript scriptfile - run the Lua script file scriptfile");
    PRINTF("%s\n", " -killscript    - exit smokeview (with an error code) if the script fails");
#endif
    PRINTF("%s\n", _(" -sizes         - output files sizes then exit"));
    PRINTF("%s\n", _(" -skipframe n   - render every n frames"));
    PRINTF("%s\n", _(" -smoke3d       - only show 3d smoke"));
    PRINTF("%s\n", _(" -startframe n  - start rendering at frame n"));
    PRINTF("%s\n", _(" -stereo        - activate stereo mode"));
    PRINTF("%s\n", _(" -tempdir       - forces output files to be written to the temporary directory"));
    PRINTF("%s\n", _(" -update_bounds - calculate boundary file bounds and save to casename.binfo"));
    PRINTF("%s\n", _(" -update_slice  - calculate slice file parameters"));
    PRINTF("%s\n", _(" -update        - equivalent to -update_bounds and -update_slice"));
    PRINTF("%s\n", _(" -update_ini case.ini - update case.ini to the current format"));
    PRINTF("%s\n", _(" -volrender     - generate images of volume rendered smoke and fire"));
    UsageCommon(HELP_ALL);
  }

  if(showbuild == 1){
    char label[1024], *labelptr;

    labelptr = label + 2;
    strcpy(label, "");
#ifdef _DEBUG
    strcat(label, ", _DEBUG");
#endif
#ifdef pp_append
    strcat(label, ", pp_append");
#endif
#ifdef pp_BETA
    strcat(label, ", pp_BETA");
#endif
#ifdef pp_COMPRESS
    strcat(label, ", pp_COMPRESS");
#endif
#ifdef pp_DEG
    strcat(label, ", pp_DEG");
#endif
#ifdef pp_DRAWISO
    strcat(label, ", pp_DRAWISO");
#endif
#ifdef pp_ffmpeg
    strcat(label, ", pp_ffmpeg");
#endif
#ifdef pp_FILELIST
    strcat(label, ", pp_FILELIST");
#endif
#ifdef pp_GCC
    strcat(label, ", pp_GCC");
#endif
#ifdef pp_GLUTGET
    strcat(label, ", pp_GLUTGET");
#endif
#ifdef pp_GPU
    strcat(label, ", pp_GPU");
#endif
#ifdef pp_GPU_CULL_STATE
    strcat(label, ", pp_GPU_CULL_STATE");
#endif
#ifdef pp_GPUDEPTH
    strcat(label, ", pp_GPUDEPTH");
#endif
#ifdef pp_GPUTHROTTLE
    strcat(label, ", pp_GPUTHROTTLE");
#endif
#ifdef pp_HASH
    strcat(label, ", pp_HASH");
#endif
#ifdef pp_INTEL
    strcat(label, ", pp_INTEL");
#endif
#ifdef pp_LANG
    strcat(label, ", pp_LANG");
#endif
#ifdef pp_LINUX
    strcat(label, ", pp_LINUX");
#endif
#ifdef pp_LUA
    strcat(label, ", pp_LUA");
#endif
#ifdef pp_LUA_SSF
    strcat(label, ", pp_LUA_SSF");
#endif
#ifdef pp_MEMDEBUG
    strcat(label, ", pp_MEMDEBUG");
#endif
#ifdef pp_MEMPRINT
    strcat(label, ", pp_MEMPRINT");
#endif
#ifdef pp_memstatus
    strcat(label, ", pp_memstatus");
#endif
#ifdef pp_OFFICIAL_RELEASE
    strcat(label, ", pp_OFFICIAL_RELEASE");
#endif
#ifdef pp_OSX
    strcat(label, ", pp_OSX");
#endif
#ifdef pp_OSXGLUT32
    strcat(label, ", pp_OSXGLUT32");
#endif
#ifdef pp_PARTTEST
    strcat(label, ", pp_PARTTEST");
#endif
#ifdef pp_READBUFFER
    strcat(label, ", pp_READBUFFER");
#endif
#ifdef pp_release
    strcat(label, ", pp_release");
#endif
#ifdef pp_RENDER360_DEBUG
    strcat(label, ", pp_RENDER360_DEBUG");
#endif
#ifdef pp_SETTIME
    strcat(label, ", pp_SETTIME");
#endif
#ifdef pp_SHOWTERRAIN
    strcat(label, ", pp_SHOWTERRAIN");
#endif
    strcat(label, ", pp_SLICELOAD");
#ifdef pp_THREAD
    strcat(label, ", pp_THREAD");
#endif
#ifdef pp_THREADIBLANK
    strcat(label, ", pp_THREADIBLANK");
#endif
#ifdef pp_TIMINGS
    strcat(label, ", pp_TIMINGS");
#endif
#ifdef WIN32
    strcat(label, ", WIN32");
#endif
#ifdef X64
    strcat(label, ", X64");
#endif

    PRINTF("  \n");
    PRINTF("%s\n\n", _("  Smokeview was built using the following pre-processing directives:"));
    PRINTF("%s \n", labelptr);
  }
}

/* ------------------ ParseCommandline ------------------------ */

void ParseCommandline(int argc, char **argv){
  int i, len_casename;
  int iarg;
  size_t len_memory;
  char *argi;
  char SMVFILENAME[1024];
  int smv_parse;

  CheckMemory;
  partscale = a_partscale;
  zonescale = a_zonescale;

  if(argc == 1){
    exit(1);
  }
  if(strncmp(argv[1], "-ini", 3) == 0){
    InitCameraList();
    InitOpenGL();
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    WriteIni(GLOBAL_INI, NULL);
    exit(0);
  }

  if(strncmp(argv[1], "-ng_ini", 6) == 0){
    InitCameraList();
    use_graphics = 0;
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    WriteIni(GLOBAL_INI, NULL);
    exit(0);
  }
  strcpy(SMVFILENAME, "");
  smv_parse = 0;
  for(iarg = 1; iarg < argc; iarg++){
    argi = argv[iarg];
    if(strncmp(argi, "-", 1) == 0){
      if(
        strncmp(argi, "-points", 7) == 0 ||
        strncmp(argi, "-frames", 7) == 0 ||
#ifdef pp_LANG
        strncmp(argi, "-lang", 5) == 0 ||
#endif
        strncmp(argi, "-script", 7) == 0 ||
#ifdef pp_LUA
        strncmp(argi, "-luascript", 10) == 0 ||
#endif
        strncmp(argi, "-startframe", 11) == 0 ||
        strncmp(argi, "-skipframe", 10) == 0 ||
        strncmp(argi, "-bindir", 7) == 0 ||
        strncmp(argi, "-update_ini", 11) == 0
        ){
        iarg++;
      }
      if(strncmp(argi, "-convert_ini", 12) == 0 ||
        strncmp(argi, "-convert_ssf", 12) == 0){
        iarg += 2;
      }

      if(smv_parse == 0)continue;
      if(smv_parse == 1)break;
    }
    if(smv_parse == 1)strcat(SMVFILENAME, " ");
    smv_parse = 1;
    strcat(SMVFILENAME, argi);
  }

  argi = SMVFILENAME;
#ifndef pp_OSX
  argi = LastName(argi);
#endif
  len_casename = (int)strlen(argi);
  CheckMemory;
  FREEMEMORY(fdsprefix);
  len_memory = len_casename + strlen(part_ext) + 100;
  NewMemory((void **)&fdsprefix, (unsigned int)len_memory);
  STRCPY(fdsprefix, argi);
  strcpy(movie_name, fdsprefix);
  strcpy(render_file_base, fdsprefix);
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
  NewMemory((void **)&caseini_filename, len_casename + strlen(ini_ext) + 1);
  STRCPY(caseini_filename, fdsprefix);
  STRCAT(caseini_filename, ini_ext);

  FREEMEMORY(boundinfo_filename);
  NewMemory((void **)&boundinfo_filename, len_casename + strlen(".binfo") + 1);
  STRCPY(boundinfo_filename, fdsprefix);
  STRCAT(boundinfo_filename, ".binfo");

  if(smv_filename == NULL){
    NewMemory((void **)&smv_filename, (unsigned int)(len_casename + 6));
    STRCPY(smv_filename, fdsprefix);
    STRCAT(smv_filename, ".smv");
    {
      char scriptbuffer[1024];

      STRCPY(scriptbuffer, fdsprefix);
      STRCAT(scriptbuffer, ".ssf");
      if(default_script == NULL&&FILE_EXISTS(scriptbuffer) == YES){
        default_script = InsertScriptFile(scriptbuffer);
      }
    }
#ifdef pp_LUA
    {
      char luascriptbuffer[1024];

      STRCPY(luascriptbuffer, fdsprefix);
      STRCAT(luascriptbuffer, ".lua");
      if(default_luascript == NULL&&FILE_EXISTS(luascriptbuffer) == YES){
        default_luascript = insert_luascriptfile(luascriptbuffer);
      }
    }
#endif
  }
  if(smv_filename != NULL){
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
  if(fed_filename == NULL){
    STRCPY(fed_filename_base, fdsprefix);
    STRCAT(fed_filename_base, ".fed_smv");
    fed_filename = GetFileName(smokeviewtempdir, fed_filename_base, tempdir_flag);
  }
  if(stop_filename == NULL){
    NewMemory((void **)&stop_filename, (unsigned int)(len_casename + strlen(".stop") + 1));
    STRCPY(stop_filename, fdsprefix);
    STRCAT(stop_filename, ".stop");
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

  for(i = 1; i < argc; i++){
    if(strncmp(argv[i], "-", 1) != 0)continue;
    if(strncmp(argv[i], "-update_bounds", 14) == 0){
      use_graphics = 0;
      update_bounds = 1;
    }
    else if(strncmp(argv[i], "-update_slice", 13)==0){
      use_graphics = 0;
      update_slice = 1;
    }
#ifdef pp_READBUFFER
    else if(strncmp(argv[i], "-no_buffer", 10)==0){
      readfile_option = READFILE;
    }
    else if(strncmp(argv[i], "-buffer", 7)==0){
      readfile_option = READBUFFER;
    }
#endif
    else if(strncmp(argv[i], "-update", 7)==0){
      if(strncmp(argv[i], "-update_slice", 13)!=0&&strncmp(argv[i], "-update_bounds", 14)!=0){
        use_graphics = 0;
        update_slice = 1;
        update_bounds = 1;
      }
    }
    else if(strncmp(argv[i], "-nogpu", 6) == 0){
      disable_gpu = 1;
    }
    else if(strncmp(argv[i], "-demo", 5) == 0){
      demo_option = 1;
    }
    else if(strncmp(argv[1], "-sizes", 6)==0){
      update_filesizes = 1;
      use_graphics = 0;
    }
    else if(strncmp(argv[i], "-stereo", 7) == 0){
      stereoactive = 1;
      stereotype = STEREO_TIME;
      PRINTF("stereo option activated\n");
    }
#ifdef pp_LANG
    else if(strncmp(argv[i], "-lang", 5) == 0){
      ++i;
      if(i < argc){
        int langlen;
        char *lang;

        FREEMEMORY(tr_name);
        lang = argv[i];
        langlen = strlen(lang);
        NewMemory((void **)&tr_name, langlen + 48 + 1);
        strcpy(tr_name, lang);
      }
    }
#endif
    else if(strncmp(argv[i], "-convert_ini", 12) == 0){
      char *local_ini_from = NULL, *local_ini_to = NULL;

      if(++i < argc)local_ini_from = argv[i];
      if(++i < argc)local_ini_to = argv[i];
      if(local_ini_from != NULL&&local_ini_to != NULL){
        NewMemory((void **)&ini_from, strlen(local_ini_from) + 1);
        strcpy(ini_from, local_ini_from);

        NewMemory((void **)&ini_to, strlen(local_ini_to) + 1);
        strcpy(ini_to, local_ini_to);
        convert_ini = 1;
      }
    }
    else if(strncmp(argv[i], "-convert_ssf", 12) == 0){
      char *local_ssf_from = NULL, *local_ssf_to = NULL;

      if(++i < argc)local_ssf_from = argv[i];
      if(++i < argc)local_ssf_to = argv[i];
      if(local_ssf_from != NULL&&local_ssf_to != NULL){
        NewMemory((void **)&ssf_from, strlen(local_ssf_from) + 1);
        strcpy(ssf_from, local_ssf_from);

        NewMemory((void **)&ssf_to, strlen(local_ssf_to) + 1);
        strcpy(ssf_to, local_ssf_to);
        convert_ssf = 1;
      }
    }
    else if(strncmp(argv[i], "-update_ssf", 11) == 0){
      update_ssf = 1;
    }
    else if(strncmp(argv[i], "-update_ini", 11) == 0){
      char *local_ini_from = NULL, *local_ini_to = NULL;

      if(++i < argc)local_ini_from = argv[i];
      local_ini_to = local_ini_from;
      if(local_ini_from != NULL){
        NewMemory((void **)&ini_from, strlen(local_ini_from) + 1);
        strcpy(ini_from, local_ini_from);

        NewMemory((void **)&ini_to, strlen(local_ini_to) + 1);
        strcpy(ini_to, local_ini_to);
        convert_ini = 1;
      }
    }
    else if(strncmp(argv[i], "-isotest", 8) == 0){
      isotest = 1;
    }
    else if(strncmp(argv[i], "-smoke3d", 8) == 0){
      smoke3d_only = 1;
    }
#ifdef _DEBUG
    else if(strncmp(argv[i], "-tempdir", 8) == 0){
      tempdir_flag = 1;
    }
#endif
    else if(strncmp(argv[i], "-h", 2) == 0&&strncmp(argv[i], "-help_all", 9)!=0){
      Usage(argv[0],HELP_SUMMARY);
      exit(0);
    }
    else if(strncmp(argv[i], "-help_all", 9)==0){
      Usage(argv[0],HELP_ALL);
      exit(0);
    }
    else if(strncmp(argv[i], "-noblank", 8) == 0){
      iblank_set_on_commandline = 1;
      use_iblank = 0;
    }
    else if(strncmp(argv[i], "-fed", 4) == 0){
      compute_fed = 1;
    }
    else if(strncmp(argv[i], "-fast", 5) == 0){
      fast_startup = 1;
      lookfor_zip = 0;
    }
    else if(strncmp(argv[i], "-blank", 6) == 0){
      iblank_set_on_commandline = 1;
      use_iblank = 1;
    }
    else if(strncmp(argv[i], "-gversion", 9) == 0){
      gversion = 1;
    }
    else if(
      strncmp(argv[i], "-volrender", 10) != 0 && (strncmp(argv[i], "-version", 8) == 0 || strncmp(argv[i], "-v", 2) == 0)
      ){
      DisplayVersionInfo("Smokeview ");
      exit(0);
    }
    else if(
      strncmp(argv[i], "-redirect", 9) == 0
      ){
      LOG_FILENAME = fopen(log_filename, "w");
      if(LOG_FILENAME != NULL){
        redirect = 1;
        SetStdOut(LOG_FILENAME);
      }
    }
    else if(strncmp(argv[i], "-runscript", 10) == 0){
      from_commandline = 1;
#ifdef pp_LUA
      strcpy(script_filename, "");
#endif
      runscript = 1;
    }
#ifdef pp_LUA
    else if(strncmp(argv[i], "-runluascript", 13) == 0){
      from_commandline = 1;
      strcpy(luascript_filename, "");
      strncpy(luascript_filename, fdsprefix, 1020);
      strcat(luascript_filename, ".lua");
      runluascript = 1;
    }
    else if(strncmp(argv[i], "-killscript", 11) == 0){
      from_commandline = 1;
      exit_on_script_crash = 1;
    }
#endif
    else if(strncmp(argv[i], "-skipframe", 10) == 0){
      from_commandline = 1;
      ++i;
      if(i < argc){
        sscanf(argv[i], "%i", &skipframe0);
      }
    }
    else if(strncmp(argv[i], "-startframe", 11) == 0){
      from_commandline = 1;
      ++i;
      if(i < argc){
        sscanf(argv[i], "%i", &startframe0);
      }
    }
    else if(strncmp(argv[i], "-volrender", 10) == 0){
      from_commandline = 1;
      make_volrender_script = 1;
    }
    else if(strncmp(argv[i], "-script", 7) == 0){
      from_commandline = 1;
      ++i;
      if(i < argc){
        char scriptbuffer[256];
        scriptfiledata *sfd;

        strcpy(scriptbuffer, argv[i]);
        sfd = InsertScriptFile(scriptbuffer);
        if(sfd != NULL)default_script = sfd;
        runscript = 1;
      }
    }
#ifdef pp_LUA
    else if(strncmp(argv[i], "-luascript", 10) == 0){
      from_commandline = 1;
      ++i;
      if(i < argc){
        strncpy(luascript_filename, argv[i], 1024);
        runluascript = 1;
      }
    }
#endif
    else if(strncmp(argv[i], "-noexit", 7) == 0){
      noexit = 1;
    }
    else if(strncmp(argv[i], "-setup", 6) == 0){
      setup_only = 1;
    }
    else if(strncmp(argv[i], "-bindir", 7) == 0){
      ++i;
      if(i < argc){
        int len2;

        len2 = strlen(argv[i]);
        NewMemory((void **)&smokeview_bindir, len2 + 2);
        strcpy(smokeview_bindir, argv[i]);
        if(smokeview_bindir[len2 - 1] != dirseparator[0])strcat(smokeview_bindir, dirseparator);
      }
    }
    else if(strncmp(argv[i], "-build", 6) == 0){
      showbuild = 1;
      Usage(argv[0],HELP_ALL);
      exit(0);
    }
    else{
      fprintf(stderr, "*** Error: unknown option: %s\n", argv[i]);
      Usage(argv[0],HELP_ALL);
      exit(1);
    }
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
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  char **argv_sv;
  int return_code;
  char *progname;
#ifdef pp_LUA
  // If we are using lua, let lua take control here.
  // Initialise the lua interpreter, it does not take control at this point
  lua_State *L = initLua();
  // This code branch gives more control to the interpreter during startup.
  return_code = RunLuaBranch(L, argc, argv);
  // All of the below code is run by the lua interpreter, therefore if we want
  // to use the lua interpreter we ignore the code below and exit once the lua
  // run is complete.
  return return_code;
#endif
  SetStdOut(stdout);
  initMALLOC();
  InitRandAB(1000000);
  InitVars();
  if(argc==1)DisplayVersionInfo("Smokeview ");
  CopyArgs(&argc, argv, &argv_sv);
  if(argc==0||argc==1)return 0;

  progname=argv_sv[0];

  ParseCommonOptions(argc, argv_sv);
  if(show_help==1){
    Usage("smokeview",HELP_SUMMARY);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("smokeview", argv_sv[0]);
    return 1;
  }

  prog_fullpath = progname;
#ifdef pp_LUA
  smokeview_bindir_abs=getprogdirabs(progname,&smokeviewpath);
#endif
  ParseCommandline(argc, argv_sv);
  if(smokeview_bindir==NULL){
    smokeview_bindir= GetProgDir(progname,&smokeviewpath);
  }
  InitTextureDir();
  InitScriptErrorFiles();
  smokezippath= GetSmokeZipPath(smokeview_bindir);
#ifdef pp_ffmpeg
#ifdef WIN32
  have_ffmpeg = HaveProg("ffmpeg -version> Nul 2>Nul");
  have_ffplay = HaveProg("ffplay -version> Nul 2>Nul");
#else
  have_ffmpeg = HaveProg("ffmpeg -version >/dev/null 2>/dev/null");
  have_ffplay = HaveProg("ffplay -version >/dev/null 2>/dev/null");
#endif
#endif
  DisplayVersionInfo("Smokeview ");
  SetupGlut(argc,argv_sv);
  START_TIMER(startup_time);
  START_TIMER(read_time_elapsed);

  return_code= SetupCase(argc,argv_sv);
  if(return_code==0&&update_bounds==1)return_code=Update_Bounds();
  if(return_code!=0)return 1;
  if(convert_ini==1){
    ReadIni(ini_from);
  }

  STOP_TIMER(startup_time);
  PRINTF("\nStartup time: %.1f s\n", startup_time);
  glutMainLoop();
  return 0;
}
