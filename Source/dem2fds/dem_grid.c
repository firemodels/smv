#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "string_util.h"
#include "file_util.h"
#include "datadefs.h"
#include "MALLOCC.h"
#include "gd.h"
#include "dem_grid.h"
#include "dem_util.h"
/* ------------------ ReadGridHeader ------------------------ */

int ReadGridHeader(griddata *data){
  FILE *stream = NULL;

  stream = fopen(data->file, "r");
  if(stream==NULL)return 0;

  for(;;){
    char *blank, *val, buffer[255];

    if(fgets(buffer, 255, stream)==NULL)break;
    TrimBack(buffer);
    blank = strchr(buffer, ' ');
    if(blank==NULL)break;
    val = blank+1;
    blank[0] = 0;
    if(strcmp(buffer, "ncols")==0){
      sscanf(val, "%i", &(data->ncols));
      data->have_ncols = 1;
      continue;
    }
    if(strcmp(buffer, "nrows")==0){
      sscanf(val, "%i", &(data->nrows));
      data->have_nrows = 1;
      continue;
    }
    if(strcmp(buffer, "xllcorner")==0){
      sscanf(val, "%f", &(data->xllcorner));
      data->have_xllcorner = 1;
      continue;
    }
    if(strcmp(buffer, "yllcorner")==0){
      sscanf(val, "%f", &(data->yllcorner));
      data->have_yllcorner = 1;
      continue;
    }
    if(strcmp(buffer, "latmin")==0){
      sscanf(val, "%f", &(data->latmin));
      data->have_latmin = 1;
      continue;
    }
    if(strcmp(buffer, "latmax")==0){
      sscanf(val, "%f", &(data->latmax));
      data->have_latmax = 1;
      continue;
    }
    if(strcmp(buffer, "longmin")==0){
      sscanf(val, "%f", &(data->longmin));
      data->have_longmin = 1;
      continue;
    }
    if(strcmp(buffer, "longmax")==0){
      sscanf(val, "%f", &(data->longmax));
      data->have_longmax = 1;
      continue;
    }
    if(strcmp(buffer, "dx")==0){
      sscanf(val, "%f", &(data->dx));
      data->have_dx = 1;
      continue;
    }
    if(strcmp(buffer, "dy")==0){
      sscanf(val, "%f", &(data->dy));
      data->have_dy = 1;
      continue;
    }
    if(strcmp(buffer, "cellsize")==0){
      sscanf(val, "%f", &(data->cellsize));
      data->have_cellsize = 1;
      continue;
    }
    if(strcmp(buffer, "NODATA_value")==0)break;
    if(buffer[0]>='a'&&buffer[0]<='z')continue;
    if(buffer[0]>='A'&&buffer[0]<='A')continue;
    break;
  }

  if(data->type!=GRID_IMAGE_DATA&&data->ncols>0&&data->nrows>0){
    char *buffer;

    int size_buffer = 40*data->ncols;
    NewMemory((void **)&buffer, size_buffer*sizeof(char));
    fgets(buffer, size_buffer, stream);
    if(strchr(buffer, '.')!=NULL){
      data->type = GRID_FLOAT_DATA;
    }
    else{
      data->type = GRID_INT_DATA;
    }
    FREEMEMORY(buffer);
  }

  if(data->type!=GRID_IMAGE_DATA){
    if(data->have_cellsize==1){
      data->dx = data->cellsize;
      data->dy = data->cellsize;
    }
    if(data->have_xllcorner==1){
      data->longmin = data->xllcorner;
      data->longmax = data->longmin+(float)data->ncols*data->dx;
      data->latmin = data->yllcorner;
      data->latmax = data->latmin+(float)data->nrows*data->dy;
    }
  }

  fclose(stream);

  if(data->type==GRID_IMAGE_DATA){
    char *image_file;

    char *file = data->file;
    if(file==NULL||strlen(file)==0)return 0;
    NewMemory((void **)&image_file, (strlen(file)+6)*sizeof(char));
    strcpy(image_file, file);
    char *ext = strrchr(image_file, '.');
    if(ext==NULL){
      FREEMEMORY(image_file);
      return 0;
    }

    *ext = 0;
    strcat(image_file, ".jpg");
    data->image_file = image_file;
    data->image = NULL;
    gdImagePtr image = GetJPEGImage(data->image_file, &data->ncols, &data->nrows);
    if(image==NULL)return 0;
    gdImageDestroy(image);
  }
  return 1;
}

/* ------------------ AllocateGridData ------------------------ */

int AllocateGridData(griddata *data){
  int size, size_data;

  size = data->ncols*data->nrows;
  if(size<=0)return 0;

  if(data->type==GRID_INT_DATA){
    size_data = size*sizeof(int);
  }
  else if(data->type==GRID_FLOAT_DATA){
    size_data = size*sizeof(float);
  }
  else{
    ASSERT(0);
    return 0;
  }

  NewMemory((void **)&(data->vals), size_data);
  return 1;
}

/* ------------------ InitGridData ------------------------ */

