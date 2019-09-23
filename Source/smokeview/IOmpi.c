#ifdef pp_MPI
#define IN_IOMPI_C
#include <stdio.h>
#include <stdlib.h>
#include "IOmpi.h"

/* ------------------ MPIMessages ------------------------ */

void HandleMPIMessages(int rank){
  MPI_Status Stat;
  int code;

  MPI_Recv(&code, 1, MPI_INT, 0, SMV_MPI_QUIT, MPI_COMM_WORLD, &Stat);
  MPI_Finalize();
  exit(code);
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
