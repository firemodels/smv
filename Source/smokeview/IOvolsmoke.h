#ifndef IOVOLSMOKE_H_DEFINED
#define IOVOLSMOKE_H_DEFINED
#define GPU_VOLframemax 1.5
#define MLEFT 0
#define MFRONT 1
#define MDOWN 2
#define MRIGHT 3
#define MBACK 4
#define MUP 5
#define MEPS 0.1

void UnloadVolsmokeFrameAllMeshes(int framenum);
void ComputeAllSmokecolors(void);
void DrawSmoke3DGPUVOL(void);
void DrawSmoke3DVOLdebug(void);
void GetVolsmokeAllTimes(volrenderdata *vr);
void InitVolRender(void);
void InitVolRenderSurface(int firstcall);
void ReadVolsmokeAllFrames(volrenderdata *vr);
void ReadVolsmokeAllFramesAllMeshes(void);
void FreeVolsmokeFrame(volrenderdata *vr, int framenum);
void ReadVolsmokeFrame(volrenderdata *vr, int framenum, int *first);
void UnloadVolsmokeAllFrames(volrenderdata *vr);
void *ReadVolsmokeAllFramesAllMeshes2(void *arg);
#endif

