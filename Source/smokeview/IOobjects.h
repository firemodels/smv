#ifndef IOOBJECT_H_DEFINED
#define IOOBJECT_H_DEFINED


void DrawBox2(float *origin, float *dxyz, float *color, int draw_outline);
void DrawCone(float d1, float height, unsigned char *rgbcolor);
void DrawTruncCone(float d1, float d2, float height, unsigned char *rgbcolor);
void DrawSphere(float diameter, unsigned char *rgbcolor);
void DrawDisk(float diameter, float height, unsigned char *rgbcolor);
void DrawWheel(float diameter, float thickness, unsigned char *rgbcolor);
void DrawSmvObject(sv_object *object, int frame_index_local, propdata *prop, int recurse_level, float *valrgb, int vis_override);
#endif
