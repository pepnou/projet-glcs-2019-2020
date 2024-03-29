#include <hdf5.h>
#include <mpi.h>

#include <stdarg.h>
#include <glib/gprintf.h>
#include <string.h>

// stores all opened files and some properties
#define MAX_FILE_NUM 10
hid_t files[MAX_FILE_NUM];
hid_t plistIds[MAX_FILE_NUM];
int files_init = 0;

// return the string resulting of sprintf, but using va_list
#define GET_NAME \
  char s[100]; \
  va_list arg; \
  va_start (arg, format); \
  g_vsprintf(s, format, arg); \
  va_end (arg);


// check the return value of hdf5 value and handle errors
hid_t H5Guard(hid_t ret) {
  if(ret < 0) {
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
  } else {
    return ret;
  }
}


// open a file which name is define with (format, ...) using the same syntax as printf.
// set multiAccess to 1 if the file is going to be accessed by mutltiple processes
int createFile(int multiAccess, const char* format, ...) {
  // initialise the array of file id
  if(!files_init) {
    for(int i = 0; i < MAX_FILE_NUM; i++) {
      files[i] = -1;
    }
    files_init = 1;
  }
  
  // get the name of the file we're trying to open
  GET_NAME

  // get MPI rank
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  // Check the array of ids to find an empty slot
  for(int i = 0; i < MAX_FILE_NUM; i++) {
    if(files[i] == -1) {
      // if the file is going to be accessed by multiple processes
      if( multiAccess ) {
        // create access rules
        plistIds[i] = H5Guard(H5Pcreate(H5P_FILE_ACCESS));
        H5Guard(H5Pset_fapl_mpio(plistIds[i], MPI_COMM_WORLD, MPI_INFO_NULL));
        // create HDF5 file
        files[i] = H5Guard(H5Fcreate(s, H5F_ACC_TRUNC, H5P_DEFAULT, plistIds[i]));
      } else {
        // create HDF5 file
        files[i] = H5Guard(H5Fcreate(s, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT));
      }

      return i;
    }
  }

  // No space left in the ids array to open a new file
  fprintf(stderr, "Too much files or group opened.\n");
  MPI_Abort(MPI_COMM_WORLD, 1);
  exit(1);
}


// open a file which name is define with (format, ...) using the same syntax as printf.
// set multiAccess to 1 if the file is going to be accessed by mutltiple processes
int openFile(int multiAccess, const char* format, ...) {
  // initialise the array of file id
  if(!files_init) {
    for(int i = 0; i < MAX_FILE_NUM; i++) {
      files[i] = -1;
    }
    files_init = 1;
  }
  
  // get the name of the file we're trying to open
  GET_NAME

  // get MPI rank
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  // Check the array of ids to find an empty slot
  for(int i = 0; i < MAX_FILE_NUM; i++) {
    if(files[i] == -1) {
      // if the file is going to be accessed by multiple processes
      if( multiAccess ) {
        // create access rules
        plistIds[i] = H5Guard(H5Pcreate(H5P_FILE_ACCESS));
        H5Guard(H5Pset_fapl_mpio(plistIds[i], MPI_COMM_WORLD, MPI_INFO_NULL));
        // create HDF5 file
        files[i] = H5Guard(H5Fopen(s, H5F_ACC_RDONLY, plistIds[i]));
      } else {
        // open HDF5 file
        files[i] = H5Guard(H5Fopen(s, H5F_ACC_RDONLY, H5P_DEFAULT));
      }

      return i;
    }
  }

  // No space left in the ids array to open a new file
  fprintf(stderr, "Too much files opened.\n");
  MPI_Abort(MPI_COMM_WORLD, 1);
  exit(1);
}


// Write in the HDF5 file defined by id, in the dataset which name is defined with (format, ...) using the same syntax as printf,  a 2D array.
// dataMargin defines the size of the margin of the 2D array which is no going to be written in the file.
// fileDims defines the dimension of the file.
// The data array will be written in the file at the position defined by fileXOffset and fileYOffset
void writeFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, int multiAccess, const char* format, ...) {
  // get the name of the dataset we're writing in
  GET_NAME
 
  // get the MPI rank
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


  
  // initialise arrays defining size and offset for the dataspaces and hyperslabs
  hsize_t arraySize[2]  = {arrayDims[0], arrayDims[1]};
  hsize_t memOffset[2]  = {dataMargin, dataMargin};

  hsize_t fileSize[2]   = {fileDims[0], fileDims[1]};
  hsize_t fileOffset[2] = {fileXOffset, fileYOffset};

  hsize_t dataSize[2]   = {arraySize[0] - 2 * dataMargin, arraySize[1] - 2 * dataMargin};


  hid_t mdataspace_id = 0;
  hid_t fdataspace_id = 0;
  hid_t dataset_id    = 0;

  // create dataspace for the file and the memory
  mdataspace_id = H5Guard(H5Screate_simple(2, arraySize, NULL));
  fdataspace_id = H5Guard(H5Screate_simple(2, fileSize, NULL));
  


  // create the dataset
  dataset_id = H5Guard(H5Dcreate(files[id], s, H5T_NATIVE_DOUBLE, fdataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));



  // create the hyperslabs
  H5Guard(H5Sselect_hyperslab(mdataspace_id, H5S_SELECT_SET, memOffset,  NULL, dataSize, NULL));
  H5Guard(H5Sselect_hyperslab(fdataspace_id, H5S_SELECT_SET, fileOffset, NULL, dataSize, NULL));



  // write in the file
  if( multiAccess ) {
    hid_t plist_id = H5Guard(H5Pcreate(H5P_DATASET_XFER));
    H5Guard(H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE));

    H5Guard(H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, mdataspace_id, fdataspace_id, plist_id, data));
    
    H5Guard(H5Pclose(plist_id));
  } else {
    H5Guard(H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, mdataspace_id, fdataspace_id, H5P_DEFAULT, data));
  }


  //close dataset and dataspaces
  H5Guard(H5Dclose(dataset_id));
  H5Guard(H5Sclose(mdataspace_id));
  H5Guard(H5Sclose(fdataspace_id));
}