griddata *InitGridData(char *file, char *filemode){
  griddata *data;

  if(file==NULL||strlen(file)==0)return NULL;

  NewMemory((void **)&(data), sizeof(griddata));
  NewMemory((void **)&(data->file), strlen(file)+1);
  strcpy(data->file, file);

  data->type = 0;
  if(strcmp(filemode,"image")==0){
    data->type=GRID_IMAGE_DATA;
  }
  data->vals = NULL;
  data->ncols = -1;
  data->have_ncols = 0;

  data->nrows = -1;
  data->have_nrows = 0;

  data->xllcorner = 0.0;
  data->have_xllcorner = 0;

  data->yllcorner = 0.0;
  data->have_yllcorner = 0;

  data->cellsize = -1.0;
  data->have_cellsize = 0;

  data->dx = -1.0;
  data->have_dx = 0;

  data->dy = -1.0;
  data->have_dy = 0;

  data->have_longmin = 0;
  data->have_longmax = 0;
  data->have_latmin = 0;
  data->have_latmax = 0;

  if(ReadGridHeader(data)==0){
    FREEMEMORY(data->file);
    FREEMEMORY(data);
    return NULL;
  }
  if(data->type!=GRID_IMAGE_DATA&&AllocateGridData(data)==0){
    FREEMEMORY(data->file);
    FREEMEMORY(data);
    return NULL;
  }
    
  return data;
}

/* ------------------ CopyGridData ------------------------ */

int CopyGridData(griddata *data, char *file){
  if(file==NULL||strlen(file)==0||data->ncols<=0||data->nrows<=0)return 0;
  int size = data->ncols*data->nrows;

  FILE *stream = fopen(file, "wb");
  if(stream==NULL)return 0;

  fwrite(&(data->type), sizeof(int), 1, stream);
  fwrite(&(data->ncols), sizeof(int), 1, stream);
  fwrite(&(data->nrows), sizeof(int), 1, stream);
  fwrite(&(data->xllcorner), sizeof(float),1, stream);
  fwrite(&(data->yllcorner), sizeof(float),1, stream);
  fwrite(&(data->dx), sizeof(float), 1, stream);
  fwrite(&(data->dy), sizeof(float), 1, stream);
  if(data->type==GRID_INT_DATA){
    fwrite((int *)data->vals, sizeof(int), size,stream);
  }
  else if(data->type==GRID_FLOAT_DATA){
    fwrite((float *)data->vals, sizeof(float), size,stream);
  }
  else{
    ASSERT(0);
    fclose(stream);
    return 0;
  }
  fclose(stream);
  return 1;
}

/* ------------------ FreeGridData ------------------------ */

void FreeGridData(griddata *data){
  FREEMEMORY(data->file);
  FREEMEMORY(data->vals);
  FREEMEMORY(data);
}

  /* ------------------ ReadGridData ------------------------ */

griddata *ReadGridData(char *directory, char *file, char *mode){
  int i;
  FILE *stream;
  char *buffer=NULL;
  int size_buffer = 256;
  griddata *data;
  char fullfile[256];

  if(file==NULL||strlen(file)==0)return NULL;

  if(directory==NULL||strlen(directory)==0||strcmp(directory, ".")==0){
    strcpy(fullfile, file);
  }
  else{
    strcpy(fullfile, directory);
    strcat(fullfile, dirseparator);
    strcat(fullfile, file);
  }

  data = InitGridData(fullfile,mode);
  if(data==NULL)return NULL;
  if(data->type==GRID_IMAGE_DATA)return data;
    
  stream = fopen(data->file, "r");
  if(stream==NULL||
    (data->type==GRID_FLOAT_DATA&&strcmp(mode, "float")!=0)||
    (data->type==GRID_INT_DATA&&strcmp(mode, "int")!=0)
    ){
    FreeGridData(data);
    return NULL;
  }

  NewMemory((void **)&buffer, size_buffer*sizeof(char));

  // skip over header
  for(;;){
    char *blank;

    fgets(buffer, size_buffer, stream);
    blank = strchr(buffer, ' ');
    if(blank==NULL){
      fclose(stream);
      FreeGridData(data);
      return NULL;
    }
    blank[0] = 0;
    if(strcmp(buffer, "NODATA_value")==0)break;
    if(buffer[0]>='a'&&buffer[0]<='z')continue;
    if(buffer[0]>='A'&&buffer[0]<='A')continue;
    break;
  }

  FREEMEMORY(buffer);

  if(data->type==GRID_INT_DATA){
    size_buffer = 5*data->ncols;
  }
  else if(data->type==GRID_FLOAT_DATA){
    size_buffer = 40*data->ncols;
  }
  else{
    fclose(stream);
    FreeGridData(data);
    ASSERT(0);
    return NULL;
  }
  
  NewMemory((void **)&buffer, size_buffer*sizeof(char));
  float *fvals = (float *)(data->vals);
  int *ivals = (int *)(data->vals);

  for(i = 0; i<data->nrows; i++){
    int j;
    char *tok;

    if(fgets(buffer, size_buffer, stream)==NULL)break;
    tok = strtok(buffer, " ");
    if(data->type==GRID_FLOAT_DATA){
      for(j = 0; j<data->ncols; j++){
        sscanf(tok, "%f", (float *)fvals);
        tok = strtok(NULL, " ");
        if(*fvals<10.0){
          *fvals = -10.0;
        }
        fvals++;
      }
    }
    else if(data->type==GRID_INT_DATA){
      for(j = 0; j<data->ncols; j++){
        sscanf(tok, "%i", (int *)ivals);
        tok = strtok(NULL, " ");
        ivals++;
      }
    }
  }
  FREEMEMORY(buffer);
  return data;
}
