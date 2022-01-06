#ifndef SMOKEHEADERS_H_DEFINED
#define SMOKEHEADERS_H_DEFINED

#ifdef pp_LUA
#include "gd.h"
#endif

EXTERNCPP void SplitCB(int var);

#ifdef pp_REFRESH
EXTERNCPP void PeriodicRefresh(int var);
#endif

EXTERNCPP void UpdateTerrainTexture(int val);

EXTERNCPP void UpdateShowOnlyTop(void);

EXTERNCPP void UpdateGLuiGridLocation(void);
EXTERNCPP void EnableDisablePartPercentileDraw(int flag);
EXTERNCPP void GeneratePartHistograms(void);
EXTERNCPP void GeneratePartHistogramsMT(void);
FILE_SIZE LoadAllMSlicesMT(int last_slice, multislicedata *mslicei, int *fcount);

EXTERNCPP void PrintTime(const char *tag, int line, float *timer, const char *label);

EXTERNCPP void DrawObstBoundingBox(void);
EXTERNCPP void DrawGeomBoundingBox(float *boundingbox_color);

EXTERNCPP void UpdateGeomBoundingBox(void);

EXTERNCPP void UpdateGluiCfaces(void);

EXTERNCPP void SetTimeState(void);

EXTERNCPP void SetCurrentViewPoint(char *viewpoint_label);

EXTERNCPP int GetGeomDataSize(char *file, int *nvals, float *tmin, float *tmax, int time_frame,
                              int *geom_offsets, int *geom_offset_flag, int *error);

EXTERNCPP void UpdateMovieParms(void);

EXTERNCPP void ShowObjectsMenu(int var);
EXTERNCPP void UpdateShowHRRPUVPlot(int val);

EXTERNCPP void UpdateUseGeomFactors(void);

EXTERNCPP void AdjustY(cameradata *ca);

EXTERNCPP void UpdateFrameTimelabel(void);

EXTERNCPP void ScriptViewXYZMINMAXOrtho(int option);
EXTERNCPP void SetCameraView(cameradata *ca, int option);
EXTERNCPP void SetCameraViewPersp(cameradata *ca, int option);

EXTERNCPP void DeviceCB(int val);
EXTERNCPP void UpdateDeviceTypes(int val);

EXTERNCPP void InitStartupDirs(void);

EXTERNCPP int GetFontHeight(void);

EXTERNCPP void UpdateShowSliceColorbar(int *showcfast_arg, int *show_slice_colorbar_arg);

EXTERNCPP void UpdateSliceSkip(void);

EXTERNCPP void MergePartHistograms(void);
EXTERNCPP void MergePlot3DHistograms(void);

EXTERNCPP void UpdateColorbarControls(void);
EXTERNCPP void UpdateColorbarControls2(void);

EXTERNCPP void SetColorbarDigits(void);

EXTERNCPP void IncrementPartPropIndex(void);

EXTERNCPP int GetStringWidth(char *string);
EXTERNCPP void DrawDevicePlots(void);
EXTERNCPP void DrawTreeDevicePlots(void);
EXTERNCPP int GetPercentileDraw(int type);
EXTERNCPP int GetPlotState(int choice);

#ifndef TERRAIN_FIRE_LINE_UPDATE
#define TERRAIN_FIRE_LINE_UPDATE 39
#endif

EXTERNCPP FILE_SIZE LoadVSliceMenu2(int val);

EXTERNCPP void UpdateDeviceShow(void);
EXTERNCPP void UpdateClipbounds(int set_i0, int *i0, int set_i1, int *i1, int maxi);
EXTERNCPP int CompareFloat(const void *arg1, const void *arg2);
EXTERNCPP void UpdateHRRInfo(int val);
EXTERNCPP void ResetItimes0(void);
EXTERNCPP void UpdateShow(void);
EXTERNCPP void SynchTimes(void);
EXTERNCPP void UpdateTimes(void);
EXTERNCPP int GetIndex(float key, const float *list, int nlist);
EXTERNCPP int ISearch(float *list, int nlist, float key, int guess);

#ifdef pp_THREAD
EXTERNCPP void LockUnlockCompress(int flag);
#endif

EXTERNCPP void HideSlices(char *longlabel);

EXTERNCPP void OutputMinMax(char *meshlabel, char *label, char *unit, float valmin_fds, float valmax_fds, float valmin_smv, float valmax_smv);

EXTERNCPP void ScriptLoadSliceRender(scriptdata*scripti);
EXTERNCPP int GetNSliceFrames(char *file, float *stime_min, float *stime_max);
EXTERNCPP void GenerateSliceMenu(int from_commandline);
#ifdef pp_WUI_VAO
int InitTerrainVAO(int sizeof_vertices, int sizeof_indices);
void DrawTerrainGeomGPU(void);
#endif
void DrawTerrainGeom(int option);
void GenerateTerrainGeom(float **vertices_arg, int *sizeof_vertices_arg, unsigned int **indices_arg, int *sizeof_indices_arg, int *nindices_arg);

#ifdef pp_REFRESH
EXTERNCPP void RefreshGluiDialogs(void);
#endif
EXTERNCPP void SetMainWindow(void);
EXTERNCPP void UpdatePartType(void);
EXTERNCPP void GetSliceFileHeader(char *file, int *ip1, int *ip2, int *jp1, int *jp2, int *kp1, int *kp2, int *error);
EXTERNCPP int TimeAverageData(float *data_out, float *data_in, int ndata, int data_per_timestep, float *times_local, int ntimes_local, float average_time);
bufferstreamdata *GetSMVBuffer(char *file, char *file2);
EXTERNCPP void UpdateBlockType(void);
boundsdata *GetSliceBoundsInfo(char *shortlabel);
boundsdata *GetPatchBoundsInfo(char *shortlabel);
EXTERNCPP int GetGlobalPartBounds(int flag);
EXTERNCPP void GetGlobalPatchBounds(void);
EXTERNCPP void GetLoadedPlot3dBounds(int *compute_loaded, float *loaded_min, float *loaded_max);
EXTERNCPP void GetGlobalPlot3DBounds(void);
EXTERNCPP void GetGlobalSliceBounds(void);
EXTERNCPP void UpdateGlobalFEDSliceBounds(void);

