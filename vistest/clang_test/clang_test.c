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

//access NULL array 
c=array[0];

//access undefined array
d=barray[0];

//access undefined array position
e=carray[0];

return;
// dead code
b=1;
c=1;
}
