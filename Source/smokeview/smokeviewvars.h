#ifndef SMOKEVIEWVARS_H_DEFINED
#define SMOKEVIEWVARS_H_DEFINED
#include <time.h>
#include "dmalloc.h"
#ifdef CPP
#include "glui.h"
#endif
#include "datadefs.h"
#include "translate.h"
#include "csphere.h"
#include "smokeviewdefs.h"
#include "isobox.h"
#include "scontour2d.h"
#include "histogram.h"
#include "structures.h"
#include "readhvac.h"
#include "readobject.h"
#include "readtour.h"
#include "readlabel.h"
#ifndef CPP
#include <zlib.h>
#endif
#include "smokeheaders.h"
#include "threader.h"
#ifdef pp_OSX_HIGHRES
#include "glutbitmap.h"
#endif

#include GLUT_H
#include "readsmoke.h"

//*** threader variables

//***mergesmoke
SVEXTERN int SVDECL(n_mergesmoke_threads, 4), SVDECL(use_mergesmoke_threads, 1);
SVEXTERN threaderdata SVDECL(*mergesmoke_threads, NULL);
SVEXTERN smokethreaddata smokethreadinfo[MAX_THREADS];
SVEXTERN int SVDECL(n_mergesmoke_glui_threads, 4), SVDECL(use_mergesmoke_glui_threads, 1);
SVEXTERN int SVDECL(update_glui_merge_smoke, 1);

//***isosurface
SVEXTERN int SVDECL(n_isosurface_threads, 1), SVDECL(use_isosurface_threads, 1);
SVEXTERN threaderdata SVDECL(*isosurface_threads, NULL);

//***sliceparms
SVEXTERN int SVDECL(n_sliceparms_threads, 1), SVDECL(use_sliceparms_threads, 1);
SVEXTERN threaderdata SVDECL(*sliceparms_threads, NULL);

//***meshnabors
SVEXTERN int SVDECL(n_meshnabors_threads, 1), SVDECL(use_meshnabors_threads, 1);
SVEXTERN threaderdata SVDECL(*meshnabors_threads, NULL);
SVEXTERN int SVDECL(have_mesh_nabors, 0);

//***checkfiles
SVEXTERN int SVDECL(n_checkfiles_threads, 1), SVDECL(use_checkfiles_threads, 1);
SVEXTERN threaderdata SVDECL(*checkfiles_threads,       NULL);

//*** compress
SVEXTERN int SVDECL(n_compress_threads, 1), SVDECL(use_compress_threads, 1);
SVEXTERN threaderdata SVDECL(*compress_threads,        NULL);

//*** ffmpeg
SVEXTERN int SVDECL(n_ffmpeg_threads, 1), SVDECL(use_ffmpeg_threads, 1);
SVEXTERN threaderdata SVDECL(*ffmpeg_threads,         NULL);

//*** iso
SVEXTERN int SVDECL(n_iso_threads, 1), SVDECL(use_iso_threads, 0), SVDECL(use_iso_threads_save,0);

//*** part
SVEXTERN int SVDECL(n_partload_threads, 2);
#ifdef pp_PARTFRAME
SVEXTERN int SVDECL(use_partload_threads, 0);
#else
SVEXTERN int SVDECL(use_partload_threads, 1);
#endif
SVEXTERN threaderdata SVDECL(*partload_threads,         NULL);

//*** sorttags
SVEXTERN int SVDECL(n_sorttags_threads, 1), SVDECL(use_sorttags_threads, 1);
SVEXTERN threaderdata SVDECL(*sorttags_threads, NULL);
SVEXTERN int SVDECL(sorting_tags, 0);

//*** patchbounds
SVEXTERN int SVDECL(n_patchbound_threads, 1), SVDECL(use_patchbound_threads, 1);
SVEXTERN threaderdata SVDECL(*patchbound_threads, NULL);

//*** playmovie
SVEXTERN int SVDECL(n_playmovie_threads, 1), SVDECL(use_playmovie_threads, 1);
SVEXTERN threaderdata SVDECL(*playmovie_threads,       NULL);

//*** readallgeom
SVEXTERN int SVDECL(n_readallgeom_threads, 4), SVDECL(use_readallgeom_threads, 1);
SVEXTERN threaderdata SVDECL(*readallgeom_threads,     NULL);
SVEXTERN threaderdata SVDECL(*classifyallgeom_threads, NULL);

//***slice bounds
SVEXTERN int SVDECL(n_slicebound_threads, 1), SVDECL(use_slicebound_threads, 1);
SVEXTERN threaderdata SVDECL(*slicebound_threads, NULL);

//***part bounds
SVEXTERN int SVDECL(n_partbound_threads, 1), SVDECL(use_partbound_threads, 1);
SVEXTERN threaderdata SVDECL(*partbound_threads, NULL);

//***triangles
SVEXTERN int SVDECL(n_triangles_threads, 1), SVDECL(use_triangles_threads, 1);
SVEXTERN threaderdata SVDECL(*triangles_threads, NULL);

//*** volsmoke
SVEXTERN int SVDECL(n_volsmokeload_threads, 1), SVDECL(use_volsmokeload_threads, 0);
SVEXTERN threaderdata SVDECL(*volsmokeload_threads, NULL);

SVEXTERN keyworddata SVDECL(*keywordinfo, NULL);
SVEXTERN int SVDECL(nkeywordinfo, 0);

SVEXTERN int SVDECL(update_plot2dini, 0);
SVEXTERN int SVDECL(update_device_timeaverage, 0);
SVEXTERN int SVDECL(update_colorbar_list, 0);
SVEXTERN int SVDECL(force_bound_update, 0);
SVEXTERN int SVDECL(update_colorbar_dialog, 1);
#ifdef pp_FRAME
SVEXTERN int SVDECL(update_frame, 0);
#endif

// hvac data
SVEXTERN int SVDECL(hvac_show_connections, 0), SVDECL(hvac_show_networks, 1);
SVEXTERN int SVDECL(hvac_metro_view, 0), SVDECL(hvac_cell_view, 0);

SVEXTERN hvacdata SVDECL(*glui_hvac, NULL);
SVEXTERN int SVDECL(hvac_network_ductnode_index, -1);
#define HVAC_NCIRC 72
SVEXTERN float SVDECL(*hvac_circ_x, NULL), SVDECL(*hvac_circ_y, NULL);


//*** buffers for consolidating memory allocations
#define MAXFILELEN 360
SVEXTERN float SVDECL(*part_bound_buffer, NULL);

SVEXTERN FILE_SIZE SVDECL(last_size_for_slice, 0);
SVEXTERN FILE_SIZE SVDECL(last_size_for_boundary, 0);

SVEXTERN char SVDECL(*plot3d_gbnd_filename, NULL), SVDECL(**sorted_plot3d_filenames, NULL);
SVEXTERN char SVDECL(*slice_gbnd_filename,  NULL), SVDECL(**sorted_slice_filenames,  NULL);
SVEXTERN char SVDECL(*patch_gbnd_filename,  NULL), SVDECL(**sorted_patch_filenames,  NULL);

SVEXTERN globalboundsdata SVDECL(*plot3dglobalboundsinfo, NULL);
SVEXTERN globalboundsdata SVDECL(*sliceglobalboundsinfo,  NULL);
SVEXTERN globalboundsdata SVDECL(*patchglobalboundsinfo,  NULL);

SVEXTERN int SVDECL(nplot3dglobalboundsinfo,   0);
SVEXTERN int SVDECL(nsliceglobalboundsinfo,    0);
SVEXTERN int SVDECL(npatchglobalboundsinfo,    0);

SVEXTERN FILE_SIZE SVDECL(last_size_for_bound, 0);

SVEXTERN int SVDECL(histogram_nframes, 40);
SVEXTERN int SVDECL(glui_surf_index, 0);
SVEXTERN int SVDECL(clip_commandline, 0), SVDECL(special_modifier, 0);
SVEXTERN int SVDECL(update_slicexyz, 0);
SVEXTERN int SVDECL(update_splitcolorbar, 0);
SVEXTERN int SVDECL(slice_plot_bound_option, 1);
#ifdef INMAIN
SVEXTERN float geom_bounding_box[6] = {1000000000.0, -1000000000.0,
                                       1000000000.0, -1000000000.0,
                                       1000000000.0, -1000000000.0
                                      };
SVEXTERN int glui_surface_color[4] = {255, 255, 255, 255};
#else
SVEXTERN float geom_bounding_box[6];
SVEXTERN int glui_surface_color[4];
#endif

SVEXTERN int SVDECL(attachmenu_print, 0), SVDECL(attachmenu_status, 0), SVDECL(attachmenu_counter, 0);

SVEXTERN int SVDECL(slice_plot_csv, 0);
SVEXTERN char slice_plot_filename[256];
SVEXTERN char slice_plot_label[256];
SVEXTERN int SVDECL(update_plot_label, 0);

SVEXTERN int SVDECL(terrain_skip, 1);
SVEXTERN int nsmoke3dtypes, smoke3d_other;
SVEXTERN smoke3dtypedata SVDECL(*smoke3dtypes, NULL);

SVEXTERN int SOOT_index, HRRPUV_index, TEMP_index, CO2_index;

SVEXTERN int SVDECL(agl_offset_actual, 1);

SVEXTERN int SVDECL(texture_showall, 0);
SVEXTERN int SVDECL(texture_hideall, 0);
SVEXTERN int SVDECL(force_gray_smoke, 1);
SVEXTERN int SVDECL(verbose_output, 0);
SVEXTERN float glui_smoke3d_extinct;
SVEXTERN float glui_smoke3d_extinct_default;
SVEXTERN int SVDECL(force_alpha_opaque, 0);
SVEXTERN int SVDECL(max_colorbar_label_width, 0);
SVEXTERN char SVDECL(**colorbar_labels,  NULL);
SVEXTERN float SVDECL(*colorbar_vals,    NULL);
SVEXTERN int SVDECL(*colorbar_exponents, NULL);
SVEXTERN int SVDECL(frame360, 0);
SVEXTERN int SVDECL(sliceval_ndigits, 0);
SVEXTERN int SVDECL(rotate_center, 0);
SVEXTERN int SVDECL(have_geom_bb, 0);
SVEXTERN int SVDECL(show_geom_boundingbox, SHOW_BOUNDING_BOX_NEVER);
SVEXTERN int SVDECL(have_obsts, 0);
SVEXTERN int SVDECL(have_geometry_dialog, 0);
SVEXTERN int SVDECL(chop_patch, 0);
SVEXTERN float SVDECL(colorbar_slice_min, 0.0), SVDECL(colorbar_slice_max, 1.0);
SVEXTERN int SVDECL(have_geom_triangles, 0);
SVEXTERN int SVDECL(force_fixedpoint, FORCE_FIXEDPOINT_NO);
SVEXTERN int SVDECL(force_exponential, 0);
SVEXTERN int SVDECL(force_decimal, 0);
SVEXTERN int SVDECL(force_zero_pad, 0);
SVEXTERN int SVDECL(geom_cface_type, 1);
SVEXTERN int SVDECL(glui_use_cfaces, 1);
SVEXTERN int SVDECL(use_cfaces, 0);
SVEXTERN int SVDECL(update_reshape, 0);
SVEXTERN int SVDECL(last_time_paused, 0);
SVEXTERN float SVDECL(time_paused,0.0);
SVEXTERN int SVDECL(update_stept, -1);
SVEXTERN int SVDECL(update_viewpoint_script, 0);
SVEXTERN char viewpoint_script[256], SVDECL(*viewpoint_script_ptr, NULL);
SVEXTERN char viewpoint_label_saved[256];
SVEXTERN int SVDECL(update_saving_viewpoint, 0);
SVEXTERN float SVDECL(timer_startup, 0.0), SVDECL(timer_render, -1.0);
SVEXTERN int SVDECL(frames_total, 0 );
SVEXTERN int SVDECL(open_movie_dialog, 0);
SVEXTERN int SVDECL(geom_bounding_box_mousedown, 0);
SVEXTERN float SVDECL(plot2d_time_average, 0.0);
#ifdef pp_REFRESH
SVEXTERN int SVDECL(periodic_refresh, 0), SVDECL(update_refresh, 1);
SVEXTERN int SVDECL(glui_refresh_rate, 10), SVDECL(glui_refresh_rate_old, 10), SVDECL(refresh_interval, 100);
#endif
SVEXTERN int SVDECL(nslicemenuinfo, 0);

// movie batch variables
SVEXTERN int SVDECL(update_movie_parms, 0);
SVEXTERN int SVDECL(have_slurm, 0);
SVEXTERN int SVDECL(movie_slice_index, 0), SVDECL(movie_queue_index, 0), SVDECL(movie_nprocs, 10), SVDECL(nmovie_queues, 0);
SVEXTERN char SVDECL(**movie_queues, NULL), movie_htmldir[256], movie_email[256], movie_queue_list[256], movie_url[256];
SVEXTERN char movie_basename[256], movie_ssf_script[256], movie_bash_script[256], movie_ini_filename[256];
SVEXTERN slicedata SVDECL(*movie_sliceinfo, NULL);

SVEXTERN subslicemenudata SVDECL(*subslicemenuinfo, NULL), SVDECL(*subvectorslicemenuinfo, NULL);
SVEXTERN int SVDECL(nsubslicemenuinfo, 0), SVDECL(nsubvectorslicemenuinfo, 0);
SVEXTERN int SVDECL(nsubslicex, 0), SVDECL(nsubslicey, 0), SVDECL(nsubslicez, 0), SVDECL(nsubslicexyz, 0);
SVEXTERN int SVDECL(nsubvectorslicex, 0), SVDECL(nsubvectorslicey, 0), SVDECL(nsubvectorslicez, 0), SVDECL(nsubvectorslicexyz, 0);
SVEXTERN slicemenudata SVDECL(**slicemenu_sorted, NULL);
SVEXTERN int SVDECL(plot_option, 0);
SVEXTERN float hrr_valmin, hrr_valmax;
SVEXTERN int SVDECL(visFrameTimelabel, 1);
SVEXTERN int SVDECL(rotation_axis, 1);
SVEXTERN ztreedevicedata SVDECL(*ztreedeviceinfo, NULL);
SVEXTERN devicedata SVDECL(**deviceinfo_sortedz, NULL);
SVEXTERN int SVDECL(nztreedeviceinfo, 0);

SVEXTERN int SVDECL(readini_output, 0);
SVEXTERN int SVDECL(show_trirates, 0);

SVEXTERN float SVDECL(pixel_dens, 1.0);

#ifdef pp_OSX_HIGHRES
SVEXTERN int SVDECL(force_scale, 0);
extern CCC const BitmapFontRec glutBitmapHelvetica20;
extern CCC const BitmapFontRec glutBitmapHelvetica24;
extern CCC const BitmapFontRec glutBitmapHelvetica36;
#ifndef _GLUI_H_
extern int double_scale;
#endif
#endif

SVEXTERN int SVDECL(windowsize_pointer_old, -999);

SVEXTERN float SVDECL(xmin_draw, 1.0), SVDECL(xmax_draw, 0.0);
SVEXTERN float SVDECL(gmin_draw, 1.0), SVDECL(gmax_draw, 0.0);

#ifdef INMAIN
SVEXTERN char *cslice_label = "Slice",  *cpart_label = "Part", *cbound_label = "Bndry", *cplot3d_label = "Plot3D";
SVEXTERN char *dimensionless = "dimensionless";
#else
SVEXTERN char *cslice_label, *cpart_label, *cbound_label, *cplot3d_label;
SVEXTERN char *dimensionless;
#endif
SVEXTERN int SVDECL(colorbar_linewidth, 3);

SVEXTERN float SVDECL(plot2d_size_factor, 0.15), SVDECL(plot2d_font_spacing, 1.2);
SVEXTERN float SVDECL(plot2d_line_width, 1.0), SVDECL(plot2d_point_size, 5.0), SVDECL(plot2d_frame_width, 1.0);
#ifdef INMAIN
SVEXTERN float plot2d_xyz_offset[3] = {0.0, 0.0, 0.0};
#else
SVEXTERN float plot2d_xyz_offset[3];
#endif

SVEXTERN int SVDECL(update_glui_devices, 0);
SVEXTERN int SVDECL(update_patch_bounds, -1), SVDECL(update_slice_bounds, -1), SVDECL(update_part_bounds, -1), SVDECL(update_plot3d_bounds, -1);
SVEXTERN float max_dx, max_dy, max_dz;
SVEXTERN int SVDECL(vec_uniform_length, 0);
SVEXTERN int SVDECL(vec_uniform_spacing, 0);
#define NRENDER_SKIPS 8
#ifdef INMAIN
SVEXTERN int render_skips[NRENDER_SKIPS] = {RENDER_CURRENT_SINGLE, 1, 2, 3, 4, 5, 10, 20};
SVEXTERN char *crender_skips[NRENDER_SKIPS] =
{
  "Current", "All frames", "Every 2nd frame", "Every 3rd frame", "Every 4th frame",
  "Every 5th frame", "Every 10th frame", "Every 20th frame"
};
#else
SVEXTERN int render_skips[NRENDER_SKIPS];
SVEXTERN char *crender_skips[NRENDER_SKIPS];
#endif

SVEXTERN int SVDECL(bounds_each_mesh, 0);
SVEXTERN int SVDECL(show_bound_diffs, 0);

SVEXTERN slicemenudata SVDECL(*slicemenuinfo, NULL);
SVEXTERN int SVDECL(generate_info_from_commandline, 0);
SVEXTERN int SVDECL(vector_debug, 0);
SVEXTERN float SVDECL(*terrain_vertices, NULL), SVDECL(*terrain_tvertices, NULL), SVDECL(*terrain_colors, NULL);
SVEXTERN unsigned int SVDECL(*terrain_indices, NULL);
SVEXTERN int SVDECL(terrain_nindices, 0);
SVEXTERN int SVDECL(terrain_nfaces, 0);