EXTERNCPP void SetPercentileDrawOff(void);
EXTERNCPP void ClosePartFiles(void);
EXTERNCPP void PartBoundsCPP_CB(int var);
EXTERNCPP void UpdatdateResearchModeCPP(void);
EXTERNCPP void UpdatePartColors(partdata *parti);
EXTERNCPP void SetPercentilePartBounds(void);
EXTERNCPP void SetPercentilePlot3DBounds(void);
EXTERNCPP void DrawHistogram(histogramdata *histogram, float xxmin, float xxmax, float gmin, float gmax, int ndigits);
EXTERNCPP void SliceBoundsSetupNoGraphics(void);
EXTERNCPP void GetHistogramValProc(histogramdata*histogram, float cdf, float *val);
EXTERNCPP void ComputeLoadedSliceHist(char *label, histogramdata **histptr);
EXTERNCPP void ComputeLoadedPatchHist(char *label, histogramdata **histptr, float *valmin, float *valmax);
EXTERNCPP void GetGlobalBoundsMinMax(int type, char *label, float *valmin, float *valmax);
EXTERNCPP void SliceBoundsCPP_CB(int var);
EXTERNCPP void PatchBoundsCPP_CB(int var);
EXTERNCPP cpp_boundsdata *GetBoundsData(int type);
EXTERNCPP void SetCacheFlag(int type, int cache_flag);
EXTERNCPP int GetCacheFlag(int type);
EXTERNCPP void SetValTypeIndex(int type, int valtype_index);
EXTERNCPP int GetNValtypes(int type);
EXTERNCPP int GetValType(int type);
EXTERNCPP void GetMinMax(int type, char *label, int *set_valmin, float *valmin, int *set_valmax, float *valmax);
EXTERNCPP void GetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int *nall);
EXTERNCPP void SetMin(int type, char *label, int set_valmin, float valmin);
EXTERNCPP void SetMax(int type, char *label, int set_valmax, float valmax);
EXTERNCPP void SetMinMax(int type, char *label, int set_valmin, float valmin, int set_valmax, float valmax);
EXTERNCPP void SetMinMaxAll(int type, int *set_valmin, float *valmin, int *set_valmax, float *valmax, int nall);
EXTERNCPP void SetChopMin(int type, char *label, int set_valmin, float valmin);
EXTERNCPP void SetChopMax(int type, char *label, int set_valmax, float valmax);
EXTERNCPP void UpdateGluiBounds(void);
EXTERNCPP void SetLoadedSliceBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPatchBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPlot3DBounds(int *list, int nlist);
EXTERNCPP void SetLoadedPartBounds(int *list, int nlist);
EXTERNCPP void SetResearchMode(int flag);
EXTERNCPP void SetPercentileMode(int flag);
EXTERNCPP void SetPercentileMinMax(float p_min, float p_max);
EXTERNCPP void Plot3DBoundsCPP_CB(int var);
EXTERNCPP void SetColorbarDigitsCPP(int ndigits);

EXTERNCPP void ShiftColorbars(void);
EXTERNCPP int GetColorbarState(void);
EXTERNCPP void ViewpointCB(int val);
EXTERNCPP void SMV_EXIT(int code);
EXTERNCPP void UpdateSelectGeom(void);
EXTERNCPP void DrawSelectGeom(void);
EXTERNCPP void UpdateTriangleInfo(surfdata *tri_surf, float tri_area);
EXTERNCPP void UpdateVertexInfo(float *xyz1, float *xyz2);
EXTERNCPP void UpdateGeomAreas(void);
EXTERNCPP void GetZoneTempBounds(void);
EXTERNCPP FILE_SIZE GetSliceData(slicedata *sd, const char *slicefilename, int time_frame, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr,
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int sliceframestep_arg, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg
);
EXTERNCPP void GetSliceSizes(slicedata *sd, const char *slicefilenameptr, int time_frame, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int sliceframestep_arg,
  int *errorptr, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr);
EXTERNCPP void PrintPartLoadSummary(int option, int type);
EXTERNCPP void CreatePartSizeFile(partdata *parti, int angle_flag_arg);
EXTERNCPP void GetAllPartBounds(void);
EXTERNCPP void MergeAllPartBounds(void);
EXTERNCPP void ShrinkDialogs(void);
#ifdef CPP
EXTERNCPP void InsertRollout(GLUI_Rollout *rollout, GLUI *dialog);
EXTERNCPP void CloseRollouts(GLUI *dialog);
#endif

