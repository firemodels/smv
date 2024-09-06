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

/* --------------------------  circdata ------------------------------------ */

typedef struct _sliceparmdata {
  int nsliceinfo;
  int nvsliceinfo;
  int nmultisliceinfo;
  int nmultivsliceinfo;
} sliceparmdata;


/* --------------------------  langlistdata ------------------------------------ */

typedef struct _smokethreaddata{
  int ithread, nthreads;
} smokethreaddata;

/* --------------------------  langlistdata ------------------------------------ */

typedef struct _lanlistdata {
  char *file;
  char lang_code[3];
  char lang_name[32];
} langlistdata;

/* --------------------------  fueldata ------------------------------------ */

typedef struct _fueldata{
  char *fuel;
  float hoc;
} fueldata;

/* --------------------------  procdata ------------------------------------ */
#ifdef CPP
typedef struct _procdata {
  GLUI_Rollout *rollout;
  GLUI *dialog;
  int rollout_id;
} procdata;
#endif

/* --------------------------  csvdata ------------------------------------ */
typedef struct _csvdata{
  flowlabels label;
  float val, *vals, *vals_orig;
  float valmin, valmax;
  int nvals;
  int dimensionless, skip;
} csvdata;

/* --------------------------  _csvfiledata ------------------------------------ */

#define CSV_FDS_FORMAT   0
#define CSV_CFAST_FORMAT 1
typedef struct _csvfiledata {
  char *file;
  csvdata *csvinfo, *time;
  int defined, glui_defined;
  int ncsvinfo;
  int format;
  int loaded, display;
  char c_type[32];
} csvfiledata;

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

/* --------------------------  bounddata ------------------------------------ */

typedef struct _boundata {
  float global_min, global_max;
  int defined;
} bounddata;

/* --------------------------  shootpointdata ------------------------------------ */

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

/* --------------------------  luascriptfiledata ------------------------------------ */

#ifdef pp_LUA
typedef struct _luascriptfiledata {
  struct _luascriptfiledata *prev, *next;
  int id;
  char *file;
} luascriptfiledata;
#endif

/* --------------------------  scriptdata ------------------------------------ */

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

/* --------------------------  treedata ------------------------------------ */

typedef struct _treedata {
  float xyz[3];
  float time_char, time_complete;
  float trunk_diam;
  float tree_height;
  float base_diam;
  float base_height;
  int state;
} treedata;

/* --------------------------  colortabledata ------------------------------------ */

typedef struct _colortabledata {
  char label[1024];
  int color[4];
} colortabledata;

/* --------------------------  surfid ------------------------------------ */

typedef struct _surfid {
  char *label;
  int show;
  int location;
} surfid;

/* --------------------------  colordata ------------------------------------ */

typedef struct _colordata {
  float color[4], full_color[4], bw_color[4];
  struct _colordata *nextcolor;
} colordata;

/* --------------------------  outlinedata ------------------------------------ */

typedef struct _outlinedata {
  int nlines;
  float *x1, *y1, *z1;
  float *x2, *y2, *z2;
} outlinedata;

/* --------------------------  terraindata ------------------------------------ */

typedef struct _terraindata {
  char *file;
  int defined;
  texturedata *ter_texture;
  int ibar, jbar;
  float xmin, xmax, ymin, ymax;
  float zmin_cutoff;
  float *xplt, *yplt;
  float *zcell, *znode, *znode_scaled, *znode_offset;
  int nvalues; // number of values above zmin
  unsigned char *uc_znormal;
  struct _meshdata *terrain_mesh;
} terraindata;

/* --------------------------  surfdata ------------------------------------ */

typedef struct _surfdata {
  char *surfacelabel,*texturefile;
  int type; /*
               0 - regular block non-textured
               1 - regular block textured
               2 - outline
               3 - smoothed block
               4 - invisible
             */
  float *color, *color_orig, emis, temp_ignition;
  float transparent_level, transparent_level_orig;
  int iso_level;
  float t_width, t_height;
  texturedata *textureinfo;
  int obst_surface;
  int invisible;
  int location;
  int transparent;
  int used_by_obst,used_by_vent;
  int used_by_geom;
  int geom_surf_color[3];
  float axis[3];
  int in_geom_list;
  int ntris;
  int in_color_dialog;
  float geom_area;
} surfdata;

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
  struct _culldata *cullport;
  int **showtimelist_handle;
  int show_bothsides;
  struct _blockagedata *bc;
  surfdata *surfinfo;
  texturedata *textureinfo;
} facedata;

