#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#include <mpi.h>
#include "hdf5IO.h"

void Derivative(double* previous_data, double* data, int mdims[2]) {
  for(int i = 0; i < mdims[0]; i++) {
    for(int j = 0; j < mdims[1]; j++) {
      data[i*mdims[1] + j] -= previous_data[i*mdims[1] + j];
    }
  }
}

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);


  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("%d %d\n\n", rank, size);
 
  // open heat.h5, diags.h5
  int id_heat = openFile(1, "heat.h5"),
      id_de = createFile(1, "diags.h5");

  int fdims[2];
  getDims(id_heat, fdims);
  
  int mdims[2];
  mdims[1] = fdims[1];
  if(fdims[0] % size) {
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  mdims[0] = fdims[0] / size;



  double* data  = (double*)malloc(mdims[0] * mdims[1] * sizeof(double));
  double* previous_data  = (double*)malloc(mdims[0] * mdims[1] * sizeof(double));
  


  for(int i = 1; i < argc; i++) {
    int step = strtol(argv[i], NULL, 10);
    if(errno == EINVAL || errno == ERANGE) {
      MPI_Abort(MPI_COMM_WORLD, errno);
    }
    
    int group_id = createGroup(id_de, "/%d", step);

    readFrame(id_heat, previous_data, mdims, 0, fdims, 0, mdims[0] * rank, 1, "/step%d", step-1);
    readFrame(id_heat, data         , mdims, 0, fdims, 0, mdims[0] * rank, 1, "/step%d", step);

    Derivative(previous_data, data, mdims);

    writeFrame(group_id, data, mdims, 0, fdims, 0, mdims[0] * rank, 1,"./derivative");

    closeGroup(group_id);
  }

  closeFile(id_heat, 1);
  closeFile(id_de, 1);


  MPI_Finalize();
}
