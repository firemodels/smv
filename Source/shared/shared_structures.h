#ifndef SHARED_STRUCTURES_H_DEFINED
#define SHARED_STRUCTURES_H_DEFINED
#include "isobox.h"
#include "options.h"
#include <stdio.h>
#ifdef pp_FRAME
#include "IOframe.h"
#endif

#if defined(WIN32)
#include <windows.h>
#endif
#include GLU_H
#include GL_H
#include "string_util.h"

#define PROPVARMAX 100

/* --------------------------  tokendata ------------------------------------ */

typedef struct _tokendata {
  float var, *varptr, default_val;
  int command, loc, type, reads, nvars, noutvars, is_label, is_string,
      is_texturefile;
  struct _sv_object *included_object;
  int included_frame;
  int texture_index;
  struct _tokendata *next, *elsenext;
  char token[64], tokenlabel[64], tokenfulllabel[64];
  char string[256], default_string[256], *stringptr;
} tokendata;

/* --------------------------  sv_object_frame ------------------------------------ */

/**
 * @brief The graphical definition of an object. This represents a single state.
 * This can form a node in a linked-list.
 */
typedef struct _sv_object_frame {
  int error;
  int *symbols, nsymbols;
  tokendata *tokens, **command_list;
  int ntokens, ncommands, ntextures;
  struct _sv_object *device;
  struct _sv_object_frame *prev, *next;
} sv_object_frame;


/* --------------------------  sv_object ------------------------------------ */

/**
 * @brief An object that can be rendered. This can form a node in a linked-list.
 */
typedef struct _sv_object {
  char label[256];
  /** @brief Is this object an avatar? */
  int type;
  int visible;
  int used, used_by_device;
  int select_mode;
  /** @brief The number of frames (i.e., possible states) associated with this
   * object. */
  int nframes;
  /** @brief A list of possible graphical representations of this object. While
   * described as a series of frames this is used as a number of different
   * possible states, not an animation. */
  sv_object_frame **obj_frames, first_frame, last_frame;
  /** @brief If this sv_object is part of a linked list, a pointer to the
   * previous sv_object in the list */
  struct _sv_object *prev;
  /** @brief If this sv_object is part of a linked list, a pointer to the
   * next sv_object in the list */
  struct _sv_object *next;
} sv_object;

#include "colorbars.h"
#include "scontour2d.h"
#include "csphere.h"
#include "histogram.h"
#include "isobox.h"
#include "stdio_m.h"
#include "string_util.h"

#define MAXPLOT3DVARS   6
#define MAXRGB        256
#define MAXFILELEN    360

/* --------------------------  outlinedata ---------------------------------- */

typedef struct _outlinedata {
  int nlines;
  float *x1, *y1, *z1;
  float *x2, *y2, *z2;
} outlinedata;

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

/* --------------------------  smokethreaddata ---------- ---------------------- */

typedef struct _smokethreaddata {
  int ithread, nthreads;
} smokethreaddata;

/* --------------------------  colortabledata ------------------------------- */

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

/* --------------------------  keyframe ------------------------------------ */

typedef struct _keyframe {
  int selected, npoints;
  int set_tour_time;
  float time;
  float pause_time, cum_pause_time;
  float view_smv[3], view2_smv[3];
  float xyz_fds[3], xyz_smv[3];
  float arc_dist, line_dist, xyz_diff[3], view_diff[3];
  float xyz_tangent_left[3], view_tangent_left[3];
  float xyz_tangent_right[3], view_tangent_right[3];
  struct _keyframe *next, *prev;
} keyframe;

/* --------------------------  tourdata ------------------------------------ */

typedef struct _tourdata {
  char label[300], menulabel[128];
  keyframe first_frame, last_frame, **keyframe_list;
  int glui_avatar_index, display2;
  float *path_times, *keyframe_times;
  float xyz_smv[3], view_smv[3];
  float global_dist;
  int *timeslist;
  int ntimes, nkeyframes;
  int display, periodic;
  int startup;
  int isDefault;
} tourdata;

/* --------------------------  texturedata ---------------------------------- */

typedef struct _texturedata {
  char *file;
  int loaded, display, used, is_transparent;
  GLuint name;
} texturedata;

/* --------------------------  terraindata ---------------------------------- */

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

/* --------------------------  mesh ----------------------------------------- */

