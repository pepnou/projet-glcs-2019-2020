#ifndef __HDF5IO__
#define __HDF5IO__

int createFile(int multiAccess, const char* format, ...);

int openFile(int multiAccess, const char* format, ...);

void writeFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, int multiAccess, const char* format, ...);

void readFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, int multiAccess, const char* format, ...);

void closeFile(int id, int multiAccess);

void getDims(int id, int dims[2]);

int createGroup(int id, const char* format, ...);

void closeGroup(int id);

#endif
