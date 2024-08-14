#ifndef SHARED_STRUCTURES_H_DEFINED
#define SHARED_STRUCTURES_H_DEFINED
#include "options.h"
#include <stdio.h>
#include "isodefs.h"

#if defined(WIN32)
#include <windows.h>
#endif
#include GLU_H
#include GL_H

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

  struct _meshdata *skip_nabors[6];

#ifdef pp_BOUNDMEM
  unsigned char *buffer1;
#endif
  int *boundarytype;
  int *blockstart;

  struct _meshdata *nabors[6];
  struct _supermeshdata *super;
  int *ptype;
  unsigned int *zipoffset, *zipsize;

#ifdef pp_BOUNDMEM
  unsigned char *buffer2;
#endif
  float *xyzpatch, *xyzpatch_threshold;
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
  int npatch_times;
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

/* --------------------------  circdata ------------------------------------- */

typedef struct _circdata {
  float *xcirc, *ycirc;
  int ncirc;
} circdata;
#endif
