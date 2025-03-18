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
int GetNDevices(char *file);
void AddCfastCsvf(smv_case *scase);
int ReadSMV_Init(smv_case *scase);
int ReadSMV_Parse(smv_case *scase, bufferstreamdata *stream);
int CompareSmoketypes(const void *arg1, const void *arg2);
int IsDupTexture(smv_case *scase, texturedata *texti);
int IsTerrainTexture(smv_case *scase, texturedata *texti);
surfdata *GetSurface(smv_case *scase, const char *label);
int ParseCHIDProcess(smv_case *scase, bufferstreamdata *stream, int option);
float *GetColorPtr(smv_case *scase, float *color);
void GetElevAz(float *xyznorm, float *dtheta, float *rotate_axis, float *dpsi);

#endif