typedef struct _meshdata {
  int ibar, jbar, kbar;
  float cellsize;
  int ncvents, nvents, ndummyvents;
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
  GLuint volsmoke_texture_id, volfire_texture_id;
  float *volsmoke_texture_buffer, *volfire_texture_buffer;
  int voltest_update;
  GLuint slice3d_texture_id;
  float *slice3d_c_buffer;
#ifdef pp_WINGPU
  float *slice3d_texture_buffer;
#endif
#endif
  float meshrgb[3], *meshrgb_ptr;
  float mesh_offset[3], *mesh_offset_ptr;
  int blockvis, datavis;
  float *xplt, *yplt, *zplt;
  double *xpltd, *ypltd, *zpltd;
  int ivolbar, jvolbar, kvolbar;
  float *xvolplt, *yvolplt, *zvolplt;
  float *xplt_cen, *yplt_cen, *zplt_cen;
  float *xplt_orig, *yplt_orig, *zplt_orig;
  float x0, x1, y0, y1, z0, z1;
  int drawsides[7];
  int extsides[7];   // 1 if on exterior side of a supermesh, 0 otherwise
  int is_extface[6]; //  MESH_EXT if face i is completely adjacent to exterior,
                     //  MESH_INT if face i is completely adjacent to another
                     //  mesh,
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
  int plot3dfilenum, isofilenum, patchfilenum;
  int *iplotx_all, *iploty_all, *iplotz_all;
  int plotx, ploty, plotz;
  int slicedir;
  int plotn;
  int *imap, *jmap, *kmap;
  int n_imap, n_jmap, n_kmap;

  unsigned char *boundary_mask;
  char *c_iblank_node0,      *c_iblank_cell0,      *c_iblank_x0,      *c_iblank_y0,      *c_iblank_z0;
  char *c_iblank_node0_temp, *c_iblank_cell0_temp, *c_iblank_x0_temp, *c_iblank_y0_temp, *c_iblank_z0_temp;
  char *c_iblank_node_html;
  char *c_iblank_node_html_temp;
  float *f_iblank_cell0;
  float *f_iblank_cell0_temp;
  char *c_iblank_embed0;
  float *block_zdist0;
  float *opacity_adjustments;
  unsigned char *is_firenode, *is_firenodeptr;

  char *c_iblank_node,      *c_iblank_cell,      *c_iblank_x,      *c_iblank_y,      *c_iblank_z;
  char *c_iblank_node_temp, *c_iblank_cell_temp, *c_iblank_x_temp, *c_iblank_y_temp, *c_iblank_z_temp;
  float *f_iblank_cell;
  float *f_iblank_cell_temp;
  char *c_iblank_embed;
  float *block_zdist;

  int zdist_flag;
  unsigned char *iblank_smoke3d;
  int iblank_smoke3d_defined;
  struct _blockagedata **blockageinfoptrs, *blockageinfo, **bc_faces[6];
  int  n_bc_faces[6];
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
  float plot3d_speedmax, plot3d_uvw_max;
  struct _contour *plot3dcontour1, *plot3dcontour2, *plot3dcontour3;
  struct _isosurface *currentsurf, *currentsurf2;
  struct _isosurface *blockagesurface;
  struct _isosurface **blockagesurfaces;

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

  struct _meshdata *skip_nabors[6], *nabors[6];
  struct _supermeshdata *super;

  int *ptype;
  unsigned int *zipoffset, *zipsize;

  float *xyzpatch_offset, *xyzpatch_no_offset, *xyzpatch_threshold;
  float *thresholdtime;
  int *patchblank;

  unsigned char *cpatchval_zlib, *cpatchval_iframe_zlib;
  unsigned char *cpatchval, *cpatchval_iframe;
  float *patch_times, *patch_timesi, *patchval;
#ifndef pp_BOUNDFRAME
  float *patchval_iframe;
#endif
  unsigned char *patch_times_map;
  float **patchventcolors;
  int patch_itime;
  int *patch_timeslist;
  int npatchsize;
  int visInteriorBoundaries;

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

/* --------------------------  cellmeshdata --------------------------------- */

typedef struct _cellmeshdata {
  int nxyz[3];
  float xyzminmax[6], dxyz[3];
  meshdata **cellmeshes;
} cellmeshdata;

/* --------------------------  supermeshdata -------------------------------- */

typedef struct _supermeshdata {
#ifdef pp_GPU
  GLuint blockage_texture_id;
  GLuint volsmoke_texture_id, volfire_texture_id;
  float *volsmoke_texture_buffer, *volfire_texture_buffer;
#endif
  float *f_iblank_cell;
  float boxmin_scaled[3], boxmax_scaled[3];
  int drawsides[7];
  int nmeshes;
  meshdata **meshes;
  int ibar, jbar, kbar;
} supermeshdata;

/* --------------------------  propdata ------------------------------------- */
#define PROPVARMAX 100
typedef struct _propdata {
  char *label;
  int menu_id;
  float *rotate_axis, rotate_angle;
  int inblockage, blockvis;
  int nsmokeview_ids, ismokeview_ids;
  char *smokeview_id, **smokeview_ids;
  struct _sv_object *smv_object, **smv_objects;
  int ntextures;
  char **texturefiles, **vars_indep, **svals;
  int *vars_indep_index, vars_dep_index[PROPVARMAX];
  int nvars_indep, nvars_dep;
  float *fvals, fvars_dep[PROPVARMAX];
  int tag_number;
} propdata;

/* --------------------------  partpropdata --------------------------------- */

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

/* ------------------------- partclassdata ---------------------------------- */

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

/* --------------------------  part5data ------------------------------------ */

typedef struct _part5data {
  partclassdata *partclassbase;
  float time;
  int npoints_file, npoints_loaded, n_rtypes, n_itypes;
  float *xpos, *ypos, *zpos;
  float *dsx, *dsy, *dsz;
  float *avatar_angle, *avatar_width, *avatar_depth, *avatar_height;
  int *tags, *sort_tags;
  unsigned char *vis_part;
  float *rvals, **rvalsptr;
  unsigned char *irvals;
  unsigned char **cvals;
} part5data;

/* --------------------------  partdata ------------------------------------- */

typedef struct _partdata {
  FILE_m *stream;

  char *file, *size_file, *reg_file, *bound_file;
  int have_bound_file;
  int seq_id, autoload, loaded, skipload, request_load, display, reload,
      finalize;
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
  float *valmin_part, *valmax_part;
  int nfilebounds;
  unsigned char *vis_part;
  int *sort_tags;
  unsigned char *irvals;
#ifdef pp_PARTFRAME
  framedata *frameinfo;
#endif
} partdata;

/* --------------------------  device --------------------------------------- */

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
  int ival, nvals, type2, type2vis;
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
  struct _sv_object *object;
  struct _vdevicedata *vdevice;
  int type, is_beam;
  int selected;
  int inlist;
  int valid;
} devicedata;

