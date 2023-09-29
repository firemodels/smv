#ifndef IOVOLSMOKE_H_DEFINED
#define IOVOLSMOKE_H_DEFINED
#define GPU_VOLframemax 1.5

EXTERNCPP void UnloadVolsmokeFrameAllMeshes(int framenum);
EXTERNCPP void ComputeAllSmokecolors(void);
EXTERNCPP void DrawSmoke3DGPUVol(void);
EXTERNCPP void DrawSmoke3dVolDebug(void);
EXTERNCPP void GetVolsmokeAllTimes(volrenderdata *vr);
EXTERNCPP void InitVolRender(void);
EXTERNCPP void InitVolRenderSurface(int firstcall);
EXTERNCPP void ReadVolsmokeAllFrames(volrenderdata *vr);
EXTERNCPP void ReadVolsmokeAllFramesAllMeshes(void);
EXTERNCPP void FreeVolsmokeFrame(volrenderdata *vr, int framenum);
EXTERNCPP void ReadVolsmokeFrame(volrenderdata *vr, int framenum, int *first);
EXTERNCPP void UnloadVolsmokeAllFrames(volrenderdata *vr);
EXTERNCPP void *ReadVolsmokeAllFramesAllMeshes2(void *arg);
#endif

