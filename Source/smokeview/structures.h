#ifndef STRUCTURES_H_DEFINED
#define STRUCTURES_H_DEFINED

#if defined(WIN32)
#include <windows.h>
#endif
#include GL_H

#include "stdio_m.h"
#include "string_util.h" // necessary for flowlabels
#include "smokestream.h"

/* --------------------------  circdata ------------------------------------ */

typedef struct _sliceparmdata {
  int nsliceinfo;
  int nvsliceinfo;
  int nmultisliceinfo;
  int nmultivsliceinfo;
  int nfedinfo;
  int nfediso;
} sliceparmdata;

/* --------------------------  circdata ------------------------------------ */

typedef struct _circdata {
  float *xcirc, *ycirc;
  int ncirc;
} circdata;

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

/* --------------------------  vertdata ------------------------------------ */

typedef struct _vertdata {
  float xyz[3],vert_norm[3],texture_xy[3];
  int itriangle,ntriangles,nused;
  unsigned char on_mesh_boundary;
  int geomtype;
  int isdup;
  struct _tridata **triangles, *triangle1;
} vertdata;

/* --------------------------  edgedata ------------------------------------ */

typedef struct _edgedata {
  int ntriangles;
  int vert_index[2], itri;
  vertdata *verts[2];
  struct _tridata *triangles[2];
} edgedata;

/* --------------------------  tridata ------------------------------------ */

typedef struct _tridata {
  unsigned char skinny;
  float distance, *color, tverts[6], tri_norm[3], vert_norm[9], area;
  float cface_norm1[3], cface_norm2[3];
  struct _texturedata *textureinfo;
  struct _surfdata *geomsurf;
  struct _geomlistdata *geomlisti;
  struct _geomobjdata *geomobj;
  int vert_index[3], exterior, geomtype, insolid, outside_domain;
  vertdata *verts[3];
  edgedata *edges[3];
#ifdef pp_DECIMATE
  int ival;
#endif
} tridata;

/* --------------------------  geomlistdata ------------------------------------ */

typedef struct _geomlistdata {
  int nverts,nedges,ntriangles,norms_defined;
  float *zORIG;
  vertdata *verts;
  float *vertvals;
  tridata *triangles, **triangleptrs, **connected_triangles;
  edgedata *edges;
} geomlistdata;

/* --------------------------  geomobjdata ------------------------------------ */

typedef struct _geomobjdata {
  struct _surfdata *surf;
  struct _texturedata *texture;
  float *bounding_box, *color;
  char *texture_name;
  float texture_width, texture_height, texture_center[3];
  int texture_mapping;
  int use_geom_color;
} geomobjdata;

/* --------------------------  geomdata ------------------------------------ */

typedef struct _geomdata {
  char *file, *file2, *topo_file;
  int read_status;
  int cache_defined;
  int memory_id, loaded, display;
  int is_terrain;
  int block_number;
  int have_cface_normals, ncface_normals;
  float *cface_normals;
  float *float_vals;
  float bounding_box[6];
  int *file2_tris, nfile2_tris;
  int *int_vals, nfloat_vals, nint_vals;
  float *times;
  int ntimes,itime,*timeslist;
  int ngeomobjinfo, geomtype, patchactive, fdsblock;
  struct _surfdata *surfgeom;
  geomlistdata *geomlistinfo,*geomlistinfo_0, *currentframe;
  geomobjdata *geomobjinfo;
} geomdata;


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

/* --------------------------  propdata ------------------------------------ */
#define PROPVARMAX 100
typedef struct _propdata {
  char *label;
  int menu_id;
  float *rotate_axis, rotate_angle;
  int inblockage,blockvis;
  int nsmokeview_ids,ismokeview_ids;
  char *smokeview_id,**smokeview_ids;
  struct _sv_object *smv_object, **smv_objects;
  int ntextures;
  char **texturefiles, **vars_indep, **svals;
  int *vars_indep_index, vars_dep_index[PROPVARMAX];
  int nvars_indep, nvars_dep;
  float *fvals, fvars_dep[PROPVARMAX];
  int tag_number;
} propdata;

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

/* --------------------------  colorbardata ------------------------------------ */

