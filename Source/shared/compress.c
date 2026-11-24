#include "options_common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dmalloc.h"
#include "compress.h"

#define MARK 255

/* ------------------ CompressZLIB ------------------------ */

int CompressZLIB(unsigned char *dest, uLongf *destLen, unsigned char *source, int sourceLen){
  return compress(dest, destLen, source, sourceLen);
}

/* ------------------ UnCompressZLIB ------------------------ */

int UnCompressZLIB(unsigned char *dest, uLongf *destLen, unsigned char *source, int sourceLen){
  return uncompress(dest, destLen, source, sourceLen);
}

/* ------------------ CompressRLE ------------------------ */

unsigned int CompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out){
  unsigned char lastchar=MARK, cmark=MARK, thischar, *buffer_start;
  unsigned char *buffer_in_end;
  int nrepeats=1;

  buffer_start=buffer_out;
  buffer_in_end = buffer_in + nchars_in;

  while(buffer_in<buffer_in_end){
    thischar=*buffer_in;
    if(thischar==lastchar){
      nrepeats++;
    }
    else{
      nrepeats=1;
    }
    switch(nrepeats){
    case 1:
    case 2:
    case 3:
      *buffer_out=thischar;
      lastchar=thischar;
      break;
    default:
      if(nrepeats==4){
        buffer_out-=3;
        *buffer_out++=cmark;
        *buffer_out++=thischar;
      }
      if(nrepeats!=4)buffer_out--;
      *buffer_out=nrepeats;
      if(nrepeats==254){
        nrepeats=1;
        lastchar=MARK;
      }
      break;
    }
    buffer_in++;
    buffer_out++;

  }
  return buffer_out-buffer_start;
}

/* ------------------ UnCompressRLE ------------------------ */

unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out){
  int nrepeats,nn;
  unsigned char thischar, *buffer_in_end;

  nn=0;
  buffer_in_end  = buffer_in  + nchars_in;

  while(buffer_in<buffer_in_end){
    if(*buffer_in==MARK){
      if(buffer_in+2>=buffer_in_end)break;
      buffer_in++;
      thischar=*buffer_in++;
      nrepeats=*buffer_in++;
      nn+=nrepeats;
      memset(buffer_out,thischar,nrepeats);
      buffer_out+=nrepeats;
    }
    else{
      *buffer_out++=*buffer_in++;
      nn++;
    }

  }
  return nn;
}

/* ------------------ AllZeroRLE ------------------------ */

unsigned char AllZeroRLE(unsigned char *buffer_in, int nchars_in){
  unsigned char *buffer_in_end;

  buffer_in_end = buffer_in + nchars_in;
  while(buffer_in < buffer_in_end){
    if(*buffer_in == MARK){
      unsigned char thischar;

      if(buffer_in + 2 >= buffer_in_end)break;
      thischar = buffer_in[1];
      if(thischar > 0)return 0;
      buffer_in+=3;
    }
    else{
      if(*buffer_in > 0)return 0;
      buffer_in++;
    }
  }
  return 1;
}
