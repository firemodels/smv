#ifndef SMOKEVIEWDEFS_H_DEFINED
#define SMOKEVIEWDEFS_H_DEFINED
#ifdef pp_SNIFF_ERROR
EXTERNCPP void _Sniff_Errors(const char *whereat, const char *file, int line);
#define SNIFF_ERRORS(f) _Sniff_Errors(f,__FILE__,__LINE__)
#else
#define SNIFF_ERRORS(f)
#endif

#define TOA_LIMIT 9.9E5 // time of arrival limit - for wui cases

#define SLICE_LOAD_SPECIFIED    0
#define SLICE_LOADALL_XorYorZ   1
#define SLICE_LOADALL_XandYandZ 2

#define VECLENGTH 0.1

#define BOUND_UPDATE_COLORS       110
#define BOUND_DONTUPDATE_COLORS   128
#define BOUND_COMPUTE_PERCENTILES 116

#define MENU_HVAC_LOAD     0
#define MENU_HVAC_UNLOAD   1

#define SPLIT_COLORBAR         1

#define LABELS_vcolorbar 34
#define LABELS_hcolorbar 35
#define LABELS_shade      5
#define LABELS_shadedata 30
#define LABELS_colorbar_shift 36


#define OBJECT_PLOT_SHOW_ALL        -8
#define OBJECT_PLOT_SHOW_SELECTED  -11
#define OBJECT_PLOT_SHOW_TREE_ALL  -12


#define PLOT_HRRPUV           -13

#define DEVICE_HRRPUV_PLOT            30
#define DEVICE_HRRPUV2_PLOT           31
#define DEVICE_sensorsize             20
#define DEVICE_SHOWDEVICEVALS         26
#define DEVICE_SHOWDEVICEPLOT         29
#define DEVICE_COLORDEVICEVALS        27
#define DEVICE_SAVE_SETTINGS_OBJECTS  99
#define DEVICE_close                   3
#define DEVICE_show_orientation        4
#define DEVICE_NBUCKETS                5
#define DEVICE_SHOWBEAM                6
#define DEVICE_RADIUS                  7
#define DEVICE_TIMEAVERAGE            32

#ifdef pp_OSX_HIGHRES
#define GLUT_BITMAP_HELVETICA_20	(&glutBitmapHelvetica20)
#define GLUT_BITMAP_HELVETICA_24	(&glutBitmapHelvetica24)
#define GLUT_BITMAP_HELVETICA_36	(&glutBitmapHelvetica36)
#endif

#define MENU_VIEW_XMIN            -109
#define MENU_VIEW_XMAX            -110
#define MENU_VIEW_YMIN            -111
#define MENU_VIEW_YMAX            -112
#define MENU_VIEW_ZMIN            -113
#define MENU_VIEW_ZMAX            -114
#define SAVE_CURRENT_VIEWPOINT    -115

#define VIEW_XMIN                    0
#define VIEW_XMAX                   -1
#define VIEW_YMIN                   -2
#define VIEW_YMAX                   -3
#define VIEW_ZMIN                   -4
#define VIEW_ZMAX                   -5
#define XYZ_CENTER                 -6

#define DEVICE_devicetypes     28

#define DEVICE_PLOT_HIDDEN        0
#define DEVICE_PLOT_SHOW_SELECTED 1
#define DEVICE_PLOT_SHOW_ALL      2
#define DEVICE_PLOT_SHOW_TREE_ALL 3
#define PLOT_ONLY_DATA  0
#define PLOT_ALL        1
#define PLOT_ONLY_FRAME 2


#define    ALL_FILES 0
#define LOADED_FILES 1

#define BOUND_SET_MAX        0
#define BOUND_LOADED_MAX     1
#define BOUND_GLOBAL_MAX     2
#define BOUND_PERCENTILE_MAX 3

#define BOUND_SET_MIN        0
#define BOUND_LOADED_MIN     1
#define BOUND_GLOBAL_MIN     2
#define BOUND_PERCENTILE_MIN 3

#define BOUND_PART     0
#define BOUND_SLICE    1
#define BOUND_PLOT3D   2
#define BOUND_PATCH    3
#define BOUND_HVACDUCT 4
#define BOUND_HVACNODE 5

#define MINMAX_ALL    0
#define MINMAX_LOADED 1
#define MINMAX_INI    2

#define CFACE_NORMALS_NO  0
#define CFACE_NORMALS_YES 1

#define ALL_FRAMES       -1

#define COLORBAR_SHIFT_MIN          0.1
#define COLORBAR_SHIFT_MAX         10.0