#define CB_RAINBOW    0
#define CB_ORIGINAL   1
#define CB_LINEAR     2
#define CB_DIVERGENT  3
#define CB_CIRCULAR   4
#define CB_DEPRECATED 5
#define CB_USER       6
#define CB_OTHER      7
#define INTERP_RGB    0
#define INTERP_LAB    1
typedef struct _colorbardata {
  char menu_label[1024];        // menu label
  char colorbar_type[256];      // rainbow, linear, divergent, etc
  int nnodes, nnodes_orig, node_index_orig[1024], nodehilight, type;
  unsigned char node_rgb_orig[3*1024], node_rgb[3*1024], colorbar_alpha[1024];
  unsigned char node_index[1024];  // colorbar index
  float node_dist[1024];
  float colorbar_dist[256];
  int dist_ind[256];
  float colorbar_dist_delta[1024];
  int interp;   // (LAB or RGB)
  int can_adjust, adjusted;
  float colorbar_rgb[3*1024], colorbar_lab[3*1024];
} colorbardata;

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

/* --------------------------  labeldata ------------------------------------ */

typedef struct _labeldata {
  struct _labeldata *prev, *next;
  char name[300];
  float xyz[3],frgb[4],tstart_stop[2];
  float tick_begin[3], tick_direction[3];
  int show_tick;
  int rgb[4], glui_id, labeltype; // smv or ini
  int useforegroundcolor,show_always;
} labeldata;

/* --------------------------  texturedata ------------------------------------ */

typedef struct _texturedata {
  char *file;
  int loaded, display, used, is_transparent;
  GLuint name;
} texturedata;

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
  int dir,hidden,dup,interior;
  int del;
  int transparent;
  int patchpresent;
  struct _culldata *cullport;
  int **showtimelist_handle;
  int show_bothsides, is_interior;
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
  float xyzEXACT[6], xyzDELTA[18];
  struct _surfdata *surf[6];
  struct _propdata *prop;
  int walltype,walltypeORIG;
  int surf_index[6];
  int patchvis[7];
  int usecolorindex;
  int blockage_id,dup;
  int is_wuiblock;
  int hole;
  int nnodes;
  int hidden, invisible, interior[6];
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

/* --------------------------  cadlookdata ------------------------------------ */

typedef struct _cadlookdata {
  int index;
  float texture_width, texture_height, texture_origin[3];
  float rgb[4], shininess;
  texturedata textureinfo;
  int onesided;
} cadlookdata;

/* --------------------------  cadquad ------------------------------------ */

typedef struct _cadquad {
  float xyzpoints[12];
  float txypoints[8];
  float normals[3];
  int colorindex;
  float colors[4];
  float time_show;
  cadlookdata *cadlookq;
} cadquad;

/* --------------------------  clipdata ------------------------------------ */

typedef struct _clipdata {
  int option;
  GLdouble clipvals[24];
  int clip_xmin, clip_xmax;
  int clip_ymin, clip_ymax;
  int clip_zmin, clip_zmax;
  float xmin, xmax;
  float ymin, ymax;
  float zmin, zmax;
} clipdata;

/* --------------------------  cadgeomdata ------------------------------------ */

typedef struct _cadgeomdata{
  char *file;
  int *order;
  int version;
  int ncadlookinfo;
  cadlookdata *cadlookinfo;
  int nquads;
  cadquad *quad;
} cadgeomdata;

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

/* --------------------------  feddata ------------------------------------ */

typedef struct _feddata {
  struct _slicedata *co,*co2,*o2,*fed_slice;
  struct _isodata *fed_iso;
  int co_index, co2_index, o2_index, fed_index;
  int loaded,display;
} feddata;

/* --------------------------  isodata ------------------------------------ */

