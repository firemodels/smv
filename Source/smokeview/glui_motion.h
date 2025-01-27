#ifndef IGLUI_MOTION_H_DEFINED
#define IGLUI_MOTION_H_DEFINED

#define TRANSLATE_XY 101
#define ROTATE_2AXIS 102
#define GLUI_Z         2
#define ROTATE_ABOUT   4
#define EYE_ROTATE     5
#define EYE_ROTATE_90  6
#define EYELEVEL       7
#define FLOORLEVEL     8
#define ROTATE_90    103
#define RESET_VIEW   104

#define LABEL_VIEW   4
#define GEOM_FACTORS 33

#define MOVIE_SLICE_INDEX 1

#define CUSTOM_ROTATION_XYZ    9
#define LIST_VIEW              5
#define LIST_VIEW_FROM_DIALOG 45
#define ADD_VIEW               6
#define DELETE_VIEW            7
#define RESTORE_VIEW           8
#define REPLACE_VIEW           9
#define REPLACE_CURRENT_VIEW  44
#define MOTION_STARTUP        10
#define CYCLEVIEWS_ALL        46
#define CYCLEVIEWS_DEFAULT    47
#define CYCLEVIEWS_USER       48
#define ZOOM                  12
#define APERTURE              15
#define CURSOR                13
#define SAVE_SETTINGS_MOTION  14
#define WINDOW_RESIZE         16
#define WINDOW_RESIZE_WIDTH   50
#define WINDOW_RESIZE_HEIGHT  51
#define WINDOW_PRESERVE       52
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

#define SLICE_ROLLOUT_MOTION    0
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

#define LOWER_SCREEN_ROLLOUT  0
#define MIDDLE_SCREEN_ROLLOUT 1
#define UPPER_SCREEN_ROLLOUT  2

#define RENDER_360CB 9
#define RENDER_HTML 15

//*** glui_motion.cpp headers

EXTERNCPP void GLUIMotionSetup(int main_window);
EXTERNCPP void GLUIViewpointCB(int val);
EXTERNCPP void GLUIUpdateMovieParms(void);
EXTERNCPP void GLUISetCurrentViewPoint(char *viewpoint_label);
EXTERNCPP void GLUIUpdateUseGeomFactors(void);
EXTERNCPP void GLUIUpdateWindowAspect(void);
EXTERNCPP void GLUIShrinkDialogs(void);
#ifdef CPP
EXTERNCPP void GLUICloseRollouts(GLUI *dialog);
EXTERNCPP void GLUIToggleRollout(procdata *procinfo, int nprocinfo, int motion_id);
#endif
EXTERNCPP void GLUIUpdatePosView(void);
EXTERNCPP void GLUIUpdateRenderRadioButtons(int width_low, int height_low, int width_high, int height_high);
EXTERNCPP void GLUIUpdateZAxisCustom(void);
EXTERNCPP void GLUIUpdateShowGravityVector(void);
EXTERNCPP void GLUIUpdateShowRotationCenter(void);
EXTERNCPP void GLUIUpdateShowRotationCenter2(void);
EXTERNCPP void GLUIUpdateRotationIndex(int val);
EXTERNCPP void GLUIUpdateRender(void);
EXTERNCPP void GLUIEnable360Zoom(void);
EXTERNCPP void GLUIEnableDisableMakeMovieCPP(int onoff);
EXTERNCPP void GLUIEnableDisablePlayMovieCPP(void);
EXTERNCPP void GLUIAddListView(char *label_in);
EXTERNCPP void GLUIUpdateViewpointList(void);
EXTERNCPP void GLUIUpdateCameraLabel(void);
EXTERNCPP void GLUISceneMotionCB(int var);
EXTERNCPP void GLUIUpdateWindowSizeList(void);
EXTERNCPP void GLUIUpdateGsliceParms(void);
EXTERNCPP void GLUIUpdateResolutionMultiplier(void);
EXTERNCPP void GLUISetColorControls(void);
EXTERNCPP void GLUIShowMotion(int menu_id);
EXTERNCPP void GLUIHideMotion(void);
EXTERNCPP void GLUIUpdateZoom(void);
EXTERNCPP void GLUISetPosXYZSMV(float *xyz);
EXTERNCPP void GLUISetPosXYZFDS(float *xyz);
EXTERNCPP void GLUIUpdateFileLabel(int var);
EXTERNCPP void GLUIRotationTypeCB(int var);
EXTERNCPP void GLUIUpdateRotationType(int val);
EXTERNCPP void GLUIEnableResetSavedView(void);
EXTERNCPP void GLUIResetView(int ival);
EXTERNCPP void GLUIUpdateProjectionType(void);
EXTERNCPP void GLUIUpdateMeshList1(int val);
EXTERNCPP void GLUIUpdateTranslate(void);
EXTERNCPP void GLUIShowHideTranslate(int var);
EXTERNCPP void GLUISetStartupView(void);
#ifdef CPP
EXTERNCPP void InsertRollout(GLUI_Rollout *rollout, GLUI *dialog);
#endif
EXTERNCPP void InitRolloutList(void);
EXTERNCPP void UpdateRenderListSkip(void);
EXTERNCPP void UpdateGluiRotateAbout(int val);
EXTERNCPP void UpdateRenderStartButton(void);
EXTERNCPP void UpdateRenderType(int type);
EXTERNCPP void UpdateMovieType(int type);
EXTERNCPP void RenderCB(int var);

#endif