/* --------------------------  windrosedata --------------------------------- */

typedef struct _windrosedata {
  histogramdata histogram[3];
  float *xyz;
} windrosedata;

/* --------------------------  vdevicedata ---------------------------------- */

typedef struct _vdevicedata {
  int unique, filetype, display;
  int nwindroseinfo;
  windrosedata *windroseinfo;
  devicedata *udev, *vdev, *wdev, *valdev, *colordev, *veldev, *angledev,
      *sd_veldev, *sd_angledev;
} vdevicedata;

/* --------------------------  vdevicesortdata ------------------------------ */

typedef struct _vdevicesortdata {
  int dir;
  vdevicedata *vdeviceinfo;
} vdevicesortdata;

/* --------------------------  clipdata  ------------------------------------ */

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

/* --------------------------  compdata ------------------------------------ */

typedef struct _compdata {
  int offset, size;
} compdata;

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
  unsigned char *slice_mask;
  int compression_type;
  int colorbar_autoflip;
  FILE_SIZE ncompressed;
  int slice_filetype;
  struct _multislicedata *mslice;
  int menu_show;
  float *constant_color;
  float qval256[256];
  int loaded, loading, display;
  int loaded_save, display_save;
  float position_orig;
  int blocknumber;
  int cell_center_edge;
  int vec_comp;
  int skipdup;
  int setvalmin, setvalmax;
  float globalmin_slice, globalmax_slice;
  float valmin_slice, valmax_slice;
  float diff_valmin,  diff_valmax;
  flowlabels label;
  float *qslicedata, *qsliceframe, *times, *qslice;
  unsigned char *times_map;
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
  int idir, fds_dir;
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
#ifdef pp_SLICEFRAME
  framedata *frameinfo;
#endif
} slicedata;

/* --------------------------  multislicedata ------------------------------------ */

typedef struct _multislicedata {
  int seq_id, autoload;
  int loaded, display, loadable;
  int *islices, nslices;
  int slice_filetype;
  char menulabel[128];
  char menulabel2[128];
} multislicedata;

/* --------------------------  multivslicedata ------------------------------------ */

typedef struct _multivslicedata {
  int seq_id, autoload;
  int loaded,display,mvslicefile_labelindex,loadable;
  int nvslices;
  int *ivslices;
  char menulabel[128];
  char menulabel2[128];
} multivslicedata;

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

/* --------------------------  circdata ------------------------------------- */

typedef struct _circdata {
  float *xcirc, *ycirc;
  int ncirc;
} circdata;

/* --------------------------  meshescollection ------------------------------------- */

typedef struct {
  meshdata *meshinfo;
  int nmeshes;
} meshescollection;

/* --------------------------  hrrdata ------------------------------------ */

typedef struct _hrrdata {
  float *vals, *vals_orig, valmin, valmax;
  int inlist1;
  int nvals;
  int base_col;
  flowlabels label;
} hrrdata;

/* --------------------------  surfdata ------------------------------------ */

typedef struct _surfdata {
  char *surfacelabel, *texturefile;
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
  int used_by_obst, used_by_vent;
  int used_by_geom;
  int geom_surf_color[3];
  float axis[3];
  int in_geom_list;
  int ntris;
  int in_color_dialog;
  float geom_area;
} surfdata;

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
  int patch_face_index[6];
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

/* --------------------------  ventdata ------------------------------------ */

