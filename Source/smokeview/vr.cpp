#define CPP
#include <stdio.h>
#include "options.h"
#ifdef pp_OPENVR
#include "vr.h"
//#include <SDL.h>
//#include <SDL_opengl.h>
#include <openvr.h>
vr::IVRSystem *m_pHMD=NULL;

/* ----------------------- ShutdownVR ----------------------------- */

extern "C" void ShutdownVR(void){
  if(m_pHMD!=NULL){
    vr::VR_Shutdown();
    m_pHMD = NULL;
  }
}

/* ----------------------- InitVR ----------------------------- */

extern "C" int InitVR(void){

 // if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)<0){
 //   printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
 //   return 0;
 // }

  vr::EVRInitError eError = vr::VRInitError_None;

  m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
  if(eError!=vr::VRInitError_None){
    m_pHMD = NULL;
    printf("Unable to initialize VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
    return 0;
  }
  return 1;
}

/* ----------------------- HaveVR ----------------------------- */

extern "C" int HaveVR(void){
  if (vr::VR_IsHmdPresent())return 1;
  return 0;
}
#endif
