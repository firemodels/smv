#include "options.h"
#ifdef pp_STREAM
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef pp_LINUX
#include <sys/mman.h>
#endif
#ifdef pp_OSX
#include <sys/mman.h>
#endif
#include <time.h>
#include <math.h>
#include "dmalloc.h"
#include "smokeviewvars.h"
#include "smokestream.h"

#ifdef WIN32
#include <io.h>
#include <windows.h>
#include <sys/types.h>

/* mmap() replacement for Windows
 *
 * Author: Mike Frysinger <vapier@gentoo.org>
 * Placed into the public domain
 */

/* References:
 * CreateFileMapping: http://msdn.microsoft.com/en-us/library/aa366537(VS.85).aspx
 * CloseHandle:       http://msdn.microsoft.com/en-us/library/ms724211(VS.85).aspx
 * MapViewOfFile:     http://msdn.microsoft.com/en-us/library/aa366761(VS.85).aspx
 * UnmapViewOfFile:   http://msdn.microsoft.com/en-us/library/aa366882(VS.85).aspx
 */


#define PROT_READ     0x1
#define PROT_WRITE    0x2
/* This flag is only available in WinXP+ */
#ifdef FILE_MAP_EXECUTE
#define PROT_EXEC     0x4
#else
#define PROT_EXEC        0x0
#define FILE_MAP_EXECUTE 0
#endif

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void *) -1)

#ifdef __USE_FILE_OFFSET64
# define DWORD_HI(x) (x >> 32)
# define DWORD_LO(x) ((x) & 0xffffffff)
#else
# define DWORD_HI(x) (0)
# define DWORD_LO(x) (x)
#endif

/* ------------------  GetFrameIndex ------------------------ */

int GetFrameIndex(float time, float *times, int ntimes){
  int left, right, mid;
  left = 0;
  right = ntimes-1;
  if(time<=times[left])return left;
  if(time>=times[right])return right;
  while(right-left>1){
    mid = (left+right)/2;
    if(time<times[mid]){
      right = mid;
    }
    else{
      left = mid;
    }
  }
  if(time-times[left]<times[right]-time)return left;
  return right;
}

/* ------------------  mmap ------------------------ */

static void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  if(prot&~(PROT_READ|PROT_WRITE|PROT_EXEC))
    return MAP_FAILED;
  if(fd==-1) {
    if(!(flags&MAP_ANON)||offset)
      return MAP_FAILED;
  }
  else if(flags&MAP_ANON)
    return MAP_FAILED;

  DWORD flProtect;
  flProtect = PAGE_READWRITE;
  if(prot&PROT_WRITE) {
    if(prot&PROT_EXEC)
      flProtect = PAGE_EXECUTE_READWRITE;
    else
      flProtect = PAGE_READWRITE;
  }
  else if(prot&PROT_EXEC) {
    if(prot&PROT_READ)
      flProtect = PAGE_EXECUTE_READ;
    else if(prot&PROT_EXEC)
      flProtect = PAGE_EXECUTE;
  }
  else
    flProtect = PAGE_READONLY;

  off_t end = length+offset;
  HANDLE mmap_fd, h;
  if(fd==-1)
    mmap_fd = INVALID_HANDLE_VALUE;
  else
    mmap_fd = (HANDLE)_get_osfhandle(fd);
  h = CreateFileMapping(mmap_fd, NULL, flProtect, DWORD_HI(end), DWORD_LO(end), NULL);
  if(h==NULL)
    return MAP_FAILED;

  DWORD dwDesiredAccess;
  if(prot&PROT_WRITE)
    dwDesiredAccess = FILE_MAP_WRITE;
  else
    dwDesiredAccess = FILE_MAP_READ;
  if(prot&PROT_EXEC)
    dwDesiredAccess |= FILE_MAP_EXECUTE;
  if(flags&MAP_PRIVATE)
    dwDesiredAccess |= FILE_MAP_COPY;
  void *ret = MapViewOfFile(h, dwDesiredAccess, DWORD_HI(offset), DWORD_LO(offset), length);
  if(ret==NULL) {
    CloseHandle(h);
    ret = MAP_FAILED;
  }
  return ret;
}

/* ------------------  munmap ------------------------ */

static void munmap(void *addr, size_t length)
{
  UnmapViewOfFile(addr);
  /* ruh-ro, we leaked handle from CreateFileMapping() ... */
}

#undef DWORD_HI
#undef DWORD_LO
#endif

/* ------------------  MemMap ------------------------ */

