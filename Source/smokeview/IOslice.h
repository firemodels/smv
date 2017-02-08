#ifndef IOSLICE_H_DEFINED
#define IOSLICE_H_DEFINED
void DrawTriangle(float *v1, float *v2, float *v3, float t1, float t2, float t3, float del, int level);
void DrawQuad(float *v1, float *v2, float *v3, float *v4, float t1, float t2, float t3, float t4, float del, int level);
void DrawQuadOutline(float *v1, float *v2, float *v3, float *v4, float del, int level);
void DrawTriangleOutline(float *v1, float *v2, float *v3, float del, int level);
void DrawQuadVector(float *v1, float *v2, float *v3, float *v4, float del, int level);

int MakeSliceSizefile(char *file, char *sizefile, int compression_type);
#endif
