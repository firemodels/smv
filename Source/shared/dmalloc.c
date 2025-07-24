#include "options.h"
#define INDMALLOC
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dmalloc.h"
#ifdef pp_MEMDEBUG
static int checkmemoryflag=1;
#endif
#ifdef WIN32
#include <windows.h>
#endif

#ifdef pp_MEMDEBUG
static blockinfo *GetBlockInfo(bbyte *pb);
#endif

#ifdef WIN32

/* ------------------ memusage ------------------------ */

// return memory usage between 0% and 100%

int memusage(void){
  MEMORYSTATUS stat;
  int load;

  GlobalMemoryStatus(&stat);
  load=stat.dwMemoryLoad;
  return load;
}
#else

// return memory usage between 0% and 100%

int memusage(void){
  return 0;
}
#endif

/* ------------------ _memoryload ------------------------ */

#ifdef pp_memload
#ifdef WIN32
void _memoryload(unsigned int size,unsigned int *availmem){
  MEMORYSTATUS stat;

    GlobalMemoryStatus(&stat);
    if(availmem!=NULL)*availmem=stat.dwMemoryLoad;
    if(size!=0&&size>stat.dwAvailPhys-0.1*stat.dwTotalPhys){
      fprintf(stderr,"*** Warning: Low Memory. Only %i M available for viewing data.\n",
           (int)stat.dwAvailPhys/(1024*1024));
      fprintf(stderr,"    Unload datafiles or system performance may degrade.\n");
    }
}
#endif
#endif

/* ------------------ initMALLOC ------------------------ */

void initMALLOC(void){

  MMfirstptr=&MMfirst;
  MMlastptr=&MMlast;

  MMfirstptr->prev=NULL;
  MMfirstptr->next=MMlastptr;

  MMlastptr->prev=MMfirstptr;
  MMlastptr->next=NULL;
#ifdef pp_THREAD
  pthread_mutex_init(&mutexMEM,NULL);
#endif
  MMmaxmemory=0;
  MMtotalmemory=0;
}

/* ------------------ PrintMemoryError ------------------------ */

void PrintMemoryError(size_t size, const char *varname, const char *file, int linenumber){
  if(varname!=NULL){
    char *varname2 = NULL;

    varname2 = strrchr(varname, '&');
    if(varname2 != NULL)varname = varname2+1;
    fprintf(stderr, "\n***Error: Failure when allocating %llu bytes for the variable '%s'", (unsigned long long)size, varname);
  }
  else{
    fprintf(stderr, "\n***Error: Failure when allocating %llu bytes", (unsigned long long)size);
  }
  if(file!=NULL){
    char *file2=NULL;

    file2 = strrchr(file,'\\');
    if(file2==NULL)file2 = strrchr(file, '/');
    if(file2 != NULL)file = file2+1;
    fprintf(stderr," at %s(%i)\n",file,linenumber);
  }
  fprintf(stderr, "\n");
  assert(1==0); // force smokeview to abort when in debug mode
}

/* ------------------ _NewMemory ------------------------ */

mallocflag _NewMemory(void **ppv, size_t size, int memory_id, const char *varname, const char *file, int linenumber){
  mallocflag returnval;

  LOCK_MEM;
  returnval=_NewMemoryNOTHREAD(ppv, size, memory_id);
  if(returnval!=1){
     PrintMemoryError(size, varname, file, linenumber);
  }
  UNLOCK_MEM;
  return returnval;
}

/* ------------------ _NewMemoryNOTHREAD ------------------------ */

