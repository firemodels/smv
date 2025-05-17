#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

#include "dmalloc.h"
#include "colorbars.h"
#include "datadefs.h"
#include "shared_structures.h"
#include "colorbar_defs.h"

#include "file_util.h"
#include "string_util.h"

void TrimBack(char *line);
int STRCMP(const char *s1, const char *s2);
void Rgb2Lab(unsigned char *rgb_arg, float *lab);
void Rgbf2Lab(float *rgbf_arg, float *lab);

/* ------------------ GetColorbar ------------------------ */

colorbardata *GetColorbar(colorbar_collection *colorbars, const char *menu_label){
  for(int i = 0; i < colorbars->ncolorbars; i++){
    colorbardata *cb = colorbars->colorbarinfo + i;
    if(strcmp(cb->menu_label, menu_label) == 0) return cb;
  }
  return NULL;
}

/* ------------------ AdjustColorBar ------------------------ */

void AdjustColorBar(colorbardata *cbi){
  int i;

  cbi->node_dist[0] = 0.0;
  for(i = 1; i < cbi->nnodes; i++){
    unsigned char *rgb1_local, *rgb2_local;
    float lab1[3], lab2[3], dist;

    rgb2_local = cbi->node_rgb + 3 * i;
    rgb1_local = rgb2_local - 3;
    Rgb2Lab(rgb1_local, lab1);
    Rgb2Lab(rgb2_local, lab2);
    float dx, dy, dz;

    DDIST3(lab1, lab2, dist);
    cbi->node_dist[i] = cbi->node_dist[i - 1] + dist;
  }

  float total_dist;
  int nnodes;

  total_dist = cbi->node_dist[cbi->nnodes - 1];
  nnodes = cbi->node_index[cbi->nnodes - 1];

  if(total_dist > 0.0){
    for(i = 1; i < cbi->nnodes - 1; i++){
      int inode;

      inode = nnodes * (cbi->node_dist[i] / total_dist);
      cbi->node_index[i] = inode;
    }
  }
  cbi->adjusted = 1;
}

/* ------------------ IsColorbarSplit ------------------------ */

int IsColorbarSplit(colorbardata *cbi){
  int i;

  for(i = 0; i < cbi->nnodes - 1; i++){
    if(cbi->node_index[i] + 1 == cbi->node_index[i + 1]) return 1;
    if(cbi->node_index[i] == cbi->node_index[i + 1]) return 1;
  }
  return 0;
}

/* ------------------ AdjustColorBarLab ------------------------ */

void AdjustColorBarLab(colorbardata *cbi){
  if(cbi->can_adjust == 0 || IsColorbarSplit(cbi) == 1) return;
  AdjustColorBar(cbi);
}

/* ------------------ FRgb2Lab ------------------------ */

void FRgb2Lab(float *rgb_arg, float *lab){

  // Convert RGB values to XYZ
  float var_R = rgb_arg[0] / 255.0f;
  float var_G = rgb_arg[1] / 255.0f;
  float var_B = rgb_arg[2] / 255.0f;

  if(var_R > 0.04045f){
    var_R = pow((var_R + 0.055f) / 1.055f, 2.4f);
  }
  else {
    var_R /= 12.92f;
  }
  if(var_G > 0.04045f){
    var_G = pow((var_G + 0.055f) / 1.055f, 2.4f);
  }
  else {
    var_G /= 12.92f;
  }
  if(var_B > 0.04045f){
    var_B = pow((var_B + 0.055f) / 1.055f, 2.4f);
  }
  else {
    var_B /= 12.92f;
  }

  var_R *= 100.0f;
  var_G *= 100.0f;
  var_B *= 100.0f;

  float X = var_R * 0.4124f + var_G * 0.3576f + var_B * 0.1805f;
  float Y = var_R * 0.2126f + var_G * 0.7152f + var_B * 0.0722f;
  float Z = var_R * 0.0193f + var_G * 0.1192f + var_B * 0.9505f;

  // Convert XYZ to CIELAB
  float var_X = X / 95.047f;
  float var_Y = Y / 100.0f;
  float var_Z = Z / 108.883f;

  if(var_X > 0.008856f){
    var_X = pow(var_X, 1.0f / 3.0f);
  }
  else {
    var_X = (7.787f * var_X) + (16.0f / 116.0f);
  }
  if(var_Y > 0.008856f){
    var_Y = pow(var_Y, 1.0f / 3.0f);
  }
  else {
    var_Y = (7.787f * var_Y) + (16.0f / 116.0f);
  }
  if(var_Z > 0.008856f){
    var_Z = pow(var_Z, 1.0f / 3.0f);
  }
  else {
    var_Z = (7.787f * var_Z) + (16.0f / 116.0f);
  }

  lab[0] = (116.0f * var_Y) - 16.0f;
  lab[1] = 500.0f * (var_X - var_Y);
  lab[2] = 200.0f * (var_Y - var_Z);
}