SVEXTERN int SVDECL(terrain_show_geometry_surface, 1);
SVEXTERN int SVDECL(terrain_show_geometry_outline, 0);
SVEXTERN int SVDECL(terrain_show_geometry_points, 0);
SVEXTERN int SVDECL(terrain_showonly_top, 1), SVDECL(terrain_update_normals, 1);
SVEXTERN int SVDECL(showhide_textures, 0);

SVEXTERN int SVDECL(print_geominfo, 1);

SVEXTERN float SVDECL(slice_dz, 0.0);
SVEXTERN float SVDECL(terrain_normal_length, 1.0);
SVEXTERN int SVDECL(terrain_normal_skip, 1);
SVEXTERN int SVDECL(show_terrain_normals, 0), SVDECL(show_terrain_grid,0);
SVEXTERN int SVDECL(terrain_slice_overlap, 0);

SVEXTERN int SVDECL(use_slice_glui_bounds, 0);
SVEXTERN int SVDECL(vis_title_gversion,0);
SVEXTERN int SVDECL(vis_title_smv_version, 1);
SVEXTERN int SVDECL(vis_title_fds, 0);
SVEXTERN int SVDECL(vis_title_CHID,0);

#ifdef pp_REFRESH
SVEXTERN int SVDECL(refresh_glui_dialogs, 0);
#endif

SVEXTERN float SVDECL(colorbar_shift, 1.0);

SVEXTERN int SVDECL(compute_slice_file_sizes, 0);

SVEXTERN int SVDECL(update_colorbar_digits, 0);

SVEXTERN int SVDECL(ncolorlabel_digits, 4), SVDECL(ncolorlabel_padding, 0);
SVEXTERN int SVDECL(ngridloc_digits, 4);
SVEXTERN int SVDECL(ntick_decimals, 1);
SVEXTERN int SVDECL(mpi_nprocesses, -1), SVDECL(mpi_iprocess,-1);

#ifdef INMAIN
SVEXTERN unsigned int geom_vertex1_rgb[3] = {255, 255, 255};
SVEXTERN unsigned int geom_vertex2_rgb[3] = {0,   0,   0};
SVEXTERN unsigned int geom_triangle_rgb[3] = {255, 128,   0};
SVEXTERN unsigned int glui_surf_rgb[3]     = {128, 128, 128};
#else
SVEXTERN unsigned int geom_vertex1_rgb[3], geom_vertex2_rgb[3], geom_triangle_rgb[3];
SVEXTERN unsigned int glui_surf_rgb[3];
#endif

SVEXTERN int SVDECL(show_surf_axis, 0);
SVEXTERN float SVDECL(glui_surf_axis_length, 1.0);
SVEXTERN float SVDECL(glui_surf_axis_width, 1.0);
SVEXTERN float glui_surf_axis[3];
SVEXTERN int SVDECL(use_surf_color, 0);
SVEXTERN int SVDECL(geom_surf_index, 0);
SVEXTERN int SVDECL(select_geom, GEOM_PROP_NONE);
SVEXTERN int SVDECL(selected_geom_vertex1,  -1);
SVEXTERN int SVDECL(selected_geom_vertex2,  -1);
SVEXTERN int SVDECL(selected_geom_triangle, -1);

SVEXTERN int SVDECL(nlist_slice_index, 0);
SVEXTERN int SVDECL(slice_fileupdate, 0);
SVEXTERN int SVDECL(zone_temp_bounds_defined, 0);
SVEXTERN int SVDECL(slice_temp_bounds_defined, 0);

SVEXTERN int nplot3dloaded, nsmoke3dloaded, nisoloaded, nsliceloaded, nvsliceloaded, npartloaded, npatchloaded;
SVEXTERN int nvolsmoke3dloaded;
SVEXTERN int npart5loaded, npartloaded;
SVEXTERN int SVDECL(select_part, 0), SVDECL(selected_part_index, -1);
SVEXTERN int SVDECL(smoke3d_compression_type, COMPRESSED_UNKNOWN);
SVEXTERN int SVDECL(update_smoke3dmenulabels, 0);

SVEXTERN int SVDECL(global_have_global_bound_file, 0);
SVEXTERN FILE_SIZE  SVDECL(global_part_boundsize, 0);

SVEXTERN int SVDECL(glui_mesh_intersection_option, 0);

//---------------------------------------------------------------

SVEXTERN int SVDECL(partfast, 1);
SVEXTERN int SVDECL(have_vr, 0), SVDECL(use_vr,0);
SVEXTERN int SVDECL(use_fire_alpha, 0);
SVEXTERN int SVDECL(glui_use_fire_alpha, 1);
SVEXTERN int SVDECL(update_fire_alpha, 0);
SVEXTERN float SVDECL(emission_factor, 3.0);
#ifdef pp_OSX
SVEXTERN int SVDECL(monitor_screen_height, -1);
#endif
SVEXTERN int SVDECL(rollout_count, 0);
SVEXTERN int SVDECL(smoke3d_start_frame, 0);
SVEXTERN int SVDECL(smoke3d_skip_frame, 1);
SVEXTERN int SVDECL(smoke3d_use_skip, 0);

SVEXTERN int SVDECL(glui_defined, 0);
SVEXTERN int SVDECL(update_times,0);
SVEXTERN int SVDECL(show_geom_bndf, 0),SVDECL(glui_show_geom_bndf, 0);
SVEXTERN int SVDECL(update_windrose, 0);
SVEXTERN int SVDECL(update_use_lighting, 0), SVDECL(use_lighting, 1);

SVEXTERN cellmeshdata SVDECL(*cellmeshinfo, NULL);
SVEXTERN int SVDECL(is_convex, 0);

SVEXTERN int SVDECL(cancel_update_triangles, 0);
SVEXTERN int SVDECL(updating_triangles, 0);
SVEXTERN int SVDECL(lighting_on,0);
SVEXTERN int SVDECL(update_texturebar, 0);
SVEXTERN float SVDECL(iso_valmin, 20.0), SVDECL(iso_valmax, 1020.0);
SVEXTERN float SVDECL(glui_iso_valmin, 20.0), SVDECL(glui_iso_valmax, 1020.0);
SVEXTERN float SVDECL(iso_global_min,0.0), SVDECL(iso_global_max,1.0);
SVEXTERN colorbardata SVDECL(*iso_colorbar, NULL);
SVEXTERN int SVDECL(show_iso_color, 1);
SVEXTERN int SVDECL(update_iso_ini, 0);

SVEXTERN int SVDECL(script_defer_loading, 0);

SVEXTERN int SVDECL(use_light0, 1), SVDECL(use_light1, 1);
SVEXTERN int SVDECL(iso_transparency_option, 1);
SVEXTERN int SVDECL(iso_opacity_change, 1);

#ifdef pp_RENDER360_DEBUG
SVEXTERN int SVDECL(debug_360, 0), SVDECL(debug_360_skip_x,25), SVDECL(debug_360_skip_y,25);
#endif
SVEXTERN int SVDECL(output_ffmpeg_command, 0);
SVEXTERN int SVDECL(margin360_size, 0);

SVEXTERN float SVDECL(update_rgb_test,0), SVDECL(rgb_test_delta,10), rgb_test_xyz[3];
SVEXTERN int   rgb_test_rgb[3];

SVEXTERN float SVDECL(customview_azimuth, 0.0), SVDECL(customview_elevation, 0.0), SVDECL(customview_up, 90.0);
SVEXTERN int SVDECL(use_customview, 0);
SVEXTERN screendata SVDECL(*screenglobal, NULL);

SVEXTERN int SVDECL(smoke_outline_type, SMOKE_OUTLINE_TRIANGLE);
SVEXTERN int SVDECL(update_filesizes, 0);
SVEXTERN int SVDECL(compute_smoke3d_planes_par, 0);
SVEXTERN float SVDECL(smoke3d_delta_perp,0.5);
SVEXTERN int SVDECL(smoke3d_black, 0);
SVEXTERN int SVDECL(smoke3d_skip, 1),  SVDECL(smoke3d_skipxy, 1);
SVEXTERN int SVDECL(smoke3d_skipx, 1), SVDECL(smoke3d_skipy, 1), SVDECL(smoke3d_skipz, 1), SVDECL(smoke3d_kmax, -1);
SVEXTERN int SVDECL(slice_skip, 1), SVDECL(slice_skipx, 1), SVDECL(slice_skipy, 1), SVDECL(slice_skipz, 1), SVDECL(max_slice_skip,-1);

SVEXTERN int SVDECL(update_research_mode, 1);
SVEXTERN int SVDECL(research_mode, 1);
SVEXTERN int SVDECL(research_mode_override, -1);

SVEXTERN float SVDECL(geomboundary_pointsize, 5.0);
SVEXTERN float SVDECL(geomboundary_linewidth, 5.0);

SVEXTERN int SVDECL(update_tour_path,1);
SVEXTERN int SVDECL(tour_circular_index, -1);
SVEXTERN float tour_circular_center[3], tour_circular_radius, tour_circular_view[3], SVDECL(tour_circular_angle0,0.0);
SVEXTERN float tour_circular_center_default[3], tour_circular_radius_default, tour_circular_view_default[3];
SVEXTERN float SVDECL(tour_speedup_factor, 1.0);
SVEXTERN int SVDECL(ncircletournodes, 16);

SVEXTERN int SVDECL(render_resolution, RENDER_RESOLUTION_CURRENT);
SVEXTERN int SVDECL(timebar_overlap, TIMEBAR_OVERLAP_AUTO);
SVEXTERN int SVDECL(vis_colorbar, COLORBAR_HIDDEN);
#define N_COLORBARS 8
SVEXTERN int hcolorbar_vis[N_COLORBARS];

SVEXTERN int SVDECL(windrose_ttype, 2);
#ifdef INMAIN
  SVEXTERN float windrose_merge_dxyzt[6] = { 0.0,0.0,0.0,0.0,0.0,0.0 };
#else
  SVEXTERN float windrose_merge_dxyzt[6];
#endif

SVEXTERN int SVDECL(drawing_boundary_files, 0);
SVEXTERN int SVDECL(lock_mouse_aperture, 0);
SVEXTERN int SVDECL(windrose_merge_type,WINDROSE_POINT);
SVEXTERN int SVDECL(windrose_first, 0), SVDECL(windrose_next, 1);
SVEXTERN int SVDECL(windrose_xy_active, 0), SVDECL(windrose_xz_active, 0), SVDECL(windrose_yz_active, 0);

SVEXTERN int SVDECL(showgeom_inside_domain, 1);
SVEXTERN int SVDECL(showgeom_outside_domain, 1);
SVEXTERN int nsootloaded, nhrrpuvloaded, ntemploaded, nco2loaded;
SVEXTERN int nsootfiles, nhrrpuvfiles, ntempfiles, nco2files;
SVEXTERN int SVDECL(have_fire, NO_FIRE), SVDECL(have_smoke, NO_SMOKE);
SVEXTERN int SVDECL(nsmoke3d_temp, 0);
SVEXTERN int SVDECL(nsmoke3d_co2, 0);
SVEXTERN int SVDECL(update_zaxis_custom, 0);
SVEXTERN int SVDECL(from_DisplayCB, 0);
SVEXTERN float SVDECL(*fire_rgbs, NULL);
SVEXTERN int SVDECL(nfire_colors, 1024);
SVEXTERN float SVDECL(fire_temp_min, 100.0), SVDECL(fire_temp_max, 5500.0);
SVEXTERN float SVDECL(fire_temp_data_min, 1.0), SVDECL(fire_temp_data_max, 0.0);
SVEXTERN int SVDECL(show_blackbody_colormap, 0);
SVEXTERN int SVDECL(use_blackbody_colors, 0);
SVEXTERN int SVDECL(have_geom_slice_menus, 0), SVDECL(geom_slice_loaded,0);
SVEXTERN FILE SVDECL(*stderr2,NULL);
SVEXTERN char SVDECL(*script_error1_filename,NULL);
SVEXTERN int SVDECL(render_firsttime, NO);
SVEXTERN int SVDECL(load_incremental, 0);
SVEXTERN int SVDECL(show_tour_hint, 1);
SVEXTERN int cb_rgb[3];
SVEXTERN int cb_simple_rgb[15];
SVEXTERN int SVDECL(colorbar_simple_type, 0);
SVEXTERN float SVDECL(geom_linewidth, 2.0);
SVEXTERN float SVDECL(geom_pointsize, 6.0);
SVEXTERN int SVDECL(slice_opacity_adjustment, 0);
SVEXTERN int SVDECL(showall_3dslices, 0);
SVEXTERN int SVDECL(slices3d_max_blending, 0);
SVEXTERN int SVDECL(hrrpuv_max_blending, 0);
SVEXTERN int SVDECL(nplotx_list,0), SVDECL(nploty_list,0), SVDECL(nplotz_list,0);
SVEXTERN int SVDECL(*plotx_list, NULL);
SVEXTERN int SVDECL(*ploty_list, NULL);
SVEXTERN int SVDECL(*plotz_list, NULL);
SVEXTERN int SVDECL(colorbar_autonode, 1);
SVEXTERN float SVDECL(*blackbody_colors, NULL);
SVEXTERN int SVDECL(show_volsmokefiles, 1), SVDECL(show_3dsmoke,1);
SVEXTERN float SVDECL(voltest_temp1, 20.0), SVDECL(voltest_temp2, 620.0);
SVEXTERN float SVDECL(voltest_soot1, 0.5), SVDECL(voltest_soot2, 1.2);
SVEXTERN float SVDECL(voltest_depth1, 0.3), SVDECL(voltest_depth2, 0.6);
SVEXTERN float SVDECL(voltest_r1, 0.4), SVDECL(voltest_r2, 0.6), SVDECL(voltest_r3, 0.2);
#ifdef INMAIN
SVEXTERN float voltest_center[3] = {0.8, 0.8, 1.0};
#else
SVEXTERN float voltest_center[3];
#endif
SVEXTERN int SVDECL(show_plot3dfiles, 1), SVDECL(show_isofiles,1);
SVEXTERN int SVDECL(show_boundaryfiles, 1);

SVEXTERN int SVDECL(smoke_framenumber, 0);
SVEXTERN float SVDECL(time_frameval, 0.0), SVDECL(time_framemax,0.0), SVDECL(time_framemin,1.0);

SVEXTERN int SVDECL(clip_rotate, 0);
SVEXTERN char SVDECL(*file_smokesensors, NULL);
SVEXTERN int SVDECL(light_faces, 1);
SVEXTERN int SVDECL(nwindrosez_checkboxes, 0);
SVEXTERN float startup_time;
#ifdef pp_FRAME
SVEXTERN int SVDECL(nframe_threads, 4), SVDECL(read_buffer_size, 10);
#endif
SVEXTERN int SVDECL(alt_ctrl_key_state, KEY_NONE);
SVEXTERN devicedata SVDECL(**vel_devices, NULL);
SVEXTERN int SVDECL(nvel_devices, 0);

SVEXTERN int SVDECL(update_slice, 0);
SVEXTERN int SVDECL(slice_bounds_defined, 0), SVDECL(patch_bounds_defined, 0);
SVEXTERN int SVDECL(*windrosez_showhide, NULL), SVDECL(nwindrosez_showhide,0), SVDECL(update_windrose_showhide,0);
#ifdef INMAIN
  SVEXTERN float smoke_test_color[4] = {0.0,0.0,0.0,1.0};
  SVEXTERN float smoke_test_target_color[4] = {1.0,0.0,0.0,1.0};
#else
SVEXTERN float smoke_test_color[4];
SVEXTERN float smoke_test_target_color[4];
#endif
SVEXTERN float SVDECL(smoke_test_range,1.0), SVDECL(smoke_test_opacity,0.5);
SVEXTERN int SVDECL(smoke_test_nslices,3);

SVEXTERN float box_corners[8][3], box_geom_corners[8][3];
SVEXTERN int SVDECL(have_box_geom_corners, 0);
SVEXTERN float boxmin_global[3], boxmax_global[3], max_cell_length;
SVEXTERN int SVDECL(update_boxbounds, 1);
SVEXTERN int SVDECL(showbeam_as_line, 1), SVDECL(use_beamcolor,0), beam_color[3];
SVEXTERN float SVDECL(beam_line_width, 4.0);

SVEXTERN float SVDECL(zone_hvac_diam, 0.05);
SVEXTERN int SVDECL(timearray_test, 0);
SVEXTERN int SVDECL(slice_time, 0);
SVEXTERN int SVDECL(in_part_mt, 0);
SVEXTERN histogramdata SVDECL(*full_plot3D_histograms, NULL);
SVEXTERN histogramdata SVDECL(*full_part_histogram, NULL);
SVEXTERN int SVDECL(update_loadall_textures, 1);

