#ifndef C_API_H_DEFINED
#define C_API_H_DEFINED
#include "options_common.h"

#include "gd.h"

// Verified, the declarations below are part of the verified and test API.
#define ERROR_CODE int
#define ERR_OK 0
#define ERR_NOK 1

typedef struct _simple_bounds {
  float min, max;
} simple_bounds;

void SetColorbar(size_t value);
ERROR_CODE SetNamedColorbar(const char *name);
ERROR_CODE GetNamedColorbar(const char *name, size_t *index);
ERROR_CODE CApiSetSliceBounds(const char *slice_type, int set_min,
                              float value_min, int set_max, float value_max);
ERROR_CODE SetSliceBoundMin(const char *slice_type, int set, float value);
ERROR_CODE SetSliceBoundMax(const char *slice_type, int set, float value);
ERROR_CODE GetSliceBounds(const char *slice_type, simple_bounds *bounds);
ERROR_CODE CApiRender(const char *filename);
int Getframe();
void Setframe(int framenumber);
float Gettime();
int Settime(float timeval);

// Non-Verified, the declarations below are of variable quality.
int Loadsmvall(const char *input_filepath);
int Loadsmv(char *input_filename, char *input_filename_ext);
void Renderclip(int flag, int left, int right, int bottom, int top);
void Gsliceview(int data, int show_triangles, int show_triangulation,
                int show_normal);
void ShowPlot3dData(int meshnumber, int plane_orientation, int display,
                    int showhide, float position, int isolevel);
void Gslicepos(float x, float y, float z);
void Gsliceorien(float az, float elev);
void Settourkeyframe(float keyframe_time);
void Settourview(int edittourArg, int mode, int show_tourlocusArg,
                 float tour_global_tensionArg);
int Loadfile(const char *filename);
void Loadinifile(const char *filepath);
int Loadvfile(const char *filepath);
void Loadboundaryfile(const char *filepath);
void Loadboundary(const char *filepath);
void Label(const char *label);
void Load3dsmoke(const char *smoke_type);
void SetSliceInObst(int setting);
int GetSliceInObst();
void Loadvolsmoke(int meshnumber);
void Loadvolsmokeframe(int meshnumber, int framenumber, int flag);
int SetRendertype(const char *type);
int GetRendertype(void);
void SetMovietype(const char *type);
int GetMovietype(void);
void Makemovie(const char *name, const char *base, float framerate);
int Loadtour(const char *tourname);
void Loadparticles(const char *name);
void Partclasscolor(const char *color);
void Partclasstype(const char *part_type);
void Plot3dprops(int variable_index, int showvector, int vector_length_index,
                 int display_type, float vector_length);
void Loadplot3d(int meshnumber, float time_local);
void Loadiso(const char *type);
void Loadslice(const char *type, int axis, float distance);
FILE_SIZE Loadsliceindex(size_t index, int *errorcode);
void Loadvslice(const char *type, int axis, float distance);
int Unloadall();
void Unloadtour();
void ExitSmokeview();
void Setcolorbarflip(int flip);
int Getcolorbarflip();
int Setviewpoint(const char *viewpoint);
int SetOrthoPreset(const char *viewpoint);
int Setrenderdir(const char *dir);
void Setwindowsize(int width, int height);
void Setgridvisibility(int selection);
void Setgridparms(int x_vis, int y_vis, int z_vis, int x_plot, int y_plot,
                  int z_plot);
void Setcolorbarindex(int chosen_index);
int Getcolorbarindex();
void CameraSetEyeview(int eyeview);
void CameraSetRotationIndex(int rotation_index);
int CameraGetRotationIndex();

void CameraSetRotationType(int rotation_type);
int CameraGetRotationType();

void CameraSetViewId(int view_id);
// void viewpoint_set_viewdir(float xcen, float ycen, float zcen);

void CameraModEyex(float delta);
void CameraModEyey(float delta);
void CameraModEyez(float delta);

void CameraSetEyex(float eyex);
void CameraSetEyey(float eyey);
void CameraSetEyez(float eyez);