/* ------------------ Rgb2Dist ------------------------ */

void Rgb2Dist(colorbardata *cbi){
  int i;

  float total_dist, *colorbar_dist;
  int jstart, *dist_ind;

  colorbar_dist = cbi->colorbar_dist;
  dist_ind = cbi->dist_ind;

  colorbar_dist[0] = 0.0;
  for(i = 1; i < 256; i++){
    float dist_lab, lab2[3], *rgb1f, *rgb2f, lab1[3];
    float dx, dy, dz;

    rgb1f = cbi->colorbar_rgb + 3 * (i - 1);
    rgb2f = cbi->colorbar_rgb + 3 * i;
    Rgbf2Lab(rgb1f, lab1);
    Rgbf2Lab(rgb2f, lab2);
    DDIST3(lab1, lab2, dist_lab);
    colorbar_dist[i] = colorbar_dist[i - 1] + dist_lab;
  }
  total_dist = colorbar_dist[255];

  dist_ind[0] = 1;
  dist_ind[255] = 1;
  for(i = 1; i < 255; i++){
    dist_ind[i] = 0;
  }
  jstart = 0;
  for(i = 1; i < 16; i++){
    float val;
    int j;

    val = (float)i * total_dist / 16.0;
    for(j = jstart; j < 255; j++){
      if(colorbar_dist[j] <= val && val <= colorbar_dist[j + 1]){
        dist_ind[j] = 1;
        jstart = j;
        break;
      }
    }
  }
}

/* ------------------ Lab2Rgb ------------------------ */

void Lab2Rgb(unsigned char *rgb_arg, float *frgb_arg, float *lab){
  float L, a, b;

  L = lab[0];
  a = lab[1];
  b = lab[2];

  // Convert CIELAB to XYZ
  float var_Y = (L + 16.0f) / 116.0f;
  float var_X = a / 500.0f + var_Y;
  float var_Z = var_Y - b / 200.0f;

  if(pow(var_Y, 3.0f) > 0.008856f){
    var_Y = pow(var_Y, 3.0f);
  }
  else {
    var_Y = (var_Y - 16.0f / 116.0f) / 7.787f;
  }
  if(pow(var_X, 3.0f) > 0.008856f){
    var_X = pow(var_X, 3.0f);
  }
  else {
    var_X = (var_X - 16.0f / 116.0f) / 7.787f;
  }
  if(pow(var_Z, 3.0f) > 0.008856f){
    var_Z = pow(var_Z, 3.0f);
  }
  else {
    var_Z = (var_Z - 16.0f / 116.0f) / 7.787f;
  }

  float X = var_X * 0.95047f;
  float Y = var_Y;
  float Z = var_Z * 1.08883f;

  // Convert XYZ to RGB
  float var_R = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
  float var_G = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
  float var_B = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;

  if(var_R > 0.0031308f){
    var_R = 1.055f * pow(var_R, 1.0f / 2.4f) - 0.055f;
  }
  else {
    var_R *= 12.92f;
  }
  if(var_G > 0.0031308f){
    var_G = 1.055f * pow(var_G, 1.0f / 2.4f) - 0.055f;
  }
  else {
    var_G *= 12.92f;
  }
  if(var_B > 0.0031308f){
    var_B = 1.055f * pow(var_B, 1.0f / 2.4f) - 0.055f;
  }
  else {
    var_B *= 12.92f;
  }

  frgb_arg[0] = CLAMP(var_R * 255.0f, 0.0, 255.0f);
  frgb_arg[1] = CLAMP(var_G * 255.0f, 0.0, 255.0f);
  frgb_arg[2] = CLAMP(var_B * 255.0f, 0.0, 255.0f);
  rgb_arg[0] = (unsigned char)CLAMP(frgb_arg[0] + 0.5, 0, 255);
  rgb_arg[1] = (unsigned char)CLAMP(frgb_arg[1] + 0.5, 0, 255);
  rgb_arg[2] = (unsigned char)CLAMP(frgb_arg[2] + 0.5, 0, 255);
}

