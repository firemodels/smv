#ifndef SMV_ENDIAN_H_DEFINED
#define SMV_ENDIAN_H_DEFINED

#define ENDIAN_BIG    0
#define ENDIAN_LITTLE 1

int GetEndian(void);
float FloatSwitch(float val);
double DoubleSwitch(double val);
short ShortSwitch(short val);
int IntSwitch(int val);
void EndianSwitch(void *val, int nval);
#endif

