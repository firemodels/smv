
typedef struct {
  FILE *stream;
  unsigned char *file, *buffer, *buffer_base;
  unsigned int nbuffer;
} FILE_m;

FILE_m *fopen_m(char *file, char *mode);
void fclose_m(FILE_m *stream_m);
size_t fread_m(void *ptr, size_t size, size_t nmemb, FILE_m *stream_m);
int fseek_m(FILE_m *stream_m, long int offset, int whence);
long int ftell_m(FILE_m *stream_m);