float CameraGetEyex();
float CameraGetEyey();
float CameraGetEyez();

void CameraModAz(float delta);
void CameraSetAz(float az);
float CameraGetAz();
int CameraZoomToFit();
void CameraModElev(float delta);
void CameraSetElev(float elev);
float CameraGetElev();

void CameraSetViewdir(float xcen, float ycen, float zcen);
float CameraGetXcen();
float CameraGetYcen();
float CameraGetZcen();
void CameraSetXcen(float xcen);
void CameraSetYcen(float ycen);
void CameraSetZcen(float zcen);

void CameraToggleProjectionType();
int CameraSetProjectionType(int projection_type);
int CameraGetProjectionType();

int GetClippingMode();
void SetClippingMode(int mode);
void SetSceneclipX(int clipMin, float min, int clipMax, float max);
void SetSceneclipXMin(int flag, float value);
void SetSceneclipXMax(int flag, float value);
void SetSceneclipY(int clipMin, float min, int clipMax, float max);
void SetSceneclipYMin(int flag, float value);
void SetSceneclipYMax(int flag, float value);
void SetSceneclipZ(int clipMin, float min, int clipMax, float max);
void SetSceneclipZMin(int flag, float value);
void SetSceneclipZMax(int flag, float value);

int RenderFrameLua(int view_mode, const char *basename);
char *FormFilename(int view_mode, char *renderfile_name, char *renderfile_dir,
                   char *renderfile_path, int woffset, int hoffset, int screenH,
                   const char *basename);

int ParseSmvFilepath(const char *smv_filepath, char *fdsprefix,
                     char *input_filename_ext);

// --------- show/hide label options--------

// colorbar
void SetColorbarVisibilityVertical(int setting);
int GetColorbarVisibilityVertical();
void ToggleColorbarVisibilityVertical();

void SetColorbarVisibilityHorizontal(int setting);
int GetColorbarVisibilityHorizontal();
void ToggleColorbarVisibilityHorizontal();

void SetColorbarVisibility(int setting);
int GetColorbarVisibility();
void ToggleColorbarVisibility();

// timebar
void SetTimebarVisibility(int setting);
int GetTimebarVisibility();
void ToggleTimebarVisibility();

// title
void SetTitleVisibility(int setting);
int GetTitleVisibility();
void ToggleTitleVisibility();

// smv_version
void SetSmvVersionVisibility(int setting);
int GetSmvVersionVisibility();
void ToggleSmvVersionVisibility();

// chid
void SetChidVisibility(int setting);
int GetChidVisibility();
void ToggleChidVisibility();

// blockages
void BlockagesHideAll();

// outlines
void OutlinesHide();
void OutlinesShow();

// surfaces
void SurfacesHideAll();

// devices
void DevicesHideAll();

// axis
void SetAxisVisibility(int setting);
int GetAxisVisibility();
void ToggleAxisVisibility();

// frame
void SetFramelabelVisibility(int setting);
int GetFramelabelVisibility();
void ToggleFramelabelVisibility();

// framerate
void SetFramerateVisibility(int setting);
int GetFramerateVisibility();
void ToggleFramerateVisibility();

// grid locations
void SetGridlocVisibility(int setting);
int GetGridlocVisibility();
void ToggleGridlocVisibility();

// hrrpuv cutoff
void SetHrrcutoffVisibility(int setting);
int GetHrrcutoffVisibility();
void ToggleHrrcutoffVisibility();

// hrr label
void SetHrrlabelVisibility(int setting);
int GetHrrlabelVisibility();
void ToggleHrrlabelVisibility();

// memory load
#ifdef pp_memstatus
void set_memload_visibility(int setting);
int get_memload_visibility();
void toggle_memload_visibility();
#endif

// mesh
void SetMeshlabelVisibility(int setting);
int GetMeshlabelVisibility();
void ToggleMeshlabelVisibility();