typedef struct _isodata {
  int seq_id, autoload;
  int isof_index;
  char *reg_file, *size_file, *topo_file;
  short *normaltable;
  int memory_id;
  int fds_skip;
  float fds_delta;
  int nnormaltable;
  char *file,*tfile;
  int dataflag,geomflag,get_isolevels;
  int is_fed;
  feddata *fedptr;
  int type;
  int setvalmin, setvalmax;
  float valmin, valmax;
  int firstshort;
  flowlabels surface_label, color_label;
  geomdata *geominfo;
  int blocknumber,display,loaded,loading;
  float tmin,tmax;
  float valmin_data, valmax_data;
  int extreme_min, extreme_max;
  int isoupdate_timestep;
  float *levels, **colorlevels;
  int nlevels;
  char menulabel[128];
  int *geom_nstatics, *geom_ndynamics;
  float *geom_times, *geom_vals;
  unsigned char *times_map;
  int have_restart;
  float geom_globalmin, geom_globalmax;
  int geom_nvals;
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

/* --------------------------  mesh ------------------------------------ */

typedef struct _meshdata {
  int ibar, jbar, kbar;
  float cellsize;
  int ncvents,nvents,ndummyvents;
  int nbptrs;
  int is_bottom;

  int *cutcells, ncutcells;
  int update_smoke3dcolors;
  struct _terraindata *terrain;
  float *znodes_complete;
  int nznodes;
  struct _meshdata *floor_mesh;
#ifdef pp_GPU
  GLuint blockage_texture_id;
  struct _smoke3ddata *smoke3d_soot, *smoke3d_hrrpuv, *smoke3d_temp, *smoke3d_co2;
  GLuint     volsmoke_texture_id,     volfire_texture_id;
  float *volsmoke_texture_buffer,*volfire_texture_buffer;
  int voltest_update;
  GLuint slice3d_texture_id;
  float *slice3d_c_buffer;
#ifdef pp_WINGPU
  float *slice3d_texture_buffer;
#endif
#endif
  float meshrgb[3], *meshrgb_ptr;
  float mesh_offset[3], *mesh_offset_ptr;
  int blockvis;
  float *xplt, *yplt, *zplt;
  int ivolbar, jvolbar, kvolbar;
  float *xvolplt, *yvolplt, *zvolplt;
  float *xplt_cen, *yplt_cen, *zplt_cen;
  float *xplt_orig, *yplt_orig, *zplt_orig;
  float x0, x1, y0, y1, z0, z1;
  int drawsides[7];
  int extsides[7];   // 1 if on exterior side of a supermesh, 0 otherwise
  int is_extface[6]; //  MESH_EXT if face i is completely adjacent to exterior,
                     //  MESH_INT if face i is completely adjacent to another mesh,
                     // MESH_BOTH if face i is neither
  int inside;
  int in_frustum;    // 1 if part or all of mesh is in the view frustum
  float boxmin[3], boxmiddle[3], boxmax[3], dbox[3], boxeps[3], dcell, dcell3[3], verts[24], eyedist;
  float boxeps_fds[3];
  float slice_min[3], slice_max[3];
  float boxmin_scaled[3], boxmiddle_scaled[3], boxmax_scaled[3];
  float xyz_bar0[3], xyz_bar[3];
  float xcen, ycen, zcen;
  float face_centers[18];
  float offset[3];
  float xyzmaxdiff;
  float boxoffset;
  int plot3dfilenum,isofilenum,patchfilenum;
  int *iplotx_all, *iploty_all, *iplotz_all;
  int plotx, ploty, plotz;
  int slicedir;
  int plotn;
  int *imap, *jmap, *kmap;
  int n_imap, n_jmap, n_kmap;

  char *c_iblank_node0, *c_iblank_cell0, *c_iblank_x0, *c_iblank_y0, *c_iblank_z0;
  char *c_iblank_node_html;
  float *f_iblank_cell0;
  char *c_iblank_embed0;
  float *block_zdist0;
  float *opacity_adjustments;

  char *c_iblank_node, *c_iblank_cell, *c_iblank_x, *c_iblank_y, *c_iblank_z;
  float *f_iblank_cell;
  char *c_iblank_embed;
  float *block_zdist;

  int zdist_flag;
  unsigned char *iblank_smoke3d;
  int iblank_smoke3d_defined;
  struct _blockagedata **blockageinfoptrs;
  int *obst_bysize;
  struct _ventdata *ventinfo;
  struct _cventdata *cventinfo;
  unsigned char *is_block_terrain;
  unsigned char *iqdata;
  float *qdata, *udata, *vdata, *wdata;
  unsigned char *yzcolorbase, *xzcolorbase, *xycolorbase;
  float *yzcolorfbase, *xzcolorfbase, *xycolorfbase;
  float *yzcolortbase, *xzcolortbase, *xycolortbase;
  float *dx_xy, *dy_xy, *dz_xy;
  float *dx_xz, *dy_xz, *dz_xz;
  float *dx_yz, *dy_yz, *dz_yz;
  char *c_iblank_xy, *c_iblank_xz, *c_iblank_yz;
  float plot3d_speedmax;
  struct _contour *plot3dcontour1, *plot3dcontour2, *plot3dcontour3;
  struct _isosurface *currentsurf, *currentsurf2;
  struct _isosurface *blockagesurface;
  struct _isosurface **blockagesurfaces;
  int ntc;
  int nspr;
  float *xsprplot, *ysprplot, *zsprplot, *tspr;
  int nheat;
  float *xheatplot, *yheatplot, *zheatplot, *theat;
  float *xspr, *yspr, *zspr;
  float *xheat, *yheat, *zheat;

  struct _isosurface *animatedsurfaces;
  int nisolevels, *showlevels;
  float *isolevels;
  int isomin_index, isomax_index;
  int niso_times;
  float *iso_times;
  unsigned char *iso_times_map;
  int *iso_timeslist;
  int iso_itime;
  int smokedir,smokedir_old;
  float dxDdx, dyDdx, dzDdx, dxyDdx, dxzDdx, dyzDdx, dxyz_orig[3];
  float smoke_dist[6];
  float norm[3];
  float dplane_min[4], dplane_max[4];

  int *boundarytype;
  int *patchdir,*patch_surfindex;
  int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2;
  struct _meshdata *skip_nabors[6];
  int *blockonpatch;
  struct _meshdata **meshonpatch;
  struct _meshdata *nabors[6];
  struct _supermeshdata *super;
  int *ptype;
  int *boundary_row, *boundary_col, *blockstart;
  unsigned int *zipoffset, *zipsize;
  int *vis_boundaries;
  float *xyzpatch, *xyzpatch_threshold;
  unsigned char *cpatchval_zlib, *cpatchval_iframe_zlib;
  unsigned char *cpatchval, *cpatchval_iframe;
  float *patch_times, *patch_timesi, *patchval, *patchval_iframe;
  unsigned char *patch_times_map;
  float **patchventcolors;
  float *thresholdtime;
  int *patchblank;
  int npatch_times,npatches;
  int patch_itime;
  int *patch_timeslist;
  int npatchsize;
  int visInteriorBoundaries;
  float surface_tempmin, surface_tempmax;

  int nface_textures, nface_outlines, nfaces;
  int nface_normals_single, nface_normals_double, nface_transparent_double;
  struct _facedata *faceinfo, **face_normals_single, **face_normals_double, **face_transparent_double, **face_textures, **face_outlines;
  struct _facedata **face_normals_single_DOWN_X,**face_normals_single_UP_X;
  struct _facedata **face_normals_single_DOWN_Y,**face_normals_single_UP_Y;
  struct _facedata **face_normals_single_DOWN_Z,**face_normals_single_UP_Z;
  int nface_normals_single_DOWN_X,nface_normals_single_UP_X;
  int nface_normals_single_DOWN_Y,nface_normals_single_UP_Y;
  int nface_normals_single_DOWN_Z,nface_normals_single_UP_Z;

  int itextureoffset;

  int maxtimes_boundary;
  float vent_offset[3];
  int select_min, select_max;

  struct _clipdata *box_clipinfo;

  unsigned char *merge_color,*merge_alpha;
  unsigned char *smokecolor_ptr, *smokealpha_ptr;

  char *label;

  int ncullgeominfo,nxyzgeomcull[3],nxyzskipgeomcull[3];
  struct _culldata *cullgeominfo;
#ifdef pp_DECIMATE
  struct _vertdata *dec_verts;
  struct _tridata *dec_triangles;
  int ndec_triangles, ndec_verts, decimated;
#endif


  struct _volrenderdata *volrenderinfo;
  int  nslicex,  nslicey,  nslicez;
  struct _slicedata **slicex, **slicey, **slicez;

  struct _meshplanedata *gsliceinfo;
  struct _meshplanedata *smokeplaneinfo;
  int nsmokeplaneinfo;
  int s_offset[3];
  int isliceinfo, nsliceinfo;
  int use;
} meshdata;

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

/* --------------------------  keyframe ------------------------------------ */

typedef struct _keyframe {
  int selected, npoints;
  float time;
  float pause_time, cum_pause_time;
  float view_smv[3], view2_smv[3];
  float xyz_fds[3], xyz_smv[3];
  float arc_dist, line_dist, xyz_diff[3], view_diff[3];
  float  xyz_tangent_left[3],  view_tangent_left[3];
  float xyz_tangent_right[3], view_tangent_right[3];
  struct _keyframe *next,*prev;
} keyframe;

/* --------------------------  tourdata ------------------------------------ */

typedef struct _tourdata {
  char label[300],menulabel[128];
  keyframe first_frame,last_frame, **keyframe_list;
  int glui_avatar_index, display2;
  float *path_times,*keyframe_times;
  float xyz_smv[3], view_smv[3];
  float global_dist;
  int *timeslist;
  int ntimes, nkeyframes;
  int display, periodic;
  int startup;
  int isDefault;
} tourdata;

/* --------------------------  tokendata ------------------------------------ */

typedef struct _tokendata {
  float var, *varptr, default_val;
  int command,loc,type,reads,nvars,noutvars,is_label,is_string,is_texturefile;
  struct _sv_object *included_object;
  int included_frame;
  int texture_index;
  struct _tokendata *next,*elsenext;
  char token[64],tokenlabel[64],tokenfulllabel[64];
  char string[256],default_string[256],*stringptr;
} tokendata;

/* --------------------------  sv_object_frame ------------------------------------ */

typedef struct _sv_object_frame {
  int use_bw;
  int error;
  int display_list_ID;
  int *symbols, nsymbols;
  tokendata *tokens, **command_list;
  int ntokens, ncommands, ntextures;
  struct _sv_object *device;
  struct _sv_object_frame *prev, *next;
} sv_object_frame;

/* --------------------------  sv_object ------------------------------------ */

typedef struct _sv_object {
  char label[256];
  int type;
  int visible;
  int used, used_by_device;
  int use_displaylist;
  int select_mode;
  int nframes;
  sv_object_frame **obj_frames, first_frame, last_frame;
  struct _sv_object *prev, *next;
} sv_object;

/* --------------------------  device ------------------------------------ */

#define LENDEVICEBUFFER 255
typedef struct _device {
  int active;
  int show;
  int screenijk[3], visval, target_index;
  char deviceID[LENDEVICEBUFFER], csvlabel[LENDEVICEBUFFER], *labelptr;
  char quantity[LENDEVICEBUFFER], unit[LENDEVICEBUFFER];
  float *times, *vals;
  float *vals_orig;
  int update_avg;
  int *valids;
  int ival,nvals,type2,type2vis;
  int in_devc_csv;
  meshdata *device_mesh;
  texturedata *textureinfo;
  char *texturefile;
  int ntextures;
  float xyz[3], xyz1[3], xyz2[3], eyedist;
  int have_xyz, have_xyz1, have_xyz2;
  float val;
  float xyzplot[3];
  float xyznorm[3];
  float dtheta, rotate_axis[3];
  float act_time;
  float *color, line_width;
  int filetype;
  float *act_times;
  float global_valmin, global_valmax;
  int *state_values;
  int nparams;
  float *params;
  int istate_changes, nstate_changes, state0;
  int *showstatelist;
  int in_zone_csv;
  isosurface **plane_surface;
  propdata *prop;
  sv_object *object;
  struct _vdevicedata *vdevice;
  int type, is_beam;
  int selected;
  int inlist;
  int valid;
} devicedata;

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

/* --------------------------  partclassdata ------------------------------------ */

typedef struct _partclassdata {
  char *name;
  int col_diameter, col_length, col_azimuth, col_elevation;
  int col_u_vel, col_v_vel, col_w_vel;
  float dx, dy, dz;
  float diameter, length, azimuth, elevation;
  char *device_name;
  propdata *prop;
  sv_object *sphere, *smv_device;
  int vis_type;
  int maxpoints, ntypes;
  float *xyz, *rgb;
  int nvars_dep;
  int vars_dep_index[PROPVARMAX];
  float fvars_dep[PROPVARMAX];
  char *vars_dep[PROPVARMAX];
  flowlabels *labels;
} partclassdata;

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
  int npoints,n_rtypes, n_itypes;
  short *sx, *sy, *sz;
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
  int npoints;

  float zoffset, *times;
  unsigned char *times_map;
  int have_restart;
  FILE_SIZE reg_file_size, file_size;
  LINT *filepos;

  char menulabel[128];

  int nclasses;
  partclassdata **partclassptr;
  part5data *data5;
  histogramdata **histograms;
  int hist_update;
  int bounds_set;
  float *global_min, *global_max;
  float *valmin_fds, *valmax_fds;   // read in from .bnd files
  float *valmin_smv, *valmax_smv;   // computed by smokeview
  int nfilebounds;
  unsigned char *vis_part;
  int *tags;
  int *sort_tags;
  short *sx, *sy, *sz;
  unsigned char *irvals;
} partdata;