SVEXTERN int SVDECL(max_LISTBOX_cb_bound, 0);
SVEXTERN int SVDECL(max_LISTBOX_cb_edit, 0);
SVEXTERN int SVDECL(*colorbar_list_sorted, NULL);
SVEXTERN int SVDECL(*colorbar_list_inverse, NULL);
SVEXTERN int SVDECL(show_Lab_dist_bars, 0);
SVEXTERN int SVDECL(update_colorbar_orig, 0);
SVEXTERN float SVDECL(*lab_check_xyz, NULL);
SVEXTERN float cb_lab2[3], cb_frgb2[3];
SVEXTERN int cb_rgb2[3];
SVEXTERN int SVDECL(index_colorbar1, 0), SVDECL(index_colorbar2, 1);
SVEXTERN int SVDECL(colorbar_toggle, 1);
SVEXTERN int SVDECL(color_vector_black, 0);
SVEXTERN float SVDECL(geom_transparency, 0.5);
SVEXTERN int SVDECL(geom_force_transparent, 0);
SVEXTERN int SVDECL(override_3dsmoke_cutoff, 0);
SVEXTERN int SVDECL(visCompartments, 1);
SVEXTERN int render_mode, render_times;
SVEXTERN int SVDECL(render_status, 0);
SVEXTERN int SVDECL(resolution_multiplier, 1);
SVEXTERN int SVDECL(glui_resolution_multiplier, 2);
SVEXTERN char render_file_base[1024];
SVEXTERN char html_file_base[1024];
SVEXTERN int SVDECL(script_render_width, 320), SVDECL(script_render_height, 240);
SVEXTERN int SVDECL(render_clip_left, 0);
SVEXTERN int SVDECL(render_clip_right, 0);
SVEXTERN int SVDECL(render_clip_bottom, 0);
SVEXTERN int SVDECL(render_clip_top, 0);
SVEXTERN int render_size_index;
SVEXTERN int SVDECL(renderW, 640), SVDECL(renderH, 480), render_window_size;
SVEXTERN int render_filetype;
SVEXTERN int SVDECL(render_label_type, RENDER_LABEL_FRAMENUM);

SVEXTERN int SVDECL(movie_crf, 17);
SVEXTERN int SVDECL(movie_bitrate, 5000);
SVEXTERN int SVDECL(disable_reshape, 0);

SVEXTERN int SVDECL(nscreeninfo,26);
#ifdef pp_RENDER360_DEBUG
SVEXTERN int SVDECL(screenview, 0);
SVEXTERN int SVDECL(*screenvis,NULL);
#endif
SVEXTERN int SVDECL(update_screeninfo, 0);
SVEXTERN screendata SVDECL(*screeninfo,NULL);
SVEXTERN int SVDECL(nwidth360,1024), SVDECL(nheight360,512);
SVEXTERN unsigned int SVDECL(*screenmap360, NULL);
SVEXTERN float SVDECL(*screenmap360IX, NULL), SVDECL(*screenmap360IY, NULL);

SVEXTERN colorbardata SVDECL(*split_colorbar, NULL);
#ifdef INMAIN
SVEXTERN float splitvals[3]={-1.0,0.0,1.0};
#else
SVEXTERN float splitvals[3];
#endif
#ifdef INMAIN
SVEXTERN int colorsplit[12] = {0,0,0,  64,64,255,  0,192,0,  255,0,0};
#else
SVEXTERN int colorsplit[12];
#endif

SVEXTERN int SVDECL(load_smoke_density, 1);
SVEXTERN int SVDECL(have_smoke_density, 0);
SVEXTERN int SVDECL(show_zlevel, 0);
SVEXTERN float terrain_zlevel;
SVEXTERN float terrain_zmin, terrain_zmax;
SVEXTERN int SVDECL(show_texture_1dimage, 0);
SVEXTERN int SVDECL(force_UpdateHistograms, 1);
SVEXTERN float SVDECL(geom_vert_exag, 1.0);
SVEXTERN float SVDECL(geom_vecfactor, .030);
SVEXTERN int SVDECL(geom_ivecfactor, 30);
SVEXTERN float SVDECL(geom_norm_offset,0.0);
SVEXTERN float SVDECL(geom_dz_offset,0.0);
SVEXTERN int SVDECL(iso_outline_ioffset, 1);
SVEXTERN float SVDECL(iso_outline_offset, 0.001);
SVEXTERN int SVDECL(update_setvents, 0);
SVEXTERN int SVDECL(update_setcvents, 0);
SVEXTERN int SVDECL(update_ff, 0);
SVEXTERN int SVDECL(use_set_slicecolor, 1);
SVEXTERN int SVDECL(cvents_defined, 0);
SVEXTERN int SVDECL(boundaryslicedup_option, SLICEDUP_KEEPFINE);
SVEXTERN int SVDECL(slicedup_option, SLICEDUP_KEEPFINE);
SVEXTERN int SVDECL(vectorslicedup_option, SLICEDUP_KEEPALL);
SVEXTERN int SVDECL(nslicedups, 0);
SVEXTERN int SVDECL(nboundaryslicedups, 0);
SVEXTERN int SVDECL(vis_xtree, 0), SVDECL(vis_ytree, 0), SVDECL(vis_ztree, 1);
SVEXTERN int SVDECL(max_device_tree,0);
#ifdef INMAIN
SVEXTERN float northangle_position[3] = {0.0, 0.0, 0.1};
#else
SVEXTERN float northangle_position[3];
#endif
SVEXTERN int SVDECL(vis_northangle, 0);

SVEXTERN int SVDECL(viswindrose, 0), SVDECL(windrose_xy_vis, 1), SVDECL(windrose_xz_vis, 0), SVDECL(windrose_yz_vis, 0);
SVEXTERN int SVDECL(nr_windrose, 8), SVDECL(ntheta_windrose, 12);
SVEXTERN float SVDECL(radius_windrose, 1.0),SVDECL(maxr_windrose,0.0);
SVEXTERN int SVDECL(scale_increment_windrose, 5), SVDECL(scale_max_windrose, 25);
SVEXTERN int    SVDECL(showref_windrose,1), SVDECL(scale_windrose,WINDROSE_LOCALSCALE);
SVEXTERN int SVDECL(showlabels_windrose,1), SVDECL(windstate_windrose,WINDROSE_DIRECTION);

SVEXTERN int SVDECL(zone_rho, 1);
SVEXTERN int SVDECL(visventslab, 0), SVDECL(visventprofile, 1);
SVEXTERN int SVDECL(update_readiso_geom_wrapup, UPDATE_ISO_OFF);
SVEXTERN int SVDECL(update_playmovie, 0);
SVEXTERN int SVDECL(play_movie_now, 1);
SVEXTERN int SVDECL(update_makemovie, 0),SVDECL(movie_filetype,AVI);
SVEXTERN char movie_name[1024], movie_ext[10];
SVEXTERN int SVDECL(movie_framerate, 10), SVDECL(have_ffmpeg, 0), SVDECL(have_ffplay, 0), SVDECL(overwrite_movie, 1);

SVEXTERN int SVDECL(show_missing_objects, 1);
SVEXTERN int SVDECL(toggle_dialogs, 1);
SVEXTERN int SVDECL(use_data_extremes, 1);
SVEXTERN int SVDECL(extreme_data_offset, 1), SVDECL(colorbar_offset, 0);
SVEXTERN int SVDECL(colorbar_flip, 0);
SVEXTERN int SVDECL(colorbar_autoflip,1);
SVEXTERN int SVDECL(update_flipped_colorbar,0);

#ifdef INMAIN
SVEXTERN float gvecphys_orig[3] = {0.0,0.0,-9.8};
#else
SVEXTERN float gvecphys_orig[3];
#endif
SVEXTERN int SVDECL(gvec_down,1),SVDECL(zaxis_custom,0),SVDECL(showgravity_vector,0);
SVEXTERN float SVDECL(slice_line_contour_width,1.0);
SVEXTERN int SVDECL(slice_contour_type,0);
SVEXTERN int SVDECL(viscadopaque,0);
SVEXTERN int SVDECL(structured_isopen,0), SVDECL(unstructured_isopen,0);
SVEXTERN float SVDECL(patchout_tmin,1.0), SVDECL(patchout_tmax,-1.0);
SVEXTERN float SVDECL(patchout_xmin,1.0), SVDECL(patchout_xmax,-1.0);
SVEXTERN float SVDECL(patchout_ymin,1.0), SVDECL(patchout_ymax,-1.0);
SVEXTERN float SVDECL(patchout_zmin,1.0), SVDECL(patchout_zmax,-1.0);
SVEXTERN int SVDECL(showpatch_both,0);
SVEXTERN int SVDECL(show_all_exterior_patch_data, 0);
SVEXTERN int SVDECL(hide_all_exterior_patch_data, 0);
SVEXTERN int SVDECL(show_all_interior_patch_data, 0);
SVEXTERN int SVDECL(hide_all_interior_patch_data, 1);
SVEXTERN int SVDECL(update_boundary_loaded, 0);
SVEXTERN int SVDECL(show_triangle_count,0);
SVEXTERN int SVDECL(triangle_count ,0);
SVEXTERN int SVDECL(n_geom_triangles,0);
SVEXTERN int SVDECL(show_device_orientation,0);
SVEXTERN float SVDECL(orientation_scale,1.0);
SVEXTERN int SVDECL(clipon,0);
SVEXTERN int SVDECL(vectortype,0);
SVEXTERN int SVDECL(show_cutcells,0);
SVEXTERN int b_state[7],SVDECL(*box_state,b_state+1);
SVEXTERN int face_id[200],face_vis[10], face_vis_old[10];
SVEXTERN int SVDECL(update_volbox_controls,0);
SVEXTERN float SVDECL(face_factor,0.01);
SVEXTERN int SVDECL(show_node_slices_and_vectors,0);
SVEXTERN int SVDECL(show_cell_slices_and_vectors,1);
SVEXTERN int SVDECL(update_patch_vis, 0);

SVEXTERN int SVDECL(convert_ini,0), SVDECL(convert_ssf,0);
SVEXTERN int SVDECL(update_ssf,0);
SVEXTERN char SVDECL(*ini_from,NULL), SVDECL(*ini_to,NULL);
SVEXTERN char SVDECL(*ssf_from, NULL), SVDECL(*ssf_to, NULL);

SVEXTERN int SVDECL(tour_drag,0);

SVEXTERN int SVDECL(update_gslice,0);
SVEXTERN int SVDECL(wall_cell_color_flag,0);
SVEXTERN circdata cvent_circ, object_circ, windrose_circ;
SVEXTERN int SVDECL(show_all_units,0);
SVEXTERN int SVDECL(circle_outline,0);
SVEXTERN unsigned char SVDECL(*patchmin_unit,NULL),SVDECL(*patchmax_unit,NULL);
SVEXTERN unsigned char SVDECL(*glui_slicemin_unit,NULL),SVDECL(*glui_slicemax_unit,NULL);
SVEXTERN unsigned char SVDECL(*plot3dmin_unit,NULL),SVDECL(*plot3dmax_unit,NULL);
SVEXTERN unsigned char SVDECL(*partmin_unit,NULL),SVDECL(*partmax_unit,NULL);
SVEXTERN unsigned char degC[3], degF[3];
SVEXTERN float SVDECL(tmax_part,16.0);
SVEXTERN int SVDECL(redirect,0);
SVEXTERN int SVDECL(tempdir_flag,0),SVDECL(time_flag,0);
SVEXTERN int SVDECL(script_render,0);
SVEXTERN int SVDECL(make_volrender_script,0);
SVEXTERN char SVDECL(*volrender_scriptname,NULL);
SVEXTERN float SVDECL(nongpu_vol_factor,1.0);
SVEXTERN float SVDECL(gpu_vol_factor,1.0);
SVEXTERN int SVDECL(disable_gpu,0);
SVEXTERN int SVDECL(check_colorbar, 0);
SVEXTERN int SVDECL(script_startframe,-1), SVDECL(script_skipframe,-1);
SVEXTERN int SVDECL(vol_startframe0,-1), SVDECL(vol_skipframe0,-1);
SVEXTERN int SVDECL(render_startframe0,-1), SVDECL(render_skipframe0,-1);
SVEXTERN int SVDECL(update_smokecolorbar,0);
SVEXTERN int SVDECL(combine_meshes,1);
SVEXTERN int vcolorbar_left_pos, vcolorbar_right_pos, vcolorbar_down_pos, vcolorbar_top_pos;
SVEXTERN int hcolorbar_left_pos, hcolorbar_right_pos, hcolorbar_down_pos, hcolorbar_top_pos;
SVEXTERN float scale_2d_x, scale_2d_y;
SVEXTERN int SVDECL(vcolorbar_delta,35);
SVEXTERN int SVDECL(hcolorbar_delta, 25);
SVEXTERN int colorbar_label_width;

SVEXTERN int glui_up_rgb[3], glui_down_rgb[3];

SVEXTERN int timebar_left_width, timebar_right_width;
SVEXTERN int SVDECL(h_space,2), SVDECL(v_space,2);
SVEXTERN portdata VP_fullscreen, VP_title, VP_timebar, VP_vcolorbar, VP_scene, VP_info;
SVEXTERN portdata VP_hrr_plot, VP_slice_plot;

SVEXTERN int SVDECL(in_external,0);
SVEXTERN int SVDECL(label_list_index,0);
SVEXTERN labeldata LABEL_local, SVDECL(*LABEL_global_ptr,NULL), LABEL_default;

SVEXTERN int port_pixel_width, port_pixel_height;
SVEXTERN float port_unit_width, port_unit_height;

SVEXTERN float quat_general[4], quat_rotation[16];

SVEXTERN float modelview_identity[16];
SVEXTERN mousedata mouseinfo;
SVEXTERN int SVDECL(use_glui_rotate,0);

SVEXTERN int SVDECL(colorbar_coord_type, 0);

SVEXTERN int SVDECL(*meshvisptr,NULL);
SVEXTERN int SVDECL(from_commandline,0);
SVEXTERN float this_mouse_time, SVDECL(last_mouse_time,0.0);
SVEXTERN int move_gslice;

SVEXTERN int SVDECL(visGeomTextures,0);
SVEXTERN int SVDECL(visGeomTextures_last, -1), SVDECL(have_non_textures,1);
SVEXTERN int nplotx_all, nploty_all, nplotz_all;
SVEXTERN int iplotx_all, iploty_all, iplotz_all;
SVEXTERN int SVDECL(iplot_state,0);
SVEXTERN int SVDECL(visx_all,0),SVDECL(visy_all,1),SVDECL(visz_all,0);
SVEXTERN float SVDECL(*plotx_all,NULL), SVDECL(*ploty_all,NULL), SVDECL(*plotz_all,NULL);
SVEXTERN int SVDECL(debug_count,0);
SVEXTERN geomdata SVDECL(**geominfoptrs,NULL);
SVEXTERN int SVDECL(ngeominfoptrs,0);

SVEXTERN char startup_lang_code[3];

#ifdef pp_GPU
  SVEXTERN float SVDECL(thisGPUtime,0.0), SVDECL(lastGPUtime,0.0);
  SVEXTERN float SVDECL(thisMOTIONtime,0.0), SVDECL(lastMOTIONtime,0.0);
  SVEXTERN int SVDECL(GPUnframes,0),SVDECL(MOTIONnframes,0);
#endif
SVEXTERN int SVDECL(mouse_down,0);
SVEXTERN int SVDECL(show_volsmoke_moving,1);
SVEXTERN int SVDECL(freeze_volsmoke,0);
SVEXTERN int SVDECL(autofreeze_volsmoke, 0);

SVEXTERN int SVDECL(glui_show_vector_slice, 1);
SVEXTERN int SVDECL(glui_show_slice_shaded,1);
SVEXTERN int SVDECL(glui_show_slice_outlines,0);
SVEXTERN int SVDECL(glui_show_slice_points,0);
SVEXTERN int SVDECL(glui_show_slice_values, 0);

#ifdef INMAIN
SVEXTERN int show_slice_shaded[MAX_CELL_TYPES]   = {1,0,1};
SVEXTERN int show_slice_outlines[MAX_CELL_TYPES] = {0,0,0};
SVEXTERN int show_slice_points[MAX_CELL_TYPES]   = {0,0,0};
SVEXTERN int show_slice_values[MAX_CELL_TYPES]   = {0,0,0};
SVEXTERN int show_vector_slice[MAX_CELL_TYPES]   = {1,0,1};
#else
SVEXTERN int show_slice_shaded[MAX_CELL_TYPES];
SVEXTERN int show_slice_outlines[MAX_CELL_TYPES];
SVEXTERN int show_slice_points[MAX_CELL_TYPES];
SVEXTERN int show_slice_values[MAX_CELL_TYPES];
SVEXTERN int show_vector_slice[MAX_CELL_TYPES];
#endif

SVEXTERN int SVDECL(show_boundary_shaded, 1);
SVEXTERN int SVDECL(show_boundary_outline, 0);
SVEXTERN int SVDECL(show_boundary_points, 0);

SVEXTERN int SVDECL(show_iso_shaded,1);
SVEXTERN int SVDECL(show_iso_outline,1);
SVEXTERN int SVDECL(show_iso_points,0);

SVEXTERN int SVDECL(show_faces_shaded, 1);
SVEXTERN int SVDECL(show_faces_outline, 0);
SVEXTERN int SVDECL(show_geom_verts, 0);
SVEXTERN int SVDECL(show_cface_normals, 0);

SVEXTERN int SVDECL(show_iso_normal, 0), SVDECL(smooth_iso_normal, 1);

SVEXTERN int SVDECL(boundary_edgetype, OUTLINE_HIDDEN);

SVEXTERN int SVDECL(glui_slice_edgetype, OUTLINE_HIDDEN);
SVEXTERN int SVDECL(slice_celltype, 0);
#ifdef INMAIN
  SVEXTERN int slice_edgetypes[3] = {OUTLINE_HIDDEN, OUTLINE_HIDDEN, OUTLINE_HIDDEN};
#else
  SVEXTERN int slice_edgetypes[3];
#endif


SVEXTERN int SVDECL(show_geom_normal, 0), SVDECL(smooth_geom_normal, 1);

