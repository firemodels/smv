#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include GLUT_H

#include "MALLOCC.h"
#include "command_args.h"
#include "smokeviewvars.h"
#include "string_util.h"

#ifndef _WIN32
#include <libgen.h>
#endif

/// @brief Given a file path, get the filename excluding the final extension.
/// This allocates a new copy which can be deallocated with free().
/// @param input_file a file path
/// @return an allocated string containing the basename or NULL on failure.
char *GetBaseName(const char *input_file) {
  if (input_file == NULL) return NULL;
#ifdef _WIN32
  char *result = malloc(_MAX_FNAME + 1);
  errno_t err =
      _splitpath_s(input_file, NULL, 0, NULL, 0, result, _MAX_FNAME, NULL, 0);
  if (err) return NULL;
#else
  // POSIX basename can modify it's contents, so we'll make some copies.
  char *input_file_temp = strdup(input_file);
  // Get the filename (final component of the path, including any extensions).
  char *bname = basename(input_file_temp);
  // If a '.' exists, set it to '\0' to trim the extension.
  char *dot = strrchr(bname, '.');
  if (dot) *dot = '\0';
  char *result = strdup(bname);
  free(input_file_temp);
#endif
  return result;
}

int SetGlobalFilenames(const char *fdsprefix) {
  int len_casename = strlen(fdsprefix);
  strcpy(movie_name, fdsprefix);
  strcpy(render_file_base, fdsprefix);
  strcpy(html_file_base, fdsprefix);

  FREEMEMORY(log_filename);
  NewMemory((void **)&log_filename, len_casename + strlen(".smvlog") + 1);
  STRCPY(log_filename, fdsprefix);
  STRCAT(log_filename, ".smvlog");

  FREEMEMORY(caseini_filename);
  NewMemory((void **)&caseini_filename, len_casename + strlen(".ini") + 1);
  STRCPY(caseini_filename, fdsprefix);
  STRCAT(caseini_filename, ".ini");

  FREEMEMORY(expcsv_filename);
  NewMemory((void **)&expcsv_filename, len_casename + strlen("_exp.csv") + 1);
  STRCPY(expcsv_filename, fdsprefix);
  STRCAT(expcsv_filename, "_exp.csv");

  FREEMEMORY(dEcsv_filename);
  NewMemory((void **)&dEcsv_filename, len_casename + strlen("_dE.csv") + 1);
  STRCPY(dEcsv_filename, fdsprefix);
  STRCAT(dEcsv_filename, "_dE.csv");

  FREEMEMORY(html_filename);
  NewMemory((void **)&html_filename, len_casename + strlen(".html") + 1);
  STRCPY(html_filename, fdsprefix);
  STRCAT(html_filename, ".html");

  FREEMEMORY(smv_orig_filename);
  NewMemory((void **)&smv_orig_filename, len_casename + strlen(".smo") + 1);
  STRCPY(smv_orig_filename, fdsprefix);
  STRCAT(smv_orig_filename, ".smo");

  FREEMEMORY(hrr_filename);
  NewMemory((void **)&hrr_filename, len_casename + strlen("_hrr.csv") + 1);
  STRCPY(hrr_filename, fdsprefix);
  STRCAT(hrr_filename, "_hrr.csv");

  FREEMEMORY(htmlvr_filename);
  NewMemory((void **)&htmlvr_filename, len_casename + strlen("_vr.html") + 1);
  STRCPY(htmlvr_filename, fdsprefix);
  STRCAT(htmlvr_filename, "_vr.html");

  FREEMEMORY(htmlobst_filename);
  NewMemory((void **)&htmlobst_filename,
            len_casename + strlen("_obst.json") + 1);
  STRCPY(htmlobst_filename, fdsprefix);
  STRCAT(htmlobst_filename, "_obst.json");

  FREEMEMORY(htmlslicenode_filename);
  NewMemory((void **)&htmlslicenode_filename,
            len_casename + strlen("_slicenode.json") + 1);
  STRCPY(htmlslicenode_filename, fdsprefix);
  STRCAT(htmlslicenode_filename, "_slicenode.json");

  FREEMEMORY(htmlslicecell_filename);
  NewMemory((void **)&htmlslicecell_filename,
            len_casename + strlen("_slicecell.json") + 1);
  STRCPY(htmlslicecell_filename, fdsprefix);
  STRCAT(htmlslicecell_filename, "_slicecell.json");

  FREEMEMORY(event_filename);
  NewMemory((void **)&event_filename, len_casename + strlen("_events.csv") + 1);
  STRCPY(event_filename, fdsprefix);
  STRCAT(event_filename, "_events.csv");

  if (ffmpeg_command_filename == NULL) {
    NewMemory((void **)&ffmpeg_command_filename,
              (unsigned int)(len_casename + 12));
    STRCPY(ffmpeg_command_filename, fdsprefix);
    STRCAT(ffmpeg_command_filename, "_ffmpeg");
#ifdef WIN32
    STRCAT(ffmpeg_command_filename, ".bat");
#else
    STRCAT(ffmpeg_command_filename, ".sh");
#endif
  }
  if (fed_filename == NULL) {
    STRCPY(fed_filename_base, fdsprefix);
    STRCAT(fed_filename_base, ".fed_smv");
    fed_filename =
        GetFileName(smokeview_scratchdir, fed_filename_base, NOT_FORCE_IN_DIR);
  }
  if (stop_filename == NULL) {
    NewMemory((void **)&stop_filename,
              (unsigned int)(len_casename + strlen(".stop") + 1));
    STRCPY(stop_filename, fdsprefix);
    STRCAT(stop_filename, ".stop");
  }
  if (smvzip_filename == NULL) {
    NewMemory((void **)&smvzip_filename,
              (unsigned int)(len_casename + strlen(".smvzip") + 1));
    STRCPY(smvzip_filename, fdsprefix);
    STRCAT(smvzip_filename, ".smvzip");
  }
  if (sliceinfo_filename == NULL) {
    NewMemory((void **)&sliceinfo_filename,
              strlen(fdsprefix) + strlen(".sinfo") + 1);
    STRCPY(sliceinfo_filename, fdsprefix);
    STRCAT(sliceinfo_filename, ".sinfo");
  }
  if (deviceinfo_filename == NULL) {
    NewMemory((void **)&deviceinfo_filename,
              strlen(fdsprefix) + strlen("_device.info") + 1);
    STRCPY(deviceinfo_filename, fdsprefix);
    STRCAT(deviceinfo_filename, "_device.info");
  }

  // if smokezip created part2iso files then concatenate .smv entries found in
  // the .isosmv file to the end of the .smv file creating a new .smv file. Then
  // read in that .smv file.

  {
    FILE *stream_iso = NULL;

    NewMemory((void **)&iso_filename, len_casename + strlen(".isosmv") + 1);
    STRCPY(iso_filename, fdsprefix);
    STRCAT(iso_filename, ".isosmv");
    stream_iso = fopen(iso_filename, "r");
    if (stream_iso != NULL) {
      fclose(stream_iso);
    }
    else {
      FREEMEMORY(iso_filename);
    }
  }

  if (trainer_filename == NULL) {
    NewMemory((void **)&trainer_filename, (unsigned int)(len_casename + 6));
    STRCPY(trainer_filename, fdsprefix);
    STRCAT(trainer_filename, ".svd");
  }
  if (test_filename == NULL) {
    NewMemory((void **)&test_filename, (unsigned int)(len_casename + 6));
    STRCPY(test_filename, fdsprefix);
    STRCAT(test_filename, ".svd");
  }
  return 0;
}