/* --------------------------  compdata ------------------------------------ */

typedef struct _compdata {
  int offset, size;
} compdata;


#define DUCT_COMPONENT_TEXT    0
#define DUCT_COMPONENT_SYMBOLS 1
#define DUCT_COMPONENT_HIDE    2

#define NODE_FILTERS_LABELS  0
#define NODE_FILTERS_SYMBOLS 1
#define NODE_FILTERS_HIDE    2

#define DUCT_XYZ 0
#define DUCT_YXZ 1
#define DUCT_XZY 2
#define DUCT_ZXY 3
#define DUCT_YZX 4
#define DUCT_ZYX 5


/* --------------------------  hvacconnectdata ------------------------------------ */

typedef struct hvacconnectdata {
  int index, display;
} hvacconnectdata;


/* --------------------------  hvacnodedata ------------------------------------ */

typedef struct _hvacnodedata {
  char *node_name, *vent_name, *duct_name, *network_name;
  char c_filter[10];
  int node_id, filter, use_node, connect_id;
  hvacconnectdata *connect;
  struct _hvacductdata *duct;
  float xyz[3], xyz_orig[3];
} hvacnodedata;

/* --------------------------  hvacductdata ------------------------------------ */

typedef struct _hvacductdata {
  char *duct_name, *network_name, c_component[4];
  int duct_id, component, nduct_cells;
  int node_id_from, node_id_to, use_duct, connect_id;
  hvacconnectdata *connect;
  int nact_times, *act_states, metro_path;
  float *act_times;
  float xyz_symbol[3], xyz_symbol_metro[3];
  float xyz_label[3],  xyz_label_metro[3];
  float normal[3], normal_metro[3];
  hvacnodedata* node_from, * node_to;
  float xyz_met[12], *xyz_reg;
  int   nxyz_met, nxyz_reg;
  float *xyz_met_cell, *xyz_reg_cell;
  int   nxyz_met_cell, nxyz_reg_cell;
  int    *cell_met,    *cell_reg;
} hvacductdata;

