all: heat.c hdf5IO.c
	h5pcc -Wall -Werror heat.c hdf5IO.c `pkg-config --cflags --libs glib-2.0` -o heat.out
	

run: all
	mpirun -np 4 ./heat.out 10 10 10
