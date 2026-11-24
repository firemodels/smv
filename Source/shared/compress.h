#ifndef COMPRESS_H_DEFINED
#define COMPRESS_H_DEFINED
#ifndef DEF_ZLIB
#define DEF_ZLIB
#include <zlib.h>
#endif
int CompressZLIB(unsigned char *dest, uLongf *destLen, unsigned char *source, int sourceLen);
int UnCompressZLIB(unsigned char *dest, uLongf *destLen, unsigned char *source, int sourceLen);
unsigned int CompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out);
unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out);
unsigned char AllZeroRLE(unsigned char *buffer_in, int nchars_in);
#endif
