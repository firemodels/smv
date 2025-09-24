#ifndef SMOKEHEADERS_H_DEFINED
#define SMOKEHEADERS_H_DEFINED

#ifndef TERRAIN_FIRE_LINE_UPDATE
#define TERRAIN_FIRE_LINE_UPDATE 39
#endif

#include "shared_structures.h"

//*** glui_clip.cpp headers

EXTERNCPP void GLUIClipSetup(int main_window);
EXTERNCPP void GLUIUpdateClip(void);
EXTERNCPP void GLUIShowClip(void);
EXTERNCPP void GLUIHideClip(void);
EXTERNCPP void GLUIUpdateClipAll(void);
EXTERNCPP void ClipCB(int var);
EXTERNCPP void SetClipControls(int val);

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
EXTERNCPP void GLUIUpdateNodeLabel(colorbardata *cbi);

//*** glui_display.cpp headers

EXTERNCPP void GLUIUpdateFastBlockageDraw(void);
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
EXTERNCPP void GLUIUpdateVisSkyboxOutline(void);
EXTERNCPP void GLUISkyCB(int var);


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
EXTERNCPP void GLUIHideGeometry(void);
EXTERNCPP void GLUIHideHVAC(void);
EXTERNCPP void GLUIShowHVAC(void);
EXTERNCPP void GLUIHVAC2Glui(int index);
EXTERNCPP void GLUIUpdateTerrainTexture(int val);
EXTERNCPP void GLUIUpdateGeomBoundingBox(void);
EXTERNCPP void GLUIUpdateSelectGeom(void);
EXTERNCPP void GLUIUpdateShowOnlyTop(void);
EXTERNCPP void GLUIUpdateTriangleInfo(surfdata *tri_surf, float tri_area);
EXTERNCPP void GLUIUpdateVertexInfo(float *xyz1, float *xyz2);
EXTERNCPP void GLUIUpdateWhereFaceVolumes(void);
EXTERNCPP void GLUIGetGeomDialogState(void);
EXTERNCPP void GLUIUpdateCfaces(void);
EXTERNCPP void GLUIUpdateGeometryControls(void);
EXTERNCPP void GLUIUpdateHVACVarLists(void);
EXTERNCPP void GetGeomZBounds(float *zmin, float *zmax);

//*** glui_motion.cpp headers

EXTERNCPP void GLUIUpdateFarclip(void);

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
EXTERNCPP void GLUIUpdatePlot2DSize(void);
EXTERNCPP void GLUIUpdateDeviceAdd(void);
EXTERNCPP void GLUIGenPlotCB(int var);
EXTERNCPP void UpdateCSVFileTypes(void);

//*** glui_stereo.cpp headers

EXTERNCPP void GLUIStereoSetup(int main_window);
EXTERNCPP void GLUIUpdateStereo(void);
EXTERNCPP void GLUIShowStereo(void);
EXTERNCPP void GLUIHideStereo(void);

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
EXTERNCPP void TourCB(int var);
EXTERNCPP void AddDeleteKeyframe(int flag);

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

//*** callbacks.c headers

EXTERNCPP void ClearBuffers(int mode);
EXTERNCPP void DisplayCB(void);
EXTERNCPP void DoScriptHtml(void);
EXTERNCPP void ForceIdle(void);
EXTERNCPP int  GetGridIndex(float x, int dir, float *plotxyz, int nplotxyz);
EXTERNCPP int  HandleColorbarIndex(int colorbar_index);
EXTERNCPP void HandleIso(void);
EXTERNCPP void HandleMoveKeys(int  key);
EXTERNCPP void HandlePLOT3DKeys(int  key);
EXTERNCPP void HandleRotationType(int flag);
EXTERNCPP void IdleCB(void);
EXTERNCPP void Keyboard(unsigned char key, int flag);
EXTERNCPP void KeyboardUpCB(unsigned char key, int x, int y);
EXTERNCPP void MouseCB(int button, int state, int x, int y);
EXTERNCPP void MouseDragCB(int xm, int ym);
EXTERNCPP void NextXIndex(int inc,int flag);
EXTERNCPP void NextYIndex(int inc,int flag);
EXTERNCPP void NextZIndex(int inc,int flag);
EXTERNCPP void ResetGLTime(void);
EXTERNCPP void ReshapeCB(int width, int height);
EXTERNCPP void ResizeWindow(int width, int height);
EXTERNCPP void SetMainWindow(void);
EXTERNCPP void SetScreenSize(int *width, int *height);
EXTERNCPP void SpecialKeyboardCB(int key, int x, int y);
EXTERNCPP void SpecialKeyboardUpCB(int key, int x, int y);
EXTERNCPP void UpdateClipPlanes(void);
EXTERNCPP void UpdateCurrentMesh(meshdata *meshi);
EXTERNCPP void UpdatePlot3dTitle(void);
EXTERNCPP void WindowStatus(int state);
EXTERNCPP void KeyboardCB(unsigned char key, int x, int y);

//*** camera.c headers

EXTERNCPP void  AddDefaultViewpoints(void);
EXTERNCPP float Aperture2Zoom(float ap);
EXTERNCPP void  Cam2Clip(cameradata *cam);
EXTERNCPP void  Clip2Cam(cameradata *cam);
EXTERNCPP void  CopyCamera(cameradata *to, cameradata *from);
EXTERNCPP void  CopyViewCamera(cameradata *to, cameradata *from);
EXTERNCPP void  DeleteCamera(cameradata *cam1);
EXTERNCPP cameradata *GetCamera(char *name);
EXTERNCPP char  *GetCameraLabel(int index);
EXTERNCPP void  InitCamera(cameradata *camera_data,char *name);
EXTERNCPP void  InitCameraList(void);
EXTERNCPP cameradata *InsertCamera(cameradata *cb,cameradata *source, char *name);
EXTERNCPP void  SetCameraView(cameradata *ca, int option);
EXTERNCPP void  SetCameraViewPersp(cameradata *ca, int option);
EXTERNCPP void  SortCameras(void);
EXTERNCPP void  SortCamerasID(void);
EXTERNCPP void  UpdateCamera(cameradata *ca);
EXTERNCPP void  UpdateCameraYpos(cameradata *camera_data, int option);
EXTERNCPP float Zoom2Aperture(float zoom0);

