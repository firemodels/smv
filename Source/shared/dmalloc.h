#ifndef dmalloc_H_DEFINED
#define dmalloc_H_DEFINED

#ifdef __MINGW32__
#include "options.h"
#endif

#ifdef pp_THREAD
#include <pthread.h>
#endif

#ifndef pp_OSX
#include <malloc.h>
#endif
#include <stddef.h>
#define memGarbage 0xA3

typedef int mallocflag;
typedef char bbyte;

#ifdef CPP
#define MMCCC "C"
#else
#define MMCCC
#endif

#ifdef INDMALLOC
#define MMEXTERN
#else
#define MMEXTERN extern MMCCC
#endif

#define MMsize unsigned long long

typedef struct {
  unsigned char marker;
  void *prev, *next;
  int memory_id;
  size_t size;
} MMdata;

MMEXTERN MMdata MMfirst, MMlast;
MMEXTERN MMdata *MMfirstptr, *MMlastptr;
MMEXTERN MMsize MMtotalmemory,MMmaxmemory;

#define DEBUG_BYTE 0xE1
#define MARKER_BYTE 0xE1
#ifdef pp_MEMDEBUG
  #define sizeofDebugByte 1
#else
  #define sizeofDebugByte 0
#endif

#ifdef pp_THREAD
MMEXTERN pthread_mutex_t mutexMEM;
#define LOCK_MEM           pthread_mutex_lock(&mutexMEM)
#define UNLOCK_MEM         pthread_mutex_unlock(&mutexMEM)
#else
#define LOCK_MEM
#define UNLOCK_MEM
#endif

#ifdef pp_MEMDEBUG
#define NewMemory(f,g)           __NewMemory((f),(g),0,(#f),__FILE__,__LINE__)
#define ResizeMemory(f,g)        __ResizeMemory((f),(g),0,(#f),__FILE__,__LINE__)

#define NewMemoryMemID(f,g,h)    __NewMemory((f),(g),(h),(#f),__FILE__,__LINE__)
#define ResizeMemoryMemID(f,g,h) __ResizeMemory((f),(g),(j),(#f),__FILE__,__LINE__)
#else
#define NewMemory(f,g)           _NewMemory((f),(g),0,(#f),__FILE__,__LINE__)
#define ResizeMemory(f,g)        _ResizeMemory((f),(g),0,(#f),__FILE__,__LINE__)

#define NewMemoryMemID(f,g,h)    _NewMemory((f),(g),(h),(#f),__FILE__,__LINE__)
#define ResizeMemoryMemID(f,g,h) _ResizeMemory((f),(g),(h),(#f),__FILE__,__LINE__)
#endif

#ifndef NEWMEM
#define NEWMEM(a,b) ((a)==NULL ? NewMemory((void **)&a,b): ResizeMemory((void **)&a,b))
#endif

#define NewResizeMemory(f,g) ((f)==NULL ? NewMemory((void **)&f,g) : ResizeMemory((void **)&f,g))

#define NEWMEMORY(f,g)       NewMemory((void **)&(f),(g))
#define RESIZEMEMORY(f,g) ResizeMemory((void **)&(f),(g))

#ifdef pp_memstatus
#ifdef WIN32
void _memorystatus(unsigned int size,unsigned int *availmem);
#define MEMSTATUS(f,g) _memorystatus(f,g)
#else
#define MEMSTATUS(f,g)
#endif
#endif

#ifndef pp_memstatus
#define MEMSTATUS(f,g,h,i)
#endif

#ifdef pp_MEMPRINT
#define PrintMemoryInfo \
{\
  MMdata *thisptr;int n;LINT size;\
  for(n=0,size=0,thisptr = MMfirstptr->next; thisptr->next!=NULL; thisptr = thisptr->next,size+=thisptr->size,n++){}\
  PRINTF("file=%s line=%i \n nblocks=%i size=%llu\n", __FILE__, __LINE__, n, size);\
}
#else
#define PrintMemoryInfo
#endif

#ifdef pp_MEMDEBUG
void getMemusage(MMsize totalmemory,char *MEMlabel);
#ifdef CPP
extern "C" void _CheckMemory(void);
#else
void _CheckMemory(void);
#endif
void _CheckMemoryNOTHREAD(void);
void _CheckMemoryOn(void);
void _CheckMemoryOff(void);
void _PrintAllMemoryInfo(void);
int _CountMemoryBlocks(void);
#define ValidPointer(pv,size) _ValidPointer(pv, size)
#define GETTOTALMEMORY (_GetTotalMemory())
#define CheckMemory _CheckMemory()
#define CheckMemoryNOTHREAD _CheckMemoryNOTHREAD()
#define CheckMemoryOn _CheckMemoryOn()
#define CheckMemoryOff _CheckMemoryOff()
#define PrintAllMemoryInfo _PrintAllMemoryInfo()
#define COUNTMEMORYBLOCKS(f)  (_CountMemoryBlocks()-f)
char *_strcpy(char *s1, const char *s2);
char *_strcat(char *s1, const char *s2);
#define STRCPY(f,g) _strcpy((f),(g))
#define STRCAT(f,g) _strcat((f),(g))
#else
#define ValidPointer(pv,size)
#define GETTOTALMEMORY
#define CheckMemory
#define CheckMemoryOn
#define CheckMemoryOff
#define PrintAllMemoryInfo
#define STRCPY(f,g) strcpy((f),(g))
#define STRCAT(f,g) strcat((f),(g))
#define COUNTMEMORYBLOCKS(f)
#endif

#ifdef pp_MEMDEBUG
typedef struct BLOCKINFO {
  struct BLOCKINFO *pbiNext;
  bbyte *pb;
  size_t size;
  mallocflag  fref;
  char filename[256], varname[256];
  int linenumber;
} blockinfo;

MMEXTERN MMsize _GetTotalMemory(void);
mallocflag CreateBlockInfo(bbyte *pbNew, size_t sizeNew);
void FreeBlockInfo(bbyte *pb);
void UpdateBlockInfo(bbyte *pbOld, bbyte *pbNew, size_t sizeNew);
size_t sizeofBlock(bbyte *pv);
MMEXTERN mallocflag __ResizeMemory(void **ppv, size_t sizeNew, int memory_id, const char *varname, const char *file, int linenumber);
MMEXTERN mallocflag __NewMemory(void **ppv, size_t size, int memory_id, const char *varname, const char *file,int linenumber);
#endif
MMEXTERN mallocflag _ResizeMemory(void **ppv, size_t sizeNew, int memory_id, const char *varname, const char *file,int linenumber);
MMEXTERN mallocflag _NewMemory(void **ppv, size_t size, int memory_id, const char *varname, const char *file,int linenumber);
MMEXTERN void FreeMemory(void *pv);
MMEXTERN mallocflag _ResizeMemoryNOTHREAD(void **ppv, size_t sizeNew, int memory_id);
MMEXTERN mallocflag _NewMemoryNOTHREAD(void **ppv, size_t size, int memory_id);
MMEXTERN void FreeMemoryNOTHREAD(void *pv);
MMEXTERN int memusage(void);
MMEXTERN void SetMemCheck(float memGB);
void initMALLOC(void);
void FreeAllMemory(int memory_id);
mallocflag _ValidPointer(void *pv, size_t size);

#define FREEMEMORY(f) if((f)!=NULL){LOCK_MEM;FreeMemoryNOTHREAD((f));UNLOCK_MEM;(f)=NULL;}
#endif
