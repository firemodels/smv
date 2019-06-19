#include <stdio.h>
#include "options.h"
#ifdef pp_OPENVR
#include "vr.h"
//#include <SDL.h>
//#include <SDL_opengl.h>
#include <openvr.h>

int InitVR(void){
  vr::IVRSystem *m_pHMD;

 // if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)<0){
 //   printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
 //   return 0;
 // }
  vr::EVRInitError eError = vr::VRInitError_None;
  m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
  return 1;
}
#endif