SVEXTERN geomlistdata SVDECL(*geomlistinfo, NULL);
SVEXTERN int SVDECL(glui_load_volcompressed,0),SVDECL(load_volcompressed,0);
SVEXTERN int SVDECL(use_multi_threading,1);
SVEXTERN int SVDECL(load_at_rendertimes,1);
SVEXTERN int nvolrenderinfo;
SVEXTERN int SVDECL(compress_volsmoke,0),SVDECL(glui_compress_volsmoke,0);
SVEXTERN int SVDECL(read_vol_mesh,VOL_READNONE);
SVEXTERN int SVDECL(trainer_temp_index,0),SVDECL(trainer_oxy_index,0);
SVEXTERN int SVDECL(*trainer_temp_indexes,NULL),SVDECL(*trainer_oxy_indexes,NULL);
SVEXTERN int SVDECL(trainer_showall_mslice,0),SVDECL(trainer_cycle_mslice,1);
SVEXTERN int SVDECL(trainer_temp_n,0),SVDECL(trainer_oxy_n,0);
SVEXTERN char SVDECL(*tr_name,NULL);
SVEXTERN int SVDECL(showdevice_val, 0), SVDECL(showvdevice_val, 0);
SVEXTERN int SVDECL(show_plot2d_xlabels, 1), SVDECL(show_plot2d_ylabels, 1), SVDECL(show_plot2d_title, 1);
SVEXTERN int SVDECL(average_plot2d_slice_region, 0), SVDECL(show_plot2d_slice_position, 1);;
SVEXTERN int SVDECL(colordevice_val, 0), SVDECL(showdevice_id, 0);
SVEXTERN int SVDECL(select_device, 0);
SVEXTERN char plot2d_xaxis_label[301];
SVEXTERN float SVDECL(plot2d_xaxis_position, 0.0);
SVEXTERN int SVDECL(showdevice_type,1), SVDECL(showdevice_unit,1);
SVEXTERN float SVDECL(device_valmin,0.0), SVDECL(device_valmax,1.0);
SVEXTERN devicedata SVDECL(**devicetypes,NULL);
SVEXTERN int SVDECL(ndevicetypes,0);

// gen plot variables
SVEXTERN int SVDECL(idevice_add, 0);
SVEXTERN float plot2d_xyzstart[3], plot2d_xyzend[3];
SVEXTERN int SVDECL(plot2d_show_plot_title,   1);
SVEXTERN int SVDECL(plot2d_show_yaxis_bounds, 1);
SVEXTERN int SVDECL(plot2d_show_yaxis_units,  1);
SVEXTERN int SVDECL(plot2d_show_xaxis_labels, 0);
SVEXTERN int SVDECL(plot2d_show_xaxis_bounds, 1);
SVEXTERN int SVDECL(plot2d_show_curve_labels, 1);
SVEXTERN int SVDECL(plot2d_show_curve_values, 0);
SVEXTERN int SVDECL(plot2d_show_plots,        1);

SVEXTERN int SVDECL(glui_remove_selected_curve, 0);
SVEXTERN int SVDECL(glui_csv_file_index, 0);
SVEXTERN int SVDECL(icsv_cols, 0), SVDECL(icsv_units, 0);
SVEXTERN int SVDECL(icsv_compartments, -1);
SVEXTERN int SVDECL(plot2d_max_columns, 0);
SVEXTERN int SVDECL(deviceunits_index, 0);
SVEXTERN int SVDECL(plot2d_dialogs_defined, 0);
SVEXTERN int SVDECL(glui_device_unit_index, -1);
SVEXTERN int SVDECL(glui_device_index, 0);
SVEXTERN int SVDECL(glui_hrr_unit_index, -1);
SVEXTERN int SVDECL(glui_hrr_index, 1);
SVEXTERN plot2ddata SVDECL(*glui_plot2dinfo, NULL), SVDECL(*plot2dinfo, NULL), SVDECL(*plot2dini, NULL);
SVEXTERN int SVDECL(nplot2dinfo, 0), SVDECL(iplot2dinfo, 0), SVDECL(nplot2dini, 0);
SVEXTERN curvedata glui_curve, glui_curve_default;
SVEXTERN devicedata SVDECL(**deviceunits, NULL);
SVEXTERN int SVDECL(ndeviceunits, 0);
SVEXTERN hrrdata SVDECL(**hrrunits, NULL);
SVEXTERN int SVDECL(nhrrunits, 0);

SVEXTERN int SVDECL(sort_geometry,1),SVDECL(sort_transparent_faces,1);
SVEXTERN isotri SVDECL(***iso_trans_list,NULL),SVDECL(***iso_opaques_list,NULL);
SVEXTERN int SVDECL(*niso_trans_list,NULL),SVDECL(*niso_opaques_list,NULL);
SVEXTERN int SVDECL(niso_timesteps,0);
SVEXTERN isotri SVDECL(**iso_trans,NULL),SVDECL(**iso_opaques,NULL);
SVEXTERN int SVDECL(niso_trans,0),SVDECL(niso_opaques,0);
SVEXTERN int SVDECL(sort_iso_triangles,1);
SVEXTERN int SVDECL(object_outlines,0), SVDECL(object_box, 0);
SVEXTERN int SVDECL(usemenu,1);
SVEXTERN float direction_color[4], SVDECL(*direction_color_ptr,NULL);
#ifdef INMAIN
SVEXTERN float hrrpuv_iso_color[4]={1.0,0.5,0.0,1.0};
#else
SVEXTERN float hrrpuv_iso_color[4];
#endif
SVEXTERN float SVDECL(right_green,0.0), SVDECL(right_blue,1.0);

SVEXTERN int SVDECL(saved_colorbar, -1);
SVEXTERN int SVDECL(levelset_colorbar,-1), SVDECL(wallthickness_colorbar,-1);
SVEXTERN colorbardata SVDECL(*fire_colorbar,NULL);
SVEXTERN float SVDECL(glui_time,0.0);
SVEXTERN int show_mode;
SVEXTERN int SVDECL(part5colorindex,0), SVDECL(show_tracers_always,0);
SVEXTERN int SVDECL(select_avatar,0), SVDECL(selected_avatar_tag,-1), SVDECL(view_from_selected_avatar,0);
SVEXTERN float selected_avatar_pos[3], selected_avatar_angle;
SVEXTERN unsigned char select_device_color[4], SVDECL(*select_device_color_ptr,NULL);
SVEXTERN int SVDECL(script_render_flag,0), SVDECL(script_itime,0);

SVEXTERN int SVDECL(offset_slice,0);
SVEXTERN int SVDECL(show_slice_in_solid, 0), SVDECL(show_slice_in_gas, 1);
SVEXTERN int SVDECL(skip_slice_in_embedded_mesh,0);
SVEXTERN int n_embedded_meshes;

SVEXTERN int npartframes_max;
SVEXTERN int SVDECL(force_isometric,0);
SVEXTERN int SVDECL(update_startup_view,0);
SVEXTERN int SVDECL(cullgeom_portsize,16);
SVEXTERN int SVDECL(update_initcullgeom,1),SVDECL(cullgeom,1);
SVEXTERN int opengl_version;
SVEXTERN char opengl_version_label[256];

SVEXTERN int SVDECL(usevolrender,1);
#ifdef pp_GPU
SVEXTERN int SVDECL(usegpu,0),SVDECL(gpuactive,0);
SVEXTERN int GPU_skip, GPU_hrrcutoff, GPU_hrr, GPU_hrrpuv_max_smv, GPU_hrrpuv_cutoff;
SVEXTERN int GPU_fire_alpha, GPU_firecolor, GPU_have_smoke, GPU_smokecolormap;
SVEXTERN int GPU_smokeshade,GPU_smokealpha;
SVEXTERN int GPU_use_fire_alpha, GPU_emission_factor;

SVEXTERN int GPUzone_zonedir;
SVEXTERN int GPUzone_zoneinside;
SVEXTERN int GPUzone_eyepos;
SVEXTERN int GPUzone_xyzmaxdiff;
SVEXTERN int GPUzone_boxmin, GPUzone_boxmax;
SVEXTERN int GPUzone_zlay;
SVEXTERN int GPUzone_odl, GPUzone_odu;

SVEXTERN int GPUvol_inside, GPUvol_eyepos, GPUvol_xyzmaxdiff, GPUvol_slicetype,GPUvol_dcell3;
SVEXTERN int GPUvol_gpu_vol_factor;
SVEXTERN int GPUvol_soot_density, GPUvol_fire, GPUvol_blockage;
SVEXTERN int GPUvol_fire_opacity_factor, GPUvol_volbw, GPUvol_mass_extinct;
SVEXTERN int GPUvol_temperature_min,GPUvol_temperature_cutoff,GPUvol_temperature_max;
SVEXTERN int GPUvol_boxmin, GPUvol_boxmax, GPUvol_drawsides;
SVEXTERN int GPUvol_smokecolormap, GPUvol_dcell, GPUvol_havefire;

SVEXTERN int GPU3dslice_valtexture,GPU3dslice_colormap;
SVEXTERN int GPU3dslice_val_min,GPU3dslice_val_max;
SVEXTERN int GPU3dslice_boxmin, GPU3dslice_boxmax;
SVEXTERN int GPU3dslice_transparent_level;
SVEXTERN int GPUvol_block_volsmoke;
#endif

SVEXTERN int SVDECL(vis_device_plot, 0);
SVEXTERN int SVDECL(vis_hrr_plot, 0);
SVEXTERN int SVDECL(vis_slice_plot, 0);
SVEXTERN int SVDECL(vis_colorbar_dists_plot, 0);

SVEXTERN char hrrlabel[256];
SVEXTERN int SVDECL(glui_hrr, 1);
SVEXTERN int SVDECL(hoc_hrr, 0);
SVEXTERN int SVDECL(update_avg, 0);
SVEXTERN int smoke_render_option;
SVEXTERN float fnear, ffar;
#ifdef INMAIN
SVEXTERN float partfacedir[3]={0.0,0.0,1.0};
#else
SVEXTERN float partfacedir[3];
#endif
SVEXTERN int SVDECL(demo_option,0);
SVEXTERN int SVDECL(outout_patch_faces, 0);
SVEXTERN int SVDECL(boundary_interface_unhide, 0), SVDECL(boundary_loaded, 0);

SVEXTERN int colorbar_font_height, font_height;
SVEXTERN void SVDECL(*colorbar_font_ptr, NULL), SVDECL(*font_ptr,NULL);

SVEXTERN float SVDECL(cb_valmin,0.0), SVDECL(cb_valmax,100.0), SVDECL(cb_val,50.0);
SVEXTERN int SVDECL(cb_colorindex,128);
SVEXTERN float rgbterrain[4*MAXRGB];
#ifdef INMAIN
SVEXTERN int terrain_rgba_zmin[3]={90,50,50};
SVEXTERN int terrain_rgba_zmax[3]={200,200,200};
#else
SVEXTERN int terrain_rgba_zmin[3];
SVEXTERN int terrain_rgba_zmax[3];
#endif
SVEXTERN float SVDECL(vertical_factor,1.0);

SVEXTERN char input_filename_ext[5];

SVEXTERN float SVDECL(fire_line_min,150.0), SVDECL(fire_line_max,200.0);
SVEXTERN int SVDECL(fire_line_index,-1);
SVEXTERN int SVDECL(slice_bounds_dialog,1);

SVEXTERN float xtemp;

SVEXTERN float glui_xyz_fds[3],user_zaxis[3];
#ifdef INMAIN
  SVEXTERN float zaxis_angles[3]={0.000000, 90.000000, 0.000000};
  SVEXTERN float zaxis_angles_orig[3] = {0.000000, 90.000000, 0.000000};
#else
  SVEXTERN float zaxis_angles[3];
  SVEXTERN float zaxis_angles_orig[3];
#endif

#ifdef INMAIN
SVEXTERN float tourcol_selectedpathline[3]={1.0,0.0,0.0};
SVEXTERN float tourcol_selectedpathlineknots[3]={1.0,0.0,0.0};
SVEXTERN float tourcol_selectedknot[3]={1.0,0.0,0.0};
SVEXTERN float tourcol_selectedview[3]={1.0,0.0,0.0};
SVEXTERN float tourcol_pathline[3]={-1.0,-1.0,-1.0};
SVEXTERN float tourcol_pathknots[3]={-1.0,-1.0,-1.0};
SVEXTERN float tourcol_text[3]={-1.0,-1.0,-1.0};
SVEXTERN float tourcol_avatar[3]={1.0,0.0,0.0};
#else
SVEXTERN float tourcol_selectedpathline[3];
SVEXTERN float tourcol_selectedpathlineknots[3];
SVEXTERN float tourcol_selectedknot[3];
SVEXTERN float tourcol_selectedview[3];
SVEXTERN float tourcol_pathline[3];
SVEXTERN float tourcol_pathknots[3];
SVEXTERN float tourcol_text[3];
SVEXTERN float tourcol_avatar[3];
#endif
SVEXTERN float mat_ambient_orig[4];
SVEXTERN float mat_specular_orig[4];
SVEXTERN float SVDECL(*mat_ambient2,NULL), SVDECL(*mat_specular2,NULL);

#ifdef INMAIN
SVEXTERN GLfloat iso_specular[4]={0.7,0.7,0.7,1.0};
#else
SVEXTERN GLfloat iso_specular[4];
#endif
SVEXTERN GLfloat SVDECL(iso_shininess, 50.0), SVDECL(glui_shininess, 50.0);

SVEXTERN float block_ambient_orig[4];
SVEXTERN float SVDECL(*block_ambient2,NULL);
SVEXTERN float block_specular_orig[4];
SVEXTERN float SVDECL(*block_specular2,NULL);
SVEXTERN GLfloat SVDECL(block_shininess,100.0);

#ifdef INMAIN
SVEXTERN GLfloat light_position0[4]={1.0,1.0,1.0,0.0};
SVEXTERN GLfloat light_position1[4]={-1.0,-1.0,1.0,0.0};
SVEXTERN GLfloat ambientlight[4]={0.6f,0.6f,0.6f,1.0f};
SVEXTERN GLfloat diffuselight[4]={0.5f,0.5f,0.5f,1.0f};
#else
SVEXTERN GLfloat light_position0[4];
SVEXTERN GLfloat light_position1[4];
SVEXTERN GLfloat ambientlight[4];
SVEXTERN GLfloat diffuselight[4];
#endif

SVEXTERN int SVDECL(drawlights,0);
SVEXTERN GLfloat specularlight[4];
SVEXTERN float SVDECL(speculargrey, 0.4);
SVEXTERN float SVDECL(ambientgrey,0.4);
SVEXTERN float SVDECL(diffusegrey,0.4);

SVEXTERN float SVDECL(glui_light_az0,45.0);
SVEXTERN float SVDECL(glui_light_elev0,30.0);
SVEXTERN float SVDECL(glui_light_az1,135.0);
SVEXTERN float SVDECL(glui_light_elev1,30.0);

SVEXTERN int glui_specularlight[4], glui_speculargrey;
SVEXTERN int glui_ambientlight[4], glui_ambientgrey;
SVEXTERN int glui_diffuselight[4], glui_diffusegrey;

SVEXTERN int list_p3_index,list_slice_index,list_patch_index,list_iso_index;
SVEXTERN int SVDECL(list_p3_index_old,0), SVDECL(list_patch_index_old,0);

SVEXTERN float glui_block_xmin, glui_block_ymin, glui_block_zmin;
SVEXTERN float glui_block_xmax, glui_block_ymax, glui_block_zmax;

SVEXTERN int SVDECL(nzonetotal,0);
SVEXTERN float SVDECL(zoneglobalmin,0.0), SVDECL(zoneglobalmax,0.0);
SVEXTERN float SVDECL(zoneusermin,0.0), SVDECL(zoneusermax,0.0);
SVEXTERN float zonelevels256[256];
SVEXTERN float boundarylevels256[256];
SVEXTERN float partlevels256[256];
SVEXTERN float SVDECL(*zone_times,NULL), SVDECL(*zoneylay,NULL), SVDECL(*zonetl,NULL), SVDECL(*zonetu,NULL), SVDECL(*zonepr,NULL);
SVEXTERN float SVDECL(*zonefl, NULL), SVDECL(*zonelw, NULL), SVDECL(*zoneuw, NULL), SVDECL(*zonecl, NULL);
SVEXTERN float SVDECL(*zonetargets, NULL);
SVEXTERN int SVDECL(have_zonefl, 0), SVDECL(have_zonelw, 0), SVDECL(have_zoneuw, 0), SVDECL(have_zonecl, 0);
SVEXTERN int SVDECL(have_wall_data,0), SVDECL(vis_wall_data,0);
SVEXTERN int SVDECL(have_target_data, 0), SVDECL(vis_target_data, 1);
SVEXTERN int SVDECL(nzonetotal_targets, 0), SVDECL(nzone_targets,0);

SVEXTERN float SVDECL(*zonerhol, NULL), SVDECL(*zonerhou, NULL);
SVEXTERN float SVDECL(*zoneqfire,NULL), SVDECL(*zonefheight,NULL), SVDECL(*zonefbase,NULL), SVDECL(*zonefdiam,NULL);
SVEXTERN float SVDECL(*zoneodl,NULL), SVDECL(*zoneodu,NULL), SVDECL(*zonevents,NULL);
SVEXTERN float SVDECL(maxslabflow, 0.0);
SVEXTERN int SVDECL(have_ventslab_flow,0);
SVEXTERN float SVDECL(*zoneslab_T, NULL), SVDECL(*zoneslab_F, NULL), SVDECL(*zoneslab_YB, NULL), SVDECL(*zoneslab_YT, NULL);
SVEXTERN int SVDECL(*zoneslab_n, NULL);
SVEXTERN float zone_maxventflow;
SVEXTERN unsigned char SVDECL(*hazardcolor,NULL);
SVEXTERN float SVDECL(zone_ventfactor,1.0);
SVEXTERN unsigned char SVDECL(*izonetu,NULL);
SVEXTERN int SVDECL(show_zonelower, 1);
SVEXTERN unsigned char SVDECL(*izonetl, NULL);
SVEXTERN unsigned char SVDECL(*izonefl, NULL);
SVEXTERN unsigned char SVDECL(*izonelw, NULL);
SVEXTERN unsigned char SVDECL(*izoneuw, NULL);
SVEXTERN unsigned char SVDECL(*izonecl, NULL);
SVEXTERN unsigned char SVDECL(*izonetargets, NULL);
SVEXTERN int nzone_times;
SVEXTERN float barright;
SVEXTERN float SVDECL(*tspr,NULL);

