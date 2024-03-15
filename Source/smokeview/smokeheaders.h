#ifndef SMOKEHEADERS_H_DEFINED
#define SMOKEHEADERS_H_DEFINED

#ifdef pp_LUA
#include "gd.h"
#endif

//*** threader headers
EXTERNCPP void *CheckFiles(void *arg);
EXTERNCPP void *ClassifyAllGeom(void *arg);
EXTERNCPP void *Compress(void *arg);
EXTERNCPP void *SetupFF(void *arg);
EXTERNCPP void *MtLoadAllPartFiles(void *arg);
EXTERNCPP void *GetGlobalPatchBoundsFull(void *arg);
EXTERNCPP void *PlayMovie(void *arg);
EXTERNCPP void *ReadAllGeom(void *arg);
EXTERNCPP void *GetGlobalSliceBoundsFull(void *arg);
EXTERNCPP void *UpdateTrianglesAll(void *arg);
EXTERNCPP void *ReadVolsmokeAllFramesAllMeshes2(void *arg);
EXTERNCPP void *SetupAllIsosurfaces(void *arg);

//*** glui_bounds.cpp headers
EXTERNCPP void GLUIUpdateMeshBounds(void);
EXTERNCPP void DrawBoxMinMax(float *bbmin, float *bbmax, float *box_color);
EXTERNCPP void DrawBox(float *bb, float *box_color);
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
EXTERNCPP void GLUIGetOnlyMinMax(int type, char* label, int* set_valmin, float* valmin, int* set_valmax, float* valmax);
EXTERNCPP void GLUIGetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int *nall);
EXTERNCPP void GLUISetMin(int type, char *label, int set_valmin, float valmin);
EXTERNCPP void GLUISetMax(int type, char *label, int set_valmax, float valmax);
EXTERNCPP void GLUISetMinMax(int type, char *label, int set_valmin, float valmin, int set_valmax, float valmax);
EXTERNCPP void GLUISetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int nall);
#ifdef pp_BOUNDS
EXTERNCPP void GLUISetGlobalMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUISetLoadedMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUIGetGlobalMinMaxAll(int type, float *valmin, float *valmax, int nall);
EXTERNCPP void GLUIGetLoadedMinMaxAll(int type, float *valmin, float *valmax, int nall);
#endif
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
EXTERNCPP void UpdateTriangles(int time_flag, int update);
EXTERNCPP void CancelUpdateTriangles(void);
#ifdef pp_REFRESH
EXTERNCPP void GLUIRefreshDialogs(void);
#endif
EXTERNCPP void GLUIUpdateVectorpointsize(void);
EXTERNCPP void GLUIUpdateSliceDupDialog(void);

//*** glui_clip.cpp headers
EXTERNCPP void GLUIClipSetup(int main_window);
EXTERNCPP void GLUIUpdateClip(void);
EXTERNCPP void GLUIShowClip(void);
EXTERNCPP void GLUIHideClip(void);
EXTERNCPP void GLUIUpdateClipAll(void);

//*** glui_colorbar.cpp headers
EXTERNCPP void GLUIColorbarSetup(int main_window);
EXTERNCPP void GLUIUpdateColorbarList(void);
EXTERNCPP void GLUIColorbarCB(int var);
EXTERNCPP void GLUIColorbarGlobal2Local(void);
EXTERNCPP void GLUIShowColorbar(void);
EXTERNCPP void GLUIHideColorbar(void);
EXTERNCPP void GLUISetColorbarListEdit(int val);
EXTERNCPP void GLUIUpdateColorbarListEdit(int flag,int del);
EXTERNCPP void GLUIUpdateColorbarEdit(void);
EXTERNCPP void GLUIUpdateColorbarType(void);

//*** glui_display.cpp headers
EXTERNCPP void GLUIDisplaySetup(int main_window);
EXTERNCPP void GLUIUpdateGridLocation(void);
EXTERNCPP void GLUIUpdateFrameTimelabel(void);
EXTERNCPP void GLUIUpdateColorbarControls(void);
EXTERNCPP void GLUIUpdateBackgroundFlip(int flip);
EXTERNCPP void GLUIUpdateBackgroundFlip2(int flip);
EXTERNCPP void GLUIUpdateTimebarOverlap(void);
EXTERNCPP void GLUIUpdateVisAxisLabels(void);
EXTERNCPP void GLUIUpdateFontIndex(void);
EXTERNCPP void GLUIShowDisplay(int menu_id);
EXTERNCPP void GLUISetLabelControls(void);
EXTERNCPP void GLUIHideDisplay(void);
EXTERNCPP void GLUILabelsCB(int value);

//*** glui_shooter.cpp headers
EXTERNCPP void GLUIShooterSetup(int main_window);
EXTERNCPP void GLUIShowShooter(void);
EXTERNCPP void GLUIHideShooter(void);

//*** glui_geometry.cpp headers
EXTERNCPP void GLUIGeometrySetup(int main_window);
EXTERNCPP void GLUIUpdateBlockVals(int flag);
EXTERNCPP void GLUIUpdateTerrain(void);
EXTERNCPP void GLUIObjectCB(int flag);
EXTERNCPP void GLUIUpdateHVACViews(void);
EXTERNCPP void GLUIShowGeometry(void);
EXTERNCPP void GLUIShowTerrain(void);
EXTERNCPP void GLUIHideTerrain(void);
EXTERNCPP void GLUIHideHVAC(void);
EXTERNCPP void GLUIShowHVAC(void);
EXTERNCPP void GLUIHVAC2Glui(int index);
EXTERNCPP void GLUIUpdateTerrainTexture(int val);
EXTERNCPP void GLUIUpdateGeomBoundingBox(void);
EXTERNCPP void GLUIUpdateSelectGeom(void);
EXTERNCPP void GLUIUpdateTriangleInfo(surfdata *tri_surf, float tri_area);
EXTERNCPP void GLUIUpdateVertexInfo(float *xyz1, float *xyz2);
EXTERNCPP void GLUIUpdateWhereFaceVolumes(void);
EXTERNCPP void GLUIGetGeomDialogState(void);
EXTERNCPP void GLUIUpdateGeometryControls(void);
EXTERNCPP void GLUIUpdateHVACVarLists(void);
EXTERNCPP void GLUIHideGeometry(void);

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
EXTERNCPP void GLUIUpdateSetViewXYZ(float *xyz);
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

//*** glui_objects.cpp headers
EXTERNCPP void GLUIDeviceCB(int val);
EXTERNCPP void GLUIDeviceSetup(int main_window);
EXTERNCPP void GLUIPlot2DSetup(int main_window);
EXTERNCPP void GLUIUpdatePlot2DTbounds(void);
EXTERNCPP void GLUIUpdatePlot2DINI(void);
EXTERNCPP void GLUIShowPlot2D(void);
EXTERNCPP void GLUIHidePlot2D(void);
EXTERNCPP void GLUIShowDevice(void);
EXTERNCPP void GLUIShowPlotDevice(void);
EXTERNCPP void GLUIHideDevice(void);
EXTERNCPP void GLUIUpdateDeviceSize(void);
EXTERNCPP void GLUIUpdateDeviceOrientation(void);
EXTERNCPP void GLUIUpdateDevices(void);
EXTERNCPP void GLUIUpdateDeviceTypes(int val);
EXTERNCPP void GLUIUpdateDeviceShow(void);
EXTERNCPP void GLUIUpdateWindRoseDevices(int option);
EXTERNCPP void GLUIUpdateShowbeamAsLine(void);
SVEXTERN void GLUIUpdatePlot2DSize(void);
EXTERNCPP void GLUIUpdateDeviceAdd(void);

//*** glui_smoke.cpp headers
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
#ifdef pp_SMOKE16
EXTERNCPP void GLUIUpdateSmoke16(void);
#endif
EXTERNCPP void GLUIUpdateSmoke3dFlags(void);
EXTERNCPP void GLUICreateVolTourList(void);
EXTERNCPP void GLUIDeleteVolTourList(void);

//*** glui_stereo.cpp headers
EXTERNCPP void GLUIStereoSetup(int main_window);
EXTERNCPP void GLUIUpdateStereo(void);
EXTERNCPP void GLUIShowStereo(void);
EXTERNCPP void GLUIHideStereo(void);

EXTERNCPP float GetTime(void);
EXTERNCPP void StartTimer(float *timerptr);

