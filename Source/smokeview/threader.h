#ifndef THREADER_H_DEFINED
#define THREADER_H_DEFINED
#ifndef CPP
#ifdef pp_THREAD
#include <pthread.h>
#endif
#endif

#ifdef INTHREADER
#define MT_EXTERN
#else
#define MT_EXTERN extern CCC
#endif

#define MAX_PART_THREADS 16
#ifdef pp_SLICETHREAD
#define MAX_SLICE_THREADS 16
#endif

// setup LOCKS

#ifdef pp_THREAD
#ifdef pp_ISOTRIANGLES
  #define LOCK_TRIANGLES    pthread_mutex_lock(&mutexTRIANGLES);
  #define UNLOCK_TRIANGLES  pthread_mutex_unlock(&mutexTRIANGLES);
#else
#define LOCK_TRIANGLES
#define UNLOCK_TRIANGLES
#endif
#ifdef pp_SLICETHREAD
  #define LOCK_SLICE_LOAD    pthread_mutex_lock(&mutexSLICE_LOAD);
  #define UNLOCK_SLICE_LOAD  pthread_mutex_unlock(&mutexSLICE_LOAD);
#endif
  #define LOCK_PART_LOAD    pthread_mutex_lock(&mutexPART_LOAD);
  #define UNLOCK_PART_LOAD  pthread_mutex_unlock(&mutexPART_LOAD);
  #define LOCK_COMPRESS     pthread_mutex_lock(&mutexCOMPRESS);
  #define UNLOCK_COMPRESS   pthread_mutex_unlock(&mutexCOMPRESS);
  #define LOCK_VOLLOAD      pthread_mutex_lock(&mutexVOLLOAD);
  #define UNLOCK_VOLLOAD    pthread_mutex_unlock(&mutexVOLLOAD);
#ifdef pp_THREADIBLANK
  #define LOCK_IBLANK       pthread_mutex_lock(&mutexIBLANK);
  #define UNLOCK_IBLANK     pthread_mutex_unlock(&mutexIBLANK);
  #define JOIN_IBLANK       pthread_join(makeiblank_thread_id,NULL);
#else
  #define LOCK_IBLANK
  #define UNLOCK_IBLANK
  #define JOIN_IBLANK
#endif
#endif

#ifndef pp_THREAD
#ifdef pp_SLICETHREAD
  #define LOCK_SLICE_LOAD
  #define UNLOCK_SLICE_LOAD
#endif
  #define LOCK_PART_LOAD
  #define UNLOCK_PART_LOAD
  #define LOCK_TRIANGLES
  #define UNLOCK_TRIANGLES
  #define LOCK_COMPRESS
  #define UNLOCK_COMPRESS
  #define LOCK_VOLLOAD
  #define UNLOCK_VOLLOAD
  #define LOCK_IBLANK
  #define UNLOCK_IBLANK
  #define JOIN_IBLANK
#endif

#ifdef pp_THREAD
void MtReadVolsmokeAllFramesAllMeshes2(void);
#endif

// define mutex's and thread_ids

#ifndef CPP
#ifdef pp_THREAD
MT_EXTERN pthread_t makeiblank_thread_id;
#ifdef pp_SLICETHREAD
MT_EXTERN pthread_mutex_t mutexSLICE_LOAD;
#endif
MT_EXTERN pthread_mutex_t mutexPART_LOAD;
MT_EXTERN pthread_mutex_t mutexIBLANK;
MT_EXTERN pthread_mutex_t mutexVOLLOAD;
MT_EXTERN pthread_mutex_t mutexCOMPRESS;
#ifdef pp_ISOTHREAD
MT_EXTERN pthread_mutex_t mutexTRIANGLES;
#endif
MT_EXTERN pthread_t system_thread_id;
MT_EXTERN pthread_t compress_thread_id;
MT_EXTERN pthread_t update_all_patch_bounds_id;
MT_EXTERN pthread_t read_volsmoke_id;
MT_EXTERN pthread_t triangles_id;
MT_EXTERN pthread_t partthread_ids[MAX_PART_THREADS];
#ifdef pp_SLICETHREAD
MT_EXTERN pthread_t slicethread_ids[MAX_SLICE_THREADS];
#endif
#endif
#endif
#endif

