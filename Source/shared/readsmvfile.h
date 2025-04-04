#ifndef READSMVFILE_H_DEFINED
#define READSMVFILE_H_DEFINED

#include "options_common.h"
#include "shared_structures.h"

/**
 * @brief A library-global variable that sets options for the parsing process.
 * This uses "extern CCC" as sometimes this file is included as part of a C++
 * compilation unit rather than compiled into a library.
 *
 */
extern CCC parse_options parse_opts;
EXTERNCPP int GetNDevices(char *file);
EXTERNCPP void AddCfastCsvf(smv_case *scase);
EXTERNCPP int ReadSMV_Init(smv_case *scase);
EXTERNCPP int ReadSMV_Parse(smv_case *scase, bufferstreamdata *stream);
EXTERNCPP int CompareSmoketypes(const void *arg1, const void *arg2);
EXTERNCPP int IsDupTexture(smv_case *scase, texturedata *texti);
EXTERNCPP int IsTerrainTexture(smv_case *scase, texturedata *texti);
EXTERNCPP surfdata *GetSurface(smv_case *scase, const char *label);
EXTERNCPP int ParseCHIDProcess(smv_case *scase, bufferstreamdata *stream, int option);
EXTERNCPP float *GetColorPtr(smv_case *scase, float *color);
EXTERNCPP void GetElevAz(float *xyznorm, float *dtheta, float *rotate_axis, float *dpsi);
EXTERNCPP void ReadSMVOrig(smv_case *scase);
EXTERNCPP void ReadSMVDynamic(smv_case *scase, char *file);
EXTERNCPP FILE_SIZE ReadCSVFile(csvfiledata *csvfi, int flag);
EXTERNCPP void ReadHRR(smv_case *scase, int flag);
EXTERNCPP int GetSmoke3DType(smv_case *scase, const char *label);

#endif