//*** glui_tour.cpp headers
EXTERNCPP void GLUITourSetup(int main_window);
EXTERNCPP void GLUIShowTour(void);
EXTERNCPP void GLUIHideTour(void);
EXTERNCPP void GLUIUpdateTourState(void);
EXTERNCPP void GLUIUpdateTourParms(void);
EXTERNCPP void GLUIAddNewTour(void);
EXTERNCPP void GLUIUpdateTourList(void);
EXTERNCPP void GLUIUpdateTourIndex(void);
EXTERNCPP void GLUICreateTourList(void);
EXTERNCPP void GLUIDeleteTourList(void);
EXTERNCPP void GLUIUpdateTourControls(void);
EXTERNCPP void GLUISetTourKeyframe(void);
EXTERNCPP void GLUIUpdateKeyframe(void);

//*** glui_trainer.cpp headers
EXTERNCPP void GLUIUpdateTrainerOutline(void);
EXTERNCPP void GLUIUpdateTrainerMoves(void);
EXTERNCPP void GLUISetViewListManual(void);
EXTERNCPP void GLUITrainerSetup(int main_window);
EXTERNCPP void GLUIAlertSetup(int main_window);
EXTERNCPP void GLUIShowAlert(void);
EXTERNCPP void GLUIHideAlert(void);
EXTERNCPP void GLUIShowTrainer(void);
EXTERNCPP void GLUIHideTrainer(void);

// gen plot routines

EXTERNCPP void PrintFileLoadTimes(int file_count, FILE_SIZE load_size, float load_time);

EXTERNCPP int HavePlot2D(float **times, int *ntimes);
EXTERNCPP char *GetPlotUnit2(plot2ddata *plot2di, curvedata *curve);
EXTERNCPP char *GetPlotShortLabel2(plot2ddata *plot2di, curvedata *curv);
EXTERNCPP void GetPlot2DBounds(plot2ddata*plot2di, float *valmin, float *valmax);
EXTERNCPP char *GetPlotUnit(plot2ddata * plot2di, int curv_index);
EXTERNCPP void GenPlotCB(int var);
EXTERNCPP void UpdateCurveControls(char *unit);
EXTERNCPP void InitPlot2D(plot2ddata *plot2di, int plot_index);
EXTERNCPP void DrawGenPlots(void);
EXTERNCPP char *GetPlotShortLabel(plot2ddata *plot2di, int curv_index);
EXTERNCPP csvdata *GetCsvData(int file_index, int col_index, csvfiledata **csvf_ptr);
EXTERNCPP csvdata *GetCsvCurve(int i, csvfiledata **csvf_ptr);
EXTERNCPP void SetupPlot2DUnitData(void);

EXTERNCPP void TimeAveragePlot2DData(float *times, float *vals, float *vals_avg, int nvals, float time_interval);

EXTERNCPP void SaveScreenOrigin(int x, int y);
EXTERNCPP int GetScreenOrigin(int *x, int *y);

EXTERNCPP void SetHVACInfo(void);
EXTERNCPP void DrawHVACS(void);
EXTERNCPP hvacnodedata *GetHVACNode(hvacdata *hvaci, int node_id);
EXTERNCPP void InitHvacData(hvacvaldata *hi);
EXTERNCPP void ReadHVACData(int flag);
EXTERNCPP int IsHVACVisible(void);
EXTERNCPP void UpdateHVACDuctColorLabels(int index);
EXTERNCPP void UpdateNodeLabel(colorbardata *cbi);
EXTERNCPP int IsColorbarSplit(colorbardata *cbi);

EXTERNCPP void UpdateHVACNodeColorLabels(int index);
EXTERNCPP void UpdateAllHVACColorLabels(void);

#ifdef pp_SLICE_MENU_DEBUG
EXTERNCPP void PrintSliceInfo(void);
#endif

#ifdef pp_REFRESH
EXTERNCPP void PeriodicRefresh(int var);
#endif

EXTERNCPP void GLUIUpdateShowOnlyTop(void);

EXTERNCPP void GeneratePartHistograms(void);
FILE_SIZE LoadAllMSlicesMT(int last_slice, multislicedata *mslicei, int *fcount);

EXTERNCPP void PrintTime(const char *tag, int line, float *timer, const char *label, int stop_flag);

EXTERNCPP void DrawObstBoundingBox(void);
EXTERNCPP void DrawGeomBoundingBox(float *boundingbox_color);
EXTERNCPP void ClassifyGeom(geomdata *geomi, int *geom_frame_index);

EXTERNCPP void GLUIUpdateCfaces(void);

EXTERNCPP void SetTimeState(void);

EXTERNCPP int GetGeomDataSize(char *file, int *nvals, int time_frame,
              int *cvals_offsets, int *cvals_sizes, int *geom_offsets, int *geom_offset_flag,
              int *max_buffer_size, int *error);
EXTERNCPP FILE_SIZE GetGeomData(char *filename, int ntimes, int nvals, float *times,
              int *nstatics, int *ndynamics, float *vals, int time_frame, float *time_value,
              int *geom_offsets, int *error);

EXTERNCPP void ShowObjectsMenu(int var);
EXTERNCPP void UpdateShowHRRPUVPlot(int val);

EXTERNCPP void ScriptViewXYZMINMAXOrtho(int option);
EXTERNCPP void SetCameraView(cameradata *ca, int option);
EXTERNCPP void SetCameraViewPersp(cameradata *ca, int option);

EXTERNCPP void InitStartupDirs(void);

EXTERNCPP int GetFontHeight(void);

EXTERNCPP void LoadAllMultiSliceMenu(void);
EXTERNCPP void LoadAllMultiVSliceMenu(void);

EXTERNCPP void UpdateShowColorbar(int *showcfast_arg, int *show_slice_colorbar_arg, 
  int *show_hvacduct_colorbar_arg, int *show_hvacnode_colorbar_arg);

EXTERNCPP void DrawPlot2D(int option, float *x, float *z, float *z2, int n,
  float highlight_x, float highlight_y, float highlight_y2, int valid, int position,
  float global_valmin, float global_valmax, char *quantity, char *quantity2, char *unit,
  float left, float right, float down, float top);

EXTERNCPP void MergePlot3DHistograms(void);
EXTERNCPP void GetPlot3DHists(plot3ddata *p);

EXTERNCPP void UpdateHoc(void);

EXTERNCPP int GetStringWidth(char *string);
EXTERNCPP void DrawDevicePlots(void);
EXTERNCPP void DrawTreeDevicePlots(void);
EXTERNCPP int GetPlotState(int choice);

#ifndef TERRAIN_FIRE_LINE_UPDATE
#define TERRAIN_FIRE_LINE_UPDATE 39
#endif

EXTERNCPP FILE_SIZE LoadVSliceMenu2(int val);

EXTERNCPP void UpdateClipbounds(int set_i0, int *i0, int set_i1, int *i1, int maxi);
EXTERNCPP void ClipCB(int var);
EXTERNCPP int CompareFloat(const void *arg1, const void *arg2);
EXTERNCPP void ResetItimes0(void);
EXTERNCPP void UpdateShow(void);
EXTERNCPP void SynchTimes(void);
EXTERNCPP void UpdateTimes(void);
EXTERNCPP int GetIndex(float key, const float *list, int nlist);
EXTERNCPP int ISearch(float *list, int nlist, float key, int guess);

EXTERNCPP int GLUTGetScreenWidth(void);
EXTERNCPP int GLUTGetScreenHeight(void);

EXTERNCPP void HideSlices(char *longlabel);

EXTERNCPP void OutputMinMax(char *meshlabel, char *label, char *unit, float valmin_fds, float valmax_fds, float valmin_smv, float valmax_smv);

EXTERNCPP void ScriptLoadSliceRender(scriptdata*scripti);
EXTERNCPP int GetSmokeNFrames(int type, float *tmin, float *tmax);
EXTERNCPP void ScriptLoadSmokeRender(scriptdata *scripti);
EXTERNCPP void SmokeWrapup(void);
FILE_SIZE LoadSmoke3D(int type, int frame, int *count, float *time_value);
EXTERNCPP int GetNSliceFrames(char *file, float *stime_min, float *stime_max);
EXTERNCPP void GenerateSliceMenu(int from_commandline);
void DrawTerrainGeom(int option);
void GenerateTerrainGeom(float **vertices_arg, unsigned int **indices_arg, int *nindices_arg);
#ifdef pp_DECIMATE
EXTERNCPP void DecimateAllTerrains(void);
#endif

