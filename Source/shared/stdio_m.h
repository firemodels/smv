#ifndef STDIO_M_H_DEFINED
#define STDIO_M_H_DEFINED

typedef struct {
  char *file;
  FILE *stream;
  unsigned char *buffer, *buffer_beg, *buffer_end;
} FILE_m;

#define PASS_m 0
#define FAIL_m 1

#ifndef FFALSE
#define FFALSE 0
#endif

#ifndef FORTREAD_m
#define FORTREAD_m(a,b,c,d)  fseek_m(stream,4,SEEK_CUR);count_read=fread_m(a,b,c,d);fseek_m(stream,4,SEEK_CUR)
#endif
#ifndef FORTREAD_mv
#define FORTREAD_mv(a,b,c,d)  fseek_m(stream,4,SEEK_CUR);count_read=fread_mv(a,b,c,d);fseek_m(stream,4,SEEK_CUR)
#endif


void   fclose_b(FILE_m *stream_m);
void   fclose_m(FILE_m *stream_m);
char   *fgets_m(char *str, int num, FILE_m *stream_m);
int    feof_m(FILE_m *stream_m);
FILE_m *fopen_m(char *file, char *mode);
FILE_m *fopen_b(char *file, unsigned char *buffer, size_t nbuffer, char *mode);
FILE_m *fopen_mo(char *file, FILE_SIZE offset, FILE_SIZE size, char *mode);
size_t fread_m(void *ptr, size_t size, size_t nmemb, FILE_m *stream_m);
size_t fread_mv(void **ptr, size_t size, size_t nmemb, FILE_m *stream_m);
int    fseek_m(FILE_m *stream_m, long int offset, int whence);
int    fseek_m_long(FILE_m *stream_m, long long offset, int whence);
long   int ftell_m(FILE_m *stream_m);
void   rewind_m(FILE_m *stream_m);
#endif
