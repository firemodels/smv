#define INTHREADER
#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "MALLOCC.h"
#include "threader.h"
#include GLUT_H

//***************************** multi-threaded compression ***********************************

/* ------------------ Sample ------------------------ */

#ifdef pp_SAMPLE
// example multi threading routines

/* ------------------ Sample ------------------------ */

void *Sample(void *arg){

  sample code

  THREAD_EXIT(use_sample_threads);
}
if(sample_threads==NULL){
  sample_threads = THREADinit(&n_sample_threads, &use_sample_threads, Sample);
}
THREADrun(sample_threads, arg);
#endif

/* ------------------ THREADinit ------------------------ */

threaderdata *THREADinit(int *nthreads_ptr, int *use_threads_ptr, void *(*run_arg)(void *arg)){
  threaderdata *thi;
  int nthreads_local=1, use_threads_local=0;

  //create two routines
    // void run(void){
    // }
    // void *mtrun(void *arg){
    //   run();
    //   pthread_exit(NULL);
    //   return NULL;
    // }

  NewMemory(( void ** )&thi, sizeof(threaderdata));

  if(nthreads_ptr != NULL && *nthreads_ptr > 1)nthreads_local = *nthreads_ptr;
  if(nthreads_local > MAX_THREADS)nthreads_local = MAX_THREADS;
  if(use_threads_ptr != NULL && *use_threads_ptr != 0)use_threads_local = 1;
  thi->n_threads_ptr   = nthreads_ptr;
  thi->use_threads_ptr = use_threads_ptr;
  thi->n_threads       = nthreads_local;
  thi->use_threads     = use_threads_local;
  thi->run             = run_arg;
  NewMemory(( void ** )&thi->thread_ids, nthreads_local*sizeof(pthread_t));
  pthread_mutex_init(&thi->mutex, NULL);
  return thi;
}

/* ------------------ THREADcontrol ------------------------ */

void THREADcontrol(threaderdata *thi, int var){
  if(thi == NULL)return;
  switch(var){
  case THREAD_LOCK:
    if(thi->use_threads == 1)pthread_mutex_lock(&thi->mutex);
    break;
  case THREAD_UNLOCK:
    if(thi->use_threads == 1)pthread_mutex_unlock(&thi->mutex);
    break;
  case THREAD_JOIN:
    if(thi->use_threads == 1){
      int i;

      for(i = 0;i < thi->n_threads;i++){
        pthread_join(thi->thread_ids[i], NULL);
      }
    }
    break;
  case THREAD_FREE:
    FREEMEMORY(thi->thread_ids);
    FREEMEMORY(thi);
    break;
  default:
    assert(0);
    break;
  }
}

/* ------------------ THREADrun ------------------------ */

void THREADrun(threaderdata *thi, void *arg){
  if(thi == NULL)return;
  if(thi->use_threads_ptr!=NULL)thi->use_threads = *(thi->use_threads_ptr);
  if(thi->n_threads_ptr != NULL){
    thi->n_threads = *(thi->n_threads_ptr);
    if(thi->n_threads>MAX_THREADS)thi->n_threads = MAX_THREADS;
  }
  if(thi->use_threads == 1){
    int i;

    for(i = 0; i < thi->n_threads; i++){
      pthread_create(thi->thread_ids + i, NULL, thi->run, arg);
    }
  }
  else{
   thi->run(arg);
  }
}
