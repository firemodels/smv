#include "options_common.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif
#include GLU_H
#include GL_H

#include "dmalloc.h"
#include "file_util.h"
#include "string_util.h"

#include "isobox.h"
#include "readcad.h"
#include "readimage.h"

/* ------------------ CalcQuadNormal ------------------------ */

void CalcQuadNormal(float *xyz, float *out) {
  float u[3], v[3];
  static const int x = 0;
  static const int y = 1;
  static const int z = 2;
  float *p1, *p2, *p3;
  float *pp1, *pp2, *pp3, *pp4;

  pp1 = xyz;
  pp2 = xyz + 3;
  pp3 = xyz + 6;
  pp4 = xyz + 9;

  p1 = pp1;
  p2 = pp2;
  p3 = pp3;

  if(pp1[0] == pp2[0] && pp1[1] == pp2[1] && pp1[2] == pp2[2]) {
    p1 = pp2;
    p2 = pp3;
    p3 = pp4;
  }
  if(pp2[0] == pp3[0] && pp2[1] == pp3[1] && pp2[2] == pp3[2]) {
    p1 = pp1;
    p2 = pp3;
    p3 = pp4;
  }

  u[x] = p2[x] - p1[x];
  u[y] = p2[y] - p1[y];
  u[z] = p2[z] - p1[z];

  v[x] = p3[x] - p1[x];
  v[y] = p3[y] - p1[y];
  v[z] = p3[z] - p1[z];

  out[x] = u[y] * v[z] - u[z] * v[y];
  out[y] = u[z] * v[x] - u[x] * v[z];
  out[z] = u[x] * v[y] - u[y] * v[x];

  ReduceToUnit(out);
}

/* ------------------ CompareQuad ------------------------ */

static cadgeomdata *current_cadgeom;
int CompareQuad(const void *arg1, const void *arg2) {
  int i1, i2;
  cadgeomdata *cd;
  cadquad *quadi, *quadj;
  cadlookdata *cli, *clj;

  cd = current_cadgeom;

  i1 = *(int *)arg1;
  i2 = *(int *)arg2;

  quadi = cd->quad + i1;
  cli = quadi->cadlookq;

  quadj = cd->quad + i2;
  clj = quadj->cadlookq;

  if(cli < clj) return 1;
  if(cli > clj) return -1;
  return 0;
}
static char *texturedir = NULL;

cadgeomdata *CreateNewCADGeom(void) {
  cadgeomdata *cd;
  NewMemory((void **)&cd, sizeof(cadgeomdata));
  memset(cd, 0, sizeof(cadgeomdata));
  return cd;
}

/* ------------------ ReadCAD2Geom ------------------------ */