#define FORCE_FIXEDPOINT_YES          1
#define FORCE_FIXEDPOINT_NO           0
#define GRIDLOC_NDECIMALS_MIN         2
#define GRIDLOC_NDECIMALS_MAX         8
#define COLORBAR_NDECIMALS_MIN        2
#define COLORBAR_NDECIMALS_MAX        8

#define COLORBAR_SELECTION_WIDTH_MIN  1
#define COLORBAR_SELECTION_WIDTH_MAX 10

#define CB_DELETE               1
#define CB_KEEP                 0
#define COLORBAR_TOGGLE              28
#define COLORBAR_LISTA               30
#define COLORBAR_LISTB               31

//*** need to consolidate these two parameters
#define MESHEPS 0.001
#define MESH_EPS 0.0001

#define PART_BOUND_UNDEFINED 0
#define PART_BOUND_COMPUTING 1
#define PART_BOUND_DEFINED   2

#define VERTEX1 1
#define VERTEX2 2

#define PART_SIZING  0
#define PART_LOADING 1
#define PART_BEFORE  0
#define PART_AFTER   1

#define VR_NO  0
#define VR_YES 1

#define FILE_UNLOADED 0
#define FILE_LOADING  1
#define FILE_LOADED   2

#define MLEFT  0
#define MFRONT 1
#define MDOWN  2
#define MRIGHT 3
#define MBACK  4
#define MUP    5
#define MEPS   0.1

#define NO_SMOKE -1
#define NO_FIRE  -1

#define GLUTPOSTREDISPLAY  if(use_graphics==1)glutPostRedisplay()
#define GLUTSETCURSOR(val) if(use_graphics==1)glutSetCursor(val)

#define ENABLE_LIGHTING if(use_lighting==1&&lighting_on==0){glEnable(GL_LIGHTING);lighting_on=1;}
#define DISABLE_LIGHTING if(use_lighting==1&&lighting_on==1){glDisable(GL_LIGHTING);lighting_on=0;}

#define PROJECTION_PERSPECTIVE  0
#define PROJECTION_ORTHOGRAPHIC 1

#define ALL_SMOKE_FRAMES -1

#define INIT   0
#define UPDATE 1

#define PRINT    1
#define NO_PRINT 0

#define SKY_BOX      0
#define SKY_SPHERE   1
#define RESET_COLORS 2

#define TIMEBAR_OVERLAP_ALWAYS 0
#define TIMEBAR_OVERLAP_NEVER  1
#define TIMEBAR_OVERLAP_AUTO   2

#define RENDER_START         3
#define RENDER_START_NORMAL 12
#define RENDER_START_GIF    21
#define RENDER_START_360    10

#define RESEARCH_MODE 114

#define LOAD_ALL_PART_FILES      -1
#define RELOAD_LOADED_PART_FILES -2

#define COLORBAR_HIDDEN          0
#define COLORBAR_SHOW_VERTICAL   1
#define COLORBAR_SHOW_HORIZONTAL 2

#define COLORBAR_SET 18
#define COLORBAR_RGB  2
#define COLORBAR_LIST 0

#define DLG_3DSMOKE  0
#define DLG_BOUNDARY 1
#define DLG_SLICE    2
#define DLG_PART     3
#define DLG_PLOT3D   4
#define DLG_ISO      5
#define DLG_HVACDUCT 6
#define DLG_HVACNODE 7

#define COLORBAR_LIST2 112
#define COLORBAR_LIST2_PREV 128
#define COLORBAR_LIST2_NEXT 129

#define ZONEVENT_CIRCLE 1
#define ZONEVENT_SQUARE 2

#define ZONE_HIDDEN 0
#define ZONE_XPLANE 1
#define ZONE_YPLANE 2
#define ZONE_ZPLANE 3

#define ROTATE_ABOUT_USER_CENTER     -1
#define ROTATE_ABOUT_CLIPPING_CENTER -2
#define ROTATE_ABOUT_FDS_CENTER      -3
#define ROTATE_ABOUT_WORLD_CENTER    -4
#define ROTATE_ABOUT_MESH_CENTER     -5
#define MESH_INDEX                   -6

#define ONLY_IN_GAS           0
#define GAS_AND_SOLID         1
#define ONLY_IN_SOLID         2
#define NEITHER_GAS_NOR_SOLID 3

#define UPDATE_WINDROSE_DEVICE   0
#define UPDATE_WINDROSE_CHECKBOX 1
#define UPDATE_WINDROSE_SHOWHIDE 2

