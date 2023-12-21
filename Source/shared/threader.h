#ifndef THREADER_H_DEFINED
#define THREADER_H_DEFINED
#include <pthread.h>

#ifdef INTHREADER
#define MT_EXTERN
#else
#define MT_EXTERN extern CCC
#endif

#define MAX_THREADS 16

#define THREAD_LOCK   0
#define THREAD_UNLOCK 1
#define THREAD_JOIN   2
#define THREAD_FREE   3

typedef struct _threaderdata{
  char label[32];
  int n_threads,   *n_threads_ptr;
  int use_threads, *use_threads_ptr;
  int count;
  pthread_t *thread_ids;
  pthread_mutex_t mutex;
  void *(*run)(void *arg);
} threaderdata;

//***checkfiles
SVEXTERN int SVDECL(n_checkfiles_threads, 1), SVDECL(use_checkfiles_threads, 1);
SVEXTERN threaderdata SVDECL(*checkfiles_threads,       NULL);

//*** compress
SVEXTERN int SVDECL(n_compress_threads, 1), SVDECL(use_compress_threads, 1);
SVEXTERN threaderdata SVDECL(*compress_threads,        NULL);

//*** ffmpeg
SVEXTERN int SVDECL(n_ffmpeg_threads, 1), SVDECL(use_ffmpeg_threads, 1);
SVEXTERN threaderdata SVDECL(*ffmpeg_threads,         NULL);

//*** iso
SVEXTERN int SVDECL(n_iso_threads, 1), SVDECL(use_iso_threads, 0), SVDECL(use_iso_threads_save,0);

//*** part
SVEXTERN int SVDECL(n_partload_threads, 2), SVDECL(use_partload_threads, 1);
SVEXTERN threaderdata SVDECL(*partload_threads,         NULL);

//*** patchbounds
SVEXTERN int SVDECL(n_patchbound_threads, 1), SVDECL(use_patchbound_threads, 1);
SVEXTERN threaderdata SVDECL(*patchbound_threads, NULL);

//*** playmovie
SVEXTERN int SVDECL(n_playmovie_threads, 1), SVDECL(use_playmovie_threads, 1);
SVEXTERN threaderdata SVDECL(*playmovie_threads,       NULL);

//*** readallgeom
SVEXTERN int SVDECL(n_readallgeom_threads, 4), SVDECL(use_readallgeom_threads, 1);
SVEXTERN threaderdata SVDECL(*readallgeom_threads,     NULL);
SVEXTERN threaderdata SVDECL(*classifyallgeom_threads, NULL);

//***slice bounds
SVEXTERN int SVDECL(n_slicebound_threads, 1), SVDECL(use_slicebound_threads, 1);
SVEXTERN threaderdata SVDECL(*slicebound_threads, NULL);

//*** slice
#ifdef pp_SLICE_MULTI // not implemented
SVEXTERN int SVDECL(n_sliceload_threads, 4), SVDECL(use_sliceload_threads, 0);
SVEXTERN threaderdata SVDECL(*sliceload_threads, NULL);
#endif

//*** smoke
SVEXTERN int SVDECL(n_smokeload_threads, 1), SVDECL(use_smokeload_threads, 0);

//***triangles
SVEXTERN int SVDECL(n_triangles_threads, 1), SVDECL(use_triangles_threads, 1);
SVEXTERN threaderdata SVDECL(*triangles_threads, NULL);

//*** volsmoke
SVEXTERN int SVDECL(n_volsmokeload_threads, 1), SVDECL(use_volsmokeload_threads, 0);
SVEXTERN threaderdata SVDECL(*volsmokeload_threads, NULL);

EXTERNCPP void THREADcontrol(threaderdata *thi, int var);
EXTERNCPP void THREADrun(threaderdata *thi, void *arg);
EXTERNCPP threaderdata *THREADinit(char *label, int *nthreads_arg, int *threading_on_arg, void *(*run_arg)(void *arg));

EXTERNCPP void *CheckFiles(void *arg);
EXTERNCPP void *ClassifyAllGeom(void *arg);
EXTERNCPP void *Compress(void *arg);
EXTERNCPP void *GetGlobalPatchBoundsFull(void *arg);
EXTERNCPP void *GetGlobalSliceBoundsFull(void *arg);
EXTERNCPP void *PlayMovie(void *arg);
EXTERNCPP void *ReadAllGeom(void *arg);
EXTERNCPP void *ReadVolsmokeAllFramesAllMeshes2(void *arg);
EXTERNCPP void *SetupFF(void *arg);
EXTERNCPP void *UpdateTrianglesAll(void *arg);

#ifdef pp_THREAD
#define LOCK_THREADS(thi)   THREADcontrol(thi, THEAD_LOCK)
#define UNLOCK_THREADS(thi) THREADcontrol(thi, THEAD_UNLOCK)
#define JOIN_THREADS(thi)   THREADcontrol(thi, THEAD_JOIN)
#define THREAD_EXIT(threads)  \
    if(threads!=NULL)threads->count--;\
    if(use_ ## threads==1)pthread_exit(NULL);\
    return NULL
#else
#define LOCK_THREADS(thi)
#define UNLOCK_THREADS(thi)
#define JOIN_THREADS(thi)
#define THREAD_EXIT(threads)
#endif
#endif

