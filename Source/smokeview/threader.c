#define INTHREADER
#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "smokestream.h"

/* ------------------ InitMultiThreading ------------------------ */

void InitMultiThreading(void){
#ifdef pp_THREAD
  pthread_mutex_init(&mutexREADALLGEOM, NULL);
#ifdef pp_SLICE_MULTI
  pthread_mutex_init(&mutexSLICE_LOAD, NULL);
#endif
  pthread_mutex_init(&mutexPART_LOAD, NULL);
  pthread_mutex_init(&mutexCOMPRESS,NULL);
  pthread_mutex_init(&mutexVOLLOAD,NULL);
  pthread_mutex_init(&mutexIBLANK, NULL);
#ifdef pp_STREAM
  pthread_mutex_init(&mutexSTREAM, NULL);
#endif
#endif
}

//***************************** multi-threaded compression ***********************************

/* ------------------ CompressSVZip2 ------------------------ */

void CompressSVZip2(void){
  char shellcommand[1024];

  PRINTF("Compressing...\n");
  CompressOnOff(OFF);

  WriteIni(LOCAL_INI, NULL);

  // surround smokezip path name with "'s so that the system call can handle embedded blanks

  strcpy(shellcommand, "\"");
  strcat(shellcommand, smokezippath);
  strcat(shellcommand, "\" ");
  if(overwrite_all == 1){
    strcat(shellcommand, " -f ");
  }
  if(erase_all == 1){
    strcat(shellcommand, " -c ");
  }
  if(compress_autoloaded == 1){
    strcat(shellcommand, " -auto ");
  }
  strcat(shellcommand, " ");
  strcat(shellcommand, smv_filename);

  PRINTF("Executing shell command: %s\n", shellcommand);
  system(shellcommand);
  UpdateSmoke3dMenuLabels();
  UpdateBoundaryMenuLabels();
  CompressOnOff(ON);
  updatemenu = 1;
  PRINTF("Compression completed\n");
}

#ifdef pp_THREAD
/* ------------------ LockUnlockCompress ------------------------ */

void LockUnlockCompress(int flag){
  if(flag==1){
    LOCK_COMPRESS;
  }
  else{
    UNLOCK_COMPRESS;
  }
}
 /* ------------------ MtCompressSVZip ------------------------ */

void *MtCompressSVZip(void *arg){
  LOCK_COMPRESS
  CompressSVZip2();
  updatemenu=1;
  UNLOCK_COMPRESS
  pthread_exit(NULL);
  return NULL;
}
#endif

/* ------------------ CompressSVZip ------------------------ */

#ifdef pp_THREAD
void CompressSVZip(void){
  pthread_create(&compress_thread_id,NULL, MtCompressSVZip,NULL);
}
#else
void CompressSVZip(void){
  CompressSVZip2();
}
#endif

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

  if(part_multithread==1&&current_script_command==NULL&&update_generate_part_histograms==-1){
    JOIN_PART_HIST;
  }
  if(part_multithread==0){
    LoadAllPartFiles(partnum);
    return;
  }

  for(i = 0; i<npartthread_ids; i++){
    pthread_create(partthread_ids+i, NULL, MtLoadAllPartFiles, &partnum);
  }
  for(i=0;i<npartthread_ids;i++){
    pthread_join(partthread_ids[i],NULL);
  }
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
/* ------------------ MtClassifyAllGeom ------------------------ */

void *MtClassifyAllGeom(void *arg){
  ClassifyAllGeom();
  pthread_exit(NULL);
  return NULL;
}

void ClassifyAllGeomMT(void){
  if(readallgeom_multithread==1){
    int i;

    SetupReadAllGeom();
    for(i = 0; i<nreadallgeomthread_ids; i++){
      pthread_create(classifyallgeomthread_ids+i, NULL, MtClassifyAllGeom, NULL);
    }
  }
  else{
    SetupReadAllGeom();
    ClassifyAllGeom();
  }
}

/* ------------------ MtReadAllGeom ------------------------ */

void *MtReadAllGeom(void *arg){
  ReadAllGeom();
  pthread_exit(NULL);
  return NULL;
}

void ReadAllGeomMT(void){
  if(readallgeom_multithread==1){
    int i;

    SetupReadAllGeom();
    for(i = 0; i<nreadallgeomthread_ids; i++){
      pthread_create(readallgeomthread_ids+i, NULL, MtReadAllGeom, NULL);
    }
    for(i = 0; i<nreadallgeomthread_ids; i++){
      pthread_join(readallgeomthread_ids[i], NULL);
    }
  }
  else{
    SetupReadAllGeom();
    ReadAllGeom();
  }
}