EXTERNCPP void MakeColorLabels(char colorlabels[12][11], float colorvalues[12], float tmin_arg, float tmax_arg, int nlevel);
EXTERNCPP void FinalizePartLoad(partdata *parti);
EXTERNCPP void LoadAllPartFilesMT(int val);
EXTERNCPP void ReadAllGeomMT(void);
EXTERNCPP void ClassifyAllGeomMT(void);
EXTERNCPP void GetAllPartBoundsMT(void);
EXTERNCPP void LoadAllPartFiles(int partnum);
EXTERNCPP void CreatePartBoundFile(partdata *parti);
EXTERNCPP void UpdateGluiPartFast(void);
EXTERNCPP void InitRolloutList(void);
EXTERNCPP void UpdateCO2ColorbarList(int value);
EXTERNCPP void GetTourXYZ(float t, keyframe *this_key, float *xyz);
EXTERNCPP void GetTourView(float t, keyframe *this_key, float *view);
EXTERNCPP int GetTourFrame(tourdata *touri, int itime);
EXTERNCPP int MeshConnect(meshdata *mesh_from, int val, meshdata *mesh_to);
EXTERNCPP void InitNabors(void);
EXTERNCPP int Smv2Html(char *html_out, int option, int from_where, int vr_flag);
EXTERNCPP int Smv2Geom(char *html_file);
EXTERNCPP int Obst2Data(char *html_file);
EXTERNCPP int SliceNode2Data(char *html_file, int option);
EXTERNCPP int SliceCell2Data(char *html_file, int option);
EXTERNCPP void UpdateBackgroundFlip(int flip);
EXTERNCPP void UpdateBackgroundFlip2(int flip);
EXTERNCPP void UpdateVectorpointsize(void);
EXTERNCPP void UpdateGluiIsoBounds(void);
EXTERNCPP void UpdateListIsoColorobar(void);
EXTERNCPP void UpdateTexturebar(void);
EXTERNCPP void UpdatePosView(void);
EXTERNCPP void UpdateUseLighting(void);
#ifdef pp_GPUSMOKE
EXTERNCPP  void UpdateGluiPlanes(float dmin, float dmax);
EXTERNCPP void UpdateSmoke3DPlanes(float delta_perp, float delta_par);
EXTERNCPP int IsSmokeInMesh(meshdata *meshi);
#endif
EXTERNCPP void GetFileSizes(void);
EXTERNCPP int IsSmokeComponentPresent(smoke3ddata *smoke3di);
EXTERNCPP void GetSliceDataBounds(slicedata *sd, float *pmin, float *pmax);
EXTERNCPP void UpdateAllSliceColors(int slicetype, int *errorcode);
EXTERNCPP void UpdateSliceBounds(void);
EXTERNCPP FILE_SIZE ReadGeomData(patchdata *patchi, slicedata *slicei, int load_flag, int time_frame, float *time_value, int *errorcode);
EXTERNCPP void UpdateWhereFaceVolumes(void);
EXTERNCPP void UpdateTimebarOverlap(void);
EXTERNCPP void UpdateRenderRadioButtons(int width_low, int height_low, int width_high, int height_high);
EXTERNCPP void ResetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high);
EXTERNCPP void GetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high);
EXTERNCPP void UpdateSmoke3dFileParms(void);
EXTERNCPP void UpdateZAxisCustom(void);
EXTERNCPP void SkipMenu(int value);
EXTERNCPP void UpdateShowGravityVector(void);
EXTERNCPP void ImmersedBoundCB(int var);
EXTERNCPP void UpdateImmersedControls(void);
EXTERNCPP void InitScriptErrorFiles(void);
EXTERNCPP void UpdateRenderListSkip(void);
EXTERNCPP void ForceIdle(void);
EXTERNCPP void UpdateFrameNumber(int changetime);
EXTERNCPP void UpdateVentOffset(void);
EXTERNCPP void ColorbarCB(int var);
EXTERNCPP void UpdateOpacityMap(void);
EXTERNCPP void ShowBoundsDialog(int type);
#ifdef  pp_SPECTRAL
EXTERNCPP void GetBlackBodyColors(float tmin, float tmax, float *intensities, int n);
#endif
EXTERNCPP void UpdateFreeze(int val);
EXTERNCPP void UpdateLoadTimeVal(float val);
EXTERNCPP void UpdateTimeFrameBounds(float time_min, float time_max);
EXTERNCPP int  GetVolFrameMax(int meshnum);
EXTERNCPP void UpdateLoadFrameVal(int frames);
EXTERNCPP void UpdateLoadFrameMax(int max_frames);
EXTERNCPP void UpdateShowRotationCenter(void);
EXTERNCPP void UpdateShowRotationCenter2(void);
EXTERNCPP void UpdateGluiRotateAbout(int val);
EXTERNCPP void UpdateRotationIndex(int val);
EXTERNCPP void ReloadAllSliceFiles(void);
EXTERNCPP void ReloadAllVectorSliceFiles(void);
EXTERNCPP void UnloadAllSliceFiles(char *longlabel);
EXTERNCPP void UpdateWindRoseDevices(int option);
EXTERNCPP void ParticleStreakShowMenu(int var);
EXTERNCPP void UpdateGeomNormals();
EXTERNCPP void Plot3DListMenu(int value);
EXTERNCPP void InitShooterData(void);
EXTERNCPP void ScriptCB(int var);
EXTERNCPP void TourCB(int var);
EXTERNCPP void SetClipControls(int val);
EXTERNCPP void ExtremeCB(int var);
EXTERNCPP void AddColorbarList2(int index, char *label);
EXTERNCPP void UpdateExtremeVals(void);
EXTERNCPP void PartBoundCB(int var);
EXTERNCPP void ShowHideMenu(int val);
EXTERNCPP void Plot3DBoundCB(int var);
EXTERNCPP void ColorbarGlobal2Local(void);
EXTERNCPP void UpdateShowbeamAsLine(void);
EXTERNCPP void UpdateVSliceDups(void);
EXTERNCPP void UnloadVSliceMenu(int value);
EXTERNCPP void UpdateSliceDups(void);
EXTERNCPP void UpdateBoundarySliceDups(void);
EXTERNCPP void IsoBoundCB(int var);
EXTERNCPP void UpdateScriptStep(void);
EXTERNCPP void UnLoadVolsmoke3DMenu(int value);
EXTERNCPP void UpdateGluiRender(void);
EXTERNCPP void AddScriptList(char *file, int id);
EXTERNCPP void UpdateMenu(void);
EXTERNCPP void VolumeCB(int var);
#ifdef pp_SMOKETEST
EXTERNCPP void DrawSmokeTest(void);
#endif
EXTERNCPP void Smoke3dCB(int var);
EXTERNCPP void InitAllLightFractions(float *xyz_light, int light_type);
EXTERNCPP void GetAllSliceHists(void);
EXTERNCPP void UpdateHistogramType(void);
EXTERNCPP void UpdateSliceHist(void);
EXTERNCPP void Enable360Zoom(void);
#ifdef pp_RENDER360_DEBUG
EXTERNCPP void DrawScreenInfo(void);
#endif
EXTERNCPP void UpdateShowSliceInObst(int var);
EXTERNCPP void GetGeomZBounds(float *zmin, float *zmax);
EXTERNCPP void MakeIBlankAll(void);
EXTERNCPP void UpdateSliceDupDialog(void);
EXTERNCPP void DrawNorth(void);
EXTERNCPP void DrawGeomValues(slicedata *sd, patchdata *patchi, int geom_type);
EXTERNCPP void DrawGeomData(int flag, slicedata *sd, patchdata *patchi, int geom_type);
EXTERNCPP void DrawGeomVData(vslicedata *vd);
EXTERNCPP void UpdateCurrentColorbar(colorbardata *cb);
EXTERNCPP int  HaveFire(void);
EXTERNCPP void UpdateFireAlpha(void);
EXTERNCPP int  HaveSoot(void);
EXTERNCPP void UpdateObjectUsed(void);
EXTERNCPP void UpdateColorTableList(int ncolortableinfo_old);
EXTERNCPP void UpdateColorTable(colortabledata *ctableinfo, int nctableinfo);
EXTERNCPP colortabledata *GetColorTable(char *label);
EXTERNCPP void UpdateIsoColorlevel(void);
EXTERNCPP void ReadIsoGeomWrapup(int flag);
EXTERNCPP void PSystem(char *commandline);
EXTERNCPP char *GetMovieFilePath(char *moviefile_path);
  EXTERNCPP int GetNumActiveDevices(void);