// slice average
void SetSliceAverageVisibility(int setting);
int GetSliceAverageVisibility();
void ToggleSliceAverageVisibility();

// time
void SetTimeVisibility(int setting);
int GetTimeVisibility();
void ToggleTimeVisibility();

// user settable ticks
void SetUserTicksVisibility(int setting);
int GetUserTicksVisibility();
void ToggleUserTicksVisibility();

// version info
void SetVersionInfoVisibility(int setting);
int GetVersionInfoVisibility();
void ToggleVersionInfoVisibility();

// set all
void SetAllLabelVisibility(int setting);

// --------- options--------

// Display Units
// time
void SetTimehms(int setting);
int GetTimehms();
void ToggleTimehms();

// arbitrary
void SetUnits(int unitclass, int unit_index);
void SetUnitsDefault();
void SetUnitclassDefault(int unitclass);

int BlockageViewMethod(int setting);
int BlockageOutlineColor(int setting);
int BlockageLocations(int setting);

// .ini config options
int SetAmbientlight(float r, float g, float b);    // AMBIENTLIGHT
int SetBackgroundcolor(float r, float g, float b); // BACKGROUNDCOLOR
int SetBlockcolor(float r, float g, float b);      // BLOCKCOLOR
int SetBlockshininess(float v);                    // BLOCKSHININESS
int SetBlockspecular(float r, float g, float b);   // BLOCKSPECULAR
int SetBoundcolor(float r, float g, float b);      // BOUNDCOLOR
int SetColorbarTextureflag(int v);
int GetColorbarTextureflag();
int SetColorbarContourvalue(int v);
int SetColorbarColors(int ncolors, float *colors);
int SetColor2barColors(int ncolors, float *colors);
int SetDiffuselight(float r, float g, float b);   // DIFFUSELIGHT
int SetDirectioncolor(float r, float g, float b); // DIRECTIONCOLOR
int SetFlip(int setting);                         // FLIP
int GetFlip();
int SetForegroundcolor(float r, float g, float b); // FOREGROUNDCOLOR
int SetHeatoffcolor(float r, float g, float b);    // HEATOFFCOLOR
int SetHeatoncolor(float r, float g, float b);     // HEATONCOLOR
int SetIsocolors(float shininess, float transparency, int transparency_option,
                 int opacity_change, float specular[3], int n_colors,
                 float colors[][4]);
int SetColortable(int ncolors, int colors[][4], char **names);
int SetLightpos0(float x, float y, float z, float w); // LIGHTPOS0
int SetLightpos1(float x, float y, float z, float w); // LIGHTPOS1
int SetSensorcolor(float r, float g, float b);        // SENSORCOLOR
int SetSensornormcolor(float r, float g, float b);    // SENSORNORMCOLOR
int SetBw(int geo_setting, int data_setting);         // SETBW
int SetSprinkleroffcolor(float r, float g, float b);  // SPRINKOFFCOLOR
int SetSprinkleroncolor(float r, float g, float b);   // SPRINKONCOLOR
int SetStaticpartcolor(float r, float g, float b);    // STATICPARTCOLOR
int SetTimebarcolor(float r, float g, float b);       // TIMEBARCOLOR
int SetVentcolor(float r, float g, float b);          // VENTCOLOR

// --    *** SIZES/OFFSETS ***

int SetGridlinewidth(float v);    // GRIDLINEWIDTH
int SetIsolinewidth(float v);     // ISOLINEWIDTH
int SetIsopointsize(float v);     // ISOPOINTSIZE
int SetLinewidth(float v);        // LINEWIDTH
int SetPartpointsize(float v);    // PARTPOINTSIZE
int SetPlot3dlinewidth(float v);  // PLOT3DLINEWIDTH
int SetPlot3dpointsize(float v);  // PLOT3DPOINTSIZE
int SetSensorabssize(float v);    // SENSORABSSIZE
int SetSensorrelsize(float v);    // SENSORRELSIZE
int SetSliceoffset(float v);      // SLICEOFFSET
int SetSmoothlines(int v);        // SMOOTHLINES
int SetSpheresegs(int v);         // SPHERESEGS
int SetSprinklerabssize(float v); // SPRINKLERABSSIZE
int SetStreaklinewidth(float v);  // STREAKLINEWIDTH
int SetTicklinewidth(float v);    // TICKLINEWIDTH
int SetUsenewdrawface(int v);     // USENEWDRAWFACE
int SetVeclength(float vf, int vec_uniform_length_in,
                 int vec_uniform_spacing_in); // VECLENGTH
