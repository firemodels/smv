#include "options.h"
#include "smv_endian.h"

/* ------------------ GetEndian ------------------------ */

int GetEndian(void){
  short int word = 0x0001;
  char *b = (char *)&word;
  return (b[0] ? ENDIAN_LITTLE : ENDIAN_BIG);
}

/* ------------------ FloatSwitch ------------------------ */

float FloatSwitch(float val){
  float *val2ptr;
  unsigned char *buffer;
  unsigned char buffer2[4];

  buffer=(unsigned char *)&val;

  buffer2[0]=*(buffer+3);
  buffer2[1]=*(buffer+2);
  buffer2[2]=*(buffer+1);
  buffer2[3]=*buffer;

  val2ptr=(float *)buffer2;
  return *val2ptr;
}

/* ------------------ IntSwitch ------------------------ */

int IntSwitch(int val){
  int *val2ptr;
  unsigned char *buffer;
  unsigned char buffer2[4];

  buffer=(unsigned char *)&val;

  buffer2[0]=*(buffer+3);
  buffer2[1]=*(buffer+2);
  buffer2[2]=*(buffer+1);
  buffer2[3]=*buffer;

  val2ptr=(int *)buffer2;
  return *val2ptr;
}

/* ------------------ EndianSwitch ------------------------ */

void EndianSwitch(void *val, int nval){
  int i;

  for(i=0;i<nval;i++){
    int ii;
    unsigned char *ca, *cb, *cc, *cd;
    unsigned char c1, c2, c3, c4;

    ii=4*i;
    ca=(unsigned char *)val+ii;
    cb=(unsigned char *)val+ii+1;
    cc=(unsigned char *)val+ii+2;
    cd=(unsigned char *)val+ii+3;
    c1=*ca;
    c2=*cb;
    c3=*cc;
    c4=*cd;
    *ca=c4;
    *cb=c3;
    *cc=c2;
    *cd=c1;
  }
}