#ifdef CPP
EXTERNCPP void ToggleRollout(procdata *procinfo, int nprocinfo, int motion_id);
#endif
EXTERNCPP void EnableDisablePlayMovie(void);
EXTERNCPP void UpdateRenderStartButton(void);
EXTERNCPP void EnableDisableMakeMovie(int onoff);
EXTERNCPP void MakeMovie(void);
EXTERNCPP void PlayMovie(void);
EXTERNCPP void UpdateRenderType(int type);
EXTERNCPP void UpdateMovieType(int type);
EXTERNCPP void UpdateDeviceSize(void);
EXTERNCPP void UpdateDisplay(void);
EXTERNCPP void UpdateShowScene(void);
EXTERNCPP void DrawGravityAxis(void);
EXTERNCPP void XYZ2AzElev(float *xyz,float *azimuth, float *elevation);
EXTERNCPP void GetGeomDialogState(void);
EXTERNCPP void UpdateDeviceOrientation(void);
EXTERNCPP void UpdateGluiDevices(void);
EXTERNCPP void UpdateColorDevices(void);
EXTERNCPP void UpdateVisAxisLabels(void);
EXTERNCPP void UpdateGeometryControls(void);
EXTERNCPP void InitVolrenderScript(char *prefix, char *tour_label, int startframe, int skipframe);

// glui headers

EXTERNCPP void UpdateGluiZoneBounds(void);
EXTERNCPP void Glui3dSmokeSetup(int main_window);
EXTERNCPP void GluiBoundsSetup(int main_window);
EXTERNCPP void GluiClipSetup(int main_window);
EXTERNCPP void GluiColorbarSetup(int main_window);
EXTERNCPP void GluiDeviceSetup(int main_window);
EXTERNCPP void GluiGeometrySetup(int main_window);
EXTERNCPP void GluiLabelsSetup(int main_window);
EXTERNCPP void GluiMotionSetup(int main_window);
EXTERNCPP void GluiShooterSetup(int main_window);
EXTERNCPP void GluiStereoSetup(int main_window);
EXTERNCPP void GluiTourSetup(int main_window);
EXTERNCPP void GluiTrainerSetup(int main_window);

EXTERNCPP void GluiUpdateFontIndex(void);
EXTERNCPP void GluiScriptDisable(void);
EXTERNCPP void GluiScriptEnable(void);
EXTERNCPP void GluiAlertSetup(int main_window);
EXTERNCPP void GluiIdle(void);
EXTERNCPP void GluiIdleNULL(void);
EXTERNCPP void UpdateGluiSetViewXYZ(float *xyz);
EXTERNCPP void UpdateGluiFileLabel(int var);
EXTERNCPP void UpdateGluiVecFactor(void);
EXTERNCPP void UpdateGluiKeyframe(void);
EXTERNCPP void UpdateGluiBoundaryUnits(void);
EXTERNCPP void UpdateGluiSliceUnits(void);
EXTERNCPP void UpdateGluiPlot3Dtype(void);
EXTERNCPP void UpdateGluiIsotype(void);
EXTERNCPP void SetGLuiViewListManual(void);
EXTERNCPP void UpdateGluiStereo(void);
EXTERNCPP void UpdateGluiStreakValue(float rvalue);
EXTERNCPP void UpdateGluiZoom(void);
EXTERNCPP void Update_Glui_Clip(void);

EXTERNCPP void ShowGluiAlert(void);
EXTERNCPP void HideGluiAlert(void);
EXTERNCPP void ShowGluiShooter(void);
EXTERNCPP void HideGluiShooter(void);
EXTERNCPP void ShowGluiTrainer(void);
EXTERNCPP void HideGluiTrainer(void);
EXTERNCPP void ShowGluiColorbar(void);
EXTERNCPP void HideGluiColorbar(void);
EXTERNCPP void ShowGluiMotion(int menu_id);
EXTERNCPP void HideGluiMotion(void);
EXTERNCPP void ShowGluiClip(void);

EXTERNCPP void HideGluiClip(void);
EXTERNCPP void ShowGluiDisplay(int menu_id);
EXTERNCPP void ShowGluiDevice(void);
EXTERNCPP void ShowGluiPlotDevice(void);
EXTERNCPP void HideGluiDevice(void);
EXTERNCPP void SetLabelControls(void);
EXTERNCPP void SetLabelControls2(void);
EXTERNCPP void SetColorControls(void);
EXTERNCPP void HideGluiDisplay(void);
EXTERNCPP void ShowGluiTour(void);
EXTERNCPP void HideGluiTour(void);
EXTERNCPP void ShowGluiStereo(void);
EXTERNCPP void HideGluiStereo(void);

