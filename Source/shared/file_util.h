#ifndef FILE_UTIL_H_DEFINED
#define FILE_UTIL_H_DEFINED

// vvvvvvvvvvvvvvvvvvvvvvvv header files vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#include <time.h>
#ifdef __MINGW32__
#include <stdio.h>
#include "options.h"
#endif
#ifdef pp_GCC
#include <unistd.h>
#endif
#ifdef pp_LINUX
#include <unistd.h>
#endif
#ifndef WIN32
#include <sys/stat.h>
#endif

// vvvvvvvvvvvvvvvvvvvvvvvv structures vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

/* --------------------------  filelistdata ------------------------------------ */

typedef struct {
  char *file;
  int type;
} filelistdata;

// vvvvvvvvvvvvvvvvvvvvvvvv preprocessing directives vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#ifdef WIN32
#define UNLINK _unlink
#else
#define UNLINK unlink
#endif

#ifdef X64
#define FSEEK(a,b,c) _fseeki64(a,b,c)
#define FTELL(a) _ftelli64(a)
#else
#define FSEEK(a,b,c) fseeko(a,b,c)
#define FTELL(a) ftello(a)
#endif

#define REPLACE_FILE 0
#define APPEND_FILE 1

#define NOT_FORCE_IN_DIR 0
#define FORCE_IN_DIR 1

#define FEOF(stream)              feof_buffer(stream->fileinfo)
#define FGETS(buffer,size,stream) fgets_buffer(stream->fileinfo,buffer,size)
#define REWIND(stream)            rewind_buffer(stream->fileinfo)
#define FCLOSE(stream)            fclose_buffer(stream->fileinfo)

#define BFILE bufferstreamdata

#define FILE_EXISTS(a)         FileExists(a,NULL,0,NULL,0)
#define FILE_EXISTS_CASEDIR(a) FileExists(a,filelist_casename, nfilelist_casename,filelist_casedir,nfilelist_casedir)
int FileExistsOrig(char *filename);

#ifdef WIN32
#define MKDIR(a) CreateDirectory(a,NULL)
#else
#define MKDIR(a) mkdir(a,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#endif

#ifdef WIN32
#define ACCESS _access
#define F_OK 0
#define W_OK 2
#else
#define ACCESS access
#endif

#ifndef NO
#define NO 0
#endif

#ifndef YES
#define YES 1
#endif

#ifdef WIN32
#define CHDIR _chdir
#define GETCWD _getcwd
#define SEP '\\'
#else
#define CHDIR chdir
#define GETCWD getcwd
#define SEP '/'
#endif


// vvvvvvvvvvvvvvvvvvvvvvvv headers vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

EXTERNCPP FILE *fopen_indir(char *dir, char *file, char *mode);

EXTERNCPP void TestWrite(char *scratchdir, char **fileptr);
EXTERNCPP int FFLUSH(void);
EXTERNCPP int PRINTF(const char * format, ...);
EXTERNCPP void SetStdOut(FILE *stream);
EXTERNCPP void GetFileSizeLabel(int size, char *sizelabel);
EXTERNCPP char *GetFloatFileSizeLabel(float size, char *sizelabel);
EXTERNCPP void CopyFILE(char *destdir, char *filein, char *fileout, int mode);
EXTERNCPP char *GetSmokeZipPath(char *progdir);
EXTERNCPP int IfFirstLineBlank(char *file);
EXTERNCPP int HaveProg(char *prog);
EXTERNCPP int FileCat(char *file_in1, char *file_in2, char *file_out);
EXTERNCPP unsigned int StreamCopy(FILE *stream_in, FILE *stream_out, int flag);
EXTERNCPP void FileCopy(char *file_in, char *file_out);
EXTERNCPP void MakeOutFile(char *outfile, char *destdir, char *file1, char *ext);
EXTERNCPP void FullFile(char *fileout, char *dir, char *file);
EXTERNCPP char *GetFileName(char *temp_dir, char *file, int force_in_temp_dir);
EXTERNCPP char *GetBaseFileName(char *buffer, char *file);

EXTERNCPP char *SetDir(char *argdir);
EXTERNCPP int GetFileInfo(char *filename, char *sourcedir, FILE_SIZE *filesize);
EXTERNCPP char *GetZoneFileName(char *buffer);
EXTERNCPP int Writable(char *dir);

EXTERNCPP   int FileExists(char *filename, filelistdata *filelist, int nfiles, filelistdata *filelist2, int nfiles2);
EXTERNCPP filelistdata *FileInList(char *file, filelistdata *filelist, int nfiles, filelistdata *filelist2, int nfiles2);
EXTERNCPP void FreeFileList(filelistdata *filelist, int *nfilelist);
EXTERNCPP int GetFileListSize(const char *path, char *filter) ;
EXTERNCPP int MakeFileList(const char *path, char *filter, int maxfiles, int sort_files, filelistdata **filelist);
EXTERNCPP char *Which(char *progname);
EXTERNCPP FILE_SIZE GetFileSizeSMV(const char *filename);
EXTERNCPP time_t FileModtime(char *filename);
EXTERNCPP int IsFileNewer(char *file1, char *file2);
EXTERNCPP char *GetProgDir(char *progname, char **svpath);

#ifdef pp_LUA
EXTERNCPP char *getprogdirabs(char *progname, char **svpath);
#endif

EXTERNCPP char *LastName(char *argi);

// vvvvvvvvvvvvvvvvvvvvvvvv variables vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#ifndef STREXTERN
#ifdef WIN32
STREXTERN char STRDECL(dirseparator[],"\\");
#else
STREXTERN char STRDECL(dirseparator[],"/");
#endif
#endif

#endif
