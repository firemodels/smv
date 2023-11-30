#define INTHREADER
#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "smokestream.h"
#include GLUT_H

#ifndef pp_THREAD
#undef pp_CSV_MULTI
#endif

/* ------------------ InitMultiThreading ------------------------ */

void InitMultiThreading(void){
#ifdef pp_THREAD
  pthread_mutex_init(&mutexREADALLGEOM, NULL);
#ifdef pp_SLICE_MULTI
  pthread_mutex_init(&mutexSLICE_LOAD, NULL);
#endif
  pthread_mutex_init(&mutexCSV_LOAD, NULL);
  pthread_mutex_init(&mutexPART_LOAD, NULL);
  pthread_mutex_init(&mutexCOMPRESS,NULL);
  pthread_mutex_init(&mutexVOLLOAD,NULL);
  pthread_mutex_init(&mutexIBLANK, NULL);
  pthread_mutex_init(&mutexSETUP_FFMPEG, NULL);
  pthread_mutex_init(&mutexCHECKFILES, NULL);
  pthread_mutex_init(&mutexSLICEBOUNDS, NULL);
  pthread_mutex_init(&mutexPATCHBOUNDS, NULL);
#endif
}

//***************************** multi-threaded compression ***********************************

/* ------------------ CompressSVZip2 ------------------------ */

void CompressSVZip2(void){
  char shellcommand[1024];

  PRINTF("Compressing...\n");
  GLUICompressOnOff(OFF);

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
  GLUICompressOnOff(ON);
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

#ifdef pp_HIST
  if(part_multithread==1&&current_script_command==NULL&&update_generate_part_histograms==-1){
    JOIN_PART_HIST;
  }
#endif
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

/* ------------------ ReadAllGeomMT ------------------------ */

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

/* ------------------ MTGeneratePartHistograms ------------------------ */

#ifdef pp_HIST
void *MTGeneratePartHistograms(void *arg){
  GeneratePartHistograms();
  pthread_exit(NULL);
  return NULL;
}

void GeneratePartHistogramsMT(void){
  in_part_mt = 1;
  pthread_create(&generate_part_histogram_id, NULL, MTGeneratePartHistograms, NULL);
}
#endif
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

//***************************** multi threading read in csv file ***********************************

/* ------------------ MtReadAllCSVFiles ------------------------ */

#ifdef pp_THREAD
void ReadAllCSVFiles(void);
void *MtReadAllCSVFiles(void *arg){
  ReadAllCSVFiles();
  pthread_exit(NULL);
  return NULL;
}
#endif

/* ------------------ LockCSV ------------------------ */
#ifdef pp_CSV_MULTI
void LockCSV(void){
  LOCK_CSV_LOAD;
}

/* ------------------ UnLockCSV ------------------------ */

void UnLockCSV(void){
  UNLOCK_CSV_LOAD;
}
#endif

/* ------------------ void ReadAllCSVFilesMT ------------------------ */

void ReadAllCSVFilesMT(void){
  StartTimer(&csv_timer);
#ifdef pp_CSV_MULTI
  if(csv_multithread == 1){
    int i;

    for(i=0;i<ncsv_threads;i++){
     pthread_create(csv_ids+i, NULL, MtReadAllCSVFiles, NULL);
    }
  }
  else{
    ReadAllCSVFiles();
  }
#else
  ReadAllCSVFiles();
#endif
}

/* ------------------ void FinishAllCSVFiles ------------------------ */

#ifdef pp_CSV_MULTI
void FinishAllCSVFiles(void){
  int i;

  for(i = 0; i < ncsv_threads; i++){
    pthread_join(csv_ids[i], NULL);
  }
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

/* ------------------ MtSetupFF ------------------------ */

void SetupFF(void){
  int have_ffmpeg_local, have_ffplay_local;

#ifdef WIN32
  have_ffmpeg_local = HaveProg("ffmpeg -version> Nul 2>Nul");
  have_ffplay_local = HaveProg("ffplay -version> Nul 2>Nul");
#else
  have_ffmpeg_local = HaveProg("ffmpeg -version >/dev/null 2>/dev/null");
  have_ffplay_local = HaveProg("ffplay -version >/dev/null 2>/dev/null");
#endif

  update_ff = 1;
  have_ffmpeg = have_ffmpeg_local;
  have_ffplay = have_ffplay_local;
}

/* ------------------ MtSetupFF ------------------------ */

void *MtSetupFF(void *arg){
  int have_ffmpeg_local, have_ffplay_local;

#ifdef WIN32
  have_ffmpeg_local = HaveProg("ffmpeg -version> Nul 2>Nul");
  have_ffplay_local = HaveProg("ffplay -version> Nul 2>Nul");
#else
  have_ffmpeg_local = HaveProg("ffmpeg -version >/dev/null 2>/dev/null");
  have_ffplay_local = HaveProg("ffplay -version >/dev/null 2>/dev/null");
#endif

  LOCK_SETUP_FFMPEG
  update_ff = 1;
  have_ffmpeg = have_ffmpeg_local;
  have_ffplay = have_ffplay_local;
  UNLOCK_SETUP_FFMPEG
#ifdef pp_THREAD
  pthread_exit(NULL);
#endif
  return NULL;
}

/* ------------------ SetupFFMT ------------------------ */

void SetupFFMT(void){
#ifdef pp_THREAD
  if(ffmpeg_multithread == 1){
    pthread_create(&setupff_thread_id, NULL, MtSetupFF, NULL);
  }
  else{
    SetupFF();
  }
#else
  SetupFF();
#endif
}

#ifndef pp_CHECK
/* ------------------ CheckFiles ------------------------ */

void CheckFiles(void){
  int i;

  LOCK_CHECKFILES;
  have_compressed_files = 0;
  UNLOCK_CHECKFILES;
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;
    int have_file;

    patchi = patchinfo + i;
    have_file = FILE_EXISTS_CASEDIR(patchi->comp_file);
    LOCK_CHECKFILES;
    if(have_file==YES){
      patchi->compression_type_temp = COMPRESSED_ZLIB;
      have_compressed_files = 1;
    }
    UNLOCK_CHECKFILES;
  }
  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int have_file;

    smoke3di = smoke3dinfo + i;
    have_file = FILE_EXISTS_CASEDIR(smoke3di->comp_file);
    LOCK_CHECKFILES;
    if(have_file==YES){
      smoke3di->compression_type_temp = COMPRESSED_ZLIB;
      have_compressed_files = 1;
    }
    UNLOCK_CHECKFILES;
  }
  if(have_compressed_files==0)return;
  LOCK_CHECKFILES;
  for(i = 0; i < npatchinfo; i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->compression_type_temp==COMPRESSED_ZLIB){
      patchi->compression_type = COMPRESSED_ZLIB;
      patchi->file             = patchi->comp_file;
    }
  }
  for(i = 0; i < nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->compression_type_temp==COMPRESSED_ZLIB){
      smoke3di->file             = smoke3di->comp_file;
      smoke3di->is_zlib          = 1;
      smoke3di->compression_type = COMPRESSED_ZLIB;
    }
  }
  updatemenu = 1;
  UNLOCK_CHECKFILES;
}

