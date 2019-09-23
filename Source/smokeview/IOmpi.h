#ifndef IOMPI_H_DEFINED
#define IOMPI_H_DEFINED
#include <mpi.h>

#ifdef IN_IOMPI_C
#define  MPIEXTERN
#define MPIDECL(var,val)  var=val
#else
#define MPIEXTERN extern CCC
#define MPIDECL(var,val)  var
#endif

MPIEXTERN void TestMPI(void);
MPIEXTERN void HandleMPIMessages(void);

#endif