/* --------------------------  hvacdata ------------------------------------ */

typedef struct _hvacdata {
  char *network_name;
  int display;
  int show_node_labels, show_duct_labels;
  int show_filters, show_component;
  float cell_node_size, node_size, component_size, duct_width, filter_size;
  int duct_color[3], node_color[3];
} hvacdata;

/* --------------------------  hvacvaldata ------------------------------------ */

typedef struct _hvacvaldata{
  float *vals, valmin, valmax;
  int setvalmin, setvalmax;
  int vis, nvals;
  int firstshort;
  char  colorlabels[12][11];
  float colorvalues[12];
  float levels256[256];
  flowlabels label;
} hvacvaldata;

/* --------------------------  _hvacvalsdata ------------------------------------ */

typedef struct _hvacvalsdata {
  char *file;
  int loaded;
  int n_node_vars, n_duct_vars, ntimes;
  float *times;
  hvacvaldata *node_vars, *duct_vars;
} hvacvalsdata;

/* --------------------------  menudata ------------------------------------ */

typedef struct _menudata {
  int menuvar;
  int status;
#ifdef pp_DEBUG_SUBMENU
  int *menuvar_ptr;
#endif
  char label[256];
} menudata;

/* --------------------------  filebounddata ------------------------------------ */

typedef struct _fileboundsdata {
  char file[255];
  float valmin, valmax;
} fileboundsdata;