/* --------------------------  selectdata ------------------------------------ */

typedef struct _selectdadta {
  int mesh, blockage, side,dir;
  facedata *facei;
  int type;
} selectdata;

/* -------------------------- xbdata ------------------------------------ */

typedef struct _xbdata {
  float xyz[6], *color;
  int surf_index[6];
  struct _blockagedata *bc;
  int blocktype, transparent;
  int usecolorindex, colorindex;
  int invisible;
  surfdata *surfs[6];
} xbdata;

/* -------------------------- blockagedata ------------------------------------ */

typedef struct _blockagedata {
  int ijk[6];
  float xmin, xmax, ymin, ymax, zmin, zmax;
  float xyz[6], xyzEXACT[6], xyzDELTA[18];
  struct _surfdata *surf[6];
  struct _propdata *prop;
  int walltype,walltypeORIG;
  int surf_index[6];
  int usecolorindex;
  int blockage_id,dup;
  int is_wuiblock;
  int hole;
  int nnodes;
  int hidden, invisible, interior[6];
  int hidden6[6];
  int transparent;
  int meshindex;
  int del;
  int changed, changed_surface;
  int type;
  float *showtime;
  int *showtimelist;
  unsigned char *showhide;
  int nshowtime, show;
  char *label, *id_label;
  float *color;
  int colorindex;
  int useblockcolor;
  struct _facedata *faceinfo[6];
  float texture_origin[3];
} blockagedata;

/* --------------------------  cventdata ------------------------------------ */

typedef struct _cventdata {
  int dir,type,colorindex,cvent_id,isOpenvent;
  float boxmin[3], boxmax[3], texture_origin[3];
  float xmin, xmax, ymin, ymax, zmin, zmax;
  unsigned char *blank0, *blank;
  int   imin, imax, jmin, jmax, kmin, kmax;
  int nshowtime, *showtimelist;
  float *showtime;
  unsigned char *showhide;
  int useventcolor,hideboundary;
  float origin[3], radius;
  float *color;
  surfdata *surf[1];
  texturedata *textureinfo[1];
} cventdata;

/* --------------------------  ventdata ------------------------------------ */

typedef struct _ventdata {
  int type,dummy;
  struct _ventdata *dummyptr;
  int hideboundary;
  int dir,dir2,vent_id;
  int useventcolor;
  int isOpenvent, isMirrorvent;
  int wall_type;
  float xvent1_orig, xvent2_orig;
  float yvent1_orig, yvent2_orig;
  float zvent1_orig, zvent2_orig;
  float xvent1, xvent2;
  float yvent1, yvent2;
  float zvent1, zvent2;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int imin, imax, jmin, jmax, kmin, kmax;
  float xvent1plot, xvent2plot;
  float yvent1plot, yvent2plot;
  float zvent1plot, zvent2plot;
  float *showtime;
  int *showtimelist;
  unsigned char *showhide;
  int nshowtime;
  float *color,*color_bak;
  int transparent;
  int colorindex;
  int usecolorindex;
  surfdata *surf[1];
  texturedata *textureinfo[1];
  float texture_origin[3];
  float *linewidth;
} ventdata;

/* --------------------------  tickdata ------------------------------------ */

typedef struct _tickdata {
  float begin[3],end[3],length;
  float dxyz[3],dlength;
  int dir,nbars,useforegroundcolor;
  float width, rgb[3];
} tickdata;

/* --------------------------  isodata ------------------------------------ */

