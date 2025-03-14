// _GNU_SOURCE needs to be switched on to access qsort_r. qsort_r is included in
// POSIX 2024 as standard but is also available via GNU extensions.
#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include "options.h"
#include "glew.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "stdio_buffer.h"
#include "glui_motion.h"
#include "glui_bounds.h"
#include "shared_structures.h"
#include "readimage.h"
#include "readhvac.h"
#include "readgeom.h"
#include "readslice.h"
#include "readobject.h"
#include "readlabel.h"
#include "readsmoke.h"
#include "IOobjects.h"
#include "IOscript.h"

#define BREAK break
#define BREAK2 \
      if((stream==stream1&&stream2==NULL)||stream==stream2)break;\
      stream=stream2;\
      continue

#define COLOR_INVISIBLE -2

#define BLOCK_OUTLINE 2

#define DEVICE_DEVICE 0

#define ZVENT_1ROOM 1
#define ZVENT_2ROOM 2

#define PARTBUFFER(len)    scase->part_buffer;    scase->part_buffer    += (len)
#define SMOKE3DBUFFER(len) scase->smoke3d_buffer; scase->smoke3d_buffer += (len)
#define SLICEBUFFER(len)   scase->slice_buffer;   scase->slice_buffer   += (len)

int GetNDevices(char *file);

/* ------------------ GetHrrCsvCol ------------------------ */

int GetHrrCsvCol(smv_case *scase, char *label){
  int i;

  if(label==NULL||strlen(label)==0||scase->hrr_coll.nhrrinfo==0)return -1;
  for(i = 0; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    if(hi->label.shortlabel==NULL)continue;
    if(strcmp(hi->label.shortlabel, label)==0)return i;
  }
  return -1;
}

/* ----------------------- GetTokensBlank ----------------------------- */

int GetTokensBlank(char *buffer, char **tokens, int lenbuffer){

  int nt = 0;
  char *token;

  TrimBack(buffer);
  token = strtok(buffer, " ");
  while(nt<lenbuffer&&token!=NULL){
    tokens[nt++] = token;
    token = strtok(NULL, " ");
  }
  return nt;
}

/// @brief Given a case, if there are any fuels, take take the name and heat of
/// combustion of that fuel and store those values in the output parameters. If
/// there are no defined fuels, scan the ${fdsprefix}.out file to find the first
/// "Heat of Combustion". If neither is found, hoc is set to 0.0 and name to the
/// empty string.
/// @param scase The case
/// @param hoc A pointer in which to store the heat of combustion value
/// @param name A pointer to a pre-allocated buffer in which to store the name of the species
void GetHoc(smv_case *scase, float *hoc, char *name){
  char outfile[256], buffer[255];
  FILE *stream;

  if(scase->fuelcoll.nfuelinfo > 0){
    *hoc = scase->fuelcoll.fuelinfo->hoc;
    strcpy(name, scase->fuelcoll.fuelinfo->fuel);
    return;
  }
  strcpy(outfile, scase->fdsprefix);
  strcat(outfile, ".out");
  stream = fopen(outfile, "r");
  if(stream==NULL){
    *hoc = 0.0;
    strcpy(name, "");
    return;
  }

  while(!feof(stream)){
    fgets(buffer, 255, stream);
    if(strstr(buffer, "Heat of Combustion")!=NULL){
      char *tokens[256], *token;
      int ntokens, ii;
      float val;

      fgets(buffer, 255, stream);
      for(ii = 0;ii < 255;ii++){
        tokens[ii] = NULL;
      }
      ntokens = GetTokensBlank(buffer, tokens, 255);
      token = tokens[ntokens-1];
      sscanf(token, "%f", &val);
      fclose(stream);
      *hoc = val;
      strcpy(name, tokens[0]);
      return;
    }
  }
  fclose(stream);
  *hoc = 0.0;
  strcpy(name, "");
}

/* ------------------ UpdateHoc ------------------------ */

void UpdateHoc(smv_case *scase){
  int i;

// construct column for each MLR column by heat of combustion except for air and products
  for(i = scase->hrr_coll.nhrrinfo-scase->hrr_coll.nhrrhcinfo; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    if(hi->base_col>=0){
      hrrdata *hi_from;
      int j;

      hi_from = scase->hrr_coll.hrrinfo+hi->base_col;
      memcpy(hi->vals, hi_from->vals, hi_from->nvals*sizeof(float));
      hi->nvals = hi_from->nvals;
      for(j = 0; j<hi->nvals; j++){
        hi->vals[j] *= scase->fuel_hoc;
      }
      memcpy(hi->vals_orig, hi->vals, hi->nvals*sizeof(float));

      float valmin, valmax;

      valmin = hi->vals[0];
      valmax = valmin;
      for(j = 1; j<hi->nvals; j++){
        valmin = MIN(valmin, hi->vals[j]);
        valmax = MAX(valmax, hi->vals[j]);
      }
      hi->valmin = valmin;
      hi->valmax = valmax;
    }
  }
}

/* ------------------ IsDimensionless ------------------------ */

int IsDimensionless(char *unit){
// unit is dimensionless if unit is NULL, unit is blank or
// the same characters occur before and after the first slash '/'
  char *slash, *tok1, *tok2;
  char unit2[64];

  if(unit==NULL || strlen(unit)==0)return 1;
  strcpy(unit2, unit);
  slash = strchr(unit2, '/');
  if(slash == NULL)return 0;
  tok1 = unit2;
  tok2 = slash+1;
  slash[0]=0;
  if(strcmp(tok1, tok2)==0)return 1;
  return 0;
}

/* ------------------ ReadCSVFile ------------------------ */

FILE_SIZE ReadCSVFile(csvfiledata *csvfi, int flag){
  FILE *stream;
  int nrows, ncols;
  int nunits, nlabels;
  char *buffer, *buffer_labels, *buffer_units, *buffer_temp;
  char *buffer_dummy;
  char *buffptr;
  char **labels, **units;
  float *vals;
  int *valids;
  int len_buffer;
  int i;

  if(csvfi->csvinfo != NULL){
    for(i = 0; i < csvfi->ncsvinfo; i++){
      csvdata *ci;

      ci = csvfi->csvinfo + i;
      FREEMEMORY(ci->vals);
      FREEMEMORY(ci->vals_orig);
      FREEMEMORY(ci->label.longlabel);
      FREEMEMORY(ci->label.shortlabel);
      FREEMEMORY(ci->label.unit);
    }
    FREEMEMORY(csvfi->csvinfo);
  }
  if(flag == UNLOAD){
    csvfi->defined = CSV_UNDEFINED;
    return 0;
  }

  stream = fopen(csvfi->file, "r");
  if(stream == NULL){
    csvfi->defined = CSV_UNDEFINED;
    return 0;
  }

  len_buffer = GetRowCols(stream, &nrows, &ncols);
  if(nrows==0||ncols==0){
    csvfi->defined = CSV_UNDEFINED;
    fclose(stream);
    return 0;
  }
  len_buffer = MAX(len_buffer + 100 + ncols, 1000);
  csvfi->ncsvinfo = ncols;

  // allocate memory
  NewMemory((void **)&(buffer),        len_buffer);
  NewMemory((void **)&(buffer_labels), len_buffer);
  NewMemory((void **)&(buffer_units),  len_buffer);
  NewMemory((void **)&(buffer_dummy),  len_buffer);
  NewMemory((void **)&(buffer_temp),   len_buffer);

  if(strcmp(csvfi->c_type, "ext") == 0){
    fgets(buffer, len_buffer, stream);
    if(feof(stream)){
      FREEMEMORY(buffer);
      FREEMEMORY(buffer_labels);
      FREEMEMORY(buffer_units);
      csvfi->defined = CSV_UNDEFINED;
      fclose(stream);
      return 0;
    }
    while(strstr(buffer, "//DATA") == NULL){
      fgets(buffer, len_buffer, stream);
      if(feof(stream)){
        FREEMEMORY(buffer);
        FREEMEMORY(buffer_labels);
        FREEMEMORY(buffer_units);
        csvfi->defined = CSV_UNDEFINED;
        fclose(stream);
        return 0;
      }
    }
  }

  int nsize;
  nsize = csvfi->ncsvinfo+1;
  NewMemory((void **)&(csvfi->csvinfo), nsize*sizeof(csvdata));
  NewMemory((void **)&labels,           nsize*sizeof(char *));
  NewMemory((void **)&units,            nsize*sizeof(char *));
  NewMemory((void **)&vals,             nsize*sizeof(float));
  NewMemory((void **)&valids,           nsize*sizeof(int));

  // initialize each column
  for(i=0; i<nsize; i++){
    csvdata *ci;

    ci = csvfi->csvinfo + i;
    ci->nvals = nrows-2;
    if(csvfi->format == CSV_CFAST_FORMAT){
      ci->nvals = nrows-4;
    }
    NewMemory((void **)&ci->vals,      MAX(1, ci->nvals)*sizeof(csvdata));
    NewMemory((void **)&ci->vals_orig, MAX(1, ci->nvals)*sizeof(csvdata));
  }
  CheckMemory;

  // setup labels and units

  if(csvfi->format == CSV_CFAST_FORMAT){
    fgets(buffer_labels,    len_buffer, stream);
    TrimBack(buffer_labels);
    ParseCSV(buffer_labels, buffer_temp, labels,    &nlabels);
    CheckMemory;

    fgets(buffer_dummy,    len_buffer, stream);
    fgets(buffer_dummy,    len_buffer, stream);

    fgets(buffer_units,    len_buffer, stream);
    TrimBack(buffer_units);
    ParseCSV(buffer_units, buffer_temp, units,     &nunits);
  }
  else{
    fgets(buffer_units,    len_buffer, stream);
    TrimBack(buffer_units);
    ParseCSV(buffer_units, buffer_temp, units,     &nunits);

    fgets(buffer_labels,    len_buffer, stream);
    TrimBack(buffer_labels);
    ParseCSV(buffer_labels, buffer_temp, labels,    &nlabels);
    CheckMemory;
  }

  for(i=0; i<csvfi->ncsvinfo; i++){
    csvdata *ci;
    char label[64];
    char *unit;

    ci = csvfi->csvinfo + i;
    TrimBack(labels[i]);
    strcpy(label, labels[i]);
    buffptr = TrimFrontBack(label);
    if(strcmp(buffptr, "null") == 0){
      sprintf(label, "%s%03i", csvfi->c_type, i + 1);
    }

    unit = TrimFrontBack(units[i]);
    ci->skip = 0;
    if(strcmp(unit, "status") == 0)ci->skip = 1;
    ci->dimensionless = IsDimensionless(unit);
    ci->label.longlabel = NULL;
    ci->label.shortlabel = NULL;
    ci->label.unit = NULL;
    SetLabels(&(ci->label), label, label, unit);
  }
  CheckMemory;

  // find Time column index

  csvfi->time = NULL;
  for(i=0; i<csvfi->ncsvinfo; i++){
    csvdata *ci;

    ci = csvfi->csvinfo + i;
    if(strcmp(ci->label.shortlabel, "Time") == 0  || strcmp(ci->label.shortlabel, "time") == 0  || strcmp(ci->label.shortlabel, "Simulation Time") == 0){
      csvfi->time = ci;
      break;
    }
  }

  for(i = 0; i < csvfi->ncsvinfo; i++){
    csvdata *ci;

    ci = csvfi->csvinfo + i;
    ci->valmin = 1.0;
    ci->valmax = 0.0;
  }
  // read in data
  int irow;
  irow = 0;
  while(!feof(stream)){
    if(fgets(buffer, len_buffer, stream) == NULL)break;
    TrimBack(buffer);
    if(strlen(buffer) == 0)break;
    int nvals;
    FParseCSV(buffer, vals, valids, ncols, &nvals);
    if(nvals < ncols)break;
    for(i=0; i<csvfi->ncsvinfo; i++){
      csvdata *ci;

      ci = csvfi->csvinfo + i;
      ci->vals[irow] = 0.0;
      if(valids[i] == 1){
        ci->vals[irow] = vals[i];
        if(ci->valmin > ci->valmax){
          ci->valmin = vals[i];
          ci->valmax = vals[i];
        }
        else{
          ci->valmin = MIN(ci->valmin, vals[i]);
          ci->valmax = MAX(ci->valmax, vals[i]);
        }
      }
      if(irow==0 && valids[i] == 0){
        ci->skip = 1;
      }
    }
    irow++;
    if(irow >= nrows)break;
  }
  CheckMemory;

  // compute -QRAD_I/HRR if columns are present
  csvdata *hrr_csvcol=NULL, *qradi_csvcol=NULL;
  for(i = 0; i<csvfi->ncsvinfo; i++){
    csvdata *ci;

    ci = csvfi->csvinfo+i;
    if(strcmp(ci->label.shortlabel, "HRR")==0)hrr_csvcol = ci;
    if(strcmp(ci->label.shortlabel, "Q_RADI")==0)qradi_csvcol = ci;
  }
  if(hrr_csvcol!=NULL&&qradi_csvcol!=NULL){
    csvdata *cchirad;
    float *vals2;

    cchirad = csvfi->csvinfo+csvfi->ncsvinfo;
    cchirad->dimensionless = 1;
    cchirad->skip = 0;
    vals2 = cchirad->vals;
    for(i=0;i<nrows;i++){
      if(hrr_csvcol->vals[i]!=0.0){
        vals2[i] = -qradi_csvcol->vals[i]/hrr_csvcol->vals[i];
      }
      else{
        vals2[i] = 0.0;
      }
      if(i==0){
        cchirad->valmin = vals2[i];
        cchirad->valmax = vals2[i];
      }
      else{
        cchirad->valmin = MIN(cchirad->valmin, vals2[i]);
        cchirad->valmax = MAX(cchirad->valmax, vals2[i]);
      }
    }
    cchirad->label.longlabel = NULL;
    cchirad->label.shortlabel = NULL;
    cchirad->label.unit = NULL;
    SetLabels(&(cchirad->label), "-QRAD_I/HRR", "-QRAD_I/HRR", "");
    csvfi->ncsvinfo++;
  }

  //copy vals into vals_orig
  for(i=0; i<csvfi->ncsvinfo; i++){
    csvdata *ci;

    ci = csvfi->csvinfo + i;
    memcpy(ci->vals_orig, ci->vals, ci->nvals*sizeof(float));
  }

  CheckMemory;
  FREEMEMORY(units);
  FREEMEMORY(labels);
  FREEMEMORY(vals);
  FREEMEMORY(valids);
  FREEMEMORY(buffer);
  FREEMEMORY(buffer_labels);
  FREEMEMORY(buffer_units);

  fclose(stream);

  FILE_SIZE file_size;
  file_size = GetFileSizeSMV(csvfi->file);
  return file_size;
}

/* ------------------ CompareCSV ------------------------ */

int CompareCSV(const void *arg1, const void *arg2){
  csvfiledata *csvi, *csvj;

  csvi = (csvfiledata *)arg1;
  csvj = (csvfiledata *)arg2;

  return strcmp(csvi->c_type, csvj->c_type);
}

/* ------------------ ReadAllCSVFiles ------------------------ */

FILE_SIZE ReadAllCSVFiles(int flag){
  int i;
  FILE_SIZE file_size=0;

  if(global_scase.csvcoll.ncsvfileinfo == 0)return 0;
#define GENPLOT_REM_ALL_PLOTS       136
  GLUIGenPlotCB(GENPLOT_REM_ALL_PLOTS);
  for(i = 0; i < global_scase.csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = global_scase.csvcoll.csvfileinfo + i;
    ReadCSVFile(csvfi, UNLOAD);
  }
  if(flag == UNLOAD)return 0;
  for(i = 0; i < global_scase.csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = global_scase.csvcoll.csvfileinfo + i;
    if(csvfi->defined == CSV_DEFINING|| csvfi->defined == CSV_DEFINED){
      continue;
    }
    csvfi->defined = CSV_DEFINING;
    file_size += ReadCSVFile(csvfi, flag);
    plot2d_max_columns = MAX(plot2d_max_columns, csvfi->ncsvinfo);
    csvfi->defined = CSV_DEFINED;
    UpdateCSVFileTypes();
  }
  for(i = 0; i < global_scase.csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvfi;

    csvfi = global_scase.csvcoll.csvfileinfo + i;
    if(csvfi->defined != CSV_DEFINED){
      break;
    }
  }
  return file_size;
}

/* ------------------ ReadHRR ------------------------ */

void ReadHRR(smv_case *scase, int flag){
  FILE *stream;
  int nrows, ncols;
  char **labels, **units;
  int nlabels, nunits, nvals;
  float *vals;
  int *valids;
  int i, irow;
  char *buffer, *buffer_labels, *buffer_units, *buffer_temp;
  int len_buffer;

  GetHoc(scase, &scase->fuel_hoc, scase->fuel_name);
  scase->fuel_hoc_default = scase->fuel_hoc;
  if(scase->hrr_coll.nhrrinfo>0){
    for(i=0;i<scase->hrr_coll.nhrrinfo;i++){
      hrrdata *hi;

      hi = scase->hrr_coll.hrrinfo + i;
      FREEMEMORY(hi->vals);
      FREEMEMORY(hi->vals_orig);
      FREEMEMORY(hi->label.longlabel);
      FREEMEMORY(hi->label.shortlabel);
      FREEMEMORY(hi->label.unit);
    }
    FREEMEMORY(scase->hrr_coll.hrrinfo);
    scase->hrr_coll.nhrrinfo = 0;
  }
  scase->time_col  = -1;
  scase->hrr_col   = -1;
  scase->qradi_col = -1;
  if(flag==UNLOAD)return;

  stream = fopen(scase->paths.hrr_csv_filename, "r");
  if(stream==NULL)return;

  len_buffer = GetRowCols(stream, &nrows, &ncols);
  len_buffer = MAX(len_buffer + 100, 1000) + ncols;
  scase->hrr_coll.nhrrinfo = ncols;

  if(scase->hrr_coll.nhrrinfo == 0)return;
  // allocate memory

  NewMemory((void **)&(buffer),        len_buffer);
  NewMemory((void **)&(buffer_labels), len_buffer);
  NewMemory((void **)&(buffer_units),  len_buffer);
  NewMemory((void **)&(buffer_temp),   len_buffer);

  NewMemory((void **)&labels,                     scase->hrr_coll.nhrrinfo*sizeof(char *));
  NewMemory((void **)&units,                      scase->hrr_coll.nhrrinfo*sizeof(char *));
  NewMemory((void **)&scase->hrr_coll.hrrinfo,  2*scase->hrr_coll.nhrrinfo*sizeof(hrrdata));
  NewMemory((void **)&vals,                       scase->hrr_coll.nhrrinfo*sizeof(float));
  NewMemory((void **)&valids,                     scase->hrr_coll.nhrrinfo*sizeof(int));

// initialize each column
  for(i = 0; i<2*scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    NewMemory((void **)&hi->vals,      MAX(1,(nrows+10))*sizeof(float));
    NewMemory((void **)&hi->vals_orig, MAX(1,(nrows+10))*sizeof(float));
    hi->base_col = -1;
    hi->nvals = nrows-2;
  }
  CheckMemory;

// setup labels and units

  fgets(buffer_units, len_buffer, stream);
  ParseCSV(buffer_units, buffer_temp, units, &nunits);

  fgets(buffer_labels, len_buffer, stream);
  ParseCSV(buffer_labels, buffer_temp, labels, &nlabels);
  CheckMemory;

  for(i = 0; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    TrimBack(labels[i]);
    TrimBack(units[i]);
    hi->label.longlabel = NULL;
    hi->label.shortlabel = NULL;
    hi->label.unit = NULL;
    SetLabels(&(hi->label), labels[i], labels[i], units[i]);
  }
  CheckMemory;

// find column index of several quantities

  scase->time_col  = GetHrrCsvCol(&global_scase, "Time");
  if(scase->time_col>=0)scase->timeptr = scase->hrr_coll.hrrinfo+scase->time_col;

  scase->hrr_col   = GetHrrCsvCol(&global_scase, "HRR");
  if(scase->hrr_col>=0&&scase->time_col>=0)scase->hrrptr = scase->hrr_coll.hrrinfo+scase->hrr_col;

  scase->qradi_col = GetHrrCsvCol(&global_scase, "Q_RADI");
  CheckMemory;

// define column for each MLR column by heat of combustion except for air and products
  scase->hrr_coll.nhrrhcinfo = 0;
  for(i = 0; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi, *hi2;

    hi = scase->hrr_coll.hrrinfo+i;
    if(strlen(hi->label.longlabel)>3&&strncmp(hi->label.longlabel,"MLR_",4)==0){
      char label[256];
      int doit = 0;

      doit = 0;
      if(strlen(scase->fuel_name)>0&&strstr(hi->label.longlabel, scase->fuel_name)!=NULL)doit = 1;
      if(doit==0&&strstr(hi->label.longlabel, "FUEL")!=NULL)doit = 1;
      if(doit==0)continue;
      hi2 = scase->hrr_coll.hrrinfo + scase->hrr_coll.nhrrinfo + scase->hrr_coll.nhrrhcinfo;
      hi2->base_col = i;
      strcpy(label, "HOC*");
      strcat(label, hi->label.longlabel);
      hi2->label.longlabel = NULL;
      hi2->label.shortlabel = NULL;
      hi2->label.unit = NULL;
      SetLabels(&(hi2->label), label, label, "kW");
      scase->mlr_col = hi2-scase->hrr_coll.hrrinfo;
      scase->have_mlr = 1;
      scase->hrr_coll.nhrrhcinfo++;
    }
  }
  CheckMemory;

// read in data
  irow = 0;
  while(!feof(stream)){
    if(fgets(buffer, len_buffer, stream)==NULL)break;
    TrimBack(buffer);
    if(strlen(buffer)==0)break;
    FParseCSV(buffer, vals, valids, ncols, &nvals);
    if(nvals<ncols)break;
    for(i = 0; i<scase->hrr_coll.nhrrinfo; i++){
      hrrdata *hi;

      hi = scase->hrr_coll.hrrinfo+i;
      hi->vals[irow] = 0.0;
      if(valids[i]==1)hi->vals[irow] = vals[i];
    }
    irow++;
    if(irow>=nrows)break;
  }
  CheckMemory;

//define column of hrr/qradi
  if(scase->hrr_col>=0&&scase->qradi_col>=0){
    char label[256];
    hrrdata *hi_chirad;


    scase->chirad_col = scase->hrr_coll.nhrrinfo+scase->hrr_coll.nhrrhcinfo;
    hi_chirad = scase->hrr_coll.hrrinfo+scase->chirad_col;

    strcpy(label, "CHIRAD");
    hi_chirad->label.longlabel = NULL;
    hi_chirad->label.shortlabel = NULL;
    hi_chirad->label.unit = NULL;
    SetLabels(&(hi_chirad->label), label, label, "-");
    hi_chirad->nvals = nrows - 2;
    scase->hrr_coll.nhrrhcinfo++;
  }
  CheckMemory;

  scase->hrr_coll.nhrrinfo += scase->hrr_coll.nhrrhcinfo;
// construct column for each MLR column by heat of combustion except for air and products
  for(i = scase->hrr_coll.nhrrinfo- scase->hrr_coll.nhrrhcinfo; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    hi->nvals = nrows-2;
  }
  UpdateHoc(&global_scase);
  CheckMemory;

//construct column of qradi/hrr
  if(scase->hrr_col>=0&&scase->qradi_col>=0){
    hrrdata *hi_chirad, *hi_hrr, *hi_qradi;

    hi_chirad = scase->hrr_coll.hrrinfo+scase->chirad_col;
    hi_hrr = scase->hrr_coll.hrrinfo+scase->hrr_col;
    hi_qradi = scase->hrr_coll.hrrinfo+scase->qradi_col;
    hi_chirad->nvals = MIN(hi_qradi->nvals, hi_hrr->nvals);
    for(i=0;i<hi_chirad->nvals;i++){
      if(hi_hrr->vals[i]!=0.0){
        hi_chirad->vals[i] = -hi_qradi->vals[i]/hi_hrr->vals[i];
      }
      else{
        hi_chirad->vals[i] = 0.0;
      }
    }
  }
  CheckMemory;

//compute vals into vals_orig
  for(i = 0; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    memcpy(hi->vals_orig, hi->vals, hi->nvals*sizeof(float));
  }

//compute min and max of each column
  for(i = 0; i<scase->hrr_coll.nhrrinfo; i++){
    hrrdata *hi;
    float valmin, valmax;
    int j;

    hi = scase->hrr_coll.hrrinfo+i;
    hi->nvals = irow;
    valmin = hi->vals[0];
    valmax = valmin;
    for(j = 1; j<hi->nvals; j++){
      valmin = MIN(valmin, hi->vals[j]);
      valmax = MAX(valmax, hi->vals[j]);
    }
    hi->valmin = valmin;
    hi->valmax = valmax;
  }
  CheckMemory;

// free arrays that were not used
  for(i = scase->hrr_coll.nhrrinfo; i<2*(scase->hrr_coll.nhrrinfo-scase->hrr_coll.nhrrhcinfo); i++){
    hrrdata *hi;

    hi = scase->hrr_coll.hrrinfo+i;
    FREEMEMORY(hi->vals);
    FREEMEMORY(hi->vals_orig);
  }
  CheckMemory;
  FREEMEMORY(buffer);
  FREEMEMORY(buffer_labels);
  FREEMEMORY(buffer_units);

  FREEMEMORY(units);
  FREEMEMORY(labels);
  FREEMEMORY(vals);
  FREEMEMORY(valids);
  fclose(stream);
}

/* ------------------ InitProp ------------------------ */

void InitProp(propdata *propi, int nsmokeview_ids, char *label){
  int nlabel;

  nlabel = strlen(label);
  if(nlabel == 0){
    NewMemory((void **)&propi->label, 5);
    strcpy(propi->label, "null");
  }
  else{
    NewMemory((void **)&propi->label, nlabel + 1);
    strcpy(propi->label, label);
  }

  NewMemory((void **)&propi->smokeview_ids, nsmokeview_ids*sizeof(char *));
  NewMemory((void **)&propi->smv_objects, nsmokeview_ids*sizeof(sv_object *));

  propi->nsmokeview_ids = nsmokeview_ids;
  propi->blockvis = 1;
  propi->inblockage = 0;
  propi->ntextures = 0;
  propi->nvars_dep = 0;
  propi->nvars_indep = 0;
  propi->vars_indep = NULL;
  propi->svals = NULL;
  propi->texturefiles = NULL;
  propi->rotate_axis = NULL;
  propi->rotate_angle = 0.0;
  propi->fvals = NULL;
  propi->vars_indep_index = NULL;
  propi->tag_number = 0;
}

/* ------------------ InitDefaultProp ------------------------ */

void InitDefaultProp(smv_case *scase){
/*
PROP
 Human_props
  4
 human_fixed
 human_altered_with_data
 ellipsoid
 disk
  1
 D=0.2
 */
  propdata *propi;
  char proplabel[255];
  int lenbuf;
  int ntextures_local;
  int nsmokeview_ids;
  char *smokeview_id;
  char buffer[255];
  int i;

  propi = scase->propcoll.propinfo + scase->propcoll.npropinfo;

  strcpy(proplabel,"Human_props(default)");           // from input

  nsmokeview_ids=4;                                   // from input

  InitProp(propi,nsmokeview_ids,proplabel);
  for(i=0;i<nsmokeview_ids;i++){
    if(i==0)strcpy(buffer,"human_fixed");             // from input
    if(i==1)strcpy(buffer,"human_altered_with_data"); // from input
    if(i==2)strcpy(buffer,"ellipsoid");               // from input
    if(i==3)strcpy(buffer,"disk");                    // from input
    lenbuf=strlen(buffer);
    NewMemory((void **)&smokeview_id,lenbuf+1);
    strcpy(smokeview_id,buffer);
    propi->smokeview_ids[i]=smokeview_id;
    propi->smv_objects[i]=GetSmvObjectType(&scase->objectscoll, propi->smokeview_ids[i],scase->objectscoll.std_object_defs.missing_device);
  }
  propi->smv_object=propi->smv_objects[0];
  propi->smokeview_id=propi->smokeview_ids[0];

  propi->nvars_indep=1;                               // from input
  propi->vars_indep=NULL;
  propi->svals=NULL;
  propi->texturefiles=NULL;
  ntextures_local=0;
  if(propi->nvars_indep>0){
    NewMemory((void **)&propi->vars_indep,propi->nvars_indep*sizeof(char *));
    NewMemory((void **)&propi->svals,propi->nvars_indep*sizeof(char *));
    NewMemory((void **)&propi->fvals,propi->nvars_indep*sizeof(float));
    NewMemory((void **)&propi->vars_indep_index,propi->nvars_indep*sizeof(int));
    NewMemory((void **)&propi->texturefiles,propi->nvars_indep*sizeof(char *));

    for(i=0;i<propi->nvars_indep;i++){
      char *equal;

      propi->svals[i]=NULL;
      propi->vars_indep[i]=NULL;
      propi->fvals[i]=0.0;
      if(i==0)strcpy(buffer,"D=0.2");                 // from input
      equal=strchr(buffer,'=');
      if(equal!=NULL){
        char *buf1, *buf2, *keyword, *val;
        int lenkey, lenval;
        char *texturefile;

        buf1=buffer;
        buf2=equal+1;
        *equal=0;

        TrimBack(buf1);
        keyword=TrimFront(buf1);
        lenkey=strlen(keyword);

        TrimBack(buf2);
        val=TrimFront(buf2);
        lenval=strlen(val);

        if(lenkey==0||lenval==0)continue;

        if(val[0]=='"'){
          val[0]=' ';
          if(val[lenval-1]=='"')val[lenval-1]=' ';
          TrimBack(val);
          val=TrimFront(val);
          NewMemory((void **)&propi->svals[i],lenval+1);
          strcpy(propi->svals[i],val);
          texturefile=strstr(val,"t%");
          if(texturefile!=NULL){
            texturefile+=2;
            texturefile=TrimFront(texturefile);
            propi->texturefiles[ntextures_local]=propi->svals[i];
            strcpy(propi->svals[i],texturefile);

            ntextures_local++;
          }
        }

        NewMemory((void **)&propi->vars_indep[i],lenkey+1);
        strcpy(propi->vars_indep[i],keyword);
        sscanf(val,"%f",propi->fvals+i);
      }
    }
    GetIndepVarIndices(propi->smv_object,propi->vars_indep,propi->nvars_indep,propi->vars_indep_index);
  }
  propi->ntextures=ntextures_local;
}

/* ------------------ UpdateINIList ------------------------ */

void UpdateINIList(void){
  char filter[256];
  int i;

  strcpy(filter,global_scase.fdsprefix);
  strcat(filter,"*.ini");
  FreeFileList(global_scase.filelist_coll.ini_filelist,&global_scase.filelist_coll.nini_filelist);
  global_scase.filelist_coll.nini_filelist=GetFileListSize(".",filter, FILE_MODE);
  if(global_scase.filelist_coll.nini_filelist>0){
    MakeFileList(".",filter,global_scase.filelist_coll.nini_filelist,NO,&global_scase.filelist_coll.ini_filelist, FILE_MODE);

    for(i=0;i<global_scase.filelist_coll.nini_filelist;i++){
      filelistdata *filei;

      filei = global_scase.filelist_coll.ini_filelist + i;
      if(filei->type!=0)continue;
      InsertIniFile(filei->file);
    }
  }
}

/* ------------------ FreeLabels ------------------------ */

void FreeLabels(flowlabels *flowlabel){
  FREEMEMORY(flowlabel->longlabel);
  FREEMEMORY(flowlabel->shortlabel);
  FREEMEMORY(flowlabel->unit);
}

/* ------------------ InitMesh ------------------------ */

void InitMesh(meshdata *meshi){
  int i;

  meshi->use = 1;
  meshi->isliceinfo    = 0;
  meshi->nsliceinfo    = 0;
  for(i = 0;i < 6;i++){
    meshi->skip_nabors[i] = NULL;
    meshi->nabors[i]      = NULL;
  }
  NewMemory((void **)&meshi->plot3dcontour1, sizeof(contour));
  memset(meshi->plot3dcontour1, 0, sizeof(contour));
  NewMemory((void **)&meshi->plot3dcontour2, sizeof(contour));
  memset(meshi->plot3dcontour2, 0, sizeof(contour));
  NewMemory((void **)&meshi->plot3dcontour3, sizeof(contour));
  memset(meshi->plot3dcontour3, 0, sizeof(contour));
  NewMemory((void **)&meshi->currentsurf, sizeof(isosurface));
  memset(meshi->currentsurf, 0, sizeof(isosurface));
  NewMemory((void **)&meshi->currentsurf2, sizeof(isosurface));
  memset(meshi->currentsurf2, 0, sizeof(isosurface));
  NewMemory((void **)&meshi->box_clipinfo, sizeof(clipdata));
  memset(meshi->box_clipinfo, 0, sizeof(clipdata));
  NewMemory((void **)&meshi->gsliceinfo, sizeof(meshplanedata));
  memset(meshi->gsliceinfo, 0, sizeof(meshplanedata));
  NewMemory((void **)&meshi->volrenderinfo, sizeof(volrenderdata));
  memset(meshi->volrenderinfo, 0, sizeof(volrenderdata));
  for(i=0; i<6; i++){
    meshi->bc_faces[i]   = NULL;
    meshi->n_bc_faces[i] = 0;
  }

  meshi->is_firenode = NULL;
  meshi->is_firenodeptr = NULL;
  meshi->terrain = NULL;
  meshi->boundary_mask = NULL;
  meshi->in_frustum = 1;
  meshi->imap = NULL;
  meshi->jmap = NULL;
  meshi->kmap = NULL;
  meshi->n_imap = 0;
  meshi->n_jmap = 0;
  meshi->n_kmap = 0;
  meshi->znodes_complete = NULL;
  meshi->nznodes = 0;
  meshi->floor_mesh = meshi;
  meshi->is_bottom = 0;
  meshi->is_block_terrain = NULL;
  meshi->smoke3d_soot = NULL;
  meshi->smoke3d_hrrpuv = NULL;
  meshi->smoke3d_temp = NULL;
  meshi->smoke3d_co2 = NULL;
  meshi->smokeplaneinfo = NULL;
  meshi->nsmokeplaneinfo = 0;
  meshi->opacity_adjustments = NULL;
  for(i = 0; i<6; i++){
    meshi->is_extface[i] = 1;
  }
  meshi->ncutcells = 0;
  meshi->cutcells = NULL;
  meshi->slice_min[0] = 1.0;
  meshi->slice_min[1] = 1.0;
  meshi->slice_min[2] = 1.0;
  meshi->slice_max[0] = 0.0;
  meshi->slice_max[1] = 0.0;
  meshi->slice_max[2] = 0.0;
  meshi->s_offset[0] = -1;
  meshi->s_offset[1] = -1;
  meshi->s_offset[2] = -1;
  meshi->super = NULL;
  meshi->update_smoke3dcolors = 0;
  meshi->iplotx_all = NULL;
  meshi->iploty_all = NULL;
  meshi->iplotz_all = NULL;
#ifdef pp_GPU

  meshi->volsmoke_texture_buffer = NULL;
  meshi->volsmoke_texture_id = 0;
  meshi->voltest_update = 0;

  meshi->volfire_texture_buffer = NULL;
  meshi->volfire_texture_id = 0;

#ifdef pp_WINGPU
  meshi->slice3d_texture_buffer = NULL;
#endif
  meshi->slice3d_texture_id = 0;
  meshi->slice3d_c_buffer = NULL;
#endif
  meshi->mesh_offset_ptr = NULL;
#ifdef pp_CULL_GEOM
  meshi->cullgeominfo = NULL;
#endif
  meshi->blockvis = 1;
  meshi->datavis = 1;
  // set meshi->terrain to NULL just after meshinfo is allocated
  meshi->meshrgb[0] = 0.0;
  meshi->meshrgb[1] = 0.0;
  meshi->meshrgb[2] = 0.0;
  meshi->meshrgb_ptr = NULL;
  meshi->cellsize = 0.0;
  meshi->smokedir = 1;
  meshi->merge_alpha = NULL;
  meshi->merge_color = NULL;
  meshi->smokecolor_ptr = NULL;
  meshi->smokealpha_ptr = NULL;
  meshi->dxDdx  = 1.0;
  meshi->dyDdx  = 1.0;
  meshi->dzDdx  = 1.0;
  meshi->dxyDdx = 1.0;
  meshi->dxzDdx = 1.0;
  meshi->dyzDdx = 1.0;
  meshi->label = NULL;
  meshi->maxtimes_boundary = 0;
  meshi->slicedir = YDIR;
  meshi->visInteriorBoundaries = 0;
  meshi->plot3dfilenum = -1;
  meshi->patchfilenum = -1;
  meshi->obst_bysize = NULL;
  meshi->iqdata = NULL;
  meshi->qdata = NULL;
  meshi->yzcolorbase = NULL;
  meshi->xzcolorbase = NULL;
  meshi->xycolorbase = NULL;
  meshi->yzcolorfbase = NULL;
  meshi->xzcolorfbase = NULL;
  meshi->xycolorfbase = NULL;
  meshi->yzcolortbase = NULL;
  meshi->xzcolortbase = NULL;
  meshi->xycolortbase = NULL;
  meshi->dx_xy = NULL;
  meshi->dy_xy = NULL;
  meshi->dz_xy = NULL;
  meshi->dx_xz = NULL;
  meshi->dy_xz = NULL;
  meshi->dz_xz = NULL;
  meshi->dx_yz = NULL;
  meshi->dy_yz = NULL;
  meshi->dz_yz = NULL;
  meshi->c_iblank_xy = NULL;
  meshi->c_iblank_xz = NULL;
  meshi->c_iblank_yz = NULL;
  meshi->iblank_smoke3d = NULL;
  meshi->iblank_smoke3d_defined = 0;
  meshi->animatedsurfaces = NULL;
  meshi->blockagesurface = NULL;
  meshi->blockagesurfaces = NULL;
  meshi->showlevels = NULL;
  meshi->isolevels = NULL;
  meshi->nisolevels = 0;
  meshi->iso_times = NULL;
  meshi->iso_times_map = NULL;
  meshi->iso_timeslist = NULL;
  meshi->isofilenum = -1;
  meshi->nvents = 0;
  meshi->ndummyvents = 0;
  meshi->ncvents = 0;
  meshi->offset[XXX] = 0.0;
  meshi->offset[YYY] = 0.0;
  meshi->offset[ZZZ] = 0.0;
  meshi->ptype = NULL;
  meshi->zipoffset = NULL, meshi->zipsize = NULL;
  meshi->xyzpatch_offset = NULL;
  meshi->xyzpatch_no_offset = NULL;
  meshi->xyzpatch_threshold = NULL;
  meshi->patchventcolors = NULL;
  meshi->cpatchval = NULL;
  meshi->cpatchval_iframe = NULL;
  meshi->cpatchval_iframe_zlib = NULL;
  meshi->cpatchval_zlib = NULL;
  meshi->patch_times = NULL;
  meshi->patch_times_map = NULL;
  meshi->patchval = NULL;
#ifndef pp_BOUNDFRAME
  meshi->patchval_iframe = NULL;
#endif
  meshi->thresholdtime = NULL;
  meshi->patchblank = NULL;
  meshi->patch_timeslist = NULL;
  meshi->blockageinfoptrs = NULL;
  meshi->blockageinfo     = NULL;

  meshi->faceinfo = NULL;
  meshi->face_normals_single = NULL;
  meshi->face_normals_double = NULL;
  meshi->face_transparent_double = NULL;
  meshi->face_textures = NULL;
  meshi->face_outlines = NULL;

  meshi->xplt = NULL;
  meshi->yplt = NULL;
  meshi->zplt = NULL;
  meshi->xvolplt = NULL;
  meshi->yvolplt = NULL;
  meshi->zvolplt = NULL;
  meshi->xplt_cen = NULL;
  meshi->yplt_cen = NULL;
  meshi->zplt_cen = NULL;
  meshi->xplt_orig = NULL;
  meshi->yplt_orig = NULL;
  meshi->zplt_orig = NULL;

  meshi->f_iblank_cell = NULL;
  meshi->c_iblank_cell = NULL;
  meshi->c_iblank_x = NULL;
  meshi->c_iblank_y = NULL;
  meshi->c_iblank_z = NULL;
  meshi->c_iblank_node = NULL;
  meshi->c_iblank_embed = NULL;
  meshi->block_zdist = NULL;

  meshi->f_iblank_cell0 = NULL;
  meshi->c_iblank_cell0 = NULL;
  meshi->c_iblank_x0 = NULL;
  meshi->c_iblank_y0 = NULL;
  meshi->c_iblank_z0 = NULL;
  meshi->c_iblank_node0 = NULL;
  meshi->c_iblank_node_html = NULL;
  meshi->c_iblank_embed0 = NULL;
  meshi->block_zdist0 = NULL;

  meshi->xyz_bar[XXX] = 1.0;
  meshi->xyz_bar0[XXX] = 0.0;
  meshi->xyz_bar[YYY] = 1.0;
  meshi->xyz_bar0[YYY] = 0.0;
  meshi->xyz_bar[ZZZ] = 1.0;
  meshi->xyz_bar0[ZZZ] = 0.0;

  meshi->xyzmaxdiff = 1.0;

  meshi->xcen = 0.5;
  meshi->ycen = 0.5;
  meshi->zcen = 0.5;

  meshi->plotx = 1;
  meshi->ploty = 1;
  meshi->plotz = 1;

  meshi->boxoffset = 0.0;
  meshi->ventinfo = NULL;
  meshi->cventinfo = NULL;
  meshi->select_min = 0;
  meshi->select_max = 0;
}

/* ------------------ GetCloseVent ------------------------ */

ventdata *GetCloseVent(meshdata *ventmesh, int ivent){
  ventdata *close_vent, *vdummy_start;
  int i;

  close_vent = ventmesh->ventinfo+ivent;
  if(close_vent->dir2==XDIR&&close_vent->imin>0&&close_vent->imax<ventmesh->ibar)return close_vent;
  if(close_vent->dir2==YDIR&&close_vent->jmin>0&&close_vent->jmax<ventmesh->jbar)return close_vent;
  if(close_vent->dir2==ZDIR&&close_vent->kmin>0&&close_vent->kmax<ventmesh->kbar)return close_vent;
  vdummy_start = ventmesh->ventinfo+ventmesh->nvents-ventmesh->ndummyvents;
  for(i = 0;i<ventmesh->ndummyvents;i++){
    ventdata *vi;

    vi = vdummy_start+i;
    if(close_vent->imin==vi->imin&&close_vent->imax==vi->imax&&
       close_vent->jmin==vi->jmin&&close_vent->jmax==vi->jmax&&
       close_vent->kmin==vi->kmin&&close_vent->kmax==vi->kmax
      ){
      return vi;
    }
  }
  return close_vent;
}

/* ------------------ UpdateSMVDynamic ------------------------ */

/// @brief Re-read an *.smv file to read any updates.
/// @param file The path to the *.smv file.
void UpdateSMVDynamic(char *file){
  INIT_PRINT_TIMER(smv_timer1);
  ReadSMVDynamic(&global_scase, file);
  PRINT_TIMER(smv_timer1, "ReadSMVDynamic");
  INIT_PRINT_TIMER(smv_timer2);
  UpdatePlot3dMenuLabels();
  InitPlot3dTimeList();
  UpdateTimes();
  GetGlobalPlot3DBounds();
  PRINT_TIMER(smv_timer2, "UpdateSMVDynamic wrapup");
}
/* ------------------ ReadSMVDynamic ------------------------ */

void ReadSMVDynamic(smv_case *scase, char *file){
  int ioffset;
  float time_local;
  int i;
  int nn_plot3d=0,iplot3d=0;
  int do_pass2=0, do_pass3=0, minmaxpl3d=0;
  int nplot3dinfo_old;
  bufferstreamdata streaminfo, *stream=&streaminfo;

  stream->fileinfo = fopen_buffer(file,"r", 1, 0);

  nplot3dinfo_old=scase->nplot3dinfo;

  updatefacelists=1;
  updatemenu=1;
  if(scase->nplot3dinfo>0){
    int n;

    for(i=0;i<scase->nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = scase->plot3dinfo + i;
      for(n=0;n<6;n++){
        FreeLabels(&plot3di->label[n]);
      }
      FREEMEMORY(plot3di->reg_file);
    }
//    FREEMEMORY(scase->plot3dinfo);
  }
  scase->nplot3dinfo=0;

  for(i=0;i<scase->meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi=scase->meshescoll.meshinfo+i;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc_local;

      bc_local=meshi->blockageinfoptrs[j];
      bc_local->nshowtime=0;
      FREEMEMORY(bc_local->showtime);
      FREEMEMORY(bc_local->showhide);
    }
    for(j=0;j<meshi->nvents;j++){
      ventdata *vi;

      vi = meshi->ventinfo + j;
      vi->nshowtime=0;
      FREEMEMORY(vi->showhide);
      FREEMEMORY(vi->showtime);
    }
    for(j = 0; j<meshi->ncvents; j++){
      cventdata *cvi;

      cvi = meshi->cventinfo + j;
      cvi->nshowtime = 0;
      FREEMEMORY(cvi->showhide);
      FREEMEMORY(cvi->showtime);
    }
  }
  for(i=scase->devicecoll.ndeviceinfo_exp;i<scase->devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei =scase->devicecoll.deviceinfo + i;
    devicei->nstate_changes=0;
    FREEMEMORY(devicei->act_times);
    FREEMEMORY(devicei->state_values);
  }

  ioffset=0;

  // ------------------------------- pass 1 dynamic - start ------------------------------------

  for(;;){
    char buffer[255],buffer2[255];

    if(FGETS(buffer,255,stream)==NULL)break;
    if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OFFSET") == 1){
      ioffset++;
      continue;
    }
/*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PL3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"PL3D") == 1){
      int n;

      do_pass2=1;
      if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)continue;
      FGETS(buffer, 255, stream);
      for(n = 0; n<5; n++){
        if(ReadLabels(NULL, stream, NULL)==LABEL_ERR)break;
      }
      scase->nplot3dinfo++;
      continue;
    }
/*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OPEN_VENT ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OPEN_VENT") == 1||
       MatchSMV(buffer,"CLOSE_VENT")==1||
       MatchSMV(buffer, "OPEN_CVENT") == 1 ||
       MatchSMV(buffer, "CLOSE_CVENT") == 1){
      meshdata *meshi;
      int len;
      int showvent, blocknumber, tempval, isvent;

      do_pass2=1;
      showvent=1;
      isvent = 0;
      if(MatchSMV(buffer, "CLOSE_VENT") == 1 ||
         MatchSMV(buffer, "OPEN_VENT") == 1)isvent = 1;
      if(MatchSMV(buffer,"CLOSE_VENT") == 1 ||
         MatchSMV(buffer, "CLOSE_CVENT") == 1)showvent = 0;
      if(scase->meshescoll.nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      TrimBack(buffer);
      len=strlen(buffer);
      if(showvent==1){
        if(len>10){
          sscanf(buffer+10,"%i",&blocknumber);
          blocknumber--;
          if(blocknumber<0)blocknumber=0;
          if(blocknumber>scase->meshescoll.nmeshes-1)blocknumber=scase->meshescoll.nmeshes-1;
        }
      }
      else{
        if(len>11){
          sscanf(buffer+11,"%i",&blocknumber);
          blocknumber--;
          if(blocknumber<0)blocknumber=0;
          if(blocknumber>scase->meshescoll.nmeshes-1)blocknumber=scase->meshescoll.nmeshes-1;
        }
      }
      meshi=scase->meshescoll.meshinfo + blocknumber;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f",&tempval,&time_local);
      tempval--;
      if(tempval<0)continue;
      if(isvent == 1){
        if(meshi->ventinfo == NULL || tempval >= meshi->nvents)continue;
      }
      else{
        if(meshi->cventinfo == NULL || tempval >= meshi->ncvents)continue;
      }
      if(isvent == 1){
        ventdata *vi;

        vi = GetCloseVent(meshi, tempval);
        vi->nshowtime++;
      }
      else{
        cventdata *cvi;

        cvi = meshi->cventinfo + tempval;
        cvi->showtimelist = NULL;
        cvi->nshowtime++;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SHOW_OBST ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"SHOW_OBST") == 1||
       MatchSMV(buffer,"HIDE_OBST")==1){
      meshdata *meshi;
      int blocknumber,blocktemp,tempval;
      blockagedata *bc;

      do_pass2=1;
      if(scase->meshescoll.nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      if(strlen(buffer)>10){
        sscanf(buffer,"%s %i",buffer2,&blocktemp);
        if(blocktemp>0&&blocktemp<=scase->meshescoll.nmeshes)blocknumber = blocktemp-1;
      }
      meshi=scase->meshescoll.meshinfo + blocknumber;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f",&tempval,&time_local);
      tempval--;
      if(tempval<0||tempval>=meshi->nbptrs)continue;
      bc=meshi->blockageinfoptrs[tempval];
      bc->nshowtime++;
      scase->have_animate_blockages = 1;
      scase->have_removable_obsts = 1;
      continue;
    }

    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ DUCT_ACT +++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    //DUCT_ACT
    // duct_label (char)
    // time state  (float int)
    if(MatchSMV(buffer, "DUCT_ACT") == 1){
      char *ductname;
      hvacductdata *ducti;
      float *act_time, *act_times;
      int *act_state, *act_states, dummy;

      ductname = strchr(buffer, ' ');
      if(ductname == NULL)continue;
      ductname = TrimFrontBack(ductname + 1);
      ducti = GetHVACDuctID(&scase->hvaccoll, ductname);
      if(ducti == NULL)continue;

      act_times  = ducti->act_times;
      act_states = ducti->act_states;
      ducti->nact_times++;
      if(ducti->nact_times == 1){
        NewMemory((void **)&act_times,  ducti->nact_times * sizeof(float));
        NewMemory((void **)&act_states, ducti->nact_times * sizeof(int));
      }
      else{
        ResizeMemory((void **)&act_times,  ducti->nact_times * sizeof(float));
        ResizeMemory((void **)&act_states, ducti->nact_times * sizeof(int));
      }

      act_time  = act_times + ducti->nact_times - 1;
      act_state = act_states + ducti->nact_times -1;

      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &dummy, act_time, act_state);
      ONEORZERO(*act_state);
      ducti->act_times  = act_times;
      ducti->act_states = act_states;
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ DEVICE_ACT +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"DEVICE_ACT") == 1){
      devicedata *devicei;
      int idevice;
      float act_time;
      int act_state;

      do_pass2=1;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f %i",&idevice,&act_time,&act_state);
      idevice--;
      if(idevice>=0&&idevice<scase->devicecoll.ndeviceinfo){
        devicei = scase->devicecoll.deviceinfo + idevice;
        devicei->act_time=act_time;
        devicei->nstate_changes++;
      }

      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ MINMAXPL3D +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"MINMAXPL3D") == 1){
      minmaxpl3d=1;
      do_pass2=1;
      continue;
    }
  }

  // ------------------------------- pass 1 dynamic - end ------------------------------------

  if(scase->nplot3dinfo>0){
    if(scase->plot3dinfo==NULL){
      NewMemory((void **)&scase->plot3dinfo,scase->nplot3dinfo*sizeof(plot3ddata));
    }
    else{
      ResizeMemory((void **)&scase->plot3dinfo,scase->nplot3dinfo*sizeof(plot3ddata));
    }
  }
  for(i=0;i<scase->devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei = scase->devicecoll.deviceinfo + i;
    devicei->istate_changes=0;
  }

  ioffset=0;
  REWIND(stream);

  // ------------------------------- pass 2 dynamic - start ------------------------------------

  while(do_pass2==1){
    char buffer[255],buffer2[255];

    if(FGETS(buffer,255,stream)==NULL)break;
    if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OFFSET") == 1){
      ioffset++;
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PL3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"PL3D") == 1){
      plot3ddata *plot3di;
      int len,blocknumber,blocktemp;
      char *bufferptr;

      if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)continue;
      if(minmaxpl3d==1)do_pass3=1;
      nn_plot3d++;
      TrimBack(buffer);
      if(scase->meshescoll.nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      if(strlen(buffer)>5){
        sscanf(buffer,"%s %f %i",buffer2,&time_local,&blocktemp);
        if(blocktemp>0&&blocktemp<=scase->meshescoll.nmeshes)blocknumber = blocktemp-1;
      }
      else{
        time_local=-1.0;
      }
      if(FGETS(buffer,255,stream)==NULL){
        scase->nplot3dinfo--;
        break;
      }
      bufferptr=TrimFrontBack(buffer);
      len=strlen(bufferptr);

      plot3di=scase->plot3dinfo+iplot3d;
      for(i = 0; i < 5; i++){
        plot3di->valmin_plot3d[i] = 1.0;
        plot3di->valmax_plot3d[i] = 0.0;
      }
      plot3di->blocknumber = blocknumber;
      plot3di->seq_id=nn_plot3d;
      plot3di->autoload=0;
      plot3di->time=time_local;
      plot3di->finalize = 1;
      plot3di->hist_update = 0;
      scase->nmemory_ids++;
      plot3di->memory_id = scase->nmemory_ids;

      for(i=0;i<MAXPLOT3DVARS;i++){
        plot3di->histograms[i] = NULL;
      }

      if(plot3di>scase->plot3dinfo+nplot3dinfo_old-1){
        plot3di->loaded=0;
        plot3di->display=0;
      }

      NewMemory((void **)&plot3di->reg_file,(unsigned int)(len+1));
      STRCPY(plot3di->reg_file,bufferptr);

      NewMemory((void **)&plot3di->bound_file, (unsigned int)(len+4+1));
      STRCPY(plot3di->bound_file, bufferptr);
      STRCAT(plot3di->bound_file, ".bnd");
      plot3di->have_bound_file = NO;

      plot3di->file=plot3di->reg_file;

      if(parse_opts.fast_startup==1||FileExistsCaseDir(scase, plot3di->file)==YES){
        int n;
        int read_ok = YES;

        plot3di->u = -1;
        plot3di->v = -1;
        plot3di->w = -1;
        for(n = 0;n<5;n++){
          if(ReadLabels(&plot3di->label[n], stream, NULL)!=LABEL_OK){
            read_ok=NO;
            break;
          }
          if(STRCMP(plot3di->label[n].shortlabel, "U-VEL")==0){
            plot3di->u = n;
          }
          if(STRCMP(plot3di->label[n].shortlabel, "V-VEL")==0){
            plot3di->v = n;
          }
          if(STRCMP(plot3di->label[n].shortlabel, "W-VEL")==0){
            plot3di->w = n;
          }
        }
        if(read_ok==NO){
          scase->nplot3dinfo--;
          continue;
        }
        if(plot3di->u>-1||plot3di->v>-1||plot3di->w>-1){
          plot3di->nplot3dvars = MAXPLOT3DVARS;
        }
        else{
          plot3di->nplot3dvars = 5;
        }
        if(NewMemory((void **)&plot3di->label[5].longlabel, 6)==0)return;
        if(NewMemory((void **)&plot3di->label[5].shortlabel, 6)==0)return;
        if(NewMemory((void **)&plot3di->label[5].unit, 4)==0)return;

        STRCPY(plot3di->label[5].longlabel, "Speed");
        STRCPY(plot3di->label[5].shortlabel, "Speed");
        STRCPY(plot3di->label[5].unit, "m/s");

        STRCPY(plot3di->longlabel, "");
        for(n = 0;n<5;n++){
          STRCAT(plot3di->longlabel, plot3di->label[n].shortlabel);
          if(n!=4)STRCAT(plot3di->longlabel, ", ");
        }

        iplot3d++;
      }
      else{
        int n;

        for(n = 0;n<5;n++){
          if(ReadLabels(&plot3di->label[n], stream, NULL)==LABEL_ERR)break;
        }
        scase->nplot3dinfo--;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OPEN_VENT ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OPEN_VENT") == 1||
       MatchSMV(buffer,"CLOSE_VENT")==1||
       MatchSMV(buffer, "OPEN_CVENT") == 1 ||
       MatchSMV(buffer, "CLOSE_CVENT") == 1){
      meshdata *meshi;
      int len,showvent,blocknumber,tempval,isvent;

      showvent=1;
      isvent = 0;
      if(MatchSMV(buffer, "CLOSE_VENT") == 1 ||
         MatchSMV(buffer, "OPEN_VENT") == 1)isvent = 1;
      if(MatchSMV(buffer,"CLOSE_VENT") == 1||
         MatchSMV(buffer, "CLOSE_CVENT") == 1)showvent=0;
      if(scase->meshescoll.nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      TrimBack(buffer);
      len=strlen(buffer);
      if(showvent==1){
        if(len>10){
          sscanf(buffer+10,"%i",&blocknumber);
          blocknumber--;
          if(blocknumber<0)blocknumber=0;
          if(blocknumber>scase->meshescoll.nmeshes-1)blocknumber=scase->meshescoll.nmeshes-1;
        }
      }
      else{
        if(len>11){
          sscanf(buffer+11,"%i",&blocknumber);
          blocknumber--;
          if(blocknumber<0)blocknumber=0;
          if(blocknumber>scase->meshescoll.nmeshes-1)blocknumber=scase->meshescoll.nmeshes-1;
        }
      }
      meshi=scase->meshescoll.meshinfo + blocknumber;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f",&tempval,&time_local);
      tempval--;
      if(isvent == 1){
        ventdata *vi;

        if(meshi->ventinfo == NULL || tempval < 0 || tempval >= meshi->nvents)continue;
        vi = GetCloseVent(meshi, tempval);
        if(vi->showtime == NULL){
          NewMemory((void **)&vi->showtime, (vi->nshowtime + 1) * sizeof(float));
          NewMemory((void **)&vi->showhide, (vi->nshowtime + 1) * sizeof(unsigned char));
          vi->nshowtime = 1;
          vi->showtime[0] = 0.0;
          vi->showhide[0] = 1;
        }
        if(showvent == 1){
          vi->showhide[vi->nshowtime] = 1;
        }
        else{
          vi->showhide[vi->nshowtime] = 0;
        }
        vi->showtime[vi->nshowtime++] = time_local;
      }
      else{
        cventdata *cvi;

        if(meshi->cventinfo == NULL || tempval < 0 || tempval >= meshi->ncvents)continue;
        cvi = meshi->cventinfo + tempval;
        if(cvi->showtime == NULL){
          NewMemory((void **)&cvi->showtime, (cvi->nshowtime + 1) * sizeof(float));
          NewMemory((void **)&cvi->showhide, (cvi->nshowtime + 1) * sizeof(unsigned char));
          cvi->nshowtime = 1;
          cvi->showtime[0] = 0.0;
          cvi->showhide[0] = 1;
        }
        if(showvent == 1){
          cvi->showhide[cvi->nshowtime] = 1;
        }
        else{
          cvi->showhide[cvi->nshowtime] = 0;
        }
        cvi->showtime[cvi->nshowtime++] = time_local;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SHOW_OBST ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"SHOW_OBST") == 1||
       MatchSMV(buffer,"HIDE_OBST")==1){
      meshdata *meshi;
      int blocknumber,tempval,showobst,blocktemp;
      blockagedata *bc;

      if(scase->meshescoll.nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      if(strlen(buffer)>10){
        sscanf(buffer,"%s %i",buffer2,&blocktemp);
        if(blocktemp>0&&blocktemp<=scase->meshescoll.nmeshes)blocknumber = blocktemp-1;
      }
      showobst=0;
      if(MatchSMV(buffer,"SHOW_OBST") == 1)showobst=1;
      meshi=scase->meshescoll.meshinfo + blocknumber;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f",&tempval,&time_local);
      tempval--;
      if(tempval<0||tempval>=meshi->nbptrs)continue;
      bc=meshi->blockageinfoptrs[tempval];

      if(bc->showtime==NULL){
        if(time_local!=0.0)bc->nshowtime++;
        NewMemory((void **)&bc->showtime,bc->nshowtime*sizeof(float));
        NewMemory((void **)&bc->showhide,bc->nshowtime*sizeof(unsigned char));
        bc->nshowtime=0;
        if(time_local!=0.0){
          bc->nshowtime=1;
          bc->showtime[0]=0.0;
          if(showobst==1){
            bc->showhide[0]=0;
          }
          else{
            bc->showhide[0]=1;
          }
        }
      }
      bc->nshowtime++;
      if(showobst==1){
        bc->showhide[bc->nshowtime-1]=1;
      }
      else{
        bc->showhide[bc->nshowtime-1]=0;
      }
      bc->showtime[bc->nshowtime-1]=time_local;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ DEVICE_ACT ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"DEVICE_ACT") == 1){
      devicedata *devicei;
      int idevice;
      float act_time;
      int act_state=1;

      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f %i",&idevice,&act_time,&act_state);
      idevice--;
      if(idevice>=0&&idevice<scase->devicecoll.ndeviceinfo){
        int istate;

        devicei = scase->devicecoll.deviceinfo + idevice;
        devicei->act_time=act_time;
        if(devicei->act_times==NULL){
          devicei->nstate_changes++;
          NewMemory((void **)&devicei->act_times,devicei->nstate_changes*sizeof(int));
          NewMemory((void **)&devicei->state_values,devicei->nstate_changes*sizeof(int));
          devicei->act_times[0]=0.0;
          devicei->state_values[0]=devicei->state0;
          devicei->istate_changes++;
        }
        istate = devicei->istate_changes++;
        devicei->act_times[istate]=act_time;
        devicei->state_values[istate]=act_state;
      }
      continue;
    }

  }

  // ------------------------------- pass 2 dynamic - end ------------------------------------

  REWIND(stream);

  // ------------------------------- pass 3 dynamic - start ------------------------------------

  while(do_pass3==1){
    char buffer[255];

    if(FGETS(buffer,255,stream)==NULL)break;
    if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ MINMAXPL3D +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"MINMAXPL3D") == 1){
      char *file_ptr, file2[1024];
      float valmin[5], valmax[5];
      float percentile_min[5], percentile_max[5];

      FGETS(buffer,255,stream);
      strcpy(file2,buffer);
      TrimBack(file2);
      file_ptr = TrimFront(file2);

      for(i=0;i<5;i++){
        FGETS(buffer,255,stream);
        sscanf(buffer,"%f %f %f %f",valmin +i,valmax+i, percentile_min+i,percentile_max+i);
      }

      for(i=0;i<scase->nplot3dinfo;i++){
        plot3ddata *plot3di;

        plot3di = scase->plot3dinfo + i;
        if(strcmp(file_ptr,plot3di->file)==0){
          int j;

          for(j=0;j<5;j++){
            plot3di->diff_valmin[j]=percentile_min[j];
            plot3di->diff_valmax[j]=percentile_max[j];
          }
          break;
        }
      }
      continue;
    }
  }
  FCLOSE(stream);
}


/* ------------------ GetLabels ------------------------ */

void GetLabels(char *buffer, char **label1, char **label2){
  char *tok0, *tok1, *tok2;

  tok0 = NULL;
  tok1 = NULL;
  tok2 = NULL;
  tok0 = strtok(buffer, "%");
  if(tok0 != NULL)tok1 = strtok(NULL, "%");
  if(tok1 != NULL)tok2 = strtok(NULL, "%");
  if(tok1 != NULL){
    TrimBack(tok1);
    tok1 = TrimFront(tok1);
    if(strlen(tok1) == 0)tok1 = NULL;
  }
  if(tok2 != NULL){
    TrimBack(tok2);
    tok2 = TrimFront(tok2);
    if(strlen(tok2) == 0)tok2 = NULL;
  }
  if(label2 != NULL){
    *label2 = tok2;
  }
  if(label1 != NULL)*label1 = tok1;
}

/// @brief Given a case, find the first instance of propdata that has the given
/// prop_id.
/// @param scase The case
/// @param prop_id The id to search for
/// @return A pointer to the first instance of propdata with the matching id.
/// Returns NULL if there are no matching props.
propdata *GetPropID(smv_case *scase, char *prop_id){
  int i;

  if(scase->propcoll.propinfo == NULL || prop_id == NULL || strlen(prop_id) == 0)return NULL;
  for(i = 0; i < scase->propcoll.npropinfo; i++){
    propdata *propi;

    propi = scase->propcoll.propinfo + i;

    if(strcmp(propi->label, prop_id) == 0)return propi;
  }
  return NULL;
}

/* ----------------------- InitDevice ----------------------------- */

void InitDevice(smv_case *scase, devicedata *devicei, float *xyz, int is_beam, float *xyz1, float *xyz2, float *xyzn, int state0, int nparams, float *params, char *labelptr){
  float norm;
  int i;

  devicei->show        = 1;
  devicei->selected    = 0;
  devicei->nvals       = 0;
  devicei->filetype    = -1;
  devicei->in_zone_csv = 0;
  devicei->in_devc_csv = 0;
  devicei->labelptr    = devicei->deviceID;
  devicei->color       = NULL;
  devicei->line_width  = 1.0;
  devicei->have_xyz    = 0;
  devicei->have_xyz1   = 0;
  devicei->have_xyz2   = 0;
  if(labelptr != NULL){
    strcpy(devicei->deviceID, labelptr);
  }
  if(STRCMP(devicei->object->label, "plane") == 0){
    float color[4];

    NewMemory((void **)&devicei->plane_surface, scase->meshescoll.nmeshes * sizeof(isosurface *));
    for(i = 0; i < scase->meshescoll.nmeshes; i++){
      NewMemory((void **)&devicei->plane_surface[i], sizeof(isosurface));
    }
    if(nparams >= 3){
      color[0] = params[0];
      color[1] = params[1];
      color[2] = params[2];
      color[3] = 1.0;
      devicei->color = GetColorPtr(&global_scase, color);
    }
    if(nparams >= 4){
      devicei->line_width = params[3];
    }
  }
  else{
    devicei->plane_surface = NULL;
  }
  if(xyz != NULL){
    devicei->xyz[0]   = xyz[0];
    devicei->xyz[1]   = xyz[1];
    devicei->xyz[2]   = xyz[2];
    devicei->have_xyz = 1;
  }
  if(xyz1 != NULL){
    devicei->xyz1[0]   = xyz1[0];
    devicei->xyz1[1]   = xyz1[1];
    devicei->xyz1[2]   = xyz1[2];
    devicei->have_xyz1 = 1;
  }
  if(xyz2 != NULL){
    devicei->xyz2[0]   = xyz2[0];
    devicei->xyz2[1]   = xyz2[1];
    devicei->xyz2[2]   = xyz2[2];
    devicei->have_xyz2 = 1;
  }
  if(xyz1!=NULL&&xyz2!=NULL)scase->have_object_box = 1;
  if(is_beam == 1)scase->have_beam = 1;
  devicei->is_beam = is_beam;
  norm = sqrt(xyzn[0] * xyzn[0] + xyzn[1] * xyzn[1] + xyzn[2] * xyzn[2]);
  if(norm != 0.0){
    devicei->xyznorm[0] = xyzn[0] / norm;
    devicei->xyznorm[1] = xyzn[1] / norm;
    devicei->xyznorm[2] = xyzn[2] / norm;
  }
  else{
    devicei->xyznorm[0] = 0.0;
    devicei->xyznorm[1] = 0.0;
    devicei->xyznorm[2] = 1.0;
  }
  devicei->times          = NULL;
  devicei->vals           = NULL;
  devicei->vals_orig      = NULL;
  devicei->update_avg     = 0;
  devicei->nstate_changes = 0;
  devicei->istate_changes = 0;
  devicei->act_times      = NULL;
  devicei->state_values   = NULL;
  devicei->showstatelist  = NULL;
  devicei->act_time       = -1.0;
  devicei->device_mesh    = NULL;
  devicei->state0         = state0;
  devicei->nparams        = nparams;
  devicei->params         = params;
  devicei->ival           = 0;
  if(nparams > 0 && params != NULL){
    for(i = 0; i < nparams; i++){
      devicei->params[i] = params[i];
    }
  }
}

/* ------------------ ParseDevicekeyword ------------------------ */

void ParseDevicekeyword(smv_case *scase, BFILE *stream, devicedata *devicei){
  float xyz[3]={0.0,0.0,0.0}, xyzn[3]={0.0,0.0,0.0};
  float xyz1[3] = { 0.0,0.0,0.0 }, xyz2[3] = { 0.0,0.0,0.0 };
  int state0=0;
  int nparams=0, nparams_textures=0;
  char *labelptr, *prop_id;
  char buffer[255],*buffer3;
  int i;
  char *tok1, *tok2, *tok3, *tok4;
  int is_beam=0;

  devicei->type=DEVICE_DEVICE;
  FGETS(buffer,255,stream);
  TrimCommas(buffer);

  tok1=strtok(buffer,"%");
  tok1=TrimFrontBack(tok1);

  tok2=strtok(NULL,"%");
  tok2=TrimFrontBack(tok2);

  tok3=strtok(NULL,"%");
  tok3=TrimFrontBack(tok3);

  tok4=strtok(NULL,"%");
  tok4=TrimFrontBack(tok4);

  strcpy(devicei->quantity,"");
  if(tok2!=NULL){
    strcpy(devicei->quantity,tok2);
  }

  if(tok4==NULL){
    strcpy(devicei->csvlabel,tok1);
  }
  else{
    strcpy(devicei->csvlabel,tok4);
  }
  if(strlen(tok1)>=4&&strncmp(tok1, "null",4)==0){
    strcpy(devicei->deviceID, "null");
  }
  else{
    strcpy(devicei->deviceID, tok1);
  }
  devicei->object = GetSmvObjectType(&scase->objectscoll,  tok1,scase->objectscoll.std_object_defs.missing_device);
  if(devicei->object==scase->objectscoll.std_object_defs.missing_device&&tok3!=NULL){
    devicei->object = GetSmvObjectType(&scase->objectscoll,  tok3,scase->objectscoll.std_object_defs.missing_device);
  }
  if(devicei->object == scase->objectscoll.std_object_defs.missing_device)scase->have_missing_objects = 1;
  devicei->params=NULL;
  devicei->times=NULL;
  devicei->vals=NULL;
  devicei->vals_orig = NULL;
  devicei->update_avg = 0;
  devicei->target_index = -1;
  devicei->global_valmin = 1.0;
  devicei->global_valmax = 0.0;
  FGETS(buffer,255,stream);
  TrimCommas(buffer);

  sscanf(buffer,"%f %f %f %f %f %f %i %i %i",
    xyz,xyz+1,xyz+2,xyzn,xyzn+1,xyzn+2,&state0,&nparams,&nparams_textures);

  labelptr = strchr(buffer, '#'); // read in coordinates of beam detector
  if(labelptr != NULL){
    sscanf(labelptr + 1, "%f %f %f %f %f %f", xyz1, xyz1 + 1, xyz1 + 2, xyz2, xyz2 + 1, xyz2 + 2);
    if(strcmp(devicei->quantity, "PATH OBSCURATION") == 0 ||
      strcmp(devicei->quantity, "TRANSMISSION") == 0){
      is_beam = 1;
    }
  }
  devicei->is_beam = is_beam;

  GetLabels(buffer,&prop_id,NULL);
  devicei->prop=GetPropID(&global_scase, prop_id);
  if(prop_id!=NULL&&devicei->prop!=NULL&&devicei->prop->smv_object!=NULL){
    devicei->object=devicei->prop->smv_object;
  }
  else{
    NewMemory((void **)&devicei->prop,sizeof(propdata));
    InitProp(devicei->prop,1,devicei->deviceID);
    devicei->prop->smv_object=devicei->object;
    devicei->prop->smv_objects[0]=devicei->prop->smv_object;
  }
  if(nparams_textures<0)nparams_textures=0;
  if(nparams_textures>1)nparams_textures=1;
  devicei->ntextures=nparams_textures;
  if(nparams_textures>0){
     NewMemory((void **)&devicei->textureinfo,sizeof(texturedata));
  }
  else{
    devicei->textureinfo=NULL;
    devicei->texturefile=NULL;
  }

  labelptr=strchr(buffer,'%');
  if(labelptr!=NULL){
    TrimBack(labelptr);
    if(strlen(labelptr)>1){
      labelptr++;
      labelptr=TrimFront(labelptr);
      if(strlen(labelptr)==0)labelptr=NULL;
    }
    else{
      labelptr=NULL;
    }
  }

  if(nparams<=0){
    InitDevice(scase, devicei,xyz,is_beam,xyz1,xyz2,xyzn,state0,0,NULL,labelptr);
  }
  else{
    float *params,*pc;
    int nsize;

    nsize = 6*((nparams-1)/6+1);
    NewMemory((void **)&params,(nsize+devicei->ntextures)*sizeof(float));
    pc=params;
    for(i=0;i<nsize/6;i++){
      FGETS(buffer,255,stream);
      TrimCommas(buffer);

      sscanf(buffer,"%f %f %f %f %f %f",pc,pc+1,pc+2,pc+3,pc+4,pc+5);
      pc+=6;
    }
    InitDevice(scase, devicei,xyz,is_beam,xyz1,xyz2,xyzn,state0,nparams,params,labelptr);
  }
  GetElevAz(devicei->xyznorm,&devicei->dtheta,devicei->rotate_axis,NULL);
  if(nparams_textures>0){
    FGETS(buffer,255,stream);
    TrimCommas(buffer);
    TrimBack(buffer);
    buffer3=TrimFront(buffer);
    NewMemory((void **)&devicei->texturefile,strlen(buffer3)+1);
    strcpy(devicei->texturefile,buffer3);
  }
}

/* ------------------ ParseDevicekeyword2 ------------------------ */

void ParseDevicekeyword2(smv_case *scase, FILE *stream, devicedata *devicei){
  float xyz[3] = {0.0,0.0,0.0}, xyzn[3] = {0.0,0.0,0.0};
  float xyz1[3] = {0.0,0.0,0.0}, xyz2[3] = {0.0,0.0,0.0};
  int state0 = 0;
  int nparams = 0, nparams_textures = 0;
  char *labelptr, *prop_id;
  char buffer[255], *buffer3;
  int i;
  char *tok1, *tok2, *tok3;
  int is_beam = 0;

  devicei->type = DEVICE_DEVICE;
  fgets(buffer, 255, stream);
  TrimCommas(buffer);

  tok1 = strtok(buffer, "%");
  tok1 = TrimFrontBack(tok1);

  tok2 = strtok(NULL, "%");
  tok2 = TrimFrontBack(tok2);

  tok3 = strtok(NULL, "%");
  tok3 = TrimFrontBack(tok3);

  strcpy(devicei->quantity, "");
  if(tok2!=NULL){
    strcpy(devicei->quantity, tok2);
  }

  if(strlen(tok1)>=4&&strncmp(tok1, "null", 4)==0){
    strcpy(devicei->deviceID, "null");
  }
  else{
    strcpy(devicei->deviceID, tok1);
  }
  devicei->object = GetSmvObjectType(&scase->objectscoll,  tok1, scase->objectscoll.std_object_defs.missing_device);
  if(devicei->object==scase->objectscoll.std_object_defs.missing_device&&tok3!=NULL){
    devicei->object = GetSmvObjectType(&scase->objectscoll,  tok3, scase->objectscoll.std_object_defs.missing_device);
  }
  if(devicei->object==scase->objectscoll.std_object_defs.missing_device)scase->have_missing_objects = 1;
  devicei->params = NULL;
  devicei->times = NULL;
  devicei->vals = NULL;
  devicei->vals_orig = NULL;
  devicei->update_avg = 0;
  fgets(buffer, 255, stream);
  TrimCommas(buffer);

  sscanf(buffer, "%f %f %f %f %f %f %i %i %i",
    xyz, xyz+1, xyz+2, xyzn, xyzn+1, xyzn+2, &state0, &nparams, &nparams_textures);

  labelptr = strchr(buffer, '#'); // read in coordinates of a possible beam detector
  if(labelptr!=NULL){
    sscanf(labelptr+1, "%f %f %f %f %f %f", xyz1, xyz1+1, xyz1+2, xyz2, xyz2+1, xyz2+2);
    if(strcmp(devicei->quantity, "PATH OBSCURATION") == 0||
       strcmp(devicei->quantity, "TRANSMISSION") == 0){
      is_beam = 1;
    }
  }
  devicei->is_beam = is_beam;

  GetLabels(buffer, &prop_id, NULL);
  devicei->prop = GetPropID(&global_scase, prop_id);
  if(prop_id!=NULL&&devicei->prop!=NULL&&devicei->prop->smv_object!=NULL){
    devicei->object = devicei->prop->smv_object;
  }
  else{
    NewMemory((void **)&devicei->prop, sizeof(propdata));
    InitProp(devicei->prop, 1, devicei->deviceID);
    devicei->prop->smv_object = devicei->object;
    devicei->prop->smv_objects[0] = devicei->prop->smv_object;
  }
  if(nparams_textures<0)nparams_textures = 0;
  if(nparams_textures>1)nparams_textures = 1;
  devicei->ntextures = nparams_textures;
  if(nparams_textures>0){
    NewMemory((void **)&devicei->textureinfo, sizeof(texturedata));
  }
  else{
    devicei->textureinfo = NULL;
    devicei->texturefile = NULL;
  }

  labelptr = strchr(buffer, '%');
  if(labelptr!=NULL){
    TrimBack(labelptr);
    if(strlen(labelptr)>1){
      labelptr++;
      labelptr = TrimFront(labelptr);
      if(strlen(labelptr)==0)labelptr = NULL;
    }
    else{
      labelptr = NULL;
    }
  }

  if(nparams<=0){
    InitDevice(scase, devicei, xyz, is_beam, xyz1, xyz2, xyzn, state0, 0, NULL, labelptr);
  }
  else{
    float *params, *pc;
    int nsize;

    nsize = 6*((nparams-1)/6+1);
    NewMemory((void **)&params, (nsize+devicei->ntextures)*sizeof(float));
    pc = params;
    for(i = 0;i<nsize/6;i++){
      fgets(buffer, 255, stream);
      TrimCommas(buffer);

      sscanf(buffer, "%f %f %f %f %f %f", pc, pc+1, pc+2, pc+3, pc+4, pc+5);
      pc += 6;
    }
    InitDevice(scase, devicei, xyz, is_beam, xyz1, xyz2, xyzn, state0, nparams, params, labelptr);
  }
  GetElevAz(devicei->xyznorm, &devicei->dtheta, devicei->rotate_axis, NULL);
  if(nparams_textures>0){
    fgets(buffer, 255, stream);
    TrimCommas(buffer);
    TrimBack(buffer);
    buffer3 = TrimFront(buffer);
    NewMemory((void **)&devicei->texturefile, strlen(buffer3)+1);
    strcpy(devicei->texturefile, buffer3);
  }
}

bufferstreamdata *CopySMVBuffer(bufferstreamdata *stream_in);

/* ------------------ GetInpf ------------------------ */

int GetInpf(smv_case *scase, bufferstreamdata *stream_in){
  char buffer[255], *bufferptr;
  bufferstreamdata *stream;
  int len;

  if(stream_in==NULL)return 1;
  stream = CopySMVBuffer(stream_in);
  if(stream==NULL)return 1;
  for(;;){
    if(FEOF(stream)!=0){
      BREAK;
    }
    if(FGETS(buffer, 255,stream)==NULL){
      BREAK;
    }
    if(strncmp(buffer," ",1)==0)continue;
    if(MatchSMV(buffer,"INPF") == 1){
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      bufferptr=TrimFrontBack(buffer);

      len=strlen(bufferptr);
      FREEMEMORY(scase->paths.fds_filein);
      if(NewMemory((void **)&scase->paths.fds_filein,(unsigned int)(len+1))==0)return 2;
      STRCPY(scase->paths.fds_filein,bufferptr);
      if(FileExistsCaseDir(scase, scase->paths.fds_filein)==NO){
        FreeMemory(scase->paths.fds_filein);
      }

      if(scase->paths.chidfilebase==NULL){
        char *chidptr=NULL;
        char buffer_chid[1024];

        if(scase->paths.fds_filein!=NULL)chidptr=GetChid(scase->paths.fds_filein,buffer_chid);
        if(chidptr!=NULL){
          NewMemory((void **)&scase->paths.chidfilebase,(unsigned int)(strlen(chidptr)+1));
          STRCPY(scase->paths.chidfilebase,chidptr);
        }
      }
      if(scase->paths.chidfilebase!=NULL){
        NewMemory((void **)&scase->paths.hrr_csv_filename,(unsigned int)(strlen(scase->paths.chidfilebase)+8+1));
        STRCPY(scase->paths.hrr_csv_filename,scase->paths.chidfilebase);
        STRCAT(scase->paths.hrr_csv_filename,"_hrr.csv");
        if(FileExistsCaseDir(scase, scase->paths.hrr_csv_filename)==NO){
          FREEMEMORY(scase->paths.hrr_csv_filename);
        }

        NewMemory((void **)&scase->paths.devc_csv_filename,(unsigned int)(strlen(scase->paths.chidfilebase)+9+1));
        STRCPY(scase->paths.devc_csv_filename,scase->paths.chidfilebase);
        STRCAT(scase->paths.devc_csv_filename,"_devc.csv");
        if(FileExistsCaseDir(scase, scase->paths.devc_csv_filename)==NO){
          FREEMEMORY(scase->paths.devc_csv_filename);
        }

        NewMemory((void **)&scase->paths.exp_csv_filename,(unsigned int)(strlen(scase->paths.chidfilebase)+8+1));
        STRCPY(scase->paths.exp_csv_filename,scase->paths.chidfilebase);
        STRCAT(scase->paths.exp_csv_filename,"_exp.csv");
        if(FileExistsCaseDir(scase, scase->paths.exp_csv_filename)==NO){
          FREEMEMORY(scase->paths.exp_csv_filename);
        }
      }
      break;
    }
  }
  return 0;
}

/// @brief Given a pointer to texturedata, determine if there is another
/// *loaded* texturedata in the case that has the same filename. If there is a
/// duplicate, set the name and loaded status of texti to that duplicate.
/// @param[inout] scase The case
/// @param[inout] texti A pointer to the texture data
/// @return 1 if there is a duplicate, 0 if there is no duplicate.
int IsDupTexture(smv_case *scase, texturedata *texti) {
  int dup_texture;
  int i, j;

  i = texti - scase->texture_coll.textureinfo;
  dup_texture=0;
  for(j=0;j<i;j++){
    texturedata *textj;

    textj = scase->texture_coll.textureinfo + j;
    if(textj->loaded==0)continue;
    if(strcmp(texti->file,textj->file)==0){
      texti->name=textj->name;
      texti->loaded=1;
      dup_texture=1;
    }
  }
  return dup_texture;
}

/// @brief Given a pointer to texturedata, determine if this texture is a terrain texture
/// @param[in] scase The case
/// @param[in] texti A pointer to the texture data
/// @return 1 if it is a terrain texture, 0 otherwise.
int IsTerrainTexture(smv_case *scase, texturedata *texti){
  int is_terrain_texture;
  int i;

  is_terrain_texture=0;
  for(i=0;i<scase->terrain_texture_coll.nterrain_textures;i++){
    texturedata *tt;

    tt = scase->terrain_texture_coll.terrain_textures + i;
    if(tt->file==NULL||strcmp(tt->file, texti->file)!=0)continue;
    return 1;
  }
  return is_terrain_texture;
}

/* ------------------ InitTextures0 ------------------------ */

void InitTextures0(void){
  // get texture filename from SURF and device info
  int i;

  INIT_PRINT_TIMER(texture_timer);
  global_scase.texture_coll.ntextureinfo = 0;
  for(i=0;i<global_scase.surfcoll.nsurfinfo;i++){
    surfdata *surfi;
    texturedata *texti;
    int len;

    surfi = global_scase.surfcoll.surfinfo + i;
    if(surfi->texturefile==NULL)continue;
    texti = global_scase.texture_coll.textureinfo + global_scase.texture_coll.ntextureinfo;
    len = strlen(surfi->texturefile);
    NewMemory((void **)&texti->file,(len+1)*sizeof(char));
    strcpy(texti->file,surfi->texturefile);
    texti->loaded=0;
    texti->used=0;
    texti->display=0;
    global_scase.texture_coll.ntextureinfo++;
    surfi->textureinfo=global_scase.texture_coll.textureinfo+global_scase.texture_coll.ntextureinfo-1;
  }
  PRINT_TIMER(texture_timer, "SURF textures");

  for(i=0;i<global_scase.device_texture_list_coll.ndevice_texture_list;i++){
    char *texturefile;
    texturedata *texti;
    int len;

    texturefile = global_scase.device_texture_list_coll.device_texture_list[i];
    texti = global_scase.texture_coll.textureinfo + global_scase.texture_coll.ntextureinfo;
    len = strlen(texturefile);
    NewMemory((void **)&texti->file,(len+1)*sizeof(char));
    global_scase.device_texture_list_coll.device_texture_list_index[i]=global_scase.texture_coll.ntextureinfo;
    strcpy(texti->file,texturefile);
    texti->loaded=0;
    texti->used=0;
    texti->display=0;
    global_scase.texture_coll.ntextureinfo++;
  }
  PRINT_TIMER(texture_timer, "device textures");

  if(global_scase.terrain_texture_coll.nterrain_textures>0){
    texturedata *texture_base;

    texture_base = global_scase.texture_coll.textureinfo + global_scase.texture_coll.ntextureinfo;
    for(i=0;i<global_scase.terrain_texture_coll.nterrain_textures;i++){
      char *texturefile;
      texturedata *texti;
      int len;

      texturefile = global_scase.terrain_texture_coll.terrain_textures[i].file;
      texti = global_scase.texture_coll.textureinfo + global_scase.texture_coll.ntextureinfo;
      len = strlen(texturefile);
      NewMemory((void **)&texti->file,(len+1)*sizeof(char));
      strcpy(texti->file,texturefile);
      texti->loaded=0;
      texti->used=0;
      texti->display=0;
      global_scase.texture_coll.ntextureinfo++;
    }
    FREEMEMORY(global_scase.terrain_texture_coll.terrain_textures);
    global_scase.terrain_texture_coll.terrain_textures = texture_base;
  }
  PRINT_TIMER(texture_timer, "terrain textures");

  // check to see if texture files exist .
  // If so, then convert to OpenGL format

  for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
    unsigned char *floortex;
    int texwid, texht;
    texturedata *texti;
    char *filename;
    int max_texture_size;
    int is_transparent;

    texti = global_scase.texture_coll.textureinfo + i;
    texti->loaded=0;
    if(texti->file==NULL||IsDupTexture(&global_scase, texti)==1||IsTerrainTexture(&global_scase, texti)==1)continue;

    CheckMemory;
    filename=strrchr(texti->file,*dirseparator);
    if(filename!=NULL){
      filename++;
    }
    else{
      filename=texti->file;
    }
    glGenTextures(1,&texti->name);
    glBindTexture(GL_TEXTURE_2D,texti->name);
    if(verbose_output==1)printf("  reading in texture image: %s",texti->file);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

    floortex=ReadPicture(global_scase.texturedir, texti->file,&texwid,&texht,&is_transparent,0);
    texti->is_transparent = is_transparent;
    if(floortex==NULL){
      PRINTF("\n***Error: Texture %s failed to load\n", filename);
      continue;
    }
    if(verbose_output==1)printf(" - complete\n");
    if(texwid>max_texture_size||texht>max_texture_size){
      printf("***error: image size: %i x %i, is larger than the maximum allowed texture size %i x %i\n", texwid, texht, max_texture_size, max_texture_size);
    }
    if(verbose_output==1)printf("  installing texture: %s",texti->file);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, texwid, texht, 0, GL_RGBA, GL_UNSIGNED_BYTE, floortex);
    SNIFF_ERRORS("after glTexImage2D");
    if(verbose_output==1)printf(" - complete\n");
    glGenerateMipmap(GL_TEXTURE_2D);
    SNIFF_ERRORS("after glGenerateMipmap");
    FREEMEMORY(floortex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    texti->loaded=1;
  }

  CheckMemory;
  if(global_scase.texture_coll.ntextureinfo==0){
    FREEMEMORY(global_scase.texture_coll.textureinfo);
  }

  // define colorbar textures

 // glActiveTexture(GL_TEXTURE0);
  glGenTextures(1,&texture_colorbar_id);
  glBindTexture(GL_TEXTURE_1D,texture_colorbar_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  if(gpuactive==1){
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  }
  else{
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  }
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_full);

  glGenTextures(1, &terrain_colorbar_id);
  glBindTexture(GL_TEXTURE_1D, terrain_colorbar_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, rgb_terrain2);

  glGenTextures(1,&texture_slice_colorbar_id);
  glBindTexture(GL_TEXTURE_1D,texture_slice_colorbar_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_slice);

  glGenTextures(1,&texture_patch_colorbar_id);
  glBindTexture(GL_TEXTURE_1D,texture_patch_colorbar_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_patch);

  glGenTextures(1,&texture_plot3d_colorbar_id);
  glBindTexture(GL_TEXTURE_1D,texture_plot3d_colorbar_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_plot3d);

  glGenTextures(1,&texture_iso_colorbar_id);
  glBindTexture(GL_TEXTURE_1D,texture_iso_colorbar_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_iso);

  glGenTextures(1,&volsmoke_colormap_id);
  glBindTexture(GL_TEXTURE_1D,volsmoke_colormap_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,MAXSMOKERGB,0,GL_RGBA,GL_FLOAT,rgb_volsmokecolormap);

  glGenTextures(1,&slicesmoke_colormap_id);
  glBindTexture(GL_TEXTURE_1D,slicesmoke_colormap_id);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,MAXSMOKERGB,0,GL_RGBA,GL_FLOAT,rgb_slicesmokecolormap_01);

  PRINT_TIMER(texture_timer, "texture setup");
  CheckMemory;

  // define terrain texture

  if(global_scase.terrain_texture_coll.nterrain_textures>0){
    texturedata *tt;
    unsigned char *floortex;
    int texwid, texht, nloaded=0;

    for(i=0;i<global_scase.terrain_texture_coll.nterrain_textures;i++){
      int is_transparent;

      tt = global_scase.terrain_texture_coll.terrain_textures + i;
      tt->loaded=0;
      tt->used=0;
      tt->display=0;
      tt->is_transparent = 0;

      glGenTextures(1,&tt->name);
      glBindTexture(GL_TEXTURE_2D,tt->name);
      floortex=NULL;
      if(tt->file!=NULL){
#ifdef _DEBUG
        PRINTF("terrain texture file: %s\n",tt->file);
#endif
        floortex=ReadPicture(global_scase.texturedir, tt->file,&texwid,&texht,&is_transparent,0);
        tt->is_transparent = is_transparent;
        if(floortex==NULL)PRINTF("***Error: Texture file %s failed to load\n",tt->file);
      }
      if(floortex!=NULL){
        glTexImage2D(GL_TEXTURE_2D, 0, 4, texwid, texht, 0, GL_RGBA, GL_UNSIGNED_BYTE, floortex);
        glGenerateMipmap(GL_TEXTURE_2D);
        SNIFF_ERRORS("after glTexImage2D for terrain texture");

        FREEMEMORY(floortex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        nloaded++;
        tt->loaded=1;
        if(nloaded==1)tt->display = 1; // display first texture by default
      }
    }
  }
  PRINT_TIMER(texture_timer, "terrain texture setup");

  // define sky texture

  if(nsky_texture > 0){
    texturedata *tt;
    unsigned char *floortex=NULL;
    int texwid, texht;

    int is_transparent;

    tt                 = sky_texture;
    tt->loaded         = 0;
    tt->used           = 0;
    tt->display        = 0;
    tt->is_transparent = 0;

    glGenTextures(1, &tt->name);
    glBindTexture(GL_TEXTURE_2D, tt->name);
    floortex = NULL;
    if(tt->file != NULL){
#ifdef _DEBUG
      PRINTF("sky texture file: %s\n", tt->file);
#endif
      floortex = ReadPicture(global_scase.texturedir, tt->file, &texwid, &texht, &is_transparent, 0);
      tt->is_transparent = is_transparent;
      if(floortex == NULL)PRINTF("***Error: Texture file %s failed to load\n", tt->file);
    }
    if(floortex != NULL){
      glTexImage2D(GL_TEXTURE_2D, 0, 4, texwid, texht, 0, GL_RGBA, GL_UNSIGNED_BYTE, floortex);
      glGenerateMipmap(GL_TEXTURE_2D);
      SNIFF_ERRORS("after glTexImage2D for terrain texture");

      FREEMEMORY(floortex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      tt->loaded = 1;
      tt->display = 1;
    }
  }
  PRINT_TIMER(texture_timer, "sky texture setup");
}

  /* ------------------ InitTextures ------------------------ */

void InitTextures(int use_graphics_arg){
  INIT_PRINT_TIMER(total_texture_time);
  UpdateDeviceTextures(&global_scase.objectscoll, global_scase.devicecoll.ndeviceinfo, global_scase.devicecoll.deviceinfo,
                       global_scase.propcoll.npropinfo, global_scase.propcoll.propinfo, &global_scase.device_texture_list_coll.ndevice_texture_list,
                       &global_scase.device_texture_list_coll.device_texture_list_index, &global_scase.device_texture_list_coll.device_texture_list);
  if(global_scase.surfcoll.nsurfinfo>0||global_scase.device_texture_list_coll.ndevice_texture_list>0){
    if(NewMemory((void **)&global_scase.texture_coll.textureinfo, (global_scase.surfcoll.nsurfinfo+global_scase.device_texture_list_coll.ndevice_texture_list+global_scase.terrain_texture_coll.nterrain_textures)*sizeof(texturedata))==0)return;
  }
  if(use_graphics_arg==1){
    InitTextures0();
  }
  PRINT_TIMER(total_texture_time, "total texure time");
}

  /* ------------------ UpdateBoundInfo ------------------------ */

void UpdateBoundInfo(void){
  int i,n;
  float bound_timer;

  START_TIMER(bound_timer);
  if(global_scase.nisoinfo>0){
    FREEMEMORY(isoindex);
    FREEMEMORY(isobounds);
    NewMemory((void*)&isoindex,global_scase.nisoinfo*sizeof(int));
    NewMemory((void*)&isobounds,global_scase.nisoinfo*sizeof(boundsdata));
    niso_bounds=0;
    for(i=0;i<global_scase.nisoinfo;i++){
      isodata *isoi;

      isoi = global_scase.isoinfo + i;
      if(isoi->dataflag==0)continue;
      isoi->firstshort_iso=1;
      isoindex[niso_bounds]=i;
      isobounds[niso_bounds].ini_defined = 0;
      isobounds[niso_bounds].shortlabel=isoi->color_label.shortlabel;
      isobounds[niso_bounds].dlg_setvalmin=0;
      isobounds[niso_bounds].dlg_setvalmax=0;
      isobounds[niso_bounds].dlg_valmin=1.0;
      isobounds[niso_bounds].dlg_valmax=0.0;
      isobounds[niso_bounds].edit_valmin = 0.0;
      isobounds[niso_bounds].edit_valmax = 1.0;
      isobounds[niso_bounds].edit_valmin_defined = 0;
      isobounds[niso_bounds].edit_valmax_defined = 0;
      isobounds[niso_bounds].setchopmax=0;
      isobounds[niso_bounds].setchopmin=0;
      isobounds[niso_bounds].chopmax=0.0;
      isobounds[niso_bounds].chopmin=1.0;
      isobounds[niso_bounds].label=&isoi->color_label;
      niso_bounds++;
      for(n=0;n<i;n++){
        isodata *ison;

        ison = global_scase.isoinfo + n;
        if(ison->dataflag==0)continue;
        if(strcmp(isoi->color_label.shortlabel,ison->color_label.shortlabel)==0){
          isoi->firstshort_iso=0;
          niso_bounds--;
          break;
        }
      }
    }
  }
  PRINT_TIMER(bound_timer, "isobounds");

  if(global_scase.slicecoll.nsliceinfo > 0){
    FREEMEMORY(slicebounds);
    NewMemory((void*)&slicebounds,global_scase.slicecoll.nsliceinfo*sizeof(boundsdata));
    nslicebounds=0;
    for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
      slicedata *slicei;
      boundsdata *sbi;

      slicei = global_scase.slicecoll.sliceinfo + i;
      slicei->valmin_slice =1.0;
      slicei->valmax_slice =0.0;
      slicei->setvalmin=0;
      slicei->setvalmax=0;

      sbi = slicebounds + nslicebounds;
      sbi->shortlabel=slicei->label.shortlabel;
      if(strcmp(sbi->shortlabel, "TEMP")==0)slicebounds_temp = sbi;
      sbi->dlg_setvalmin=PERCENTILE_MIN;
      sbi->dlg_setvalmax=PERCENTILE_MAX;
      sbi->dlg_valmin=1.0;
      sbi->dlg_valmax=0.0;
      sbi->chopmax=0.0;
      sbi->chopmin=1.0;
      sbi->setchopmax=0;
      sbi->setchopmin=0;
      sbi->line_contour_min=0.0;
      sbi->line_contour_max=1.0;
      sbi->line_contour_num=1;
      sbi->label = &(slicei->label);
      nslicebounds++;
      for(n=0;n<i;n++){
        slicedata *slicen;

        slicen = global_scase.slicecoll.sliceinfo + n;
        if(strcmp(slicei->label.shortlabel,slicen->label.shortlabel)==0){
          nslicebounds--;
          break;
        }
      }
    }
  }
  PRINT_TIMER(bound_timer, "slicebounds");

  canshow_threshold=0;
  if(global_scase.npatchinfo>0){
    FREEMEMORY(patchbounds);
    NewMemory((void*)&patchbounds, global_scase.npatchinfo*sizeof(boundsdata));
    npatchbounds = 0;
    npatch2=0;
    FREEMEMORY(patchlabellist);
    FREEMEMORY(patchlabellist_index);
    NewMemory((void **)&patchlabellist,global_scase.npatchinfo*sizeof(char *));
    NewMemory((void **)&patchlabellist_index,global_scase.npatchinfo*sizeof(int));
    for(i=0;i<global_scase.npatchinfo;i++){
      patchdata *patchi;
      boundsdata *sbi;

      patchi = global_scase.patchinfo + i;
      patchi->firstshort_patch=1;
      if(strncmp(patchi->label.shortlabel,"temp",4)==0||
         strncmp(patchi->label.shortlabel,"TEMP",4)==0){
        canshow_threshold=1;
      }
      patchlabellist[npatch2]=patchi->label.shortlabel;
      patchlabellist_index[npatch2]=i;
      npatch2++;
      for(n=0;n<i;n++){
        patchdata *patchn;

        patchn = global_scase.patchinfo + n;
        if(strcmp(patchi->label.shortlabel,patchn->label.shortlabel)==0){
          patchi->firstshort_patch = 0;
          npatch2--;
          break;
        }
      }
      sbi = patchbounds+npatchbounds;
      sbi->shortlabel = patchi->label.shortlabel;
      sbi->dlg_setvalmin = 0;
      sbi->dlg_setvalmax = 0;
      sbi->dlg_valmin = 1.0;
      sbi->dlg_valmax = 0.0;
      sbi->dlg_global_valmin = 1.0;
      sbi->dlg_global_valmax = 0.0;
      sbi->chopmax = 0.0;
      sbi->chopmin = 1.0;
      sbi->setchopmax = 0;
      sbi->setchopmin = 0;
      sbi->line_contour_min = 0.0;
      sbi->line_contour_max = 1.0;
      sbi->line_contour_num = 1;
      sbi->label = &(patchi->label);
      npatchbounds++;
      for(n = 0; n<i; n++){
        patchdata *patchn;

        patchn = global_scase.patchinfo+n;
        if(strcmp(patchi->label.shortlabel, patchn->label.shortlabel)==0){
          patchi->firstshort_patch = 0;
          npatchbounds--;
          break;
        }
      }
    }
  }
  PRINT_TIMER(bound_timer, "boundary file bounds");

  int nhvacboundsmax = 0;
  if(global_scase.hvaccoll.hvacductvalsinfo != NULL)nhvacboundsmax += global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;
  if(global_scase.hvaccoll.hvacnodevalsinfo != NULL)nhvacboundsmax += global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars;
  if(nhvacboundsmax>0 && global_scase.hvaccoll.hvacductvalsinfo!=NULL){
    FREEMEMORY(hvacductbounds);
    NewMemory((void*)&hvacductbounds,global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars*sizeof(boundsdata));
    nhvacductbounds=0;

    FREEMEMORY(hvacnodebounds);
    NewMemory((void*)&hvacnodebounds,global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars*sizeof(boundsdata));
    nhvacnodebounds=0;

    for(i=0;i<nhvacboundsmax;i++){
      hvacvaldata *hi;
      boundsdata *hbi;

      if(i<global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars){
        hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + i;
        hbi = hvacductbounds + nhvacductbounds;
      }
      else{
        hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + i - global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;
        hbi = hvacnodebounds + nhvacnodebounds;
      }
      hi->valmin=1.0;
      hi->valmax=0.0;
      hi->setvalmin=0;
      hi->setvalmax=0;

      hbi->shortlabel       = hi->label.shortlabel;
      hbi->dlg_setvalmin    = PERCENTILE_MIN;
      hbi->dlg_setvalmax    = PERCENTILE_MAX;
      hbi->dlg_valmin       = 1.0;
      hbi->dlg_valmax       = 0.0;
      hbi->chopmax          = 0.0;
      hbi->chopmin          = 1.0;
      hbi->setchopmax       = 0;
      hbi->setchopmin       = 0;
      hbi->line_contour_min = 0.0;
      hbi->line_contour_max = 1.0;
      hbi->line_contour_num = 1;
      hbi->label            = &(hi->label);
      int nbeg;
      if(i<global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars){
        nbeg = 0;
        nhvacductbounds++;
      }
      else{
        nbeg = global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;
        nhvacnodebounds++;
      }
      for(n=nbeg;n<i;n++){
        hvacvaldata *hn;

        if(n<global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars){
          hn = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + n;
        }
        else{
          hn = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + n - global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;
        }
        if(strcmp(hi->label.shortlabel,hn->label.shortlabel)==0){
          if(n<global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars){
            nhvacductbounds--;
          }
          else{
            nhvacnodebounds--;
          }
          break;
        }
      }
    }
  }
  PRINT_TIMER(bound_timer, "hvacbounds");

  GLUIUpdateChar();
  PRINT_TIMER(bound_timer, "GLUIUpdateChar");
  GetGlobalPartBounds(0);
  PRINT_TIMER(bound_timer, "GetGlobalPartBounds");

  GetGlobalSliceBoundsReduced();
  if(slicebound_threads == NULL){
    slicebound_threads = THREADinit(&n_slicebound_threads, &use_slicebound_threads, GetGlobalSliceBoundsFull);
  }
  THREADrun(slicebound_threads);
  PRINT_TIMER(bound_timer, "GetGlobalSliceBounds");

  GetGlobalPatchBoundsReduced();
  if(patchbound_threads == NULL){
    patchbound_threads = THREADinit(&n_patchbound_threads, &use_patchbound_threads, GetGlobalPatchBoundsFull);
  }
  THREADrun(patchbound_threads);
  PRINT_TIMER(bound_timer, "GetGlobalPatchBounds");

  GetGlobalHVACDuctBounds(0);
  PRINT_TIMER(bound_timer, "GetGlobalHVACDuctBounds");

  GetGlobalHVACNodeBounds(0);
  PRINT_TIMER(bound_timer, "GetGlobalHVACNodeBounds");
}

/*
new OBST format:
i1 i2 j1 j2 k1 k2 colorindex blocktype       : if blocktype!=1&&colorindex!=-3
i1 i2 j1 j2 k1 k2 colorindex blocktype r g b : if blocktype!=1&&colorindex==-3
i1 i2 j1 j2 k1 k2 ceiling texture blocktype [wall texture floor texture] : if blocktype==1
int colorindex, blocktype;
colorindex: -1 default color
-2 invisible
-3 use r g b color
>=0 color/color2/texture index
blocktype: 0 regular block non-textured
1 regular block textured
2 outline
3 smoothed block
r g b           colors used if colorindex==-3
*/

/* ------------------ UpdateVentOffset ------------------------ */

void UpdateVentOffset(void){
  int i;

  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    meshi->vent_offset[XXX] = ventoffset_factor*(meshi->xplt[1] - meshi->xplt[0]);
    meshi->vent_offset[YYY] = ventoffset_factor*(meshi->yplt[1] - meshi->yplt[0]);
    meshi->vent_offset[ZZZ] = ventoffset_factor*(meshi->zplt[1] - meshi->zplt[0]);
  }
}

/* ------------------ UpdateBlockType ------------------------ */

void UpdateBlockType(void){
  int igrid, i;

  ntransparentblocks = 0;
  ntransparentvents = 0;
  nopenvents = 0;
  nopenvents_nonoutline = 0;
  global_scase.ndummyvents = 0;
  for(igrid = 0; igrid<global_scase.meshescoll.nmeshes; igrid++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo+igrid;
    for(i = 0; i<meshi->nbptrs; i++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[i];
      if(bc->color[3]<0.99)ntransparentblocks++;
    }
    for(i = 0; i<meshi->nvents; i++){
      ventdata *vi;

      vi = meshi->ventinfo+i;
      if(vi->isOpenvent==1){
        nopenvents++;
        if(vi->type!=BLOCK_OUTLINE)nopenvents_nonoutline++;
      }
      if(vi->dummy==1)global_scase.ndummyvents++;
      if(vi->color[3]<0.99)ntransparentvents++;
    }
  }
}

/* ------------------ GetBoxSkyCorners ------------------------ */

void GetBoxSkyCorners(void){
  float dxFDS, dyFDS, dzFDS, radius;
  float xmin, ymin, zmin;
  float xmax, ymax, zmax;

  dxFDS = (xbarFDS - xbar0FDS);
  dyFDS = (ybarFDS - ybar0FDS);
  dzFDS = (zbarFDS - zbar0FDS);
  radius = sqrt(dxFDS * dxFDS + dyFDS * dyFDS + dzFDS * dzFDS) / 2.0;

  xmin = (xbar0FDS + xbarFDS) / 2.0 - sky_diam * radius;
  xmax = (xbar0FDS + xbarFDS) / 2.0 + sky_diam * radius;
  ymin = (ybar0FDS + ybarFDS) / 2.0 - sky_diam * radius;
  ymax = (ybar0FDS + ybarFDS) / 2.0 + sky_diam * radius;
  zmin = 0.0;
  zmax = sky_diam * radius;

  xmin = FDS2SMV_X(xmin);
  xmax = FDS2SMV_X(xmax);
  ymin = FDS2SMV_Y(ymin);
  ymax = FDS2SMV_Y(ymax);
  zmin = FDS2SMV_Z(zmin);
  zmax = FDS2SMV_Z(zmax);

  box_sky_corners[0][0] = xmin;
  box_sky_corners[0][1] = ymin;
  box_sky_corners[0][2] = zmin;

  box_sky_corners[1][0] = xmax;
  box_sky_corners[1][1] = ymin;
  box_sky_corners[1][2] = zmin;

  box_sky_corners[2][0] = xmin;
  box_sky_corners[2][1] = ymax;
  box_sky_corners[2][2] = zmin;

  box_sky_corners[3][0] = xmax;
  box_sky_corners[3][1] = ymax;
  box_sky_corners[3][2] = zmin;

  box_sky_corners[4][0] = xmin;
  box_sky_corners[4][1] = ymin;
  box_sky_corners[4][2] = zmax;

  box_sky_corners[5][0] = xmax;
  box_sky_corners[5][1] = ymin;
  box_sky_corners[5][2] = zmax;

  box_sky_corners[6][0] = xmin;
  box_sky_corners[6][1] = ymax;
  box_sky_corners[6][2] = zmax;

  box_sky_corners[7][0] = xmax;
  box_sky_corners[7][1] = ymax;
  box_sky_corners[7][2] = zmax;
}

/* ------------------ GetBoxGeomCorners ------------------------ */

void GetBoxGeomCorners(void){
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int i;
  float *xyz;
  geomdata *geomi;
  vertdata *verti;
  geomlistdata *geomlisti;

  have_box_geom_corners = 0;
  if(global_scase.geominfo==NULL||global_scase.geominfo->geomlistinfo==NULL||global_scase.geominfo==0)return;

  geomi = global_scase.geominfo;
  geomlisti = geomi->geomlistinfo-1;
  if(geomlisti->nverts<=0)return;

  have_box_geom_corners = 1;

  verti = geomlisti->verts;
  xyz = verti->xyz;

  xmin = xyz[0];
  xmax = xmin;
  ymin = xyz[1];
  ymax = ymin;
  zmin = xyz[2];
  zmax = zmin;

  for(i = 1; i<geomlisti->nverts; i++){
    verti = geomlisti->verts+i;
    xyz = verti->xyz;
    xmin = MIN(xyz[0], xmin);
    xmax = MAX(xyz[0], xmax);
    ymin = MIN(xyz[1], ymin);
    ymax = MAX(xyz[1], ymax);
    zmin = MIN(xyz[2], zmin);
    zmax = MAX(xyz[2], zmax);
  }

  xmin = FDS2SMV_X(xmin);
  xmax = FDS2SMV_X(xmax);
  ymin = FDS2SMV_Y(ymin);
  ymax = FDS2SMV_Y(ymax);
  zmin = FDS2SMV_Z(zmin);
  zmax = FDS2SMV_Z(zmax);

  box_geom_corners[0][0] = xmin;
  box_geom_corners[0][1] = ymin;
  box_geom_corners[0][2] = zmin;

  box_geom_corners[1][0] = xmax;
  box_geom_corners[1][1] = ymin;
  box_geom_corners[1][2] = zmin;

  box_geom_corners[2][0] = xmin;
  box_geom_corners[2][1] = ymax;
  box_geom_corners[2][2] = zmin;

  box_geom_corners[3][0] = xmax;
  box_geom_corners[3][1] = ymax;
  box_geom_corners[3][2] = zmin;

  box_geom_corners[4][0] = xmin;
  box_geom_corners[4][1] = ymin;
  box_geom_corners[4][2] = zmax;

  box_geom_corners[5][0] = xmax;
  box_geom_corners[5][1] = ymin;
  box_geom_corners[5][2] = zmax;

  box_geom_corners[6][0] = xmin;
  box_geom_corners[6][1] = ymax;
  box_geom_corners[6][2] = zmax;

  box_geom_corners[7][0] = xmax;
  box_geom_corners[7][1] = ymax;
  box_geom_corners[7][2] = zmax;
}

  /* ------------------ GetBoxCorners ------------------------ */

void GetBoxCorners(float xbar_local, float ybar_local, float zbar_local){
  box_corners[0][0] = 0.0;
  box_corners[0][1] = 0.0;
  box_corners[0][2] = 0.0;

  box_corners[1][0] = xbar_local;
  box_corners[1][1] = 0.0;
  box_corners[1][2] = 0.0;

  box_corners[2][0] = 0.0;
  box_corners[2][1] = ybar_local;
  box_corners[2][2] = 0.0;

  box_corners[3][0] = xbar_local;
  box_corners[3][1] = ybar_local;
  box_corners[3][2] = 0.0;

  box_corners[4][0] = 0.0;
  box_corners[4][1] = 0.0;
  box_corners[4][2] = zbar_local;

  box_corners[5][0] = xbar_local;
  box_corners[5][1] = 0.0;
  box_corners[5][2] = zbar_local;

  box_corners[6][0] = 0.0;
  box_corners[6][1] = ybar_local;
  box_corners[6][2] = zbar_local;

  box_corners[7][0] = xbar_local;
  box_corners[7][1] = ybar_local;
  box_corners[7][2] = zbar_local;
}

/* ------------------ UpdateMeshBoxBounds ------------------------ */

void UpdateMeshBoxBounds(void){
  int i;

  for(i = 0; i<global_scase.meshescoll.nmeshes;  i++){
    meshdata *meshi;

    // xplt, yplt, zplt has original coordinates because this routine is called before UpdateMeshCoords
    meshi = global_scase.meshescoll.meshinfo+i;
    meshi->boxmin[0] = meshi->xplt[0];
    meshi->boxmin[1] = meshi->yplt[0];
    meshi->boxmin[2] = meshi->zplt[0];
    meshi->boxmax[0] = meshi->xplt[meshi->ibar];
    meshi->boxmax[1] = meshi->yplt[meshi->jbar];
    meshi->boxmax[2] = meshi->zplt[meshi->kbar];
    meshi->boxeps_fds[0] = (meshi->xplt[1] - meshi->xplt[0]) / 2.0;
    meshi->boxeps_fds[1] = (meshi->yplt[1] - meshi->yplt[0]) / 2.0;
    meshi->boxeps_fds[2] = (meshi->zplt[1] - meshi->zplt[0]) / 2.0;
  }
}

/// @brief Given a case, find the first instance of smoke3dtype that has the
/// given label.
/// @param scase The case
/// @param label The label to search for
/// @return An offset into scase->smoke3dcoll.nsmoke3dtypes of the first
/// matching smoke3dtype. Returns -1 if there are no matching props.
int GetSmoke3DType(smv_case *scase, const char *label) {
  int i;

  if(scase->smoke3dcoll.smoke3dtypes != NULL){
    for(i = 0; i < scase->smoke3dcoll.nsmoke3dtypes; i++){
      smoke3ddata *smoke3di;

      smoke3di = scase->smoke3dcoll.smoke3dtypes[i].smoke3d;
      if(Match(smoke3di->label.shortlabel, label) == 1)return i;
    }
  }
  return -1;
}

/* ------------------ CompareSmoketypes ------------------------ */

int CompareSmoketypes(const void *arg1, const void *arg2){
  smoke3dtypedata *smoketypei, *smoketypej;
  smoke3ddata *smoke3di, *smoke3dj;
  char *labeli, *labelj;
  float exti, extj;

  smoketypei = (smoke3dtypedata *)arg1;
  smoketypej = (smoke3dtypedata *)arg2;
  smoke3di = smoketypei->smoke3d;
  smoke3dj = smoketypej->smoke3d;
  labeli = smoke3di->label.longlabel;
  labelj = smoke3dj->label.longlabel;
  exti = smoke3di->extinct;
  extj = smoke3dj->extinct;

  if(Match(labeli,labelj)==1)return 0;

  if(exti>0.0&&extj>0.0)return strcmp(labeli, labelj);

  if(exti>0.0)return -1;
  if(extj>0.0)return 1;

  if(Match(labeli, "HRRPUV")==1)return -1;
  if(Match(labelj, "HRRPUV")==1)return  1;

  if(strstr(labeli, "TEMPERATURE")!=NULL)return  -1;
  if(strstr(labelj, "TEMPERATURE")!=NULL)return   1;

  return strcmp(labeli, labelj);
}

/* ------------------ UpdateSmokeTypes ------------------------ */

void UpdateSmoke3DTypes(void){
  int i;

  if(global_scase.smoke3dcoll.nsmoke3dinfo==0)return;
  NewMemory((void **)&global_scase.smoke3dcoll.smoke3dtypes, global_scase.smoke3dcoll.nsmoke3dinfo*sizeof(smoke3dtypedata));
  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    int j, doit;
    char *labeli;
    smoke3dtypedata *typen;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    labeli = smoke3di->label.shortlabel;
    doit = 1;
    for(j = 0; j<i; j++){
      smoke3ddata *smoke3dj;
      char *labelj;

      smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
      labelj = smoke3dj->label.shortlabel;
      if(strcmp(labeli, labelj)==0){
        doit = 0;
        break;
      }
    }
    if(doit==1){
      typen = global_scase.smoke3dcoll.smoke3dtypes+global_scase.smoke3dcoll.nsmoke3dtypes;
      typen->smoke3d = smoke3di;
      typen->shortlabel = smoke3di->label.shortlabel;
      typen->longlabel = smoke3di->label.longlabel;
      typen->extinction = smoke3di->extinct;
      global_scase.smoke3dcoll.nsmoke3dtypes++;
    }
  }
  if(global_scase.smoke3dcoll.nsmoke3dtypes>0){
    qsort((smoke3ddata **)global_scase.smoke3dcoll.smoke3dtypes, global_scase.smoke3dcoll.nsmoke3dtypes, sizeof(smoke3dtypedata), CompareSmoketypes);
    ResizeMemory((void **)&global_scase.smoke3dcoll.smoke3dtypes, global_scase.smoke3dcoll.nsmoke3dtypes*sizeof(smoke3dtypedata));
  }
  else{
    FREEMEMORY(global_scase.smoke3dcoll.smoke3dtypes);
  }
  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    int j;
    smokestatedata *smokestate;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    smoke3di->type = GetSmoke3DType(&global_scase, smoke3di->label.shortlabel);

    NewMemory((void **)&smokestate, global_scase.smoke3dcoll.nsmoke3dtypes*sizeof(smokestatedata));
    smoke3di->smokestate = smokestate;
    for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dtypes; j++){
      smoke3di->smokestate[j].color = NULL;
      smoke3di->smokestate[j].index = -1;
    }
  }
  global_scase.smoke3dcoll.smoke3d_other     = global_scase.smoke3dcoll.nsmoke3dtypes;
  SOOT_index   = -1;
  HRRPUV_index = -1;
  TEMP_index   = -1;
  CO2_index    = -1;

  if(global_scase.smoke3dcoll.smoke3dtypes != NULL){
    for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dtypes; i++){
      smoke3ddata *smoke3di;
      char *label;
      float ext;

      smoke3di = global_scase.smoke3dcoll.smoke3dtypes[i].smoke3d;
      label = smoke3di->label.shortlabel;
      ext = smoke3di->extinct;
      if(ext > 0.0){
        SOOT_index = i;
        glui_smoke3d_extinct = global_scase.smoke3dcoll.smoke3dtypes[i].extinction;
        continue;
      }
      if(Match(label, "hrrpuv") == 1){
        HRRPUV_index = i;
        continue;
      }
      if(Match(label, "temp") == 1){
        TEMP_index = i;
        continue;
      }
      if(Match(label, "rho_CO2") == 1 || Match(label, "Y_CO2") == 1){
        CO2_index = i;
        continue;
      }
    }
  }
  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->type == SOOT_index && FileExistsCaseDir(&global_scase, smoke3di->smoke_density_file)==YES){
      smoke3di->is_smoke_density = 1;
      have_smoke_density = 1;
    }
  }
}

/* ------------------ UpdateMeshCoords ------------------------ */

void UpdateMeshCoords(void){
  int nn, i,n;
  float dxsbar, dysbar, dzsbar;
  int factor;
  int igrid;

  if(global_scase.setPDIM==0){
    for(nn=0;nn<=current_mesh->ibar;nn++){
      current_mesh->xplt[nn]=global_scase.xbar0+(float)nn*(global_scase.xbar-global_scase.xbar0)/(float)current_mesh->ibar;
    }
    for(nn=0;nn<=current_mesh->jbar;nn++){
      current_mesh->yplt[nn]=global_scase.ybar0+(float)nn*(global_scase.ybar-global_scase.ybar0)/(float)current_mesh->jbar;
    }
    for(nn=0;nn<=current_mesh->kbar;nn++){
      current_mesh->zplt[nn]=global_scase.zbar0+(float)nn*(global_scase.zbar-global_scase.zbar0)/(float)current_mesh->kbar;
    }
  }

  /* define highlighted block */

  /* add in offsets */
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int ii;

    meshi=global_scase.meshescoll.meshinfo+i;
    meshi->xyz_bar[XXX] += meshi->offset[XXX];
    meshi->xyz_bar[YYY] += meshi->offset[YYY];
    meshi->xyz_bar[ZZZ] += meshi->offset[ZZZ];
    meshi->xyz_bar0[XXX] += meshi->offset[XXX];
    meshi->xyz_bar0[YYY] += meshi->offset[YYY];
    meshi->xyz_bar0[ZZZ] += meshi->offset[ZZZ];
    {
      float dx, dy, dz;

      dx = (meshi->xyz_bar[XXX] - meshi->xyz_bar0[XXX])/meshi->ibar;
      dy = (meshi->xyz_bar[YYY] - meshi->xyz_bar0[YYY])/meshi->jbar;
      dz = (meshi->xyz_bar[ZZZ] - meshi->xyz_bar0[ZZZ])/meshi->kbar;
      meshi->cellsize=sqrt(dx*dx+dy*dy+dz*dz);
    }
    for(ii=0;ii<meshi->ibar+1;ii++){
      meshi->xplt[ii] += meshi->offset[XXX];
    }
    for(ii=0;ii<meshi->jbar+1;ii++){
      meshi->yplt[ii] += meshi->offset[YYY];
    }
    for(ii=0;ii<meshi->kbar+1;ii++){
      meshi->zplt[ii] += meshi->offset[ZZZ];
    }
    meshi->xcen+=meshi->offset[XXX];
    meshi->ycen+=meshi->offset[YYY];
    meshi->zcen+=meshi->offset[ZZZ];
  }
  for(i=1;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    if(meshi->xyz_bar0[ZZZ]!=global_scase.meshescoll.meshinfo->xyz_bar0[ZZZ]){
      global_scase.visFloor=0;
      updatefacelists=1;
      updatemenu=1;
      break;
    }
  }

  ijkbarmax=global_scase.meshescoll.meshinfo->ibar;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;

    ijkbarmax = MAX(ijkbarmax,meshi->ibar);
    ijkbarmax = MAX(ijkbarmax,meshi->jbar);
    ijkbarmax = MAX(ijkbarmax,meshi->kbar);
  }

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    float xminmax[2], yminmax[2], zminmax[2];
    float *verts;
    int xindex[8]={0,1,0,1,0,1,0,1};
    int yindex[8]={0,0,1,1,0,0,1,1};
    int zindex[8]={0,0,0,0,1,1,1,1};
    int j;

    meshi=global_scase.meshescoll.meshinfo+i;
    if(i==0){
      global_scase.xbar = meshi->xyz_bar[XXX];
      global_scase.ybar = meshi->xyz_bar[YYY];
      global_scase.zbar = meshi->xyz_bar[ZZZ];

      global_scase.xbar0 = meshi->xyz_bar0[XXX];
      global_scase.ybar0 = meshi->xyz_bar0[YYY];
      global_scase.zbar0 = meshi->xyz_bar0[ZZZ];
    }
    global_scase.xbar = MAX(global_scase.xbar, meshi->xyz_bar[XXX]);
    global_scase.ybar = MAX(global_scase.ybar, meshi->xyz_bar[YYY]);
    global_scase.zbar = MAX(global_scase.zbar, meshi->xyz_bar[ZZZ]);
    global_scase.xbar0 = MIN(global_scase.xbar0, meshi->xyz_bar0[XXX]);
    global_scase.ybar0 = MIN(global_scase.ybar0, meshi->xyz_bar0[YYY]);
    global_scase.zbar0 = MIN(global_scase.zbar0, meshi->xyz_bar0[ZZZ]);

    xminmax[0] = meshi->xyz_bar0[XXX];
    xminmax[1] = meshi->xyz_bar[XXX];
    yminmax[0] = meshi->xyz_bar0[YYY];
    yminmax[1] = meshi->xyz_bar[YYY];
    zminmax[0] = meshi->xyz_bar0[ZZZ];
    zminmax[1] = meshi->xyz_bar[ZZZ];
    verts = meshi->verts;
    for(j=0;j<8;j++){
      verts[3*j+0] = xminmax[xindex[j]];
      verts[3*j+1] = yminmax[yindex[j]];
      verts[3*j+2] = zminmax[zindex[j]];
    }
  }

  xbar0FDS = global_scase.xbar0;
  ybar0FDS = global_scase.ybar0;
  zbar0FDS = global_scase.zbar0;
  xbarFDS  = global_scase.xbar;
  ybarFDS  = global_scase.ybar;
  zbarFDS  = global_scase.zbar;

  use_meshclip[0] = 0;
  use_meshclip[1] = 0;
  use_meshclip[2] = 0;
  use_meshclip[3] = 0;
  use_meshclip[4] = 0;
  use_meshclip[5] = 0;
  meshclip[0] = xbar0FDS;
  meshclip[1] = xbarFDS;
  meshclip[2] = ybar0FDS;
  meshclip[3] = ybarFDS;
  meshclip[4] = zbar0FDS;
  meshclip[5] = zbarFDS;

  geomlistdata *geomlisti;
  if(global_scase.geominfo!=NULL&&global_scase.geominfo->geomlistinfo!=NULL){
    geomlisti = global_scase.geominfo->geomlistinfo-1;
    if(geomlisti->nverts>0){
      vertdata *verti;
      float *xyz;
      float xmin, xmax, ymin, ymax, zmin, zmax;

      verti = geomlisti->verts;
      xyz = verti->xyz;

      xmin = xyz[0];
      xmax = xmin;
      ymin = xyz[1];
      ymax = ymin;
      zmin = xyz[2];
      zmax = zmin;

      geom_bounding_box[0] = geomlisti->verts[0].xyz[0];
      geom_bounding_box[1] = geomlisti->verts[0].xyz[0];
      geom_bounding_box[2] = geomlisti->verts[0].xyz[1];
      geom_bounding_box[3] = geomlisti->verts[0].xyz[1];
      geom_bounding_box[4] = geomlisti->verts[0].xyz[2];
      geom_bounding_box[5] = geomlisti->verts[0].xyz[2];
      for(i = 1; i<geomlisti->nverts; i++){
        verti = geomlisti->verts+i;
        xyz = verti->xyz;
        xmin = MIN(xyz[0], xmin);
        xmax = MAX(xyz[0], xmax);
        ymin = MIN(xyz[1], ymin);
        ymax = MAX(xyz[1], ymax);
        zmin = MIN(xyz[2], zmin);
        zmax = MAX(xyz[2], zmax);
      }
      global_scase.xbar0 = MIN(global_scase.xbar0, xmin);
      global_scase.ybar0 = MIN(global_scase.ybar0, ymin);
      global_scase.zbar0 = MIN(global_scase.zbar0, zmin);
      global_scase.xbar = MAX(global_scase.xbar, xmax);
      global_scase.ybar = MAX(global_scase.ybar, ymax);
      global_scase.zbar = MAX(global_scase.zbar, zmax);
      geom_bounding_box[0] = MIN(xmin, geom_bounding_box[0]);
      geom_bounding_box[1] = MAX(xmax, geom_bounding_box[1]);
      geom_bounding_box[2] = MIN(ymin, geom_bounding_box[2]);
      geom_bounding_box[3] = MAX(ymax, geom_bounding_box[3]);
      geom_bounding_box[4] = MIN(zmin, geom_bounding_box[4]);
      geom_bounding_box[5] = MAX(zmax, geom_bounding_box[5]);
      have_geom_bb = 1;
    }
  }

  factor = 256*128;
  dxsbar = (global_scase.xbar-global_scase.xbar0)/factor;
  dysbar = (global_scase.ybar-global_scase.ybar0)/factor;
  dzsbar = (global_scase.zbar-global_scase.zbar0)/factor;

  for(nn=0;nn<factor;nn++){
    xplts[nn]=global_scase.xbar0+((float)nn+0.5)*dxsbar;
  }
  for(nn=0;nn<factor;nn++){
    yplts[nn]=global_scase.ybar0+((float)nn+0.5)*dysbar;
  }
  for(nn=0;nn<factor;nn++){
    zplts[nn]=global_scase.zbar0+((float)nn+0.5)*dzsbar;
  }

  /* compute scaling factors */

  {
    float dxclip, dyclip, dzclip;

    dxclip = (global_scase.xbar-global_scase.xbar0)/1000.0;
    dyclip = (global_scase.ybar-global_scase.ybar0)/1000.0;
    dzclip = (global_scase.zbar-global_scase.zbar0)/1000.0;
    xclip_min = global_scase.xbar0-dxclip;
    yclip_min = global_scase.ybar0-dyclip;
    zclip_min = global_scase.zbar0-dzclip;
    xclip_max = global_scase.xbar+dxclip;
    yclip_max = global_scase.ybar+dyclip;
    zclip_max = global_scase.zbar+dzclip;
  }

  // compute scaling factor used in NORMALIXE_X, NORMALIZE_Y, NORMALIZE_Z macros

  xyzmaxdiff=MAX(MAX(global_scase.xbar-global_scase.xbar0,global_scase.ybar-global_scase.ybar0),global_scase.zbar-global_scase.zbar0);

  // normalize various coordinates.

  for(nn=0;nn<factor;nn++){
    xplts[nn]=FDS2SMV_X(xplts[nn]);
  }
  for(nn=0;nn<factor;nn++){
    yplts[nn]=FDS2SMV_Y(yplts[nn]);
  }
  for(nn=0;nn<factor;nn++){
    zplts[nn]=FDS2SMV_Z(zplts[nn]);
  }

  /* rescale both global and local xbar, ybar and zbar */

  xbar0ORIG = global_scase.xbar0;
  ybar0ORIG = global_scase.ybar0;
  zbar0ORIG = global_scase.zbar0;
  xbarORIG = global_scase.xbar;
  ybarORIG = global_scase.ybar;
  zbarORIG = global_scase.zbar;

  patchout_xmin = xbar0ORIG;
  patchout_xmax = xbarORIG;
  patchout_ymin = ybar0ORIG;
  patchout_ymax = ybarORIG;
  patchout_zmin = zbar0ORIG;
  patchout_zmax = zbarORIG;
  patchout_tmin = 0.0;
  if(global_scase.tourcoll.tour_tstop>0.0){
    patchout_tmax = global_scase.tourcoll.tour_tstop;
  }
  else{
    patchout_tmax = 1.0;
  }

  global_scase.xbar = FDS2SMV_X(global_scase.xbar);
  global_scase.ybar = FDS2SMV_Y(global_scase.ybar);
  global_scase.zbar = FDS2SMV_Z(global_scase.zbar);

  float outline_offset;
  outline_offset = (global_scase.meshescoll.meshinfo->zplt[1] - global_scase.meshescoll.meshinfo->zplt[0]) / 10.0;
  if(global_scase.is_terrain_case==1){
    geom_dz_offset = outline_offset;
    geom_norm_offset = 0.0;
  }
  else{
    geom_dz_offset = 0.0;
    geom_norm_offset = outline_offset;
  }
  GetBoxCorners(global_scase.xbar, global_scase.ybar, global_scase.zbar);

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    /* compute a local scaling factor for each block */
    meshi->xyzmaxdiff=MAX(MAX(meshi->xyz_bar[XXX]-meshi->xyz_bar0[XXX],meshi->xyz_bar[YYY]-meshi->xyz_bar0[YYY]),meshi->xyz_bar[ZZZ]-meshi->xyz_bar0[ZZZ]);

    FDS2SMV_XYZ(meshi->xyz_bar,meshi->xyz_bar);
    meshi->xcen = FDS2SMV_X(meshi->xcen);
    meshi->ycen = FDS2SMV_Y(meshi->ycen);
    meshi->zcen = FDS2SMV_Z(meshi->zcen);
  }

  for(i=0;i<global_scase.noutlineinfo;i++){
    outlinedata *outlinei;
    float *x1, *x2, *yy1, *yy2, *z1, *z2;
    int j;

    outlinei = global_scase.outlineinfo + i;
    x1 = outlinei->x1;
    x2 = outlinei->x2;
    yy1 = outlinei->y1;
    yy2 = outlinei->y2;
    z1 = outlinei->z1;
    z2 = outlinei->z2;
    for(j=0;j<outlinei->nlines;j++){
      x1[j]=FDS2SMV_X(x1[j]);
      x2[j]=FDS2SMV_X(x2[j]);
      yy1[j]=FDS2SMV_Y(yy1[j]);
      yy2[j]=FDS2SMV_Y(yy2[j]);
      z1[j]=FDS2SMV_Z(z1[j]);
      z2[j]=FDS2SMV_Z(z2[j]);
    }
  }

  min_gridcell_size=global_scase.meshescoll.meshinfo->xplt[1]-global_scase.meshescoll.meshinfo->xplt[0];
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    float dx, dy, dz;
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+i;
    dx = meshi->xplt[1] - meshi->xplt[0];
    dy = meshi->yplt[1] - meshi->yplt[0];
    dz = meshi->zplt[1] - meshi->zplt[0];
    min_gridcell_size=MIN(dx,min_gridcell_size);
    min_gridcell_size=MIN(dy,min_gridcell_size);
    min_gridcell_size=MIN(dz,min_gridcell_size);
  }

  for(igrid=0;igrid<global_scase.meshescoll.nmeshes;igrid++){
    meshdata *meshi;
    float *face_centers;
    float *xplt_cen, *yplt_cen, *zplt_cen;
    int ibar, jbar, kbar;
    float *xplt_orig, *yplt_orig, *zplt_orig;
    float *xplt, *yplt, *zplt;
    int j,k;
    float dx, dy, dz;
    float *dplane_min, *dplane_max;

    meshi=global_scase.meshescoll.meshinfo+igrid;
    ibar=meshi->ibar;
    jbar=meshi->jbar;
    kbar=meshi->kbar;
    xplt_orig = meshi->xplt_orig;
    yplt_orig = meshi->yplt_orig;
    zplt_orig = meshi->zplt_orig;
    xplt = meshi->xplt;
    yplt = meshi->yplt;
    zplt = meshi->zplt;
    xplt_cen = meshi->xplt_cen;
    yplt_cen = meshi->yplt_cen;
    zplt_cen = meshi->zplt_cen;

    for(i=0;i<ibar+1;i++){
      xplt_orig[i]=xplt[i];
      xplt[i]=FDS2SMV_X(meshi->xpltd[i]);
    }
    for(j=0;j<jbar+1;j++){
      yplt_orig[j]=yplt[j];
      yplt[j]=FDS2SMV_Y(meshi->ypltd[j]);
    }
    for(k=0;k<kbar+1;k++){
      zplt_orig[k]=zplt[k];
      zplt[k]=FDS2SMV_Z(meshi->zpltd[k]);
    }

    for(nn=0;nn<ibar;nn++){
      xplt_cen[nn]=(xplt[nn]+xplt[nn+1])/2.0;
    }
    for(nn=0;nn<jbar;nn++){
      yplt_cen[nn]=(yplt[nn]+yplt[nn+1])/2.0;
    }
    for(nn=0;nn<kbar;nn++){
      zplt_cen[nn]=(zplt[nn]+zplt[nn+1])/2.0;
    }

    meshi->boxoffset=-(zplt[1]-zplt[0])/10.0;
    meshi->dbox[0]=meshi->boxmax[0]-meshi->boxmin[0];
    meshi->dbox[1]=meshi->boxmax[1]-meshi->boxmin[1];
    meshi->dbox[2]=meshi->boxmax[2]-meshi->boxmin[2];
    meshi->boxmiddle[0] = meshi->boxmin[0]+meshi->dbox[0]/2.0;
    meshi->boxmiddle[1] = meshi->boxmin[1]+meshi->dbox[1]/2.0;
    meshi->boxmiddle[2] = meshi->boxmin[2]+meshi->dbox[2]/2.0;
    meshi->boxeps[0]=0.5*(xplt[ibar]-xplt[0])/(float)ibar;
    meshi->boxeps[1]=0.5*(yplt[jbar]-yplt[0])/(float)jbar;
    meshi->boxeps[2]=0.5*(zplt[kbar]-zplt[0])/(float)kbar;
    meshi->dcell3[0] = xplt[1]-xplt[0];
    meshi->dcell3[1] = yplt[1]-yplt[0];
    meshi->dcell3[2] = zplt[1]-zplt[0];
    FDS2SMV_XYZ(meshi->boxmin_scaled,meshi->boxmin);
    FDS2SMV_XYZ(meshi->boxmax_scaled,meshi->boxmax);
    FDS2SMV_XYZ(meshi->boxmiddle_scaled, meshi->boxmiddle);
    meshi->x0 = xplt[0];
    meshi->x1 = xplt[ibar];
    meshi->y0 = yplt[0];
    meshi->y1 = yplt[jbar];
    meshi->z0 = zplt[0];
    meshi->z1 = zplt[kbar];
    dx = xplt[1]-xplt[0];
    dy = yplt[1]-yplt[0];
    dz = zplt[1]-zplt[0];
    meshi->dcell = sqrt(dx*dx+dy*dy+dz*dz);

    dplane_min = meshi->dplane_min;
    dplane_min[0] = MIN(MIN(dx, dy), dz);
    dplane_min[1] = MIN(dy, dz);
    dplane_min[2] = MIN(dx, dz);
    dplane_min[3] = MIN(dx, dy);

    dplane_max = meshi->dplane_max;
    dplane_max[0] = MAX(MAX(dx, dy), dz);
    dplane_max[1] = MAX(dy, dz);
    dplane_max[2] = MAX(dx, dz);
    dplane_max[3] = MAX(dx, dy);

    face_centers = meshi->face_centers;
    for(j=0;j<6;j++){
      face_centers[0]=meshi->xcen;
      face_centers[1]=meshi->ycen;
      face_centers[2]=meshi->zcen;
      face_centers+=3;
    }
    face_centers = meshi->face_centers;
    face_centers[0]=meshi->boxmin_scaled[0];
    face_centers[3]=meshi->boxmax_scaled[0];
    face_centers[7]=meshi->boxmin_scaled[1];
    face_centers[10]=meshi->boxmax_scaled[1];
    face_centers[14]=meshi->boxmin_scaled[2];
    face_centers[17]=meshi->boxmax_scaled[2];
  }
  if(global_scase.nterraininfo>0){
    boundaryoffset = (global_scase.meshescoll.meshinfo->zplt_orig[1] - global_scase.meshescoll.meshinfo->zplt_orig[0]) / 10.0;
  }

  UpdateBlockType();

  for(igrid=0;igrid<global_scase.meshescoll.nmeshes;igrid++){
    meshdata *meshi;

    meshi=global_scase.meshescoll.meshinfo+igrid;
    for(i=0;i<meshi->nbptrs;i++){
      blockagedata *bc;

      bc=meshi->blockageinfoptrs[i];
      bc->xmin += meshi->offset[XXX];
      bc->xmax += meshi->offset[XXX];
      bc->ymin += meshi->offset[YYY];
      bc->ymax += meshi->offset[YYY];
      bc->zmin += meshi->offset[ZZZ];
      bc->zmax += meshi->offset[ZZZ];
      bc->xmin = FDS2SMV_X(bc->xmin);
      bc->xmax = FDS2SMV_X(bc->xmax);
      bc->ymin = FDS2SMV_Y(bc->ymin);
      bc->ymax = FDS2SMV_Y(bc->ymax);
      bc->zmin = FDS2SMV_Z(bc->zmin);
      bc->zmax = FDS2SMV_Z(bc->zmax);
    }
    for(i=0;i<meshi->nvents+12;i++){
      ventdata *vi;

      vi=meshi->ventinfo+i;
      vi->xmin = FDS2SMV_X(vi->xmin);
      vi->xmax = FDS2SMV_X(vi->xmax);
      vi->ymin = FDS2SMV_Y(vi->ymin);
      vi->ymax = FDS2SMV_Y(vi->ymax);
      vi->zmin = FDS2SMV_Z(vi->zmin);
      vi->zmax = FDS2SMV_Z(vi->zmax);
    }
  }
  for(i=0;i<NCADGeom(&global_scase.cadgeomcoll);i++){
    cadgeomdata *cd;
    int j;

    cd=global_scase.cadgeomcoll.cadgeominfo+i;
    for(j=0;j<cd->nquads;j++){
      int k;
      cadquad *quadi;

      quadi = cd->quad+j;
      for(k=0;k<4;k++){
        FDS2SMV_XYZ(quadi->xyzpoints+3*k,quadi->xyzpoints+3*k);
      }
      if(cd->version==2&&quadi->cadlookq->textureinfo.loaded==1){
        UpdateCADTextCoords(quadi);
      }
    }
  }
  for(n=0;n<global_scase.nrooms;n++){
    roomdata *roomi;

    roomi = global_scase.roominfo + n;
    roomi->x0=FDS2SMV_X(roomi->x0);
    roomi->y0=FDS2SMV_Y(roomi->y0);
    roomi->z0=FDS2SMV_Z(roomi->z0);
    roomi->x1=FDS2SMV_X(roomi->x1);
    roomi->y1=FDS2SMV_Y(roomi->y1);
    roomi->z1=FDS2SMV_Z(roomi->z1);
    roomi->dx=SCALE2SMV(roomi->dx);
    roomi->dy=SCALE2SMV(roomi->dy);
    roomi->dz=SCALE2SMV(roomi->dz);
  }
  for(n=0;n<global_scase.nfires;n++){
    firedata *firen;

    firen = global_scase.fireinfo + n;
    firen->absx=FDS2SMV_X(firen->absx);
    firen->absy=FDS2SMV_Y(firen->absy);
    firen->absz=FDS2SMV_Z(firen->absz);
    firen->dz=SCALE2SMV(firen->dz);
  }
  for(n=0;n<global_scase.nzvents;n++){
    zventdata *zvi;

    zvi = global_scase.zventinfo + n;

    zvi->x0 = FDS2SMV_X(zvi->x0);
    zvi->x1 = FDS2SMV_X(zvi->x1);
    zvi->y0 = FDS2SMV_Y(zvi->y0);
    zvi->y1 = FDS2SMV_Y(zvi->y1);
    zvi->z0 = FDS2SMV_Z(zvi->z0);
    zvi->z1 = FDS2SMV_Z(zvi->z1);
  }

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    float *offset;

    meshi=global_scase.meshescoll.meshinfo + i;
    offset = meshi->offset;
    for(n=0;n<meshi->nvents+12;n++){
      ventdata *vi;

      vi = meshi->ventinfo+n;
      vi->xvent1plot=FDS2SMV_X(offset[XXX]+vi->xvent1);
      vi->xvent2plot=FDS2SMV_X(offset[XXX]+vi->xvent2);
      vi->yvent1plot=FDS2SMV_Y(offset[YYY]+vi->yvent1);
      vi->yvent2plot=FDS2SMV_Y(offset[YYY]+vi->yvent2);
      vi->zvent1plot=FDS2SMV_Z(offset[ZZZ]+vi->zvent1);
      vi->zvent2plot=FDS2SMV_Z(offset[ZZZ]+vi->zvent2);
    }
  }
  UpdateVentOffset();
  if(global_scase.smoke3dcoll.nsmoke3dinfo>0)NewMemory((void **)&global_scase.smoke3dcoll.smoke3dinfo_sorted,global_scase.smoke3dcoll.nsmoke3dinfo*sizeof(smoke3ddata *));
  NewMemory((void **)&meshvisptr,global_scase.meshescoll.nmeshes*sizeof(int));
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshvisptr[i]=1;
  }
  for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    global_scase.smoke3dcoll.smoke3dinfo_sorted[i]=global_scase.smoke3dcoll.smoke3dinfo+i;
  }
  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    float dx, dy, dz;

    meshi = global_scase.meshescoll.meshinfo+i;

    dx = meshi->xplt_orig[1]-meshi->xplt_orig[0];
    dy = meshi->yplt_orig[1]-meshi->yplt_orig[0];
    dz = meshi->zplt_orig[1]-meshi->zplt_orig[0];

    meshi->dxyz_orig[0] = dx;
    meshi->dxyz_orig[1] = dy;
    meshi->dxyz_orig[2] = dz;

    meshi->dxDdx  = 1.0;
    meshi->dyDdx  = dy/dx;
    meshi->dzDdx  = dz/dx;
    meshi->dxyDdx = dy/sqrt(dx*dx+dy*dy);
    meshi->dxzDdx = dz/sqrt(dx*dx+dz*dz);
    meshi->dyzDdx = (dy*dz/dx)/sqrt(dy*dy+dz*dz);

    meshi->smoke_dist[ALPHA_X]  = dx;
    meshi->smoke_dist[ALPHA_Y]  = dy;
    meshi->smoke_dist[ALPHA_Z]  = dz;
    meshi->smoke_dist[ALPHA_XY] = dx*dy/sqrt(dx*dx + dy*dy);
    meshi->smoke_dist[ALPHA_YZ] = dy*dz/sqrt(dy*dy + dz*dz);
    meshi->smoke_dist[ALPHA_XZ] = dx*dz/sqrt(dx*dx + dz*dz);
  }
}

/* ------------------ IsSliceDup ------------------------ */

int IsSliceDup(smv_case *scase, slicedata *sd, int nslice){
  int i;

  for(i=0;i<nslice-1;i++){
    slicedata *slicei;

    slicei = scase->slicecoll.sliceinfo + i;
    if(slicei->ijk_min[0]!=sd->ijk_min[0]||slicei->ijk_max[0]!=sd->ijk_max[0])continue;
    if(slicei->ijk_min[1]!=sd->ijk_min[1]||slicei->ijk_max[1]!=sd->ijk_max[1])continue;
    if(slicei->ijk_min[2]!=sd->ijk_min[2]||slicei->ijk_max[2]!=sd->ijk_max[2])continue;
    if(strcmp(slicei->label.longlabel,sd->label.longlabel)!=0)continue;
    if(slicei->slice_filetype!=sd->slice_filetype)continue;
    if(slicei->blocknumber!=sd->blocknumber)continue;
    if(slicei->volslice!=sd->volslice)continue;
    if(slicei->idir!=sd->idir)continue;
    return 1;
  }
  return 0;
}

/* ------------------ CreateNullLabel ------------------------ */

int CreateNullLabel(flowlabels *flowlabel){
  char buffer[255];
  size_t len;

  strcpy(buffer, "Particles");
  TrimBack(buffer);
  len = strlen(buffer);
  if(NewMemory((void **)&flowlabel->longlabel, (unsigned int)(len + 1)) == 0)return 2;
  STRCPY(flowlabel->longlabel, buffer);

  strcpy(buffer, "Particles");
  TrimBack(buffer);
  len = strlen(buffer);
  if(NewMemory((void **)&flowlabel->shortlabel, (unsigned int)(len + 1)) == 0)return 2;
  STRCPY(flowlabel->shortlabel, buffer);

  strcpy(buffer, "Particles");
  TrimBack(buffer);
  len = strlen(buffer);
  if(NewMemory((void *)&flowlabel->unit, (unsigned int)(len + 1)) == 0)return 2;
  STRCPY(flowlabel->unit, buffer);
  return 0;
}

/// @brief Given a case, find the first instance of a surface that has that
/// label.
/// @param scase The case
/// @param label The label to search for
/// @return An offset into scase->surfcoll.nsurfinfo of the first
/// matching surface. Returns -1 if there are no matching props.
surfdata *GetSurface(smv_case *scase, const char *label){
  int i;

  for(i = 0; i < scase->surfcoll.nsurfinfo; i++){
    surfdata *surfi;

    surfi = scase->surfcoll.surfinfo + i;
    if(strcmp(surfi->surfacelabel, label) == 0)return surfi;
  }
  return global_scase.surfacedefault;
}

/* ------------------ InitObst ------------------------ */

void InitObst(smv_case *scase, blockagedata *bc, surfdata *surf, int index, int meshindex){
  int colorindex, blocktype;
  int i;
  char blocklabel[255];
  size_t len;

  int ind_default[] = {-1, -1, -1, -1, -1, -1};

  memcpy(bc->patch_face_index, ind_default, 6 * sizeof(int));
  bc->prop = NULL;
  bc->is_wuiblock = 0;
  bc->transparent = 0;
  bc->usecolorindex = 0;
  bc->colorindex = -1;
  bc->nshowtime = 0;
  bc->hole = 0;
  bc->showtime = NULL;
  bc->showhide = NULL;
  bc->showtimelist = NULL;
  bc->show = 1;
  bc->blockage_id = index;
  bc->meshindex = meshindex;
  bc->hidden = 0;
  bc->invisible = 0;
  bc->texture_origin[0] = scase->texture_origin[0];
  bc->texture_origin[1] = scase->texture_origin[1];
  bc->texture_origin[2] = scase->texture_origin[2];

  /*
  new OBST format:
  i1 i2 j1 j2 k1 k2 colorindex blocktype       : if blocktype!=1&&colorindex!=-3
  i1 i2 j1 j2 k1 k2 colorindex blocktype r g b : if blocktype!=1&&colorindex==-3
  i1 i2 j1 j2 k1 k2 ceiling texture blocktype [wall texture floor texture] : if blocktype==1
  int colorindex, blocktype;
  colorindex: -1 default color
  -2 invisible
  -3 use r g b color
  >=0 color/color2/texture index
  blocktype: 0 regular block non-textured
  1 regular block textured
  2 outline
  3 smoothed block
  r g b           colors used if colorindex==-3
  */
  colorindex = -1;
  blocktype = 0;
  bc->ijk[IMIN] = 0;
  bc->ijk[IMAX] = 1;
  bc->ijk[JMIN] = 0;
  bc->ijk[JMAX] = 1;
  bc->ijk[KMIN] = 0;
  bc->ijk[KMAX] = 1;
  bc->colorindex = colorindex;
  bc->type = blocktype;

  bc->del = 0;
  bc->changed = 0;
  bc->changed_surface = 0;
  bc->walltype = WALL_1;

  bc->color = surf->color;
  bc->useblockcolor = 0;
  for(i = 0; i<6; i++){
    bc->surf_index[i] = -1;
    bc->surf[i] = surf;
    bc->faceinfo[i] = NULL;
  }
  sprintf(blocklabel, "**blockage %i", index);
  len = strlen(blocklabel);
  NewMemory((void **)&bc->label, ((unsigned int)(len + 1))*sizeof(char));
  strcpy(bc->label, blocklabel);
}

/* ------------------ InitSurface ------------------------ */

void InitSurface(surfdata *surf){
  surf->in_color_dialog = 0;
  surf->iso_level = -1;
  surf->used_by_obst = 0;
  surf->used_by_geom = 0;
  surf->in_geom_list = -1;
  surf->geom_area = -1.0;
  surf->used_by_vent = 0;
  surf->emis = 1.0;
  surf->temp_ignition = TEMP_IGNITION_MAX;
  surf->surfacelabel = NULL;
  surf->texturefile = NULL;
  surf->textureinfo = NULL;
  surf->color = block_ambient2;
  surf->t_width = 1.0;
  surf->t_height = 1.0;
  surf->type = BLOCK_regular;
  surf->obst_surface = 1;
  surf->location = 0;
  surf->invisible = 0;
  surf->transparent = 0;
}

/* ------------------ InitVentSurface ------------------------ */

void InitVentSurface(surfdata *surf){
  surf->emis = 1.0;
  surf->temp_ignition = TEMP_IGNITION_MAX;
  surf->surfacelabel = NULL;
  surf->texturefile = NULL;
  surf->textureinfo = NULL;
  surf->color = ventcolor;
  surf->t_width = 1.0;
  surf->t_height = 1.0;
  surf->type = BLOCK_outline;
  surf->obst_surface = 0;

}

/* ----------------------- ReadDeviceHeader ----------------------------- */
#define BUFFER_LEN 255

void ReadDeviceHeader(smv_case *scase, char *file, devicedata *devices, int ndevices){
  FILE *stream;
  devicedata *devicecopy;
  char buffer[BUFFER_LEN], *comma;
  int buffer_len = BUFFER_LEN;

  if(file == NULL)return;
  stream = fopen(file, "r");
  if(stream == NULL)return;

  devicecopy = devices;

  while(!feof(stream)){
    fgets(buffer, buffer_len, stream);
    comma = strchr(buffer, ',');
    if(comma != NULL)*comma = 0;
    TrimBack(buffer);
    if(strcmp(buffer, "//DATA") == 0){
      break;
    }
    if(strcmp(buffer, "DEVICE") == 0){
      ParseDevicekeyword2(scase, stream, devicecopy);
      devicecopy++;
    }
  }
  fclose(stream);
}

/* ------------------ SetSurfaceIndex ------------------------ */

void SetSurfaceIndex(smv_case *scase, blockagedata *bc){
  int i, j, jj;
  surfdata *surfj;
  char *surflabel, *bclabel;
  int *surf_index;
  int wall_flag;

  for(i = 0; i < 6; i++){
    bc->surf_index[i] = -1;
    bclabel = bc->surf[i]->surfacelabel;
    if(bc->surf[i] == NULL)continue;
    for(jj = 1; jj < scase->surfcoll.nsurfinfo + 1; jj++){
      j = jj;
      if(jj == scase->surfcoll.nsurfinfo)j = 0;
      surfj = scase->surfcoll.surfinfo + j;
      surflabel = surfj->surfacelabel;
      if(strcmp(bclabel, surflabel) != 0)continue;
      bc->surf_index[i] = j;
      break;
    }
  }
  surf_index = bc->surf_index;
  wall_flag = 1;
  for(i = 1; i < 6; i++){
    if(surf_index[i] != surf_index[0]){
      wall_flag = 0;
      break;
    }
  }
  if(wall_flag == 1){
    bc->walltype = WALL_1;
    bc->walltypeORIG = WALL_1;
    return;
  }
  if(
    surf_index[UP_X] == surf_index[DOWN_X] &&
    surf_index[DOWN_Y] == surf_index[DOWN_X] &&
    surf_index[UP_Y] == surf_index[DOWN_X]
    ){
    bc->walltype = WALL_3;
    bc->walltypeORIG = WALL_3;
    return;
  }
  bc->walltype = WALL_6;
  bc->walltypeORIG = WALL_6;

}

/* ------------------ SurfIdCompare ------------------------ */

#ifdef __linux__
int SurfIdCompare(const void *arg1, const void *arg2, void *surfinfo) {
#else // assumed to be osx or windows
int SurfIdCompare(void *surfinfo, const void *arg1, const void *arg2) {
#endif
  int i = *(int *)arg1;
  int j = *(int *)arg2;

  surfdata *surfi = (surfdata *)surfinfo+i;
  surfdata *surfj = (surfdata *)surfinfo+j;

  return(strcmp(surfi->surfacelabel, surfj->surfacelabel));
}

/* ------------------ UpdateSortedSurfIdList ------------------------ */

void UpdateSortedSurfIdList(surf_collection *surfcoll){
  int i;

  FREEMEMORY(surfcoll->sorted_surfidlist);
  FREEMEMORY(surfcoll->inv_sorted_surfidlist);
  NewMemory((void **)&surfcoll->sorted_surfidlist, surfcoll->nsurfinfo*sizeof(int));
  NewMemory((void **)&surfcoll->inv_sorted_surfidlist, surfcoll->nsurfinfo*sizeof(int));


  surfcoll->nsorted_surfidlist = surfcoll->nsurfinfo;
  for(i = 0; i<surfcoll->nsorted_surfidlist; i++){
    surfcoll->sorted_surfidlist[i] = i;
  }
// Sort surfaces by name in a separate list. Each platform has sort-with-context
// (qsort_s, qsort_r) with either a different name or the arguments in a
// different order. They are functionally the same.
#ifdef _WIN32
  qsort_s(surfcoll->sorted_surfidlist, (size_t)surfcoll->nsurfinfo, sizeof(int), SurfIdCompare,(void *)surfcoll->surfinfo);
#elif __linux__
  qsort_r(surfcoll->sorted_surfidlist, (size_t)surfcoll->nsurfinfo, sizeof(int), SurfIdCompare,(void *)surfcoll->surfinfo);
#else // assumed to be osx
  qsort_r(surfcoll->sorted_surfidlist, (size_t)surfcoll->nsurfinfo, sizeof(int), (void *)surfcoll->surfinfo, SurfIdCompare);
#endif
  for(i = 0; i<surfcoll->nsorted_surfidlist; i++){
    surfcoll->inv_sorted_surfidlist[surfcoll->sorted_surfidlist[i]] = i;
  }
}


/* ------------------ ParseDatabase ------------------------ */
// TODO: this needs to be renamed as it never actually parses a database
void ParseDatabase(smv_case *scase, char *file){
  FILE *stream;
  char buffer[1000], *buffer2 = NULL, *buffer3, *slashptr;
  size_t lenbuffer, lenbuffer2;
  size_t sizebuffer2;
  char *surf_id = NULL, *start, *surf_id2;
  char *c;
  int i, j;
  surfdata *surfj;
  char *labeli, *labelj;
  int nexti;
  int nsurfids_shown;

  /* free memory called before */

  for(i = 0; i<scase->surfcoll.nsurfids; i++){
    surf_id = scase->surfcoll.surfids[i].label;
    FREEMEMORY(surf_id);
  }
  FREEMEMORY(scase->surfcoll.surfids);
  scase->surfcoll.nsurfids = 0;


  if(file==NULL||strlen(file)==0||(stream = fopen(file, "r"))==NULL){
    NewMemory((void **)&scase->surfcoll.surfids, (scase->surfcoll.nsurfids+1)*sizeof(surfid));
    surf_id = NULL;
    NewMemory((void **)&surf_id, 6);
    strcpy(surf_id, "INERT");
    scase->surfcoll.surfids[0].label = surf_id;
    scase->surfcoll.surfids[0].location = 0;
    scase->surfcoll.surfids[0].show = 1;
    scase->surfcoll.nsurfids = 1;
  }

  else{
    sizebuffer2 = 1001;
    NewMemory((void **)&buffer2, sizebuffer2);

    /* find out how many surfs are in database file so memory can be allocated */

    while(!feof(stream)){
      if(fgets(buffer, 1000, stream)==NULL)break;
      if(STRSTR(buffer, "&SURF")==NULL)continue;


      slashptr = strstr(buffer, "/");
      if(slashptr!=NULL)strcpy(buffer2, buffer);
      buffer3 = buffer;
      while(slashptr!=NULL){
        fgets(buffer, 1000, stream);
        lenbuffer = strlen(buffer);
        lenbuffer2 = strlen(buffer2);
        if(lenbuffer2+lenbuffer+2>sizebuffer2){
          sizebuffer2 = lenbuffer2+lenbuffer+2+1000;
          ResizeMemory((void **)&buffer2, (unsigned int)sizebuffer2);
        }
        strcat(buffer2, buffer);
        slashptr = strstr(buffer, "/");
        buffer3 = buffer2;
      }
      start = STRSTR(buffer3, "ID");
      if(start!=NULL)scase->surfcoll.nsurfids++;
    }

    /* allocate memory */

    NewMemory((void **)&scase->surfcoll.surfids, (scase->surfcoll.nsurfids+1)*sizeof(surfid));
    surf_id = NULL;
    NewMemory((void **)&surf_id, 6);
    strcpy(surf_id, "INERT");
    scase->surfcoll.surfids[0].label = surf_id;
    scase->surfcoll.surfids[0].location = 0;
    scase->surfcoll.surfids[0].show = 1;


    /* now look for IDs and copy them into an array */

    rewind(stream);
    scase->surfcoll.nsurfids = 1;
    while(!feof(stream)){
      if(fgets(buffer, 1000, stream)==NULL)break;
      if(STRSTR(buffer, "&SURF")==NULL)continue;


      slashptr = strstr(buffer, "/");
      if(slashptr!=NULL)strcpy(buffer2, buffer);
      buffer3 = buffer2;
      while(slashptr!=NULL){
        fgets(buffer, 1000, stream);
        lenbuffer = strlen(buffer);
        lenbuffer2 = strlen(buffer2);
        if(lenbuffer2+lenbuffer+2>sizebuffer2){
          sizebuffer2 = lenbuffer2+lenbuffer+2+1000;
          ResizeMemory((void **)&buffer2, (unsigned int)sizebuffer2);
        }
        strcat(buffer2, buffer);
        slashptr = strstr(buffer, "/");
        buffer3 = buffer2;
      }
      start = STRSTR(buffer3+3, "ID");
      if(start!=NULL)scase->surfcoll.nsurfids++;
      surf_id = NULL;
      surf_id2 = NULL;
      for(c = start; c!=NULL&&*c!='\0'; c++){
        if(surf_id==NULL&&*c=='\''){
          surf_id = c+1;
          continue;
        }
        if(surf_id!=NULL&&*c=='\''){
          *c = '\0';
          NewMemory((void **)&surf_id2, strlen(surf_id)+1);
          strcpy(surf_id2, surf_id);
          scase->surfcoll.surfids[ scase->surfcoll.nsurfids-1].label = surf_id2;
          scase->surfcoll.surfids[ scase->surfcoll.nsurfids-1].location = 1;
          scase->surfcoll.surfids[ scase->surfcoll.nsurfids-1].show = 1;
          break;
        }
      }

    }
  }

  /* identify duplicate surfaces */
  /*** debug: make sure ->show is defined for all cases ***/

  nsurfids_shown = 0;
  for(i = 0; i<scase->surfcoll.nsurfids; i++){
    labeli = scase->surfcoll.surfids[i].label;
    nexti = 0;
    for(j = 0; j<scase->surfcoll.nsurfinfo; j++){
      surfj = scase->surfcoll.surfinfo+j;
      labelj = surfj->surfacelabel;
      if(strcmp(labeli, labelj)==0){
        nexti = 1;
        break;
      }
    }
    if(nexti==1){
      scase->surfcoll.surfids[i].show = 0;
      continue;
    }
    for(j = 0; j<i; j++){
      labelj = scase->surfcoll.surfids[j].label;
      if(strcmp(labeli, labelj)==0){
        nexti = 1;
        break;
      }
    }
    if(nexti==1){
      scase->surfcoll.surfids[i].show = 0;
      continue;
    }
    nsurfids_shown++;

  }

  /* add surfaces found in database to those surfaces defined in previous SURF lines */

  if(nsurfids_shown>0){
    if(scase->surfcoll.nsurfinfo==0){
      FREEMEMORY(scase->surfcoll.surfinfo);
      FREEMEMORY(scase->texture_coll.textureinfo);
      NewMemory((void **)&scase->surfcoll.surfinfo, (nsurfids_shown+MAX_ISO_COLORS+1)*sizeof(surfdata));
      NewMemory((void **)&scase->texture_coll.textureinfo, nsurfids_shown*sizeof(texturedata));
    }
    if(scase->surfcoll.nsurfinfo>0){
      if(scase->surfcoll.surfinfo==NULL){
        NewMemory((void **)&scase->surfcoll.surfinfo, (nsurfids_shown+scase->surfcoll.nsurfinfo+MAX_ISO_COLORS+1)*sizeof(surfdata));
      }
      else{
        ResizeMemory((void **)&scase->surfcoll.surfinfo, (nsurfids_shown+scase->surfcoll.nsurfinfo+MAX_ISO_COLORS+1)*sizeof(surfdata));
      }
      if(scase->texture_coll.textureinfo==NULL){
        NewMemory((void **)&scase->texture_coll.textureinfo, (nsurfids_shown+scase->surfcoll.nsurfinfo)*sizeof(texturedata));
      }
      else{
        ResizeMemory((void **)&scase->texture_coll.textureinfo, (nsurfids_shown+scase->surfcoll.nsurfinfo)*sizeof(texturedata));
      }
    }
    surfj = scase->surfcoll.surfinfo+scase->surfcoll.nsurfinfo-1;
    for(j = 0; j<scase->surfcoll.nsurfids; j++){
      if(scase->surfcoll.surfids[j].show==0)continue;
      surfj++;
      InitSurface(surfj);
      surfj->surfacelabel = scase->surfcoll.surfids[j].label;
    }
    scase->surfcoll.nsurfinfo += nsurfids_shown;
  }
  UpdateSortedSurfIdList(&scase->surfcoll);
}

/* ------------------ ReadZVentData ------------------------ */

void ReadZVentData(smv_case *scase, zventdata *zvi, char *buffer, int flag){
  float dxyz[3];
  float xyz[6];
  float color[4];
  roomdata *roomi;
  int roomfrom=-1, roomto=-1;
  int vertical_vent_type=0;
  float area_fraction = 1.0;

  color[0]=1.0;
  color[1]=0.0;
  color[2]=1.0;
  color[3]=1.0;
  if(flag==ZVENT_2ROOM){
    sscanf(buffer, "%i %i %f %f %f %f %f %f %i",
      &roomfrom, &roomto, xyz, xyz + 1, xyz + 2, xyz + 3, xyz + 4, xyz + 5,
      &vertical_vent_type
  );
  }
  else{
    sscanf(buffer, "%i %f %f %f %f %f %f",
      &roomfrom, xyz, xyz + 1, xyz + 2, xyz + 3, xyz + 4, xyz + 5);
    roomto = roomfrom;
  }

  if(roomfrom<1 || roomfrom>scase->nrooms)roomfrom = scase->nrooms + 1;
  roomi = scase->roominfo + roomfrom - 1;
  zvi->room1 = roomi;
  if(roomto<1 || roomto>scase->nrooms)roomto = scase->nrooms + 1;
  zvi->room2 = scase->roominfo + roomto - 1;
  zvi->x0 = roomi->x0 + xyz[0];
  zvi->x1 = roomi->x0 + xyz[1];
  zvi->y0 = roomi->y0 + xyz[2];
  zvi->y1 = roomi->y0 + xyz[3];
  zvi->z0 = roomi->z0 + xyz[4];
  zvi->z1 = roomi->z0 + xyz[5];
  zvi->xcen = (zvi->x0 + zvi->x1)/2.0;
  zvi->ycen = (zvi->y0 + zvi->y1)/2.0;
  dxyz[0] = ABS(xyz[0] - xyz[1]);
  dxyz[1] = ABS(xyz[2] - xyz[3]);
  dxyz[2] = ABS(xyz[4] - xyz[5]);
  zvi->area = 1.0;
  zvi->vertical_vent_type = vertical_vent_type;
  if(dxyz[0] > 0.0)zvi->area *= dxyz[0];
  if(dxyz[1] > 0.0)zvi->area *= dxyz[1];
  if(dxyz[2] > 0.0)zvi->area *= dxyz[2];
  zvi->radius = sqrt(zvi->area/PI);

  // see which side of room vent is closest too
  if(dxyz[0] < MIN(dxyz[1], dxyz[2])){
    if(ABS(zvi->x0 - roomi->x0) < ABS(zvi->x0 - roomi->x1)){
      zvi->wall = LEFT_WALL;
    }
    else{
      zvi->wall = RIGHT_WALL;
    }
  }
  else if(dxyz[1] < MIN(dxyz[0], dxyz[2])){
    if(ABS(zvi->y0 - roomi->y0) < ABS(zvi->y0 - roomi->y1)){
      zvi->wall = FRONT_WALL;
    }
    else{
      zvi->wall = BACK_WALL;
    }
  }
  else{
    zvi->wall = BOTTOM_WALL;
    if(ABS(zvi->z0 - roomi->z0) < ABS(zvi->z0 - roomi->z1)){
      zvi->wall = BOTTOM_WALL;
    }
    else{
      zvi->wall = TOP_WALL;
    }
  }
  zvi->color = GetColorPtr(&global_scase, color);
  zvi->area_fraction = area_fraction;
}

#define CELLMESH_FACTOR 8
#define IJKCELLINFO(ii,jj,kk) ((ii) + (jj)*nxyz[0] + (kk)*nxyz[0]*nxyz[1])

/* ------------------ InitCellMeshInfo ------------------------ */

void InitCellMeshInfo(void){
  int i, *nxyz, ntotal;
  float *xyzminmax, *dxyz;
  float *x, *y, *z;
  meshdata **cellmeshes;

  if(cellmeshinfo!=NULL){
    nxyz = cellmeshinfo->nxyz;
    ntotal = nxyz[0]*nxyz[1]*nxyz[2];
    for(i=0;i<ntotal;i++){
      if(cellmeshinfo->cellmeshes[i] == NULL){
        is_convex = 0;
        return;
      }
    }
    is_convex = 1;
    return;
  }

  NewMemory((void **)&cellmeshinfo, sizeof(cellmeshdata));
  xyzminmax = cellmeshinfo->xyzminmax;
  dxyz      = cellmeshinfo->dxyz;
  nxyz      = cellmeshinfo->nxyz;

  x = global_scase.meshescoll.meshinfo->xplt_orig;
  y = global_scase.meshescoll.meshinfo->yplt_orig;
  z = global_scase.meshescoll.meshinfo->zplt_orig;

  xyzminmax[0] = x[0];
  xyzminmax[1] = x[global_scase.meshescoll.meshinfo->ibar];
  xyzminmax[2] = y[0];
  xyzminmax[3] = y[global_scase.meshescoll.meshinfo->jbar];
  xyzminmax[4] = z[0];
  xyzminmax[5] = z[global_scase.meshescoll.meshinfo->kbar];
  dxyz[0] = x[global_scase.meshescoll.meshinfo->ibar] - x[0];
  dxyz[1] = y[global_scase.meshescoll.meshinfo->jbar] - y[0];
  dxyz[2] = z[global_scase.meshescoll.meshinfo->kbar] - z[0];

  for(i = 1; i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    x = meshi->xplt_orig;
    y = meshi->yplt_orig;
    z = meshi->zplt_orig;

    xyzminmax[0] = MIN(xyzminmax[0], x[0]);
    xyzminmax[1] = MAX(xyzminmax[1], x[global_scase.meshescoll.meshinfo->ibar]);
    xyzminmax[2] = MIN(xyzminmax[2], y[0]);
    xyzminmax[3] = MAX(xyzminmax[3], y[global_scase.meshescoll.meshinfo->jbar]);
    xyzminmax[4] = MIN(xyzminmax[4], z[0]);
    xyzminmax[5] = MAX(xyzminmax[5], z[global_scase.meshescoll.meshinfo->kbar]);
    dxyz[0] = MIN(dxyz[0], x[global_scase.meshescoll.meshinfo->ibar] - x[0]);
    dxyz[1] = MIN(dxyz[1], y[global_scase.meshescoll.meshinfo->jbar] - y[0]);
    dxyz[2] = MIN(dxyz[2], z[global_scase.meshescoll.meshinfo->kbar] - z[0]);
  }
  dxyz[0] /= (float)CELLMESH_FACTOR;
  dxyz[1] /= (float)CELLMESH_FACTOR;
  dxyz[2] /= (float)CELLMESH_FACTOR;
  nxyz[0] = MAX((int)( (xyzminmax[1] - xyzminmax[0])/dxyz[0] + 0.5), 1);
  nxyz[1] = MAX((int)( (xyzminmax[3] - xyzminmax[2])/dxyz[1] + 0.5), 1);
  nxyz[2] = MAX((int)( (xyzminmax[5] - xyzminmax[4])/dxyz[2] + 0.5), 1);

  ntotal = nxyz[0]*nxyz[1]*nxyz[2];
  NewMemory((void **)&cellmeshinfo->cellmeshes, ntotal*sizeof(meshdata *));
  cellmeshes = cellmeshinfo->cellmeshes;
  for(i=0;i<ntotal;i++){
    cellmeshinfo->cellmeshes[i] = NULL;
  }
  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int i1, i2, j1, j2, k1, k2;
    float xmin, xmax, ymin, ymax, zmin, zmax;

    meshi = global_scase.meshescoll.meshinfo + i;
    x = meshi->xplt_orig;
    y = meshi->yplt_orig;
    z = meshi->zplt_orig;
    xmin = x[0];
    xmax = x[meshi->ibar];
    ymin = y[0];
    ymax = y[meshi->jbar];
    zmin = z[0];
    zmax = z[meshi->kbar];
    i1 = CLAMP(nxyz[0]*(xmin - xyzminmax[0])/dxyz[0], 0, nxyz[0]-1);
    i2 = CLAMP(nxyz[0]*(xmax - xyzminmax[0])/dxyz[0], 0, nxyz[0]-1);
    j1 = CLAMP(nxyz[1]*(ymin - xyzminmax[2])/dxyz[1], 0, nxyz[1]-1);
    j2 = CLAMP(nxyz[1]*(ymax - xyzminmax[2])/dxyz[1], 0, nxyz[1]-1);
    k1 = CLAMP(nxyz[2]*(zmin - xyzminmax[4])/dxyz[2], 0, nxyz[2]-1);
    k2 = CLAMP(nxyz[2]*(zmax - xyzminmax[4])/dxyz[2], 0, nxyz[2]-1);
    int ii, jj, kk;

    for(kk = k1; kk <= k2; kk++){
      for(jj = j1; jj <= j2; jj++){
        for(ii = i1; ii <= i2; ii++){
          cellmeshes[IJKCELLINFO(ii,jj,kk)] = meshi;
        }
      }
    }
  }
  for(i=0;i<ntotal;i++){
    if(cellmeshinfo->cellmeshes[i] == NULL){
      is_convex = 0;
      return;
    }
  }
  is_convex = 1;
  return;
}

/* ------------------ SetupMeshWalls ------------------------ */

void SetupMeshWalls(void){
  int i;
  if(cellmeshinfo == NULL)InitCellMeshInfo();

  for(i = 0; i < global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    float xyz[3], *bmin, *bmax, bmid[3];
    int *is_extface;

    meshi = global_scase.meshescoll.meshinfo + i;
    bmin = meshi->boxmin;
    bmax = meshi->boxmax;
    is_extface = meshi->is_extface;

    bmid[0] = (bmin[0] + bmax[0]) / 2.0;
    bmid[1] = (bmin[1] + bmax[1]) / 2.0;
    bmid[2] = (bmin[2] + bmax[2]) / 2.0;

#define EPSMESH 0.001
    xyz[0] = bmin[0] - EPSMESH;
    xyz[1] = bmid[1];
    xyz[2] = bmid[2];
    if(InExterior(xyz) == 0)is_extface[0] = 0;

    xyz[0] = bmax[0] + EPSMESH;
    xyz[1] = bmid[1];
    xyz[2] = bmid[2];
    if(InExterior(xyz) == 0)is_extface[1] = 0;

    xyz[0] = bmid[0];
    xyz[1] = bmin[1] - EPSMESH;
    xyz[2] = bmid[2];
    if(InExterior(xyz) == 0)is_extface[2] = 0;

    xyz[0] = bmid[0];
    xyz[1] = bmax[1] + EPSMESH;
    xyz[2] = bmid[2];
    if(InExterior(xyz) == 0)is_extface[3] = 0;

    xyz[0] = bmid[0];
    xyz[1] = bmid[1];
    xyz[2] = bmin[2] - EPSMESH;
    if(InExterior(xyz) == 0)is_extface[4] = 0;

    xyz[0] = bmid[0];
    xyz[1] = bmid[1];
    xyz[2] = bmax[2] + EPSMESH;
    if(InExterior(xyz) == 0)is_extface[5] = 0;
  }
}

/// @brief Set the file lists of a case. That is, lists of files associated with the given case.
/// @param[inout] scase The case
void MakeFileLists(smv_case *scase){
  char filter_casedir[256], filter_casename[256];

  // create list of all files for the case being visualized (casename*.* )

  strcpy(filter_casename, "");
  if(scase->fdsprefix != NULL&&strlen(scase->fdsprefix) > 0){
    strcat(filter_casename, scase->fdsprefix);
    strcat(filter_casename, "*");
  }

  // create a list of all files in the current directory

  scase->filelist_coll.nfilelist_casename = GetFileListSize(".", filter_casename, FILE_MODE);
  MakeFileList(".", filter_casename, scase->filelist_coll.nfilelist_casename, YES, &scase->filelist_coll.filelist_casename, FILE_MODE);

  strcpy(filter_casedir, "");
  scase->filelist_coll.nfilelist_casedir = GetFileListSize(".", filter_casedir, FILE_MODE);
  MakeFileList(".", filter_casedir, scase->filelist_coll.nfilelist_casedir, YES, &scase->filelist_coll.filelist_casedir, FILE_MODE);
}

#define RETURN_TWO        2
#define RETURN_BREAK      3
#define RETURN_CONTINUE   4
#define RETURN_PROCEED    5

/* ------------------ SetupIsosurface ------------------------ */

void SetupIsosurface(isodata *isoi){
  if(isoi->geomflag == 1){
    int ntimes_local;
    geomdata *geomi;
    float **colorlevels, *levels;

    geomi = isoi->geominfo;
    geomi->file = isoi->file;
    geomi->topo_file = isoi->topo_file;
    geomi->file = isoi->file;
    ReadGeomHeader(geomi, NULL, &ntimes_local);
    isoi->nlevels = geomi->nfloat_vals;
    if(isoi->nlevels > 0){
      int i;

      NewMemory((void **)&levels, isoi->nlevels * sizeof(float));
      NewMemory((void **)&colorlevels, isoi->nlevels * sizeof(float *));
      for(i = 0; i < isoi->nlevels; i++){
        colorlevels[i] = NULL;
        levels[i] = geomi->float_vals[i];
      }
      isoi->levels = levels;
      isoi->colorlevels = colorlevels;
    }
  }
  else{
    GetIsoLevels(isoi->file, isoi->dataflag, &isoi->levels, &isoi->colorlevels, &isoi->nlevels);
  }
  if(isoi->get_isolevels==1){
    int len_clevels;
    char clevels[1024];

    Array2String(isoi->levels, isoi->nlevels, clevels);
    len_clevels = strlen(clevels);
    if(len_clevels>0){
      int len_long;
      char *long_label, *unit_label;

      long_label = isoi->surface_label.longlabel;
      unit_label = isoi->surface_label.unit;
      len_long = strlen(long_label)+strlen(unit_label)+len_clevels+3+1;
      if(isoi->dataflag==1)len_long += (strlen(isoi->color_label.longlabel)+15+1);
      ResizeMemory((void **)&long_label, (unsigned int)len_long);
      isoi->surface_label.longlabel = long_label;
      strcat(long_label, ": ");
      strcat(long_label, clevels);
      strcat(long_label, " ");
      strcat(long_label, unit_label);
      if(isoi->dataflag==1){
        strcat(long_label, " (Colored by: ");
        strcat(long_label, isoi->color_label.longlabel);
        strcat(long_label, ")");
      }
      TrimBack(long_label);
    }
  }
}

/* ------------------ SetupAllIsosurfaces ------------------------ */

void *SetupAllIsosurfaces(void *arg){
  int i;

  for(i = 0; i < global_scase.nisoinfo; i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    SetupIsosurface(isoi);
  }
  THREAD_EXIT(isosurface_threads);
}

/* ------------------ ParseISOFCount ------------------------ */

void ParseISOFCount(smv_case *scase){
  if(parse_opts.setup_only == 1 || parse_opts.smoke3d_only == 1)return;
  scase->nisoinfo++;
}

/* ------------------ ParseISOFProcess ------------------------ */

int ParseISOFProcess(smv_case *scase, bufferstreamdata *stream, char *buffer, int *iiso_in, int *ioffset_in, int *nn_iso_in, int nisos_per_mesh_in){
  isodata *isoi;
  int dataflag = 0, geomflag = 0;
  char tbuffer[255], *tbufferptr;
  int blocknumber;
  size_t len;
  char *buffer3, *ext;
  int fds_skip = 1;
  float fds_delta = -1.0;
  char *bufferptr;

  int ioffset, iiso, nn_iso, nisos_per_mesh;

  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)return RETURN_CONTINUE;

  iiso = *iiso_in;
  ioffset = *ioffset_in;
  nn_iso = *nn_iso_in;
  nisos_per_mesh = nisos_per_mesh_in;

  isoi = scase->isoinfo+iiso;
  isoi->isof_index = nn_iso%nisos_per_mesh;
  nn_iso++;
  *nn_iso_in = nn_iso;

  if(Match(buffer, "TISOF")==1||Match(buffer, "TISOG")==1)dataflag = 1;
  if(Match(buffer, "ISOG")==1||Match(buffer, "TISOG")==1)geomflag = 1;
  TrimBack(buffer);
  len = strlen(buffer);

  if(scase->meshescoll.nmeshes>1){
    blocknumber = ioffset-1;
  }
  else{
    blocknumber = 0;
  }
  if(len>5&&dataflag==0){
    buffer3 = buffer+4;
    sscanf(buffer3, "%i %i %f", &blocknumber, &fds_skip, &fds_delta);
    blocknumber--;
  }
  if(len>6&&dataflag==1){
    buffer3 = buffer+5;
    sscanf(buffer3, "%i", &blocknumber);
    blocknumber--;
  }
  if(FGETS(buffer, 255, stream)==NULL){
    scase->nisoinfo--;
    return RETURN_BREAK;
  }

#ifdef pp_ISOFRAME
  isoi->frameinfo = NULL;
#endif
  isoi->fds_skip = fds_skip;
  isoi->fds_delta = fds_delta;
  isoi->tfile = NULL;
  isoi->seq_id = nn_iso;
  isoi->autoload = 0;
  isoi->blocknumber = blocknumber;
  isoi->loaded = 0;
  isoi->loading = 0;
  isoi->display = 0;
  isoi->dataflag = dataflag;
  isoi->geomflag = geomflag;
  isoi->nlevels = 0;
  isoi->levels = NULL;
  isoi->memory_id = ++scase->nmemory_ids;
  isoi->geom_nstatics = NULL;
  isoi->geom_ndynamics = NULL;
  isoi->geom_times = NULL;
  isoi->geom_times_map = NULL;
  isoi->geom_vals = NULL;
  isoi->get_isolevels = 0;

  isoi->normaltable = NULL;
  isoi->color_label.longlabel = NULL;
  isoi->color_label.shortlabel = NULL;
  isoi->color_label.unit = NULL;
  isoi->geominfo = NULL;
  NewMemory((void **)&isoi->geominfo, sizeof(geomdata));
  scase->nmemory_ids++;
  isoi->geominfo->memory_id = scase->nmemory_ids;
  InitGeom(isoi->geominfo, GEOM_ISO, NOT_FDSBLOCK, CFACE_NORMALS_NO,blocknumber);

  bufferptr = TrimFrontBack(buffer);

  len = strlen(bufferptr);

  NewMemory((void **)&isoi->reg_file, (unsigned int)(len+1));
  STRCPY(isoi->reg_file, bufferptr);

  ext = strrchr(bufferptr, '.');
  if(ext!=NULL)*ext = 0;
  NewMemory((void **)&isoi->topo_file, (unsigned int)(strlen(bufferptr)+5+1));
  STRCPY(isoi->topo_file, bufferptr);
  strcat(isoi->topo_file, ".niso");

  NewMemory((void **)&isoi->size_file, (unsigned int)(len+3+1));
  STRCPY(isoi->size_file, bufferptr);
  STRCAT(isoi->size_file, ".sz");

  if(isoi->dataflag==1&&isoi->geomflag==1){
    if(FGETS(tbuffer, 255, stream)==NULL){
      scase->nisoinfo--;
      return RETURN_BREAK;
    }
    TrimBack(tbuffer);
    tbufferptr = TrimFront(tbuffer);
    NewMemory((void **)&isoi->tfile, strlen(tbufferptr)+1);
    strcpy(isoi->tfile, tbufferptr);
  }

  if(parse_opts.fast_startup==1||FileExistsCaseDir(scase, isoi->reg_file)==YES){
    isoi->get_isolevels = 1;
    isoi->file = isoi->reg_file;
    if(ReadLabels(&isoi->surface_label, stream, NULL)==LABEL_ERR)return 2;
    if(isoi->fds_delta>0.0){  // only append delete parameter if it is > 0.0
      char delta_label[100];

      sprintf(delta_label, "%f", isoi->fds_delta);
      TrimZeros(delta_label);
      strcat(isoi->surface_label.longlabel, "(");
      strcat(isoi->surface_label.longlabel, delta_label);
      strcat(isoi->surface_label.longlabel, ")");
    }
    if(isoi->fds_skip!=1){  // only append skip parameter if it is > 1
      char skip_label[100];

      sprintf(skip_label, "/%i", isoi->fds_skip);
      strcat(isoi->surface_label.longlabel, skip_label);
    }
    if(isoi->dataflag==1){
      if(ReadLabels(&isoi->color_label, stream, NULL)==LABEL_ERR)return 2;
    }
    iiso++;
    *iiso_in = iiso;
  }
  else{
    isoi->get_isolevels = 0;
    if(ReadLabels(&isoi->surface_label, stream, NULL)==LABEL_ERR)return 2;
    if(isoi->dataflag==1){
      if(ReadLabels(&isoi->color_label, stream, NULL)==LABEL_ERR)return 2;
    }
    scase->nisoinfo--;
  }
  return RETURN_CONTINUE;
}

#define SCAN    0
#define NO_SCAN 1
/* ------------------ ParseCHIDProcess ------------------------ */

int ParseCHIDProcess(smv_case *scase, bufferstreamdata *stream, int option){
  size_t len;
  char buffer[255], *bufferptr;

  if(option==SCAN){
    for(;;){
      if(FGETS(buffer, 255, stream)==NULL){
        return RETURN_BREAK;
      }
      if(Match(buffer,"CHID") != 1)continue;
      break;
    }
    return RETURN_BREAK;
  }

  if(FGETS(buffer, 255, stream)==NULL){
    return RETURN_BREAK;
  }
  bufferptr = TrimFrontBack(buffer);
  len = strlen(bufferptr);
  FREEMEMORY(scase->paths.chidfilebase);
  NewMemory((void **)&scase->paths.chidfilebase, (unsigned int)(len+1));
  STRCPY(scase->paths.chidfilebase, bufferptr);

  if(scase->paths.chidfilebase!=NULL){
    NewMemory((void **)&scase->paths.hrr_csv_filename, (unsigned int)(strlen(scase->paths.chidfilebase)+8+1));
    STRCPY(scase->paths.hrr_csv_filename, scase->paths.chidfilebase);
    STRCAT(scase->paths.hrr_csv_filename, "_hrr.csv");
    if(FileExistsCaseDir(scase, scase->paths.hrr_csv_filename)==NO){
      FREEMEMORY(scase->paths.hrr_csv_filename);
    }
  }
  return RETURN_CONTINUE;
}

/* ------------------ ReadSMVCHID ------------------------ */

int ReadSMVCHID(bufferstreamdata *stream){
  ParseCHIDProcess(&global_scase, stream, SCAN);
  return 0;
}


/* ------------------ ParsePRTCount ------------------------ */

void ParsePRT5Count(smv_case *scase){
  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)return;
  scase->npartinfo++;
}

/* ------------------ ParsePRT5Process ------------------------ */

int ParsePRT5Process(smv_case *scase, bufferstreamdata *stream, char *buffer, int *nn_part_in, int *ipart_in, int *ioffset_in){
  unsigned int lenkey;
  partdata *parti;
  int blocknumber;
  size_t len;
  char *buffer3, *bufferptr;

  int nn_part, ipart, ioffset;

  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)return RETURN_CONTINUE;

  nn_part = *nn_part_in;
  ioffset = *ioffset_in;
  ipart = *ipart_in;

  nn_part++;
  *nn_part_in = nn_part;
  parti = scase->partinfo+ipart;
  lenkey = 4;
  len = strlen(buffer);
  if(scase->meshescoll.nmeshes>1){
    blocknumber = ioffset-1;
  }
  else{
    blocknumber = 0;
  }
  if(len>lenkey+1){
    buffer3 = buffer+lenkey;
    sscanf(buffer3, "%i", &blocknumber);
    blocknumber--;
  }

#ifdef pp_PARTFRAME
  parti->frameinfo = NULL;
#endif
  parti->blocknumber = blocknumber;
  parti->seq_id = nn_part;
  parti->autoload = 0;
  parti->reload = 0;
  parti->finalize = 1;
  parti->valmin_part = NULL;
  parti->valmax_part = NULL;
  parti->stream     = NULL;
  parti->hist_update = 0;
  parti->skipload = 1;
  if(FGETS(buffer, 255, stream)==NULL){
    scase->npartinfo--;
    return RETURN_BREAK;
  }

  bufferptr = TrimFrontBack(buffer);
  len = strlen(bufferptr);
  parti->reg_file = NULL;
  parti->reg_file   = PARTBUFFER(len+1);
  parti->bound_file = PARTBUFFER(len+4+1);
  parti->size_file  = PARTBUFFER(len+3+1);
  STRCPY(parti->reg_file, bufferptr);
  parti->reg_file_size = GetFileSizeSMV(parti->reg_file);

  STRCPY(parti->bound_file, bufferptr);
  STRCAT(parti->bound_file, ".bnd");
  parti->have_bound_file = NO;

  STRCPY(parti->size_file, bufferptr);
  STRCAT(parti->size_file, ".sz");

  // parti->size_file can't be written to, then put it in a world writable temp directory
  char *smokeview_scratchdir = GetUserConfigDir();
  if(FileExistsCaseDir(scase, parti->size_file)==NO&&scase->curdir_writable==NO&&smokeview_scratchdir!=NULL){
    len = strlen(smokeview_scratchdir)+strlen(bufferptr)+1+3+1;
    parti->size_file = NULL;
    if(NewMemory((void **)&parti->size_file, (unsigned int)len)==0)return RETURN_TWO;
    STRCPY(parti->size_file, smokeview_scratchdir);
    STRCAT(parti->size_file, dirseparator);
    STRCAT(parti->size_file, bufferptr);
    STRCAT(parti->size_file, ".sz");
  }
  FREEMEMORY(smokeview_scratchdir);

  // parti->hist_file can't be written to, then put it in a world writable temp directory

  parti->compression_type = UNCOMPRESSED;
  if(FileExistsCaseDir(scase, parti->reg_file)==YES){
    parti->file = parti->reg_file;
  }
  else{
    parti->file = NULL;
  }
  parti->compression_type = UNCOMPRESSED;
  parti->loaded = 0;
  parti->request_load = 0;
  parti->finalize = 0;
  parti->display = 0;
  parti->times = NULL;
  parti->times_map = NULL;
  parti->timeslist = NULL;
  parti->histograms = NULL;
  parti->bounds_set = 0;
#ifndef pp_PARTFRAME
  parti->filepos = NULL;
#endif
  parti->sort_tags = NULL;
  parti->vis_part = NULL;
  parti->irvals = NULL;

  parti->data5 = NULL;
  parti->partclassptr = NULL;

  FGETS(buffer, 255, stream);
  sscanf(buffer, "%i", &parti->nclasses);
  if(parti->nclasses>0){
    int i;

    NewMemory((void **)&parti->partclassptr, parti->nclasses*sizeof(partclassdata *));
    for(i = 0; i<parti->nclasses; i++){
      int iclass;
      int ic, iii;

      FGETS(buffer, 255, stream);
      if(parti->file==NULL)continue;
      sscanf(buffer, "%i", &iclass);
      if(iclass<1)iclass = 1;
      if(iclass>scase->npartclassinfo)iclass = scase->npartclassinfo;
      ic = 0;
      for(iii = 0; iii<scase->npartclassinfo; iii++){
        partclassdata *pci;

        pci = scase->partclassinfo+iii;
        if(iclass-1==ic){
          parti->partclassptr[i] = pci;
          break;
        }
        ic++;
      }
    }
  }

  // if no classes were specified for the prt5 entry then assign it the default class

  if(parti->file!=NULL&&parti->nclasses==0){
    NewMemory((void **)&parti->partclassptr, sizeof(partclassdata *));
    parti->partclassptr[0] = scase->partclassinfo+parti->nclasses;
  }
  if(parse_opts.fast_startup==1||(parti->file!=NULL&&FileExistsCaseDir(scase, parti->file)==YES)){
    ipart++;
    *ipart_in = ipart;
  }
  else{
    scase->npartinfo--;
  }
  return RETURN_CONTINUE;
}

/* ------------------ ParseBNDFCount ------------------------ */

int ParseBNDFCount(smv_case *scase){
  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)return RETURN_CONTINUE;
  scase->npatchinfo++;
  return RETURN_CONTINUE;
}

/* ------------------ ParseBNDFProcess ------------------------ */

int ParseBNDFProcess(smv_case *scase, bufferstreamdata *stream, char *buffer, int *nn_patch_in, int *ioffset_in, patchdata **patchgeom_in, int *ipatch_in, char buffers[6][256]){
  patchdata *patchi;
  int blocknumber;
  size_t len;
  char *filetype_label;
  int slicegeom = 0;

  int i;
  int nn_patch, ioffset, ipatch;
  patchdata *patchgeom;
  char *bufferptr;

  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1)return RETURN_CONTINUE;

  nn_patch = *nn_patch_in;
  ioffset = *ioffset_in;
  ipatch = *ipatch_in;
  patchgeom = *patchgeom_in;

  if(Match(buffer, "BNDS")==1){
    slicegeom = 1;
  }
  nn_patch++;
  *nn_patch_in = nn_patch;

  TrimBack(buffer);
  len = strlen(buffer);

  if(scase->meshescoll.nmeshes>1){
    blocknumber = ioffset-1;
  }
  else{
    blocknumber = 0;
  }
  if(len>5){
    char *buffer3;
    int version=0;

    buffer3 = buffer+4;
    sscanf(buffer3, "%i %i", &blocknumber, &version);
    blocknumber--;
  }
  if(slicegeom==1){
    patchi = patchgeom;
  }
  else{
    patchi = scase->patchinfo+ipatch;
  }

  for(i = 0; i<6; i++){
    patchi->ijk[i] = -1;
  }
#ifdef pp_BOUNDFRAME
  patchi->frameinfo         = NULL;
#endif
  patchi->finalize          = 1;
  patchi->valmin_patch      = 1.0;
  patchi->valmax_patch      = 0.0;
  patchi->skip              = 0;
  patchi->ntimes            = 0;
  patchi->ntimes_old        = 0;
  patchi->hist_update = 0;
  patchi->filetype_label    = NULL;
  patchi->patchfaceinfo = NULL;
  patchi->patch_filetype    = PATCH_STRUCTURED_NODE_CENTER;
  patchi->structured        = YES;
  patchi->boundary          = 1;
  if(Match(buffer, "BNDC")==1){
    patchi->patch_filetype = PATCH_STRUCTURED_CELL_CENTER;
  }
  if(Match(buffer, "BNDE")==1){
    scase->ngeom_data++;
    patchi->patch_filetype = PATCH_GEOMETRY_BOUNDARY;
    patchi->structured = NO;
  }

  if(Match(buffer, "BNDS")==1){
    char *sliceparms;

    CheckMemory;
    scase->ngeom_data++;
    patchi->patch_filetype = PATCH_GEOMETRY_SLICE;
    patchi->structured = NO;
    patchi->boundary = 0;

    sliceparms = strchr(buffer, '&');
    if(sliceparms!=NULL){
      int ijk[6], j;

      sliceparms++;
      sliceparms[-1] = 0;
      sscanf(sliceparms, "%i %i %i %i %i %i", ijk, ijk+1, ijk+2, ijk+3, ijk+4, ijk+5);
      for(j = 0; j<6; j++){
        patchi->ijk[j] = ijk[j];
      }
    }
    filetype_label = strchr(buffer, '#');
    if(filetype_label!=NULL){
      int len_filetype_label;

      filetype_label++;
      filetype_label[-1] = 0;
      filetype_label = TrimFrontBack(filetype_label);
      len_filetype_label = strlen(filetype_label);
      if(len_filetype_label>0){
        NewMemory((void **)&patchi->filetype_label, (unsigned int)(len_filetype_label+1));
        strcpy(patchi->filetype_label, filetype_label);
      }
    }
    CheckMemory;
  }

  if(slicegeom==1){
    strcpy(buffer, buffers[1]);
  }
  else{
    if(FGETS(buffer, 255, stream)==NULL){
      scase->npatchinfo--;
      return RETURN_BREAK;
    }
  }

  bufferptr = TrimFrontBack(buffer);
  len = strlen(bufferptr);
  NewMemory((void **)&patchi->reg_file, (unsigned int)(len+1));
  STRCPY(patchi->reg_file, bufferptr);

  NewMemory((void **)&patchi->bound_file, (unsigned int)(len+4+1));
  STRCPY(patchi->bound_file, bufferptr);
  strcat(patchi->bound_file, ".bnd");
  patchi->have_bound_file = NO;

  NewMemory((void **)&patchi->comp_file, (unsigned int)(len + 4 + 1));
  STRCPY(patchi->comp_file, bufferptr);
  STRCAT(patchi->comp_file, ".svz");

  NewMemory((void **)&patchi->size_file, (unsigned int)(len+4+1));
  STRCPY(patchi->size_file, bufferptr);
  //      STRCAT(patchi->size_file,".szz"); when we actully use file check both .sz and .szz extensions

  if(parse_opts.lookfor_compressed_files==1&&FileExistsCaseDir(scase, patchi->comp_file) == YES){
    patchi->compression_type = COMPRESSED_ZLIB;
    patchi->file             = patchi->comp_file;
  }
  else{
    patchi->compression_type = UNCOMPRESSED;
    patchi->file             = patchi->reg_file;
  }
  patchi->geominfo = NULL;
  if(patchi->structured==NO){
    int igeom;
    char *geomfile;

    if(slicegeom==1){
      strcpy(buffer, buffers[2]);
    }
    else{
      if(FGETS(buffer, 255, stream)==NULL){
        scase->npatchinfo--;
        return RETURN_BREAK;
      }
    }

    if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY&&scase->ncgeominfo>0){
      patchi->geominfo = scase->cgeominfo+blocknumber;
    }
    else{
      geomfile = TrimFrontBack(buffer);
      for(igeom = 0; igeom<scase->ngeominfo; igeom++){
        geomdata *geomi;

        geomi = scase->geominfo+igeom;
        if(strcmp(geomi->file, geomfile)==0){
          patchi->geominfo = geomi;
          if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY){
            geomi->geomtype = GEOM_BOUNDARY;
            geomi->fdsblock = FDSBLOCK;
          }
          else{
            geomi->geomtype = GEOM_SLICE;
            geomi->fdsblock = NOT_FDSBLOCK;
          }
          break;
        }
      }
    }
  }
  patchi->modtime = 0;
  patchi->geom_timeslist = NULL;
  patchi->geom_offsets = NULL;
  patchi->geom_ivals_dynamic_offset = NULL;
  patchi->geom_ivals_static_offset  = NULL;
  patchi->geom_vals_static_offset   = NULL;
  patchi->geom_vals_dynamic_offset  = NULL;
  patchi->geom_vert2tri = 0;
  patchi->geom_ndynamics = NULL;
  patchi->geom_nstatics = NULL;
  patchi->geom_times = NULL;
  patchi->geom_times_map = NULL;
  patchi->geom_vals = NULL;
  patchi->geom_ivals = NULL;
  patchi->geom_nvals = 0;
  patchi->cvals_offsets = NULL;
  patchi->cvals_sizes = NULL;
  patchi->cbuffer = NULL;
  patchi->cbuffer_size = 0;
  patchi->is_compressed = 0;
  patchi->blocknumber = blocknumber;
  patchi->seq_id = nn_patch;
  patchi->autoload = 0;
  patchi->loaded = 0;
  patchi->display = 0;
  patchi->bounds.defined = 0;
  patchi->setchopmin = 0;
  patchi->chopmin = 1.0;
  patchi->setchopmax = 0;
  patchi->chopmax = 0.0;
  scase->meshescoll.meshinfo[blocknumber].patchfilenum = -1;
  {
    char geomlabel2[256], *geomptr = NULL;

    strcpy(geomlabel2, "");
    if(patchi->patch_filetype==PATCH_STRUCTURED_CELL_CENTER){
      if(ReadLabels(&patchi->label, stream, "(cell centered)")==LABEL_ERR)return RETURN_TWO;
    }
    else if(patchi->patch_filetype==PATCH_STRUCTURED_NODE_CENTER){
      if(ReadLabels(&patchi->label, stream, NULL)==LABEL_ERR)return RETURN_TWO;
    }
    else if(patchi->structured==NO){
      char geomlabel[256];

      strcpy(geomlabel, "(geometry)");
      if(patchi->filetype_label!=NULL){
        if(strcmp(patchi->filetype_label, "EXIMBND_FACES")==0){
          strcat(geomlabel, " - EXIM faces");
          strcpy(geomlabel2, " - EXIM faces");
        }
        if(strcmp(patchi->filetype_label, "CUT_CELLS")==0){
          strcat(geomlabel, " - Cut cell faces");
          strcpy(geomlabel2, " - Cut cell faces");
        }
      }
      if(slicegeom==1){
        if(ReadLabelsBNDS(&patchi->label, NULL, buffers[3], buffers[4], buffers[5], geomlabel)==2)return RETURN_TWO;
      }
      else{
        if(ReadLabels(&patchi->label, stream, geomlabel)==LABEL_ERR)return RETURN_TWO;
      }
    }
    strcpy(patchi->menulabel_base, patchi->label.longlabel);
    if(strlen(geomlabel2)>0){
      geomptr = strstr(patchi->menulabel_base, geomlabel2);
      if(geomptr!=NULL)geomptr[0] = 0;
    }
    if(slicegeom==0){
      ipatch++;
      *ipatch_in = ipatch;
    }
  }
  return RETURN_CONTINUE;
}

/* ------------------ ParseSMOKE3DCount ------------------------ */

void ParseSMOKE3DCount(smv_case *scase){
  if(parse_opts.setup_only==1)return;
  scase->smoke3dcoll.nsmoke3dinfo++;
}

/* ------------------ ParseSMOKE3DProcess ------------------------ */

int ParseSMOKE3DProcess(smv_case *scase, bufferstreamdata *stream, char *buffer, int *nn_smoke3d_in, int *ioffset_in, int *ismoke3dcount_in, int *ismoke3d_in){
  size_t len;
  size_t lenbuffer;
  int filetype = C_GENERATED;
  int blocknumber;
  char buffer2[256];
  char *bufferptr;
  float extinct = -1.0;
  float alpha_factor = 1.0;

  int nn_smoke3d, ioffset, ismoke3dcount, ismoke3d;

  if(parse_opts.setup_only==1)return RETURN_CONTINUE;

  nn_smoke3d    = *nn_smoke3d_in;
  ioffset       = *ioffset_in;
  ismoke3dcount = *ismoke3dcount_in;
  ismoke3d      = *ismoke3d_in;

  if(Match(buffer, "SMOKF3D")==1){
    filetype = FORTRAN_GENERATED;
  }

  nn_smoke3d++;
  *nn_smoke3d_in = nn_smoke3d;

  TrimBack(buffer);
  len = strlen(buffer);
  if(scase->meshescoll.nmeshes>1){
    blocknumber = ioffset-1;
  }
  else{
    blocknumber = 0;
  }
  if(len>8){
    char *buffer3;

    buffer3 = buffer+8;
    sscanf(buffer3, "%i %f %f", &blocknumber, &extinct, &alpha_factor);
    blocknumber--;
  }
  if(FGETS(buffer, 255, stream)==NULL){
    scase->smoke3dcoll.nsmoke3dinfo--;
    return RETURN_BREAK;
  }
  bufferptr = TrimFrontBack(buffer);
  len = strlen(buffer);
  lenbuffer = len;
  {
    smoke3ddata *smoke3di;
    int i;

    smoke3di = scase->smoke3dcoll.smoke3dinfo+ismoke3d;

#ifdef _DEBUG
    if(scase->smoke3dcoll.nsmoke3dinfo>500&&(ismoke3d%100==0||ismoke3d==scase->smoke3dcoll.nsmoke3dinfo-1)){
      PRINTF("     examining %i'st 3D smoke file\n", ismoke3dcount);
    }
#endif
    ismoke3dcount++;
    *ismoke3dcount_in = ismoke3dcount;

    smoke3di->reg_file = SMOKE3DBUFFER(len + 1);
    STRCPY(smoke3di->reg_file, bufferptr);
    for(i=0; i<6; i++){
      smoke3di->alphas_smokedir[i] = smoke3di->alphas_smokebuffer + 256*i;
      smoke3di->alphas_firedir[i]  = smoke3di->alphas_firebuffer  + 256*i;
    }
    smoke3di->ntimes = 0;
    smoke3di->ntimes_old = 0;
    smoke3di->filetype = filetype;
    smoke3di->is_zlib = 0;
    smoke3di->is_smoke_density    = 0;
    smoke3di->soot_density_loaded = 0;
#ifdef pp_SMOKEFRAME
    smoke3di->frameinfo = NULL;
#endif
    smoke3di->seq_id = nn_smoke3d;
    smoke3di->autoload = 0;
    smoke3di->compression_type = COMPRESSED_UNKNOWN;
    smoke3di->file = NULL;
    smoke3di->smokeframe_in = NULL;
    smoke3di->smokeframe_comp_list = NULL;
    smoke3di->smokeframe_out = NULL;
    smoke3di->timeslist = NULL;
#ifndef pp_SMOKEFRAME
    smoke3di->smoke_comp_all = NULL;
#endif
    smoke3di->smokeview_tmp = NULL;
    smoke3di->times = NULL;
    smoke3di->times_map = NULL;
    smoke3di->use_smokeframe = NULL;
    smoke3di->smokeframe_loaded = NULL;
    smoke3di->nchars_compressed_smoke = NULL;
    smoke3di->nchars_compressed_smoke_full = NULL;
    smoke3di->maxval = -1.0;
    smoke3di->maxvals = NULL;
    smoke3di->frame_all_zeros = NULL;
    smoke3di->smoke_boxmin = NULL;
    smoke3di->smoke_boxmax = NULL;
    smoke3di->display = 0;
    smoke3di->loaded = 0;
    smoke3di->request_load = 0;
    smoke3di->finalize = 0;
    smoke3di->primary_file = 0;
    smoke3di->is_smoke = 0;
    smoke3di->is_fire = 0;
    smoke3di->file_size = 0;
    smoke3di->blocknumber = blocknumber;
    smoke3di->lastiframe = -999;
    smoke3di->ismoke3d_time = 0;
    STRCPY(buffer2, bufferptr);
    STRCAT(buffer2, ".svz");

    len = lenbuffer+4;
    smoke3di->comp_file = SMOKE3DBUFFER(len + 1);
    STRCPY(smoke3di->comp_file, buffer2);

    if(FileExistsCaseDir(scase, smoke3di->comp_file) == YES){
      smoke3di->file = smoke3di->comp_file;
      smoke3di->is_zlib = 1;
      smoke3di->compression_type = COMPRESSED_ZLIB;
    }
    else{
      smoke3di->file = smoke3di->reg_file;
    }

    char buffer_s3dd[256], *ext;

    strcpy(buffer_s3dd, bufferptr);
    ext = strrchr(buffer_s3dd, '.');
    if(ext != NULL)*ext = 0;
    strcat(buffer_s3dd, ".s3dd");
    smoke3di->smoke_density_file = SMOKE3DBUFFER(strlen(buffer_s3dd) + 1);
    STRCPY(smoke3di->smoke_density_file, buffer_s3dd);

    {
      if(ReadLabels(&smoke3di->label, stream, NULL) == LABEL_ERR)return RETURN_TWO;
      smoke3di->valmin = -1.0;
      smoke3di->valmax = -1.0;
      if(strcmp(smoke3di->label.longlabel, "SOOT DENSITY") == 0){
        smoke3di->is_smoke = 1;
        smoke3di->valmin = 0.0;
        smoke3di->valmax = -1.0;
      }
      if(strcmp(smoke3di->label.longlabel, "HRRPUV") == 0){
        scase->show_hrrcutoff_active = 1;
        smoke3di->is_fire = 1;
        smoke3di->valmin = 0.0;
        smoke3di->valmax = 1200.0;
      }
      if(strstr(smoke3di->label.longlabel, "TEMPERATURE") != NULL){
        scase->show_tempcutoff_active = 1;
        smoke3di->is_fire = 1;
        smoke3di->valmin = 20.0;
        smoke3di->valmax = 2000.0;
      }
      ismoke3d++;
      *ismoke3d_in = ismoke3d;
    }
    if(extinct<0.0){
      extinct = 0.0;
      if(IsSootFile(smoke3di->label.shortlabel, smoke3di->label.longlabel)==1)extinct = 8700.0;
    }
    smoke3di->extinct = extinct;

    strcpy(smoke3di->cextinct, "");
    if(extinct>0.0){
      char cextinct[32], *per;

      sprintf(cextinct, "%f", extinct);
      per = strchr(cextinct, '.');
      if(per!=NULL)per[0] = 0;
      strcpy(smoke3di->cextinct, "(");
      strcat(smoke3di->cextinct, cextinct);
      strcat(smoke3di->cextinct, ")");
    }
    scase->update_smoke_alphas = 1;
  }
  return RETURN_CONTINUE;
}


/* ------------------ ParseSLCFCount ------------------------ */

int ParseSLCFCount(smv_case *scase, int option, bufferstreamdata *stream, char *buffer, int *nslicefiles_in){
  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1||parse_opts.handle_slice_files==0)return RETURN_CONTINUE;
  if(option==SCAN){
    for(;;){
      if(FGETS(buffer, 255, stream)==NULL){
        return RETURN_BREAK;
      }
      if((Match(buffer, "SLCF")==1)||
        (Match(buffer, "SLCC")==1)||
        (Match(buffer, "SLCT")==1)||
        (Match(buffer, "BNDS")==1)
        ){
        break;
      }
      return RETURN_BREAK;
    }
  }
  scase->slicecoll.nsliceinfo++;
  *nslicefiles_in = scase->slicecoll.nsliceinfo;
  if(Match(buffer, "BNDS")==1){
    if(FGETS(buffer, 255, stream)==NULL){
      return RETURN_BREAK;
    }
  }
  if(FGETS(buffer, 255, stream)==NULL){
    return RETURN_BREAK;
  }
  if(FGETS(buffer, 255, stream)==NULL){
    return RETURN_BREAK;
  }
  if(FGETS(buffer, 255, stream)==NULL){
    return RETURN_BREAK;
  }
  if(FGETS(buffer, 255, stream)==NULL){
    return RETURN_BREAK;
  }
  return RETURN_CONTINUE;
}

/* ------------------ ParseSLCFProcess ------------------------ */

int ParseSLCFProcess(smv_case *scase, int option, bufferstreamdata *stream, char *buffer, int *nn_slice_in, int ioffset_in,
  int *nslicefiles_in, slicedata **sliceinfo_copy_in, patchdata **patchgeom_in,
  char buffers[6][256]){
  char *slicelabelptr, slicelabel[256], *sliceparms;
  float above_ground_level = 0.0;
  int terrain = 0, cellcenter = 0;
  int slicegeom = 0;
  int slcf_index = 0;
  char *char_slcf_index;
  int has_reg, compression_type;
  int ii1 = -1, ii2 = -1, jj1 = -1, jj2 = -1, kk1 = -1, kk2 = -1;
  int blocknumber;
  slicedata *sd;
  size_t len;
  int read_slice_header = 0;
  char zlib_file[255], rle_file[255];
  int cell_center_flag = -1;
  int fds_dir = -1;

  char *bufferptr, *bufferptr2;
  int nslicefiles, nn_slice;
  slicedata *sliceinfo_copy;

  if(parse_opts.setup_only==1||parse_opts.smoke3d_only==1||parse_opts.handle_slice_files==0)return RETURN_CONTINUE;
  if(option==SCAN){
    for(;;){
      if(FGETS(buffer, 255, stream)==NULL){
        return RETURN_BREAK;
      }
      if( (Match(buffer, "SLCF") == 1)  ||
          (Match(buffer, "SLCC") == 1)  ||
          (Match(buffer, "SLCT") == 1)  ||
          (Match(buffer, "BNDS") == 1)
        ){
        break;
      }
    }
    return RETURN_BREAK;
  }

  nn_slice       = *nn_slice_in;
  nslicefiles    = *nslicefiles_in;
  sliceinfo_copy = *sliceinfo_copy_in;

  char_slcf_index = strchr(buffer, '!');
  if(char_slcf_index!=NULL){
    *char_slcf_index = 0;
    char_slcf_index++;
    sscanf(char_slcf_index, "%i %i %i", &slcf_index, &cell_center_flag, &fds_dir);
  }

  sliceparms = strchr(buffer, '&');
  if(sliceparms==NULL){
    read_slice_header = 1;
  }
  else{
    sliceparms++;
    sliceparms[-1] = 0;
    sscanf(sliceparms, "%i %i %i %i %i %i", &ii1, &ii2, &jj1, &jj2, &kk1, &kk2);
  }

  nn_slice++;
  *nn_slice_in = nn_slice;

  slicelabelptr = strchr(buffer, '%');
  if(slicelabelptr!=NULL){
    *slicelabelptr = 0;
    slicelabelptr++;
    TrimBack(slicelabelptr);
    slicelabelptr = TrimFront(slicelabelptr);
    strcpy(slicelabel, slicelabelptr);
    slicelabelptr = slicelabel;
  }
  if(Match(buffer, "BNDS")==1){
    strcpy(buffers[0], buffer);
    slicegeom = 1;
  }
  if(Match(buffer, "SLCT")==1){
    terrain = 1;
  }
  if(Match(buffer, "SLCC")==1){
    scase->cellcenter_slice_active = 1;
    cellcenter = 1;
  }
  TrimBack(buffer);
  len = strlen(buffer);
  if(scase->meshescoll.nmeshes>1){
    blocknumber = ioffset_in-1;
  }
  else{
    blocknumber = 0;
  }
  if(len>5){
    char *buffer3;

    buffer3 = buffer+4;
    sscanf(buffer3, "%i %f", &blocknumber, &above_ground_level);
    blocknumber--;
  }

  // read in slice file name

  if(FGETS(buffer, 255, stream)==NULL){
    scase->slicecoll.nsliceinfo--;
    return RETURN_BREAK;
  }
  if(slicegeom==1){
    strcpy(buffers[1], buffer);
  }

  bufferptr = TrimFrontBack(buffer);
  len = strlen(bufferptr);

  sd = scase->slicecoll.sliceinfo+nn_slice-1;

#ifdef pp_SLICEFRAME
  sd->frameinfo        = NULL;
#endif
  sd->fds_dir          = fds_dir;
  sd->slice_mask       = NULL;
  sd->vals2d.vals      = NULL;
  sd->vals2d.vals_orig = NULL;
  sd->vals2d.times     = NULL;
  sd->vals2d.nvals     = 0;
  sd->geom_offsets = NULL;
  sd->slcf_index = slcf_index;
  sd->finalize = 1;
  sd->ntimes = 0;
  sd->skipdup = 0;
  sd->ntimes_old = 0;
  sd->globalmax_slice = -1.0e30;
  sd->globalmin_slice = -sd->globalmax_slice;
  sd->valmin_slice = 1.0;
  sd->valmax_slice = 0.0;
  sd->imap = NULL;
  sd->jmap = NULL;
  sd->kmap = NULL;
  sd->n_imap=0;
  sd->n_jmap=0;
  sd->n_kmap=0;
  sd->cell_center = cellcenter;
  if(slicegeom==1&&cell_center_flag==1)sd->cell_center = 1;
 // sd->file_size = 0;
  sd->nframes = 0;
  sd->reg_file = NULL;
  sd->comp_file = NULL;
  sd->vol_file = NULL;
  sd->slicelabel = NULL;
  sd->cell_center_edge = 0;
  sd->file_size = 0;
  sd->slice_filetype = SLICE_NODE_CENTER;
  sd->patchgeom = NULL;
  if(slicegeom==1){
    patchdata *patchgeom_local;

    if(cell_center_flag==1)sd->cell_center = 1;
    sd->slice_filetype = SLICE_GEOM;
    patchgeom_local = (patchdata *)SLICEBUFFER(sizeof(patchdata));
    sd->patchgeom = patchgeom_local;
  }
  if(terrain==1){
    sd->slice_filetype = SLICE_TERRAIN;
  }
  if(cellcenter==1){
    sd->slice_filetype = SLICE_CELL_CENTER;
  }

  strcpy(zlib_file, bufferptr);
  strcat(zlib_file, ".svz");
  strcpy(rle_file, bufferptr);
  strcat(rle_file, ".rle");

  has_reg = NO;
  compression_type = UNCOMPRESSED;
  if(parse_opts.lookfor_compressed_files==1){
    if(FileExistsCaseDir(scase, rle_file)==YES)compression_type  = COMPRESSED_RLE;
    if(FileExistsCaseDir(scase, zlib_file)==YES)compression_type = COMPRESSED_ZLIB;
  }
  if(compression_type==UNCOMPRESSED&&(parse_opts.fast_startup==1||FileExistsCaseDir(scase, bufferptr)==YES))has_reg = YES;
  if(has_reg==NO&&compression_type==UNCOMPRESSED){
    scase->slicecoll.nsliceinfo--;

    nslicefiles--;
    *nslicefiles_in = nslicefiles;

    nn_slice--;
    *nn_slice_in = nn_slice;

    if(FGETS(buffer, 255, stream)==NULL){
      return RETURN_BREAK;
    }
    if(FGETS(buffer, 255, stream)==NULL){
      return RETURN_BREAK;
    }
    if(FGETS(buffer, 255, stream)==NULL){
      return RETURN_BREAK;
    }
    if(slicegeom==1){
      if(FGETS(buffer, 255, stream)==NULL){
        return RETURN_BREAK;
      }
    }
    return RETURN_CONTINUE;
  }

  sd->reg_file  = SLICEBUFFER(len + 1);
  sd->comp_file = SLICEBUFFER(len + 4 + 1);
  STRCPY(sd->reg_file, bufferptr);

  sd->compression_type = compression_type;
  switch(compression_type){
  case UNCOMPRESSED:
    sd->file = sd->reg_file;
    break;
  case COMPRESSED_ZLIB:
    STRCPY(sd->comp_file, zlib_file);
    sd->file = sd->comp_file;
    break;
  case COMPRESSED_RLE:
    STRCPY(sd->comp_file, rle_file);
    sd->file = sd->comp_file;
    break;
  default:
    assert(FFALSE);
    break;
  }

  // read in geometry file name

  if(slicegeom==1){
    int lengeom;
    char buffer2[256];

    if(FGETS(buffer2, 255, stream)==NULL){
      scase->slicecoll.nsliceinfo--;
      return RETURN_BREAK;
    }
    strcpy(buffers[2], buffer2);
    bufferptr2 = TrimFrontBack(buffer2);
    lengeom = strlen(bufferptr2);
    sd->geom_file = NULL;
    sd->geom_file = SLICEBUFFER(lengeom + 1);
    STRCPY(sd->geom_file, bufferptr2);
  }

  // read in labels

  if(sd->slice_filetype==SLICE_TERRAIN){
    if(ReadLabels(&sd->label, stream, "(terrain)")==LABEL_ERR)return RETURN_TWO;
  }
  else if(sd->slice_filetype==SLICE_CELL_CENTER){
    if(ReadLabels(&sd->label, stream, "(cell centered)")==LABEL_ERR)return RETURN_TWO;
  }
  else if(sd->slice_filetype==SLICE_GEOM){
    char geom_label[20];

    if(sd->cell_center==1){
      strcpy(geom_label, "(cell centered)");
    }
    else{
      strcpy(geom_label, "");
    }
    if(ReadLabelsBNDS(&sd->label, stream, buffers[3], buffers[4], buffers[5], geom_label)==LABEL_ERR)return RETURN_TWO;
  }
  else{
    if(ReadLabels(&sd->label, stream, NULL)==LABEL_ERR)return RETURN_TWO;
  }
  if(strlen(sd->label.longlabel)>14&&
    strncmp(sd->label.longlabel, "SOOT VISIBILITY", 15)==0){
    sd->colorbar_autoflip = 1;
  }
  else{
    sd->colorbar_autoflip = 0;
  }

  sd->size_file  = SLICEBUFFER(len + 3 + 1);
  sd->bound_file = SLICEBUFFER(len + 4 + 1);
  STRCPY(sd->size_file, bufferptr);
  STRCAT(sd->size_file, ".sz");
  STRCPY(sd->bound_file, bufferptr);
  STRCAT(sd->bound_file, ".bnd");
  sd->have_bound_file = NO;

  sd->slicelabel = NULL;
  if(slicelabelptr!=NULL){
    int lenslicelabel;

    lenslicelabel = strlen(slicelabel)+1;
    sd->slicelabel = SLICEBUFFER(lenslicelabel);
    strcpy(sd->slicelabel, slicelabel);
  }
  if(read_slice_header==1){
    int error;

    GetSliceFileHeader(sd->file, &ii1, &ii2, &jj1, &jj2, &kk1, &kk2, &error);
  }
  if(cellcenter==1){
    ii1 = MAX(ii1, 1);
    ii2 = MAX(ii1, ii2);
  }
  sd->is1 = ii1;
  sd->is2 = ii2;
  sd->js1 = jj1;
  sd->js2 = jj2;
  sd->ks1 = kk1;
  sd->ks2 = kk2;
  sd->iis1 = ii1;
  sd->iis2 = ii2;
  sd->jjs1 = jj1;
  sd->jjs2 = jj2;
  sd->kks1 = kk1;
  sd->kks2 = kk2;
  sd->plotx = -1;
  sd->ploty = -1;
  sd->plotz = -1;
  if(ii1==ii2)sd->plotx = ii1;
  if(jj1==jj2)sd->ploty = jj1;
  if(kk1==kk2)sd->plotz = kk1;
  sd->ijk_min[0] = ii1;
  sd->ijk_max[0] = ii2;
  sd->ijk_min[1] = jj1;
  sd->ijk_max[1] = jj2;
  sd->ijk_min[2] = kk1;
  sd->ijk_max[2] = kk2;
  sd->above_ground_level = above_ground_level;
  sd->have_agl_data = 0;
  sd->seq_id = nn_slice;
  sd->autoload = 0;
  sd->display = 0;
  sd->loaded = 0;
  sd->loading = 0;
  sd->hist_update = 0;
  sd->qslicedata = NULL;
  sd->compindex = NULL;
  sd->slicecomplevel = NULL;
  sd->qslicedata_compressed = NULL;
  if(sd->is1!=sd->is2&&sd->js1!=sd->js2&&sd->ks1!=sd->ks2){
    sd->volslice = 1;
  }
  else{
    sd->volslice = 0;
  }
  sd->times = NULL;
  sd->times_map = NULL;
  sd->slicelevel = NULL;
  sd->iqsliceframe = NULL;
  sd->qsliceframe = NULL;
  sd->timeslist = NULL;
  sd->blocknumber = blocknumber;
  sd->vloaded = 0;
  sd->uvw = 0;
  sd->nline_contours = 0;
  sd->line_contours = NULL;
  sd->menu_show = 1;
  sd->constant_color = NULL;
  sd->histograms = NULL;
  sd->nhistograms = 0;
  {
    meshdata *meshi;

    meshi = scase->meshescoll.meshinfo+blocknumber;
    sd->full_mesh = NO;
    if(sd->is2-sd->is1==meshi->ibar &&
      sd->js2-sd->js1==meshi->jbar &&
      sd->ks2-sd->ks1==meshi->kbar){
        sd->full_mesh = YES;
    }
    if(sd->slice_filetype==SLICE_CELL_CENTER){
      if(                         sd->is1==sd->is2&&sd->is1==1)sd->cell_center_edge = 1;
      if(sd->cell_center_edge==0&&sd->js1==sd->js2&&sd->js1==1)sd->cell_center_edge = 1;
      if(sd->cell_center_edge==0&&sd->ks1==sd->ks2&&sd->ks1==1)sd->cell_center_edge = 1;
      if(sd->cell_center_edge==0&&sd->is1==sd->is2&&sd->is1==meshi->ibar)sd->cell_center_edge = 1;
      if(sd->cell_center_edge==0&&sd->js1==sd->js2&&sd->js1==meshi->jbar)sd->cell_center_edge = 1;
      if(sd->cell_center_edge==0&&sd->ks1==sd->ks2&&sd->ks1==meshi->kbar)sd->cell_center_edge = 1;
    }
  }

  sliceinfo_copy++;
  *sliceinfo_copy_in = sliceinfo_copy;

  meshdata *meshi;

  meshi = scase->meshescoll.meshinfo + blocknumber;
  meshi->nsliceinfo++;

  if(slicegeom==1){
    strcpy(buffer, buffers[0]);
    *patchgeom_in = sd->patchgeom;
  }
  else{
    return RETURN_CONTINUE;
  }
  return RETURN_PROCEED;
}

/// @brief Free all of the slice data associated with a case. This includes both
/// scalar and vectors slices. After this nsliceinfo, nmultisliceinfo,
/// nvsliceinfo, and nmultivsliceinfo are all zero.
/// @param scase The case from which to clear slice data
void FreeSliceData(smv_case *scase){
  int i;

  FREEMEMORY(scase->surfcoll.surfinfo);
  if(scase->slicecoll.nsliceinfo>0){
    for(i = 0; i<scase->slicecoll.nsliceinfo; i++){
      slicedata *sd;
      sd = scase->slicecoll.sliceinfo+i;
      FreeLabels(&scase->slicecoll.sliceinfo[i].label);
      FREEMEMORY(sd->reg_file);
      FREEMEMORY(sd->comp_file);
      FREEMEMORY(sd->size_file);
    }
    FREEMEMORY(scase->sliceorderindex);
    for(i = 0; i<scase->slicecoll.nmultisliceinfo; i++){
      multislicedata *mslicei;

      mslicei = scase->slicecoll.multisliceinfo+i;
      mslicei->loadable = 1;
      FREEMEMORY(mslicei->islices);
    }
    FREEMEMORY(scase->slicecoll.multisliceinfo);
    scase->slicecoll.nmultisliceinfo = 0;
    FREEMEMORY(scase->slicecoll.sliceinfo);
  }
  scase->slicecoll.nsliceinfo = 0;

  //*** free multi-vector slice data

  if(scase->slicecoll.nvsliceinfo>0){
    FREEMEMORY(scase->vsliceorderindex);
    for(i = 0; i<scase->slicecoll.nmultivsliceinfo; i++){
      multivslicedata *mvslicei;

      mvslicei = scase->slicecoll.multivsliceinfo+i;
      FREEMEMORY(mvslicei->ivslices);
    }
    FREEMEMORY(scase->slicecoll.multivsliceinfo);
    scase->slicecoll.nmultivsliceinfo = 0;
  }
}

/* ------------------ GetViewPointPtr ------------------------ */

char *GetViewPointPtr(char **viewpoint_list, int nviewpoint_list, char *viewpoint){
  int i;

  for(i = 0; i<nviewpoint_list; i++){
    if(strcmp(viewpoint_list[i], viewpoint)==0)return viewpoint_list[i];
  }
  return NULL;
}

/* ------------------ GetViewPoints ------------------------ */

int GetViewpoints(char *inifile, char ***viewpointlist_ptr){
  FILE *stream;
  char **viewpointlist;

  if(inifile==NULL||strlen(inifile)==0)return 0;
  stream = fopen(inifile, "r");
  if(stream==NULL)return 0;

  int nviewpoints = 0;
  while(!feof(stream)){
    char buffer[255];


    CheckMemory;
    if(fgets(buffer, 255, stream)==NULL)break;
    if(Match(buffer, "VIEWPOINT5")==1||Match(buffer, "VIEWPOINT6")==1){
      nviewpoints++;
    }
  }
  if(nviewpoints==0){
    fclose(stream);
    return 0;
  }

  NewMemory((void **)&viewpointlist, nviewpoints*sizeof(char *));
  *viewpointlist_ptr = viewpointlist;

  rewind(stream);

  nviewpoints = 0;
  while(!feof(stream)){
    char buffer[255], *buffptr;

    CheckMemory;
    if(fgets(buffer, 255, stream)==NULL)break;
    if(Match(buffer, "VIEWPOINT5")==1||Match(buffer, "VIEWPOINT6")==1){
      int nskip, i;

      nskip = 11;
      if(Match(buffer, "VIEWPOINT6")==1)nskip = 12;
      for(i = 0; i<nskip; i++){
        if(fgets(buffer, 255, stream)==NULL)break;
      }
      if(fgets(buffer, 255, stream)==NULL)break;
      buffptr = GetCharPtr(buffer);
      if(buffptr!=NULL){
        viewpointlist[nviewpoints++] = buffptr;
      }
    }
  }
  fclose(stream);
  return nviewpoints;
}

/* ------------------ GetAllViewPoints ------------------------ */

int GetAllViewPoints(char *casenameini, char ***all_viewpoints_ptr){
  int n1 = 0, nall_viewpoints = 0;
  char **vp1 = NULL, **all_viewpoints = NULL;
  int i;
#define NDEFAULT_VIEWS 1
  char *default_views[NDEFAULT_VIEWS] = {"external"};

  n1 = GetViewpoints(casenameini, &vp1);
  nall_viewpoints = 7+n1;
  NewMemory((void **)&all_viewpoints, nall_viewpoints*sizeof(char *));

  nall_viewpoints = 0;
  for(i = 0; i<NDEFAULT_VIEWS; i++){
    char *viewptr;

    if(GetViewPointPtr(all_viewpoints, nall_viewpoints, default_views[i])==NULL){
      viewptr = GetCharPtr(default_views[i]);
      if(viewptr!=NULL){
        all_viewpoints[nall_viewpoints++] = viewptr;
      }
    }
  }
  for(i = 0; i<n1; i++){
    if(GetViewPointPtr(all_viewpoints, nall_viewpoints, vp1[i])==NULL){
      all_viewpoints[nall_viewpoints] = vp1[i];
      nall_viewpoints++;
    }
  }
  *all_viewpoints_ptr = all_viewpoints;
  return nall_viewpoints;
}
/* ------------------ GenerateViewpointMenu ------------------------ */

void GenerateViewpointMenu(void){
  char viewpiontemenu_filename[256];
  FILE *stream = NULL;
  int i;
  char cform1[20], cform2[20];
  char **all_viewpoints;
  int nviewpoints;
  char casenameini[256];

  strcpy(viewpiontemenu_filename, "");
  char *smokeview_scratchdir = GetUserConfigDir();
  if(smokeview_scratchdir!=NULL){
    strcat(viewpiontemenu_filename, smokeview_scratchdir);
    strcat(viewpiontemenu_filename, dirseparator);
  }
  FREEMEMORY(smokeview_scratchdir);
  strcat(viewpiontemenu_filename, global_scase.fdsprefix);
  strcat(viewpiontemenu_filename, ".viewpoints");
  strcpy(casenameini, global_scase.fdsprefix);
  strcat(casenameini, ".ini");

  nviewpoints = GetAllViewPoints(casenameini, &all_viewpoints);
  if(nviewpoints==0)return;

  // if we can't write out to the viewpoint menu file then abort
  stream = fopen(viewpiontemenu_filename, "w");
  if(stream==NULL)return;

  int max1 = 5;
  int max2 = 20;
  sprintf(cform1, "%s%i.%is", "%", max1, max1);/* %20.20s*/
  sprintf(cform2, "%s-%i.%is", "%", max2, max2);

  char format[80];
  sprintf(format, "%s, %s\n", cform1, cform2);

  int count = 1;
  fprintf(stream, "\n");
  fprintf(stream, format, "index", "viewpoint");
  fprintf(stream, format, "d", "delete");
  for(i = 0; i<nviewpoints; i++){
    char index[20];

    sprintf(index, "%i", count++);
    fprintf(stream, format, index, all_viewpoints[i]);
  }
  fclose(stream);
}

/* ------------------ UpdateEvents ------------------------ */

void UpdateEvents(void){
  FILE *stream = NULL;
  int nrows, ncols, buffer_len;
  char **tokens;
  int i;

  stream = fopen(global_scase.paths.event_filename, "r");
  if(stream==NULL)return;

  buffer_len = GetRowCols(stream, &nrows, &ncols);
  if(nrows<=0||ncols<=0||buffer_len<=0){
    fclose(stream);
    return;
  }
  rewind(stream);

  NewMemory((void **)&tokens, ncols*sizeof(char *));

  for(i = 0; i<ncols; i++){
    tokens[i] = NULL;
  }

#define LEN_BUFFER 1000
  char temp_buffer[LEN_BUFFER];
  if(fgets(temp_buffer, LEN_BUFFER, stream)==NULL)return;
  while(!feof(stream)){
    char buffer[LEN_BUFFER], buffer_temp[LEN_BUFFER], *message;
    int ntokens;
    float tminmax[2], xyz[3], frgb[3];
    int rgblabel[3];
    labeldata label;

    CheckMemory;
    if(fgets(buffer, LEN_BUFFER, stream)==NULL)break;
    if(buffer[0]=='#')continue;
    // tmin, tmax, x, y, z, r, g, b, 1/0 (foreground color) ! message
    ParseCSV(buffer, buffer_temp, tokens, &ntokens);
    if(ntokens>=9){
      char *c_tstart, *c_tend, *c_x, *c_y, *c_type;

      c_tstart = tokens[10];
      c_tend   = tokens[11];
      c_x      = tokens[13];
      c_y      = tokens[14];
      c_type   = tokens[9];

      sscanf(c_tstart, "%f", tminmax);
      sscanf(c_tend,   "%f", tminmax+1);
      sscanf(c_x,      "%f", xyz);
      sscanf(c_y,      "%f", xyz+1);
      xyz[2] = 1700.0;

      memcpy(&label, &LABEL_default, sizeof(labeldata));

      message = TrimFrontBack(c_type);
      strcpy(label.name, "");
      if(strlen(message)>0){
        strcat(label.name, message);
      }
 //     message = TrimFrontBack(c_id);
 //     if(strlen(message)>0){
 //       strcat(label.name, "-");
 //       strcat(label.name, message);
 //     }

      tminmax[0] *= 60.0;
      tminmax[1] *= 60.0;
      memcpy(label.tstart_stop, tminmax, 2*sizeof(float));

      memcpy(label.xyz, xyz, 3*sizeof(float));

      rgblabel[0] = 255;
      rgblabel[1] = 255;
      rgblabel[2] = 255;
      memcpy(label.rgb, global_scase.rgb, 3*sizeof(int));
      frgb[0] = (float)rgblabel[0]/255.0;
      frgb[1] = (float)rgblabel[1]/255.0;
      frgb[2] = (float)rgblabel[2]/255.0;
      memcpy(label.frgb, frgb, 3*sizeof(int));

      label.useforegroundcolor = 0;
      label.show_always = 0;
      LabelInsert(&global_scase.labelscoll, &label);
      event_file_exists = 1;
    }
  }
  fclose(stream);
}

/* ------------------ UpdateObstBoundingBox ------------------------ */

void UpdateObstBoundingBox(smv_case *scase, float *XB){
  float XB0[6];
  int i;

  XB0[0] = MIN(XB[0], XB[1]);
  XB0[1] = MAX(XB[0], XB[1]);
  XB0[2] = MIN(XB[2], XB[3]);
  XB0[3] = MAX(XB[2], XB[3]);
  XB0[4] = MIN(XB[4], XB[5]);
  XB0[5] = MAX(XB[4], XB[5]);
  for(i = 0; i<3; i++){
    int imin, imax;

    imin = 2*i;
    imax = 2*i+1;
    if(scase->obst_bounding_box[imin]>scase->obst_bounding_box[imax]){
      scase->obst_bounding_box[imin] = XB0[imin];
      scase->obst_bounding_box[imax] = XB0[imax];
    }
    else{
      scase->obst_bounding_box[imin] = MIN(scase->obst_bounding_box[imin], XB0[imin]);
      scase->obst_bounding_box[imax] = MAX(scase->obst_bounding_box[imax], XB0[imax]);
    }
  }
}

/* ------------------ GetBlockagePtr ------------------------ */

blockagedata *GetBlockagePtr(smv_case *scase, float *xyz){
  float xyzcenter[3];
  int i;

  xyzcenter[0] = (xyz[0]+xyz[1])/2.0;
  xyzcenter[1] = (xyz[2]+xyz[3])/2.0;
  xyzcenter[2] = (xyz[4]+xyz[5])/2.0;
  for(i=0;i<scase->meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi = scase->meshescoll.meshinfo + i;
    if(xyzcenter[0]<meshi->boxmin[0]||xyzcenter[0]>meshi->boxmax[0])continue;
    if(xyzcenter[1]<meshi->boxmin[1]||xyzcenter[1]>meshi->boxmax[1])continue;
    if(xyzcenter[2]<meshi->boxmin[2]||xyzcenter[2]>meshi->boxmax[2])continue;
    for(j=0;j<meshi->nbptrs;j++){
      blockagedata *bc;
      float *xyzEXACT;

      bc=meshi->blockageinfoptrs[j];
      xyzEXACT = bc->xyzEXACT;
      if(xyzcenter[0]<xyzEXACT[0]||xyzcenter[0]>xyzEXACT[1])continue;
      if(xyzcenter[1]<xyzEXACT[2]||xyzcenter[1]>xyzEXACT[3])continue;
      if(xyzcenter[2]<xyzEXACT[4]||xyzcenter[2]>xyzEXACT[5])continue;
      return bc;
    }
  }
  return NULL;
}

/* ------------------ ReadSMVOrig ------------------------ */

void ReadSMVOrig(smv_case *scase){
  FILE *stream=NULL;

  stream = fopen(scase->paths.smv_orig_filename, "r");
  if(stream == NULL)return;
  PRINTF("reading  %s\n", scase->paths.smv_orig_filename);

  for(;;){
    char buffer[255];

    if(fgets(buffer, 255, stream)==NULL)break;
        /*
        OBST format:
        nblockages
        xmin xmax ymin ymax zmin zmax bid s_i1 s_i2 s_j1 s_j2 s_k1 s_k2 t_x0 t_y0 t_z0
        ...
        ...

        bid             - blockage id
        s_i1, ..., s_k2 - surf indices for i1, ..., k2 blockage faces
        t_x0,t_y0,t_z0  - texture origin

        i1 i2 j1 j2 k1 k2 colorindex blocktype r g b : ignore rgb if blocktype != -3
        ...
        ...

        int colorindex, blocktype;
        colorindex: -1 default color
                    -2 invisible
                    -3 use r g b color
                    >=0 color/color2/texture index
        blocktype: 0 regular block
                   2 outline
                   3 smoothed block
                   -1 (use type from surf)
                   (note: if blocktype&8 == 8 then this is a "terrain" blockage
                         if so then subtract 8 and set bc->is_wuiblock=1)
        r g b           colors used if colorindex==-3
        */
    if(Match(buffer, "OBST")==1){
      float *xyz;
      int i;

      FREEMEMORY(scase->obstcoll.obstinfo);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &scase->obstcoll.nobstinfo);
      NewMemory((void **)&scase->obstcoll.obstinfo, scase->obstcoll.nobstinfo*sizeof(xbdata));
      for(i = 0; i<scase->obstcoll.nobstinfo; i++){
        xbdata *obi;
        int blockid, *surf_index;

        obi = scase->obstcoll.obstinfo+i;
        xyz = obi->xyz;
        surf_index = obi->surf_index;
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f %f %f %f %i %i %i %i %i %i %i",
             xyz, xyz+1, xyz+2, xyz+3, xyz+4, xyz+5,
             &blockid,
             surf_index, surf_index+1, surf_index+2, surf_index+3, surf_index+4, surf_index+5);
      }
      for(i = 0; i<scase->obstcoll.nobstinfo; i++){
        xbdata *obi;
        int dummy[6];
        float s_color[4];
        int colorindex, blocktype;

        obi = scase->obstcoll.obstinfo+i;
        obi->transparent   = 0;
        obi->invisible     = 0;
        obi->usecolorindex = 0;
        obi->color = NULL;

        fgets(buffer, 255, stream);

        s_color[0] = -1.0;
        s_color[1] = -1.0;
        s_color[2] = -1.0;
        s_color[3] = 1.0;
        sscanf(buffer, "%i %i %i %i %i %i %i %i %f %f %f %f",
        dummy, dummy+1, dummy+2, dummy+3, dummy+4, dummy+5,
        &colorindex, &blocktype, s_color, s_color+1, s_color+2, s_color+3);

        if((blocktype&3)==3)blocktype -= 3; // convert any smooth blocks to 'normal' blocks
        if(blocktype>0&&(blocktype&8)==8)blocktype -= 8;
        if(s_color[3]<0.999)obi->transparent=1;

        /* custom color */

        if(colorindex==0||colorindex==7)colorindex=-3;
        if(colorindex==COLOR_INVISIBLE){
          obi->blocktype=BLOCK_hidden;
          obi->invisible=1;
        }
        if(colorindex>=0){
          obi->color = GetColorPtr(scase, scase->rgb[scase->nrgb+colorindex]);
          obi->usecolorindex=1;
          obi->colorindex=colorindex;
          scase->updateindexcolors=1;
        }
        if(colorindex==-3){
          obi->color = GetColorPtr(scase, s_color);
          scase->updateindexcolors=1;
        }
        obi->colorindex = colorindex;
        obi->blocktype = blocktype;

        int j;

        for(j=0;j<6;j++){
          obi->surfs[0] = NULL;
        }
        if(scase->surfcoll.surfinfo!=NULL){
          for(j=0;j<6;j++){
            if(obi->surf_index[j]>=0)obi->surfs[j] = scase->surfcoll.surfinfo + obi->surf_index[j];
          }
        }
        obi->bc = GetBlockagePtr(scase, obi->xyz);
      }
      break;
    }
  }
  fclose(stream);
}

/// @brief Initialize a CSV file definition.
/// @param[out] csvi The previously allocated csvfiledata
/// @param[in] file The filename of the CSV file
/// @param[in] type The type of the file. Typically just a label
/// @param[in] format The format of the CSV file, either FDS or CFAST
void InitCSV(csvfiledata *csvi, const char *file, const char *type, int format){
  csvi->loaded       = 0;
  csvi->display      = 0;
  csvi->defined      = 0;
  csvi->glui_defined = 0;
  csvi->time         = NULL;
  csvi->ncsvinfo     = 0;
  csvi->csvinfo      = NULL;
  csvi->format       = format;

  if(file != NULL){
    NewMemory((void **)&csvi->file, strlen(file) + 1);
    strcpy(csvi->file, file);
  }
  else{
    csvi->file = NULL;
  }
  strcpy(csvi->c_type, type);
}

/// @brief Add the definition of a CSV file to case.
/// @param[inout] scase The case to add the CSV file defintion to.
/// @param[in] suffix The suffix of the file (e.g. "_devices")
/// @param[in] type The type of the file. Typically just a label
/// @param[in] format The format of the CSV file, either FDS or CFAST
void AddCfastCsvfi(smv_case *scase, const char *suffix, const char *type, int format){
  char filename[255];
  int i;

  strcpy(filename, scase->fdsprefix);
  strcat(filename, suffix);
  strcat(filename, ".csv");
  for(i=0;i<scase->csvcoll.ncsvfileinfo;i++){
    csvfiledata *csvfi;

    csvfi = scase->csvcoll.csvfileinfo + i;
    if(strcmp(csvfi->c_type,type)==0)return;
  }
  if(FileExistsCaseDir(scase, filename) == NO)return;
  InitCSV(scase->csvcoll.csvfileinfo + scase->csvcoll.ncsvfileinfo, filename, type, format);
  scase->csvcoll.ncsvfileinfo++;
}

  /* ------------------ AddCfastCsvf ------------------------ */

void AddCfastCsvf(smv_case *scase){
#define CFAST_CSV_MAX 10
  AddCfastCsvfi(scase, "_zone",         "zone",         CSV_FDS_FORMAT);
  AddCfastCsvfi(scase, "_compartments", "compartments", CSV_CFAST_FORMAT);
  AddCfastCsvfi(scase, "_devices",      "devices",      CSV_CFAST_FORMAT);
  AddCfastCsvfi(scase, "_walls",        "walls",        CSV_CFAST_FORMAT);
  AddCfastCsvfi(scase, "_masses",       "masses",       CSV_CFAST_FORMAT);
  AddCfastCsvfi(scase, "_vents",        "vents",        CSV_CFAST_FORMAT);
 // AddCfastCsvfi(scase, "_diagnostics",  "diagnostics",  CSV_CFAST_FORMAT);
 // AddCfastCsvfi(scase, "_resid",        "resid",        CSV_CFAST_FORMAT);
 // AddCfastCsvfi(scase, "_slab",         "slab",         CSV_CFAST_FORMAT);
 // AddCfastCsvfi(scase, "_calculations", "calculations", CSV_CFAST_FORMAT);
}

/* ------------------ Compress ------------------------ */

void *Compress(void *arg){
  char shellcommand[1024];

  PRINTF("Compressing...\n");
  GLUICompressOnOff(OFF);

  WriteIni(LOCAL_INI, NULL);

  // surround smokezip path name with "'s so that the system call can handle embedded blanks

  strcpy(shellcommand, "\"");
  strcat(shellcommand, smokezippath);
  strcat(shellcommand, "\" ");
  if(overwrite_all == 1){
    strcat(shellcommand, " -f ");
  }
  if(erase_all == 1){
    strcat(shellcommand, " -c ");
  }
  if(compress_autoloaded == 1){
    strcat(shellcommand, " -auto ");
  }
  strcat(shellcommand, " ");
  strcat(shellcommand, smv_filename);

  PRINTF("Executing shell command: %s\n", shellcommand);
  system(shellcommand);
  UpdateSmoke3dMenuLabels();
  UpdateBoundaryMenuLabels();
  GLUICompressOnOff(ON);
  updatemenu = 1;
  PRINTF("Compression completed\n");
  THREAD_EXIT(compress_threads);
}

/* ------------------ CheckFiles ------------------------ */

void *CheckFiles(void *arg){
  int i;

  THREADcontrol(checkfiles_threads, THREAD_LOCK);
  global_scase.have_compressed_files = 0;
  THREADcontrol(checkfiles_threads, THREAD_UNLOCK);
  for(i = 0;i < global_scase.npatchinfo;i++){
    patchdata *patchi;
    int have_file;

    patchi = global_scase.patchinfo + i;
    have_file = FileExistsCaseDir(&global_scase, patchi->comp_file);
    THREADcontrol(checkfiles_threads, THREAD_LOCK);
    if(have_file == YES){
      patchi->compression_type_temp = COMPRESSED_ZLIB;
      global_scase.have_compressed_files = 1;
    }
    THREADcontrol(checkfiles_threads, THREAD_UNLOCK);
  }
  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int have_file;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    have_file = FileExistsCaseDir(&global_scase, smoke3di->comp_file);
    THREADcontrol(checkfiles_threads, THREAD_LOCK);
    if(have_file == YES){
      smoke3di->compression_type_temp = COMPRESSED_ZLIB;
      global_scase.have_compressed_files = 1;
    }
    THREADcontrol(checkfiles_threads, THREAD_UNLOCK);
  }
  if(global_scase.have_compressed_files == 0){
    THREAD_EXIT(checkfiles_threads);
  }
  THREADcontrol(checkfiles_threads, THREAD_LOCK);
  for(i = 0; i < global_scase.npatchinfo; i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->compression_type_temp == COMPRESSED_ZLIB){
      patchi->compression_type = COMPRESSED_ZLIB;
      patchi->file = patchi->comp_file;
    }
  }
  for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->compression_type_temp == COMPRESSED_ZLIB){
      smoke3di->file = smoke3di->comp_file;
      smoke3di->is_zlib = 1;
      smoke3di->compression_type = COMPRESSED_ZLIB;
    }
  }
  updatemenu = 1;
  THREADcontrol(checkfiles_threads, THREAD_UNLOCK);
  THREAD_EXIT(checkfiles_threads);
}

/* ------------------ InitMeshBlockages ------------------------ */

void InitMeshBlockages(void){
  int i;

  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;
    int counts[6];
    int *is_extface;
    float *xplt, *yplt, *zplt;

    meshi = global_scase.meshescoll.meshinfo + i;
    if(meshi->nbptrs == 0)continue;
    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;
    is_extface = meshi->is_extface;
    for(j=0; j< 6; j++){
      counts[j]            = 0;
      meshi->bc_faces[j]   = NULL;
      meshi->n_bc_faces[j] = 0;
    }
    for(j=0; j<meshi->nbptrs; j++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[j];

      if(bc->ijk[0] == 0           && is_extface[0] == 0)counts[0]++;
      if(bc->ijk[1] == meshi->ibar && is_extface[1] == 0)counts[1]++;
      if(bc->ijk[2] == 0           && is_extface[2] == 0)counts[2]++;
      if(bc->ijk[3] == meshi->jbar && is_extface[3] == 0)counts[3]++;
      if(bc->ijk[4] == 0           && is_extface[4] == 0)counts[4]++;
      if(bc->ijk[5] == meshi->kbar && is_extface[5] == 0)counts[5]++;
    }
    for(j=0; j<6; j++){
      if(counts[j]>0)NewMemory((void **)&meshi->bc_faces[j],  meshi->nbptrs*sizeof(blockagedata *));
      meshi->n_bc_faces[j] = counts[j];
      counts[j] = 0;
    }
    for(j=0; j<meshi->nbptrs; j++){
      blockagedata *bc;
      blockagedata **bclist;

      bc = meshi->blockageinfoptrs[j];

      bclist = meshi->bc_faces[0]; if(bc->ijk[0] == 0           && is_extface[0] == 0)bclist[counts[0]++] = bc;
      bclist = meshi->bc_faces[1]; if(bc->ijk[1] == meshi->ibar && is_extface[1] == 0)bclist[counts[1]++] = bc;
      bclist = meshi->bc_faces[2]; if(bc->ijk[2] == 0           && is_extface[2] == 0)bclist[counts[2]++] = bc;
      bclist = meshi->bc_faces[3]; if(bc->ijk[3] == meshi->jbar && is_extface[3] == 0)bclist[counts[3]++] = bc;
      bclist = meshi->bc_faces[4]; if(bc->ijk[4] == 0           && is_extface[4] == 0)bclist[counts[4]++] = bc;
      bclist = meshi->bc_faces[5]; if(bc->ijk[5] == meshi->kbar && is_extface[5] == 0)bclist[counts[5]++] = bc;
    }
    for(j = 0;j < meshi->nbptrs;j++){
      blockagedata *bc;
      float *xyz;
      int *ijk;

      bc = meshi->blockageinfoptrs[j];
      xyz = bc->xyz;
      ijk = bc->ijk;
      xyz[0] = xplt[ijk[0]];
      xyz[1] = xplt[ijk[1]];
      xyz[2] = yplt[ijk[2]];
      xyz[3] = yplt[ijk[3]];
      xyz[4] = zplt[ijk[4]];
      xyz[5] = zplt[ijk[5]];
    }
  }
}

/* ------------------ GetSliceParmInfo ------------------------ */

void GetSliceParmInfo(sliceparmdata *sp){
  global_scase.slicecoll.nsliceinfo = sp->nsliceinfo;
  global_scase.slicecoll.nmultisliceinfo=sp->nmultisliceinfo;
  global_scase.slicecoll.nvsliceinfo = sp->nvsliceinfo;
  global_scase.slicecoll.nmultivsliceinfo =sp->nmultivsliceinfo;
}

/* ------------------ SetSliceParmInfo ------------------------ */

void SetSliceParmInfo(sliceparmdata *sp){
  sp->nsliceinfo       = global_scase.slicecoll.nsliceinfo;
  sp->nmultisliceinfo  = global_scase.slicecoll.nmultisliceinfo;
  sp->nvsliceinfo      = global_scase.slicecoll.nvsliceinfo;
  sp->nmultivsliceinfo = global_scase.slicecoll.nmultivsliceinfo;
}

/* ------------------ ReadSMV_Init ------------------------ */

/// @brief Initialise any global variables necessary to being parsing an SMV
/// file. This should be called before @ref ReadSMV_Parse.
/// @return zero on success, nonzero on failure.
int ReadSMV_Init(smv_case *scase){
  float timer_readsmv;
  float timer_setup;

  START_TIMER(timer_setup);
  START_TIMER(timer_readsmv);
  START_TIMER(scase->processing_time);

#ifdef pp_ISOFRAME
  use_isosurface_threads = 0;
#endif
//** initialize multi-threading
  if(runscript == 1){
    use_checkfiles_threads  = 0;
    use_ffmpeg_threads      = 0;
    use_readallgeom_threads = 0;
    use_isosurface_threads  = 0;
    use_meshnabors_threads  = 0;
  }

  START_TIMER(scase->getfilelist_time);
  MakeFileLists(scase);
  PRINT_TIMER(timer_setup, "MakeFileLists");
  STOP_TIMER(scase->getfilelist_time);

  START_TIMER(scase->pass0_time);

  scase->propcoll.npropinfo=1; // the 0'th prop is the default human property

  FREEMEMORY(global_scase.fds_title);

  FREEMEMORY(scase->treeinfo);
  scase->ntreeinfo=0;

  int i;
  for(i=0;i<scase->nterraininfo;i++){
    terraindata *terri;

    terri = scase->terraininfo + i;
    FREEMEMORY(terri->xplt);
    FREEMEMORY(terri->yplt);
    FREEMEMORY(terri->zcell);
    FREEMEMORY(terri->znode);
  }
  FREEMEMORY(scase->terraininfo);
  scase->nterraininfo=0;
  scase->niso_compressed=0;
  if(scase->sphereinfo==NULL){
    NewMemory((void **)&scase->sphereinfo,sizeof(spherepoints));
    InitSpherePoints(scase->sphereinfo,14);
  }
  if(scase->wui_sphereinfo==NULL){
    NewMemory((void **)&scase->wui_sphereinfo,sizeof(spherepoints));
    InitSpherePoints(scase->wui_sphereinfo,14);
  }
  PRINT_TIMER(timer_setup, "InitSpherePoints");
  scase->ntotal_blockages=0;

  if(scase->csvcoll.ncsvfileinfo>0){
    csvfiledata *csvi;

    for(i=0;i<scase->csvcoll.ncsvfileinfo;i++){
      csvi = scase->csvcoll.csvfileinfo + i;
      FREEMEMORY(csvi->file);
    }
    FREEMEMORY(scase->csvcoll.csvfileinfo);
  }
  scase->csvcoll.ncsvfileinfo=0;

  if(scase->ngeominfo>0){
    for(i=0;i<scase->ngeominfo;i++){
      geomdata *geomi;

      geomi = scase->geominfo + i;
      if(geomi->ngeomobjinfo>0){
        FREEMEMORY(geomi->geomobjinfo);
        geomi->ngeomobjinfo=0;
      }
      FREEMEMORY(geomi->file);
    }
    FREEMEMORY(scase->geominfo);
    scase->ngeominfo=0;
  }

  if(scase->ncgeominfo>0){
    for(i = 0; i<scase->ncgeominfo; i++){
      geomdata *geomi;

      geomi = scase->cgeominfo+i;
      FREEMEMORY(geomi->file);
    }
    FREEMEMORY(scase->cgeominfo);
    scase->ncgeominfo = 0;
  }

  FREEMEMORY(scase->tickinfo);
  scase->ntickinfo=0;
  scase->ntickinfo_smv=0;

  scase->updatefaces=1;
  scase->nfires=0;
  scase->nrooms=0;

  START_TIMER(timer_setup);
  InitSurface(&scase->sdefault);
  PRINT_TIMER(timer_setup, "InitSurface");
  NewMemory((void **)&scase->sdefault.surfacelabel,(5+1));
  strcpy(scase->sdefault.surfacelabel,"INERT");

  InitVentSurface(&scase->v_surfacedefault);
  PRINT_TIMER(timer_setup, "InitVentSurface");
  NewMemory((void **)&scase->v_surfacedefault.surfacelabel,(4+1));
  strcpy(scase->v_surfacedefault.surfacelabel,"VENT");

  InitSurface(&scase->e_surfacedefault);
  PRINT_TIMER(timer_setup, "InitSurface");
  NewMemory((void **)&scase->e_surfacedefault.surfacelabel,(8+1));
  strcpy(scase->e_surfacedefault.surfacelabel,"EXTERIOR");
  scase->e_surfacedefault.color=mat_ambient2;

  // free memory for particle class

  if(scase->partclassinfo!=NULL){
    int j;

    for(i=0;i<scase->npartclassinfo+1;i++){
      partclassdata *partclassi;

      partclassi = scase->partclassinfo + i;
      FREEMEMORY(partclassi->name);
      if(partclassi->ntypes>0){
        for(j=0;j<partclassi->ntypes;j++){
          flowlabels *labelj;

          labelj = partclassi->labels+j;
          FreeLabels(labelj);
        }
        FREEMEMORY(partclassi->labels);
        partclassi->ntypes=0;
      }
    }
    FREEMEMORY(scase->partclassinfo);
  }
  scase->npartclassinfo=0;

  if(scase->devicecoll.ndeviceinfo>0){
    for(i=0;i<scase->devicecoll.ndeviceinfo;i++){
    }
    FREEMEMORY(scase->devicecoll.deviceinfo);
    scase->devicecoll.ndeviceinfo=0;
  }

  // read in device (.svo) definitions

  START_TIMER(timer_setup);
  ReadDefaultObjectCollection(&scase->objectscoll, scase->fdsprefix, scase->isZoneFireModel);
  PRINT_TIMER(timer_setup, "InitSurface");

  if(scase->noutlineinfo>0){
    for(i=0;i<scase->noutlineinfo;i++){
      outlinedata *outlinei;

      outlinei = scase->outlineinfo + i;
      FREEMEMORY(outlinei->x1);
      FREEMEMORY(outlinei->y1);
      FREEMEMORY(outlinei->z1);
      FREEMEMORY(outlinei->x2);
      FREEMEMORY(outlinei->y2);
      FREEMEMORY(outlinei->z2);
    }
    FREEMEMORY(scase->outlineinfo);
    scase->noutlineinfo=0;
  }

  if(scase->nzoneinfo>0){
    for(i=0;i<scase->nzoneinfo;i++){
      zonedata *zonei;
      int n;

      zonei = scase->zoneinfo + i;
      for(n=0;n<4;n++){
        FreeLabels(&zonei->label[n]);
      }
      FREEMEMORY(zonei->file);
    }
    FREEMEMORY(scase->zoneinfo);
  }
  scase->nzoneinfo=0;

  if(scase->smoke3dcoll.nsmoke3dinfo>0){
    {
      smoke3ddata *smoke3di;

      for(i=0;i<scase->smoke3dcoll.nsmoke3dinfo;i++){
        smoke3di = scase->smoke3dcoll.smoke3dinfo + i;
        FreeSmoke3D(&global_scase, smoke3di);
        FREEMEMORY(smoke3di->comp_file);
        FREEMEMORY(smoke3di->reg_file);
      }
      FREEMEMORY(scase->smoke3dcoll.smoke3dinfo);
      scase->smoke3dcoll.nsmoke3dinfo=0;
    }
  }

  if(scase->npartinfo>0){
    for(i=0;i<scase->npartinfo;i++){
      FREEMEMORY(scase->partinfo[i].partclassptr);
      FREEMEMORY(scase->partinfo[i].reg_file);
      FREEMEMORY(scase->partinfo[i].size_file);
    }
    FREEMEMORY(scase->partinfo);
  }
  scase->npartinfo=0;


  //*** free slice data
  FreeSliceData(scase);

  if(scase->npatchinfo>0){
    for(i=0;i<scase->npatchinfo;i++){
      patchdata *patchi;

      patchi = scase->patchinfo + i;
      FreeLabels(&patchi->label);
      FREEMEMORY(patchi->reg_file);
      FREEMEMORY(patchi->comp_file);
      FREEMEMORY(patchi->size_file);
    }
    FREEMEMORY(scase->patchinfo);
  }
  scase->npatchinfo=0;

  if(scase->nisoinfo>0){
    for(i=0;i<scase->nisoinfo;i++){
      FreeLabels(&scase->isoinfo[i].surface_label);
      FREEMEMORY(scase->isoinfo[i].file);
    }
    FREEMEMORY(scase->isoinfo);
  }
  scase->nisoinfo=0;

  scase->updateindexcolors=0;
  scase->ntrnx=0;
  scase->ntrny=0;
  scase->ntrnz=0;
  scase->meshescoll.nmeshes=0;
  scase->npdim=0;
  scase->nVENT=0;
  scase->nCVENT=0;
  scase->ncvents=0;
  scase->nOBST=0;
  scase->noffset=0;
  scase->surfcoll.nsurfinfo=0;
  scase->nvent_transparent=0;

  scase->setPDIM=0;

  FREEMEMORY(scase->slicecoll.vsliceinfo);
  FREEMEMORY(scase->slicecoll.sliceinfo);

  FREEMEMORY(scase->plot3dinfo);
  FREEMEMORY(scase->patchinfo);
  FREEMEMORY(scase->boundarytypes);
  FREEMEMORY(scase->isoinfo);
  FREEMEMORY(scase->isotypes);
  FREEMEMORY(scase->roominfo);
  FREEMEMORY(scase->fireinfo);
  FREEMEMORY(scase->zoneinfo);
  FREEMEMORY(scase->zventinfo);
  FREEMEMORY(scase->texture_coll.textureinfo);
  FREEMEMORY(scase->surfcoll.surfinfo);
  FREEMEMORY(scase->terrain_texture_coll.terrain_textures);

  STOP_TIMER(scase->pass0_time );
  PRINT_TIMER(timer_readsmv, "readsmv setup");
  return 0;
}

#define VENT_EPS 0.02

/* ------------------ SetExternalVents ------------------------ */

void SetExternalVents(void){
  int i;

  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    for(j = 0;j < meshi->nvents;j++){
      ventdata *vj;
      float xyz[3];

      vj = meshi->ventinfo + j;
      vj->wall_type = INTERIORwall;
      switch(vj->dir){
      case UP_X:
        xyz[0] =  vj->xvent1_orig - VENT_EPS;
        xyz[1] = (vj->yvent1_orig + vj->yvent2_orig) / 2.0;
        xyz[2] = (vj->zvent1_orig + vj->zvent2_orig) / 2.0;
        if(InExterior(xyz)==1)vj->wall_type = LEFTwall;
        break;
      case DOWN_X:
        xyz[0] =  vj->xvent1_orig + VENT_EPS;
        xyz[1] = (vj->yvent1_orig + vj->yvent2_orig) / 2.0;
        xyz[2] = (vj->zvent1_orig + vj->zvent2_orig) / 2.0;
        if(InExterior(xyz) == 1)vj->wall_type = RIGHTwall;
        break;
      case UP_Y:
        xyz[0] = (vj->xvent1_orig + vj->xvent2_orig) / 2.0;
        xyz[1] =  vj->yvent1_orig - VENT_EPS;
        xyz[2] = (vj->zvent1_orig + vj->zvent2_orig) / 2.0;
        if(InExterior(xyz)==1)vj->wall_type = FRONTwall;
        break;
      case DOWN_Y:
        xyz[0] = (vj->xvent1_orig + vj->xvent2_orig) / 2.0;
        xyz[1] = vj->yvent1_orig  + VENT_EPS;
        xyz[2] = (vj->zvent1_orig + vj->zvent2_orig) / 2.0;
        if(InExterior(xyz)==1)vj->wall_type = BACKwall;
        break;
      case UP_Z:
        xyz[0] = (vj->xvent1_orig + vj->xvent2_orig) / 2.0;
        xyz[1] = (vj->yvent1_orig + vj->yvent2_orig) / 2.0;
        xyz[2] = vj->zvent1_orig  - VENT_EPS;
        if(InExterior(xyz)==1)vj->wall_type = DOWNwall;
        break;
      case DOWN_Z:
        xyz[0] = (vj->xvent1_orig + vj->xvent2_orig) / 2.0;
        xyz[1] = (vj->yvent1_orig + vj->yvent2_orig) / 2.0;
        xyz[2] = vj->zvent1_orig  + VENT_EPS;
        if(InExterior(xyz)==1)vj->wall_type = UPwall;
        break;
      default:
        vj->wall_type = INTERIORwall;
      }
    }
  }
}

/* ------------------ GetSkyBox ------------------------ */

void GetSkyBoxTextures(void){
  int have_textures = 1;
  int i;

  if(skyboxinfo != NULL)return;
  for(i = 0; i < 6; i++){
    char buffer[256];
    char *sides[6] = {"_ymax.jpg", "_xmin.jpg", "_ymin.jpg", "_xmax.jpg", "_zmin.jpg", "_zmax.jpg"};

    strcpy(buffer, global_scase.fdsprefix);
    strcat(buffer, sides[i]);
    if(FileExistsOrig(buffer) == 0){
      have_textures = 0;
      break;
    }
  }
  if(have_textures == 0)return;
  nskyboxinfo = 1;
  NewMemory((void **)&skyboxinfo, nskyboxinfo * sizeof(skyboxdata));
  for(i = 0; i < 6; i++){
    char buffer[256];
    char *sides[6] = {"_ymax.jpg", "_xmin.jpg", "_ymin.jpg", "_xmax.jpg", "_zmin.jpg", "_zmax.jpg"};

    strcpy(buffer, global_scase.fdsprefix);
    strcat(buffer, sides[i]);
    LoadSkyTexture(buffer, skyboxinfo->face + i);
  }
}

/* ------------------ GetSkyImage ------------------------ */

void GetSkyImageTexture(void){
  char buffer[256];
  
  strcpy(buffer, global_scase.fdsprefix);
  strcat(buffer, "_sky.jpg");
  if(sky_texture != NULL || FileExistsOrig(buffer) == 0)return;
  
  nsky_texture = 1;
  NewMemory((void **)&sky_texture, nsky_texture * sizeof(texturedata));
  NewMemory((void **)&sky_texture->file, (strlen(buffer) + 1) * sizeof(char));
  strcpy(sky_texture->file, buffer);
}

/* ------------------ ReadSMV_Parse ------------------------ */
/// @brief Parse an SMV file into global variables. This should only be called
/// after ReadSMV_Init to ensure that the appropriate variables are set.
/// @param stream the smv file stream
/// @return zero on success, non-zero on failure
int ReadSMV_Parse(smv_case *scase, bufferstreamdata *stream){
  int i;
  int have_zonevents,nzventsnew=0;
  int do_pass4=0, do_pass5=0;
  int roomdefined=0;
  int GRIDpresent=0,startpass;
  slicedata *sliceinfo_copy=NULL;
  int nisos_per_mesh=1;
  float timer_readsmv;

  int nn_smoke3d=0,nn_patch=0,nn_iso=0,nn_part=0,nn_slice=0,nslicefiles=0,nvents;

  int ipart=0, ipatch=0, iroom=0,izone_local=0,ifire=0,iiso=0;
  int ismoke3d=0,ismoke3dcount=1,igrid,ioffset;
  int itrnx, itrny, itrnz, ipdim, iobst, ivent, icvent;
  int ibartemp=2, jbartemp=2, kbartemp=2;

  int setGRID=0;
  int have_auto_terrain_image=0;

  int n_cadgeom_keywords = 0;
  int n_surf_keywords = 0;

  char buffer[256], buffers[6][256];
  patchdata *patchgeom=NULL;

 {
    int return_code;

  // get input file name

    return_code=GetInpf(scase, stream);
    if(return_code!=0)return return_code;
  }

/*
   ************************************************************************
   ************************ start of pass 1 *******************************
   ************************************************************************
 */

   START_TIMER(timer_readsmv);
   START_TIMER(scase->pass1_time);

  nvents=0;
  igrid=0;
  ioffset=0;
  scase->ntc_total=0;
  scase->nspr_total=0;
  scase->nheat_total=0;
  PRINTF("%s","  pass 1\n");
  for(;;){
    if(FEOF(stream)!=0){
      BREAK;
    }
    if(FGETS(buffer,255,stream)==NULL){
      BREAK;
    }
    TrimBack(buffer);
    if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
    if(MatchSMV(buffer, "PL3D")==1){// read plot3d entries in ReadSMVDynamic routine
      continue;
    }

    /*
      The keywords TRNX, TRNY, TRNZ, GRID, PDIM, OBST and VENT are not required
      BUT if any one these keywords are present then the number of each MUST be equal
    */

    // HVAC
    // label
    // n_nodes
    //  id x y z vent filter comp
    // ...
    // n_ducts
    // id node1 node2 comp

    if(MatchSMV(buffer, "HoC") == 1){
      int nfuelinfo_local;

      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i", &nfuelinfo_local);
      if(scase->fuelcoll.fuelinfo==NULL){
        scase->fuelcoll.nfuelinfo = nfuelinfo_local;
        NewMemory((void **)&scase->fuelcoll.fuelinfo, scase->fuelcoll.nfuelinfo*sizeof(fueldata));
      }
      else{
        scase->fuelcoll.nfuelinfo = MIN(nfuelinfo_local, scase->fuelcoll.nfuelinfo);
        ResizeMemory((void **)&scase->fuelcoll.fuelinfo, scase->fuelcoll.nfuelinfo*sizeof(fueldata));
      }

      for(i=0; i<nfuelinfo_local; i++){
        fueldata *fueli;

        FGETS(buffer, 255, stream);
        if(i<scase->fuelcoll.nfuelinfo){
          fueli = scase->fuelcoll.fuelinfo + i;
          sscanf(buffer, "%f", &(fueli->hoc));
        }
      }
      continue;
    }
    if(MatchSMV(buffer, "FUEL") == 1){
      int nfuelinfo_local;

      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i", &nfuelinfo_local);
      if(scase->fuelcoll.fuelinfo==NULL){
        scase->fuelcoll.nfuelinfo = nfuelinfo_local;
        NewMemory((void **)&scase->fuelcoll.fuelinfo, scase->fuelcoll.nfuelinfo*sizeof(fueldata));
      }
      else{
        scase->fuelcoll.nfuelinfo = MIN(nfuelinfo_local, scase->fuelcoll.nfuelinfo);
        ResizeMemory((void **)&scase->fuelcoll.fuelinfo, scase->fuelcoll.nfuelinfo*sizeof(fueldata));
      }

      for(i=0; i<nfuelinfo_local; i++){
        fueldata *fueli;

        FGETS(buffer, 255, stream);
        if(i<scase->fuelcoll.nfuelinfo){
          fueli = scase->fuelcoll.fuelinfo + i;
          fueli->fuel = GetStringPtr(buffer);
        }
      }
      continue;
    }
    if(MatchSMV(buffer, "TITLE")==1){
      char *fds_title_local;
      int len_title;

      FGETS(buffer, 255, stream);
      fds_title_local = TrimFrontBack(buffer);
      if(fds_title_local==NULL)continue;
      len_title = strlen(fds_title_local);
      if(len_title==0)continue;
      NewMemory((void **)&global_scase.fds_title, len_title+1);
      strcpy(global_scase.fds_title, fds_title_local);
      continue;
    }
    if(MatchSMV(buffer, "SOLID_HT3D")==1){
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%i", &scase->solid_ht3d);
      ONEORZERO(scase->solid_ht3d);
      if(scase->solid_ht3d==1)scase->show_slice_in_obst=GAS_AND_SOLID;
      continue;
    }
    if(MatchSMV(buffer, "IBLANK")==1){
      FGETS(buffer, 255, stream);
      if(scase->iblank_set_on_commandline==0){
        sscanf(buffer, "%i", &scase->use_iblank);
        scase->use_iblank = CLAMP(scase->use_iblank, 0, 1);
      }
      continue;
    }
    if(MatchSMV(buffer,"GVEC") == 1){
      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f",scase->gvecphys,scase->gvecphys+1,scase->gvecphys+2);
      scase->gvecunit[0]=scase->gvecphys[0];
      scase->gvecunit[1]=scase->gvecphys[1];
      scase->gvecunit[2]=scase->gvecphys[2];
      NORMALIZE3(scase->gvecunit);
      if(NORM3(scase->gvecphys)>0.0){
        scase->have_gvec=1;
      }
      continue;
    }
    if(MatchSMV(buffer,"CSVF") == 1){
      char *file_ptr;
      char buffer2[256];

      FGETS(buffer,255,stream);
      TrimBack(buffer);

      FGETS(buffer2,255,stream);
      TrimBack(buffer2);
      file_ptr=TrimFront(buffer2);
      if(FileExistsCaseDir(scase, file_ptr)==YES)scase->csvcoll.ncsvfileinfo++;
      continue;
    }
    if(MatchSMV(buffer, "CGEOM")==1){
      scase->ncgeominfo++;
      continue;
    }
    if(MatchSMV(buffer, "GEOM") == 1 ||
       MatchSMV(buffer, "SGEOM") == 1){
      scase->ngeominfo++;
      continue;
    }
    if(MatchSMV(buffer,"PROP") == 1){
      scase->propcoll.npropinfo++;
      continue;
    }
    if(MatchSMV(buffer,"SMOKEDIFF") == 1){
      scase->smokediff=1;
      continue;
    }
    if(MatchSMV(buffer,"ALBEDO") == 1){
      FGETS(buffer,255,stream);
      sscanf(buffer,"%f",&scase->smoke_albedo);
      scase->smoke_albedo = CLAMP(scase->smoke_albedo, 0.0, 1.0);
      scase->smoke_albedo_base = scase->smoke_albedo;
      continue;
    }
    if(MatchSMV(buffer, "NORTHANGLE")==1){
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%f", &scase->northangle);
      scase->northangle = CLAMP(scase->northangle, -180.0, 180.0);
      scase->have_northangle = 1;
      continue;
    }
    if(MatchSMV(buffer,"TERRAIN") == 1){
      scase->manual_terrain = 1;
      FGETS(buffer, 255, stream);
      scase->nterraininfo++;
      continue;
    }
    if(MatchSMV(buffer,"CLASS_OF_PARTICLES") == 1){
      scase->npartclassinfo++;
      continue;
    }
    if(MatchSMV(buffer,"AUTOTERRAIN") == 1){
      int len_buffer;
      char *buff2;

      scase->is_terrain_case = 1;
      scase->auto_terrain=1;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&scase->visTerrainType);
      scase->visTerrainType=CLAMP(scase->visTerrainType,0,4);
      if(scase->visTerrainType==TERRAIN_HIDDEN){
        if(scase->visOtherVents!=scase->visOtherVentsSAVE)scase->visOtherVents=scase->visOtherVentsSAVE;
      }
      else{
        if(scase->visOtherVents!=0){
          scase->visOtherVentsSAVE=scase->visOtherVents;
          scase->visOtherVents=0;
        }
      }
      scase->update_terrain_type = 1;
      FGETS(buffer,255,stream);
      buff2 = TrimFront(buffer);
      TrimBack(buff2);
      len_buffer = strlen(buff2);
      if(len_buffer>0&&strcmp(buff2, "null")!=0){
        scase->terrain_texture_coll.nterrain_textures = 1;
        NewMemory((void **)&scase->terrain_texture_coll.terrain_textures, sizeof(texturedata));
        NewMemory((void **)&(scase->terrain_texture_coll.terrain_textures->file), (len_buffer+1)*sizeof(char));
        strcpy(scase->terrain_texture_coll.terrain_textures->file, buff2);
      }
      have_auto_terrain_image=1;
      continue;
    }
    if(MatchSMV(buffer, "TERRAINIMAGE")==1){
      int len_buffer;
      char *buff2, *blank;

      scase->is_terrain_case = 1;
      if(have_auto_terrain_image == 1){
        FREEMEMORY(scase->terrain_texture_coll.terrain_textures->file);
        FREEMEMORY(scase->terrain_texture_coll.terrain_textures);
      }
      scase->terrain_texture_coll.nterrain_textures = 1;
      blank = strchr(buffer,' ');
      if(blank!=NULL){
        int nvals=0;

        sscanf(blank+1,"%i",&nvals);
        if(nvals!=0)scase->terrain_texture_coll.nterrain_textures = MAX(nvals,0);
      }


      if(scase->terrain_texture_coll.nterrain_textures>0){
        NewMemory((void **)&scase->terrain_texture_coll.terrain_textures, scase->terrain_texture_coll.nterrain_textures*sizeof(texturedata));

        for(i=0;i<scase->terrain_texture_coll.nterrain_textures;i++){
          FGETS(buffer, 255, stream);
          buff2 = TrimFrontBack(buffer);
          len_buffer = strlen(buff2);
          if(len_buffer>0&&strcmp(buff2, "null")!=0){
            NewMemory((void **)&scase->terrain_texture_coll.terrain_textures[i].file, (len_buffer+1)*sizeof(char));
            strcpy(scase->terrain_texture_coll.terrain_textures[i].file, buff2);
          }
        }
      }
      continue;
    }
    if(MatchSMV(buffer, "SKYIMAGE") == 1){
      char *buff2;
      int len_buffer;

      if(sky_texture != NULL){
        FREEMEMORY(sky_texture->file);
        FREEMEMORY(sky_texture);
      }
      nsky_texture = 1;
      NewMemory((void **)&sky_texture, nsky_texture * sizeof(texturedata));
      FGETS(buffer, 255, stream);
      buff2 = TrimFrontBack(buffer);
      len_buffer = strlen(buff2);
      sky_texture->file = NULL;
      if(len_buffer > 0 && strcmp(buff2, "null") != 0){
         NewMemory((void **)&sky_texture->file, (len_buffer + 1) * sizeof(char));
         strcpy(sky_texture->file, buff2);
      }
      continue;
    }
    if(
      (MatchSMV(buffer,"DEVICE") == 1)&&
      (MatchSMV(buffer,"DEVICE_ACT") != 1)
      ){
      FGETS(buffer,255,stream);
      FGETS(buffer,255,stream);
      scase->devicecoll.ndeviceinfo++;
      continue;
    }
    if(MatchSMV(buffer,"FDSVERSION")==1){
      int lenbuffer;
      char *buffptr;

      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      TrimBack(buffer);
      buffptr = TrimFront(buffer);
      lenbuffer = strlen(buffptr);
      if(lenbuffer>0){
        NewMemory((void **)&scase->fds_version,lenbuffer+1);
        NewMemory((void **)&scase->fds_githash, lenbuffer+1);
        strcpy(scase->fds_version,buffer);
        strcpy(scase->fds_githash, buffer);
      }
      else{
        NewMemory((void **)&scase->fds_version,7+1);
        NewMemory((void **)&scase->fds_githash, 7+1);
        strcpy(scase->fds_version,"unknown");
        strcpy(scase->fds_githash, "unknown");
      }
      continue;
    }
    if(MatchSMV(buffer,"TOFFSET")==1){
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      sscanf(buffer,"%f %f %f",scase->texture_origin,scase->texture_origin+1,scase->texture_origin+2);
      continue;
    }

    if(MatchSMV(buffer,"USETEXTURES") == 1){
      scase->usetextures=1;
      continue;
    }

    if(MatchSMV(buffer,"CADTEXTUREPATH") == 1||
       MatchSMV(buffer,"TEXTUREDIR") == 1){
         if(FGETS(buffer,255,stream)==NULL){
           BREAK;
         }
      TrimBack(buffer);
      {
        size_t texturedirlen;

        texturedirlen=strlen(TrimFront(buffer));
        if(texturedirlen>0){
          FREEMEMORY(scase->texturedir);
          NewMemory( (void **)&scase->texturedir,texturedirlen+1);
          strcpy(scase->texturedir,TrimFront(buffer));
        }
      }
      continue;
    }

    if(MatchSMV(buffer,"VIEWTIMES") == 1){
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      sscanf(buffer,"%f %f %i",&scase->tourcoll.tour_tstart,&scase->tourcoll.tour_tstop,&scase->tourcoll.tour_ntimes);
      scase->global_tbegin = scase->tourcoll.tour_tstart;
      scase->tload_begin   = scase->tourcoll.tour_tstart;

      scase->global_tend   = scase->tourcoll.tour_tstop;
      scase->tload_end     = scase->tourcoll.tour_tstop;
      if(scase->tourcoll.tour_ntimes<2)scase->tourcoll.tour_ntimes=2;
      ReallocTourMemory(&scase->tourcoll);
      continue;
    }
    if(MatchSMV(buffer,"OUTLINE") == 1){
      scase->noutlineinfo++;
      continue;
    }
    if(MatchSMV(buffer,"TICKS") == 1){
      scase->ntickinfo++;
      scase->ntickinfo_smv++;
      continue;
    }
    if(MatchSMV(buffer,"TRNX") == 1){
      scase->ntrnx++;
      continue;
    }
    if(MatchSMV(buffer,"TRNY") == 1){
      scase->ntrny++;
      continue;
    }
    if(MatchSMV(buffer,"TRNZ") == 1){
      scase->ntrnz++;
      continue;
    }
    if(MatchSMV(buffer,"SURFACE") ==1){
      n_surf_keywords++;
      continue;
    }
    if(MatchSMV(buffer,"GRID") == 1){
      GRIDpresent=1;
      scase->meshescoll.nmeshes++;
      continue;
    }
    if(MatchSMV(buffer,"OFFSET") == 1){
      scase->noffset++;
      continue;
    }
    if(MatchSMV(buffer,"PDIM") == 1){
      scase->npdim++;
      scase->setPDIM=1;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f %f %f %f",&scase->xbar0,&scase->xbar,&scase->ybar0,&scase->ybar,&scase->zbar0,&scase->zbar);
      continue;
    }
    if(MatchSMV(buffer,"OBST") == 1){
      scase->nOBST++;
      continue;
    }
    if(MatchSMV(buffer,"CADGEOM") == 1){
      n_cadgeom_keywords++;
      continue;
    }
    if(MatchSMV(buffer,"CVENT") == 1){
      scase->nCVENT++;
      continue;
    }
    if(MatchSMV(buffer,"VENT") == 1){
      scase->nVENT++;
      continue;
    }
    if(
      MatchSMV(buffer, "MINMAXBNDF") == 1 ||
      MatchSMV(buffer, "MINMAXPL3D") == 1 ||
      MatchSMV(buffer, "MINMAXSLCF") == 1
      ){
      do_pass4 = 1;
      continue;
    }

//-----------------
// count file types
//-----------------

//*** PRT5

    if(MatchSMV(buffer,"PRT5")==1||
       MatchSMV(buffer,"EVA5")==1
      ){
      ParsePRT5Count(scase);
      continue;
    }

//*** SLCF

    if( (MatchSMV(buffer,"SLCF") == 1)  ||
        (MatchSMV(buffer,"SLCC") == 1)  ||
        (MatchSMV(buffer, "SLCD") == 1) ||
        (MatchSMV(buffer,"SLCT") == 1)  ||
        (MatchSMV(buffer, "BNDS") == 1)
      ){
      int return_val;

      return_val = ParseSLCFCount(scase, NO_SCAN, stream, buffer, &nslicefiles);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      if(return_val==RETURN_CONTINUE){
        continue;
      }
      continue;
    }

//*** SMOKE3D

    if(
      MatchSMV(buffer, "SMOKE3D") == 1  ||
      MatchSMV(buffer, "SMOKF3D") == 1  ||
      MatchSMV(buffer, "SMOKG3D") == 1){
      ParseSMOKE3DCount(scase);
      continue;
    }

//*** BNDF

    if(MatchSMV(buffer, "BNDF") == 1 ||
       MatchSMV(buffer, "BNDC") == 1 ||
       MatchSMV(buffer, "BNDE") == 1
      || MatchSMV(buffer, "BNDS") == 1
      ){
      ParseBNDFCount(scase);
      continue;
    }

//*** ISOF

    if(MatchSMV(buffer,"ISOF") == 1||
       MatchSMV(buffer,"TISOF")==1||
       MatchSMV(buffer,"ISOG") == 1||
       MatchSMV(buffer, "TISOG")==1){
      ParseISOFCount(scase);
      continue;
    }

    if(MatchSMV(buffer,"ROOM") == 1){
      scase->isZoneFireModel=1;
      scase->nrooms++;
      continue;
    }
    if(MatchSMV(buffer,"FIRE") == 1){
      scase->nfires++;
      continue;
    }
    if(MatchSMV(buffer,"ZONE") == 1){
      scase->nzoneinfo++;
      continue;
    }
    if(MatchSMV(buffer, "VENTGEOM")==1||
       MatchSMV(buffer, "HFLOWGEOM")==1||
       MatchSMV(buffer, "VFLOWGEOM")==1||
       MatchSMV(buffer, "MFLOWGEOM")==1){
      scase->nzvents++;
      continue;
    }
    if(MatchSMV(buffer, "HVENTGEOM")==1||
       MatchSMV(buffer, "VVENTGEOM")==1||
       MatchSMV(buffer, "MVENTGEOM")==1){
      nzventsnew++;
      continue;
    }
    if(MatchSMV(buffer, "HVENTPOS") == 1 ||
       MatchSMV(buffer, "VVENTPOS") == 1 ||
       MatchSMV(buffer, "MVENTPOS") == 1){
      nzventsnew++;
      continue;
    }

  }
  STOP_TIMER(scase->pass1_time);

/*
   ************************************************************************
   ************************ end of pass 1 *********************************
   ************************************************************************
 */

  START_TIMER(scase->pass2_time);

 if(scase->fds_version==NULL){
   NewMemory((void **)&scase->fds_version,7+1);
   strcpy(scase->fds_version,"unknown");
 }
 if(scase->fds_githash==NULL){
   NewMemory((void **)&scase->fds_githash,7+1);
   strcpy(scase->fds_githash,"unknown");
 }
 if(scase->nisoinfo>0&&scase->meshescoll.nmeshes>0)nisos_per_mesh = MAX(scase->nisoinfo / scase->meshescoll.nmeshes,1);
 NewMemory((void **)&scase->csvcoll.csvfileinfo,(scase->csvcoll.ncsvfileinfo+CFAST_CSV_MAX+2)*sizeof(csvfiledata));
 scase->csvcoll.ncsvfileinfo=0;
 if(scase->ngeominfo>0){
   NewMemory((void **)&scase->geominfo,scase->ngeominfo*sizeof(geomdata));
   scase->ngeominfo=0;
 }
 if(scase->ncgeominfo>0){
   NewMemory((void **)&scase->cgeominfo, scase->ncgeominfo*sizeof(geomdata));
   scase->ncgeominfo = 0;
 }
 if(scase->propcoll.npropinfo>0){
   NewMemory((void **)&scase->propcoll.propinfo,scase->propcoll.npropinfo*sizeof(propdata));
   scase->propcoll.npropinfo=1; // the 0'th prop is the default human property
 }
 if(scase->nterraininfo>0){
   NewMemory((void **)&scase->terraininfo,scase->nterraininfo*sizeof(terraindata));
   scase->nterraininfo=0;
 }
 if(scase->npartclassinfo>=0){
   float rgb_class[4];
   partclassdata *partclassi;
   size_t len;

   NewMemory((void **)&scase->partclassinfo,(scase->npartclassinfo+1)*sizeof(partclassdata));

   // define a dummy class

   partclassi = scase->partclassinfo + scase->npartclassinfo;
   strcpy(buffer,"Default");
   TrimBack(buffer);
   len=strlen(buffer);
   partclassi->name=NULL;
   if(len>0){
     NewMemory((void **)&partclassi->name,len+1);
     STRCPY(partclassi->name,TrimFront(buffer));
   }

   rgb_class[0]=1.0;
   rgb_class[1]=0.0;
   rgb_class[2]=0.0;
   rgb_class[3]=1.0;
   partclassi->rgb=GetColorPtr(scase, rgb_class);

   partclassi->ntypes=0;
   partclassi->xyz=NULL;
   partclassi->maxpoints=0;
   partclassi->labels=NULL;

   NewMemory((void **)&partclassi->labels,sizeof(flowlabels));
   CreateNullLabel(partclassi->labels);

   scase->npartclassinfo=0;


 }

  ibartemp=2;
  jbartemp=2;
  kbartemp=2;

  /* --------- set up multi-block data structures ------------- */

  /*
     The keywords TRNX, TRNY, TRNZ, GRID, PDIM, OBST and VENT are not required
     BUT if any one is present then the number of each must be equal
  */

  if(scase->meshescoll.nmeshes==0&&scase->ntrnx==0&&scase->ntrny==0&&scase->ntrnz==0&&scase->npdim==0&&scase->nOBST==0&&scase->nVENT==0&&scase->noffset==0){
    scase->meshescoll.nmeshes=1;
    scase->ntrnx=1;
    scase->ntrny=1;
    scase->ntrnz=1;
    scase->npdim=1;
    scase->nOBST=1;
    scase->noffset=1;
  }
  else{
    if(scase->meshescoll.nmeshes>1){
      if((scase->meshescoll.nmeshes!=scase->ntrnx||scase->meshescoll.nmeshes!=scase->ntrny||scase->meshescoll.nmeshes!=scase->ntrnz||scase->meshescoll.nmeshes!=scase->npdim||scase->meshescoll.nmeshes!=scase->nOBST||scase->meshescoll.nmeshes!=scase->nVENT||scase->meshescoll.nmeshes!=scase->noffset)&&
         (scase->nCVENT!=0&&scase->nCVENT!=scase->meshescoll.nmeshes)){
        fprintf(stderr,"*** Error:\n");
        if(scase->meshescoll.nmeshes!=scase->ntrnx)fprintf(stderr,"*** Error:  found %i TRNX keywords, was expecting %i\n",scase->ntrnx,scase->meshescoll.nmeshes);
        if(scase->meshescoll.nmeshes!=scase->ntrny)fprintf(stderr,"*** Error:  found %i TRNY keywords, was expecting %i\n",scase->ntrny,scase->meshescoll.nmeshes);
        if(scase->meshescoll.nmeshes!=scase->ntrnz)fprintf(stderr,"*** Error:  found %i TRNZ keywords, was expecting %i\n",scase->ntrnz,scase->meshescoll.nmeshes);
        if(scase->meshescoll.nmeshes!=scase->npdim)fprintf(stderr,"*** Error:  found %i PDIM keywords, was expecting %i\n",scase->npdim,scase->meshescoll.nmeshes);
        if(scase->meshescoll.nmeshes!=scase->nOBST)fprintf(stderr,"*** Error:  found %i OBST keywords, was expecting %i\n",scase->nOBST,scase->meshescoll.nmeshes);
        if(scase->meshescoll.nmeshes!=scase->nVENT)fprintf(stderr,"*** Error:  found %i VENT keywords, was expecting %i\n",scase->nVENT,scase->meshescoll.nmeshes);
        if(scase->nCVENT!=0&&scase->meshescoll.nmeshes!=scase->nCVENT)fprintf(stderr,"*** Error:  found %i CVENT keywords, was expecting %i\n",scase->noffset,scase->meshescoll.nmeshes);
        return 2;
      }
    }
  }
  FREEMEMORY(scase->meshescoll.meshinfo);
  if(NewMemory((void **)&scase->meshescoll.meshinfo,scase->meshescoll.nmeshes*sizeof(meshdata))==0)return 2;
  for(i = 0; i < scase->meshescoll.nmeshes; i++){
    meshdata *meshi;

    meshi = scase->meshescoll.meshinfo + i;
    InitMesh(meshi); // initialize mesh here so order of order GRID/TERRAIN keywords won't cause a problem
  }
  FREEMEMORY(scase->supermeshinfo);
  if(NewMemory((void **)&scase->supermeshinfo,scase->meshescoll.nmeshes*sizeof(supermeshdata))==0)return 2;
  scase->meshescoll.meshinfo->plot3dfilenum=-1;
  UpdateCurrentMesh(scase->meshescoll.meshinfo);
  for(i=0;i<scase->meshescoll.nmeshes;i++){
    meshdata *meshi;
    supermeshdata *smeshi;

    smeshi = scase->supermeshinfo + i;
    smeshi->nmeshes=0;

    meshi=scase->meshescoll.meshinfo+i;
    meshi->ibar=0;
    meshi->jbar=0;
    meshi->kbar=0;
    meshi->nbptrs=0;
    meshi->nvents=0;
    meshi->ncvents=0;
    meshi->plotn=1;
    meshi->itextureoffset=0;
  }
  if(scase->setPDIM==0){
    meshdata *meshi;

    if(roomdefined==0){
      scase->xbar0 = 0.0;    scase->xbar = 1.0;
      scase->ybar0 = 0.0;    scase->ybar = 1.0;
      scase->zbar0 = 0.0;    scase->zbar = 1.0;
    }
    meshi=scase->meshescoll.meshinfo;
    meshi->xyz_bar0[XXX]=scase->xbar0;
    meshi->xyz_bar[XXX] =scase->xbar;
    meshi->xcen=(scase->xbar+scase->xbar0)/2.0;
    meshi->xyz_bar0[YYY]=scase->ybar0;
    meshi->xyz_bar[YYY] =scase->ybar;
    meshi->ycen=(scase->ybar+scase->ybar0)/2.0;
    meshi->xyz_bar0[ZZZ]=scase->zbar0;
    meshi->xyz_bar[ZZZ] =scase->zbar;
    meshi->zcen=(scase->zbar+scase->zbar0)/2.0;
  }

  // define labels and memory for default colorbars

  FREEMEMORY(scase->partinfo);
  if(scase->npartinfo!=0){
    if(NewMemory((void **)&scase->partinfo,scase->npartinfo*sizeof(partdata))==0)return 2;
  }

  FREEMEMORY(scase->slicecoll.vsliceinfo);
  FREEMEMORY(scase->slicecoll.sliceinfo);
  if(scase->slicecoll.nsliceinfo>0){
    if(NewMemory((void **)&scase->slicecoll.vsliceinfo,         3*scase->slicecoll.nsliceinfo*sizeof(vslicedata))==0    ||
       NewMemory((void **)&scase->slicecoll.sliceinfo,            scase->slicecoll.nsliceinfo*sizeof(slicedata))==0     ||
       NewMemory((void **)&scase->sliceinfoptrs,        scase->slicecoll.nsliceinfo*sizeof(slicedata *)) == 0 ||
       NewMemory((void **)&scase->subslice_menuindex,   scase->slicecoll.nsliceinfo*sizeof(int))==0           ||
       NewMemory((void **)&scase->msubslice_menuindex,  scase->slicecoll.nsliceinfo*sizeof(int))==0           ||
       NewMemory((void **)&scase->subvslice_menuindex,  scase->slicecoll.nsliceinfo*sizeof(int))==0           ||
       NewMemory((void **)&scase->msubvslice_menuindex, scase->slicecoll.nsliceinfo*sizeof(int))==0){
       return 2;
    }
    sliceinfo_copy=scase->slicecoll.sliceinfo;
  }
  if(scase->smoke3dcoll.nsmoke3dinfo>0){
    if(NewMemory( (void **)&scase->smoke3dcoll.smoke3dinfo, scase->smoke3dcoll.nsmoke3dinfo*sizeof(smoke3ddata))==0)return 2;
  }

  FREEMEMORY(scase->patchinfo);
  FREEMEMORY(scase->boundarytypes);
  if(scase->npatchinfo!=0){
    if(NewMemory((void **)&scase->patchinfo,scase->npatchinfo*sizeof(patchdata))==0)return 2;
    for(i=0;i<scase->npatchinfo;i++){
      patchdata *patchi;

      patchi = scase->patchinfo + i;
      patchi->reg_file=NULL;
      patchi->comp_file=NULL;
      patchi->file=NULL;
      patchi->size_file=NULL;
    }
    if(NewMemory((void **)&scase->boundarytypes,scase->npatchinfo*sizeof(int))==0)return 2;
  }
  FREEMEMORY(scase->isoinfo);
  FREEMEMORY(scase->isotypes);
  if(scase->nisoinfo>0){
    if(NewMemory((void **)&scase->isoinfo,scase->nisoinfo*sizeof(isodata))==0)return 2;
    if(NewMemory((void **)&scase->isotypes,scase->nisoinfo*sizeof(int))==0)return 2;
  }
  FREEMEMORY(scase->roominfo);
  if(scase->nrooms>0){
    if(NewMemory((void **)&scase->roominfo,(scase->nrooms+1)*sizeof(roomdata))==0)return 2;
  }
  FREEMEMORY(scase->fireinfo);
  if(scase->nfires>0){
    if(NewMemory((void **)&scase->fireinfo,scase->nfires*sizeof(firedata))==0)return 2;
  }
  FREEMEMORY(scase->zoneinfo);
  if(scase->nzoneinfo>0){
    if(NewMemory((void **)&scase->zoneinfo,scase->nzoneinfo*sizeof(zonedata))==0)return 2;
  }
  FREEMEMORY(scase->zventinfo);
  if(nzventsnew>0)scase->nzvents=nzventsnew;
  if(scase->nzvents>0){
    if(NewMemory((void **)&scase->zventinfo,scase->nzvents*sizeof(zventdata))==0)return 2;
  }
  scase->nzvents=0;
  scase->nzhvents=0;
  scase->nzvvents=0;
  scase->nzmvents = 0;

  FREEMEMORY(scase->texture_coll.textureinfo);
  FREEMEMORY(scase->surfcoll.surfinfo);
  if(NewMemory((void **)&scase->surfcoll.surfinfo,(n_surf_keywords+MAX_ISO_COLORS+1)*sizeof(surfdata))==0)return 2;

  ClearCADGeomCollection(&scase->cadgeomcoll);
  if (n_cadgeom_keywords > 0) {
    // Allocate a fixed-size collection large enough to hold each of the CADGEOM
    // definitions.
    int err = InitCADGeomCollection(&scase->cadgeomcoll, n_cadgeom_keywords);
    if (err != 0) return 2;
  }

  if(scase->noutlineinfo>0){
    if(NewMemory((void **)&scase->outlineinfo,scase->noutlineinfo*sizeof(outlinedata))==0)return 2;
    for(i=0;i<scase->noutlineinfo;i++){
      outlinedata *outlinei;

      outlinei = scase->outlineinfo + i;
      outlinei->x1=NULL;
      outlinei->x2=NULL;
      outlinei->y1=NULL;
      outlinei->y2=NULL;
      outlinei->z1=NULL;
      outlinei->z2=NULL;
    }
  }
  if(scase->ntickinfo>0){
    if(NewMemory((void **)&scase->tickinfo,scase->ntickinfo*sizeof(tickdata))==0)return 2;
    scase->ntickinfo=0;
    scase->ntickinfo_smv=0;
  }

  if(scase->propcoll.npropinfo>0){
    scase->propcoll.npropinfo=0;
    InitDefaultProp(scase);
    scase->propcoll.npropinfo=1;
  }

  if(scase->npartinfo>0 && NewMemory((void **)&scase->part_buffer,       3*scase->npartinfo*MAXFILELEN)    == 0)return 2;
  if(scase->slicecoll.nsliceinfo>0 && NewMemory((void **)&scase->slice_buffer,     7*scase->slicecoll.nsliceinfo*MAXFILELEN)   == 0)return 2;
  if(scase->smoke3dcoll.nsmoke3dinfo>0 && NewMemory((void **)&scase->smoke3d_buffer, 9*scase->smoke3dcoll.nsmoke3dinfo*MAXFILELEN) == 0)return 2;

  PRINT_TIMER(timer_readsmv, "pass 1");

/*
   ************************************************************************
   ************************ start of pass 2 *******************************
   ************************************************************************
 */

  startpass=1;
  ioffset=0;
  iobst=0;
  scase->noutlineinfo=0;
  if(scase->noffset==0)ioffset=1;

  REWIND(stream);
  PRINTF("%s","  pass 2\n");
  for(;;){
    if(FEOF(stream)!=0){
      BREAK;
    }
    if(GRIDpresent==0&&startpass==1){
      strcpy(buffer,"GRID");
      startpass=0;
    }
    else{
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      TrimBack(buffer);
      if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
    }
    if(MatchSMV(buffer, "PL3D")==1){
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++ HVACVALS ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "HVACVALS") == 1){
      int r =
          ParseHVACValsEntry(&scase->hvaccoll, stream );
      if (r == 1) {
        BREAK;
      }
      else if (r == 2) {
        continue;
      }
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ HVAC ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "HVAC") == 1){
      int r =
          ParseHVACEntry(&scase->hvaccoll, stream, scase->hvac_node_color, scase->hvac_duct_color);
      if (r == 1) {
        BREAK;
      }
      else if (r == 2) {
        continue;
      }
    }
      /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ CSVF ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"CSVF") == 1){
      csvfiledata *csvi;
      char *type_ptr, *file_ptr;
      char buffer2[256];

      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      TrimBack(buffer);
      type_ptr=TrimFront(buffer);

      if(FGETS(buffer2,255,stream)==NULL){
        BREAK;
      }
      TrimBack(buffer2);
      file_ptr=TrimFront(buffer2);
      if(FileExistsCaseDir(scase, file_ptr) == NO)continue;

      csvi = scase->csvcoll.csvfileinfo + scase->csvcoll.ncsvfileinfo;
      InitCSV(csvi, file_ptr, type_ptr, CSV_FDS_FORMAT);

      scase->csvcoll.ncsvfileinfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ TIMES +++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "TIMES")==1){
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%f %f", &scase->global_tbegin, &scase->global_tend);
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ BOXGEOM ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "BOXGEOM")==1){
      int nbounds = 0;

      TrimBack(buffer);
      if(strlen(buffer)>7){
        sscanf(buffer+7, "%i", &nbounds);
      }
      if(nbounds>0){
        float *bounds;

        NewMemory((void **)&bounds, 6*nbounds*sizeof(float));
        for(i = 0; i<nbounds; i++){
          float *xyz;

          xyz = bounds+6*i;
          FGETS(buffer, 255, stream);
          sscanf(buffer, "%f %f %f %f %f %f", xyz, xyz+1, xyz+2, xyz+3, xyz+4, xyz+5);
        }
        if(scase->ngeominfo>0){
          geomdata *geomi;

          geomi = scase->geominfo+scase->ngeominfo-1;
          for(i = 0; i<MIN(nbounds, geomi->ngeomobjinfo); i++){
            geomobjdata *geomobji;

            geomobji = geomi->geomobjinfo+i;
            geomobji->bounding_box = bounds + 6*i;
          }
        }
      }

    }

       /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ CGEOM ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "CGEOM")==1){
      geomdata *geomi;
      char *buff2;
      int have_vectors = CFACE_NORMALS_NO;

      geomi = scase->cgeominfo+scase->ncgeominfo;
      buff2 = buffer+6;
      sscanf(buff2, "%i", &have_vectors);
      if(have_vectors!=CFACE_NORMALS_YES)have_vectors=CFACE_NORMALS_NO;
      if(have_vectors == CFACE_NORMALS_YES)scase->have_cface_normals = CFACE_NORMALS_YES;
      InitGeom(geomi, GEOM_CGEOM, FDSBLOCK, have_vectors,-1);
      geomi->memory_id = ++scase->nmemory_ids;

      FGETS(buffer,255,stream);
      TrimBack(buffer);
      buff2 = TrimFront(buffer);
      NewMemory((void **)&geomi->file,strlen(buff2)+1);
      strcpy(geomi->file,buff2);
      scase->ncgeominfo++;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ GEOM ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "GEOM") == 1 ||
       MatchSMV(buffer, "SGEOM") == 1){
      geomdata *geomi;
      char *buff2;
      int ngeomobjinfo=0;
      int is_geom=0;

      geomi = scase->geominfo + scase->ngeominfo;
      geomi->ngeomobjinfo=0;
      geomi->geomobjinfo=NULL;
      geomi->memory_id = ++scase->nmemory_ids;

      TrimBack(buffer);
      if(strlen(buffer)>4){

        buff2 = buffer+5;
        sscanf(buff2,"%i",&ngeomobjinfo);
      }
      if(MatchSMV(buffer, "SGEOM") == 1){
        InitGeom(geomi, GEOM_SLICE, NOT_FDSBLOCK, CFACE_NORMALS_NO,-1);
      }
      else{
        is_geom = 1;
        InitGeom(geomi, GEOM_GEOM, FDSBLOCK, CFACE_NORMALS_NO,-1);
      }

      FGETS(buffer,255,stream);
      TrimBack(buffer);
      buff2 = TrimFront(buffer);
      NewMemory((void **)&geomi->file,strlen(buff2)+1);
      strcpy(geomi->file,buff2);

      geomi->file2 = NULL;
      if(parse_opts.fast_startup==0&&is_geom==1){
        char *ext;

        ext = strrchr(buff2,'.');
        if(ext!=NULL){
          ext[0] = 0;
          strcat(buff2,".ge2");
          if(FileExistsCaseDir(scase, buff2)==YES){
            NewMemory((void **)&geomi->file2,strlen(buff2)+1);
            strcpy(geomi->file2,buff2);
            ReadGeomFile2(geomi);
          }
        }
      }

      if(ngeomobjinfo>0){
        geomi->ngeomobjinfo = ngeomobjinfo;
        geomi->geomobj_offsets = NULL;
        NewMemory((void **)&geomi->geomobjinfo,ngeomobjinfo*sizeof(geomobjdata));
        int ntotal_triangles;

        ntotal_triangles = 0;
        for(i=0;i<ngeomobjinfo;i++){
          geomobjdata *geomobji;
          float *center;
          char *texture_mapping=NULL, *texture_vals=NULL;
          char *colorlabel;

          geomobji = geomi->geomobjinfo + i;

          geomobji->texture_name=NULL;
          geomobji->texture_mapping=TEXTURE_RECTANGULAR;
          geomobji->ntriangles = -1;

          FGETS(buffer,255,stream);

          colorlabel = strchr(buffer, '!');
          geomobji->color = NULL;
          geomobji->use_geom_color = 0;
          geomobji->bounding_box = NULL;
          if(colorlabel!=NULL){
            int colors[3] = {-1, -1, -1};
            float transparency = -1.0;
            int ntriangles;

            colorlabel++;
            if(colorlabel!=buffer)colorlabel[-1] = 0;
            ntriangles = -1;
            sscanf(colorlabel, "%i %i %i %f %i", colors, colors+1, colors+2, &transparency, &ntriangles);
            if(colors[0]>=0&&colors[1]>=0&&colors[2]>=0){
              float fcolors[4];

              fcolors[0] = colors[0]/255.0;
              fcolors[1] = colors[1]/255.0;
              fcolors[2] = colors[2]/255.0;
              if(transparency<0.0)transparency = 1.0;
              fcolors[3] = transparency;
              geomobji->color = GetColorPtr(scase, fcolors);
              geomobji->use_geom_color = 1;
            }
            geomobji->ntriangles = ntriangles;
            if(ntriangles > 0)ntotal_triangles += ntriangles;
          }

          texture_mapping = TrimFront(buffer);
          if(texture_mapping!=NULL)texture_vals = strchr(texture_mapping,' ');

          if(texture_vals!=NULL){
            char *surflabel;
            int is_terrain=0;

            texture_vals++;
            texture_vals[-1]=0;
            center = geomobji->texture_center;
            surflabel=strchr(texture_vals,'%');
            if(surflabel!=NULL){
              surflabel++;
              surflabel[-1] = 0;
              TrimBack(surflabel);
              surflabel=TrimFront(surflabel+1);
              geomi->surfgeom=GetSurface(scase, surflabel);
              if(geomobji->color==NULL)geomobji->color = geomi->surfgeom->color;
            }
            sscanf(texture_vals, "%f %f %f %i", center, center+1, center+2, &is_terrain);
            geomi->is_terrain = is_terrain;
          }
          if(geomi->is_terrain==1){
            scase->is_terrain_case = 1;
            scase->auto_terrain = 1;
          }
          if(texture_mapping!=NULL&&strcmp(texture_mapping,"SPHERICAL")==0){
            geomobji->texture_mapping=TEXTURE_SPHERICAL;
          }
        }
        geomi->ngeomobj_offsets = ntotal_triangles;
        geomi->geomobj_offsets  = NULL;
        if(ntotal_triangles>0){
          int count;

          NewMemory((void **)&geomi->geomobj_offsets,ntotal_triangles*sizeof(int));
          count = 0;
          for(i=0;i<ngeomobjinfo;i++){
            geomobjdata *geomobji;
            int j;

            geomobji = geomi->geomobjinfo + i;
            for(j=0;j<geomobji->ntriangles;j++){
              geomi->geomobj_offsets[count++] = i;
            }
          }
        }
      }

      scase->ngeominfo++;
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ OBST ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OBST") == 1&&scase->auto_terrain==1){
      int nobsts=0;
      meshdata *meshi;
      unsigned char *is_block_terrain;
      int nn;

      iobst++;

      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&nobsts);

      meshi=scase->meshescoll.meshinfo+iobst-1;

      if(nobsts<=0)continue;

      NewMemory((void **)&meshi->is_block_terrain,nobsts*sizeof(unsigned char));
      is_block_terrain=meshi->is_block_terrain;

      for(nn=0;nn<nobsts;nn++){
        FGETS(buffer,255,stream);
      }
      for(nn=0;nn<nobsts;nn++){
        int ijk2[6],colorindex_local=0,blocktype_local=-1;

        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %i %i %i %i %i %i %i",ijk2,ijk2+1,ijk2+2,ijk2+3,ijk2+4,ijk2+5,&colorindex_local,&blocktype_local);
        if(blocktype_local>=0&&(blocktype_local&8)==8){
          is_block_terrain[nn]=1;
        }
        else{
          is_block_terrain[nn]=0;
        }
      }
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ PROP ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"PROP") == 1){
      propdata *propi;
      char *file_buffer;
      char proplabel[255];
      int lenbuf;
      int ntextures_local;
      int nsmokeview_ids;
      char *smokeview_id;

      propi = scase->propcoll.propinfo + scase->propcoll.npropinfo;

      if(FGETS(proplabel,255,stream)==NULL){
        BREAK;  // prop label
      }
      TrimBack(proplabel);
      file_buffer=TrimFront(proplabel);

      if(FGETS(buffer,255,stream)==NULL){
        BREAK;  // number of smokeview_id's
      }
      sscanf(buffer,"%i",&nsmokeview_ids);

      InitProp(propi,nsmokeview_ids, file_buffer);
      for(i=0;i<nsmokeview_ids;i++){
        if(FGETS(buffer,255,stream)==NULL){
          BREAK; // smokeview_id
        }
        TrimBack(buffer);
        file_buffer =TrimFront(buffer);
        lenbuf=strlen(file_buffer);
        NewMemory((void **)&smokeview_id,lenbuf+1);
        strcpy(smokeview_id, file_buffer);
        propi->smokeview_ids[i]=smokeview_id;
        propi->smv_objects[i]=GetSmvObjectType(&scase->objectscoll,propi->smokeview_ids[i],scase->objectscoll.std_object_defs.missing_device);
      }
      propi->smv_object=propi->smv_objects[0];
      propi->smokeview_id=propi->smokeview_ids[0];

      if(FGETS(buffer,255,stream)==NULL){
        BREAK; // keyword_values
      }
      sscanf(buffer,"%i",&propi->nvars_indep);
      propi->vars_indep=NULL;
      propi->svals=NULL;
      propi->texturefiles=NULL;
      ntextures_local=0;
      if(propi->nvars_indep>0){
        NewMemory((void **)&propi->vars_indep,propi->nvars_indep*sizeof(char *));
        NewMemory((void **)&propi->svals,propi->nvars_indep*sizeof(char *));
        NewMemory((void **)&propi->fvals,propi->nvars_indep*sizeof(float));
        NewMemory((void **)&propi->vars_indep_index,propi->nvars_indep*sizeof(int));
        NewMemory((void **)&propi->texturefiles,propi->nvars_indep*sizeof(char *));

        for(i=0;i<propi->nvars_indep;i++){
          char *equal;

          propi->svals[i]=NULL;
          propi->vars_indep[i]=NULL;
          propi->fvals[i]=0.0;
          FGETS(buffer,255,stream);
          equal=strchr(buffer,'=');
          if(equal!=NULL){
            char *buf1, *buf2, *keyword, *val;
            int lenkey, lenval;
            char *texturefile;

            buf1=buffer;
            buf2=equal+1;
            *equal=0;

            TrimBack(buf1);
            keyword=TrimFront(buf1);
            lenkey=strlen(keyword);

            TrimBack(buf2);
            val=TrimFront(buf2);
            lenval=strlen(val);

            if(lenkey==0||lenval==0)continue;

            if(val[0]=='"'){
              val[0]=' ';
              if(val[lenval-1]=='"')val[lenval-1]=' ';
              TrimBack(val);
              val=TrimFront(val);
              NewMemory((void **)&propi->svals[i],lenval+1);
              strcpy(propi->svals[i],val);
              texturefile=strstr(val,"t%");
              if(texturefile!=NULL){
                texturefile+=2;
                texturefile=TrimFront(texturefile);
                propi->texturefiles[ntextures_local]=propi->svals[i];
                strcpy(propi->svals[i],texturefile);

                ntextures_local++;
              }
            }

            NewMemory((void **)&propi->vars_indep[i],lenkey+1);
            strcpy(propi->vars_indep[i],keyword);

            sscanf(val,"%f",propi->fvals+i);
          }
        }
        GetIndepVarIndices(propi->smv_object,propi->vars_indep,propi->nvars_indep,propi->vars_indep_index);
      }
      propi->ntextures=ntextures_local;
      scase->propcoll.npropinfo++;
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ TERRAIN +++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"TERRAIN") == 1){
      terraindata *terraini;
      int len_buffer;
      char *file, *buffer_ptr;
      int mesh_terrain = -1;

      if(strlen(buffer)>7){
        sscanf(buffer+7, "%i", &mesh_terrain);
      }

      FGETS(buffer,255,stream);
      buffer_ptr = TrimFrontBack(buffer);
      len_buffer = strlen(buffer_ptr);
      NewMemory((void **)&file, len_buffer+1);
      strcpy(file, buffer_ptr);

      terraini = scase->terraininfo + scase->nterraininfo;
      terraini->file = file;
      if(mesh_terrain==-1){
        mesh_terrain = scase->nterraininfo;    // no mesh_terrain on TERRAIN line so assume that number of TERRAIN and MESH lines are the same
      }
      else{
        mesh_terrain--;                 // mesh_terrain on TERRAIN line goes from 1 to number of meshes so subtract 1
      }
      scase->meshescoll.meshinfo[mesh_terrain].terrain = terraini;
      terraini->terrain_mesh = scase->meshescoll.meshinfo+mesh_terrain;
      terraini->defined = 0;
      scase->nterraininfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++ CLASS_OF_PARTICLES +++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    // CLASS_OF_PARTICLES
    //  name
    //  r g b (color)
    // ntypes
    // long label
    // short label
    // unit
    // ....
    // ...
    // ...
    // long label
    // short label
    // unit

    // 1'st type  hidden
    // 2'nd type  default (uniform)
    // 3'rd type first type read in
    // 2+ntypes  ntypes type read in

    if(MatchSMV(buffer,"CLASS_OF_PARTICLES") == 1){
      float rgb_class[4];
      partclassdata *partclassi;
      char *device_ptr;
      char *prop_id;
      size_t len;

      partclassi = scase->partclassinfo + scase->npartclassinfo;
      FGETS(buffer,255,stream);

      GetLabels(buffer,&device_ptr,&prop_id);
      if(prop_id!=NULL){
        device_ptr=NULL;
      }
      partclassi->prop=NULL;

      partclassi->sphere=NULL;
      partclassi->smv_device=NULL;
      partclassi->device_name=NULL;
      if(device_ptr!=NULL){
        partclassi->sphere=GetSmvObjectType(&scase->objectscoll,"SPHERE",scase->objectscoll.std_object_defs.missing_device);

        partclassi->smv_device=GetSmvObjectType(&scase->objectscoll,device_ptr,scase->objectscoll.std_object_defs.missing_device);
        if(partclassi->smv_device!=NULL){
          len = strlen(device_ptr);
          NewMemory((void **)&partclassi->device_name,len+1);
          STRCPY(partclassi->device_name,device_ptr);
        }
        else{
          char tube[10];

          strcpy(tube,"TUBE");
          len = strlen(tube);
          NewMemory((void **)&partclassi->device_name,len+1);
          STRCPY(partclassi->device_name,tube);
          partclassi->smv_device=GetSmvObjectType(&scase->objectscoll,tube,scase->objectscoll.std_object_defs.missing_device);
        }
      }

      TrimBack(buffer);
      len=strlen(buffer);
      partclassi->name=NULL;
      if(len>0){
        NewMemory((void **)&partclassi->name,len+1);
        STRCPY(partclassi->name,TrimFront(buffer));
      }

      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f",rgb_class,rgb_class+1,rgb_class+2);
      rgb_class[3]=1.0;
      partclassi->rgb=GetColorPtr(scase, rgb_class);

      partclassi->ntypes=0;
      partclassi->xyz=NULL;
      partclassi->maxpoints=0;
      partclassi->labels=NULL;

      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&partclassi->ntypes);
      partclassi->ntypes+=2;
      partclassi->nvars_dep=partclassi->ntypes-2+3; // subtract off two "dummies" at beginning and add 3 at end for r,g,b
      if(partclassi->ntypes>0){
        flowlabels *labelj;
        char shortdefaultlabel[]="Uniform";
        char longdefaultlabel[]="Uniform color";
        int j;

        NewMemory((void **)&partclassi->labels,partclassi->ntypes*sizeof(flowlabels));

        labelj = partclassi->labels; // placeholder for hidden

        labelj->longlabel=NULL;
        labelj->shortlabel=NULL;
        labelj->unit=NULL;

        labelj = partclassi->labels+1;  // placeholder for default

        labelj->longlabel=NULL;
        NewMemory((void **)&labelj->longlabel,strlen(longdefaultlabel)+1);
        strcpy(labelj->longlabel,longdefaultlabel);
        labelj->shortlabel=NULL;
        NewMemory((void **)&labelj->shortlabel,strlen(shortdefaultlabel)+1);
        strcpy(labelj->shortlabel,shortdefaultlabel);
        labelj->unit=NULL;

        partclassi->col_azimuth=-1;
        partclassi->col_diameter=-1;
        partclassi->col_elevation=-1;
        partclassi->col_length=-1;
        partclassi->col_u_vel=-1;
        partclassi->col_v_vel=-1;
        partclassi->col_w_vel=-1;
        partclassi->vis_type=PART_POINTS;
        for(j=2;j<partclassi->ntypes;j++){
          labelj = partclassi->labels+j;
          labelj->longlabel=NULL;
          labelj->shortlabel=NULL;
          labelj->unit=NULL;
          ReadLabels(labelj,stream,NULL);
          partclassi->vars_dep[j-2]=labelj->shortlabel;
          if(strcmp(labelj->shortlabel,"DIAMETER")==0){
            partclassi->col_diameter=j-2;
          }
          if(strcmp(labelj->shortlabel,"LENGTH")==0){
            partclassi->col_length=j-2;
          }
          if(strcmp(labelj->shortlabel,"AZIMUTH")==0){
            partclassi->col_azimuth=j-2;
          }
          if(strcmp(labelj->shortlabel,"ELEVATION")==0){
            partclassi->col_elevation=j-2;
          }
          if(STRCMP(labelj->shortlabel,"U-VEL")==0){
            partclassi->col_u_vel=j-2;
          }
          if(STRCMP(labelj->shortlabel,"V-VEL")==0){
            partclassi->col_v_vel=j-2;
          }
          if(STRCMP(labelj->shortlabel,"W-VEL")==0){
            partclassi->col_w_vel=j-2;
          }
        }
      }
      partclassi->diameter=1.0;
      partclassi->length=1.0;
      partclassi->azimuth=0.0;
      partclassi->elevation=0.0;
      partclassi->dx=0.0;
      partclassi->dy=0.0;
      partclassi->dz=0.0;
      if(device_ptr!=NULL){
        float diameter, length, azimuth, elevation;

        FGETS(buffer,255,stream);
        sscanf(buffer,"%f %f %f %f",&diameter,&length,&azimuth,&elevation);
        partclassi->diameter=diameter;
        partclassi->length=length;
        partclassi->azimuth=azimuth;
        partclassi->elevation=elevation;
      }
      scase->npartclassinfo++;
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ LABEL ++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"LABEL") == 1){

      /*
      LABEL
      x y z r g b tstart tstop
      label

      */
      {
        float *xyz, *frgbtemp, *tstart_stop;
        int *rgbtemp;
        labeldata labeltemp, *labeli;
        char *bufferptr;

        labeli = &labeltemp;

        xyz = labeli->xyz;
        frgbtemp = labeli->frgb;
        rgbtemp = labeli->rgb;
        tstart_stop = labeli->tstart_stop;

        labeli->labeltype=TYPE_SMV;
        FGETS(buffer,255,stream);
        frgbtemp[0]=-1.0;
        frgbtemp[1]=-1.0;
        frgbtemp[2]=-1.0;
        frgbtemp[3]=1.0;
        tstart_stop[0]=-1.0;
        tstart_stop[1]=-1.0;
        sscanf(buffer,"%f %f %f %f %f %f %f %f",
          xyz,xyz+1,xyz+2,
          frgbtemp,frgbtemp+1,frgbtemp+2,
          tstart_stop,tstart_stop+1);

        if(frgbtemp[0]<0.0||frgbtemp[1]<0.0||frgbtemp[2]<0.0||frgbtemp[0]>1.0||frgbtemp[1]>1.0||frgbtemp[2]>1.0){
          labeli->useforegroundcolor=1;
        }
        else{
          labeli->useforegroundcolor=0;
        }
        FGETS(buffer,255,stream);
        TrimBack(buffer);
        bufferptr = TrimFront(buffer);

        strcpy(labeli->name,bufferptr);
        rgbtemp[0]=frgbtemp[0]*255;
        rgbtemp[1]=frgbtemp[1]*255;
        rgbtemp[2]=frgbtemp[2]*255;
        LabelInsert(&scase->labelscoll, labeli);
      }
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TICKS ++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"TICKS") == 1){
      scase->ntickinfo++;
      scase->ntickinfo_smv++;
      {
        tickdata *ticki;
        float *begt, *endt;
        int *nbarst;
        float term;
        float length=0.0;
        float *dxyz;
        float sum;

        ticki = scase->tickinfo + scase->ntickinfo - 1;
        begt = ticki->begin;
        endt = ticki->end;
        nbarst=&ticki->nbars;
        dxyz = ticki->dxyz;

        if(FGETS(buffer,255,stream)==NULL){
          BREAK;
        }
        *nbarst=0;
        sscanf(buffer,"%f %f %f %f %f %f %i",begt,begt+1,begt+2,endt,endt+1,endt+2,nbarst);
        if(*nbarst<1)*nbarst=1;
        if(FGETS(buffer,255,stream)==NULL){
          BREAK;
        }
        {
          float *rgbtemp;

          rgbtemp=ticki->rgb;
          rgbtemp[0]=-1.0;
          rgbtemp[1]=-1.0;
          rgbtemp[2]=-1.0;
          ticki->width=-1.0;
          sscanf(buffer,"%f %i %f %f %f %f",&ticki->dlength,&ticki->dir,rgbtemp,rgbtemp+1,rgbtemp+2,&ticki->width);
          if(rgbtemp[0]<0.0||rgbtemp[0]>1.0||
             rgbtemp[1]<0.0||rgbtemp[1]>1.0||
             rgbtemp[2]<0.0||rgbtemp[2]>1.0){
            ticki->useforegroundcolor=1;
          }
          else{
            ticki->useforegroundcolor=0;
          }
          if(ticki->width<0.0)ticki->width=1.0;
        }
        for(i=0;i<3;i++){
          term = endt[i]-begt[i];
          length += term*term;
        }
        if(length<=0.0){
          endt[0]=begt[0]+1.0;
          length = 1.0;
        }
        ticki->length=sqrt(length);
        dxyz[0] =  0.0;
        dxyz[1] =  0.0;
        dxyz[2] =  0.0;
        switch(ticki->dir){
        case XRIGHT:
        case XLEFT:
          dxyz[0]=1.0;
          break;
        case YFRONT:
        case YBACK:
          dxyz[1]=1.0;
          break;
        case ZBOTTOM:
        case ZTOP:
          dxyz[2]=1.0;
          break;
        default:
          assert(FFALSE);
          break;
        }
        if(ticki->dir<0){
          for(i=0;i<3;i++){
            dxyz[i]=-dxyz[i];
          }
        }
        sum = dxyz[0]*dxyz[0] + dxyz[1]*dxyz[1] + dxyz[2]*dxyz[2];
        if(sum>0.0){
          sum=sqrt(sum);
          dxyz[0] *= (ticki->dlength/sum);
          dxyz[1] *= (ticki->dlength/sum);
          dxyz[2] *= (ticki->dlength/sum);
        }
      }
      continue;
    }


  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OUTLINE ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"OUTLINE") == 1){
      outlinedata *outlinei;

      scase->noutlineinfo++;
      outlinei = scase->outlineinfo + scase->noutlineinfo - 1;
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      sscanf(buffer,"%i",&outlinei->nlines);
      if(outlinei->nlines>0){
        NewMemory((void **)&outlinei->x1,outlinei->nlines*sizeof(float));
        NewMemory((void **)&outlinei->y1,outlinei->nlines*sizeof(float));
        NewMemory((void **)&outlinei->z1,outlinei->nlines*sizeof(float));
        NewMemory((void **)&outlinei->x2,outlinei->nlines*sizeof(float));
        NewMemory((void **)&outlinei->y2,outlinei->nlines*sizeof(float));
        NewMemory((void **)&outlinei->z2,outlinei->nlines*sizeof(float));
        for(i=0;i<outlinei->nlines;i++){
          FGETS(buffer,255,stream);
          sscanf(buffer,"%f %f %f %f %f %f",
            outlinei->x1+i,outlinei->y1+i, outlinei->z1+i,
            outlinei->x2+i,outlinei->y2+i, outlinei->z2+i
            );
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ CADGEOM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"CADGEOM") == 1){
      char *bufferptr;

      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      bufferptr=TrimFrontBack(buffer);
      if (FileExistsCaseDir(scase, bufferptr) == YES) {
        ReadCADGeomToCollection(&scase->cadgeomcoll, bufferptr, block_shininess);
      }
      else {
        PRINTF(_("***Error: CAD geometry file: %s could not be opened"),
               bufferptr);
        PRINTF("\n");
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OFFSET") == 1){
      ioffset++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SURFDEF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"SURFDEF") == 1){
      char *bufferptr;

      FGETS(buffer,255,stream);
      bufferptr=TrimFrontBack(buffer);
      strcpy(scase->surfacedefaultlabel,TrimFront(bufferptr));
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SURFACE ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"SURFACE") ==1){
      surfdata *surfi;
      float s_color[4];
      int s_type;
      float temp_ignition,emis,t_width, t_height;
      size_t len;
      char *buffer3;

      surfi = scase->surfcoll.surfinfo + scase->surfcoll.nsurfinfo;
      InitSurface(surfi);
      FGETS(buffer,255,stream);
      TrimBack(buffer);
      len=strlen(buffer);
      NewMemory((void **)&surfi->surfacelabel,(len+1)*sizeof(char));
      strcpy(surfi->surfacelabel,TrimFront(buffer));
      if(strstr(surfi->surfacelabel,"MIRROR")!=NULL||
         strstr(surfi->surfacelabel,"INTERPOLATED")!=NULL||
         strstr(surfi->surfacelabel,"OPEN")!=NULL){
        surfi->obst_surface=0;
      }
      if(strstr(surfi->surfacelabel,"INERT")!=NULL){
        scase->surfcoll.surfinfo[0].obst_surface=1;
      }

      temp_ignition=TEMP_IGNITION_MAX;
      emis = 1.0;
      t_width=1.0;
      t_height=1.0;
      s_type=0;
      s_color[0]=surfi->color[0];
      s_color[1]=surfi->color[1];
      s_color[2]=surfi->color[2];
      //s_color[3]=1.0;
      s_color[3]=surfi->color[3];
      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f",&temp_ignition,&emis);
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %f %f %f %f %f %f",
        &s_type,&t_width, &t_height,s_color,s_color+1,s_color+2,s_color+3);

      surfi->type=s_type;
      if(s_color[0]<=0.0&&s_color[1]<=0.0&&s_color[2]<=0.0){
        if(s_color[0]!=0.0||s_color[1]!=0.0||s_color[2]!=0.0){
          surfi->invisible=1;
          surfi->type=BLOCK_hidden;
          s_color[0]=-s_color[0];
          s_color[1]=-s_color[1];
          s_color[2]=-s_color[2];
        }
      }
      if(s_color[3]<=0.001){
        surfi->invisible = 1;
        surfi->type = BLOCK_hidden;
      }
      surfi->color = GetColorPtr(scase, s_color);
      if(s_color[3]<0.99){
        surfi->transparent=1;
        surfi->transparent_level = s_color[3];
      }
      else{
        surfi->transparent_level = 1.0;
      }
      surfi->color_orig = surfi->color;
      surfi->transparent_level_orig = surfi->transparent_level;
      surfi->geom_surf_color[0] = CLAMP(255*surfi->color[0],0,255);
      surfi->geom_surf_color[1] = CLAMP(255*surfi->color[1], 0, 255);
      surfi->geom_surf_color[2] = CLAMP(255*surfi->color[2], 0, 255);
      surfi->temp_ignition=temp_ignition;
      surfi->emis=emis;
      surfi->t_height=t_height;
      surfi->t_width=t_width;
      surfi->texturefile=NULL;
      surfi->textureinfo=NULL;

      FGETS(buffer,255,stream);
      TrimBack(buffer);
      buffer3 = TrimFront(buffer);
      {
        int found_texture;
        char texturebuffer[1024];

        found_texture=0;
        if(scase->texturedir!=NULL&&FileExistsCaseDir(scase, buffer3)==NO){
          STRCPY(texturebuffer,scase->texturedir);
          STRCAT(texturebuffer,dirseparator);
          STRCAT(texturebuffer,buffer3);
          if(FileExistsCaseDir(scase, texturebuffer)==YES){
            if(NewMemory((void **)&surfi->texturefile,strlen(texturebuffer)+1)==0)return 2;
            STRCPY(surfi->texturefile,texturebuffer);
            found_texture=1;
          }
        }
        if(FileExistsCaseDir(scase, buffer3)==YES){
          len=strlen(buffer3);
          if(NewMemory((void **)&surfi->texturefile,(unsigned int)(len+1))==0)return 2;
          STRCPY(surfi->texturefile,buffer3);
          found_texture=1;
        }
        if(buffer3!=NULL&&found_texture==0&&strncmp(buffer3,"null",4)!=0){
          fprintf(stderr,"*** Error: The texture file %s was not found\n",buffer3);
        }
      }
      scase->surfcoll.nsurfinfo++;
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"GRID") == 1){
      meshdata *meshi;
      float *xp, *yp, *zp;
      double *xpd, *ypd, *zpd;
      float *xp2, *yp2, *zp2;
      float *xplt_cen, *yplt_cen,*zplt_cen;
      int *imap, *jmap, *kmap;
      int mesh_nabors[6] = {-2, -2, -2, -2, -2, -2};

      igrid++;
      if(scase->meshescoll.meshinfo!=NULL){
        size_t len_meshlabel;
        char *meshlabel;

        meshi=scase->meshescoll.meshinfo+igrid-1;
        len_meshlabel=0;
        if(strlen(buffer)>5){
          meshlabel=TrimFront(buffer+5);
          TrimBack(meshlabel);
          len_meshlabel=strlen(meshlabel);
        }
        if(len_meshlabel>0){
          NewMemory((void **)&meshi->label,(len_meshlabel+1));
          strcpy(meshi->label,meshlabel);
        }
        else{
          sprintf(buffer,"%i",igrid);
          NewMemory((void **)&meshi->label,strlen(buffer)+1);
          strcpy(meshi->label,buffer);
        }
      }
      setGRID=1;
      if(GRIDpresent==0){
        ibartemp=2;
        jbartemp=2;
        kbartemp=2;
      }
      else{
        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %i %i %i %i %i %i %i %i",&ibartemp,&jbartemp,&kbartemp, 
          mesh_nabors, mesh_nabors+1, mesh_nabors+2, mesh_nabors+3, mesh_nabors+4, mesh_nabors+5);
          if(mesh_nabors[5]>=-1)have_mesh_nabors = 1;
      }
      if(ibartemp<1)ibartemp=1;
      if(jbartemp<1)jbartemp=1;
      if(kbartemp<1)kbartemp=1;
      xp=NULL; yp=NULL; zp=NULL;
      xpd = NULL; ypd = NULL; zpd = NULL;
      xp2=NULL; yp2=NULL; zp2=NULL;
      if(
         NewMemory((void **)&xp,sizeof(float)*(ibartemp+1))==0||
         NewMemory((void **)&yp,sizeof(float)*(jbartemp+1))==0||
         NewMemory((void **)&zp,sizeof(float)*(kbartemp+1))==0||
         NewMemory((void **)&xpd, sizeof(double)*(ibartemp + 1)) == 0 ||
         NewMemory((void **)&ypd, sizeof(double)*(jbartemp + 1)) == 0 ||
         NewMemory((void **)&zpd, sizeof(double)*(kbartemp + 1)) == 0 ||
         NewMemory((void **)&xplt_cen, sizeof(float) * ibartemp) == 0 ||
         NewMemory((void **)&yplt_cen,sizeof(float)*jbartemp)==0||
         NewMemory((void **)&zplt_cen,sizeof(float)*kbartemp)==0||
         NewMemory((void **)&xp2,sizeof(float)*(ibartemp+1))==0||
         NewMemory((void **)&yp2,sizeof(float)*(jbartemp+1))==0||
         NewMemory((void **)&zp2,sizeof(float)*(kbartemp+1))==0
         )return 2;
      if(
        NewMemory((void **)&imap, sizeof(int) * (ibartemp + 1)) == 0 ||
        NewMemory((void **)&jmap, sizeof(int) * (jbartemp + 1)) == 0 ||
        NewMemory((void **)&kmap, sizeof(int) * (kbartemp + 1)) == 0
        )return 2;
      if(scase->meshescoll.meshinfo!=NULL){
        meshi->xplt=xp;
        meshi->yplt=yp;
        meshi->zplt=zp;
        meshi->xpltd = xpd;
        meshi->ypltd = ypd;
        meshi->zpltd = zpd;
        meshi->xplt_cen=xplt_cen;
        meshi->yplt_cen=yplt_cen;
        meshi->zplt_cen=zplt_cen;
        meshi->xplt_orig=xp2;
        meshi->yplt_orig=yp2;
        meshi->zplt_orig=zp2;
        meshi->ibar=ibartemp;
        meshi->jbar=jbartemp;
        meshi->kbar=kbartemp;
        meshi->plotx=ibartemp/2;
        meshi->ploty=jbartemp/2;
        meshi->plotz=kbartemp/2;
        meshi->imap = imap;
        meshi->n_imap = 0;
        meshi->jmap = jmap;
        meshi->n_jmap = 0;
        meshi->kmap = kmap;
        meshi->n_kmap = 0;
        if(have_mesh_nabors == 1){
          for(i = 0; i < 6; i++){
            if(mesh_nabors[i] >= 1 && mesh_nabors[i] <= scase->meshescoll.nmeshes){
              meshi->nabors[i] = scase->meshescoll.meshinfo + mesh_nabors[i]-1;
            }
          }
          if(mesh_nabors[4] == 0)meshi->is_bottom = 1;
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ ZONE ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"ZONE") == 1){
      char *filename;
      zonedata *zonei;
      char buffer_csv[1000],*buffer_csvptr;
      char *period=NULL;
      int n;
      char *bufferptr;

      zonei = scase->zoneinfo + izone_local;
      if(FGETS(buffer,255,stream)==NULL){
        scase->nzoneinfo--;
        BREAK;
      }
      bufferptr=TrimFrontBack(buffer);
      zonei->loaded=0;
      zonei->display=0;

      buffer_csvptr=buffer_csv;
      strcpy(buffer_csv,bufferptr);
      filename= GetZoneFileName(buffer_csvptr);
      if(filename!=NULL)period=strrchr(filename,'.');
      if(filename!=NULL&&period!=NULL&&strcmp(period,".csv")==0){
        zonei->csv=1;
        scase->zonecsv=1;
      }
      else{
        zonei->csv=0;
        scase->zonecsv=0;
        filename= GetZoneFileName(bufferptr);
      }

      if(filename==NULL){
        int nn;

        for(nn=0;nn<4;nn++){
          if(ReadLabels(&zonei->label[nn],stream,NULL)==LABEL_ERR){
            return 2;
          }
        }
        scase->nzoneinfo--;
      }
      else{
        size_t len;

        len=strlen(filename);
        NewMemory((void **)&zonei->file,(unsigned int)(len+1));
        STRCPY(zonei->file,filename);
        for(n=0;n<4;n++){
          if(ReadLabels(&zonei->label[n],stream,NULL)==LABEL_ERR){
            return 2;
          }
        }
        izone_local++;
      }
      if(scase->colorlabelzone!=NULL){
        for(n=0;n<MAXRGB;n++){
          FREEMEMORY(scase->colorlabelzone[n]);
        }
        FREEMEMORY(scase->colorlabelzone);
      }
      CheckMemory;
      NewMemory((void **)&scase->colorlabelzone,MAXRGB*sizeof(char *));
      for(n=0;n<MAXRGB;n++){
        scase->colorlabelzone[n]=NULL;
      }
      for(n=0;n<scase->nrgb;n++){
        NewMemory((void **)&scase->colorlabelzone[n],11);
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ ROOM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"ROOM") == 1){
      roomdata *roomi;

      scase->isZoneFireModel=1;
      global_scase.visFrame=0;
      roomdefined=1;
      iroom++;
      roomi = scase->roominfo + iroom - 1;
      roomi->valid=0;
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      sscanf(buffer,"%f %f %f",&roomi->dx,&roomi->dy,&roomi->dz);
      roomi->valid=1;
      if(FGETS(buffer,255,stream)==NULL){
        roomi->x0=0.0;
        roomi->y0=0.0;
        roomi->z0=0.0;
      }
      else{
        sscanf(buffer,"%f %f %f",&roomi->x0,&roomi->y0,&roomi->z0);
      }
      roomi->x1=roomi->x0+roomi->dx;
      roomi->y1=roomi->y0+roomi->dy;
      roomi->z1=roomi->z0+roomi->dz;

      if(scase->setPDIM==0){
        if(roomi->x0<scase->xbar0)scase->xbar0=roomi->x0;
        if(roomi->y0<scase->ybar0)scase->ybar0=roomi->y0;
        if(roomi->z0<scase->zbar0)scase->zbar0=roomi->z0;
        if(roomi->x1>scase->xbar)scase->xbar=roomi->x1;
        if(roomi->y1>scase->ybar)scase->ybar=roomi->y1;
        if(roomi->z1>scase->zbar)scase->zbar=roomi->z1;
      }
      continue;
    }
  }

  STOP_TIMER(scase->pass2_time);

/*
   ************************************************************************
   ************************ end of pass 2 *********************************
   ************************************************************************
 */

  START_TIMER(scase->pass3_time);

  CheckMemory;
  ParseDatabase(scase, NULL);

  if(setGRID==0){
    meshdata *meshi;
    float *xp, *yp, *zp;
    float *xp2, *yp2, *zp2;
    float *xplt_cen, *yplt_cen, *zplt_cen;
    int  nn;

    xp=NULL; yp=NULL; zp=NULL;
    xp2=NULL; yp2=NULL; zp2=NULL;
    if(
       NewMemory((void **)&xp,sizeof(float)*(ibartemp+1))==0||
       NewMemory((void **)&yp,sizeof(float)*(jbartemp+1))==0||
       NewMemory((void **)&zp,sizeof(float)*(kbartemp+1))==0||
       NewMemory((void **)&xplt_cen,sizeof(float)*ibartemp)==0||
       NewMemory((void **)&yplt_cen,sizeof(float)*jbartemp)==0||
       NewMemory((void **)&zplt_cen,sizeof(float)*kbartemp)==0||
       NewMemory((void **)&xp2,sizeof(float)*(ibartemp+1))==0||
       NewMemory((void **)&yp2,sizeof(float)*(jbartemp+1))==0||
       NewMemory((void **)&zp2,sizeof(float)*(kbartemp+1))==0
       )return 2;
    for(nn=0;nn<=ibartemp;nn++){
      xp[nn]=scase->xbar0+(float)nn*(scase->xbar-scase->xbar0)/(float)ibartemp;
    }
    for(nn=0;nn<=jbartemp;nn++){
      yp[nn]=scase->ybar0+(float)nn*(scase->ybar-scase->ybar0)/(float)jbartemp;
    }
    for(nn=0;nn<=kbartemp;nn++){
      zp[nn]=scase->zbar0+(float)nn*(scase->zbar-scase->zbar0)/(float)kbartemp;
    }
    meshi=scase->meshescoll.meshinfo;
    meshi->xplt=xp;
    meshi->yplt=yp;
    meshi->zplt=zp;
    meshi->xplt_cen=xplt_cen;
    meshi->yplt_cen=yplt_cen;
    meshi->zplt_cen=zplt_cen;
    meshi->xplt_orig=xp2;
    meshi->yplt_orig=yp2;
    meshi->zplt_orig=zp2;
    meshi->ibar=ibartemp;
    meshi->jbar=jbartemp;
    meshi->kbar=kbartemp;
    meshi->plotx=ibartemp/2;
    meshi->ploty=jbartemp/2;
    meshi->plotz=kbartemp/2;
  }
  if(scase->setPDIM==0&&roomdefined==1){
    meshdata *meshi;

    meshi=scase->meshescoll.meshinfo;
    meshi->xyz_bar0[XXX]=scase->xbar0;
    meshi->xyz_bar[XXX] =scase->xbar;
    meshi->xcen=(scase->xbar+scase->xbar0)/2.0;
    meshi->xyz_bar0[YYY]=scase->ybar0;
    meshi->xyz_bar[YYY] =scase->ybar;
    meshi->ycen=(scase->ybar+scase->ybar0)/2.0;
    meshi->xyz_bar0[ZZZ]=scase->zbar0;
    meshi->xyz_bar[ZZZ] =scase->zbar;
    meshi->zcen=(scase->zbar+scase->zbar0)/2.0;
  }

  if(scase->devicecoll.ndeviceinfo>0){
    if(NewMemory((void **)&scase->devicecoll.deviceinfo,scase->devicecoll.ndeviceinfo*sizeof(devicedata))==0)return 2;
  }
  scase->devicecoll.ndeviceinfo=0;
  REWIND(stream);

  if(FileExistsCaseDir(scase, scase->paths.expcsv_filename)==YES){
    csvfiledata *csvi;
    char csv_type[256];

    csvi = scase->csvcoll.csvfileinfo + scase->csvcoll.ncsvfileinfo;
    strcpy(csv_type, "ext");
    InitCSV(csvi, scase->paths.expcsv_filename, csv_type, CSV_FDS_FORMAT);
    scase->csvcoll.ncsvfileinfo++;
  }

  PRINTF("%s","  pass 3\n");
  PRINT_TIMER(timer_readsmv, "pass 2");

  /*
   ************************************************************************
   ************************ start of pass 3 *******************************
   ************************************************************************
 */

  for(;;){
    if(FEOF(stream)!=0){
      BREAK;
    }
    if(FGETS(buffer,255,stream)==NULL){
      BREAK;
    }
    TrimBack(buffer);
    if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
    if(MatchSMV(buffer, "PL3D")==1){
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ AMBIENT ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"AMBIENT")==1){
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      sscanf(buffer,"%f %f %f",&scase->pref,&scase->pamb,&scase->tamb);
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++ CUTCELLS ++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"CUTCELLS") == 1){
      meshdata *meshi;
      int imesh,ncutcells;

      sscanf(buffer+10,"%i",&imesh);
      imesh=CLAMP(imesh-1,0,scase->meshescoll.nmeshes-1);
      meshi = scase->meshescoll.meshinfo + imesh;

      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&ncutcells);
      meshi->ncutcells=ncutcells;

      if(ncutcells>0){
        NewMemory((void **)&meshi->cutcells,ncutcells*sizeof(int));
        for(i=0;i<1+(ncutcells-1)/15;i++){
          int cc[15],j;

          FGETS(buffer,255,stream);
          for(j=0;j<15;j++){
            cc[j]=0;
          }
          sscanf(buffer,"%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
            cc,cc+1,cc+2,cc+3,cc+4,cc+5,cc+6,cc+7,cc+8,cc+9,cc+10,cc+11,cc+12,cc+13,cc+14);
          for(j=15*i;j<MIN(15*(i+1),ncutcells);j++){
            meshi->cutcells[j]=cc[j%15];
          }
        }
      }

      continue;
    }

/*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ DEVICE +++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    DEVICE
    label
    x y z xn yn zn state nparams ntextures
    p0 p1 ... p5
    p6 ...    p11
    texturefile1
    ...
    texturefilen

    */
    if(
      (MatchSMV(buffer,"DEVICE") == 1)&&
      (MatchSMV(buffer,"DEVICE_ACT") != 1)
      ){
      devicedata *devicei;

      devicei = scase->devicecoll.deviceinfo + scase->devicecoll.ndeviceinfo;
      ParseDevicekeyword(scase,stream,devicei);
      CheckMemory;
      scase->update_device = 1;
      scase->devicecoll.ndeviceinfo++;
      continue;
    }
  }

  STOP_TIMER(scase->pass3_time);

  /*
   ************************************************************************
   ************************ end of pass 3 *********************************
   ************************************************************************
 */

  START_TIMER(scase->pass4_time);

  // look for DEVICE entries in "experimental" spread sheet files

  if(scase->csvcoll.ncsvfileinfo>0){
    int *nexp_devices=NULL;

    NewMemory((void **)&nexp_devices,(scase->csvcoll.ncsvfileinfo+1)*sizeof(int));
    for(i=0;i<scase->csvcoll.ncsvfileinfo;i++){
      csvfiledata *csvi;

      csvi = scase->csvcoll.csvfileinfo + i;
      if(strcmp(csvi->c_type, "ext") == 0){
        nexp_devices[i] = GetNDevices(csvi->file);
        scase->devicecoll.ndeviceinfo_exp += nexp_devices[i];
      }
    }
    if(scase->devicecoll.ndeviceinfo>0){
      if(scase->devicecoll.ndeviceinfo_exp>0){
        ResizeMemory((void **)&scase->devicecoll.deviceinfo,(scase->devicecoll.ndeviceinfo_exp+scase->devicecoll.ndeviceinfo)*sizeof(devicedata));
      }
    }
    else{
      if(scase->devicecoll.ndeviceinfo_exp>0)NewMemory((void **)&scase->devicecoll.deviceinfo,scase->devicecoll.ndeviceinfo_exp*sizeof(devicedata));
    }
    if(scase->devicecoll.ndeviceinfo_exp >0){
      devicedata *devicecopy2;

      devicecopy2 = scase->devicecoll.deviceinfo+scase->devicecoll.ndeviceinfo;
      for(i=0;i<scase->csvcoll.ncsvfileinfo;i++){
        csvfiledata *csvi;

        csvi = scase->csvcoll.csvfileinfo + i;
        if(strcmp(csvi->c_type, "ext") == 0){
          ReadDeviceHeader(scase, csvi->file,devicecopy2,nexp_devices[i]);
          devicecopy2 += nexp_devices[i];
        }
      }
    }
    scase->devicecoll.ndeviceinfo += scase->devicecoll.ndeviceinfo_exp;
    FREEMEMORY(nexp_devices);
  }
  for(i = 0; i < scase->devicecoll.ndeviceinfo; i++){
    devicedata *devicei;

    devicei = scase->devicecoll.deviceinfo + i;
    if(strcmp(devicei->deviceID, "null") == 0){
      sprintf(devicei->deviceID, "DEV%03i", i + 1);
    }
  }

  // define texture data structures by constructing a list of unique file names from surfinfo and devices

/*
    Initialize blockage labels and blockage surface labels

    Define default surface for each block.
    Define default vent surface for each block.

  */

  global_scase.surfacedefault=&scase->sdefault;
  for(i=0;i<scase->surfcoll.nsurfinfo;i++){
    if(strcmp(scase->surfacedefaultlabel,scase->surfcoll.surfinfo[i].surfacelabel)==0){
      global_scase.surfacedefault=scase->surfcoll.surfinfo+i;
      break;
    }
  }
  global_scase.vent_surfacedefault=&scase->v_surfacedefault;
  for(i=0;i<scase->surfcoll.nsurfinfo;i++){
    if(strcmp(global_scase.vent_surfacedefault->surfacelabel,scase->surfcoll.surfinfo[i].surfacelabel)==0){
      global_scase.vent_surfacedefault=scase->surfcoll.surfinfo+i;
      break;
    }
  }

  scase->exterior_surfacedefault=&scase->e_surfacedefault;
  for(i=0;i<scase->surfcoll.nsurfinfo;i++){
    if(strcmp(scase->exterior_surfacedefault->surfacelabel,scase->surfcoll.surfinfo[i].surfacelabel)==0){
      scase->exterior_surfacedefault=scase->surfcoll.surfinfo+i;
      break;
    }
  }

  nvents=0;
  itrnx=0, itrny=0, itrnz=0, ipdim=0, iobst=0, ivent=0, icvent=0;
  ioffset=0;
  scase->npartclassinfo=0;
  if(scase->noffset==0)ioffset=1;
  PRINT_TIMER(timer_readsmv, "pass 3");

/*
   ************************************************************************
   ************************ start of pass 4 *******************************
   ************************************************************************
 */

  float ISOF_timer, SLCF_timer, BNDF_timer, SMOKE3D_timer, PRT5_timer;
  float cum_ISOF_timer=0.0, cum_SLCF_timer=0.0, cum_BNDF_timer=0.0, cum_SMOKE3D_timer=0.0, cum_PRT5_timer=0.0;

  REWIND(stream);
  PRINTF("%s","  pass 4\n");
  startpass=1;
  CheckMemory;

  for(;;){
    if(FEOF(stream)!=0){
      BREAK;
    }

    if(scase->nVENT==0){
      scase->nVENT=0;
      strcpy(buffer,"VENT");
    }
    else{
      if(startpass==1&&GRIDpresent==0){
        strcpy(buffer,"GRID");
        startpass=0;
      }
      else{
        if(FGETS(buffer,255,stream)==NULL){
          BREAK;
        }
        if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
      }
    }
    CheckMemory;
    if(MatchSMV(buffer, "PL3D")==1){
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++ CLASS_OF_PARTICLES +++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"CLASS_OF_PARTICLES") == 1){
      partclassdata *partclassi;
      char *device_ptr;
      char *prop_id;

      partclassi = scase->partclassinfo + scase->npartclassinfo;
      FGETS(buffer,255,stream);

      GetLabels(buffer,&device_ptr,&prop_id);
      partclassi->prop=GetPropID(scase, prop_id);
      UpdatePartClassDepend(partclassi);

      scase->npartclassinfo++;
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ HRRPUVCUT ++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"HRRPUVCUT") == 1){
      int nhrrpuvcut;

      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&nhrrpuvcut);
      if(nhrrpuvcut>=1){
        FGETS(buffer,255,stream);
        sscanf(buffer,"%f",&scase->global_hrrpuv_cutoff_default);
        scase->global_hrrpuv_cutoff = scase->global_hrrpuv_cutoff_default;
        scase->load_hrrpuv_cutoff = scase->global_hrrpuv_cutoff;
        for(i=1;i<nhrrpuvcut;i++){
          FGETS(buffer,255,stream);
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OFFSET") == 1){
      meshdata *meshi;

      ioffset++;
      meshi=scase->meshescoll.meshinfo+ioffset-1;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f",meshi->offset,meshi->offset+1,meshi->offset+2);
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++ HFLOWGEOM/VHFLOWGEOM/MFLOWGEOM +++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    have_zonevents=0;
    if(nzventsnew==0&&(
       MatchSMV(buffer, "VENTGEOM")==1||
       MatchSMV(buffer, "HFLOWGEOM")==1||
       MatchSMV(buffer, "VFLOWGEOM")==1||
       MatchSMV(buffer, "MFLOWGEOM")==1))have_zonevents=1;
    if(nzventsnew>0&&(
       MatchSMV(buffer, "HVENTGEOM")==1||
       MatchSMV(buffer, "VVENTGEOM")==1||
       MatchSMV(buffer, "MVENTGEOM")==1))have_zonevents=1;
    if(nzventsnew > 0 && (
      MatchSMV(buffer, "HVENTPOS") == 1 ||
      MatchSMV(buffer, "VVENTPOS") == 1 ||
      MatchSMV(buffer, "MVENTPOS") == 1))have_zonevents = 2;
    if(have_zonevents==1){
      int vent_type=HFLOW_VENT;
      int vertical_vent_type=0;
      zventdata *zvi;
      float vent_area;
      int roomfrom, roomto, wall;
      roomdata *roomi;
      float color[4];
      float vent_width,ventoffset,bottom,top;

      scase->nzvents++;
      zvi = scase->zventinfo + scase->nzvents - 1;
      if(MatchSMV(buffer,"VFLOWGEOM")==1||
         MatchSMV(buffer,"VVENTGEOM")==1)vent_type=VFLOW_VENT;
      if(MatchSMV(buffer, "MFLOWGEOM") == 1 ||
         MatchSMV(buffer, "MVENTGEOM") == 1)vent_type = MFLOW_VENT;
      zvi->vent_type=vent_type;
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      color[0]=1.0;
      color[1]=0.0;
      color[2]=1.0;
      color[3]=1.0;
      if(vent_type==HFLOW_VENT){
        float area_fraction=1.0;

        scase->nzhvents++;
        sscanf(buffer,"%i %i %i %f %f %f %f %f %f %f %f",
          &roomfrom,&roomto, &wall,&vent_width,&ventoffset,&bottom,&top,
          color,color+1,color+2,&area_fraction
          );

        zvi->area=vent_width*(top-bottom);

        if(roomfrom<1||roomfrom>scase->nrooms)roomfrom=scase->nrooms+1;
        roomi = scase->roominfo + roomfrom-1;
        zvi->room1 = roomi;

        if(roomto<1||roomto>scase->nrooms)roomto=scase->nrooms+1;
        zvi->room2=scase->roominfo+roomto-1;

        zvi->wall=wall;
        zvi->z0=roomi->z0+bottom;
        zvi->z1=roomi->z0+top;
        switch(wall){
        case FRONT_WALL:
          zvi->x0 = roomi->x0 + ventoffset;
          zvi->x1 = zvi->x0 + vent_width;
          zvi->y0 = roomi->y0;
          zvi->y1 = roomi->y0;
          break;
        case RIGHT_WALL:
          zvi->x0 = roomi->x1;
          zvi->x1 = roomi->x1;
          zvi->y0 = roomi->y0 + ventoffset;
          zvi->y1 = zvi->y0 + vent_width;
          break;
        case BACK_WALL:
          zvi->x0 = roomi->x0 + ventoffset;
          zvi->x1 = zvi->x0 + vent_width;
          zvi->y0 = roomi->y1;
          zvi->y1 = roomi->y1;
          break;
        case LEFT_WALL:
          zvi->x0 = roomi->x0;
          zvi->x1 = roomi->x0;
          zvi->y0 = roomi->y0 + ventoffset;
          zvi->y1 = zvi->y0 + vent_width;
          break;
        default:
          assert(FFALSE);
        }
        zvi->color = GetColorPtr(scase, color);
        zvi->area_fraction = area_fraction;
      }
      else if(vent_type==VFLOW_VENT){
        float ventside;
        float xcen, ycen;
        int r_from, r_to;
        float area_fraction=1.0;

        scase->nzvvents++;
        sscanf(buffer,"%i %i %i %f %i %f %f %f %f",
          &r_from,&r_to,&wall,&vent_area,&vertical_vent_type,
          color,color+1,color+2, &area_fraction
          );
        zvi->wall=wall;
        roomfrom=r_from;
        roomto=r_to;
        if(roomfrom<1||roomfrom>scase->nrooms){
          roomfrom=r_to;
          roomto=r_from;
          if(roomfrom<1||roomfrom>scase->nrooms){
            roomfrom=1;
          }
        }
        roomi = scase->roominfo + roomfrom - 1;
        vent_area=ABS(vent_area);
        ventside=sqrt(vent_area);
        xcen = (roomi->x0+roomi->x1)/2.0;
        ycen = (roomi->y0+roomi->y1)/2.0;
        if(wall==BOTTOM_WALL){
          zvi->z0 = roomi->z0;
          zvi->z1 = roomi->z0;
        }
        else{
          zvi->z0 = roomi->z1;
          zvi->z1 = roomi->z1;
        }
        switch(vertical_vent_type){
        case ZONEVENT_SQUARE:
        case ZONEVENT_CIRCLE:
          zvi->xcen = xcen;
          zvi->ycen = ycen;
          zvi->x0 = xcen - ventside/2.0;
          zvi->x1 = xcen + ventside/2.0;
          zvi->y0 = ycen - ventside/2.0;
          zvi->y1 = ycen + ventside/2.0;
          break;
        default:
          assert(FFALSE);
          break;
        }

        zvi->vertical_vent_type = vertical_vent_type;
        zvi->area = vent_area;
        zvi->area_fraction = area_fraction;
        zvi->color = GetColorPtr(scase, color);
      }
      else if(vent_type==MFLOW_VENT){
        scase->nzmvents++;
        ReadZVentData(scase, zvi, buffer, ZVENT_1ROOM);
      }
      CheckMemory;
      continue;
    }
    if(have_zonevents==2){
      int vent_type = HFLOW_VENT;
      zventdata *zvi;

      if(MatchSMV(buffer, "VFLOWPOS") == 1 ||
         MatchSMV(buffer, "VVENTPOS") == 1)vent_type = VFLOW_VENT;
      if(MatchSMV(buffer, "MFLOWPOS") == 1 ||
         MatchSMV(buffer, "MVENTPOS") == 1)vent_type = MFLOW_VENT;

      scase->nzvents++;
      zvi = scase->zventinfo + scase->nzvents - 1;

      zvi->vent_type = vent_type;
      if(FGETS(buffer, 255, stream) == NULL){
        BREAK;
      }

      CheckMemory;
      switch(vent_type){
      case HFLOW_VENT:
        scase->nzhvents++;
        ReadZVentData(scase, zvi, buffer,ZVENT_2ROOM);
        break;
      case VFLOW_VENT:
        scase->nzvvents++;
        ReadZVentData(scase, zvi, buffer, ZVENT_2ROOM);
        break;
      case MFLOW_VENT:
        scase->nzmvents++;
        ReadZVentData(scase, zvi, buffer, ZVENT_1ROOM);
        break;
      default:
        assert(FFALSE);
        break;
      }
      CheckMemory;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ FIRE ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"FIRE")==1){
      firedata *firei;
      int roomnumber;

      ifire++;
      if(FGETS(buffer,255,stream)==NULL){
        BREAK;
      }
      firei = scase->fireinfo + ifire - 1;
      sscanf(buffer,"%i %f %f %f",&roomnumber,&firei->x,&firei->y,&firei->z);
      if(roomnumber>=1&&roomnumber<=scase->nrooms){
        roomdata *roomi;

        roomi = scase->roominfo + roomnumber - 1;
        firei->valid=1;
        firei->roomnumber=roomnumber;
        firei->absx=roomi->x0+firei->x;
        firei->absy=roomi->y0+firei->y;
        firei->absz=roomi->z0+firei->z;
      }
      else{
        firei->valid=0;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PDIM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"PDIM") == 1){
      meshdata *meshi;
      float *meshrgb;

      ipdim++;
      meshi=scase->meshescoll.meshinfo+ipdim-1;
      meshrgb = meshi->meshrgb;

      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f %f %f %f %f %f %f",&scase->xbar0,&scase->xbar,&scase->ybar0,&scase->ybar,&scase->zbar0,&scase->zbar,meshrgb,meshrgb+1,meshrgb+2);

      if(meshrgb[0]!=0.0||meshrgb[1]!=0.0||meshrgb[2]!=0.0){
        meshi->meshrgb_ptr=meshi->meshrgb;
      }
      else{
        meshi->meshrgb_ptr=NULL;
      }

      meshi->xyz_bar0[XXX]=scase->xbar0;
      meshi->xyz_bar[XXX] =scase->xbar;
      meshi->xcen=(scase->xbar+scase->xbar0)/2.0;
      meshi->xyz_bar0[YYY]=scase->ybar0;
      meshi->xyz_bar[YYY] =scase->ybar;
      meshi->ycen =(scase->ybar+scase->ybar0)/2.0;
      meshi->xyz_bar0[ZZZ]=scase->zbar0;
      meshi->xyz_bar[ZZZ] =scase->zbar;
      meshi->zcen =(scase->zbar+scase->zbar0)/2.0;
      InitBoxClipInfo(meshi->box_clipinfo,scase->xbar0,scase->xbar,scase->ybar0,scase->ybar,scase->zbar0,scase->zbar);
      if(scase->ntrnx==0){
        int nn;

        for(nn = 0; nn<=meshi->ibar; nn++){
          meshi->xplt[nn] = scase->xbar0+(float)nn*(scase->xbar-scase->xbar0)/(float)meshi->ibar;
        }
      }
      if(scase->ntrny==0){
        int nn;

        for(nn = 0; nn<=meshi->jbar; nn++){
          meshi->yplt[nn] = scase->ybar0+(float)nn*(scase->ybar-scase->ybar0)/(float)meshi->jbar;
        }
      }
      if(scase->ntrnz==0){
        int nn;

        for(nn = 0; nn<=meshi->kbar; nn++){
          meshi->zplt[nn]=scase->zbar0+(float)nn*(scase->zbar-scase->zbar0)/(float)meshi->kbar;
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TRNX ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"TRNX")==1){
      float *xpltcopy;
      double *xpltdcopy;
      int idummy;
      int nn;

      itrnx++;
      xpltcopy=scase->meshescoll.meshinfo[itrnx-1].xplt;
      xpltdcopy=scase->meshescoll.meshinfo[itrnx - 1].xpltd;
      ibartemp=scase->meshescoll.meshinfo[itrnx-1].ibar;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i ",&idummy);
      for(nn=0;nn<idummy;nn++){
        FGETS(buffer,255,stream);
      }
      for(nn=0;nn<=ibartemp;nn++){
        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %lf",&idummy,xpltdcopy+nn);
        xpltcopy[nn] = xpltdcopy[nn];
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TRNY ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"TRNY") == 1){
      float *ypltcopy;
      double *ypltdcopy;
      int idummy;
      int nn;

      itrny++;
      ypltcopy=scase->meshescoll.meshinfo[itrny-1].yplt;
      ypltdcopy = scase->meshescoll.meshinfo[itrny - 1].ypltd;
      jbartemp=scase->meshescoll.meshinfo[itrny-1].jbar;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i ",&idummy);
      for(nn=0;nn<idummy;nn++){
        FGETS(buffer,255,stream);
      }
      for(nn=0;nn<=jbartemp;nn++){
        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %lf",&idummy,ypltdcopy+nn);
        ypltcopy[nn] = ypltdcopy[nn];
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TRNZ ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"TRNZ") == 1){
      float *zpltcopy;
      double *zpltdcopy;
      int idummy;
      int nn;

      itrnz++;
      zpltcopy=scase->meshescoll.meshinfo[itrnz-1].zplt;
      zpltdcopy=scase->meshescoll.meshinfo[itrnz - 1].zpltd;
      kbartemp=scase->meshescoll.meshinfo[itrnz-1].kbar;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i ",&idummy);
      for(nn=0;nn<idummy;nn++){
        FGETS(buffer,255,stream);
      }
      for(nn=0;nn<=kbartemp;nn++){
        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %lf",&idummy,zpltdcopy+nn);
        zpltcopy[nn] = zpltdcopy[nn];
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TREE ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    /*
typedef struct {
  float xyz[3];
  float trunk_diam;
  float tree_height;
  float base_diam;
  float base_height;
*/
    if(MatchSMV(buffer,"TREE") == 1){
      FGETS(buffer,255,stream);
      if(scase->ntreeinfo!=0)continue;
      sscanf(buffer,"%i",&scase->ntreeinfo);
      if(scase->ntreeinfo>0){
        NewMemory((void **)&scase->treeinfo,sizeof(treedata)*scase->ntreeinfo);
        for(i=0;i<scase->ntreeinfo;i++){
          treedata *treei;
          float *xyz;

          treei = scase->treeinfo + i;
          treei->time_char=-1.0;
          treei->time_complete=-1.0;
          xyz = treei->xyz;
          FGETS(buffer,255,stream);
          sscanf(buffer,"%f %f %f %f %f %f %f",
            xyz,xyz+1,xyz+2,
            &treei->trunk_diam,&treei->tree_height,
            &treei->base_diam,&treei->base_height);
        }
      }
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TREESTATE ++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if(MatchSMV(buffer, "TREESTATE")==1){
      int tree_index, tree_state;
      float tree_time;
      treedata *treei;

      if(scase->ntreeinfo==0)continue;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i %i %f",&tree_index,&tree_state,&tree_time);
      if(tree_index>=1&&tree_index<=scase->ntreeinfo){
        treei = scase->treeinfo + tree_index - 1;
        if(tree_state==1){
          treei->time_char = tree_time;
        }
        else if(tree_state==2){
          treei->time_complete = tree_time;
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OBST ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"OBST") == 1){
      meshdata *meshi;
      propdata *prop;
      char *proplabel;
      int n_blocks=0;
      int n_blocks_normal=0;
      unsigned char *is_block_terrain=NULL;
      int iblock;
      int nn;
      size_t len;

      CheckMemoryOff;
      iobst++;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&n_blocks);

      meshi=scase->meshescoll.meshinfo+iobst-1;
      if(n_blocks<=0)n_blocks=0;
      meshi->nbptrs=n_blocks;
      n_blocks_normal=n_blocks;
      if(n_blocks==0)continue;

      if(scase->auto_terrain==1||scase->manual_terrain==1){
        is_block_terrain=meshi->is_block_terrain;
        n_blocks_normal=0;
        for(iblock=0;iblock<n_blocks;iblock++){
          if(is_block_terrain==NULL||is_block_terrain[iblock]==0)n_blocks_normal++;
        }
        meshi->nbptrs=n_blocks_normal;
      }
      meshi->blockageinfoptrs=NULL;
      if(n_blocks_normal>0){
        NewMemory((void **)&meshi->blockageinfoptrs,sizeof(blockagedata *)*n_blocks_normal);
        NewMemory(( void ** )&meshi->blockageinfo,  sizeof(blockagedata)  *n_blocks_normal);
      }

      scase->ntotal_blockages+=n_blocks_normal;
      nn=-1;
      for(iblock=0;iblock<n_blocks;iblock++){
        int s_num[6];
        blockagedata *bc=NULL;

        if((scase->auto_terrain==1||scase->manual_terrain==1)&&meshi->is_block_terrain!=NULL&&meshi->is_block_terrain[iblock]==1){
          FGETS(buffer,255,stream);
          continue;
        }
        nn++;
        if(meshi->blockageinfo != NULL && meshi->blockageinfoptrs != NULL){
          meshi->blockageinfoptrs[nn] = meshi->blockageinfo + nn;
          bc = meshi->blockageinfoptrs[nn];
        }
        if(bc == NULL)continue;
        InitObst(scase,bc,global_scase.surfacedefault,nn+1,iobst-1);
        FGETS(buffer,255,stream);

        char id_label[100], *id_labelptr;

        id_labelptr = strchr(buffer, '!');
        if(id_labelptr == NULL){
          sprintf(id_label, "OB%i_%i", bc->meshindex+1,iblock+1);
          id_labelptr = id_label;
        }
        else{
          char *id_labelptr2;

          id_labelptr2 = id_labelptr;
          id_labelptr = TrimFrontBack(id_labelptr+1);
          *id_labelptr2 = 0;
        }
        bc->id_label = GetCharPtr(id_labelptr);

        TrimBack(buffer);
        for(i=0;i<6;i++){
          s_num[i]=-1;
        }
        proplabel=strchr(buffer,'%');
        prop=NULL;
        if(proplabel!=NULL){
          proplabel++;
          TrimBack(proplabel);
          proplabel = TrimFront(proplabel);
          for(i=0;i<scase->propcoll.npropinfo;i++){
            propdata *propi;

            propi = scase->propcoll.propinfo + i;
            if(STRCMP(proplabel,propi->label)==0){
              prop = propi;
              propi->inblockage=1;
              break;
            }
          }
        }
        bc->prop=prop;
        {
          float t_origin[3];
          float *xyzEXACT;

          t_origin[0]=scase->texture_origin[0];
          t_origin[1]=scase->texture_origin[1];
          t_origin[2]=scase->texture_origin[2];
          xyzEXACT = bc->xyzEXACT;
          sscanf(buffer,"%f %f %f %f %f %f %i %i %i %i %i %i %i %f %f %f",
            xyzEXACT,xyzEXACT+1,xyzEXACT+2,xyzEXACT+3,xyzEXACT+4,xyzEXACT+5,
            &(bc->blockage_id),s_num+DOWN_X,s_num+UP_X,s_num+DOWN_Y,s_num+UP_Y,s_num+DOWN_Z,s_num+UP_Z,
            t_origin,t_origin+1,t_origin+2);

          UpdateObstBoundingBox(scase, xyzEXACT);
          bc->xmin=xyzEXACT[0];
          bc->xmax=xyzEXACT[1];
          bc->ymin=xyzEXACT[2];
          bc->ymax=xyzEXACT[3];
          bc->zmin=xyzEXACT[4];
          bc->zmax=xyzEXACT[5];
          bc->texture_origin[0]=t_origin[0];
          bc->texture_origin[1]=t_origin[1];
          bc->texture_origin[2]=t_origin[2];
          if(bc->blockage_id<0){
            bc->changed=1;
            bc->blockage_id=-bc->blockage_id;
          }
        }

        /* define block label */

        sprintf(buffer,"**blockage %i",bc->blockage_id);
        len=strlen(buffer);
        ResizeMemory((void **)&bc->label,(len+1)*sizeof(char));
        strcpy(bc->label,buffer);

        for(i=0;i<6;i++){
          surfdata *surfi;

          if(scase->surfcoll.surfinfo==NULL||s_num[i]<0||s_num[i]>=scase->surfcoll.nsurfinfo)continue;
          surfi=scase->surfcoll.surfinfo+s_num[i];
          bc->surf[i]=surfi;
        }
        for(i=0;i<6;i++){
          bc->surf[i]->used_by_obst=1;
        }
        SetSurfaceIndex(scase, bc);
      }

      nn=-1;
      for(iblock=0;iblock<n_blocks;iblock++){
        blockagedata *bc=NULL;
        int *ijk;
        int colorindex, blocktype;

        if((scase->auto_terrain==1||scase->manual_terrain==1)&&meshi->is_block_terrain!=NULL&&meshi->is_block_terrain[iblock]==1){
          FGETS(buffer,255,stream);
          continue;
        }
        nn++;


        if(meshi->blockageinfoptrs != NULL && meshi->blockageinfoptrs[nn] != NULL){
          bc = meshi->blockageinfoptrs[nn];
        }
        if(bc == NULL)continue;
        colorindex=-1;
        blocktype=-1;

        /*
        OBST format:
        nblockages
        xmin xmax ymin ymax zmin zmax bid s_i1 s_i2 s_j1 s_j2 s_k1 s_k2 t_x0 t_y0 t_z0
        ...
        ...

        bid             - blockage id
        s_i1, ..., s_k2 - surf indices for i1, ..., k2 blockage faces
        t_x0,t_y0,t_z0  - texture origin

        i1 i2 j1 j2 k1 k2 colorindex blocktype r g b : ignore rgb if blocktype != -3
        ...
        ...

        int colorindex, blocktype;
        colorindex: -1 default color
                    -2 invisible
                    -3 use r g b color
                    >=0 color/color2/texture index
        blocktype: 0 regular block
                   2 outline
                   3 smoothed block
                   -1 (use type from surf)
                   (note: if blocktype&8 == 8 then this is a "terrain" blockage
                         if so then subtract 8 and set bc->is_wuiblock=1)
        r g b           colors used if colorindex==-3
        */

        FGETS(buffer,255,stream);
        {
          char *exclaim;
          int hidden6[6] = {-1,-1,-1,-1,-1,-1};

          memcpy(bc->hidden6, hidden6, 6*sizeof(int));
          exclaim = strchr(buffer, '!');
          if(exclaim != NULL){
            exclaim[0] = 0;
            exclaim = TrimFront(exclaim + 1);
            if(exclaim[0] == 'T' || exclaim[0] == 't')scase->have_removable_obsts = 1;
            exclaim++;
            if(strlen(exclaim) > 0){
              sscanf(exclaim, "%i %i %i %i %i %i",
                hidden6, hidden6 + 1, hidden6 + 2, hidden6 + 3, hidden6 + 4, hidden6 + 5);
              int ii;
              for(ii = 0; ii < 6; ii++){
                if(hidden6[ii] >= 0)hidden6[ii] = 1 - hidden6[ii];
              }
              memcpy(bc->hidden6, hidden6, 6*sizeof(int));
              if(hidden6[0] >= 0)scase->have_hidden6 = 1;
            }
          }
        }
        ijk = bc->ijk;
        sscanf(buffer,"%i %i %i %i %i %i %i %i",
          ijk,ijk+1,ijk+2,ijk+3,ijk+4,ijk+5,
          &colorindex,&blocktype);
        if((blocktype&3)==3)blocktype -= 3; // convert any smooth blocks to 'normal' blocks
        if(blocktype>0&&(blocktype&8)==8){
          bc->is_wuiblock=1;
          blocktype -= 8;
        }

        /* custom color */

        if(colorindex==0||colorindex==7)colorindex=-3;

        if(colorindex==-3){
          float s_color[4];

          ijk = bc->ijk;

          s_color[0]=-1.0;
          s_color[1]=-1.0;
          s_color[2]=-1.0;
          s_color[3]=1.0;

          sscanf(buffer,"%i %i %i %i %i %i %i %i %f %f %f %f",
            ijk,ijk+1,ijk+2,ijk+3,ijk+4,ijk+5,
            &colorindex,&blocktype,s_color,s_color+1,s_color+2,s_color+3);
          if(blocktype>0&&(blocktype&8)==8){
            bc->is_wuiblock=1;
            blocktype -= 8;
          }
          if(s_color[3]<0.999){
            bc->transparent=1;
          }
          if(colorindex==0||colorindex==7){
            switch(colorindex){
            case 0:
              s_color[0]=1.0;
              s_color[1]=1.0;
              s_color[2]=1.0;
              break;
            case 7:
              s_color[0]=0.0;
              s_color[1]=0.0;
              s_color[2]=0.0;
              break;
            default:
              assert(FFALSE);
              break;
            }
            colorindex=-3;
          }
          if(s_color[0]>=0.0&&s_color[1]>=0.0&&s_color[2]>=0.0){
            bc->color=GetColorPtr(scase, s_color);
          }
          bc->nnodes=(ijk[1]+1-ijk[0])*(ijk[3]+1-ijk[2])*(ijk[5]+1-ijk[4]);
          bc->useblockcolor = 1;
        }
        else{
          if(colorindex>=0){
            bc->color = GetColorPtr(scase, scase->rgb[scase->nrgb+colorindex]);
            bc->usecolorindex=1;
            bc->colorindex=colorindex;
            scase->updateindexcolors=1;
          }
        }

        bc->dup=0;
        bc->colorindex=colorindex;
        bc->type=blocktype;

        if(colorindex==COLOR_INVISIBLE){
          bc->type=BLOCK_hidden;
          bc->invisible=1;
        }
        if(bc->useblockcolor==0){
          bc->color=bc->surf[0]->color;
        }
        else{
          if(colorindex==-1){
            scase->updateindexcolors=1;
          }
        }
      }
      CheckMemoryOn;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ CVENT ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
        /*
        CVENT
        ncvents
        xmin xmax ymin ymax zmin zmax id surface_index tx ty tz % x0 y0 z0 radius
          ....  (ncvents rows)
          ....
        imin imax jmin jmax kmin kmax ventindex venttype r g b
          ....
          ....

          ventindex: -99 or 99 : use default color
                     +n or -n : use n'th palette color
                     < 0       : DO NOT draw boundary file over this vent
                     > 0       : DO draw boundary file over this vent
          vent type: 0 solid surface
                     2 outline
                     -2 hidden
          r g b:     red, green, blue color components
                     only specify if you wish to over-ride surface or default
                     range from 0.0 to 1.0
        */
    if(MatchSMV(buffer,"CVENT") == 1){
      cventdata *cvinfo;
      meshdata *meshi;
      float *origin;
      int ncv;
      int j;

      icvent++;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&ncv);

      meshi = scase->meshescoll.meshinfo + icvent - 1;
      meshi->cventinfo=NULL;
      meshi->ncvents=ncv;
      if(ncv==0)continue;
      scase->ncvents+=ncv;

      NewMemory((void **)&cvinfo,ncv*sizeof(cventdata));
      meshi->cventinfo=cvinfo;

      for(j=0;j<ncv;j++){
        cventdata *cvi;
        char *cbuf;
        int s_num[1];
        float t_origin[3];

        cvi = meshi->cventinfo + j;
        cvi->isOpenvent=0;
        cvi->surf[0]=global_scase.vent_surfacedefault;
        cvi->textureinfo[0]=NULL;
        cvi->texture_origin[0]=scase->texture_origin[0];
        cvi->texture_origin[1]=scase->texture_origin[1];
        cvi->texture_origin[2]=scase->texture_origin[2];
        cvi->useventcolor=1;
        cvi->hideboundary=0;
        cvi->have_boundary_file = 1;
        cvi->cvent_id=-1;
        cvi->color=NULL;
        cvi->blank=NULL;
        cvi->showtime = NULL;
        cvi->showhide = NULL;
        cvi->showtimelist = NULL;

        origin=cvi->origin;
        s_num[0]=-1;
        t_origin[0]=scase->texture_origin[0];
        t_origin[1]=scase->texture_origin[1];
        t_origin[2]=scase->texture_origin[2];
        FGETS(buffer,255,stream);
        sscanf(buffer,"%f %f %f %f %f %f %i %i %f %f %f",
          &cvi->xmin,&cvi->xmax,&cvi->ymin,&cvi->ymax,&cvi->zmin,&cvi->zmax,
          &cvi->cvent_id,s_num,t_origin,t_origin+1,t_origin+2);

        if(scase->surfcoll.surfinfo!=NULL&&s_num[0]>=0&&s_num[0]<scase->surfcoll.nsurfinfo){
          cvi->surf[0]=scase->surfcoll.surfinfo+s_num[0];
          if(cvi->surf[0]!=NULL&&strncmp(cvi->surf[0]->surfacelabel,"OPEN",4)==0){
            cvi->isOpenvent=1;
          }
          cvi->surf[0]->used_by_vent=1;
        }
        if(t_origin[0]<=-998.0){
          t_origin[0]=scase->texture_origin[0];
          t_origin[1]=scase->texture_origin[1];
          t_origin[2]=scase->texture_origin[2];
        }
        cvi->texture_origin[0]=t_origin[0];
        cvi->texture_origin[1]=t_origin[1];
        cvi->texture_origin[2]=t_origin[2];
        origin[0]=0.0;
        origin[1]=0.0;
        origin[2]=0.0;
        cvi->radius=0.0;
        cbuf=strchr(buffer,'%');
        if(cbuf!=NULL){
          TrimBack(cbuf);
          cbuf++;
          cbuf=TrimFront(cbuf);
          if(strlen(cbuf)>0){
            sscanf(cbuf,"%f %f %f %f",origin,origin+1,origin+2,&cvi->radius);
          }
        }
      }
      for(j=0;j<ncv;j++){
        cventdata *cvi;
        float *vcolor;
        int venttype,ventindex;
        float s_color[4],s2_color[4];
        int have_boundary_file;

        s2_color[0]=-1.0;
        s2_color[1]=-1.0;
        s2_color[2]=-1.0;
        s2_color[3]=1.0;

        cvi = meshi->cventinfo + j;

        // use properties from &SURF

        cvi->type=cvi->surf[0]->type;
        vcolor=cvi->surf[0]->color;
        cvi->color=vcolor;

        s_color[0]=vcolor[0];
        s_color[1]=vcolor[1];
        s_color[2]=vcolor[2];
        s_color[3]=vcolor[3];
        venttype=-99;
        have_boundary_file=1;

        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %i %i %i %i %i %i %i %f %f %f %i",
          &cvi->imin,&cvi->imax,&cvi->jmin,&cvi->jmax,&cvi->kmin,&cvi->kmax,
          &ventindex,&venttype,s2_color,s2_color+1,s2_color+2,&have_boundary_file);
        cvi->have_boundary_file = have_boundary_file;

        // use color from &VENT

        if(s2_color[0]>=0.0&&s2_color[1]>=0.0&&s2_color[2]>=0.0){
          s_color[0]=s2_color[0];
          s_color[1]=s2_color[1];
          s_color[2]=s2_color[2];
          cvi->useventcolor=1;
        }
        if(ventindex<0)cvi->hideboundary=1;
        if(venttype!=-99)cvi->type=venttype;

        // use pallet color

        if(ABS(ventindex)!=99){
          ventindex=ABS(ventindex);
          if(ventindex>scase->nrgb2-1)ventindex=scase->nrgb2-1;
          s_color[0]=scase->rgb[scase->nrgb+ventindex][0];
          s_color[1]=scase->rgb[scase->nrgb+ventindex][1];
          s_color[2]=scase->rgb[scase->nrgb+ventindex][2];
          s_color[3]=1.0;
          cvi->useventcolor=1;
        }
        s_color[3]=1.0; // set color to opaque until CVENT transparency is implemented
        cvi->color = GetColorPtr(scase, s_color);
      }
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ VENT ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

        /*
        new VENT format:

        xmin xmax ymin ymax zmin zmax ventid surf_index tx ty tz

        xyz min/max - vent boundaries
        ventid - index of vent
        surf_index - index of SURF
        tx, ty, yz - texture origin

        i1 i2 j1 j2 k1 k2 ventindex venttype r g b a

        ventindex: -99 or 99 : use default color
                   +n or -n : use n'th palette color
                   < 0       : DO NOT draw boundary file over this vent
                   > 0       : DO draw boundary file over this vent
        vent type: 0 solid surface
                   2 outline
                  -2 hidden
        r g b:     red, green, blue color components
                   only specify if you wish to over-ride surface or default
                   range from 0.0 to 1.0
        */

    if(MatchSMV(buffer,"VENT") == 1){
      meshdata *meshi;
      ventdata *vinfo;
      float *xplttemp,*yplttemp,*zplttemp;
      int nn;

      ivent++;
      meshi=scase->meshescoll.meshinfo+ivent-1;
      xplttemp=meshi->xplt;
      yplttemp=meshi->yplt;
      zplttemp=meshi->zplt;
      if(scase->nVENT==0){
        strcpy(buffer,"0 0");
        scase->nVENT=1;
      }
      else{
        FGETS(buffer,255,stream);
      }
      scase->ndummyvents=0;
      sscanf(buffer,"%i %i",&nvents,&scase->ndummyvents);
      if(scase->ndummyvents!=0){
        global_scase.visFloor=0;
        global_scase.visCeiling=0;
        global_scase.visWalls=0;
      }
      meshi->nvents=nvents;
      meshi->ndummyvents=scase->ndummyvents;
      vinfo=NULL;
      meshi->ventinfo=vinfo;
      if(NewMemory((void **)&vinfo,(nvents+12)*sizeof(ventdata))==0)return 2;
      meshi->ventinfo=vinfo;

      for(nn=0;nn<nvents+12;nn++){
        int s_num[6];
        ventdata *vi;

        vi=vinfo+nn;
        vi->patch_index = -1;
        vi->type=VENT_SOLID;
        vi->dummyptr=NULL;
        vi->transparent=0;
        vi->useventcolor=0;
        vi->usecolorindex=0;
        vi->nshowtime=0;
        vi->isOpenvent=0;
        vi->wall_type = INTERIORwall;
        vi->isMirrorvent = 0;
        vi->hideboundary=0;
        vi->surf[0]=global_scase.vent_surfacedefault;
        vi->textureinfo[0]=NULL;
        vi->texture_origin[0]=scase->texture_origin[0];
        vi->texture_origin[1]=scase->texture_origin[1];
        vi->texture_origin[2]=scase->texture_origin[2];
        vi->colorindex=-1;
        if(nn>nvents-scase->ndummyvents-1&&nn<nvents){
          vi->dummy=1;
        }
        else{
          vi->dummy=0;
        }
        s_num[0]=-1;
        if(nn<nvents){
          float t_origin[3];

          t_origin[0]=scase->texture_origin[0];
          t_origin[1]=scase->texture_origin[1];
          t_origin[2]=scase->texture_origin[2];
          FGETS(buffer,255,stream);
          sscanf(buffer,"%f %f %f %f %f %f %i %i %f %f %f",
                 &vi->xmin,&vi->xmax,&vi->ymin,&vi->ymax,&vi->zmin,&vi->zmax,
                 &vi->vent_id,s_num,t_origin,t_origin+1,t_origin+2);
          if(t_origin[0]<=-998.0){
            t_origin[0]=scase->texture_origin[0];
            t_origin[1]=scase->texture_origin[1];
            t_origin[2]=scase->texture_origin[2];
          }
          vi->texture_origin[0]=t_origin[0];
          vi->texture_origin[1]=t_origin[1];
          vi->texture_origin[2]=t_origin[2];
        }
        else{
          vi->xmin=meshi->xyz_bar0[XXX]+meshi->offset[XXX];
          vi->xmax=meshi->xyz_bar[XXX] +meshi->offset[XXX];
          vi->ymin=meshi->xyz_bar0[YYY]+meshi->offset[YYY];
          vi->ymax=meshi->xyz_bar[YYY] +meshi->offset[YYY];
          vi->zmin=meshi->xyz_bar0[ZZZ]+meshi->offset[ZZZ];
          vi->zmax=meshi->xyz_bar[ZZZ] +meshi->offset[ZZZ];
          s_num[0]=-1;
          switch(nn-nvents){
          case DOWN_Y:
          case DOWN_Y+6:
            vi->ymax=vi->ymin;
            break;
          case UP_X:
          case UP_X+6:
            vi->xmin=vi->xmax;
            break;
          case UP_Y:
          case UP_Y+6:
            vi->ymin=vi->ymax;
            break;
          case DOWN_X:
          case DOWN_X+6:
            vi->xmax=vi->xmin;
            break;
          case DOWN_Z:
          case DOWN_Z+6:
            vi->zmax=vi->zmin;
            break;
          case UP_Z:
          case UP_Z+6:
            vi->zmin=vi->zmax;
            break;
          default:
            assert(FFALSE);
            break;
          }
          if(nn>=nvents+6){
            vi->surf[0]=scase->exterior_surfacedefault;
          }
        }
        if(scase->surfcoll.surfinfo!=NULL&&s_num[0]>=0&&s_num[0]<scase->surfcoll.nsurfinfo){
          vi->surf[0]=scase->surfcoll.surfinfo+s_num[0];
          if(strncmp(vi->surf[0]->surfacelabel,"OPEN",4)==0)vi->isOpenvent=1;
          if(strncmp(vi->surf[0]->surfacelabel, "MIRROR", 6)==0)vi->isMirrorvent = 1;
          vi->surf[0]->used_by_vent=1;
        }
        vi->color_bak=NULL;
        if(scase->surfcoll.surfinfo!=NULL)vi->color_bak=scase->surfcoll.surfinfo[0].color;
      }
      for(nn=0;nn<nvents+12;nn++){
        ventdata *vi;
        int iv1, iv2, jv1, jv2, kv1, kv2;
        float s_color[4];
        int venttype,ventindex;
        char *cventdir;
        int ventdir;

        vi = vinfo+nn;
        vi->type=vi->surf[0]->type;
        vi->color=vi->surf[0]->color;
        s_color[0]=vi->surf[0]->color[0];
        s_color[1]=vi->surf[0]->color[1];
        s_color[2]=vi->surf[0]->color[2];
        s_color[3]=vi->surf[0]->color[3];
        venttype=-99;
        if(nn<nvents){
          float s2_color[4];

          s2_color[0]=-1.0;
          s2_color[1]=-1.0;
          s2_color[2]=-1.0;
          s2_color[3]=1.0;

          FGETS(buffer,255,stream);
          cventdir = strchr(buffer, '!');
          ventdir = 0;
          if(cventdir != NULL){
            *cventdir = 0;
            cventdir++;
            sscanf(cventdir, "%i", &ventdir);
          }

          sscanf(buffer,"%i %i %i %i %i %i %i %i %f %f %f %f",
               &iv1,&iv2,&jv1,&jv2,&kv1,&kv2,
               &ventindex,&venttype,
               s2_color,s2_color+1,s2_color+2,s2_color+3);

          // get vent direction
          if(ventdir == 0){
            vi->dir = DIR_UNDEFINED;
          }
          else if(ventdir>0){
            if(iv1 == iv2)vi->dir = UP_X;
            if(jv1 == jv2)vi->dir = UP_Y;
            if(kv1 == kv2)vi->dir = UP_Z;
          }
          else{
            if(iv1 == iv2)vi->dir = DOWN_X;
            if(jv1 == jv2)vi->dir = DOWN_Y;
            if(kv1 == kv2)vi->dir = DOWN_Z;
          }

          if(s2_color[0]>=0.0&&s2_color[1]>=0.0&&s2_color[2]>=0.0){
            s_color[0]=s2_color[0];
            s_color[1]=s2_color[1];
            s_color[2]=s2_color[2];

            if(s2_color[3]<0.99){
              vi->transparent=1;
              s_color[3]=s2_color[3];
            }
            vi->useventcolor=1;
            ventindex = SIGN(ventindex)*99;
          }
          if(ventindex<0)vi->hideboundary=1;
          if(venttype!=-99)vi->type=venttype;
          if(ABS(ventindex)!=99){
            ventindex=ABS(ventindex);
            if(ventindex>scase->nrgb2-1)ventindex=scase->nrgb2-1;
            s_color[0]=scase->rgb[scase->nrgb+ventindex][0];
            s_color[1]=scase->rgb[scase->nrgb+ventindex][1];
            s_color[2]=scase->rgb[scase->nrgb+ventindex][2];
            s_color[3]=1.0;
            vi->colorindex=ventindex;
            vi->usecolorindex=1;
            vi->useventcolor=1;
            scase->updateindexcolors=1;
          }
          vi->color = GetColorPtr(scase, s_color);
        }
        else{
          iv1=0;
          iv2 = meshi->ibar;
          jv1=0;
          jv2 = meshi->jbar;
          kv1=0;
          kv2 = meshi->kbar;
          ventindex=-99;
          vi->dir=nn-nvents;
          if(vi->dir>5)vi->dir-=6;
          vi->dir2=0;
          switch(nn-nvents){
          case DOWN_Y:
          case DOWN_Y+6:
            jv2=jv1;
            if(nn>=nvents+6)vi->dir=UP_Y;
            vi->dir2=YDIR;
            break;
          case UP_X:
          case UP_X+6:
            iv1=iv2;
            if(nn>=nvents+6)vi->dir=DOWN_X;
            vi->dir2=XDIR;
            break;
          case UP_Y:
          case UP_Y+6:
            jv1=jv2;
            if(nn>=nvents+6)vi->dir=DOWN_Y;
            vi->dir2=YDIR;
            break;
          case DOWN_X:
          case DOWN_X+6:
            iv2=iv1;
            if(nn>=nvents+6)vi->dir=UP_X;
            vi->dir2=XDIR;
            break;
          case DOWN_Z:
          case DOWN_Z+6:
            kv2=kv1;
            if(nn>=nvents+6)vi->dir=UP_Z;
            vi->dir2=ZDIR;
            break;
          case UP_Z:
          case UP_Z+6:
            kv1=kv2;
            if(nn>=nvents+6)vi->dir=DOWN_Z;
            vi->dir2=ZDIR;
            break;
          default:
            assert(FFALSE);
            break;
          }
        }
        if(vi->transparent==1)scase->nvent_transparent++;
        vi->linewidth=&scase->ventlinewidth;
        vi->showhide=NULL;
        vi->showtime=NULL;
        vi->showtimelist=NULL;
        vi->xvent1 = xplttemp[iv1];
        vi->xvent2 = xplttemp[iv2];
        vi->yvent1 = yplttemp[jv1];
        vi->yvent2 = yplttemp[jv2];
        vi->zvent1 = zplttemp[kv1];
        vi->zvent2 = zplttemp[kv2];
        vi->xvent1_orig = xplttemp[iv1];
        vi->xvent2_orig = xplttemp[iv2];
        vi->yvent1_orig = yplttemp[jv1];
        vi->yvent2_orig = yplttemp[jv2];
        vi->zvent1_orig = zplttemp[kv1];
        vi->zvent2_orig = zplttemp[kv2];
        vi->imin = iv1;
        vi->imax = iv2;
        vi->jmin = jv1;
        vi->jmax = jv2;
        vi->kmin = kv1;
        vi->kmax = kv2;
        if(nn>=nvents&&nn<nvents+6){
          vi->color=foregroundcolor;
        }
        assert(vi->color!=NULL);
      }
      for(nn=0;nn<nvents-scase->ndummyvents;nn++){
        int j;
        ventdata *vi;

        vi = meshi->ventinfo + nn;
        for(j=nvents-scase->ndummyvents;j<nvents;j++){ // look for dummy vent that matches real vent
          ventdata *vj;

          vj = meshi->ventinfo + j;
          if(vi->imin!=vj->imin&&vi->imax!=vj->imax)continue;
          if(vi->jmin!=vj->jmin&&vi->jmax!=vj->jmax)continue;
          if(vi->kmin!=vj->kmin&&vi->kmax!=vj->kmax)continue;
          vi->dummyptr=vj;
          vj->dummyptr=vi;
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ CHID +++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"CHID") == 1){
      int return_val;

      return_val = ParseCHIDProcess(scase, stream, NO_SCAN);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      else if(return_val==RETURN_CONTINUE){
        continue;
      }
      else{
        assert(FFALSE);
      }
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SMOKE3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      MatchSMV(buffer,"SMOKE3D") == 1 ||
      MatchSMV(buffer,"SMOKF3D") == 1 ||
      MatchSMV(buffer, "SMOKG3D") == 1){
      int return_val;

      START_TIMER(SMOKE3D_timer);
      return_val = ParseSMOKE3DProcess(scase, stream, buffer, &nn_smoke3d, &ioffset, &ismoke3dcount, &ismoke3d);
      CUM_TIMER(SMOKE3D_timer, cum_SMOKE3D_timer);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      else if(return_val==RETURN_CONTINUE){
        continue;
      }
      else if(return_val==RETURN_TWO){
        return 2;
      }
      else{
        assert(FFALSE);
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PART ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer,"PRT5")==1||MatchSMV(buffer,"EVA5")==1){
      int return_val;

      START_TIMER(PRT5_timer);
      return_val = ParsePRT5Process(scase, stream, buffer, &nn_part, &ipart, &ioffset);
      CUM_TIMER(PRT5_timer, cum_PRT5_timer);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      else if(return_val==RETURN_CONTINUE){
        continue;
      }
      else if(return_val==RETURN_TWO){
        return 2;
      }
      else{
        assert(FFALSE);
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if( (MatchSMV(buffer,"SLCF") == 1)  ||
        (MatchSMV(buffer,"SLCC") == 1)  ||
        (MatchSMV(buffer, "SLCD") == 1) ||
        (MatchSMV(buffer,"SLCT") == 1)  ||
        (MatchSMV(buffer, "BNDS") == 1)
      ){
      int return_val;

      START_TIMER(SLCF_timer);
      return_val = ParseSLCFProcess(scase, NO_SCAN, stream, buffer, &nn_slice, ioffset, &nslicefiles, &sliceinfo_copy, &patchgeom, buffers);
      CUM_TIMER(SLCF_timer, cum_SLCF_timer);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      else if(return_val==RETURN_CONTINUE){
        continue;
      }
      else if(return_val==RETURN_TWO){
        return 2;
      }
      else if(return_val==RETURN_PROCEED){
      }
      else{
        assert(FFALSE);
      }
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ BNDF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "BNDF") == 1 ||
       MatchSMV(buffer, "BNDC") == 1 ||
       MatchSMV(buffer, "BNDE") == 1 ||
       MatchSMV(buffer, "BNDS")==1
      ){
      int return_val;

      START_TIMER(BNDF_timer);
      return_val = ParseBNDFProcess(scase, stream, buffer, &nn_patch, &ioffset, &patchgeom, &ipatch, buffers);
      CUM_TIMER(BNDF_timer, cum_BNDF_timer);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      else if(return_val==RETURN_CONTINUE){
        continue;
      }
      else if(return_val==RETURN_TWO){
        return 2;
      }
      else{
        assert(FFALSE);
      }
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ ISOF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(MatchSMV(buffer,"ISOF") == 1||
       MatchSMV(buffer,"TISOF")==1||
       MatchSMV(buffer,"ISOG") == 1||
       MatchSMV(buffer, "TISOG")==1){
      int return_val;

      START_TIMER(ISOF_timer);
      return_val = ParseISOFProcess(scase, stream, buffer, &iiso, &ioffset, &nn_iso, nisos_per_mesh);
      CUM_TIMER(ISOF_timer, cum_ISOF_timer);
      if(return_val==RETURN_BREAK){
        BREAK;
      }
      else if(return_val==RETURN_CONTINUE){
        continue;
      }
      else{
        assert(FFALSE);
      }
      continue;
    }

  }

  STOP_TIMER(scase->pass4_time);

/*
   ************************************************************************
   ************************ end of pass 4 *********************************
   ************************************************************************
 */

  START_TIMER(scase->pass5_time);

  if(scase->auto_terrain==1&&scase->manual_terrain==0){
    scase->nOBST=0;
    iobst=0;
  }

  PRINT_CUM_TIMER(cum_BNDF_timer, "BNDF");
  PRINT_CUM_TIMER(cum_ISOF_timer, "ISOF");
  PRINT_CUM_TIMER(cum_PRT5_timer, "PRT5");
  PRINT_CUM_TIMER(cum_SLCF_timer, "SLCF");
  PRINT_CUM_TIMER(cum_SMOKE3D_timer, "SMOKE3D");
  PRINT_TIMER(timer_readsmv, "pass 4");

  /*
   ************************************************************************
   ************************ start of pass 5 *******************************
   ************************************************************************
 */

  REWIND(stream);
  if(do_pass4==1||(scase->auto_terrain==1&&scase->manual_terrain==0)){
    do_pass5 = 1;
    PRINTF("%s","  pass 5\n");
  }

  while(((scase->auto_terrain==1&&scase->manual_terrain==0)||do_pass4==1)){
    if(FEOF(stream)!=0){
      BREAK;
    }

    if(FGETS(buffer,255,stream)==NULL){
      BREAK;
    }
    if(strncmp(buffer," ",1)==0||buffer[0]==0)continue;
    if(MatchSMV(buffer, "PL3D")==1){
      continue;
    }

    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ MINMAXBNDF +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */

    if(MatchSMV(buffer, "MINMAXBNDF")==1){
      char *file_ptr, file2_local[1024];
      float valmin, valmax;
      float percentile_min, percentile_max;

      FGETS(buffer,255,stream);
      strcpy(file2_local,buffer);
      TrimBack(file2_local);
      file_ptr = TrimFront(file2_local);

      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f %f",&valmin,&valmax,&percentile_min,&percentile_max);

      for(i=0;i<scase->npatchinfo;i++){
        patchdata *patchi;

        patchi = scase->patchinfo + i;
        if(strcmp(file_ptr,patchi->file)==0){
          patchi->diff_valmin=percentile_min;
          patchi->diff_valmax=percentile_max;
          break;
        }
      }
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ MINMAXSLCF +++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if(MatchSMV(buffer, "MINMAXSLCF")==1){
      char *file_ptr, file2_local[1024];
      float valmin, valmax;
      float percentile_min, percentile_max;

      FGETS(buffer,255,stream);
      strcpy(file2_local,buffer);
      TrimBack(file2_local);
      file_ptr = TrimFront(file2_local);

      FGETS(buffer,255,stream);
      sscanf(buffer,"%f %f %f %f",&valmin,&valmax,&percentile_min,&percentile_max);

      for(i=0;i<scase->slicecoll.nsliceinfo;i++){
        slicedata *slicei;

        slicei = scase->slicecoll.sliceinfo + i;
        if(strcmp(file_ptr,slicei->file)==0){
          slicei->diff_valmin=percentile_min;
          slicei->diff_valmax=percentile_max;
          break;
        }
      }
      continue;
    }
    /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OBST +++++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if(MatchSMV(buffer, "OBST")==1&&scase->auto_terrain==1&&scase->manual_terrain==0){
      meshdata *meshi;
      int n_blocks;
      int iblock;
      int nn;

      scase->nOBST++;
      iobst++;
      FGETS(buffer,255,stream);
      sscanf(buffer,"%i",&n_blocks);


      if(n_blocks<=0)n_blocks=0;
      if(n_blocks==0)continue;

      meshi=scase->meshescoll.meshinfo+iobst-1;

      for(nn=0;nn<n_blocks;nn++){
        FGETS(buffer,255,stream);
      }
      nn=-1;
      for(iblock=0;iblock<n_blocks;iblock++){
        int ijk2[5],kmax;

        if(meshi->is_block_terrain!=NULL&&meshi->is_block_terrain[iblock]==0){
          FGETS(buffer,255,stream);
          continue;
        }
        nn++;

        FGETS(buffer,255,stream);
        sscanf(buffer,"%i %i %i %i %i %i",ijk2,ijk2+1,ijk2+2,ijk2+3,ijk2+4,&kmax);
      }
      FREEMEMORY(meshi->is_block_terrain);
      continue;
    }
  }
  PRINT_TIMER(timer_readsmv, "pass 5");
  PrintMemoryInfo;
  if(do_pass5==1){
    STOP_TIMER(scase->pass5_time);
  }
  else{
    scase->pass5_time = 0.0;
  }
  scase->clip_I=ibartemp; scase->clip_J=jbartemp; scase->clip_K=kbartemp;
  return 0;
}

/* ------------------ ReadSMV_Configure ------------------------ */

/// @brief Finish setting global variables after an SMV file has been parsed.
/// This should be called after @ref ReadSMV_Parse.
/// @return zero on success, nonzero on failure.
int ReadSMV_Configure(){
  int i;
  float wrapup_time;
  float timer_readsmv;

/*
   ************************************************************************
   ************************ wrap up ***************************************
   ************************************************************************
 */

  INIT_PRINT_TIMER(total_wrapup_time);
  update_colorbar_orig = 1;
  if(update_filesizes==1){
    GetFileSizes();
    SMV_EXIT(0);
  }

  START_TIMER(wrapup_time);
  STOP_TIMER(global_scase.processing_time);
  START_TIMER(timer_readsmv);

  PRINTF("  wrapping up\n");

  GetSkyBoxTextures();
  GetSkyImageTexture();
  InitTextures(use_graphics);

  FREEMEMORY(camera_external);
  NewMemory((void **)&camera_external,sizeof(cameradata));

  FREEMEMORY(camera_defaults);
  NewMemory((void **)&camera_defaults, 6*sizeof(cameradata *));
  NewMemory((void **)&(camera_defaults[0]), sizeof(cameradata));
  NewMemory((void **)&(camera_defaults[1]), sizeof(cameradata));
  NewMemory((void **)&(camera_defaults[2]), sizeof(cameradata));
  NewMemory((void **)&(camera_defaults[3]), sizeof(cameradata));
  NewMemory((void **)&(camera_defaults[4]), sizeof(cameradata));
  NewMemory((void **)&(camera_defaults[5]), sizeof(cameradata));

  FREEMEMORY(camera_external_save);
  NewMemory((void **)&camera_external_save,sizeof(cameradata));

  FREEMEMORY(camera_current);
  NewMemory((void **)&camera_current,sizeof(cameradata));

  FREEMEMORY(camera_save);
  NewMemory((void **)&camera_save,sizeof(cameradata));

  FREEMEMORY(camera_last);
  NewMemory((void **)&camera_last,sizeof(cameradata));

  INIT_PRINT_TIMER(fdsrunning_timer);
  last_size_for_slice = GetFileSizeSMV(global_scase.paths.stepcsv_filename); // used by IsFDSRunning
  last_size_for_boundary = last_size_for_slice;
  PRINT_TIMER(fdsrunning_timer, "filesize_timer");   // if file size changes then assume fds is running

  have_obsts = 0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    if(meshi->nbptrs>0){
      have_obsts = 1;
      break;
    }
  }
  if(global_scase.ntotal_blockages > 250000)hide_scene = 1;

  if(checkfiles_threads != NULL){
    checkfiles_threads = THREADinit(&n_checkfiles_threads, &use_checkfiles_threads, CheckFiles);
  }
  THREADrun(checkfiles_threads);
  PRINT_TIMER(timer_readsmv, "CheckFiles");
  CheckMemory;
  UpdateIsoColors();
  PRINT_TIMER(timer_readsmv, "UpdateIsoColors");
  CheckMemory;

  UpdateSmoke3dFileParms();
  PRINT_TIMER(timer_readsmv, "UpdateSmoke3dFileParms");

  AddCfastCsvf(&global_scase);
  PRINT_TIMER(timer_readsmv, "AddCfastCsvf");

  //RemoveDupBlockages();
#ifdef pp_CULL_GEOM
  InitCullGeom(cullgeom);
  PRINT_TIMER(timer_readsmv, "InitCullGeom");
#endif
  UpdateINIList();
  PRINT_TIMER(timer_readsmv, "UpdateINIList");

  if(global_scase.meshescoll.meshinfo!=NULL&&global_scase.meshescoll.meshinfo->jbar==1)force_isometric=1;

  if(global_scase.paths.hrr_csv_filename != NULL)ReadHRR(&global_scase, LOAD);
  PRINT_TIMER(timer_readsmv, "ReadHRR");

  if(runscript == 1){
    InitializeDeviceCsvData(LOAD);
  }
  PRINT_TIMER(timer_readsmv, "InitializeDeviceCsvData");

  SetupPlot2DUnitData();
  PRINT_TIMER(timer_readsmv, "SetupPlot2DUnitData");

  if(global_scase.nzoneinfo>0)SetupZoneDevs();
  PRINT_TIMER(timer_readsmv, "SetupPlot2DUnitData");

  InitPartProp();
  PRINT_TIMER(timer_readsmv, "InitPartProp");

  InitClip();
  PRINT_TIMER(timer_readsmv, "InitClip");

  if(global_scase.noutlineinfo>0){
    highlight_flag=2;
  }
  else{
    highlight_flag=1;
  }
  InitCadColors();
  PRINT_TIMER(timer_readsmv, "InitCadColors");

  // update loaded lists

  FREEMEMORY(slice_loaded_list);
  if(global_scase.slicecoll.nsliceinfo>0){
    NewMemory((void **)&slice_loaded_list,global_scase.slicecoll.nsliceinfo*sizeof(int));
  }
  FREEMEMORY(slice_sorted_loaded_list);
  if(global_scase.slicecoll.nsliceinfo>0){
    NewMemory((void **)&slice_sorted_loaded_list, global_scase.slicecoll.nsliceinfo*sizeof(int));
  }

  UpdateLoadedLists();
  PRINT_TIMER(timer_readsmv, "UpdateLoadedLists");
  CheckMemory;

  UpdateMeshBoxBounds();
  PRINT_TIMER(timer_readsmv, "UpdateMeshBoxBounds");

  SetupReadAllGeom();
  if(readallgeom_threads == NULL){
    readallgeom_threads = THREADinit(&n_readallgeom_threads, &use_readallgeom_threads, ReadAllGeom);
  }
  THREADrun(readallgeom_threads);
  THREADcontrol(readallgeom_threads, THREAD_JOIN);
  PRINT_TIMER(timer_readsmv, "ReadAllGeomMT");

  UpdateMeshCoords();
  PRINT_TIMER(timer_readsmv, "UpdateMeshCoords");

  UpdateSmoke3DTypes();
  PRINT_TIMER(timer_readsmv, "UpdateSmoke3DTypes");
  CheckMemory;

  // allocate memory for geometry pointers (only once)

  GetGeomInfoPtrs(1);
  PRINT_TIMER(timer_readsmv, "GetGeomInfoPtrs");
  /*
    Associate a surface with each block.
  */
  UpdateUseTextures();
  PRINT_TIMER(timer_readsmv, "UpdateUseTextures");
  CheckMemory;

  /* compute global bar's and box's */


  for(i=0;i<global_scase.npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = global_scase.partclassinfo + i;

    if(partclassi->device_name!=NULL){
        float length, azimuth, elevation;

        partclassi->diameter=SCALE2SMV(partclassi->diameter);
        partclassi->length=SCALE2SMV(partclassi->length);
        length=partclassi->length;
        azimuth = partclassi->azimuth*DEG2RAD;
        elevation = partclassi->elevation*DEG2RAD;
        partclassi->dx = cos(azimuth)*cos(elevation)*length/2.0;
        partclassi->dy = sin(azimuth)*cos(elevation)*length/2.0;
        partclassi->dz =              sin(elevation)*length/2.0;
    }
  }
  if(global_scase.npartinfo>=64){
#ifndef pp_PARTFRAME
    use_partload_threads = 1;
#endif
    partfast = 1;
  }

  shooter_xyz[0]=global_scase.xbar/2.0;
  shooter_xyz[1] = 0.0;
  shooter_xyz[2] = global_scase.zbar/2.0;
  shooter_dxyz[0]=global_scase.xbar/4.0;
  shooter_dxyz[1]=0.0;
  shooter_dxyz[2]=0.0;
  shooter_nparts=100;
  shooter_velmag=1.0;
  shooter_veldir=0.0;
  shooter_fps=10;
  shooter_vel_type=1;

  UpdatePlotxyzAll();
  CheckMemory;

  START_TIMER(timer_readsmv);
  SetSliceParmInfo(&sliceparminfo);
  PRINT_TIMER(timer_readsmv, "SetSliceParmInfo");
  global_scase.slicecoll.nsliceinfo            = 0;
  global_scase.slicecoll.nmultisliceinfo       = 0;
  global_scase.slicecoll.nmultivsliceinfo      = 0;
  global_scase.slicecoll.nvsliceinfo           = 0;
  if(sliceparms_threads == NULL){
    sliceparms_threads = THREADinit(&n_sliceparms_threads, &use_sliceparms_threads, UpdateVSlices);
  }
  THREADruni(sliceparms_threads, (unsigned char *)&sliceparminfo, 0);
  THREADcontrol(sliceparms_threads, THREAD_JOIN);
  PRINT_TIMER(timer_readsmv, "UpdateVSlices");

  GetSliceParmInfo(&sliceparminfo);
  PRINT_TIMER(timer_readsmv, "GetSliceParmInfo");
  if(update_slice==1)return 3;

  GenerateSliceMenu(generate_info_from_commandline);
  PRINT_TIMER(timer_readsmv, "GenerateSliceMenu");

  if(generate_info_from_commandline==1){
    GenerateViewpointMenu();
    SMV_EXIT(0);
  }

  GetBoundaryParams();
  PRINT_TIMER(timer_readsmv, "GetBoundaryParams");

  GetGSliceParams();
  PRINT_TIMER(timer_readsmv, "GetGSliceParams");

  active_smokesensors=0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;
    char *label;

    devicei = global_scase.devicecoll.deviceinfo + i;
    devicei->device_mesh= GetMeshNoFail(devicei->xyz);
    label = devicei->object->label;
    if(strcmp(label,"smokesensor")==0){
      active_smokesensors=1;
    }
    if(devicei->plane_surface!=NULL){
      InitDevicePlane(devicei);
    }
  }

  START_TIMER(timer_readsmv);

  MakeIBlankCarve();
  PRINT_TIMER(timer_readsmv, "MakeIBlankCarve");

  if(ffmpeg_threads == NULL){
    ffmpeg_threads = THREADinit(&n_ffmpeg_threads, &use_ffmpeg_threads, SetupFF);
  }
  THREADrun(ffmpeg_threads);
  PRINT_TIMER(timer_readsmv, "SetupFFMT");

  if(sorttags_threads == NULL){
    sorttags_threads = THREADinit(&n_sorttags_threads, &use_sorttags_threads, SortAllPartTags);
  }

  if(isosurface_threads == NULL){
    isosurface_threads = THREADinit(&n_isosurface_threads, &use_isosurface_threads, SetupAllIsosurfaces);
  }
  THREADrun(isosurface_threads);
  THREADcontrol(isosurface_threads, THREAD_JOIN);
  PRINT_TIMER(timer_readsmv, "SetupAllIsosurfaces");

  MakeIBlankSmoke3D();
  PRINT_TIMER(timer_readsmv, "MakeIBlankSmoke3D");

  if(HaveCircularVents()==1|| global_scase.meshescoll.nmeshes < 100 || parse_opts.fast_startup == 0){
    MakeIBlank();
    PRINT_TIMER(timer_readsmv, "MakeIBlank");
  }

  SetCVentDirs();
  PRINT_TIMER(timer_readsmv, "SetCVentDirs");

  SetVentDirs();
  update_setvents = 1;
  PRINT_TIMER(timer_readsmv, "SetVentDirs");

  UpdateFaces();
  PRINT_TIMER(timer_readsmv, "UpdateFaces");

  xcenGLOBAL=global_scase.xbar/2.0;  ycenGLOBAL=global_scase.ybar/2.0; zcenGLOBAL=global_scase.zbar/2.0;
  xcenCUSTOM=global_scase.xbar/2.0;  ycenCUSTOM=global_scase.ybar/2.0; zcenCUSTOM=global_scase.zbar/2.0;

  glui_rotation_index = ROTATE_ABOUT_FDS_CENTER;

  UpdateBoundInfo();
  PRINT_TIMER(timer_readsmv, "UpdateBoundInfo");

  UpdateObjectUsed();
  PRINT_TIMER(timer_readsmv, "UpdateObjectUsed");

  // close .smv file

  UpdateSelectFaces();
  PRINT_TIMER(timer_readsmv, "UpdateSelectFaces");

  UpdateSliceBoundIndexes();
  PRINT_TIMER(timer_readsmv, "UpdateSliceBoundIndexes");

  UpdateSliceBoundLabels();
  PRINT_TIMER(timer_readsmv, "UpdateSliceBoundLabels");

  UpdateIsoTypes();
  PRINT_TIMER(timer_readsmv, "UpdateIsoTypes");

  UpdateBoundaryTypes();
  PRINT_TIMER(timer_readsmv, "UpdateBoundaryTypes");

  if(meshnabors_threads == NULL){
    meshnabors_threads = THREADinit(&n_meshnabors_threads, &use_meshnabors_threads, InitNabors);
  }
  THREADrun(meshnabors_threads);

  UpdateTerrain(1); // xxslow
  UpdateTerrainColors();
  PRINT_TIMER(timer_readsmv, "UpdateTerrain");

  UpdateSmoke3dMenuLabels();
  PRINT_TIMER(timer_readsmv, "UpdateSmoke3dMenuLabels");

  UpdateVSliceBoundIndexes();
  PRINT_TIMER(timer_readsmv, "UpdateVSliceBoundIndexes");

  UpdateBoundaryMenuLabels();
  PRINT_TIMER(timer_readsmv, "UpdateBoundaryMenuLabels");

  UpdateIsoMenuLabels();
  PRINT_TIMER(timer_readsmv, "UpdateIsoMenuLabels");

  UpdatePartMenuLabels();
  PRINT_TIMER(timer_readsmv, "UpdatePartMenuLabels");

  UpdateTourMenuLabels();
  PRINT_TIMER(timer_readsmv, "UpdateTourMenuLabels");

  SetupCircularTourNodes();
  PRINT_TIMER(timer_readsmv, "SetupCircularTourNodes");

  InitUserTicks();
  PRINT_TIMER(timer_readsmv, "InitUserTicks");


  // define changed_idlist used for blockage editing

  {
    int ntotal=0;

    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      ntotal += meshi->nbptrs;
    }
    FREEMEMORY(changed_idlist);

    NewMemory((void **)&changed_idlist,sizeof(int)*(ntotal+1));

    for(i=0;i<ntotal;i++){
      changed_idlist[i]=0;
    }
    nchanged_idlist=ntotal;
  }

  START_TIMER(timer_readsmv);
  InitVolRender();
  InitVolRenderSurface(FIRSTCALL);
  radius_windrose = 0.2*xyzmaxdiff;
  PRINT_TIMER(timer_readsmv, "InitVolRender");

  if(large_case==0){
    SetupReadAllGeom();

    if(classifyallgeom_threads == NULL){
      classifyallgeom_threads = THREADinit(&n_readallgeom_threads, &use_readallgeom_threads, ClassifyAllGeom);
    }
    THREADrun(classifyallgeom_threads);
  }
  PRINT_TIMER(timer_readsmv, "ClassifyGeom");

  UpdateTriangles(GEOM_STATIC,GEOM_UPDATE_ALL);
  GetFaceInfo();
  GetBoxGeomCorners();
  GetBoxSkyCorners();
  PRINT_TIMER(timer_readsmv, "update trianglesfaces");

  if(global_scase.ngeominfo>0&&global_scase.auto_terrain==1){
    START_TIMER(timer_readsmv);
    GenerateTerrainGeom(&terrain_vertices, &terrain_indices, &terrain_nindices);
    PRINT_TIMER(timer_readsmv, "GenerateTerrainGeom");
  }

  // update event labels
  UpdateEvents();
  PRINT_TIMER(timer_readsmv, "UpdateEvents");

  InitCellMeshInfo();
  PRINT_TIMER(timer_readsmv, "InitCellMeshInfo");

  SetupMeshWalls();
  PRINT_TIMER(timer_readsmv, "SetupMeshWalls");

  if(viswindrose==1)update_windrose = 1;

// initialize 2d plot data structures
  NewMemory((void **)&glui_plot2dinfo, sizeof(plot2ddata));
  InitPlot2D(glui_plot2dinfo, 0);
  PRINT_TIMER(timer_readsmv, "InitPlot2D");

  SetInteriorBlockages();
  PRINT_TIMER(timer_readsmv, "SetInteriorBlockages");

  InitMeshBlockages();
  SetExternalVents();

  if(global_scase.meshescoll.nmeshes > 200){
    hide_scene = 1;
  }

  PRINTF("%s", _("complete"));
  PRINTF("\n\n");
  PrintMemoryInfo;

  STOP_TIMER(wrapup_time);
  if(show_timings==1){
    PRINTF(".smv Processing Times\n");
    PRINTF("---------------------\n");
    PRINTF("      filelist: %.1f s\n", global_scase.getfilelist_time);
    PRINTF("         setup: %.1f s\n", global_scase.pass0_time);
    PRINTF("        pass 1: %.1f s\n", global_scase.pass1_time);
    PRINTF("        pass 2: %.1f s\n", global_scase.pass2_time);
    PRINTF("        pass 3: %.1f s\n", global_scase.pass3_time);
    PRINTF("        pass 4: %.1f s\n", global_scase.pass4_time);
    PRINTF("        pass 5: %.1f s\n", global_scase.pass5_time);
    PRINTF("all passes: %.1f s\n", global_scase.processing_time);
    PRINTF("   wrap up: %.1f s\n", wrapup_time);
    PRINTF("\n");
  }
  STOP_TIMER(timer_startup);
  START_TIMER(timer_render);
  PRINT_TIMER(total_wrapup_time, "total wrapup time");
  return 0;
}

/* ------------------ ReadSMV ------------------------ */

/// @brief Parse an SMV file.
/// @param stream the file stream to parse.
/// @return zero on sucess, non-zero on error
int ReadSMV(bufferstreamdata *stream){
  ReadSMV_Init(&global_scase);
  ReadSMV_Parse(&global_scase, stream);
  ReadSMV_Configure();
  return 0;
}

/* ------------------ UpdateUseTextures ------------------------ */

void UpdateUseTextures(void){
  int i;

  for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
    texturedata *texti;

    texti=global_scase.texture_coll.textureinfo + i;
    texti->used=0;
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi=global_scase.meshescoll.meshinfo + i;
    if(global_scase.texture_coll.textureinfo!=NULL){
      for(j=0;j<meshi->nbptrs;j++){
        int k;
        blockagedata *bc;

        bc=meshi->blockageinfoptrs[j];
        for(k=0;k<6;k++){
          texturedata *texti;

          texti = bc->surf[k]->textureinfo;
          if(texti!=NULL&&texti->loaded==1){
            if(global_scase.usetextures==1)texti->display=1;
            texti->used=1;
          }
        }
      }
    }
    for(j=0;j<meshi->nvents+12;j++){
      ventdata *vi;

      vi = meshi->ventinfo + j;
      if(vi->surf[0]==NULL){
        if(global_scase.vent_surfacedefault!=NULL){
          if(j>=meshi->nvents+6){
            vi->surf[0]=global_scase.exterior_surfacedefault;
          }
          else{
            vi->surf[0]=global_scase.vent_surfacedefault;
          }
        }
      }
      if(global_scase.texture_coll.textureinfo!=NULL){
        if(vi->surf[0]!=NULL){
          texturedata *texti;

          texti = vi->surf[0]->textureinfo;
          if(texti!=NULL&&texti->loaded==1){
            if(global_scase.usetextures==1)texti->display=1;
            texti->used=1;
          }
        }
      }
    }
  }
  for(i=0;i<global_scase.device_texture_list_coll.ndevice_texture_list;i++){
    int texture_index;
    texturedata *texti;

    texture_index  = global_scase.device_texture_list_coll.device_texture_list_index[i];
    texti=global_scase.texture_coll.textureinfo + texture_index;
    if(texti!=NULL&&texti->loaded==1){
      if(global_scase.usetextures==1)texti->display=1;
      texti->used=1;
    }
  }
  for(i=0;i<global_scase.ngeominfo;i++){
    geomdata *geomi;

    geomi = global_scase.geominfo + i;
    if(global_scase.texture_coll.textureinfo!=NULL&&geomi->surfgeom!=NULL){
        texturedata *texti;

      texti = geomi->surfgeom->textureinfo;
      if(texti!=NULL&&texti->loaded==1){
        if(global_scase.usetextures==1)texti->display=1;
        texti->used=1;
      }
    }
  }
  if(global_scase.terrain_texture_coll.nterrain_textures>0 && global_scase.texture_coll.textureinfo != NULL){
    for(i=0;i<global_scase.terrain_texture_coll.nterrain_textures;i++){
      texturedata *texti;

      texti =global_scase.texture_coll.textureinfo + global_scase.texture_coll.ntextureinfo - global_scase.terrain_texture_coll.nterrain_textures + i;
      if(texti == global_scase.terrain_texture_coll.terrain_textures + i){
        texti->used = 1;
      }
    }
  }
  ntextures_loaded_used=0;
  for(i=0;i<global_scase.texture_coll.ntextureinfo;i++){
    texturedata *texti;

    texti = global_scase.texture_coll.textureinfo + i;
    if(texti->loaded==0)continue;
    if(texti->used==0)continue;
    ntextures_loaded_used++;
  }
}

/* ------------------ GetNewBoundIndex ------------------------ */

int GetNewBoundIndex(int old_index){
#define OLD_PERCENTILE 0
#define OLD_SET        1
#define OLD_GLOBAL     2

#define NEW_SET        0
#define NEW_GLOBAL     1
#define NEW_PERCENTILE 3

  int bound_map[] = {NEW_GLOBAL, NEW_SET, NEW_GLOBAL};

  assert(old_index>=0&&old_index<=2);
  old_index=CLAMP(old_index,0, 2);
  return bound_map[old_index];
}

/* ------------------ SetHVACDuctBounds ------------------------ */

void SetHVACDuctBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *quantity){
  int i;

  for(i = 0; i < nhvacductbounds; i++){
    boundsdata *boundi;

    boundi = hvacductbounds + i;
    if(strcmp(quantity, "") == 0 || strcmp(quantity, boundi->shortlabel) == 0){
      hvacductbounds[i].dlg_setvalmin = glui_setpatchmin;
      hvacductbounds[i].dlg_setvalmax = glui_setpatchmax;
      GLUISetMinMax(BOUND_HVACDUCT, boundi->shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
    }
  }
}

/* ------------------ SetHVACNodeBounds ------------------------ */

void SetHVACNodeBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *quantity){
  int i;

  for(i = 0; i < nhvacnodebounds; i++){
    boundsdata *boundi;

    boundi = hvacnodebounds + i;
    if(strcmp(quantity, "") == 0 || strcmp(quantity, boundi->shortlabel) == 0){
      hvacnodebounds[i].dlg_setvalmin = glui_setpatchmin;
      hvacnodebounds[i].dlg_setvalmax = glui_setpatchmax;
      GLUISetMinMax(BOUND_HVACNODE, boundi->shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
    }
  }
}

/* ------------------ SetBoundBounds ------------------------ */

void SetBoundBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *quantity){
  int i;

  GLUI2GlobalBoundaryBounds(quantity);
  for(i = 0; i<npatchbounds; i++){
    boundsdata *boundi;

    boundi = patchbounds+i;
    if(strcmp(quantity, "")==0||strcmp(quantity, boundi->shortlabel)==0){
      patchbounds[i].dlg_setvalmin = glui_setpatchmin;
      patchbounds[i].dlg_setvalmax = glui_setpatchmax;
      GLUISetMinMax(BOUND_PATCH, boundi->shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
    }
  }
}

/* ------------------ SetSliceBounds ------------------------ */

void SetSliceBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *buffer2){
  int i;

  for(i = 0; i<nslicebounds; i++){
    if(strcmp(buffer2, "")==0||strcmp(slicebounds[i].shortlabel, buffer2)==0){
      slicebounds[i].dlg_setvalmin = set_valmin;
      slicebounds[i].dlg_setvalmax = set_valmax;
      slicebounds[i].dlg_valmin = valmin;
      slicebounds[i].dlg_valmax = valmax;
      GLUISetMinMax(BOUND_SLICE, slicebounds[i].shortlabel, set_valmin, valmin, set_valmax, valmax);
      update_glui_bounds = 1;
      if(strcmp(slicebounds[i].shortlabel, buffer2)==0){
        break;
      }
    }
  }
}

/* ------------------ SetPatchMin ------------------------ */

void SetPatchMin(int set_valmin, float valmin, char *buffer2){
  int i;

  for(i = 0; i < npatchbounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(patchbounds[i].shortlabel, buffer2) == 0){
      patchbounds[i].dlg_setvalmin = set_valmin;
      patchbounds[i].dlg_valmin = valmin;
      GLUISetMin(BOUND_PATCH, patchbounds[i].shortlabel, set_valmin, valmin);
      update_glui_bounds = 1;
      if(strcmp(patchbounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}

/* ------------------ SetPatchMax ------------------------ */

void SetPatchMax(int set_valmax, float valmax, char *buffer2){
  int i;

  for(i = 0; i < npatchbounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(patchbounds[i].shortlabel, buffer2) == 0){
      patchbounds[i].dlg_setvalmax = set_valmax;
      patchbounds[i].dlg_valmax = valmax;
      GLUISetMax(BOUND_PATCH, patchbounds[i].shortlabel, set_valmax, valmax);
      update_glui_bounds = 1;
      if(strcmp(patchbounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}
/* ------------------ SetSliceMin ------------------------ */

void SetSliceMin(int set_valmin, float valmin, char *buffer2){
  int i;

  for(i = 0; i < nslicebounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(slicebounds[i].shortlabel, buffer2) == 0){
      slicebounds[i].dlg_setvalmin = set_valmin;
      slicebounds[i].dlg_valmin    = valmin;
      GLUISetMin(BOUND_SLICE, slicebounds[i].shortlabel, set_valmin, valmin);
      update_glui_bounds = 1;
      if(strcmp(slicebounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}

/* ------------------ SetSliceMax ------------------------ */

void SetSliceMax(int set_valmax, float valmax, char *buffer2){
  int i;

  for(i = 0; i < nslicebounds; i++){
    if(strcmp(buffer2, "") == 0 || strcmp(slicebounds[i].shortlabel, buffer2) == 0){
      slicebounds[i].dlg_setvalmax = set_valmax;
      slicebounds[i].dlg_valmax    = valmax;
      GLUISetMax(BOUND_SLICE, slicebounds[i].shortlabel, set_valmax, valmax);
      update_glui_bounds = 1;
      if(strcmp(slicebounds[i].shortlabel, buffer2) == 0)break;
    }
  }
}

/* ------------------ ReadIni2 ------------------------ */

int ReadIni2(const char *inifile, int localfile){
  int i;
  FILE *stream;
  int have_tours=0, have_tour7=0;

  updatemenu = 1;
  updatefacelists = 1;

  if((stream = fopen(inifile, "r")) == NULL)return 1;
  if(readini_output==1){
    if(verbose_output==1)PRINTF("reading %s ", inifile);
  }

  for(i = 0; i<nunitclasses_ini; i++){
    f_units *uc;

    uc = unitclasses_ini + i;
    FREEMEMORY(uc->units);
  }
  FREEMEMORY(unitclasses_ini);
  nunitclasses_ini = 0;

  if(localfile == 1){
    UpdateINIList();
  }

  if(localfile == 1){
    update_selectedtour_index = 0;
  }

  /* find number of each kind of file */

  while(!feof(stream)){
    char buffer[255], buffer2[255];

    CheckMemory;
    if(fgets(buffer, 255, stream) == NULL)break;

    if(MatchINI(buffer, "RESEARCHMODE") == 1){
      int dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %f %i %i %i %i %i %i %i", &research_mode, &dummy, &colorbar_shift, &ncolorlabel_digits, &force_fixedpoint, &ngridloc_digits, &sliceval_ndigits, &force_exponential, &force_decimal, &force_zero_pad);
      colorbar_shift = CLAMP(colorbar_shift, COLORBAR_SHIFT_MIN, COLORBAR_SHIFT_MAX);
      if(research_mode==1&&research_mode_override==0)research_mode=0;
      ncolorlabel_digits = CLAMP(ncolorlabel_digits, COLORBAR_NDECIMALS_MIN, COLORBAR_NDECIMALS_MAX);
      sliceval_ndigits   = CLAMP(sliceval_ndigits, 0, 10);
      ngridloc_digits    = CLAMP(ngridloc_digits, GRIDLOC_NDECIMALS_MIN, GRIDLOC_NDECIMALS_MAX);
      ONEORZERO(research_mode);
      ONEORZERO(force_fixedpoint);
      ONEORZERO(force_exponential);
      ONEORZERO(force_decimal);
      ONEORZERO(force_zero_pad);
      if(force_fixedpoint == 1 && force_exponential == 1)force_exponential = 0;
      update_research_mode=1;
      continue;
    }
#ifdef pp_FRAME
    if(MatchINI(buffer, "FRAMETHREADS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &nframe_threads);
    }
#endif
    if(MatchINI(buffer, "LOADMESH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i", &show_intersection_box, &show_intersected_meshes);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %f %i %f", use_meshclip + 0, meshclip + 0, use_meshclip + 1, meshclip + 1);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %f %i %f", use_meshclip + 2, meshclip + 2, use_meshclip + 3, meshclip + 3);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %f %i %f", use_meshclip + 4, meshclip + 4, use_meshclip + 5, meshclip + 5);

      for(i = 0;i < 6;i++){
        if(use_meshclip[i] != 0)use_meshclip[i] = 1;
      }
      if(show_intersection_box != 0)show_intersection_box = 1;
      if(show_intersected_meshes != 0)show_intersected_meshes = 1;
      update_meshclip = 1;
    }
    if(MatchINI(buffer, "GEOMDOMAIN") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i ", &showgeom_inside_domain, &showgeom_outside_domain);
      showgeom_inside_domain = CLAMP(showgeom_inside_domain, 0, 1);
      showgeom_outside_domain = CLAMP(showgeom_outside_domain, 0, 1);
      continue;
    }
    if(MatchINI(buffer, "SLICEDUP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i", &slicedup_option, &vectorslicedup_option,&boundaryslicedup_option);
      continue;
    }
    if(MatchINI(buffer, "SHOWBOUNDS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i", &bounds_each_mesh, &show_bound_diffs);
      continue;
    }
    if(MatchINI(buffer, "BLENDMODE")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i,%i", &slices3d_max_blending, &hrrpuv_max_blending,&showall_3dslices);
    }
    if(MatchINI(buffer, "FREEZEVOLSMOKE")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i", &freeze_volsmoke,&autofreeze_volsmoke);
      continue;
    }
    if(MatchINI(buffer, "VISBOUNDARYTYPE")==1){
      int *vbt = vis_boundary_type;

      update_ini_boundary_type = 1;
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %i %i %i", vbt,vbt+1,vbt+2,vbt+3,vbt+4,vbt+5,vbt+6, &show_mirror_boundary, &show_mirror_boundary);
      show_all_interior_patch_data = vbt[INTERIORwall];
      hide_all_interior_patch_data = 1 - show_all_interior_patch_data;
      continue;
    }
    if(MatchINI(buffer, "GEOMBOUNDARYPROPS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %f %f %i", &show_boundary_shaded, &show_boundary_outline, &show_boundary_points, &geomboundary_linewidth, &geomboundary_pointsize, &boundary_edgetype);
      ONEORZERO(show_boundary_shaded);
      ONEORZERO(show_boundary_outline);
      ONEORZERO(show_boundary_points);
      ONEORZERO(boundary_edgetype);
      continue;
    }
    if(MatchINI(buffer, "HVACVIEW") == 1&&global_scase.hvaccoll.hvacinfo!=NULL&&global_scase.hvaccoll.nhvacinfo > 0){
      int nh, dummy;
      float rdummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %f %i",
        &nh, &hvac_metro_view, &dummy, &dummy, &rdummy, &hvac_cell_view);
      ONEORZERO(hvac_metro_view);
      ONEORZERO(hvac_cell_view);

      nh = MIN(global_scase.hvaccoll.nhvacinfo, nh);
      for(i = 0; i < nh; i++){
        hvacdata *hvaci;
        int dc[3], nc[3];
        int j;

        hvaci = global_scase.hvaccoll.hvacinfo + i;
        fgets(buffer, 255, stream);
        sscanf(buffer, " %i %i %i %i %i %f %f %f %f %f",
          &hvaci->display,  &hvaci->show_node_labels, &hvaci->show_duct_labels,
          &hvaci->show_component, &hvaci->show_filters, &hvaci->duct_width,
          &hvaci->node_size, &hvaci->component_size, &hvaci->filter_size, &hvaci->cell_node_size);
        fgets(buffer, 255, stream);
        sscanf(buffer, " %i %i %i %i %i %i", dc, dc + 1, dc + 2, nc, nc + 1, nc + 2);
        for(j=0;j<3;j++){
          hvaci->duct_color[j] = CLAMP(dc[j], 0, 255);
          hvaci->node_color[j] = CLAMP(nc[j], 0, 255);
        }
        hvaci->duct_width     = MAX(1.0, hvaci->duct_width);
        hvaci->node_size     = MAX(1.0, hvaci->node_size);
        hvaci->cell_node_size = MAX(1.0, hvaci->cell_node_size);
        hvaci->component_size = MAX(0.1, hvaci->component_size);
        hvaci->filter_size    = MAX(0.1, hvaci->filter_size);
        ONEORZERO(hvaci->show_node_labels);
        ONEORZERO(hvaci->show_duct_labels);
      }
    }
    if(MatchINI(buffer, "SHOWSLICEVALS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i", show_slice_values, show_slice_values+1, show_slice_values+2);
    }
    if(MatchINI(buffer, "GEOMCELLPROPS")==1){
      int vector_slice[3] = {-1, -1, -1};
      float dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i",
        &slice_celltype);
      slice_celltype = CLAMP(slice_celltype,0,MAX_CELL_TYPES-1);

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
        slice_edgetypes,slice_edgetypes+1,slice_edgetypes+2);

      for(i=0;i<3;i++){
        slice_edgetypes[i] = CLAMP(slice_edgetypes[i],0,2);
      }

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %f %i %i %i",
        show_slice_shaded,show_slice_shaded+1,show_slice_shaded+2,&dummy, vector_slice, vector_slice+1, vector_slice+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
        show_slice_outlines,show_slice_outlines+1,show_slice_outlines+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
        show_slice_points,show_slice_points+1,show_slice_points+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i",
             show_vector_slice, show_vector_slice+1, show_vector_slice+2);

      for(i=0;i<MAX_CELL_TYPES;i++){
        show_slice_shaded[i]   = CLAMP(show_slice_shaded[i],0,1);
        show_slice_outlines[i] = CLAMP(show_slice_outlines[i],0,1);
        show_slice_points[i]   = CLAMP(show_slice_points[i],0,1);
        if(vector_slice[i]>=0)show_vector_slice[i] = CLAMP(vector_slice[i], 0, 1);
      }
      continue;
    }
    if(MatchINI(buffer, "NORTHANGLE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &vis_northangle);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f", northangle_position, northangle_position + 1, northangle_position + 2);
      continue;
    }
    if(MatchINI(buffer, "SHOWAVATAR")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &show_avatar);
      ONEORZERO(show_avatar);
      continue;
    }
    if(MatchINI(buffer, "TREEPARMS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &mintreesize, &vis_xtree, &vis_ytree, &vis_ztree);
      mintreesize = MAX(mintreesize, 2);
      vis_xtree = CLAMP(vis_xtree, 0, 1);
      vis_ytree = CLAMP(vis_ytree, 0, 1);
      vis_ztree = CLAMP(vis_ztree, 0, 1);
      continue;
    }
    if(MatchINI(buffer, "COLORBAR_SPLIT") == 1){
      int ii;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i ", colorsplit, colorsplit + 1, colorsplit + 2, colorsplit + 3, colorsplit + 4, colorsplit + 5);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i ", colorsplit + 6, colorsplit + 7, colorsplit + 8, colorsplit + 9, colorsplit + 10, colorsplit + 11);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f ", splitvals, splitvals + 1, splitvals + 2);

      for(ii = 0; ii < 12; ii++){
        colorsplit[ii] = CLAMP(colorsplit[ii], 0, 255);
      }
      if(scriptinfo == NULL){
        update_splitcolorbar = 1;
      }
      else{
        GLUISplitCB(SPLIT_COLORBAR);
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWGRAVVECTOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &showgravity_vector);
      continue;
    }
    if(MatchINI(buffer, "ZAXISANGLES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f", zaxis_angles, zaxis_angles + 1, zaxis_angles + 2);
      zaxis_angles_orig[0] = zaxis_angles[0];
      zaxis_angles_orig[1] = zaxis_angles[1];
      zaxis_angles_orig[2] = zaxis_angles[2];
      zaxis_custom = 1;
      update_zaxis_custom = 1;
      continue;
    }
    if(MatchINI(buffer, "GEOMSHOW") == 1){
      int dummy, dummy2;
      float rdummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %f %f %i %i %f %f %f",
        &dummy, &dummy2, &show_faces_shaded, &show_faces_outline, &smooth_geom_normal,
        &geom_force_transparent, &geom_transparency,&geom_linewidth, &use_geom_factors, &show_cface_normals, &geom_pointsize, &geom_dz_offset, &geom_norm_offset);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i", &dummy, &dummy, &dummy, &dummy);
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %i %i %i %i", &geom_vert_exag, &rdummy, &dummy, &dummy2, &hide_scene, &show_geom_bndf );
      if(hide_scene !=1)hide_scene = 0;
      continue;
    }
    if(MatchINI(buffer, "SHOWTRIANGLECOUNT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &show_triangle_count);
      continue;
    }
    if(MatchINI(buffer, "SHOWDEVICEVALS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %i %i %i",
        &showdevice_val, &showvdevice_val, &devicetypes_index, &colordevice_val, &vectortype, &viswindrose, &showdevice_type, &showdevice_unit,&showdevice_id);
      devicetypes_index = CLAMP(devicetypes_index, 0, MAX(ndevicetypes - 1,0));
      update_glui_devices = 1;
      if(viswindrose==1)update_windrose = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWSLICEPLOT")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %f %f %f %f %i %i %f %f %f %i %i",
         slice_xyz, slice_xyz+1, slice_xyz+2,
         &plot2d_size_factor, &vis_slice_plot, &slice_plot_bound_option,
         slice_dxyz, slice_dxyz+1, slice_dxyz+2, &average_plot2d_slice_region, &show_plot2d_slice_position
      );
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRPLOT")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %f %f %i", &glui_hrr, &hoc_hrr, &global_scase.fuel_hoc, &plot2d_size_factor, &vis_hrr_plot);
      continue;
    }
    if(MatchINI(buffer, "SHOWDEVICEPLOTS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %f %f %f %f %f %f %f",
             &vis_device_plot, &show_plot2d_xlabels, &show_plot2d_ylabels, &plot2d_size_factor, &plot2d_line_width, &plot2d_point_size,
             plot2d_xyz_offset, plot2d_xyz_offset+1, plot2d_xyz_offset+2, &plot2d_font_spacing
      );
      update_glui_devices = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWGENPLOTXLABEL") == 1){
      char *xlabelptr;

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i, %f", &plot2d_show_xaxis_labels, &plot2d_xaxis_position);
      fgets(buffer, 255, stream);
      xlabelptr = TrimFrontBack(buffer);
      strcpy(plot2d_xaxis_label, xlabelptr);
    }
    if(MatchINI(buffer, "SHOWGENPLOTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &nplot2dini);

      FREEMEMORY(plot2dini);

      if(nplot2dini==0)continue;
      NewMemory((void **)&plot2dini, nplot2dini*sizeof(plot2ddata));

      fgets(buffer, 255, stream);
      sscanf(buffer, " %i %i %i %i %i %i %f %i %i",
             &plot2d_show_plot_title, &plot2d_show_curve_labels, &plot2d_show_curve_values,
             &plot2d_show_xaxis_bounds, &plot2d_show_yaxis_bounds, &idevice_add, &plot2d_time_average,
             &plot2d_show_yaxis_units, &plot2d_show_plots
             );
      update_device_timeaverage = 1;
      GLUIUpdateDeviceAdd();
      for(i=0;i<nplot2dini;i++){
        plot2ddata *plot2di;
        char *labelptr;
        int j;

        plot2di = plot2dini + i;
        plot2di->plot_index = i;
        plot2di->mult_devc  = 0;
        fgets(buffer, 255, stream);
        TrimBack(buffer);
        labelptr = TrimFront(buffer);
        strcpy(plot2di->plot_label, labelptr);

        fgets(buffer, 255, stream);
        sscanf(buffer, " %f %f %f %i %i %i", plot2di->xyz, plot2di->xyz+1, plot2di->xyz+2, &plot2di->show, &plot2di->ncurves, &plot2di->mult_devc);
        fgets(buffer, 255, stream);
        sscanf(buffer, " %f %i %f %i %f %i %f %i ",
               plot2di->valmin,   plot2di->use_valmin,   plot2di->valmax,   plot2di->use_valmax,
               plot2di->valmin+1, plot2di->use_valmin+1, plot2di->valmax+1, plot2di->use_valmax+1);
        plot2di->bounds_defined = 0;
        for(j=0; j<plot2di->ncurves; j++){
          int color[3];
          float linewidth1;
          int file_index, col_index;
          curvedata *curve;
          float factor;
          int apply_factor;
          int use_foreground_color;

          fgets(buffer, 255, stream);
          TrimBack(buffer);
          linewidth1 = 1.0;
          factor = 1.0;
          apply_factor = 0;
          use_foreground_color = 0;
          sscanf(buffer, " %i %i %i %i %i %f %f %i %i",    &file_index, &col_index, color, color+1, color+2, &linewidth1, &factor, &apply_factor, &use_foreground_color);

          plot2di->curve[j].csv_file_index = file_index;
          plot2di->curve[j].csv_col_index  = col_index;
          curve                            = plot2di->curve+j;
          strcpy(curve->c_type, global_scase.csvcoll.csvfileinfo[file_index].c_type);
          curve->color[0]                  = color[0];
          curve->color[1]                  = color[1];
          curve->color[2]                  = color[2];
          curve->linewidth                 = linewidth1;
          curve->curve_factor              = factor;
          curve->apply_curve_factor        = apply_factor;
          curve->vals                      = NULL;
          curve->use_foreground_color      = use_foreground_color;
          if(strcmp(curve->c_type, "devc")==0){
            curve->quantity = global_scase.csvcoll.csvfileinfo[file_index].csvinfo[col_index].label.longlabel;
          }
          else{
            curve->quantity = NULL;
          }
        }
      }
      update_glui_devices = 1;
      update_plot2dini = 1;
      continue;
    }
    if(MatchINI(buffer, "GENPLOTLABELS") == 1){
      fgets(buffer, 255, stream);
      for(i=0;i<nplot2dini;i++){
        plot2ddata *plot2di;
        int j;

        plot2di = plot2dini + i;
        fgets(buffer, 255, stream);
        for(j=0; j<plot2di->ncurves; j++){
          curvedata *curve;

          curve = plot2di->curve + j;
          fgets(buffer, 255, stream);
          fgets(buffer, 255, stream);
          strcpy(curve->scaled_label, TrimFrontBack(buffer));
          fgets(buffer, 255, stream);
          strcpy(curve->scaled_unit, TrimFrontBack(buffer));
        }
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWMISSINGOBJECTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, " %i", &show_missing_objects);
      ONEORZERO(show_missing_objects);
      continue;
    }
    if(MatchINI(buffer, "DEVICEVECTORDIMENSIONS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", &vector_baselength, &vector_basediameter, &vector_headlength, &vector_headdiameter);
      continue;
    }
    if(MatchINI(buffer, "DEVICEBOUNDS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &device_valmin, &device_valmax);
      continue;
    }
    if(MatchINI(buffer, "DEVICEORIENTATION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f", &show_device_orientation, &orientation_scale);
      show_device_orientation = CLAMP(show_device_orientation, 0, 1);
      orientation_scale = CLAMP(orientation_scale, 0.1, 10.0);
      continue;
    }
    if(MatchINI(buffer, "GVERSION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis_title_gversion);
      ONEORZERO(vis_title_gversion);
    }
    if(MatchINI(buffer, "GVECDOWN") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &gvec_down);
      ONEORZERO(gvec_down);
    }
    if(MatchINI(buffer, "SCALEDFONT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &scaled_font2d_height, &scaled_font2d_height2width, &scaled_font2d_thickness);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &scaled_font3d_height, &scaled_font3d_height2width, &scaled_font3d_thickness);
    }
    if(MatchINI(buffer, "NEWDRAWFACE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &blockage_draw_option);
      updatefacelists = 1;
      blockage_draw_option = CLAMP(blockage_draw_option, 0, 3);
      continue;
    }
    if(MatchINI(buffer, "TLOAD") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f %i %i", &use_tload_begin, &global_scase.tload_begin, &use_tload_end, &global_scase.tload_end, &use_tload_skip, &tload_skip);
      continue;
    }
    if(MatchINI(buffer, "VOLSMOKE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i",
        &glui_compress_volsmoke, &use_multi_threading, &load_at_rendertimes, &volbw, &show_volsmoke_moving);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f %f %f %f",
        &global_temp_min, &global_temp_cb_min, &global_temp_cb_max, &fire_opacity_factor, &mass_extinct, &gpu_vol_factor, &nongpu_vol_factor);
      global_temp_cb_min_default = global_temp_cb_min;
      ONEORZERO(glui_compress_volsmoke);
      ONEORZERO(use_multi_threading);
      ONEORZERO(load_at_rendertimes);
      fire_opacity_factor = CLAMP(fire_opacity_factor, 1.0, 10.0);
      mass_extinct = CLAMP(mass_extinct, 100.0, 100000.0);
      InitVolRenderSurface(NOT_FIRSTCALL);
      continue;
    }
    if(MatchINI(buffer, "WINDROSEMERGE")==1){
      float *xyzt;

      xyzt = windrose_merge_dxyzt;
      fgets(buffer, 255, stream);
      sscanf(buffer," %i %f %f %f %f",&windrose_merge_type,xyzt,xyzt+1,xyzt+2,xyzt+3);
      xyzt[0]=MAX(xyzt[0],0.0);
      xyzt[1]=MAX(xyzt[1],0.0);
      xyzt[2]=MAX(xyzt[2],0.0);
      xyzt[3]=MAX(xyzt[3],0.0);
    }
    if(MatchINI(buffer, "WINDROSEDEVICE")==1){
      float rad_windrose;

      fgets(buffer, 255, stream);
      sscanf(buffer," %i %i %i %i %i %i %i %i %i",
        &viswindrose, &showref_windrose, &windrose_xy_vis, &windrose_xz_vis, &windrose_yz_vis, &windstate_windrose, &showlabels_windrose,
        &windrose_first,&windrose_next);
      viswindrose         = CLAMP(viswindrose, 0, 1);
      showref_windrose    = CLAMP(showref_windrose, 0, 1);
      windrose_xy_vis      = CLAMP(windrose_xy_vis, 0, 1);
      windrose_xz_vis      = CLAMP(windrose_xz_vis, 0, 1);
      windrose_yz_vis      = CLAMP(windrose_yz_vis, 0, 1);
      windstate_windrose  = CLAMP(windstate_windrose, 0, 1);
      showlabels_windrose = CLAMP(showlabels_windrose, 0, 1);
      if(windrose_first < 0)windrose_first = 0;
      if(windrose_next < 1)windrose_next = 1;
      if(viswindrose==1)update_windrose = 1;

      fgets(buffer, 255, stream);
      sscanf(buffer," %i %i %i %f %i %i",    &nr_windrose, &ntheta_windrose, &scale_windrose, &rad_windrose, &scale_increment_windrose, &scale_max_windrose);
      nr_windrose              = ABS(nr_windrose);
      ntheta_windrose          = ABS(ntheta_windrose);
      if(localfile==1)radius_windrose          = ABS(rad_windrose);
      scale_windrose           = CLAMP(scale_windrose,0,1);
      scale_increment_windrose = CLAMP(scale_increment_windrose, 1, 50);
      scale_max_windrose       = CLAMP(scale_max_windrose, 0, 100);
      continue;
    }
    if(MatchINI(buffer, "BOUNDARYTWOSIDE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &showpatch_both);
      ONEORZERO(showpatch_both);
    }
    if(MatchINI(buffer, "MESHOFFSET") == 1){
      int meshnum;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &meshnum);
      if(meshnum >= 0 && meshnum<global_scase.meshescoll.nmeshes){
        meshdata *meshi;

        meshi = global_scase.meshescoll.meshinfo + meshnum;
        meshi->mesh_offset_ptr = meshi->mesh_offset;
      }
      continue;
    }
    if(MatchINI(buffer, "STARTUPLANG") == 1){
      char *bufptr;

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      bufptr = TrimFront(buffer);
      strncpy(startup_lang_code, bufptr, 2);
      startup_lang_code[2] = '\0';
      if(tr_name == NULL){
        int langlen;

        langlen = strlen(bufptr);
        NewMemory((void **)&tr_name, langlen + 48 + 1);
        strcpy(tr_name, bufptr);
      }
      continue;
    }
    if(MatchINI(buffer, "MESHVIS") == 1){
      int nm;
      meshdata *meshi;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nm);
      for(i = 0; i<nm; i++){
        if(i>global_scase.meshescoll.nmeshes - 1)break;
        meshi = global_scase.meshescoll.meshinfo + i;
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &meshi->blockvis);
        ONEORZERO(meshi->blockvis);
      }
      continue;
    }
    if(MatchINI(buffer, "SPHERESEGS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &device_sphere_segments);
      device_sphere_segments = CLAMP(device_sphere_segments, 6, 48);
      InitSphere(device_sphere_segments, 2 * device_sphere_segments);
      InitCircle(2 * device_sphere_segments, &object_circ);
      continue;
    }
    if(MatchINI(buffer, "DIRECTIONCOLOR") == 1){
      float *dc;

      dc = direction_color;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", dc, dc + 1, dc + 2);
      dc[3] = 1.0;
      direction_color_ptr = GetColorPtr(&global_scase, direction_color);
      GetSliceParmInfo(&sliceparminfo);
      UpdateSliceMenuShow(&sliceparminfo);
      continue;
    }

    if(MatchINI(buffer, "OFFSETSLICE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &offset_slice);
      ONEORZERO(offset_slice);
      continue;
    }
    if(MatchINI(buffer, "VECLENGTH") == 1){
      float vf = 1.0;
      int idummy;
      float dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %f %i %i %i", &idummy, &vf, &dummy, &vec_uniform_length, &vec_uniform_spacing, &color_vector_black);
      vecfactor = vf;
      continue;
    }
    if(MatchINI(buffer, "VECCONTOURS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &show_node_slices_and_vectors,&show_cell_slices_and_vectors);
      ONEORZERO(show_node_slices_and_vectors);
      ONEORZERO(show_cell_slices_and_vectors);
      continue;
    }
    if(MatchINI(buffer, "ISOTRAN2") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &transparent_state);
      continue;
    }
    if(MatchINI(buffer, "SHOWTRIANGLES") == 1){
      int dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %i %i", &show_iso_shaded, &show_iso_outline, &show_iso_points, &show_iso_normal, &dummy, &smooth_iso_normal, &sort_iso_triangles);
      ONEORZERO(show_iso_shaded);
      ONEORZERO(show_iso_outline);
      ONEORZERO(show_iso_points);
      ONEORZERO(show_iso_normal);
      ONEORZERO(smooth_iso_normal);
      ONEORZERO(show_iso_normal);
      ONEORZERO(sort_iso_triangles);
      sort_geometry = sort_iso_triangles;
      visAIso = show_iso_shaded * 1 + show_iso_outline * 2 + show_iso_points * 4;
      continue;
    }
    if(MatchINI(buffer, "SHOWSTREAK") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &streak5show, &streak5step, &showstreakhead, &streak_index);
      ONEORZERO(streak5show);
      if(streak5show == 0)streak_index = -2;
      ONEORZERO(showstreakhead);
      update_streaks = 1;
      continue;
    }

    if(MatchINI(buffer, "SHOWTERRAIN") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &global_scase.visTerrainType, &terrain_slice_overlap);
      continue;
    }
    if(MatchINI(buffer, "STEREO") == 1){
      fgets(buffer, 255, stream);
      stereotypeOLD = stereotype;
      sscanf(buffer, "%i", &stereotype);
      stereotype = CLAMP(stereotype, 0, 5);
      if(stereotype == STEREO_TIME&&videoSTEREO != 1)stereotype = STEREO_NONE;
      GLUIUpdateStereo();
      continue;
    }
    if(MatchINI(buffer, "TERRAINPARMS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", terrain_rgba_zmin, terrain_rgba_zmin + 1, terrain_rgba_zmin + 2);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", terrain_rgba_zmax, terrain_rgba_zmax + 1, terrain_rgba_zmax + 2);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &vertical_factor);

      for(i = 0; i<3; i++){
        terrain_rgba_zmin[i] = CLAMP(terrain_rgba_zmin[i], 0, 2255);
        terrain_rgba_zmax[i] = CLAMP(terrain_rgba_zmax[i], 0, 2255);
      }
      vertical_factor = CLAMP(vertical_factor, 0.25, 4.0);
      UpdateTerrain(0);
      UpdateTerrainColors();
      continue;
    }
    if(MatchINI(buffer, "SMOKESENSORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &show_smokesensors, &test_smokesensors);
      continue;
    }
#ifdef pp_GPU
    if(gpuactive == 1 && MatchINI(buffer, "USEGPU") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &usegpu);
      ONEORZERO(usegpu);
      continue;
    }
#endif
    if(MatchINI(buffer, "V2_PLOT3D") == 1||
       MatchINI(buffer, "V_PLOT3D")==1){
      int is_old_bound;

      is_old_bound=0;
      if(MatchINI(buffer, "V_PLOT3D")==1){
        is_old_bound = 1;
      }
      int tempval;
      int n3d;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &tempval);
      if(tempval<0)tempval = 0;
      n3d = tempval;
      if(n3d>MAXPLOT3DVARS)n3d = MAXPLOT3DVARS;
      for(i = 0; i<n3d; i++){
        int iplot3d, isetmin, isetmax;
        float p3mintemp, p3maxtemp;

        fgets(buffer, 255, stream);
        strcpy(buffer2, "");
        sscanf(buffer, "%i %i %f %i %f %s", &iplot3d, &isetmin, &p3mintemp, &isetmax, &p3maxtemp, buffer2);
        if(is_old_bound==1){
          isetmin = GetNewBoundIndex(isetmin);
          isetmax = GetNewBoundIndex(isetmax);
        }
        if(isetmin == BOUND_SET_MIN || isetmax == BOUND_SET_MAX){
          research_mode = 0;
          update_research_mode = 1;
        }
        iplot3d--;
        if(iplot3d >= 0 && iplot3d<MAXPLOT3DVARS){
          setp3min_all[iplot3d] = isetmin;
          setp3max_all[iplot3d] = isetmax;
          p3min_all[iplot3d]    = p3mintemp;
          p3max_all[iplot3d]    = p3maxtemp;
          if(global_scase.plot3dinfo!=NULL){
            GLUISetMinMax(BOUND_PLOT3D, global_scase.plot3dinfo[0].label[iplot3d].shortlabel, isetmin, p3mintemp, isetmax, p3maxtemp);
            update_glui_bounds = 1;
          }
        }
      }
      continue;
    }
    if(MatchINI(buffer, "UNLOAD_QDATA") == 1){
      int unload_qdata;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &unload_qdata);
      cache_plot3d_data = 1 - unload_qdata;
      ONEORZERO(cache_plot3d_data);
      update_cache_data = 1;
      continue;
    }
    if(MatchINI(buffer, "CACHE_DATA") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &cache_boundary_data, &cache_part_data, &cache_plot3d_data, &cache_slice_data);
      ONEORZERO(cache_boundary_data);
      ONEORZERO(cache_part_data);
      ONEORZERO(cache_plot3d_data);
      ONEORZERO(cache_slice_data);
      update_cache_data = 1;
      continue;
    }
    if(MatchINI(buffer, "TREECOLORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", trunccolor, trunccolor + 1, trunccolor + 2);
      sscanf(buffer, "%f %f %f", treecolor, treecolor + 1, treecolor + 2);
      sscanf(buffer, "%f %f %f", treecharcolor, treecharcolor + 1, treecharcolor + 2);
      for(i = 0; i<3; i++){
        treecolor[i] = CLAMP(treecolor[i], 0.0, 1.0);
        treecolor_uc[i] = 255 * treecolor[i];
        treecharcolor[i] = CLAMP(treecharcolor[i], 0.0, 1.0);
        treecharcolor_uc[i] = 255 * treecharcolor[i];
        trunccolor[i] = CLAMP(trunccolor[i], 0.0, 1.0);
        trunccolor_uc[i] = 255 * trunccolor[i];
      }
      treecolor[3] = 1.0;
      treecharcolor[3] = 1.0;
      trunccolor[3] = 1.0;
      treecolor_uc[3] = 255;
      treecharcolor_uc[3] = 255;
      trunccolor_uc[3] = 255;
      continue;
    }
    if(MatchINI(buffer, "TRAINERVIEW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &trainerview);
      if(trainerview != 2 && trainerview != 3)trainerview = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWTRANSPARENTVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_transparent_vents);
      ONEORZERO(show_transparent_vents);
      continue;
    }
    if(MatchINI(buffer, "COLORBARTYPE") == 1){
      char *label;

      fgets(buffer, 255, stream);
      label = strchr(buffer, '%');
      if(label != NULL){
        update_colorbartype = 1;
        label++;
        TrimBack(label);
        label = TrimFront(label);
        strcpy(colorbarname, label);
      }
      continue;
    }
    if(MatchINI(buffer, "COLORMAP") == 1){
      char *ctype, *cmaptype, *cmap;
      colorbardata *cb;

      fgets(buffer, 255, stream);
      ctype = strtok(buffer, " ");
      cmaptype = strtok(NULL, " ");
      cmap = strtok(NULL, " ");
      if(strcmp(ctype, "FIRE") == 0){
        sscanf(cmaptype, "%i", &fire_colormap_type);
        cb = GetColorbar(&colorbars, cmap);
        if(cb == NULL)continue;
        fire_colormap_type_save = fire_colormap_type;
        fire_colorbar_index_ini = cb - colorbars.colorbarinfo;
        update_fire_colorbar_index = 1;
      }
      else if(strcmp(ctype, "CO2") == 0){
        sscanf(cmaptype, "%i", &co2_colormap_type);
        cb = GetColorbar(&colorbars, cmap);
        if(cb == NULL)continue;
        co2_colorbar_index_ini = cb - colorbars.colorbarinfo;
        update_co2_colorbar_index = 1;
      }
      else{
        continue;
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWEXTREMEDATA") == 1){
      int below = -1, above = -1, dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &dummy, &below, &above);
      if(below != 1)below = 0;
      if(above != 1)above = 0;
      show_extreme_mindata = below;
      show_extreme_maxdata = above;
      continue;
    }
    if(MatchINI(buffer, "EXTREMECOLORS") == 1){
      int mmin[3], mmax[3];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %i",
        mmin, mmin + 1, mmin + 2,
        mmax, mmax + 1, mmax + 2);
      for(i = 0; i<3; i++){
        rgb_below_min[i] = CLAMP(mmin[i], 0, 255);
        rgb_above_max[i] = CLAMP(mmax[i], 0, 255);
      }
      continue;
    }
    if(MatchINI(buffer, "SLICEAVERAGE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i", &slice_average_flag, &slice_average_interval, &vis_slice_average);
      ONEORZERO(slice_average_flag);
      if(slice_average_interval<0.0)slice_average_interval = 0.0;
      continue;
    }
    if(MatchINI(buffer, "SKYBOX") == 1){
      FreeSkybox();
      nskyboxinfo = 1;
      NewMemory((void **)&skyboxinfo, nskyboxinfo*sizeof(skyboxdata));
      for(i = 0; i<6; i++){
        char *skybox_texture;

        fgets(buffer, 255, stream);
        skybox_texture = TrimFrontBack(buffer);
        LoadSkyTexture(skybox_texture, skyboxinfo->face + i);
      }
    }
    if(MatchINI(buffer, "C_PLOT3D")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;
      int tempval, j;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &tempval);
      tempval = MIN(tempval, nplot3dbounds_cpp);

      for(j=0;j<tempval;j++){
        int iplot3d;

        cpp_boundsdata *boundi;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %f %i %f", &iplot3d, &setvalmin, &valmin, &setvalmax, &valmax);
        iplot3d--;
        if(iplot3d>=0 && iplot3d<tempval){
          boundi = plot3dbounds_cpp+iplot3d;
          boundi->set_chopmin = setvalmin;
          boundi->chopmin     = valmin;
          boundi->set_chopmax = setvalmax;
          boundi->chopmax     = valmax;
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "DEVICENORMLENGTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &devicenorm_length);
      if(devicenorm_length<0.0 || devicenorm_length>1.0)devicenorm_length = 0.1;
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRLABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis_hrr_label);
      ONEORZERO(vis_hrr_label);
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRCUTOFF") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_firecutoff);
      ONEORZERO(show_firecutoff);
      continue;
    }
    if(MatchINI(buffer, "SHOWFIRECUTOFF") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_firecutoff);
      ONEORZERO(show_firecutoff);
      continue;
    }
    if(MatchINI(buffer, "TWOSIDEDVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &show_bothsides_int, &show_bothsides_ext);
      ONEORZERO(show_bothsides_int);
      ONEORZERO(show_bothsides_ext);
      continue;
    }
    if(MatchINI(buffer, "SHOWSLICEINOBST") == 1){
      if((localfile==0&&global_scase.solid_ht3d==0)||localfile==1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &global_scase.show_slice_in_obst);
        global_scase.show_slice_in_obst=CLAMP(global_scase.show_slice_in_obst,0,2);
      }
      continue;
    }
    if(MatchINI(buffer, "SKIPEMBEDSLICE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &skip_slice_in_embedded_mesh);
      ONEORZERO(skip_slice_in_embedded_mesh);
      continue;
    }
    if(MatchINI(buffer, "TRAINERMODE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &trainer_mode);
      continue;
    }
    if(MatchINI(buffer, "COMPRESSAUTO") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &compress_autoloaded);
      continue;
    }
    if(MatchINI(buffer, "PLOT3DAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &n3dsmokes);
        for(i = 0; i<n3dsmokes; i++){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &seq_id);
          GetStartupPlot3D(seq_id);
        }
        update_load_files = 1;
        continue;
      }
      if(MatchINI(buffer, "VSLICEAUTO") == 1){
        int n3dsmokes = 0;
        int seq_id;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &n3dsmokes);
        for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
          vslicedata *vslicei;

          vslicei = global_scase.slicecoll.vsliceinfo + i;
          vslicei->autoload = 0;
        }
        for(i = 0; i<n3dsmokes; i++){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &seq_id);
          GetStartupVSlice(seq_id);
        }
        update_load_files = 1;
        continue;
      }
      if(MatchINI(buffer, "SLICEAUTO") == 1){
        int n3dsmokes = 0;
        int seq_id;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &n3dsmokes);
        for(i = 0; i<n3dsmokes; i++){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &seq_id);
          GetStartupSlice(seq_id);
        }
        update_load_files = 1;
        continue;
      }
    if(MatchINI(buffer, "MSLICEAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0;i < global_scase.slicecoll.nmultisliceinfo;i++){
        multislicedata *mslicei;

        mslicei = global_scase.slicecoll.multisliceinfo + i;
        mslicei->autoload = 0;
        mslicei->loadable = 0;
      }
      for(i = 0; i<n3dsmokes; i++){

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);

        if(seq_id >= 0 && seq_id<global_scase.slicecoll.nmultisliceinfo){
          multislicedata *mslicei;

          mslicei = global_scase.slicecoll.multisliceinfo + seq_id;
          mslicei->autoload = 1;
          mslicei->loadable = 1;
        }
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "PARTAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);

      for(i = 0; i < global_scase.npartinfo; i++){
        partdata *parti;

        parti = global_scase.partinfo + i;
        parti->autoload = 0;
      }
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupPart(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "ISOAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupISO(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "S3DAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupSmoke(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "PATCHAUTO") == 1){
      int n3dsmokes = 0;
      int seq_id;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n3dsmokes);
      for(i = 0; i<n3dsmokes; i++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &seq_id);
        GetStartupBoundary(seq_id);
      }
      update_load_files = 1;
      continue;
    }
    if(MatchINI(buffer, "LOADFILESATSTARTUP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &loadfiles_at_startup);
      continue;
    }
    if(MatchINI(buffer, "SHOWALLTEXTURES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &showall_textures);
      continue;
    }
    if(MatchINI(buffer, "ENABLETEXTURELIGHTING") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &enable_texture_lighting);
      continue;
    }
    if(MatchINI(buffer, "PROJECTION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &projection_type);
      projection_type = CLAMP(projection_type, 0, 1);
      GLUISceneMotionCB(PROJECTION);
      GLUIUpdateProjectionType();
      continue;
    }
    if(MatchINI(buffer, "V_PARTICLES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f", &setpartmin, &glui_partmin, &setpartmax, &glui_partmax);
      if(setpartmin==PERCENTILE_MIN){
        setpartmin = GLOBAL_MIN;
      }
      if(setpartmax==PERCENTILE_MAX){
        setpartmax = GLOBAL_MAX;
      }
      continue;
    }
    if(MatchINI(buffer, "V2_PARTICLES") == 1||
       MatchINI(buffer, "V_PARTICLES")==1||
       MatchINI(buffer, "V5_PARTICLES")==1
    ){
      int is_old_bound = 0;

      if(MatchINI(buffer, "V_PARTICLES")==1){
        is_old_bound = 1;
      }
      int ivmin, ivmax;
      float vmin, vmax;
      char short_label[256], *s1;

      strcpy(short_label, "");
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f %s", &ivmin, &vmin, &ivmax, &vmax, short_label);
      if(is_old_bound==1){
        ivmin = GetNewBoundIndex(ivmin);
        ivmax = GetNewBoundIndex(ivmax);
      }
      if(ivmin==BOUND_SET_MIN||ivmax==BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }

#define MAXVAL 100000000.0
#define MINVAL -100000000.0

      if(vmax > MAXVAL)vmax = 1.0;
      if(vmin < MINVAL)vmin = 0.0;

      if(ivmin == PERCENTILE_MIN && vmin > vmax)continue;
      if(ivmax == PERCENTILE_MAX && vmin > vmax)continue;

      if(npart5prop>0){
        int label_index = 0;

        TrimBack(short_label);
        s1 = TrimFront(short_label);
        if(strlen(s1)>0)label_index = GetPartPropIndexS(s1);
        if(label_index >= 0 && label_index<npart5prop){
          partpropdata *propi;

          propi = part5propinfo + label_index;
          propi->setvalmin = ivmin;
          propi->setvalmax = ivmax;
          propi->valmin = vmin;
          propi->valmax = vmax;
          if(is_old_bound==1){
            switch(ivmin){
            case PERCENTILE_MIN:
              propi->user_min = vmin;
              break;
              case GLOBAL_MIN:
                propi->dlg_global_valmin = vmin;
                break;
              case SET_MIN:
                propi->user_min = vmin;
                break;
              default:
                assert(FFALSE);
                break;
            }
            switch(ivmax){
            case PERCENTILE_MAX:
              propi->user_max = vmax;
              break;
              case GLOBAL_MAX:
                propi->dlg_global_valmax = vmax;
                break;
              case SET_MAX:
                propi->user_max = vmax;
                break;
              default:
                assert(FFALSE);
                break;
            }
          }
          else{
            switch(ivmin){
            case BOUND_PERCENTILE_MIN:
              propi->user_min = vmin;
              break;
              case BOUND_LOADED_MIN:
              case BOUND_GLOBAL_MIN:
                propi->dlg_global_valmin = vmin;
                break;
              case BOUND_SET_MIN:
                propi->user_min = vmin;
                break;
              default:
                assert(FFALSE);
                break;
            }
            switch(ivmax){
            case BOUND_PERCENTILE_MAX:
              propi->user_max = vmax;
              break;
              case BOUND_LOADED_MAX:
              case BOUND_GLOBAL_MAX:
                propi->dlg_global_valmax = vmax;
                break;
              case BOUND_SET_MAX:
                propi->user_max = vmax;
                break;
              default:
                assert(FFALSE);
                break;
            }
          }
#define MAX_PART_TYPES 100
          if(strcmp(short_label, "")==0){
            int npart_types;

            npart_types = GLUIGetNValtypes(BOUND_PART);
            if(npart_types>0){
              int  *ivmins, *ivmaxs;
              float *vmins, *vmaxs;
              int ivalmins[MAX_PART_TYPES],  ivalmaxs[MAX_PART_TYPES];
              float valmins[MAX_PART_TYPES], valmaxs[MAX_PART_TYPES];

              if(npart_types>MAX_PART_TYPES){
                NewMemory((void **)&ivmins, npart_types*sizeof(int));
                NewMemory((void **)&vmins,  npart_types*sizeof(float));
                NewMemory((void **)&ivmaxs, npart_types*sizeof(int));
                NewMemory((void **)&vmaxs,  npart_types*sizeof(float));
              }
              else{
                ivmins = ivalmins;
                ivmaxs = ivalmaxs;
                vmins = valmins;
                vmaxs = valmaxs;
              }
              for(i = 0; i<npart_types; i++){
                ivmins[i] = ivmin;
                ivmaxs[i] = ivmax;
                vmins[i]  = vmin;
                vmaxs[i]  = vmax;
              }
              GLUISetMinMaxAll(BOUND_PART, ivmins, vmins, ivmaxs, vmaxs, npart_types);
              if(npart_types>MAX_PART_TYPES){
                FREEMEMORY(ivmins);
                FREEMEMORY(vmins);
                FREEMEMORY(ivmaxs);
                FREEMEMORY(vmaxs);
              }
            }
          }
          else{
            GLUISetMinMax(BOUND_PART, short_label, ivmin, vmin, ivmax, vmax);
          }
          update_glui_bounds=1;
        }
      }
      continue;
    }
    if(MatchINI(buffer, "C_PARTICLES")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer2, "")!=0&&strcmp(buffer2,"Uniform")!=0){
        for(i = 0; i<npartbounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = partbounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->set_chopmin = setvalmin;
            boundi->chopmin     = valmin;
            boundi->set_chopmax = setvalmax;
            boundi->chopmax     = valmax;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "V2_SLICE")==1||
       MatchINI(buffer, "V_SLICE")==1){
      int is_old_bound;
      char *colon;

      is_old_bound=0;
      if(MatchINI(buffer, "V_SLICE")==1){
        is_old_bound = 1;
      }
      float valmin, valmax;
      int set_valmin, set_valmax;
      char *level_val;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &set_valmin, &valmin, &set_valmax, &valmax, buffer2);

      if(is_old_bound==1){
        set_valmin = GetNewBoundIndex(set_valmin);
        set_valmax = GetNewBoundIndex(set_valmax);
      }
      if(set_valmin==BOUND_SET_MIN||set_valmax==BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      colon = strstr(buffer, ":");
      level_val = NULL;
      if(colon != NULL){
        level_val = colon + 1;
        TrimBack(level_val);
        *colon = 0;
        if(strlen(level_val)>1){
          sscanf(level_val, "%f %f %i", &slice_line_contour_min, &slice_line_contour_max, &slice_line_contour_num);
        }
        level_val = NULL;
      }
      TrimBack(buffer2);
      SetSliceBounds(set_valmin, valmin, set_valmax, valmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "C_SLICE_HIDE")==1){
      int chop_hide;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i  %s", &chop_hide, buffer2);
      if(strcmp(buffer, "")!=0){
        for(i = 0; i<nslicebounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = slicebounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->chop_hide = chop_hide;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "C_SLICE")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer, "")!=0){
        for(i = 0; i<nslicebounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = slicebounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->set_chopmin = setvalmin;
            boundi->chopmin     = valmin;
            boundi->set_chopmax = setvalmax;
            boundi->chopmax     = valmax;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "V_ISO") == 1){
      float valmin, valmax;
      int setvalmin, setvalmax;
      char *isolabel;

#define SETVALMIN_ORIG 1
#define SETVALMAX_ORIG 1

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      isolabel = TrimFrontBack(buffer2);
      if(strcmp(isolabel, "") != 0){
        for(i = 0; i<niso_bounds; i++){
          if(strcmp(isolabel, isobounds[i].label->shortlabel) != 0)continue;
          isobounds[i].ini_defined = 1;
          isobounds[i].ini_setvalmin = setvalmin;
          isobounds[i].ini_setvalmax = setvalmax;
          if(setvalmin == SETVALMIN_ORIG)isobounds[i].ini_valmin = valmin;
          if(setvalmax == SETVALMAX_ORIG)isobounds[i].ini_valmax = valmax;
          update_iso_ini = 1;
          break;
        }
      }
      continue;
    }
    if(MatchINI(buffer, "C_ISO") == 1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer, "") != 0){
        for(i = 0; i<niso_bounds; i++){
          if(strcmp(isobounds[i].shortlabel, buffer2) != 0)continue;
          isobounds[i].setchopmin = setvalmin;
          isobounds[i].setchopmax = setvalmax;
          isobounds[i].chopmin = valmin;
          isobounds[i].chopmax = valmax;
          break;
        }
      }
      else{
        for(i = 0; i<niso_bounds; i++){
          isobounds[i].setchopmin = setvalmin;
          isobounds[i].setchopmax = setvalmax;
          isobounds[i].chopmin = valmin;
          isobounds[i].chopmax = valmax;
        }
      }
      continue;
    }
    if(MatchINI(buffer, "V2_BOUNDARY") == 1||
       MatchINI(buffer, "V_BOUNDARY")==1){
      int is_old_bound;

      is_old_bound=0;
      if(MatchINI(buffer, "V_BOUNDARY")==1){
        is_old_bound = 1;
      }
      fgets(buffer, 255, stream);
      TrimBack(buffer);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &glui_setpatchmin, &glui_patchmin, &glui_setpatchmax, &glui_patchmax, buffer2);
      if(is_old_bound==1){

        glui_setpatchmin = GetNewBoundIndex(glui_setpatchmin);
        glui_setpatchmax = GetNewBoundIndex(glui_setpatchmax);
      }
      if(glui_setpatchmin==BOUND_SET_MIN||glui_setpatchmax==BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      SetBoundBounds(glui_setpatchmin, glui_patchmin, glui_setpatchmax, glui_patchmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "C_BOUNDARY")==1){
      float valmin, valmax;
      int setvalmin, setvalmax;

      fgets(buffer, 255, stream);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &setvalmin, &valmin, &setvalmax, &valmax, buffer2);
      if(strcmp(buffer, "")!=0){
        for(i = 0; i<npatchbounds_cpp; i++){
          cpp_boundsdata *boundi;

          boundi = patchbounds_cpp+i;
          if(strcmp(buffer2,boundi->label)==0){
            boundi->set_chopmin = setvalmin;
            boundi->chopmin     = valmin;
            boundi->set_chopmax = setvalmax;
            boundi->chopmax     = valmax;
            break;
          }
        }
      }
      update_chop_colors = 1;
      continue;
    }
    if(MatchINI(buffer, "V2_HVACDUCT") == 1){
      int sethvacductmin, sethvacductmax;
      float hvacductmin, hvacductmax;

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &sethvacductmin, &hvacductmin, &sethvacductmax, &hvacductmax, buffer2);
      if(sethvacductmin != BOUND_SET_MIN)sethvacductmin = BOUND_LOADED_MIN;
      if(sethvacductmax != BOUND_SET_MAX)sethvacductmax = BOUND_LOADED_MAX;
      if(sethvacductmin == BOUND_SET_MIN || sethvacductmax == BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      SetHVACDuctBounds(sethvacductmin, hvacductmin, sethvacductmax, hvacductmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "V2_HVACNODE") == 1){
      int sethvacnodemin, sethvacnodemax;
      float hvacnodemin, hvacnodemax;

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      strcpy(buffer2, "");
      sscanf(buffer, "%i %f %i %f %s", &sethvacnodemin, &hvacnodemin, &sethvacnodemax, &hvacnodemax, buffer2);
      if(sethvacnodemin != BOUND_SET_MIN)sethvacnodemin = BOUND_LOADED_MIN;
      if(sethvacnodemax != BOUND_SET_MAX)sethvacnodemax = BOUND_LOADED_MAX;
      if(sethvacnodemin == BOUND_SET_MIN || sethvacnodemax == BOUND_SET_MAX){
        research_mode = 0;
        update_research_mode = 1;
      }
      SetHVACNodeBounds(sethvacnodemin, hvacnodemin, sethvacnodemax, hvacnodemax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "V_ZONE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f", &setzonemin, &zoneusermin, &setzonemax, &zoneusermax);
      if(setzonemin == PERCENTILE_MIN)setzonemin = GLOBAL_MIN;
      if(setzonemax == PERCENTILE_MIN)setzonemax = GLOBAL_MIN;
      if(setzonemin == SET_MIN)zonemin = zoneusermin;
      if(setzonemax == SET_MAX)zonemax = zoneusermax;
      GLUIUpdateZoneBounds();
      continue;
    }
    if(MatchINI(buffer, "V_TARGET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f %s", &settargetmin, &targetmin, &settargetmax, &targetmax, buffer2);
      continue;
    }
    if(MatchINI(buffer, "OUTLINECOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", glui_outlinecolor, glui_outlinecolor + 1, glui_outlinecolor + 2);
      glui_outlinecolor[0] = CLAMP(glui_outlinecolor[0], 0, 255);
      glui_outlinecolor[1] = CLAMP(glui_outlinecolor[1], 0, 255);
      glui_outlinecolor[2] = CLAMP(glui_outlinecolor[2], 0, 255);
      continue;
    }
    if(MatchINI(buffer, "ZONEVIEW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &zone_hvac_diam);
      if(zone_hvac_diam < 0.0)zone_hvac_diam = 0.0;
      continue;
    }
    if(MatchINI(buffer, "OUTLINEMODE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &highlight_flag, &outline_color_flag);
      if(global_scase.meshescoll.nmeshes<2){
        ONEORZERO(highlight_flag);
      }
      continue;
    }
    if(MatchINI(buffer, "SLICEDATAOUT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &output_slicedata);
      ONEORZERO(output_slicedata);
      continue;
    }
    if(MatchINI(buffer, "SMOKE3DZIPSTEP") == 1 ||
       MatchINI(buffer, "SLICEZIPSTEP")   == 1 ||
       MatchINI(buffer, "ISOZIPSTEP")     == 1 ||
       MatchINI(buffer, "BOUNDZIPSTEP")   == 1 ||
       MatchINI(buffer, "ZIPSTEP")        == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &tload_zipstep);
      tload_zipstep = MAX(tload_zipstep, 1);
      tload_zipskip = tload_zipstep - 1;
      continue;
    }
    if(MatchINI(buffer, "LOADINC") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &load_incremental);
      continue;
    }
    if(MatchINI(buffer, "MSCALE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", mscale, mscale + 1, mscale + 2);
      continue;
    }
    if(MatchINI(buffer, "RENDERCLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i",
        &clip_rendered_scene, &render_clip_left, &render_clip_right, &render_clip_bottom, &render_clip_top);
      continue;
    }
    if(MatchINI(buffer, "CLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &nearclip, &farclip);
      farclip_save = farclip;
      continue;
    }
    if(MatchINI(buffer, "SHOWTRACERSALWAYS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_tracers_always);
      ONEORZERO(show_tracers_always);
      continue;
    }
    if(MatchINI(buffer, "SHOWPARTTAG") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &select_part);
      ONEORZERO(select_part);
      continue;
    }
    if(MatchINI(buffer, "PARTSKIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &partdrawskip);
      partdrawskip = MAX(partdrawskip, 1);
    }
    if(MatchINI(buffer, "PART5COLOR") == 1){
      for(i = 0; i<npart5prop; i++){
        partpropdata *propi;

        propi = part5propinfo + i;
        propi->display = 0;
      }
      part5colorindex = 0;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &i);
      if(i >= 0 && i<npart5prop){
        partpropdata *propi;

        part5colorindex = i;
        propi = part5propinfo + i;
        propi->display = 1;
      }
      continue;
    }
    if(MatchINI(buffer, "PART5PROPDISP") == 1){
      char *token;

      for(i = 0; i<npart5prop; i++){
        partpropdata *propi;
        int j;

        propi = part5propinfo + i;
        fgets(buffer, 255, stream);

        TrimBack(buffer);
        token = strtok(buffer, " ");
        j = 0;
        while(token != NULL&&j<global_scase.npartclassinfo){
          int visval;

          sscanf(token, "%i", &visval);
          propi->class_vis[j] = visval;
          token = strtok(NULL, " ");
          j++;
        }
      }
      CheckMemory;
      continue;
    }
    if(MatchINI(buffer, "COLORBAR") == 1){
      float *rgb_ini_copy;
      int nn;
      int dummy;

      CheckMemory;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i", &nrgb_ini, &dummy, &colorbar_select_index, &colorbar_selection_width);
      if(nrgb_ini!=12){
        fprintf(stderr, "***warning: COLORBAR ini parameter skipped - only colorbars with 12 entries are supported.\n");
        continue;
      }
      colorbar_selection_width = CLAMP(colorbar_selection_width, COLORBAR_SELECTION_WIDTH_MIN, COLORBAR_SELECTION_WIDTH_MAX);
      FREEMEMORY(rgb_ini);
      if(NewMemory((void **)&rgb_ini, 4 * nrgb_ini*sizeof(float)) == 0)return 2;
      rgb_ini_copy = rgb_ini;
      for(nn = 0; nn<nrgb_ini; nn++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f ", rgb_ini_copy, rgb_ini_copy + 1, rgb_ini_copy + 2);
        rgb_ini_copy += 3;
      }
      InitRGB();
      if(colorbar_select_index >= 0 && colorbar_select_index <= 255){
        update_colorbar_select_index = 1;
      }
      continue;
    }
    if(MatchINI(buffer, "COLOR2BAR") == 1){
      float *rgb_ini_copy;
      int nn;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &nrgb2_ini);
      if(nrgb2_ini<8){
        fprintf(stderr, "*** Error: must have at lease 8 colors in COLOR2BAR\n");
        SMV_EXIT(1);
      }
      FREEMEMORY(rgb2_ini);
      if(NewMemory((void **)&rgb2_ini, 4 * nrgb_ini*sizeof(float)) == 0)return 2;
      rgb_ini_copy = rgb2_ini;
      for(nn = 0; nn<nrgb2_ini; nn++){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f ", rgb_ini_copy, rgb_ini_copy + 1, rgb_ini_copy + 2);
        rgb_ini_copy += 3;
      }
      continue;
    }
    if(MatchINI(buffer, "PLOT2DHRRBOUNDS") == 1){
      int dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %f", &dummy, &plot2d_hrr_min, &dummy, &plot2d_hrr_max);
      continue;
    }
    if(MatchINI(buffer, "P3DSURFACETYPE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &p3dsurfacetype);
      continue;
    }
    if(MatchINI(buffer, "P3DSURFACESMOOTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &p3dsurfacesmooth);
      continue;
    }
    if(MatchINI(buffer, "CULLFACES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &cullfaces);
      continue;
    }
    if(MatchINI(buffer, "PARTPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &partpointsize);
      continue;
    }
    if(MatchINI(buffer, "ISOPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &isopointsize);
      continue;
    }
    if(MatchINI(buffer, "ISOLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &isolinewidth);
      continue;
    }
    if(MatchINI(buffer, "PLOT3DPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &plot3dpointsize);
      continue;
    }
    if(MatchINI(buffer, "GRIDLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &gridlinewidth);
      continue;
    }
    if(MatchINI(buffer, "TICKLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &ticklinewidth);
      continue;
    }
    if(MatchINI(buffer, "PLOT3DLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &plot3dlinewidth);
      continue;
    }
    if(MatchINI(buffer, "VECTORCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &color_vector_black);
      continue;
    }
    if(MatchINI(buffer, "VECTORPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &vectorpointsize);
      continue;
    }
    if(MatchINI(buffer, "VECTORLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &vectorlinewidth, &slice_line_contour_width);
      continue;
    }
    if(MatchINI(buffer, "STREAKLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &streaklinewidth);
      continue;
    }
    if(MatchINI(buffer, "LINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &global_scase.linewidth);
      solidlinewidth = global_scase.linewidth;
      continue;
    }
    if(MatchINI(buffer, "VENTLINEWIDTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &global_scase.ventlinewidth);
      continue;
    }
    if(MatchINI(buffer, "BOUNDARYOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &boundaryoffset);
      continue;
    }
    if(MatchINI(buffer, "SLICEOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i", &sliceoffset_factor, &slice_dz, &agl_offset_actual);
      continue;
    }
    if(MatchINI(buffer, "TITLESAFE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &titlesafe_offset);
      continue;
    }
    if(MatchINI(buffer, "VENTOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &ventoffset_factor);
      continue;
    }
    if(MatchINI(buffer, "SHOWBLOCKS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &visBlocks_ini, &solid_state_ini, &outline_state_ini);
      update_showblock_ini = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWSENSORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i ", &visSensor, &visSensorNorm);
      ONEORZERO(visSensor);
      ONEORZERO(visSensorNorm);
      continue;
    }
    if(MatchINI(buffer, "SHOWVENTFLOW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i", &visVentHFlow, &visventslab, &visventprofile, &visVentVFlow, &visVentMFlow);
      continue;
    }
    if(MatchINI(buffer, "SHOWVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visVents);
      continue;
    }
    if(MatchINI(buffer, "SHOWROOMS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visCompartments);
      continue;
    }
    if(MatchINI(buffer, "SHOWTIMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visTimelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWHMSTIMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vishmsTimelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAMETIMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visFrameTimelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAMELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visFramelabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWHRRLABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vis_hrr_label);
      continue;
    }
    if(MatchINI(buffer, "RENDERFILETYPE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &render_filetype, &movie_filetype, &render_resolution);
      RenderCB(RENDER_RESOLUTION);
      continue;
    }
    if(MatchINI(buffer, "MOVIEFILETYPE") == 1){
      int quicktime_dummy;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i", &movie_filetype,&movie_framerate,&movie_bitrate,&quicktime_dummy,&movie_crf);
      continue;
    }
    if(MatchINI(buffer, "MOVIEPARMS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &movie_queue_index, &movie_nprocs, &movie_slice_index);
      movie_queue_index = CLAMP(movie_queue_index, 0, nmovie_queues-1);
      movie_slice_index = CLAMP(movie_slice_index, 0, nslicemenuinfo-1);
      update_movie_parms = 1;
      continue;
    }
    if(MatchINI(buffer, "RENDERFILELABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &render_label_type);
      ONEORZERO(render_label_type);
      continue;
    }
    if(MatchINI(buffer, "CELLCENTERTEXT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &show_slice_values_all_regions);
      continue;
    }
    if(MatchINI(buffer, "SHOWSLICEVALS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &show_slice_values_all_regions);
      continue;
    }
    if(MatchINI(buffer, "SHOWGRIDLOC") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visgridloc);
      continue;
    }
    if(MatchINI(buffer, "SHOWGRID") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visGrid);
      continue;
    }
    if(MatchINI(buffer, "SHOWFLOOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &global_scase.visFloor);
      continue;
    }
    if(MatchINI(buffer, "SPEED") == 1){
      fgets(buffer, 255, stream);
      //  sscanf(buffer,"%f %f",&speed_crawl,&speed_walk);
      continue;
    }
    if(MatchINI(buffer, "FONTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &fontindex);
      fontindex = CLAMP(fontindex, 0, SCALED_FONT);
      FontMenu(fontindex);
      continue;
    }
    if(MatchINI(buffer, "ZOOM") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f ", &zoomindex, &zoom);
      if(zoomindex<0)zoomindex = ZOOMINDEX_ONE;
      if(zoomindex>MAX_ZOOMS+1)zoomindex = ZOOMINDEX_ONE;
      zooms[zoomindex] = zoom;
      zoomini = zoom;
      updatezoomini = 1;
      ZoomMenu(zoomindex);
      continue;
    }
    if(MatchINI(buffer, "APERATURE") == 1 ||
       MatchINI(buffer, "APERTURE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &apertureindex);
      apertureindex = CLAMP(apertureindex, 0, 4);
      ApertureMenu(apertureindex);
      continue;
    }
    if(MatchINI(buffer, "SHOWTARGETS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis_target_data);
      continue;
    }
    if(MatchINI(buffer, "SHOWWALLS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &global_scase.visWalls,&vis_wall_data);
      continue;
    }
    if(MatchINI(buffer, "SHOWCEILING") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &global_scase.visCeiling);
      continue;
    }
    if(MatchINI(buffer, "SHOWTITLE") == 1){
      int dummy_val;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &vis_title_smv_version,&dummy_val,&vis_title_fds);
      continue;
    }
    if(MatchINI(buffer, "SHOWCHID") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vis_title_CHID);
      continue;
    }
    if(MatchINI(buffer, "SHOWTRANSPARENT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &visTransparentBlockage);
      continue;
    }
    if(MatchINI(buffer, "SHOWCADOPAQUE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &viscadopaque);
      continue;
    }
    if(MatchINI(buffer, "VECTORPOINTSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &vectorpointsize);
      continue;
    }
    if(MatchINI(buffer, "VECTORSKIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &vectorskip);
      if(vectorskip<1)vectorskip = 1;
      update_vectorskip = 1;
      continue;
    }
    if(MatchINI(buffer, "SPRINKLERABSSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &sprinklerabssize);
      continue;
    }
    if(MatchINI(buffer, "SENSORABSSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &sensorabssize);
      continue;
    }
    if(MatchINI(buffer, "SENSORRELSIZE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f ", &sensorrelsize);
      if(sensorrelsize<sensorrelsizeMIN)sensorrelsize = sensorrelsizeMIN;
      continue;
    }
    if(MatchINI(buffer, "SETBW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &setbw, &setbwdata);
      continue;
    }
    if(MatchINI(buffer, "FLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &background_flip);
      continue;
    }
    if(MatchINI(buffer, "COLORBAR_FLIP") == 1 ||
       MatchINI(buffer, "COLORBARFLIP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &colorbar_flip,&colorbar_autoflip);
      continue;
    }
    if(MatchINI(buffer, "TRANSPARENT") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f", &use_transparency_data, &transparent_level);
      continue;
    }
    if(MatchINI(buffer, "SORTSLICES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &sortslices);
      ONEORZERO(sortslices);
      continue;
    }
    if(MatchINI(buffer, "VENTCOLOR") == 1){
      float ventcolor_temp[4];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", ventcolor_temp, ventcolor_temp + 1, ventcolor_temp + 2);
      ventcolor_temp[3] = 1.0;
      ventcolor = GetColorPtr(&global_scase, ventcolor_temp);
      global_scase.updatefaces = 1;
      global_scase.updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "STATICPARTCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", static_color, static_color + 1, static_color + 2);
      continue;
    }
    if(MatchINI(buffer, "HEATOFFCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", heatoffcolor, heatoffcolor + 1, heatoffcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "HEATONCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", heatoncolor, heatoncolor + 1, heatoncolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SENSORCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sensorcolor, sensorcolor + 1, sensorcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SENSORNORMCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sensornormcolor, sensornormcolor + 1, sensornormcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SPRINKONCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sprinkoncolor, sprinkoncolor + 1, sprinkoncolor + 2);
      continue;
    }
    if(MatchINI(buffer, "SPRINKOFFCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", sprinkoffcolor, sprinkoffcolor + 1, sprinkoffcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "BACKGROUNDCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", backgroundbasecolor, backgroundbasecolor + 1, backgroundbasecolor + 2);
      GLUISetColorControls();
      continue;
    }
    if(MatchINI(buffer, "SURFCOLORS")==1){
      int ncolors;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &ncolors, &use_surf_color);
      for(i = 0; i<ncolors; i++){
        surfdata *surfi;
        int *ini_surf_color;
        char *surflabel;

        fgets(buffer, 255, stream);
        surflabel = strchr(buffer, ':');
        if(surflabel==NULL)continue;
        surflabel = TrimFrontBack(surflabel+1);
        surfi = GetSurface(&global_scase, surflabel);
        if(surfi==NULL)continue;
        ini_surf_color = surfi->geom_surf_color;
        sscanf(buffer, "%i %i %i", ini_surf_color, ini_surf_color+1, ini_surf_color+2);
        ini_surf_color[0] = CLAMP(ini_surf_color[0], 0, 255);
        ini_surf_color[1] = CLAMP(ini_surf_color[1], 0, 255);
        ini_surf_color[2] = CLAMP(ini_surf_color[2], 0, 255);
      }
      continue;
    }
    if(MatchINI(buffer, "OBSTSURFCOLORS")==1){
      int ncolors;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &ncolors);
      for(i = 0; i<ncolors; i++){
        surfdata *surfi;
        char *surflabel;
        float s_color[4];

        fgets(buffer, 255, stream);
        surflabel = strchr(buffer, ':');
        if(surflabel==NULL)continue;
        surflabel = TrimFrontBack(surflabel+1);
        surfi = GetSurface(&global_scase, surflabel);
        if(surfi==NULL)continue;
        s_color[0] = -1.0;
        s_color[1] = -1.0;
        s_color[2] = -1.0;
        s_color[3] = -1.0;
        sscanf(buffer, "%f %f %f %f", s_color, s_color+1, s_color+2, s_color+3);
        if(s_color[3]<0.0)s_color[3] = 1.0;
        if(s_color[0]<0.0||s_color[1]<0.0||s_color[2]<0.0)continue;
        s_color[0] = CLAMP(s_color[0], 0.0, 1.0);
        s_color[1] = CLAMP(s_color[1], 0.0, 1.0);
        s_color[2] = CLAMP(s_color[2], 0.0, 1.0);
        surfi->color = GetColorPtr(&global_scase, s_color);
        surfi->transparent_level=s_color[3];
      }
      continue;
    }
    if(MatchINI(buffer, "GEOMSELECTCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%u %u %u",  geom_vertex1_rgb,  geom_vertex1_rgb+1,  geom_vertex1_rgb+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%u %u %u",  geom_vertex2_rgb,  geom_vertex2_rgb+1,  geom_vertex2_rgb+2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%u %u %u", geom_triangle_rgb, geom_triangle_rgb+1, geom_triangle_rgb+2);
      for(i = 0; i<3; i++){
        geom_vertex1_rgb[i]  = CLAMP(geom_vertex1_rgb[i], 0, 255);
        geom_vertex2_rgb[i]  = CLAMP(geom_vertex2_rgb[i], 0, 255);
        geom_triangle_rgb[i] = CLAMP(geom_triangle_rgb[i], 0, 255);
      }
      continue;
    }
    if(MatchINI(buffer, "GEOMAXIS")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", &glui_surf_axis_length, &glui_surf_axis_width);
      continue;
    }
    if(MatchINI(buffer, "FOREGROUNDCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", foregroundbasecolor, foregroundbasecolor + 1, foregroundbasecolor + 2);
      GLUISetColorControls();
      continue;
    }
    if(MatchINI(buffer, "BEAM") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %f %i %i %i %i",
          &showbeam_as_line,&beam_line_width,&use_beamcolor,beam_color,beam_color+1,beam_color+2);
      showbeam_as_line = CLAMP(showbeam_as_line,0,1);
      use_beamcolor = CLAMP(use_beamcolor,0,1);
      beam_color[0] = CLAMP(beam_color[0], 0, 255);
      beam_color[1] = CLAMP(beam_color[1], 0, 255);
      beam_color[2] = CLAMP(beam_color[2], 0, 255);
      continue;
    }
    if(MatchINI(buffer, "BLOCKLOCATION") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &blocklocation);
      continue;
    }
    if(MatchINI(buffer, "BLOCKCOLOR") == 1){
      float blockcolor_temp[4];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", blockcolor_temp, blockcolor_temp + 1, blockcolor_temp + 2);
      blockcolor_temp[3] = 1.0;
      block_ambient2 = GetColorPtr(&global_scase, blockcolor_temp);
      global_scase.updatefaces = 1;
      global_scase.updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "BLOCKSHININESS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", &block_shininess);
      global_scase.updatefaces = 1;
      global_scase.updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "BLOCKSPECULAR") == 1){
      float blockspec_temp[4];

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", blockspec_temp, blockspec_temp + 1, blockspec_temp + 2);
      blockspec_temp[3] = 1.0;
      block_specular2 = GetColorPtr(&global_scase, blockspec_temp);
      global_scase.updatefaces = 1;
      global_scase.updateindexcolors = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWOPENVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visOpenVents, &visOpenVentsAsOutline);
      continue;
    }
    if(MatchINI(buffer, "SHOWDUMMYVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visDummyVents);
      continue;
    }
    if(MatchINI(buffer, "SHOWOTHERVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &global_scase.visOtherVents);
      update_ini_vents = 1;
      ONEORZERO(global_scase.visOtherVents);
      global_scase.visOtherVentsSAVE = global_scase.visOtherVents;
      continue;
    }
    if(MatchINI(buffer, "SHOWCVENTS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visCircularVents, &circle_outline);
      visCircularVents = CLAMP(visCircularVents, 0, 2);
      circle_outline = CLAMP(circle_outline, 0, 1);
      continue;
    }
    if(MatchINI(buffer, "SHOWTICKS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFDSticks);
      continue;
    }
    if(MatchINI(buffer, "USERTICKS") == 1){
      fgets(buffer, 255, stream);

      sscanf(buffer, "%i %i %i %i %i %i %f %i", &visUSERticks, &auto_user_tick_placement, &user_tick_sub,
        &user_tick_show_x, &user_tick_show_y, &user_tick_show_z, &user_tick_direction, &ntick_decimals);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_origin, user_tick_origin + 1, user_tick_origin + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_min, user_tick_min + 1, user_tick_min + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_max, user_tick_max + 1, user_tick_max + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", user_tick_step, user_tick_step + 1, user_tick_step + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &user_tick_show_x, &user_tick_show_y, &user_tick_show_z);
      continue;
    }
    if(MatchINI(buffer, "SHOWLABELS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visLabels);
      continue;
    }
    if(MatchINI(buffer, "BOUNDCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", boundcolor, boundcolor + 1, boundcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "TIMEBARCOLOR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", timebarcolor, timebarcolor + 1, timebarcolor + 2);
      continue;
    }
    if(MatchINI(buffer, "CONTOURTYPE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &contour_type, &colorbar_linewidth);
      contour_type = CLAMP(contour_type, 0, 2);
      colorbar_linewidth = CLAMP(colorbar_linewidth, 1, 10);
      continue;
    }
    if(MatchINI(buffer, "P3CONT3DSMOOTH") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &p3cont3dsmooth);
      continue;
    }
    if(MatchINI(buffer, "SURFINC") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &surfincrement);
      continue;
    }
    if(MatchINI(buffer, "FRAMERATE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFramerate);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAMERATE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visFramerate);
      continue;
    }
    if(MatchINI(buffer, "SHOWFRAME") == 1 &&
       MatchINI(buffer, "SHOWFRAMERATE") != 1 &&
       MatchINI(buffer, "SHOWFRAMELABEL") != 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &global_scase.visFrame);
      ONEORZERO(global_scase.visFrame);
      continue;
    }
    if(MatchINI(buffer, "FRAMERATEVALUE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &frameratevalue);
      FrameRateMenu(frameratevalue);
      continue;
    }
    if(MatchINI(buffer, "SHOWSPRINKPART") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSprinkPart);
      continue;
    }
    if(MatchINI(buffer, "SHOWAXISLABELS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visaxislabels);
      continue;
    }
#ifdef pp_memstatus
    if(MatchINI(buffer, "SHOWMEMLOAD") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visAvailmemory);
      continue;
    }
#endif
    if(MatchINI(buffer, "SHOWBLOCKLABEL") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visMeshlabel);
      continue;
    }
    if(MatchINI(buffer, "SHOWZONEPLANE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visZonePlane, &show_zonelower);
      continue;
    }
    if(MatchINI(buffer, "SHOWVZONE") == 1){
      int vis;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &vis, &show_zonelower);
      if(vis==1){
        visZonePlane = ZONE_YPLANE;
      }
      else{
        visZonePlane = ZONE_HIDDEN;
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWZONEFIRE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i ", &viszonefire);
      if(viszonefire != 0)viszonefire = 1;
      continue;
    }
    if(MatchINI(buffer, "SHOWSZONE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSZone);
      continue;
    }
    if(MatchINI(buffer, "SHOWHZONE") == 1){
      int vis;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &vis);
      if(vis==1){
        visZonePlane = ZONE_ZPLANE;
      }
      else{
        visZonePlane = ZONE_HIDDEN;
      }
      continue;
    }
    if(MatchINI(buffer, "SHOWHAZARDCOLORS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &zonecolortype);
      zonecolortype = CLAMP(zonecolortype, 0, 2);
      continue;
    }
    if(MatchINI(buffer, "SHOWSKYSPHERE") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %f", &visSkysphere, &visSkybox, &visSkyground, &visSkyboxoutline, &visSkySpheretexture, &sky_diam);
      ONEORZERO(visSkysphere);
      ONEORZERO(visSkybox);
      ONEORZERO(visSkyground);
      ONEORZERO(visSkyboxoutline);
      ONEORZERO(visSkySpheretexture);
      GLUISkyCB(SKY_BOX);
      GLUISkyCB(SKY_SPHERE);
      continue;
    }
    if(MatchINI(buffer, "SHOWSMOKEPART") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visSmokePart);
      continue;
    }
    if(MatchINI(buffer, "RENDEROPTION") == 1){
      int nheight360_temp = 0;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i", &render_window_size, &resolution_multiplier, &nheight360_temp);
      if(nheight360_temp > 0){
        nheight360 = nheight360_temp;
        nwidth360 = 2 * nheight360;
      }
      RenderMenu(render_window_size);
      continue;
    }
    if(MatchINI(buffer, "SHOWISONORMALS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &show_iso_normal);
      if(show_iso_normal != 1)show_iso_normal = 0;
      continue;
    }
    if(MatchINI(buffer, "SHOWISO") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &visAIso);
      visAIso &= 7;
      show_iso_shaded = (visAIso & 1) / 1;
      show_iso_outline = (visAIso & 2) / 2;
      show_iso_points = (visAIso & 4) / 4;
      continue;
    }
    if(trainer_mode == 0 && windowresized == 0){
      if(MatchINI(buffer, "WINDOWWIDTH") == 1){
        int scrWidth;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &scrWidth);
        if(scrWidth <= 0){
          scrWidth = GLUTGetScreenWidth();
        }
        if(scrWidth != screenWidth){
          SetScreenSize(&scrWidth, NULL);
          screenWidthINI = scrWidth;
          update_screensize = 1;
        }
        continue;
      }
      if(MatchINI(buffer, "WINDOWHEIGHT") == 1){
        int scrHeight;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &scrHeight);
        if(scrHeight <= 0){
          scrHeight = GLUTGetScreenHeight();
        }
        if(scrHeight != screenHeight){
          SetScreenSize(NULL, &scrHeight);
          screenHeightINI = scrHeight;
          update_screensize = 1;
        }
        continue;
      }
    }
    if(MatchINI(buffer, "SHOWTIMEBAR") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visTimebar,&timebar_overlap);
      continue;
    }
    if(MatchINI(buffer, "SHOWCOLORBARS") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i", &visColorbarVertical_val, &visColorbarHorizontal_val);
      if(visColorbarVertical_val==1)visColorbarHorizontal_val=0;
      if(visColorbarHorizontal_val==1)visColorbarVertical_val=0;
  // if colorbars are hidden then research mode needs to be off
      update_visColorbars=1;
      continue;
    }
    if(MatchINI(buffer, "EYEVIEW") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &rotation_type);
      continue;
    }
    if(MatchINI(buffer, "NOPART") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nopart);
      continue;
    }
    if(MatchINI(buffer, "PARTFAST")==1){
      fgets(buffer, 255, stream);
      if(current_script_command==NULL){
        sscanf(buffer, "%i %i %i", &partfast, &use_partload_threads, &n_partload_threads);
      }
#ifdef pp_PARTFRAME
      use_partload_threads = 0;
#endif
      continue;
    }
    if(MatchINI(buffer, "WINDOWOFFSET") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &titlesafe_offsetBASE);
      continue;
    }
    if(MatchINI(buffer, "LIGHTING")==1){
      fgets(buffer, 255, stream);
      if(HaveSmokeSensor()==0){ // don't use lighting if there are devices of type smokesensor
        sscanf(buffer, "%i", &use_lighting);
        update_use_lighting = 1;
      }
      continue;
    }
    if(MatchINI(buffer, "LIGHTFACES")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%d", &light_faces);
      ONEORZERO(light_faces);
      continue;
    }
    if(MatchINI(buffer, "LIGHTANGLES0")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i", &glui_light_az0, &glui_light_elev0, &use_light0);
      continue;
    }
    if(MatchINI(buffer, "LIGHTANGLES1")==1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i", &glui_light_az1, &glui_light_elev1, &use_light1);
      continue;
    }
    if(MatchINI(buffer, "LIGHTPROP") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", ambientlight, ambientlight + 1, ambientlight + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", diffuselight, diffuselight + 1, diffuselight + 2);
      continue;
    }
    if(MatchINI(buffer, "LABELSTARTUPVIEW") == 1){
      char *front;

      fgets(buffer, 255, stream);
      front = TrimFront(buffer);
      TrimBack(front);
      strcpy(viewpoint_label_startup, front);
      update_startup_view = 3;
      continue;
    }
    if(MatchINI(buffer, "USER_ROTATE") == 1){
      if(fgets(buffer, 255, stream) == NULL)break;
      sscanf(buffer, "%i %i %f %f %f", &glui_rotation_index_ini, &show_rotation_center, &xcenCUSTOM, &ycenCUSTOM, &zcenCUSTOM);
      if(glui_rotation_index_ini>=0)glui_rotation_index = ROTATE_ABOUT_FDS_CENTER;
      update_rotation_center_ini = 1;
      continue;
    }
    if(MatchINI(buffer, "INPUT_FILE") == 1){
      size_t len;

      if(fgets(buffer, 255, stream) == NULL)break;
      len = strlen(buffer);
      buffer[len - 1] = '\0';
      TrimBack(buffer);
      len = strlen(buffer);

      FREEMEMORY(INI_fds_filein);
      if(NewMemory((void **)&INI_fds_filein, (unsigned int)(len + 1)) == 0)return 2;
      STRCPY(INI_fds_filein, buffer);
      continue;
    }
    if(MatchINI(buffer, "VIEWPOINT5") == 1 ||
       MatchINI(buffer, "VIEWPOINT6") == 1){
      int p_type;
      float *eye, mat[16], *az_elev;
      int is_viewpoint6 = 0;
      float xyzmaxdiff_local = -1.0;
      float xmin_local = 0.0, ymin_local = 0.0, zmin_local = 0.0;
      char name_ini[32];
      float zoom_in;
      int zoomindex_in;
      cameradata camera_local, *ci;
      char *bufferptr;

      ci = &camera_local;

      if(MatchINI(buffer, "VIEWPOINT6") == 1)is_viewpoint6 = 1;

      eye = ci->eye;
      az_elev = ci->az_elev;

      strcpy(name_ini, "ini");
      InitCamera(ci, name_ini);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %f %f %f %f",
        &ci->rotation_type, &ci->rotation_index, &ci->view_id,
        &xyzmaxdiff_local, &xmin_local, &ymin_local, &zmin_local);

      zoom_in = zoom;
      zoomindex_in = zoomindex;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f %i", eye, eye + 1, eye + 2, &zoom_in, &zoomindex_in);
      if(xyzmaxdiff_local>0.0){
        eye[0] = xmin_local + eye[0] * xyzmaxdiff_local;
        eye[1] = ymin_local + eye[1] * xyzmaxdiff_local;
        eye[2] = zmin_local + eye[2] * xyzmaxdiff_local;
      }
      zoom = zoom_in;
      zoomindex = zoomindex_in;
      if(zoomindex != -1){
        if(zoomindex<0)zoomindex = ZOOMINDEX_ONE;
        if(zooms[MAX_ZOOMS]>0.0&&zoomindex>MAX_ZOOMS)zoomindex = ZOOMINDEX_ONE;
        if(zooms[MAX_ZOOMS]<=0.0&&zoomindex>MAX_ZOOMS-1)zoomindex = ZOOMINDEX_ONE;
        zoom = zooms[zoomindex];
      }
      else{
        if(zoom<zooms[0]){
          zoom = zooms[0];
          zoomindex = 0;
        }
        if(zoomindex!=MAX_ZOOMS&&zoom>zooms[MAX_ZOOMS-1]){
          zoom = zooms[MAX_ZOOMS-1];
          zoomindex = MAX_ZOOMS-1;
        }
      }
      updatezoommenu = 1;
      p_type = 0;
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %i", &ci->view_angle, &ci->azimuth, &ci->elevation, &p_type);
      if(p_type != 1)p_type = 0;
      ci->projection_type = p_type;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", &ci->xcen, &ci->ycen, &ci->zcen);
      if(xyzmaxdiff_local>0.0){
        ci->xcen = xmin_local + ci->xcen*xyzmaxdiff_local;
        ci->ycen = ymin_local + ci->ycen*xyzmaxdiff_local;
        ci->zcen = zmin_local + ci->zcen*xyzmaxdiff_local;
      }

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f", az_elev, az_elev + 1);

      if(is_viewpoint6 == 1){
        float *q;

        for(i = 0; i<16; i++){
          mat[i] = 0.0;
          if(i % 5 == 0)mat[i] = 1.0;
        }
        q = ci->quaternion;
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %f %f %f", &ci->quat_defined, q, q + 1, q + 2, q + 3);
      }

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat, mat + 1, mat + 2, mat + 3);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat + 4, mat + 5, mat + 6, mat + 7);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat + 8, mat + 9, mat + 10, mat + 11);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f", mat + 12, mat + 13, mat + 14, mat + 15);

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i %i %i %i %i %i %i",
        &ci->clip_mode,
        &ci->clip_xmin, &ci->clip_ymin, &ci->clip_zmin,
        &ci->clip_xmax, &ci->clip_ymax, &ci->clip_zmax);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f %f %f %f",
        &ci->xmin, &ci->ymin, &ci->zmin,
        &ci->xmax, &ci->ymax, &ci->zmax);
      if(xyzmaxdiff_local>0.0){
        ci->xmin = xmin_local + ci->xmin*xyzmaxdiff_local;
        ci->xmax = xmin_local + ci->xmax*xyzmaxdiff_local;
        ci->ymin = ymin_local + ci->ymin*xyzmaxdiff_local;
        ci->zmax = ymin_local + ci->ymax*xyzmaxdiff_local;
        ci->ymin = zmin_local + ci->zmin*xyzmaxdiff_local;
        ci->zmax = zmin_local + ci->zmax*xyzmaxdiff_local;
      }

      fgets(buffer, 255, stream);
      TrimBack(buffer);
      bufferptr = TrimFront(buffer);
      strcpy(ci->name, bufferptr);
      InitCameraList();
      InsertCamera(&camera_list_first, ci, bufferptr);

      GLUIEnableResetSavedView();
      ci->dirty = 1;
      ci->defined = 1;
      continue;
    }
    if(MatchINI(buffer, "COLORTABLE") == 1){
      int nctableinfo;
      colortabledata *ctableinfo = NULL;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nctableinfo);
      nctableinfo = MAX(nctableinfo, 0);
      if(nctableinfo>0){
        NewMemory((void **)&ctableinfo, nctableinfo*sizeof(colortabledata));
        for(i = 0; i<nctableinfo; i++){
          colortabledata *rgbi;
          char *labelptr, *percenptr, label[256];
          int  colori[4];


          rgbi = ctableinfo + i;
          fgets(buffer, 255, stream);
          percenptr = strchr(buffer, '%');
          if(percenptr != NULL){
            labelptr = TrimFront(percenptr + 1);
            TrimBack(labelptr);
            strcpy(rgbi->label, labelptr);
            percenptr[0] = 0;
          }
          else{
            sprintf(label, "Color %i", i + 1);
            strcpy(rgbi->label, label);
          }
          colori[3] = 255;
          sscanf(buffer, "%i %i %i %i", colori, colori + 1, colori + 2, colori + 3);
          rgbi->color[0] = CLAMP(colori[0], 0, 255);
          rgbi->color[1] = CLAMP(colori[1], 0, 255);
          rgbi->color[2] = CLAMP(colori[2], 0, 255);
          rgbi->color[3] = CLAMP(colori[3], 0, 255);
        }
        UpdateColorTable(ctableinfo, nctableinfo);
        FREEMEMORY(ctableinfo);
      }
      continue;
    }

    if(MatchINI(buffer, "ISOCOLORS") == 1){
      int nn, n_iso_c = 0;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %i %i", &iso_shininess, &iso_transparency, &iso_transparency_option, &iso_opacity_change);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f %f %f", iso_specular, iso_specular + 1, iso_specular + 2);
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &n_iso_c);
      for(nn = 0; nn<MAX_ISO_COLORS; nn++){
        float *iso_color;

        iso_color = iso_colors + 4 * nn;
        if(nn < n_iso_c){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%f %f %f %f", iso_color, iso_color + 1, iso_color + 2, iso_color + 3);
        }
        iso_color[0] = CLAMP(iso_color[0], 0.0, 1.0);
        iso_color[1] = CLAMP(iso_color[1], 0.0, 1.0);
        iso_color[2] = CLAMP(iso_color[2], 0.0, 1.0);
        iso_color[3] = CLAMP(iso_color[3], 0.0, 1.0);
      }
      UpdateIsoColors();
      GLUIUpdateIsoColorlevel();
      continue;
    }
    if(MatchINI(buffer, "UNITCLASSES") == 1){
      int nuc;

      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &nuc);
      for(i = 0; i<nuc; i++){
        int unit_index;

        fgets(buffer, 255, stream);
        if(i>nunitclasses - 1)continue;
        sscanf(buffer, "%i", &unit_index);
        unitclasses[i].unit_index = unit_index;
      }
      continue;
    }
    if(MatchINI(buffer, "SMOOTHLINES") == 1){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%i", &antialiasflag);
      continue;
    }
    {
      if(MatchINI(buffer, "SMOKECULL") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &smokecullflag);
        continue;
      }
      if(MatchINI(buffer, "SMOKESKIP") == 1){
        int smokeskippm1_local;
        
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i %i %i", &smokeskippm1_local, &smoke3d_skip, &smoke3d_skipx, &smoke3d_skipy, &smoke3d_skipz);
        if(smokeskippm1_local<0)smokeskippm1_local = 0;
        smoke3d_frame_inc = smokeskippm1_local + 1;
        smoke3d_skip  = CLAMP(smoke3d_skip,1,10);
        smoke3d_skipx = CLAMP(smoke3d_skipx, 1, 10);
        smoke3d_skipy = CLAMP(smoke3d_skipy, 1, 10);
        smoke3d_skipz = CLAMP(smoke3d_skipz, 1, 10);
        update_smoke3d_frame_inc = 1;
        continue;
      }
      if(MatchINI(buffer, "SLICESKIP")==1){
        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%i %i %i %i", &slice_skip, &slice_skipx, &slice_skipy, &slice_skipz);
        if(slice_skip<1)slice_skip = 1;
        slice_skipx = slice_skip;
        slice_skipy = slice_skip;
        slice_skipz = slice_skip;
        continue;
      }
      if(MatchINI(buffer, "SMOKEALBEDO") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f", &global_scase.smoke_albedo);
        global_scase.smoke_albedo = CLAMP(global_scase.smoke_albedo, 0.0, 1.0);
        continue;
      }
      if(MatchINI(buffer, "SMOKEFIREPROP") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i", &use_opacity_depth_ini, &use_opacity_multiplier_ini);
        use_opacity_ini = 1;
        continue;
      }
      if(MatchINI(buffer, "SMOKEPROP")==1){
        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%f", &glui_smoke3d_extinct);
        glui_smoke3d_extinct_default = glui_smoke3d_extinct;
        continue;
      }
      if(MatchINI(buffer, "FIRECOLOR") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i", fire_color_int255, fire_color_int255+1, fire_color_int255+2);
        fire_color_int255[0] = CLAMP(fire_color_int255[0], 0, 255);
        fire_color_int255[1] = CLAMP(fire_color_int255[1], 0, 255);
        fire_color_int255[2] = CLAMP(fire_color_int255[2], 0, 255);
        continue;
      }
      if(MatchINI(buffer, "SMOKECOLOR") == 1){
        int f_gray_smoke = -1;

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i, %i", smoke_color_int255, smoke_color_int255+1, smoke_color_int255+2, &f_gray_smoke);
        smoke_color_int255[0] = CLAMP(smoke_color_int255[0], 0, 255);
        smoke_color_int255[1] = CLAMP(smoke_color_int255[1], 0, 255);
        smoke_color_int255[2] = CLAMP(smoke_color_int255[2], 0, 255);
        if(f_gray_smoke>=0){
          force_gray_smoke = f_gray_smoke;
          if(force_gray_smoke!=0)force_gray_smoke = 1;
        }
        continue;
      }
      if(MatchINI(buffer, "CO2COLOR") == 1){
         fgets(buffer, 255, stream);
         sscanf(buffer, " %i %i %i", co2_color_int255,co2_color_int255+1,co2_color_int255+2);
         co2_color_int255[0] = CLAMP(co2_color_int255[0], 0, 255);
         co2_color_int255[1] = CLAMP(co2_color_int255[1], 0, 255);
         co2_color_int255[2] = CLAMP(co2_color_int255[2], 0, 255);
         continue;
       }
      if(MatchINI(buffer, "HRRPUVCUTOFF")==1){
        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%f", &global_scase.global_hrrpuv_cutoff_default);
        global_scase.global_hrrpuv_cutoff = global_scase.global_hrrpuv_cutoff_default;
        continue;
      }
      if(MatchINI(buffer, "FDEPTH") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f %i %i", &fire_halfdepth,&co2_halfdepth,&emission_factor,&use_fire_alpha, &force_alpha_opaque);
        continue;
      }
      if(MatchINI(buffer, "VIEWTOURFROMPATH") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &viewtourfrompath);
        continue;
      }
      if(MatchINI(buffer, "TOURCONSTANTVEL") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &tour_constant_velocity);
        continue;
      }
      if(MatchINI(buffer, "VIEWALLTOURS") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &viewalltours);
        continue;
      }
      if(MatchINI(buffer, "SHOWTOURROUTE") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &edittour);
        continue;
      }
      if(MatchINI(buffer, "TIMEOFFSET") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f", &timeoffset);
        continue;
      }
      if(MatchINI(buffer, "SHOWPATHNODES") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i", &show_path_knots);
        continue;
      }
      if(MatchINI(buffer, "SHOWGEOMTERRAIN")==1){
        int nt;

        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%i %i %i %i %i",
          &nt, &terrain_show_geometry_surface, &terrain_show_geometry_outline, &terrain_show_geometry_points, &terrain_showonly_top);
        if(global_scase.terrain_texture_coll.terrain_textures!=NULL){
          for(i = 0; i<MIN(nt, global_scase.terrain_texture_coll.nterrain_textures); i++){
            texturedata *texti;

            texti = global_scase.terrain_texture_coll.terrain_textures+i;
            if(fgets(buffer, 255, stream)==NULL)break;
            sscanf(buffer, "%i ", &(texti->display));
          }
        }
        continue;
      }

      if(MatchINI(buffer, "SHOWIGNITION") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i", &vis_threshold, &vis_onlythreshold);
        continue;
      }
      if(MatchINI(buffer, "SHOWTHRESHOLD") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %f", &vis_threshold, &vis_onlythreshold, &temp_threshold);
        continue;
      }
      if(MatchINI(buffer, "TOUR_AVATAR") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        continue;
      }
      if(MatchINI(buffer, "TOURCIRCLE") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer,"%f %f %f %f %f %f %f %f",
          tour_circular_center+0, tour_circular_center+1, tour_circular_center+2,
          tour_circular_view+0, tour_circular_view+1, tour_circular_view+2,
          &tour_circular_radius, &tour_circular_angle0);
        continue;
      }

      /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ GCOLORBAR ++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

      if(MatchINI(buffer, "GCOLORBAR") == 1){
        colorbardata *cbi;
        int r1, g1, b1;
        int n;
        int ncolorbarini;

        fgets(buffer, 255, stream);
        ncolorbarini = 0;
        sscanf(buffer, "%i", &ncolorbarini);

        ncolorbarini = MAX(ncolorbarini, 0);
        InitDefaultColorbars(&colorbars, ncolorbarini, show_extreme_mindata,
                             rgb_below_min, show_extreme_maxdata,
                             rgb_above_max, &colorbarcopyinfo);
        UpdateColorbarDialogs();
        UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
        update_colorbar_dialog = 0;

        colorbars.ncolorbars = colorbars.ndefaultcolorbars + ncolorbarini;
        for(n = colorbars.ndefaultcolorbars; n<colorbars.ncolorbars; n++){
          char *cb_buffptr;

          cbi = colorbars.colorbarinfo + n;
          fgets(buffer, 255, stream);
          TrimBack(buffer);
          cb_buffptr = TrimFront(buffer);
          strcpy(cbi->menu_label, cb_buffptr);
          cbi->type = CB_USER;
          strcpy(cbi->colorbar_type, "user defined");
          cbi->interp = INTERP_LAB;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i %i", &cbi->nnodes, &cbi->nodehilight);
          if(cbi->nnodes<0)cbi->nnodes = 0;
          if(cbi->nodehilight<0 || cbi->nodehilight >= cbi->nnodes){
            cbi->nodehilight = 0;
          }

          for(i = 0; i<cbi->nnodes; i++){
            int icbar;
            int nn;

            fgets(buffer, 255, stream);
            r1 = -1; g1 = -1; b1 = -1;
            sscanf(buffer, "%i %i %i %i", &icbar, &r1, &g1, &b1);
            cbi->node_index[i] = icbar;
            nn = 3 * i;
            cbi->node_rgb[nn]     = r1;
            cbi->node_rgb[nn + 1] = g1;
            cbi->node_rgb[nn + 2] = b1;
          }
          RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
          UpdateColorbarDialogs();
        }

        continue;
      }

      if(MatchINI(buffer, "TOURCOLORS") == 1){
        float *col;

        col = tourcol_selectedpathline;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_selectedpathlineknots;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_selectedknot;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_pathline;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_pathknots;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_text;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        col = tourcol_avatar;
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", col, col + 1, col + 2);

        continue;
      }

      if(localfile!=1)continue;

// ---------------------------------------------------------------------------------------------------------
//   keywords below are 'local', only in the casename.ini file
// ---------------------------------------------------------------------------------------------------------

#define WINDROSE_PER_ROW 10
      if(MatchINI(buffer, "WINDROSESHOWHIDE")==1){
        int i1, i2, *vals, nrows;

        FREEMEMORY(windrosez_showhide);
        nwindrosez_showhide = 0;
        fgets(buffer, 255, stream);
        sscanf(buffer, " %i", &nwindrosez_showhide);
        if(nwindrosez_showhide>0){
          nrows = ((nwindrosez_showhide-1)/WINDROSE_PER_ROW+1);
          NewMemory((void **)&windrosez_showhide, nrows*WINDROSE_PER_ROW*sizeof(int));
          for(vals=windrosez_showhide,i=0;i<nrows;i++,vals+=WINDROSE_PER_ROW){
            int j;

            i1 = WINDROSE_PER_ROW*i;
            i2 = MIN(i1+WINDROSE_PER_ROW,nwindrosez_showhide);
            fgets(buffer, 255, stream);
            sscanf(buffer, " %i %i %i %i %i %i %i %i %i %i ",
              vals,vals+1,vals+2,vals+3,vals+4,vals+5,vals+6,vals+7,vals+8,vals+9);
            for(j=i1;j<i2;j++){
              windrosez_showhide[j] = CLAMP(vals[j-i1],0,1);
            }
          }
          update_windrose_showhide = 1;
        }
      }

      if(MatchINI(buffer, "PATCHDATAOUT") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %f %f %f %f %f %f %f", &output_patchdata,
          &patchout_tmin, &patchout_tmax,
          &patchout_xmin, &patchout_xmax,
          &patchout_ymin, &patchout_ymax,
          &patchout_zmin, &patchout_zmax
          );
        ONEORZERO(output_patchdata);
        continue;
      }

      /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ LABEL ++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

      if(MatchINI(buffer, "LABEL") == 1 ||
         (event_file_exists==0&&MatchINI(buffer, "TICKLABEL") == 1)){

        /*
        LABEL
        x y z r g b tstart tstop
        label

        */
        float *xyz, *rgbtemp, *tstart_stop;
        labeldata labeltemp, *labeli;
        int *useforegroundcolor;
        char *bufferptr;
        int *show_always;
        int ticklabel = 0;
        float *xyztick, *xyztickdir;
        int *showtick;

        if(MatchINI(buffer, "TICKLABEL") == 1)ticklabel = 1;

        labeli = &labeltemp;

        labeli->labeltype = TYPE_INI;
        xyz = labeli->xyz;
        rgbtemp = labeli->frgb;
        xyztick = labeli->tick_begin;
        xyztickdir = labeli->tick_direction;
        showtick = &labeli->show_tick;

        useforegroundcolor = &labeli->useforegroundcolor;
        tstart_stop = labeli->tstart_stop;
        show_always = &labeli->show_always;

        fgets(buffer, 255, stream);
        rgbtemp[0] = -1.0;
        rgbtemp[1] = -1.0;
        rgbtemp[2] = -1.0;
        rgbtemp[3] = 1.0;
        tstart_stop[0] = -1.0;
        tstart_stop[1] = -1.0;
        *useforegroundcolor = -1;
        *show_always = 1;

        sscanf(buffer, "%f %f %f %f %f %f %f %f %i %i",
          xyz, xyz + 1, xyz + 2,
          rgbtemp, rgbtemp + 1, rgbtemp + 2,
          tstart_stop, tstart_stop + 1, useforegroundcolor, show_always);

        if(ticklabel == 1){
          fgets(buffer, 255, stream);
          sscanf(buffer, "%f %f %f %f %f %f %i",
            xyztick, xyztick + 1, xyztick + 2,
            xyztickdir, xyztickdir + 1, xyztickdir + 2,
            showtick);
          *showtick = CLAMP(*showtick, 0, 1);
        }
        else{
          xyztick[0] = 0.0;
          xyztick[1] = 0.0;
          xyztick[2] = 0.0;
          xyztickdir[0] = 1.0;
          xyztickdir[1] = 0.0;
          xyztickdir[2] = 0.0;
          *showtick = 0;
        }
        *show_always = CLAMP(*show_always, 0, 1);
        *useforegroundcolor = CLAMP(*useforegroundcolor, -1, 1);
        if(*useforegroundcolor == -1){
          if(rgbtemp[0]<0.0 || rgbtemp[1]<0.0 || rgbtemp[2]<0.0 || rgbtemp[0]>1.0 || rgbtemp[1]>1.0 || rgbtemp[2]>1.0){
            *useforegroundcolor = 1;
          }
          else{
            *useforegroundcolor = 0;
          }
        }
        fgets(buffer, 255, stream);
        TrimBack(buffer);
        bufferptr = TrimFront(buffer);
        strcpy(labeli->name, bufferptr);
        LabelInsert(&global_scase.labelscoll, labeli);
        continue;
      }
      if(MatchINI(buffer, "VIEWTIMES") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %i", &global_scase.tourcoll.tour_tstart, &global_scase.tourcoll.tour_tstop, &global_scase.tourcoll.tour_ntimes);
        if(global_scase.tourcoll.tour_ntimes<2)global_scase.tourcoll.tour_ntimes = 2;
        ReallocTourMemory(&global_scase.tourcoll);
        continue;
      }
      if(MatchINI(buffer, "SHOOTER") == 1){
        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", shooter_xyz, shooter_xyz + 1, shooter_xyz + 2);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", shooter_dxyz, shooter_dxyz + 1, shooter_dxyz + 2);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", shooter_uvw, shooter_uvw + 1, shooter_uvw + 2);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f %f", &shooter_velmag, &shooter_veldir, &shooterpointsize);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%i %i %i %i %i", &shooter_fps, &shooter_vel_type, &shooter_nparts, &visShooter, &shooter_cont_update);

        if(fgets(buffer, 255, stream) == NULL)break;
        sscanf(buffer, "%f %f", &shooter_duration, &shooter_v_inf);
        continue;
      }
      if(MatchINI(buffer, "SCRIPTFILE") == 1){
        if(fgets(buffer2, 255, stream) == NULL)break;
        InsertScriptFile(RemoveComment(buffer2));
        updatemenu = 1;
        continue;
      }
      if(MatchINI(buffer, "SHOWDEVICES") == 1){
        sv_object *obj_typei;
        char *dev_label;
        int ndevices_ini;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i", &ndevices_ini, &object_outlines, &object_box);

        for(i = 0; i<global_scase.objectscoll.nobject_defs; i++){
          obj_typei = global_scase.objectscoll.object_defs[i];
          obj_typei->visible = 0;
        }
        for(i = 0; i<ndevices_ini; i++){
          fgets(buffer, 255, stream);
          TrimBack(buffer);
          dev_label = TrimFront(buffer);
          obj_typei = GetSmvObject(&global_scase.objectscoll, dev_label);
          if(obj_typei != NULL){
            obj_typei->visible = 1;
          }
        }
        continue;
      }
      if(MatchINI(buffer, "XYZCLIP") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &clip_mode);
        clip_mode = CLAMP(clip_mode, 0, CLIP_MAX);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %i %f", &clipinfo.clip_xmin, &clipinfo.xmin, &clipinfo.clip_xmax, &clipinfo.xmax);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %i %f", &clipinfo.clip_ymin, &clipinfo.ymin, &clipinfo.clip_ymax, &clipinfo.ymax);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %f %i %f", &clipinfo.clip_zmin, &clipinfo.zmin, &clipinfo.clip_zmax, &clipinfo.zmax);
        updateclipvals = 1;
        continue;
      }


      /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ TICKS ++++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      */

      if(MatchINI(buffer, "TICKS") == 1){
        global_scase.ntickinfo++;
        if(global_scase.tickinfo==NULL){
          NewMemory((void **)&global_scase.tickinfo, (global_scase.ntickinfo)*sizeof(tickdata));
        }
        else{
          ResizeMemory((void **)&global_scase.tickinfo, (global_scase.ntickinfo)*sizeof(tickdata));
        }

        {
          tickdata *ticki;
          float *begt, *endt;
          int *nbarst;
          float term;
          float length = 0.0;
          float *dxyz;
          float sum;

          ticki = global_scase.tickinfo + global_scase.ntickinfo - 1;
          begt = ticki->begin;
          endt = ticki->end;
          nbarst = &ticki->nbars;
          dxyz = ticki->dxyz;


          /*
          TICKS
          b1 b2 b3 e1 e2 e3 nb
          ticklength tickdir tickcolor (r g b) tickwidth
          */
          if(fgets(buffer, 255, stream) == NULL)break;
          *nbarst = 0;
          sscanf(buffer, "%f %f %f %f %f %f %i", begt, begt + 1, begt + 2, endt, endt + 1, endt + 2, nbarst);
          if(*nbarst<1)*nbarst = 1;
          if(fgets(buffer, 255, stream) == NULL)break;
          {
            float *rgbtemp;

            rgbtemp = ticki->rgb;
            VEC3EQCONS(rgbtemp, -1.0);
            ticki->width = -1.0;
            sscanf(buffer, "%f %i %f %f %f %f", &ticki->dlength, &ticki->dir, rgbtemp, rgbtemp + 1, rgbtemp + 2, &ticki->width);
            if(rgbtemp[0]<0.0 || rgbtemp[0]>1.0 ||
              rgbtemp[1]<0.0 || rgbtemp[1]>1.0 ||
              rgbtemp[2]<0.0 || rgbtemp[2]>1.0){
              ticki->useforegroundcolor = 1;
            }
            else{
              ticki->useforegroundcolor = 0;
            }
            if(ticki->width<0.0)ticki->width = 1.0;
          }
          for(i = 0; i<3; i++){
            term = endt[i] - begt[i];
            length += term*term;
          }
          if(length <= 0.0){
            endt[0] = begt[0] + 1.0;
            length = 1.0;
          }
          ticki->length = sqrt(length);
          VEC3EQCONS(dxyz, 0.0);
          switch(ticki->dir){
          case XLEFT:
          case XRIGHT:
            dxyz[0] = 1.0;
            break;
          case YBACK:
          case YFRONT:
            dxyz[1] = 1.0;
            break;
          case ZBOTTOM:
          case ZTOP:
            dxyz[2] = 1.0;
            break;
          default:
            assert(FFALSE);
            break;
          }
          if(ticki->dir<0){
            for(i = 0; i<3; i++){
              dxyz[i] = -dxyz[i];
            }
          }
          sum = dxyz[0] * dxyz[0] + dxyz[1] * dxyz[1] + dxyz[2] * dxyz[2];
          if(sum>0.0){
            sum = sqrt(sum);
            dxyz[0] *= (ticki->dlength / sum);
            dxyz[1] *= (ticki->dlength / sum);
            dxyz[2] *= (ticki->dlength / sum);
          }
        }
        continue;
      }
      if(MatchINI(buffer, "PROPINDEX") == 1){
        int nvals;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &nvals);
        for(i = 0; i<nvals; i++){
          propdata *propi;
          int ind, val;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i %i", &ind, &val);
          if(ind<0 || ind>global_scase.propcoll.npropinfo - 1)continue;
          propi = global_scase.propcoll.propinfo + ind;
          if(val<0 || val>propi->nsmokeview_ids - 1)continue;
          propi->smokeview_id = propi->smokeview_ids[val];
          propi->smv_object = propi->smv_objects[val];
        }
        for(i = 0; i<global_scase.npartclassinfo; i++){
          partclassdata *partclassi;

          partclassi = global_scase.partclassinfo + i;
          UpdatePartClassDepend(partclassi);

        }
        continue;
      }
      if(MatchINI(buffer, "partclassdataVIS") == 1){
        int ntemp;
        int j;

        fgets(buffer, 255, stream);
        sscanf(buffer, "%i", &ntemp);

        for(j = 0; j<ntemp; j++){
          partclassdata *partclassj;

          if(j>global_scase.npartclassinfo)break;

          partclassj = global_scase.partclassinfo + j;
          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &partclassj->vis_type);
        }
        continue;
      }

      if(MatchINI(buffer, "GSLICEPARMS") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i %i", &vis_gslice_data, &show_gslice_triangles, &show_gslice_triangulation, &show_gslice_normal);
        ONEORZERO(vis_gslice_data);
        ONEORZERO(show_gslice_triangles);
        ONEORZERO(show_gslice_triangulation);
        ONEORZERO(show_gslice_normal);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f %f", gslice_xyz, gslice_xyz + 1, gslice_xyz + 2);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%f %f", gslice_normal_azelev, gslice_normal_azelev + 1);
        update_gslice = 1;
        continue;
      }
      if(MatchINI(buffer, "GRIDPARMS") == 1){
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i", &visx_all, &visy_all, &visz_all);
        fgets(buffer, 255, stream);
        sscanf(buffer, "%i %i %i", &iplotx_all, &iploty_all, &iplotz_all);
        if(iplotx_all>nplotx_all - 1)iplotx_all = 0;
        if(iploty_all>nploty_all - 1)iploty_all = 0;
        if(iplotz_all>nplotz_all - 1)iplotz_all = 0;
        continue;
      }
      {
        int tours_flag;
        int nkeyframes;
        float key_time, key_xyz[3], key_view[3];
        float key_pause_time;

        tours_flag = 0;
    //TOUR7
    // index
    //   tourlabel
    //   nkeyframes avatar_index display
    //      time pause_time x y z
    //      vx vy vz
        if(have_tours==0&&MatchINI(buffer, "TOUR7") == 1)tours_flag = 1;
        if(tours_flag == 1){
          have_tour7 = 1;
          if(global_scase.tourcoll.ntourinfo > 0){
            for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = global_scase.tourcoll.tourinfo + i;
              FreeTour(touri);
            }
            FREEMEMORY(global_scase.tourcoll.tourinfo);
          }
          global_scase.tourcoll.ntourinfo = 0;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &global_scase.tourcoll.ntourinfo);
          global_scase.tourcoll.ntourinfo++;
          if(global_scase.tourcoll.ntourinfo > 0){
            if(NewMemory((void **)&global_scase.tourcoll.tourinfo, global_scase.tourcoll.ntourinfo*sizeof(tourdata)) == 0)return 2;
            for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = global_scase.tourcoll.tourinfo + i;
              touri->path_times = NULL;
              touri->display = 0;
            }
          }
          ReallocTourMemory(&global_scase.tourcoll);
          InitCircularTour(global_scase.tourcoll.tourinfo,ncircletournodes,INIT);
          {
            keyframe *thisframe, *addedframe;
            tourdata *touri;
            int glui_avatar_index_local;

            for(i = 1; i < global_scase.tourcoll.ntourinfo; i++){
              int j;

              touri = global_scase.tourcoll.tourinfo + i;
              InitTour(&global_scase.tourcoll, touri);
              fgets(buffer, 255, stream);
              TrimBack(buffer);
              strcpy(touri->label, TrimFront(buffer));

              fgets(buffer, 255, stream);
              glui_avatar_index_local = 0;
              sscanf(buffer, "%i %i %i", &nkeyframes, &glui_avatar_index_local, &touri->display2);
              glui_avatar_index_local = CLAMP(glui_avatar_index_local, 0, global_scase.objectscoll.navatar_types - 1);
              touri->glui_avatar_index = glui_avatar_index_local;
              if(touri->display2 != 1)touri->display2 = 0;
              touri->nkeyframes = nkeyframes;

              if(NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float)) == 0)return 2;
              if(NewMemory((void **)&touri->path_times, global_scase.tourcoll.tour_ntimes*sizeof(float)) == 0)return 2;
              thisframe = &touri->first_frame;
              for(j = 0; j < nkeyframes; j++){
                int key_set_time;

                key_set_time = 0;
                key_pause_time = 0.0;
                key_view[0] = 0.0;
                key_view[1] = 0.0;
                key_view[2] = 0.0;
                fgets(buffer, 255, stream);
                sscanf(buffer, "%f %f %f %f %f %i",
                  &key_time, &key_pause_time, key_xyz, key_xyz + 1, key_xyz + 2, &key_set_time);

                fgets(buffer, 255, stream);
                sscanf(buffer, "%f %f %f", key_view, key_view + 1, key_view + 2);
                addedframe = AddFrame(thisframe, key_time, key_pause_time, key_xyz, key_view, key_set_time);
                thisframe = addedframe;
                touri->keyframe_times[j] = key_time;
              }
            }
          }
          if(tours_flag == 1){
            for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = global_scase.tourcoll.tourinfo + i;
              touri->first_frame.next->prev = &touri->first_frame;
              touri->last_frame.prev->next = &touri->last_frame;
            }
            UpdateTourMenuLabels();
            CreateTourPaths();
            UpdateTimes();
            plotstate = GetPlotState(DYNAMIC_PLOTS);
            selectedtour_index = TOURINDEX_MANUAL;
            selected_frame = NULL;
            selected_tour = NULL;
            if(viewalltours == 1)TourMenu(MENU_TOUR_SHOWALL);
          }
          else{
            global_scase.tourcoll.ntourinfo = 0;
          }
          strcpy(buffer, "1.00000 1.00000 2.0000 0");
          TrimMZeros(buffer);
          continue;
        }
        if(MatchINI(buffer, "TOURINDEX")){
          if(fgets(buffer, 255, stream) == NULL)break;
          sscanf(buffer, "%i", &selectedtour_index_ini);
          if(selectedtour_index_ini < 0)selectedtour_index_ini = -1;
          update_selectedtour_index = 1;
        }
      }
    }
      {
        int tours_flag;
        int nkeyframes;
        float key_time, key_xyz[3], key_az_path, key_view[3], zzoom;
        float key_pause_time;
        int viewtype, uselocalspeed;

        tours_flag = 0;
        if(have_tour7==0&&MatchINI(buffer, "TOURS") == 1)tours_flag = 1;
        if(tours_flag == 1){
          have_tours = 1;
          if(global_scase.tourcoll.ntourinfo > 0){
            for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = global_scase.tourcoll.tourinfo + i;
              FreeTour(touri);
            }
            FREEMEMORY(global_scase.tourcoll.tourinfo);
          }
          global_scase.tourcoll.ntourinfo = 0;

          fgets(buffer, 255, stream);
          sscanf(buffer, "%i", &global_scase.tourcoll.ntourinfo);
          global_scase.tourcoll.ntourinfo++;
          if(global_scase.tourcoll.ntourinfo > 0){
            if(NewMemory((void **)&global_scase.tourcoll.tourinfo, global_scase.tourcoll.ntourinfo*sizeof(tourdata)) == 0)return 2;
            for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = global_scase.tourcoll.tourinfo + i;
              touri->path_times = NULL;
              touri->display = 0;
            }
          }
          ReallocTourMemory(&global_scase.tourcoll);
          InitCircularTour(global_scase.tourcoll.tourinfo,ncircletournodes,INIT);
          {
            keyframe *thisframe, *addedframe;
            tourdata *touri;
            int glui_avatar_index_local;

            for(i = 1; i < global_scase.tourcoll.ntourinfo; i++){
              int j;
              float dummy;
              int idummy;

              touri = global_scase.tourcoll.tourinfo + i;
              InitTour(&global_scase.tourcoll, touri);
              fgets(buffer, 255, stream);
              TrimBack(buffer);
              strcpy(touri->label, TrimFront(buffer));

              fgets(buffer, 255, stream);
              glui_avatar_index_local = 0;
              sscanf(buffer, "%i %i %f %i %i",
                &nkeyframes, &idummy, &dummy, &glui_avatar_index_local, &touri->display2);
              glui_avatar_index_local = CLAMP(glui_avatar_index_local, 0, global_scase.objectscoll.navatar_types - 1);
              touri->glui_avatar_index = glui_avatar_index_local;
              if(touri->display2 != 1)touri->display2 = 0;
              touri->nkeyframes = nkeyframes;

              if(NewMemory((void **)&touri->keyframe_times, nkeyframes*sizeof(float)) == 0)return 2;
              if(NewMemory((void **)&touri->path_times, global_scase.tourcoll.tour_ntimes*sizeof(float)) == 0)return 2;
              thisframe = &touri->first_frame;
              for(j = 0; j < nkeyframes; j++){
                key_view[0] = 0.0;
                key_view[1] = 0.0;
                key_view[2] = 0.0;
                key_az_path = 0.0;
                viewtype = 0;
                zzoom = 1.0;
                uselocalspeed = 0;
                fgets(buffer, 255, stream);

                sscanf(buffer, "%f %f %f %f %i",
                  &key_time,
                  key_xyz, key_xyz + 1, key_xyz + 2,
                  &viewtype);
                key_pause_time = 0.0;

                if(viewtype == 0){
                  float dummy3[3];

                  sscanf(buffer, "%f %f %f %f %i %f %f %f %f %f %f %f %i",
                    &key_time,
                    key_xyz, key_xyz + 1, key_xyz + 2,
                    &viewtype, &key_az_path, &dummy, &dummy,
                    dummy3, dummy3 + 1, dummy3 + 2,
                    &zzoom, &uselocalspeed);
                }
                else{
                  float dummy3[3];

                  sscanf(buffer, "%f %f %f %f %i %f %f %f %f %f %f %f %i",
                    &key_time,
                    key_xyz, key_xyz + 1, key_xyz + 2,
                    &viewtype, key_view, key_view + 1, key_view + 2,
                    dummy3, dummy3 + 1, dummy3 + 2,
                    &zzoom, &uselocalspeed);
                }
                if(zzoom<0.25)zzoom = 0.25;
                if(zzoom>4.00)zzoom = 4.0;
                addedframe = AddFrame(thisframe, key_time, key_pause_time, key_xyz, key_view, 0);
                thisframe = addedframe;
                touri->keyframe_times[j] = key_time;
              }
            }
          }
          if(tours_flag == 1){
            for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
              tourdata *touri;

              touri = global_scase.tourcoll.tourinfo + i;
              touri->first_frame.next->prev = &touri->first_frame;
              touri->last_frame.prev->next = &touri->last_frame;
            }
            UpdateTourMenuLabels();
            CreateTourPaths();
            UpdateTimes();
            plotstate = GetPlotState(DYNAMIC_PLOTS);
            selectedtour_index = TOURINDEX_MANUAL;
            selected_frame = NULL;
            selected_tour = NULL;
            if(viewalltours == 1)TourMenu(MENU_TOUR_SHOWALL);
          }
          else{
            global_scase.tourcoll.ntourinfo = 0;
          }
          strcpy(buffer, "1.00000 1.00000 2.0000 0");
          TrimMZeros(buffer);
          continue;
        }
        if(MatchINI(buffer, "TOURINDEX")){
          if(fgets(buffer, 255, stream) == NULL)break;
          sscanf(buffer, "%i", &selectedtour_index_ini);
          if(selectedtour_index_ini < 0)selectedtour_index_ini = -1;
          update_selectedtour_index = 1;
        }
      }
  }
  fclose(stream);
  return 0;
}

/* ------------------ ReadBinIni ------------------------ */

int ReadBinIni(void){
  char *smokeviewini = GetSystemIniPath();
  //*** read in config files if they exist
  // smokeview.ini ini in install directory
  int returnval = 0;
  if(smokeviewini!=NULL){
    returnval = ReadIni2(smokeviewini, 0);
  }
  FREEMEMORY(smokeviewini);
  return returnval;
}

/* ------------------ ReadIni ------------------------ */

int ReadIni(char *inifile){
  // There are 7 places to retrieve configuration file from:
  //
  //   1. A file within the same directory as the smokeview executable named
  //      "smokeview.ini".
  //   2. A file in the user's config directory named "smokeview.ini".
  //   3. A file in the current directory named "smokeview.ini".
  //   4. A file in the current directory named "${fdsprefix}.ini".
  //   5. A file in the scratch directory named "${fdsprefix}.ini".
  //   6. A file pointed to by SMOKEVIEW_CONFIG_PATH.
  //   7. A file pointed to be envar SMOKEVIEW_CONFIG.
  //
  // Last definition wins.

  global_scase.ntickinfo=global_scase.ntickinfo_smv;

  // Read "smokeview.ini" from bin dir
  char *global_ini = GetSystemIniPath();
  if(global_ini!=NULL){
    int returnval;

    returnval = ReadIni2(global_ini, 0);
    if(returnval==2)return 2;
    if(returnval == 0 && readini_output==1){
      if(verbose_output==1)PRINTF("- complete\n");
    }
    UpdateTerrainOptions();
  }
  FREEMEMORY(global_ini);

  // Read "${fdsprefix}.ini" from the current directory
  if(global_scase.paths.caseini_filename!=NULL){
    int returnval;
    char localdir[10];

    returnval = ReadIni2(global_scase.paths.caseini_filename, 1);

    // if directory is not writable then look for another ini file in the scratch directory
    strcpy(localdir, ".");
    if(Writable(localdir)==0){
      // Read "${fdsprefix}.ini" from the scratch directory
      char *scratch_ini_filename = GetUserConfigSubPath(global_scase.paths.caseini_filename);
      returnval = ReadIni2(scratch_ini_filename, 1);
      FREEMEMORY(scratch_ini_filename);
    }
    if(returnval==2)return 2;
    if(returnval == 0 && readini_output==1){
      if(verbose_output==1)PRINTF("- complete\n");
    }
  }

  // Read file specified in the SSF script
  if(inifile!=NULL){
    int return_code;

    return_code = ReadIni2(inifile,1);
    if(return_code == 0 && readini_output==1){
      if(verbose_output==1)PRINTF("- complete\n");
    }

    if(return_code==1||return_code==2){
      if(inifile==NULL){
        fprintf(stderr,"*** Error: Unable to read .ini file\n");
      }
      else{
        fprintf(stderr,"*** Error: Unable to read %s\n",inifile);
      }
    }
    if(return_code==2)return 2;

    UpdateRGBColors(colorbar_select_index);
  }

  if(use_graphics==1){
    if(showall_textures==1)TextureShowMenu(MENU_TEXTURE_SHOWALL);
  }
  if(colorbars.ncolorbars<=colorbars.ndefaultcolorbars){
    InitDefaultColorbars(&colorbars, 0, show_extreme_mindata, rgb_below_min,
                         show_extreme_maxdata, rgb_above_max, &colorbarcopyinfo);
    UpdateColorbarDialogs();
    UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
    update_colorbar_dialog = 0;
  }
  updatezoommenu=1;
  GetSliceParams2();
  return 0;
}

/* ------------------ OutputViewpoints ------------------------ */

void OutputViewpoints(FILE *fileout){
  float *eye, *az_elev, *mat;
  cameradata *ca;

  for(ca = camera_list_first.next; ca->next != NULL; ca = ca->next){
    if(strcmp(ca->name, "external") == 0)continue;
    if(ca->view_id<=1)continue;

    if(ca->quat_defined == 1){
      fprintf(fileout, "VIEWPOINT6\n");
    }
    else{
      fprintf(fileout, "VIEWPOINT5\n");
    }
    eye = ca->eye;
    az_elev = ca->az_elev;
    mat = modelview_identity;

    fprintf(fileout, " %i %i %i\n", ca->rotation_type, ca->rotation_index, ca->view_id);
    fprintf(fileout, " %f %f %f %f %i\n", eye[0], eye[1], eye[2], zoom, zoomindex);
    fprintf(fileout, " %f %f %f %i\n", ca->view_angle, ca->azimuth, ca->elevation, ca->projection_type);
    fprintf(fileout, " %f %f %f\n", ca->xcen, ca->ycen, ca->zcen);

    fprintf(fileout, " %f %f\n", az_elev[0], az_elev[1]);
    if(ca->quat_defined == 1){
      fprintf(fileout, " 1 %f %f %f %f\n", ca->quaternion[0], ca->quaternion[1], ca->quaternion[2], ca->quaternion[3]);
    }
    else{
      fprintf(fileout, " %f %f %f %f\n", mat[0], mat[1], mat[2], mat[3]);
      fprintf(fileout, " %f %f %f %f\n", mat[4], mat[5], mat[6], mat[7]);
      fprintf(fileout, " %f %f %f %f\n", mat[8], mat[9], mat[10], mat[11]);
      fprintf(fileout, " %f %f %f %f\n", mat[12], mat[13], mat[14], mat[15]);
    }
    fprintf(fileout, " %i %i %i %i %i %i %i\n",
      ca->clip_mode,
      ca->clip_xmin, ca->clip_ymin, ca->clip_zmin,
      ca->clip_xmax, ca->clip_ymax, ca->clip_zmax);
    fprintf(fileout, " %f %f %f %f %f %f\n",
      ca->xmin, ca->ymin, ca->zmin,
      ca->xmax, ca->ymax, ca->zmax);
    fprintf(fileout, " %s\n", ca->name);
  }
}

  /* ------------------ WriteIniLocal ------------------------ */


void WriteIniLocal(FILE *fileout){
  int i;
  int ndevice_vis = 0;
  sv_object *obj_typei;
  labeldata *thislabel;
  int startup_count;
  scriptfiledata *scriptfile;

  fprintf(fileout, "\n ------------ local ini settings ------------\n\n");

  fprintf(fileout, "BOUNDARYOFFSET\n");
  fprintf(fileout, " %f \n", boundaryoffset);
  fprintf(fileout, "DEVICEVECTORDIMENSIONS\n");
  fprintf(fileout, " %f %f %f %f\n", vector_baselength, vector_basediameter, vector_headlength, vector_headdiameter);
  fprintf(fileout, "DEVICEBOUNDS\n");
  fprintf(fileout, " %f %f\n", device_valmin, device_valmax);
  fprintf(fileout, "DEVICEORIENTATION\n");
  fprintf(fileout, " %i %f\n", show_device_orientation, orientation_scale);
  fprintf(fileout, "GRIDPARMS\n");
  fprintf(fileout, " %i %i %i\n", visx_all, visy_all, visz_all);
  fprintf(fileout, " %i %i %i\n", iplotx_all, iploty_all, iplotz_all);
  fprintf(fileout, "GSLICEPARMS\n");
  fprintf(fileout, " %i %i %i %i\n", vis_gslice_data, show_gslice_triangles, show_gslice_triangulation, show_gslice_normal);
  fprintf(fileout, " %f %f %f\n", gslice_xyz[0], gslice_xyz[1], gslice_xyz[2]);
  fprintf(fileout, " %f %f\n", gslice_normal_azelev[0], gslice_normal_azelev[1]);
  for(thislabel = global_scase.labelscoll.label_first_ptr->next; thislabel->next != NULL; thislabel = thislabel->next){
    labeldata *labeli;
    float *xyz, *rgbtemp, *tstart_stop;
    int *useforegroundcolor, *show_always;
    float *xyztick, *xyztickdir;
    int *showtick;

    labeli = thislabel;
    if(labeli->labeltype == TYPE_SMV)continue;
    xyz = labeli->xyz;
    rgbtemp = labeli->frgb;
    tstart_stop = labeli->tstart_stop;
    useforegroundcolor = &labeli->useforegroundcolor;
    show_always = &labeli->show_always;
    xyztick = labeli->tick_begin;
    xyztickdir = labeli->tick_direction;
    showtick = &labeli->show_tick;

    fprintf(fileout, "TICKLABEL\n");
    fprintf(fileout, " %f %f %f %f %f %f %f %f %i %i\n",
      xyz[0], xyz[1], xyz[2],
      rgbtemp[0], rgbtemp[1], rgbtemp[2],
      tstart_stop[0], tstart_stop[1],
      *useforegroundcolor, *show_always);
    fprintf(fileout, " %f %f %f %f %f %f %i\n",
      xyztick[0], xyztick[1], xyztick[2],
      xyztickdir[0], xyztickdir[1], xyztickdir[2],
      *showtick);
    fprintf(fileout, " %s\n", labeli->name);
  }
  fprintf(fileout, "LOADFILESATSTARTUP\n");
  fprintf(fileout, " %i\n", loadfiles_at_startup);
  fprintf(fileout, "MSCALE\n");
  fprintf(fileout, " %f %f %f\n", mscale[0], mscale[1], mscale[2]);
  PutStartupSmoke3D(fileout);
  if(npart5prop > 0){
    fprintf(fileout, "PART5PROPDISP\n");
    for(i = 0; i < npart5prop; i++){
      partpropdata *propi;
      int j;

      propi = part5propinfo + i;
      fprintf(fileout, " ");
      for(j = 0; j < global_scase.npartclassinfo; j++){
        fprintf(fileout, " %i ", (int)propi->class_vis[j]);
      }
      fprintf(fileout, "\n");
    }
    fprintf(fileout, "PART5COLOR\n");
    for(i = 0; i < npart5prop; i++){
      partpropdata *propi;

      propi = part5propinfo + i;
      if(propi->display == 1){
        fprintf(fileout, " %i\n", i);
        break;
      }
    }

  }

  if(global_scase.npartclassinfo > 0){
    int j;

    fprintf(fileout, "partclassdataVIS\n");
    fprintf(fileout, " %i\n", global_scase.npartclassinfo);
    for(j = 0; j<global_scase.npartclassinfo; j++){
      partclassdata *partclassj;

      partclassj = global_scase.partclassinfo + j;
      fprintf(fileout, " %i\n", partclassj->vis_type);
    }
  }
  fprintf(fileout, "PARTSKIP\n");
  fprintf(fileout, " %i\n", partdrawskip);

  if(global_scase.propcoll.npropinfo>0){
    fprintf(fileout, "PROPINDEX\n");
    fprintf(fileout, " %i\n", global_scase.propcoll.npropinfo);
    for(i = 0; i < global_scase.propcoll.npropinfo; i++){
      propdata *propi;
      int offset;
      int jj;

      propi = global_scase.propcoll.propinfo + i;
      offset = -1;
      for(jj = 0; jj < propi->nsmokeview_ids; jj++){
        if(strcmp(propi->smokeview_id, propi->smokeview_ids[jj]) == 0){
          offset = jj;
          break;
        }
      }
      fprintf(fileout, " %i %i\n", i, offset);
    }
  }

  for(scriptfile = first_scriptfile.next; scriptfile->next != NULL; scriptfile = scriptfile->next){
    char *file;

    file = scriptfile->file;
    if(file != NULL){
      fprintf(fileout, "SCRIPTFILE\n");
      fprintf(fileout, " %s\n", file);
    }
  }
  fprintf(fileout, "SHOOTER\n");
  fprintf(fileout, " %f %f %f\n", shooter_xyz[0], shooter_xyz[1], shooter_xyz[2]);
  fprintf(fileout, " %f %f %f\n", shooter_dxyz[0], shooter_dxyz[1], shooter_dxyz[2]);
  fprintf(fileout, " %f %f %f\n", shooter_uvw[0], shooter_uvw[1], shooter_uvw[2]);
  fprintf(fileout, " %f %f %f\n", shooter_velmag, shooter_veldir, shooterpointsize);
  fprintf(fileout, " %i %i %i %i %i\n", shooter_fps, shooter_vel_type, shooter_nparts, visShooter, shooter_cont_update);
  fprintf(fileout, " %f %f\n", shooter_duration, shooter_v_inf);

  for(i = 0; i < global_scase.objectscoll.nobject_defs; i++){
    obj_typei = global_scase.objectscoll.object_defs[i];
    if(obj_typei->used == 1 && obj_typei->visible == 1){
      ndevice_vis++;
    }
  }
  fprintf(fileout, "SHOWDEVICES\n");
  fprintf(fileout, " %i %i %i\n", ndevice_vis, object_outlines, object_box);
  for(i = 0; i < global_scase.objectscoll.nobject_defs; i++){
    obj_typei = global_scase.objectscoll.object_defs[i];
    if(obj_typei->used == 1 && obj_typei->visible == 1){
      fprintf(fileout, " %s\n", obj_typei->label);
    }
  }
  fprintf(fileout, "SHOWDEVICEPLOTS\n");
  fprintf(fileout, " %i %i %i %f %f %f %f %f %f %f\n",
          vis_device_plot, show_plot2d_xlabels, show_plot2d_ylabels, plot2d_size_factor, plot2d_line_width, plot2d_point_size,
          plot2d_xyz_offset[0], plot2d_xyz_offset[1], plot2d_xyz_offset[2], plot2d_font_spacing
  );
  fprintf(fileout, "SHOWGENPLOTXLABEL\n");
  fprintf(fileout, " %i, %f\n", plot2d_show_xaxis_labels, plot2d_xaxis_position);
  fprintf(fileout, "%s\n", plot2d_xaxis_label);

  fprintf(fileout, "SHOWGENPLOTS\n");
  fprintf(fileout, " %i\n", nplot2dinfo);
  fprintf(fileout, " %i %i %i %i %i %i %f %i %i\n",
         plot2d_show_plot_title, plot2d_show_curve_labels, plot2d_show_curve_values,
         plot2d_show_xaxis_bounds, plot2d_show_yaxis_bounds, idevice_add, plot2d_time_average,
         plot2d_show_yaxis_units, plot2d_show_plots);
  for(i=0; i<nplot2dinfo; i++){
    plot2ddata *plot2di;
    int j;

    plot2di = plot2dinfo + i;
    fprintf(fileout, " %s\n", plot2di->plot_label);
    fprintf(fileout, " %f %f %f %i %i %i\n", plot2di->xyz[0], plot2di->xyz[1], plot2di->xyz[2], plot2di->show, plot2di->ncurves, plot2di->mult_devc);
    fprintf(fileout, " %f %i %f %i %f %i %f %i\n",
            plot2di->valmin[0], plot2di->use_valmin[0], plot2di->valmax[0], plot2di->use_valmax[0],
            plot2di->valmin[1], plot2di->use_valmin[1], plot2di->valmax[1], plot2di->use_valmax[1]
            );
    for(j = 0; j < plot2di->ncurves; j++){
      int *color;
      float linewidth1;
      int file_index, col_index;
      curvedata *curve;
      float factor;
      int apply_factor;
      int use_foreground_color;

      file_index        = plot2di->curve[j].csv_file_index;
      col_index         = plot2di->curve[j].csv_col_index;
      curve             = plot2di->curve+j;
      color             = curve->color;
      linewidth1        = curve->linewidth;
      factor            = curve->curve_factor;
      apply_factor      = curve->apply_curve_factor;
      use_foreground_color  = curve->use_foreground_color;
      fprintf(fileout, " %i %i %i %i %i %f %f %i %i\n", file_index, col_index, color[0], color[1], color[2], linewidth1, factor, apply_factor, use_foreground_color);
    };
  }
  fprintf(fileout, "GENPLOTLABELS\n");
  fprintf(fileout, " %i\n", nplot2dinfo);
  for(i = 0; i<nplot2dinfo; i++){
    plot2ddata *plot2di;
    int j;

    plot2di = plot2dinfo+i;
    fprintf(fileout, " %i\n", plot2di->ncurves);
    for(j = 0; j<plot2di->ncurves; j++){
      curvedata *curve;

      curve = plot2di->curve+j;
      fprintf(fileout, " %i %i\n", i, j);
      fprintf(fileout, " %s\n", curve->scaled_label);
      fprintf(fileout, " %s\n", curve->scaled_unit);
    }
  }

  fprintf(fileout, "SHOWDEVICEVALS\n");
  fprintf(fileout, " %i %i %i %i %i %i %i %i %i\n", showdevice_val, showvdevice_val, devicetypes_index, colordevice_val, vectortype, viswindrose, showdevice_type,showdevice_unit,showdevice_id);
  fprintf(fileout, "SHOWHRRPLOT\n");
  fprintf(fileout, " %i %i %f %f %i\n", glui_hrr, hoc_hrr, global_scase.fuel_hoc, plot2d_size_factor, vis_hrr_plot);
  fprintf(fileout, "SHOWMISSINGOBJECTS\n");
  fprintf(fileout, " %i\n", show_missing_objects);
  fprintf(fileout, "SHOWSLICEPLOT\n");
  fprintf(fileout, " %f %f %f %f %i %i %f %f %f %i %i\n",
                    slice_xyz[0], slice_xyz[1], slice_xyz[2],
                    plot2d_size_factor, vis_slice_plot, slice_plot_bound_option,
                    slice_dxyz[0], slice_dxyz[1], slice_dxyz[2], average_plot2d_slice_region, show_plot2d_slice_position
                    );
  
  for(i = global_scase.ntickinfo_smv; i < global_scase.ntickinfo; i++){
    float *begt;
    float *endt;
    float *rgbtemp;
    tickdata *ticki;

    ticki = global_scase.tickinfo + i;
    begt = ticki->begin;
    endt = ticki->end;
    rgbtemp = ticki->rgb;

    fprintf(fileout, "TICKS\n");
    fprintf(fileout, " %f %f %f %f %f %f %i\n", begt[0], begt[1], begt[2], endt[0], endt[1], endt[2], ticki->nbars);
    fprintf(fileout, " %f %i %f %f %f %f\n", ticki->dlength, ticki->dir, rgbtemp[0], rgbtemp[1], rgbtemp[2], ticki->width);
  }
  fprintf(fileout, "SHOWGEOMTERRAIN\n");
  fprintf(fileout, "%i %i %i %i %i\n",
    global_scase.terrain_texture_coll.nterrain_textures, terrain_show_geometry_surface, terrain_show_geometry_outline, terrain_show_geometry_points, terrain_showonly_top);
  for(i = 0; i<global_scase.terrain_texture_coll.nterrain_textures; i++){
    texturedata *texti;

    texti = global_scase.terrain_texture_coll.terrain_textures+i;
    fprintf(fileout, "%i\n", texti->display);
  }

  fprintf(fileout, "TOURCIRCLE\n");
  fprintf(fileout, "%f %f %f %f %f %f %f %f\n",
    tour_circular_center[0],
    tour_circular_center[1], tour_circular_center[2],
    tour_circular_view[0], tour_circular_view[1], tour_circular_view[2],
    tour_circular_radius, tour_circular_angle0);
  fprintf(fileout, "TOURINDEX\n");
  fprintf(fileout, " %i\n", selectedtour_index);
  startup_count = 0;
  for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
    tourdata *touri;

    touri = global_scase.tourcoll.tourinfo + i;
    if(touri->startup == 1)startup_count++;
  }
  if(startup_count < global_scase.tourcoll.ntourinfo){
    //TOUR7
    // index
    //   tourlabel
    //   nkeyframes avatar_index display
    //      time pause_time x y z
    //      vx vy vz
    fprintf(fileout, "TOUR7\n");
    fprintf(fileout, " %i\n", global_scase.tourcoll.ntourinfo - startup_count);
    for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
      tourdata *touri;
      keyframe *framei;
      int j;

      touri =global_scase.tourcoll.tourinfo + i;
      if(touri->startup == 1)continue;

      TrimBack(touri->label);
      fprintf(fileout, "  %s\n", touri->label);
      fprintf(fileout, "  %i %i %i\n", touri->nkeyframes, touri->glui_avatar_index, touri->display);

      framei = &touri->first_frame;
      for(j = 0; j<touri->nkeyframes; j++){
        float xyz_smv[3], view_smv[3];

        framei = framei->next;
        SMV2FDS_XYZ(xyz_smv, framei->xyz_smv);
        fprintf(fileout, "    %f %f %f %f %f %i\n", framei->time, framei->pause_time, xyz_smv[0], xyz_smv[1], xyz_smv[2], framei->set_tour_time);

        SMV2FDS_XYZ(view_smv, framei->view_smv);
        fprintf(fileout, "    %f %f %f\n", view_smv[0], view_smv[1], view_smv[2]);
      }
    }
  }
  fprintf(fileout, "USERTICKS\n");
  fprintf(fileout, " %i %i %i %i %i %i %f %i\n", visUSERticks, auto_user_tick_placement, user_tick_sub,
    user_tick_show_x, user_tick_show_y, user_tick_show_z, user_tick_direction, ntick_decimals);
  fprintf(fileout, " %f %f %f\n", user_tick_origin[0], user_tick_origin[1], user_tick_origin[2]);
  fprintf(fileout, " %f %f %f\n", user_tick_min[0], user_tick_min[1], user_tick_min[2]);
  fprintf(fileout, " %f %f %f\n", user_tick_max[0], user_tick_max[1], user_tick_max[2]);
  fprintf(fileout, " %f %f %f\n", user_tick_step[0], user_tick_step[1], user_tick_step[2]);
  fprintf(fileout, " %i %i %i\n", user_tick_show_x, user_tick_show_y, user_tick_show_z);
  fprintf(fileout, "XYZCLIP\n");
  fprintf(fileout, " %i\n", clip_mode);
  fprintf(fileout, " %i %f %i %f\n", clipinfo.clip_xmin, clipinfo.xmin, clipinfo.clip_xmax, clipinfo.xmax);
  fprintf(fileout, " %i %f %i %f\n", clipinfo.clip_ymin, clipinfo.ymin, clipinfo.clip_ymax, clipinfo.ymax);
  fprintf(fileout, " %i %f %i %f\n", clipinfo.clip_zmin, clipinfo.zmin, clipinfo.clip_zmax, clipinfo.zmax);

  fprintf(fileout, "\n *** TIME/DATA BOUNDS ***\n");
  fprintf(fileout, "  (0/1 min max skip (1=set, 0=unset)\n\n");

  for(i = 0; i<npatchbounds_cpp; i++){
    cpp_boundsdata *boundi;

    boundi = patchbounds_cpp+i;
    fprintf(fileout, "C_BOUNDARY\n");
    fprintf(fileout, " %i %f %i %f %s\n", boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax, boundi->label);
  }
  if(niso_bounds > 0){
    for(i = 0; i < niso_bounds; i++){
      fprintf(fileout, "C_ISO\n");
      fprintf(fileout, " %i %f %i %f %s\n",
        isobounds[i].setchopmin, isobounds[i].chopmin,
        isobounds[i].setchopmax, isobounds[i].chopmax,
        isobounds[i].label->shortlabel
        );
    }
  }
  for(i = 0; i<npartbounds_cpp; i++){
    cpp_boundsdata *boundi;

    boundi = partbounds_cpp+i;
    fprintf(fileout, "C_PARTICLES\n");
    fprintf(fileout, " %i %f %i %f %s\n", boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax, boundi->label);
  }

  fprintf(fileout, "C_PLOT3D\n");
  fprintf(fileout, " %i\n", nplot3dbounds_cpp);
  for(i = 0; i<nplot3dbounds_cpp; i++){
    cpp_boundsdata *boundi;

    boundi = plot3dbounds_cpp+i;
    fprintf(fileout, " %i %i %f %i %f\n", i+1, boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax);
  }

  if(nslicebounds_cpp>0){
    for(i = 0; i<nslicebounds_cpp; i++){
      cpp_boundsdata *boundi;

      boundi = slicebounds_cpp+i;
      fprintf(fileout, "C_SLICE\n");
      fprintf(fileout, " %i %f %i %f %s\n", boundi->set_chopmin, boundi->chopmin, boundi->set_chopmax, boundi->chopmax, boundi->label);
      fprintf(fileout, "C_SLICE_HIDE\n");
      fprintf(fileout, " %i %s\n", boundi->chop_hide, boundi->label);
    }
  }
  fprintf(fileout, "CACHE_DATA\n");
  fprintf(fileout, " %i %i %i %i \n", cache_boundary_data, cache_part_data, cache_plot3d_data, cache_slice_data);
  fprintf(fileout, "LOADMESH\n");
  fprintf(fileout, " %i %i\n", show_intersection_box, show_intersected_meshes);
  fprintf(fileout, " %i %f %i %f\n", use_meshclip[0], meshclip[0], use_meshclip[1], meshclip[1]);
  fprintf(fileout, " %i %f %i %f\n", use_meshclip[2], meshclip[2], use_meshclip[3], meshclip[3]);
  fprintf(fileout, " %i %f %i %f\n", use_meshclip[4], meshclip[4], use_meshclip[5], meshclip[5]);
  fprintf(fileout, "PATCHDATAOUT\n");
  fprintf(fileout, " %i %f %f %f %f %f %f %f %f\n", output_patchdata,
    patchout_tmin, patchout_tmax,
    patchout_xmin, patchout_xmax,
    patchout_ymin, patchout_ymax,
    patchout_zmin, patchout_zmax
    );
  fprintf(fileout, "TIMEOFFSET\n");
  fprintf(fileout, " %f\n", timeoffset);
  fprintf(fileout, "TLOAD\n");
  fprintf(fileout, " %i %f %i %f %i %i\n", use_tload_begin, global_scase.tload_begin, use_tload_end, global_scase.tload_end, use_tload_skip, tload_skip);
  for(i = 0; i < global_scase.npatchinfo; i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->firstshort_patch == 1){
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = patchi->label.shortlabel;

      GLUIGetOnlyMinMax(BOUND_PATCH, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, "V2_BOUNDARY\n");
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  if(niso_bounds > 0){
    for(i = 0; i < niso_bounds; i++){
      fprintf(fileout, "V_ISO\n");
      fprintf(fileout, " %i %f %i %f %s\n",
        isobounds[i].dlg_setvalmin, isobounds[i].dlg_valmin,
        isobounds[i].dlg_setvalmax, isobounds[i].dlg_valmax,
        isobounds[i].label->shortlabel
        );
    }
  }
  if(npart5prop > 0){
    for(i = 0; i < npart5prop; i++){
      partpropdata *propi;

      propi = part5propinfo + i;
      fprintf(fileout, "V2_PARTICLES\n");
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = propi->label->shortlabel;

      GLUIGetOnlyMinMax(BOUND_PART, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  {
    int n3d;

    n3d = MAXPLOT3DVARS;
    if(n3d<numplot3dvars)n3d = numplot3dvars;
    if(n3d>MAXPLOT3DVARS)n3d = MAXPLOT3DVARS;
    if(global_scase.plot3dinfo!=NULL){
      fprintf(fileout, "V2_PLOT3D\n");
      fprintf(fileout, " %i\n", n3d);
    }
    for(i = 0; i < n3d; i++){
    if(global_scase.plot3dinfo!=NULL){
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = global_scase.plot3dinfo[0].label[i].shortlabel;
      GLUIGetOnlyMinMax(BOUND_PLOT3D, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %i %f %i %f %s\n", i+1, set_valmin, valmin, set_valmax, valmax, label);
    }
    }
  }
  if(nhvacductbounds > 0){
    for(i = 0; i < nhvacductbounds; i++){
      fprintf(fileout, "V2_HVACDUCT\n");
      int set_valmin = 0, set_valmax = 0;
      float valmin = 1.0, valmax = 0.0;
      char *label;

      label = hvacductbounds[i].label->shortlabel;
      GLUIGetOnlyMinMax(BOUND_HVACDUCT, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  if(nhvacnodebounds > 0){
    for(i = 0; i < nhvacnodebounds; i++){
      fprintf(fileout, "V2_HVACNODE\n");
      int set_valmin = 0, set_valmax = 0;
      float valmin = 1.0, valmax = 0.0;
      char *label;

      label = hvacnodebounds[i].label->shortlabel;
      GLUIGetOnlyMinMax(BOUND_HVACNODE, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s\n", set_valmin, valmin, set_valmax, valmax, label);
    }
  }
  if(nslicebounds > 0){
    for(i = 0; i < nslicebounds; i++){
      fprintf(fileout, "V2_SLICE\n");
      int set_valmin=0, set_valmax=0;
      float valmin=1.0, valmax=0.0;
      char *label;

      label = slicebounds[i].label->shortlabel;
      GLUIGetOnlyMinMax(BOUND_SLICE, label, &set_valmin, &valmin, &set_valmax, &valmax);
      fprintf(fileout, " %i %f %i %f %s : %f %f %i\n", set_valmin, valmin, set_valmax, valmax, label,
        slicebounds[i].line_contour_min, slicebounds[i].line_contour_max, slicebounds[i].line_contour_num
        );
    }
  }
  fprintf(fileout, "V_TARGET\n");
  fprintf(fileout, " %i %f %i %f\n", settargetmin, targetmin, settargetmax, targetmax);
  if(global_scase.nzoneinfo > 0){
    fprintf(fileout, "V_ZONE\n");
    fprintf(fileout, " %i %f %i %f\n", setzonemin, zoneusermin, setzonemax, zoneusermax);
    fprintf(fileout, "ZONEVIEW\n");
    fprintf(fileout, " %f\n", zone_hvac_diam);
  }
}

  /* ------------------ WriteIni ------------------------ */

void WriteIni(int flag,char *filename){
  FILE *fileout=NULL;
  int i;
  char *outfilename=NULL, *outfiledir=NULL;
  char *smokeviewini_filename = GetSystemIniPath();
  char *smokeview_scratchdir = GetUserConfigDir();

  switch(flag){
  case GLOBAL_INI:
    if(smokeviewini_filename!=NULL)fileout=fopen(smokeviewini_filename,"w");
    outfilename= smokeviewini_filename;
    break;
  case STDOUT_INI:
    fileout=stdout;
    break;
  case SCRIPT_INI:
    fileout=fopen(filename,"w");
    outfilename=filename;
    break;
  case LOCAL_INI:
    fileout=fopen(global_scase.paths.caseini_filename,"w");
    if(fileout==NULL&&smokeview_scratchdir!=NULL){
      fileout = fopen_indir(smokeview_scratchdir, global_scase.paths.caseini_filename, "w");
      outfiledir = smokeview_scratchdir;
     }
    outfilename=global_scase.paths.caseini_filename;
    break;
  default:
    assert(FFALSE);
    break;
  }
  if(flag==SCRIPT_INI)flag=LOCAL_INI;
  if(fileout==NULL){
    if(outfilename!=NULL){
      fprintf(stderr,"*** Error: unable to open %s for writing ",outfilename);
      return;
    }
    else{
      fprintf(stderr,"*** Error: unable to open ini file for output ");
    }
    if(outfiledir==NULL){
      printf("in current directory\n");
    }
    else{
      printf("in directory %s\n", outfiledir);
    }
  }
  FREEMEMORY(smokeviewini_filename);

  fprintf(fileout,"# NIST Smokeview configuration file, Release %s\n\n",__DATE__);
  fprintf(fileout, "\n ------------ global ini settings ------------\n\n");

  fprintf(fileout,"   *** COLOR/LIGHTING ***\n\n");

  fprintf(fileout, "BACKGROUNDCOLOR\n");
  fprintf(fileout, " %f %f %f\n", backgroundbasecolor[0], backgroundbasecolor[1], backgroundbasecolor[2]);
  fprintf(fileout, "BLOCKCOLOR\n");
  fprintf(fileout, " %f %f %f\n", block_ambient2[0], block_ambient2[1], block_ambient2[2]);
  fprintf(fileout, "BLOCKSHININESS\n");
  fprintf(fileout, " %f\n", block_shininess);
  fprintf(fileout, "BLOCKSPECULAR\n");
  fprintf(fileout, " %f %f %f\n", block_specular2[0], block_specular2[1], block_specular2[2]);
  fprintf(fileout, "BOUNDCOLOR\n");
  fprintf(fileout, " %f %f %f\n", boundcolor[0], boundcolor[1], boundcolor[2]);
  fprintf(fileout, "COLORBAR\n");
  {
    int usetexturebar = 1; //for older smokeviews
    fprintf(fileout, " %i %i %i %i\n", global_scase.nrgb, usetexturebar, colorbar_select_index, colorbar_selection_width);
  }
  for(i=0;i<global_scase.nrgb;i++){
    fprintf(fileout," %f %f %f\n",global_scase.rgb[i][0],global_scase.rgb[i][1],global_scase.rgb[i][2]);
  }
  fprintf(fileout,"COLOR2BAR\n");
  fprintf(fileout," %i\n",8);
  fprintf(fileout," %f %f %f :white  \n",rgb2[0][0],rgb2[0][1],rgb2[0][2]);
  fprintf(fileout," %f %f %f :yellow \n",rgb2[1][0],rgb2[1][1],rgb2[1][2]);
  fprintf(fileout," %f %f %f :blue   \n",rgb2[2][0],rgb2[2][1],rgb2[2][2]);
  fprintf(fileout," %f %f %f :red    \n",rgb2[3][0],rgb2[3][1],rgb2[3][2]);
  fprintf(fileout," %f %f %f :green  \n",rgb2[4][0],rgb2[4][1],rgb2[4][2]);
  fprintf(fileout," %f %f %f :magenta\n",rgb2[5][0],rgb2[5][1],rgb2[5][2]);
  fprintf(fileout," %f %f %f :cyan   \n",rgb2[6][0],rgb2[6][1],rgb2[6][2]);
  fprintf(fileout," %f %f %f :black  \n",rgb2[7][0],rgb2[7][1],rgb2[7][2]);
  fprintf(fileout, "COLORBAR_FLIP\n");
  fprintf(fileout, " %i %i\n", colorbar_flip,colorbar_autoflip);
  fprintf(fileout, "COLORBAR_SPLIT\n");
  fprintf(fileout, " %i %i %i %i %i %i\n", colorsplit[0], colorsplit[1], colorsplit[2], colorsplit[3], colorsplit[4], colorsplit[5]);
  fprintf(fileout, " %i %i %i %i %i %i\n", colorsplit[6], colorsplit[7], colorsplit[8], colorsplit[9], colorsplit[10], colorsplit[11]);
  fprintf(fileout, " %f %f %f\n", splitvals[0], splitvals[1], splitvals[2]);
  fprintf(fileout,"CO2COLOR\n");
  fprintf(fileout," %i %i %i\n", co2_color_int255[0],co2_color_int255[1],co2_color_int255[2]);
  fprintf(fileout, "DIRECTIONCOLOR\n");
  fprintf(fileout, " %f %f %f\n", direction_color[0], direction_color[1], direction_color[2]);
  fprintf(fileout, "FLIP\n");
  fprintf(fileout, " %i\n", background_flip);
  fprintf(fileout, "FOREGROUNDCOLOR\n");
  fprintf(fileout, " %f %f %f\n", foregroundbasecolor[0], foregroundbasecolor[1], foregroundbasecolor[2]);
  fprintf(fileout, "GEOMSELECTCOLOR\n");
  fprintf(fileout, " %u %u %u\n",  geom_vertex1_rgb[0],  geom_vertex1_rgb[1],  geom_vertex1_rgb[2]);
  fprintf(fileout, " %u %u %u\n",  geom_vertex2_rgb[0],  geom_vertex2_rgb[1],  geom_vertex2_rgb[2]);
  fprintf(fileout, " %u %u %u\n", geom_triangle_rgb[0], geom_triangle_rgb[1], geom_triangle_rgb[2]);
  fprintf(fileout, "HEATOFFCOLOR\n");
  fprintf(fileout, " %f %f %f\n", heatoffcolor[0], heatoffcolor[1], heatoffcolor[2]);
  fprintf(fileout, "HEATONCOLOR\n");
  fprintf(fileout, " %f %f %f\n", heatoncolor[0], heatoncolor[1], heatoncolor[2]);
  fprintf(fileout, "ISOCOLORS\n");
  fprintf(fileout," %f %f %i %i : shininess, default opaqueness\n",iso_shininess, iso_transparency, iso_transparency_option, iso_opacity_change);
  fprintf(fileout," %f %f %f : specular\n",iso_specular[0],iso_specular[1],iso_specular[2]);
  fprintf(fileout," %i : number of levels\n",MAX_ISO_COLORS);
  for(i=0;i<MAX_ISO_COLORS;i++){
    fprintf(fileout, " %f %f %f %f", iso_colors[4*i], iso_colors[4*i+1], iso_colors[4*i+2], iso_colors[4*i+3]);
    if(i==0)fprintf(fileout, " : red, green, blue, alpha (opaqueness)");
    fprintf(fileout, "\n");
  }
  if(ncolortableinfo>0){
    char percen[2];

    strcpy(percen,"%");
    fprintf(fileout, "COLORTABLE\n");
    fprintf(fileout, " %i \n", ncolortableinfo);

    for(i = 0; i<ncolortableinfo; i++){
      colortabledata *rgbi;

      rgbi = colortableinfo+i;
      fprintf(fileout, " %i %i %i %i %s %s\n",
        rgbi->color[0], rgbi->color[1], rgbi->color[2], rgbi->color[3], percen, rgbi->label);
    }
  }
  fprintf(fileout, "LIGHTING\n");
  fprintf(fileout, " %i\n", use_lighting);
  fprintf(fileout, "LIGHTFACES\n");
  fprintf(fileout, " %i\n", light_faces);
  fprintf(fileout, "LIGHTANGLES0\n");
  fprintf(fileout, " %f %f %i\n", glui_light_az0, glui_light_elev0, use_light0);
  fprintf(fileout, "LIGHTANGLES1\n");
  fprintf(fileout, " %f %f %i\n", glui_light_az1, glui_light_elev1, use_light1);
  fprintf(fileout, "LIGHTPROP\n");
  fprintf(fileout, " %f %f %f\n", ambientlight[0], ambientlight[1], ambientlight[2]);
  fprintf(fileout, " %f %f %f\n", diffuselight[0], diffuselight[1], diffuselight[2]);
  fprintf(fileout, "OUTLINECOLOR\n");
  fprintf(fileout, " %i %i %i\n", glui_outlinecolor[0], glui_outlinecolor[1], glui_outlinecolor[2]);
  fprintf(fileout, "SENSORCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sensorcolor[0], sensorcolor[1], sensorcolor[2]);
  fprintf(fileout, "SENSORNORMCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sensornormcolor[0], sensornormcolor[1], sensornormcolor[2]);
  fprintf(fileout, "SETBW\n");
  fprintf(fileout, " %i %i\n", setbw,setbwdata);
  fprintf(fileout, "SPRINKOFFCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sprinkoffcolor[0], sprinkoffcolor[1], sprinkoffcolor[2]);
  fprintf(fileout, "SPRINKONCOLOR\n");
  fprintf(fileout, " %f %f %f\n", sprinkoncolor[0], sprinkoncolor[1], sprinkoncolor[2]);
  fprintf(fileout, "STATICPARTCOLOR\n");
  fprintf(fileout, " %f %f %f\n", static_color[0], static_color[1], static_color[2]);
  {
    int scount;

    scount = 0;
    for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo+global_scase.surfcoll.sorted_surfidlist[i];
      if(surfi->used_by_geom==1)scount++;
    }
    if(scount>0){
      fprintf(fileout, "SURFCOLORS\n");
      fprintf(fileout, " %i %i\n", scount, use_surf_color);
      for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
        surfdata *surfi;

        surfi = global_scase.surfcoll.surfinfo+global_scase.surfcoll.sorted_surfidlist[i];
        if(surfi->used_by_geom==1){
          int *ini_surf_color;

          ini_surf_color = surfi->geom_surf_color;
          fprintf(fileout, " %i %i %i : %s\n", ini_surf_color[0], ini_surf_color[1], ini_surf_color[2], surfi->surfacelabel);
        }
      }
    }
  }
  {
    int scount;

    scount = 0;
    for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo+global_scase.surfcoll.sorted_surfidlist[i];
      if(surfi->in_color_dialog==1&&surfi->color!=surfi->color_orig)scount++;
    }
    if(scount>0){
      fprintf(fileout, "OBSTSURFCOLORS\n");
      fprintf(fileout, " %i %i\n", scount, use_surf_color);
      for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
        surfdata *surfi;

        surfi = global_scase.surfcoll.surfinfo+global_scase.surfcoll.sorted_surfidlist[i];
        if(surfi->in_color_dialog==1&&surfi->color!=surfi->color_orig){
          float *color;

          color = surfi->color;
          fprintf(fileout, " %f %f %f %f: %s\n", color[0], color[1], color[2], surfi->transparent_level, surfi->surfacelabel);
        }
      }
    }
  }
  fprintf(fileout, "TIMEBARCOLOR\n");
  fprintf(fileout, " %f %f %f\n", timebarcolor[0], timebarcolor[1], timebarcolor[2]);
  fprintf(fileout, "VENTCOLOR\n");
  fprintf(fileout," %f %f %f\n",ventcolor[0],ventcolor[1],ventcolor[2]);

  fprintf(fileout, "\n   *** SIZES/OFFSETS ***\n\n");

  fprintf(fileout, "GEOMSAXIS\n");
  fprintf(fileout, " %f %f\n",  glui_surf_axis_length, glui_surf_axis_width);
  fprintf(fileout, "GRIDLINEWIDTH\n");
  fprintf(fileout, " %f\n", gridlinewidth);
  fprintf(fileout, "ISOLINEWIDTH\n");
  fprintf(fileout, " %f\n", isolinewidth);
  fprintf(fileout, "ISOPOINTSIZE\n");
  fprintf(fileout, " %f\n", isopointsize);
  fprintf(fileout, "LINEWIDTH\n");
  fprintf(fileout, " %f\n", global_scase.linewidth);
  fprintf(fileout, "PARTPOINTSIZE\n");
  fprintf(fileout, " %f\n", partpointsize);
  fprintf(fileout, "PLOT3DLINEWIDTH\n");
  fprintf(fileout, " %f\n", plot3dlinewidth);
  fprintf(fileout, "PLOT3DPOINTSIZE\n");
  fprintf(fileout, " %f\n", plot3dpointsize);
  fprintf(fileout, "SENSORABSSIZE\n");
  fprintf(fileout, " %f\n", sensorabssize);
  fprintf(fileout, "SENSORRELSIZE\n");
  fprintf(fileout, " %f\n", sensorrelsize);
  fprintf(fileout, "SLICEOFFSET\n");
  fprintf(fileout, " %f %f %i\n", sliceoffset_factor,slice_dz, agl_offset_actual);
  fprintf(fileout, "SMOOTHLINES\n");
  fprintf(fileout, " %i\n", antialiasflag);
  fprintf(fileout, "SPHERESEGS\n");
  fprintf(fileout, " %i\n", device_sphere_segments);
  fprintf(fileout, "SORTSLICES\n");
  fprintf(fileout, " %i\n", sortslices);
  fprintf(fileout, "SPRINKLERABSSIZE\n");
  fprintf(fileout, " %f\n", sprinklerabssize);
  fprintf(fileout, "STREAKLINEWIDTH\n");
  fprintf(fileout, " %f\n", streaklinewidth);
  fprintf(fileout, "TICKLINEWIDTH\n");
  fprintf(fileout, " %f\n", ticklinewidth);
  fprintf(fileout, "NEWDRAWFACE\n");
  fprintf(fileout, " %i\n", blockage_draw_option);
  fprintf(fileout, "VECCONTOURS\n");
  fprintf(fileout, " %i %i\n", show_node_slices_and_vectors,show_cell_slices_and_vectors);
  fprintf(fileout, "VECLENGTH\n");
  fprintf(fileout, " %i %f %f %i %i %i\n", 4, vecfactor, 1.0, vec_uniform_length, vec_uniform_spacing, color_vector_black);
  fprintf(fileout, "VECTORLINEWIDTH\n");
  fprintf(fileout, " %f %f\n", vectorlinewidth, slice_line_contour_width);
  fprintf(fileout, "VECTORPOINTSIZE\n");
  fprintf(fileout," %f\n",vectorpointsize);
  fprintf(fileout, "VENTLINEWIDTH\n");
  fprintf(fileout, " %f\n", global_scase.ventlinewidth);
  fprintf(fileout, "VENTOFFSET\n");
  fprintf(fileout, " %f\n", ventoffset_factor);
  fprintf(fileout, "WINDOWOFFSET\n");
  fprintf(fileout, " %i\n", titlesafe_offsetBASE);
  if(use_graphics == 1 &&
     (screenWidth == GLUTGetScreenWidth()||screenHeight == GLUTGetScreenHeight())
    ){
    fprintf(fileout,"WINDOWWIDTH\n");
    fprintf(fileout," %i\n",-1);
    fprintf(fileout,"WINDOWHEIGHT\n");
    fprintf(fileout," %i\n",-1);
  }
  else{

#ifdef pp_OSX_HIGHRES
    if(double_scale==1){
      fprintf(fileout,"WINDOWWIDTH\n");
      fprintf(fileout," %i\n",screenWidth/2);
      fprintf(fileout,"WINDOWHEIGHT\n");
      fprintf(fileout," %i\n",screenHeight/2);
    }
    else{
      fprintf(fileout,"WINDOWWIDTH\n");
      fprintf(fileout," %i\n",screenWidth);
      fprintf(fileout,"WINDOWHEIGHT\n");
      fprintf(fileout," %i\n",screenHeight);
    }
#else
    fprintf(fileout,"WINDOWWIDTH\n");
    fprintf(fileout," %i\n",screenWidth);
    fprintf(fileout,"WINDOWHEIGHT\n");
    fprintf(fileout," %i\n",screenHeight);
#endif
  }

  fprintf(fileout, "\n *** DATA LOADING ***\n\n");

  fprintf(fileout, "CSV\n");
  fprintf(fileout, " %i\n", csv_loaded);
#ifdef pp_FRAME
  fprintf(fileout, "FRAMETHREADS\n");
  fprintf(fileout, " %i\n", nframe_threads);
#endif
  fprintf(fileout, "LOADINC\n");
  fprintf(fileout, " %i\n", load_incremental);
  fprintf(fileout, "NOPART\n");
  fprintf(fileout, " %i\n", nopart);
  fprintf(fileout, "PARTFAST\n");
  fprintf(fileout, " %i %i %i\n", partfast, use_partload_threads, n_partload_threads);
  fprintf(fileout, "RESEARCHMODE\n");
  fprintf(fileout, " %i %i %f %i %i %i %i %i %i %i\n", research_mode, 1, colorbar_shift, ncolorlabel_digits, force_fixedpoint, ngridloc_digits, sliceval_ndigits, force_exponential, force_decimal, force_zero_pad);
  fprintf(fileout, "SLICEAVERAGE\n");
  fprintf(fileout, " %i %f %i\n", slice_average_flag, slice_average_interval, vis_slice_average);
  fprintf(fileout, "SLICEDATAOUT\n");
  fprintf(fileout, " %i \n", output_slicedata);
  fprintf(fileout, "USER_ROTATE\n");
  fprintf(fileout, " %i %i %f %f %f\n", glui_rotation_index, show_rotation_center, xcenCUSTOM, ycenCUSTOM, zcenCUSTOM);
  fprintf(fileout, "ZIPSTEP\n");
  fprintf(fileout, " %i\n", tload_zipstep);

  fprintf(fileout,"\n *** VIEW PARAMETERS ***\n\n");

  GLUIGetGeomDialogState();
  fprintf(fileout, "APERTURE\n");
  fprintf(fileout, " %i\n", apertureindex);
  fprintf(fileout, "BLOCKLOCATION\n");
  fprintf(fileout, " %i\n", blocklocation);
  fprintf(fileout, "BEAM\n");
  fprintf(fileout, " %i %f %i %i %i %i\n", showbeam_as_line,beam_line_width,use_beamcolor,beam_color[0], beam_color[1], beam_color[2]);
  fprintf(fileout, "BLENDMODE\n");
  fprintf(fileout, " %i %i %i\n", slices3d_max_blending, hrrpuv_max_blending,showall_3dslices);
  fprintf(fileout, "BOUNDARYTWOSIDE\n");
  fprintf(fileout, " %i\n", showpatch_both);
  fprintf(fileout, "CLIP\n");
  fprintf(fileout, " %f %f\n", nearclip, farclip);
  fprintf(fileout, "CONTOURTYPE\n");
  fprintf(fileout, " %i %i\n", contour_type, colorbar_linewidth);
  fprintf(fileout, "CULLFACES\n");
  fprintf(fileout, " %i\n", cullfaces);
  fprintf(fileout, "ENABLETEXTURELIGHTING\n");
  fprintf(fileout, " %i\n", enable_texture_lighting);
  fprintf(fileout, "EYEVIEW\n");
  fprintf(fileout, " %i\n", rotation_type);
  fprintf(fileout, "FONTSIZE\n");
  fprintf(fileout, " %i\n", fontindex);
  fprintf(fileout, "FIREPARAMS\n");
  fprintf(fileout, "FRAMERATEVALUE\n");
  fprintf(fileout, " %i\n", frameratevalue);
  fprintf(fileout, "FREEZEVOLSMOKE\n");
  fprintf(fileout, " %i %i\n", freeze_volsmoke, autofreeze_volsmoke);
  fprintf(fileout, "GEOMBOUNDARYPROPS\n");
  fprintf(fileout, " %i %i %i %f %f %i\n",show_boundary_shaded, show_boundary_outline, show_boundary_points, geomboundary_linewidth, geomboundary_pointsize, boundary_edgetype);
  if(global_scase.hvaccoll.nhvacinfo > 0){
    fprintf(fileout, "HVACVIEW\n");
    fprintf(fileout, " %i %i %i %i %f %i\n", global_scase.hvaccoll.nhvacinfo, hvac_metro_view, 1, 0, 0.0, hvac_cell_view);
    for(i = 0; i < global_scase.hvaccoll.nhvacinfo; i++){
      hvacdata *hvaci;
      int *dc, *nc;

      hvaci = global_scase.hvaccoll.hvacinfo + i;
      dc = hvaci->duct_color;
      nc = hvaci->node_color;
      fprintf(fileout, " %i %i %i %i %i %f %f %f %f %f\n",
        hvaci->display, hvaci->show_node_labels, hvaci->show_duct_labels, hvaci->show_component,
        hvaci->show_filters, hvaci->duct_width, hvaci->node_size, hvaci->component_size, hvaci->filter_size, hvaci->cell_node_size);
      fprintf(fileout, " %i %i %i %i %i %i\n", dc[0], dc[1], dc[2], nc[0], nc[1], nc[2]);
    }
  }
  fprintf(fileout, "SHOWSLICEVALS\n");
  fprintf(fileout, " %i %i %i\n", show_slice_values[0], show_slice_values[1], show_slice_values[2]);
  fprintf(fileout, "GEOMCELLPROPS\n");
  fprintf(fileout, " %i\n",
    slice_celltype);
  fprintf(fileout, " %i %i %i\n",
    slice_edgetypes[0], slice_edgetypes[1], slice_edgetypes[2]);
  fprintf(fileout, " %i %i %i %f %i %i %i\n",
    show_slice_shaded[0], show_slice_shaded[1], show_slice_shaded[2], 5.0,
    show_vector_slice[0], show_vector_slice[1], show_vector_slice[2]);
  fprintf(fileout, " %i %i %i\n",
    show_slice_outlines[0], show_slice_outlines[1], show_slice_outlines[2]);
  fprintf(fileout, " %i %i %i\n",
    show_slice_points[0], show_slice_points[1], show_slice_points[2]);
  fprintf(fileout, "GEOMDOMAIN\n");
  fprintf(fileout, " %i %i\n", showgeom_inside_domain, showgeom_outside_domain);
  fprintf(fileout, "GEOMSHOW\n");
  fprintf(fileout, " %i %i %i %i %i %i %f %f %i %i %f %f %f\n",
     0, 1, show_faces_shaded, show_faces_outline, smooth_geom_normal,
     geom_force_transparent, geom_transparency, geom_linewidth, use_geom_factors, show_cface_normals, geom_pointsize, geom_dz_offset, geom_norm_offset);
  fprintf(fileout, " %i %i %i %i\n", 0, 0, 0, 0);

  int hide_scene_old;

  hide_scene_old = hide_scene;
  if(hide_scene != 1)hide_scene_old = 2;
  fprintf(fileout, " %f %f %i %i %i %i\n", geom_vert_exag, 30.0, 0, 0, hide_scene_old, show_geom_bndf);

  fprintf(fileout, "GVERSION\n");
  fprintf(fileout, " %i\n", vis_title_gversion);
  fprintf(fileout, "GVECDOWN\n");
  fprintf(fileout, " %i\n", gvec_down);
  fprintf(fileout, "ISOTRAN2\n");
  fprintf(fileout, " %i\n", transparent_state);
  for(i = 0; i < global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    if(meshi->mesh_offset_ptr != NULL){
      fprintf(fileout, "MESHOFFSET\n");
      fprintf(fileout, " %i\n", i);
    }
  }
  if(global_scase.meshescoll.nmeshes>1){
    fprintf(fileout,"MESHVIS\n");
    fprintf(fileout," %i\n",global_scase.meshescoll.nmeshes);

    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      fprintf(fileout," %i\n",meshi->blockvis);
    }
  }
  fprintf(fileout, "NORTHANGLE\n");
  fprintf(fileout, " %i\n", vis_northangle);
  fprintf(fileout, " %f %f %f\n", northangle_position[0], northangle_position[1], northangle_position[2]);
  fprintf(fileout, "OFFSETSLICE\n");
  fprintf(fileout, " %i\n", offset_slice);
  fprintf(fileout, "OUTLINEMODE\n");
  fprintf(fileout, " %i %i\n", highlight_flag, outline_color_flag);
  fprintf(fileout, "P3DSURFACETYPE\n");
  fprintf(fileout, " %i\n", p3dsurfacetype);
  fprintf(fileout, "P3DSURFACESMOOTH\n");
  fprintf(fileout, " %i\n", p3dsurfacesmooth);
  fprintf(fileout, "PLOT2DHRRBOUNDS\n");
  fprintf(fileout, " %i %f %i %f\n", 0, plot2d_hrr_min, 0, plot2d_hrr_max);
  fprintf(fileout, "PROJECTION\n");
  fprintf(fileout, " %i\n", projection_type);
  fprintf(fileout, "SCALEDFONT\n");
  fprintf(fileout, " %i %f %i\n", scaled_font2d_height, scaled_font2d_height2width, scaled_font2d_thickness);
  fprintf(fileout, " %i %f %i\n", scaled_font3d_height, scaled_font3d_height2width, scaled_font3d_thickness);
  fprintf(fileout, "SHOWALLTEXTURES\n");
  fprintf(fileout, " %i\n", showall_textures);
  fprintf(fileout, "SHOWAXISLABELS\n");
  fprintf(fileout, " %i\n", visaxislabels);
  fprintf(fileout, "SHOWBLOCKLABEL\n");
  fprintf(fileout, " %i\n", visMeshlabel);
  fprintf(fileout, "SHOWBLOCKS\n");
  fprintf(fileout, " %i %i %i\n", visBlocks, solid_state, outline_state);
  fprintf(fileout, "SHOWBOUNDS\n");
  fprintf(fileout, " %i %i\n", bounds_each_mesh, show_bound_diffs);
  fprintf(fileout, "SHOWCADOPAQUE\n");
  fprintf(fileout, " %i\n", viscadopaque);
  fprintf(fileout, "SHOWCEILING\n");
  fprintf(fileout, " %i\n", global_scase.visCeiling);
  fprintf(fileout, "SHOWCHID\n");
  fprintf(fileout, " %i\n", vis_title_CHID);
  fprintf(fileout, "SHOWCOLORBARS\n");
  fprintf(fileout, " %i %i\n", visColorbarVertical,visColorbarHorizontal);
  fprintf(fileout, "SHOWCVENTS\n");
  fprintf(fileout, " %i %i\n", visCircularVents, circle_outline);
  fprintf(fileout, "SHOWDUMMYVENTS\n");
  fprintf(fileout, " %i\n", visDummyVents);
  fprintf(fileout, "SHOWFIRECUTOFF\n");
  fprintf(fileout, " %i\n", show_firecutoff);
  fprintf(fileout, "SHOWFLOOR\n");
  fprintf(fileout, " %i\n", global_scase.visFloor);
  fprintf(fileout, "SHOWFRAME\n");
  fprintf(fileout, " %i\n", global_scase.visFrame);
  fprintf(fileout, "SHOWFRAMELABEL\n");
  fprintf(fileout, " %i\n", visFramelabel);
  fprintf(fileout, "SHOWFRAMETIMELABEL\n");
  fprintf(fileout, " %i\n", visFrameTimelabel);
  fprintf(fileout, "SHOWFRAMERATE\n");
  fprintf(fileout, " %i\n", visFramerate);
  fprintf(fileout, "SHOWGRID\n");
  fprintf(fileout, " %i\n", visGrid);
  fprintf(fileout, "SHOWGRIDLOC\n");
  fprintf(fileout, " %i\n", visgridloc);
  fprintf(fileout, "SHOWHMSTIMELABEL\n");
  fprintf(fileout, " %i\n", vishmsTimelabel);
  fprintf(fileout, "SHOWHRRLABEL\n");
  fprintf(fileout, " %i\n", vis_hrr_label);
  fprintf(fileout, "SHOWISO\n");
  fprintf(fileout, " %i\n", visAIso);
  fprintf(fileout, "SHOWISONORMALS\n");
  fprintf(fileout, " %i\n", show_iso_normal);
  fprintf(fileout, "SHOWLABELS\n");
  fprintf(fileout, " %i\n", visLabels);
#ifdef pp_memstatus
  fprintf(fileout, "SHOWMEMLOAD\n");
  fprintf(fileout, " %i\n", visAvailmemory);
#endif
  fprintf(fileout, "SHOWPARTTAG\n");
  fprintf(fileout, " %i\n", select_part);
  fprintf(fileout, "SHOWOPENVENTS\n");
  fprintf(fileout, " %i %i\n", visOpenVents, visOpenVentsAsOutline);
  fprintf(fileout, "SHOWOTHERVENTS\n");
  fprintf(fileout, " %i\n", global_scase.visOtherVents);
  fprintf(fileout, "SHOWROOMS\n");
  fprintf(fileout, " %i\n", visCompartments);
  fprintf(fileout, "SHOWSENSORS\n");
  fprintf(fileout, " %i %i\n", visSensor, visSensorNorm);
  fprintf(fileout, "SHOWSLICEINOBST\n");
  fprintf(fileout, " %i\n", global_scase.show_slice_in_obst);
  fprintf(fileout, "SHOWSMOKEPART\n");
  fprintf(fileout, " %i\n", visSmokePart);
  fprintf(fileout, "SHOWSKYSPHERE\n");
  fprintf(fileout, "%i %i %i %i %i %f\n", visSkysphere, visSkybox, visSkyground, visSkyboxoutline, visSkySpheretexture, sky_diam);
  fprintf(fileout, "SHOWSPRINKPART\n");
  fprintf(fileout, " %i\n", visSprinkPart);
  fprintf(fileout, "SHOWSTREAK\n");
  fprintf(fileout, " %i %i %i %i\n", streak5show, streak5step, showstreakhead, streak_index);
  fprintf(fileout, "SHOWTERRAIN\n");
  fprintf(fileout, " %i %i\n", global_scase.visTerrainType, terrain_slice_overlap);
  fprintf(fileout, "SHOWTHRESHOLD\n");
  fprintf(fileout, " %i %i %f\n", vis_threshold, vis_onlythreshold, temp_threshold);
  fprintf(fileout, "SHOWTICKS\n");
  fprintf(fileout, " %i\n", visFDSticks);
  fprintf(fileout, "SHOWTIMEBAR\n");
  fprintf(fileout, " %i %i\n", visTimebar,timebar_overlap);
  fprintf(fileout, "SHOWTIMELABEL\n");
  fprintf(fileout, " %i\n", visTimelabel);
  fprintf(fileout, "SHOWTITLE\n");
  fprintf(fileout, " %i %i %i\n", vis_title_smv_version, 0, vis_title_fds);
  fprintf(fileout, "SHOWTRACERSALWAYS\n");
  fprintf(fileout, " %i\n", show_tracers_always);
  fprintf(fileout, "SHOWTRANSPARENT\n");
  fprintf(fileout, " %i\n", visTransparentBlockage);
  fprintf(fileout, "SHOWTRIANGLES\n");
  fprintf(fileout, " %i %i %i %i 1 %i %i\n", show_iso_shaded, show_iso_outline, show_iso_points, show_iso_normal, smooth_iso_normal, sort_iso_triangles);
  fprintf(fileout, "SHOWTRANSPARENTVENTS\n");
  fprintf(fileout, " %i\n", show_transparent_vents);
  fprintf(fileout, "SHOWTRIANGLECOUNT\n");
  fprintf(fileout, " %i\n", show_triangle_count);
  fprintf(fileout, "SHOWVENTFLOW\n");
  fprintf(fileout, " %i %i %i %i %i\n", visVentHFlow, visventslab, visventprofile, visVentVFlow, visVentMFlow);
  fprintf(fileout, "SHOWTARGETS\n");
  fprintf(fileout, " %i \n", vis_target_data);
  fprintf(fileout, "SHOWVENTS\n");
  fprintf(fileout, " %i\n", visVents);
  fprintf(fileout, "SHOWWALLS\n");
  fprintf(fileout, " %i %i\n", global_scase.visWalls,vis_wall_data);
  fprintf(fileout, "SKIPEMBEDSLICE\n");
  fprintf(fileout, " %i\n", skip_slice_in_embedded_mesh);
  fprintf(fileout, "SLICEDUP\n");
  fprintf(fileout, " %i %i %i\n", slicedup_option, vectorslicedup_option, boundaryslicedup_option);
  fprintf(fileout, "SMOKESENSORS\n");
  fprintf(fileout, " %i %i\n", show_smokesensors, test_smokesensors);
  fprintf(fileout, "STARTUPLANG\n");
  fprintf(fileout, " %s\n", startup_lang_code);
  fprintf(fileout, "STEREO\n");
  fprintf(fileout, " %i\n", stereotype);
  fprintf(fileout, "SURFINC\n");
  fprintf(fileout, " %i\n", surfincrement);
  fprintf(fileout, "TERRAINPARMS\n");
  fprintf(fileout, " %i %i %i\n", terrain_rgba_zmin[0], terrain_rgba_zmin[1], terrain_rgba_zmin[2]);
  fprintf(fileout, " %i %i %i\n", terrain_rgba_zmax[0], terrain_rgba_zmax[1], terrain_rgba_zmax[2]);
  fprintf(fileout, " %f\n", vertical_factor);
  fprintf(fileout, "TITLESAFE\n");
  fprintf(fileout, " %i\n", titlesafe_offset);
  if(trainer_mode == 1){
    fprintf(fileout, "TRAINERMODE\n");
    fprintf(fileout, " %i\n", trainer_mode);
  }
  fprintf(fileout, "TRAINERVIEW\n");
  fprintf(fileout, " %i\n", trainerview);
  fprintf(fileout, "TRANSPARENT\n");
  fprintf(fileout, " %i %f %i %i\n", use_transparency_data, transparent_level, iso_transparency_option, iso_opacity_change);
  fprintf(fileout, "TREEPARMS\n");
  fprintf(fileout, " %i %i %i %i\n", mintreesize,vis_xtree,vis_ytree,vis_ztree);
  fprintf(fileout, "TWOSIDEDVENTS\n");
  fprintf(fileout, " %i %i\n", show_bothsides_int, show_bothsides_ext);
  fprintf(fileout, "VECTORSKIP\n");
  fprintf(fileout, " %i\n", vectorskip);
  fprintf(fileout, "VISBOUNDARYTYPE\n");
  for(i = 0; i<7; i++){
    fprintf(fileout, " %i ", vis_boundary_type[i]);
  }
  fprintf(fileout, " %i %i \n", show_mirror_boundary, show_mirror_boundary);
  fprintf(fileout, "WINDROSEDEVICE\n");
  fprintf(fileout, " %i %i %i %i %i %i %i %i %i\n",
    viswindrose, showref_windrose, windrose_xy_vis, windrose_xz_vis, windrose_yz_vis, windstate_windrose, showlabels_windrose,
    windrose_first,windrose_next);
  fprintf(fileout, " %i %i %i %f %i %i\n", nr_windrose, ntheta_windrose, scale_windrose, radius_windrose, scale_increment_windrose, scale_max_windrose);
  {
    if(nwindrosez_showhide > 0){
      int ii;

      GLUIUpdateWindRoseDevices(UPDATE_WINDROSE_SHOWHIDE);
      fprintf(fileout, "WINDROSESHOWHIDE\n");
      fprintf(fileout, " %i\n", nwindrosez_showhide);
      for(ii = 0; ii < nwindrosez_showhide; ii++){
        fprintf(fileout, " %i", windrosez_showhide[ii]);
        if((ii+1)%WINDROSE_PER_ROW==0)fprintf(fileout, "\n");
      }
      fprintf(fileout, "\n");
    }
  }
  {
    float *xyzt;

    xyzt = windrose_merge_dxyzt;
    fprintf(fileout, "WINDROSEMERGE\n");
    fprintf(fileout, " %i %f %f %f %f\n",windrose_merge_type,xyzt[0],xyzt[1],xyzt[2],xyzt[3]);
  }
  fprintf(fileout, "ZOOM\n");
  fprintf(fileout, " %i %f\n", zoomindex, zoom);

  fprintf(fileout,"\n *** MISC ***\n\n");

  fprintf(fileout, "SHOWSLICEVALS\n");
  fprintf(fileout, " %i\n", show_slice_values_all_regions);
  if(global_scase.paths.fds_filein != NULL&&strlen(global_scase.paths.fds_filein) > 0){
    fprintf(fileout, "INPUT_FILE\n");
    fprintf(fileout, " %s\n", global_scase.paths.fds_filein);
  }
  fprintf(fileout, "LABELSTARTUPVIEW\n");
  fprintf(fileout, " %s\n", viewpoint_label_startup);
  fprintf(fileout, "RENDERCLIP\n");
  fprintf(fileout, " %i %i %i %i %i\n",
    clip_rendered_scene, render_clip_left, render_clip_right, render_clip_bottom, render_clip_top);
  fprintf(fileout, "RENDERFILELABEL\n");
  fprintf(fileout, " %i\n", render_label_type);
  fprintf(fileout, "RENDERFILETYPE\n");
  fprintf(fileout," %i %i %i\n",render_filetype, movie_filetype, render_resolution);
  fprintf(fileout, "MOVIEFILETYPE\n");
  {
    int quicktime_dummy=1;

    fprintf(fileout, "MOVIEFILETYPE\n");
    fprintf(fileout," %i %i %i %i %i\n",movie_filetype,movie_framerate,movie_bitrate,quicktime_dummy,movie_crf);
  }
  fprintf(fileout, "MOVIEPARMS\n");
  fprintf(fileout, " %i %i %i\n", movie_queue_index, movie_nprocs, movie_slice_index);
  if(nskyboxinfo>0){
    int iskybox;
    skyboxdata *skyi;
    char *filei;
    char *nullfile="NULL";

    for(iskybox=0;iskybox<nskyboxinfo;iskybox++){
      skyi = skyboxinfo + iskybox;
      fprintf(fileout,"SKYBOX\n");
      for(i=0;i<6;i++){
        filei = skyi->face[i].file;
        if(filei==NULL)filei=nullfile;
        if(strcmp(filei,"NULL")==0){
          fprintf(fileout,"NULL\n");
        }
        else{
          fprintf(fileout," %s\n",filei);
        }
      }
    }
  }
  fprintf(fileout, "RENDEROPTION\n");
  fprintf(fileout, " %i %i %i\n", render_window_size, resolution_multiplier, nheight360);
  fprintf(fileout, "UNITCLASSES\n");
  fprintf(fileout, " %i\n", nunitclasses);
  for(i = 0; i<nunitclasses; i++){
    fprintf(fileout, " %i\n", unitclasses[i].unit_index);
  }
  fprintf(fileout, "SHOWGRAVVECTOR\n");
  fprintf(fileout, " %i\n", showgravity_vector);
  if(zaxis_custom == 1){
    fprintf(fileout, "ZAXISANGLES\n");
    fprintf(fileout, " %f %f %f\n", zaxis_angles[0], zaxis_angles[1], zaxis_angles[2]);
  }

  fprintf(fileout,"\n *** 3D SMOKE INFO ***\n\n");

  {
    colorbardata *cb;
    char percen[2];

    cb = colorbars.colorbarinfo + colorbartype;
    strcpy(percen, "%");
    fprintf(fileout, "COLORBARTYPE\n");
    fprintf(fileout, " %i %s %s \n", colorbartype, percen, cb->menu_label);
  }
  if(colorbars.co2_colorbar_index >= 0 && colorbars.co2_colorbar_index < colorbars.ncolorbars){
    fprintf(fileout, "COLORMAP\n");
    fprintf(fileout, " CO2 %i %s\n", co2_colormap_type, colorbars.colorbarinfo[colorbars.co2_colorbar_index].menu_label);
  }
  {
    int mmin[3], mmax[3];
    for(i = 0; i < 3; i++){
      mmin[i] = rgb_below_min[i];
      mmax[i] = rgb_above_max[i];
    }
    fprintf(fileout, "EXTREMECOLORS\n");
    fprintf(fileout, " %i %i %i %i %i %i\n",
      mmin[0], mmin[1], mmin[2],
      mmax[0], mmax[1], mmax[2]);
  }
  fprintf(fileout, "FIRECOLOR\n");
  fprintf(fileout, " %i %i %i\n", fire_color_int255[0], fire_color_int255[1], fire_color_int255[2]);
  if(colorbars.fire_colorbar_index >= 0 && colorbars.fire_colorbar_index < colorbars.ncolorbars){
    fprintf(fileout, "FIRECOLORMAP\n");
    fprintf(fileout, " FIRE %i %s\n", fire_colormap_type, colorbars.colorbarinfo[colorbars.fire_colorbar_index].menu_label);
  }
  fprintf(fileout, "FIREDEPTH\n");
  fprintf(fileout, " %f %f %f %i %i\n", fire_halfdepth, co2_halfdepth, emission_factor, use_fire_alpha, force_alpha_opaque);
  fprintf(fileout, "FDEPTH\n");
  fprintf(fileout, " %f %f %f %i %i\n", fire_halfdepth, co2_halfdepth, emission_factor, use_fire_alpha, force_alpha_opaque);
  if(colorbars.ncolorbars > colorbars.ndefaultcolorbars){
    colorbardata *cbi;
    unsigned char *rrgb;
    int n;

    fprintf(fileout, "GCOLORBAR\n");
    fprintf(fileout, " %i\n", colorbars.ncolorbars - colorbars.ndefaultcolorbars);
    for(n = colorbars.ndefaultcolorbars; n < colorbars.ncolorbars; n++){
      cbi = colorbars.colorbarinfo + n;
      fprintf(fileout, " %s\n", cbi->menu_label);
      fprintf(fileout, " %i %i\n", cbi->nnodes, cbi->nodehilight);
      for(i = 0; i < cbi->nnodes; i++){
        rrgb = cbi->node_rgb + 3 * i;
        fprintf(fileout, " %i %i %i %i\n", (int)cbi->node_index[i], (int)rrgb[0], (int)rrgb[1], (int)rrgb[2]);
      }
    }
  }
  fprintf(fileout, "HRRPUVCUTOFF\n");
  fprintf(fileout, " %f\n", global_scase.global_hrrpuv_cutoff);
  fprintf(fileout, "SHOWEXTREMEDATA\n");
  {
    int show_extremedata = 0;

    if(show_extreme_mindata == 1 || show_extreme_maxdata == 1)show_extremedata = 1;
    fprintf(fileout, " %i %i %i\n", show_extremedata, show_extreme_mindata, show_extreme_maxdata);
  }
  fprintf(fileout, "SLICESKIP\n");
  fprintf(fileout, " %i %i %i %i\n", slice_skip, slice_skipx, slice_skipy, slice_skipz);
  fprintf(fileout, "SMOKECOLOR\n");
  fprintf(fileout, " %i %i %i %i\n", smoke_color_int255[0], smoke_color_int255[1], smoke_color_int255[2], force_gray_smoke);
  fprintf(fileout, "SMOKECULL\n");
  fprintf(fileout," %i\n",smokecullflag);
  if(ABS(global_scase.smoke_albedo - global_scase.smoke_albedo_base) > 0.001){
    fprintf(fileout, "SMOKEALBEDO\n");
    fprintf(fileout, " %f\n", global_scase.smoke_albedo);
  }
  if((have_fire == NO_FIRE && have_smoke == NO_SMOKE)||(have_fire != NO_FIRE && have_smoke != NO_SMOKE)){
    fprintf(fileout, "SMOKEFIREPROP\n");
    fprintf(fileout, " %i %i\n", use_opacity_depth, use_opacity_multiplier);
  }
  fprintf(fileout, "SMOKEPROP\n");
  fprintf(fileout, "%f\n", glui_smoke3d_extinct);
  glui_smoke3d_extinct_default = glui_smoke3d_extinct;
  fprintf(fileout, "SMOKESKIP\n");
  fprintf(fileout," %i %i %i %i %i\n", smoke3d_frame_inc-1,smoke3d_skip, smoke3d_skipx, smoke3d_skipy, smoke3d_skipz);
#ifdef pp_GPU
  fprintf(fileout, "USEGPU\n");
  fprintf(fileout, " %i\n", usegpu);
#endif
  fprintf(fileout, "VOLSMOKE\n");
  fprintf(fileout, " %i %i %i %i %i\n",
    glui_compress_volsmoke, use_multi_threading, load_at_rendertimes, volbw, show_volsmoke_moving);
  fprintf(fileout, " %f %f %f %f %f %f %f\n",
    global_temp_min, global_temp_cb_min, global_temp_cb_max, fire_opacity_factor, mass_extinct, gpu_vol_factor, nongpu_vol_factor);

  fprintf(fileout, "\n *** ZONE FIRE PARAMETRES ***\n\n");

  fprintf(fileout, "SHOWHAZARDCOLORS\n");
  fprintf(fileout, " %i\n", zonecolortype);
  fprintf(fileout, "SHOWZONEPLANE\n");
  fprintf(fileout, " %i %i\n", visZonePlane, show_zonelower);
  fprintf(fileout, "SHOWSZONE\n");
  fprintf(fileout, " %i\n", visSZone);
  fprintf(fileout, "SHOWZONEFIRE\n");
  fprintf(fileout, " %i\n", viszonefire);

  fprintf(fileout,"\n *** TOUR INFO ***\n\n");

  fprintf(fileout, "SHOWAVATAR\n");
  fprintf(fileout, " %i\n", show_avatar);
  fprintf(fileout, "SHOWPATHNODES\n");
  fprintf(fileout, " %i\n", show_path_knots);
  fprintf(fileout, "SHOWTOURROUTE\n");
  fprintf(fileout, " %i\n", edittour);
  {
    float *col;

    fprintf(fileout, "TOURCOLORS\n");
    col = tourcol_selectedpathline;
    fprintf(fileout, " %f %f %f   :selected path line\n", col[0], col[1], col[2]);
    col = tourcol_selectedpathlineknots;
    fprintf(fileout, " %f %f %f   :selected path line knots\n", col[0], col[1], col[2]);
    col = tourcol_selectedknot;
    fprintf(fileout, " %f %f %f   :selected knot\n", col[0], col[1], col[2]);
    col = tourcol_pathline;
    fprintf(fileout, " %f %f %f   :path line\n", col[0], col[1], col[2]);
    col = tourcol_pathknots;
    fprintf(fileout, " %f %f %f   :path knots\n", col[0], col[1], col[2]);
    col = tourcol_text;
    fprintf(fileout, " %f %f %f   :text\n", col[0], col[1], col[2]);
    col = tourcol_avatar;
    fprintf(fileout, " %f %f %f   :avatar\n", col[0], col[1], col[2]);


  }
  fprintf(fileout, "TOURCONSTANTVEL\n");
  fprintf(fileout, " %i\n", tour_constant_velocity);
  fprintf(fileout, "VIEWALLTOURS\n");
  fprintf(fileout, " %i\n", viewalltours);
  fprintf(fileout, "VIEWTIMES\n");
  fprintf(fileout, " %f %f %i\n", global_scase.tourcoll.tour_tstart, global_scase.tourcoll.tour_tstop, global_scase.tourcoll.tour_ntimes);
  fprintf(fileout, "VIEWTOURFROMPATH\n");
  fprintf(fileout, " %i %i %f\n", viewtourfrompath, 0, 0.0);

  if(flag == LOCAL_INI)WriteIniLocal(fileout);
  if(
    ((INI_fds_filein != NULL&&global_scase.paths.fds_filein != NULL&&strcmp(INI_fds_filein, global_scase.paths.fds_filein) == 0) ||
    flag == LOCAL_INI))OutputViewpoints(fileout);

  {
    char githash[256];
    char gitdate[256];

    GetGitInfo(githash,gitdate);    // get githash
    fprintf(fileout,"\n\n");
    fprintf(fileout,"# FDS/Smokeview Environment\n");
    fprintf(fileout,"# -------------------------\n\n");
    fprintf(fileout,"# Smokeview Build: %s\n",githash);
    fprintf(fileout,"# Smokeview Build Date: %s\n",__DATE__);
    if(global_scase.fds_version!=NULL){
      fprintf(fileout,"# FDS Version: %s\n",global_scase.fds_version);
    }
    if(global_scase.fds_githash!=NULL){
      fprintf(fileout, "# FDS Build: %s\n", global_scase.fds_githash);
    }
    fprintf(fileout,"# Platform: WIN64\n");
#ifdef pp_OSX
    PRINTF("Platform: OSX64\n");
#endif
#ifdef pp_LINUX
    fprintf(fileout,"# Platform: LINUX64\n");
#endif

    if(use_graphics==1){
      GLint nred, ngreen, nblue, ndepth, nalpha;
      int max_texture_size;

      glGetIntegerv(GL_RED_BITS,&nred);
      glGetIntegerv(GL_GREEN_BITS,&ngreen);
      glGetIntegerv(GL_BLUE_BITS,&nblue);
      glGetIntegerv(GL_DEPTH_BITS,&ndepth);
      glGetIntegerv(GL_ALPHA_BITS,&nalpha);
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
      fprintf(fileout, "\n\n");
      fprintf(fileout,"# Graphics Environment\n");
      fprintf(fileout,"# --------------------\n\n");
      if(use_graphics == 1){
        char version_label[256];
        char *glversion = NULL;
        char *glshadeversion = NULL;

        glversion = (char *)glGetString(GL_VERSION);
        if(glversion != NULL){
          strcpy(version_label, "OpenGL Version: ");
          strcat(version_label, glversion);
          fprintf(fileout, "# %s\n", version_label);
        }
        glshadeversion=(char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if(glshadeversion != NULL){
          strcpy(version_label, "  GLSL Version: ");
          strcat(version_label, glshadeversion);
          fprintf(fileout, "# %s\n", version_label);
        }
      }
      fprintf(fileout,"#         Red bits:%i\n", nred);
      fprintf(fileout,"#       Green bits:%i\n",ngreen);
      fprintf(fileout,"#        Blue bits:%i\n",nblue);
      fprintf(fileout,"#       Alpha bits:%i\n",nalpha);
      fprintf(fileout,"#       Depth bits:%i\n",ndepth);
      fprintf(fileout,"# max texture size:%i\n\n",max_texture_size);
    }
  }

  if(fileout!=stdout){
    fclose(fileout);
  }
  FREEMEMORY(smokeview_scratchdir);
}

/* ------------------ UpdateLoadedLists ------------------------ */

void UpdateLoadedLists(void){
  int i;
  slicedata *slicei;
  patchdata *patchi;

  nslice_loaded=0;
  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicei = global_scase.slicecoll.sliceinfo + i;
    if(slicei->loaded==1){
      slice_loaded_list[nslice_loaded]=i;
      nslice_loaded++;
    }
  }

  ngeomslice_loaded = 0;
  for(i=0;i<global_scase.npatchinfo;i++){
    patchi = global_scase.patchinfo + i;
    if(patchi->loaded==1&&patchi->boundary == 0)ngeomslice_loaded++;
  }

  nvolsmoke_loaded = 0;
  if(nvolrenderinfo>0){
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      volrenderdata *vr;

      meshi = global_scase.meshescoll.meshinfo + i;
      vr = meshi->volrenderinfo;
      if(vr==NULL||vr->fireslice==NULL||vr->smokeslice==NULL)continue;
      if(vr->loaded==0||vr->display==0)continue;
      nvolsmoke_loaded++;
    }
  }
}

/* ------------------ GetElevAz ------------------------ */

void GetElevAz(float *xyznorm,float *dtheta,float *rotate_axis, float *dpsi){

  // cos(dtheta) = (xyznorm .dot. vec3(0,0,1))/||xyznorm||
  // rotate_axis = xyznorm .cross. vec3(0,0,1)

  Normalize(xyznorm,3);
  *dtheta = RAD2DEG*acos(xyznorm[2]);
  rotate_axis[0]=-xyznorm[1];
  rotate_axis[1]= xyznorm[0];
  rotate_axis[2]=0.0;
  Normalize(rotate_axis,2);
  if(dpsi!=NULL){
    float xyznorm2[2];

    xyznorm2[0]=xyznorm[0];
    xyznorm2[1]=xyznorm[1];
    Normalize(xyznorm2,2);
    *dpsi = RAD2DEG*acos(xyznorm2[1]);
    if(xyznorm2[0]<0.0)*dpsi=-(*dpsi);
  }
}
