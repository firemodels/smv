#ifndef SMOKEFORTHEADERS_H_DEFINED
#define SMOKEFORTHEADERS_H_DEFINED

#ifdef WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

#define HEADER_SIZE 4
#define TRAILER_SIZE 4

#define FORTREAD(var,count,STREAM) \
                           FSEEK(STREAM,HEADER_SIZE,SEEK_CUR);\
                           returncode=fread(var,4,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           FSEEK(STREAM,TRAILER_SIZE,SEEK_CUR)
#endif
