#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#include <mpi.h>
#include "hdf5IO.h"

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
 
  // open heat.h5, diags.h5
  int id_heat = openFile(0, "heat.h5"),
      id_mean = createFile(0, "diags.h5");


  for(int i = 1; i < argc; i++) {
    int step = strtol(argv[i], NULL, 10);
    if(errno == EINVAL || errno == ERANGE) {
      MPI_Abort(MPI_COMM_WORLD, errno);
    }

    // read file(heat.h5, step, data)
    // calcul
    // write(diag, step, result)
    
    printf("%d\n", step);
  }

  closeFile(id_heat);
  closeFile(id_mean);


  MPI_Finalize();
}
