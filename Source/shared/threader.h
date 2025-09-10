#ifndef THREADER_H_DEFINED
#define THREADER_H_DEFINED
#ifdef pp_THREAD
#include <pthread.h>
#endif
#include "file_util.h"

//*** parameters

#define MAX_THREADS 16

enum threaderparms {
  THREAD_UPDATE,
  THREAD_LOCK,
  THREAD_FORCE_UNLOCK,
  THREAD_UNLOCK,
  THREAD_JOIN
};

//*** structure

typedef struct _threaderdata{
  int n_threads,   *n_threads_ptr;
  int use_threads, *use_threads_ptr;
#ifdef pp_THREAD
  pthread_t *thread_ids;
  pthread_mutex_t mutex;
#endif
  void *(*run)(void *arg);
} threaderdata;

//*** routines

EXTERNCPP void THREADcontrol(threaderdata *thi, int var);
EXTERNCPP void THREADrun(threaderdata *thi);
EXTERNCPP void THREADruni(threaderdata *thi, unsigned char *datainfo, int sizedatai);
EXTERNCPP threaderdata *THREADinit(int *nthreads_arg, int *threading_on_arg, void *(*run_arg)(void *arg));

//*** threader controls

#ifdef pp_THREAD
#define LOCK_THREADS(thi)   THREADcontrol(thi, THEAD_LOCK)
#define UNLOCK_THREADS(thi) THREADcontrol(thi, THEAD_UNLOCK)
#define JOIN_THREADS(thi)   THREADcontrol(thi, THEAD_JOIN)
#define THREAD_EXIT(threads)  \
    if(use_ ## threads==1)pthread_exit(NULL);\
    return NULL
#else
#define LOCK_THREADS(thi)
#define UNLOCK_THREADS(thi)
#define JOIN_THREADS(thi)
#define THREAD_EXIT(threads) return NULL
#endif

#endif

