#ifndef SMOKEHEADERS_H_DEFINED
#define SMOKEHEADERS_H_DEFINED

#ifdef pp_LUA
#include "gd.h"
#endif

EXTERNCPP void ColorbarCB(int var);
EXTERNCPP void UpdateOpacityMap(void);
EXTERNCPP void ShowBoundsDialog(int type);
#ifdef  pp_SPECTRAL
EXTERNCPP void GetBlackBodyColors(float tmin, float tmax, float *intensities, int n);
#endif
EXTERNCPP void UpdateFreeze(int val);
EXTERNCPP void UpdateLoadtimeVal(float val);
EXTERNCPP void UpdateTimeFrameBounds(float time_min, float time_max);
EXTERNCPP int GetVolFrameMax(int meshnum);
EXTERNCPP void UpdateLoadframeVal(int frames);
EXTERNCPP void UpdateLoadframeMax(int max_frames);
EXTERNCPP void UpdateShowRotationCenter(void);
EXTERNCPP void UpdateShowRotationCenter2(void);
EXTERNCPP void UpdateGluiRotateAbout(int val);
EXTERNCPP void UpdateRotationIndex(int val);
EXTERNCPP void ReloadAllSliceFiles(void);
EXTERNCPP void UpdateWindRoseDevices(int option);
EXTERNCPP void ParticleStreakShowMenu(int var);
EXTERNCPP void update_geom_normals();
EXTERNCPP void Plot3DListMenu(int value);
EXTERNCPP void init_shooter_data(void);
EXTERNCPP void ScriptCB(int var);
EXTERNCPP void TourCB(int var);
EXTERNCPP void SetClipControls(int val);
EXTERNCPP void ExtremeCB(int var);
EXTERNCPP void AddColorbarList2(int index, char *label);
EXTERNCPP void UpdateExtremeVals(void);
EXTERNCPP void PartCB(int var);
EXTERNCPP void ShowHideMenu(int val);
EXTERNCPP void Plot3DCB(int var);
EXTERNCPP void ColorbarGlobal2Local(void);
EXTERNCPP void UpdateShowbeamAsLine(void);
EXTERNCPP void UpdateVSliceDups(void);
EXTERNCPP void UnloadVSliceMenu(int value);
EXTERNCPP void UpdateSliceDups(void);
EXTERNCPP void IsoCB(int var);
EXTERNCPP void UpdateScriptStep(void);
EXTERNCPP void UnLoadVolsmoke3DMenu(int value);
EXTERNCPP void UpdateGluiRender(void);
EXTERNCPP void AddScriptList(char *file, int id);
EXTERNCPP void UpdateMenu(void);
EXTERNCPP void Volume_CB(int var);
EXTERNCPP void DrawSmokeTest(void);
EXTERNCPP void Smoke3d_CB(int var);
EXTERNCPP void InitAllLightFractions(float *xyz_light, int light_type);
EXTERNCPP void GetAllSliceHists(void);
EXTERNCPP void UpdateHistogramType(void);
EXTERNCPP void UpdateSliceHist(void);
EXTERNCPP void Enable360Zoom(void);
#ifdef pp_RENDER360_DEBUG
EXTERNCPP void DrawScreenInfo(void);
#endif
EXTERNCPP void UpdateShowSliceInObst(void);
EXTERNCPP void get_geom_zbounds(float *zmin, float *zmax);
EXTERNCPP void GetPartHistogram(int flag);
EXTERNCPP void write_part_histogram(partdata *parti);
EXTERNCPP void read_part_histogram(partdata *parti);
EXTERNCPP void MakeIBlankAll(void);
EXTERNCPP void UpdateSliceDupDialog(void);
EXTERNCPP void DrawNorth(void);
EXTERNCPP void draw_geomdata(int flag, patchdata *patchi, int geom_type);
EXTERNCPP void UpdateCurrentColorbar(colorbardata *cb);
EXTERNCPP int HaveFire(void);
EXTERNCPP void update_object_used(void);
EXTERNCPP void UpdateColorTableList(int ncolortableinfo_old);
EXTERNCPP void UpdateColorTable(colortabledata *ctableinfo, int nctableinfo);
EXTERNCPP colortabledata *GetColorTable(char *label);
EXTERNCPP void UpdateIsoColorlevel(void);
EXTERNCPP void readiso_geom_wrapup(void);
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
EXTERNCPP void UpdateGvecDown(int gvec_down_local);
EXTERNCPP void DrawGravityAxis(void);
EXTERNCPP void xyz2azelev(float *xyz,float *azimuth, float *elevation);
EXTERNCPP void get_geom_dialog_state(void);
EXTERNCPP void UpdateDeviceOrientation(void);
EXTERNCPP void UpdateGluiDevices(void);
EXTERNCPP void update_colordevs(void);
EXTERNCPP void update_visaxislabels(void);
EXTERNCPP void update_geometry_controls(void);
EXTERNCPP void InitVolrenderScript(char *prefix, char *tour_label, int startframe, int skipframe);

// glui headers

EXTERNCPP void UpdateGluiZoneBounds(void);
EXTERNCPP void glui_3dsmoke_setup(int main_window);
EXTERNCPP void GluiBoundsSetup(int main_window);
EXTERNCPP void GluiClipSetup(int main_window);
EXTERNCPP void GluiColorbarSetup(int main_window);
EXTERNCPP void GluiDeviceSetup(int main_window);
EXTERNCPP void glui_geometry_setup(int main_window);
EXTERNCPP void GluiLabelsSetup(int main_window);
EXTERNCPP void gluiMotionSetup(int main_window);
EXTERNCPP void glui_shooter_setup(int main_window);
EXTERNCPP void glui_stereo_setup(int main_window);
EXTERNCPP void GluiTourSetup(int main_window);
EXTERNCPP void glui_trainer_setup(int main_window);
EXTERNCPP void glui_wui_setup(int main_window);