typedef struct _ventdata {
  int type,dummy;
  struct _ventdata *dummyptr;
  int hideboundary;
  int dir,dir2,vent_id;
  int useventcolor;
  int isOpenvent, isMirrorvent;
  int wall_type;
  int patch_index;
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

/* --------------------------  plot3ddata ----------------------------------- */

typedef struct _plot3ddata {
  int seq_id, autoload;
  char *file, *reg_file, *bound_file;
  int have_bound_file;
  int finalize;
  int memory_id;
  float time;
  int u, v, w, nplot3dvars;
  float diff_valmin[MAXPLOT3DVARS], diff_valmax[MAXPLOT3DVARS];
  int extreme_min[MAXPLOT3DVARS], extreme_max[MAXPLOT3DVARS];
  int blocknumber, loaded, display, loadnow;
  float valmin_plot3d[MAXPLOT3DVARS], valmax_plot3d[MAXPLOT3DVARS];
  flowlabels label[MAXPLOT3DVARS];
  char menulabel[256], longlabel[256], timelabel[256];
  histogramdata *histograms[MAXPLOT3DVARS];
  int hist_update;
} plot3ddata;

/* --------------------------  zonedata ------------------------------------ */

typedef struct _zonedata {
  int seq_id, autoload;
  char *file, *basefile;
  int loaded, display;
  int csv;
  flowlabels label[4];
  int setvalmin, setvalmax;
  float valmin, valmax;
} zonedata;

/* --------------------------  roomdata ------------------------------------ */

typedef struct _roomdata {
  int valid;
  float dx, dy, dz;
  float x0, y0, z0;
  float x1, y1, z1;
  int drawsides[7];
  float pfloor, ylay, tl, tu, rho_L, rho_U, od_L, od_U;
  int itl, itu;
  int zoneinside;
} roomdata;

/* --------------------------  zventdata ------------------------------------ */

#define MAX_HSLABS 10
#define MAX_VSLABS 2
#define MAX_MSLABS 2
#define NELEV_ZONE 100

typedef struct _zventdata {
  int wall, nslab;
  float x0, y0, z0;
  float x1, y1, z1;
  float xcen, ycen, radius;
  float vmin, vmax;
  float g_vmin, g_vmax;
  roomdata *room1, *room2;
  float area, area_fraction;
  float slab_bot[MAX_HSLABS], slab_top[MAX_HSLABS], slab_vel[MAX_HSLABS],
      slab_temp[MAX_HSLABS];
  float *color;
  float vdata[NELEV_ZONE];
  int itempdata[NELEV_ZONE];
  int vent_type, vertical_vent_type;
} zventdata;

/* --------------------------  firedata ------------------------------------ */

typedef struct _firedata {
  float x, y, z, dz;
  float absx, absy, absz;
  int valid, roomnumber;
} firedata;

/* --------------------------  tickdata ------------------------------------ */

typedef struct _tickdata {
  float begin[3], end[3], length;
  float dxyz[3], dlength;
  int dir, nbars, useforegroundcolor;
  float width, rgb[3];
} tickdata;

/* --------------------------  cventdata ------------------------------------ */

typedef struct _cventdata {
  int dir, type, colorindex, cvent_id, isOpenvent;
  float boxmin[3], boxmax[3], texture_origin[3];
  float xmin, xmax, ymin, ymax, zmin, zmax;
  unsigned char *blank0, *blank;
  int imin, imax, jmin, jmax, kmin, kmax;
  int nshowtime, *showtimelist;
  float *showtime;
  unsigned char *showhide;
  int useventcolor, hideboundary, have_boundary_file;
  float origin[3], radius;
  float *color;
  surfdata *surf[1];
  texturedata *textureinfo[1];
} cventdata;

/* --------------------------  fueldata ------------------------------------ */

typedef struct _fueldata {
  char *fuel;
  float hoc;
} fueldata;

/* --------------------------  hrr_collection ------------------------------------ */

typedef struct {
  int nhrrinfo;
  int nhrrhcinfo;
  hrrdata *hrrinfo;
} hrr_collection;

/* --------------------------  fuel_collection ------------------------------------ */

typedef struct {
  int nfuelinfo;
  fueldata *fuelinfo;
} fuel_collection;

/* --------------------------  prop_collection ------------------------------------ */

typedef struct {
  int npropinfo;
  propdata *propinfo;
} prop_collection;

/* --------------------------  obst_collection ------------------------------------ */

typedef struct {
  int nobstinfo;
  xbdata *obstinfo;
} obst_collection;

/* --------------------------  texture_collection ------------------------------------ */

typedef struct {
  int ntextureinfo;
  texturedata *textureinfo;
} texture_collection;

/* --------------------------  terrain_texture_collection ------------------------------------ */

typedef struct {
  int nterrain_textures;
  texturedata *terrain_textures;
} terrain_texture_collection;

/* --------------------------  device_collection ------------------------------------ */

typedef struct {
  int ndeviceinfo;
  int nvdeviceinfo;
  int ndeviceinfo_exp;
  devicedata *deviceinfo;
  vdevicedata *vdeviceinfo;
  vdevicesortdata *vdevices_sorted;
} device_collection;

/* --------------------------  device_texture_list_collection ------------------------------------ */

typedef struct {
  int ndevice_texture_list;
  char **device_texture_list;
  int *device_texture_list_index;
  // texturedata *device_textures;
} device_texture_list_collection;

/* --------------------------  slice_collection ------------------------------------ */

typedef struct {
  int nsliceinfo;
  slicedata *sliceinfo;

  int nmultisliceinfo;
  multislicedata *multisliceinfo;

  int nvsliceinfo;
  vslicedata *vsliceinfo;

  int nmultivsliceinfo;
  multivslicedata *multivsliceinfo;
} slice_collection;

/* --------------------------  sliceparmdata ------------------------------------ */

typedef struct _sliceparmdata {
  int nsliceinfo;
  int nvsliceinfo;
  int nmultisliceinfo;
  int nmultivsliceinfo;
} sliceparmdata;

/* --------------------------  surf_collection ------------------------------------ */

typedef struct {
  int nsurfinfo;
  surfdata *surfinfo;

  int nsurfids;
  surfid *surfids;

  int nsorted_surfidlist;
  int *sorted_surfidlist;
  int *inv_sorted_surfidlist;

} surf_collection;

/* --------------------------  casepaths ------------------------------------ */

typedef struct {
  char *fds_filein;
  char *chidfilebase;
  char *hrr_csv_filename;
  char *devc_csv_filename;
  char *exp_csv_filename;
  char *stepcsv_filename;

  // The base names are properties of the GUI
  // char *movie_name;
  // char *render_file_base;
  // char *html_file_base;
  char *log_filename;
  char *caseini_filename;
  char *fedsmv_filename;
#ifdef pp_FRAME
  char *frametest_filename;
#endif
  char *expcsv_filename;
  char *dEcsv_filename;
  char *html_filename;
  char *smv_orig_filename;
  char *hrr_filename;
  char *htmlvr_filename;
  char *htmlobst_filename;
  char *htmlslicenode_filename;
  char *htmlslicecell_filename;
  char *event_filename;
  char *ffmpeg_command_filename;
  char *fed_filename;
  char *smvzip_filename;
  char *sliceinfo_filename;
  char *deviceinfo_filename;
  char *iso_filename;
  char *trainer_filename;
  char *test_filename;
} casepaths;

/* --------------------------  csvdata ------------------------------------ */

typedef struct _csvdata {
  flowlabels label;
  float val, *vals, *vals_orig;
  float valmin, valmax;
  int nvals;
  int dimensionless, skip;
} csvdata;

/* --------------------------  _csvfiledata --------------------------------- */

#define CSV_FDS_FORMAT 0
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

/* --------------------------  csv_collection ------------------------------------ */

typedef struct {
  int ncsvfileinfo;
  csvfiledata *csvfileinfo;
} csv_collection;

/* --------------------------  volrenderdata ------------------------------------ */

typedef struct _volrenderdata {
  char *rendermeshlabel;
  int is_compressed;
  struct _slicedata *smokeslice, *fireslice;
  unsigned char *c_smokedata_view, *c_firedata_view;
  int *nsmokedata_compressed, *nfiredata_compressed;
  float *smokedata_full, *firedata_full;
  float *smokedata_view, *firedata_view;
  LINT *smokepos, *firepos;
  void *smokedataptr, *firedataptr;
  void **smokedataptrs, **firedataptrs;
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
  float verts[6 * 3], verts_smv[6 * 3];
  float norm0[4 * 3], norm1[4 * 3];
  int have_vals[3];
  int triangles[4 * 3], nverts, ntriangles;
  float *vals2, *verts2;
  int *tris2, nverts2, ntris2;
  int polys[10], npolys;
  int drawsmoke;
} meshplanedata;

/* --------------------------  filelist_collection -------------------------- */

typedef struct {
  int nini_filelist;
  filelistdata *ini_filelist;

  int nfilelist_casename;
  filelistdata *filelist_casename;

  int nfilelist_casedir;
  filelistdata *filelist_casedir;
} filelist_collection;

/* --------------------------  vertdata ------------------------------------ */

typedef struct _vertdata {
  float xyz[3], vert_norm[3], texture_xy[3];
  int itriangle, ntriangles, nused;
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
} tridata;

/* --------------------------  geomlistdata --------------------------------- */

typedef struct _geomlistdata {
  int nverts, nedges, ntriangles, norms_defined;
  float *zORIG;
  vertdata *verts;
  float *vertvals;
  tridata *triangles, **triangleptrs, **connected_triangles;
  edgedata *edges;
} geomlistdata;

/* --------------------------  geomobjdata ---------------------------------- */

typedef struct _geomobjdata {
  struct _surfdata *surf;
  struct _texturedata *texture;
  float *bounding_box, *color;
  char *texture_name;
  float texture_width, texture_height, texture_center[3];
  int texture_mapping;
  int use_geom_color, ntriangles;
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
  int ntimes, itime, *timeslist;
  int ngeomobjinfo, geomtype, patchactive, fdsblock;
  struct _surfdata *surfgeom;
  geomlistdata *geomlistinfo, *geomlistinfo_0, *currentframe;
  geomobjdata *geomobjinfo;
  int *geomobj_offsets;
  int ngeomobj_offsets;
#ifdef pp_ISOFRAME
  framedata *frameinfo;
#endif
} geomdata;

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
  char *file, *tfile;
  int dataflag, geomflag, get_isolevels;
  int type;
  int firstshort_iso;
  flowlabels surface_label, color_label;
  geomdata *geominfo;
  int blocknumber, display, loaded, loading;
  float tmin, tmax;
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

/* --------------------------  boundsdata ----------------------------------- */

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
  char colorlabels[12][11];
  float colorvalues[12];
  float levels256[256];
  flowlabels *label;
} boundsdata;

