#ifndef UPDATE_H_DEFINED
#define UPDATE_H_DEFINED
#ifdef IN_UPDATE
#define UPEXTERN
#else
#define UPEXTERN extern CCC
#endif

#define TERRAIN_FIRE_LINE_UPDATE 39

UPEXTERN void UpdateClipbounds(int set_i0, int *i0, int set_i1, int *i1, int maxi);
UPEXTERN int CompareFloat( const void *arg1, const void *arg2 );
UPEXTERN void UpdateHrrinfo(int val);
UPEXTERN void ResetItimes0(void);
UPEXTERN void UpdateShow(void);
UPEXTERN void SynchTimes(void);
UPEXTERN void UpdateTimes(void);
UPEXTERN int GetPlotState(int choice);
UPEXTERN int GetIndex(float key, const float *list, int nlist);
UPEXTERN int ISearch(float *list, int nlist, float key, int guess);
#endif