EXTERNCPP void GluiUpdateFontIndex(void);
EXTERNCPP void GluiScriptDisable(void);
EXTERNCPP void GluiScriptEnable(void);
EXTERNCPP void glui_alert_setup(int main_window);
EXTERNCPP void gluiIdle(void);
EXTERNCPP void gluiIdleNULL(void);
EXTERNCPP void UpdateGluiSetViewXYZ(float *xyz);
EXTERNCPP void UpdateGluiFileLabel(int var);
EXTERNCPP void UpdateGluiVecFactor(void);
EXTERNCPP void UpdateGluiKeyframe(void);
EXTERNCPP void UpdateGluiPatchUnits(void);
EXTERNCPP void UpdateGluiSliceUnits(void);
EXTERNCPP void UpdateGluiPlot3D(void);
EXTERNCPP void UpdateGluiPlot3D_units(void);
EXTERNCPP void UpdateGluiPlot3Dtype(void);
EXTERNCPP void UpdateGluiIsotype(void);
EXTERNCPP void update_glui_viewlist(void);
EXTERNCPP void Update_Glui_Wui(void);
EXTERNCPP void Update_Glui_Stereo(void);
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
EXTERNCPP void ShowGluiWui(void);
EXTERNCPP void HideGluiWui(void);
EXTERNCPP void ShowGluiDisplay(int menu_id);
EXTERNCPP void ShowGluiDevice(void);
EXTERNCPP void HideGluiDevice(void);
EXTERNCPP void SetLabelControls(void);
EXTERNCPP void SetColorControls(void);
EXTERNCPP void HideGluiDisplay(void);
EXTERNCPP void ShowGluiTour(void);
EXTERNCPP void HideGluiTour(void);
EXTERNCPP void ShowGluiStereo(void);
EXTERNCPP void HideGluiStereo(void);

EXTERNCPP void EnableBoundaryGlui(void);
EXTERNCPP void DisableBoundaryGlui(void);
EXTERNCPP void UpdateClipPlanes(void);
EXTERNCPP void ShowGluiBounds(int menu_id);
EXTERNCPP void HideGluiBounds(void);
EXTERNCPP void ShowGluiGeometry(void);
EXTERNCPP void HideGluiGeometry(void);

EXTERNCPP void UpdateAllPatchColors(void);
EXTERNCPP void UpdateSliceListIndex(int sfn);
EXTERNCPP void UpdatePatchListIndex(int patchfilenum);
EXTERNCPP void UpdatePatchListIndex2(char *label);
EXTERNCPP void UpdatePlot3dListIndex(void);

EXTERNCPP void GetSliceParams2(void);