/* ------------------ Rgb2Lab ------------------------ */

void Rgb2Lab(unsigned char *rgb_arg, float *lab){
  float frgb_arg[3];

  frgb_arg[0] = (float)rgb_arg[0];
  frgb_arg[1] = (float)rgb_arg[1];
  frgb_arg[2] = (float)rgb_arg[2];
  FRgb2Lab(frgb_arg, lab);
}

/* ------------------ Rgbf2Lab ------------------------ */

void Rgbf2Lab(float *rgbf_arg, float *lab){
  float frgb_arg[3];

  frgb_arg[0] = rgbf_arg[0] * 255.0;
  frgb_arg[1] = rgbf_arg[1] * 255.0;
  frgb_arg[2] = rgbf_arg[2] * 255.0;
  FRgb2Lab(frgb_arg, lab);
}

/* ------------------ Lab2XYZ ------------------------ */

void Lab2XYZ(float *xyz, float *lab){
  xyz[0] = lab[0] / 100.0;
  xyz[1] = (lab[1] + 87.9) / 183.28;
  xyz[2] = (lab[2] + 126.39) / 211.11;
}

// matches following website
// http://colormine.org/convert/rgb-to-lab

/* ------------------ CheckLab ------------------------ */

void CheckLab(void){
  int i, diff;
  int hist[256];
  float sum = 0.0;
  float *labxyz;
  float *lab_check_xyz;
  unsigned char *labrgb, *lab_check_rgb255;

  for(i = 0; i < 256; i++){
    hist[i] = 0;
  }

  NewMemory((void **)&lab_check_xyz, 3 * 17 * 17 * 17 * sizeof(float));
  NewMemory((void **)&lab_check_rgb255, 3 * 17 * 17 * 17);
  labxyz = lab_check_xyz;
  labrgb = lab_check_rgb255;
  for(i = 0; i < 256; i++){
    int j;

    printf("i=%i\n", i);
    for(j = 0; j < 256; j++){
      int k;

      for(k = 0; k < 256; k++){
        unsigned char rgbval[3], rgbnew[3];
        float lab[3], lab2[3], dist2, frgb[3];

        rgbval[0] = (unsigned char)k;
        rgbval[1] = (unsigned char)j;
        rgbval[2] = (unsigned char)i;
        Rgb2Lab(rgbval, lab);
        Lab2Rgb(rgbnew, frgb, lab);
        Rgb2Lab(rgbnew, lab2);
        diff = ABS(rgbval[0] - rgbnew[0]);
        diff = MAX(diff, ABS(rgbval[1] - rgbnew[1]));
        diff = MAX(diff, ABS(rgbval[2] - rgbnew[2]));
        dist2 = ABS(lab2[0] - lab[0]);
        dist2 = MAX(dist2, ABS(lab2[1] - lab[1]));
        dist2 = MAX(dist2, ABS(lab2[2] - lab[2]));
        sum += dist2;
        hist[diff]++;
      }
    }
  }
  for(i = 0; i <= 256; i += 16){
    int j;

    for(j = 0; j <= 256; j += 16){
      int k;

      for(k = 0; k <= 256; k += 16){
        unsigned char rgbval[3];
        float lab[3];

        rgbval[0] = MIN((unsigned char)k, 255);
        rgbval[1] = MIN((unsigned char)j, 255);
        rgbval[2] = MIN((unsigned char)i, 255);
        Rgb2Lab(rgbval, lab);
        memcpy(labxyz, lab, 3 * sizeof(float));
        memcpy(labrgb, rgbval, 3);
        labxyz += 3;
        labrgb += 3;
      }
    }
  }
  for(i = 0; i < 256; i++){
    printf("%i ", hist[i]);
  }
  printf("\n");
  printf("lab avg diff=%f\n", sum / (float)(256 * 256 * 256));
  FREEMEMORY(lab_check_xyz);
  FREEMEMORY(lab_check_rgb255);
}

