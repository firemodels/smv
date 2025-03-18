#define INMAIN
#include "options.h"
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dmalloc.h"
#include "shared_structures.h"

#include "smokeviewvars.h"
#include "string_util.h"
#include <math.h>
#include "readlabel.h"
#include "readsmvfile.h"

#include <json-c/json_object.h>

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

int SetGlobalFilenames() {
  int len_casename = strlen(global_scase.fdsprefix);
  strcpy(movie_name, global_scase.fdsprefix);
  strcpy(render_file_base, global_scase.fdsprefix);
  strcpy(html_file_base, global_scase.fdsprefix);

  FREEMEMORY(global_scase.paths.log_filename);
  NewMemory((void **)&global_scase.paths.log_filename, len_casename + strlen(".smvlog") + 1);
  STRCPY(global_scase.paths.log_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.log_filename, ".smvlog");

  FREEMEMORY(global_scase.paths.caseini_filename);
  NewMemory((void **)&global_scase.paths.caseini_filename, len_casename + strlen(".ini") + 1);
  STRCPY(global_scase.paths.caseini_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.caseini_filename, ".ini");

  FREEMEMORY(global_scase.paths.expcsv_filename);
  NewMemory((void **)&global_scase.paths.expcsv_filename, len_casename + strlen("_exp.csv") + 1);
  STRCPY(global_scase.paths.expcsv_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.expcsv_filename, "_exp.csv");

  FREEMEMORY(global_scase.paths.dEcsv_filename);
  NewMemory((void **)&global_scase.paths.dEcsv_filename, len_casename + strlen("_dE.csv") + 1);
  STRCPY(global_scase.paths.dEcsv_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.dEcsv_filename, "_dE.csv");

  FREEMEMORY(global_scase.paths.html_filename);
  NewMemory((void **)&global_scase.paths.html_filename, len_casename + strlen(".html") + 1);
  STRCPY(global_scase.paths.html_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.html_filename, ".html");

  FREEMEMORY(global_scase.paths.smv_orig_filename);
  NewMemory((void **)&global_scase.paths.smv_orig_filename, len_casename + strlen(".smo") + 1);
  STRCPY(global_scase.paths.smv_orig_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.smv_orig_filename, ".smo");

  FREEMEMORY(global_scase.paths.hrr_filename);
  NewMemory((void **)&global_scase.paths.hrr_filename, len_casename + strlen("_hrr.csv") + 1);
  STRCPY(global_scase.paths.hrr_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.hrr_filename, "_hrr.csv");

  FREEMEMORY(global_scase.paths.htmlvr_filename);
  NewMemory((void **)&global_scase.paths.htmlvr_filename, len_casename + strlen("_vr.html") + 1);
  STRCPY(global_scase.paths.htmlvr_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.htmlvr_filename, "_vr.html");

  FREEMEMORY(global_scase.paths.htmlobst_filename);
  NewMemory((void **)&global_scase.paths.htmlobst_filename,
            len_casename + strlen("_obst.json") + 1);
  STRCPY(global_scase.paths.htmlobst_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.htmlobst_filename, "_obst.json");

  FREEMEMORY(global_scase.paths.htmlslicenode_filename);
  NewMemory((void **)&global_scase.paths.htmlslicenode_filename,
            len_casename + strlen("_slicenode.json") + 1);
  STRCPY(global_scase.paths.htmlslicenode_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.htmlslicenode_filename, "_slicenode.json");

  FREEMEMORY(global_scase.paths.htmlslicecell_filename);
  NewMemory((void **)&global_scase.paths.htmlslicecell_filename,
            len_casename + strlen("_slicecell.json") + 1);
  STRCPY(global_scase.paths.htmlslicecell_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.htmlslicecell_filename, "_slicecell.json");

  FREEMEMORY(global_scase.paths.event_filename);
  NewMemory((void **)&global_scase.paths.event_filename, len_casename + strlen("_events.csv") + 1);
  STRCPY(global_scase.paths.event_filename, global_scase.fdsprefix);
  STRCAT(global_scase.paths.event_filename, "_events.csv");

  if (global_scase.paths.ffmpeg_command_filename == NULL) {
    NewMemory((void **)&global_scase.paths.ffmpeg_command_filename,
              (unsigned int)(len_casename + 12));
    STRCPY(global_scase.paths.ffmpeg_command_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.ffmpeg_command_filename, "_ffmpeg");
#ifdef WIN32
    STRCAT(global_scase.paths.ffmpeg_command_filename, ".bat");
#else
    STRCAT(global_scase.paths.ffmpeg_command_filename, ".sh");
#endif
  }
  if (global_scase.paths.smvzip_filename == NULL) {
    NewMemory((void **)&global_scase.paths.smvzip_filename,
              (unsigned int)(len_casename + strlen(".smvzip") + 1));
    STRCPY(global_scase.paths.smvzip_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.smvzip_filename, ".smvzip");
  }
  if (global_scase.paths.sliceinfo_filename == NULL) {
    NewMemory((void **)&global_scase.paths.sliceinfo_filename,
              strlen(global_scase.fdsprefix) + strlen(".sinfo") + 1);
    STRCPY(global_scase.paths.sliceinfo_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.sliceinfo_filename, ".sinfo");
  }
  if (global_scase.paths.deviceinfo_filename == NULL) {
    NewMemory((void **)&global_scase.paths.deviceinfo_filename,
              strlen(global_scase.fdsprefix) + strlen("_device.info") + 1);
    STRCPY(global_scase.paths.deviceinfo_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.deviceinfo_filename, "_device.info");
  }

  // if smokezip created part2iso files then concatenate .smv entries found in
  // the .isosmv file to the end of the .smv file creating a new .smv file. Then
  // read in that .smv file.

  {
    FILE *stream_iso = NULL;

    NewMemory((void **)&global_scase.paths.iso_filename, len_casename + strlen(".isosmv") + 1);
    STRCPY(global_scase.paths.iso_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.iso_filename, ".isosmv");
    stream_iso = fopen(global_scase.paths.iso_filename, "r");
    if (stream_iso != NULL) {
      fclose(stream_iso);
    }
    else {
      FREEMEMORY(global_scase.paths.iso_filename);
    }
  }

  if (global_scase.paths.trainer_filename == NULL) {
    NewMemory((void **)&global_scase.paths.trainer_filename, (unsigned int)(len_casename + 6));
    STRCPY(global_scase.paths.trainer_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.trainer_filename, ".svd");
  }
  if (global_scase.paths.test_filename == NULL) {
    NewMemory((void **)&global_scase.paths.test_filename, (unsigned int)(len_casename + 6));
    STRCPY(global_scase.paths.test_filename, global_scase.fdsprefix);
    STRCAT(global_scase.paths.test_filename, ".svd");
  }
  return 0;
}