char *MemMap(char *file, size_t *size){
   int fd;
   char *ptr;

   if(file==NULL)return NULL;
   fd = open(file, O_RDONLY);
   struct stat statbuf;
   fstat(fd, &statbuf);
   *size = statbuf.st_size;
   ptr = mmap(NULL, *size, PROT_READ|PROT_WRITE,MAP_SHARED, fd,0);
   if(ptr==MAP_FAILED)return NULL;
   return ptr;
}

/* ------------------  MemUnMap ------------------------ */

void MemUnMap(char *data, size_t size){
  munmap(data, size);
}

/* ------------------  StreamOpen ------------------------ */

streamdata *StreamOpen(streamdata *streamin, char *file, size_t offset, int *framesizes, int nframes, char *label, int constant_frame_size){
  streamdata *stream;
  STRUCTSTAT statbuffer;
  int i, statfile;
  int start, stop;

  if(file==NULL||strlen(file)==0||nframes<=0)return NULL;
  statfile = STAT(file, &statbuffer);
  if(statfile!=0)return NULL;

  if(streamin==NULL||nframes>streamin->nframes){
    if(streamin==NULL){
      NewMemory((void **)&stream, sizeof(streamdata));
      NewMemory((void **)&(stream->file), strlen(file)+1);
      strcpy(stream->file, file);
      if(label==NULL){
        stream->label = NULL;
      }
      else{
        NewMemory((void **)&(stream->label), strlen(label)+1);
        strcpy(stream->label, label);
      }
      NewMemory((void **)&(stream->frameptrs),     nframes*sizeof(char *));
      NewMemory((void **)&(stream->filebuffer),    statbuffer.st_size);
      NewMemory((void **)&(stream->framesizes),    nframes*sizeof(size_t));
      NewMemory((void **)&(stream->frame_offsets), nframes*sizeof(size_t));
      NewMemory((void **)&(stream->load_status),   nframes*sizeof(int));
      stream->frame_offsets[0] = offset;
      stream->frameptrs[0]     = NULL;
      stream->framesizes[0]    = framesizes[0];
      stream->load_status[0]   = STREAM_UNLOADED;
      stream->load_time        = 0.0;
      start = 1;
    }
    else{
      char *filebuffer_old;

      stream = streamin;
      filebuffer_old = stream->filebuffer;
      ResizeMemory((void **)&(stream->frameptrs),     nframes*sizeof(char *));
      ResizeMemory((void **)&(stream->filebuffer),    statbuffer.st_size);
      ResizeMemory((void **)&(stream->framesizes),    nframes*sizeof(size_t));
      ResizeMemory((void **)&(stream->frame_offsets), nframes*sizeof(size_t));
      if(filebuffer_old!=stream->filebuffer){
        for(i = 0; i<stream->nframes; i++){
          if(stream->frameptrs[i]!=NULL)stream->frameptrs[i] = stream->filebuffer+stream->frame_offsets[i];
        }
      }
      start = stream->nframes;
    }
    stop = nframes;
    stream->filesize = statbuffer.st_size;

    for(i = start; i<stop; i++){
      int sizei, sizeim1;

      if(constant_frame_size==1){
        sizei   = framesizes[0];
        sizeim1 = framesizes[0];
      }
      else{
        sizei   = framesizes[i];
        sizeim1 = framesizes[i-1];
      }
      stream->load_status[i]   = STREAM_UNLOADED;
      stream->framesizes[i]    = sizei;
      stream->frame_offsets[i] = stream->frame_offsets[i-1]+sizeim1;
      stream->frameptrs[i]     = NULL;
    }
  }
  else{
    stream = streamin;
  }
  stream->nframes = nframes;
  CheckMemory;
  return stream;
}

/* ------------------  StreamClose ------------------------ */

void StreamClose(streamdata **streamptr){
  streamdata *stream;

  stream = *streamptr;
  if(stream==NULL)return;
  FREEMEMORY(stream->load_status);
  FREEMEMORY(stream->frameptrs);
  FREEMEMORY(stream->filebuffer);
  FREEMEMORY(stream->framesizes);
  FREEMEMORY(stream->frame_offsets);
  FREEMEMORY(stream->file);
  FREEMEMORY(stream);
  *streamptr = NULL;
  CheckMemory;
}

/* ------------------  StreamRead ------------------------ */

