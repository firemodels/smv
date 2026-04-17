#define INTHREADER
#include "options_common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dmalloc.h"
#include "threader.h"

// /* ------------------ Sample ------------------------ */

// void *Sample(void *arg){
//  // n_sample_threads - number of threads - default 1
//  // use_sample_threads 0/1 use multi threading or not
//  // threads - data structure that holds threading instance
//  // Sample - routine that does the work, must end with THREAD_EXIT macro

//   sample code

//   THREAD_EXIT(sample_threads);
// }
// //*** call before first use of threading routines

// ThreadInit(&sample_threads, n_sample_threads, use_sample_threads, serial_override, Sample);
//
// //*** call to do the work
// ThreadRun(sample_threads);

/* ------------------ ThreadInsert ------------------------ */

void ThreadInsert(threaderdata *thi){
  threaderdata *n = &threadlast;
  threaderdata *p = n->prev;

  p->next = thi;
  n->prev = thi;

  thi->prev = p;
  thi->next = n;
}

/* ------------------ ThreadRemove ------------------------ */

void ThreadRemove(threaderdata *thi){
  for(threaderdata *t=(&threadfirst)->next;t!=NULL;t=t->next){
    if(t==thi){
      threaderdata *p=t->prev, *n=t->next;
      if(p!=NULL)p->next = n;
      if(n!=NULL)n->prev = p;
      break;
    }
  }
}

/* ------------------ THREADinit ------------------------ */

void ThreadInit(threaderdata **thiptr, int n_threads, int use_threads, int run_serial_override,
  void *(*run_arg)(void *arg)){
  threaderdata *thi;

  //create a routine
    // void *run(void *arg){
    //   run();
    //   pthread_exit(NULL); THREAD_EXIT macros handles these two lines
    //   return NULL;
    // }

  assert(*thiptr == NULL);
  NewMemory((void **)&thi, sizeof(threaderdata));

  if(n_threads<1)n_threads = 1;
  if(n_threads > MAX_THREADS)n_threads = MAX_THREADS;
  if(run_serial_override != 0)use_threads = 0;
  if(use_threads != 0)use_threads = 1;
  if(use_threads == 0)n_threads = 1;

  thi->n_threads       = n_threads;
  thi->use_threads     = use_threads;
  thi->run             = run_arg;
  thi->address         = thiptr;
#ifdef pp_THREAD
  if(use_threads == 1){
    NewMemory((void **)&thi->thread_ids, n_threads*sizeof(pthread_t));
    pthread_mutex_init(&thi->mutex, NULL);
  }
#endif
  ThreadInsert(thi);
  *thiptr = thi;
}

/* ------------------ THREADlock ------------------------ */

void ThreadLock(threaderdata *thi){
#ifdef pp_THREAD
  if(thi == NULL)return;
  if(thi->use_threads == 1)pthread_mutex_lock(&thi->mutex);
#endif
}

/* ------------------ THREADunlock ------------------------ */

void ThreadUnlock(threaderdata *thi){
#ifdef pp_THREAD
  if(thi == NULL)return;
  if(thi->use_threads == 1)pthread_mutex_unlock(&thi->mutex);
#endif
}

/* ------------------ THREADcontrol ------------------------ */

void ThreadJoin(threaderdata **thiptr){
#ifdef pp_THREAD
  if(thiptr == NULL || *thiptr == NULL)return;
  threaderdata *thi = *thiptr;
  if(thi->use_threads == 1){
    for(int i = 0; i < thi->n_threads; i++){
      pthread_join(thi->thread_ids[i], NULL);
    }
  }
  ThreadRemove(thi);
  FREEMEMORY(thi);
  *thiptr = thi;
#endif
}

/* ------------------ ThreadJoinAll ------------------------ */

int ThreadCount(void){
  int nthreads=0;
  for(threaderdata *t=(&threadfirst)->next;t!=NULL;t=t->next){
    nthreads++;
  }
  return nthreads;
}

/* ------------------ ThreadJoinAll ------------------------ */

void ThreadJoinAll(void){
  threaderdata **threadlist;

  int nthreads=ThreadCount();
  if(nthreads==0)return;
  NewMemory((void **)&threadlist, nthreads*sizeof(threaderdata *));
  nthreads = 0;
  for(threaderdata *t=(&threadfirst)->next;t!=NULL;t=t->next){
    threadlist[nthreads++] = t;
  }
  for(int i=0;i<nthreads;i++){
    ThreadJoin(threadlist[i]->address);
  }
  FREEMEMORY(threadlist);
}

/* ------------------ THREADruni ------------------------ */

void ThreadRuni(threaderdata *thi, unsigned char *datainfo, int sizedatai){
#ifdef pp_THREAD
  if(thi == NULL)return;
  for(int i = 0; i < thi->n_threads; i++){
    unsigned char *datai;

    datai = NULL;
    if(datainfo != NULL)datai = datainfo + i*sizedatai;
    if(thi->use_threads == 1){
      pthread_create(thi->thread_ids + i, NULL, thi->run, (void *)datai);
    }
    else{
      thi->run(datai);
    }
  }
#else
  int i;

  for(i = 0; i < thi->n_threads; i++){
    unsigned char *datai;

    datai = datainfo + i*sizedatai;
    thi->run(datai);
  }
#endif
}

/* ------------------ THREADrunloop ------------------------ */

void ThreadRunLoop(threaderdata *thi){
  ThreadRuni(thi, (unsigned char *)thread_ids, sizeof(int));
}

/* ------------------ THREADrun ------------------------ */

void ThreadRun(threaderdata *thi){
  ThreadRuni(thi, NULL, 0);
}

/* ------------------ THREADrun ------------------------ */

void ThreadSetup(void){
  threadfirst.prev = NULL;
  threadfirst.next = &threadlast;
  threadlast.prev  = &threadfirst;
  threadlast.next  = NULL;
  for(int i = 0; i < MAX_THREADS; i++){
    thread_ids[i] = i;
  }
}
