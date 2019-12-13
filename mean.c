#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#include <mpi.h>
#include "hdf5IO.h"

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);


  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  printf("%d\n\n", size);
 
  // open heat.h5, diags.h5
  int id_heat = openFile(0, "heat.h5"),
      id_mean = createFile(0, "diags.h5");

  int fdims[2];
  getDims(id_heat, fdims);

  double* data = (double*)malloc(fdims[0] * fdims[1] * sizeof(double));

  printf("%d %d\n", fdims[0], fdims[1]);

  // select data block
  


  for(int i = 1; i < argc; i++) {
    int step = strtol(argv[i], NULL, 10);
    if(errno == EINVAL || errno == ERANGE) {
      MPI_Abort(MPI_COMM_WORLD, errno);
    }

    readFrame(id_heat, data, fdims, 0, fdims, 0, 0, "/step%d", step);

    for(int i = 0; i < fdims[0]; i++) {
      for(int j = 0; j < fdims[1]; j++) {
        printf("%3.3lf ", data[i*fdims[1]+j]);
      }
      printf("\n");
    }

    // calcul
    

    // write(diag, step, result)
    
    printf("%d\n", step);
  }

  closeFile(id_heat);
  closeFile(id_mean);


  MPI_Finalize();
}
