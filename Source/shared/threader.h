#ifndef THREADER_H_DEFINED
#define THREADER_H_DEFINED
#include <pthread.h>

#ifdef INTHREADER
#define MTEXTERN
#define MTDECL(var,val)  var=val

#else
#define MTEXTERN extern CCC
#define MTDECL(var,val)  var
#endif

//*** parameters

#define MAX_THREADS 16

enum threaderparms {
  THREAD_LOCK,
  THREAD_UNLOCK,
  THREAD_JOIN,
  THREAD_FREE,
};

//*** structure

typedef struct _threaderdata{
  char label[32];
  int n_threads,   *n_threads_ptr;
  int use_threads, *use_threads_ptr;
  int count;
  pthread_t *thread_ids;
  pthread_mutex_t mutex;
  void *(*run)(void *arg);
} threaderdata;


//*** variables

//***checkfiles
MTEXTERN int MTDECL(n_checkfiles_threads, 1), MTDECL(use_checkfiles_threads, 1);
MTEXTERN threaderdata MTDECL(*checkfiles_threads,       NULL);

//*** compress
MTEXTERN int MTDECL(n_compress_threads, 1), MTDECL(use_compress_threads, 1);
MTEXTERN threaderdata MTDECL(*compress_threads,        NULL);

//*** ffmpeg
MTEXTERN int MTDECL(n_ffmpeg_threads, 1), MTDECL(use_ffmpeg_threads, 1);
MTEXTERN threaderdata MTDECL(*ffmpeg_threads,         NULL);

//*** iso
MTEXTERN int MTDECL(n_iso_threads, 1), MTDECL(use_iso_threads, 0), MTDECL(use_iso_threads_save,0);

//*** part
MTEXTERN int MTDECL(n_partload_threads, 2), MTDECL(use_partload_threads, 1);
MTEXTERN threaderdata MTDECL(*partload_threads,         NULL);

//*** patchbounds
MTEXTERN int MTDECL(n_patchbound_threads, 1), MTDECL(use_patchbound_threads, 1);
MTEXTERN threaderdata MTDECL(*patchbound_threads, NULL);

//*** playmovie
MTEXTERN int MTDECL(n_playmovie_threads, 1), MTDECL(use_playmovie_threads, 1);
MTEXTERN threaderdata MTDECL(*playmovie_threads,       NULL);

//*** readallgeom
MTEXTERN int MTDECL(n_readallgeom_threads, 4), MTDECL(use_readallgeom_threads, 1);
MTEXTERN threaderdata MTDECL(*readallgeom_threads,     NULL);
MTEXTERN threaderdata MTDECL(*classifyallgeom_threads, NULL);

//***slice bounds
MTEXTERN int MTDECL(n_slicebound_threads, 1), MTDECL(use_slicebound_threads, 1);
MTEXTERN threaderdata MTDECL(*slicebound_threads, NULL);

//*** slice
#ifdef pp_SLICE_MULTI // not implemented
MTEXTERN int MTDECL(n_sliceload_threads, 4), MTDECL(use_sliceload_threads, 0);
MTEXTERN threaderdata MTDECL(*sliceload_threads, NULL);
#endif

//*** smoke
MTEXTERN int MTDECL(n_smokeload_threads, 1), MTDECL(use_smokeload_threads, 0);

//***triangles
MTEXTERN int MTDECL(n_triangles_threads, 1), MTDECL(use_triangles_threads, 1);
MTEXTERN threaderdata MTDECL(*triangles_threads, NULL);

//*** volsmoke
MTEXTERN int MTDECL(n_volsmokeload_threads, 1), MTDECL(use_volsmokeload_threads, 0);
MTEXTERN threaderdata MTDECL(*volsmokeload_threads, NULL);

//*** routines

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

//*** threader controls

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