EXTERNCPP int GetPartHeader(partdata *parti, int *nf_all, int option_arg, int print_option_arg);
EXTERNCPP void SetMainWindow(void);
EXTERNCPP void GetSliceFileHeader(char *file, int *ip1, int *ip2, int *jp1, int *jp2, int *kp1, int *kp2, int *error);
EXTERNCPP int TimeAverageData(float *data_out, float *data_in, int ndata, int data_per_timestep, float *times_local, int ntimes_local, float average_time);
bufferstreamdata *GetSMVBuffer(char *file, char *file2);
EXTERNCPP void UpdateBlockType(void);
boundsdata *GetSliceBoundsInfo(char *shortlabel);
boundsdata *GetPatchBoundsInfo(char *shortlabel);
EXTERNCPP int GetGlobalPartBounds(int flag);
EXTERNCPP void GetGlobalPatchBounds(int flag, int set_flag);
EXTERNCPP void GetGlobalPatchBoundsReduced(void);
EXTERNCPP void GetLoadedPlot3dBounds(int *compute_loaded, float *loaded_min, float *loaded_max);
EXTERNCPP void GetGlobalPlot3DBounds(void);
EXTERNCPP void GetGlobalSliceBounds(int flag, int set_flag);
EXTERNCPP int IsFDSRunning(FILE_SIZE *last_size);
EXTERNCPP int BuildGbndFile(int file_type);

EXTERNCPP void GetGlobalSliceBoundsReduced(void);
EXTERNCPP void GetGlobalHVACDuctBounds(int flag);
EXTERNCPP void GetGlobalHVACNodeBounds(int flag);
#ifdef pp_FED
EXTERNCPP void UpdateGlobalFEDSliceBounds(void);
#endif

EXTERNCPP void Slice2Device(void);
EXTERNCPP void PauseTime(float pause_time);


EXTERNCPP int InMesh(float *xyz);
EXTERNCPP void UpdateVectorSkip(int skip);
EXTERNCPP char *GetHomeDir(void);
EXTERNCPP void SetPercentileDrawOff(void);
EXTERNCPP void SetPercentilePartBounds(void);
EXTERNCPP void SetPercentilePlot3DBounds(void);
EXTERNCPP void ClosePartFiles(void);
EXTERNCPP void UpdatePartColors(partdata *parti, int flag);
EXTERNCPP void MergeLoadedSliceHist(char *label, histogramdata **histptr);
EXTERNCPP void ComputeLoadedPatchHist(char *label, histogramdata **histptr, float *valmin, float *valmax);
EXTERNCPP void GetHistogramValProc(histogramdata *histogram, float cdf, float *val);
EXTERNCPP void ComputeLoadedSliceHist(char *label, float valmin, float valmax);
EXTERNCPP void SetLoadedSliceBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPatchBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPlot3DBounds(void);
EXTERNCPP void SetLoadedPartBounds(int *list, int nlist);

EXTERNCPP void ShiftColorbars(void);
EXTERNCPP int GetColorbarState(void);
EXTERNCPP void SMV_EXIT(int code);
EXTERNCPP void DrawSelectGeom(void);
EXTERNCPP void UpdateGeomAreas(void);
EXTERNCPP void GetZoneTempBounds(void);
EXTERNCPP FILE_SIZE GetSliceData(slicedata *sd, const char *slicefilename, int time_frame, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr,
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int tload_step_arg, int tload_beg_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg
);
EXTERNCPP void GetSliceSizes(const char *slicefilenameptr, int time_frame, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int tload_step_arg,
  int *errorptr, int tload_beg_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr);
EXTERNCPP void PrintPartLoadSummary(int option, int type);
EXTERNCPP void CreatePartSizeFile(partdata *parti);
EXTERNCPP void GetAllPartBounds(void);
EXTERNCPP void MergeAllPartBounds(void);
#ifdef CPP
EXTERNCPP void InsertRollout(GLUI_Rollout *rollout, GLUI *dialog);
#endif

EXTERNCPP void InitializeDeviceCsvData(int flag);
EXTERNCPP FILE_SIZE ReadAllCSVFiles(int flag);
EXTERNCPP FILE_SIZE ReadCSVFile(csvfiledata *csvfi, int flag);
EXTERNCPP void UpdateCSVFileTypes(void);
EXTERNCPP int HaveFireLoaded(void);
EXTERNCPP int HaveSootLoaded(void);
EXTERNCPP void MakeColorLabels(char colorlabels[12][11], float colorvalues[12], float tmin_arg, float tmax_arg, int nlevel);
EXTERNCPP void FinalizePartLoad(partdata *parti);
EXTERNCPP void LoadAllPartFilesMT(int val);
EXTERNCPP void LoadAllPartFiles(int partnum);
EXTERNCPP void CreatePartBoundFile(partdata *parti);
EXTERNCPP void InitRolloutList(void);
EXTERNCPP void GetKeyXYZ(float t, keyframe *this_key, float *xyz);
EXTERNCPP void GetTourXYZ(float t, tourdata *this_tour, float *xyz);
EXTERNCPP void GetKeyView(float t, keyframe *this_key, float *view);
EXTERNCPP void GetTourView(float t, tourdata *this_tour, float *view);
EXTERNCPP int GetTourFrame(tourdata *touri, int itime);
EXTERNCPP int MeshConnect(meshdata *mesh_from, int val, meshdata *mesh_to);
EXTERNCPP int IsBottomMesh(meshdata *mesh_from);
EXTERNCPP void InitNabors(void);
EXTERNCPP int Smv2Html(char *html_out, int option, int from_where);
EXTERNCPP int Smv2Geom(char *html_file);
EXTERNCPP int Obst2Data(char *html_file);
EXTERNCPP int SliceNode2Data(char *html_file, int option);
EXTERNCPP int SliceCell2Data(char *html_file, int option);
EXTERNCPP void UpdateTexturebar(void);
EXTERNCPP void GetFileSizes(void);
EXTERNCPP int IsSmokeComponentPresent(smoke3ddata *smoke3di);
EXTERNCPP void GetSliceDataBounds(slicedata *sd, float *pmin, float *pmax);
EXTERNCPP void UpdateAllSliceColors(int slicetype, int *errorcode);
EXTERNCPP void UpdateSliceBounds(void);
EXTERNCPP FILE_SIZE ReadGeomData(patchdata *patchi, slicedata *slicei, int load_flag, int time_frame, float *time_value, int flag, int *errorcode);
EXTERNCPP void ResetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high);
EXTERNCPP void GetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high);
EXTERNCPP void UpdateSmoke3dFileParms(void);
EXTERNCPP void SkipMenu(int value);
EXTERNCPP void InitScriptErrorFiles(void);
EXTERNCPP void UpdateRenderListSkip(void);
EXTERNCPP void ForceIdle(void);
EXTERNCPP void UpdateFrameNumber(int changetime);
EXTERNCPP void UpdateVentOffset(void);
EXTERNCPP void UpdateOpacityMap(void);
EXTERNCPP int  GetVolFrameMax(int meshnum);
EXTERNCPP void UpdateGluiRotateAbout(int val);
EXTERNCPP void ReloadAllSliceFiles(void);
EXTERNCPP void ReloadAllVectorSliceFiles(void);
EXTERNCPP void UnloadAllSliceFiles(char *longlabel);
EXTERNCPP void ParticleStreakShowMenu(int var);
EXTERNCPP void UpdateGeomNormals();
EXTERNCPP void Plot3DListMenu(int value);
EXTERNCPP void InitShooterData(void);
EXTERNCPP void ScriptCB(int var);
EXTERNCPP void TourCB(int var);
EXTERNCPP void SetClipControls(int val);
EXTERNCPP void PartBoundCB(int var);
EXTERNCPP void ShowHideMenu(int val);
EXTERNCPP void UpdateVSliceDups(void);
EXTERNCPP void UnloadVSliceMenu(int value);
EXTERNCPP void UpdateSliceDups(sliceparmdata *sp);
EXTERNCPP void GetSliceParmInfo(sliceparmdata *sp);
EXTERNCPP void SetSliceParmInfo(sliceparmdata *sp);