//*** colortable.c headers

EXTERNCPP colortabledata *GetColorTable(char *label);
EXTERNCPP int GetColorTableIndex(int *color);

//*** colortimebar.c headers

EXTERNCPP int  AddColorbar(int icolorbar);
EXTERNCPP void AdjustColorBar(colorbardata *cbi);
EXTERNCPP void AdjustColorBarLab(colorbardata *cbi);
EXTERNCPP void CheckLab(void);
EXTERNCPP int  CountColorbars(void);
EXTERNCPP void DrawColorbarPathCIELab(void);
EXTERNCPP void DrawColorbarPathRGB(void);
EXTERNCPP void DrawHorizontalColorbars(void);
EXTERNCPP void DrawHorizontalColorbarRegLabels(void);
EXTERNCPP void DrawVerticalColorbars(void);
EXTERNCPP void DrawVerticalColorbarRegLabels(void);
EXTERNCPP void DrawSelectColorbar(void);
EXTERNCPP void DrawTimebar(float xleft, float xright, float ybot, float ytop);
EXTERNCPP void FRgb2Lab(float *rgb_arg, float *lab);
EXTERNCPP int  IsColorbarSplit(colorbardata *cbi);
EXTERNCPP void Lab2Rgb(unsigned char *rgb255, float *frgb, float *lab);
EXTERNCPP void RevertColorBar(colorbardata *cbi);
EXTERNCPP void Rgb2Lab(unsigned char *rgb, float *lab);
EXTERNCPP void SortColorBars(void);
EXTERNCPP void UpdateColorbarDialogs(void);
EXTERNCPP void UpdateColorbarOrig(void);
EXTERNCPP void UpdateCurrentColorbar(colorbardata *cb);
EXTERNCPP void UpdateShowColorbar(int *showcfast_arg, int *show_slice_colorbar_arg,
  int *show_hvacduct_colorbar_arg, int *show_hvacnode_colorbar_arg);
EXTERNCPP void UpdateTimeLabels(void);
EXTERNCPP void UpdateOutlineMode(void);

//*** drawGeometry.c headers

EXTERNCPP void AllocateFaces(void);
EXTERNCPP void DrawBlockages(int mode, int flag);
EXTERNCPP void DrawCADGeom(const cadgeomdata *cd);
EXTERNCPP void DrawCBox(float x, float y, float z, float size);
EXTERNCPP void DrawCircVents(int option);
EXTERNCPP void DrawDemo(int nlat, int nlong);
EXTERNCPP void DrawDemo2(void);
EXTERNCPP void DrawGravityAxis(void);
EXTERNCPP void DrawObstOutlines(void);
EXTERNCPP void DrawOutlines(void);
EXTERNCPP void DrawOrigObstOutlines(void);
EXTERNCPP void DrawSelectFaces(void);
EXTERNCPP void DrawTicks(void);
EXTERNCPP void DrawTransparentFaces(void);
EXTERNCPP void DrawUserTicks(void);
EXTERNCPP void GetBlockVals(float *xmin, float *xmax,
                   float *ymin, float *ymax,
                   float *zmin, float *zmax,
                   int *imin, int *jmin, int *kmin);
EXTERNCPP void GetDrawingParms(int *drawing_transparent, int *drawing_blockage_transparent, int *drawing_vent_transparent);
EXTERNCPP void GetObstLabels(const char *filein);
EXTERNCPP int  HaveCircularVents(void);
EXTERNCPP int  InBlockage(const meshdata *gb,float x, float y, float z);
EXTERNCPP void InitDemo(float rad, int nlat, int nlong);
EXTERNCPP void InitUserTicks(void);
EXTERNCPP void LevelScene(int level_x, int level_y, float *quat);
EXTERNCPP void RemoveDupBlockages(void);
EXTERNCPP void SetCVentDirs(void);
EXTERNCPP void SetInteriorBlockages(void);
EXTERNCPP void SetVentDirs(void);
EXTERNCPP void SnapScene(int delta);
EXTERNCPP void SortTransparentFaces(float *mm);
EXTERNCPP void UpdateCADTextCoords(cadquad *quadi);
EXTERNCPP void UpdateFaces(void);
EXTERNCPP void UpdateFaceLists(void);
EXTERNCPP void UpdateIndexColors(void);
EXTERNCPP void UpdateSelectBlocks(void);
EXTERNCPP void UpdateSelectFaces(void);

//*** getdatabounds.c headers

EXTERNCPP void AdjustPart5Chops(void);
EXTERNCPP void GetAllPartBounds(void);
EXTERNCPP void GetGlobalHVACDuctBounds(int flag);
EXTERNCPP void GetGlobalHVACNodeBounds(int flag);
EXTERNCPP int  GetGlobalPartBounds(int flag);
EXTERNCPP void *GetGlobalPartBoundsReduced(void *arg);
EXTERNCPP void GetGlobalPatchBounds(int flag, int set_flag, char *label);
EXTERNCPP void *GetGlobalPatchBoundsFull(void *arg);
EXTERNCPP void GetGlobalPatchBoundsReduced(void);
EXTERNCPP void GetGlobalPlot3DBounds(void);
EXTERNCPP void GetGlobalSliceBounds(int flag, int set_flag, char *label);
EXTERNCPP void *GetGlobalSliceBoundsFull(void *arg);
EXTERNCPP void GetGlobalSliceBoundsReduced(void);
EXTERNCPP void GetLoadedPlot3dBounds(int *compute_loaded, float *loaded_min, float *loaded_max);
EXTERNCPP void MergeAllPartBounds(void);
EXTERNCPP void PrintPartLoadSummary(int option, int type);
EXTERNCPP int  WriteFileBounds(char *file, float valmin, float valmax);

