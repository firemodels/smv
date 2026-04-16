#ifndef THREADER_H_DEFINED
#define THREADER_H_DEFINED
#ifdef pp_THREAD
#include <pthread.h>
#endif
#include "file_util.h"

//*** parameters

#define MAX_THREADS 16

//*** structure

typedef struct _threaderdata{
  int n_threads, use_threads;
#ifdef pp_THREAD
  pthread_t *thread_ids;
  pthread_mutex_t mutex;
#endif
  void *(*run)(void *arg);
} threaderdata;

//*** routines

EXTERNCPP void ThreadLock(threaderdata *thi);
EXTERNCPP void ThreadUnlock(threaderdata *thi);
EXTERNCPP void ThreadRun(threaderdata *thi);
EXTERNCPP void ThreadRunLoop(threaderdata *thi);
EXTERNCPP void ThreadRuni(threaderdata * thi, unsigned char *datainfo, int sizedatai);
EXTERNCPP threaderdata *ThreadInit(int nthreads_arg, int threading_on_arg, int run_serial_override, void *(*run_arg)(void *arg));
EXTERNCPP void ThreadJoin(threaderdata **thiptr);

//*** threader controls

#ifdef pp_THREAD
#define THREAD_EXIT(threads)  \
    if(threads->use_threads==1)pthread_exit(NULL);\
    return NULL
#else
#define THREAD_EXIT(threads) return NULL
#endif

#endif