EXTERNCPP void UpdateSmokeAlphas(void);
EXTERNCPP void InitAlphas(unsigned char *alphanew,
                          float base_extinct, float new_extinct,
                          float base_dx, float new_dx);
EXTERNCPP void ShowDevicesMenu(int value);
EXTERNCPP int GetDeviceIndexFromLabel(char *label);
EXTERNCPP void UpdateBoundarySliceDups(void);
EXTERNCPP void UnLoadVolsmoke3DMenu(int value);
EXTERNCPP void UpdateMenu(void);
EXTERNCPP void GeometryMenu(int var);
#ifdef pp_RENDER360_DEBUG
EXTERNCPP void DrawScreenInfo(void);
#endif
EXTERNCPP void GetGeomZBounds(float *zmin, float *zmax);
EXTERNCPP void DrawNorth(void);
EXTERNCPP void DrawGeomValues(slicedata *sd, patchdata *patchi, int geom_type);
EXTERNCPP void DrawGeomData(int flag, slicedata *sd, patchdata *patchi, int geom_type);
EXTERNCPP void DrawGeomVData(vslicedata *vd);
EXTERNCPP void UpdateCurrentColorbar(colorbardata *cb);
EXTERNCPP void UpdateObjectUsed(void);
EXTERNCPP void UpdateColorTable(colortabledata *ctableinfo, int nctableinfo);
EXTERNCPP colortabledata *GetColorTable(char *label);
EXTERNCPP int GetColorTableIndex(int *color);
EXTERNCPP void ReadIsoGeomWrapup(int flag);
EXTERNCPP char *GetMovieFilePath(char *moviefile_path);
EXTERNCPP int GetNumActiveDevices(void);
EXTERNCPP void EnableDisableMakeMovie(int onoff);
EXTERNCPP void EnableDisablePlayMovie(void);
EXTERNCPP void HandleMakeMovie(void);
EXTERNCPP void UpdateRenderStartButton(void);
EXTERNCPP void MakeMovie(void);
EXTERNCPP void UpdateRenderType(int type);
EXTERNCPP void UpdateMovieType(int type);
EXTERNCPP void UpdateDisplay(void);
EXTERNCPP void UpdateShowScene(void);
EXTERNCPP void DrawGravityAxis(void);
EXTERNCPP void XYZ2AzElev(float *xyz,float *azimuth, float *elevation);
EXTERNCPP void UpdateColorDevices(void);
EXTERNCPP void InitVolrenderScript(char *prefix, char *tour_label, int startframe, int skipframe);

EXTERNCPP void UpdateGluiBoundaryUnits(void);
EXTERNCPP void UpdateGluiSliceUnits(void);
EXTERNCPP void HVACMenu(int value);
EXTERNCPP int GetHVACDuctValIndex(char *shortlabel);
EXTERNCPP int GetHVACNodeValIndex(char *shortlabel);
EXTERNCPP void ToggleMetroMode(void);
EXTERNCPP void UpdateClipPlanes(void);
EXTERNCPP hvacductdata *GetHVACDuctID(char *duct_name);
EXTERNCPP hvacnodedata *GetHVACNodeID(char *node_name);
EXTERNCPP void GetCellXYZs(float *xyz, int nxyz, int ncells, float **xyz_cellptr, int *nxyz_cell, int **cell_indptr);
EXTERNCPP void HVACDuctValueMenu(int value);
EXTERNCPP void HVACNodeValueMenu(int value);

EXTERNCPP void UpdateAllBoundaryColors(int flag);

EXTERNCPP void GetBoundaryParams(void);
EXTERNCPP void GetSliceParams2(void);

EXTERNCPP void DrawWindRosesDevices(void);
EXTERNCPP void DeviceData2WindRose(int nr, int ntheta);
#ifdef pp_FED
EXTERNCPP void DefineAllFEDs(void);
#endif
EXTERNCPP void AddDeleteKeyframe(int flag);
EXTERNCPP void Slerp(float *p0, float *p1, float t, float *pout);
EXTERNCPP void VentMenu(int value);
EXTERNCPP void UpdateAllSliceLabels(int slicetype, int *errorcode);