EXTERNCPP void UpdateClipPlanes(void);
EXTERNCPP void ShowGluiBounds(int menu_id);
EXTERNCPP void HideGluiBounds(void);
EXTERNCPP void ShowGluiGeometry(void);
EXTERNCPP void HideGluiGeometry(void);

EXTERNCPP void UpdateAllBoundaryColors(void);
EXTERNCPP void UpdateBoundaryListIndex(int patchfilenum);
EXTERNCPP void UpdateBoundaryListIndex2(char *label);
EXTERNCPP void UpdatePlot3dListIndex(void);

EXTERNCPP void GetBoundaryParams(void);
EXTERNCPP void GetSliceParams2(void);

EXTERNCPP void DrawWindRosesDevices(void);
EXTERNCPP void DeviceData2WindRose(int nr, int ntheta);
EXTERNCPP void DefineAllFEDs(void);
EXTERNCPP void UpdateTourState(void);
EXTERNCPP void UpdateEditTour(void);
EXTERNCPP void AddDeleteKeyframe(int flag);
EXTERNCPP void UpdateTourParms(void);
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
EXTERNCPP void SetColorbarListIndex(int val);
EXTERNCPP int  GetColorbarListIndex(void);
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

EXTERNCPP void UpdateResolutionMultiplier(void);
EXTERNCPP void RotateU2V(float *u, float *v, float *axis, float *angle);
EXTERNCPP void RotationTypeCB(int var);
EXTERNCPP void UpdateRotationType(int val);

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

SVEXTERN void UpdateVectorWidgets(void);
EXTERNCPP void UpdateGsliceParms(void);
EXTERNCPP void ReadIsoOrig(const char *file, int ifile, int flag, int *errorcode);
EXTERNCPP void UpdatePlotxyzAll(void);
EXTERNCPP void UpdateIsoColors(void);
EXTERNCPP void GetFaceInfo(void);
EXTERNCPP void GetGeomInfoPtrs(int flag);
EXTERNCPP devicedata *GetDeviceFromLabel(char *label, int index);
EXTERNCPP devicedata *GetCSVDeviceFromLabel(char *label, int index);
EXTERNCPP void SetupGlut(int argc, char **argv);
EXTERNCPP int GetNDevices(char *file);
EXTERNCPP void ReadHRR(int flag, int *errorcode);
EXTERNCPP void ReadDeviceData(char *file, int filetype, int flag);
EXTERNCPP void SetupZoneDevs(void);
EXTERNCPP void SetupDeviceData(void);
EXTERNCPP void DrawCGeom(int flag, geomdata *cgeom);
EXTERNCPP void DrawGeom(int flag,int frameflag);
EXTERNCPP void RemoveDupBlockages(void);
EXTERNCPP void SortIsoTriangles(float *mm);
EXTERNCPP void UpdateIsoTriangles(int flag);
EXTERNCPP void UpdateEvacParms(void);
EXTERNCPP void UpdateSliceMenuShow(void);
EXTERNCPP void UpdateBoundaryBounds(patchdata *patchi);
EXTERNCPP void UpdateAllBoundaryBounds(void);
EXTERNCPP void UpdateAllBoundaryBoundsST(void);
EXTERNCPP int  UpdateBoundaryHist(patchdata *patchi);
EXTERNCPP void UpdateHideBoundarySurface(void);
EXTERNCPP int  LastSliceLoadstack(void);
EXTERNCPP int  LastVSliceLoadstack(void);
EXTERNCPP void UpdateTransparency(void);
EXTERNCPP void UpdateScriptStart(void);
EXTERNCPP void UpdateResearchMode(void);
EXTERNCPP void UpdateScriptStop(void);
EXTERNCPP void UpdateDefer(void);
EXTERNCPP void UpdateTBounds(void);
EXTERNCPP void UpdateGluiTimeBounds(float time_min, float time_max);
EXTERNCPP void SetTimeVal(float timeval);
EXTERNCPP void GetIndepVarIndices(sv_object *smv_object,char **var_indep_strings, int nvars_indep,int *index);
EXTERNCPP void GetEvacIndices(sv_object *smv_object, int *evac_index,int *nevac_index);
EXTERNCPP void UpdateColorbarList(void);
EXTERNCPP void UpdateColorbarList2(void);
EXTERNCPP void UpdateColorbarFlip(void);

EXTERNCPP void LoadSmokeFrame(int meshnum, int framenum);
EXTERNCPP void LoadTimeFrame(int meshnum, float timeval);
EXTERNCPP void ScriptLoadVolSmokeFrame2(void);
EXTERNCPP void ScriptLoadIsoFrame2(scriptdata *scripti);
EXTERNCPP void InitDevicePlane(devicedata *devicei);
EXTERNCPP void DrawDevicesVal(void);
EXTERNCPP void GetSmokeSensors(void);
EXTERNCPP void AddNewTour(void);
EXTERNCPP void StartScript(void);
EXTERNCPP int RunScriptCommand(scriptdata *script_command);
EXTERNCPP void DoScriptHtml(void);
EXTERNCPP int  CompileScript(char *scriptfile);
EXTERNCPP scriptfiledata *InsertScriptFile(char *file);
#ifdef pp_LUA
EXTERNCPP luascriptfiledata *insert_luascriptfile(char *file);
#endif
EXTERNCPP char *GetIniFileName(int id);
EXTERNCPP char *GetScriptFileName(int id);
EXTERNCPP inifiledata *InsertIniFile(char *file);
EXTERNCPP void Keyboard(unsigned char key, int flag);
EXTERNCPP void GetNewScriptFileName(char *newscriptfilename);
EXTERNCPP void DrawSelectAvatars(void);
EXTERNCPP void OutputFedCSV(void);
EXTERNCPP void ParticlePropShowMenu(int value);
EXTERNCPP int  GetGridIndex(float x, int dir, float *plotxyz, int nplotxyz);
EXTERNCPP void UpdateSliceContours(int slice_type_index, float line_min, float line_max, int nline_values);
EXTERNCPP void ScriptMenu(int var);
EXTERNCPP void SmokeColorbarMenu(int var);
EXTERNCPP void ObjectCB(int flag);
EXTERNCPP void CompressOnOff(int flag);
EXTERNCPP void CompressSVZip2(void);
EXTERNCPP void UpdateTerrainColors(void);
EXTERNCPP void DrawTerrainOBST(terraindata *terri, int flag);
EXTERNCPP void DrawTerrainOBSTTexture(terraindata *terri);
EXTERNCPP void DrawTerrainOBSTSides(meshdata *meshi);
EXTERNCPP void DrawTrees(void);
EXTERNCPP void InitCullGeom(int cullflag);
EXTERNCPP void GetCullSkips(meshdata *meshi, int cullflag, int cull_portsize, int *iiskip, int *jjskip, int *kkskip);
#ifdef pp_GPU
EXTERNCPP int  InitShaders(void);
#ifdef pp_GPUSMOKE
EXTERNCPP void LoadNewSmokeShaders(void);
#endif
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