FILE_SIZE StreamRead(streamdata *stream, int frame_index){
  FILE *filestream;
  FILE_SIZE file_size;

  clock_t time_start = clock();

  LOCK_STREAM;
  if(stream==NULL||frame_index<0||frame_index>stream->nframes-1){
    stream->load_status[frame_index] = STREAM_ERROR;
    UNLOCK_STREAM;
    return 0;
  }
  if(stream->load_status[frame_index]!=STREAM_UNLOADED){
    UNLOCK_STREAM;
    return 0;
  }
  stream->load_status[frame_index] = STREAM_LOADING;
  UNLOCK_STREAM;

   filestream = fopen(stream->file, "rb");
  if(filestream==NULL)return 0;

  fseek(filestream, stream->frame_offsets[frame_index], SEEK_SET);
  file_size = fread(stream->filebuffer+stream->frame_offsets[frame_index], 1, stream->framesizes[frame_index], filestream);
  CheckMemory;
  stream->frameptrs[frame_index] = stream->filebuffer+stream->frame_offsets[frame_index];
  CheckMemory;
  fclose(filestream);
  ASSERT(file_size==stream->framesizes[frame_index]);

  stream->load_status[frame_index] = STREAM_LOADED;
  clock_t time_end = clock();
  LOCK_STREAM;
  stream->load_time += (float)(time_end - time_start)/CLOCKS_PER_SEC;
  UNLOCK_STREAM;

  return file_size;
}

/* ------------------  StreamAllLoaded ------------------------ */

int StreamAllLoaded(streamdata *stream){
  int i;

  for(i = 0; i<stream->nframes; i++){
    if(stream->frameptrs[i]==NULL)return 0;
  }
  return 1;
}

/* ------------------  StreamFrameSizeOutput ------------------------ */

void StreamFrameSizeOutput(size_t file_size, float *time){
  if(file_size>1000000000){
    fprintf(stderr, "(%.1f GB", (float)file_size/1000000000.);
  }
  else if(file_size>1000000){
    fprintf(stderr, "(%.1f MB", (float)file_size/1000000.);
  }
  else{
    fprintf(stderr, "(%.0f KB", (float)file_size/1000.);
  }
  if(time!=NULL)fprintf(stderr, "/%.1f s", *time);
  fprintf(stderr, ")\n");
}

/* ------------------  StreamReadList ------------------------ */

void StreamReadList(streamdata **streams, int nstreams){
  int frame_index, stream_index, i, max_frames;
  int stream_output = 0;
  int stream_pause = 0;
  float totaltime;

  clock_t totalticks = clock();
  max_frames = streams[0]->nframes;
  for(i = 1; i<nstreams; i++){
    max_frames = MAX(max_frames, streams[i]->nframes);
  }

  for(;;){ //continue until all frames are loaded
    for(frame_index = 0; frame_index<max_frames; frame_index++){
      int count_streams;

      for(count_streams=0, stream_index = 0; stream_index<nstreams; stream_index++){
        streamdata *stream;

        stream = streams[stream_index];
        if(frame_index>stream->nframes-1)continue;
        int read_stream;

        LOCK_STREAM;
        read_stream = 0;
        if(stream->load_status[frame_index]==STREAM_UNLOADED)read_stream = 1;
        UNLOCK_STREAM;
        if(read_stream==1){
          count_streams++;
          StreamRead(stream, frame_index);
        }
      }
      if(stream_pause==1)PauseTime(0.5);
      if(count_streams>0&&stream_output==1){
        fprintf(stderr, "Loading frame(%i streams): %i\n", count_streams, frame_index);
      }
    }
    // check if all frames have been loaded, if so then we are finished if not then load some more
    int all_loaded = 1;
    for(stream_index = 0; stream_index<nstreams; stream_index++){
      all_loaded = StreamAllLoaded(streams[stream_index]);
      if(all_loaded==0){
        break;
      }
    }
    if(all_loaded==1){
      size_t total_filesize = 0;
      float total_time=0.0;

      totalticks = (clock()-totalticks);
      totaltime = (float)totalticks/(float)CLOCKS_PER_SEC;
      for(stream_index = 0; stream_index<nstreams; stream_index++){
        streamdata *streami;

        streami = streams[stream_index];
        fprintf(stderr, "Loaded %s", streami->file);
        StreamFrameSizeOutput(streami->filesize, &(streami->load_time));
        total_filesize += streami->filesize;
        total_time     += streami->load_time;
      }
      if(nstreams>1){
        fprintf(stderr, "Loaded total(%f)", totaltime);
        StreamFrameSizeOutput(total_filesize, &total_time);
      }
      return;
    }
  }
}

/* ------------------  StreamCheck ------------------------ */

void StreamCheck(streamdata *framestream){
  FILE *filestream;
  int i;

  filestream = fopen(framestream->file, "rb");
  for(i = 0; i<framestream->nframes; i++){
    int offset;
    float time;

    offset = framestream->frame_offsets[i]+4;
    fseek(filestream, offset, SEEK_SET);
    fread(&time, 4, 1, filestream);
    fprintf(stderr, "%f ", time);
    if(i%10==0)fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
  fclose(filestream);
}

#endif