mallocflag _NewMemoryNOTHREAD(void **ppv, size_t size, int memory_id){
  void **ppb = (void **)ppv;
#ifdef pp_MEMDEBUG
  char *c;
#endif
  int infoblocksize;
  MMdata *this_ptr, *prev_ptr, *next_ptr;

  assert(ppv != NULL && size != 0);
  infoblocksize = (sizeof(MMdata) + 3) / 4;
  infoblocksize *= 4;

  //  float total, maxmem;
  //  total = MMtotalmemory/1000000000.0;
  //  maxmem = MMmaxmemory / 1000000000.0;
  //  fprintf(stderr, "memory allocated: %f GB out of %f GB\n",total,maxmem);
  if(MMmaxmemory == 0 || MMtotalmemory + size <= MMmaxmemory){
    this_ptr = (void *)malloc(infoblocksize + size + sizeofDebugByte);
  }
  else{
    this_ptr = NULL;
  }
  if(this_ptr != NULL){
    prev_ptr = MMfirstptr;
    next_ptr = MMfirstptr->next;

    prev_ptr->next = this_ptr;
    next_ptr->prev = this_ptr;

    this_ptr->size = size;
    this_ptr->memory_id = memory_id;
    this_ptr->prev = prev_ptr;
    this_ptr->next = next_ptr;
    this_ptr->marker = MARKER_BYTE;

    *ppb = (char *)this_ptr + infoblocksize;
  }
  else{
    *ppb = NULL;
  }

#ifdef pp_MEMDEBUG
  CheckMemoryNOTHREAD;
  if(*ppb != NULL){
    if(sizeofDebugByte != 0){
      c = (char *)(*ppb) + size;
      *c = (char)DEBUG_BYTE;
    }
    memset(*ppb, memGarbage, size);
    if(!CreateBlockInfo(*ppb, size)){
      free((char *)*ppb - infoblocksize);
      *ppb = NULL;
    }
  }
#endif
  MMtotalmemory += size;
  return (*ppb != NULL);
}

/* ------------------ FreeAllMemory ------------------------ */

void FreeAllMemory(int memory_id){
  MMdata *thisptr, *nextptr;
  int infoblocksize;

  LOCK_MEM;
  infoblocksize=(sizeof(MMdata)+3)/4;
  infoblocksize*=4;

#ifdef pp_MEMDEBUG
  thisptr = MMfirstptr->next;
  for(;;){
    // if the 'thisptr' memory block is freed then thisptr is no longer valid.
    // so, nextptr (which is thisptr->next) must be defined before it is freed
    nextptr = thisptr->next;
    if(thisptr->next == NULL || thisptr->marker != MARKER_BYTE)break;
    thisptr = nextptr;
  }
#endif

  thisptr = MMfirstptr->next;
  for(;;){
    // if the 'thisptr' memory block is freed then thisptr is no longer valid.
    // so, nextptr (which is thisptr->next) must be defined before it is freed
    nextptr = thisptr->next;
    if(thisptr->next == NULL || thisptr->marker != MARKER_BYTE)break;
    if(memory_id == 0 || thisptr->memory_id == memory_id){
      FreeMemoryNOTHREAD((char *)thisptr + infoblocksize);
    }
    thisptr = nextptr;
  }
  UNLOCK_MEM;
}

/* ------------------ FreeMemory ------------------------ */

void FreeMemory(void *pv){
  LOCK_MEM;
  FreeMemoryNOTHREAD(pv);
  UNLOCK_MEM;
}

/* ------------------ FreeMemoryNOTHREAD ------------------------ */

void FreeMemoryNOTHREAD(void *pv){
#ifdef pp_MEMDEBUG
  int len_memory;
#endif
  int infoblocksize;
  MMdata *this_ptr, *prev_ptr, *next_ptr;

  assert(pv != NULL);
  infoblocksize=(sizeof(MMdata)+3)/4;
  infoblocksize*=4;
#ifdef pp_MEMDEBUG
  {
    CheckMemoryNOTHREAD;
    len_memory=sizeofBlock((char *)pv);
    memset((char *)pv, memGarbage, len_memory);
    FreeBlockInfo((char *)pv);
  }
#endif
  this_ptr=(MMdata *)((char *)pv-infoblocksize);
  assert(this_ptr->marker==MARKER_BYTE);
  MMtotalmemory-=this_ptr->size;
  prev_ptr=this_ptr->prev;
  next_ptr=this_ptr->next;

  prev_ptr->next=next_ptr;
  next_ptr->prev=prev_ptr;
  free((char *)pv-infoblocksize);
}

/* ------------------ _ResizeMemory ------------------------ */

mallocflag _ResizeMemory(void **ppv, size_t sizeNew, int memory_id, const char *varname, const char *file, int linenumber){
  mallocflag returnval;

  LOCK_MEM;
  returnval=_ResizeMemoryNOTHREAD(ppv, sizeNew, memory_id);
  if(returnval!=1){
    PrintMemoryError(sizeNew, varname, file, linenumber);
  }
  UNLOCK_MEM;
  return returnval;
}