//*** getdatacolors.c headers

EXTERNCPP void ConvertColor(int flag);
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
EXTERNCPP void GetColorbarLabels(float tmin, float tmax, int nlevel,
              char labels[12][11],float *tlevels256);
EXTERNCPP float *GetColorTranPtr(float *color, float transparency);
EXTERNCPP void GetPartColors(partdata *parti, int nlevels, int flag);
EXTERNCPP void GetPlot3DColors(int iplot, float *ttmin, float *ttmax,
              int ndatalevel, int nlevel,
              char **labels,char **labelsiso, float *tlevels, float *tlevels256,
              int *extreme_min, int *extreme_max, int flag
              );
EXTERNCPP void GetRGB(unsigned int val, unsigned char *rr, unsigned char *gg, unsigned char *bb);
EXTERNCPP void GetSliceColors(const float *t, int nt, unsigned char *it,
              float tmin, float tmax,
              int ndatalevel, int nlevel,
              char colorlabels[12][11],float colorvalues[12], float *tlevels2,
              int *extreme_min, int *extreme_max, int flag
              );
EXTERNCPP int  GetZoneColor(float t, float tmin, float tmax, int nlevel);
EXTERNCPP void InitCadColors(void);
EXTERNCPP void InitRGB(void);
EXTERNCPP void InitScase(smv_case *scase);
EXTERNCPP smv_case * CreateScase();
EXTERNCPP void DestroyScase(smv_case *scase);
EXTERNCPP void MakeColorLabels(char colorlabels[12][11], float colorvalues[12], float tmin_arg, float tmax_arg, int nlevel);
EXTERNCPP void UpdateAllBoundaryColors(int flag);
EXTERNCPP void UpdateChopColors(void);
EXTERNCPP void UpdateAllPlot3DColors(int flag);
EXTERNCPP void UpdateCO2Colormap(void);
EXTERNCPP void UpdateSmokeColormap(void);
EXTERNCPP void UpdatePart5Extremes(void);
EXTERNCPP void UpdateRGBColors(int colorindex);
EXTERNCPP void UpdateSliceBounds2(void);
EXTERNCPP void UpdateSliceColors(int last_slice);
EXTERNCPP void UpdateTexturebar(void);
EXTERNCPP void GetZoneColors(const float *t, int nt, unsigned char *it,
               float tmin, float tmax, int nlevel, int nlevel_full,
               char **zonelabels, float zonevalues[12], float *tvals256
               );

//*** IOboundary.c headers

EXTERNCPP void ComputeLoadedPatchHist(char *label, histogramdata **histptr, float *valmin, float *valmax);
EXTERNCPP void DrawBoundaryFrame(int flag);
EXTERNCPP int  GetBoundaryType(const patchdata *patchi);
EXTERNCPP float *GetPatchXYZ(const meshdata *meshi);
EXTERNCPP void GLUI2GlobalBoundaryBounds(const char *key);
EXTERNCPP void Global2GLUIBoundaryBounds(const char *key);
EXTERNCPP void MakeBoundaryMask(patchdata *patchi);
EXTERNCPP FILE_SIZE ReadBoundary(int ifile, int flag, int *errorcode);
EXTERNCPP void SetTimeState(void);
EXTERNCPP void UncompressBoundaryDataGEOM(patchdata *patchi, int frame_index);
EXTERNCPP void UncompressBoundaryDataBNDF(meshdata *meshi,int frame_index);
EXTERNCPP void UpdateBoundaryTypes(void);
EXTERNCPP void UpdateBoundaryMenuLabels(void);
EXTERNCPP void UpdateBoundarySliceDups(void);
EXTERNCPP void GetBoundaryParams(void);
EXTERNCPP void UpdateBoundaryType(void);

//*** IOgeometry.c headers

EXTERNCPP void CancelUpdateTriangles(void);
EXTERNCPP void *ClassifyAllGeom(void *arg);
EXTERNCPP void DrawBoxMinMax(float *bbmin, float *bbmax, float *box_color);
EXTERNCPP void DrawBoxOutline(float *bb, float *box_color);
EXTERNCPP void DrawBoxShaded(float *bb, int flag, int *hidden6, float *box_color);
EXTERNCPP void DrawCGeom(int flag, geomdata *cgeom);
EXTERNCPP int  GetNCGeomTriangles(void);
EXTERNCPP void DrawGeom(int flag,int frameflag);
EXTERNCPP void DrawGeomBoundingBox(float *boundingbox_color);
EXTERNCPP void DrawGeomData(int flag, slicedata *sd, patchdata *patchi, int geom_type);
EXTERNCPP void DrawGeomValues(slicedata *sd, patchdata *patchi, int geom_type);
EXTERNCPP void DrawGeomVData(vslicedata *vd);
EXTERNCPP void DrawSelectGeom(void);
EXTERNCPP void GetFaceInfo(void);
EXTERNCPP FILE_SIZE GetGeomData(patchdata *patchi, char *filename, int load_flag, int ntimes, int nvals, float *times,
              int *nstatics, int *ndynamics, float *vals, int time_frame, float *time_value,
              int *geom_offsets, int *error);
EXTERNCPP int  GetGeomDataSize(char *file, int *nvals, int time_frame,
               int *cvals_offsets, int *cvals_sizes, int *geom_offsets, int *geom_offset_flag,
               int *max_buffer_size, int *error);
EXTERNCPP void GetGeomInfoPtrs(int flag);
EXTERNCPP int InMesh(float *xyz);
EXTERNCPP void *ReadAllGeom(void *arg);
EXTERNCPP FILE_SIZE ReadGeom(geomdata *geomi, unsigned char *buffer, int nbuffer, int load_flag, int type, int *geom_frame_index);
EXTERNCPP FILE_SIZE ReadGeomData(patchdata *patchi, slicedata *slicei, int load_flag, int time_frame, float *time_value, int flag, int *errorcode);
EXTERNCPP void SetupReadAllGeom(void);
EXTERNCPP void ShowHideSortGeometry(int sort_geom, float *mm);
EXTERNCPP void UpdateAllGeomTriangles(void);
EXTERNCPP void UpdateGeomAreas(void);
EXTERNCPP void UpdateGeomNormals();
EXTERNCPP void UpdateTriangles(int time_flag, int update);