SVEXTERN int SVDECL(videoSTEREO,0);
SVEXTERN float SVDECL(fzero,0.25);

SVEXTERN char blank_global[2];

SVEXTERN float SVDECL(*sphere_xyz,NULL);
SVEXTERN int SVDECL(demo_mode,0), SVDECL(update_demo,1);
SVEXTERN int SVDECL(loadplot3dall,0);
SVEXTERN char *shortp3label[MAXPLOT3DVARS], *unitp3label[MAXPLOT3DVARS];
SVEXTERN int SVDECL(update_plot3d_bnd, 0);

SVEXTERN int show3dsmoke;
SVEXTERN float frustum[6][4];
SVEXTERN int showtime, showtime2, showplot3d, showpatch, showslice, showvslice, showsmoke, showzone, showiso;
SVEXTERN int SVDECL(showvolrender,0);
SVEXTERN int SVDECL(vis_slice_contours,0);
SVEXTERN int SVDECL(update_slicecontours,0);
SVEXTERN int showiso_colorbar;
SVEXTERN int SVDECL(visgridloc,0);
SVEXTERN int SVDECL(valindex,0);

SVEXTERN int SVDECL(co2_colorbar_index_save, -1);
SVEXTERN int SVDECL(update_co2_colorbar_index, 0);
SVEXTERN int SVDECL(co2_colorbar_index_ini, 0);

SVEXTERN int SVDECL(fire_colorbar_index_save,-1);
SVEXTERN int SVDECL(update_fire_colorbar_index,0);
SVEXTERN int SVDECL(fire_colorbar_index_ini,0);
SVEXTERN float SVDECL(*rgb2_ini,NULL);
SVEXTERN float rgb_full[MAXRGB][4];
SVEXTERN float rgb_full2[MAXRGB][4];
SVEXTERN float rgb_terrain2[4 * MAXRGB];
SVEXTERN float rgb_slice[4 * MAXRGB];
SVEXTERN float rgb_volsmokecolormap[4*MAXSMOKERGB];
SVEXTERN float rgb_slicesmokecolormap_01[4*MAXSMOKERGB];
SVEXTERN float rgb_sliceco2colormap_01[4*MAXSMOKERGB];
SVEXTERN float rgb_iso[4*MAXRGB];
SVEXTERN float rgb_patch[4*MAXRGB];
SVEXTERN float rgb_plot3d[4*MAXRGB];
SVEXTERN float rgb_part[4*MAXRGB];
SVEXTERN float rgb_trans[4*MAXRGB];
SVEXTERN float rgb_cad[MAXRGB][4];

//keep comment until changes are merged
//SVEXTERN float SVDECL(iso_transparency,30.5/255.0), SVDECL(*iso_colors,NULL), SVDECL(*iso_colorsbw,NULL);
SVEXTERN float SVDECL(iso_transparency, 0.1196078), SVDECL(*iso_colors,NULL), SVDECL(*iso_colorsbw,NULL);
SVEXTERN int glui_iso_colors[4], SVDECL(glui_iso_level,1), glui_iso_transparency;

SVEXTERN float SVDECL(*rgb_ini,NULL);
SVEXTERN float SVDECL(mouse_deltax,0.0), SVDECL(mouse_deltay,0.0);
SVEXTERN float SVDECL(**rgbptr,NULL), SVDECL(**rgb_plot3d_contour,NULL);
#ifdef INMAIN
SVEXTERN float char_color[4]={0.0,0.0,0.0,0.0};
#else
SVEXTERN float char_color[4];
#endif
SVEXTERN float *rgb_step[255];
#ifdef INMAIN
SVEXTERN float movedir[3]={0.0,1.0,0.0};
#else
SVEXTERN float movedir[3];
#endif
SVEXTERN float rgb_base[MAXRGB][4];
SVEXTERN float bw_base[MAXRGB][4];
SVEXTERN float rgb2[MAXRGB][3];
SVEXTERN float inverse_modelview_setup[16];
SVEXTERN float modelview_setup[16];
SVEXTERN float modelview_rotate_last[16],modelview_rotate_save[16];
SVEXTERN float modelview_current[16];
SVEXTERN float modelview_scratch[16];

SVEXTERN int SVDECL(ncameras_sorted, 0);
SVEXTERN cameradata SVDECL(**cameras_sorted, NULL);
SVEXTERN cameradata SVDECL(*camera_current,NULL), SVDECL(*camera_save,NULL), SVDECL(*camera_last,NULL);
SVEXTERN cameradata SVDECL(*camera_external,NULL);
SVEXTERN cameradata SVDECL(**camera_defaults, NULL);
SVEXTERN cameradata SVDECL(*camera_external_save,NULL);
SVEXTERN cameradata camera_list_first, camera_list_last, SVDECL(**camera_list,NULL);
SVEXTERN int SVDECL(ncamera_list,0),SVDECL(i_view_list,1),SVDECL(init_camera_list, 1);
SVEXTERN int SVDECL(camera_max_id,2);
SVEXTERN int SVDECL(startup,0),SVDECL(startup_view_ini,1),SVDECL(selected_view,-999);
SVEXTERN char viewpoint_label_startup[256];
SVEXTERN char SVDECL(*camera_label,NULL);
SVEXTERN char SVDECL(*colorbar_label,NULL);
SVEXTERN char SVDECL(*colorbar_filename, NULL);

SVEXTERN int vis_boundary_type[7], SVDECL(update_ini_boundary_type,0);
SVEXTERN int p3_extreme_min[MAXPLOT3DVARS], p3_extreme_max[MAXPLOT3DVARS];
SVEXTERN int SVDECL(show_mirror_boundary,0), SVDECL(show_open_boundary, 0);
SVEXTERN int SVDECL(n_mirrorvents,0), SVDECL(n_openvents,0);
SVEXTERN int SVDECL(from_read_boundary, 0);
SVEXTERN int SVDECL(have_boundary_arrival, 0);


SVEXTERN int setp3min_all[MAXPLOT3DVARS];
SVEXTERN float p3min_all[MAXPLOT3DVARS], p3min_global[MAXPLOT3DVARS];

SVEXTERN int setp3max_all[MAXPLOT3DVARS];
SVEXTERN float p3max_all[MAXPLOT3DVARS], p3max_global[MAXPLOT3DVARS];
SVEXTERN float p3min_loaded[MAXPLOT3DVARS], p3max_loaded[MAXPLOT3DVARS];

SVEXTERN int setp3chopmin[MAXPLOT3DVARS], setp3chopmax[MAXPLOT3DVARS];
SVEXTERN float p3chopmin[MAXPLOT3DVARS], p3chopmax[MAXPLOT3DVARS];

SVEXTERN int SVDECL(trainer_pause,0), SVDECL(trainee_location,0), SVDECL(trainer_inside,0);
SVEXTERN int SVDECL(from_glui_trainer,0);
SVEXTERN int SVDECL(trainer_path_old,-3);
SVEXTERN int SVDECL(trainer_outline,1);
SVEXTERN int SVDECL(trainer_viewpoints,-1), SVDECL(ntrainer_viewpoints,0);
SVEXTERN int SVDECL(trainer_realtime,1);
SVEXTERN int SVDECL(trainer_path,0);
#ifdef INMAIN
SVEXTERN float trainer_xzy[3]={0.0,0.0,0.0};
SVEXTERN float trainer_ab[2]={0.0,0.0};
SVEXTERN float motion_ab[2]={0.0,0.0};
SVEXTERN float motion_dir[2]={0.0,0.0};
#else
SVEXTERN float trainer_xzy[3];
SVEXTERN float trainer_ab[2];
SVEXTERN float motion_ab[2];
SVEXTERN float motion_dir[2];
#endif
SVEXTERN float motion_ab[2], motion_dir[2];
SVEXTERN int SVDECL(trainerload,0),SVDECL(trainerload_old,0);
SVEXTERN int SVDECL(fontsize_save, 0);
SVEXTERN int SVDECL(trainer_mode,0);
SVEXTERN int SVDECL(trainer_active,0);
SVEXTERN int SVDECL(slice_average_flag,0);
SVEXTERN int SVDECL(show_slice_average, 0), SVDECL(vis_slice_average, 1);
SVEXTERN float SVDECL(slice_average_interval,10.0);

SVEXTERN int SVDECL(maxtourframes,500);
SVEXTERN int SVDECL(blockageSelect,0);
SVEXTERN int SVDECL(ntourknots,0);
SVEXTERN int SVDECL(itourknots,-1);
SVEXTERN int SVDECL(stretch_var_black,0), SVDECL(stretch_var_white,0), SVDECL(move_var,0);

SVEXTERN int SVDECL(showhide_option,SHOWALL_FILES);
SVEXTERN int SVDECL(xyz_dir,0);
SVEXTERN int SVDECL(which_face,2);

SVEXTERN float SVDECL(vecfactor,1.0);

SVEXTERN int SVDECL(glui_active,0);

SVEXTERN int SVDECL(old_draw_colorlabel,0);
SVEXTERN int SVDECL(vis3DSmoke3D,1);
SVEXTERN int SVDECL(smokeskip,1),SVDECL(smoke3d_frame_inc,1),SVDECL(update_smoke3d_frame_inc,0);;
SVEXTERN float SVDECL(scene_aspect_ratio,1.0);

SVEXTERN int SVDECL(fix_window_aspect, 0);
//keep comment until changes are merged
//SVEXTERN float SVDECL(window_aspect,640.0/480.0);
SVEXTERN float SVDECL(window_aspect,1.333333);
SVEXTERN int SVDECL(screenWidth,640), SVDECL(screenHeight,480);
SVEXTERN int SVDECL(screenX0, 0), SVDECL(screenY0, 0);
SVEXTERN int SVDECL(dialogX0, 0), SVDECL(dialogY0, 0);
SVEXTERN int SVDECL(have_dialogX0, 0), SVDECL(have_dialogY0, 0);
SVEXTERN int SVDECL(use_commandline_origin, 0);
SVEXTERN int screenWidth_save, screenHeight_save;
SVEXTERN int SVDECL(screenWidthINI,640), SVDECL(screenHeightINI,480);
SVEXTERN int SVDECL(glui_screenWidth,640), SVDECL(glui_screenHeight,480);
SVEXTERN int SVDECL(windowsize_pointer,0);
SVEXTERN int SVDECL(zonecolortype, ZONETEMP_COLOR);
SVEXTERN int mxframepoints;
SVEXTERN int SVDECL(timebar_drag,0),SVDECL(colorbar_drag,0),SVDECL(colorbar_splitdrag,0),SVDECL(colorbaredit_drag,0);
SVEXTERN int SVDECL(global_colorbar_index,-1);

SVEXTERN int SVDECL(fontindex,SMALL_FONT);
SVEXTERN int SVDECL(scaled_font2d_height,12);
SVEXTERN float SVDECL(scaled_font2d_height2width,1.0);
SVEXTERN int SVDECL(scaled_font3d_height,32);
SVEXTERN float SVDECL(scaled_font3d_height2width,1.0);

SVEXTERN int SVDECL(custom_worldcenter,0),SVDECL(show_rotation_center,0);
SVEXTERN float SVDECL(xcenGLOBAL,0.5), SVDECL(ycenGLOBAL,0.5), SVDECL(zcenGLOBAL,0.5);
SVEXTERN float SVDECL(xcenCUSTOM,0.5), SVDECL(ycenCUSTOM,0.5), SVDECL(zcenCUSTOM,0.5);
SVEXTERN float SVDECL(xcenCUSTOMsmv,0.5), SVDECL(ycenCUSTOMsmv,0.5), SVDECL(zcenCUSTOMsmv,0.5);
SVEXTERN int glui_rotation_index,SVDECL(update_rotation_center,0);
SVEXTERN int glui_rotation_index_ini,SVDECL(update_rotation_center_ini,0);

SVEXTERN float SVDECL(xbarORIG,1.0), SVDECL(ybarORIG,1.0), SVDECL(zbarORIG,1.0);
SVEXTERN float SVDECL(xbar0ORIG,0.0), SVDECL(ybar0ORIG,0.0), SVDECL(zbar0ORIG,0.0);
SVEXTERN float xbarFDS, ybarFDS, zbarFDS;
SVEXTERN float xbar0FDS, ybar0FDS, zbar0FDS;
SVEXTERN int SVDECL(ReadIsoFile,0);
SVEXTERN int SVDECL(ReadVolSlice,0);
SVEXTERN int SVDECL(ReadZoneFile,0), SVDECL(ReadPartFile,0);

SVEXTERN int SVDECL(cache_plot3d_data,1);
SVEXTERN int SVDECL(cache_boundary_data, 1);
SVEXTERN int SVDECL(cache_slice_data, 1);
SVEXTERN int SVDECL(cache_hvac_data, 1);
SVEXTERN int SVDECL(cache_part_data, 1);
SVEXTERN int SVDECL(cache_file_data, 1);

SVEXTERN int SVDECL(update_cache_data, 0);

SVEXTERN int SVDECL(editwindow_status,-1);
SVEXTERN int SVDECL(startup_pass,1);

SVEXTERN int SVDECL(slicefilenumber,0);
SVEXTERN int SVDECL(frame_count,1), SVDECL(last_frame_count,1);
SVEXTERN int SVDECL(nspr,0);
SVEXTERN int SVDECL(render_skip,RENDER_CURRENT_SINGLE);
SVEXTERN int SVDECL(vectorskip,1);
SVEXTERN int SVDECL(first_frame_index,0), SVDECL(izone,0);
SVEXTERN int SVDECL(rotation_type,ROTATION_2AXIS),SVDECL(eyeview_level,1);
SVEXTERN int SVDECL(rotation_type_old,ROTATION_2AXIS),SVDECL(eyeview_SAVE,0),SVDECL(eyeview_last,0);
SVEXTERN int SVDECL(frameratevalue,1000);
SVEXTERN int SVDECL(setpartmin,PERCENTILE_MIN),   SVDECL(setpartmax,PERCENTILE_MAX);
SVEXTERN int SVDECL(setisomin, GLOBAL_MIN),   SVDECL(setisomax, GLOBAL_MAX);
SVEXTERN int SVDECL(glui_setslicemin,GLOBAL_MIN), SVDECL(glui_setslicemax,GLOBAL_MAX);

SVEXTERN float slice_line_contour_min;
SVEXTERN float slice_line_contour_max;
SVEXTERN int slice_line_contour_num;
SVEXTERN int setpartmin_old, setpartmax_old;
SVEXTERN int SVDECL(glui_setpatchmin,GLOBAL_MIN), SVDECL(glui_setpatchmax,GLOBAL_MAX), SVDECL(setzonemin,GLOBAL_MIN), SVDECL(setzonemax,GLOBAL_MAX);
SVEXTERN int SVDECL(loadpatchbysteps,UNCOMPRESSED );
SVEXTERN int SVDECL(settargetmin,0), SVDECL(settargetmax,0);
SVEXTERN int SVDECL(setpartchopmin,0), SVDECL(setpartchopmax,0);
SVEXTERN int SVDECL(setpatchchopmin,0), SVDECL(setpatchchopmax,0);
SVEXTERN float SVDECL(partchopmin,1.0),  SVDECL(partchopmax,0.0);
SVEXTERN int SVDECL(glui_setslicechopmin, 0), SVDECL(glui_setslicechopmax, 0);
SVEXTERN float SVDECL(glui_slicechopmin,1.0), SVDECL(glui_slicechopmax,0.0);
SVEXTERN float SVDECL(patchchopmin,1.0), SVDECL(patchchopmax,0.0);
SVEXTERN int setisomin, setisomax;
SVEXTERN float isomin, isomax;
SVEXTERN int setisochopmin, setisochopmax;
SVEXTERN float isochopmin, isochopmax;

SVEXTERN int SVDECL(vis_onlythreshold,0), SVDECL(vis_threshold,0), SVDECL(canshow_threshold,1), SVDECL(activate_threshold,1);
SVEXTERN float SVDECL(glui_patchmin,1.0),SVDECL(glui_patchmax,0.0);
SVEXTERN float SVDECL(targetmin, 1.0), SVDECL(targetmax, 0.0);
SVEXTERN float SVDECL(glui_partmin,1.0), SVDECL(glui_partmax,0.0);
SVEXTERN float SVDECL(glui_slicemin,1.0),       SVDECL(glui_slicemax,0.0);

SVEXTERN float SVDECL(zonemin,1.0), SVDECL(zonemax,0.0);
SVEXTERN float SVDECL(plot3d_uvw_max,1.0);
SVEXTERN float SVDECL(hrrpuv_max_smv,1200.0);
SVEXTERN int SVDECL(FlowDir,1),SVDECL(ClipDir,1);
SVEXTERN int SVDECL(plotn,1);
SVEXTERN int SVDECL(stept,0);
SVEXTERN int SVDECL(plotstate,NO_PLOTS);
SVEXTERN int SVDECL(visVector,0);
SVEXTERN int SVDECL(visSmokePart,2), SVDECL(visSprinkPart,1), SVDECL(havesprinkpart,0);
SVEXTERN int SVDECL(visaxislabels,0);
SVEXTERN int SVDECL(numplot3dvars,0);
SVEXTERN int SVDECL(skip_global,1);
SVEXTERN int SVDECL(p3dsurfacesmooth,1);
SVEXTERN int SVDECL(p3dsurfacetype,SURFACE_SOLID);

