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

#define MAX_THREADS 16

// setup LOCKS

#ifdef pp_THREAD

  #define LOCK_READALLGEOM     pthread_mutex_lock(&mutexREADALLGEOM);
  #define UNLOCK_READALLGEOM   pthread_mutex_unlock(&mutexREADALLGEOM);

  #define LOCK_PART_LOAD    pthread_mutex_lock(&mutexPART_LOAD);
  #define UNLOCK_PART_LOAD  pthread_mutex_unlock(&mutexPART_LOAD);

#ifdef pp_SLICETHREAD
  #define LOCK_SLICE_LOAD    pthread_mutex_lock(&mutexSLICE_LOAD);
  #define UNLOCK_SLICE_LOAD  pthread_mutex_unlock(&mutexSLICE_LOAD);
#else
  #define LOCK_SLICE_LOAD
  #define UNLOCK_SLICE_LOAD
#endif

  #define LOCK_COMPRESS     pthread_mutex_lock(&mutexCOMPRESS);
  #define UNLOCK_COMPRESS   pthread_mutex_unlock(&mutexCOMPRESS);

  #define LOCK_VOLLOAD      pthread_mutex_lock(&mutexVOLLOAD);
  #define UNLOCK_VOLLOAD    pthread_mutex_unlock(&mutexVOLLOAD);

  #define LOCK_IBLANK       pthread_mutex_lock(&mutexIBLANK);
  #define UNLOCK_IBLANK     pthread_mutex_unlock(&mutexIBLANK);
  #define JOIN_IBLANK       pthread_join(makeiblank_thread_id,NULL);

  #define JOIN_PART_HIST    pthread_join(generate_part_histogram_id,NULL);
#ifdef pp_SAMPLE
  #define LOCK_SAMPLE     pthread_mutex_lock(&mutexSAMPLE);
  #define UNLOCK_SAMPLE   pthread_mutex_unlock(&mutexSAMPLE);
  #define JOIN_SAMPLE     pthread_join(sample_thread_id,NULL);
#endif

#endif

// blank out all preprocessing symbols if we arn't using threading
#ifndef pp_THREAD

  #define LOCK_READALLGEOM
  #define UNLOCK_READALLGEOM

  #define LOCK_PART_LOAD
  #define UNLOCK_PART_LOAD

  #define LOCK_COMPRESS
  #define UNLOCK_COMPRESS

  #define LOCK_VOLLOAD
  #define UNLOCK_VOLLOAD

  #define LOCK_IBLANK
  #define UNLOCK_IBLANK
  #define JOIN_IBLANK

#ifdef pp_SAMPLE
  #define LOCK_SAMPLE
  #define UNLOCK_SAMPLE
  #define JOIN_SAMPLE
#endif

#define JOIN_PART_HIST
#endif

#ifdef pp_THREAD
void MtReadVolsmokeAllFramesAllMeshes2(void);
#endif

// define mutex's and thread_ids

#ifndef CPP
#ifdef pp_THREAD

MT_EXTERN pthread_mutex_t mutexREADALLGEOM;
#ifdef pp_SLICETHREAD
MT_EXTERN pthread_mutex_t mutexSLICE_LOAD;
#endif
MT_EXTERN pthread_mutex_t mutexPART_LOAD;
MT_EXTERN pthread_mutex_t mutexIBLANK;
MT_EXTERN pthread_mutex_t mutexVOLLOAD;
MT_EXTERN pthread_mutex_t mutexCOMPRESS;
#ifdef pp_SAMPLE
MT_EXTERN pthread_mutex_t mutexSAMPLE;
#endif

MT_EXTERN pthread_t makeiblank_thread_id;
MT_EXTERN pthread_t system_thread_id;
MT_EXTERN pthread_t compress_thread_id;
MT_EXTERN pthread_t update_all_patch_bounds_id;
MT_EXTERN pthread_t read_volsmoke_id;
MT_EXTERN pthread_t triangles_id;
MT_EXTERN pthread_t partthread_ids[MAX_THREADS];
MT_EXTERN pthread_t generate_part_histogram_id;
#ifdef pp_SLICETHREAD
MT_EXTERN pthread_t slicethread_ids[MAX_THREADS];
#endif
MT_EXTERN pthread_t readallgeomthread_ids[MAX_THREADS];
MT_EXTERN pthread_t classifyallgeomthread_ids[MAX_THREADS];
#ifdef pp_SAMPLE
MT_EXTERN pthread_t sample_thread_id;
#endif

#endif
#endif

#endif