int SetVectorlinewidth(float a, float b);     // VECTORLINEWIDTH
int SetVectorpointsize(float v);              // VECTORPOINTSIZE
int SetVentlinewidth(float v);                // VENTLINEWIDTH
int SetVentoffset(float v);                   // VENTOFFSET
int SetWindowoffset(int v);                   // WINDOWOFFSET
int SetWindowwidth(int v);                    // WINDOWWIDTH
int SetWindowheight(int v);                   // WINDOWHEIGHT

// --  *** DATA LOADING ***

int SetBoundzipstep(int v); // BOUNDZIPSTEP
int SetIsozipstep(int v); // ISOZIPSTEP
int SetNopart(int v);     // NOPART
// int set_partpointstep(int v); // PARTPOINTSTEP
int SetSliceaverage(int flag, float interval, int vis); // SLICEAVERAGE
int SetSlicedataout(int v);                             // SLICEDATAOUT
int SetSlicezipstep(int v);                             // SLICEZIPSTEP
int SetSmoke3dzipstep(int v);                           // SMOKE3DZIPSTEP
int SetUserrotate(int index, int show_center, float x, float y,
                  float z); // USER_ROTATE

// --  *** VIEW PARAMETERS ***

int SetAperture(int v); // APERTURE
// int set_axissmooth(int v); // AXISSMOOTH
int SetBlocklocation(int v);        // BLOCKLOCATION
int SetBoundarytwoside(int v);      // BOUNDARYTWOSIDE
int SetClip(float near, float far); // CLIP
int SetContourtype(int v);          // CONTOURTYPE
int SetCullfaces(int v);            // CULLFACES
int SetTexturelighting(int v);      // ENABLETEXTURELIGHTING
int SetEyeview(int v);              // EYEVIEW
int SetEyex(float v);               // EYEX
int SetEyey(float v);               // EYEY
int SetEyez(float v);               // EYEZ
int SetFontsize(int v);             // FONTSIZE
int SetFrameratevalue(int v);       // FRAMERATEVALUE
// GEOMSHOW
int SetShowfacesInterior(int v);
int SetShowfacesExterior(int v);
int SetShowfacesSolid(int v);
int SetShowfacesOutline(int v);
int SetSmoothgeomnormal(int v);
int SetGeomvertexag(int v);
int SetGeommaxangle(int v);
int SetGversion(int v);                                // GVERSION
int SetIsotran2(int v);                                // ISOTRAN2
int SetMeshvis(int n, int vals[]);                     // MESHVIS
int SetMeshoffset(int meshnum, int value);             // MESHOFFSET
int SetNorthangle(int vis, float x, float y, float z); // NORTHANGLE
int SetOffsetslice(int v);                             // OFFSETSLICE
int SetOutlinemode(int a, int b);                      // OUTLINEMODE
int SetP3dsurfacetype(int v);                          // P3DSURFACETYPE
int SetP3dsurfacesmooth(int v);                        // P3DSURFACESMOOTH
int SetSbatstart(int v);                               // SBATSTART
int SetScaledfont(int height2d, float height2dwidth, int thickness2d,
                  int height3d, float height3dwidth,
                  int thickness3d); // SCALEDFONT