void ReadCAD2Geom(cadgeomdata *cd, GLfloat block_shininess) {
  char buffer[255];
  FILE *stream;
  int nquads = 0;
  int i;
  int iquad;
  int have_textures = 0;

  if((stream = FOPEN(cd->file, "r")) == NULL) {
    return;
  }

  /* read in [APPEARANCE] info */

  if(fgets(buffer, 255, stream) == NULL) {
    fclose(stream);
    return;
  }
  if(fgets(buffer, 255, stream) == NULL) {
    fclose(stream);
    return;
  }
  sscanf(buffer, "%i", &cd->ncadlookinfo);
  if(cd->ncadlookinfo <= 0) {
    cd->ncadlookinfo = 0;
    fclose(stream);
    return;
  }

  cd->cadlookinfo = NULL;
  NewMemory((void **)&cd->cadlookinfo, cd->ncadlookinfo * sizeof(cadlookdata));

  for(i = 0; i < cd->ncadlookinfo; i++) {
    cadlookdata *cdi;
    texturedata *texti;
    int errorcode;
    int ii;
    size_t lenbuffer, len;
    float *shininess;
    float *t_origin;
    float *rrgb;
    int *onesided;

    cdi = cd->cadlookinfo + i;

    if(fgets(buffer, 255, stream) == NULL)
      return; // material description (not used)

    if(fgets(buffer, 255, stream) == NULL) return;
    rrgb = cdi->rgb;
    shininess = &cdi->shininess;
    onesided = &cdi->onesided;
    cdi->texture_height = -1.0;
    cdi->texture_width = -1.0;
    t_origin = cdi->texture_origin;
    t_origin[0] = 0.0;
    t_origin[1] = 0.0;
    t_origin[2] = 0.0;
    rrgb[0] = -255.0;
    rrgb[1] = -255.0;
    rrgb[2] = -255.0;
    rrgb[3] = 1.0;
    *shininess = block_shininess;
    *onesided = 0;
    lenbuffer = strlen(buffer);
    for(ii = 0; ii < (int)lenbuffer; ii++) {
      if(buffer[ii] == ',') buffer[ii] = ' ';
    }

    sscanf(buffer, "%i %f %f %f %f %f %f %f %f %f %f %i", &cdi->index, rrgb,
           rrgb + 1, rrgb + 2, &cdi->texture_width, &cdi->texture_height,
           rrgb + 3, shininess, t_origin, t_origin + 1, t_origin + 2, onesided);

    rrgb[0] /= 255.0;
    rrgb[1] /= 255.0;
    rrgb[2] /= 255.0;
    if(rrgb[0] < 0.0 || rrgb[1] < 0.0 || rrgb[2] < 0.0) rrgb[3] = 1.0;

    if(fgets(buffer, 255, stream) == NULL) return;
    TrimBack(buffer);
    len = strlen(buffer);

    texti = &cdi->textureinfo;
    texti->file = NULL;
    if(len > 0) {
      NewMemory((void **)&texti->file, len + 1);
      strcpy(texti->file, buffer);
    }
    texti->display = 0;
    texti->loaded = 0;
    texti->used = 0;
    texti->name = 0;
    texti->is_transparent = 0;

    if(texti->file != NULL) {
      int texwid, texht;
      unsigned char *floortex;
      int is_transparent;

      if(have_textures == 0) {
        PRINTF("     Loading CAD textures\n");
        have_textures = 1;
      }
      PRINTF("       Loading texture: %s", texti->file);
      glGenTextures(1, &texti->name);
      glBindTexture(GL_TEXTURE_2D, texti->name);
      floortex = ReadPicture(texturedir, texti->file, &texwid, &texht,
                             &is_transparent, 0);
      texti->is_transparent = is_transparent;
      if(floortex == NULL) {
        PRINTF(" - failed\n");
        fprintf(stderr, "*** Error: Texture file %s failed to load\n",
                texti->file);
        continue;
      }
      errorcode = gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texwid, texht, GL_RGBA,
                                    GL_UNSIGNED_BYTE, floortex);
      if(errorcode != 0) {
        FREEMEMORY(floortex);
        PRINTF(" - failed\n");
        continue;
      }
      FREEMEMORY(floortex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      texti->loaded = 1;
      PRINTF(" - completed\n");
    }
  }

  /* read in [FACES] info */

  if(fgets(buffer, 255, stream) == NULL) return;
  if(fgets(buffer, 255, stream) == NULL) return;
  sscanf(buffer, "%i", &nquads);
  if(nquads <= 0) {
    cd->nquads = 0;
    return;
  }
  cd->nquads = nquads;
  cd->order = NULL;

  // TODO: what does this do. Seems like if there is nothing read, delete all of
  // the cad info
  if(NewMemory((void **)&cd->quad, nquads * sizeof(cadquad)) == 0 ||
     NewMemory((void **)&cd->order, nquads * sizeof(int)) == 0) {
    // If memory allocation fails, set quad data to NULL and return.
    cd->quad = NULL;
    return;
  }

  iquad = 0;
  for(i = 0; i < nquads; i++) {
    float *normal;
    int look_index;
    cadquad *quadi;
    cadlookdata *cl;
    float *xyzpoints;
    float time_show;

    if(fgets(buffer, 255, stream) == NULL) break;
    iquad++;
    quadi = cd->quad + i;
    xyzpoints = quadi->xyzpoints;
    normal = quadi->normals;
    time_show = 0.0;
    sscanf(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f %i %f", xyzpoints,
           xyzpoints + 1, xyzpoints + 2, xyzpoints + 3, xyzpoints + 4,
           xyzpoints + 5, xyzpoints + 6, xyzpoints + 7, xyzpoints + 8,
           xyzpoints + 9, xyzpoints + 10, xyzpoints + 11, &look_index,
           &time_show);
    if(look_index < 0 || look_index > cd->ncadlookinfo - 1) look_index = 0;
    quadi->cadlookq = cd->cadlookinfo + look_index;
    cl = quadi->cadlookq;
    quadi->colors[0] = cl->rgb[0];
    quadi->colors[1] = cl->rgb[1];
    quadi->colors[2] = cl->rgb[2];
    quadi->colors[3] = 1.0;
    quadi->time_show = time_show;
    CalcQuadNormal(xyzpoints, normal);
  }
  if(iquad < nquads) {
    fprintf(
        stderr,
        "*** Warning: number of faces expected=%i number of faces found=%i\n",
        cd->nquads, iquad);
    cd->nquads = iquad;
  }
  for(i = 0; i < cd->nquads; i++) {
    cd->order[i] = i;
  }
  current_cadgeom = cd;
  qsort(cd->order, (size_t)cd->nquads, sizeof(int), CompareQuad);
  fclose(stream);
  if(have_textures == 1) {
    PRINTF("     CAD textures loading completed\n");
  }
}