/* ------------------ _ResizeMemoryNOTHREAD ------------------------ */

mallocflag _ResizeMemoryNOTHREAD(void **ppv, size_t sizeNew, int memory_id){
  bbyte **ppold, *pbNew;
  int infoblocksize;
  MMdata *this_ptr, *prev_ptr, *next_ptr;
#ifdef pp_MEMDEBUG
  char *c;
  size_t sizeOld;
#endif

  infoblocksize=(sizeof(MMdata)+3)/4;
  infoblocksize*=4;

  ppold=(bbyte **)ppv;
  assert(ppold != NULL && sizeNew != 0);
#ifdef pp_MEMDEBUG
  {
    CheckMemoryNOTHREAD;
    sizeOld = sizeofBlock(*ppold);
    if(sizeNew<sizeOld){
      memset((*ppold)+sizeNew,memGarbage,sizeOld-sizeNew);
    }
    else if(sizeNew > sizeOld){
      void *pbForceNew;

      if(_NewMemoryNOTHREAD((void **)&pbForceNew, sizeNew, 0)){
        memcpy(pbForceNew, *ppold, sizeOld);
        FreeMemoryNOTHREAD(*ppold);
        *ppold = pbForceNew;
      }
    }
  }
#endif

  this_ptr=(MMdata *)((char *)(*ppold)-infoblocksize);
  prev_ptr=this_ptr->prev;
  next_ptr=this_ptr->next;
  pbNew = realloc((char *)(*ppold)-infoblocksize, infoblocksize+sizeNew+sizeofDebugByte);
  if(pbNew != NULL){
    if(pbNew!=(char *)(*ppold)-infoblocksize){
      this_ptr=(MMdata *)pbNew;

      prev_ptr->next=this_ptr;
      next_ptr->prev=this_ptr;

      this_ptr->size = sizeNew;
      this_ptr->memory_id = memory_id;
      this_ptr->next=next_ptr;
      this_ptr->prev=prev_ptr;
      this_ptr->marker=MARKER_BYTE;
    }
#ifdef pp_MEMDEBUG
    {
      if(sizeofDebugByte!=0){
        c = pbNew + infoblocksize + sizeNew;
        *c=(char)DEBUG_BYTE;
      }
      UpdateBlockInfo(*ppold, (char *)pbNew+infoblocksize, sizeNew);
      if(sizeNew>sizeOld){
        memset(pbNew+infoblocksize+sizeOld,memGarbage,sizeNew-sizeOld);
      }
    }
#endif
    *ppold = pbNew+infoblocksize;
  }
  return (pbNew != NULL);
}

/* ------------------ SetMemCheck ------------------------ */
void SetMemCheck(float memGB){
  if(memGB < 0)memGB = 0;
  MMmaxmemory = memGB * (MMsize)(1000*1000*1000);
}

#ifdef pp_MEMDEBUG
/* ------------------ pointer comparison defines ------------------------ */

#define fPtrEqual(pLeft, pRight)  ((pLeft) == (pRight))
#define fPtrLessEq(pLeft, pRight) ((pLeft) <= (pRight))
#define fPtrGrtrEq(pLeft, pRight) ((pLeft) >= (pRight))

/* ------------------ __NewMemory ------------------------ */

mallocflag __NewMemory(void **ppv, size_t size, int memory_id, const char *varname, const char *file, int linenumber){
  void **ppb=(void **)ppv;
  blockinfo *pbi;
  int return_code;
  const char *varname2;
  const char *file2;
  char ampersand='&';
#ifdef WIN32
  char dirsep='\\';
#else
  char dirsep='/';
#endif

#ifdef pp_MEM_DEBUG_PRINT
  fprintf(stderr, "file: %s line: %i\n", file, linenumber);
#endif
  LOCK_MEM;
  return_code=_NewMemoryNOTHREAD(ppb,size,memory_id);
  if(return_code != 1){
    PrintMemoryError(size, varname, file, linenumber);
  }
  pbi=GetBlockInfo((bbyte *)*ppb);
  if(return_code == 1 && pbi == NULL){ // don't print error message twice
    PrintMemoryError(size, varname, file, linenumber);
  }
  pbi->linenumber=linenumber;

  file2=strrchr(file,dirsep);
  if(file2==NULL){
    file2=file;
  }
  else{
    file2++;
  }
  if(strlen(file2)<256){
    strcpy(pbi->filename,file2);
  }
  else{
    strncpy(pbi->filename,file2,255);
    strcat(pbi->filename,"\0");
  }

  varname2 = strchr(varname,ampersand);
  if(varname2==NULL){
    varname2=varname;
  }
  else{
    varname2++;
  }
  if(strlen(varname2)<256){
    strcpy(pbi->varname,varname2);
  }
  else{
    strncpy(pbi->varname,varname2,255);
    strcat(pbi->varname,"\0");
  }
  if(return_code!=1){
    PrintMemoryError(size, varname, file, linenumber);
  }
  UNLOCK_MEM;
  return return_code;
}