EXTERNCPP char *GetChid(char *file, char *buffer);
EXTERNCPP void AddColorbar(int icolorbar);
EXTERNCPP void ReloadMenu(int value);
EXTERNCPP void ColorbarMenu(int val);
EXTERNCPP void InitDefaultColorbars(int nini);
EXTERNCPP void DrawColorbarPath(void);
EXTERNCPP void UpdateColorbarSplits(colorbardata *cbi);
EXTERNCPP void UpdateColorbarNodes(colorbardata *cbi);
EXTERNCPP void RemapColorbar(colorbardata *cbi);
EXTERNCPP colorbardata *GetColorbar(char *label);
EXTERNCPP void RemapColorbarType(int cb_oldtype, char *cb_newname);
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
EXTERNCPP void EvacShowMenu(int value);
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
EXTERNCPP void LoadEvacMenu(int value);
EXTERNCPP void LoadBoundaryMenu(int value);
EXTERNCPP void LoadSliceMenu(int value);
EXTERNCPP void LoadVSliceMenu(int value);

EXTERNCPP void InitVars(void);
EXTERNCPP void RenderState(int onoff);
EXTERNCPP void UpdateWindowSizeList(void);
EXTERNCPP void ResizeWindow(int width, int height);
EXTERNCPP void UpdateTrainerOutline(void);
EXTERNCPP void UpdateTrainerMoves(void);
EXTERNCPP meshdata *GetMesh(float *xyz, meshdata *guess);
EXTERNCPP meshdata *GetMeshNoFail(float *xyz);
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
EXTERNCPP void UpdateOverwrite(void);
EXTERNCPP void CompressSVZip(void);
EXTERNCPP void DrawBlockages(int mode, int flag);
EXTERNCPP void WriteLabels(void);
EXTERNCPP void DrawLabels(void);
EXTERNCPP void UpdateTourList(void);
EXTERNCPP void GetNewPos(float *oldpos, float dx, float dy, float dz, float speed_factor);
EXTERNCPP void FreeSkybox(void);
EXTERNCPP void DrawSkybox(void);
EXTERNCPP void LoadSkyTexture(char *filebase, texturedata *texti);
EXTERNCPP void UncompressBoundaryDataFrame(meshdata *meshi,int frame_index);
EXTERNCPP void RGBTest(void);
EXTERNCPP void UpdateCADTextCoords(cadquad *quadi);
EXTERNCPP void UpdateIndexColors(void);
EXTERNCPP void UpdateTourIndex(void);
EXTERNCPP void SetTour(tourdata *thetour);
EXTERNCPP void UpdatePlot3dDisplay(void);
EXTERNCPP void UpdateSmoke3dFlags(void);
EXTERNCPP void MergeSmoke3D(smoke3ddata *smoke3dset);
EXTERNCPP void ShowHideSortGeometry(int sort_geom, float *mm);
EXTERNCPP void SortTransparentFaces(float *mm);
EXTERNCPP void GetScreenMapping(float *xyz0, float *screen_perm);
EXTERNCPP culldata *GetFacePort(meshdata *meshi, facedata *facei);
EXTERNCPP void SetCullVis(void);
EXTERNCPP void ExtractFrustum(void);
EXTERNCPP int  FDSPointInFrustum(float *xyz);
EXTERNCPP int  PointInFrustum( float *xyz);
EXTERNCPP int BoxInFrustum(float *xx, float *yy, float *zz);
EXTERNCPP int  RectangleInFrustum( float *x11, float *x12, float *x22, float *x21);
EXTERNCPP void UpdateSmoke3D(smoke3ddata *smoke3di);
#ifdef pp_SMOKETEST
EXTERNCPP void DrawSmokeFrame(int option);
#else
EXTERNCPP void DrawSmokeFrame(void);
#endif
EXTERNCPP void DrawVolSmokeFrame(void);
EXTERNCPP void DrawLightDirections(void);
EXTERNCPP void DrawPartFrame(void);
EXTERNCPP void DrawEvacFrame(void);
EXTERNCPP void DrawPlot3dFrame(void);
EXTERNCPP void DrawVSliceFrame(void);
EXTERNCPP void DrawSliceFrame(void);
EXTERNCPP void DrawVGSliceData(vslicedata *vslicei);
EXTERNCPP void DrawGSliceData(slicedata *slicei);
EXTERNCPP void DrawGSliceOutline(void);
EXTERNCPP void DrawBoundaryFrame(int flag);
EXTERNCPP void SceneMotionCB(int var);
EXTERNCPP void DrawVolSliceLines(const slicedata *sd);
EXTERNCPP void DrawVolSliceVerts(const slicedata *sd);

