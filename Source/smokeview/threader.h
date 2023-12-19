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

  #define LOCK_VOLLOAD      pthread_mutex_lock(&mutexVOLLOAD);
  #define UNLOCK_VOLLOAD    pthread_mutex_unlock(&mutexVOLLOAD);

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
  #define LOCK_PART_LOAD
  #define UNLOCK_PART_LOAD

  #define LOCK_VOLLOAD
  #define UNLOCK_VOLLOAD

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
MT_EXTERN pthread_mutex_t mutexSLICEBOUNDS;
MT_EXTERN pthread_mutex_t mutexPATCHBOUNDS;

MT_EXTERN pthread_t PATCHBOUNDS_thread_id;
MT_EXTERN pthread_t SLICEBOUNDS_thread_id;
MT_EXTERN pthread_t update_all_patch_bounds_id;
MT_EXTERN pthread_t read_volsmoke_id;
MT_EXTERN pthread_t csv_id;
MT_EXTERN pthread_t partthread_ids[MAX_THREADS];
MT_EXTERN pthread_t generate_part_histogram_id;
MT_EXTERN pthread_t *readbuffer_ids;
#ifdef pp_SLICE_MULTI
MT_EXTERN pthread_t slicethread_ids[MAX_THREADS];
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
  int n_threads,   *n_threads_ptr;
  int use_threads, *use_threads_ptr;
  pthread_t *thread_ids;
  pthread_mutex_t mutex;
  void (*run)(void);
  void *(*mtrun)(void *arg);
} threaderdata;

EXTERNCPP void THREADcontrol(threaderdata *thi, int var);
EXTERNCPP void THREADrun(threaderdata *thi);
EXTERNCPP threaderdata *THREADinit(int *nthreads_arg, int *threading_on_arg,
                                      void (*run_arg)(void), void *(*mtrun_arg)(void *arg));

EXTERNCPP void *MtUpdateTrianglesAll(void *arg);
EXTERNCPP void UpdateTrianglesAll(void);

EXTERNCPP void *MtCheckFiles(void *arg);
EXTERNCPP void CheckFiles(void);

EXTERNCPP void *MtCompress(void *arg);
EXTERNCPP void Compress(void);

EXTERNCPP void *MtPlayMovie(void *arg);
EXTERNCPP void PlayMovie(void);

EXTERNCPP void *MtSetupFF(void *arg);
EXTERNCPP void SetupFF(void);

EXTERNCPP void *MtReadAllGeom(void *arg);
EXTERNCPP void ReadAllGeom(void);

EXTERNCPP void *MtClassifyAllGeom(void *arg);
EXTERNCPP void ClassifyAllGeom(void);

#ifdef pp_THREAD
#define LOCK_THREADS(thi)   THREADcontrol(thi, THEAD_LOCK)
#define UNLOCK_THREADS(thi) THREADcontrol(thi, THEAD_UNLOCK)
#define JOIN_THREADS(thi)   THREADcontrol(thi, THEAD_JOIN)
#else
#define LOCK_THREADS(thi)
#define UNLOCK_THREADS(thi)
#define JOIN_THREADS(thi)
#endif

#endif


#endif

