#ifdef pp_MPI
#define IN_IOMPI_C
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include "IOmpi.h"
#include "ASSERT.h"
#include "smokeviewdefs.h"
#include "smokeviewvars.h"
#include "smokeheaders.h"

/* ------------------ MPIMessages ------------------------ */

void HandleMPIMessages(int rank, int nprocs){
  for(;;){
    MPI_Status Stat;
    int command[2];

    MPI_Recv(command, 2, MPI_INT, 0, SMV_MPI_COMMAND, MPI_COMM_WORLD, &Stat);
    switch(command[0]){
    case SMV_MPI_QUIT:
      MPI_Finalize();
      exit(command[1]);
      break;
    case SMV_MPI_LOAD_SLICE:
      LoadSliceMenu(command[1]);
      break;
    case SMV_MPI_LOAD_3DSMOKE:
      break;
    case SMV_MPI_LOAD_BNDF:
      break;
    case SMV_MPI_LOAD_PART:
      break;
    case SMV_MPI_LOAD_PLOT3D:
      break;
    case SMV_MPI_LOAD_ISO:
      break;
    default:
      ASSERT(FFALSE);
    }
  }
}

/* ------------------ MPITest ------------------------ */

void TestMPI(void){
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Print off a hello world message
  printf("Hello world from processor %s, rank %d out of %d processors\n",
    processor_name, world_rank, world_size);

  // Finalize the MPI environment.
  MPI_Finalize();

}
#endif
