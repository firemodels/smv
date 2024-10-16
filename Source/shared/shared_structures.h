#ifndef SHARED_STRUCTURES_H_DEFINED
#define SHARED_STRUCTURES_H_DEFINED
#include "isodefs.h"
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

/**
 * @brief The graphical definition of an object. This represents a single state.
 * This can form a node in a linked-list.
 */
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

/**
 * @brief An object that can be rendered. This can form a node in a linked-list.
 */
typedef struct _sv_object {
  char label[256];
  /** @brief Is this object an avatar? */
  int type;
  int visible;
  int used, used_by_device;
  int use_displaylist;
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

#include "string_util.h"
#include "isodefs.h"
#include "contourdefs.h"
#include "histogram.h"

/* --------------------------  keyframe ------------------------------------ */

typedef struct _keyframe {
  int selected, npoints;
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

/* --------------------------  propdata ------------------------------------- */
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

/* --------------------------  texturedata ---------------------------------- */

typedef struct _texturedata {
  char *file;
  int loaded, display, used, is_transparent;
  GLuint name;
} texturedata;

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
  int blockvis;
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

  char *c_iblank_node,      *c_iblank_cell,      *c_iblank_x,      *c_iblank_y,      *c_iblank_z;
  char *c_iblank_node_temp, *c_iblank_cell_temp, *c_iblank_x_temp, *c_iblank_y_temp, *c_iblank_z_temp;
  float *f_iblank_cell;
  float *f_iblank_cell_temp;
  char *c_iblank_embed;
  float *block_zdist;

  int zdist_flag;
  unsigned char *iblank_smoke3d;
  int iblank_smoke3d_defined;
  struct _blockagedata **blockageinfoptrs;
  struct _blockagedata **bc_faces[6];
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

  struct _meshdata *skip_nabors[6];

  struct _meshdata *nabors[6];
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

/**
 * @brief A collection of labels. At it's core this collection
 * contains a linked list, but also an array of pointers into that linked list.
 *
 */
typedef struct {
  labeldata label_first;
  labeldata label_last;
  labeldata *label_first_ptr;
  labeldata *label_last_ptr;
} labels_collection;

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
#ifdef pp_SMOKE16
  char *s16_file;
#endif
  int filetype;
  int skip_smoke, skip_fire;
  int is_smoke, is_fire;
  int loaded, request_load, finalize, display, primary_file;
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
  unsigned char *alphas_dir[6];
  int fire_alpha, co2_alpha;
  float fire_alphas[256], co2_alphas[256];
  int *timeslist;
  int ntimes, ntimes_old, ismoke3d_time, lastiframe, ntimes_full;
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

typedef struct {
  int nsmoke3dinfo;
  smoke3ddata *smoke3dinfo;
  smoke3ddata **smoke3dinfo_sorted;
} smoke3d_collection;

#endif
