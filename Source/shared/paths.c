#include <string.h>

#include "dmalloc.h"
#include "options_common.h"

#include "file_util.h"
#include "shared_structures.h"

/// @brief Create (and allocate) a path to be used by smokeview. Each of the
/// arguments are optional. All NULL paramaters returns an empty string.
/// @param dirpath  A correctly formatted directory path with no trailing path
/// separator. If NULL, no directory is added.
/// @param basename The name of the file, usually the CHID. Ignored if NULL.
/// @param suffix A suffix to be appended to the file path, usually a constant
/// and usually a constant. Ignored if NULL.
/// @return An allocated path combining each of the components. If all elements
/// are NULL it returns an empty (but still allocated) string. Returns NULL if
/// the path could not be constructed.

/* ------------------ SetupPath ------------------------ */

char *SetupPath(const char *dirpath, const char *basename, const char *suffix){
  char *path = NULL;
  char *filename;
  size_t baselen = basename == NULL ? 0 : strlen(basename);
  size_t suffixlen = suffix == NULL ? 0 : strlen(suffix);
  NEWMEMORY(filename, sizeof(char) * (baselen + suffixlen + 1));
  filename[0] = '\0';
  if(basename != NULL) strcat(filename, basename);
  if(suffix != NULL) strcat(filename, suffix);
  if(dirpath != NULL){
    path = CombinePaths(dirpath, filename);
    FREEMEMORY(filename);
  }
  else {
    path = filename;
  }
  return path;
}

/* ------------------ GetCSVFilename ------------------------ */

char *GetCSVFilename(smv_case *scase, const char *csv_suffix){
  const char *ext = ".csv";
  char *suffix_ext;
  NEWMEMORY(suffix_ext,
            sizeof(char) * (strlen(csv_suffix) + strlen(ext) + 1));
  strcpy(suffix_ext, csv_suffix);
  strcpy(suffix_ext, ext);
  char *p = SetupPath(NULL, scase->fdsprefix, suffix_ext);
  FREEMEMORY(suffix_ext);
  return p;
}


/* ------------------ CasePathFdsFileIn ------------------------ */

// char *fds_filein;
char *CasePathFdsFileIn(smv_case *scase){
  return SetupPath(NULL, scase->fds_filein, NULL);
}


/* ------------------ CasePathChidFileBase ------------------------ */

// char *chidfilebase;
char *CasePathChidFileBase(smv_case *scase){
  return SetupPath(NULL, scase->chidfilebase, NULL);
}


/* ------------------ CasePathHrrCsv ------------------------ */

// char *hrr_csv_filename;
char *CasePathHrrCsv(smv_case *scase){
  return SetupPath(NULL, scase->chidfilebase, "_hrr.csv");
}

/* ------------------ CasePathDevcCsv ------------------------ */

// char *devc_csv_filename;
char *CasePathDevcCsv(smv_case *scase){
  return SetupPath(NULL, scase->chidfilebase, "_devc.csv");
}

/* ------------------ CasePathExpCsv ------------------------ */

// char *exp_csv_filename;
char *CasePathExpCsv(smv_case *scase){
  return SetupPath(NULL, scase->chidfilebase, "_exp.csv");
}

/* ------------------ CasePathStepCsv ------------------------ */

// char *stepcsv_filename;
char *CasePathStepCsv(smv_case *scase){
  return SetupPath(NULL, scase->chidfilebase, "_steps.csv");
}


/* ------------------ CasePathLogFile ------------------------ */

// char *log_filename;
char *CasePathLogFile(smv_case *scase){
  return SetupPath(NULL, scase->chidfilebase, ".smvlog");
}


/* ------------------ CasePathCaseIni ------------------------ */

// char *caseini_filename;
char *CasePathCaseIni(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".ini");
}

/* ------------------ CasePathFed ------------------------ */

// char *fedsmv_filename;
char *CasePathFed(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".fedsmv");
}

/* ------------------ CasePathDeCsv ------------------------ */

// char *dEcsv_filename;
char *CasePathDeCsv(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_dE.csv");
}

/* ------------------ CasePathHtml ------------------------ */

// char *html_filename;
char *CasePathHtml(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".html");
}

/* ------------------ CasePathSmvOrig ------------------------ */

// char *smv_orig_filename;
char *CasePathSmvOrig(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".smo");
}

/* ------------------ CasePathHtmlVr ------------------------ */

// char *htmlvr_filename;
char *CasePathHtmlVr(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_vr.html");
}

/* ------------------ CasePathHtmlObst ------------------------ */

// char *htmlobst_filename;
char *CasePathHtmlObst(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_obst.json");
}

/* ------------------ CasePathHtmlSliceNode ------------------------ */

// char *htmlslicenode_filename;
char *CasePathHtmlSliceNode(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_slicenode.json");
}

/* ------------------ CasePathHtmlSliceCell ------------------------ */

// char *htmlslicecell_filename;
char *CasePathHtmlSliceCell(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_slicecell.json");
}

/* ------------------ CasePathEvent ------------------------ */

// char *event_filename;
char *CasePathEvent(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_events.csv");
}

/* ------------------ CasePathFfmpegCommand ------------------------ */

// char *ffmpeg_command_filename;
char *CasePathFfmpegCommand(smv_case *scase){
#ifdef _WIN32
  return SetupPath(NULL, scase->fdsprefix, "_ffmpeg.bat");
#else
  return SetupPath(NULL, scase->fdsprefix, "_ffmpeg.sh");
#endif
}

/* ------------------ CasePathSmvZip ------------------------ */

// char *smvzip_filename;
char *CasePathSmvZip(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".smvzip");
}

/* ------------------ CasePathSliceInfo ------------------------ */

// char *sliceinfo_filename;
char *CasePathSliceInfo(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".sinfo");
}

/* ------------------ CasePathDeviceInfo ------------------------ */

// char *deviceinfo_filename;
char *CasePathDeviceInfo(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, "_device.info");
}
#ifdef pp_SMOKE3D_FORCE

/* ------------------ CasePathSmoke3d ------------------------ */

// char *smoke3d_filename;
char *CasePathSmoke3d(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".s3d_dummy");
}
#endif

/* ------------------ CasePathIso ------------------------ */

// char *iso_filename;
// if smokezip created part2iso files then concatenate .smv entries found in the
// .isosmv file to the end of the .smv file creating a new .smv file.  Then read
// in that .smv file.
char *CasePathIso(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".isosmv");
}

/* ------------------ CasePathTrainer ------------------------ */

// char *trainer_filename;
char *CasePathTrainer(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".svd");
}

/* ------------------ CasePathTest ------------------------ */

// char *test_filename;
char *CasePathTest(smv_case *scase){
  return SetupPath(NULL, scase->fdsprefix, ".smt");
}