/* ------------------ __ResizeMemory ------------------------ */

mallocflag __ResizeMemory(void **ppv, size_t size, int memory_id, const char *varname, const char *file, int linenumber){
  void **ppb=(void **)ppv;
  blockinfo *pbi;
  int return_code;

  LOCK_MEM;
  return_code=_ResizeMemoryNOTHREAD(ppb,size,memory_id);
  pbi=GetBlockInfo((bbyte *)*ppb);
  pbi->linenumber=linenumber;
  if(strlen(file)<256){
    strcpy(pbi->filename,file);
  }
  else{
    strncpy(pbi->filename,file,255);
    strcat(pbi->filename,"\0");
  }
  if(strlen(varname)<256){
    strcpy(pbi->varname,varname);
  }
  else{
    strncpy(pbi->varname,varname,255);
    strcat(pbi->varname,"\0");
  }
  if(return_code!=1){
    PrintMemoryError(size, varname, file, linenumber);
  }
  UNLOCK_MEM;
  return return_code;
}

static blockinfo *pbiHead = NULL;

/* ------------------ GetBlockInfo ------------------------ */

static blockinfo *GetBlockInfo(bbyte *pb){
  blockinfo *pbi;
  for(pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    bbyte *pbStart = pbi->pb;
    bbyte *pbEnd   = pbi->pb + pbi->size - 1;

    if(fPtrGrtrEq(pb, pbStart) && fPtrLessEq(pb, pbEnd))break;
  }
  return (pbi);
}

/* ------------------ CountMemoryBlocks ------------------------ */

int _CountMemoryBlocks(void){
  blockinfo *pbi;
  int n=0;

  for(pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext){
    n++;
  }
  return n;
}

/* ------------------ PrintAllMemoryInfo ------------------------ */

void _PrintAllMemoryInfo(void){
  blockinfo *pbi;
  int n=0,size=0;

  fprintf(stderr, "\n\n");
  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "********************************************\n");
  fprintf(stderr, "********************************************\n");
  for(pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    n++;
    size += pbi->size;
    fprintf(stderr, "%s allocated in %s at line %i\n",pbi->varname,pbi->filename,pbi->linenumber);
  }
  fprintf(stderr, "nblocks=%i sizeblocks=%i\n",n,size);
}

/* ------------------ GetBlockInfo_nofail ------------------------ */

static blockinfo *GetBlockInfo_nofail(bbyte *pb){
  blockinfo *pbi;
  for(pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext)
  {
    bbyte *pbStart = pbi->pb;
    bbyte *pbEnd   = pbi->pb + pbi->size - 1;

    if(fPtrGrtrEq(pb, pbStart) && fPtrLessEq(pb, pbEnd))
      break;
  }
  return (pbi);
}

/* ------------------ _CheckMemoryOn ------------------------ */

void _CheckMemoryOn(void){
  checkmemoryflag=1;
}

/* ------------------ _CheckMemoryOff ------------------------ */

void _CheckMemoryOff(void){
  checkmemoryflag=0;
}


/* ------------------ _CheckMemory ------------------------ */

void _CheckMemory(void){
  LOCK_MEM;
  _CheckMemoryNOTHREAD();
  UNLOCK_MEM;
}

/* ------------------ _CheckMemory ------------------------ */

void _CheckMemoryNOTHREAD(void){
  blockinfo *pbi;
  if(checkmemoryflag==0)return;
  for(pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext){
    if(sizeofDebugByte!=0){
      assert((char)*(pbi->pb+pbi->size)==(char)DEBUG_BYTE);
    }
  }
  return;
}

