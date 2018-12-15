#define INTHREADER
#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"

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

/* ------------------ InitMultiThreading ------------------------ */

void InitMultiThreading(void){
#ifdef pp_THREAD
  pthread_mutex_init(&mutexCOMPRESS,NULL);
#ifdef pp_ISOTHREAD
  pthread_mutex_init(&mutexTRIANGLES,NULL);
#endif
  pthread_mutex_init(&mutexVOLLOAD,NULL);
#ifdef pp_THREADIBLANK
  pthread_mutex_init(&mutexIBLANK, NULL);
#endif
#endif
}

// *************** multi-threaded compression ****************

#ifdef pp_THREAD
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

// ************** multi threaded blank creation **********************

/* ------------------ MtMakeIBlank ------------------------ */
#ifdef pp_THREAD
#ifdef pp_THREADIBLANK
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
#endif

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
  system(commandline)
}
#endif

/* ------------------ makeiblank_all ------------------------ */

#ifdef pp_THREAD
#ifdef pp_THREADIBLANK
void MakeIBlankAll(void){
  pthread_create(&makeiblank_thread_id, NULL, MtMakeIBlank, NULL);
}
#else
void MakeIBlankAll(void){
  MakeIBlank();
  SetCVentDirs();
  update_setvents=1;
}
#endif
#else
void MakeIBlankAll(void){
  MakeIBlank();
  SetCVentDirs();
  update_set_vents=1;
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
