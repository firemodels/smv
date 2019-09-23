#ifndef IOMPI_H_DEFINED
#define IOMPI_H_DEFINED
#include <mpi.h>

#ifdef IN_IOMPIC
#define  MPIEXTERN
#define MPIDECL(var,val)  var=val
#else
#define MPIEXTERN extern CCC
#define MPIDECL(var,val)  var
#endif

MPIEXTERN void MPITest(void);

#endif