int RunBenchmark(char *input_file) {
  initMALLOC();
  InitVars();
  SetGlobalFilenames(fdsprefix);

  INIT_PRINT_TIMER(parse_time);
  printf("reading:\t%s\n", input_file);
  {
    bufferstreamdata *smv_streaminfo = GetSMVBuffer(NULL, input_file);
    if (smv_streaminfo == NULL) {
      printf("could not open %s\n", input_file);
      return 1;
    }
    INIT_PRINT_TIMER(ReadSMV_time);
    int return_code = ReadSMV(smv_streaminfo);
    STOP_TIMER(ReadSMV_time);
    printf("ReadSMV:\t%8.3f ms\n", ReadSMV_time * 1000);
    if (smv_streaminfo != NULL) {
      FCLOSE(smv_streaminfo);
    }
    if (return_code) return return_code;
  }
  show_timings = 1;
  ReadSMVOrig();
  INIT_PRINT_TIMER(ReadSMVDynamic_time);
  ReadSMVDynamic(input_file);
  STOP_TIMER(ReadSMVDynamic_time);
  printf("ReadSMVDynamic:\t%8.3f ms\n", ReadSMVDynamic_time * 1000);
  STOP_TIMER(parse_time);
  printf("Total Time:\t%8.3f ms\n", parse_time * 1000);
  return 0;
}

int main(int argc, char **argv) {
  char *input_file = argv[1];
  if (input_file == NULL) {
    fprintf(stderr, "No input file specified.\n");
    return 1;
  }
  fdsprefix = GetBaseName(input_file);
  int result = RunBenchmark(input_file);
  if (fdsprefix != NULL) free(fdsprefix);
  return result;
}