/* --------------------------  slicedata ------------------------------------ */

typedef struct _slicedata {
  int seq_id, autoload;
  char *file, *size_file, *bound_file;
  int have_bound_file;
  char *comp_file, *reg_file, *vol_file;
  char *geom_file;
  int nframes;
  int finalize;
  int slcf_index;
  char *slicelabel;
  int compression_type;
  int colorbar_autoflip;
  int ncompressed;
  int slice_filetype;
  struct _multislicedata *mslice;
  int is_fed;
  feddata *fedptr;
  int menu_show;
  float *constant_color;
  float qval256[256];
  int loaded, loading, display;
  int loaded_save, display_save;
  float position_orig;
  int blocknumber;
  int cell_center_edge;
  int firstshort_slice;
  int vec_comp;
  int skipdup;
  int setvalmin, setvalmax;
  float valmin, valmax;
  float globalmin, globalmax;
  float valmin_data, valmax_data;
  float valmin_fds, valmax_fds;   // read in from .bnd files
  float valmin_smv, valmax_smv;   // computed by smokeview
  float diff_valmin,  diff_valmax;
  flowlabels label;
  float *qslicedata, *qsliceframe, *times, *qslice;
  unsigned char *times_map;
  int have_restart;
  unsigned char *qslicedata_compressed;
  unsigned char *slicecomplevel;
  unsigned char full_mesh;
  contour *line_contours;
  int nline_contours;
  compdata *compindex;
  unsigned char *slicelevel;
  char menulabel[128];
  char menulabel2[128];
  float *rgb_slice_ptr[256];
  int ntimes,ntimes_old,itime;
  unsigned char *iqsliceframe;
  float above_ground_level;
  int have_agl_data;
  int volslice;
  int is1, is2, js1, js2, ks1, ks2;
  int iis1, iis2, jjs1, jjs2, kks1, kks2;
  int *imap, *jmap, *kmap;
  int n_imap, n_jmap, n_kmap;
  int plotx, ploty, plotz;
  int ijk_min[3], ijk_max[3];
  float xmin,xmax,ymin,ymax,zmin,zmax;
  float xyz_min[3], xyz_max[3];
  int nsliceijk;
  int *timeslist;
  char cdir[256];
  int idir;
  float sliceoffset;
  int nslicei, nslicej, nslicek;
  int nslicex, nslicey;
  int nslicetotal;
  int slicefile_labelindex;
  int vloaded, uvw;
  int cell_center;
  float delta_orig, dplane_min, dplane_max;
  int extreme_min, extreme_max;
  int hist_update;
  int nhistograms;
  histogramdata *histograms;
  histogramdata *histogram;
  struct _patchdata *patchgeom;
  FILE_SIZE file_size;
  int *geom_offsets;
  devicedata vals2d;
#ifdef pp_SLICE_MULTI
  int loadstatus;
#endif
} slicedata;