int GetScaledfontHeight2d();
int SetScaledfontHeight2d(int height2d);
int SetShowalltextures(int v);      // SHOWALLTEXTURES
int SetShowaxislabels(int v);       // SHOWAXISLABELS
int SetShowblocklabel(int v);       // SHOWBLOCKLABEL
int SetShowblocks(int v);           // SHOWBLOCKS
int SetShowcadandgrid(int v);       // SHOWCADANDGRID
int SetShowcadopaque(int v);        // SHOWCADOPAQUE
int SetShowceiling(int v);          // SHOWCEILING
int SetShowcolorbars(int v);        // SHOWCOLORBARS
int SetShowcvents(int a, int b);    // SHOWCVENTS
int SetShowdummyvents(int v);       // SHOWDUMMYVENTS
int SetShowfloor(int v);            // SHOWFLOOR
int SetShowframe(int v);            // SHOWFRAME
int SetShowframelabel(int v);       // SHOWFRAMELABEL
int SetShowframerate(int v);        // SHOWFRAMERATE
int SetShowgrid(int v);             // SHOWGRID
int SetShowgridloc(int v);          // SHOWGRIDLOC
int SetShowhmstimelabel(int v);     // SHOWHMSTIMELABEL
int SetShowhrrcutoff(int v);        // SHOWHRRCUTOFF
int SetShowiso(int v);              // SHOWISO
int SetShowisonormals(int v);       // SHOWISONORMALS
int SetShowlabels(int v);           // SHOWLABELS
int SetShowmemload(int v);          // SHOWMEMLOAD
int SetShownormalwhensmooth(int v); // SHOWNORMALWHENSMOOTH
int SetShowopenvents(int a, int b); // SHOWOPENVENTS
int SetShowothervents(int v);       // SHOWOTHERVENTS
int SetShowsensors(int a, int b);   // SHOWSENSORS
int SetShowsliceinobst(int v);      // SHOWSLICEINOBST
int SetShowsmokepart(int v);        // SHOWSMOKEPART
int SetShowsprinkpart(int v);       // SHOWSPRINKPART
int SetShowstreak(int show, int step, int showhead, int index); // SHOWSTREAK
int SetShowterrain(int v);                                      // SHOWTERRAIN
int SetShowthreshold(int a, int b, float c);                    // SHOWTHRESHOLD
int SetShowticks(int v);                                        // SHOWTICKS
int SetShowtimebar(int v);                                      // SHOWTIMEBAR
int SetShowtimelabel(int v);                                    // SHOWTIMELABEL
int SetShowtitle(int v);                                        // SHOWTITLE
int SetShowtracersalways(int v); // SHOWTRACERSALWAYS
int SetShowtriangles(int a, int b, int c, int d, int e,
                     int f);                            // SHOWTRIANGLES
int SetShowtransparent(int v);                          // SHOWTRANSPARENT
int SetShowtransparentvents(int v);                     // SHOWTRANSPARENTVENTS
int SetShowtrianglecount(int v);                        // SHOWTRIANGLECOUNT
int SetShowventflow(int a, int b, int c, int d, int e); // SHOWVENTFLOW
int SetShowvents(int v);                                // SHOWVENTS
int SetShowwalls(int v);                                // SHOWWALLS
int SetSkipembedslice(int v);                           // SKIPEMBEDSLICE
int SetSmokesensors(int a, int b);                      // SMOKESENSORS
int SetSmoothblocksolid(int v);                         // SMOOTHBLOCKSOLID
int SetStartuplang(const char *lang);                   // STARTUPLANG
int SetStereo(int v);                                   // STEREO
int SetSurfinc(int v);                                  // SURFINC
int SetTerrainparams(int r_min, int g_min, int b_min, int r_max, int g_max,
                     int b_max, int v);                      // TERRAINPARMS
int SetTitlesafe(int v);                                     // TITLESAFE
int SetTrainermode(int v);                                   // TRAINERMODE
int SetTrainerview(int v);                                   // TRAINERVIEW
int SetTransparent(int a, float b);                          // TRANSPARENT
int SetTreeparms(int minsize, int visx, int visy, int visz); // TREEPARMS
int SetTwosidedvents(int internal, int external);            // TWOSIDEDVENTS
int SetVectorskip(int v);                                    // VECTORSKIP
int SetVolsmoke(int a, int b, int c, int d, int e, float f, float g, float h,
                float i, float j, float k, float l); // VOLSMOKE
