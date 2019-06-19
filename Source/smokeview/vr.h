#ifndef VR_H_DEFINED
#define VR_H_DEFINED
#ifdef IN_VR
#define VREXTERN
#else
#define VREXTERN extern CCC
#endif

VREXTERN int InitVR(void);
#endif



