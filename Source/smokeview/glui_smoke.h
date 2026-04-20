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
#define FORCE_GRAY             7
#define SMOKE_COLORBAR_LIST   16
#define FIRECOLORMAP_TYPE     17
#define CO2_COLORBAR_LIST     82
#define CO2COLORMAP_TYPE      83
#define USE_SMOKE_RGB         87
#define USE_FIRE_RGB          88
#define USE_FIRE_COLORMAP     89
#define UPDATE_FIRE_HISTOGRAM 116
#define USE_CO2_RGB           90
#define USE_CO2_COLORMAP      91
#define SET_RGB_COLORGB_CHECKBOXES 92
#define ENABLE_DISABLE_FIRE   115
#define SHOW_FIRECOLORMAP 25
#ifdef pp_GPU
#define SMOKE_RTHICK 8
#endif
#define SAVE_SETTINGS_SMOKE 9
#define SMOKE_OPTIONS 19
#define TEMP_MIN 21
#define MASS_EXTINCTION 85
#define GENERATE_IMAGES 28
#define START_FRAME 29
#define SKIP_FRAME 30
#define CANCEL_GENERATE_IMAGES 31
#define SMOKE3D_LOAD_INCREMENTAL 18
#define CO2_COLOR 71
#define SMOKE_SKIP_ALL       107
#define SMOKE_SKIP_HORIZ     109
#define SMOKE_SKIP_VERT      110
#define SMOKE_SKIP_FRONTBACK 111
#define SMOKE_BLACK 75
#define SMOKE_DELTA_PAR 76
#define SMOKE_NEW 77
#define SMOKE_DELTA_MULTIPLE 78
#define SMOKEBOX_BUFFER 79
#define SMOKE_NUM 80
#define BACKGROUND_FLIP 81
#define SHOW_SMOKEMESH 118
#define SHOW_ONLY_SMOKEMESH 119
// #define USE_OPACITY_MULTIPLIER 94 // defined now in smokeviewdefs.h
#define SMOKE_EXTINCT 95
#define CUTOFF_RESET 96
#define EXTINCTION_RESET_FDS 97
#define EXTINCTION_RESET_SMV 98
#define BLACKBODY_TEMPS      101
#define FORCE_ALPHA_OPAQUE 104
#define SMOKE_FRAME_INC 113
#define REFRESH_FIRE       114
#define SMOKE_DEMO_MODE    117
#define USE_SOOT_MULTIPLIER 121
#define FIRE_HALFDEPTH    122 
#define SOOT_MULTIPLIER      123
#define SET_SMOKE_THREADS  124

#define GLOBAL_HRRPUV_MIN  101
#define GLOBAL_HRRPUV_MAX  102
#define GLOBAL_TEMP_MIN    103
#define GLOBAL_TEMP_MAX    104
#define COLORBAR_INDEX_MIN 105
#define COLORBAR_INDEX_MAX 106

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
EXTERNCPP void GLUIUpdateSmoke3dFlags(void);

#endif

