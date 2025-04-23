// _GNU_SOURCE needs to be switched on to access qsort_r. qsort_r is included in
// POSIX 2024 as standard but is also available via GNU extensions.
#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include "options.h"
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


#include "translate.h"
#include "file_util.h"
#include "datadefs.h"
#include "readhvac.h"
#include "readslice.h"
#include "readobject.h"
#include "readsmoke.h"
#include "readlabel.h"
#include "readtour.h"
#include "readgeom.h"
#include "colorbars.h"
#include "readcad.h"
#include "readsmoke.h"
#include "readcad.h"
#include "readtour.h"
#include "readsmvfile.h"

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

#define SCAN    0
#define NO_SCAN 1

parse_options parse_opts = {
  .smoke3d_only = 0,
  .setup_only = 0,
  .fast_startup = 1,
  .lookfor_compressed_files = 0,
  .handle_slice_files = 1
};

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

  scase->time_col  = GetHrrCsvCol(scase, "Time");
  if(scase->time_col>=0)scase->timeptr = scase->hrr_coll.hrrinfo+scase->time_col;

  scase->hrr_col   = GetHrrCsvCol(scase, "HRR");
  if(scase->hrr_col>=0&&scase->time_col>=0)scase->hrrptr = scase->hrr_coll.hrrinfo+scase->hrr_col;

  scase->qradi_col = GetHrrCsvCol(scase, "Q_RADI");
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
  UpdateHoc(scase);
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
      devicei->color = GetColorPtr(scase, color);
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
  devicei->prop=GetPropID(scase, prop_id);
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
  devicei->prop = GetPropID(scase, prop_id);
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
  return scase->surfacedefault;
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

void InitSurface(surfdata *surf, float *color){
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
  surf->color = color;
  surf->t_width = 1.0;
  surf->t_height = 1.0;
  surf->type = BLOCK_regular;
  surf->obst_surface = 1;
  surf->location = 0;
  surf->invisible = 0;
  surf->transparent = 0;
}

/* ------------------ InitVentSurface ------------------------ */