EXTERNCPP void MergeClipPlanes(clipdata *ci, clipdata *cj);
EXTERNCPP void InitBoxClipInfo(clipdata *ci,float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
EXTERNCPP void InitTetraClipInfo(clipdata *ci,float *v1, float *v2, float *v3, float *v4);
EXTERNCPP void InitClip(void);
EXTERNCPP void SetClipPlanes(clipdata *ci, int option);

EXTERNCPP void DrawFilledTetra(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor);
EXTERNCPP void DrawFilled2Tetra(float *v1, float *v2, float *v3, float *v4,
   unsigned char *rgb0color,unsigned char *rgb1color,unsigned char *rgb2color,unsigned char *rgb3color,int *vis_state);
EXTERNCPP void DrawTetraOutline(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor);
EXTERNCPP void DrawFilledCircle(float diameter, unsigned char *rgbcolor, circdata *circinfo);
EXTERNCPP void DrawCircle(float diameter, unsigned char *rgbcolor, circdata *circinfo);
EXTERNCPP void DrawFilledRectangle(float width, float height, unsigned char *rgbcolor);
EXTERNCPP void DrawRectangle(float width, float height, unsigned char *rgbcolor);
EXTERNCPP void DrawCircVents(int option);
EXTERNCPP void UpdateSmokeColormap(int option);
EXTERNCPP void UpdateCO2Colormap(void);
EXTERNCPP void DefineVolsmokeTextures(void);
EXTERNCPP int  GetColorbarIndex(int flag, int x, int y);
EXTERNCPP void GetViewportInfo(void);

EXTERNCPP void ScaleFont2D(void);
EXTERNCPP void ScaleFont3D(void);
EXTERNCPP int  LabelGetNUserLabels(void);
EXTERNCPP labeldata *LabelNext(labeldata *gl);
EXTERNCPP labeldata *LabelPrevious(labeldata *gl);
EXTERNCPP int  LabelInit(labeldata *gl);
EXTERNCPP void LabelResort(labeldata *label);
EXTERNCPP void LabelCopy(labeldata *label_to, labeldata *label_from);
EXTERNCPP labeldata *LabelGet(char *name);
EXTERNCPP void LabelDelete(labeldata *label);
EXTERNCPP void LabelPrint(void);
EXTERNCPP labeldata *LabelInsert(labeldata *labeltemp);

EXTERNCPP void RotateU2V(float *u, float *v, float *axis, float *angle);

EXTERNCPP void AngleAxis2Quat(float angle, float *axis, float *quat);
EXTERNCPP void Quat2Rot(float quat[4],float rot[16]);
EXTERNCPP void MultQuat(float x[4], float y[4], float z[4]);

EXTERNCPP void SetScreenSize(int *width, int *height);
EXTERNCPP void KeyboardCB(unsigned char key, int x, int y);
EXTERNCPP void KeyboardUpCB(unsigned char key, int x, int y);
EXTERNCPP void ReshapeCB(int width, int height);
EXTERNCPP void DisplayCB(void);
EXTERNCPP void SpecialKeyboardCB(int key, int x, int y);
EXTERNCPP void SpecialKeyboardUpCB(int key, int x, int y);
EXTERNCPP void MouseCB(int button, int state, int x, int y);
EXTERNCPP void MouseDragCB(int xm, int ym);
EXTERNCPP void MenuStatusCB(int status, int x, int y);
EXTERNCPP void IdleCB(void);

EXTERNCPP void ReadIsoOrig(const char *file, int ifile, int flag, int *errorcode);
EXTERNCPP void UnloadIso(meshdata *meshi);
EXTERNCPP void UpdatePlotxyzAll(void);
EXTERNCPP void UpdateIsoColors(void);
EXTERNCPP void GetFaceInfo(void);
EXTERNCPP void GetGeomInfoPtrs(int flag);
EXTERNCPP devicedata *GetDeviceFromLabel(char *label, int index);
EXTERNCPP devicedata *GetCSVDeviceFromLabel(char *label, int index);
EXTERNCPP void SetupGlut(int argc, char **argv);
EXTERNCPP int GetNDevices(char *file);
EXTERNCPP void ReadHRR(int flag);
EXTERNCPP FILE_SIZE ReadDeviceData(char *file, int filetype, int flag);
EXTERNCPP void SetupZoneDevs(void);

EXTERNCPP void DrawCGeom(int flag, geomdata *cgeom);
EXTERNCPP void DrawGeom(int flag,int frameflag);
EXTERNCPP void RemoveDupBlockages(void);
EXTERNCPP void SortIsoTriangles(float *mm);
EXTERNCPP void UpdateIsoTriangles(int flag);
EXTERNCPP void UpdateSliceMenuShow(sliceparmdata *sp);
EXTERNCPP void UpdateHideBoundarySurface(void);
EXTERNCPP int  LastSliceLoadstack(void);
EXTERNCPP int  LastVSliceLoadstack(void);
EXTERNCPP void UpdateDefer(void);
EXTERNCPP void SetTimeVal(float timeval);
EXTERNCPP void GetIndepVarIndices(sv_object *smv_object,char **var_indep_strings, int nvars_indep,int *index);

EXTERNCPP void LoadSmokeFrame(int meshnum, int framenum);
EXTERNCPP void LoadTimeFrame(int meshnum, float timeval);
EXTERNCPP void ScriptLoadVolSmokeFrame2(void);
EXTERNCPP void ScriptLoadIsoFrame2(scriptdata *scripti);
EXTERNCPP void InitDevicePlane(devicedata *devicei);
EXTERNCPP void DrawDevicesVal(void);
EXTERNCPP void GetSmokeSensors(void);
EXTERNCPP void StartScript(void);
EXTERNCPP int RunScriptCommand(scriptdata *script_command);
EXTERNCPP void DoScriptHtml(void);
EXTERNCPP int  CompileScript(char *scriptfile);
EXTERNCPP scriptfiledata *InsertScriptFile(char *file);
EXTERNCPP char *GetIniFileName(int id);
EXTERNCPP char *GetScriptFileName(int id);
EXTERNCPP inifiledata *InsertIniFile(char *file);
EXTERNCPP void Keyboard(unsigned char key, int flag);
EXTERNCPP void GetNewScriptFileName(char *newscriptfilename);
#ifdef pp_FED
EXTERNCPP void OutputFedCSV(void);
#endif
EXTERNCPP void ParticlePropShowMenu(int value);
EXTERNCPP int  GetGridIndex(float x, int dir, float *plotxyz, int nplotxyz);
EXTERNCPP void UpdateSliceContours(int slice_type_index, float line_min, float line_max, int nline_values);
EXTERNCPP void ScriptMenu(int var);
EXTERNCPP void SmokeColorbarMenu(int var);
EXTERNCPP void UpdateTerrainColors(void);
EXTERNCPP void DrawTerrainOBST(terraindata *terri, int flag);
EXTERNCPP void DrawTerrainOBSTTexture(terraindata *terri);
EXTERNCPP void DrawTerrainOBSTSides(meshdata *meshi);
EXTERNCPP void DrawTrees(void);
EXTERNCPP void InitCullGeom(int cullflag);
EXTERNCPP void GetCullSkips(meshdata *meshi, int cullflag, int cull_portsize, int *iiskip, int *jjskip, int *kkskip);
#ifdef pp_GPU
EXTERNCPP int  InitShaders(void);
EXTERNCPP void LoadSmokeShaders(void);
EXTERNCPP void Load3DSliceShaders(void);
EXTERNCPP void LoadZoneSmokeShaders(void);
EXTERNCPP void LoadVolsmokeShaders(void);
EXTERNCPP void UnLoadShaders(void);
#endif
EXTERNCPP void NextXIndex(int inc,int flag);
EXTERNCPP void NextYIndex(int inc,int flag);
EXTERNCPP void NextZIndex(int inc,int flag);
EXTERNCPP void InitSphere(int nlat, int nlong);
EXTERNCPP void InitCircle(unsigned int npoints, circdata *circinfo);
EXTERNCPP int  HaveTerrainSlice(void);
EXTERNCPP int HaveSmokeSensor(void);
EXTERNCPP int GetSmoke3DType(char *label);
EXTERNCPP float GetZCellValOffset(meshdata *meshi,float xval, float yval, int *loc);
EXTERNCPP void UpdateCameraYpos(cameradata *camera_data, int option);
EXTERNCPP cameradata *GetCamera(char *name);
EXTERNCPP void SortCameras(void);
EXTERNCPP void SortCamerasID(void);
EXTERNCPP char *GetCameraLabel(int index);
EXTERNCPP void Clip2Cam(cameradata *cam);
EXTERNCPP void Cam2Clip(cameradata *cam);
EXTERNCPP void InitObjectDefs(void);
EXTERNCPP void UpdateDeviceTextures(void);
EXTERNCPP char *GetDeviceLabel(char *buffer);
EXTERNCPP void GetElevAz(float *xyznorm,float *dtheta, float *rotate_axis, float *dpsi);
EXTERNCPP void DrawDevices(int mode);
EXTERNCPP sv_object *InitSmvObject1(char *label, char *commands,int visible);
EXTERNCPP sv_object *InitSmvObject2(char *label, char *commandson, char *commandsoff,int visible);
EXTERNCPP sv_object *GetSmvObjectType(char *label, sv_object *default_object);
EXTERNCPP sv_object *GetSmvObjectType2(char *label, sv_object *default_object);
EXTERNCPP void FreeAllObjects(void);
EXTERNCPP void UpdatePartClassDepend(partclassdata *partclassi);

EXTERNCPP void CheckLab(void);

EXTERNCPP void AdjustColorBar(colorbardata *cbi);
EXTERNCPP void AdjustColorBarLab(colorbardata *cbi);
EXTERNCPP void RevertColorBar(colorbardata *cbi);
EXTERNCPP void Rgb2Lab(unsigned char *rgb, float *lab);
EXTERNCPP void Rgb2Labs(unsigned char *rgbs255, float *labs);
EXTERNCPP void UpdateColorbarOrig(void);
EXTERNCPP void Lab2Rgb(unsigned char *rgb255, float *frgb, float *lab);
EXTERNCPP void FRgb2Lab(float *rgb_arg, float *lab);

EXTERNCPP char *GetChid(char *file, char *buffer);
EXTERNCPP int AddColorbar(int icolorbar);
EXTERNCPP void ReloadMenu(int value);
EXTERNCPP void ColorbarMenu(int val);
EXTERNCPP void InitDefaultColorbars(int nini);
EXTERNCPP void DrawColorbarPathRGB(void);
EXTERNCPP void RemapColorbar(colorbardata *cbi);
EXTERNCPP void SortColorBars(void);
EXTERNCPP colorbardata *GetColorbar(char *label);
EXTERNCPP void InitOpenGL(int option);
EXTERNCPP void TextureShowMenu(int value);


EXTERNCPP void CopyArgs(int *argc, char **aargv, char ***argv_sv);
EXTERNCPP void InitUserTicks(void);
EXTERNCPP void DrawUserTicks(void);
EXTERNCPP void InitMultiThreading(void);
#ifdef WIN32
EXTERNCPP void OpenSMVFile(char *filename,int filenamelength,int *openfile);
#endif
EXTERNCPP int AnySmoke(void);
EXTERNCPP int AnySlices(const char *type);
EXTERNCPP void TrainerViewMenu(int var);

EXTERNCPP void DeleteCamera(cameradata *cam1);
EXTERNCPP void UnloadSliceMenu(int value);
EXTERNCPP void ViewpointMenu(int value);
EXTERNCPP void FrameRateMenu(int var);
EXTERNCPP void LoadUnloadMenu(int value);
EXTERNCPP void TourMenu(int var);
EXTERNCPP void ResetMenu(int var);
EXTERNCPP void LabelMenu(int value);
EXTERNCPP void FontMenu(int value);
EXTERNCPP void ShowHideSliceMenu(int var);
EXTERNCPP void ParticleShowMenu(int value);
EXTERNCPP void Plot3DShowMenu(int value);
EXTERNCPP void IsoShowMenu(int value);
EXTERNCPP void ShowBoundaryMenu(int value);
EXTERNCPP void Smoke3DShowMenu(int value);
EXTERNCPP void ShowVSliceMenu(int value);
EXTERNCPP int  GetPartPropIndexS(char *shortlabel);
EXTERNCPP int GetPartPropIndex(int class_i, int class_i_j);

#ifdef _DEBUG
EXTERNCPP void PrintPartProp(void);
#endif
EXTERNCPP partpropdata *GetPartProp(char *label);
EXTERNCPP void InitPartProp(void);
EXTERNCPP void UpdateStreakValue(float value);
EXTERNCPP void LoadParticleMenu(int value);
EXTERNCPP void LoadBoundaryMenu(int value);
EXTERNCPP void LoadSliceMenu(int value);
EXTERNCPP void LoadVSliceMenu(int value);

EXTERNCPP void InitVars(void);
EXTERNCPP void RenderState(int onoff);
EXTERNCPP void ResizeWindow(int width, int height);
EXTERNCPP meshdata *GetMesh(float *xyz);
EXTERNCPP meshdata *GetMeshNoFail(float *xyz);
EXTERNCPP void InitCellMeshInfo(void);
EXTERNCPP int InExterior(float *xyz);

EXTERNCPP int  OnMeshBoundary(float *xyz);

EXTERNCPP void RenderCB(int var);
EXTERNCPP sv_object *GetSmvObject(char *label);
EXTERNCPP void SnapScene(void);
EXTERNCPP void LevelScene(int level_x, int level_y, float *quat);
EXTERNCPP void GetPlot3dUVW(float xyz[3], float uvw[3]);
EXTERNCPP void SolveShooterData(void);
EXTERNCPP void DrawShooter(void);
EXTERNCPP void LoadFiles(void);
EXTERNCPP void GetStartupVSlice(int seq_id);
EXTERNCPP void GetStartupSlice(int seq_id);
EXTERNCPP void GetStartupPart(int seq_id);
EXTERNCPP void GetStartupPlot3D(int seq_id);
EXTERNCPP void GetStartupSmoke(int seq_id);
EXTERNCPP void GetStartupISO(int seq_id);
EXTERNCPP void GetStartupBoundary(int seq_id);
EXTERNCPP void Set3DSmokeStartup(void);
EXTERNCPP void PutStartupSmoke3D(FILE *fileout);
EXTERNCPP void DrawTransparentFaces(void);
EXTERNCPP int  IsBlockageVisible(blockagedata *bc, float time);
EXTERNCPP float Zoom2Aperture(float zoom0);
EXTERNCPP float Aperture2Zoom(float ap);
EXTERNCPP int  GetZoneColor(float t, float tmin, float tmax, int nlevel);
EXTERNCPP void DrawBlockages(int mode, int flag);
EXTERNCPP void WriteLabels(void);
EXTERNCPP void DrawLabels(void);
EXTERNCPP void GetNewPos(float *oldpos, float dx, float dy, float dz, float speed_factor);
EXTERNCPP void FreeSkybox(void);
EXTERNCPP void DrawSkybox(void);
EXTERNCPP void LoadSkyTexture(char *filebase, texturedata *texti);
EXTERNCPP void UncompressBoundaryDataGEOM(patchdata *patchi, int frame_index);
EXTERNCPP void UncompressBoundaryDataBNDF(meshdata *meshi,int frame_index);
EXTERNCPP void RGBTest(void);
EXTERNCPP void UpdateCADTextCoords(cadquad *quadi);
EXTERNCPP void UpdateIndexColors(void);
EXTERNCPP void SetTour(tourdata *thetour);
EXTERNCPP void MergeSmoke3D(smoke3ddata *smoke3dset);
EXTERNCPP void ShowHideSortGeometry(int sort_geom, float *mm);
EXTERNCPP void SortTransparentFaces(float *mm);
EXTERNCPP void GetScreenMapping(float *xyz0, float *screen_perm);
EXTERNCPP culldata *GetFacePort(meshdata *meshi, facedata *facei);
EXTERNCPP void SetCullVis(void);
EXTERNCPP void ExtractFrustum(void);
EXTERNCPP int BoxInFrustum(float *xx, float *yy, float *zz, int n);
EXTERNCPP int MeshInFrustum(meshdata *meshi);
EXTERNCPP int  RectangleInFrustum( float *x11, float *x12, float *x22, float *x21);
EXTERNCPP int UpdateSmoke3D(smoke3ddata *smoke3di);
EXTERNCPP void DrawSmokeFrame(void);
EXTERNCPP void DrawVolSmokeFrame(void);
EXTERNCPP void DrawPartFrame(void);
EXTERNCPP void DrawPlot3dFrame(void);
EXTERNCPP void DrawVSliceFrame(void);
EXTERNCPP void DrawSliceFrame(void);
EXTERNCPP void DrawVGSliceData(vslicedata *vslicei);
EXTERNCPP void DrawGSliceData(slicedata *slicei);
EXTERNCPP void DrawGSliceOutline(void);
EXTERNCPP void DrawBoundaryFrame(int flag);
EXTERNCPP void DrawVolSliceLines(const slicedata *sd);
EXTERNCPP void DrawVolSliceVerts(const slicedata *sd);
EXTERNCPP void SortSlices(void);
EXTERNCPP void DrawSortSlices(void);
EXTERNCPP void DrawSortSlicesDebug(void);
EXTERNCPP int HaveTerrainTexture(int *draw_surfaceptr);

EXTERNCPP void DrawSmoke3DVol(void);
EXTERNCPP void GetDrawingParms(int *drawing_transparent, int *drawing_blockage_transparent, int *drawing_vent_transparent);
EXTERNCPP void UpdateSmoke3dMenuLabels(void);
EXTERNCPP void InitSliceData(void);
EXTERNCPP void InitCameraList(void);
EXTERNCPP cameradata *InsertCamera(cameradata *cb,cameradata *source, char *name);
EXTERNCPP void AddDefaultViewpoints(void);
EXTERNCPP void ResetGLTime(void);
EXTERNCPP void InitCamera(cameradata *camera_data,char *name);
EXTERNCPP void CopyCamera(cameradata *to, cameradata *from);
EXTERNCPP void CopyViewCamera(cameradata *to, cameradata *from);
EXTERNCPP void UpdateCamera(cameradata *ca);
EXTERNCPP void GetInverse(float *m, float *mi);
EXTERNCPP void MatMultMat(float *m1, float *m2, float *m3);
EXTERNCPP void BlockageMenu(int value);
EXTERNCPP char *STRSTR(char *c, const char *key);
EXTERNCPP void HandlePLOT3DKeys(int  key);
EXTERNCPP void HandleMoveKeys(int  key);
EXTERNCPP int GetInterval(float val, float *array, int n);
EXTERNCPP int GetTimeInterval(float val, float *array, int n);

EXTERNCPP void SetUnitVis(void);
EXTERNCPP void UpdateAllPlotSlices(void);
EXTERNCPP int  MakeIBlank(void);
EXTERNCPP int  MakeIBlankCarve(void);
EXTERNCPP void MakeIBlankSmoke3D(void);
EXTERNCPP void GetUnitInfo(const char *unitlabel, int *unitclass, int *unittype);
EXTERNCPP float GetUnitVal(const char *unitlabel, float oldval, int ndecimals);

EXTERNCPP void UpdateUnitDefs(void);

EXTERNCPP void OutputAllIsoBounds(void);
EXTERNCPP void SmoothIsoSurface(isosurface *surfacedata);
EXTERNCPP void UpdateSliceFilenum(void);
EXTERNCPP void DrawStaticIso(const isosurface *asurface,int surfacetype,
                             int smoothnorms, int trans_flag, int data_type,
                             float line_width);
EXTERNCPP int  GetPlot3dTime(float *time);
EXTERNCPP void Normalize(float *xyz, int n);
EXTERNCPP void Array2String(float *array, int narray, char *string);
EXTERNCPP void GetIsoLevels(const char *isofile, int dataflag, float **levelsptr, float ***colorlevelsptr, int *nisolevels);

EXTERNCPP void *UpdateVSlices(void *arg);
EXTERNCPP void GetGSliceParams(void);
EXTERNCPP void UpdatePartMenuLabels(void);
EXTERNCPP void UpdateIsoMenuLabels(void);
EXTERNCPP void UpdateBoundaryMenuLabels(void);
EXTERNCPP void UpdateSliceMenuLabels(sliceparmdata *sp);
EXTERNCPP void UpdateVsliceMenuLabels(sliceparmdata *sp);
EXTERNCPP void UpdatePlot3dMenuLabels(void);
EXTERNCPP void HandleRotationType(int flag);

EXTERNCPP void Rgb2Hsl(unsigned char *rgbvals, float *hslvals);
EXTERNCPP void Hsl2Rgb(float *hslvals, unsigned char *rgbvals);

EXTERNCPP void InitColorbarsDir(void);
EXTERNCPP void InitTextureDir(void);
EXTERNCPP void GetRGB(unsigned int val, unsigned char *rr, unsigned char *gg, unsigned char *bb);
EXTERNCPP unsigned char *ReadPicture(char *filename, int *width, int *height, int *is_transparent, int printflag);
EXTERNCPP unsigned char *ReadJPEG(const char *filename,int *width, int *height, int *is_transparent);
EXTERNCPP unsigned char *ReadPNG(const char *filename,int *width, int *height, int *is_transparent);
EXTERNCPP void UpdateColorbarDialogs(void);

EXTERNCPP void SetTourXYZView(float t, tourdata *touri);
EXTERNCPP void UpdateViewTour(void);
EXTERNCPP void SetupTour(void);
EXTERNCPP void CreateTourPaths(void);
EXTERNCPP void DrawTours(void);
EXTERNCPP void DrawSelectTours(void);
EXTERNCPP void DrawSelectColorbar(void);
EXTERNCPP void FreeTour(tourdata *touri);
EXTERNCPP void FreeTours(void);
EXTERNCPP void InitTour(tourdata *touri);
EXTERNCPP void UpdateTourMenuLabels(void);
EXTERNCPP void DefaultTour(void);
EXTERNCPP void NewSelect(keyframe *newselect);
EXTERNCPP void DeleteTour(int tour_index);
EXTERNCPP tourdata *AddTour(char *label);
EXTERNCPP void ReverseTour(char *label);
EXTERNCPP void SetupCircularTourNodes(void);
EXTERNCPP void InitCircularTour(tourdata *touri, int nkeyframes, int option);
EXTERNCPP void DeleteTourFrames(tourdata *thistour);
EXTERNCPP keyframe *DeleteFrame(keyframe *step);
EXTERNCPP void ReallocTourMemory(void);
EXTERNCPP keyframe *AddFrame(keyframe *framei, float time, float pause_time, float *xyz, float view[3]);

EXTERNCPP void GetBlockVals(float *xmin, float *xmax,
                   float *ymin, float *ymax,
                   float *zmin, float *zmax,
                   int *imin, int *jmin, int *kmin);
EXTERNCPP void TransparentOff(void);
EXTERNCPP void TransparentOn(void);
EXTERNCPP void GetObstLabels(const char *filein);
EXTERNCPP void UpdateUseTextures(void);
EXTERNCPP void AntiAliasLine(int flag);
EXTERNCPP void AntiAliasSurface(int flag);
EXTERNCPP void SliceBounds2Glui(int slicefile_labelindex);
EXTERNCPP void GLUI2GlobalBoundaryBounds(const char *key);
EXTERNCPP void Global2GLUIBoundaryBounds(const char *key);
EXTERNCPP void UpdateLoadedLists(void);
EXTERNCPP void UpdateLights(float *pos1, float *pos2);
EXTERNCPP int  MergeRenderScreenBuffers(int nscreen_rows, GLubyte **screenbuffers);
EXTERNCPP void SetupScreeninfo(void);
EXTERNCPP int  MergeRenderScreenBuffers360(void);
EXTERNCPP GLubyte *GetScreenBuffer(void);
EXTERNCPP void ShowScene(int mode, int view_mode, int quad, GLint s_left, GLint s_down, screendata *screen);
EXTERNCPP int  InBlockage(const meshdata *gb,float x, float y, float z);
EXTERNCPP void DrawIso(int tranflag);
EXTERNCPP void DrawPlot3dTexture(meshdata *gb);
EXTERNCPP void UpdateShowStep(int val, int slicedir);
EXTERNCPP void ClearBuffers(int mode);
EXTERNCPP void UpdatePlotSlice(int slicedir);

EXTERNCPP void Render(int view_mode);
EXTERNCPP void UpdateVSliceBoundIndexes(void);
EXTERNCPP int  GetSliceBoundsIndex(const slicedata *sd);
EXTERNCPP int  GetSliceBoundsIndexFromLabel(char *label);
EXTERNCPP void UpdateSliceBoundIndexes(void);
EXTERNCPP void UpdateSliceBoundLabels(void);
EXTERNCPP void UpdateIsoType(void);
EXTERNCPP void UpdateIsoTypes(void);
EXTERNCPP int  GetBoundaryType(const patchdata *patchi);
EXTERNCPP void UpdateBoundaryType(void);
EXTERNCPP void UpdateBoundaryTypes(void);

EXTERNCPP int WriteFileBounds(char *file, float valmin, float valmax);
EXTERNCPP void SetViewZMAXPersp(void);
EXTERNCPP void UpdateTerrainOptions(void);
EXTERNCPP void UpdatePlot3dTitle(void);
EXTERNCPP void LoadPlot3dMenu(int value);
EXTERNCPP void InitPlot3dTimeList(void);
EXTERNCPP void UpdateIsoShowLevels(void);
EXTERNCPP void UpdateCurrentMesh(meshdata *meshi);
EXTERNCPP void DialogMenu(int value);
EXTERNCPP void ApertureMenu(int value);
EXTERNCPP void ZoomMenu(int value);
EXTERNCPP int  NewMultiSlice(slicedata *sdold,slicedata *sd);
EXTERNCPP void DrawTimebar(float xleft, float xright, float ybot, float ytop);
EXTERNCPP int CountColorbars(void);
EXTERNCPP void DrawHorizontalColorbars(void);
EXTERNCPP void DrawVerticalColorbars(void);
EXTERNCPP void DrawHorizontalColorbarRegLabels(void);
EXTERNCPP void DrawVerticalColorbarRegLabels(void);
EXTERNCPP void DrawGrid(const meshdata *gb);
EXTERNCPP void DrawZoneRoomGeom(void);
EXTERNCPP void DrawZoneFireData(void);
EXTERNCPP void DrawZoneRoomData(void);
EXTERNCPP void DrawZoneVentData(void);
EXTERNCPP void DrawZoneWallData(void);
EXTERNCPP void DrawZoneFirePlume(float radius, float height, float maxheight);
EXTERNCPP void DrawZoneVentDataProfile(void);
EXTERNCPP void SetViewPoint(int option);
EXTERNCPP void UpdateTimeLabels(void);
EXTERNCPP void RenderFrame(int view_mode);
EXTERNCPP void UpdateTerrain(int allocate_memory);
EXTERNCPP void RenderMenu(int value);
EXTERNCPP void LoadSmoke3DMenu(int value);
EXTERNCPP void DisplayVersionInfo(char *progname);
EXTERNCPP void DrawDemo(int nlat, int nlong);
EXTERNCPP void DrawDemo2(void);
EXTERNCPP void InitDemo(float rad, int nlat, int nlong);
EXTERNCPP void DrawOrigObstOutlines(void);
EXTERNCPP void DrawOutlines(void);
EXTERNCPP void DrawCBox(float x, float y, float z, float size);
EXTERNCPP void HandleIso(void);
EXTERNCPP void UpdateSurface(void);
EXTERNCPP void WindowStatus(int state);
EXTERNCPP void SetVentDirs(void);
EXTERNCPP void SetCVentDirs(void);
EXTERNCPP int HaveCircularVents(void);
EXTERNCPP void UpdateHiddenFaces(void);
EXTERNCPP void UpdateSelectFaces(void);
EXTERNCPP void UpdateSelectBlocks(void);
EXTERNCPP void DrawFaces(void);
EXTERNCPP void DrawFacesOLD(void);
EXTERNCPP void DrawSelectFaces(void);
EXTERNCPP void AllocateFaces(void);
EXTERNCPP void UpdateFaceLists(void);
EXTERNCPP void UpdateFaces(void);
EXTERNCPP void DrawTicks(void);
EXTERNCPP float *GetColorPtr(float *color);
EXTERNCPP float *GetColorTranPtr(float *color, float transparency);
EXTERNCPP void ConvertColor(int flag);
EXTERNCPP void InitCadColors(void);
EXTERNCPP void UpdateRGBColors(int colorindex);
EXTERNCPP void InitRGB(void);
EXTERNCPP void UpdateChopColors(void);
EXTERNCPP int  ReadIni(char *inifile);
EXTERNCPP int ReadBinIni(void);
EXTERNCPP void WriteIni(int flag,char *file);
EXTERNCPP void AdjustPart5Chops(void);
EXTERNCPP void ScaleFloat2String(float floatfrom, char *stringto, const float *scale);
EXTERNCPP float ScaleFloat2Float(float floatfrom, const float *scale);
EXTERNCPP void ScaleString(const char *stringfrom, char *stringto, const float *scale);
EXTERNCPP void Num2String(char *string, float tval);
EXTERNCPP int  SetupCase(char *file);
EXTERNCPP int  GetMinPartFrames(int flag);

EXTERNCPP void FreeCADInfo(void);

EXTERNCPP void InitUnitDefs(void);
EXTERNCPP void InitUnits(void);
EXTERNCPP f_units *GetUnitClass(char *unit);

EXTERNCPP void ReadCAD2Geom(cadgeomdata *cd);
EXTERNCPP void ReadCADGeom(cadgeomdata *cd);
EXTERNCPP void DrawCADGeom(const cadgeomdata *cd);

EXTERNCPP void ReadPlot3D(char *file, int ifile, int flag,int *errorcode);
EXTERNCPP void ReadGeomHeader(geomdata *geomi, int *geom_frame_index, int *ntimes_local);
EXTERNCPP void SetupReadAllGeom(void);
EXTERNCPP FILE_SIZE ReadGeom(geomdata *geomi, int load_flag, int type, int *geom_frame_index);
EXTERNCPP void ReadGeomFile2(geomdata *geomi);
EXTERNCPP void InitGeom(geomdata *geomi, int hasdata, int fdsblock, int have_vectors, int block_number);
EXTERNCPP FILE_SIZE ReadBoundary(int ifile, int flag, int *errorcode);
EXTERNCPP FILE_SIZE ReadPart(char *file, int ifile, int loadflag, int *errorcode);

EXTERNCPP void ReadZone(int ifile, int flag, int *errorcode);
EXTERNCPP FILE_SIZE ReadVSlice(int ivslice, int time_frame, float *time_value, int flag, int set_slice_color, int *errorcode);

EXTERNCPP void FreeSmoke3D(smoke3ddata *smoke3di);
EXTERNCPP void GetSmoke3DTimeSteps(int fortran_skip, char *smokefile, int version, int *ntimes_found, int *ntimes_full);
EXTERNCPP void ReadSmoke3DAllMeshes(int iframe, int smoketype, int *errorcode);
EXTERNCPP FILE_SIZE ReadSmoke3D(int iframe, int ifile, int flag, int first_time, float *time_value, int *errorcode);
#ifdef pp_FED
EXTERNCPP void ReadFed(int ifile, int time_frame, float *time_value, int flag, int file_type, int *errorcode);
#endif
EXTERNCPP FILE_SIZE ReadSlice(const char *file, int ifile, int time_frame, float *time_value, int flag, int set_slicecolor, int *errorcode);
EXTERNCPP FILE_SIZE ReadIso(const char *file, int ifile, int flag, int *geom_frame_index, int *errorcode);

EXTERNCPP void GetGlobalDeviceBounds(int type);
EXTERNCPP float GetDeviceVal(float time_local, devicedata *devicei, int *valid);
EXTERNCPP void InitMenus(void);

EXTERNCPP int ReadSMV(bufferstreamdata *stream);
EXTERNCPP void ReadSMVOrig(void);
EXTERNCPP void UpdateSMVDynamic(char *file);
EXTERNCPP void ReadSMVDynamic(char *file);
EXTERNCPP void SetInteriorBlockages(int flag);

EXTERNCPP int  STRCMP(const char *s1, const char *s2);
EXTERNCPP void OutputAxisLabels(void);
EXTERNCPP void OutputLargeText(float x, float y, char *string);
EXTERNCPP void OutputText(float x, float y, char *string);
EXTERNCPP void OutputTextColor(float *fontcolor, float x, float y, char *string);
EXTERNCPP void Output3TextRight(float *color, float x, float y, float z, char *string, float pad_length);
EXTERNCPP void Output3Text(float *color, float x, float y, float z, char *string);
EXTERNCPP void Output3Val(float x, float y, float z, float val);
EXTERNCPP void OutputBarText(float x, float y, const GLfloat *color, char *string);
EXTERNCPP float GetStringLength(char *string);
EXTERNCPP void UpdateGslicePlanes(void);

EXTERNCPP void UpdateAllGeomTriangles(void);
#ifdef pp_BOUNDS
EXTERNCPP void MakeBoundaryMask(patchdata *patchi);
EXTERNCPP void SetPatchMin(int set_valmin, float valmin, char *buffer2);
EXTERNCPP void SetPatchMax(int set_valmax, float valmax, char *buffer2);
EXTERNCPP void SetSliceMin(int set_valmin, float valmin, char *buffer2);
EXTERNCPP void SetSliceMax(int set_valmax, float valmax, char *buffer2);
#endif
EXTERNCPP void SetSliceBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *buffer2);
EXTERNCPP void SetBoundBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *buffer2);

