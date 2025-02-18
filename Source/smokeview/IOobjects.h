#ifndef IOOBJECT_H_DEFINED
#define IOOBJECT_H_DEFINED


void DrawBox2(float *origin, float *dxyz, float *color, int draw_outline);
void DrawCone(float d1, float height, unsigned char *rgbcolor);
void DrawTruncCone(float d1, float d2, float height, unsigned char *rgbcolor);
void DrawSphere(float diameter, unsigned char *rgbcolor);
void DrawDisk(float diameter, float height, unsigned char *rgbcolor);
void DrawWheel(float diameter, float thickness, unsigned char *rgbcolor);
void DrawSmvObject(sv_object *object, int frame_index_local, propdata *prop, int recurse_level, float *valrgb, int vis_override);

//*** IOobjects.c headers

EXTERNCPP void DeviceData2WindRose(int nr, int ntheta);
EXTERNCPP void DrawDevices(int mode);
EXTERNCPP void DrawDevicesVal(void);
EXTERNCPP void DrawCircle(float diameter, unsigned char *rgbcolor, circdata *circinfo);
EXTERNCPP void DrawFilledCircle(float diameter, unsigned char *rgbcolor, circdata *circinfo);
EXTERNCPP void DrawFilledRectangle(float width, float height, unsigned char *rgbcolor);
EXTERNCPP void DrawRectangle(float width, float height, unsigned char *rgbcolor);
EXTERNCPP void DrawWindRosesDevices(void);
EXTERNCPP devicedata *GetCSVDeviceFromLabel(char *label, int index);
EXTERNCPP devicedata *GetDeviceFromLabel(char *label, int index);
EXTERNCPP int GetDeviceIndexFromLabel(char *label);
EXTERNCPP char *GetDeviceLabel(char *buffer);
EXTERNCPP float GetDeviceVal(float time_local, devicedata *devicei, int *valid);
EXTERNCPP void GetGlobalDeviceBounds(int type);
EXTERNCPP int GetNDevices(char *file);
EXTERNCPP void GetSmokeSensors(void);
EXTERNCPP int HaveSmokeSensor(void);
EXTERNCPP void InitDevicePlane(devicedata *devicei);
EXTERNCPP void InitializeDeviceCsvData(int flag);
EXTERNCPP void InitSphere(int nlat, int nlong);
EXTERNCPP FILE_SIZE ReadDeviceData(char *file, int filetype, int flag);
EXTERNCPP void RGBTest(void);
EXTERNCPP void SetupZoneDevs(void);
EXTERNCPP void UpdateColorDevices(void);
EXTERNCPP void UpdateObjectUsed(void);
EXTERNCPP float *InitSphere2(int nlat, int nlong);
EXTERNCPP void DrawHalfSphere(void);

#endif
