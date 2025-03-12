#ifndef STRUCTURES_H_DEFINED
#define STRUCTURES_H_DEFINED

#if defined(WIN32)
#include <windows.h>
#endif
#include GL_H

#include "stdio_m.h"
#include "string_util.h" // necessary for flowlabels
#include "smokestream.h"
#ifdef pp_FRAME
#include "IOframe.h"
#endif

#include "readcad.h"
#include "readgeom.h"
#include "readobject.h"
#include "shared_structures.h"

/* --------------------------  langlistdata --------------------------------- */

typedef struct _lanlistdata {
  char *file;
  char lang_code[3];
  char lang_name[32];
} langlistdata;

/* --------------------------  procdata ------------------------------------ */
#ifdef CPP
typedef struct _procdata {
  GLUI_Rollout *rollout;
  GLUI *dialog;
  int rollout_id;
} procdata;
#endif


/* --------------------------  geomdiagdata ------------------------------------ */

typedef struct _geomdiagdata {
  char *geomfile, *geomdatafile;
  geomdata *geom;
} geomdiagdata;

/* --------------------------  screendata ------------------------------------ */

typedef struct _screendata {
  int nwidth, nheight;
  GLubyte *screenbuffer;
  float width, height, cosmax;
  float view[3], up[3], right[3];
} screendata;

/* --------------------------  shootpointdata ------------------------------- */

typedef struct _shootpointdata {
  struct _shootpointdata *prev;
  int visible;
  float xyz[3], uvw[3], uvw_air[3], val;
} shootpointdata;

/* --------------------------  shoottimedata ------------------------------------ */

typedef struct _shoottimedata {
  float time;
  int frame;
  shootpointdata *beg, *end;
} shoottimedata;

/* --------------------------  infiledata ------------------------------------ */

typedef struct _inifiledata {
  struct _inifiledata *prev, *next;
  int id;
  char *file;
} inifiledata;

/* --------------------------  rolloutlistdata ------------------------------------ */

#ifdef CPP
typedef struct _rolloutlistdata {
  struct _rolloutlistdata *prev, *next;
  GLUI_Rollout *rollout;
  GLUI *dialog;
} rolloutlistdata;
#endif

/* --------------------------  scriptfiledata ------------------------------------ */

typedef struct _scriptfiledata {
  struct _scriptfiledata *prev, *next;
  int id;
  int recording;
  char *file;
} scriptfiledata;

/* --------------------------  scriptdata ----------------------------------- */

typedef struct _scriptdata {
  int command;
  char command_label[32], quantity2[32];
  int ival,ival2,ival3,ival4,ival5;
  int need_graphics;
  char *cval,*cval2,*cval3;
  float fval,fval2,fval3,fval4,fval5;
  int exit,first,remove_frame;
  char *quantity, *id, *c_pbxyz;
  float pbxyz_val;
  int pbxyz_dir, cell_centered, vector;
} scriptdata;

/* --------------------------  keyworddata ------------------------------------ */

typedef struct _keyworddata{
  char keyword[32];
  int index, nparams, line_number;
} keyworddata;


/* --------------------------  facedata ------------------------------------ */

typedef struct _facedata {
  int type,type2;
  float approx_vertex_coords[12];
  float exact_vertex_coords[12];
  float approx_texture_coords[8];
  float exact_texture_coords[8];
  float *texture_origin;
  float normal[3];
  float *color;
  float *linecolor,*linewidth;
  float approx_center_coord[3];
  float dist2eye;
  int meshindex, blockageindex;
  int imin, imax, jmin, jmax, kmin, kmax;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int dir, hidden, dup, interior;
  int transparent;
#ifdef pp_CULL_GEOM
  struct _culldata *cullport;
#endif
  int **showtimelist_handle;
  int show_bothsides;
  struct _blockagedata *bc;
  struct _ventdata     *vi;
  surfdata *surfinfo;
  texturedata *textureinfo;
} facedata;

/* --------------------------  selectdata ------------------------------------ */

typedef struct _selectdadta {
  int mesh, blockage, side,dir;
  facedata *facei;
  int type;
} selectdata;

/* --------------------------  volfacelistdata ------------------------------ */

typedef struct _volfacelistdata {
  float *xyz,dist2;
  int iwall;
  meshdata *facemesh;
} volfacelistdata;

/* --------------------------  culldata ------------------------------------ */

typedef struct _culldata {
  float xbeg, xend, ybeg, yend, zbeg, zend;
  int   ibeg, iend, jbeg, jend, kbeg, kend;
  int iskip, jskip, kskip;
  int vis;
  meshdata *cull_mesh;
  int npixels,npixels_old;
} culldata;

/* --------------------------  curvedata ------------------------------------ */

