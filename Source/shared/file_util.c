#include "options.h"
#define IN_FILE_UTIL
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include <math.h>
#ifdef WIN32
#ifdef __MINGW32__
#undef S_IFBLK
#undef S_ISBLK
#undef S_ISFIFO
#undef S_ISDIR
#undef S_ISCHR
#undef S_ISREG
#endif
#include <io.h>
#include <direct.h>
#include <dirent_win.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#else
#include <dirent.h>
#include <libgen.h>
#endif
#include "dmalloc.h"
#include "string_util.h"
#include "file_util.h"
#include "threader.h"

FILE *alt_stdout=NULL;

/* ------------------ TestWrite ------------------------ */

void TestWrite(char *scratchdir, char **fileptr){
  char *file, filecopy[1024], newfile[1024], *beg;

  if(Writable(".")==1||fileptr==NULL||scratchdir==NULL)return;
  strcpy(filecopy, *fileptr);
  //FREEMEMORY(*fileptr); possible memory leak if commented but line was causing crash

  strcpy(newfile, scratchdir);
  strcat(newfile, dirseparator);
  file = filecopy;
  beg = strrchr(filecopy, SEP);
  if(beg!=NULL)file = beg+1;
  strcat(newfile, file);
  NewMemory((void **)fileptr, strlen(newfile)+1);
  strcpy(*fileptr, newfile);
}

/* ------------------ FFLUSH ------------------------ */

int FFLUSH(void){
  int return_val=0;

  if(alt_stdout!=NULL){
    return_val = fflush(alt_stdout);
  }
  return return_val;
}

/* ------------------ PRINTF ------------------------ */

int PRINTF(const char *format, ...){
  va_list args;
  int return_val=0;

  if(alt_stdout!=NULL){
    va_start(args, format);
    return_val=vfprintf(alt_stdout, format, args);
    va_end(args);
  }
  return return_val;
}

/* ------------------ SetStdOut ------------------------ */

void SetStdOut(FILE *stream){
  alt_stdout=stream;
}

#define FILE_BUFFER 1000

/* ------------------ CopyFile ------------------------ */

void CopyFILE(char *destdir, char *file_in, char *file_out, int mode){
  char buffer[FILE_BUFFER];
  FILE *streamin=NULL, *streamout=NULL;
  char *full_file_out=NULL;
  size_t chars_in;

  if(destdir==NULL||file_in==NULL)return;
  streamin=fopen(file_in,"rb");
  if(streamin==NULL)return;

  full_file_out=NULL;
  NewMemory((void **)&full_file_out,strlen(file_out)+strlen(destdir)+1+1);
  strcpy(full_file_out,destdir);
  if(destdir[strlen(destdir)-1]!=*dirseparator){
    strcat(full_file_out,dirseparator);
  }
  strcat(full_file_out,file_out);

  if(mode==REPLACE_FILE){
    streamout=fopen(full_file_out,"wb");
  }
  else if(mode==APPEND_FILE){
    streamout=fopen(full_file_out,"ab");
  }
  else{
    assert(FFALSE);
  }

  if(streamout==NULL){
    FREEMEMORY(full_file_out);
    fclose(streamin);
    return;
  }
  fprintf(stderr, "  Copying %s to %s\n",file_in,file_out);
  for(;;){
    int end_of_file;

    end_of_file=0;
    chars_in=fread(buffer,1,FILE_BUFFER,streamin);
    if(chars_in!=FILE_BUFFER)end_of_file=1;
    if(chars_in>0)fwrite(buffer,chars_in,1,streamout);
    if(end_of_file==1)break;
  }
  FREEMEMORY(full_file_out);
  fclose(streamin);
  fclose(streamout);
}

/* ------------------ HaveProg ------------------------ */

int HaveProg(char *prog){
  if(system(prog) == 0)return 1;
  return 0;
}

/* ------------------ GetSmokeZipPath ------------------------ */

char *GetSmokeZipPath(char *progdir){
  char *zip_path;

  if(progdir!=NULL){
    NewMemory((void **)&zip_path,strlen(progdir)+20);
    strcpy(zip_path,progdir);
  }
  else{
    NewMemory((void **)&zip_path,2+20);
    strcpy(zip_path,".");
    strcat(zip_path,dirseparator);
  }

  strcat(zip_path,"smokezip");
#ifdef WIN32
  strcat(zip_path,".exe");
#endif
  if(FILE_EXISTS(zip_path)==YES)return zip_path;
  FREEMEMORY(zip_path);
  return NULL;
}

/* ------------------ SetDir ------------------------ */

char *SetDir(char *argdir){
  int lendir;
  char *dir;

  lendir=strlen(argdir);
  NewMemory((void **)&dir,lendir+2);
  strcpy(dir,argdir);
  if(dir[lendir-1]!=dirseparator[0]){
    strcat(dir,dirseparator);
  }
  return dir;
}

/* ------------------ GetBaseFileName ------------------------ */

char *GetBaseFileName(char *buffer, const char *file){
  char *filebase,*ext;

  strcpy(buffer,file);
#ifdef WIN32
  filebase=strrchr(buffer,'\\');
#else
  filebase=strrchr(buffer,'/');
#endif
  if(filebase==NULL){
    filebase=buffer;
  }
  else{
    filebase++;
  }
  ext = strrchr(filebase,'.');
  if(ext!=NULL)*ext=0;
  return filebase;
}

/* ------------------ GetFileName ------------------------ */

char *GetFileName(char *temp_dir, char *file, int force_in_temp_dir){
  char *file2;
  char *file_out=NULL;
  FILE *stream=NULL;

  TrimBack(file);
  file2=TrimFront(file);
  if(force_in_temp_dir==NOT_FORCE_IN_DIR){
    stream=fopen(file2,"r");
    if(Writable(".")==YES||stream!=NULL){
      NewMemory((void **)&file_out,strlen(file2)+1);
      strcpy(file_out,file2);
    }
    if(stream!=NULL)fclose(stream);
  }
  if(file_out==NULL&&temp_dir!=NULL&&Writable(temp_dir)==YES){
    NewMemory((void **)&file_out,strlen(temp_dir)+1+strlen(file2)+1);
    strcpy(file_out, "");
    if(strcmp(temp_dir, ".")!=0){
      strcat(file_out, temp_dir);
      strcat(file_out, dirseparator);
    }
    strcat(file_out, file);
  }
  return file_out;
}