//*** IOhvac.c headers

EXTERNCPP void DrawHVACS(void);
EXTERNCPP void ReadHVACData(int flag);
EXTERNCPP void UpdateAllHVACColorLabels(void);
EXTERNCPP void UpdateHVACDuctColorLabels(int index);
EXTERNCPP void UpdateHVACNodeColorLabels(int index);

//*** IOiso.c headers

EXTERNCPP void DrawIso(int tranflag);
EXTERNCPP void DrawStaticIso(const isosurface *asurface,int surfacetype,
                             int smoothnorms, int trans_flag, int data_type,
                             float line_width);
EXTERNCPP void GetIsoLevels(const char *isofile, int dataflag, float **levelsptr, float ***colorlevelsptr, int *nisolevels);
EXTERNCPP meshdata *GetLoadedIsoMesh(void);
EXTERNCPP void OutputAllIsoBounds(void);
EXTERNCPP void UnloadIso(meshdata *meshi);
EXTERNCPP FILE_SIZE ReadIso(const char *file, int ifile, int flag, int *geom_frame_index, int *errorcode);
EXTERNCPP void ReadIsoGeomWrapup(int flag);
EXTERNCPP void ReadIsoOrig(const char *file, int ifile, int flag, int *errorcode);
EXTERNCPP void SetIsoLabels(float smin, float smax,
                    isodata *sd, int *errorcode);
EXTERNCPP void SortIsoTriangles(float *mm);
EXTERNCPP void UpdateIsoColors(void);
EXTERNCPP void UpdateIsoMenuLabels(void);
EXTERNCPP void UpdateIsoTriangles(int flag);
EXTERNCPP void UpdateIsoType(void);
EXTERNCPP void UpdateIsoTypes(void);
EXTERNCPP void UpdateIsoShowLevels(smv_case *scase, meshdata *isomesh);
EXTERNCPP void *UpdateTrianglesAll(void *arg);

//*** IOpart.c headers

EXTERNCPP void ClosePartFiles(void);
EXTERNCPP void CreatePartBoundFile(partdata *parti);
EXTERNCPP void CreatePartSizeFile(partdata *parti);
EXTERNCPP void DrawPartFrame(int mode);
EXTERNCPP void FinalizePartLoad(partdata *parti);
EXTERNCPP void GeneratePartHistograms(void);
EXTERNCPP int  GetMinPartFrames(int flag);
EXTERNCPP partpropdata *GetPartProp(char *label);
EXTERNCPP int  GetPartPropIndex(int class_i, int class_i_j);
EXTERNCPP int  GetPartPropIndexS(char *shortlabel);
EXTERNCPP void InitPartProp(void);
#ifdef _DEBUG
EXTERNCPP void PrintPartProp(void);
#endif
EXTERNCPP FILE_SIZE ReadPart(char *file, int ifile, int loadflag, int *errorcode);
EXTERNCPP void SetStreakShow(int show);
EXTERNCPP void *SortAllPartTags(void *arg);
EXTERNCPP void UpdatePartColors(partdata *parti, int flag);
EXTERNCPP void UpdatePartMenuLabels(void);

//*** IOplot2d.c headers

EXTERNCPP void DrawDevicePlots(void);
EXTERNCPP void DrawGenPlots(void);
EXTERNCPP void DrawPlot2D(int option, float *x, float *z, float *z2, int n,
  float highlight_x, float highlight_y, float highlight_y2, int valid, int position,
  float global_valmin, float global_valmax, char *quantity, char *quantity2, char *unit,
  float left, float right, float down, float top);
EXTERNCPP void DrawTreeDevicePlots(void);

EXTERNCPP csvdata *GetCsvCurve(int i, csvfiledata **csvf_ptr);
EXTERNCPP char *GetPlotShortLabel(plot2ddata *plot2di, int curv_index);
EXTERNCPP char *GetPlotShortLabel2(plot2ddata *plot2di, curvedata *curv);
EXTERNCPP char *GetPlotUnit(plot2ddata * plot2di, int curv_index);
EXTERNCPP char *GetPlotUnit2(plot2ddata *plot2di, curvedata *curve);
EXTERNCPP void GetPlot2DBounds(plot2ddata*plot2di, float *valmin, float *valmax);
EXTERNCPP int HavePlot2D(float **times, int *ntimes);
EXTERNCPP void InitPlot2D(plot2ddata *plot2di, int plot_index);
EXTERNCPP void SetupPlot2DUnitData(void);
EXTERNCPP void TimeAveragePlot2DData(float *times, float *vals, float *vals_avg, int nvals, float time_interval);

//*** IOplot3d.c headers

EXTERNCPP void DrawGrid(const meshdata *gb);
EXTERNCPP void DrawPlot3dFrame(void);
EXTERNCPP void GetPlot3DHists(plot3ddata *p);
EXTERNCPP int  GetPlot3dTime(float *time);
EXTERNCPP void GetPlot3dUVW(float xyz[3], float uvw[3]);
EXTERNCPP void InitPlot3dTimeList(void);
EXTERNCPP FILE_SIZE ReadPlot3D(char *file, int ifile, int flag,int *errorcode);
EXTERNCPP void UpdatePlot3DColors(plot3ddata *plot3di, int flag, int *errorcode);
EXTERNCPP void MergePlot3DHistograms(void);
EXTERNCPP void UpdateAllPlotSlices(void);
EXTERNCPP void UpdatePlotSlice(int slicedir);
EXTERNCPP void UpdatePlot3dMenuLabels(void);
EXTERNCPP void UpdateShowStep(int val, int slicedir);
EXTERNCPP void UpdateSurface(void);

