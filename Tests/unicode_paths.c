#include "options.h"

#include "getdata.h"

#include "dmalloc.h"

#include "file_util.h"
#include "string_util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int show_help;
int hash_option;
int show_version;
char append_string[1024];

static char *test_file_name_unicode = "test_файл_O’S.txt";
static char *test_file_name_unicode_new = "test_файл_O’S_new.txt";
static char *test_file_content_unicode = "test_file_content_unicode";

static char *test_file_name_ascii = "test_ascii.txt";
static char *test_file_content_ascii = "test_file_content_ascii";

int WriteTo(const char *path, const char *content) {
  FILE *stream = FOPEN(path, "wb");
  if(stream == NULL) {
    fprintf(stderr, "failed to open file\n");
    return 1;
  }
  int l = strlen(content);
  int n_written = fwrite(content, 1, l, stream);
  if(l != n_written) {
    perror("Error writing file");
    fprintf(stderr, "failed to write file\n");
    return 2;
  }
  return fclose(stream);
}

int ReadFrom(const char *path, const char *expected) {
  FILE *stream = FOPEN(path, "rb");
  if(stream == NULL) {
    fprintf(stderr, "failed to open file\n");
    return 1;
  }
  char buf[256];
  char *b = fgets(buf, 256, stream);
  if(b == NULL) {
    perror("Error reading file");
    fprintf(stderr, "failed to read file\n");
    return 2;
  }
  TrimBack(b);
  if(strcmp(b, expected) != 0) {
    fprintf(stderr,
            "file content doesn't match\n   found: \"%s\"\nexpected: \"%s\"\n",
            b, expected);
    return 3;
  }
  return fclose(stream);
}

int main(int argc, char **argv) {
  int err = 0;
  initMALLOC();
  char *test_file_dir = argv[1];
  {
    // Test reading of existing unicode file
    char *path = CombinePaths(test_file_dir, test_file_name_unicode);
    err = ReadFrom(path, test_file_content_unicode);
    if(err) return err;
  }
  {
    char *path = CombinePaths(".", test_file_name_unicode_new);
    // Test creation of new unicode file
    err = WriteTo(path, test_file_content_unicode);
    if(err) return err;
    // Test reading of new unicode file
    err = ReadFrom(path, test_file_content_unicode);
    if(err) return err;
    err = UNLINK(path);
    if(err) return err;
  }
  {
    // Test reading of existing ascii file
    char *path = CombinePaths(test_file_dir, test_file_name_ascii);
    err = ReadFrom(path, test_file_content_ascii);
    if(err) return err;
  }
  return err;
}