/* ------------------ GetColorDist ------------------------ */

void GetColorDist(colorbardata *cbi, int option, float *min, float *max){
  int i;

  for(i = 1; i < 255; i++){
    cbi->colorbar_dist_delta[i - 1] =
        cbi->colorbar_dist[i] - cbi->colorbar_dist[i - 1];
  }
  *min = cbi->colorbar_dist_delta[0];
  *max = *min;
  for(i = 1; i < 255 - 1; i++){
    *min = MIN(*min, cbi->colorbar_dist_delta[i]);
    *max = MAX(*max, cbi->colorbar_dist_delta[i]);
  }
}

/* ------------------ SetAlpha ------------------------ */

unsigned char SetAlpha(unsigned char *node_rgb){
  if((node_rgb[0] == 0 && node_rgb[1] == 1 && node_rgb[2] == 2) ||
      (node_rgb[0] == 253 && node_rgb[1] == 254 && node_rgb[2] == 255)){
    return 0;
  }
  return 255;
}

/* ------------------ RemapColorbar ------------------------ */

void RemapColorbar(colorbardata *cbi, int show_extreme_mindata,
                   unsigned char rgb_below_min[3], int show_extreme_maxdata,
                   unsigned char rgb_above_max[3]){
  int i;
  float *colorbar_rgb;
  unsigned char *node_rgb;
  unsigned char *colorbar_alpha;
  float *colorbar_lab;
  int interp;

  interp = cbi->interp;
  CheckMemory;
  colorbar_rgb = cbi->colorbar_rgb;
  node_rgb = cbi->node_rgb;
  colorbar_lab = cbi->colorbar_lab;
  colorbar_alpha = cbi->colorbar_alpha;

  AdjustColorBarLab(cbi);

  for(i = 0; i < cbi->node_index[0]; i++){
    colorbar_rgb[0 + 3 * i] = node_rgb[0] / 255.0;
    colorbar_rgb[1 + 3 * i] = node_rgb[1] / 255.0;
    colorbar_rgb[2 + 3 * i] = node_rgb[2] / 255.0;
    colorbar_alpha[i] = SetAlpha(node_rgb);
  }
  for(i = 0; i < cbi->nnodes - 1; i++){
    int i1, i2, j;
    float lab1[3], lab2[3];

    i1 = cbi->node_index[i];
    i2 = cbi->node_index[i + 1];
    if(i2 == i1) continue;
    node_rgb = cbi->node_rgb + 3 * i;
    Rgb2Lab(node_rgb, lab1);
    Rgb2Lab(node_rgb + 3, lab2);
    for(j = i1; j < i2; j++){
      float factor;

      factor = (float)(j - i1) / (float)(i2 - i1);
      float *labj;

      labj = colorbar_lab + 3 * j;
      labj[0] = MIX(factor, lab2[0], lab1[0]);
      labj[1] = MIX(factor, lab2[1], lab1[1]);
      labj[2] = MIX(factor, lab2[2], lab1[2]);
      if(interp == INTERP_LAB){
        unsigned char rgb_val[3];
        float frgb[3];

        Lab2Rgb(rgb_val, frgb, labj);
        colorbar_rgb[0 + 3 * j] = frgb[0] / 255.0;
        colorbar_rgb[1 + 3 * j] = frgb[1] / 255.0;
        colorbar_rgb[2 + 3 * j] = frgb[2] / 255.0;
      }
      else {
        colorbar_rgb[0 + 3 * j] = MIX(factor, node_rgb[3], node_rgb[0]) / 255.0;
        colorbar_rgb[1 + 3 * j] = MIX(factor, node_rgb[4], node_rgb[1]) / 255.0;
        colorbar_rgb[2 + 3 * j] = MIX(factor, node_rgb[5], node_rgb[2]) / 255.0;
      }
      colorbar_alpha[j] = SetAlpha(node_rgb);
    }
  }
  node_rgb = cbi->node_rgb + 3 * (cbi->nnodes - 1);
  for(i = cbi->node_index[cbi->nnodes - 1]; i < 256; i++){
    colorbar_rgb[0 + 3 * i] = node_rgb[0] / 255.0;
    colorbar_rgb[1 + 3 * i] = node_rgb[1] / 255.0;
    colorbar_rgb[2 + 3 * i] = node_rgb[2] / 255.0;
    colorbar_alpha[i] = SetAlpha(node_rgb);
  }
  if(show_extreme_mindata == 1){
    colorbar_rgb[0] = rgb_below_min[0];
    colorbar_rgb[1] = rgb_below_min[1];
    colorbar_rgb[2] = rgb_below_min[2];
  }
  if(show_extreme_maxdata == 1){
    colorbar_rgb[0 + 3 * 255] = rgb_above_max[0];
    colorbar_rgb[1 + 3 * 255] = rgb_above_max[1];
    colorbar_rgb[2 + 3 * 255] = rgb_above_max[2];
  }
  Rgb2Dist(cbi);
  CheckMemory;
}

