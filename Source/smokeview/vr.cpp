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
float m_fNearClip;
float m_fFarClip;
Matrix4 m_mat4ProjectionLeft;
Matrix4 m_mat4ProjectionRight;
Matrix4 m_mat4eyePosLeft;
Matrix4 m_mat4eyePosRight;

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

void GetTranspose(float *m_out, float *m_in){
  m_out[0]  =  m_in[0];
  m_out[1]  =  m_in[4];
  m_out[2]  =  m_in[8];
  m_out[3]  = m_in[12];
  m_out[4]  =  m_in[1];
  m_out[5]  =  m_in[5];
  m_out[6]  =  m_in[9];
  m_out[7]  = m_in[13];
  m_out[8]  =  m_in[2];
  m_out[9]  =  m_in[6];
  m_out[10] = m_in[10];
  m_out[11] = m_in[14];
  m_out[12] =  m_in[3];
  m_out[13] =  m_in[7];
  m_out[14] = m_in[11];
  m_out[15] = m_in[15];
}

Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

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

/* ----------------------- GetHMDMatrixProjectionEye ----------------------------- */

Matrix4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye ){
  if ( !m_pHMD )return Matrix4();

  vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip );

  return Matrix4(
    mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
    mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
    mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
    mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
  );
}

/* ----------------------- GetHMDMatrixPoseEye ----------------------------- */

Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye ){
  if ( !m_pHMD )return Matrix4();

  vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
  Matrix4 matrixObj(
    matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
    matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
    matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
    matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
    );

  return matrixObj.invert();
}

/* ----------------------- GetCurrentViewProjectionMatrix ----------------------------- */

Matrix4 GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye ){
  Matrix4 matMVP;

  if( nEye == vr::Eye_Left ){
    matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
  }
  else if( nEye == vr::Eye_Right ){
    matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
  }
  return matMVP;
}

/* ----------------------- GetPoseMatrix ----------------------------- */

void UpdateHMDMatrixPose(void){
  vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

  m_iValidPoseCount = 0;
  m_strPoseClasses = "";
  for ( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice ){
    if ( m_rTrackedDevicePose[nDevice].bPoseIsValid ){
      m_iValidPoseCount++;
      m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
      if (m_rDevClassChar[nDevice]==0){
        switch (m_pHMD->GetTrackedDeviceClass(nDevice)){
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

  if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid ){
    m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
    m_mat4HMDPose.invert();
  }
}
#endif