//*** IOshooter.c headers

EXTERNCPP void DrawShooter(void);
EXTERNCPP void InitShooterData(void);
EXTERNCPP void SolveShooterData(void);

//*** IOslice.c headers

EXTERNCPP void ComputeLoadedSliceHist(char *label, float valmin, float valmax);
EXTERNCPP void DrawGSliceData(slicedata *slicei);
EXTERNCPP void DrawGSliceOutline(void);
EXTERNCPP void DrawSliceFrame(void);
EXTERNCPP void DrawSortSlices(void);
EXTERNCPP void DrawSortSlicesDebug(void);
EXTERNCPP void DrawVGSliceData(vslicedata *vslicei);
EXTERNCPP void DrawVolSliceLines(const slicedata *sd);
EXTERNCPP void DrawVolSliceVerts(const slicedata *sd);
EXTERNCPP void DrawVSliceFrame(void);
EXTERNCPP void GenerateSliceMenu(int from_commandline);
EXTERNCPP void GetGSliceParams(void);
EXTERNCPP void GetHistogramValProc(histogramdata *histogram, float cdf, float *val);
EXTERNCPP int  GetSliceBoundsIndex(const slicedata *sd);
EXTERNCPP int  GetSliceBoundsIndexFromLabel(char *label);
EXTERNCPP FILE_SIZE GetSliceData(slicedata *sd, const char *slicefilename, int time_frame, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr,
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int tload_step_arg, int tload_beg_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg);
EXTERNCPP void GetSliceDataBounds(slicedata *sd, float *pmin, float *pmax);
EXTERNCPP int  GetNSliceFrames(char *file, float *stime_min, float *stime_max);
EXTERNCPP void GetSliceParams2(void);
EXTERNCPP void GetSliceSizes(const char *slicefilenameptr, int time_frame, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int tload_step_arg,
  int *errorptr, int tload_beg_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr);
EXTERNCPP void HideSlices(char *longlabel);
EXTERNCPP void InitSliceData(void);
EXTERNCPP void MergeLoadedSliceHist(char *label, histogramdata **histptr);
EXTERNCPP int  NewMultiSlice(slicedata *sdold,slicedata *sd);
EXTERNCPP FILE_SIZE ReadSlice(const char *file, int ifile, int time_frame, float *time_value, int flag, int set_slicecolor, int *errorcode);
EXTERNCPP FILE_SIZE ReadVSlice(int ivslice, int time_frame, float *time_value, int flag, int set_slice_color, int *errorcode);
EXTERNCPP void SetSliceColors(float smin, float smax, slicedata *sd, int flag, int *errorcode);
EXTERNCPP void Slice2Device(void);
EXTERNCPP void SortSlices(void);
EXTERNCPP int  TimeAverageData(float **data_out, float **data_in, int ndata, int data_per_timestep, float *times_local, int ntimes_local, float average_time);
EXTERNCPP void UpdateAllSliceColors(int slicetype, int *errorcode);
EXTERNCPP void UpdateAllSliceLabels(int slicetype, int *errorcode);
EXTERNCPP void UpdateGslicePlanes(void);
EXTERNCPP void UpdateSliceBoundIndexes(void);
EXTERNCPP void UpdateSliceBoundLabels(void);
EXTERNCPP void UpdateSliceBounds(void);
EXTERNCPP void UpdateSliceContours(int slice_type_index, float line_min, float line_max, int nline_values);
EXTERNCPP void UpdateSliceDups(sliceparmdata *sp);
EXTERNCPP void UpdateSliceFilenum(void);
EXTERNCPP void UpdateSliceMenuShow(sliceparmdata *sp);
EXTERNCPP void UpdateSliceMenuLabels(sliceparmdata *sp);
EXTERNCPP void UpdateVectorSkip(int skip);
EXTERNCPP void UpdateVSliceDups(void);
EXTERNCPP void UpdateVsliceMenuLabels(sliceparmdata *sp);
EXTERNCPP void *UpdateVSlices(void *arg);
EXTERNCPP void UpdateVSliceBoundIndexes(void);

//*** IOsmoke.c headers

EXTERNCPP void DrawSmokeFrame(void);
EXTERNCPP void DrawVolSmokeFrame(void);
EXTERNCPP void GetSmoke3DTimeSteps(int fortran_skip, char *smokefile, int version, int *ntimes_found, int *ntimes_full);
EXTERNCPP int GetSmokeNFrames(int type, float *tmin, float *tmax);
EXTERNCPP void InitAlphas(unsigned char *smokealphanew,  unsigned char *firealphanew,
                          float base_extinct, int use_smoke_density, float maxval, float new_extinct,
                          float base_dx, float new_dx);
EXTERNCPP int  IsSmokeComponentPresent(smoke3ddata *smoke3di);
EXTERNCPP int  IsSmokeLoaded(smv_case *scase);
EXTERNCPP void MakeIBlankSmoke3D(void);
EXTERNCPP void MakeTimesMap(float *times, unsigned char **times_map_ptr, int n);
EXTERNCPP void MergeSmoke3D(smoke3ddata *smoke3dset);
EXTERNCPP void MergeSmoke3DAll(void);
EXTERNCPP FILE_SIZE ReadSmoke3D(int iframe, int ifile, int flag, int first_time, int *errorcode);
EXTERNCPP void ReadSmoke3DAllMeshes(int iframe, int smoketype, int *errorcode);
EXTERNCPP void SmokeWrapup(void);
EXTERNCPP int   UpdateSmoke3D(smoke3ddata *smoke3di);
EXTERNCPP void UpdateSmoke3dFileParms(void);
EXTERNCPP void UpdateSmoke3dMenuLabels(void);
EXTERNCPP void UpdateSmokeAlphas(void);


//*** IOtour.c headers