/* --------------------------  bounddata ------------------------------------ */

typedef struct _boundata {
  float global_min, global_max;
  int defined;
} bounddata;

/* --------------------------  globalboundsdata ----------------------------- */

typedef struct _globalboundsdata {
  char *file;
  int defined;
  int nbounds;
  float valmins_save[MAXPLOT3DVARS], valmaxs_save[MAXPLOT3DVARS];
  float valmins[MAXPLOT3DVARS], valmaxs[MAXPLOT3DVARS];
} globalboundsdata;

/* --------------------------  patchfacedata -------------------------------- */

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

/* --------------------------  std_objects ------------------------------------ */

/**
 * @brief A number of standard objects to be used.
 *
 */
typedef struct {
  sv_object *thcp_object_backup;
  sv_object *target_object_backup;
  sv_object *heat_detector_object_backup;
  sv_object *sprinkler_upright_object_backup;
  sv_object *smoke_detector_object_backup;
  sv_object *error_device;
  sv_object *missing_device;
} std_objects;

/* --------------------------  object_collection ------------------------------------ */

/**
 * @brief A collection of object definitions. At it's core this collection
 * contains a linked list, but also an array of pointers into that linked list.
 *
 */
typedef struct {
  /** @brief The number object definitions in object_defs. */
  int nobject_defs;
  /** @brief An array of pointers object definitions. */
  sv_object **object_defs;
  /** @brief The start of a linked list of object definitions. This is a dummy
   * object and isn't actually used. */
  sv_object object_def_first;
  /** @brief The end of a linked list of object definitions. This is a dummy
   * object and isn't actually used. */
  sv_object object_def_last;
  /** @brief A number of standard objects to be used. */
  std_objects std_object_defs;
  int iavatar_types;
  int navatar_types;
  sv_object **avatar_types;
  sv_object *avatar_defs_backup[2];
} object_collection;

