#ifndef IGLUI_BOUNDS_H_DEFINED
#define IGLUI_BOUNDS_H_DEFINED

#define IMMERSED_SWITCH_CELLTYPE 0
#define IMMERSED_SET_DRAWTYPE    1
#define IMMERSED_SWITCH_EDGETYPE 2
#define SHOW_ALL_SLICE_VALUES    3
#define HIDE_ALL_SLICE_VALUES    4

#define ISO_BOUNDS_OUTPUT 403
#define SMOOTH_SURFACES 402
#define SORT_SURFACES 401
#define ISO_SURFACE 1
#define ISO_OUTLINE 2
#define ISO_POINTS 3
#define ISO_COLORS 4
#define ISO_LEVEL 5
#define ISO_TRANSPARENCY 6
#define ISO_SETVALMIN 10
#define ISO_SETVALMAX 11
#define ISO_VALMIN 12
#define ISO_VALMAX 13
#define GLOBAL_ALPHA 7
#define COLORTABLE_LIST 8
#define SETVALMIN 1
#define SETVALMAX 2
#define VALMIN 3
#define VALMAX 4
#define FILETYPE_INDEX      5
#define FILE_UPDATE         6
#define FILE_RELOAD         7
#define UPDATE_DATA_COLORS 8
#define UPDATEPLOT 10
#define PLOTISO 11
#define SHOWCHAR 12
#define CHOPVALMIN 13
#define CHOPVALMAX 14
#define SETCHOPMINVAL 15
#define SETCHOPMAXVAL 16
#define CHOPUPDATE 17
#define FRAMELOADING 18
#define STREAKLENGTH 19
#define TRACERS 21
#define PARTFAST 22
#define PLOTISOTYPE 22
#define CACHE_DATA 23
#define SHOW_ALL_EXTERIOR_PATCH_DATA 24
#define HIDE_ALL_EXTERIOR_PATCH_DATA 29
#define SHOW_ALL_INTERIOR_PATCH_DATA 30
#define HIDE_ALL_INTERIOR_PATCH_DATA 31
#define SHOW_EXTERIOR_PATCH_DATA     32
#define HIDEPATCHSURFACE 25
#define DATA_transparent 26
#define SET_GLOBAL_BOUNDS 27
#define PARTSKIP 28
#define UNLOAD_QDATA 203
#define SET_TIME 204
#define TBOUNDS 205
#define TBOUNDS_USE 206
#define RELOAD_ALL_DATA 207
#define RELOAD_INCREMENTAL_DATA 215
#define SLICE_VECTORSKIP         210
#define PLOT3D_VECTORSKIP        211
#define UPDATE_SLICEDUPS         212
#define UPDATE_HISTOGRAM         213
#define FRAMES_HISTOGRAM         223
#define INIT_HISTOGRAM           214
#define UPDATE_BOUNDARYSLICEDUPS 215
#define BOUNDARY_EDGETYPE        227
#define SHOW_BOUNDARY_OUTLINE    228
#define ISO_TRANSPARENCY_OPTION  216
#define ISO_COLORBAR_LIST        217
#define ISO_OUTLINE_IOFFSET      218
#define SET_FDS_TIMES            220
#define SLICE_PLOT2D             221
#define SLICE_SIZE               222
#define SLICE_PLOT_CSV           224
#define SLICE_PLOT_LABEL         225
#define SLICE_PLOT_FILENAME      226
#define SLICE_DPLOT              227
#define COLORBAR_PLOT2D          228

#define USEMESH_XYZ              100
#define USEMESH_USE_XYZ          200
#define USEMESH_USE_XYZ_ALL      199
#define USEMESH_DRAW_MESH        198
#define USEMESH_DRAW_BOX         197
#define USEMESH_SET_ONE          196
#define USEMESH_SET_ALL          195
#define USEMESH_LOAD_WHEN_LOADED 194
#define USEMESH_REMOVE_ALL       229
#define USEMESH_REMOVE_ONE          230

#define ISO_TRANSPARENT_CONSTANT 0
#define ISO_TRANSPARENT_VARYING  1
#define ISO_OPAQUE               2

#define SCRIPT_START 31
#define SCRIPT_STOP 32
#define SCRIPT_LIST 33
#define SCRIPT_SAVEINI 34
#define SCRIPT_EDIT_INI 35
#define SCRIPT_SETSUFFIX 36
#define SCRIPT_RUNSCRIPT 37
#define SCRIPT_LOADINI 38
#define SCRIPT_RENDER 41
#define SCRIPT_RENDER_SUFFIX 42
#define SCRIPT_RENDER_DIR 43
#define SCRIPT_STEP_NOW 44

#define ZONEVALMINMAX    50
#define SETZONEVALMINMAX 52