/* ------------------ ReadCSVColorbar ------------------------ */

int ReadCSVColorbar(colorbardata *colorbar, const char *filepath,
                     const char *colorbar_type, int type){
  int n = 0;
  char buffer[255];
  int have_name = 0;

  // Open the file
  FILE *stream = fopen(filepath, "r");
  if(stream == NULL) return 1;
  if(fgets(buffer, 255, stream) == NULL){
    fclose(stream);
    return 1;
  }
  char *fname_buffer;
  NEWMEMORY(fname_buffer, (strlen(filepath) + 1) * sizeof(char));
  GetBaseFileName(fname_buffer, filepath);
  strcpy(colorbar->menu_label, filepath);
  FREEMEMORY(fname_buffer);
  colorbar->nodehilight = 0;

  // If the first field of the first line is alphanumeric, there's a name and
  // the value of the second field should be used as a name.
  char *field1 = strtok(buffer, ",");
  if(field1 != NULL && isalpha(field1[0]) != 0){
    have_name = 1;
    char *field2 = strtok(NULL, ",");
    if(field2 != NULL){
      strcpy(colorbar->menu_label, field2);
      TrimBack(colorbar->menu_label);
    }
  }
  rewind(stream);
  if(have_name == 1){
    fgets(buffer, 255, stream);
    TrimBack(buffer);
  }
  // Count the lines in the file TODO: rather than read the file twice (to count
  // lines), just reallocate memory when needed.
  for(;;){
    if(fgets(buffer, 255, stream) == NULL) break;
    n++;
  }
  rewind(stream);
  // TODO: we don't need to allocate rgbscopy has it only needs to be 3 chars
  // long.
  int *rgbs, *rgbscopy;
  NewMemory((void **)&rgbs, 3 * n * sizeof(int));
  rgbscopy = rgbs;

  if(have_name == 1){
    fgets(buffer, 255, stream);
    TrimBack(buffer);
  }
  for(int i = 0; i < n; i++){
    char *crgb;

    if(fgets(buffer, 255, stream) == NULL) break;
    TrimBack(buffer);
    crgb = strtok(buffer, ",");
    sscanf(crgb, "%i", rgbscopy);
    crgb = strtok(NULL, ",");
    sscanf(crgb, "%i", rgbscopy + 1);
    crgb = strtok(NULL, ",");
    sscanf(crgb, "%i", rgbscopy + 2);
    colorbar->node_rgb[3 * i + 0] = (unsigned char)CLAMP(rgbscopy[0], 0, 255);
    colorbar->node_rgb[3 * i + 1] = (unsigned char)CLAMP(rgbscopy[1], 0, 255);
    colorbar->node_rgb[3 * i + 2] = (unsigned char)CLAMP(rgbscopy[2], 0, 255);
    colorbar->node_index[i] = i;
    strcpy(colorbar->colorbar_type, colorbar_type);
    colorbar->type = type;
    rgbscopy += 3;
  }
  colorbar->nnodes = n;
  fclose(stream);
  return 0;
}

