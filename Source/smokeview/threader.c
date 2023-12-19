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

  if(part_multithread==0){
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

/* ------------------ MtUpdateTriangles ------------------------ */

#ifdef pp_THREAD
void *MtUpdateTriangles(void *arg){
  UpdateTriangles(GEOM_DYNAMIC,GEOM_UPDATE_ALL);
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ UpdateTrianglesMT ------------------------ */

void UpdateTrianglesMT(void){
  if(iso_multithread==1){
    pthread_create(&triangles_id, NULL, MtUpdateTriangles, NULL);
  }
  else{
    UpdateTriangles(GEOM_DYNAMIC, GEOM_UPDATE_ALL);
  }
}

/* ------------------ FinishUpdateTriangles ------------------------ */

void FinishUpdateTriangles(void){
  if(iso_multithread==1)pthread_join(triangles_id, NULL);
}

/* ------------------ CancelUpdateTriangles ------------------------ */

void CancelUpdateTriangles(void){
  cancel_update_triangles = 1;
  FinishUpdateTriangles();
  cancel_update_triangles = 0;
}

#else

/* ------------------ UpdateTrianglesMT ------------------------ */

void UpdateTrianglesMT(void){
  UpdateTriangles(GEOM_DYNAMIC,GEOM_UPDATE_ALL);
}

/* ------------------ CancelUpdateTriangles ------------------------ */

void CancelUpdateTriangles(void){
}

/* ------------------ FinishUpdateTriangles ------------------------ */

void FinishUpdateTriangles(void){
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
  if(slicebounds_thread == 1){
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
  if(patchbounds_thread == 1){
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

//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// -------------------- new threader routines


/* ------------------ Sample ------------------------ */

#ifdef pp_SAMPLE
// example multi threading routines
// need to declare sample_thread_id in threader.h
// need to declare sample_multithread in smokeviewvars.h

/* ------------------ Sample ------------------------ */

void Sample(void){
}

/* ------------------ MtSample ------------------------ */

void *MtSample(void *arg){
  Sample();
  pthread_exit(NULL);
  return NULL;
}
if(threader_sample==NULL){
  threader_sample = THREADinit(1,1,Sample,MtSample);
}
THREADrun(threader_sample);
#endif

#ifdef pp_THREAD_NEW

void *MtCheckFiles(void *arg){
  CheckFiles();
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ MtClassifyAllGeom ------------------------ */

void *MtClassifyAllGeom(void *arg){
  ClassifyAllGeom();
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ MtReadAllGeom ------------------------ */

void *MtReadAllGeom(void *arg){
  ReadAllGeom();
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ MtCompress ------------------------ */

void *MtCompress(void *arg){
  THREADcontrol(compress_threads, THREAD_LOCK);
  Compress();
  updatemenu=1;
  THREADcontrol(compress_threads, THREAD_UNLOCK);
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ MtPlayMovie ------------------------ */

void *MtPlayMovie(void *arg){
  PlayMovie();
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ MtSetupFF ------------------------ */

void *MtSetupFF(void *arg){
  SetupFF();
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ THREADinit ------------------------ */

threaderdata *THREADinit(int nthreads_arg, int threading_on_arg,
  void (*run_arg)(void), void *(*mtrun_arg)(void *arg)){
  threaderdata *thi;

  //create two routines
    // void run(void){
    // }
    // void *mtrun(void *arg){
    //   run();
    //   pthread_exit(NULL);
    //   return NULL;
    // }

  if(nthreads_arg < 0)nthreads_arg = 0;
  if(threading_on_arg != 1)threading_on_arg = 0;
  if(nthreads_arg == 0)threading_on_arg = 0;
  NewMemory(( void ** )&thi, sizeof(threaderdata));
  thi->nthreads     = nthreads_arg;
  thi->threading_on = threading_on_arg;
  thi->run          = run_arg;
  thi->mtrun        = mtrun_arg;
  NewMemory(( void ** )&thi->thread_ids, nthreads_arg*sizeof(pthread_t));
  pthread_mutex_init(&thi->mutex, NULL);
  return thi;
}

/* ------------------ THREADcontrol ------------------------ */

void THREADcontrol(threaderdata *thi, int var){
  if(thi == NULL)return;
  switch(var){
  case THREAD_LOCK:
    if(thi->threading_on == 1)pthread_mutex_lock(&thi->mutex);
    break;
  case THREAD_UNLOCK:
    if(thi->threading_on == 1)pthread_mutex_unlock(&thi->mutex);
    break;
  case THREAD_JOIN:
    if(thi->threading_on == 1){
      int i;

      for(i = 0;i < thi->nthreads;i++){
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

void THREADrun(threaderdata *thi){
  if(thi->threading_on == 1){
    int i;

    for(i = 0; i < thi->nthreads; i++){
      pthread_create(thi->thread_ids + i, NULL, thi->mtrun, NULL);
    }
  }
  else{
   thi->run();
  }
}
#endif