/* ------------------ CreateBlockInfo ------------------------ */

mallocflag CreateBlockInfo(bbyte *pbNew, size_t sizeNew){
  blockinfo *pbi;

  assert(pbNew != NULL && sizeNew != 0);

  pbi = (blockinfo *)malloc(sizeof(blockinfo));
  if( pbi != NULL){
    pbi->pb = pbNew;
    pbi->size = sizeNew;
    pbi->pbiNext = pbiHead;
    pbiHead = pbi;
  }
  return (mallocflag)(pbi != NULL);
}

/* ------------------ FreeBlockIfno ------------------------ */

void FreeBlockInfo(bbyte *pbToFree){
  blockinfo *pbi, *pbiPrev;

  pbiPrev = NULL;
  for(pbi = pbiHead; pbi != NULL; pbi = pbi->pbiNext){
    if(fPtrEqual(pbi->pb, pbToFree)){
      if(pbiPrev == NULL){
        pbiHead = pbi->pbiNext;
      }
      else{
        pbiPrev->pbiNext = pbi->pbiNext;
      }
      break;
    }
    pbiPrev = pbi;
  }
  assert(pbi != NULL);
  if(sizeofDebugByte!=0){
    assert((char)*(pbi->pb+pbi->size)==(char)DEBUG_BYTE);
  }
  free(pbi);
}

/* ------------------ UpdateBlockInfo ------------------------ */

void UpdateBlockInfo(bbyte *pbOld, bbyte *pbNew, size_t sizeNew){
  blockinfo *pbi;

  assert(pbNew != NULL && sizeNew != 0);

  pbi = GetBlockInfo(pbOld);
  assert(pbOld == pbi->pb);

  pbi->pb = pbNew;
  pbi->size = sizeNew;
}

/* ------------------ sizeofBlock ------------------------ */

size_t sizeofBlock(bbyte *pb){
  blockinfo *pbi;

  pbi = GetBlockInfo(pb);
  assert(pb==pbi->pb);
  if(sizeofDebugByte!=0){
    assert((char)*(pbi->pb+pbi->size)==(char)DEBUG_BYTE);
  }
  return(pbi->size);
}

/* ------------------ ValidPointer ------------------------ */

mallocflag _ValidPointer(void *pv, size_t size){
  blockinfo *pbi;
  bbyte *pb = (bbyte *)pv;

  assert(pv != NULL && size != 0);

  pbi = GetBlockInfo(pb);
  assert(pb==pbi->pb);

  assert(fPtrLessEq(pb+size,pbi->pb + pbi->size));

  if(sizeofDebugByte!=0){
    assert((char)*(pbi->pb+pbi->size)==(char)DEBUG_BYTE);
  }
  return(1);
}

/* ------------------ strcpy ------------------------ */

char *_strcpy(char *s1, const char *s2){
  blockinfo *pbi;
  int offset;

  LOCK_MEM;
  CheckMemoryNOTHREAD;
  pbi = GetBlockInfo_nofail(s1);
  if(pbi!=NULL){
    offset = s1 - pbi->pb;
    assert(pbi->size - offset >= strlen(s2)+1);
  }
  UNLOCK_MEM;

  return strcpy(s1,s2);
}

/* ------------------ strcat ------------------------ */

char *_strcat(char *s1, const char *s2){
  blockinfo *pbi;
  int offset;

  LOCK_MEM;
  CheckMemoryNOTHREAD;
  pbi = GetBlockInfo_nofail(s1);
  if(pbi!=NULL){
    offset = s1 - pbi->pb;
    assert(pbi->size - offset >= strlen(s1)+strlen(s2)+1);
  }
  UNLOCK_MEM;

  return strcat(s1,s2);
}

/* ------------------ GetTotalMemory ------------------------ */

MMsize _GetTotalMemory(void){
  return MMtotalmemory;
}

/* ------------------ getMemusage ------------------------ */

void getMemusage(MMsize totalmemory,char *MEMlabel){
  int size;
  float rsize;

  if(totalmemory<1000000000){
    size = totalmemory/1000000;
    sprintf(MEMlabel,"%i MB",size);
  }
  else{
    rsize = totalmemory/1000000000.0;
    sprintf(MEMlabel,"%4.2f GB",rsize);
  }
}
#endif
