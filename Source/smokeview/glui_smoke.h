#ifndef IGLUI_SMOKE_H_DEFINED
#define IGLUI_SMOKE_H_DEFINED

#define SCRIPT_CANCEL_NOW 45

#define FIRE_RED 1
#define FIRE_GREEN 2
#define FIRE_BLUE 3
#define SMOKE_RED 51
#define SMOKE_GREEN 52
#define SMOKE_BLUE 53
#define SMOKE_GRAY 56
#define UPDATE_SMOKEFIRE_COLORS 54
#define UPDATE_CO2_COLORS 61
#define UPDATE_SMOKEFIRE_COLORS_COMMON 62
#define CO2SMOKE 63
#define UPDATE_SMOKECOLORS 4
#define GLOBAL_FIRE_CUTOFF 15
#define FORCE_GRAY             7
#define SMOKE_COLORBAR_LIST   16
#define FIRECOLORMAP_TYPE     17
#define CO2_COLORBAR_LIST     82
#define CO2COLORMAP_TYPE      83
#define USE_SMOKE_RGB         87
#define USE_FIRE_RGB          88
#define USE_FIRE_COLORMAP     89
#define USE_CO2_RGB           90
#define USE_CO2_COLORMAP      91
#define SET_RGB_COLORGB_CHECKBOXES 92
#define SHOW_FIRECOLORMAP 25
#ifdef pp_GPU
#define SMOKE_RTHICK 8
#endif
#define SAVE_SETTINGS_SMOKE 9
#define VOL_SMOKE 13
#define VOL_NGRID 18
#define SMOKE_OPTIONS 19
#define TEMP_MIN 21
#define TEMP_CUTOFF 22
#define TEMP_MAX 23
#define COMBINE_MESHES 24
#define MASS_EXTINCTION 85
#define NONGPU_VOL_FACTOR 26
#define GPU_VOL_FACTOR 27
#define GENERATE_IMAGES 28
#define START_FRAME 29
#define SKIP_FRAME 30
#define CANCEL_GENERATE_IMAGES 31
#define VOL_TOUR_LIST 46
#define VOL_PREFIX 47
#define VOL_UNLOAD_ALL 48
#define LOAD_SMOKEFRAME 55
#define LOAD_TIMEFRAME 58
#define SMOKE3D_LOAD_INCREMENTAL 18
#define CO2_COLOR 71
#define SMOKE_SKIP_XYZ 107
#define SMOKE_SKIP_XY  108
#define SMOKE_SKIP_X   109
#define SMOKE_SKIP_Y   110
#define SMOKE_SKIP_Z   111
#define SMOKE_BLACK 75
#define SMOKE_DELTA_PAR 76
#define SMOKE_NEW 77
#define SMOKE_DELTA_MULTIPLE 78
#define SMOKEBOX_BUFFER 79
#define SMOKE_NUM 80
#define BACKGROUND_FLIP 81
#define USE_FIRE_ALPHA 84
#define USE_OPACITY_DEPTH 93
// #define USE_OPACITY_MULTIPLIER 94 // defined now in smokeviewdefs.h
#define USE_OPACITY_DEPTH_CHECK 99
#define USE_OPACITY_MULTIPLIER_CHECK 100
#define SMOKE_EXTINCT 95
#define CUTOFF_RESET 96
#define EXTINCTION_RESET_FDS 97
#define EXTINCTION_RESET_SMV 98
#define BLACKBODY_TEMPS      101
#define VOLTEST_DEPTH 102
#define VOLTEST_UPDATE 103
#define FORCE_ALPHA_OPAQUE 104
#define MERGE_SMOKE 105
#define SMOKE_FRAME_INC 113
#define REFRESH_FIRE       114

//*** glui_smoke.cpp headers

EXTERNCPP void GLUIGetPixelsPerTriangle(void);
EXTERNCPP void GLUIForceAlphaOpaque(void);
EXTERNCPP void GLUI3dSmokeSetup(int main_window);
EXTERNCPP void GLUISmoke3dCB(int var);
EXTERNCPP void GLUIUpdateCO2ColorbarList(int value);
EXTERNCPP void GLUIUpdateFireColorbarList(void);
EXTERNCPP void GLUIUpdateFreeze(int val);
EXTERNCPP void GLUIUpdateLoadTimeVal(float val);
EXTERNCPP void GLUIUpdateTimeFrameBounds(float time_min, float time_max);
EXTERNCPP void GLUIUpdateLoadFrameVal(int frames);
EXTERNCPP void GLUIUpdateLoadFrameMax(int max_frames);
EXTERNCPP void GLUISmoke3dCB(int var);
EXTERNCPP void GLUIUpdateFireAlpha(void);
EXTERNCPP void GLUIUpdateSmoke3dFlags(void);
EXTERNCPP void GLUICreateVolTourList(void);
EXTERNCPP void GLUIDeleteVolTourList(void);

#endif