SVEXTERN int SVDECL(parttype,0);
SVEXTERN int SVDECL(showedit_dialog,0);
SVEXTERN int SVDECL(showterrain_dialog, 0);
SVEXTERN int SVDECL(showhvac_dialog, 0);
SVEXTERN int SVDECL(showcolorbar_dialog,0);
SVEXTERN int SVDECL(showtour_dialog,0),SVDECL(showtrainer_dialog,0);
SVEXTERN int SVDECL(showtours,0);
SVEXTERN int SVDECL(large_case, 0);

SVEXTERN float shooter_xyz[3], shooter_dxyz[3], SVDECL(shooterpointsize,4.0);
#ifdef INMAIN
SVEXTERN float shooter_uvw[3]={0.0,0.0,0.0};
#else
SVEXTERN float shooter_uvw[3];
#endif
SVEXTERN float shooter_velx, shooter_vely, shooter_velz, shooter_time, shooter_time_max;
SVEXTERN int SVDECL(shooter_cont_update,0),SVDECL(shooter_firstframe,0);
//keep comment until changes are merged
//SVEXTERN float SVDECL(shooter_u0,2.0), SVDECL(shooter_z0,1.0), SVDECL(shooter_p,1.0/7.0), SVDECL(shooter_v_inf,1.0);
SVEXTERN float SVDECL(shooter_u0,2.0), SVDECL(shooter_z0,1.0), SVDECL(shooter_p, 0.1428571), SVDECL(shooter_v_inf,1.0);
SVEXTERN float shooter_velmag, shooter_veldir, shooter_duration, SVDECL(shooter_history,10.0);
SVEXTERN int SVDECL(shooter_active,0);
SVEXTERN int shooter_fps,shooter_vel_type, shooter_nparts, SVDECL(visShooter,0), showshooter, nshooter_frames, max_shooter_points;
SVEXTERN shootpointdata SVDECL(*shootpointinfo,NULL);
SVEXTERN shoottimedata SVDECL(*shoottimeinfo,NULL);
SVEXTERN int SVDECL(*shooter_timeslist,NULL);
SVEXTERN int SVDECL(shooter_itime,0);

SVEXTERN int SVDECL(showgluitrainer,0);
SVEXTERN int SVDECL(colorbartype,0),SVDECL(colorbartype_ini,-1), SVDECL(colorbartype_default, 0), SVDECL(colorbartype_last, -1);
SVEXTERN char colorbarname[1024];
SVEXTERN int SVDECL(update_colorbartype,0);
SVEXTERN int SVDECL(colorbartype_save,-1);
SVEXTERN int SVDECL(colorbarpoint,0);
SVEXTERN int SVDECL(vectorspresent,0);
SVEXTERN int SVDECL(colorbar_showscene,0);
SVEXTERN int SVDECL(no_bounds, 0), SVDECL(force_bounds, 0);

SVEXTERN int SVDECL(visAIso,1);
SVEXTERN int SVDECL(surfincrement,0),SVDECL(visiso,0);
SVEXTERN int SVDECL(isotest,0);
SVEXTERN int SVDECL(isolevelindex,0), SVDECL(isolevelindex2,0);
SVEXTERN int SVDECL(n_devices,0);

SVEXTERN float SVDECL(*globalmin_part, NULL), SVDECL(*globalmax_part, NULL);

SVEXTERN int SVDECL(sliceload_boundtype, 0);
SVEXTERN int SVDECL(sliceload_filetype, 0);
SVEXTERN int SVDECL(sliceload_dir,0);
SVEXTERN int SVDECL(sliceload_isvector, 0);

SVEXTERN sliceparmdata sliceparminfo;

SVEXTERN int SVDECL(nslicebounds, 0), SVDECL(npatchbounds,0), SVDECL(npatch2,0);
SVEXTERN int SVDECL(niso_bounds,0);
SVEXTERN int SVDECL(visLabels,0);
SVEXTERN float SVDECL(framerate,-1.0);
SVEXTERN int SVDECL(seqnum,0),SVDECL(RenderTime,0),SVDECL(RenderTimeOld,0), SVDECL(itime_cycle,0);
SVEXTERN int SVDECL(nopart,1);
SVEXTERN int SVDECL(uindex,-1), SVDECL(vindex,-1), SVDECL(windex,-1);

SVEXTERN int SVDECL(contour_type,0), SVDECL(p3cont3dsmooth,0);
SVEXTERN int SVDECL(contour_type_save, -1);
SVEXTERN int SVDECL(is_toa_slice, 0);
SVEXTERN int SVDECL(cullfaces,1);
SVEXTERN int SVDECL(showonly_hiddenfaces,0);
SVEXTERN int SVDECL(update_bound_chop_data, 0);

SVEXTERN int SVDECL(windowresized,0);

SVEXTERN int SVDECL(updatemenu,0), SVDECL(first_display,2);
SVEXTERN int SVDECL(updatezoommenu,0),SVDECL(updatezoomini,0);
SVEXTERN int SVDECL(updatemenu_count,0);
SVEXTERN int SVDECL(use_graphics,1);

SVEXTERN int SVDECL(updatefacelists,0);
SVEXTERN int SVDECL(updateOpenSMVFile,0);

SVEXTERN int SVDECL(periodic_reloads, 0), SVDECL(periodic_reload_value, 2);

SVEXTERN int SVDECL(slicefilenum,-1), SVDECL(zonefilenum,-1), SVDECL(targfilenum,-1);

SVEXTERN float min_gridcell_size;

SVEXTERN volfacelistdata SVDECL(*volfacelistinfo,NULL),SVDECL(**volfacelistinfoptrs,NULL);
SVEXTERN int SVDECL(nvolfacelistinfo,0);
SVEXTERN int SVDECL(menustatus,GLUT_MENU_NOT_IN_USE);
SVEXTERN int SVDECL(visTimeZone,1), SVDECL(visTimeParticles,1), SVDECL(visTimeSlice,1), SVDECL(visTimeBoundary,1);
SVEXTERN int SVDECL(visTimeIso,1);
SVEXTERN int SVDECL(vishmsTimelabel,0), SVDECL(visTimebar,1);
SVEXTERN int SVDECL(visColorbarVertical,1), SVDECL(visColorbarVertical_save,1);
SVEXTERN int SVDECL(update_visColorbars,0), visColorbarVertical_val, visColorbarHorizontal_val;

SVEXTERN int SVDECL(visColorbarHorizontal, 0), SVDECL(visColorbarHorizontal_save, 0);
SVEXTERN int SVDECL(visFullTitle, 1), SVDECL(visFramerate, 0);
SVEXTERN int SVDECL(visFramelabel,1), SVDECL(visTimelabel,1);
SVEXTERN int SVDECL(vis_hrr_label,0);
#ifdef pp_memstatus
SVEXTERN int SVDECL(visAvailmemory, 0);
#endif
SVEXTERN int SVDECL(block_volsmoke,1),SVDECL(smoke3dVoldebug,0);
SVEXTERN slicedata SVDECL(*sd_shown,NULL);
SVEXTERN vslicedata SVDECL(*vd_shown,NULL);
SVEXTERN int SVDECL(showall_slices,1);
SVEXTERN float zterrain_max, zterrain_min;
SVEXTERN char smv_githash[256], smv_gitdate[256];
SVEXTERN int SVDECL(visMeshlabel, 1);
SVEXTERN int SVDECL(visOpenVents,1),SVDECL(visDummyVents,1),SVDECL(visCircularVents,VENT_CIRCLE);
SVEXTERN int SVDECL(visOpenVentsAsOutline,0);
SVEXTERN int SVDECL(visParticles,1), SVDECL(visZone,0);

// need to clean up these variables
#ifdef pp_PATCH_HIDE
SVEXTERN int SVDECL(menu_hide_internal_blockages, 0);
#endif
SVEXTERN int SVDECL(hide_internal_blockages, 0);

SVEXTERN int SVDECL(outline_color_flag,0);
SVEXTERN int visBlocks,                SVDECL(solid_state,-1),      SVDECL(outline_state,-1);
SVEXTERN int SVDECL(visBlocks_ini,-1), SVDECL(solid_state_ini, -1), SVDECL(outline_state_ini, -1);
SVEXTERN int SVDECL(update_showblock_ini, 0);

SVEXTERN int SVDECL(visTransparentBlockage,0);
SVEXTERN int SVDECL(blocklocation,BLOCKlocation_grid);
SVEXTERN int SVDECL(blocklocation_menu, BLOCKlocation_grid);
SVEXTERN int SVDECL(visNormalEditColors,1);
SVEXTERN int SVDECL(visGrid,0);
SVEXTERN int SVDECL(visZonePlane,ZONE_YPLANE), SVDECL(viszonefire,1), SVDECL(visSZone,0);
SVEXTERN int SVDECL(visSensor,1), SVDECL(visSensorNorm,1);
SVEXTERN int SVDECL(visVents, 1), SVDECL(visVentFlow, 1),SVDECL(visVentHFlow, 1),SVDECL(visVentVFlow, 1),SVDECL(visVentMFlow, 1);
SVEXTERN int SVDECL(update_ini_vents, 0);
SVEXTERN int SVDECL(viewoption,0);
SVEXTERN int SVDECL(clip_mode,CLIP_OFF),clip_mode_last;
SVEXTERN int clip_i,clip_j,clip_k;
SVEXTERN clipdata clipinfo,colorbar_clipinfo;
SVEXTERN int stepclip_xmin,stepclip_ymin,stepclip_zmin;
SVEXTERN int stepclip_xmax,stepclip_ymax,stepclip_zmax;
SVEXTERN float SVDECL(partpointsize,4.0),SVDECL(vectorpointsize,3.0),SVDECL(streaklinewidth,1.0);
SVEXTERN float SVDECL(isopointsize,4.0), SVDECL(isolinewidth,2.0);
SVEXTERN float SVDECL(plot3dpointsize,4.0), SVDECL(plot3dlinewidth,2.0);
SVEXTERN int SVDECL(scaled_font3d_thickness,1);
SVEXTERN int SVDECL(scaled_font2d_thickness,1);
SVEXTERN float SVDECL(vectorlinewidth,1.0);
SVEXTERN int SVDECL(show_slice_values_all_regions,0);
SVEXTERN float SVDECL(gridlinewidth,2.0),SVDECL(ticklinewidth,2.0);
SVEXTERN int SVDECL(zone_highlight,0),SVDECL(zone_highlight_room,0);
SVEXTERN int SVDECL(script_step,0), SVDECL(script_step_now,0);
SVEXTERN int SVDECL(script_keystate,0);
SVEXTERN int SVDECL(clip_rendered_scene,0);

SVEXTERN float SVDECL(sprinklerabssize,0.076f), SVDECL(sensorabssize,0.38f), SVDECL(heatabssize,0.076f);
SVEXTERN float SVDECL(sensorrelsize,1.0),SVDECL(sensorrelsizeMIN,0.0);
SVEXTERN float SVDECL(vector_baselength,1.0);
SVEXTERN float SVDECL(vector_basediameter,0.1);
SVEXTERN float SVDECL(vector_headlength,0.2);
SVEXTERN float SVDECL(vector_headdiameter,0.2);

SVEXTERN float SVDECL(highlight_linewidth, 4.0);
SVEXTERN float solidlinewidth;
SVEXTERN float SVDECL(sliceoffset_factor,0.1), SVDECL(ventoffset_factor,0.2), SVDECL(boundaryoffset, 0.0);
SVEXTERN int SVDECL(visBLOCKold,-1);

SVEXTERN int SVDECL(planar_terrain_slice,0);
SVEXTERN int SVDECL(nrgb_ini,-1);
SVEXTERN int SVDECL(nrgb2_ini,0);
SVEXTERN int SVDECL(rgb_white,NRGB), SVDECL(rgb_yellow,NRGB+1), SVDECL(rgb_blue,NRGB+2), SVDECL(rgb_red,NRGB+3);
SVEXTERN int SVDECL(rgb_green,NRGB+4), SVDECL(rgb_magenta,NRGB+5), SVDECL(rgb_cyan,NRGB+6), SVDECL(rgb_black,NRGB+7);
SVEXTERN int SVDECL(num_colorbars,0);
SVEXTERN int SVDECL(setbw,0),SVDECL(setbwdata,0);
SVEXTERN int setbwSAVE;
SVEXTERN int SVDECL(background_flip,1);
SVEXTERN float SVDECL(transparent_level,0.8);
SVEXTERN int SVDECL(use_transparency_data,1);
SVEXTERN int SVDECL(antialiasflag,1);
SVEXTERN int SVDECL(nrgb_full,256);
SVEXTERN int SVDECL(nrgb_cad,256);
SVEXTERN float SVDECL(eyexfactor,0.5f), SVDECL(eyeyfactor,-0.9f), SVDECL(eyezfactor,0.5f);
SVEXTERN float SVDECL(geomyfactor,1.0), SVDECL(geomzfactor,1.0);
SVEXTERN float SVDECL(geom_xmin,0.0), SVDECL(geom_xmax,1.0);
SVEXTERN float SVDECL(geom_ymin,0.0), SVDECL(geom_ymax,1.0);
SVEXTERN float SVDECL(geom_zmin, 0.0), SVDECL(geom_zmax, 1.0);
SVEXTERN int SVDECL(use_geom_factors, 1), SVDECL(have_geom_factors, 0);
SVEXTERN int SVDECL(transparent_state,ALL_SOLID);

SVEXTERN int SVDECL(use_tload_begin, 0), SVDECL(use_tload_end, 0);
SVEXTERN float SVDECL(tload_begin2, 0.0), SVDECL(tload_end2, 0.0);
SVEXTERN int SVDECL(use_tload_begin2, 0), SVDECL(use_tload_end2, 0);
SVEXTERN int SVDECL(use_tload_skip, 0);

SVEXTERN int SVDECL(tload_zipskip, 0), SVDECL(tload_zipstep, 1);
SVEXTERN int SVDECL(tload_skip, 0), SVDECL(tload_step, 1);

SVEXTERN float SVDECL(frameinterval,1);

SVEXTERN int SVDECL(blockages_dirty,0);
SVEXTERN int SVDECL(canrestorelastview,0);
SVEXTERN int SVDECL(ntargets,0);

SVEXTERN int SVDECL(mainwindow_id,0);

SVEXTERN float SVDECL(max_mem_GB,0.0);
SVEXTERN int SVDECL(visUsagememory,0);
SVEXTERN float gslice_norm[3];
#ifdef INMAIN
SVEXTERN float glui_tour_view[3] = {0.0,0.0,0.0};
SVEXTERN float glui_tour_xyz[3]={0.0,0.0,0.0};
SVEXTERN float gslice_xyz[3]={-1000001.0,-1000001.0,-1000001.0};
SVEXTERN float gslice_normal_xyz[3]={0.0,0.0,1.0};
SVEXTERN float gslice_normal_azelev[2]={0.0,90.0};
#else
SVEXTERN float glui_tour_view[3];
SVEXTERN float glui_tour_xyz[3];
SVEXTERN float gslice_xyz[3];
SVEXTERN float gslice_normal_xyz[3];
SVEXTERN float gslice_normal_azelev[2];
#endif
SVEXTERN int SVDECL(glui_set_tour_time, 0);
SVEXTERN float SVDECL(glui_tour_time, 0.0);
SVEXTERN float SVDECL(glui_tour_pause_time, 0.0);

SVEXTERN float gslice_xyz0[3],gslice_normal_azelev0[2];
SVEXTERN int SVDECL(vis_gslice_data,0),SVDECL(SHOW_gslice_data,0),SVDECL(SHOW_gslice_data_old,0),SVDECL(show_gslice_triangles,0);
SVEXTERN int SVDECL(show_gslice_triangulation,0);
SVEXTERN int SVDECL(show_gslice_normal,0),SVDECL(show_gslice_normal_keyboard,0);


SVEXTERN float ventcolor_orig[4];
SVEXTERN float SVDECL(*ventcolor,NULL);
#ifdef INMAIN
SVEXTERN float static_color[4]={0.0,1.0,0.0,1.0};
SVEXTERN float sensorcolor[4]={1.0,1.0,0.0,1.0};
SVEXTERN float sensornormcolor[4]={1.0,1.0,0.0,1.0};
SVEXTERN float sprinkoncolor[4]={0.0,1.0,0.0,1.0};
SVEXTERN float sprinkoffcolor[4]={1.0,0.0,0.0,1.0};
SVEXTERN float heatoncolor[4]={1.0,0.0,0.0,1.0};
SVEXTERN float heatoffcolor[4]={1.0,0.0,0.0,1.0};
#else
SVEXTERN float static_color[4];
SVEXTERN float sensorcolor[4];
SVEXTERN float sensornormcolor[4];
SVEXTERN float sprinkoncolor[4];
SVEXTERN float sprinkoffcolor[4];
SVEXTERN float heatoncolor[4];
SVEXTERN float heatoffcolor[4];
#endif
#ifdef INMAIN
SVEXTERN float backgroundbasecolor[4]  = {0.0, 0.0, 0.0, 1.0};
SVEXTERN float backgroundcolor[4]      = {0.0, 0.0, 0.0, 1.0};
SVEXTERN float foregroundbasecolor[4]  = {1.0, 1.0, 1.0, 1.0};
SVEXTERN float foregroundcolor[4]      = {1.0, 1.0, 1.0, 1.0};
SVEXTERN int   glui_outlinecolor[4]    = {0, 255, 255, 255};
#else
SVEXTERN float backgroundbasecolor[4];
SVEXTERN float backgroundcolor[4];
SVEXTERN float foregroundbasecolor[4];
SVEXTERN float foregroundcolor[4];
SVEXTERN int   glui_outlinecolor[4];
#endif
SVEXTERN int glui_foregroundbasecolor[4];
SVEXTERN int glui_backgroundbasecolor[4];
#ifdef INMAIN
SVEXTERN float boundcolor[4]={0.5,0.5,0.5,1.0};
SVEXTERN float timebarcolor[4]={0.6,0.6,0.6,1.0};
SVEXTERN float redcolor[4]={1.0,0.0,0.0,1.0};
#else
SVEXTERN float boundcolor[4];
SVEXTERN float timebarcolor[4];
SVEXTERN float redcolor[4];
#endif

