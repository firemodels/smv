#ifndef SMV_ENDIAN_H_DEFINED
#define SMV_ENDIAN_H_DEFINED
int GetEndian(void);
float FloatSwitch(float val);
int IntSwitch(int val);
void EndianSwitch(void *val, int nval);
#endif