EXTERNCPP void DrawSmoke3DVol(void);
EXTERNCPP void GetDrawingParms(int *drawing_transparent, int *drawing_blockage_transparent, int *drawing_vent_transparent);
EXTERNCPP void UpdateSmoke3dMenuLabels(void);
EXTERNCPP void LabelsCB(int value);
EXTERNCPP void InitSliceData(void);
EXTERNCPP void UpdateCameraLabel(void);
EXTERNCPP void UpdateExtreme(void);
EXTERNCPP void UpdateColorbarType(void);
EXTERNCPP void InitCameraList(void);
EXTERNCPP cameradata *InsertCamera(cameradata *cb,cameradata *source, char *name);
EXTERNCPP void AddDefaultViewpoints(void);
EXTERNCPP void UpdateGluiViewpointList(void);
EXTERNCPP void ResetGLTime(void);
EXTERNCPP void EnableResetSavedView(void);
EXTERNCPP void ResetGluiView(int ival);
EXTERNCPP void InitCamera(cameradata *camera_data,char *name);
EXTERNCPP void CopyCamera(cameradata *to, cameradata *from);
EXTERNCPP void UpdateCamera(cameradata *ca);
EXTERNCPP void UpdateProjectionType(void);
EXTERNCPP void UpdateClipAll(void);
EXTERNCPP void GetInverse(float *m, float *mi);
EXTERNCPP void MatMultMat(float *m1, float *m2, float *m3);
EXTERNCPP void UpdateMeshList1(int val);
EXTERNCPP void UpdateTranslate(void);
EXTERNCPP void BlockageMenu(int value);
EXTERNCPP char *STRSTR(char *c, const char *key);
EXTERNCPP void HandlePLOT3DKeys(int  key);
EXTERNCPP void HandleMoveKeys(int  key);
EXTERNCPP int GetInterval(float val, float *array, int n);

EXTERNCPP void SetUnitVis(void);
EXTERNCPP void ShowHideTranslate(int var);
EXTERNCPP void UpdateAllPlotSlices(void);
EXTERNCPP int  MakeIBlank(void);
EXTERNCPP int  MakeIBlankCarve(void);
EXTERNCPP void MakeIBlankSmoke3D(void);
EXTERNCPP void GetUnitInfo(const char *unitlabel, int *unitclass, int *unittype);
EXTERNCPP float GetUnitVal(const char *unitlabel, float oldval, int ndecimals);

EXTERNCPP void UpdateUnitDefs(void);

EXTERNCPP void SmoothIsoSurface(isosurface *surfacedata);
EXTERNCPP void UpdateSliceFilenum(void);
EXTERNCPP void DrawStaticIso(const isosurface *asurface,int surfacetype,
                             int smoothnorms, int trans_flag, int data_type,
                             float line_width);
EXTERNCPP int  GetPlot3dTime(float *time);
EXTERNCPP void Normalize(float *xyz, int n);
EXTERNCPP void Array2String(float *array, int narray, char *string);
EXTERNCPP void GetIsoLevels(const char *isofile, int dataflag, float **levelsptr, float ***colorlevelsptr, int *nisolevels);

EXTERNCPP void UpdateVSlices(void);
EXTERNCPP void GetGSliceParams(void);
EXTERNCPP void UpdatePartMenuLabels(void);
EXTERNCPP void UpdateIsoMenuLabels(void);
EXTERNCPP void UpdateBoundaryMenuLabels(void);
EXTERNCPP void UpdateSliceMenuLabels(void);
EXTERNCPP void UpdateVsliceMenuLabels(void);
EXTERNCPP void UpdatePlot3dMenuLabels(void);
EXTERNCPP void HandleRotationType(int flag);

EXTERNCPP void InitTextureDir(void);
EXTERNCPP void GetRGB(unsigned int val, unsigned char *rr, unsigned char *gg, unsigned char *bb);
EXTERNCPP unsigned char *ReadPicture(char *filename, int *width, int *height, int *is_transparent, int printflag);
EXTERNCPP unsigned char *ReadJPEG(const char *filename,int *width, int *height, int *is_transparent);
EXTERNCPP unsigned char *ReadPNG(const char *filename,int *width, int *height, int *is_transparent);

EXTERNCPP void UpdateBlockVals(int flag);

EXTERNCPP void SetTourXYZView(float t, tourdata *touri);
EXTERNCPP void CreateVolTourList(void);
EXTERNCPP void DeleteVolTourList(void);
EXTERNCPP void CreateTourList(void);
EXTERNCPP void DeleteTourList(void);
EXTERNCPP void UpdateViewTour(void);
EXTERNCPP void UpdateTourControls(void);
EXTERNCPP void SetupTour(void);
EXTERNCPP void CreateTourPaths(void);
EXTERNCPP void DrawTours(void);
EXTERNCPP void SetGluiTourKeyframe(void);
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
EXTERNCPP keyframe *AddFrame(keyframe *framei, float time, float *xyz,float view[3]);

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
EXTERNCPP void DrawEvac(const partdata *parti);
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
EXTERNCPP void SliceBoundCB(int var);
EXTERNCPP void RenderMenu(int value);
EXTERNCPP void LoadSmoke3DMenu(int value);
EXTERNCPP void DisplayVersionInfo(char *progname);
EXTERNCPP void DrawDemo(int nlat, int nlong);
EXTERNCPP void DrawDemo2(void);
EXTERNCPP void InitDemo(float rad, int nlat, int nlong);
EXTERNCPP void DrawOutlines(void);
EXTERNCPP void DrawCBox(float x, float y, float z, float size);
EXTERNCPP void HandleIso(void);
EXTERNCPP void UpdateSurface(void);
EXTERNCPP void WindowStatus(int state);
EXTERNCPP void SetVentDirs(void);
EXTERNCPP void SetCVentDirs(void);
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
EXTERNCPP void SetStartupView(void);
EXTERNCPP void AddListView(char *label_in);
EXTERNCPP float *GetColorPtr(float *color);
EXTERNCPP float *GetColorTranPtr(float *color, float transparency);
EXTERNCPP void ConvertColor(int flag);
EXTERNCPP void InitCadColors(void);
EXTERNCPP void UpdateRGBColors(int colorindex);
EXTERNCPP void InitRGB(void);
EXTERNCPP void UpdateChopColors(void);
EXTERNCPP int  ReadIni(char *inifile);
EXTERNCPP void WriteIni(int flag,char *file);
EXTERNCPP void AdjustPart5Chops(void);
EXTERNCPP void ScaleFloat2String(float floatfrom, char *stringto, const float *scale);
EXTERNCPP float ScaleFloat2Float(float floatfrom, const float *scale);
EXTERNCPP void ScaleString(const char *stringfrom, char *stringto, const float *scale);
EXTERNCPP void Num2String(char *string, float tval);
EXTERNCPP int  SetupCase(char *file);
EXTERNCPP int  GetMinPartFrames(int flag);
EXTERNCPP int  Update_Bounds(void);

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
EXTERNCPP void ReadAllGeom(void);
EXTERNCPP void ClassifyAllGeom(void);
EXTERNCPP FILE_SIZE ReadGeom(geomdata *geomi, int load_flag, int type, int *geom_frame_index);
EXTERNCPP void ReadGeomFile2(geomdata *geomi);
EXTERNCPP void InitGeom(geomdata *geomi, int hasdata, int fdsblock, int have_vectors);
EXTERNCPP FILE_SIZE ReadBoundary(int ifile, int flag, int *errorcode);
EXTERNCPP FILE_SIZE ReadPart(char *file, int ifile, int loadflag, int *errorcode);