SVEXTERN int SVDECL(loadfiles_at_startup,0);
#define LOAD_3DCO2    1
#define LOAD_3DHRRPUV 2
#define LOAD_3DSOOT   3
#define LOAD_3DTEMP   4
#ifdef INMAIN
SVEXTERN int loadfiles_commandline[5]={0,0,0,0,0};
#else
SVEXTERN int loadfiles_commandline[5];
#endif

SVEXTERN int SVDECL(nmenus,0);
#define MAXMENUS 10000
SVEXTERN menudata menuinfo[MAXMENUS];
SVEXTERN int max_screenWidth, max_screenHeight;
SVEXTERN int saveW, saveH;
SVEXTERN char SVDECL(*colorbars_dir, NULL);
SVEXTERN char SVDECL(*colorbars_linear_dir,  NULL);
SVEXTERN char SVDECL(*colorbars_rainbow_dir, NULL);
SVEXTERN char SVDECL(*colorbars_divergent_dir, NULL);
SVEXTERN char SVDECL(*colorbars_circular_dir, NULL);
SVEXTERN int SVDECL(ndeprecated_filelist, 0);
SVEXTERN int SVDECL(nuser_filelist, 0);
SVEXTERN char release_title[1024];
SVEXTERN char plot3d_title[1024];
SVEXTERN char SVDECL(*partshortlabel,NULL),SVDECL(*partunitlabel,NULL);
SVEXTERN char emptylabel[2];

SVEXTERN int nopenvents,nopenvents_nonoutline,ntransparentblocks,ntransparentvents;
SVEXTERN int nventcolors;
SVEXTERN float SVDECL(**ventcolors,NULL);

SVEXTERN int vslicecolorbarflag;
SVEXTERN int SVDECL(blockage_draw_option, 1);
SVEXTERN int SVDECL(mesh_index_debug, 0);
SVEXTERN int SVDECL(min_blockage_index_debug, 0);
SVEXTERN int SVDECL(n_blockages_debug, 0);
#ifdef INMAIN
  SVEXTERN unsigned char rgb_below_min[3]={255-64,255-64,255-64}, rgb_above_max[3]={0,0,0};
#else
  SVEXTERN unsigned char rgb_below_min[3], rgb_above_max[3];
#endif
SVEXTERN int SVDECL(colorbar_select_index,-1),SVDECL(update_colorbar_select_index,0);
SVEXTERN float fds_eyepos[3],smv_eyepos[3],fds_viewdir[3],smv_viewpos[3];
SVEXTERN int SVDECL(tour_usecurrent,0);
SVEXTERN int SVDECL(output_slicedata,0),SVDECL(output_patchdata,0);
SVEXTERN f_units SVDECL(*unitclasses,NULL),SVDECL(*unitclasses_default,NULL),SVDECL(*unitclasses_ini,NULL);
SVEXTERN int SVDECL(nunitclasses,0),SVDECL(nunitclasses_default,0),SVDECL(nunitclasses_ini,0);
#ifdef INMAIN
SVEXTERN smv_case global_scase = {.tourcoll = {.ntourinfo = 0,
                                        .tourinfo = NULL,
                                        .tour_ntimes = 1000,
                                        .tour_t = NULL,
                                        .tour_t2 = NULL,
                                        .tour_dist = NULL,
                                        .tour_dist2 = NULL,
                                        .tour_dist3 = NULL,
                                        .tour_tstart = 0.0,
                                        .tour_tstop = 100.0},
                           .fuel_hoc = -1.0,
                           .fuel_hoc_default = -1.0,
                           .have_cface_normals = CFACE_NORMALS_NO,
                           .gvecphys = {0.0, 0.0, -9.8},
                           .gvecunit = {0.0, 0.0, -1.0},
                           .global_tbegin = 1.0,
                           .global_tend = 0.0,
                           .tload_begin = 0.0,
                           .tload_end = 0.0,
                           .load_hrrpuv_cutoff = 200.0,
                           .global_hrrpuv_cutoff = 200.0,
                           .global_hrrpuv_cutoff_default = 200.0,
                           .smoke_albedo = 0.3,
                           .smoke_albedo_base = 0.3,
                           .xbar = 1.0,
                           .ybar = 1.0,
                           .zbar = 1.0,
                           .show_slice_in_obst = ONLY_IN_GAS,
                           .use_iblank = 1,
                           .visOtherVents = 1,
                           .visOtherVentsSAVE = 1,
                           .hvac_duct_color = {63, 0, 15},
                           .hvac_node_color = {63, 0, 15},
                           .nrgb2 = 8,
                           .pref = 101325.0,
                           .pamb = 0.0,
                           .tamb = 293.15,
                           .nrgb = NRGB,
                           .linewidth = 2.0,
                           .ventlinewidth = 2.0,
                           .obst_bounding_box = {1.0,0.0,1.0,0.0,1.0,0.0},
                           .hvaccoll = {
                              .hvacductvar_index= -1,
                              .hvacnodevar_index= -1,
                              0
                            }
                          };
parse_options parse_opts = {
    .smoke3d_only = 0,
    .setup_only = 0,
    .fast_startup = 1,
    .lookfor_compressed_files = 0,
    .handle_slice_files = 1
};
#else
SVEXTERN smv_case global_scase;
SVEXTERN parse_options parse_opts;
#endif
SVEXTERN meshdata SVDECL(*current_mesh,NULL), SVDECL(*mesh_save,NULL);
SVEXTERN meshdata SVDECL(*mesh_last,NULL), SVDECL(*loaded_isomesh,NULL);
SVEXTERN float SVDECL(devicenorm_length,0.1);
SVEXTERN float max_dev_vel;
SVEXTERN int SVDECL(last_prop_display,-1);
SVEXTERN int SVDECL(devicetypes_index,0);

SVEXTERN float SVDECL(plot2d_hrr_min,0.0), SVDECL(plot2d_hrr_max,1.0);

SVEXTERN int SVDECL(have_ext, 0);
SVEXTERN int SVDECL(ntreedeviceinfo, 0), SVDECL(mintreesize, 3);
SVEXTERN int SVDECL(nzwindtreeinfo, 0);
SVEXTERN treedevicedata SVDECL(*treedeviceinfo,NULL);
SVEXTERN treedevicedata SVDECL(**zwindtreeinfo, NULL);

SVEXTERN int SVDECL(show_smokesensors,SMOKESENSORS_0255),SVDECL(active_smokesensors,0),SVDECL(test_smokesensors,0);
SVEXTERN float SVDECL(smoke3d_cvis,1.0);
SVEXTERN std_objects SVDECL(std_object_defs,{0});
SVEXTERN float treecolor[4];
#ifdef INMAIN
SVEXTERN float treecharcolor[4]={0.3,0.3,0.3,1.0};
SVEXTERN float trunccolor[4]={0.6,0.2,0.0,1.0};
#else
SVEXTERN float treecharcolor[4];
SVEXTERN float trunccolor[4];
#endif
SVEXTERN unsigned char treecolor_uc[4], treecharcolor_uc[4], trunccolor_uc[4];
SVEXTERN float rgb_terrain[10][4];
SVEXTERN keyframe SVDECL(**tourknotskeylist,NULL);
SVEXTERN tourdata SVDECL(**tourknotstourlist,NULL);
SVEXTERN keyframe SVDECL(*selected_frame,NULL);
SVEXTERN tourdata SVDECL(*selected_tour,NULL);
SVEXTERN int SVDECL(callfrom_tourglui,0);
SVEXTERN int SVDECL(showtours_whenediting,0);

SVEXTERN float xtimeleft, xtimeright;

SVEXTERN int SVDECL(stereoactive,0);
SVEXTERN int SVDECL(stereotype,STEREO_NONE), SVDECL(stereotypeOLD, STEREO_NONE);
SVEXTERN int SVDECL(show_parallax,0), SVDECL(stereotype_frame, BOTH_EYES);

SVEXTERN int SVDECL(show_firecutoff,0),SVDECL(hrrpuv_loaded,0);
SVEXTERN int SVDECL(temp_loaded,0);
SVEXTERN int SVDECL(trainerview,1);
SVEXTERN int SVDECL(apertureindex,1);
SVEXTERN int SVDECL(projection_type, PROJECTION_PERSPECTIVE);
#ifdef INMAIN
SVEXTERN float apertures[5]={30.0,45.0,60.0,75.0,90.0};
#else
SVEXTERN float apertures[5];
#endif
SVEXTERN float SVDECL(zoom_min, 0.1), SVDECL(zoom_max, 10.0);
SVEXTERN float aperture_min, aperture_max;
SVEXTERN float aperture,aperture_glui,aperture_default,aperture_glui0;
#define MAX_ZOOMS 6
#define ZOOMINDEX_ONE 2
SVEXTERN int SVDECL(zoomindex, ZOOMINDEX_ONE);
#ifdef INMAIN
SVEXTERN float zooms[MAX_ZOOMS+1]={0.25,0.5,1.0,2.0,4.0,10.0,-1.0};
#else
SVEXTERN float zooms[MAX_ZOOMS+1];
#endif
SVEXTERN float SVDECL(zoom,1.0),SVDECL(zoomini,1.0);
SVEXTERN int rgbmask[16];
SVEXTERN GLint nredbits, ngreenbits, nbluebits;
SVEXTERN int nredshift, ngreenshift, nblueshift;
SVEXTERN float xyzbox;
SVEXTERN float xplts[256*256], yplts[256*256], zplts[256*256];
SVEXTERN float SVDECL(*targtimes,NULL);
SVEXTERN int SVDECL(*targtimeslist,NULL);
SVEXTERN int SVDECL(*zone_timeslist,NULL);
SVEXTERN int delete_view_is_disabled;
SVEXTERN int old_listview;

SVEXTERN int SVDECL(titlesafe_offset,0);
SVEXTERN int SVDECL(titlesafe_offsetBASE,45);
SVEXTERN int   SVDECL(reset_frame,0);
SVEXTERN float SVDECL(reset_time,0.0),SVDECL(start_frametime,0.0),SVDECL(stop_frametime,0.0);
SVEXTERN float SVDECL(max_velocity,0.0);
SVEXTERN int nslice_loaded, ngeomslice_loaded, nvolsmoke_loaded;
SVEXTERN int SVDECL(*slice_loaded_list,NULL), SVDECL(*slice_sorted_loaded_list,NULL);
SVEXTERN char SVDECL(*fdsprefix2,NULL);
SVEXTERN char SVDECL(*endian_filename,NULL);
SVEXTERN char SVDECL(*target_filename,NULL);

SVEXTERN int SVDECL(update_bounds,0);
SVEXTERN FILE SVDECL(*STREAM_SB,NULL);
SVEXTERN float SVDECL(temp_threshold,400.0);
SVEXTERN char SVDECL(*smv_filename,NULL);
SVEXTERN int  SVDECL(have_multislice, 0), SVDECL(have_multivslice, 0);
SVEXTERN char SVDECL(*part_globalbound_filename, NULL);
SVEXTERN char SVDECL(*smokeview_casedir, NULL);
SVEXTERN int SVDECL(update_vectorskip, 0);
SVEXTERN int SVDECL(smoke_offaxis, 0), SVDECL(smoke_adjust, 1);
SVEXTERN scriptfiledata first_scriptfile, last_scriptfile, SVDECL(*default_script,NULL);
SVEXTERN scriptdata SVDECL(*scriptinfo,NULL), SVDECL(*current_script_command,NULL);
SVEXTERN char SVDECL(*script_dir_path,NULL), SVDECL(*script_htmldir_path, NULL);
SVEXTERN int SVDECL(nscriptinfo,0);
SVEXTERN scriptfiledata SVDECL(*script_recording,NULL);
SVEXTERN int SVDECL(runscript,0), SVDECL(noexit,0);
SVEXTERN int SVDECL(runhtmlscript, 0);
#ifdef INMAIN
SVEXTERN float slice_xyz[3]={0.0,0.0,0.0}, slice_dxyz[3] = {0.0, 0.0, 0.0};
#else
SVEXTERN float slice_xyz[3], slice_dxyz[3];
#endif
SVEXTERN int   SVDECL(update_slice2device, 0);
SVEXTERN int SVDECL(script_multislice,0), SVDECL(script_multivslice,0), SVDECL(script_iso,0);
SVEXTERN FILE SVDECL(*scriptoutstream,NULL);
SVEXTERN FILE SVDECL(*LOG_FILENAME,NULL);
SVEXTERN char SVDECL(*flushfile,NULL);
SVEXTERN int SVDECL(csv_loaded, 0), SVDECL(devices_setup,0),SVDECL(update_csv_load,0);
SVEXTERN char SVDECL(*smokezippath,NULL), SVDECL(*smokeviewpath,NULL), SVDECL(*fdsprog, NULL);
SVEXTERN char SVDECL(*INI_fds_filein,NULL);
#ifdef pp_FRAME
SVEXTERN char SVDECL(*frametest_filename, NULL);
#endif
SVEXTERN int SVDECL(event_file_exists,0);
SVEXTERN char SVDECL(*zonelonglabels,NULL), SVDECL(*zoneshortlabels,NULL), SVDECL(*zoneunits,NULL);
SVEXTERN int SVDECL(overwrite_all,0),SVDECL(erase_all,0);
SVEXTERN int SVDECL(compress_autoloaded,0);
SVEXTERN tridata SVDECL(**opaque_triangles,NULL),SVDECL(**transparent_triangles,NULL),SVDECL(**alltriangles,NULL);
SVEXTERN int SVDECL(nopaque_triangles,0),SVDECL(ntransparent_triangles,0),SVDECL(nalltriangles,0);
#ifdef WIN32
SVEXTERN   char openfilebuffer[1024];
SVEXTERN   int openfileflag;
#endif
SVEXTERN float xyzmaxdiff;
SVEXTERN char ext_png[5];
SVEXTERN char ext_jpg[5];

SVEXTERN int SVDECL(updatehiddenfaces,1),SVDECL(hide_overlaps,0);
SVEXTERN int key_state;
SVEXTERN float starteyex, starteyey;
SVEXTERN float eye_xyz0[3];
#ifdef INMAIN
SVEXTERN float start_xyz0[3]={0.0,0.0,0.0};
#else
SVEXTERN float start_xyz0[3];
#endif
SVEXTERN int SVDECL(glui_move_mode,-1);

SVEXTERN float SVDECL(timeoffset,0.0);
SVEXTERN int npartpoints, npartframes, SVDECL(partdrawskip, 1);
SVEXTERN float xslicemid, yslicemid, zslicemid;
SVEXTERN float delx;
SVEXTERN float delz;
SVEXTERN float d_eye_xyz[3],dsave_eye_xyz[3];
SVEXTERN float eyex0, eyey0, eyez0;
SVEXTERN float viewx, viewy, viewz;
SVEXTERN float anglexy0,azimuth0;
SVEXTERN int mouse_down_xy0[2];
SVEXTERN int touring;
SVEXTERN int SVDECL(update_tour_list,0);
SVEXTERN float SVDECL(desired_view_height,1.5);
SVEXTERN int thistime, lasttime, SVDECL(resetclock,1),SVDECL(initialtime,0);
SVEXTERN int SVDECL(realtime_flag,0);
SVEXTERN char timelabel[30];
SVEXTERN char frameratelabel[30];
SVEXTERN char framelabel[30];
SVEXTERN float SVDECL(**p3levels,NULL), SVDECL(*zonelevels,NULL);
SVEXTERN float SVDECL(**p3levels256,NULL);
SVEXTERN char SVDECL(***colorlabelp3,NULL),SVDECL(***colorlabeliso,NULL);
SVEXTERN int SVDECL(slicefile_labelindex,-1),SVDECL(slicefile_labelindex_save,-1),SVDECL(iboundarytype,-1);
SVEXTERN int SVDECL(iisotype,-1),iisottype;
SVEXTERN char SVDECL(**colorlabelpart,NULL), SVDECL(**colorlabelpatch,NULL);
SVEXTERN float colorvaluespatch[12], colorvalueszone[12], colorvaluesp3[MAXPLOT3DVARS][12];
SVEXTERN int SVDECL(hilight_skinny,0);

SVEXTERN int minfill, maxfill;

SVEXTERN int SVDECL(*plotiso,NULL);

SVEXTERN int nglobal_times, SVDECL(itimes,0), SVDECL(itime_save,-1), SVDECL(itimeold,-999);
SVEXTERN float SVDECL(*global_times,NULL), SVDECL(*times_buffer, NULL), cputimes[20];
SVEXTERN int SVDECL(ntimes_buffer, 0);

SVEXTERN int SVDECL(cpuframe,0);

SVEXTERN float eye_position_smv[3],xeyedir[3], yeyedir[3], zeyedir[3];
//#ifdef INMAIN
//  SVEXTERN float eyzeyeorig_OLD[3]={-1.000000, -1.000000, -1.000000};
// #else
//  SVEXTERN float eyzeyeorig_OLD[3];
//#endif
SVEXTERN int SVDECL(colorbar_selection_width,5);
SVEXTERN int SVDECL(have_extreme_mindata,0), SVDECL(have_extreme_maxdata,0);
SVEXTERN int SVDECL(show_extreme_mindata,0), SVDECL(show_extreme_maxdata,0);
SVEXTERN int SVDECL(show_extreme_mindata_save,0), SVDECL(show_extreme_maxdata_save,0);

SVEXTERN int SVDECL(update_make_iblank, 0);