/* ------------------ MtCheckFiles ------------------------ */

#ifdef pp_THREAD
void *MtCheckFiles(void *arg){
  CheckFiles();
  pthread_exit(NULL);
  return NULL;
}

void CheckFilesMT(void){
  if(checkfiles_multithread==1){
    pthread_create(&checkfiles_multithread_id, NULL, MtCheckFiles, NULL);
  }
  else{
    CheckFiles();
  }
}
#else
void CheckFilesMT(void){
  CheckFiles();
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

//***************************** multi threaded blank creation ***********************************

#ifdef pp_THREAD
/* ------------------ MtMakeIBlank ------------------------ */

void *MtMakeIBlank(void *arg){
  MakeIBlank();
  SetCVentDirs();
  LOCK_IBLANK
  update_setvents = 1;
  UNLOCK_IBLANK
  pthread_exit(NULL);
  return NULL;
}

/* ------------------ makeiblank_all ------------------------ */

void MakeIBlankAllMT(void){
  if(iblank_multithread == 1){
    pthread_create(&makeiblank_thread_id, NULL, MtMakeIBlank, NULL);
  }
  else{
    MakeIBlank();
    SetCVentDirs();
    update_setvents = 1;
  }
}
#else
void MakeIBlankAllMT(void){
  MakeIBlank();
  SetCVentDirs();
  update_setvents = 1;
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

#ifdef pp_HIST
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
#endif

/* ------------------ MtReadVolsmokeAllFramesAllMeshes2 ------------------------ */

#ifdef pp_THREAD
void MtReadVolsmokeAllFramesAllMeshes2(void){
  pthread_create(&read_volsmoke_id,NULL,ReadVolsmokeAllFramesAllMeshes2,NULL);
}
#endif
