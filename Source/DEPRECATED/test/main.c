#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct example {
  uint64_t i1;
  uint8_t b1;
  uint64_t i2;
  uint8_t b2;
  uint64_t i3;
  uint8_t b3;
    uint64_t i4;
  uint8_t b4;
  uint64_t i5;
  uint8_t b5;
  uint64_t i6;
  uint8_t b6;
  uint64_t i7;
  uint8_t b7;
  uint64_t i8;
  uint8_t b8;
};

/* ------------------ main ------------------------ */

int main(int argc, char **argv)

{
  // Warning: i is not modified so this is an endless loop. Not a compiler error
  // but almost certainly not what we want, hence a clang-tidy warning.
  for (int i = 0; i < 30;) {
    printf("i: %d",i);
  }
  // (Potential) Warning: by default clang-tidy warns against strcat, but this
  // is disabled in the ".clang-tidy" file as it's pervasive in smokeview and
  // would generate a lot of noise.
  char buf[256];
    strcat(buf, "some string");
}
