#ifndef PATHS_H_DEFINED
#define PATHS_H_DEFINED

#include "shared_structures.h"

EXTERNCPP char *GetCSVFilename(smv_case *scase, const char *csv_suffix);

// char *fds_filein;
EXTERNCPP char *CasePathFdsFileIn(smv_case *scase);

// char *chidfilebase;
EXTERNCPP char *CasePathChidFileBase(smv_case *scase);
// char *hrr_csv_filename;
EXTERNCPP char *CasePathHrrCsv(smv_case *scase);
// char *devc_csv_filename;
EXTERNCPP char *CasePathDevcCsv(smv_case *scase);
// char *exp_csv_filename;
EXTERNCPP char *CasePathExpCsv(smv_case *scase);
// char *stepcsv_filename;
EXTERNCPP char *CasePathStepCsv(smv_case *scase);

// char *log_filename;
EXTERNCPP char *CasePathLogFile(smv_case *scase);
// char *caseini_filename;
EXTERNCPP char *CasePathCaseIni(smv_case *scase);
// char *fedsmv_filename;
EXTERNCPP char *CasePathFed(smv_case *scase);
// char *dEcsv_filename;
EXTERNCPP char *CasePathDeCsv(smv_case *scase);
#ifdef pp_HTML
// char *htmlvr_filename;
EXTERNCPP char *CasePathHtmlVr(smv_case *scase);
// char *html_filename;
EXTERNCPP char *CasePathHtml(smv_case *scase);
// char *htmlobst_filename;
EXTERNCPP char *CasePathHtmlObst(smv_case *scase);
// char *htmlslicenode_filename;
EXTERNCPP char *CasePathHtmlSliceNode(smv_case *scase);
// char *htmlslicecell_filename;
EXTERNCPP char *CasePathHtmlSliceCell(smv_case *scase);
// char *smv_orig_filename;
#endif
EXTERNCPP char *CasePathSmvOrig(smv_case *scase);
// char *event_filename;
EXTERNCPP char *CasePathEvent(smv_case *scase);
// char *ffmpeg_command_filename;
EXTERNCPP char *CasePathFfmpegCommand(smv_case *scase);
// char *smvzip_filename;
EXTERNCPP char *CasePathSmvZip(smv_case *scase);
// char *sliceinfo_filename;
EXTERNCPP char *CasePathSliceInfo(smv_case *scase);
// char *deviceinfo_filename;
EXTERNCPP char *CasePathDeviceInfo(smv_case *scase);
#ifdef pp_SMOKE3D_FORCE
// char *smoke3d_filename;
EXTERNCPP char *CasePathSmoke3d(smv_case *scase);
#endif
// char *iso_filename;
EXTERNCPP char *CasePathIso(smv_case *scase);
// char *trainer_filename;
EXTERNCPP char *CasePathTrainer(smv_case *scase);
// char *test_filename;
EXTERNCPP char *CasePathTest(smv_case *scase);
#endif
