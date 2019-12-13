#ifndef __HDF5IO__
#define __HDF5IO__

int createFile(int multiAccess, const char* format, ...);

int openFile(int multiAccess, const char* format, ...);

void writeFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, const char* format, ...);

void readFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, const char* format, ...);

void closeFile(int id);

#endif