typedef struct _curvedata{
// when updating curve variables look for occurrences of color in glui_objects.cpp and in
// UpdateCurveBounds in IOplot2d.c
  char c_type[64];
  char scaled_label[301], scaled_unit[301];
  char *quantity;
  int csv_file_index, csv_col_index, csv_col_index_ini, color[3], use_foreground_color;
  int apply_curve_factor;
  float curve_factor;
  float vmin, vmax;
  float *vals;
  int update_avg;
  float linewidth;
} curvedata;

/* --------------------------  plot2ddata ----------------------------------- */

#define PLOT2D_MAX_CURVES 50
typedef struct _plot2ddata{
  char plot_label[350];
  int ncurves, ncurves_ini, curve_index, plot_index, show, mult_devc;
  float valmin[2],     valmax[2];
  int   use_valmin[2], use_valmax[2];
  curvedata curve[PLOT2D_MAX_CURVES];
  int bounds_defined;
  float xyz[3], xyz_orig[3];
} plot2ddata;

/* --------------------------  treedevicedata ------------------------------- */

typedef struct _treedevicedata {
  int first, last, n, nz;
  float *xyz;
} treedevicedata;

/* --------------------------  ztreedevicedata ------------------------------ */

typedef struct _ztreedevicedata {
  char *quantity, *unit;
  int first, n;
} ztreedevicedata;

/* --------------------------  camviewdata ---------------------------------- */

typedef struct _camviewdata {
  float time, eye0[3], view0[3], aperture, up[3];
  float eye[3], view[3];
} camviewdata;

/* --------------------------  camdata ------------------------------------ */

typedef struct _camdata {
  char *file, *label;
  int ncamviews;
  int available;
  float *time;
  camviewdata *camviews;
  char menulabel[128];
} camdata;

/* --------------------------  portdata ------------------------------------ */

typedef struct _portdata {
GLint left, right, down, top, width, height;
int text_height, text_width;
int doit;
} portdata;

/* --------------------------  mousedata ------------------------------------ */

typedef struct _mousedata {
  int current[2], last[2], direction[2];
  float xcurrent[2], xdirection[2];
  int region;
  float angle, lastangle;
  float lasttime;
} mousedata;

/* --------------------------  cameradata ------------------------------------ */

typedef struct _cameradata {
  int defined,dirty;
  int projection_type;
  int rotation_type, rotation_index;
  float eye[3], view[3], up[3], eye_save[3];
  float isometric_y;
  float az_elev[2];
  float view_angle, azimuth, elevation;
  float xcen, ycen, zcen;
  float zoom;
  int quat_defined;
  float quaternion[4];

  int clip_mode;
  int clip_xmin, clip_xmax;
  int clip_ymin, clip_ymax;
  int clip_zmin, clip_zmax;
  float xmin, xmax;
  float ymin, ymax;
  float zmin, zmax;

  int view_id;
  struct _cameradata *next,*prev;
  char name[301];
} cameradata;

/* --------------------------  menudata ------------------------------------ */

typedef struct _menudata {
  int menuvar;
  int status;
} menudata;

/* --------------------------  filebounddata ------------------------------------ */

typedef struct _fileboundsdata {
  char file[255];
  float valmin, valmax;
} fileboundsdata;

/* --------------------------  slicemenudata ------------------------------------ */

typedef struct _slicemenudata {
  slicedata *sliceinfo;
} slicemenudata;

/* --------------------------  _subslicemenudata ------------------------------------ */

typedef struct _subslicemenudata {
  char *menulabel, *shortlabel;
  int slicetype;
  int havex, havey, havez, havexyz;
} subslicemenudata;

/* --------------------------  splitslicedata ------------------------------------ */

typedef struct _splitslicedata {
  slicedata *slice;
  meshdata *mesh;
  int splitdir;
  int is1, is2, js1, js2, ks1, ks2;
  int plotx, ploty, plotz;
} splitslicedata;

/* --------------------------  cpp_boundsdata ------------------------------------ */

typedef struct _cpp_boundsdata {
  char label[32], unit[32];
  int set_valmin, set_valmax, set_chopmin, set_chopmax, chop_hide;
  float valmin[4], valmax[4], chopmin, chopmax;
  float glui_valmin, glui_valmax;
  int set_valtype, cache;
  histogramdata *hist;
} cpp_boundsdata;

/* --------------------------  f_unit ------------------------------------ */

typedef struct _f_unit {
  char unit[10];   /* m/s, mph etc - appears in the colorbar */
  float scale[2];  /* newval=scale[0]*oldval+scale[1] */
  char rel_val[20];
  int rel_defined;
} f_unit;

/* --------------------------  f_units ------------------------------------ */

typedef struct _f_units {
  int nunits;
  int unit_index,submenuid,visible;
  char unitclass[30]; /* ie: velocity, temperature */
  int diff_index;
  f_unit *units;
} f_units;

/* --------------------------  skyboxdata ------------------------------------ */

typedef struct _skyboxdata {
  texturedata face[6];
} skyboxdata;

#endif