/* --------------------------  smokedata ------------------------------------ */

typedef struct _smokedata {
  int ncomp_total;
  int *nchars_compressed, *nchars_compressed_full;
  unsigned char *frame_in, *frame_out, *view_tmp, *comp_all, **frame_comp_list;
} smokedata;

/* --------------------------  smokestatedata ------------------------------- */

typedef struct {
  int loaded, index;
  unsigned char *color;
} smokestatedata;

/* --------------------------  smoke3ddata ---------------------------------- */

typedef struct _smoke3ddata {
  int seq_id, autoload;
  char *file;
  char *comp_file, *reg_file;
  char *smoke_density_file;
  int filetype;
  int skip_smoke, skip_fire;
  int is_smoke, is_fire;
  int loaded, request_load, finalize, display, primary_file;
  int is_zlib;
  int is_smoke_density;
  int soot_density_loaded;
  smokestatedata *smokestate;
  int blocknumber;
  int type;
  int is1, is2, js1, js2, ks1, ks2;
  int compression_type, compression_type_temp;
  flowlabels label;
  char menulabel[128];
  float *times;
  unsigned char *times_map;
  int *use_smokeframe;
  int *smokeframe_loaded;
  float extinct, valmin, valmax;
  char cextinct[32];
#define ALPHA_X 0
#define ALPHA_Y 1
#define ALPHA_Z 2
#define ALPHA_XY 3
#define ALPHA_YZ 4
#define ALPHA_XZ 5
  unsigned char *alphas_smokedir[6], *alphas_firedir[6];
  unsigned char alphas_smokebuffer[6*256], alphas_firebuffer[6*256];
  int fire_alpha, co2_alpha;
  float fire_alphas[256], co2_alphas[256];
  int *timeslist;
  int ntimes, ntimes_old, ismoke3d_time, lastiframe, ntimes_full;
  int nchars_uncompressed;

  int ncomp_smoke_total;
  int *nchars_compressed_smoke, *nchars_compressed_smoke_full;
  float *maxvals;
  float maxval;
  unsigned char *smokeframe_in, *smokeframe_out, **smokeframe_comp_list;
  unsigned char *smokeview_tmp;
#ifndef pp_SMOKEFRAME
  unsigned char *smoke_comp_all;
#endif
  unsigned char *frame_all_zeros;
  FILE_SIZE file_size;
  float *smoke_boxmin, *smoke_boxmax;
  smokedata smoke;
  int dir;
#ifdef pp_SMOKEFRAME
  framedata *frameinfo;
#endif
} smoke3ddata;

/* --------------------------  smoke3dtypedata ------------------------------ */

typedef struct _smoke3dtypedata {
  char *shortlabel, *longlabel;
  int type; // color based or opacity based
  int menu_id;
  smoke3ddata *smoke3d;
  float extinction, valmin, valmax;
} smoke3dtypedata;

/* --------------------------  smoke3d_collection ------------------------------------ */

typedef struct {
  int nsmoke3dinfo;
  smoke3ddata *smoke3dinfo;
  smoke3ddata **smoke3dinfo_sorted;
  int nsmoke3dtypes;
  int smoke3d_other;
  smoke3dtypedata *smoke3dtypes;
} smoke3d_collection;

/* --------------------------  tour_collection ------------------------------------ */

typedef struct {
  /// @brief Number of tours. This is the length of the array tourinfo
  int ntourinfo;
  /// @brief An array of tours.
  tourdata *tourinfo;
  /// @brief The number of times in each tour. This is the same across all
  /// tours.
  int tour_ntimes;
  float *tour_t;
  float *tour_t2;
  float *tour_dist;
  float *tour_dist2;
  float *tour_dist3;
  float tour_tstart;
  float tour_tstop;
} tour_collection;

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