/* ------------------ FullFile ------------------------ */

void FullFile(char *file_out, char *dir, char *file){
  char *file2;

  TrimBack(file);
  file2=TrimFront(file);
  strcpy(file_out,"");
  if(dir!=NULL)strcat(file_out,dir);
  strcat(file_out,file2);
}

/* ------------------ StreamCopy ------------------------ */

unsigned int StreamCopy(FILE *stream_in, FILE *stream_out, int flag){
  int c;
  unsigned int nchars = 0;

  if(stream_in == NULL || stream_out == NULL)return 0;

  rewind(stream_in);
  c = fgetc(stream_in);
  while(c != EOF){
    if(flag == 0)return 1;
    fputc(c, stream_out);
    c = fgetc(stream_in);
    nchars++;
  }
  return nchars;
}

/* ------------------ FileErase ------------------------ */

void FileErase(char *file){
  if(FileExistsOrig(file) == 1){
    UNLINK(file);
  }
}

/* ------------------ FileCopy ------------------------ */

void FileCopy(char *file_in, char *file_out){
  FILE *stream_in, *stream_out;
  int c;

  if(file_in == NULL || file_out == NULL)return;
  stream_in = fopen(file_in, "rb");
  if(stream_in == NULL)return;
  stream_out = fopen(file_out, "wb");
  if(stream_out == NULL){
    fclose(stream_in);
    return;
  }

  c = fgetc(stream_in);
  while(c != EOF){
    fputc(c, stream_out);
    c = fgetc(stream_in);
  }
  fclose(stream_in);
  fclose(stream_out);
}

  /* ------------------ FileCat ------------------------ */

int FileCat(char *file_in1, char *file_in2, char *file_out){
  char buffer[FILE_BUFFER];
  FILE *stream_in1, *stream_in2, *stream_out;
  int chars_in;

  if(file_in1==NULL||file_in2==NULL)return -1;
  if(file_out==NULL)return -2;

  stream_in1=fopen(file_in1,"r");
  if(stream_in1==NULL)return -1;

  stream_in2=fopen(file_in2,"r");
  if(stream_in2==NULL){
    fclose(stream_in1);
    return -1;
  }

  stream_out=fopen(file_out,"w");
  if(stream_out==NULL){
    fclose(stream_in1);
    fclose(stream_in2);
    return -2;
  }

  for(;;){
    int end_of_file;

    end_of_file=0;
    chars_in=fread(buffer,1,FILE_BUFFER,stream_in1);
    if(chars_in!=FILE_BUFFER)end_of_file=1;
    if(chars_in>0)fwrite(buffer,chars_in,1,stream_out);
    if(end_of_file==1)break;
  }
  fclose(stream_in1);

  for(;;){
    int end_of_file;

    end_of_file=0;
    chars_in=fread(buffer,1,FILE_BUFFER,stream_in2);
    if(chars_in!=FILE_BUFFER)end_of_file=1;
    if(chars_in>0)fwrite(buffer,chars_in,1,stream_out);
    if(end_of_file==1)break;
  }
  fclose(stream_in2);
  fclose(stream_out);
  return 0;

}

/* ------------------ MakeOutFile ------------------------ */

void MakeOutFile(char *outfile, char *destdir, char *file1, char *ext){
  char filename_buffer[1024], *file1_noext;

  TrimBack(file1);
  strcpy(filename_buffer,TrimFront(file1));
  file1_noext=strstr(filename_buffer,ext);
  strcpy(outfile,"");
  if(file1_noext==NULL)return;
  file1_noext[0]='\0';
  if(destdir!=NULL){
    strcpy(outfile,destdir);
  }
  strcat(outfile,filename_buffer);
  strcat(outfile,"_diff");
  strcat(outfile,ext);
}


/* ------------------ Writable ------------------------ */

int Writable(char *dir){

  // returns 1 if the directory can be written to, 0 otherwise

  if(dir == NULL || strlen(dir) == 0)return NO;

#ifdef pp_LINUX
  if(ACCESS(dir,F_OK|W_OK)==-1){
    return NO;
  }
  else{
    return YES;
  }
#else
  {
    char tempfullfile[100], tempfile[40];
    FILE *stream;

    strcpy(tempfullfile,dir);
    strcat(tempfullfile,dirseparator);
    RandStr(tempfile,35);
    strcat(tempfullfile,tempfile);
    stream = fopen(tempfullfile,"w");
    if(stream==NULL){
      UNLINK(tempfullfile);
      return NO;
    }
    fclose(stream);
    UNLINK(tempfullfile);
    return YES;
  }
#endif
}

/* ------------------ IfFirstLineBlank ------------------------ */

int IfFirstLineBlank(char *file){

  // returns 1 if first line of file is blank

  STRUCTSTAT statbuff1;
  int statfile1;
  FILE *stream = NULL;
  char buffer[255], *buffptr;

  if(file==NULL)return 1;

  statfile1 = STAT(file, &statbuff1);
  if(statfile1!=0)return 1;

  stream = fopen(file, "r");
  if(stream==NULL||fgets(buffer, 255, stream)==NULL){
    if(stream!=NULL)fclose(stream);
    return 1;
  }
  fclose(stream);
  buffptr = TrimFrontBack(buffer);
  if(strlen(buffptr)==0)return 1;
  return 0;
}

/* ------------------ IsFileNewer ------------------------ */

int IsFileNewer(char *file1, char *file2){

// returns 1 if file1 is newer than file2, 0 otherwise

  STRUCTSTAT statbuff1, statbuff2;
  int statfile1, statfile2;

  if(file1==NULL||file2==NULL)return -1;

  statfile1=STAT(file1,&statbuff1);
  statfile2=STAT(file2,&statbuff2);
  if(statfile1!=0||statfile2!=0)return -1;

  if(statbuff1.st_mtime>statbuff2.st_mtime)return 1;
  return 0;
}

  /* ------------------ GetFileInfo ------------------------ */