int PrintJson(smv_case *scase) {
  struct json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "version", json_object_new_int(1));
  json_object_object_add(jobj, "chid",
                         json_object_new_string(scase->paths.chidfilebase));
  if(scase->paths.fds_filein != NULL) {
    json_object_object_add(jobj, "input_file",
                           json_object_new_string(scase->paths.fds_filein));
  }
  if(scase->fds_title != NULL) {
    json_object_object_add(jobj, "title",
                           json_object_new_string(scase->fds_title));
  }
  if(scase->fds_version != NULL) {
  json_object_object_add(jobj, "fds_version",
                         json_object_new_string(scase->fds_version));
  }
  struct json_object *mesh_array = json_object_new_array();
  for(int i = 0; i < scase->meshescoll.nmeshes; i++) {
    meshdata *mesh = &scase->meshescoll.meshinfo[i];
    struct json_object *mesh_obj = json_object_new_object();
    json_object_object_add(mesh_obj, "index", json_object_new_int(i + 1));
    if(mesh->label != NULL) {
      json_object_object_add(mesh_obj, "id",
                             json_object_new_string(mesh->label));
    }
    struct json_object *mesh_coordinates = json_object_new_object();
    json_object_object_add(mesh_coordinates, "i",
                           json_object_new_int(mesh->ibar));
    json_object_object_add(mesh_coordinates, "j",
                           json_object_new_int(mesh->jbar));
    json_object_object_add(mesh_coordinates, "k",
                           json_object_new_int(mesh->kbar));
    json_object_object_add(mesh_obj, "coordinates", mesh_coordinates);
    struct json_object *mesh_dimensions = json_object_new_object();
    json_object_object_add(mesh_dimensions, "x_min",
                           json_object_new_double(mesh->x0));
    json_object_object_add(mesh_dimensions, "x_max",
                           json_object_new_double(mesh->x1));
    json_object_object_add(mesh_dimensions, "y_min",
                           json_object_new_double(mesh->y0));
    json_object_object_add(mesh_dimensions, "y_max",
                           json_object_new_double(mesh->y1));
    json_object_object_add(mesh_dimensions, "z_min",
                           json_object_new_double(mesh->z0));
    json_object_object_add(mesh_dimensions, "z_max",
                           json_object_new_double(mesh->z1));
    json_object_object_add(mesh_obj, "dimensions", mesh_dimensions);

    struct json_object *vents = json_object_new_array();
    for(int j = 0; j < mesh->nvents; j++) {
      ventdata *vent = &mesh->ventinfo[j];
      struct json_object *vent_obj = json_object_new_object();
      json_object_object_add(vent_obj, "index", json_object_new_int(j + 1));
      json_object_object_add(
          vent_obj, "surface_id",
          json_object_new_string(vent->surf[0]->surfacelabel));
      struct json_object *dimensions = json_object_new_object();
      json_object_object_add(dimensions, "x_min",
                             json_object_new_double(vent->xmin));
      json_object_object_add(dimensions, "x_max",
                             json_object_new_double(vent->xmax));
      json_object_object_add(dimensions, "y_min",
                             json_object_new_double(vent->ymin));
      json_object_object_add(dimensions, "y_max",
                             json_object_new_double(vent->ymax));
      json_object_object_add(dimensions, "z_min",
                             json_object_new_double(vent->zmin));
      json_object_object_add(dimensions, "z_max",
                             json_object_new_double(vent->zmax));
      json_object_object_add(vent_obj, "dimensions", dimensions);

      struct json_object *coordinates = json_object_new_object();
      json_object_object_add(coordinates, "i_min",
                             json_object_new_int(vent->imin));
      json_object_object_add(coordinates, "i_max",
                             json_object_new_int(vent->imax));
      json_object_object_add(coordinates, "j_min",
                             json_object_new_int(vent->jmin));
      json_object_object_add(coordinates, "j_max",
                             json_object_new_int(vent->jmax));
      json_object_object_add(coordinates, "k_min",
                             json_object_new_int(vent->kmin));
      json_object_object_add(coordinates, "k_max",
                             json_object_new_int(vent->kmax));
      json_object_object_add(vent_obj, "coordinates", coordinates);
      json_object_array_add(vents, vent_obj);
    }
    json_object_object_add(mesh_obj, "vents", vents);

    json_object_array_add(mesh_array, mesh_obj);
  }
  json_object_object_add(jobj, "meshes", mesh_array);

  // TODO: the parse rejects CSV files that it doesn't find in it's own working
  // directory.
  struct json_object *csv_files = json_object_new_array();
  for(int i = 0; i < scase->csvcoll.ncsvfileinfo; i++) {
    csvfiledata *csv_file = &scase->csvcoll.csvfileinfo[i];
    struct json_object *csv_obj = json_object_new_object();
    json_object_object_add(csv_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(csv_obj, "filename",
                           json_object_new_string(csv_file->file));
    json_object_object_add(csv_obj, "type",
                           json_object_new_string(csv_file->c_type));
    json_object_array_add(csv_files, csv_obj);
  }
  json_object_object_add(jobj, "csv_files", csv_files);

  // Add devices to JSON
  struct json_object *devices = json_object_new_array();
  for(int i = 0; i < scase->devicecoll.ndeviceinfo; i++) {
    devicedata *device = &scase->devicecoll.deviceinfo[i];
    struct json_object *device_obj = json_object_new_object();
    json_object_object_add(device_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(device_obj, "id",
                           json_object_new_string(device->deviceID));
    json_object_object_add(device_obj, "csvlabel",
                           json_object_new_string(device->csvlabel));
    json_object_object_add(device_obj, "label",
                           json_object_new_string(device->labelptr));
    json_object_object_add(device_obj, "quantity",
                           json_object_new_string(device->quantity));
    if (device->have_xyz) {
      struct json_object *device_position = json_object_new_object();
      json_object_object_add(device_position, "x",
                             json_object_new_double(device->xyz[0]));
      json_object_object_add(device_position, "y",
                             json_object_new_double(device->xyz[1]));
      json_object_object_add(device_position, "z",
                             json_object_new_double(device->xyz[2]));
      json_object_object_add(device_obj, "position", device_position);
    }
    if (device->act_times != NULL) {
      struct json_object *state_changes = json_object_new_array();
      for (int j = 0; j < device->nstate_changes; j++) {
        struct json_object *state_change = json_object_new_object();
        json_object_object_add(state_change, "time",
                               json_object_new_double(device->act_times[j]));
        json_object_object_add(state_change, "value",
                               json_object_new_int(device->state_values[j]));
        json_object_array_add(state_changes, state_change);
      }
      json_object_object_add(device_obj, "state_changes", state_changes);
    }
    json_object_array_add(devices, device_obj);
  }
  json_object_object_add(jobj, "devices", devices);

  // Add slices to JSON
  struct json_object *slices = json_object_new_array();
  for(int i = 0; i < scase->slicecoll.nsliceinfo; i++) {
    slicedata *slice = &scase->slicecoll.sliceinfo[i];
    struct json_object *slice_obj = json_object_new_object();
    json_object_object_add(slice_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(slice_obj, "mesh",
                           json_object_new_int(slice->blocknumber));
    if (slice->label.longlabel != NULL) {
      json_object_object_add(slice_obj, "longlabel",
                             json_object_new_string(slice->label.longlabel));
    }
    if (slice->label.shortlabel) {
      json_object_object_add(slice_obj, "shortlabel",
                             json_object_new_string(slice->label.shortlabel));
    }
    if (slice->slicelabel) {
      json_object_object_add(slice_obj, "id",
                             json_object_new_string(slice->slicelabel));
    }
    if (slice->label.unit) {
      json_object_object_add(slice_obj, "unit",
                             json_object_new_string(slice->label.unit));
    }
    struct json_object *coordinates = json_object_new_object();
    json_object_object_add(coordinates, "i_min",
                           json_object_new_int(slice->ijk_min[0]));
    json_object_object_add(coordinates, "i_max",
                           json_object_new_int(slice->ijk_max[0]));
    json_object_object_add(coordinates, "j_min",
                           json_object_new_int(slice->ijk_min[1]));
    json_object_object_add(coordinates, "j_max",
                           json_object_new_int(slice->ijk_max[1]));
    json_object_object_add(coordinates, "k_min",
                           json_object_new_int(slice->ijk_min[2]));
    json_object_object_add(coordinates, "k_max",
                           json_object_new_int(slice->ijk_max[2]));
    json_object_object_add(slice_obj, "coordinates", coordinates);
    json_object_array_add(slices, slice_obj);
  }
  json_object_object_add(jobj, "slices", slices);

  // Add surfaces to JSON
  struct json_object *surfaces = json_object_new_array();
  for(int i = 0; i < scase->surfcoll.nsurfinfo; i++) {
    surfdata *surf = &scase->surfcoll.surfinfo[i];
    struct json_object *surf_obj = json_object_new_object();
    json_object_object_add(surf_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(surf_obj, "id",
                           json_object_new_string(surf->surfacelabel));
    json_object_array_add(surfaces, surf_obj);
  }
  json_object_object_add(jobj, "surfaces", surfaces);

  // Add materials to JSON
  struct json_object *materials = json_object_new_array();
  for(int i = 0; i < scase->surfcoll.nsurfinfo; i++) {
    surfdata *surf = &scase->surfcoll.surfinfo[i];
    struct json_object *surf_obj = json_object_new_object();
    json_object_object_add(surf_obj, "index", json_object_new_int(i + 1));
    json_object_object_add(surf_obj, "id",
                           json_object_new_string(surf->surfacelabel));
    json_object_array_add(materials, surf_obj);
  }
  json_object_object_add(jobj, "surfaces", materials);

  const char *json_output =
      json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
  printf("%s\n", json_output);
  json_object_put(jobj);
  return 0;
}


int RunBenchmark(char *input_file) {
  initMALLOC();
  InitVars();
  SetGlobalFilenames();

  INIT_PRINT_TIMER(parse_time);
  fprintf(stderr, "reading:\t%s\n", input_file);
  {
    bufferstreamdata *smv_streaminfo = GetSMVBuffer(input_file);
    if(smv_streaminfo == NULL) {
      fprintf(stderr, "could not open %s\n", input_file);
      return 1;
    }
    INIT_PRINT_TIMER(ReadSMV_time);
    int return_code = ReadSMV(smv_streaminfo);
    STOP_TIMER(ReadSMV_time);
    fprintf(stderr, "ReadSMV:\t%8.3f ms\n", ReadSMV_time * 1000);
    if(smv_streaminfo != NULL) {
      FCLOSE(smv_streaminfo);
    }
    if(return_code) return return_code;
  }
  show_timings = 1;
  ReadSMVOrig(&global_scase);
  INIT_PRINT_TIMER(ReadSMVDynamic_time);
  ReadSMVDynamic(&global_scase, input_file);
  STOP_TIMER(ReadSMVDynamic_time);
  fprintf(stderr, "ReadSMVDynamic:\t%8.3f ms\n", ReadSMVDynamic_time * 1000);
  STOP_TIMER(parse_time);
  fprintf(stderr, "Total Time:\t%8.3f ms\n", parse_time * 1000);
  PrintJson(&global_scase);
  FreeVars();
  return 0;
}

int main(int argc, char **argv) {

  bool print_help = false;
  bool print_version = false;

  int c;

  opterr = 0;

  while ((c = getopt(argc, argv, "hV")) != -1)
    switch (c) {
    case 'h':
      print_help = true;
      break;
    case 'V':
      print_version = true;
      break;
    case '?':
      if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      abort();
    }
  if (print_help) {
    printf("smvq-%s\n", PROGVERSION);
    printf("\nUsage:  smvq [OPTIONS] <FILE>\n");
    printf("\nOptions:\n");
    printf("  -h Print help\n");
    printf("  -V Print version\n");
    return 0;
  }
  if (print_version) {
    printf("smvq - smv query processor (v%s)\n", PROGVERSION);
    return 0;
  }
  char *input_file = argv[optind];

  if (input_file == NULL) {
    fprintf(stderr, "No input file specified.\n");
    return 1;
  }
  global_scase.fdsprefix = GetBaseName(input_file);
  int result = RunBenchmark(input_file);
  return result;
}