int SetZoom(int a, float b);                         // ZOOM

// --  *** MISC ***

int SetCellcentertext(int v);                     // CELLCENTERTEXT
int SetInputfile(const char *filename);           // INPUT_FILE
int SetLabelstartupview(const char *startupview); // LABELSTARTUPVIEW
// int set_pixelskip(int v); // PIXELSKIP
int SetRenderclip(int a, int b, int c, int d, int e); // RENDERCLIP
// int set_renderfilelabel(int v); // RENDERFILELABEL
int SetRenderfiletype(int a, int b); // RENDERFILETYPE
// int set_renderoption(int a, int b); // RENDEROPTION
int SetUnitclasses(int n, int values[]); // UNITCLASSES
int SetZaxisangles(float a, float b, float c);

// --  *** 3D SMOKE INFO ***

int SetColorbartype(int v, const char *label); // COLORBARTYPE
int SetExtremecolors(int a, int b, int c, int d, int e,
                     int f);                 // EXTREMECOLORS
int SetFirecolor(int r, int g, int b);       // FIRECOLOR
int SetFirecolormap(int a, int b);           // FIRECOLORMAP
int SetFiredepth(float v);                   // FIREDEPTH
int SetShowextremedata(int a, int b, int c); // SHOWEXTREMEDATA
int SetSmokecolor(int r, int g, int b);      // SMOKECOLOR
int SetSmokecull(int v);                     // SMOKECULL
int SetSmokeskip(int v);                     // SMOKESKIP
int SetSmokealbedo(float v);                 // SMOKEALBEDO
int SetSmokerthick(float v);                 // SMOKERTHICK
// int set_smokethick(float v); // SMOKETHICK
int SetUsegpu(int v); // USEGPU

// --  *** ZONE FIRE PARAMETRES ***

int SetShowhazardcolors(int v); // SHOWHAZARDCOLORS
int SetShowhzone(int v);        // SHOWHZONE
int SetShowszone(int v);        // SHOWSZONE
int SetShowvzone(int v);        // SHOWVZONE
int SetShowzonefire(int v);     // SHOWZONEFIRE

// --  *** TOUR INFO ***

int SetShowpathnodes(int v); // SHOWPATHNODES
int SetShowtourroute(int v); // SHOWTOURROUTE

// TOURCOLORS
int SetTourcolorsSelectedpathline(float r, float g, float b);
int SetTourcolorsSelectedpathlineknots(float r, float g, float b);
int SetTourcolorsSelectedknot(float r, float g, float b);
int SetTourcolorsPathline(float r, float g, float b);
int SetTourcolorsPathknots(float r, float g, float b);
int SetTourcolorsText(float r, float g, float b);
int SetTourcolorsAvatar(float r, float g, float b);

int SetViewalltours(int v);                // VIEWALLTOURS
int SetViewtimes(float a, float b, int c); // VIEWTIMES
int SetViewtourfrompath(int v);            // VIEWTOURFROMPATH

// --  ------------ local ini settings ------------

int SetAvatarevac(int v); // AVATAREVAC
int SetDevicevectordimensions(float baselength, float basediameter,
                              float headlength,
                              float headdiameter); // DEVICEVECTORDIMENSIONS
int SetDevicebounds(float a, float b);             // DEVICEBOUNDS
int SetDeviceorientation(int a, float b);          // DEVICEORIENTATION
int SetGridparms(int vx, int vy, int vz, int px, int py, int pz); // GRIDPARMS
int SetGsliceparms(int vis_data, int vis_triangles, int vis_triangulation,
                   int vis_normal, float xyz[], float azelev[]); // GSLICEPARMS
