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
  return 0;
}
