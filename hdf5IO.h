#ifndef __HDF5IO__
#define __HDF5IO__

int openFile(int multiAccess, const char* format, ...);

void writeFrame(int id, double *data, int *arrayDims, int dataMargin, int *fileDims, int fileXOffset, int fileYOffset, const char* format, ...);

void closeFile(int id);

#endif