EXTERNCPP keyframe *AddFrame(keyframe *framei, float time, float pause_time, float *xyz, float view[3], int set_time);
EXTERNCPP tourdata *AddTour(char *label);
EXTERNCPP void CreateTourPaths(void);
EXTERNCPP keyframe *DeleteFrame(keyframe *step);
EXTERNCPP void DeleteTour(int tour_index);
EXTERNCPP void DrawSelectTours(void);
EXTERNCPP void DefaultTour(void);
EXTERNCPP void DrawTours(void);
EXTERNCPP void GetTourXYZ(float t, tourdata *this_tour, float *xyz);
EXTERNCPP void InitCircularTour(tourdata *touri, int nkeyframes, int option);
EXTERNCPP void NewSelect(keyframe *newselect);
EXTERNCPP void ReverseTour(char *label);
EXTERNCPP void SetupCircularTourNodes(void);
EXTERNCPP void SetupTour(void);
EXTERNCPP void UpdateTourMenuLabels(void);
EXTERNCPP void UpdateViewTour(void);

//*** IOwui.c headers

EXTERNCPP void DrawTerrainGeom(int option);
EXTERNCPP void DrawTerrainOBST(terraindata *terri, int flag);
EXTERNCPP void DrawTerrainOBSTSides(meshdata *meshi);
EXTERNCPP void DrawTerrainOBSTTexture(terraindata *terri);
EXTERNCPP void DrawNorth(void);
EXTERNCPP void DrawTrees(void);
EXTERNCPP void GenerateTerrainGeom(float **vertices_arg, unsigned int **indices_arg, int *nindices_arg);
EXTERNCPP float GetZCellValOffset(meshdata *meshi,float xval, float yval, int *loc);
EXTERNCPP int  HaveTerrainSlice(void);
EXTERNCPP int  HaveTerrainTexture(int *draw_surfaceptr);
EXTERNCPP void UpdateTerrain(int allocate_memory);
EXTERNCPP void UpdateTerrainColors(void);
EXTERNCPP void UpdateTerrainOptions(void);

//*** IOzone.c headers

EXTERNCPP void DrawZoneFireData(void);
EXTERNCPP void DrawZoneRoomData(void);
EXTERNCPP void DrawZoneRoomGeom(void);
EXTERNCPP void DrawZoneVentData(void);
EXTERNCPP void DrawZoneWallData(void);
EXTERNCPP void ReadZone(int ifile, int flag, int *errorcode);

//*** menu.c headers

EXTERNCPP int  AnySlices(const char *type);
EXTERNCPP int  AnySmoke(void);
EXTERNCPP void ApertureMenu(int value);
EXTERNCPP void BlockageMenu(int value);
EXTERNCPP void ColorbarMenu(int val);
EXTERNCPP void DialogMenu(int value);
EXTERNCPP void FontMenu(int value);
EXTERNCPP void FrameRateMenu(int var);
EXTERNCPP void GeometryMenu(int var);
EXTERNCPP void GetFileSizes(void);
EXTERNCPP  int GetInternalFaceShow(void);
EXTERNCPP  int GetNumActiveDevices(void);
EXTERNCPP void IsoShowMenu(int value);
EXTERNCPP void HVACDuctValueMenu(int value);
EXTERNCPP void HVACMenu(int value);
EXTERNCPP void HVACNodeValueMenu(int value);
EXTERNCPP void InitMenus(void);
EXTERNCPP void LabelMenu(int value);
EXTERNCPP FILE_SIZE LoadAllMSlicesMT(int last_slice, multislicedata *mslicei, int *fcount);
EXTERNCPP void LoadAllMultiSliceMenu(void);
EXTERNCPP void LoadAllMultiVSliceMenu(void);
EXTERNCPP void LoadAllPartFilesMT(int val);
EXTERNCPP void LoadAllPartFiles(int partnum);
EXTERNCPP int  LoadAllPlot3D(float time);
EXTERNCPP void LoadBoundaryMenu(int value);
EXTERNCPP void LoadHVACMenu(int value);
EXTERNCPP void LoadParticleMenu(int value);
EXTERNCPP void LoadPlot3dMenu(int value);
EXTERNCPP void LoadSliceMenu(int value);
EXTERNCPP FILE_SIZE LoadSmoke3D(int type, int frame, int *count, float *time_value);
EXTERNCPP void LoadSmoke3DMenu(int value);
EXTERNCPP void LoadUnloadMenu(int value);
EXTERNCPP void LoadVSliceMenu(int value);
EXTERNCPP FILE_SIZE LoadVSliceMenu2(int val);
EXTERNCPP void MenuStatusCB(int status, int x, int y);
EXTERNCPP void *MtLoadAllPartFiles(void *arg);
#ifdef _WIN32
EXTERNCPP void OpenSMVFile(char *filename,int filenamelength,int *openfile);
#endif
EXTERNCPP void ParticlePropShowMenu(int value);
EXTERNCPP void ParticleShowMenu(int value);
EXTERNCPP void ParticleStreakShowMenu(int var);
#ifdef pp_REFRESH
EXTERNCPP void PeriodicRefresh(int var);
#endif
EXTERNCPP void Plot3DListMenu(int value);
EXTERNCPP void Plot3DShowMenu(int value);
EXTERNCPP void PrintFileLoadTimes(int file_count, FILE_SIZE load_size, float load_time);
EXTERNCPP void ReloadAllSliceFiles(int load_flag);
EXTERNCPP void ReloadAllVectorSliceFiles(int load_flag);
EXTERNCPP void ReloadMenu(int value);
EXTERNCPP void RenderMenu(int value);
EXTERNCPP void RenderState(int onoff);
EXTERNCPP void ResetMenu(int var);
EXTERNCPP void SetTour(tourdata *thetour);
EXTERNCPP void ShowBoundaryMenu(int value);
EXTERNCPP void ShowDevicesMenu(int value);
EXTERNCPP void ShowHideMenu(int val);
EXTERNCPP void ShowHideSliceMenu(int var);
EXTERNCPP void ShowInternalBlockages(void);
EXTERNCPP void ShowObjectsMenu(int var);
EXTERNCPP void ShowVSliceMenu(int value);
EXTERNCPP void ScriptMenu(int var);
EXTERNCPP void SkipMenu(int value);
EXTERNCPP void Smoke3DShowMenu(int value);
EXTERNCPP void SmokeColorbarMenu(int var);
EXTERNCPP void ToggleMetroMode(void);
EXTERNCPP void TourMenu(int var);
EXTERNCPP void TrainerViewMenu(int var);
EXTERNCPP void TextureShowMenu(int value);
EXTERNCPP void UnloadAllSliceFiles(char *longlabel);
EXTERNCPP void UnloadSliceMenu(int value);
EXTERNCPP void UnLoadVolsmoke3DMenu(int value);
EXTERNCPP void UpdateMenu(void);
EXTERNCPP void UnloadVSliceMenu(int value);
EXTERNCPP void UpdateStreakValue(float value);
EXTERNCPP void ViewpointMenu(int value);
EXTERNCPP void ZoomMenu(int value);