/* ------------------ MtReadAllGeom ------------------------ */

void *MTGeneratePartHistograms(void *arg){
  GeneratePartHistograms();
  pthread_exit(NULL);
  return NULL;
}

void GeneratePartHistogramsMT(void){
  pthread_create(&generate_part_histogram_id, NULL, MTGeneratePartHistograms, NULL);
}
#else
void GeneratePartHistogramsMT(void){
  GeneratePartHistograms();
}
void ClassifyAllGeomMT(void){
  SetupReadAllGeom();
  ClassifyAllGeom();
}
void ReadAllGeomMT(void){
  SetupReadAllGeom();
  ReadAllGeom();
}
#endif

#ifdef pp_THREAD
/* ------------------ MtGetAllPartBounds ------------------------ */

void *MtGetAllPartBounds(void *arg){
  GetAllPartBounds();
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ GetAllPartBoundsMT ------------------------ */

void GetAllPartBoundsMT(void){
  if(part_multithread==1){
    int i;

    for(i = 0; i<npartthread_ids; i++){
      pthread_create(partthread_ids+i, NULL, MtGetAllPartBounds, NULL);
    }
    for(i = 0; i<npartthread_ids; i++){
      pthread_join(partthread_ids[i], NULL);
    }
  }
  else{
    GetAllPartBounds();
  }
  MergeAllPartBounds();
}
#else
void GetAllPartBoundsMT(void){
    GetAllPartBounds();
    MergeAllPartBounds();
}
#endif

#ifdef pp_THREAD
/* ------------------ MtReadBufferi ------------------------ */

void *MtReadBufferi(void *arg){
  ReadBufferi(arg);
  pthread_exit(NULL);
  return NULL;
}
#endif

#ifdef pp_THREADBUFFER
/* ------------------ ReadBuffer ------------------------ */

int ReadBuffer(char *filename, int filesize, char *buffer, int nthreads, int use_multithread){
  int i, filesizei, returnval;
  readbufferdata *readbufferinfo;

  returnval = 1;
  filesizei = filesize/nthreads;

  NewMemory((void **)&readbufferinfo, nthreads*sizeof(readbufferdata));
#ifdef pp_THREAD
  if(use_multithread==1&&nthreads>1){
    NewMemory((void **)&readbuffer_ids, nthreads*sizeof(pthread_t));
  }
#endif

  for(i = 0; i<nthreads; i++){
    readbufferdata *readbufferi;
    int start, end;

    start = i*filesizei;
    if(i==nthreads-1){
      end = filesize;
    }
    else{
      end = start+filesizei;
    }
    if(end>filesize)end = filesize;

    readbufferi = readbufferinfo+i;
    readbufferi->buffer = buffer;
    readbufferi->filename = filename;
    readbufferi->start = start;
    readbufferi->size = end-start;
#ifdef pp_THREAD
    if(use_multithread==1&&nthreads>1){
      pthread_create(readbuffer_ids+i, NULL, MtReadBufferi, readbufferi);
    }
    else{
      ReadBufferi(readbufferi);
    }
  }
  if(use_multithread==1&&nthreads>1){
    for(i = 0; i<nthreads; i++){
      pthread_join(readbuffer_ids[i], NULL);
    }
  }
#else
    ReadBufferi(readbufferi);
  }
#endif
  for(i = 0; i<nthreads; i++){
    readbufferdata *readbufferi;

    readbufferi = readbufferinfo+i;
    if(readbufferi->returnval==0){
      returnval = 0;
      break;
    }
  }
  FREEMEMORY(readbufferinfo);
#ifdef pp_THREAD
  if(use_multithread==1&&nthreads>1){
    FREEMEMORY(readbuffer_ids);
  }
#endif
  return returnval;
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

//***************************** multi threaded blank creation ***********************************

/* ------------------ MtMakeIBlank ------------------------ */
#ifdef pp_THREAD
void *MtMakeIBlank(void *arg){

  MakeIBlank();
  SetCVentDirs();
  LOCK_IBLANK
  update_setvents = 1;
  UNLOCK_IBLANK
  pthread_exit(NULL);
  return NULL;
}
#endif

/* ------------------ Sample ------------------------ */

#ifdef pp_SAMPLE
// example multi threading routines
// need to declare sample_thread_id in threader.h
// need to declare sample_multithread in smokeviewvars.h

#ifdef pp_THREAD
void *MtSample(void *arg){
  Sample();
  pthread_exit(NULL);
  return NULL;
}

void SampleMT(void){
  if(sample_multithread==1){
    pthread_create(&sample_thread_id, NULL, MtSample, NULL);
  }
  else{
    Sample();
  }
}
#else
void SampleMT(void){
  Sample();
}
#endif
#endif

/* ------------------ I/O streaming ------------------------ */

#ifdef pp_STREAM

#ifdef pp_THREAD

/* ------------------ MtStreamReadList ------------------------ */

void *MtStreamReadList(void *arg){
  streamlistargdata *streamlist;
  streamdata **streams;
  int nstreams;

  streamlist = (streamlistargdata *)arg;

  streams  = streamlist->streams;
  nstreams = streamlist->nstreams;

  StreamReadList(streams, nstreams);
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ GetThreads ------------------------ */

pthread_t *GetThreads(int nthreads){
  pthread_t *threads = NULL;

  if(nthreads<=0)return NULL;
  NewMemory((void **)&threads, nthreads*sizeof(pthread_t));
  return threads;
}

/* ------------------ StreamReadListMT ------------------------ */

void StreamReadListMT(streamlistargdata *arg, int nthreads){
  pthread_t *threads=NULL;

  if(nthreads>0&&stream_multithread==1)threads = GetThreads(nthreads);
  if(threads!=NULL){
    int i;

    for(i=0;i<nthreads;i++){
      pthread_create(threads+i, NULL, MtStreamReadList, (void *)arg);
    }
  }
  else{
    streamdata **streams;
    int nstreams;

    streams = arg->streams;
    nstreams = arg->nstreams;
    StreamReadList(streams, nstreams);
  }
}
#else
void StreamReadListMT(streamlistargdata *arg){
  streamdata **streams;
  int nstreams;

  streams = arg->streams;
  nstreams = arg->nstreams;
  StreamReadList(streams, nstreams);
}
#endif
#endif

/* ------------------ makeiblank_all ------------------------ */

#ifdef pp_THREAD
void MakeIBlankAll(void){
  pthread_create(&makeiblank_thread_id, NULL, MtMakeIBlank, NULL);
}
#else
void MakeIBlankAll(void){
  MakeIBlank();
  SetCVentDirs();
}
#endif

//***************************** multi threaded system call ***********************************

/* ------------------ MtPSystem ------------------------ */

#ifdef pp_THREAD
void *MtPSystem(void *arg){
  char command_line[1024], moviefile_path[1024];

  if(FILE_EXISTS(GetMovieFilePath(moviefile_path))==YES){
    strcpy(command_line, "ffplay ");
    strcat(command_line, moviefile_path);
#ifdef WIN32
    strcat(command_line, " 2>Nul ");
#else
    strcat(command_line, " 2>/dev/null ");
#endif
    play_movie_now = 0;
    update_playmovie = 1;
    system(command_line);
    play_movie_now = 1;
    update_playmovie = 1;
  }
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ PSystem ------------------------ */

void PSystem(char *commandline){
  pthread_create(&system_thread_id, NULL, MtPSystem, NULL);
}
#else
void PSystem(char *commandline){
  system(commandline);
}
#endif

/* ------------------ Update_Bounds ------------------------ */

int Update_Bounds(void){
  UpdateAllBoundaryBounds();
#ifdef pp_THREAD
  pthread_join(update_all_patch_bounds_id,NULL);
#endif
  return 1;
}

/* ------------------ UpdateAllBoundaryBoundsMT ------------------------ */

#ifdef pp_THREAD
void *UpdateAllBoundaryBoundsMT(void *arg){
  UpdateAllBoundaryBoundsST();
  pthread_exit(NULL);
  return NULL;
}
void UpdateAllBoundaryBounds(void){
  pthread_create(&update_all_patch_bounds_id,NULL, UpdateAllBoundaryBoundsMT,NULL);
}
#else
void UpdateAllBoundaryBounds(void){
  UpdateAllBoundaryBoundsST();
}
#endif

/* ------------------ MtReadVolsmokeAllFramesAllMeshes2 ------------------------ */

#ifdef pp_THREAD
void MtReadVolsmokeAllFramesAllMeshes2(void){
  pthread_create(&read_volsmoke_id,NULL,ReadVolsmokeAllFramesAllMeshes2,NULL);
}
#endif