// Read from the HDF5 file defined by id, in the dataset which name is defined with (format, ...) using the same syntax as printf,  a 2D array.
// dataMargin defines the size of the margin of the 2D array which is no going to be written in the file.
// fileDims defines the dimension of the file.
// The data array will be written in the file at the position defined by fileXOffset and fileYOffset
void readFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, int multiAccess, const char* format, ...) {
  // get the name of the dataset we're writing in
  GET_NAME
 
  // get the MPI rank
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


  
  // initialise arrays defining size and offset for the dataspaces and hyperslabs
  hsize_t arraySize[2]  = {arrayDims[0], arrayDims[1]};
  hsize_t memOffset[2]  = {dataMargin, dataMargin};

  hsize_t fileSize[2]   = {fileDims[0], fileDims[1]};
  hsize_t fileOffset[2] = {fileXOffset, fileYOffset};

  hsize_t dataSize[2]   = {arraySize[0] - 2 * dataMargin, arraySize[1] - 2 * dataMargin};


  hid_t mdataspace_id = 0;
  hid_t fdataspace_id = 0;
  hid_t dataset_id    = 0;

  // create dataspace for the file and the memory
  mdataspace_id = H5Guard(H5Screate_simple(2, arraySize, NULL));
  fdataspace_id = H5Guard(H5Screate_simple(2, fileSize, NULL));
  


  // create the dataset
  dataset_id = H5Guard(H5Dopen(files[id], s, H5P_DEFAULT));



  // open the hyperslabs
  H5Guard(H5Sselect_hyperslab(mdataspace_id, H5S_SELECT_SET, memOffset,  NULL, dataSize, NULL));
  H5Guard(H5Sselect_hyperslab(fdataspace_id, H5S_SELECT_SET, fileOffset, NULL, dataSize, NULL));



  // read in the file
  if( multiAccess ) {
    hid_t plist_id = H5Guard(H5Pcreate(H5P_DATASET_XFER));
    H5Guard(H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE));

    H5Guard(H5Dread(dataset_id, H5T_NATIVE_DOUBLE, mdataspace_id, fdataspace_id, plist_id, data));
    
    H5Guard(H5Pclose(plist_id));
  } else {
    H5Guard(H5Dread(dataset_id, H5T_NATIVE_DOUBLE, mdataspace_id, fdataspace_id, H5P_DEFAULT, data));
  }


  //close dataset and dataspaces
  H5Guard(H5Dclose(dataset_id));
  H5Guard(H5Sclose(mdataspace_id));
  H5Guard(H5Sclose(fdataspace_id));
}



// Close the HDF5 file
void closeFile(int id, int multiAccess) {

  // close the access rules
  if( multiAccess ) {
    H5Guard(H5Pclose(plistIds[id]));
  }

  // close the file
  H5Guard(H5Fclose (files[id]));
  
  // set the id back to -1 to mark it free to be used again
  files[id] = -1;
}


void getDims(int id, int dims[2]) {
  hsize_t dim[2];

  hid_t dataset_id =  H5Guard(H5Dopen(files[id], "/step0", H5P_DEFAULT));

  hid_t dataspace_id = H5Guard(H5Dget_space(dataset_id));
  H5Guard(H5Sget_simple_extent_dims(dataspace_id, dim, NULL));

  H5Guard(H5Dclose(dataset_id));

  dims[0] = dim[0]; dims[1] = dim[1];
}

int createGroup(int id, const char* format, ...) {
  // get the name of the file we're trying to open
  GET_NAME

  // Check the array of ids to find an empty slot
  for(int i = 0; i < MAX_FILE_NUM; i++) {
    if(files[i] == -1) {
      files[i] = H5Guard(H5Gcreate( files[id], s, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
      return i;
    }
  }

  // No space left in the ids array to open a new file
  fprintf(stderr, "Too much files or groups opened.\n");
  MPI_Abort(MPI_COMM_WORLD, 1);
  exit(1);
}

void closeGroup(int id) {
  H5Guard(H5Gclose (files[id]));
  
  files[id] = -1;
}