void InitVentSurface(surfdata *surf, float color[4]){
  surf->emis = 1.0;
  surf->temp_ignition = TEMP_IGNITION_MAX;
  surf->surfacelabel = NULL;
  surf->texturefile = NULL;
  surf->textureinfo = NULL;
  surf->color = color;
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
      InitSurface(surfj, scase->color_defs.block_ambient2);
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
  zvi->color = GetColorPtr(scase, color);
  zvi->area_fraction = area_fraction;
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
#ifdef pp_FIRE_HIST
    smoke3di->histtimes = NULL;
#endif
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
#ifdef pp_SMOKE3D_FORCE
    if(strcmp(smoke3di->file, "dummy.xyz") == 0){
      smoke3di->dummy = 1;
    }
    else{
      smoke3di->dummy = 0;
    }
#endif
    char buffer_s3dd[256], *ext;

    strcpy(buffer_s3dd, bufferptr);
    ext = strrchr(buffer_s3dd, '.');
    if(ext != NULL)*ext = 0;
    strcat(buffer_s3dd, ".s3dd");
    smoke3di->smoke_density_file = SMOKE3DBUFFER(strlen(buffer_s3dd) + 1);
    STRCPY(smoke3di->smoke_density_file, buffer_s3dd);

    {
      if(ReadLabels(&smoke3di->label, stream, NULL)==LABEL_ERR)return RETURN_TWO;
      if(strcmp(smoke3di->label.longlabel, "SOOT DENSITY") == 0){
        smoke3di->is_smoke = 1;
      }
      if(strcmp(smoke3di->label.longlabel, "HRRPUV")==0){
        scase->show_hrrcutoff_active = 1;
        smoke3di->is_fire = 1;
      }
      if(strstr(smoke3di->label.longlabel, "TEMPERATURE") !=NULL){
        scase->show_tempcutoff_active = 1;
        smoke3di->is_fire = 1;
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

  START_TIMER(scase->getfilelist_time);
  MakeFileLists(scase);
  PRINT_TIMER(timer_setup, "MakeFileLists");
  STOP_TIMER(scase->getfilelist_time);

  START_TIMER(scase->pass0_time);

  scase->propcoll.npropinfo=1; // the 0'th prop is the default human property

  FREEMEMORY(scase->fds_title);

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
  InitSurface(&scase->sdefault, scase->color_defs.block_ambient2);
  PRINT_TIMER(timer_setup, "InitSurface");
  NewMemory((void **)&scase->sdefault.surfacelabel,(5+1));
  strcpy(scase->sdefault.surfacelabel,"INERT");

  InitVentSurface(&scase->v_surfacedefault, scase->color_defs.ventcolor);
  PRINT_TIMER(timer_setup, "InitVentSurface");
  NewMemory((void **)&scase->v_surfacedefault.surfacelabel,(4+1));
  strcpy(scase->v_surfacedefault.surfacelabel,"VENT");

  InitSurface(&scase->e_surfacedefault, scase->color_defs.block_ambient2);
  PRINT_TIMER(timer_setup, "InitSurface");
  NewMemory((void **)&scase->e_surfacedefault.surfacelabel,(8+1));
  strcpy(scase->e_surfacedefault.surfacelabel,"EXTERIOR");
  scase->e_surfacedefault.color=scase->color_defs.block_ambient2;

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
        FreeSmoke3D(scase, smoke3di);
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

#ifdef pp_SMOKE3D_FORCE
/* ------------------ HaveSmoke3D ------------------------ */

int HaveSmoke3D(bufferstreamdata *stream){
  char buffer[256];

  for(;;){
    if(FEOF(stream) != 0){
      BREAK;
    }
    if(FGETS(buffer, 255, stream) == NULL){
      BREAK;
    }
    TrimBack(buffer);
    if(strncmp(buffer, " ", 1) == 0 || buffer[0] == 0)continue;
    if(MatchSMV(buffer,"SMOKE3D") == 1 || MatchSMV(buffer,"SMOKF3D") == 1 || MatchSMV(buffer, "SMOKG3D") == 1){
      rewind_buffer(stream->fileinfo);
      return 1;
      }
  }
  rewind_buffer(stream->fileinfo);
  return 0;
}
#endif
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
      NewMemory((void **)&scase->fds_title, len_title+1);
      strcpy(scase->fds_title, fds_title_local);
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

      if(scase->sky_texture != NULL){
        FREEMEMORY(scase->sky_texture->file);
        FREEMEMORY(scase->sky_texture);
      }
      scase->nsky_texture = 1;
      NewMemory((void **)&scase->sky_texture, scase->nsky_texture * sizeof(texturedata));
      FGETS(buffer, 255, stream);
      buff2 = TrimFrontBack(buffer);
      len_buffer = strlen(buff2);
      scase->sky_texture->file = NULL;
      if(len_buffer > 0 && strcmp(buff2, "null") != 0){
         NewMemory((void **)&scase->sky_texture->file, (len_buffer + 1) * sizeof(char));
         strcpy(scase->sky_texture->file, buff2);
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
        ReadCADGeomToCollection(&scase->cadgeomcoll, bufferptr, scase->color_defs.block_shininess);
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
      InitSurface(surfi, scase->color_defs.block_ambient2);
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
          if(mesh_nabors[5]>=-1)scase->have_mesh_nabors = 1;
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
        if(scase->have_mesh_nabors == 1){
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
      scase->visFrame=0;
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

  scase->surfacedefault=&scase->sdefault;
  for(i=0;i<scase->surfcoll.nsurfinfo;i++){
    if(strcmp(scase->surfacedefaultlabel,scase->surfcoll.surfinfo[i].surfacelabel)==0){
      scase->surfacedefault=scase->surfcoll.surfinfo+i;
      break;
    }
  }
  scase->vent_surfacedefault=&scase->v_surfacedefault;
  for(i=0;i<scase->surfcoll.nsurfinfo;i++){
    if(strcmp(scase->vent_surfacedefault->surfacelabel,scase->surfcoll.surfinfo[i].surfacelabel)==0){
      scase->vent_surfacedefault=scase->surfcoll.surfinfo+i;
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
        sscanf(buffer,"%f",&scase->hrrpuvcut);
        scase->hrrpuvcut_set = 1;
        for(i=1;i<nhrrpuvcut;i++){
          FGETS(buffer,255,stream);
        }
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ HRRPUV_MINMAX ++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "HRRPUV_MINMAX") == 1){
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%f %f", &scase->hrrpuv_min, &scase->hrrpuv_max);
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TEMP_MINMAX ++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(MatchSMV(buffer, "TEMP_MINMAX") == 1){
      FGETS(buffer, 255, stream);
      sscanf(buffer, "%f %f", &scase->temp_min, &scase->temp_max);
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
        InitObst(scase,bc,scase->surfacedefault,nn+1,iobst-1);
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
        cvi->surf[0]=scase->vent_surfacedefault;
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
        scase->visFloor=0;
        scase->visCeiling=0;
        scase->visWalls=0;
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
        vi->surf[0]=scase->vent_surfacedefault;
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
          vi->color=scase->color_defs.ventcolor;
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

/* ------------------ InitScase ------------------------ */

/// @brief Initialize a smokeview case (smv_case) which has already been
/// allocated. This should be avoided and CreateScase/DestroyScase should be
/// used instead.
/// @param scase An uninitialized scase
void InitScase(smv_case *scase) {
  // set all of the defaults that are non-zero

  assert(scase->smoke3dcoll.nsmoke3dinfo == 0);
  assert(scase->smoke3dcoll.smoke3dinfo == NULL);
  assert(scase->smoke3dcoll.nsmoke3dtypes == 0);
  assert(scase->smoke3dcoll.smoke3dtypes == NULL);
  assert(scase->smoke3dcoll.smoke3d_other == 0);
  assert(scase->smoke3dcoll.smoke3dinfo_sorted == NULL);
  assert(scase->slicecoll.nsliceinfo == 0);
  assert(scase->slicecoll.sliceinfo == NULL);
  assert(scase->slicecoll.nmultisliceinfo == 0);
  assert(scase->slicecoll.multisliceinfo == NULL);
  assert(scase->slicecoll.nmultivsliceinfo == 0);
  assert(scase->slicecoll.multivsliceinfo == NULL);
  assert(scase->slicecoll.nvsliceinfo == 0);
  assert(scase->slicecoll.vsliceinfo == NULL);
  assert(NULL == 0);

  scase->smoke3dcoll.nsmoke3dinfo       = 0;
  scase->smoke3dcoll.smoke3dinfo        = NULL;
  scase->smoke3dcoll.nsmoke3dtypes      = 0;
  scase->smoke3dcoll.smoke3dtypes       = NULL;
  scase->smoke3dcoll.smoke3d_other      = 0;
  scase->smoke3dcoll.smoke3dinfo_sorted = NULL;
  scase->slicecoll.nsliceinfo       = 0;
  scase->slicecoll.sliceinfo        = NULL;
  scase->slicecoll.nmultisliceinfo  = 0;
  scase->slicecoll.multisliceinfo   = NULL;
  scase->slicecoll.nmultivsliceinfo = 0;
  scase->slicecoll.multivsliceinfo  = NULL;
  scase->slicecoll.nvsliceinfo      = 0;
  scase->slicecoll.vsliceinfo       = NULL;

  scase->tourcoll.ntourinfo = 0;
  scase->tourcoll.tourinfo = NULL;
  scase->tourcoll.tour_ntimes = 1000;
  scase->tourcoll.tour_t = NULL;
  scase->tourcoll.tour_t2 = NULL;
  scase->tourcoll.tour_dist = NULL;
  scase->tourcoll.tour_dist2 = NULL;
  scase->tourcoll.tour_dist3 = NULL;
  scase->tourcoll.tour_tstart = 0.0;
  scase->tourcoll.tour_tstop = 100.0;
  scase->fuel_hoc = -1.0;
  scase->fuel_hoc_default = -1.0;
  scase->have_cface_normals = CFACE_NORMALS_NO;
  scase->gvecphys[2] =  -9.8;
  scase->gvecunit[2] =  -1.0;
  scase->global_tbegin = 1.0;
  scase->smoke_albedo = 0.3;
  scase->smoke_albedo_base = 0.3;
  scase->hrrpuv_min = 0.0;
  scase->hrrpuv_max = 1200;
  scase->temp_min = 0.0;
  scase->temp_max = 2000;
  scase->xbar = 1.0;
  scase->ybar = 1.0;
  scase->zbar = 1.0;
  scase->show_slice_in_obst = ONLY_IN_GAS;
  scase->use_iblank = 1;
  scase->visOtherVents = 1;
  scase->visOtherVentsSAVE = 1;
  scase->hvac_duct_color[0] = 63;
  scase->hvac_duct_color[1] = 0;
  scase->hvac_duct_color[2] = 15;
  scase->hvac_node_color[0] = 63;
  scase->hvac_node_color[1] = 0;
  scase->hvac_node_color[2] = 15;
  scase->nrgb2 = 8;
  scase->pref = 101325.0;
  scase->pamb = 0.0;
  scase->tamb = 293.15;
  scase->nrgb = NRGB;
  scase->linewidth = 2.0;
  scase->ventlinewidth = 2.0;
  scase->obst_bounding_box[0] = 1.0;
  scase->obst_bounding_box[2] = 1.0;
  scase->obst_bounding_box[4] = 1.0;
  scase->hvaccoll.hvacductvar_index = -1;
  scase->hvaccoll.hvacnodevar_index = -1;
  // Initialize default colors
  scase->color_defs.block_shininess = 100.0;
  scase->color_defs.mat_specular2=GetColorPtr(scase, mat_specular_orig);
  scase->color_defs.mat_ambient2=GetColorPtr(scase, mat_ambient_orig);
  scase->color_defs.ventcolor=GetColorPtr(scase, ventcolor_orig);
  scase->color_defs.block_ambient2=GetColorPtr(scase, block_ambient_orig);
  scase->color_defs.block_specular2=GetColorPtr(scase, block_specular_orig);

  scase->visFrame = 1;

  InitLabelsCollection(&scase->labelscoll);

  InitObjectCollection(&scase->objectscoll);
}

/* ------------------ CreateScase ------------------------ */

/// @brief Create and initalize and a smokeview case (smv_case).
/// @return An initialized smv_case.
smv_case *CreateScase() {
  smv_case *scase;
  NewMemory((void **)&scase, sizeof(smv_case));
  memset(scase, 0, sizeof(smv_case));
  InitScase(scase);
  return scase;
}

/* ------------------ DestroyScase ------------------------ */

/// @brief Cleanup and free the memory of an smv_case.
/// @param scase An smv_case created with CreateScase.
void DestroyScase(smv_case *scase) {
  FreeObjectCollection(&scase->objectscoll);
  FreeCADGeomCollection(&scase->cadgeomcoll);
  FreeLabelsCollection(&scase->labelscoll);
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
