#define CPP
#include <stdio.h>
#include "options.h"
#ifdef pp_OPENVR
#include "vr.h"
//#include <SDL.h>
//#include <SDL_opengl.h>
#include <openvr.h>
vr::IVRSystem *m_pHMD=NULL;
vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];

#define IJ(i,j) (4*(i)+(j))
#define IJ2(i,j) (4*(i)+(j)+4)

/* ----------------------- Mat4Mult ----------------------------- */

void Mat4Mult(float *matmult, float *a, float *b){
  int i, j, k;
  float mm[16];

  for(i = 0; i<4; i++){
    for(j = 0; j<4; j++){
      float prod;
      int k;

      prod = 0.0;
      for(k = 0; k<4; k++){
        prod += a[IJ(i, k)]*b[IJ(k, j)];
      }
      mm[IJ(i, j)] = prod;
    }
  }
  for(i = 0; i<16; i++){
    matmult[i] = mm[i];
  }
}

  /* ----------------------- ElimRows ----------------------------- */

void ElimRows(float *mat, int i1){
  int i,j;
  float factor;

  factor = mat[IJ(i1, i1)];
  for(j = 0; j<8; j++){
    mat[IJ(i1, j)] /= factor;
  }
  for(i = 0; i<4; i++){
    int j;
    float factor;

    if(i1==i)continue;
    factor = mat[IJ(i, i1)];
    for(j = i1; j<8; j++){
      mat[IJ(i, j)] = -factor*mat[IJ(i1, j)]+mat[IJ(i, j)];
    }
  }
}

  /* ----------------------- SwapRow ----------------------------- */

void SwapRow(float *mat, int i1, int i2){
  int j;

  if(i1==i2)return;
  for(j = 0; j<8; j++){
    float val1, val2;
    int i1val, i2val;

    i1val = IJ(i1, j);
    i2val = IJ(i2, j);

    val1 = mat[i1val];
    val2 = mat[i2val];
    mat[i1val] = val2;
    mat[i2val] = val1;
  }
}

/* ----------------------- GetMaxVal ----------------------------- */

int GetMaxVal(float *mat, int istart){
  int i, maxind;
  float maxval;

  maxind = istart;
  maxval = mat[IJ(istart, istart)];
  if(maxval<0.0)maxval *= -1.0;
  for(i = istart+1; i<4; i++){
    float val;

    val = mat[IJ(i, istart)];
    if(val<0.0)val *= -1.0;
    if(val>maxval){
      maxind = i;
      maxval = val;
    }
  }
  return maxind;
}

/* ----------------------- GetMat4Inv ----------------------------- */

void GetMat4Inv(float *mat4, float *mat4inv){
  float mm[32];
  int i,j;

  // form augmented matrix

  for(i = 0; i<4; i++){
    for(j = 0; j<4; j++){
      mm[IJ(i,j)] = mat4[IJ(i, j)];
      mm[IJ2(i, j)] = 0.0;
    }
  }
  for(i = 0; i<4; i++){
    mm[IJ2(i, i)] = 1.0;
  }

  for(j = 0; j<4; j++){
    int maxind;

    maxind = GetMaxVal(mm, j);
    SwapRow(mm, maxind, j);
    ElimRows(mm, j);
  }
  for(i = 0; i<4; i++){
    for(j = 0; j<4; j++){
      mat4inv[IJ(i, j)] = mm[IJ2(i, j)];
    }
  }
}

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

/* ----------------------- GetProjectionMatrix ----------------------------- */

extern "C" void GetProjectionMatrix(int which_eye, float *proj){
  int i, j;
  float near_clip = 0.001, far_clip = 100.0;
  vr::Hmd_Eye nEye;

  if(which_eye==0){
    nEye = vr::Eye_Left;
  }
  else{
    nEye = vr::Eye_Right;
  }
  vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, near_clip,far_clip);

  for(i = 0; i<4; i++){
    int j;

    for(j = 0; j<4; j++){
      proj[4*i+j] = mat.m[i][j];
    }
  }
}

/* ----------------------- GetEyePosMatrix ----------------------------- */

void GetEyePosMatrix(int which_eye, float *eyepos_inv){
  vr::Hmd_Eye nEye;
  float eyepos[16];
  int i;

  if(which_eye==0){
    nEye = vr::Eye_Left;
  }
  else{
    nEye = vr::Eye_Right;
  }

  vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
  for(i = 0; i<3; i++){
    int j;

    for(j = 0; j<4; j++){
      eyepos[4*i+j] = matEyeRight.m[i][j];
    }
  }
  for(i = 0; i<3; i++){
    eyepos[4*3+i] = 0.0;
  }
  eyepos[4*3+3] = 1.0;
  GetMat4Inv(eyepos, eyepos_inv);
}

/* ----------------------- UpdateHMDMatrixPose ----------------------------- */

void UpdateHMDMatrixPose(void){
  vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  if(m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)  {
//    m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
//    m_mat4HMDPose.invert();
  }

}
#endif
