#define INTHREADER
#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "smokestream.h"
#include GLUT_H

/* ------------------ InitMultiThreading ------------------------ */

void InitMultiThreading(void){
#ifdef pp_THREAD
#ifdef pp_SLICE_MULTI
  pthread_mutex_init(&mutexSLICE_LOAD, NULL);
#endif
  pthread_mutex_init(&mutexPART_LOAD, NULL);
  pthread_mutex_init(&mutexVOLLOAD,NULL);
  pthread_mutex_init(&mutexSLICEBOUNDS, NULL);
  pthread_mutex_init(&mutexPATCHBOUNDS, NULL);
#endif
}

//***************************** multi-threaded compression ***********************************

#ifdef pp_THREAD

/* --------------------------  slicethreaddata ------------------------------------ */

typedef struct _slicethreaddata {
  int slice_index;
  FILE_SIZE file_size;
} slicethreaddata;

/* ------------------ MtLoadAllPartFiles ------------------------ */

void *MtLoadAllPartFiles(void *arg){
  int *valptr;

  valptr = (int *)(arg);
  LoadAllPartFiles(*valptr);
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ LoadAllPartFilesMT ------------------------ */

void LoadAllPartFilesMT(int partnum){
  int i;

  if(use_part_threads==0){
    LoadAllPartFiles(partnum);
    return;
  }

  for(i = 0; i<n_part_threads; i++){
    pthread_create(partthread_ids+i, NULL, MtLoadAllPartFiles, &partnum);
  }
  for(i=0;i<n_part_threads;i++){
    pthread_join(partthread_ids[i],NULL);
  }
  INIT_PRINT_TIMER(part_timer);
  if(partnum<0){
    for(i = 0; i<npartinfo; i++){
      partdata *parti;

      parti = partinfo+i;
      parti->finalize=0;
    }
    for(i = npartinfo-1; i>=0; i--){
      partdata *parti;

      parti = partinfo+i;
      if(parti->loaded==1){
        parti->finalize = 1;
        FinalizePartLoad(parti);
        break;
      }
    }
  }
  else{
    FinalizePartLoad(partinfo+partnum);
  }
  PRINT_TIMER(part_timer, "finalize particle time");
}
#else
void LoadAllPartFilesMT(int partnum){
  LoadAllPartFiles(partnum);
}
#endif

FILE_SIZE LoadSlicei(int set_slicecolor, int value, int time_frame, float *time_value);

/* ------------------ LoadAllMSlicesMT ------------------------ */

FILE_SIZE LoadAllMSlicesMT(int last_slice, multislicedata *mslicei, int *fcount){
  FILE_SIZE file_size = 0;
  int file_count = 0;
  int i;
  //  slicethreaddata slicethreadinfo[MAX_THREADS];

  file_count = 0;
  file_size = 0;
  for(i = 0; i < mslicei->nslices; i++){
    slicedata *slicei;
    int set_slicecolor;

    slicei = sliceinfo + mslicei->islices[i];
    set_slicecolor = DEFER_SLICECOLOR;

    slicei->finalize = 0;
    if(last_slice == mslicei->islices[i]){
      slicei->finalize = 1;
      set_slicecolor = SET_SLICECOLOR;
      }
    if(slicei->skipdup == 0 && last_slice != mslicei->islices[i]){
      file_size += LoadSlicei(set_slicecolor, mslicei->islices[i], ALL_FRAMES, NULL);
      file_count++;
      }
    }
  file_size += LoadSlicei(SET_SLICECOLOR, last_slice, ALL_FRAMES, NULL);
  file_count++;
  *fcount = file_count;
  return file_size;
  }

#ifdef pp_THREAD
/* ------------------ MtReadBufferi ------------------------ */

void *MtReadBufferi(void *arg){
  ReadBufferi(arg);
  pthread_exit(NULL);
  return NULL;
}
#endif

//***************************** multi threading triangle update ***********************************

#ifdef pp_THREAD

/* ------------------ CancelUpdateTriangles ------------------------ */

void CancelUpdateTriangles(void){
  cancel_update_triangles = 1;
  THREADcontrol(triangles_threads, THREAD_JOIN);
  cancel_update_triangles = 0;
}
#endif

/* ------------------ MtGetGlobalSliceBounds ------------------------ */

#ifdef pp_THREAD
void *MtGetGlobalSliceBounds(void *arg){
  GetGlobalSliceBoundsFull();
  pthread_exit(NULL);
  return NULL;
}

void GetGlobalSliceBoundsMT(void){
  if(use_slicebounds_threads == 1){
    pthread_create(&SLICEBOUNDS_thread_id, NULL, MtGetGlobalSliceBounds, NULL);
  }
  else{
    GetGlobalSliceBoundsFull();
  }
}
#else
void GetGlobalSliceBoundsMT(void){
  GetGlobalSliceBounds();
}
#endif

/* ------------------ MtGetGlobalPatchBounds ------------------------ */

#ifdef pp_THREAD
void *MtGetGlobalPatchBounds(void *arg){
  GetGlobalPatchBoundsFull();
  pthread_exit(NULL);
  return NULL;
}

void GetGlobalPatchBoundsMT(void){
  if(use_patchbounds_threads == 1){
    pthread_create(&PATCHBOUNDS_thread_id, NULL, MtGetGlobalPatchBounds, NULL);
  }
  else{
    GetGlobalPatchBoundsFull();
  }
}
#else
void GetGlobalPatchBoundsMT(void){
  GetGlobalPatchBounds();
}
#endif

/* ------------------ MtReadVolsmokeAllFramesAllMeshes2 ------------------------ */

#ifdef pp_THREAD
void MtReadVolsmokeAllFramesAllMeshes2(void){
  pthread_create(&read_volsmoke_id,NULL,ReadVolsmokeAllFramesAllMeshes2,NULL);
}
#endif

// -------------------- old threader routines above
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// -------------------- new threader routines below


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
  if(thi->n_threads_ptr!=NULL)thi->n_threads = MIN(*(thi->n_threads_ptr),MAX_THREADS);
  if(thi->use_threads == 1){
    int i;

    for(i = 0; i < thi->n_threads; i++){
      pthread_create(thi->thread_ids + i, NULL, thi->run, NULL);
    }
  }
  else{
   thi->run(arg);
  }
}
