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

#ifdef pp_STREAM
  #define LOCK_STREAM     pthread_mutex_lock(&mutexSTREAM);
  #define UNLOCK_STREAM   pthread_mutex_unlock(&mutexSTREAM);
#else
  #define LOCK_STREAM
  #define UNLOCK_STREAM
#endif

  #define LOCK_READALLGEOM     pthread_mutex_lock(&mutexREADALLGEOM);
  #define UNLOCK_READALLGEOM   pthread_mutex_unlock(&mutexREADALLGEOM);

  #define LOCK_PART_LOAD    pthread_mutex_lock(&mutexPART_LOAD);
  #define UNLOCK_PART_LOAD  pthread_mutex_unlock(&mutexPART_LOAD);

#ifdef pp_CSV_MULTI
  #define JOIN_CSVFILES     FinishAllCSVFiles();
  #define LOCK_CSV_LOAD     if(csv_multithread==1)pthread_mutex_lock(&mutexCSV_LOAD);
  #define UNLOCK_CSV_LOAD   if(csv_multithread==1)pthread_mutex_unlock(&mutexCSV_LOAD);
  #define LOCK_CSV_LOAD_CPP LockCSV()
  #define UNLOCK_CSV_LOAD_CPP UnLockCSV()
#else
  #define JOIN_CSVFILES
  #define LOCK_CSV_LOAD
  #define UNLOCK_CSV_LOAD
  #define LOCK_CSV_LOAD_CPP
  #define UNLOCK_CSV_LOAD_CPP
#endif

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

  #define LOCK_IBLANK       pthread_mutex_lock(&mutexIBLANK);
  #define UNLOCK_IBLANK     pthread_mutex_unlock(&mutexIBLANK);
  #define JOIN_IBLANK       pthread_join(makeiblank_thread_id,NULL);

  #define LOCK_SETUPFF      pthread_mutex_lock(&mutexSETUPFF);
  #define UNLOCK_SETUPFF    pthread_mutex_unlock(&mutexSETUPFF);
  #define JOIN_SETUPFF      pthread_join(setupff_thread_id,NULL);

  #define LOCK_CHECKFILES   pthread_mutex_lock(&mutexCHECKFILES);
  #define UNLOCK_CHECKFILES pthread_mutex_unlock(&mutexCHECKFILES);
  #define JOIN_CHECKFILES   pthread_join(CHECKFILES_thread_id,NULL);

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

  #define LOCK_STREAM
  #define UNLOCK_STREAM

  #define LOCK_READALLGEOM
  #define UNLOCK_READALLGEOM

  #define LOCK_CSV_LOAD
  #define UNLOCK_CSV_LOAD

  #define LOCK_PART_LOAD
  #define UNLOCK_PART_LOAD

  #define LOCK_COMPRESS
  #define UNLOCK_COMPRESS

  #define LOCK_VOLLOAD
  #define UNLOCK_VOLLOAD

  #define LOCK_IBLANK
  #define UNLOCK_IBLANK
  #define JOIN_IBLANK

  #define LOCK_SETUPFF
  #define UNLOCK_SETUPFF
  #define JOIN_SETUPFF

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

#define JOIN_CSVFILES
#define LOCK_CSV_LOAD_CPP
#define UNLOCK_CSV_LOAD_CPP

#define JOIN_PART_HIST
#endif

#ifdef pp_THREAD
void MtReadVolsmokeAllFramesAllMeshes2(void);
#endif

// define mutex's and thread_ids

#ifndef CPP
#ifdef pp_THREAD

MT_EXTERN pthread_mutex_t mutexREADALLGEOM;
#ifdef pp_SLICE_MULTI
MT_EXTERN pthread_mutex_t mutexSLICE_LOAD;
#endif
MT_EXTERN pthread_mutex_t mutexCSV_LOAD;
MT_EXTERN pthread_mutex_t mutexPART_LOAD;
MT_EXTERN pthread_mutex_t mutexIBLANK;
MT_EXTERN pthread_mutex_t mutexVOLLOAD;
MT_EXTERN pthread_mutex_t mutexCOMPRESS;
MT_EXTERN pthread_mutex_t mutexSETUPFF;
MT_EXTERN pthread_mutex_t mutexCHECKFILES;
MT_EXTERN pthread_mutex_t mutexSLICEBOUNDS;
MT_EXTERN pthread_mutex_t mutexPATCHBOUNDS;
#ifdef pp_STREAM
MT_EXTERN pthread_mutex_t mutexSTREAM;
#endif
#ifdef pp_SAMPLE
MT_EXTERN pthread_mutex_t mutexSAMPLE;
#endif

#ifdef pp_STREAM
MT_EXTERN pthread_t stream_thread_id;
#endif
MT_EXTERN pthread_t makeiblank_thread_id;
MT_EXTERN pthread_t setupff_thread_id;
MT_EXTERN pthread_t PATCHBOUNDS_thread_id;
MT_EXTERN pthread_t SLICEBOUNDS_thread_id;
MT_EXTERN pthread_t CHECKFILES_thread_id;
MT_EXTERN pthread_t system_thread_id;
MT_EXTERN pthread_t compress_thread_id;
MT_EXTERN pthread_t update_all_patch_bounds_id;
MT_EXTERN pthread_t read_volsmoke_id;
MT_EXTERN pthread_t triangles_id;
MT_EXTERN pthread_t csv_ids[MAX_THREADS];
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

#endif

