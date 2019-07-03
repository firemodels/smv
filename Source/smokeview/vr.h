#ifndef VR_H_DEFINED
#define VR_H_DEFINED

EXTERNCPP void ShutdownVR(void);
EXTERNCPP int InitVR(void);
EXTERNCPP int HaveVR(void);
EXTERNCPP void GetCurrentViewProjectionMatrix(int which_eye, float *m);

#endif