/* --------------------------  cadgeomdata ------------------------------------ */

typedef struct _cadgeomdata {
  char *file;
  int *order;
  int version;
  int ncadlookinfo;
  cadlookdata *cadlookinfo;
  int nquads;
  cadquad *quad;
} cadgeomdata;

/* --------------------------  cadgeom_collection ------------------------------------ */

/**
 * @brief A collection of CAD geometry definitions. This is allocated with a
 * fixed capacity.
 *
 */
typedef struct {
  /** @brief The capacity of the array. This cannot be changed after creation.
   */
  int capacity;
  /** @brief The number of CAD geometry objects currently defined. */
  int ncadgeom;
  /** @brief The array of object definitions. The length of this array is @ref
   * capacity but only the first @ref ncadgeom entries contain valid
   * information. */
  cadgeomdata *cadgeominfo;
} cadgeom_collection;

/* --------------------------  hvacconnectdata ------------------------------ */

typedef struct hvacconnectdata {
  int index, display;
} hvacconnectdata;

/* --------------------------  hvacnodedata --------------------------------- */

typedef struct _hvacnodedata {
  char *node_name, *vent_name, *duct_name, *network_name;
  char c_filter[10];
  int node_id, filter, use_node, connect_id;
  hvacconnectdata *connect;
  struct _hvacductdata *duct;
  float xyz[3], xyz_orig[3];
} hvacnodedata;

/* --------------------------  hvacductdata --------------------------------- */