int GetFileInfo(char *filename, char *source_dir, FILE_SIZE *filesize){
  STRUCTSTAT statbuffer;
  int statfile;
  char buffer[1024];

  if(source_dir==NULL){
    strcpy(buffer,filename);
  }
  else{
    strcpy(buffer,source_dir);
    strcat(buffer,filename);
  }
  if(filesize!=NULL)*filesize=0;
  statfile=STAT(buffer,&statbuffer);
  if(statfile!=0)return statfile;
  if(filesize!=NULL)*filesize=statbuffer.st_size;
  return statfile;
}

/* ------------------ GetFileSizeSMV ------------------------ */

FILE_SIZE GetFileSizeSMV(const char *filename){
  STRUCTSTAT statbuffer;
  int statfile;
  FILE_SIZE return_val;

  return_val=0;
  if(filename==NULL)return return_val;
  statfile=STAT(filename,&statbuffer);
  if(statfile!=0)return return_val;
  return_val = statbuffer.st_size;
  return return_val;
}

/* ------------------ fread_mt ------------------------ */

void *fread_mt(void *mtfileinfo){
  FILE_SIZE file_beg, buffer_beg, file_end, buffer_size, file_size;
  FILE *stream;
  int i, nthreads;
  char *file;
  unsigned char *buffer;
  mtfiledata *mtf;
  FILE_SIZE file_offset, nchars;

  mtf = (mtfiledata *)mtfileinfo;

  i              = mtf->i;
  nthreads       = mtf->nthreads;
  file           = mtf->file;
  buffer         = mtf->buffer;
  file_size      = mtf->file_size;
  file_offset    = mtf->file_offset;
  nchars         = mtf->nchars;

  buffer_size = nchars/nthreads;
  buffer_beg  = i*buffer_size;
  file_beg    = file_offset + buffer_beg;
  file_end    = file_beg + buffer_size - 1;
  if(i == nthreads - 1||file_end>file_size-1){
    file_end    = file_size - 1;
    buffer_size = file_end + 1 - file_beg;
  }
  stream = fopen(file, "rb");
  if(stream == NULL){
#ifdef pp_THREAD
    if(nthreads>1)pthread_exit(NULL);
#endif
    return NULL;
  }
  FSEEK(stream, file_beg, SEEK_SET);
  mtf->chars_read = fread(buffer + buffer_beg, 1, buffer_size, stream);
  fclose(stream);

#ifdef pp_THREAD
  if(nthreads>1)pthread_exit(NULL);
#endif
  return NULL;
}

/* ------------------ SetMtFileInfo ------------------------ */

mtfiledata *SetMtFileInfo(char *file, unsigned char *buffer, FILE_SIZE file_offset, FILE_SIZE nchars, int nthreads){
  mtfiledata *mtfileinfo;
  int i;

  NewMemory((void **)&mtfileinfo,nthreads*sizeof(mtfiledata));

  for(i=0;i<nthreads;i++){
    mtfiledata *mti;

    mti = mtfileinfo + i;
    mti->i               = i;
    mti->nthreads        = nthreads;
    mti->file            = file;
    mti->buffer          = buffer;
    mti->file_size       = nchars;
    mti->file_offset     = file_offset;
    mti->nchars          = nchars;
    mti->chars_read      = 0;
  }
  return mtfileinfo;
}

/* ------------------ MakeFile ------------------------ */

#define BUFFERSIZE 1000000
int MakeFile(char *file, int size){
  unsigned char *buffer;
  FILE *stream;
  int i;

  if(file == NULL || strlen(file) == 0)return 0;
  stream = fopen(file, "w");
  if(stream == NULL)return 0;

  NewMemory((void **)&buffer, BUFFERSIZE);
  for(i = 0; i < BUFFERSIZE; i++){
    buffer[i] = i % 255;
  }
  for(i = 0; i < size; i++){
    fwrite(buffer, 1, BUFFERSIZE, stream);
  }
  fclose(stream);

  FREEMEMORY(buffer);
  return 1;
}

/* ------------------ fread_p ------------------------ */

FILE_SIZE fread_p(char *file, unsigned char *buffer, FILE_SIZE offset, FILE_SIZE nchars, int nthreads){
  FILE_SIZE chars_read;
  mtfiledata *mtfileinfo;

  mtfileinfo = SetMtFileInfo(file, buffer, offset, nchars, nthreads);
  if(nthreads == 1){
    FILE *stream;

    stream = fopen(file, "rb");
    if(stream == NULL)return 0;
    chars_read = fread(buffer, 1, mtfileinfo->file_size, stream);
    fclose(stream);
  }
#ifdef pp_THREAD
  else{
    threaderdata *read_threads;
    int use_read_threads, i;

    use_read_threads = 1;
    read_threads = THREADinit(&nthreads, &use_read_threads, fread_mt);
    THREADruni(read_threads, (unsigned char *)mtfileinfo, sizeof(mtfiledata));
    THREADcontrol(read_threads, THREAD_JOIN);
    chars_read = 0;
    for(i = 0;i < nthreads;i++){
      chars_read += mtfileinfo[i].chars_read;
    }
  }
#else
  else{
    int i;

    chars_read = 0;
    for(i = 0;i < nthreads;i++){
      mtfiledata *mti;

      mti = mtfileinfo + i;
      fread_mt(mti);
      chars_read += mti->chars_read;
    }
  }
#endif
  return chars_read;
}


/* ------------------ PrintTime ------------------------ */

void PrintTime(const char *filepath, int line, float *timer, const char *label, int stop_flag){
  char *file;

  if(show_timings == 0)return;
  file = strrchr(filepath, '\\');
  if(file == NULL)file = strrchr(filepath, '/');
  if(file == NULL){
    file = (char *)filepath;
  }
  else{
    file++;
  }
  if(label != NULL){
    if(stop_flag == 1)STOP_TIMER(*timer);
    if(*timer > 0.1)fprintf(stderr, "%s/%i/%s %.1f s\n", file, line, label, *timer);
  }
  START_TIMER(*timer);
}

/* ------------------ InitBufferData ------------------------ */

