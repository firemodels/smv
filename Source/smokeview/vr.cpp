#define CPP
#include <stdio.h>
#include "options.h"
#ifdef pp_OPENVR
#include "vr.h"
//#include <SDL.h>
//#include <SDL_opengl.h>
#include "Matrices.h"
#include <openvr.h>

vr::IVRSystem *m_pHMD=NULL;
vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
int m_iValidPoseCount;
Matrix4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose );
std::string m_strPoseClasses;                            // what classes we saw poses for this frame
char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
Matrix4 m_mat4HMDPose;
float m_fNearClip=0.1;
float m_fFarClip=100.0;
Matrix4 m_mat4ProjectionLeft;
Matrix4 m_mat4ProjectionRight;
Matrix4 m_mat4eyePosLeft;
Matrix4 m_mat4eyePosRight;
Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
#define EYE_LEFT  0
#define EYE_RIGHT 1

/* ----------------------- ShutdownVR ----------------------------- */

extern "C" void ShutdownVR(void){
  if(m_pHMD!=NULL){
    vr::VR_Shutdown();
    m_pHMD = NULL;
  }
}

/* ----------------------- GetHMDMatrixProjectionEye ----------------------------- */

Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye){
  if(!m_pHMD)return Matrix4();

  vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

  return Matrix4(
    mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
    mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
    mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
    mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
  );
}

/* ----------------------- GetHMDMatrixPoseEye ----------------------------- */

Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye){
  if(!m_pHMD)return Matrix4();

  vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
  Matrix4 matrixObj(
    matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
    matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
    matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
    matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
  );

  return matrixObj.invert();
}

/* ----------------------- SetupCameras ----------------------------- */

void SetupCameras(){
  m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
  m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
  m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
  m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
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
  SetupCameras();
  return 1;
}

/* ----------------------- HaveVR ----------------------------- */

extern "C" int HaveVR(void){
  if (vr::VR_IsHmdPresent())return 1;
  return 0;
}

/* ----------------------- ConvertSteamVRMatrixToMatrix4 ----------------------------- */

Matrix4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose ){
  Matrix4 matrixObj(
  matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
  matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
  matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
  matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
  );
  return matrixObj;
}

/* ----------------------- UpdateHMDMatrixPose ----------------------------- */

void UpdateHMDMatrixPose(void){
  vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  m_iValidPoseCount = 0;
  m_strPoseClasses = "";
  for(int nDevice = 0; nDevice<vr::k_unMaxTrackedDeviceCount; ++nDevice){
    if(m_rTrackedDevicePose[nDevice].bPoseIsValid){
      m_iValidPoseCount++;
      m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
      if(m_rDevClassChar[nDevice]==0){
        switch(m_pHMD->GetTrackedDeviceClass(nDevice)){
        case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
        case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
        case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
        case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
        case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
        default:                                       m_rDevClassChar[nDevice] = '?'; break;
        }
      }
      m_strPoseClasses += m_rDevClassChar[nDevice];
    }
  }

  if(m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid){
    m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
    m_mat4HMDPose.invert();
  }
}

/* ----------------------- GetCurrentViewProjectionMatrix ----------------------------- */

void GetCurrentViewProjectionMatrix( int which_eye,  float *m_return){
  Matrix4 matMVP;
  float *m;
  int i;

  UpdateHMDMatrixPose();
  if( which_eye == EYE_LEFT ){
    matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
  }
  else if( which_eye == EYE_RIGHT ){
    matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
  }
  m = (float *)matMVP.get();
  for(i = 0; i<16; i++){
    m_return[i] = m[i];
  }
}
#endif