/* ------------------ CreateColorbar ------------------------ */

colorbardata *CreateColorbar(void){
  colorbardata *colorbar;
  NewMemory((void **)&colorbar, sizeof(colorbardata));
  return colorbar;
}

/* ------------------ FreeColorbar ------------------------ */

void FreeColorbar(colorbardata *colorbar){
  FreeMemory(colorbar);
}

/* ------------------ CreateColorbarFromCsv ------------------------ */

colorbardata *CreateColorbarFromCsv(const char *filepath, char *colorbar_type, int type){
  colorbardata *colorbar = CreateColorbar();
  ReadCSVColorbar(colorbar, filepath, colorbar_type, type);
  return colorbar;
}

/* ------------------ NextColorbar ------------------------ */

colorbardata *NextColorbar(colorbar_collection *colorbars){
  if(colorbars->colorbarinfo == NULL){
    size_t new_capacity = 1;
    NEWMEMORY(colorbars->colorbarinfo, new_capacity * sizeof(colorbardata));
    colorbars->capacity = new_capacity;
  }
  else if(colorbars->ncolorbars >= colorbars->capacity){
    size_t new_capacity = MAX(colorbars->capacity * 2, 1);
    CheckMemory;
    RESIZEMEMORY(colorbars->colorbarinfo, new_capacity * sizeof(colorbardata));
    colorbars->capacity = new_capacity;
  }
  return &colorbars->colorbarinfo[colorbars->ncolorbars];
}

/* ------------------ NewColorbar ------------------------ */

colorbardata *NewColorbar(colorbar_collection *colorbars){
  colorbardata *cb = NextColorbar(colorbars);
  memset(cb, 0, sizeof(colorbardata));
  colorbars->ncolorbars++;
  return cb;
}

/* ------------------ GetColorbarsSubDir ------------------------ */

/**
 * @brief Get a path for a colorbar subdir. This is generally in the form
 * ${SMV_ROOT_DIR}/colorbars/${subdir}.
 *
 * @param subdir The name of the subdir
 * @return Path to directory (allocated via NEWMEMORY) or NULL if subdir is NULL
 * or if GetSmvRootDir returns NULL.
 */