bufferdata *InitBufferData(char *file, char *size_file, int *options){
  bufferdata *buffinfo = NULL;
  unsigned char *buffer = NULL;
  FILE_SIZE nbuffer = 0;

  NewMemory((void **)&buffinfo, sizeof(bufferdata));
  buffinfo->file = file;
  buffinfo->size_file = size_file;
  buffinfo->options = options;
  if(options != NULL && options[0]>0){
    int *optionsptr;

    NewMemory((void **)&optionsptr, (options[0]+1)*sizeof(int));
    memcpy(optionsptr, options, (options[0]+1)*sizeof(int));
    buffinfo->options = optionsptr;
  }
  nbuffer = GetFileSizeSMV(file);
  NewMemory((void **)&buffer, nbuffer);
  buffinfo->buffer   = buffer;
  buffinfo->nbuffer  = nbuffer;
  buffinfo->nfile    = 0;
  return buffinfo;
}

/* ------------------ FreeBufferInfo ------------------------ */

void FreeBufferInfo(bufferdata *bufferinfo){
  if(bufferinfo == NULL)return;
  FREEMEMORY(bufferinfo->buffer);
  FREEMEMORY(bufferinfo->options);
  FREEMEMORY(bufferinfo);
}

/* ------------------ File2Buffer ------------------------ */

bufferdata *File2Buffer(char *file, char *size_file, int *options, bufferdata *bufferinfo,  FILE_SIZE *nreadptr){
  FILE_SIZE nfile=0, offset_buffer = 0, offset_file = 0, nread_actual, nread_try;

  *nreadptr = 0;
  if(file==NULL || strlen(file)==0 || FileExistsOrig(file) == 0)return NULL;

  INIT_PRINT_TIMER(timer_file2buffer);
  if(bufferinfo == NULL){ // read entire file
    bufferinfo     = InitBufferData(file, size_file, options);
    offset_file    = 0;
    offset_buffer  = 0;
    nread_try      = bufferinfo->nbuffer;
  }
  else{ // read in part of file that was not read in previously
    unsigned char *buffer;

    buffer  = bufferinfo->buffer;
    nfile   = GetFileSizeSMV(file);
    if(nfile == 0){
      FreeBufferInfo(bufferinfo);
      *nreadptr = 0;
      return NULL;
    }
    if(buffer!=NULL&&nfile == bufferinfo->nfile){ // file hasn't changed so nothing more to read in
      PRINT_TIMER(timer_file2buffer, "File2Buffer");
      *nreadptr = 0;
      return bufferinfo;
    }
    if(buffer == NULL){
      NewMemory((void **)&buffer, nfile*sizeof(unsigned char));
      offset_file   = 0;
      offset_buffer = 0;
    }
    else{
      ResizeMemory((void **)&buffer, nfile);
      offset_file   = bufferinfo->nfile;
      offset_buffer = bufferinfo->nfile;
    }
    bufferinfo->buffer  = buffer;
    nread_try           = nfile - offset_file;
    bufferinfo->nbuffer = nfile;
  }
//  nread = fread_p(file, buffer, offset, delta, nthreads);

//#define XXXX
#ifdef XXXX
  FILE *stream;
#ifdef WIN32
  stream = _fsopen(file, "rb", _SH_DENYNO);
#else
  stream = fopen(file, "rb");
#endif
#endif

#ifndef XXXX
  FILE *stream;
  stream = fopen(file, "rb");
#endif
  if(stream == NULL){
    FreeBufferInfo(bufferinfo);
    return NULL;
  }
  if(offset_file!=0)fseek(stream, offset_file, SEEK_SET);
  nread_actual = fread(bufferinfo->buffer+offset_buffer, 1, nread_try, stream);
  fclose(stream);
  if(nread_actual != nread_try){
    FreeBufferInfo(bufferinfo);
    return NULL;
  }
  bufferinfo->nfile = nfile;
  PRINT_TIMER(timer_file2buffer, "File2Buffer");
  *nreadptr = nread_actual;
  return bufferinfo;
}

/* ------------------ FileExistsOrig ------------------------ */

int FileExistsOrig(char *filename){
  if(ACCESS(filename, F_OK) == -1){
    return NO;
  }
  else{
    return YES;
  }
}

  /* ------------------ FileExists ------------------------ */

int FileExists(char *filename, filelistdata *filelist, int nfilelist, filelistdata *filelist2, int nfilelist2){

// returns YES if the file filename exists, NO otherwise

  if(filename == NULL)return NO;
  if(filelist != NULL&&nfilelist>0){
    if(FileInList(filename, filelist, nfilelist, filelist2, nfilelist2) != NULL){
      return YES;
    }
  }
  if(ACCESS(filename,F_OK)==-1){
    return NO;
  }
  return YES;
}

/* ------------------ FreeFileList ------------------------ */

void FreeFileList(filelistdata *filelist, int *nfilelist){
  int i;

  for(i=0;i<*nfilelist;i++){
    FREEMEMORY(filelist[i].file);
  }
  FREEMEMORY(filelist);
  *nfilelist=0;
}

/* ------------------ GetFileListSize ------------------------ */

int GetFileListSize(const char *path, char *filter, int mode){
  struct dirent *entry;
  DIR *dp;
  int maxfiles=0;
  int d_type;

  if(path == NULL||filter==NULL)return maxfiles;
  dp = opendir(path);
  if(dp == NULL)return 0;
  d_type = DT_REG;
  if(mode==DIR_MODE)d_type = DT_DIR;
  while((entry = readdir(dp))!=NULL){
    if(((entry->d_type==d_type||entry->d_type==DT_UNKNOWN)&&MatchWild(entry->d_name,filter)==1)){
      if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)continue;
      maxfiles++;
    }
  }
  closedir(dp);
  return maxfiles;
}

/* ------------------ fopen_indir  ------------------------ */

FILE *fopen_indir(char *dir, char *file, char *mode){
  FILE *stream;

  if(file==NULL||strlen(file)==0)return NULL;
  if(dir==NULL||strlen(dir)==0){
#ifdef WIN32
    stream = _fsopen(file, mode, _SH_DENYNO);
#else
    stream = fopen(file,mode);
#endif
  }
  else{
    char *filebuffer;
    int lenfile;

    lenfile = strlen(dir)+1+strlen(file)+1;
    NewMemory((void **)&filebuffer,lenfile*sizeof(char));
    strcpy(filebuffer,dir);
    strcat(filebuffer,dirseparator);
    strcat(filebuffer,file);
#ifdef WIN32
    stream = _fsopen(filebuffer, mode, _SH_DENYNO);
#else
    stream = fopen(filebuffer, mode);
#endif
    FREEMEMORY(filebuffer);
  }
  return stream;
}

