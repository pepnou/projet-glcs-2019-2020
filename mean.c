#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <mpi.h>
#include "hdf5IO.h"

void Mean(double* data, int mdims[2], int fdims[2], int yOffset, double *xmean, double *ymean) {
  for(int y = 0; y < mdims[0]; y++) {
    for(int x = 0; x < mdims[1]; x++) {
      xmean[y] += data[y*mdims[1] + x];
      ymean[x + yOffset + 1] += data[y*mdims[1] + x];
      ymean[0] += data[y*mdims[1] + x];
    }
  }

  for(int i = 0; i < mdims[0]; i++) {
    xmean[i] /= mdims[1];
  }

  for(int i = 0; i < mdims[1]; i++) {
    ymean[i + yOffset + 1] /= fdims[0];
  }

  ymean[0] /= (fdims[0] * fdims[1]);

  

 
  double *ymean_cpy = (double*)calloc(fdims[1] + 1, sizeof(double));

  MPI_Reduce(ymean, ymean_cpy, fdims[1] + 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  memcpy(ymean, ymean_cpy, (fdims[1] + 1) * sizeof(double));
  free(ymean_cpy);
}

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);


  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("%d %d\n\n", rank, size);
 
  // open heat.h5, diags.h5
  int id_heat = openFile(1, "heat.h5"),
      id_mean = createFile(1, "diags.h5");

  int fdims[2];
  getDims(id_heat, fdims);
  
  int mdims[2];
  mdims[1] = fdims[1];
  if(fdims[0] % size) {
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  mdims[0] = fdims[0] / size;



  double* data  = (double*)malloc(mdims[0] * mdims[1] * sizeof(double));
  double *xmean = (double*)calloc(mdims[0], sizeof(double));
  double *ymean = (double*)calloc(fdims[1] + 1, sizeof(double));

  int meanSize[2]  = {1, 1};
  int xmeanSize[2] = {mdims[0], 1};
  int ymeanSize[2] = {fdims[1], 1};

  //printf("%d %d\n", fdims[0], fdims[1]);

  




  for(int i = 1; i < argc; i++) {
    int step = strtol(argv[i], NULL, 10);
    if(errno == EINVAL || errno == ERANGE) {
      MPI_Abort(MPI_COMM_WORLD, errno);
    }
    
    int group_id = createGroup(id_mean, "/%d", step);

    readFrame(id_heat, data, mdims, 0, fdims, mdims[0] * rank, 0, 1, "/step%d", step);

    Mean(data, mdims, fdims, mdims[0] * rank, xmean, ymean);

    if(rank == 0) {
      writeFrame(group_id, ymean, meanSize, 0, meanSize, 0, 0, 0,"./mean");
    }


    writeFrame(group_id, xmean, xmeanSize, 0, xmeanSize, 0, 0, 1,"./x_mean");
    writeFrame(group_id, &ymean[1], ymeanSize, 0, ymeanSize, 0, 0, 1,"./y_mean");


    closeGroup(group_id);
  }

  closeFile(id_heat, 1);
  closeFile(id_mean, 1);


  MPI_Finalize();
}