/* ------------------ ReadCADGeom ------------------------ */

/**
 * @brief Read CAD geometry data into 'cd'.
 *
 * @param cd A pointer to the cadgeomdata, this must already be allocated.
 * @param file A path to the file to read.
 * @param block_shininess The block shininess to apply.
 */
int ReadCADGeom(cadgeomdata *cd, const char *file, GLfloat block_shininess) {
  char buffer[255];
  float lastcolor[3];
  FILE *stream;
  int nquads = 0;
  int colorindex;

  if(NewMemory((void **)&cd->file, (unsigned int)(strlen(file) + 1)) == 0)
    return 2;
  STRCPY(cd->file, file);
  stream = FOPEN(cd->file, "r");
  if(stream == NULL) return 1;

  if(fgets(buffer, 255, stream) == NULL) {
    fclose(stream);
    return 1;
  }
  TrimBack(buffer);
  if(strncmp(buffer, "[APPEARANCE]", 12) == 0) {
    cd->version = 2;
    fclose(stream);
    ReadCAD2Geom(cd, block_shininess);
    return 0;
  }
  cd->version = 1;
  rewind(stream);
  while(!feof(stream)) {
    if(fgets(buffer, 255, stream) == NULL) break;
    if(fgets(buffer, 255, stream) == NULL) break;
    nquads++;
  }
  cd->nquads = nquads;
  rewind(stream);
  if(NewMemory((void **)&cd->quad, nquads * sizeof(cadquad)) == 0) {
    // If memory allocation fails, set quad data to NULL and return.
    cd->quad = NULL;
    fclose(stream);
    return 1;
  }
  nquads = 0;
  colorindex = 0;
  lastcolor[0] = -1.0;
  lastcolor[1] = -1.0;
  lastcolor[2] = -1.0;
  while(!feof(stream)) {
    char obstlabel[255];
    float *xyzpoints;
    float *normal;
    char *colors;
    float rgbtemp[4] = {(float)-1., (float)-1., (float)-1., (float)1.};
    cadquad *quadi;

    if(fgets(buffer, 255, stream) == NULL) break;
    quadi = cd->quad + nquads;
    xyzpoints = quadi->xyzpoints;
    normal = quadi->normals;
    sscanf(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f ", xyzpoints,
           xyzpoints + 1, xyzpoints + 2, xyzpoints + 3, xyzpoints + 4,
           xyzpoints + 5, xyzpoints + 6, xyzpoints + 7, xyzpoints + 8,
           xyzpoints + 9, xyzpoints + 10, xyzpoints + 11);
    CalcQuadNormal(xyzpoints, normal);

    if(fgets(buffer, 255, stream) == NULL) break;
    colors = strstr(buffer, " ");
    strcpy(obstlabel, buffer);
    rgbtemp[0] = (float)-1.0;
    rgbtemp[1] = (float)-1.0;
    rgbtemp[2] = (float)-1.0;
    rgbtemp[3] = (float)1.0;
    if(colors != NULL) {
      colors[0] = '\0';
      sscanf(colors + 1, "%f %f %f", rgbtemp, rgbtemp + 1, rgbtemp + 2);
    }
    if(lastcolor[0] != rgbtemp[0] || lastcolor[1] != rgbtemp[1] ||
       lastcolor[2] != rgbtemp[2]) {
      quadi->colorindex = colorindex;
      colorindex++;
      lastcolor[0] = rgbtemp[0];
      lastcolor[1] = rgbtemp[1];
      lastcolor[2] = rgbtemp[2];
    }
    else {
      quadi->colorindex = colorindex;
    }
    if(colors != NULL && rgbtemp[0] >= 0.0) {
      quadi->colorindex = -1;
    }
    quadi->colors[0] = rgbtemp[0];
    quadi->colors[1] = rgbtemp[1];
    quadi->colors[2] = rgbtemp[2];
    quadi->colors[3] = rgbtemp[3];
    nquads++;
  }
  fclose(stream);
  return 0;
}