#define SAVE_SETTINGS_BOUNDS 99
#define CLOSE_BOUNDS 98
#define COMPRESS_FILES 97
#define OVERWRITE 96
#define COMPRESS_AUTOLOADED 91
#define ERASE 95
#define BOUND_STARTUP 94
#define SAVE_FILE_LIST 93
#define LOAD_FILES 92
#define COLORBAR_EXTREME2 109
#define TRANSPARENTLEVEL 110
#define RESEARCH_MODE 114
#define COLORBAND 115
#define SLICE_IN_GAS 116
#define SLICE_IN_SOLID 117
#define COLORLABEL_DIGITS 123
#define COLORLABEL_ZERO_PAD 134
#define SLICE_SKIP 124
#define FORCE_EXPONENTIAL 125
#define SORTSLICES       126
#define SORTSLICES_DEBUG 127
#define SLICE_OPTION 131
#define COLORINDEX 133

#define UPDATE_VECTOR          101
#define UPDATE_VECTOR_FROM_SMV 102
#define VEC_UNIFORM_SPACING    103

#define TRUNCATE_BOUNDS 1
#define DONT_TRUNCATE_BOUNDS 0
#define UPDATE_BOUNDS 1
#define RELOAD_BOUNDS 0
#define UPDATERELOAD_BOUNDS 2

#define LINE_CONTOUR_VALUE 301
#define UPDATE_LINE_CONTOUR_VALUE 302

#define FILESHOW_particle    10
#define FILESHOW_slice       11
#define FILESHOW_vslice      12
#define FILESHOW_boundary    13
#define FILESHOW_3dsmoke     14
#define FILESHOW_isosurface  15
#define FILESHOW_plot3d      16
#define FILESHOW_sizes       20
#define BOUNDARY_LOAD_INCREMENTAL 16
#define SLICE_LOAD_INCREMENTAL 17

//*** glui_bounds.cpp headers

EXTERNCPP int  GLUIGetChopMin(int type, char *label, int *set_valmin, float *valmin);
EXTERNCPP int  GLUIGetChopMax(int type, char *label, int *set_valmax, float *valmax);
EXTERNCPP int  GLUISetChopMin(int type, char *label, int set_chopmin, float chopmin);
EXTERNCPP int  GLUISetChopMax(int type, char *label, int set_chopmax, float chopmax);