#ifndef START_TICKS
#define START_TICKS(a) a = glutGet(GLUT_ELAPSED_TIME)
#endif
#ifndef STOP_TICKS
#define STOP_TICKS(a) a = glutGet(GLUT_ELAPSED_TIME) - a
#endif

#define TMAX 1000000000.0

#define PARTFILE_LOADALL   -11
#define PARTFILE_RELOADALL -12

#define FIRST_TIME     1
#define NOT_FIRST_TIME 2

#define SET_SLICECOLOR   0
#define DEFER_SLICECOLOR 1

#define SLICEDUP_KEEPALL    0
#define SLICEDUP_KEEPFINE   1
#define SLICEDUP_KEEPCOARSE 2

#define SMOKESENSORS_HIDDEN 0
#define SMOKESENSORS_0255   1
#define SMOKESENSORS_01     2
#define SMOKESENSORS_SCALED 3
#define SMOKESENSORS_0INF   4

#define TOURINDEX_ALL     -3
#define TOURINDEX_MANUAL  -1
#define TOURINDEX_DEFAULT -4

#define SURFACE_HIDDEN  0
#define SURFACE_SOLID   1
#define SURFACE_OUTLINE 2
#define SURFACE_POINTS  3

#define SHOWALL_FILES 0
#define SHOWONLY_FILE 1
#define HIDEALL_FILES 2

#define UNCOMPRESSED_ALLFRAMES 0
#define COMPRESSED_ALLFRAMES   1

#define COMPRESSED_UNKNOWN -1
#define UNCOMPRESSED        2
#define COMPRESSED_RLE      0 // 3d smoke file format assumes rle parameter is 0
#define COMPRESSED_ZLIB     1 // 3d smoke file format assumes zlib parameter is 1

#define DISABLE 0
#define ENABLE  1

#define SMOKE3D_FIRE_ONLY      0
#define SMOKE3D_SMOKE_ONLY     1
#define SMOKE3D_SMOKE_AND_FIRE 2

#define XWALLMIN -1
#define XWALLMAX  1
#define YWALLMIN -2
#define YWALLMAX  2
#define ZWALLMIN -3
#define ZWALLMAX  3

#define NOT_FDSBLOCK 0
#define FDSBLOCK     1

#define PATCH_STRUCTURED_NODE_CENTER 0
#define PATCH_STRUCTURED_CELL_CENTER 1
#define PATCH_GEOMETRY_BOUNDARY      2
#define PATCH_GEOMETRY_SLICE         3

#define NODATA  0
#define HASDATA 1

#define NO_TEST          0
#define TRIANGLE_TEST    1
#define POLYGON_TEST     2
#define TETRAHEDRON_TEST 3

#define NELEV_ZONE 100

#define UPDATE_ISO_OFF        0
#define UPDATE_ISO_ONE_NOW    1
#define UPDATE_ISO_ALL_NOW    2
#define UPDATE_ISO_START_ALL -1

#define MAX_ISO_COLORS 10

#define ZONETEMP_COLOR   0
#define ZONEHAZARD_COLOR 1
#define ZONESMOKE_COLOR  2

#define MAX_HSLABS 10
#define MAX_VSLABS  2
#define MAX_MSLABS  2

#define MAKE_MOVIE        28
#define MAKE_MOVIE_BATCH 130
#define OUTPUT_FFMPEG    129

#define PNG        0
#define JPEG       1
#define IMAGE_NONE 2

#define AVI 0
#define MP4 1
#define WMV 2
#define MOV 3

#define EXTERNAL_LIST_ID 1

#define TEXTURE_SPHERICAL   0
#define TEXTURE_RECTANGULAR 1

#define ADD_KEYFRAME     1
#define DELETE_KEYFRAME -1


#define C_GENERATED       0
#define FORTRAN_GENERATED 1

#define SHOW_ALL_VENTS 10
#define HIDE_ALL_VENTS 22

#define VENT_SOLID   0
#define VENT_OUTLINE 2
#define VENT_HIDDEN -2

#define HFLOW_VENT 0
#define VFLOW_VENT 1
#define MFLOW_VENT 2

#define CLIP_ON_DENORMAL 2
#define CLIP_ON          1

#define RENDER_ON  1
#define RENDER_OFF 0

// render_mode values:
#define RENDER_NORMAL 0
#define RENDER_360    1
#define RENDER_GIF    2

// render_times values:
#define RENDER_SINGLETIME 0
#define RENDER_ALLTIMES   1

#define RENDER_LABEL_FRAMENUM 0
#define RENDER_LABEL_TIME     1