/* ------------------ GetScratchFilename ------------------------ */

char *GetScratchFilename(char *file){
  char *smokeview_scratchdir = GetUserConfigDir();
  char *fullfile;

  if(smokeview_scratchdir!=NULL){
    int len;

    len = strlen(file) + strlen(smokeview_scratchdir) + 2;
      NewMemory((void **)&fullfile,len);
      strcpy(fullfile, smokeview_scratchdir);
      strcat(fullfile, dirseparator);
      strcat(fullfile, file);
  }
  else{
    fullfile = file;
  }
  return fullfile;
}

/* ------------------ fopen_2dir_scratch ------------------------ */

FILE *fopen_2dir_scratch(char *file, char *mode) {
  char *smokeview_scratchdir = GetUserConfigDir();
  FILE *f = fopen_2dir(file, mode, smokeview_scratchdir);
  FREEMEMORY(smokeview_scratchdir);
  return f;
}

/* ------------------ fopen_3dir ------------------------ */

FILE *fopen_3dir(char *file, char *mode, char *dir1, char *dir2, char *dir3){
  FILE *stream = NULL;
  char buffer[4096];
  // try opening file in the current directory, dir1 then in dir2 then in dir3
  // (currently results direcrory defined by fds, current directory, scratch directory)

  if(file == NULL)return NULL;
  if(dir1 != NULL){
    strcpy(buffer, dir1);
    strcat(buffer, dirseparator);
    strcat(buffer, file);
#ifdef WIN32
    stream = _fsopen(buffer, mode, _SH_DENYNO);
#else
    stream = fopen(buffer, mode);
#endif
    if(stream!=NULL)return stream;
  }
  if(dir2 != NULL){
    strcpy(buffer, dir2);
    strcat(buffer, dirseparator);
    strcat(buffer, file);
#ifdef WIN32
    stream = _fsopen(buffer, mode, _SH_DENYNO);
#else
    stream = fopen(buffer, mode);
#endif
  }
  if(dir3 != NULL){
    strcpy(buffer, dir3);
    strcat(buffer, dirseparator);
    strcat(buffer, file);
#ifdef WIN32
    stream = _fsopen(buffer, mode, _SH_DENYNO);
#else
    stream = fopen(buffer, mode);
#endif
  }
  return stream;
}

/* ------------------ SetResultsDir ------------------------ */

char *SetResultsDir(char *file){
  char *dirsep, filecopy[1024], *results_dir;

  if(file==NULL)return NULL;
  strcpy(filecopy, file);
  dirsep = strrchr(filecopy, '/');
  if(dirsep == NULL)return NULL;
  dirsep[0] = 0;
  NewMemory((void **)&results_dir,strlen(filecopy)+1);
  strcpy(results_dir, filecopy);
  return results_dir;
}

/* ------------------ fopen_2dir ------------------------ */

FILE *fopen_2dir(char *file, char *mode, char *scratch_dir){
  FILE *stream;

  if(file == NULL)return NULL;
#ifdef WIN32
  stream = _fsopen(file,mode,_SH_DENYNO);
#else
  stream = fopen(file,mode);
#endif
  if(stream == NULL && scratch_dir != NULL){
    stream = fopen_indir(scratch_dir, file, mode);
  }
  return stream;
}


/* ------------------ CompareFileList ------------------------ */

int CompareFileList(const void *arg1, const void *arg2){
  filelistdata *x, *y;

  x = (filelistdata *)arg1;
  y = (filelistdata *)arg2;

  return strcmp(x->file, y->file);
}

/* ------------------ FileInList ------------------------ */

filelistdata *FileInList(char *file, filelistdata *filelist, int nfiles, filelistdata *filelist2, int nfiles2){
  filelistdata *entry=NULL, fileitem;

  if(file==NULL)return NULL;
  fileitem.file = file;
  fileitem.type = 0;
  if(filelist!=NULL&&nfiles>0){
    entry = bsearch(&fileitem, (filelistdata *)filelist, (size_t)nfiles, sizeof(filelistdata), CompareFileList);
    if(entry!=NULL)return entry;
  }
  if(filelist2!=NULL&&nfiles2>0){
    entry = bsearch(&fileitem, (filelistdata *)filelist2, (size_t)nfiles2, sizeof(filelistdata), CompareFileList);
  }
  return entry;
}

/* ------------------ MakeFileList ------------------------ */

int MakeFileList(const char *path, char *filter, int maxfiles, int sort_files, filelistdata **filelist, int mode){
  struct dirent *entry;
  DIR *dp;
  int nfiles=0;
  filelistdata *flist;
  int d_type;

  // DT_DIR - is a directory
  // DT_REG - is a regular file

  if(maxfiles == 0||path==NULL||filter==NULL){
    *filelist = NULL;
    return 0;
  }
  dp = opendir(path);
  *filelist=NULL;
  if(dp == NULL)return 0;
  NewMemory((void **)&flist,maxfiles*sizeof(filelistdata));
  d_type = DT_REG;
  if(mode==DIR_MODE)d_type = DT_DIR;
  while((entry = readdir(dp))!=NULL&&nfiles<maxfiles){
    if((entry->d_type==d_type||entry->d_type==DT_UNKNOWN)&&MatchWild(entry->d_name,filter)==1){
      char *file;
      filelistdata *flisti;

      if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)continue;
      flisti = flist + nfiles;
      if(mode == DIR_MODE){
        NewMemory((void **)&file, strlen(path)+1+strlen(entry->d_name) + 1);
      }
      else{
        NewMemory((void **)&file, strlen(entry->d_name) + 1);
      }
      strcpy(file, "");
      if(mode == DIR_MODE){
        strcat(file, path);
        strcat(file, dirseparator);
      }
      strcat(file,entry->d_name);
      flisti->file=file;
      flisti->type=0;
      nfiles++;
    }
  }
  closedir(dp);
  if(sort_files == YES&&nfiles>0){
    qsort((filelistdata *)flist, (size_t)nfiles, sizeof(filelistdata), CompareFileList);
  }
  *filelist=flist;
  return nfiles;
}

