#ifndef FILE_FOPEN_H_DEFINED
#define FILE_FOPEN_H_DEFINED
#ifdef pp_OPEN_TEST
#ifdef pp_THREAD
#include <pthread.h>
#endif

//*** structure

typedef struct _opendata{
  char file[1024], source[1024];
  int line;
  FILE *stream;
} opendata;

//*** headers

EXTERNCPP void AddOpenFile(const char *file, FILE * stream, char *source, int line);
EXTERNCPP void InitOpenTest(void);
EXTERNCPP FILE *fopen_counting(const char *path, const char *mode, char *source, int line);
EXTERNCPP int fclose_counting(FILE *fp);

//*** preprocessing directives

#ifdef pp_THREAD
#define LOCK_FOPEN   pthread_mutex_lock(&fopen_mutex)
#define UNLOCK_FOPEN pthread_mutex_unlock(&fopen_mutex)
#else
#define LOCK_FOPEN
#define UNLOCK_FOPEN
#endif

//*** variables

#ifdef IN_FOPEN
int open_files=0, nopeninfo=0;
opendata *openinfo = NULL;
#ifdef pp_THREAD
pthread_mutex_t fopen_mutex;
#endif

#else

extern int open_files, nopeninfo;
extern opendata *openinfo;
#ifdef pp_THREAD
extern pthread_mutex_t fopen_mutex;
#endif
#endif
#endif
#endif