/* ------------------ ReadCADGeomToCollection ------------------------ */

int ReadCADGeomToCollection(cadgeom_collection *coll, const char *file,
                            GLfloat block_shininess) {
  if(coll->ncadgeom >= coll->capacity){
    fprintf(stderr, "CADGeomCollection has exceeded capacity");
    return -1;
  }
  int res = ReadCADGeom(&coll->cadgeominfo[coll->ncadgeom], file, block_shininess);
  coll->ncadgeom++;
  return res;
}

/* ------------------ InitCADGeomCollection ------------------------ */

int InitCADGeomCollection(cadgeom_collection *coll, int capacity) {
  // Set everything to NULL
  memset(coll, 0, sizeof(cadgeom_collection));
  // Allocate capacity
  if(NEWMEMORY(coll->cadgeominfo, capacity * sizeof(cadgeomdata)) == 0)
    return 1;
  coll->capacity = capacity;
  return 0;
}

/* ------------------ CreateCADGeomCollection ------------------------ */

cadgeom_collection *CreateCADGeomCollection(int capacity) {
  cadgeom_collection *coll;
  if(NewMemory((void **)&coll, capacity * sizeof(cadgeom_collection)) == 0)
    return NULL;
  int ret = InitCADGeomCollection(coll, capacity);
  if(ret != 0) {
    FREEMEMORY(coll);
    return NULL;
  }
  else {
    return coll;
  }
}

/* ------------------ FreeCADGeom ------------------------ */

void FreeCADGeom(cadgeomdata *cd) {
  FreeMemory(cd->quad);
  FreeMemory(cd->order);
}

/* ------------------ FreeCADGeomCollection ------------------------ */

void FreeCADGeomCollection(cadgeom_collection *coll) {
  ClearCADGeomCollection(coll);
  FREEMEMORY(coll);
}

/* ------------------ ClearCADGeomCollection ------------------------ */

void ClearCADGeomCollection(cadgeom_collection *coll) {
  for(int i = 0; i < coll->ncadgeom; i++) {
    FreeCADGeom(&(coll->cadgeominfo[i]));
  }
  if(coll->cadgeominfo != NULL) FreeMemory(coll->cadgeominfo);
}

/* ------------------ NCADGeom ------------------------ */

int NCADGeom(cadgeom_collection *coll) {
  if(coll == NULL) {
    return 0;
  }
  else {
    return coll->ncadgeom;
  }
}