/* ------------------ GetFileSizeLabel ------------------------ */

void GetFileSizeLabel(int size, char *sizelabel){
  int leftsize,rightsize;

#define sizeGB   1000000000
#define size100MB 100000000
#define size10MB   10000000
#define sizeMB     1000000
#define size100KB    100000
#define size10KB      10000

  if(size>=sizeGB){
    size/=size10MB;
    leftsize=size/100;
    rightsize=size-100*leftsize;
    sprintf(sizelabel,"%i.%02i GB",leftsize,rightsize);
  }
  else if(size>=size100MB&&size<sizeGB){
    size/=sizeMB;
    leftsize=size;
    sprintf(sizelabel,"%i MB",leftsize);
  }
  else if(size>=size10MB&&size<size100MB){
    size/=size100KB;
    leftsize=size/10;
    rightsize=size-10*leftsize;
    sprintf(sizelabel,"%i.%i MB",leftsize,rightsize);
  }
  else if(size>=sizeMB&&size<size10MB){
    size/=size10KB;
    leftsize=size/100;
    rightsize=size-100*leftsize;
    sprintf(sizelabel,"%i.%02i MB",leftsize,rightsize);
  }
  else if(size>=size100KB&&size<sizeMB){
    size/=1000;
    leftsize=size;
    sprintf(sizelabel,"%i KB",leftsize);
  }
  else{
    size/=10;
    leftsize=size/100;
    rightsize=size-100*leftsize;
    sprintf(sizelabel,"%i.%02i KB",leftsize,rightsize);
  }
}

/* ------------------ GetFloatFileSizeLabel ------------------------ */

char *GetFloatFileSizeLabel(float size, char *sizelabel){
  int leftsize, rightsize;

  if(size>=sizeGB){
    size /= size10MB;
    leftsize = size/100;
    rightsize = size-100*leftsize;
    sprintf(sizelabel, "%i.%02i GB", leftsize, rightsize);
  }
  else if(size>=size100MB&&size<sizeGB){
    size /= sizeMB;
    leftsize = size;
    sprintf(sizelabel, "%i MB", leftsize);
  }
  else if(size>=size10MB&&size<size100MB){
    size /= size100KB;
    leftsize = size/10;
    rightsize = size-10*leftsize;
    sprintf(sizelabel, "%i.%i MB", leftsize, rightsize);
  }
  else if(size>=sizeMB&&size<size10MB){
    size /= size10KB;
    leftsize = size/100;
    rightsize = size-100*leftsize;
    sprintf(sizelabel, "%i.%02i MB", leftsize, rightsize);
  }
  else if(size>=size100KB&&size<sizeMB){
    size /= 1000;
    leftsize = size;
    sprintf(sizelabel, "%i KB", leftsize);
  }
  else{
    size /= 10;
    leftsize = size/100;
    rightsize = size-100*leftsize;
    sprintf(sizelabel, "%i.%02i KB", leftsize, rightsize);
  }
  return sizelabel;
}

#ifdef _WIN32
char *CombinePaths(const char *path_a, const char *path_b){
  char *path_out;
  NEWMEMORY(path_out, sizeof(char) * MAX_PATH);
  // NB: This uses on older function in order to support "char *".
  // PathAllocCombine would be better but requires switching to "wchar *".
  char *result = PathCombineA(path_out, path_a, path_b);
  if(result == NULL) FREEMEMORY(path_out);
  return result;
}
#else
char *CombinePaths(const char *path_a, const char *path_b) {
  char *path_out;
  size_t path_a_len = strlen(path_a);
  size_t path_b_len = strlen(path_b);
  size_t new_len = path_a_len + 1 + path_b_len;
  NEWMEMORY(path_out, sizeof(char) * (new_len + 1));
  STRCPY(path_out, path_a);
  path_out[path_a_len] = '/';
  path_out[path_a_len+1] = '\0';
  STRCAT(path_out, path_b);
  path_out[new_len] = '\0';
  return path_out;
}
#endif

/* ------------------ GetBinPath ------------------------ */
#ifdef _WIN32
char *GetBinPath(){
  size_t max_buffer_size = MAX_PATH * 20;
  char *buffer;
  size_t buffer_size = MAX_PATH * sizeof(char);
  NEWMEMORY(buffer, buffer_size);
  for(;;){
    GetModuleFileNameA(NULL, buffer, buffer_size);
    DWORD dw = GetLastError();
    if(dw == ERROR_SUCCESS){
      return buffer;
    }
    else if(dw == ERROR_INSUFFICIENT_BUFFER && buffer_size < max_buffer_size){
      // increase buffer size by a factor of 2
      buffer_size *= 2;
      RESIZEMEMORY(buffer, buffer_size);
    }
    else{
      FREEMEMORY(buffer);
      return NULL;
    }
  }
}
#elif __linux__
char *GetBinPath(){
  size_t max_buffer_size = 2048 * 20;
  char *buffer;
  size_t buffer_size = 256 * sizeof(char);
  NEWMEMORY(buffer, buffer_size);
  for(;;){
    int ret = readlink("/proc/self/exe", buffer, buffer_size);
    if(ret < buffer_size){
      buffer[ret] = '\0';
      return buffer;
    }
    else if(ret == buffer_size && buffer_size < max_buffer_size) {
      // increase buffer size by a factor of 2
      buffer_size *= 2;
      RESIZEMEMORY(buffer, buffer_size);
    }
    else{
      FREEMEMORY(buffer);
      return NULL;
    }
  }
}
#else
char *GetBinPath(){
  uint32_t  max_buffer_size = 2048 * 20;
  char *buffer;
  uint32_t buffer_size = 256 * sizeof(char);
  NEWMEMORY(buffer, buffer_size);
  for(;;){
    int ret = _NSGetExecutablePath(buffer, &buffer_size);
    if(ret == 0){
      return buffer;
    }
    else if(ret == -1 && buffer_size < max_buffer_size){
      // buffer_size has been set to the required buffer size by
      // _NSGetExecutablePath
      RESIZEMEMORY(buffer, buffer_size);
    }
    else{
      FREEMEMORY(buffer);
      return NULL;
    }
  }
}
#endif