typedef struct _isodata {
  int seq_id, autoload;
  int isof_index;
  char *reg_file, *size_file, *topo_file;
  short *normaltable;
  int memory_id;
  int fds_skip;
  int finalize;
  float fds_delta;
  int nnormaltable;
  char *file,*tfile;
  int dataflag,geomflag,get_isolevels;
  int type;
  int firstshort_iso;
  flowlabels surface_label, color_label;
  geomdata *geominfo;
  int blocknumber,display,loaded,loading;
  float tmin,tmax;
  int isoupdate_timestep;
  float *levels, **colorlevels;
  int nlevels;
  char menulabel[128];
  int *geom_nstatics, *geom_ndynamics;
  float *geom_times, *geom_vals;
  unsigned char *times_map;
  unsigned char *geom_times_map;
  float globalmin_iso, globalmax_iso;
  int geom_nvals;
#ifdef pp_ISOFRAME
  framedata *frameinfo;
#endif
} isodata;

/* --------------------------  volrenderdata ------------------------------------ */

typedef struct _volrenderdata {
  char *rendermeshlabel;
  int is_compressed;
  struct _slicedata   *smokeslice,            *fireslice;
  unsigned char *c_smokedata_view,      *c_firedata_view;
  int      *nsmokedata_compressed, *nfiredata_compressed;
  float           *smokedata_full,        *firedata_full;
  float           *smokedata_view,        *firedata_view;
  LINT                  *smokepos,              *firepos;
  void              *smokedataptr,          *firedataptr;
  void            **smokedataptrs,        **firedataptrs;
  float *times;
  int *dataready;
  int itime, ntimes, times_defined;
  int *timeslist;
  float *smokecolor_yz0, *smokecolor_xz0, *smokecolor_xy0;
  float *smokecolor_yz1, *smokecolor_xz1, *smokecolor_xy1;
  int loaded, display;
} volrenderdata;

/* --------------------------  meshplanedata ------------------------------------ */

typedef struct _meshplanedata {
  float verts[6*3],verts_smv[6*3];
  float norm0[4*3], norm1[4*3];
  int have_vals[3];
  int triangles[4*3], nverts, ntriangles;
  float *vals2, *verts2;
  int *tris2, nverts2, ntris2;
  int polys[10], npolys;
  int drawsmoke;
} meshplanedata;

/* --------------------------  cellmeshdata ------------------------------------ */

typedef struct _cellmeshdata {
  int nxyz[3];
  float xyzminmax[6], dxyz[3];
  meshdata **cellmeshes;
} cellmeshdata;

/* --------------------------  supermeshdata ------------------------------------ */

typedef struct _supermeshdata {
#ifdef pp_GPU
  GLuint blockage_texture_id;
  GLuint volsmoke_texture_id,         volfire_texture_id;
  float *volsmoke_texture_buffer,*volfire_texture_buffer;
#endif
  float *f_iblank_cell;
  float boxmin_scaled[3], boxmax_scaled[3];
  int drawsides[7];
  int nmeshes;
  meshdata **meshes;
  int ibar, jbar, kbar;
} supermeshdata;

/* --------------------------  volfacelistdata ------------------------------------ */

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

/* --------------------------  hrrdata ------------------------------------ */

typedef struct _hrrdata {
  float *vals, *vals_orig, valmin, valmax;
  int inlist1;
  int nvals;
  int base_col;
  flowlabels label;
} hrrdata;

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

/* --------------------------  plot2ddata ------------------------------------ */

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

/* --------------------------  windrosedata ------------------------------------ */

typedef struct _windrosedata {
  histogramdata histogram[3];
  float *xyz;
} windrosedata;

/* --------------------------  vdevicedata ------------------------------------ */

typedef struct _vdevicedata {
  int unique, filetype, display;
  int nwindroseinfo;
  windrosedata *windroseinfo;
  devicedata *udev,*vdev,*wdev,*valdev,*colordev,*veldev,*angledev,*sd_veldev,*sd_angledev;
} vdevicedata;


/* --------------------------  vdevicesortdata ------------------------------------ */

typedef struct _vdevicesortdata {
  int dir;
  vdevicedata *vdeviceinfo;
} vdevicesortdata;

/* --------------------------  treedevicedata ------------------------------------ */

typedef struct _treedevicedata {
  int first, last, n, nz;
  float *xyz;
} treedevicedata;

/* --------------------------  ztreedevicedata ------------------------------------ */

typedef struct _ztreedevicedata {
  char *quantity, *unit;
  int first, n;
} ztreedevicedata;

/* --------------------------  camviewdata ------------------------------------ */

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