char *GetColorbarsSubDir(const char *subdir) {
  char *return_path = NULL;
  char *smv_bindir = GetSmvRootDir();
  if(smv_bindir == NULL || subdir == NULL) return return_path;
  char *colorbar_dir = CombinePaths(smv_bindir, "colorbars");
  if(strlen(subdir) > 0) {
    return_path = CombinePaths(colorbar_dir, subdir);
    FREEMEMORY(colorbar_dir);
  }
  else {
    return_path = colorbar_dir;
  }
  FREEMEMORY(smv_bindir);
  return return_path;
}

/* ------------------ ReadColorbarDir ------------------------ */

void ReadColorbarDir(colorbar_collection *colorbars, const char *dir_path,
                     const char *label, int type){
  filelistdata *filelist = NULL;
  int n_files = GetFileListSize(dir_path, "*.csv", FILE_MODE);
  MakeFileList(dir_path, "*.csv", n_files, NO, &filelist, FILE_MODE);
  for(int i = 0; i < n_files; i++){
    colorbardata *cbi = NextColorbar(colorbars);

    if(filelist[i].file == NULL || strlen(filelist[i].file) == 0) return;
    if(dir_path == NULL || strlen(dir_path) == 0) return;
    char *filepath = CombinePaths(dir_path, filelist[i].file);
    ReadCSVColorbar(cbi, filepath, label, type);
    colorbars->ncolorbars++;
    cbi->can_adjust = 1;
    FREEMEMORY(filepath);
  }
}

/* ------------------ ReadColorbarSubDir ------------------------ */

void ReadColorbarSubDir(colorbar_collection *colorbars, const char *subdir, int type){
  char *dir_path = GetColorbarsSubDir(subdir);
  ReadColorbarDir(colorbars, dir_path, subdir, type);
  FREEMEMORY(dir_path);
}

/* ------------------ InitDefaultColorbars ------------------------ */

void InitDefaultColorbars(colorbar_collection *colorbars, int nini,
                                    int show_extreme_mindata,
                                    unsigned char rgb_below_min[3],
                                    int show_extreme_maxdata,
                                    unsigned char rgb_above_max[3],
                                    colorbardata **colorbarcopyinfoptr){

  // Add colorbars as defined in the code.
  CreateColorbarRainbow(NewColorbar(colorbars));
  CreateColorbarOriginalRainbow(NewColorbar(colorbars));
  CreateColorbarRainbow2(NewColorbar(colorbars));
  CreateColorbarYellowRed(NewColorbar(colorbars));
  CreateColorbarBlueRedGreen(NewColorbar(colorbars));
  CreateColorbarBlueYellowWhite(NewColorbar(colorbars));
  CreateColorbarBlueRedSplit(NewColorbar(colorbars));
  CreateColorbarAfacSplit(NewColorbar(colorbars));
  CreateColorbarBlackWhite(NewColorbar(colorbars));
  CreateColorbarFed(NewColorbar(colorbars));
  CreateColorbarFireOriginal(NewColorbar(colorbars));
  CreateColorbarFire2(NewColorbar(colorbars));
  CreateColorbarFire3(NewColorbar(colorbars));
  CreateColorbarCool(NewColorbar(colorbars));
  CreateColorbarFireLineLevelSet(NewColorbar(colorbars));
  CreateColorbarFireLineWallThickness(NewColorbar(colorbars));
  CreateColorbarSplit(NewColorbar(colorbars));
  CreateColorbarMethanol(NewColorbar(colorbars));
  CreateColorbarPropane(NewColorbar(colorbars));
  CreateColorbarCo2(NewColorbar(colorbars));

  // Add colorbars as found in the root directory
  ReadColorbarSubDir(colorbars, "linear", CB_LINEAR);
  ReadColorbarSubDir(colorbars, "circular", CB_CIRCULAR);
  ReadColorbarSubDir(colorbars, "rainbow", CB_RAINBOW);
  ReadColorbarSubDir(colorbars, "divergent", CB_DIVERGENT);

  // Add user colorbars as defined in the config directory.
  char *colorbars_user_dir = GetUserColorbarDirPath();
  ReadColorbarDir(colorbars, colorbars_user_dir, "user defined", CB_USER);
  FREEMEMORY(colorbars_user_dir);

  colorbars->ndefaultcolorbars = colorbars->ncolorbars;

  for(int i = 0; i < colorbars->ncolorbars; i++){
    colorbardata *cbi = colorbars->colorbarinfo + i;
    cbi->can_adjust = 1;
    cbi->adjusted = 0;
  }

  // construct colormaps from color node info

  for(int i = 0; i < colorbars->ndefaultcolorbars; i++){
    colorbardata *cbi = colorbars->colorbarinfo + i;

    if(cbi->can_adjust == 1){
      cbi->interp = INTERP_LAB;
    }
    else {
      cbi->interp = INTERP_RGB;
    }
    RemapColorbar(cbi, show_extreme_mindata, rgb_below_min,
                  show_extreme_maxdata, rgb_above_max);
    memcpy(cbi->node_rgb_orig, cbi->node_rgb,
           3 * cbi->nnodes * sizeof(unsigned char));
  }

  for(int i = 0; i < colorbars->ncolorbars; i++){
    colorbardata *cbi = colorbars->colorbarinfo + i;
    cbi->interp = INTERP_LAB;
    if(cbi->can_adjust == 1){
      AdjustColorBar(cbi);
    }
  }
  colorbardata *colorbarcopy;
  if(*colorbarcopyinfoptr != NULL){
    FREEMEMORY(*colorbarcopyinfoptr);
  }
  NEWMEMORY(colorbarcopy, colorbars->ncolorbars * sizeof(colorbardata));
  memcpy(colorbarcopy, colorbars->colorbarinfo,
         colorbars->ncolorbars * sizeof(colorbardata));
  *colorbarcopyinfoptr = colorbarcopy;
}