#define CLIP_UNDEFINED      -1
#define CLIP_OFF            0
#define CLIP_BLOCKAGES_DATA 1
#define CLIP_BLOCKAGES      2
#define CLIP_DATA           3
#define CLIP_MAX            3

#define UNCLIP SetClipPlanes(NULL,CLIP_OFF)
#define CLIP SetClipPlanes(&clipinfo,CLIP_ON)

#define CLIP_GEOMETRY   \
  {int clip_geom=0;\
    if(clip_mode==CLIP_BLOCKAGES||clip_mode==CLIP_BLOCKAGES_DATA)clip_geom=1;\
    if( clipon==0&&clip_geom==1){CLIP;}\
    else if( clipon==1&&clip_geom==0){UNCLIP;}\
  }

#define CLIP_VALS   \
  {int clip_data=0;\
  if(clip_mode==CLIP_DATA||clip_mode==CLIP_BLOCKAGES_DATA)clip_data=1;\
  if( clipon==0&&clip_data==1){CLIP;}\
    else if( clipon==1&&clip_data==0){UNCLIP;}\
  }

#define GAS        1
#define SOLID      0
#define GASGAS     2
#define SOLIDSOLID 0
#define SOLIDGAS   1
#define GASSOLID   1

#define IN_GAS     0
#define IN_SOLID   1
#define IN_CUTCELL 2

#define IN_GAS_GLUI     0
#define IN_SOLID_GLUI   1
#define IN_CUTCELL_GLUI 2

#define MAX_CELL_TYPES 3

#define GEOM_OUTLINE_POLYGON  0
#define GEOM_OUTLINE_TRIANGLE 1
#define GEOM_OUTLINE_HIDDEN   2

#define SCENE_OUTLINE_HIDDEN 0
#define SCENE_OUTLINE_MESH   1
#define SCENE_OUTLINE_SCENE  2

#define EMBED_YES 0
#define EMBED_NO  1

#define XXX 0
#define YYY 1
#define ZZZ 2

#define KEY_ALT   0
#define KEY_CTRL  1
#define KEY_SHIFT 3
#define KEY_NONE  2

#define NOGRID_NOPROBE 0
#define GRID_NOPROBE   1
#define GRID_PROBE     2
#define NOGRID_PROBE   3
#define NOGRID_PROBE2  4

#define FROM_SMOKEVIEW     0
#define FROM_CALLBACK      1
#define FROM_SCRIPT        2
#define FROM_SMOKEVIEW_ALT 3
#define FROM_GEOM_DIALOG   4

#define STEPS_PER_DEG 10.0

#define SLICE_UNKNOWN     -1
#define SLICE_NODE_CENTER 1
#define SLICE_CELL_CENTER 2
#define SLICE_TERRAIN     4
#define SLICE_GEOM        6

#define TERRAIN_SURFACE    0
#define TERRAIN_IMAGE      1
#define TERRAIN_HIDDEN     2
#define TERRAIN_TOP        3

#define TERRAIN_TOP_SIDE      0
#define TERRAIN_BOTTOM_SIDE   1
#define TERRAIN_BOTH_SIDES    2

#define CSV_FDS   0
#define CSV_CFAST 1
#define CSV_EXP   2

#define TEPS 0.00

#define CSV_UNDEFINED 0
#define CSV_DEFINED   1
#define CSV_DEFINING  2

#define PART_POINTS     1
#define PART_SPHERES    2
#define PART_LINES      3
#define PART_SMV_DEVICE 4

#define PART_MIN_SIZE    1.0
#define PART_MAX_SIZE  100.0

#define PART_MIN_WIDTH   1.0
#define PART_MAX_WIDTH 100.0

#define DOUBLE_BUFFER 2
#define SINGLE_BUFFER 1

#define PROJECTION 24

#define GLOBAL_INI 0
#define STDOUT_INI 1
#define LOCAL_INI  2
#define SCRIPT_INI 3

#define USE_OPACITY_MULTIPLIER 94

#define RESTORE_EXTERIOR_VIEW      0
#define SAVE_VIEW                  3
#define TOGGLE_TITLE_SAFE          5
#define RESTORE_EXTERIOR_VIEW_ZOOM 6

#define TRANSLATE_XY_option        0
#define TRANSLATE_Y_option         1
#define TRANSLATE_X_option         2

#define ROTATION_2AXIS             0
#define EYE_CENTERED               1
#define ROTATION_1AXIS             2
#define ROTATION_3AXIS             3