/* --------------------------  slicemenudata ------------------------------------ */

typedef struct _slicemenudata {
  slicedata *sliceinfo;
} slicemenudata;

/* --------------------------  multislicedata ------------------------------------ */

typedef struct _multislicedata {
  int seq_id, autoload;
  int loaded, display;
  int *islices, nslices;
  int slice_filetype;
  char menulabel[128];
  char menulabel2[128];
} multislicedata;

/* --------------------------  multivslicedata ------------------------------------ */

typedef struct _multivslicedata {
  int seq_id, autoload;
  int loaded,display,mvslicefile_labelindex;
  int nvslices;
  int *ivslices;
  char menulabel[128];
  char menulabel2[128];
} multivslicedata;

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
  int set_valmin, set_valmax, set_chopmin, set_chopmax;
  float valmin[4], valmax[4], chopmin, chopmax;
  float glui_valmin, glui_valmax;
  int set_valtype, cache;
  histogramdata *hist;
} cpp_boundsdata;
/* --------------------------  boundsdata ------------------------------------ */

typedef struct _boundsdata {
  char *shortlabel;
  int dlg_setvalmin, dlg_setvalmax;
  int setchopmin, setchopmax;
  float chopmin, chopmax;
  float dlg_valmin, dlg_valmax;
  float data_valmin,data_valmax;
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

#ifdef pp_BOUNDS
/* --------------------------  globalboundsdata ------------------------------------ */

typedef struct _globalboundsdata {
  char *file;
  int defined;
  int nbounds;
  float valmins_save[MAXPLOT3DVARS], valmaxs_save[MAXPLOT3DVARS];
  float valmins[MAXPLOT3DVARS],      valmaxs[MAXPLOT3DVARS];
} globalboundsdata;
#endif

/* --------------------------  vslicedata ------------------------------------ */

typedef struct _vslicedata {
  int seq_id, autoload, reload;
  slicedata *u,*v,*w,*val;
  int volslice;
  int iu, iv, iw, ival;
  int skip;
  int finalize;
  int loaded,display;
  float valmin, valmax;
  int vslice_filetype;
  int vslicefile_labelindex;
  char menulabel[128];
  char menulabel2[128];
} vslicedata;

/* --------------------------  smokedata ------------------------------------ */

typedef struct _smokedata {
  int ncomp_total;
  int *nchars_compressed, *nchars_compressed_full;
  unsigned char *frame_in, *frame_out, *view_tmp, *comp_all, **frame_comp_list;
} smokedata;


/* --------------------------  smokestatedata ------------------------------------ */

typedef struct {
  int loaded, index;
  unsigned char *color;
} smokestatedata;

  /* --------------------------  smoke3ddata ------------------------------------ */

typedef struct _smoke3ddata {
  int seq_id,autoload;
  char *file;
  char *comp_file, *reg_file;
#ifdef pp_SMOKE16
  char *s16_file;
#endif
  int filetype;
  int loaded, finalize, display, request_load, primary_file;
  int is_zlib;
#ifdef pp_SMOKE16
  int is_s16;
#endif
  smokestatedata *smokestate;
  int blocknumber;
  int type;
  int is1, is2, js1, js2, ks1, ks2;
  int compression_type, compression_type_temp;
  flowlabels label;
  char menulabel[128];
  float *times;
  unsigned char *times_map;
  int have_restart;
  int *use_smokeframe;
  int *smokeframe_loaded;
  float extinct, valmin, valmax;
  char cextinct[32];
#define ALPHA_X  0
#define ALPHA_Y  1
#define ALPHA_Z  2
#define ALPHA_XY 3
#define ALPHA_YZ 4
#define ALPHA_XZ 5
  unsigned char *alphas_dir[6];
  int fire_alpha, co2_alpha;
  float fire_alphas[256], co2_alphas[256];
  int *timeslist;
  int ntimes,ntimes_old,ismoke3d_time,lastiframe,ntimes_full;
  int nchars_uncompressed;

  int ncomp_smoke_total;
  int *nchars_compressed_smoke, *nchars_compressed_smoke_full;
#ifdef pp_SMOKE16
  unsigned short *val16s;
  float *val16_mins, *val16_maxs, *times16;
#endif
  float maxval;
  unsigned char *smokeframe_in, *smokeframe_out, **smokeframe_comp_list;
  unsigned char *smokeview_tmp;
  unsigned char *smoke_comp_all;
  unsigned char *frame_all_zeros;
  FILE_SIZE file_size;
  float *smoke_boxmin, *smoke_boxmax;
  smokedata smoke;
  int dir;
} smoke3ddata;

  /* --------------------------  smoke3dtypedata ------------------------------------ */

typedef struct smoke3dtypedata {
  char *shortlabel, *longlabel;
  int type;  // color based or opacity based
  int menu_id;
  smoke3ddata *smoke3d;
  float extinction, valmin, valmax;
} smoke3dtypedata;

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
  int version;
  int patch_filetype, structured;
  int shortlabel_index;
  int *cvals_offsets, *cvals_sizes;
  unsigned char *cbuffer;
  int is_compressed;
  int cbuffer_size;
  int boundary;
  int inuse,inuse_getbounds;
  int firstshort;
  int compression_type, compression_type_temp;
  int setvalmin, setvalmax;
  float valmin_fds, valmax_fds;   // read in from .bnd files
  float valmin_smv, valmax_smv;   // computed by smokeview
  float valmin, valmax;
  int setchopmin, setchopmax;
  float chopmin, chopmax;
  float diff_valmin, diff_valmax;
  int blocknumber,loaded,loaded2,display;
  float *geom_times, *geom_vals;
  int *geom_timeslist,geom_itime;
  unsigned char *geom_times_map;
  int have_restart;
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
  float valmin_fds[MAXPLOT3DVARS], valmax_fds[MAXPLOT3DVARS];   // read in from .bnd files
  float valmin_smv[MAXPLOT3DVARS], valmax_smv[MAXPLOT3DVARS];   // computed by smokeview
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

typedef struct {
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