//*** output.c headers

EXTERNCPP void DrawLabels(labels_collection *labelscoll);
EXTERNCPP  int GetFontHeight(void);
EXTERNCPP float GetStringLength(char *string);
EXTERNCPP int  GetStringWidth(char *string);
EXTERNCPP void Output3Text(float *color, float x, float y, float z, char *string);
EXTERNCPP void Output3TextRight(float *color, float x, float y, float z, char *string, float pad_length);
EXTERNCPP void Output3Val(float x, float y, float z, float val);
EXTERNCPP void OutputAxisLabels(void);
EXTERNCPP void OutputBarText(float x, float y, const GLfloat *color, char *string);
EXTERNCPP void OutputText(float x, float y, char *string);
EXTERNCPP void OutputTextColor(float *fontcolor, float x, float y, char *string);
EXTERNCPP void ScaleFont3D(void);

//*** readsmv.c headers

EXTERNCPP void *CheckFiles(void *arg);
EXTERNCPP void *Compress(void *arg);
EXTERNCPP void GetBoxSkyCorners(void);
EXTERNCPP void GetSliceParmInfo(sliceparmdata *sp);
EXTERNCPP void InitCellMeshInfo(void);
EXTERNCPP FILE_SIZE ReadAllCSVFiles(int flag);
EXTERNCPP int  ReadBinIni(void);
EXTERNCPP int  ReadIni(char *inifile);
EXTERNCPP int  ReadSMV(bufferstreamdata *stream);
EXTERNCPP void SetBoundBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *buffer2);
EXTERNCPP void SetPatchMin(int set_valmin, float valmin, char *buffer2);
EXTERNCPP void SetPatchMax(int set_valmax, float valmax, char *buffer2);
EXTERNCPP void SetSliceBounds(int set_valmin, float valmin, int set_valmax, float valmax, char *buffer2);
EXTERNCPP void SetSliceMin(int set_valmin, float valmin, char *buffer2);
EXTERNCPP void SetSliceMax(int set_valmax, float valmax, char *buffer2);
EXTERNCPP void SetSliceParmInfo(sliceparmdata *sp);
EXTERNCPP void *SetupAllIsosurfaces(void *arg);
EXTERNCPP void UpdateBlockType(void);
EXTERNCPP void UpdateLoadedLists(void);
EXTERNCPP void UpdateSMVDynamic(char *file);
EXTERNCPP void UpdateUseTextures(void);
EXTERNCPP void UpdateVentOffset(void);
EXTERNCPP void WriteIni(int flag,char *file);

//*** renderhtml.c headers

EXTERNCPP int Obst2Data(char *html_file);
EXTERNCPP int SliceNode2Data(char *html_file, int option);
EXTERNCPP int SliceCell2Data(char *html_file, int option);
EXTERNCPP int Smv2Html(char *html_out, int option, int from_where);
EXTERNCPP int Smv2Geom(char *html_file);

//*** renderimage.c headers

#ifdef pp_RENDER360_DEBUG
EXTERNCPP void DrawScreenInfo(void);
#endif
EXTERNCPP char *GetMovieFilePath(char *moviefile_path);
EXTERNCPP void GetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high);
EXTERNCPP GLubyte *GetScreenBuffer(void);
EXTERNCPP void MakeMovie(void);
EXTERNCPP int  MergeRenderScreenBuffers(int nscreen_rows, GLubyte **screenbuffers);
EXTERNCPP int  MergeRenderScreenBuffers360(void);
EXTERNCPP void *PlayMovie(void *arg);
EXTERNCPP void Render(int view_mode);
EXTERNCPP void RenderFrame(int view_mode);
#if pp_GIF_ANIMATED
EXTERNCPP int  GifStart(const char *path);
EXTERNCPP int  GifEnd();
EXTERNCPP int  GifAddFrame(int delay);
#endif
EXTERNCPP void ResetRenderResolution(int *width_low, int *height_low, int *width_high, int *height_high);
EXTERNCPP void *SetupFF(void *arg);
EXTERNCPP void SetupScreeninfo(void);
EXTERNCPP int  SmokeviewImage2File(char *directory, char *GIFfilename, int rendertype, int woffset, int width, int hoffset, int height);

//*** shaders.c headers

#ifdef pp_GPU
EXTERNCPP int  InitShaders(void);
EXTERNCPP void LoadSmokeShaders(void);
EXTERNCPP void Load3DSliceShaders(void);
EXTERNCPP void LoadZoneSmokeShaders(void);
EXTERNCPP void LoadVolsmokeShaders(void);
EXTERNCPP void UnLoadShaders(void);
#endif

//*** showscene.c headers

EXTERNCPP void ShowScene(int mode, int view_mode, int quad, GLint s_left, GLint s_down, screendata *screen);

//*** skybox.c headers

EXTERNCPP void DrawSkybox(void);
EXTERNCPP void FreeSkybox(void);
EXTERNCPP void LoadSkyTexture(char *filebase, texturedata *texti);

//*** smokeview.c headers