#define MENU_MOTION_SETTINGS       4
#define MENU_MOTION_GRAVITY_VECTOR 5
#define MENU_MOTION_Z_VECTOR       6
#define MENU_MOTION_SHOW_VECTORS   7

#define MENU_HVAC_HIDE_ALL_VALUES -15

#define FIRSTCALL     1
#define NOT_FIRSTCALL 0

#define SELECT_BLOCKS     1
#define NOT_SELECT_BLOCKS 0

#define TO_BW    0
#define TO_COLOR 1

#define VENT_CIRCLE    0
#define VENT_RECTANGLE 1
#define VENT_HIDE      2

#define TETRA_CLIPPLANES 1

#define DIR_UNDEFINED -1
#define DOWN_Y 0
#define UP_X   1
#define UP_Y   2
#define DOWN_X 3
#define DOWN_Z 4
#define UP_Z   5

#define GEOM_STATIC  0
#define GEOM_DYNAMIC 1

#define GEOM_UPDATE_ALL     0
#define GEOM_UPDATE_NORMALS 1

#define NO_PLOTS                0
#define STATIC_PLOTS            1
#define DYNAMIC_PLOTS           2
#define STATIC_PLOTS_NORECURSE  3
#define DYNAMIC_PLOTS_NORECURSE 4

#define DISPLAY_PLOT3D        996
#define TOGGLESHOW_PLOT3D     995
#define SHOWALL_PLOT3D        998
#define HIDEALL_PLOT3D        999
#define GLUI_SHOWALL_BOUNDARY 994
#define GLUI_HIDEALL_BOUNDARY 993
#define HIDEALL_BOUNDARY      999
#define SHOW_CHAR             997
#define HIDEALL_PARTICLE        4
#define SHOWALL_PARTICLE        3
#define TOGGLE_ISO          10003
#define HIDEALL_ISO         10002
#define SHOWALL_ISO         10001

#define GEOM_PROP_NONE     0
#define GEOM_PROP_VERTEX1  1
#define GEOM_PROP_VERTEX2  2
#define GEOM_PROP_TRIANGLE 3
#define GEOM_PROP_SURF     4

#define TEMP_IGNITION_MAX 100000.
#define SURFACE_TEMPMIN  -100000.
#define SURFACE_TEMPMAX   100000.

#define PERCENTILE_MIN    0
#define SET_MIN           1
#define GLOBAL_MIN        2
#define GLOBAL_MIN_LOADED 3
#define CHOP_MIN          3

#define PERCENTILE_MAX    0
#define SET_MAX           1
#define GLOBAL_MAX        2
#define GLOBAL_MAX_LOADED 3
#define CHOP_MAX          3

#define SHADED_CONTOURS  0
#define STEPPED_CONTOURS 1
#define LINE_CONTOURS    2

#define SLICE_LINE_CONTOUR  0
#define SLICE_SOLID_CONTOUR 1

#define BLOCK_regular 0
#define BLOCK_texture 1
#define BLOCK_outline 2
#define BLOCK_hidden -2

#define BLOCK_face         0
#define VENT_face          1
#define OUTLINE_FRAME_face 2
#define SHADED_FRAME_face  3

#define visBLOCKAsInput         1
#define visBLOCKAsInputOutline 13
#define visBLOCKNormal          8
#define visBLOCKSolidOutline   12
//#define visBLOCKFacet         3
#define visBLOCKOutline         2
#define visBLOCKTransparent    10
#define visBLOCKAddOutline     14
#define visBLOCKOnlyOutline    15
#define visBLOCKOutlineColor   16
#define visCADOpaque           17
#define visLightFaces          18
#define ANIMATE_BLOCKAGES      19
#define visBLOCKHide            0

#define OUTLINE_NONE     0
#define OUTLINE_ONLY     1
#define OUTLINE_ADDED    2
#define BLOCKAGE_ASINPUT 0
#define BLOCKAGE_SOLID   1
#define BLOCKAGE_HIDDEN  2

#define BLOCKlocation_grid  5
#define BLOCKlocation_exact 6
#define BLOCKlocation_cad   7
#define BLOCKtexture_cad   31

#define WALL_1 0
#define WALL_3 1
#define WALL_6 2

// (front wall = 1, right wall = 2, back wall = 3, left wall = 4)

#define FRONT_WALL  1
#define RIGHT_WALL  2
#define BACK_WALL   3
#define LEFT_WALL   4
#define BOTTOM_WALL 5
#define TOP_WALL    6

#define XLEFT   -1
#define XRIGHT   1
#define YFRONT  -2
#define YBACK    2
#define ZBOTTOM -3
#define ZTOP     3