/* --------------------------  partpropdata ------------------------------------ */

typedef struct _partpropdata {
  flowlabels *label;
  float *partlabelvals;
  unsigned char *class_present, *class_vis;
  unsigned int *class_types;
  int display;
  float ppartlevels256[256];
  float valmin, valmax;
  int imin, imax;
  float dlg_global_valmin, dlg_global_valmax;
  int set_global_bounds;
  float user_min, user_max;
  int setvalmin, setvalmax;
  float chopmin, chopmax;
  int setchopmin, setchopmax;
  int extreme_min, extreme_max;
} partpropdata;

/* --------------------------  part5data ------------------------------------ */

typedef struct _part5data {
  partclassdata *partclassbase;
  float time;
  int npoints_file,npoints_loaded,n_rtypes, n_itypes;
  float *xpos, *ypos, *zpos;
  float *dsx, *dsy, *dsz;
  float *avatar_angle, *avatar_width, *avatar_depth, *avatar_height;
  int *tags,*sort_tags;
  unsigned char *vis_part;
  float *rvals,**rvalsptr;
  unsigned char *irvals;
  unsigned char **cvals;
} part5data;

/* --------------------------  partdata ------------------------------------ */

typedef struct _partdata {
  FILE_m *stream;

  char *file, *size_file, *reg_file, *bound_file;
  int have_bound_file;
  int seq_id, autoload, loaded, skipload, request_load, display, reload, finalize;
  int loadstatus, boundstatus;
  int compression_type;
  int blocknumber;
  int *timeslist, ntimes, itime;
  FILE_SIZE bound_file_size;
  int npoints_file, npoints_loaded;

  float zoffset, *times;
  unsigned char *times_map;
  FILE_SIZE reg_file_size, file_size;
#ifndef pp_PARTFRAME
  LINT *filepos;
#endif

  char menulabel[128];

  int nclasses;
  partclassdata **partclassptr;
  part5data *data5;
  histogramdata **histograms;
  int hist_update;
  int bounds_set;
  float *valmin_part,    *valmax_part;
  int nfilebounds;
  unsigned char *vis_part;
  int *sort_tags;
  unsigned char *irvals;
#ifdef pp_PARTFRAME
  framedata *frameinfo;
#endif
} partdata;

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
/* --------------------------  boundsdata ------------------------------------ */

typedef struct _boundsdata {
  char *shortlabel;
  int dlg_setvalmin, dlg_setvalmax;

  int ini_defined;
  int ini_setvalmin, ini_setvalmax;
  float ini_valmin, ini_valmax;
  float edit_valmin, edit_valmax;
  int edit_valmin_defined, edit_valmax_defined;

  int setchopmin, setchopmax;
  float chopmin, chopmax;
  float dlg_valmin, dlg_valmax;
  float data_valmin, data_valmax;
  float dlg_global_valmin, dlg_global_valmax;
  float line_contour_min;
  float line_contour_max;
  float dev_min, dev_max;
  int line_contour_num;
  char  colorlabels[12][11];
  float colorvalues[12];
  float levels256[256];
  flowlabels *label;
} boundsdata;

/* --------------------------  globalboundsdata ------------------------------------ */

typedef struct _globalboundsdata {
  char *file;
  int defined;
  int nbounds;
  float valmins_save[MAXPLOT3DVARS], valmaxs_save[MAXPLOT3DVARS];
  float valmins[MAXPLOT3DVARS],      valmaxs[MAXPLOT3DVARS];
} globalboundsdata;


/* --------------------------  patchfacedata ------------------------------------ */

typedef struct _patchfacedata{
  int ib[6], dir, vis, nrow, ncol, start, type, internal_mesh_face;
  int obst_index, mesh_index;
  struct _meshdata *meshinfo;
  struct _blockagedata *obst;
} patchfacedata;

/* --------------------------  patchdata ------------------------------------ */