EXTERNCPP void DrawWindRosesDevices(void);
EXTERNCPP void DeviceData2WindRose(int nr, int ntheta, int flag);
EXTERNCPP void DefineAllFEDs(void);
EXTERNCPP void UpdateTourState(void);
EXTERNCPP void UpdateEditTour(void);
EXTERNCPP void AddDeleteKeyframe(int flag);
EXTERNCPP void UpdateTourParms(void);
EXTERNCPP void Slerp(float *p0, float *p1, float t, float *pout);
EXTERNCPP void draw_test_clip(void);
EXTERNCPP void draw_test_triangle(void);
EXTERNCPP void draw_test_polygon(void);
EXTERNCPP void draw_test_outline(void);
EXTERNCPP void draw_geom_cutcells(void);
EXTERNCPP void VentMenu(int value);
EXTERNCPP void MergeClipPlanes(clipdata *ci, clipdata *cj);
EXTERNCPP void InitBoxClipInfo(clipdata *ci,float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
EXTERNCPP void InitTetraClipInfo(clipdata *ci,float *v1, float *v2, float *v3, float *v4);
EXTERNCPP void InitClip(void);
EXTERNCPP void SetClipPlanes(clipdata *ci, int option);

EXTERNCPP void DrawFilledTetra(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor);
EXTERNCPP void DrawFilled2Tetra(float *v1, float *v2, float *v3, float *v4,
   unsigned char *rgb0color,unsigned char *rgb1color,unsigned char *rgb2color,unsigned char *rgb3color,int *vis_state);
EXTERNCPP void DrawTetraOutline(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor);
EXTERNCPP void drawfilledcircle(float diameter, unsigned char *rgbcolor, circdata *circinfo);
EXTERNCPP void drawcubec(float size, unsigned char *rgbcolor);
EXTERNCPP void drawcubec_outline(float size, unsigned char *rgbcolor);
EXTERNCPP void drawbox_outline(float x1, float x2, float y1, float y2, float z1, float z2, float *rgbcolor);
EXTERNCPP void drawcircle(float diameter, unsigned char *rgbcolor, circdata *circinfo);
EXTERNCPP void drawfilledrectangle(float width, float height, unsigned char *rgbcolor);
EXTERNCPP void drawrectangle(float width, float height, unsigned char *rgbcolor);
EXTERNCPP void DrawCircVents(int option);
EXTERNCPP void UpdateSmokeColormap(int option);
EXTERNCPP void DefineVolsmokeTextures(void);
EXTERNCPP void SetColorbarListIndex(int val);
EXTERNCPP int GetColorbarListIndex(void);
EXTERNCPP int GetColorbarIndex(int flag, int x, int y);
EXTERNCPP void GetViewportInfo(void);

EXTERNCPP void ScaleFont2D(void);
EXTERNCPP void ScaleFont3D(void);
EXTERNCPP int LabelGetNUserLabels(void);
EXTERNCPP labeldata *LabelNext(labeldata *gl);
EXTERNCPP labeldata *LabelPrevious(labeldata *gl);
EXTERNCPP int LabelInit(labeldata *gl);
EXTERNCPP void LabelResort(labeldata *label);
EXTERNCPP void LabelCopy(labeldata *label_to, labeldata *label_from);
EXTERNCPP labeldata *LabelGet(char *name);
EXTERNCPP void LabelDelete(labeldata *label);
EXTERNCPP void LabelPrint(void);
EXTERNCPP labeldata *LabelInsert(labeldata *labeltemp);

EXTERNCPP void UpdateNRenderRows(void);
EXTERNCPP void rotateu2v(float *u, float *v, float *axis, float *angle);
EXTERNCPP void RotationTypeCB(int var);
EXTERNCPP void UpdateRotationType(int val);

EXTERNCPP void angleaxis2quat(float angle, float *axis, float *quat);
EXTERNCPP void quat2rot(float quat[4],float rot[16]);
EXTERNCPP void mult_quat(float x[4], float y[4], float z[4]);
EXTERNCPP void normalize_quat(float x[4]);

EXTERNCPP void SetScreenSize(int *width, int *height);
EXTERNCPP void KeyboardCB(unsigned char key, int x, int y);
EXTERNCPP void KeyboardUpCB(unsigned char key, int x, int y);
EXTERNCPP void ReshapeCB(int width, int height);
EXTERNCPP void DisplayCB(void);
EXTERNCPP void SpecialKeyboardCB(int key, int x, int y);
EXTERNCPP void SpecialKeyboardUpCB(int key, int x, int y);
EXTERNCPP void MouseCB(int button, int state, int x, int y);
EXTERNCPP void MotionCB(int xm, int ym);
EXTERNCPP void MenuStatus_CB(int status, int x, int y);
EXTERNCPP void IdleCB(void);

SVEXTERN void UpdateVectorWidgets(void);
EXTERNCPP void UpdateGsliceParms(void);
EXTERNCPP void readiso_orig(const char *file, int ifile, int flag, int *errorcode);
EXTERNCPP void UpdatePlotxyzAll(void);
EXTERNCPP void update_isocolors(void);
EXTERNCPP void get_faceinfo(void);
EXTERNCPP void GetGeomInfoPtrs(geomdata ***geominfoptrs_local,int *ngeominfoptrs_local);
EXTERNCPP devicedata *getdevice(char *label, int index);
EXTERNCPP void SetupGlut(int argc, char **argv);
EXTERNCPP int get_ndevices(char *file);
EXTERNCPP void ReadHRR(int flag, int *errorcode);
EXTERNCPP void read_device_data(char *file, int filetype, int flag);
EXTERNCPP void setup_zone_devs(void);
EXTERNCPP void setup_device_data(void);
EXTERNCPP void draw_geom(int flag,int frameflag);
EXTERNCPP void draw_geomdiag(void);
EXTERNCPP void getzonesizecsv(int *nzone_times, int *nroom2, int *nfires, int *nzhvents, int *nzvvents, int *nzmvents, int *error);
EXTERNCPP void getzoneventbounds(void);
EXTERNCPP void RemoveDupBlockages(void);
EXTERNCPP void Sort_Iso_Triangles(float *mm);
EXTERNCPP void Update_Isotris(int flag);
EXTERNCPP void UpdateEvacParms(void);
EXTERNCPP void UpdateSliceMenuShow(void);
EXTERNCPP void UpdatePatchBounds(patchdata *patchi);
EXTERNCPP void UpdateAllPatchBounds(void);
EXTERNCPP void UpdateAllPatchBoundsST(void);
EXTERNCPP int update_patch_hist(patchdata *patchi);
EXTERNCPP void update_hidepatchsurface(void);
EXTERNCPP int LastSliceLoadstack(void);
EXTERNCPP int LastVSliceLoadstack(void);
EXTERNCPP void UpdateAxisLabelsSmooth(void);
EXTERNCPP void UpdateTransparency(void);
EXTERNCPP void UpdateScriptStart(void);
EXTERNCPP void UpdateResearchMode(void);
EXTERNCPP void UpdateScriptStop(void);
EXTERNCPP void UpdateDefer(void);
EXTERNCPP void UpdateTBounds(void);
EXTERNCPP void UpdateGluiTimeBounds(float time_min, float time_max);
EXTERNCPP void settimeval(float timeval);
EXTERNCPP void get_indep_var_indices(sv_object *smv_object,char **var_indep_strings, int nvars_indep,int *index);
EXTERNCPP void get_evac_indices(sv_object *smv_object, int *evac_index,int *nevac_index);
EXTERNCPP void UpdateColorbarList(void);
EXTERNCPP void UpdateColorbarList2(void);
EXTERNCPP void UpdateColorbarFlip(void);

EXTERNCPP void LoadSmokeFrame(int meshnum, int framenum);
EXTERNCPP void LoadTimeFrame(int meshnum, float timeval);
EXTERNCPP void script_loadvolsmokeframe2(void);
EXTERNCPP void script_loadisoframe2(scriptdata *scripti);
EXTERNCPP void init_device_plane(devicedata *devicei);
EXTERNCPP void draw_devices_val(void);
EXTERNCPP void getsmokesensors(void);
EXTERNCPP void AddNewTour(void);
EXTERNCPP void start_script(void);
EXTERNCPP int run_script(void);
EXTERNCPP int compile_script(char *scriptfile);
EXTERNCPP scriptfiledata *insert_scriptfile(char *file);
#ifdef pp_LUA
EXTERNCPP luascriptfiledata *insert_luascriptfile(char *file);
#endif
EXTERNCPP char *get_inifilename(int id);
EXTERNCPP char *get_scriptfilename(int id);
EXTERNCPP inifiledata *insert_inifile(char *file);
EXTERNCPP void Keyboard(unsigned char key, int flag);
EXTERNCPP void get_newscriptfilename(char *newscriptfilename);
EXTERNCPP void init_avatar(void);
EXTERNCPP void draw_select_avatars(void);
EXTERNCPP void ReadTerrain(char *file, int ifile, int flag, int *errorcode);
EXTERNCPP void OutputFedCSV(void);
EXTERNCPP void ParticlePropShowMenu(int value);
EXTERNCPP int GetGridIndex(float x, int dir, float *plotxyz, int nplotxyz);
EXTERNCPP void UpdateSliceContours(int slice_type_index, float line_min, float line_max, int nline_values);
EXTERNCPP void ScriptMenu(int var);
EXTERNCPP void SmokeColorbarMenu(int var);
EXTERNCPP void  OBJECT_CB(int flag);
EXTERNCPP void WUI_CB(int var);
EXTERNCPP void CompressOnOff(int flag);
EXTERNCPP void CompressSVZip2(void);
EXTERNCPP void UpdateTerrainColors(void);
EXTERNCPP void DrawTerrain(terraindata *terri, int only_geom);
EXTERNCPP void DrawTerrainTexture(terraindata *terri, int only_geom);
EXTERNCPP void DrawTrees(void);
EXTERNCPP void InitCullGeom(int cullflag);
EXTERNCPP void GetCullSkips(meshdata *meshi, int cullflag, int cull_portsize, int *iiskip, int *jjskip, int *kkskip);
#ifdef pp_CULL
EXTERNCPP void InitCull(int cullflag);
EXTERNCPP void InitCullPlane(int cullflag);
EXTERNCPP void GetPixelCount(void);
EXTERNCPP int InitCullExts(void);
#endif
#ifdef pp_GPU
#ifdef pp_GPUDEPTH
EXTERNCPP void GetDepthTexture( void );
EXTERNCPP void CreateDepthTexture( void );
#endif
EXTERNCPP int InitShaders(void);
EXTERNCPP void LoadSmokeShaders(void);
EXTERNCPP void Load3DSliceShaders(void);
EXTERNCPP void LoadZoneSmokeShaders(void);
EXTERNCPP void LoadVolsmokeShaders(void);
EXTERNCPP void UnLoadShaders(void);
#endif
EXTERNCPP void NextXIndex(int inc,int flag);
EXTERNCPP void NextYIndex(int inc,int flag);
EXTERNCPP void NextZIndex(int inc,int flag);
EXTERNCPP void Init_Sphere(int nlat, int nlong);
EXTERNCPP void Init_Circle(unsigned int npoints, circdata *circinfo);
EXTERNCPP int HaveTerrainSlice(void);
EXTERNCPP float GetZCellValOffset(meshdata *meshi,float xval, float yval, int *loc);
EXTERNCPP void UpdateCameraYpos(cameradata *camera_data);
EXTERNCPP cameradata *GetCamera(char *name);
EXTERNCPP void SortCameras(void);
EXTERNCPP char *GetCameraLabel(int index);
EXTERNCPP void Clip2Cam(cameradata *cam);
EXTERNCPP void Cam2Clip(cameradata *cam);
EXTERNCPP void init_object_defs(void);
EXTERNCPP void update_device_textures(void);
EXTERNCPP char *get_device_label(char *buffer);
EXTERNCPP void GetElevAz(float *xyznorm,float *dtheta, float *rotate_axis, float *dpsi);
EXTERNCPP void drawTargetNorm(void);
EXTERNCPP void draw_devices(void);
EXTERNCPP sv_object *parse_SVOBJECT(FILE *stream);
EXTERNCPP sv_object *init_SVOBJECT1(char *label, char *commands,int visible);
EXTERNCPP sv_object *init_SVOBJECT2(char *label, char *commandson, char *commandsoff,int visible);
EXTERNCPP sv_object *get_SVOBJECT_type(char *label, sv_object *default_object);
EXTERNCPP sv_object *get_SVOBJECT_type2(char *label, sv_object *default_object);
EXTERNCPP int read_object_defs(char *file);
EXTERNCPP void freeall_objects(void);
EXTERNCPP void parse_object_string(char *string, char **tokens, int *ntokens);
EXTERNCPP void update_partclass_depend(partclassdata *partclassi);

EXTERNCPP int get_plot3d_index(meshdata *meshi, int dir, float val);
EXTERNCPP int plot3dlistcompare( const void *arg1, const void *arg2 );
EXTERNCPP int plot3dcompare( const void *arg1, const void *arg2 );
EXTERNCPP void update_plot_xyz(meshdata *current_mesh);
EXTERNCPP void updateplotslice_mesh(meshdata *mesh_in, int slicedir);

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
EXTERNCPP void InitOpenGL(void);
EXTERNCPP void TextureShowMenu(int value);
EXTERNCPP void CopyArgs(int *argc, char **aargv, char ***argv_sv);
EXTERNCPP void InitUserTicks(void);
EXTERNCPP void DrawUserTicks(void);
EXTERNCPP int get_tick_dir(float *mm);
EXTERNCPP void InitMultiThreading(void);
#ifdef WIN32
EXTERNCPP void OpenSMVFile(char *filename,int filenamelength,int *openfile);
#endif
EXTERNCPP int AnySmoke(char *type);
EXTERNCPP int AnySlices(char *type);
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
EXTERNCPP void ShowPatchMenu(int value);
EXTERNCPP void Smoke3DShowMenu(int value);
EXTERNCPP void ShowVSliceMenu(int value);
EXTERNCPP partpropdata *get_partprop_s(char *label);
EXTERNCPP int get_partprop_index_s(char *shortlabel);
EXTERNCPP int get_partprop_index(char *label);
#ifdef _DEBUG
EXTERNCPP void print_partprop(void);
#endif
EXTERNCPP partpropdata *get_partprop(char *label);
EXTERNCPP void init_partprop(void);
EXTERNCPP void UpdateStreakValue(float value);
EXTERNCPP void LoadParticleMenu(int value);
EXTERNCPP void LoadEvacMenu(int value);
EXTERNCPP void LoadPatchMenu(int value);
EXTERNCPP void LoadSliceMenu(int value);
EXTERNCPP void LoadVSliceMenu(int value);

EXTERNCPP void InitVars(void);
EXTERNCPP void RenderState(int onoff);
EXTERNCPP void UpdateWindowSizeList(void);
EXTERNCPP void ResizeWindow(int width, int height);
EXTERNCPP void update_trainer_outline(void);
EXTERNCPP void update_trainer_moves(void);
EXTERNCPP meshdata *GetMesh(float *xyz);
EXTERNCPP meshdata *GetMeshNoFail(float *xyz);
EXTERNCPP int OnMeshBoundary(float *xyz);

EXTERNCPP void Render_CB(int var);
EXTERNCPP sv_object *get_object(char *label);
EXTERNCPP void SnapScene(void);
EXTERNCPP void LevelScene(int level_x, int level_y, float *quat);
EXTERNCPP void get_plot3d_uvw(float xyz[3], float uvw[3]);
EXTERNCPP void solve_shooter_data(void);
EXTERNCPP void increment_shooter_data(shootpointdata *pold, shootpointdata *pnew, float dt);
EXTERNCPP void draw_shooter(void);
EXTERNCPP void LoadFiles(void);
EXTERNCPP void GetStartupVSlice(int seq_id);
EXTERNCPP void GetStartupSlice(int seq_id);
EXTERNCPP void GetStartupPart(int seq_id);
EXTERNCPP void GetStartupPlot3d(int seq_id);
EXTERNCPP void GetStartupSmoke(int seq_id);
EXTERNCPP void GetStartupISO(int seq_id);
EXTERNCPP void GetStartupPatch(int seq_id);
EXTERNCPP void Set3DSmokeStartup(void);
EXTERNCPP void PutStartupSmoke3d(FILE *fileout);
EXTERNCPP void drawonlythreshold(const meshdata *meshi);
EXTERNCPP void DrawTransparentFaces(void);
EXTERNCPP int IsBlockageVisible(blockagedata *bc, float time);
EXTERNCPP float Zoom2Aperture(float zoom0);
EXTERNCPP float Aperture2Zoom(float ap);
EXTERNCPP int GetZoneColor(float t, float tmin, float tmax, int nlevel);
EXTERNCPP void fill_zonedata(int izone);
EXTERNCPP void UpdateOverwrite(void);
EXTERNCPP void CompressSVZip(void);
EXTERNCPP void DrawBlockages(int mode, int flag);
EXTERNCPP void DrawLabels(void);
EXTERNCPP void UpdateTourList(void);
EXTERNCPP void GetNewPos(float *oldpos, float dx, float dy, float dz, float speed_factor);
EXTERNCPP void FreeSkybox(void);
EXTERNCPP void DrawSkybox(void);
EXTERNCPP void LoadSkyTexture(char *filebase, texturedata *texti);
EXTERNCPP void uncompress_patchdataframe(meshdata *meshi,int frame_index);
EXTERNCPP void getpatchdata_zlib(patchdata *patchi,unsigned char *data,int ndata,
                       float *times, unsigned int *zipoffset, unsigned int *zipsize, int ntimes);
EXTERNCPP void getpatchsizeinfo(patchdata *patchi, int *nframes, int *buffersize);
EXTERNCPP void getpatchheader2(char *file, int *version, int *i1, int *i2, int *j1, int *j2, int *k1, int *k2, int *patchdir);
EXTERNCPP void getpatchheader(char *file,int *npatches,float *valmin, float *valmax);
EXTERNCPP void UpdateCADTextCoords(cadquad *quadi);
EXTERNCPP void UpdateIndexColors(void);
EXTERNCPP void AdjustTourTimes(tourdata *touri);
EXTERNCPP void UpdateTourIndex(void);
EXTERNCPP void SetTour(tourdata *thetour);
EXTERNCPP void UpdatePlot3dDisplay(void);
EXTERNCPP void update_smoke3dflags(void);
EXTERNCPP void MergeSmoke3DColors(smoke3ddata *smoke3dset);
EXTERNCPP void ShowHideSortGeometry(float *mm);
EXTERNCPP void SortTransparentFaces(float *mm);
EXTERNCPP void GetSmokeDir(float *mm);
EXTERNCPP void get_vdevice_vel(float time, vdevicedata *vdevicei, float *vel, float *angle, float *dvel, float *dangle, int *valid_vel);
EXTERNCPP float get_device_val(float time, devicedata *devicei, int *valid);
EXTERNCPP void GetScreenMapping(float *xyz0, float *screen_perm);
EXTERNCPP void GetVolSmokeDir(float *mm);
EXTERNCPP void getzonesmokedir(float *mm);
EXTERNCPP void get_world_eyepos(float *mm, float user_eyepos[3], float scaled_eyepos[3]);
EXTERNCPP culldata *GetFacePort(meshdata *meshi, facedata *facei);
EXTERNCPP void SetCullVis(void);
EXTERNCPP void ExtractFrustum(void);
EXTERNCPP int PointInFrustum( float x, float y, float z);
EXTERNCPP int RectangleInFrustum( float *x11, float *x12, float *x22, float *x21);
EXTERNCPP void UpdateSmoke3D(smoke3ddata *smoke3di);
EXTERNCPP void DrawSmokeFrame(void);
EXTERNCPP void DrawLightDirections(void);
EXTERNCPP void draw_partframe(void);
EXTERNCPP void draw_evacframe(void);
EXTERNCPP void draw_plot3dframe(void);
EXTERNCPP void DrawVSliceFrame(void);
EXTERNCPP void DrawSliceFrame(void);
EXTERNCPP void DrawVGSliceData(vslicedata *vslicei);
EXTERNCPP void DrawGSliceData(slicedata *slicei);
EXTERNCPP void DrawGSliceOutline(void);
EXTERNCPP void draw_patchframe(int flag);
EXTERNCPP void Motion_CB(int var);

EXTERNCPP void DrawSmoke3DVOL(void);
EXTERNCPP void GetDrawingParms(int *drawing_transparent, int *drawing_blockage_transparent, int *drawing_vent_transparent);
EXTERNCPP void UpdateSmoke3DMenuLabels(void);
EXTERNCPP void LabelsCB(int value);
EXTERNCPP void InitSliceData(void);
EXTERNCPP void UpdateCameraLabel(void);
EXTERNCPP void UpdateExtreme(void);
EXTERNCPP void UpdateColorbarType(void);
EXTERNCPP void InitCameraList(void);
EXTERNCPP cameradata *InsertCamera(cameradata *cb,cameradata *source, char *name);
EXTERNCPP void AddDefaultViews(void);
EXTERNCPP void UpdateGluiViewList(void);
EXTERNCPP void ResetGLTime(void);
EXTERNCPP void EnableResetSavedView(void);
EXTERNCPP void ResetGluiView(int ival);
EXTERNCPP void InitCamera(cameradata *camera_data,char *name);
EXTERNCPP void CopyCamera(cameradata *to, cameradata *from);
EXTERNCPP void UpdateCamera(cameradata *ca);
EXTERNCPP void UpdateProjectionType(void);
EXTERNCPP void UpdateCursorCheckbox(void);
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
EXTERNCPP void updateallplotslices(void);
EXTERNCPP int MakeIBlank(void);
EXTERNCPP int MakeIBlankCarve(void);
EXTERNCPP void MakeIBlankSmoke3D(void);
EXTERNCPP void GetUnitInfo(const char *unitlabel, int *unitclass, int *unittype);
EXTERNCPP float GetUnitVal(const char *unitlabel, float oldval);

EXTERNCPP void UpdateUnitDefs(void);

EXTERNCPP void SmoothIsoSurface(isosurface *surfacedata);
EXTERNCPP void UpdateSliceFilenum(void);
EXTERNCPP void drawstaticiso(const isosurface *asurface,int surfacetype,
                             int smoothnorms, int trans_flag, int data_type,
                             float line_width);
EXTERNCPP int getplot3dtime(float *time);
EXTERNCPP void normalize(float *xyz, int n);
#ifndef CPP
EXTERNCPP void getisosizes(const char *isofile, int dataflag, FILE **isostreamptr,
                           int *nvertices, int *ntriangles, float **levels, int *nisolevels,
                           int *niso_times, float *tmin, float *tmax, int endian);
#endif
EXTERNCPP void Array2String(float *array, int narray, char *string);
EXTERNCPP void getisolevels(const char *isofile, int dataflag, float **levelsptr, float ***colorlevelsptr, int *nisolevels);

EXTERNCPP void UpdateVSlices(void);
EXTERNCPP void GetGSliceParams(void);
EXTERNCPP void update_part_menulabels(void);
EXTERNCPP void update_iso_menulabels(void);
EXTERNCPP void update_patch_menulabels(void);
EXTERNCPP void UpdateSliceMenuLabels(void);
EXTERNCPP void UpdateVsliceMenulabels(void);
EXTERNCPP void update_plot3d_menulabels(void);
EXTERNCPP void HandleRotationType(int flag);

EXTERNCPP void InitTextureDir(void);
EXTERNCPP void GetRGB(unsigned int val, unsigned char *rr, unsigned char *gg, unsigned char *bb);
EXTERNCPP unsigned char *ReadPicture(char *filename, int *width, int *height, int printflag);
EXTERNCPP unsigned char *ReadJPEG(const char *filename,int *width, int *height);
EXTERNCPP unsigned char *ReadPNG(const char *filename,int *width, int *height);

EXTERNCPP void Update_Blockvals(int flag);

EXTERNCPP void create_vol_tourlist(void);
EXTERNCPP void delete_vol_tourlist(void);
EXTERNCPP void CreateTourList(void);
EXTERNCPP void DeleteTourList(void);
EXTERNCPP void UpdateViewTour(void);
EXTERNCPP void UpdateTourControls(void);
EXTERNCPP void XYZView2AzElev(keyframe *kf,float *azimuth, float *elevation);
EXTERNCPP void SetupTour(void);
EXTERNCPP void CreateTourPaths(void);
EXTERNCPP void DrawTours(void);
EXTERNCPP void SetGluiTourKeyframe(void);
EXTERNCPP void DrawSelectTours(void);
EXTERNCPP void DrawSelectColorbar(void);
EXTERNCPP void FreeTour(tourdata *touri);
EXTERNCPP void FreeTours(void);
EXTERNCPP void InitTour(tourdata *touri);
EXTERNCPP void UpdateTourMenulabels(void);
EXTERNCPP void DefaultTour(void);
EXTERNCPP void NewSelect(keyframe *newselect);
EXTERNCPP void DeleteTour(int tour_index);
EXTERNCPP tourdata *AddTour(char *label);
EXTERNCPP void InitCircularTour(void);
EXTERNCPP keyframe *DeleteFrame(keyframe *step);
EXTERNCPP void ReallocTourMemory(void);
EXTERNCPP keyframe *AddFrame(keyframe *framei, float time, float *xyz, float key_azimuth, float elevation, float bank,
                    float params[3],int viewtype,float zoom,float view[3]);

EXTERNCPP void GetBlockVals(float *xmin, float *xmax,
                   float *ymin, float *ymax,
                   float *zmin, float *zmax,
                   int *imin, int *jmin, int *kmin);
EXTERNCPP void TransparentOff(void);
EXTERNCPP void TransparentOn(void);
EXTERNCPP void GetObstLabels(const char *filein);
EXTERNCPP void UpdateUseTextures(void);
EXTERNCPP void Antialias(int flag);
EXTERNCPP void SetSliceBounds(int islicetype);
EXTERNCPP void local2globalpatchbounds(const char *key);
EXTERNCPP void global2localpatchbounds(const char *key);
EXTERNCPP void UpdateLoadedLists(void);
EXTERNCPP void UpdateLights(float *pos1, float *pos2);
EXTERNCPP int MergeRenderScreenBuffers(int nscreen_rows, GLubyte **screenbuffers);
EXTERNCPP void SetupScreeninfo(void);
EXTERNCPP int MergeRenderScreenBuffers360(void);
EXTERNCPP GLubyte *GetScreenBuffer(void);
EXTERNCPP void ShowScene(int mode, int view_mode, int quad, GLint s_left, GLint s_down, screendata *screen);
EXTERNCPP int  InBlockage(const meshdata *gb,float x, float y, float z);
EXTERNCPP void UpdateGlui(void);
EXTERNCPP void UpdateSliceList(int index);
EXTERNCPP void drawiso(int tranflag);
EXTERNCPP void drawplot3d(meshdata *gb);
EXTERNCPP void drawplot3d_texture(meshdata *gb);
EXTERNCPP void updateshowstep(int val, int slicedir);
EXTERNCPP void ClearBuffers(int mode);
EXTERNCPP void updateplotslice(int slicedir);
EXTERNCPP void drawpatch(const meshdata *gb);
EXTERNCPP void drawpatch_cellcenter(const meshdata *gb);
EXTERNCPP void drawpatch_texture(const meshdata *gb);
EXTERNCPP void drawpatch_texture_threshold(const meshdata *gb);
EXTERNCPP void drawpatch_threshold_cellcenter(const meshdata *meshi);

EXTERNCPP void Render(int view_mode);
EXTERNCPP void UpdateVSliceTypes(void);
EXTERNCPP int GetSliceType(const slicedata *sd);
EXTERNCPP int GetSliceTypeFromLabel(char *label);
EXTERNCPP void UpdateSliceTypes(void);
EXTERNCPP int GetSliceIndex(const slicedata *sd);
EXTERNCPP void UpdateSliceBoundLabels(void);
EXTERNCPP int getisotype(const isodata *isoi);
EXTERNCPP int getisottype(const isodata *isoi);
EXTERNCPP int getisoindex(const isodata *isoi);
EXTERNCPP void update_isotype(void);
EXTERNCPP void updateisotypes(void);
EXTERNCPP int GetPatchType(const patchdata *patchi);
EXTERNCPP void UpdatePatchType(void);
EXTERNCPP void UpdatePatchTypes(void);

EXTERNCPP void UpdateMeshTerrain(void);
EXTERNCPP void UpdateTerrainOptions(void);
EXTERNCPP void update_plot3dtitle(void);
EXTERNCPP void LoadPlot3dMenu(int value);
EXTERNCPP void init_plot3dtimelist(void);
EXTERNCPP void update_iso_showlevels(void);
EXTERNCPP void UpdateCurrentMesh(meshdata *meshi);
EXTERNCPP void DialogMenu(int value);
EXTERNCPP void ApertureMenu(int value);
EXTERNCPP void ZoomMenu(int value);
EXTERNCPP int NewMultiSlice(slicedata *sdold,slicedata *sd);
EXTERNCPP void DrawTimebar(float xleft, float xright, float ybot, float ytop);
EXTERNCPP void DrawColorbars(void);
EXTERNCPP void DrawColorbarRegLabels(void);
EXTERNCPP void draw_part(const partdata *parti);
EXTERNCPP void draw_evac(const partdata *parti);
EXTERNCPP void drawgrid(const meshdata *gb);
EXTERNCPP void drawroomgeom(void);
EXTERNCPP void drawfiredata(void);
EXTERNCPP void drawroomdata(void);
EXTERNCPP void drawventdata(void);
EXTERNCPP void drawventdataPROFILE(void);
EXTERNCPP void drawventdataSLAB(void);
EXTERNCPP void SetViewPoint(int option);
EXTERNCPP void UpdateTimeLabels(void);
EXTERNCPP void RenderFrame(int view_mode);
EXTERNCPP void UpdateTerrain(int allocate_memory, float vertical_factor);
EXTERNCPP void PartCB_Init(void);
EXTERNCPP void SliceCB(int var);
EXTERNCPP void RenderMenu(int value);
EXTERNCPP void LoadSmoke3DMenu(int value);
EXTERNCPP void DisplayVersionInfo(char *progname);
EXTERNCPP void DrawDemo(int nlat, int nlong);
EXTERNCPP void DrawDemo2(int option);
EXTERNCPP void InitDemo(float rad, int nlat, int nlong);
EXTERNCPP void DrawOutlines(void);
EXTERNCPP void DrawCBox(float x, float y, float z, float size);
EXTERNCPP void handleiso(void);
EXTERNCPP void updatesurface(void);
EXTERNCPP void WindowStatus(int state);
EXTERNCPP void SetVentDirs(void);
EXTERNCPP void SetCVentDirs(void);
EXTERNCPP void ObstOrVent2Faces(const meshdata *gb,blockagedata *bc, ventdata *vi, facedata *faceptr,int facetype);
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
EXTERNCPP float *GetColorPtr(const float *color);
EXTERNCPP void ConvertColor(int flag);
EXTERNCPP void InitCadColors(void);
EXTERNCPP void UpdateRGBColors(int colorindex);
EXTERNCPP void InitRGB(void);
EXTERNCPP void UpdateChopColors(void);
EXTERNCPP int ReadINI(char *inifile);
EXTERNCPP void WriteINI(int flag,char *file);
EXTERNCPP void DrawFirePlume(float radius, float height, float maxheight);
EXTERNCPP void AdjustDataBounds(const float *pdata, int skip, int ndata, int setpmin, float *pmin, int setpmax, float *pmax);
EXTERNCPP void AdjustPartBounds(const float *pdata, int particle_type, int droplet_type, const unsigned char *isprink,
                      int skip, int ndata, int setpmin, float *pmin, int setpmax, float *pmax);
EXTERNCPP void AdjustPart5Chops(partdata *parti);
EXTERNCPP void AdjustPart5Bounds(partdata *parti);
EXTERNCPP void AdjustPlot3DBounds(int iplot3d, int setpmin, float *pmin, int setpmax, float *pmax);
EXTERNCPP void ScaleFloat2String(float floatfrom, char *stringto, const float *scale);
EXTERNCPP void ScaleString(const char *stringfrom, char *stringto, const float *scale);
EXTERNCPP void Num2String(char *string, float tval);
EXTERNCPP int SetupCase(int argc, char **argv);
EXTERNCPP int GetMinPartFrames(int flag);
EXTERNCPP int Update_Bounds(void);

EXTERNCPP void FreeCADInfo(void);

EXTERNCPP void InitUnitDefs(void);
EXTERNCPP void InitUnits(void);
EXTERNCPP f_units *GetUnitClass(char *unit);

EXTERNCPP void ReadCAD2Geom(cadgeomdata *cd);
EXTERNCPP void ReadCADGeom(cadgeomdata *cd);
EXTERNCPP void DrawCADGeom(const cadgeomdata *cd);

EXTERNCPP void readplot3d(char *file, int ifile, int flag,int *errorcode);
EXTERNCPP void read_geom_header(geomdata *geomi, int *geom_frame_index, int *ntimes_local);
EXTERNCPP void read_all_geom(void);
EXTERNCPP void read_geom(geomdata *geomi, int load_flag, int type, int *geom_frame_index, int *errorcode);
EXTERNCPP void InitGeom(geomdata *geomi, int hasdata, int fdsblock);
EXTERNCPP void read_geomdata(int ifile, int load_flag, int *errorcode);
EXTERNCPP void readpatch(int ifile, int flag, int *errorcode);
EXTERNCPP void readpart(char *file, int ifile, int loadflag, int set_colorbound, int *errorcode);
EXTERNCPP void readzone(int ifile, int flag, int *errorcode);
EXTERNCPP void ReadVSlice(int ivslice, int flag, int *errorcode);

EXTERNCPP void FreeSmoke3D(smoke3ddata *smoke3di);
EXTERNCPP void ReadSmoke3D(int ifile,int flag, int *errorcode);
EXTERNCPP void ReadFed(int ifile, int flag, int file_type, int *errorcode);
EXTERNCPP void ReadSlice(char *file, int ifile, int flag, int set_slicecolor, int *errorcode);
EXTERNCPP void readiso(const char *file, int ifile, int flag, int *geom_frame_index, int *errorcode);

EXTERNCPP void InitMenus(int unload);
EXTERNCPP void SmoothLabel(float *min, float *max, int n);
EXTERNCPP int ReadSMV(char *file, char *file2);
EXTERNCPP void ReadSMVDynamic(char *file);
EXTERNCPP int STRCMP(const char *s1, const char *s2);
EXTERNCPP void OutputAxisLabels(void);
EXTERNCPP void OutputLargeText(float x, float y, char *string);
EXTERNCPP void OutputText(float x, float y, char *string);
EXTERNCPP void Output3Text(float *color, float x, float y, float z, char *string);
EXTERNCPP void Output3Val(float x, float y, float z, float val);
EXTERNCPP void OutputBarText(float x, float y, const GLfloat *color, char *string);
EXTERNCPP void GetZoneGlobalBounds(const float *pdata, int ndata, float *pglobalmin, float *pglobalmax);
EXTERNCPP void UpdateChar(void);
EXTERNCPP void UpdateTracers(void);
void UpdateGslicePlanes(void);

EXTERNCPP void GetPart5Colors(partdata *parti, int nlevels, int convert_flag);
EXTERNCPP void GetBoundaryColors(float *t, int nt, unsigned char *it,
              int settmin, float *tmin, int settmax, float *tmax,
              float *tmin_global, float *tmax_global,
              int ndatalevel, int nlevel,
              char **labels, char *scale, float *tvals256,
              int *extreme_min, int *extreme_max);
EXTERNCPP void GetBoundaryColors2(float *t, int nt, unsigned char *it,
              int settmin, float *ttmin, int settmax, float *ttmax,
              float *tmin_global, float *tmax_global,
              int ndatalevel,
              int *extreme_min, int *extreme_max
              );
EXTERNCPP void GetBoundaryColors3(patchdata *patchi, float *t, int nt, unsigned char *it,
              int settmin, float *tmin, int settmax, float *tmax,
              float *tmin_global, float *tmax_global,
              int nlevel,
              char **labels, char *scale, float *tvals256,
              int *extreme_min, int *extreme_max);
EXTERNCPP void GetBoundaryLabels(
              float tmin, float tmax,
              char **labels, char *scale, float *tvals256, int nlevel);
EXTERNCPP void GetZoneColors(const float *t, int nt, unsigned char *it,
               float tmin, float tmax, int nlevel, int nlevel_full,
               char **labels, char *scale, float *tvals256
               );

EXTERNCPP void GetPlot3DColors(int iplot, int settmin, float *ttmin, int settmax, float *ttmax,
              int ndatalevel, int nlevel,
              char **labels,char **labelsiso, char **scale, float *fscale, float *tlevels, float *tlevels256,
              int *extreme_min, int *extreme_max
              );
EXTERNCPP void GetSliceLabels(float tmin, float tmax, int nlevel,
              char labels[12][11],char **scale, float *fscale, float *tlevels256);
EXTERNCPP void UpdatePart5Extremes(void);
EXTERNCPP void GetSliceColors(const float *t, int nt, unsigned char *it,
              float tmin, float tmax,
              int ndatalevel, int nlevel,
              char labels[12][11],char **scale, float *fscale, float *tlevels2,
              int *extreme_min, int *extreme_max
              );
EXTERNCPP meshdata *get_loaded_isomesh(void);
EXTERNCPP void unload_iso_trans(void);
EXTERNCPP void setisolabels(float smin, float smax,
                    isodata *sd, int *errorcode);
EXTERNCPP void GetIsoLabels(float tmin, float tmax, int nlevel,
              char labels[12][11],char **scale, float *tlevels256);
EXTERNCPP int SmokeviewImage2File(char *directory, char *GIFfilename, int rendertype, int woffset, int width, int hoffset, int height);
#ifdef pp_LUA
EXTERNCPP int SVimage2var(int rendertype, int woffset, int width, int hoffset, int height, gdImagePtr *RENDERimage);
#endif

EXTERNCPP void UpdateShowHideButtons(void);
EXTERNCPP void UpdateFileLoad(void);
EXTERNCPP void CalcTriNormal(float *v1, float *v2, float *v3, float *norm);
EXTERNCPP void update_triangles(int time_flag, int update);

#ifndef CPP
#include "smokefortheaders.h"
#endif

#endif