EXTERNCPP void AntiAliasLine(int flag);
EXTERNCPP int BuildGbndFile(int file_type);
EXTERNCPP void DisplayVersionInfo(char *progname, common_opts *opts);
EXTERNCPP void InitVolrenderScript(char *prefix, char *tour_label, int startframe, int skipframe);
EXTERNCPP int IsFDSRunning(FILE_SIZE *last_size);
EXTERNCPP void SetViewPoint(int option);
EXTERNCPP void SMV_EXIT(int code);
EXTERNCPP void StartTimer(float *timerptr);
EXTERNCPP void TransparentOff(void);
EXTERNCPP void TransparentOn(void);
EXTERNCPP void UpdateLights(float *pos1, float *pos2);

//*** smv_geometry.c headers

EXTERNCPP int  BoxInFrustum(float *xx, float *yy, float *zz, int n);
EXTERNCPP void DrawFilledTetra(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor);
EXTERNCPP void DrawFilled2Tetra(float *v1, float *v2, float *v3, float *v4,
   unsigned char *rgb0color,unsigned char *rgb1color,unsigned char *rgb2color,unsigned char *rgb3color,int *vis_state);
EXTERNCPP void DrawTetraOutline(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor);
EXTERNCPP void ExtractFrustum(void);
EXTERNCPP void GetInverse(float *m, float *mi);
EXTERNCPP meshdata *GetMesh(float *xyz);
EXTERNCPP meshdata *GetMeshNoFail(float *xyz);
EXTERNCPP void GetNewPos(float *oldpos, float dx, float dy, float dz, float speed_factor);
EXTERNCPP void GetScreenMapping(float *xyz0, float *screen_perm);
EXTERNCPP int  GetTimeInterval(float val, float *array, int n);
EXTERNCPP int  InExterior(float *xyz);
EXTERNCPP void InitClip(void);
EXTERNCPP void InitTetraClipInfo(clipdata *ci,float *v1, float *v2, float *v3, float *v4);
EXTERNCPP void MatMultMat(float *m1, float *m2, float *m3);
EXTERNCPP int  MakeIBlank(void);
EXTERNCPP int  MakeIBlankCarve(void);
EXTERNCPP void MergeClipPlanes(clipdata *ci, clipdata *cj);
EXTERNCPP int  MeshInFrustum(meshdata *meshi);
EXTERNCPP int  RectangleInFrustum(float *x11, float *x12, float *x22, float *x21);
EXTERNCPP void SetClipPlanes(clipdata *ci, int option);
EXTERNCPP void Slerp(float *p0, float *p1, float t, float *pout);
EXTERNCPP void UpdatePlotxyzAll(void);
EXTERNCPP int  OnMeshBoundary(float *xyz);

//*** startup.c headers

EXTERNCPP void FreeVars(void);
EXTERNCPP void GetStartupVSlice(int seq_id);
EXTERNCPP void GetStartupSlice(int seq_id);
EXTERNCPP void GetStartupPart(int seq_id);
EXTERNCPP void GetStartupPlot3D(int seq_id);
EXTERNCPP void GetStartupSmoke(int seq_id);
EXTERNCPP void GetStartupISO(int seq_id);
EXTERNCPP void GetStartupBoundary(int seq_id);
EXTERNCPP int GLUTGetScreenWidth(void);
EXTERNCPP int GLUTGetScreenHeight(void);
EXTERNCPP void InitOpenGL(int option);
EXTERNCPP void InitScriptErrorFiles(void);
EXTERNCPP void InitStartupDirs(void);
EXTERNCPP void InitTextureDir(void);
EXTERNCPP void InitVars(void);
EXTERNCPP void LoadFiles(void);
EXTERNCPP void PutStartupSmoke3D(FILE *fileout);
EXTERNCPP void Set3DSmokeStartup(void);
EXTERNCPP void SetupGlut(int argc, char **argv);
EXTERNCPP int  SetupCase(char *file);

//*** unit.c headers

EXTERNCPP f_units *GetUnitClass(char *unit);
EXTERNCPP void GetUnitInfo(const char *unitlabel, int *unitclass, int *unittype);
EXTERNCPP float GetUnitVal(const char *unitlabel, float oldval, int ndecimals);
EXTERNCPP void InitUnitDefs(void);
EXTERNCPP void InitUnits(void);
EXTERNCPP void SetUnitVis(void);
EXTERNCPP void UpdateUnitDefs(void);

//*** update.c headers

EXTERNCPP int CompareFloat(const void *arg1, const void *arg2);
EXTERNCPP void EnableDisableMakeMovie(int onoff);
EXTERNCPP void EnableDisablePlayMovie(void);
EXTERNCPP int GetIndex(float key, const float *list, int nlist);
EXTERNCPP int GetPlotState(int choice);
EXTERNCPP float GetTime(void);
EXTERNCPP void HandleMakeMovie(void);
EXTERNCPP int HaveFireLoaded(void);
EXTERNCPP int HaveSootLoaded(void);
EXTERNCPP int ISearch(float *list, int nlist, float key, int guess);
EXTERNCPP void OutputMinMax(char *meshlabel, char *label, char *unit, float valmin_fds, float valmax_fds, float valmin_smv, float valmax_smv);
EXTERNCPP void PauseTime(float pause_time);
EXTERNCPP void ResetItimes0(void);
EXTERNCPP void ShiftColorbars(void);
EXTERNCPP void SynchTimes(void);
EXTERNCPP void UpdateClipbounds(int set_i0, int *i0, int set_i1, int *i1, int maxi);
EXTERNCPP int GetColorbarState(void);
EXTERNCPP void UpdateColorTable(colortabledata *ctableinfo, int nctableinfo);
EXTERNCPP void UpdateDisplay(void);
EXTERNCPP void UpdateFrameNumber(int changetime);
EXTERNCPP void UpdateShow(void);
EXTERNCPP void UpdateShowScene(void);
EXTERNCPP void UpdateTimes(void);
#endif
