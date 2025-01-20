#include "options.h"
//#define pp_CLANG_TEST
#ifdef pp_CLANG_TEST
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ----------------------- test1 ----------------------------- */

void test1(void){
int a=0, b, c=0, d, e;
int *array=NULL, *barray, carray[3];

// divide by zero
b=1/a;

//use undefined value
c=c+d;

//access array 
c=array[0];
d=barray[0];
e=carray[0];

return;
// dead code
b=1;
c=1;
}
#endif