#define IMIN 0
#define IMAX 1
#define JMIN 2
#define JMAX 3
#define KMIN 4
#define KMAX 5

#define WINDROSE_USE_DT      0
#define WINDROSE_USE_TMINMAX 1
#define WINDROSE_USE_NEITHER 2

#define WINDROSE_XY          0
#define WINDROSE_XZ          1
#define WINDROSE_YZ          2
#define WINDROSE_LOCALSCALE  0
#define WINDROSE_GLOBALSCALE 1
#define WINDROSE_DIRECTION   0
#define WINDROSE_HEADING     1

#define WINDROSE_POINT   0
#define WINDROSE_STEPPED 1
#define WINDROSE_SLIDING 2

#define CLOSE_WINDOW       -2
#define UPDATE_WINDOW      -3
#define CANCEL_WINDOW      -4
#define UNDO_BLOCKAGE      -5
#define UNDO_ALL_BLOCKAGES -6

#define VISIBLE   1
#define INVISIBLE 0

#define NBUCKETS 1000000

#define MOVE          0
#define STRETCH_BLACK 1
#define STRETCH_WHITE 2

#define XDIR     1
#define YDIR     2
#define ZDIR     3
#define XDIRNEG -1
#define YDIRNEG -2
#define ZDIRNEG -3
#define ISO 4

#define NTARGTIMES 100

#define RELOAD_ALL_NOW           0
#define RELOAD_INCREMENTAL_NOW  -2
#define RELOAD_SWITCH           -4
#define STOP_RELOADING          -1
#define RELOAD_MODE_ALL         -6
#define RELOAD_SMV_FILE         -7

#define RELOAD_INCREMENTAL_ALL 9
#define RELOADALL              4
#define UNLOADALL              1
#define SHOWFILES              5
#define REDIRECT               6
#define SHOWMESHMENUS          7
#define COMPUTE_SMV_BOUNDS    10
#define SHOW_BOUND_DIFFS      11
#define CACHE_FILE_DATA       12
#define LOAD_WHEN_LOADED      13

#define SCRIPT_START_RECORDING2 -6
#define SCRIPT_START_RECORDING  -2
#define SCRIPT_STOP_RECORDING   -3
#define SCRIPT_STEP             -5
#define SCRIPT_CONTINUE         -7
#define SCRIPT_CANCEL           -8
#define MENU_SCRIPT_SETTINGS    -9

#define MENU_DEVICES_SHOWALL -1
#define MENU_DEVICES_HIDEALL -2

#define DRAWSCENE    1
#define SELECTOBJECT 2

#define CORRECT 1

#define VIEW_LEFT   0
#define VIEW_RIGHT  1
#define VIEW_CENTER 2

#define STEREO_NONE   0
#define STEREO_TIME   1
#define STEREO_LR     2
#define STEREO_RB     3
#define STEREO_RC     4
#define STEREO_CUSTOM 5

#define LEFT_EYE  0
#define RIGHT_EYE 1
#define BOTH_EYES 2

#define IINT   int
#define UIINT  unsigned int
#define FFLOAT float

#define ALL_TRANSPARENT 1
#define ALL_SOLID            4
#define MIN_SOLID            2
#define MAX_SOLID            3
#define HIDE_ALL            -1
#define SHOW_ALL            -2
#define GLUI_HIDEALL        -5
#define GLUI_SHOWALL        -6
#define GLUI_HIDEALL_SLICE  GLUI_HIDEALL
#define GLUI_SHOWALL_SLICE  GLUI_SHOWALL
#define UNLOAD_ALL          -1
#define LOAD_ALL            -2
#define RELOAD_ALL          -3
#define SHOWALL_SLICE       SHOW_ALL
#define SHOWALL_SMOKE3D     SHOW_ALL
#define HIDEALL_SLICE       HIDE_ALL
#define HIDEALL_SMOKE3D     HIDE_ALL
#define HIDEALL_VSLICE      HIDE_ALL
#define SHOWALL_VSLICE      SHOW_ALL
#define TOGGLE_SMOKE3D      -3
#define SET_SMOKE3D         -4
#define GLUI_SHOWALL_VSLICE GLUI_SHOWALL
#define GLUI_HIDEALL_VSLICE GLUI_HIDEALL

#define RENDER_RESOLUTION_320x240 0
#define RENDER_RESOLUTION_640x480 1
#define RENDER_RESOLUTION_CURRENT 2
#define RENDER_RESOLUTION_HIGH    3
#define RENDER_RESOLUTION_360     4