/* ------------------ GetBinDir ------------------------ */
#ifdef _WIN32
char *GetBinDir(){
  char *buffer = GetBinPath();
  // NB: This uses on older function in order to support "char *".
  // PathCchRemoveFileSpec would be better but requires switching to "wchar *".
  PathRemoveFileSpecA(buffer);
  PathAddBackslashA(buffer);
  return buffer;
}
#elif __linux__
char *GetBinDir(){
  char *buffer = GetBinPath();
  dirname(buffer);
  int pathlen = strlen(buffer);
  RESIZEMEMORY(buffer, pathlen + 2);
  buffer[pathlen] = '/';
  buffer[pathlen + 1] = '\0';
  return buffer;
}
#else
char *GetBinDir(){
  char *buffer = GetBinPath();
  // The BSD and OSX version of dirname uses an internal buffer, therefore we
  // need to copy the string out.
  char *dir_buffer = dirname(buffer);
  int pathlen = strlen(buffer);
  RESIZEMEMORY(buffer, (pathlen + 2) * sizeof(char));
  STRCPY(buffer, dir_buffer);
  buffer[pathlen] = '/';
  buffer[pathlen + 1] = '\0';
  return buffer;
}
#endif

/// @brief Stored the value of the -bindir commandline option. NULL if that
/// options is not used. Only referenced by @ref SetSmvRootOverride and @ref
/// GetSmvRootDir.
char *smv_root_override = NULL;

/* ------------------ SetSmvRootOverride ------------------------ */

void SetSmvRootOverride(const char *path){
  FREEMEMORY(smv_root_override);
  if(path == NULL) return;
  size_t len = strlen(path);
  NEWMEMORY(smv_root_override, (len + 2) * sizeof(char));
  STRCPY(smv_root_override, path);
  if(path[len - 1] != dirseparator[0]){
    STRCAT(smv_root_override, dirseparator);
  }
}

/* ------------------ GetSmvRootDir ------------------------ */

char *GetSmvRootDir(){
  char *envar_path, *buffer;
  int len;

  envar_path = getenv("SMV_ROOT_OVERRIDE");

  if(smv_root_override != NULL){
    // Take the SMV_ROOT as defined on the command line
    len = strlen(smv_root_override);
    NEWMEMORY(buffer, (len + 2) * sizeof(char));
    STRCPY(buffer, smv_root_override);
  }
  else if(envar_path != NULL){
    // Take the SMV_ROOT as defined by the SMV_ROOT_OVERRIDE environment
    // variable

    len = strlen(envar_path);
    NEWMEMORY(buffer, (len + 2) * sizeof(char));
    STRCPY(buffer, envar_path);
  }
  else{
#ifdef SMV_ROOT_OVERRIDE
    // Take the SMV_ROOT as defined by the SMV_ROOT_OVERRIDE macro
    len = strlen(SMV_ROOT_OVERRIDE);
    NEWMEMORY(buffer, (len + 2) * sizeof(char));
    STRCPY(buffer, SMV_ROOT_OVERRIDE);
#else
    // Otherwise simply return the directory of the running executable (using
    // the platform-dependent code).
    char *bindir, repo_bindir[1024];
    FILE *stream1=NULL, *stream2=NULL;

    bindir =  GetBinDir();
    if(bindir == NULL)return NULL;

    strcpy(repo_bindir, bindir);
    if(strcmp(bindir+strlen(bindir)-1,dirseparator)!=0)STRCAT(repo_bindir, dirseparator);

    int i, count=0;

    for(i = strlen(repo_bindir) - 1;i >= 0;i--){
      if(repo_bindir[i] == dirseparator[0]){
        count++;
        if(count == 3){
          repo_bindir[i] = 0;
          strcat(repo_bindir, dirseparator);
          strcat(repo_bindir, "for_bundle");
          strcat(repo_bindir, dirseparator);
          break;
        }
      }
    }

    stream1 = fopen_indir(bindir, ".smokeview_bin", "r");
    if(stream1 == NULL && count==3)stream2 = fopen_indir(repo_bindir, ".smokeview_bin", "r");

    if(stream1 != NULL || stream2 == NULL){
      len = strlen(bindir);
      NEWMEMORY(buffer, len + 2);
      STRCPY(buffer, bindir);
    }
    else{ // look for root directory in ../../for_bundle
          //  this is used when using smokeview located in the build directory
      len = strlen(repo_bindir);
      NEWMEMORY(buffer, len + 2);
      STRCPY(buffer, repo_bindir);
    }
    if(stream1!=NULL)fclose(stream1);
    if(stream2!=NULL)fclose(stream2);
#endif
  }
  len = strlen(buffer);
  if(strcmp(buffer+len-1,dirseparator)!=0)STRCAT(buffer, dirseparator);
  return buffer;
}

/* ------------------ GetSmvRootSubPath ------------------------ */

char *GetSmvRootSubPath(const char *subdir) {
  char *root_dir = GetSmvRootDir();
  if (root_dir == NULL || subdir == NULL) return NULL;
  return CombinePaths(root_dir,subdir);
}

/* ------------------ GetHomeDir ------------------------ */

char *GetHomeDir() {
#ifdef WIN32
  char *homedir_env = getenv("userprofile");
#else
  char *homedir_env = getenv("HOME");
#endif
  if(homedir_env == NULL) homedir_env = ".";
  // For consistency allocate path using NEWMEMORY
  char *homedir;
  NEWMEMORY(homedir, sizeof(char) * (strlen(homedir_env) + 1));
  STRCPY(homedir, homedir_env);
  return homedir;
}

/* ------------------ GetUserConfigDir ------------------------ */

char *GetUserConfigDir() {
  char *homedir = GetHomeDir();
  if(homedir == NULL) return NULL;
  char *config_path = CombinePaths(homedir, ".smokeview");
  FREEMEMORY(homedir);
  return config_path;
}

/* ------------------ GetUserConfigSubPath ------------------------ */

char *GetUserConfigSubPath(const char *subdir) {
  char *config_dir = GetUserConfigDir();
  if (config_dir == NULL || subdir == NULL) return NULL;
  return CombinePaths(config_dir,subdir);
}

/* ------------------ GetSystemIniPath ------------------------ */

char *GetSystemIniPath() {
  return GetSmvRootSubPath("smokeview.ini");
}

/* ------------------ GetUserIniPath ------------------------ */