typedef struct _hvacductdata {
  char *duct_name, *network_name, c_component[4];
  int duct_id, component, nduct_cells;
  int node_id_from, node_id_to, use_duct, connect_id;
  hvacconnectdata *connect;
  int nact_times, *act_states, metro_path;
  float *act_times;
  float xyz_symbol[3], xyz_symbol_metro[3];
  float xyz_label[3], xyz_label_metro[3];
  float normal[3], normal_metro[3];
  hvacnodedata *node_from, *node_to;
  float xyz_met[12], *xyz_reg;
  int nxyz_met, nxyz_reg;
  float *xyz_met_cell, *xyz_reg_cell;
  int nxyz_met_cell, nxyz_reg_cell;
  int *cell_met, *cell_reg;
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

/* --------------------------  hvacvaldata ---------------------------------- */

typedef struct _hvacvaldata {
  float *vals, valmin, valmax;
  int setvalmin, setvalmax;
  int vis, nvals;
  char colorlabels[12][11];
  float colorvalues[12];
  float levels256[256];
  flowlabels label;
} hvacvaldata;

/* --------------------------  hvacvalsdata --------------------------------- */

typedef struct _hvacvalsdata {
  char *file;
  int loaded;
  int n_node_vars, n_duct_vars, ntimes;
  float *times;
  hvacvaldata *node_vars, *duct_vars;
} hvacvalsdata;

/* --------------------------  hvacdatacollection ------------------------------------ */

typedef struct {
  hvacnodedata *hvacnodeinfo;
  int nhvacnodeinfo;
  hvacductdata *hvacductinfo;
  int nhvacductinfo;
  hvacconnectdata *hvacconnectinfo;
  int nhvacconnectinfo;
  hvacdata *hvacinfo;
  int nhvacinfo;

  int nhvacfilters;

  int nhvaccomponents;

  hvacvalsdata *hvacductvalsinfo;
  int hvacductvar_index;
  hvacvalsdata *hvacnodevalsinfo;
  int hvacnodevar_index;

  int hvac_maxcells;
  int hvac_n_ducts;

} hvacdatacollection;

/* --------------------------  labeldata ------------------------------------ */

typedef struct _labeldata {
  struct _labeldata *prev, *next;
  char name[300];
  float xyz[3], frgb[4], tstart_stop[2];
  float tick_begin[3], tick_direction[3];
  int show_tick;
  int rgb[4], glui_id, labeltype; // smv or ini
  int useforegroundcolor, show_always;
} labeldata;

/**
 * @brief A collection of labels. At it's core this collection
 * contains a linked list, but also an array of pointers into that linked list.
 *
 */

/* --------------------------  labels_collection ------------------------------------ */

typedef struct {
  labeldata label_first;
  labeldata label_last;
  labeldata *label_first_ptr;
  labeldata *label_last_ptr;
} labels_collection;

/* --------------------------  smv_case ------------------------------------ */

typedef struct {
  char *fdsprefix;
  char *fds_title;
  char *fds_version;
  char *fds_githash;

  meshescollection meshescoll;
  colordata *firstcolor;
  prop_collection propcoll;
  object_collection objectscoll;
  hrr_collection hrr_coll;
  smoke3d_collection smoke3dcoll;
  surf_collection surfcoll;
  slice_collection slicecoll;
  texture_collection texture_coll;
  terrain_texture_collection terrain_texture_coll;
  device_texture_list_collection device_texture_list_coll;
  device_collection devicecoll;
  obst_collection obstcoll;
  csv_collection csvcoll;
  tour_collection tourcoll;
  fuel_collection fuelcoll;
  cadgeom_collection cadgeomcoll;
  hvacdatacollection hvaccoll;
  labels_collection labelscoll;

  casepaths paths;

  int nplot3dinfo;
  plot3ddata *plot3dinfo;

  int npartclassinfo;
  partclassdata *partclassinfo;

  int npatchinfo;
  patchdata *patchinfo;

  int nisoinfo;
  isodata *isoinfo;

  int nboundarytypes;
  int *boundarytypes;

  int nisotypes;
  int *isotypes;

  int nrooms;
  roomdata *roominfo;

  int nfires;
  firedata *fireinfo;

  int nzoneinfo;
  zonedata *zoneinfo;

  int nzvents;
  zventdata *zventinfo;

  int npartinfo;
  partdata *partinfo;

  int ngeominfo;
  geomdata *geominfo;

  int ncgeominfo;
  geomdata *cgeominfo;

  float obst_bounding_box[6];

  int nsupermeshinfo;
  supermeshdata *supermeshinfo;

  /// @brief The HoC of the fuel if present. -1.0 otherwise.
  float fuel_hoc;
  /// @brief Is this case a terrain case?
  int is_terrain_case;
  /// @brief Is this case a zone model?
  int isZoneFireModel;
  int have_object_box;
  int have_beam;
  int have_missing_objects;
  int have_hidden6;
  int show_hrrcutoff_active;
  int show_tempcutoff_active;
  int ntotal_blockages;
  int hasSensorNorm;
  char *texturedir;
  int smokediff;
  int have_northangle;
  int usetextures;
  /// @brief SMV file contains DEVICE_ACT entries
  int update_device;
  /// @brief Number of transparent vents
  int nvent_transparent;
  int ntrnx;
  int ntrny;
  int ntrnz;
  int npdim;
  int nVENT;
  int nCVENT;
  int ncvents;
  int nOBST;
  int noffset;
  int ngeom_data;
  int clip_mesh;
  int setPDIM;
  int zonecsv;
  int nzhvents;
  int nzvvents;
  int nzmvents;
  int ntreeinfo;
  char surfacedefaultlabel[256];
  int nterraininfo;
  int noutlineinfo;
  int ntickinfo;
  int ntickinfo_smv;
  int ntc_total;
  int nspr_total;
  int nheat_total;
  int ndummyvents;
  int clip_I;
  int clip_J;
  int clip_K;
  int visTerrainType;
  int have_gvec;
  outlinedata *outlineinfo;
  treedata *treeinfo;
  terraindata *terraininfo;
  spherepoints *sphereinfo;
  spherepoints *wui_sphereinfo;
  int solid_ht3d;
  float northangle;
  int niso_compressed;
  int have_compressed_files;
  int have_cface_normals;
  int cellcenter_slice_active;
  int auto_terrain;
  int manual_terrain;
  float gvecphys[3];
  float gvecunit[3];
  float global_tbegin;
  float global_tend;
  float tload_begin;
  float tload_end;
  float load_hrrpuv_cutoff;
  float global_hrrpuv_cutoff;
  float global_hrrpuv_cutoff_default;
  float smoke_albedo;
  float smoke_albedo_base;

  // TODO: the below probably don't really belong here
  filelist_collection filelist_coll;
  float xbar;
  float ybar;
  float zbar;
  float xbar0;
  float ybar0;
  float zbar0;
  char *part_buffer;
  char *smoke3d_buffer;
  char *slice_buffer;
  // TODO: this definitely doesn't belong here
  int curdir_writable;
  char *smokeview_scratchdir;
  int update_smoke_alphas;
  int *sliceorderindex;
  int *vsliceorderindex;
  float texture_origin[3];

  int updateindexcolors;
  int updatefaces;
  int show_slice_in_obst;
  int use_iblank;
  int iblank_set_on_commandline;
  int visOtherVents;
  int visOtherVentsSAVE;
  int update_terrain_type;
  int hvac_duct_color[3];
  int hvac_node_color[3];
  surfdata *surfacedefault, *vent_surfacedefault, *exterior_surfacedefault;
  tickdata *tickinfo;
  int nmemory_ids;
  char **colorlabelzone;
  int nrgb2;
  float pref, pamb, tamb;
  surfdata sdefault, v_surfacedefault, e_surfacedefault;
  int nrgb;
  float linewidth, ventlinewidth;
  float rgb[MAXRGB][4];
  char fuel_name[256];
  float fuel_hoc_default;

  int time_col;
  int hrr_col;
  int qradi_col;
  int mlr_col;
  int have_mlr;
  int chirad_col;

  hrrdata *hrrptr;
  hrrdata *timeptr;

  int have_animate_blockages;
  int have_removable_obsts;

  slicedata **sliceinfoptrs;
  int *subslice_menuindex;
  int *subvslice_menuindex;
  int *msubslice_menuindex;
  int *msubvslice_menuindex;

  int visFrame;
  int visFloor;
  int visWalls;
  int visCeiling;

  float getfilelist_time;
  float pass0_time;
  float pass1_time;
  float pass2_time;
  float pass3_time;
  float pass4_time;
  float pass5_time;
  float processing_time;
} smv_case;

typedef struct {
  int smoke3d_only;
  int setup_only;
  int fast_startup;
  int lookfor_compressed_files;
  int handle_slice_files;
} parse_options;

#endif