SVEXTERN int script_index, ini_index;
SVEXTERN char script_inifile_suffix[1024], vol_prefix[1024];
SVEXTERN char script_renderdir[1024], script_renderfilesuffix[1024], script_renderfile[1024];
SVEXTERN char SVDECL(*script_renderdir_cmd, NULL);
SVEXTERN inifiledata first_inifile, last_inifile;
SVEXTERN char script_filename[1024];
SVEXTERN int SVDECL(highlight_block,-1), SVDECL(highlight_mesh,0), SVDECL(highlight_flag,2);
SVEXTERN int SVDECL(updategetobstlabels,1);

SVEXTERN int smoke_alpha;
SVEXTERN int SVDECL(showall_textures,0);
SVEXTERN int SVDECL(enable_texture_lighting,0);

#ifdef INMAIN
SVEXTERN colorbar_collection colorbars = {.split_colorbar_index = -1, 0};
#else
SVEXTERN colorbar_collection colorbars;
#endif
SVEXTERN colorbardata SVDECL(*current_colorbar,NULL);
SVEXTERN colorbardata SVDECL(*colorbarcopyinfo, NULL);

SVEXTERN int SVDECL(ncolortableinfo, 0);
SVEXTERN colortabledata SVDECL(*colortableinfo, NULL);
SVEXTERN int SVDECL(i_colortable_list,0);
SVEXTERN int SVDECL(toggle_on, 0);
SVEXTERN int SVDECL(update_load_files, 0);
SVEXTERN int SVDECL(do_threshold,0);
SVEXTERN int SVDECL(show_path_knots,0);
SVEXTERN int SVDECL(show_avatar,1);
SVEXTERN int SVDECL(tourlocus_type,0);
SVEXTERN int SVDECL(glui_avatar_index,0);
SVEXTERN int SVDECL(device_sphere_segments,6);
SVEXTERN int SVDECL(nlat_hsphere, 20), SVDECL(nlong_hsphere, 40);
SVEXTERN int ntexturestack;

SVEXTERN float SVDECL(fire_opacity_factor,3.0),SVDECL(mass_extinct,8700.0);
SVEXTERN float SVDECL(global_temp_min,20.0),SVDECL(global_temp_max,2000.0);
SVEXTERN float SVDECL(global_temp_cutoff, 600.0), SVDECL(global_temp_cutoff_default, 600.0);
SVEXTERN float SVDECL(global_hrrpuv_min,0.0),SVDECL(global_hrrpuv_max,1200.0);
SVEXTERN int SVDECL(volbw,0);
SVEXTERN float SVDECL(tourrad_avatar,0.1);
SVEXTERN int SVDECL(dirtycircletour,0);

SVEXTERN int SVDECL(selectedtour_index, TOURINDEX_MANUAL), SVDECL(selectedtour_index_old, TOURINDEX_MANUAL), SVDECL(selectedtour_index_ini, TOURINDEX_MANUAL);
SVEXTERN int SVDECL(update_selectedtour_index,0);
SVEXTERN int SVDECL(viewtourfrompath,0),SVDECL(viewalltours,0),SVDECL(viewanytours,0),SVDECL(edittour,0);
#ifdef _DEBUG
SVEXTERN int SVDECL(showdebugtour, 1);
#endif
SVEXTERN int SVDECL(tour_constant_velocity, 1);

SVEXTERN int SVDECL(animate_blockages, 0);
SVEXTERN selectdata SVDECL(*selectfaceinfo,NULL);
SVEXTERN blockagedata SVDECL(**selectblockinfo,NULL);
SVEXTERN int SVDECL(visFDSticks,0);
SVEXTERN float user_tick_origin[3], user_tick_max[3], user_tick_min[3], user_tick_step[3], user_tick_length, user_tick_width;
SVEXTERN float user_tick_direction;
SVEXTERN int glui_tick_inside, glui_tick_outside;
SVEXTERN int user_tick_nxyz[3], user_tick_sub, user_tick_option, SVDECL(visUSERticks,0), SVDECL(auto_user_tick_placement,1);
SVEXTERN int SVDECL(user_tick_show_x,1), SVDECL(user_tick_show_y,1), SVDECL(user_tick_show_z,1);
SVEXTERN int SVDECL(visCadTextures,1), SVDECL(visTerrainTexture,1);
SVEXTERN int SVDECL(viscolorbarpath,0);
SVEXTERN int SVDECL(*sortedblocklist,NULL),SVDECL(*changed_idlist,NULL),SVDECL(nchanged_idlist,0);
SVEXTERN int SVDECL(nselectblocks,0);
#ifdef INMAIN
SVEXTERN int surface_indices[7]={0,0,0,0,0,0};
#else
SVEXTERN int surface_indices[7];
#endif
SVEXTERN int surface_indices_bak[7];
SVEXTERN int SVDECL(wall_case,0);
SVEXTERN int ntotalfaces;
SVEXTERN texturedata SVDECL(*textureinfo,NULL), SVDECL(*terrain_textures,NULL);

SVEXTERN int SVDECL(visSkysphere, 0), SVDECL(visSkybox, 1), SVDECL(visSkySpheretexture, 1);
SVEXTERN float box_sky_corners[8][3];

SVEXTERN texturedata SVDECL(*sky_texture, NULL);
SVEXTERN int SVDECL(nsky_texture, 0);
SVEXTERN float SVDECL(sky_diam, 4.0);

SVEXTERN GLuint texture_colorbar_id, texture_slice_colorbar_id, texture_patch_colorbar_id, texture_plot3d_colorbar_id, texture_iso_colorbar_id, terrain_colorbar_id;
SVEXTERN GLuint volsmoke_colormap_id,slice3d_colormap_id,slicesmoke_colormap_id;
SVEXTERN int SVDECL(volsmoke_colormap_id_defined,-1);
SVEXTERN int SVDECL(slice3d_colormap_id_defined,-1);
SVEXTERN int SVDECL(slicesmoke_colormap_id_defined, -1);
#ifdef INMAIN
SVEXTERN float mscale[3]={1.0,1.0,1.0};
#else
SVEXTERN float mscale[3];
#endif
SVEXTERN float xclip_min, yclip_min, zclip_min;
SVEXTERN float xclip_max, yclip_max, zclip_max;
SVEXTERN float SVDECL(nearclip, 0.001), SVDECL(farclip, 3.0), SVDECL(farclip_save, 3.0);
SVEXTERN int SVDECL(updateclipvals, 0);
SVEXTERN int SVDECL(updateUpdateFrameRateMenu,0);
SVEXTERN int ntextures_loaded_used, SVDECL(iterrain_textures,0);
SVEXTERN int SVDECL(nskyboxinfo,0);
SVEXTERN skyboxdata SVDECL(*skyboxinfo,NULL);
SVEXTERN zonedata SVDECL(*activezone,NULL);
SVEXTERN int SVDECL(update_screensize,0);
SVEXTERN int SVDECL(part5show,1);
SVEXTERN int SVDECL(streak5show,0),streak5value, SVDECL(streak5step,0), SVDECL(showstreakhead,1);
SVEXTERN int SVDECL(nstreak_rvalue,8);
#ifdef INMAIN
SVEXTERN float streak_rvalue[8]={0.25,0.5,1.0,2.0,4.0,8.0,16.0,32.0};
#else
SVEXTERN float streak_rvalue[8];
#endif
SVEXTERN int SVDECL(streak_index,-1), SVDECL(update_streaks,0);
SVEXTERN float SVDECL(float_streak5value,0.0);;
SVEXTERN partpropdata SVDECL(*part5propinfo,NULL), SVDECL(*current_property,NULL);
SVEXTERN int SVDECL(npart5prop,0),ipart5prop,ipart5prop_old;
SVEXTERN int SVDECL(global_prop_index,-1);

SVEXTERN splitslicedata SVDECL(*splitsliceinfo, NULL), SVDECL(**splitsliceinfoptr, NULL);
SVEXTERN int SVDECL(nsplitsliceinfo, 0), SVDECL(nsplitsliceinfoMAX, 0);
SVEXTERN int SVDECL(sortslices, 1), SVDECL(sortslices_debug, 0);
SVEXTERN slicedata SVDECL(**slicex, NULL), SVDECL(**slicey, NULL), SVDECL(**slicez, NULL);

SVEXTERN fileboundsdata SVDECL(*sliceboundsinfo, NULL), SVDECL(*patchboundsinfo, NULL);
SVEXTERN int SVDECL(nsliceboundsinfo, 0), SVDECL(npatchboundsinfo, 0);
SVEXTERN camdata SVDECL(*caminfo,NULL);
SVEXTERN int SVDECL(*partorderindex,NULL);
SVEXTERN int SVDECL(*patchorderindex,NULL),SVDECL(*isoorderindex,NULL),SVDECL(*plot3dorderindex,NULL);
SVEXTERN int SVDECL(showfiles,0);
SVEXTERN cpp_boundsdata SVDECL(*slicebounds_cpp, NULL), SVDECL(*partbounds_cpp, NULL), SVDECL(*patchbounds_cpp, NULL), SVDECL(*plot3dbounds_cpp, NULL);
SVEXTERN int SVDECL(nslicebounds_cpp, 0), SVDECL(npartbounds_cpp, 0), SVDECL(npatchbounds_cpp, 0), SVDECL(nplot3dbounds_cpp, 0);
SVEXTERN int SVDECL(update_glui_bounds, 0), SVDECL(update_ini, 0), SVDECL(update_chop_colors,0);
SVEXTERN cpp_boundsdata SVDECL(*hvacductbounds_cpp, NULL), SVDECL(*hvacnodebounds_cpp, NULL);
SVEXTERN int SVDECL(nhvacductbounds_cpp, 0), SVDECL(nhvacnodebounds_cpp, 0);
SVEXTERN boundsdata SVDECL(*slicebounds,NULL), SVDECL(*isobounds,NULL), SVDECL(*patchbounds,NULL);
SVEXTERN boundsdata SVDECL(*slicebounds_temp, NULL);
SVEXTERN boundsdata SVDECL(*hvacductbounds, NULL), SVDECL(*hvacnodebounds, NULL);
SVEXTERN int SVDECL(nhvacductbounds, 0), SVDECL(nhvacnodebounds, 0);
SVEXTERN int SVDECL(hvac_maxcells, 0), SVDECL(hvac_n_ducts, 0);

SVEXTERN int force_redisplay;
SVEXTERN int glui_setp3min, glui_setp3max;
SVEXTERN int setp3chopmin_temp, setp3chopmax_temp;
SVEXTERN float p3chopmin_temp, p3chopmax_temp;
SVEXTERN float glui_p3min, glui_p3max;

SVEXTERN int SVDECL(bound_slice_init, 1);
SVEXTERN int SVDECL(bound_part_init, 1);
SVEXTERN int SVDECL(bound_hvacduct_init, 1);
SVEXTERN int SVDECL(bound_hvacnode_init, 1);
SVEXTERN int SVDECL(bound_patch_init, 1);

#ifdef INMAIN
int fire_color_int255[3]  = { 255, 128,0 };
int smoke_color_int255[4] = { 0,0,0,0 };
int co2_color_int255[3]   = {64,156,215};
#else
SVEXTERN int fire_color_int255[3];
SVEXTERN int smoke_color_int255[4];
SVEXTERN int co2_color_int255[3];
#endif

SVEXTERN int SVDECL(use_opacity_depth, 1), SVDECL(use_opacity_depth_ini,-1);
SVEXTERN int SVDECL(use_opacity_multiplier, 0), SVDECL(use_opacity_multiplier_ini, -1);
SVEXTERN int SVDECL(use_opacity_ini, 0);

SVEXTERN int SVDECL(update_smokefire_colors, 0);
SVEXTERN float SVDECL(fire_halfdepth,0.3);
SVEXTERN float SVDECL(co2_halfdepth, 10.0);

SVEXTERN int SVDECL(co2_colormap_type, CO2_COLORBAR);

SVEXTERN int SVDECL(show_firecolormap,0);
SVEXTERN int SVDECL(fire_colormap_type, FIRECOLORMAP_CONSTRAINT);
SVEXTERN int SVDECL(fire_colormap_type_save, FIRECOLORMAP_CONSTRAINT);

SVEXTERN int SVDECL(use_smoke_colormap, 0), SVDECL(use_smoke_rgb, 1);
SVEXTERN int SVDECL(use_fire_colormap, 0),  SVDECL(use_fire_rgb, 1);
SVEXTERN int SVDECL(use_co2_colormap, 0),   SVDECL(use_co2_rgb, 1);

SVEXTERN int SVDECL(smokecullflag,1);
SVEXTERN int SVDECL(visMAINmenus,0);
SVEXTERN int SVDECL(ijkbarmax,5);
SVEXTERN int SVDECL(blockage_as_input,0), SVDECL(blockage_snapped,1);
SVEXTERN int SVDECL(show_cad_and_grid,0);
SVEXTERN int SVDECL(iplot3dtimelist, -1), SVDECL(nplot3dtimelist, 0);
SVEXTERN float SVDECL(*plot3dtimelist,NULL);

SVEXTERN blockagedata SVDECL(*bchighlight,NULL),SVDECL(*bchighlight_old,NULL);

SVEXTERN int SVDECL(buffertype,DOUBLE_BUFFER);
SVEXTERN int SVDECL(opengldefined,0);
SVEXTERN int SVDECL(restart_time,0);
SVEXTERN int SVDECL(*isosubmenus,NULL), nisosubmenus;
SVEXTERN int SVDECL(*loadpatchsubmenus,NULL), nloadpatchsubmenus;
SVEXTERN char SVDECL(**patchlabellist,NULL);
SVEXTERN char SVDECL(*socket_path,NULL);
SVEXTERN int SVDECL(*patchlabellist_index,NULL);
SVEXTERN int SVDECL(*isoindex,NULL);

SVEXTERN int have_vents_int;
SVEXTERN int nface_outlines, nface_textures, nface_transparent;
SVEXTERN int nface_normals_single, nface_normals_double, nface_transparent_double;
SVEXTERN int SVDECL(show_transparent_vents,1);
SVEXTERN int SVDECL(show_bothsides_blockages, 0);
SVEXTERN int SVDECL(show_bothsides_int,1), SVDECL(show_bothsides_ext,0);
SVEXTERN float SVDECL(transparency_geom,0.2);
SVEXTERN int SVDECL(use_transparency_geom,0);
SVEXTERN facedata SVDECL(**face_transparent,NULL);
#ifdef pp_PATCH_HIDE
SVEXTERN int SVDECL(hidepatchsurface,0);
#endif

#ifdef INMAIN
SVEXTERN float meshclip[6] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
SVEXTERN float meshclip_save[6] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
SVEXTERN int use_meshclip[6] = {0, 0, 0, 0, 0, 0};
#else
SVEXTERN float meshclip[6];
SVEXTERN float meshclip_save[6];
SVEXTERN int use_meshclip[6];
#endif
SVEXTERN int SVDECL(show_intersection_box, 0);
SVEXTERN int SVDECL(show_intersected_meshes, 0);
SVEXTERN int SVDECL(update_meshclip, 0);
SVEXTERN int SVDECL(glui_meshclip_defined, 0);
SVEXTERN int SVDECL(set_mesh, 0);
SVEXTERN int SVDECL(load_only_when_unloaded, 0);
SVEXTERN int SVDECL(show_mesh_labels, 0);

#ifdef INMAIN
  SVEXTERN float rgb_baseBASE[MAXRGB][4]=
{
  {0.000000, 0.000000, 1.000000},
  {0.000000, 0.281732, 0.959493},
  {0.000000, 0.540640, 0.841254},
  {0.000000, 0.755749, 0.654861},
  {0.000000, 0.909632, 0.415416},
  {0.000000, 0.989821, 0.142316},
  {0.142316, 0.989821, 0.000000},
  {0.415416, 0.909632, 0.000000},
  {0.654861, 0.755749, 0.000000},
  {0.841254, 0.540640, 0.000000},
  {0.959493, 0.281732, 0.000000},
  {1.000000, 0.000000, 0.000000}
};
  SVEXTERN float bw_baseBASE[MAXRGB][4]={
  {1,            1,                1},
  {0.909090909,  0.909090909,      0.909090909},
  {0.818181818,  0.818181818,      0.818181818},
  {0.727272727,  0.727272727,      0.727272727},
  {0.636363636,  0.636363636,      0.636363636},
  {0.545454545,  0.545454545,      0.545454545},
  {0.454545455,  0.454545455,      0.454545455},
  {0.363636364,  0.363636364,      0.363636364},
  {0.272727273,  0.272727273,      0.272727273},
  {0.181818182,  0.181818182,      0.181818182},
  {0.090909091,  0.090909091,      0.090909091},
  {  0,            0,                0}
};
  SVEXTERN float rgb2BASE[MAXRGB][3]={
  {1.0f, 1.0f, 1.0f}, /* white */
  {1.0f, 1.0f, 0.0f}, /* yellow */
  {0.0f, 0.0f, 1.0f}, /* blue */
  {1.0f, 0.0f, 0.0f}, /* red */
  {0.0f, 1.0f, 0.0f}, /* green */
  {1.0f, 0.0f, 1.0f}, /* magenta */
  {0.0f, 1.0f, 1.0f}, /* cyan */
  {0.0f, 0.0f, 0.0f}  /* black */
  };

  SVEXTERN float rgbhazard[MAXRGB][4]={
  {0.0f, 0.0f, 1.0f,1.0}, /* blue */
  {0.0f, 1.0f, 0.0f,1.0}, /* green */
  {1.0f, 1.0f, 0.0f,1.0}, /* yellow */
  {1.0f, 0.0f, 1.0f,1.0}, /* magenta */
  {1.0f, 0.0f, 0.0f,1.0}  /* red */
  };

#else
  SVEXTERN float rgb_baseBASE[MAXRGB][4];
  SVEXTERN float bw_baseBASE[MAXRGB][4];
  SVEXTERN float rgb2BASE[MAXRGB][3];
  SVEXTERN float rgbhazard[MAXRGB][4];
#endif
#endif