EXTERNCPP void GLUIUpdateTextureDisplay(void);
EXTERNCPP void GLUIUpdateLoadAllSlices(void);
EXTERNCPP void GLUIUpdateMeshBounds(void);
EXTERNCPP void GLUIUpdateLoadWhenLoaded(void);
EXTERNCPP void GLUIBoundsSetup(int main_window);
EXTERNCPP void GLUIUpdatePartPointSize(void);
EXTERNCPP void GLUIUpdateBoundTbounds(void);
EXTERNCPP void GLUIUpdatePlotLabel(void);
EXTERNCPP void GLUIHVACDuctBoundsCPP_CB(int var);
EXTERNCPP void GLUIHVACNodeBoundsCPP_CB(int var);
EXTERNCPP void GLUISplitCB(int var);
EXTERNCPP void GLUIPlot3DBoundCB(int var);
EXTERNCPP void GLUIUpdateSliceSkip(void);
EXTERNCPP void GLUIUpdateHVACDuctType(void);
EXTERNCPP  void GLUIUpdateSliceXYZ(void);
EXTERNCPP void GLUIPartBoundsCPP_CB(int var);
EXTERNCPP void GLUIUpdatdateResearchModeCPP(void);
EXTERNCPP void GLUISliceBoundsSetupNoGraphics(void);
EXTERNCPP void GLUIGetGlobalBoundsMinMax(int type, char *label, float *valmin, float *valmax);
EXTERNCPP void GLUIHVACSliceBoundsCPP_CB(int var);
EXTERNCPP void GLUIPatchBoundsCPP_CB(int var);
EXTERNCPP cpp_boundsdata *GLUIGetBoundsData(int type);
EXTERNCPP void GLUISetCacheFlag(int type, int cache_flag);
EXTERNCPP void GLUISetValTypeIndex(int type, int valtype_index);
EXTERNCPP int GLUIGetNValtypes(int type);
EXTERNCPP void GLUIGetMinMax(int type, char *label, int *set_valmin, float *valmin, int *set_valmax, float *valmax);
EXTERNCPP void GLUIGetOnlyMinMax(int type, char *label, int *set_valmin, float *valmin, int *set_valmax, float *valmax);
EXTERNCPP void GLUIGetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int *nall);
EXTERNCPP void GLUISetMin(int type, char *label, int set_valmin, float valmin);
EXTERNCPP void GLUISetMax(int type, char *label, int set_valmax, float valmax);
EXTERNCPP void GLUISetMinMax(int type, char *label, int set_valmin, float valmin, int set_valmax, float valmax);
EXTERNCPP void GLUISetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int nall);
EXTERNCPP void GLUISetGlobalMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUISetLoadedMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUIGetGlobalMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUIGetLoadedMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUIUpdateBounds(void);
EXTERNCPP void GLUIPlot3DBoundsCPP_CB(int var);
EXTERNCPP void GLUISetColorbarDigitsCPP(int ndigits);
EXTERNCPP void GLUIUpdatePartFast(void);
EXTERNCPP void GLUIUpdateColorbarControls2(void);
EXTERNCPP void GLUISetColorbarDigits(void);
EXTERNCPP void GLUIIncrementPartPropIndex(void);
EXTERNCPP void GLUIUpdateIsoBounds();
EXTERNCPP void GLUIUpdateListIsoColorobar(void);
EXTERNCPP void GLUIUpdateUseLighting(void);
EXTERNCPP void GLUIImmersedBoundCB(int var);
EXTERNCPP void GLUIShowBoundsDialog(int type);
EXTERNCPP void GLUIExtremeCB(int var);
EXTERNCPP void GLUIUpdateExtremeVals(void);
EXTERNCPP void GLUIIsoBoundCB(int var);
EXTERNCPP void GLUIUpdateScriptStep(void);
EXTERNCPP void GLUIAddScriptList(char *file, int id);
EXTERNCPP void GLUISliceInObstMenu2Dialog(int var);
EXTERNCPP void GLUIUpdateColorTableList(int ncolortableinfo_old);
EXTERNCPP void GLUIUpdateIsoColorlevel(void);
EXTERNCPP void GLUIUpdateZoneBounds(void);
EXTERNCPP void GLUIScriptDisable(void);
EXTERNCPP void GLUIScriptEnable(void);
EXTERNCPP void GLUIUpdateVecFactor(void);
EXTERNCPP void GLUIUpdatePlot3Dtype(void);
EXTERNCPP void GLUIUpdateIsotype(void);
EXTERNCPP void GLUIUpdateStreakValue(float rvalue);
EXTERNCPP void GLUISetLabelControls2(void);
EXTERNCPP void GLUIShowBounds(int menu_id);
EXTERNCPP void GLUIHideBounds(void);
EXTERNCPP void GLUIUpdateBoundaryListIndex(int patchfilenum);
EXTERNCPP void GLUIUpdatePlot3dListIndex(void);
EXTERNCPP void GLUISetColorbarListBound(int val);
SVEXTERN void GLUIUpdatePlot2DSize2(void);
SVEXTERN void GLUIUpdateVectorWidgets(void);
EXTERNCPP void GLUIUpdateTransparency(void);
EXTERNCPP void GLUIUpdateScriptStart(void);
EXTERNCPP void GLUIUpdateResearchMode(void);
EXTERNCPP void GLUIUpdateScriptStop(void);
EXTERNCPP void GLUIUpdateTBounds(void);
EXTERNCPP void GLUIUpdateTimeBounds(float time_min, float time_max);
EXTERNCPP void GLUIUpdateColorbarFlip(void);
EXTERNCPP void GLUICompressOnOff(int flag);
EXTERNCPP void GLUIUpdateColorbarListBound(int flag);
EXTERNCPP void GLUIUpdateColorbarBound(void);
EXTERNCPP void GLUIUpdateOverwrite(void);
EXTERNCPP void GLUIUpdatePlot3dDisplay(void);
EXTERNCPP void GLUIUpdateSortSlices(void);
EXTERNCPP void GLUIUpdateExtreme(void);
EXTERNCPP void GLUISliceBoundCB(int var);
EXTERNCPP void GLUIUpdateChar(void);
EXTERNCPP void GLUIUpdateTracers(void);
EXTERNCPP void GLUIUpdateShowHideButtons(void);
#ifdef pp_REFRESH
EXTERNCPP void GLUIRefreshDialogs(void);
#endif
EXTERNCPP void GLUIUpdateVectorpointsize(void);
EXTERNCPP void GLUIUpdateSliceDupDialog(void);
EXTERNCPP void SetLoadedSliceBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPatchBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPlot3DBounds(void);
EXTERNCPP void SetLoadedPartBounds(int *list, int nlist);
EXTERNCPP void ScriptCB(int var);
EXTERNCPP void PartBoundCB(int var);
EXTERNCPP void UpdateColorbarSelectionIndex(int val);
EXTERNCPP void SliceBounds2Glui(int slicefile_labelindex);
EXTERNCPP void UpdateShowExtPatch(int show_option, int hide_option);
EXTERNCPP void UpdateShowIntPatch(int show_option, int hide_option);


#endif

