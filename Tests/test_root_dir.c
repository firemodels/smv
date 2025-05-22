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

int main(int argc, char **argv) {
  initMALLOC();
  {
    char *root = GetSmvRootDir();
    int root_len = strlen(root);
    assert(root_len > 0);
    assert(root[root_len - 1] == dirseparator[0]);
  }
  {
    char *path = CombinePaths("1234/abc/efg", "hij/klm");
#ifdef _WIN32
    assert(strcmp(path, "1234/abc/efg\\hij/klm") == 0);
#else
    assert(strcmp(path, "1234/abc/efg/hij/klm") == 0);
#endif
  }
  {
    char *filename = "objects.svo";
    char *path = GetSmvRootFile(filename);
    assert(strcmp(LastName(path), filename) == 0);
  }
  {
    char *filename = "smokeview.ini";
    char *path = GetSmvRootFile(filename);
    assert(strcmp(LastName(path), filename) == 0);
  }
  {
    char *path = GetHomeDir();
    assert(path != NULL);
    assert(strlen(path) > 1);
  }
  {
    char *path = GetSmvRootFile("objects.svo");
    assert(path != NULL);
    assert(strlen(path) > 0);
  }
  return 0;
}
