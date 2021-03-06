#ifndef IGLUI_MOTION_H_DEFINED
#define IGLUI_MOTION_H_DEFINED

#define TRANSLATE_XY 101
#define ROTATE_2AXIS 102
#define GLUI_Z         2
#define MESH_LIST      4
#define EYE_ROTATE     5
#define EYE_ROTATE_90  6
#define EYELEVEL       7
#define FLOORLEVEL     8
#define ROTATE_90    103
#define RESET_VIEW   104

#define LABEL_VIEW   4
#define GEOM_FACTORS 33

#define MOVIE_SLICE_INDEX 1

#define CUSTOM_ROTATION_X      9
#define CUSTOM_ROTATION_Y     10
#define CUSTOM_ROTATION_Z     11
#define LIST_VIEW              5
#define LIST_VIEW_FROM_DIALOG 45
#define ADD_VIEW               6
#define DELETE_VIEW            7
#define RESTORE_VIEW           8
#define REPLACE_VIEW           9
#define REPLACE_CURRENT_VIEW  44
#define STARTUP               10
#define CYCLEVIEWS            11
#define ZOOM                  12
#define APERTURE              15
#define CURSOR                13
#define SAVE_SETTINGS_MOTION  14
#define WINDOW_RESIZE         16
#define WINDOWSIZE_LIST       17
#define SNAPSCENE             21
#define SET_VIEW_XYZ          22
#define ZAXIS_CUSTOM          25
#define USE_GVEC              28
#define GSLICE_TRANSLATE      24
#define GSLICE_NORMAL         27
#define PLAY_MOVIE            29
#define MOVIE_NAME            30
#define CLOSE_MOTION           1
#ifdef pp_RENDER360_DEBUG
#define SHOWALL_SCREENS       31
#define HIDEALL_SCREENS       32
#endif
#define WINDOW_COLORS         33
#define COLOR_FLIP            34
#define CLIP_SHOW_ROTATE      35
#define ZAXIS_UP              41
#define NEARFARCLIP           42
#define CUSTOM_VIEW           43

#define RENDER_TYPE 0
#define RENDER_RESOLUTION 1
#define RENDER_SKIP 2
#define RENDER_START_HIGHRES 11
#define RENDER_STOP 4
#define RENDER_LABEL 5
#define RENDER_MULTIPLIER 6
#define MOVIE_FILETYPE 7
#define RENDER_MODE 8
#define RENDER_START_TOP 13
#define RENDER_DEBUG_360 14

#define SLICE_ROLLOUT           0
#define VIEWPOINTS_ROLLOUT      1
#define WINDOW_ROLLOUT          2
#define SCALING_ROLLOUT         3
#define ROTATION_ROLLOUT        4
#define ORIENTATION_ROLLOUT     5
#define POSITION_VIEW_ROLLOUT   6
#define TRANSLATEROTATE_ROLLOUT 7

#define MOTION_ROLLOUT       0
#define VIEW_ROLLOUT         1
#define RENDER_ROLLOUT       2
#define MOVIE_ROLLOUT        3
#define MOVIE_ROLLOUT_BATCH  4

#define RENDER_FILE_ROLLOUT   0
#define RENDER_SIZE_ROLLOUT   1
#define RENDER_SCREEN_ROLLOUT 2
#define RENDER_CLIP_ROLLOUT   3

#define RENDER_360CB 9
#define RENDER_HTML 15

#endif