EXTERNCPP void GetPartColors(partdata *parti, int nlevels, int flag);
EXTERNCPP void GetBoundaryColors(float *t, int nt, unsigned char *it,
              int settmin, float *tmin, int settmax, float *tmax,
              float *tmin_global, float *tmax_global,
              int ndatalevel, int nlevel,
              char **labels, char *scale, float *tvals256,
              int *extreme_min, int *extreme_max);
EXTERNCPP void GetBoundaryColors3(patchdata *patchi, float *t, int start, int nt, unsigned char *it,
              float *tmin, float *tmax,
              int nlevel,
              char **patchlabels, float *patchvalues, float *tvals256,
              int *extreme_min, int *extreme_max, int flag);
EXTERNCPP void GetBoundaryLabels(
              float tmin, float tmax,
              char **labels, float *boundaryvaluespatch, float *tvals256, int nlevel);
EXTERNCPP void GetZoneColors(const float *t, int nt, unsigned char *it,
               float tmin, float tmax, int nlevel, int nlevel_full,
               char **zonelabels, float zonevalues[12], float *tvals256
               );

EXTERNCPP int LoadAllPlot3D(float time);
EXTERNCPP void UpdatePlot3DColors(plot3ddata *plot3di, int flag, int *errorcode);
EXTERNCPP void UpdateAllPlot3DColors(int flag);
EXTERNCPP void GetPlot3DColors(int iplot, float *ttmin, float *ttmax,
              int ndatalevel, int nlevel,
              char **labels,char **labelsiso, float *tlevels, float *tlevels256,
              int *extreme_min, int *extreme_max, int flag
              );