typedef struct _patchdata {
  int seq_id, autoload;
  char *file,*size_file,*bound_file;
  int have_bound_file;
  char *comp_file, *reg_file;
  char *filetype_label;
  geomdata *geominfo;
  int *geom_offsets;
  int skip,dir;
  float xyz_min[3], xyz_max[3];
  int ntimes, ntimes_old;
  int patch_filetype, structured;
  int shortlabel_index;
  int *cvals_offsets, *cvals_sizes;
  unsigned char *cbuffer;
  int is_compressed;
  int cbuffer_size;
  int boundary;
  int firstshort_patch;
  int compression_type, compression_type_temp;
  int setvalmin, setvalmax;
  float valmin_patch, valmax_patch;
  float valmin_glui,  valmax_glui;
  int setchopmin, setchopmax;
  float chopmin, chopmax;
  float diff_valmin, diff_valmax;
  int blocknumber,loaded,display;
  float *geom_times, *geom_vals;
  int *geom_timeslist,geom_itime;
  unsigned char *geom_times_map;
  unsigned char *geom_ivals;
  int *geom_ivals_static_offset, *geom_ivals_dynamic_offset;
  int *geom_vals_static_offset,  *geom_vals_dynamic_offset;
  unsigned char *geom_ival_static, *geom_ival_dynamic;
  float         *geom_val_static,  *geom_val_dynamic;
  int geom_nval_static, geom_nval_dynamic;
  int *geom_nstatics, *geom_ndynamics;
  int geom_vert2tri;
  int geom_nvals, ngeom_times;
  flowlabels label;
  char menulabel[128], menulabel_base[128], menulabel_suffix[128], gslicedir[50];
  int ijk[6];
  int extreme_min, extreme_max;
  time_t modtime;
  int finalize;
  int hist_update;
  bounddata bounds;
  boundsdata *bounds2;
  int npatches;
  patchfacedata *patchfaceinfo;
  patchfacedata *meshfaceinfo[6];
#ifdef pp_BOUNDFRAME
  framedata *frameinfo;
#endif
} patchdata;

/* --------------------------  plot3ddata ------------------------------------ */

typedef struct _plot3ddata {
  int seq_id, autoload;
  char *file, *reg_file,  *bound_file;
  int have_bound_file;
  int finalize;
  int memory_id;
  float time;
  int u, v, w, nplot3dvars;
  float diff_valmin[MAXPLOT3DVARS], diff_valmax[MAXPLOT3DVARS];
  int extreme_min[MAXPLOT3DVARS], extreme_max[MAXPLOT3DVARS];
  int blocknumber,loaded,display,loadnow;
  float valmin_plot3d[MAXPLOT3DVARS], valmax_plot3d[MAXPLOT3DVARS];
  flowlabels label[MAXPLOT3DVARS];
  char menulabel[256], longlabel[256], timelabel[256];
  histogramdata *histograms[MAXPLOT3DVARS];
  int hist_update;
} plot3ddata;

/* --------------------------  zonedata ------------------------------------ */

typedef struct _zonedata {
  int seq_id, autoload;
  char *file,*basefile;
  int loaded,display;
  int csv;
  flowlabels label[4];
  int setvalmin, setvalmax;
  float valmin, valmax;
} zonedata;

/* --------------------------  roomdata ------------------------------------ */

typedef struct _roomdata {
  int valid;
  float dx,dy,dz;
  float x0,y0,z0;
  float x1,y1,z1;
  int drawsides[7];
  float pfloor, ylay, tl, tu, rho_L, rho_U, od_L, od_U;
  int itl, itu;
  int zoneinside;
} roomdata;

/* --------------------------  zventdata ------------------------------------ */

typedef struct _zventdata {
  int wall, nslab;
  float x0, y0, z0;
  float x1, y1, z1;
  float xcen, ycen, radius;
  float vmin, vmax;
  float g_vmin, g_vmax;
  roomdata *room1, *room2;
  float area, area_fraction;
  float slab_bot[MAX_HSLABS], slab_top[MAX_HSLABS], slab_vel[MAX_HSLABS], slab_temp[MAX_HSLABS];
  float *color;
  float vdata[NELEV_ZONE];
  int itempdata[NELEV_ZONE];
  int vent_type, vertical_vent_type;
} zventdata;

/* --------------------------  firedata ------------------------------------ */

typedef struct _firedata {
  float x, y, z, dz;
  float absx,absy,absz;
  int valid,roomnumber;
} firedata;

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