/* ------------------ GetColorPtr ------------------------ */

float *GetColorPtr(smv_case *scase, float *color){
  colordata *colorptr,*oldlastcolor,*lastcolor;

  if(scase->firstcolor==NULL){
    NewMemory((void *)&scase->firstcolor,sizeof(colordata));
    memcpy(scase->firstcolor->color,      color, 4*sizeof(float));
    memcpy(scase->firstcolor->full_color, color, 4*sizeof(float));
    scase->firstcolor->bw_color[0] = TOBW(color);
    scase->firstcolor->bw_color[1] = scase->firstcolor->bw_color[0];
    scase->firstcolor->bw_color[2] = scase->firstcolor->bw_color[0];
    scase->firstcolor->bw_color[3] = color[3];
    scase->firstcolor->nextcolor=NULL;
    return scase->firstcolor->color;
  }
  oldlastcolor = scase->firstcolor;
  for(colorptr = scase->firstcolor; colorptr!=NULL; colorptr = colorptr->nextcolor){
    oldlastcolor=colorptr;
    if(ABS(colorptr->color[0]-color[0])>0.0001)continue;
    if(ABS(colorptr->color[1]-color[1])>0.0001)continue;
    if(ABS(colorptr->color[2]-color[2])>0.0001)continue;
    if(ABS(colorptr->color[3]-color[3])>0.0001)continue;
    return colorptr->color;
  }
  lastcolor=NULL;
  NewMemory((void *)&lastcolor,sizeof(colordata));
  oldlastcolor->nextcolor=lastcolor;
  memcpy(lastcolor->color,      color, 4*sizeof(float));
  memcpy(lastcolor->full_color, color, 4*sizeof(float));
  lastcolor->bw_color[0] = TOBW(color);
  lastcolor->bw_color[1] = lastcolor->bw_color[0];
  lastcolor->bw_color[2] = lastcolor->bw_color[0];
  lastcolor->bw_color[3] = color[3];
  lastcolor->nextcolor=NULL;
  return lastcolor->color;
}
