#ifndef IOVOLSMOKE_H_DEFINED
#define IOVOLSMOKE_H_DEFINED
#define GPU_VOLframemax 1.5

EXTERNCPP void UnloadVolsmokeFrameAllMeshes(int framenum);
EXTERNCPP void ComputeAllSmokecolors(void);
EXTERNCPP void DrawSmoke3DGPUVol(void);
EXTERNCPP void DrawSmoke3dVolDebug(void);
EXTERNCPP void GetVolsmokeAllTimes(volrenderdata *vr);
#ifdef pp_VOL_OLD
EXTERNCPP void InitVolRender(void);
EXTERNCPP void InitVolRenderSurface(int firstcall);
#endif
EXTERNCPP void ReadVolsmokeAllFrames(volrenderdata *vr);
EXTERNCPP void ReadVolsmokeAllFramesAllMeshes(void);
EXTERNCPP void FreeVolsmokeFrame(volrenderdata *vr, int framenum);
EXTERNCPP void ReadVolsmokeFrame(volrenderdata *vr, int framenum, int *first);
EXTERNCPP void UnloadVolsmokeAllFrames(volrenderdata *vr);

//*** IOvolsmoke.c headers

#ifdef pp_VOL_OLD
#ifdef pp_GPU
EXTERNCPP void DefineVolsmokeTextures(void);
#endif
EXTERNCPP void DrawSmoke3DVol(void);
EXTERNCPP void *ReadVolsmokeAllFramesAllMeshes2(void *arg);

#endif
EXTERNCPP void *InitNabors(void *arg);
EXTERNCPP void MakeFireColors(float temp_min, float temp_max, int nfire_colors_arg);
#endif

