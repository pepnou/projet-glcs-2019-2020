#include <hdf5.h>
#include <mpi.h>

#include <stdarg.h>
#include <glib/gprintf.h>
#include <string.h>


#define MAX_FILE_NUM 10
hid_t files[MAX_FILE_NUM];
hid_t plistIds[MAX_FILE_NUM];
hid_t multiAccessFiles[MAX_FILE_NUM];
int files_init = 0;

#define GET_NAME \
  char s[100]; \
  va_list arg; \
  va_start (arg, format); \
  g_vsprintf(s, format, arg); \
  va_end (arg);



hid_t H5Guard(hid_t ret) {
  if(ret < 0) {
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
  } else {
    return ret;
  }
}



int openFile(int multiAccess, const char* format, ...) {
  if(!files_init) {
    for(int i = 0; i < MAX_FILE_NUM; i++) {
      files[i] = -1;
    }
    files_init = 1;
  }

  GET_NAME

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  for(int i = 0; i < MAX_FILE_NUM; i++) {
    if(files[i] == -1) {
      if( multiAccess ) {
        plistIds[i] = H5Guard(H5Pcreate(H5P_FILE_ACCESS));
        H5Guard(H5Pset_fapl_mpio(plistIds[i], MPI_COMM_WORLD, MPI_INFO_NULL));
        
        files[i] = H5Guard(H5Fcreate(s, H5F_ACC_TRUNC, H5P_DEFAULT, plistIds[i]));
        
        multiAccessFiles[i] = 1;
      } else {
        files[i] = H5Guard(H5Fcreate(s, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT));
        
        multiAccessFiles[i] = 0;
      }

      return i;
    }
  }

  fprintf(stderr, "Too much files opened.\n");
  MPI_Abort(MPI_COMM_WORLD, 1);
  exit(1);
}



void writeFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, const char* format, ...) {
  GET_NAME
 
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);



  hsize_t arraySize[2]  = {arrayDims[0], arrayDims[1]};
  hsize_t memOffset[2]  = {dataMargin, dataMargin};

  hsize_t fileSize[2]   = {fileDims[0], fileDims[1]};
  hsize_t fileOffset[2] = {fileXOffset, fileYOffset};

  hsize_t dataSize[2]   = {arraySize[0] - 2 * dataMargin, arraySize[1] - 2 * dataMargin};


  hid_t mdataspace_id = 0;
  hid_t fdataspace_id = 0;
  hid_t dataset_id    = 0;

  mdataspace_id = H5Guard(H5Screate_simple(2, arraySize, NULL));
  fdataspace_id = H5Guard(H5Screate_simple(2, fileSize, NULL));
  

   
  dataset_id = H5Guard(H5Dcreate(files[id], s, H5T_NATIVE_DOUBLE, fdataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));



  H5Guard(H5Sselect_hyperslab(mdataspace_id, H5S_SELECT_SET, memOffset,  NULL, dataSize, NULL));
  H5Guard(H5Sselect_hyperslab(fdataspace_id, H5S_SELECT_SET, fileOffset, NULL, dataSize, NULL));


  if( multiAccessFiles[id] ) {
    hid_t plist_id = H5Guard(H5Pcreate(H5P_DATASET_XFER));
    H5Guard(H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE));

    H5Guard(H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, mdataspace_id, fdataspace_id, plist_id, data));
    
    H5Guard(H5Pclose(plist_id));
  } else {
    H5Guard(H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, mdataspace_id, fdataspace_id, H5P_DEFAULT, data));
  }


  H5Guard(H5Dclose(dataset_id));
  H5Guard(H5Sclose(mdataspace_id));
  H5Guard(H5Sclose(fdataspace_id));
}



void closeFile(int id) {
  if(multiAccessFiles[id]) {
    H5Guard(H5Pclose(plistIds[id]));
  }

  H5Guard(H5Fclose (files[id]));
  
  files[id] = -1;
}

