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
MPIEXTERN void HandleMPIMessages(int rank, int nprocs);

#define SMV_MPI_COMMAND      0
#define SMV_MPI_DATA         1

#define SMV_MPI_QUIT         0
#define SMV_MPI_LOAD_SLICE   1
#define SMV_MPI_LOAD_3DSMOKE 2
#define SMV_MPI_LOAD_BNDF    3
#define SMV_MPI_LOAD_PART    4
#define SMV_MPI_LOAD_PLOT3D  5
#define SMV_MPI_LOAD_ISO     6
#endif