#define SMOKE_OUTLINE_TRIANGLE 0
#define SMOKE_TRIANGULATION    1
#define SMOKE_OUTLINE_POLYGON  2

#define SMOKE3D_ZEROS_SOME    0
#define SMOKE3D_ZEROS_ALL     1
#define SMOKE3D_ZEROS_UNKNOWN 2

#define SHOW_VOLSMOKE   -2
#define HIDE_VOLSMOKE   -1
#define TOGGLE_VOLSMOKE -3

#define MAXPOINTS          50000000
#define INCFRAMES                20
#define MAXFRAMES              5001
#define MAXRGB                  256
#define MAXSMOKERGB             256
#define StepOn                10000
#define RenderCancel            999
#define RenderStart             990
#define RenderStartHIGHRES      988
#define RenderStartORIGRES      987
#define RenderStartGIF          981
#define RenderStart360          986
#define Render360               989
#define RENDER_CURRENT_SINGLE   998
#define RENDER_CURRENT_MULTIPLE 978
#define RENDER_CURRENT_360      991
#define RenderJPEG              997
#define RenderPNG               996
#define Render320               995
#define Render640               994
#define RenderWindow            993
#define RenderCustom            992
#define LABELLEN                 30
#define RenderLABELframenumber  980
#define RenderLABELtime         979
#define MENU_RENDER_SETTINGS    900
#define RenderJSON              982
#define RenderJSONALL           983
#define RenderHTML              984
#define RenderHTMLALL           985
#define HTML_CURRENT_TIME         0
#define HTML_ALL_TIMES            1

#define SHOW_EXTERIOR_WALL_MENU           -1
#define HIDE_EXTERIOR_WALL_MENU          -19
#define SHOW_INTERIOR_WALL_MENU          -21
#define HIDE_INTERIOR_WALL_MENU          -22
#define INI_EXTERIORwallmenu          -20
#define INTERIOR_WALL_MENU               -2
#define FRONTwallmenu                  -3
#define BACKwallmenu                   -4
#define LEFTwallmenu                   -5
#define RIGHTwallmenu                  -6
#define UPwallmenu                     -7
#define DOWNwallmenu                   -8
#define DUMMYwallmenu                  -9
#define SOLIDpatchmenu                -10
#define OUTLINEpatchmenu              -11
#define POINTSpatchmenu               -12
#define INSOLIDpatchmenu              -13
#define INGASpatchmenu                -14
#define INCUTCELLpatchmenu            -15
#define SHOWCUTCELLPOLYGONSpatchmenu  -16
#define SHOWCUTCELLTRIANGLESpatchmenu -17
#define HIDECUTCELLTRIANGLESpatchmenu -18

#define INTERIORwall 0
#define FRONTwall    1
#define BACKwall     2
#define LEFTwall     3
#define RIGHTwall    4
#define UPwall       5
#define DOWNwall     6

#define offsetscale 100

#define FIRECOLOR_RGB      0
#define FIRECOLOR_COLORBAR 1

#define CO2_RGB      0
#define CO2_COLORBAR 1

#define RENDER_SLICE  0
#define RENDER_VOLUME 1

#define COLORBAR_FLIP             -2
#define COLORBAR_AUTOFLIP         -6
#define COLORBAR_TOGGLE_BW       -12
#define COLORBAR_CONTINUOUS      -17
#define COLORBAR_STEPPED         -18
#define COLORBAR_LINES           -19
#define COLORBAR_HORIZONTAL      -23
#define COLORBAR_VERTICAL        -24
#define COLORBAR_LINES           -19
#define COLORBAR_HIGHLIGHT_BELOW  -7
#define COLORBAR_HIGHLIGHT_ABOVE -20
#define COLORBAR_TRANSPARENT     -13
#define COLORBAR_RESET            -4
#define COLORBAR_TOGGLE_BW_DATA  -21
#define MENU_COLORBAR_SETTINGS   -22
#define USE_LIGHTING             -25
#define TOGGLE_LIGHTING          -26
#define COLORBAR_DECIMAL         -27

#define LOAD        0
#define UNLOAD      1
#define RESETBOUNDS 2
#define RELOAD      3
#define UPDATE_HIST 4
#define BOUNDS_ONLY 5

#define FIRST_TIME  1
#define LATER_TIME  0

#define MAKE_SIZEFILE 0
#define GET_DATA      1

#define MAXPLOT3DVARS 6
#define NRGB         12

#define SMALL_FONT         0
#define LARGE_FONT         1
#define SCALED_FONT        2
#define MENU_FONT_SETTINGS 3