char *GetUserIniPath() {
  return GetUserConfigSubPath("smokeview.ini");
}

/* ------------------ GetUserColorbarDirPath ------------------------ */

char *GetUserColorbarDirPath() {
  return GetUserConfigSubPath("colorbars");
}

/* ------------------ GetSmokeviewHtmlPath ------------------------ */

char *GetSmokeviewHtmlPath() {
  return GetSmvRootSubPath("smokeview.html");
}

/* ------------------ GetSmokeviewHtmlVrPath ------------------------ */

// TODO: This is currently unused
char *GetSmokeviewHtmlVrPath() {
  return GetSmvRootSubPath("smokeview_vr.html");
}

/* ------------------ GetSmvScreenIni ------------------------ */

// TODO: This is currently unused
char *GetSmvScreenIni() {
  return GetSmvRootSubPath("smv_screen.ini");
}



/* ------------------ GetSmvRootFile ----------------------- */

char *GetSmvRootFile(const char *path) {
  char *root_path = GetSmvRootDir();
  char *result = CombinePaths(root_path, path);
  FREEMEMORY(root_path);
  return result;
}

/* ------------------ GetSmvUserDir ------------------------ */

char *GetSmvUserDir() {
  char *home_path = GetHomeDir();
  char *result = CombinePaths(home_path, ".smokeview");
  FREEMEMORY(home_path);
  return result;
}

/* ------------------ GetSmvUserFile ----------------------- */

char *GetSmvUserFile(const char *path) {
  char *user_path = GetSmvUserDir();
  char *result = CombinePaths(user_path, path);
  FREEMEMORY(user_path);
  return result;
}


/* ------------------ IsSootFile ------------------------ */

int IsSootFile(char *shortlabel, char *longlabel){
  if(STRCMP(shortlabel, "rho_C")==0)return 1;
  if(STRCMP(shortlabel, "rho_Soot")==0)return 1;
  if(STRCMP(shortlabel, "rho_C0.9H0.1")==0)return 1;
  if(strlen(longlabel)>=12&&strncmp(longlabel, "SOOT DENSITY",12)==0)return 1;
  return 0;
}

/* ------------------ LastName ------------------------ */

char *LastName(char *argi){

// returns the file name contained in the full path name argi

  char *lastdirsep;
  char *dir, *filename, cwdpath[1000];

  filename=argi;
  lastdirsep=strrchr(argi,SEP);
  if(lastdirsep!=NULL){
    dir=argi;
    filename=lastdirsep+1;
    lastdirsep[0]=0;
    GETCWD(cwdpath,1000);
    if(strcmp(cwdpath,dir)!=0){
      CHDIR(dir);
    }
  }
  return filename;
}

/* ------------------ GetZoneFileName ------------------------ */

char *GetZoneFileName(char *bufptr){
  char *full_name, *last_name, *filename;

  full_name=bufptr;
  if(FILE_EXISTS(full_name)==NO)full_name=NULL;

  last_name= LastName(bufptr);
  if(FILE_EXISTS(last_name)==NO)last_name=NULL;

  if(last_name!=NULL&&full_name!=NULL){
    if(strcmp(last_name,full_name)==0){
      last_name=NULL;
    }
  }

  if(last_name!=NULL&&full_name!=NULL){
    filename=last_name;
  }
  else if(last_name==NULL&&full_name!=NULL){
    filename=full_name;
  }
  else if(last_name!=NULL&&full_name==NULL){
    filename=last_name;
  }
  else{
    filename=NULL;
  }
  return filename;
}

/* ------------------ FileModtime ------------------------ */

time_t FileModtime(char *filename){

// returns the modification time of the file named filename

  STRUCTSTAT statbuffer;
  time_t return_val;
  int statfile;

  return_val=0;
  if(filename==NULL)return return_val;
  statfile=STAT(filename,&statbuffer);
  if(statfile!=0)return return_val;
  return_val = statbuffer.st_mtime;
  return return_val;
}

/* ------------------ Which ------------------------ */

char *Which(char *progname, char **fullprognameptr){

// returns the PATH directory containing the file progname

  char *pathlist, *pathlistcopy, *fullprogname, *prognamecopy;
  char *dir,*pathentry;
  char pathsep[2], dirsep[2];

#ifdef WIN32
  strcpy(pathsep,";");
  strcpy(dirsep,"\\");
#else
  strcpy(pathsep,":");
  strcpy(dirsep,"/");
#endif

  pathlist = getenv("PATH");
  if(pathlist==NULL||strlen(pathlist)==0||progname==NULL||strlen(progname)==0)return NULL;

  NewMemory((void **)&prognamecopy, (unsigned int)(strlen(progname)+4+1));
  strcpy(prognamecopy, progname);

  NewMemory((void **)&pathlistcopy, (unsigned int)(strlen(pathlist)+1));
  strcpy(pathlistcopy, pathlist);

#ifdef WIN32
  {
    const char *ext;

    ext = prognamecopy+strlen(progname)-4;
    if(strlen(progname)<=4|| (STRCMP(ext,".exe")!=0 && STRCMP(ext, ".bat") != 0))strcat(prognamecopy, ".exe");
  }
#endif

  NewMemory((void **)&fullprogname, (unsigned int)(strlen(progname)+4+strlen(dirsep)+strlen(pathlist)+1));

  dir=strtok(pathlistcopy,pathsep);
  while(dir!=NULL&&strlen(dir)>0){
    strcpy(fullprogname,dir);
    strcat(fullprogname,dirsep);
    strcat(fullprogname,prognamecopy);
    if(FILE_EXISTS(fullprogname)==YES){
      NewMemory((void **)&pathentry,(unsigned int)(strlen(dir)+2));
      strcpy(pathentry,dir);
      strcat(pathentry,dirsep);
      FREEMEMORY(pathlistcopy);
      if(fullprognameptr != NULL){
        *fullprognameptr = fullprogname;
      }
      else{
        FREEMEMORY(fullprogname);
      }
      FREEMEMORY(prognamecopy);
      return pathentry;
    }
    dir=strtok(NULL,pathsep);
  }
  FREEMEMORY(pathlistcopy);
  FREEMEMORY(fullprogname);
  FREEMEMORY(prognamecopy);
  return NULL;
}