EXTERNCPP void GetColorbarLabels(float tmin, float tmax, int nlevel,
              char labels[12][11],float *tlevels256);
EXTERNCPP void UpdatePart5Extremes(void);
EXTERNCPP void SetSliceColors(float smin, float smax, slicedata *sd, int flag, int *errorcode);
EXTERNCPP void UpdateSliceBounds2(void);
EXTERNCPP void UpdateSliceColors(int last_slice);
EXTERNCPP void GetSliceColors(const float *t, int nt, unsigned char *it,
              float tmin, float tmax,
              int ndatalevel, int nlevel,
              char colorlabels[12][11],float colorvalues[12], float *tlevels2,
              int *extreme_min, int *extreme_max, int flag
              );
EXTERNCPP void MakeTimesMap(float *times, unsigned char *times_map, int n);
EXTERNCPP meshdata *GetLoadedIsoMesh(void);
EXTERNCPP void SetIsoLabels(float smin, float smax,
                    isodata *sd, int *errorcode);
EXTERNCPP int  SmokeviewImage2File(char *directory, char *GIFfilename, int rendertype, int woffset, int width, int hoffset, int height);
#ifdef pp_LUA
EXTERNCPP int SVimage2var(int rendertype, int woffset, int width, int hoffset, int height, gdImagePtr *RENDERimage);
#endif

EXTERNCPP void MakeFireColors(float temp_min, float temp_max, int nfire_colors_arg);

#define HEADER_SIZE 4
#define TRAILER_SIZE 4

#define FORTREAD(var,size,count,STREAM) \
                           FSEEK(STREAM,HEADER_SIZE,SEEK_CUR);\
                           returncode=fread(var,size,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           FSEEK(STREAM,TRAILER_SIZE,SEEK_CUR)

#endif