EXTERNCPP void ReadZone(int ifile, int flag, int *errorcode);
EXTERNCPP FILE_SIZE ReadVSlice(int ivslice, int time_frame, float *time_value, int flag, int *errorcode);

EXTERNCPP void FreeSmoke3D(smoke3ddata *smoke3di);
EXTERNCPP void GetSmoke3DTimeSteps(int fortran_skip, char *smokefile, int version, int *ntimes_found, int *ntimes_full);
EXTERNCPP void ReadSmoke3DAllMeshes(int iframe, int smoketype, int *errorcode);
EXTERNCPP void ReadSmoke3DAllMeshesAllTimes(int smoketype2, int *errorcode);
EXTERNCPP FILE_SIZE ReadSmoke3D(int iframe, int ifile, int flag, int first_time, int *errorcode);
EXTERNCPP void ReadFed(int ifile, int time_frame, float *time_value, int flag, int file_type, int *errorcode);
EXTERNCPP FILE_SIZE ReadSlice(const char *file, int ifile, int time_frame, float *time_value, int flag, int set_slicecolor, int *errorcode);
EXTERNCPP FILE_SIZE ReadIso(const char *file, int ifile, int flag, int *geom_frame_index, int *errorcode);

EXTERNCPP void InitMenus(int unload);
int ReadSMV(bufferstreamdata *stream);
EXTERNCPP void ReadSMVDynamic(char *file);
EXTERNCPP int  STRCMP(const char *s1, const char *s2);
EXTERNCPP void OutputAxisLabels(void);
EXTERNCPP void OutputLargeText(float x, float y, char *string);
EXTERNCPP void OutputText(float x, float y, char *string);
EXTERNCPP void OutputTextColor(float *fontcolor, float x, float y, char *string);
EXTERNCPP void Output3Text(float *color, float x, float y, float z, char *string);
EXTERNCPP void Output3Val(float x, float y, float z, float val);
EXTERNCPP void OutputBarText(float x, float y, const GLfloat *color, char *string);
EXTERNCPP void UpdateChar(void);
EXTERNCPP void UpdateTracers(void);
EXTERNCPP void UpdateGslicePlanes(void);

EXTERNCPP void GetPartColors(partdata *parti, int nlevels);
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
              int *extreme_min, int *extreme_max);
EXTERNCPP void GetBoundaryLabels(
              float tmin, float tmax,
              char **labels, float *boundaryvaluespatch, float *tvals256, int nlevel);
EXTERNCPP void GetZoneColors(const float *t, int nt, unsigned char *it,
               float tmin, float tmax, int nlevel, int nlevel_full,
               char **zonelabels, float zonevalues[12], float *tvals256
               );

EXTERNCPP void UpdatePlot3DColors(int file, int *errorcode);
EXTERNCPP void UpdateAllPlot3DColors(void);
EXTERNCPP void GetPlot3DColors(int iplot, float *ttmin, float *ttmax,
              int ndatalevel, int nlevel,
              char **labels,char **labelsiso, float *tlevels, float *tlevels256,
              int *extreme_min, int *extreme_max
              );
EXTERNCPP void GetSliceLabels(float tmin, float tmax, int nlevel,
              char labels[12][11],float *tlevels256);
EXTERNCPP void UpdatePart5Extremes(void);
EXTERNCPP void UpdateSliceColors(int last_slice);
EXTERNCPP void GetSliceColors(const float *t, int nt, unsigned char *it,
              float tmin, float tmax,
              int ndatalevel, int nlevel,
              char colorlabels[12][11],float colorvalues[12], float *tlevels2,
              int *extreme_min, int *extreme_max
              );
EXTERNCPP meshdata *GetLoadedIsoMesh(void);
EXTERNCPP void SetIsoLabels(float smin, float smax,
                    isodata *sd, int *errorcode);
EXTERNCPP void GetIsoLabels(float tmin, float tmax, int nlevel,char labels[12][11],float *tlevels256);
EXTERNCPP int  SmokeviewImage2File(char *directory, char *GIFfilename, int rendertype, int woffset, int width, int hoffset, int height);
#ifdef pp_LUA
EXTERNCPP int SVimage2var(int rendertype, int woffset, int width, int hoffset, int height, gdImagePtr *RENDERimage);
#endif

EXTERNCPP void UpdateShowHideButtons(void);
EXTERNCPP void UpdateTriangles(int time_flag, int update);
EXTERNCPP void UpdateTrianglesMT(void);
EXTERNCPP void CancelUpdateTriangles(void);
EXTERNCPP void FinishUpdateTriangles(void);

#ifndef CPP
#include "smokefortheaders.h"
#endif

#endif