int SetLoadfilesatstartup(int v);         // LOADFILESATSTARTUP
int SetMscale(float a, float b, float c); // MSCALE
int SetSliceauto(int n, int vals[]);      // SLICEAUTO
int SetMsliceauto(int n, int vals[]);     // MSLICEAUTO
int SetCompressauto(int v);               // COMPRESSAUTO
int SetPart5propdisp(int vals[]);         // PART5PROPDISP
int SetPart5color(int v);                 // PART5COLOR
int SetPropindex(int nvals, int *vals);   // PROPINDEX
int SetShooter(float xyz[], float dxyz[], float uvw[], float velmag,
               float veldir, float pointsize, int fps, int vel_type, int nparts,
               int vis, int cont_update, float duration,
               float v_inf);                         // SHOOTER
int SetShowdevices(int n, const char *const *names); // SHOWDEVICES
int SetShowdevicevals(int showdeviceval, int showvdeviceval,
                      int devicetypes_index, int colordeviceval, int vectortype,
                      int vispilot, int showdevicetype,
                      int showdeviceunit); // SHOWDEVICEVALS
int SetShowmissingobjects(int v);          // SHOWMISSINGOBJECTS
int SetTourindex(int v);                   // TOURINDEX
int SetUserticks(int vis, int auto_place, int sub, float origin[], float min[],
                 float max[], float step[], int show_x, int show_y,
                 int show_z); // USERTICKS
int SetCParticles(int minFlag, float minValue, int maxFlag, float maxValue,
                  const char *label); // C_PARTICLES
int SetCSlice(int minFlag, float minValue, int maxFlag, float maxValue,
              const char *label);      // C_SLICE
int SetCacheBoundarydata(int setting); // CACHE_BOUNDARYDATA
int SetCacheQdata(int setting);        // CACHE_QDATA
int SetPercentilelevel(float percentile_level_min,
                       float p_level_max); // PERCENTILELEVEL
int SetTimeoffset(int setting);            // TIMEOFFSET
int SetPatchdataout(int outputFlag, float tmin, float tmax, float xmin,
                    float xmax, float ymin, float ymax, float zmin,
                    float zmax); // PATCHDATAOUT
int SetCPlot3d(int n3d, int minFlags[], int minVals[], int maxFlags[],
               int maxVals[]); // C_PLOT3D
int SetVPlot3d(int n3d, int minFlags[], int minVals[], int maxFlags[],
               int maxVals[]); // V_PLOT3D
int SetPl3dBoundMin(int pl3dValueIndex, int set, float value);
int SetPl3dBoundMax(int pl3dValueIndex, int set, float value);
int SetTload(int beginFlag, float beginVal, int endFlag, int endVal,
             int skipFlag, int skipVal); // TLOAD
int SetV5Particles(int minFlag, float minValue, int maxFlag, float maxValue,
                   const char *label); // V5_PARTICLES
int SetVParticles(int minFlag, float minValue, int maxFlag, float maxValue);
// V_PARTICLES
int SetVTarget(int minFlag, float minValue, int maxFlag, float maxValue);
// V_TARGET
int SetVSlice(int minFlag, float minValue, int maxFlag, float maxValue,
              const char *label, float lineMin, float lineMax,
              int lineNum); // V_SLICE
// -- VIEWPOINT5
// --  0 10 2
// --  0.490669 -2.257067 0.018868 1.000000 -2
// --  0.000000 0.000000 0.000000 1
// --  0.500000 0.240566 0.018868
// --  0.000000 90.000000
// --  1.000000 0.000000 0.000000 0.000000
// --  0.000000 1.000000 0.000000 0.000000
// --  0.000000 0.000000 1.000000 0.000000
// --  0.000000 0.000000 0.000000 1.000000
// --  2 0 0 0 0 0 1
// --  -0.424000 -1.204000 -0.016000 424.424011 203.203995 5.193398
// --  topDown

int ShowSmoke3dShowall();
int ShowSmoke3dHideall();
int ShowSlicesShowall();
int ShowSlicesHideall();
int RenderFrameLuaVar(int view_mode, gdImagePtr *RENDERimage);

#define PROPINDEX_STRIDE 2
#endif
