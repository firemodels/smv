#include <stdio.h>
#include <time.h>

int main ()
{
  time_t rawtime;

  time ( &rawtime );
  printf ( "%i", (int)rawtime);

  return 0;
}