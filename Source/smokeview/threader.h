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
  MMEXTERN pthread_mutex_t mutexSLICE_BOUND, mutexPATCH_BOUND, mutexPART2ISO, mutexPRINT, mutexMEM;

  #define LOCK_PART_LOAD    pthread_mutex_lock(&mutexPART_LOAD);
  #define UNLOCK_PART_LOAD  pthread_mutex_unlock(&mutexPART_LOAD);

#ifdef pp_SLICE_MULTI
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

  #define LOCK_CHECKFILES   if(checkfiles_multithread==1)pthread_mutex_lock(&mutexCHECKFILES);
  #define UNLOCK_CHECKFILES if(checkfiles_multithread==1)pthread_mutex_unlock(&mutexCHECKFILES);
  #define JOIN_CHECKFILES   if(checkfiles_multithread==1)pthread_join(checkfiles_multithread_id,NULL);

  #define LOCK_SLICEBOUNDS    pthread_mutex_lock(&mutexSLICEBOUNDS);
  #define UNLOCK_SLICEBOUNDS  pthread_mutex_unlock(&mutexSLICEBOUNDS);
  #define JOIN_SLICEBOUNDS    pthread_join(SLICEBOUNDS_thread_id,NULL);

  #define LOCK_PATCHBOUNDS    pthread_mutex_lock(&mutexPATCHBOUNDS);
  #define UNLOCK_PATCHBOUNDS  pthread_mutex_unlock(&mutexPATCHBOUNDS);
  #define JOIN_PATCHBOUNDS    pthread_join(PATCHBOUNDS_thread_id,NULL);

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

  #define LOCK_CHECKFILES
  #define UNLOCK_CHECKFILES
  #define JOIN_CHECKFILES

  #define LOCK_SLICEBOUNDS
  #define UNLOCK_SLICEBOUNDS
  #define JOIN_SLICEBOUNDS

  #define LOCK_PATCHBOUNDS
  #define UNLOCK_PATCHBOUNDS
  #define JOIN_PATCHBOUNDS

#ifdef pp_SAMPLE
  #define LOCK_SAMPLE
  #define UNLOCK_SAMPLE
  #define JOIN_SAMPLE
#endif

  #define JOIN_PART_HIST
#endif

#ifdef pp_THREAD
void *MtReadBufferi(void *arg);
void MtReadVolsmokeAllFramesAllMeshes2(void);
#endif

// define mutex's and thread_ids

#ifndef CPP
#ifdef pp_THREAD

#ifdef pp_SLICE_MULTI
MT_EXTERN pthread_mutex_t mutexSLICE_LOAD;
#endif
MT_EXTERN pthread_mutex_t mutexPART_LOAD;
MT_EXTERN pthread_mutex_t mutexVOLLOAD;
MT_EXTERN pthread_mutex_t mutexCOMPRESS;
MT_EXTERN pthread_mutex_t mutexCHECKFILES;
MT_EXTERN pthread_mutex_t mutexSLICEBOUNDS;
MT_EXTERN pthread_mutex_t mutexPATCHBOUNDS;
#ifdef pp_SAMPLE
MT_EXTERN pthread_mutex_t mutexSAMPLE;
#endif

MT_EXTERN pthread_t PATCHBOUNDS_thread_id;
MT_EXTERN pthread_t SLICEBOUNDS_thread_id;
MT_EXTERN pthread_t checkfiles_multithread_id;
MT_EXTERN pthread_t compress_thread_id;
MT_EXTERN pthread_t update_all_patch_bounds_id;
MT_EXTERN pthread_t read_volsmoke_id;
MT_EXTERN pthread_t triangles_id;
MT_EXTERN pthread_t csv_id;
MT_EXTERN pthread_t partthread_ids[MAX_THREADS];
MT_EXTERN pthread_t generate_part_histogram_id;
MT_EXTERN pthread_t *readbuffer_ids;
#ifdef pp_SLICE_MULTI
MT_EXTERN pthread_t slicethread_ids[MAX_THREADS];
#endif
MT_EXTERN pthread_t readallgeomthread_ids[MAX_THREADS];
MT_EXTERN pthread_t classifyallgeomthread_ids[MAX_THREADS];
#ifdef pp_SAMPLE
MT_EXTERN pthread_t sample_thread_id;
#endif

#endif
#endif

// ------------ new threader routines -----------------------------
#ifdef pp_THREAD_NEW
#define THREAD_LOCK   0
#define THREAD_UNLOCK 1
#define THREAD_JOIN   2
#define THREAD_FREE   3

typedef struct _threaderdata{
  int nthreads;
  int threading_on;
  pthread_t *thread_ids;
  pthread_mutex_t mutex;
  void (*run)(void);
  void *(*mtrun)(void *arg);
} threaderdata;

EXTERNCPP void THREADERcontrol(threaderdata *thi, int var);
EXTERNCPP void THREADERrun(threaderdata *thi);
EXTERNCPP threaderdata *THREADERinit(int nthreads_arg, int threading_on_arg,
                                      void (*run_arg)(void), void *(*mtrun_arg)(void *arg));

EXTERNCPP void *MTPlayMovie(void *arg);
EXTERNCPP void PlayMovie(void);

EXTERNCPP void *MTSetupFF(void *arg);
EXTERNCPP void SetupFF(void);

EXTERNCPP void *MtReadAllGeom(void *arg);
EXTERNCPP void ReadAllGeom(void);

#ifdef pp_THREAD
#define LOCK_THREADS(thi)   THREADERcontrol(thi, THEAD_LOCK)
#define UNLOCK_THREADS(thi) THREADERcontrol(thi, THEAD_UNLOCK)
#define JOIN_THREADS(thi)   THREADERcontrol(thi, THEAD_JOIN)
#else
#define LOCK_THREADS(thi)
#define UNLOCK_THREADS(thi)
#define JOIN_THREADS(thi)
#endif

#endif


#endif