#ifndef FFALSE
#define FFALSE 0
#endif

#ifndef TTRUE
#define TTRUE  1
#endif

#define BLOCKAGE_AS_INPUT  35
#define BLOCKAGE_AS_INPUT2 36

#define COLORBAR_INDEX_NONE -1

#define BLUE   0
#define GREEN  1
#define YELLOW 2
#define PINK  3
#define RED  4

#define DRAW_OPAQUE      0
#define DRAW_TRANSPARENT 1

#define VOL_READALL  -1
#define VOL_UNLOAD   -2
#define VOL_READNONE -3

#define MENU_LABEL_colorbar_vertical    0
#define MENU_LABEL_colorbar_horizontal  1
#define MENU_LABEL_timebar              2
#define MENU_LABEL_framerate            3
#define MENU_LABEL_axis                 4
#define MENU_LABEL_textlabels           5
#define MENU_LABEL_timelabel            6
#define MENU_LABEL_meshlabel            7
#define MENU_LABEL_memload              8
#define MENU_LABEL_memusage             9
#define MENU_LABEL_fdsticks            10
#define MENU_LABEL_hmslabel            11
#define MENU_LABEL_grid                12
#define MENU_LABEL_sliceaverage        13
#define MENU_LABEL_userticks           15
#define MENU_LABEL_ShowAll             16
#define MENU_LABEL_HideAll             17
#define MENU_LABEL_framelabel          18
#define MENU_LABEL_hrr                 19
#define MENU_LABEL_northangle          20
#define MENU_LABEL_SETTINGS            21
#define MENU_LABEL_frametimelabel      22

#define MENU_TITLE_title_smv_version    0
#define MENU_TITLE_title_fds            1
#define MENU_TITLE_chid                 2
#define MENU_TITLE_gversion             3
#define MENU_TITLE_show_all             4
#define MENU_TITLE_hide_all             5

#define MENU_TRAINER_smoke 1
#define MENU_TRAINER_temp  2
#define MENU_TRAINER_oxy   3

#define ON  1
#define OFF 0

#define BACKGROUND 1
#define FOREGROUND 0

#define DIALOG_3DSMOKE   20
#define DIALOG_BOUNDS    14
#define DIALOG_CLIP      18
#define DIALOG_COLORBAR  23
#define DIALOG_DEVICE    28
#define DIALOG_2DPLOTS   46
#define DIALOG_DISPLAY   22
#define DIALOG_HIDEALL   -2
#define DIALOG_MOTION    29
#define DIALOG_VIEW      30
#define DIALOG_RENDER    31
#define DIALOG_GEOMETRY_OPEN  16
#define DIALOG_GEOMETRY_CLOSE 50
#define DIALOG_SHOOTER 27
#define DIALOG_SMOKEZIP  24
#define DIALOG_STEREO    19
#define DIALOG_TOUR_SHOW 21
#define DIALOG_TOUR_HIDE 44
#define DIALOG_TRAINER   25
#define DIALOG_SHOWFILES 33
#define DIALOG_SCRIPT    32
#define DIALOG_CONFIG    34
#define DIALOG_FONTS     35
#define DIALOG_USER_TICKS   36
#define DIALOG_LABELS_TICKS 37
#define DIALOG_AUTOLOAD  38
#define DIALOG_TIME      39
#define DIALOG_SCALING   41
#define DIALOG_WINDOW_PROPERTIES 42
#define DIALOG_MOVIE     43
#define DIALOG_MOVIE_BATCH 45
#define DIALOG_SHRINKALL -3
#define DIALOG_HVAC      47
#define DIALOG_TERRAIN   48
#define DIALOG_COLORING  49

#define UPDATE_PROJECTION -2

#define MENU_TOUR_DEFAULT       -1
#define MENU_TOUR_MANUAL        -2
#define MENU_TOUR_SHOWALL       -3
#define MENU_TOUR_SHOWDIALOG    -4
#define MENU_TOUR_VIEWFROMROUTE -5
#define MENU_TOUR_NEW          -12
#define MENU_TOUR_CLEARALL     -13
#define MENU_TOUR_SETTINGS     -14

#define MENU_TEXTURE_SHOWALL  -1
#define MENU_TEXTURE_HIDEALL  -2
#define MENU_TEXTURE_SHOWALL2 -3

#define MENU_SHOWHIDE_FLIP 15

#define MAX_SMV_FILENAME_BUFFER       1024
#define MAX_LUASCRIPT_FILENAME_BUFFER 1024
#define MAX_SCRIPT_FILENAME_BUFFER    1024

#endif
